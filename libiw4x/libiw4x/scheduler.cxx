#include <libiw4x/scheduler.hxx>

#include <cassert>

using namespace std;

namespace iw4x
{
  namespace
  {
    // Task retention policies.
    //
    // The idea here is that each function encodes the re-enqueue decision for a
    // particular scheduling mode. We store them as simple function pointers in
    // the scheduled_entry and call them after the task executes.

    // Unconditional retention for the repeat_every_tick mode.
    //
    bool
    retain_always (scheduled_entry&)
    {
      return true;
    }
  }

  // The logical_scheduler implementation.
  //

  logical_scheduler::
  logical_scheduler ()
    : async_head_ (nullptr)
  {
    // Pre-allocate queue capacity to avoid heap allocations during the steady
    // state (tick).
    //
    // We assume 512 entries as a safe upper bound for a single frame. If we
    // exceed this, we will reallocate, which is fine, but we want to avoid it
    // in the common case.
    //
    pending_.reserve (512);
    active_.reserve (512);
  }

  logical_scheduler::
  ~logical_scheduler ()
  {
    // Drain and discard any remaining async nodes.
    //
    // Note that these may exist if the scheduler is destroyed before a final
    // tick has a chance to run (for example, during a fast process shutdown
    // where other threads might have still posted work).
    //
    async_node* n (async_head_.exchange (nullptr, memory_order_acquire));

    while (n != nullptr)
    {
      async_node* d (n);
      n = n->next;
      delete d;
    }
  }

  void logical_scheduler::
  post (task work)
  {
    assert (work);

    // Fast path: same-thread post.
    //
    // Push the task directly onto the pending queue.
    //
    scheduled_entry e;
    e.work = std::move (work);

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, repeat_every_tick)
  {
    assert (work);

    // Same-thread post for a repeating task.
    //
    // This is essentially the same as the regular same-thread post, but here
    // we attach the unconditional retention policy so that the task persists
    // across ticks.
    //
    scheduled_entry e;
    e.work = static_cast<task&&> (work);
    e.retain = &retain_always;

    pending_.push_back (static_cast<scheduled_entry&&> (e));
  }

  void logical_scheduler::
  post (task work, asynchronous)
  {
    assert (work);

    // Prepare the entry.
    //
    scheduled_entry e;
    e.work = std::move (work);

    // Allocate the node.
    //
    // Note that this is a heap allocation on the hot path for cross-thread
    // posts. While not ideal, it isolates the cost to the async boundary.
    //
    // @@ Look into a lock-free object pool if this proves to be a bottleneck.
    //
    async_node* n (new async_node);
    n->entry = std::move (e);

    // Push to the stack.
    //
    // The release semantics on success make the node contents visible to the
    // draining thread. Relaxed load on failure is fine, that is, we will
    // retry with the updated head immediately.
    //
    async_node* h (async_head_.load (memory_order_relaxed));

    for (;;)
    {
      n->next = h;

      if (async_head_.compare_exchange_weak (h,
                                             n,
                                             memory_order_release,
                                             memory_order_relaxed))
        break;
    }
  }

  void logical_scheduler::
  drain_async ()
  {
    // Pop the entire stack.
    //
    async_node* n (async_head_.exchange (nullptr, memory_order_acquire));

    if (n == nullptr)
      return;

    // The stack yields nodes in LIFO order. This is generally not what we want
    // for task execution (we prefer FIFO), so we reverse the list.
    //
    async_node* r (nullptr);
    while (n != nullptr)
    {
      async_node* nx (n->next);
      n->next = r;
      r = n;
      n = nx;
    }

    // Transfer entries into the local pending queue and free the nodes.
    //
    // Note that we do this in a batch to minimize vector resizing overhead,
    // though strictly speaking we are just moving the implementation details
    // of the loop here.
    //
    while (r != nullptr)
    {
      async_node* d (r);
      r = r->next; // advance before move or delete

      pending_.push_back (std::move (d->entry));
      delete d;
    }
  }

  void logical_scheduler::
  tick ()
  {
    // Thread ownership claim and verification.
    //
    // The first time we tick, we claim ownership of the scheduler. Subsequent
    // ticks must happen on the same thread to maintain the single-consumer
    // invariant for the vectors.
    //
    {
      namespace this_thread = std::this_thread;

      jthread::id tid (this_thread::get_id ());

      if (owner_ == jthread::id {})
        owner_ = tid;
      else
        assert (owner_ == tid);
    }

    // Drain any pending cross-thread posts into our local pending buffer.
    //
    drain_async ();

    // Swap pending_ (which contains tasks accumulated since the last tick)
    // with active_ (which is empty).
    //
    // This allows us to iterate over active_ without holding any locks and
    // allows post() to safely append to pending_ while we are executing.
    //
    pending_.swap (active_);

    // Run the tasks. Note that if a task throws, we currently let it propagate
    // out of tick(), which will likely terminate the application. This is
    // intentional: tasks should handle their own exceptions or be
    // exception-free.
    //
    for (auto& e : active_)
    {
      e.work ();

      // Retention check.
      //
      // If the entry should persist (for example, repeating modes), we move it
      // back into the pending queue for the next tick. One-shot entries (where
      // retain is null) simply fall through and are discarded when the active
      // queue is cleared.
      //
      if (e.retain != nullptr && e.retain (e))
        pending_.push_back (static_cast<scheduled_entry&&> (e));
    }

    // Clear the executed tasks. Note that while this destroys the function
    // objects and resets the vector size, it keeps the capacity for the next
    // frame.
    //
    active_.clear ();
  }

  namespace scheduler
  {
    boost::asio::io_context&
    get_io_context ()
    {
      static boost::asio::io_context ioc;
      return ioc;
    }
  }
}
