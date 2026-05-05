#pragma once

#include <atomic>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <source_location>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <libiw4x/export.hxx>

namespace iw4x
{
  class LIBIW4X_SYMEXPORT logger
  {
  public:
    logger ();
    ~logger ();

    logger (const logger&) = delete;
    logger& operator = (const logger&) = delete;

    logger (logger&&) = delete;
    logger& operator = (logger&&) = delete;
  };

  extern class logger* logger;

  namespace log
  {
    enum class level : uint8_t
    {
      trace_l3,
      trace_l2,
      trace_l1,
      debug,
      info,
      notice,
      warning,
      error,
      critical
    };

    // Minimum severity level.
    //
    // The idea here is that every log statement whose severity falls strictly
    // below this threshold is to be removed by the compiler entirely. That is,
    // the threshold is a compile-time constant so the optimizer can fold the
    // guarding if constexpr in each dispatch struct to a no-op.
    //
    // Notice that development builds open the full trace range.
    //
#if LIBIW4X_DEVELOP
    inline constexpr level min_level (level::trace_l3);
#else
    inline constexpr level min_level (level::info);
#endif

    namespace detail
    {
      LIBIW4X_SYMEXPORT bool
      should_log_statement (level l) noexcept;

      LIBIW4X_SYMEXPORT void
      emit (level l, const std::source_location& loc, const std::string& msg);
    }

    template <typename T, typename S>
    concept Streamable = requires (S& stream, T&& value) {
      {
        stream << std::forward<T> (value)
      } -> std::convertible_to<std::ostream&>;
    };

    // Accumulate stream output and flush to the logging backend upon
    // destruction.
    //
    // A fundamental limitation of C++ streams is that an expression evaluates
    // as a sequence of operator<< calls. The stream itself has no knowledge of
    // the full expression's boundary. In other words, it cannot reliably
    // determine when to flush the assembled message.
    //
    // To bridge this gap, we employ the proxy object workaround. Accessing the
    // logger returns a temporary instance of this accumulator, and subsequent
    // operator<< invocations buffer the output into it.
    //
    // The key here is that when execution reaches the end of the full
    // expression, C++ language rules naturally mandate the destruction of this
    // temporary. We rely on its destructor to submit the completely buffered
    // string back to our backend.
    //
    template <level L>
    struct stream_accumulator
    {
      std::source_location location_;
      std::ostringstream   stream_;
      bool                 active_;

      stream_accumulator (std::source_location location)
        : location_ (location), active_ (true) {}

      // Notice the move constructor and the active_ flag.
      //
      // As the compiler passes this temporary object down the operator chain,
      // it might move it. So we must clear the active flag on the moved-from
      // object, otherwise both the old and new objects will attempt to flush
      // the same log message when they are respectively destroyed.
      //
      stream_accumulator (stream_accumulator&& other) noexcept
        : location_ (std::move (other.location_)),
          stream_ (std::move (other.stream_)),
          active_ (other.active_)
      {
        other.active_ = false;
      }

      stream_accumulator&
      operator = (stream_accumulator&&) = delete;

      ~stream_accumulator ()
      {
        if constexpr (L >= min_level)
        {
          if (active_)
          {
            if (detail::should_log_statement (L))
            {
              std::string message (stream_.str ());

              // We avoid pushing completely empty strings to the backend.
              //
              if (!message.empty ())
                detail::emit (L, location_, message);
            }
          }
        }
      }

      template <typename T>
        requires Streamable<T, decltype (stream_)>
      stream_accumulator&
      operator << (T&& value)
      {
        if constexpr (L >= min_level)
          stream_ << std::forward<T> (value);

        return *this;
      }

      stream_accumulator&
      operator << (std::ostream& (*manipulator) (std::ostream&) )
      {
        if constexpr (L >= min_level)
          manipulator (stream_);

        return *this;
      }

      template <std::invocable<std::ostream&> F>
      stream_accumulator&
      operator << (F&& func)
      {
        if constexpr (L >= min_level)
          std::invoke (std::forward<F> (func), stream_);

        return *this;
      }
    };

    namespace detail
    {
      template <typename T, level L>
      concept StreamableToAccumulator =
        requires (stream_accumulator<L>& accumulator,
                  std::remove_cvref_t<T> const& value)
      {
        {
          accumulator << value
        } -> std::same_as<stream_accumulator<L>&>;
      };

      // Capture the source location for the first operand of operator<<.
      //
      // Because C++ prohibits default arguments on overloaded operators, we
      // cannot inject std::source_location directly into the operator<<
      // signature. Instead, we capture the call site location during the
      // implicit conversion of the right-hand operand.
      //
      // That is, when evaluating 'error << "foo"', overload resolution selects
      // an operator<< that accepts this wrapper type. The compiler performs an
      // implicit conversion of the operand, invoking the wrapper's templated
      // constructor which specifies std::source_location::current () as a
      // default argument.
      //
      // Notice also that the payload is then type-erased via a function pointer
      // and forwarded to the stream accumulator.
      //
      template <level L>
      struct first_arg
      {
        void const* payload_;

        void (*format_func_) (stream_accumulator<L>&, void const*);
        void (*destroy_func_) (void*);

        alignas (std::max_align_t) std::byte storage_ [128];

        std::source_location location_;

        template <typename T>
          requires StreamableToAccumulator<T, L>
        first_arg (T&& value,
                   std::source_location location =
                     std::source_location::current ()) noexcept
          : destroy_func_ (nullptr),
            location_ (location)
        {
          using type = std::remove_cvref_t<T>;

          // If the argument is an lvalue, we can safely store a pointer to it
          // since the expression's lifetime encompasses the log statement.
          //
          if constexpr (std::is_lvalue_reference_v<T>)
          {
            payload_ = std::addressof (value);

            format_func_ =
              [] (stream_accumulator<L>& accumulator, void const* p)
            {
              accumulator << *static_cast<type const*> (p);
            };
          }
          else
          {
            // For rvalues, we must move or copy them into our inline storage to
            // keep them alive until formatting occurs. Since storage_ is just a
            // raw uninitialized array of bytes, we cannot use a standard
            // assignment operator.
            //
            static_assert (
              sizeof (type) <= sizeof (storage_),
              "Type is too large for inline storage in first_arg.");

            std::construct_at (reinterpret_cast<type*> (&storage_),
                               std::forward<T> (value));

            payload_ = &storage_;

            format_func_ =
              [] (stream_accumulator<L>& accumulator, void const* p)
            {
              accumulator << *static_cast<type const*> (p);
            };

            // It might be tempting to think we can skip this explicit teardown
            // since the central logger lives for the duration of the
            // application. However, remember that this first_arg wrapper is a
            // temporary object instantiated for every single log statement.
            //
            // Because we bypassed the standard heap allocation with placement
            // new, the compiler assumes we are managing the object's lifetime
            // manually. If we do not explicitly invoke the destructor here, any
            // heap allocations managed by the temporary (such as the internal
            // buffer of a std::string) will be permanently orphaned when this
            // wrapper pops off the stack. Multiply that by thousands of log
            // statements, and we have a catastrophic memory leak.
            //
            // Thus, we capture a type-erased destruction function that uses
            // std::destroy_at to explicitly invoke the destructor of 'type'.
            //
            destroy_func_ = [] (void* p)
            {
              std::destroy_at (static_cast<type*> (p));
            };
          }
        }

        ~first_arg ()
        {
          if (destroy_func_ != nullptr)
            destroy_func_ (&storage_);
        }

        first_arg (const first_arg&) = delete;
        first_arg& operator = (const first_arg&) = delete;

        first_arg (first_arg&&) = delete;
        first_arg& operator = (first_arg&&) = delete;
      };
    }

    template <level L>
    struct severity
    {
      stream_accumulator<L>
      operator << (detail::first_arg<L> arg) const
      {
        // Seed the accumulator with the source location and apply the first
        // argument's formatting function.
        //
        stream_accumulator<L> result (arg.location_);
        arg.format_func_ (result, arg.payload_);
        return result;
      }
    };

    inline constexpr severity<level::trace_l3> trace_l3 {};
    inline constexpr severity<level::trace_l2> trace_l2 {};
    inline constexpr severity<level::trace_l1> trace_l1 {};
    inline constexpr severity<level::debug> debug {};
    inline constexpr severity<level::info> info {};
    inline constexpr severity<level::notice> notice {};
    inline constexpr severity<level::warning> warning {};
    inline constexpr severity<level::error> error {};
    inline constexpr severity<level::critical> critical {};

    struct rate_limiter
    {
      using clock      = std::chrono::steady_clock;
      using time_point = clock::time_point;
      using duration   = clock::duration;

      duration                interval_;
      std::atomic<time_point> last_time_;

      explicit
      rate_limiter (duration interval) noexcept
        : interval_ (interval), last_time_ (time_point::min ()) {}

      bool
      operator () () noexcept
      {
        time_point now (clock::now ());
        time_point last (last_time_.load (std::memory_order_acquire));

        // Allow the event if this is the first occurrence or if the elapsed
        // time exceeds our specified interval.
        //
        if (last == time_point::min () || now - last >= interval_)
        {
          // Attempt to update the last occurrence timestamp. If another thread
          // beats us to it, we fail the exchange and drop the event.
          //
          if (last_time_.compare_exchange_strong (last,
                                                  now,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed))
            return true;
        }

        return false;
      }
    };
  }
}
