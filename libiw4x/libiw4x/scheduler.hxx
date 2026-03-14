#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <type_traits>

#include <boost/asio.hpp>

#include <libiw4x/export.hxx>

namespace iw4x
{
  // Domain.
  //
  struct com_frame_domain_t {};
  inline constexpr com_frame_domain_t com_frame_domain;

  // Time.
  //
  using steady_clock = std::chrono::steady_clock;
  using duration = steady_clock::duration;
  using time_point = steady_clock::time_point;

  struct duration_t
  {
    duration value;

    explicit duration_t (duration d)
      : value (d)
    {
      assert (value > duration::zero ());
    }
  };

  // Callable type aliases.
  //
  using task = std::move_only_function<void ()>;
  using predicate = std::move_only_function<bool ()>;

  // Constraints.
  //
  template <typename T>
  concept SchedulerDomain = std::is_empty_v<T> && std::is_trivial_v<T>;

  template <typename P>
  concept SchedulerPredicate = std::is_invocable_r_v<bool, P&>;

  template <typename F>
  concept SchedulerTask = std::is_invocable_v<std::decay_t<F>>;

  template <typename E>
  concept SchedulerExecutor = std::copy_constructible<std::decay_t<E>> &&
                              std::equality_comparable<std::decay_t<E>> &&
    requires (const std::decay_t<E>& e, void (*f) ())
  {
    e.execute (f);
  };

  template <typename A>
  concept SchedulerAllocator = std::copy_constructible<std::decay_t<A>> &&
    requires (std::decay_t<A>& a, std::size_t n)
  {
    typename std::allocator_traits<std::decay_t<A>>::value_type;
    a.allocate (n);
  };

  // Scheduling modes.
  //
  // Each mode is a distinct type that encodes the scheduling behavior at
  // compile time. The idea here is to avoid runtime flags in the hot path.
  // Instead, overload resolution naturally selects the correct enqueue
  // mechanism (like an atomic queue versus a local vector) based purely on
  // the mode type.
  //

  // Execute the task on the next tick of the target scheduler.
  //
  // This is the default behavior when no specific mode argument is provided.
  //
  struct immediate_t {};
  inline constexpr immediate_t immediate;

  // Execute on the next tick, but force the task through the atomic
  // cross-thread ingress queue.
  //
  // This mode must be used when posting from a thread that does not own the
  // target scheduler (a foreign thread). It is also safe, though slightly
  // less efficient, to use from the owning thread.
  //
  struct asynchronous_t {};
  inline constexpr asynchronous_t asynchronous;

  // Execute on every tick indefinitely.
  //
  // The task is re-enqueued after each execution and persists until the
  // scheduler is destroyed.
  //
  struct repeat_every_tick_t {};
  inline constexpr repeat_every_tick_t repeat_every_tick;

  // Execute on every tick until the wall-clock deadline is reached.
  //
  // The deadline is computed as now() + span at the moment of posting. Once
  // the deadline passes, the task is discarded after its final execution.
  //
  struct repeat_until_time : duration_t
  {
    using duration_t::duration_t;
  };

  // Execute on every tick until the predicate returns true.
  //
  // The predicate is evaluated after each execution. When it first returns
  // true, the task is discarded.
  //
  struct repeat_until_predicate
  {
    predicate condition;

    explicit
    repeat_until_predicate (SchedulerPredicate auto predicate_function)
      : condition (std::move (predicate_function))
    {
      assert (condition);
    }
  };

  // Execute once after the specified delay has elapsed.
  //
  // The activation time is computed as now() + delay at the moment of
  // posting. On each tick before the activation time, the task is deferred
  // without executing.
  //
  struct execute_after_duration : duration_t
  {
    using duration_t::duration_t;
  };

  // Scheduled entry.
  //
  // This is the internal representation of a unit of work. We unify all
  // scheduling modes into this single layout. That allows the pending and
  // active queues to store heterogeneous entries without much fuss.
  //
  // Note that you will rarely interact with this type directly. It gets
  // populated implicitly by logical_scheduler::post().
  //
  struct scheduled_entry
  {
    // The actual work to execute when this entry becomes ready.
    //
    task work;

    // Condition callable for the repeat_until_predicate mode. It is empty
    // for all other modes.
    //
    predicate condition;

    // Time point used by time-dependent modes.
    //
    // For execute_after_duration this is the activation time (the earliest
    // tick at which the task fires). For repeat_until_time this is the
    // deadline (the last tick at which the task fires). It is unused by
    // other modes.
    //
    time_point when;

    // Retention policy.
    //
    // We call this after execution to determine whether this entry should
    // survive to the next tick.
    //
    bool (*retain) (scheduled_entry&);

    // Readiness gate.
    //
    // We call this before execution to determine whether this entry should
    // fire on the current tick. A null pointer simply means the task is
    // always ready. When readiness returns false, the entry is deferred to
    // the next tick without executing.
    //
    bool (*ready) (const scheduled_entry&);

    scheduled_entry ()
      : when (), retain (nullptr), ready (nullptr) {}

    scheduled_entry (scheduled_entry&&) = default;
    scheduled_entry& operator = (scheduled_entry&&) = default;

    scheduled_entry (const scheduled_entry&) = delete;
    scheduled_entry& operator = (const scheduled_entry&) = delete;
  };

  // Exclusive entry.
  //
  // Often we encounter a situation where an event triggers an asynchronous
  // operation. If the event fires again before the first operation completes,
  // we generally do not want to spawn a second concurrent operation. Instead,
  // we just want to drop the new request on the floor.
  //
  // Essentially, this acts as a deduplicating wrapper around
  // boost::asio::co_spawn. The idea is to keep track of whether the coroutine
  // is currently executing and simply ignore subsequent spawn requests until
  // it finishes.
  //
  // Note that we also need to handle the lifetime correctly here. The
  // coroutine might temporarily outlive the object that spawned it. So we
  // keep the control block in a shared state. We also tie into the
  // cancellation mechanism so that if this entry object is destroyed, we
  // signal the in-flight operation to abort.
  //
  // Keep in mind that this is exclusively (pun intended) meant to be used
  // with our asio coroutine bridge.
  //
  class exclusive_entry
  {
  public:
    exclusive_entry ()
      : shared_state_ (std::make_shared<state> ()) {}

    ~exclusive_entry ()
    {
      shared_state_->cancellation_sig.emit (
        boost::asio::cancellation_type::all);
    }

    exclusive_entry (const exclusive_entry&) = delete;
    exclusive_entry& operator = (const exclusive_entry&) = delete;

    void
    spawn (SchedulerExecutor auto&& executor_instance,
           SchedulerTask     auto&& work_function)
    {
      bool e (false);

      if (!shared_state_->is_running
             .compare_exchange_strong (e, true, std::memory_order_acquire))
        return;

      auto w ([s (shared_state_),
               fn (std::forward<decltype (work_function)> (
                 work_function))] () mutable -> boost::asio::awaitable<void>
      {
        struct grd
        {
          std::shared_ptr<state> s;

          explicit grd (std::shared_ptr<state> st)
            : s (std::move (st)) {}

          ~grd ()
          {
            s->is_running.store (false, std::memory_order_release);
          }
        } g (s);

        co_await fn ();
      });

      boost::asio::co_spawn (
        std::forward<decltype (executor_instance)> (executor_instance),
        std::move (w),
        boost::asio::bind_cancellation_slot (
          shared_state_->cancellation_sig.slot (),
          boost::asio::detached));
    }

    bool
    is_running () const
    {
      return shared_state_->is_running.load (std::memory_order_acquire);
    }

  private:
    struct state
    {
      std::atomic<bool> is_running;
      boost::asio::cancellation_signal cancellation_sig;

      state ()
        : is_running (false) {}
    };

    std::shared_ptr<state> shared_state_;
  };

  // Logical scheduler.
  //
  // This is an independently tickable scheduler bound to a single owning
  // thread.
  //
  // The model here is explicit ownership. Each scheduler maintains its own
  // task queues and must be ticked explicitly by its owner. There are no
  // implicit wakeups or cross-scheduler interactions, except via the
  // asynchronous ingress queue.
  //
  // Note that ownership is claimed implicitly on the first call to tick()
  // and asserted on every subsequent call.
  //
  class LIBIW4X_SYMEXPORT logical_scheduler
  {
  public:
    logical_scheduler ();
    ~logical_scheduler ();

    logical_scheduler (const logical_scheduler&) = delete;
    logical_scheduler& operator = (const logical_scheduler&) = delete;

    logical_scheduler (logical_scheduler&&) = delete;
    logical_scheduler& operator = (logical_scheduler&&) = delete;

    // Posting.
    //

    // Schedule work to be executed on the next tick.
    //
    // This overload bypasses the atomic queue and appends directly to the
    // local pending buffer. It must only be called from the thread that
    // owns this scheduler.
    //
    void
    post (task work);

    // Schedule work from a foreign thread.
    //
    // This pushes the task onto the atomic ingress queue. This is safe to
    // call from any thread. The task will be drained into the local pending
    // queue at the start of the next tick.
    //
    void
    post (task work, asynchronous_t mode);

    // Schedule work to be executed on every tick.
    //
    void
    post (task work, repeat_every_tick_t mode);

    // Schedule work to be executed on every tick until a specified deadline.
    //
    void
    post (task work, repeat_until_time mode);

    // Schedule work to be executed on every tick until a condition is met.
    //
    void
    post (task work, repeat_until_predicate mode);

    // Schedule work to be deferred until a specified duration has elapsed.
    //
    void
    post (task work, execute_after_duration mode);

    // Execution.
    //

    // Execute all pending tasks for the current tick.
    //
    // The execution logic is double-buffered. We first drain the async
    // ingress queue into the pending buffer and then swap the pending buffer
    // with the active one. Once swapped, we iterate over the active
    // snapshot front-to-back.
    //
    void
    tick ();

  private:
    // Ingress queue details.
    //

    // Node for the lock-free MPSC ingress queue.
    //
    // Each cross-thread post allocates one node. The owning thread then
    // adopts and deallocates these nodes during the drain phase. Note that
    // this allocation is isolated to the async path. Same-thread posts
    // remain allocation-free (amortized).
    //
    struct async_node
    {
      async_node* next;
      scheduled_entry entry;
    };

    // Drain all nodes from the async ingress queue into the local pending
    // buffer.
    //
    // The drained list is LIFO, so we reverse it to restore FIFO ordering
    // before appending.
    //
    void
    drain_async ();

    // Head of the ingress queue.
    //
    // Foreign threads push here via CAS. The owning thread consumes the
    // entire list via atomic exchange.
    //
    std::atomic<async_node*> async_head_;

    // Local queues.
    //

    // Double-buffered local queues.
    //
    // The pending_ queue accumulates tasks between ticks and during the
    // current tick. The active_ queue holds the snapshot being processed.
    // We swap them at the start of the tick to keep the iteration range
    // stable.
    //
    std::vector<scheduled_entry> pending_;
    std::vector<scheduled_entry> active_;

    // Diagnostics.
    //

    // Identity of the thread that first ticked this scheduler.
    //
    // We use this to detect accidental cross-thread access to
    // non-thread-safe methods.
    //
    std::jthread::id owner_;
  };

  // Global registry.
  //
  // This provides a unified interface for locating schedulers based on
  // domain tags.
  //
  // We map domain tag types (for example, com_frame_domain) to
  // logical_scheduler instances using template-parameterized static locals.
  // The idea here is to avoid dynamic map lookup by making each domain
  // resolve to exactly one scheduler instance at link time.
  //
  namespace scheduler
  {
    // Retrieve the logical scheduler instance for the specified domain.
    //
    // The instance is created on first access.
    //
    template <SchedulerDomain Domain>
    logical_scheduler&
    get ()
    {
      static logical_scheduler s;
      return s;
    }

    // Dispatch a standard task to the domain-specific scheduler.
    //
    inline void
    post (SchedulerDomain auto domain_tag, task work)
    {
      get<decltype (domain_tag)> ().post (std::move (work));
    }

    // Dispatch an asynchronous task to the domain-specific scheduler.
    //
    inline void
    post (SchedulerDomain auto domain_tag, task work, asynchronous_t mode)
    {
      get<decltype (domain_tag)> ().post (std::move (work), mode);
    }

    // Dispatch a repeating task to the domain-specific scheduler.
    //
    inline void
    post (SchedulerDomain auto domain_tag, task work, repeat_every_tick_t mode)
    {
      get<decltype (domain_tag)> ().post (std::move (work), mode);
    }

    inline void
    post (SchedulerDomain auto domain_tag, task work, repeat_until_time mode)
    {
      get<decltype (domain_tag)> ().post (std::move (work), std::move (mode));
    }

    inline void
    post (SchedulerDomain auto domain_tag,
          task work,
          repeat_until_predicate mode)
    {
      get<decltype (domain_tag)> ().post (std::move (work), std::move (mode));
    }

    inline void
    post (SchedulerDomain auto domain_tag,
          task work,
          execute_after_duration mode)
    {
      get<decltype (domain_tag)> ().post (std::move (work), std::move (mode));
    }

    // Boost.Asio executor adapter.
    //
    // This bridges logical_scheduler to an execution context for Boost.Asio
    // operations. Coroutines spawned via co_spawn() with this executor will
    // automatically resume on the scheduler's tick (main thread) after a
    // background async operation completes.
    //
    template <SchedulerDomain Domain>
    class asio_executor
    {
    public:
      boost::asio::io_context* io_ctx;

      explicit asio_executor (boost::asio::io_context& ctx) noexcept
        : io_ctx (&ctx) {}

      asio_executor (const asio_executor&) noexcept = default;
      asio_executor& operator = (const asio_executor&) noexcept = default;

      bool
      operator == (const asio_executor& other) const noexcept
      {
        return io_ctx == other.io_ctx;
      }

      bool
      operator != (const asio_executor& other) const noexcept
      {
        return io_ctx != other.io_ctx;
      }

      boost::asio::io_context&
      context () const noexcept
      {
        return *io_ctx;
      }

      void
      dispatch (SchedulerTask auto work_function,
                const SchedulerAllocator auto& allocator_instance) const
      {
        iw4x::scheduler::post (Domain (),
                               std::move (work_function),
                               asynchronous);
      }

      void
      post (SchedulerTask auto work_function,
            const SchedulerAllocator auto& allocator_instance) const
      {
        iw4x::scheduler::post (Domain (),
                               std::move (work_function),
                               asynchronous);
      }

      void
      defer (SchedulerTask auto work_function,
             const SchedulerAllocator auto& allocator_instance) const
      {
        iw4x::scheduler::post (Domain (),
                               std::move (work_function),
                               asynchronous);
      }

      void
      execute (SchedulerTask auto work_function) const
      {
        iw4x::scheduler::post (Domain (),
                               std::move (work_function),
                               asynchronous);
      }

      boost::asio::io_context&
      query (boost::asio::execution::context_t) const noexcept
      {
        return *io_ctx;
      }

      asio_executor
      require (boost::asio::execution::blocking_t::never_t) const noexcept
      {
        return *this;
      }

      asio_executor
      require (boost::asio::execution::blocking_t::possibly_t) const noexcept
      {
        return *this;
      }

      asio_executor
      require (
        boost::asio::execution::outstanding_work_t::tracked_t) const noexcept
      {
        return *this;
      }

      asio_executor
      require (
        boost::asio::execution::outstanding_work_t::untracked_t) const noexcept
      {
        return *this;
      }

      asio_executor
      require (boost::asio::execution::relationship_t::fork_t) const noexcept
      {
        return *this;
      }

      asio_executor
      require (
        boost::asio::execution::relationship_t::continuation_t) const noexcept
      {
        return *this;
      }
    };

    LIBIW4X_SYMEXPORT boost::asio::io_context&
    get_io_context ();

    inline void
    spawn (SchedulerTask auto&& work_function)
    {
      static exclusive_entry e;
      e.spawn (scheduler::asio_executor<com_frame_domain_t> (get_io_context ()),
               std::forward<decltype (work_function)> (work_function));
    }

    inline void
    spawn (SchedulerExecutor auto&& executor_instance,
           SchedulerTask auto&& work_function)
    {
      static exclusive_entry e;
      e.spawn (std::forward<decltype (executor_instance)> (executor_instance),
               std::forward<decltype (work_function)> (work_function));
    }
  }
}
