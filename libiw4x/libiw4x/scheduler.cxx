#include <libiw4x/scheduler.hxx>

#include <cassert>

using namespace std;

namespace iw4x
{
  namespace
  {
    // Task retention policies.
    //
    // The idea here is that each function encodes the re-enqueue decision for
    // a particular scheduling mode. We store them as simple function pointers
    // in the scheduled_entry and call them after the task executes.
    //

    // Unconditional retention for the repeat_every_tick mode.
    //
    bool
    retain_always (scheduled_entry&)
    {
      return true;
    }

    // Retain the task as long as the current time is before the deadline.
    //
    // The comparison is strict so that the task fires one last time on the
    // tick where the deadline is actually crossed, and then is finally
    // discarded.
    //
    bool
    retain_until_deadline (scheduled_entry& entry)
    {
      return steady_clock::now () < entry.when;
    }

    // Retain the task as long as the predicate evaluates to false.
    //
    // When the predicate first returns true, the condition is considered
    // satisfied and the task is immediately discarded. Note that we actually
    // invoke the callable here.
    //
    bool
    retain_until_satisfied (scheduled_entry& entry)
    {
      return !entry.condition ();
    }

    // Readiness gates.
    //
    // Each function here encodes the pre-execution check for a particular
    // scheduling mode. We store them as function pointers in the
    // scheduled_entry and call them before the task executes. Returning true
    // means the task should fire this tick. Returning false means it should
    // be deferred without executing.
    //

    // Ready once the current time reaches or exceeds the activation time.
    //
    bool
    ready_after_activation (const scheduled_entry& entry)
    {
      return steady_clock::now () >= entry.when;
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
    // We assume 512 entries is a safe upper bound for a single frame. If we
    // happen to exceed this, the vector will simply reallocate. That is fine,
    // but we obviously want to avoid it in the common case.
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
    // tick has a chance to run. For example, during a fast process shutdown
    // where other threads might have still posted work.
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

    // Fast path for same-thread posts.
    //
    // Push the task directly onto the pending queue.
    //
    scheduled_entry e;
    e.work = std::move (work);

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, repeat_every_tick_t)
  {
    assert (work);

    // Same-thread post for a repeating task.
    //
    // This is essentially the same as the regular same-thread post, but here
    // we attach the unconditional retention policy so that the task persists
    // across ticks.
    //
    scheduled_entry e;
    e.work = std::move (work);
    e.retain = &retain_always;

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, repeat_until_time mode)
  {
    assert (work);

    scheduled_entry e;
    e.work = std::move (work);
    e.when = steady_clock::now () + mode.value;
    e.retain = &retain_until_deadline;

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, repeat_until_predicate mode)
  {
    assert (work);
    assert (mode.condition);

    scheduled_entry e;
    e.work = std::move (work);
    e.condition = std::move (mode.condition);
    e.retain = &retain_until_satisfied;

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, execute_after_duration mode)
  {
    assert (work);

    scheduled_entry e;
    e.work = std::move (work);
    e.when = steady_clock::now () + mode.value;
    e.ready = &ready_after_activation;

    pending_.push_back (std::move (e));
  }

  void logical_scheduler::
  post (task work, asynchronous_t)
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
    // draining thread. A relaxed load on failure is fine. That is, we will
    // just retry with the updated head immediately.
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
    // for task execution (we heavily prefer FIFO), so we reverse the list.
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
    // Note that we do this in a batch to minimize vector resizing overhead.
    // Though strictly speaking, we are just moving the implementation details
    // of the loop here.
    //
    while (r != nullptr)
    {
      async_node* d (r);
      r = r->next;

      pending_.push_back (std::move (d->entry));
      delete d;
    }
  }

  void logical_scheduler::
  tick ()
  {
    // Thread ownership claim and verification.
    //
    // The first time we tick, we implicitly claim ownership of the scheduler.
    // Subsequent ticks must happen on the exact same thread to maintain the
    // single-consumer invariant for the vectors.
    //
    {
      jthread::id tid (this_thread::get_id ());

      if (owner_ == jthread::id ())
        owner_ = tid;
      else
        assert (owner_ == tid);
    }

    // Drain any pending cross-thread posts into our local pending buffer.
    //
    drain_async ();

    // Swap the pending queue (which contains tasks accumulated since the last
    // tick) with the active queue (which is currently empty).
    //
    // This allows us to iterate over the active snapshot without holding any
    // locks. It also means post() can safely append to the pending queue
    // while we are executing without invalidating iterators.
    //
    pending_.swap (active_);

    // Run the tasks.
    //
    // Note that if a task throws, we currently let it propagate out of
    // tick(), which will likely terminate the application. This is
    // intentional. Tasks should handle their own exceptions or simply be
    // exception-free.
    //
    for (auto& e : active_)
    {
      // Readiness gate.
      //
      // If the entry is not ready (for example, a delayed task whose
      // activation time has not yet arrived), we defer it to the next
      // tick without executing.
      //
      if (e.ready != nullptr && !e.ready (e))
      {
        pending_.push_back (std::move (e));
        continue;
      }

      e.work ();

      // Retention check.
      //
      // If the entry should persist (like repeating modes), we move it back
      // into the pending queue for the next tick. One-shot entries (where
      // the retain policy is null) simply fall through and are discarded
      // when the active queue is cleared.
      //
      if (e.retain != nullptr && e.retain (e))
        pending_.push_back (std::move (e));
    }

    // Clear the executed tasks.
    //
    // Note that while this destroys the function objects and resets the
    // vector size, it deliberately keeps the capacity for the next frame.
    //
    active_.clear ();
  }

  namespace scheduler
  {
    boost::asio::io_context&
    get_io_context ()
    {
      static boost::asio::io_context io_ctx;
      return io_ctx;
    }
  }
}
