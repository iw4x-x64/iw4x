#pragma once

#include <atomic>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

#include <quill/Backend.h>
#include <quill/LogFunctions.h>
#include <quill/Logger.h>

#include <libiw4x/export.hxx>

namespace iw4x
{
  class logger
  {
  public:
    logger ();
    ~logger ();

    logger (const logger&) = delete;
    logger& operator= (const logger&) = delete;

    logger (logger&&) = delete;
    logger& operator= (logger&&) = delete;
  };

  extern class logger* logger;

  namespace log
  {
    namespace detail
    {
      inline std::atomic<quill::Logger*>&
      logger () noexcept
      {
        static std::atomic<quill::Logger*> instance (nullptr);
        return instance;
      }
    }

    inline quill::Logger*
    logger () noexcept
    {
      return detail::logger ().load (std::memory_order_acquire);
    }

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
    inline constexpr quill::LogLevel
      min_level (quill::LogLevel::TraceL3);
#else
    inline constexpr quill::LogLevel
      min_level (quill::LogLevel::Info);
#endif

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
    // string back to Quill.
    //
    template <quill::LogLevel L>
    struct stream_accumulator
    {
      quill::SourceLocation location_;
      std::ostringstream    stream_;
      bool                  active_;

      stream_accumulator (quill::SourceLocation location)
        : location_ (location),
          active_ (true) {}

      // Notice the move constructor and the active_ flag.
      //
      // As the compiler passes this temporary object down the operator chain,
      // it might move it. So we must clear the active flag on the moved-from
      // object, otherwise both the old and new objects will attempt to flush
      // the same log message when they are respectively destroyed.
      //
      stream_accumulator (stream_accumulator&& other) noexcept
        : location_ (other.location_),
          stream_ (std::move (other.stream_)),
          active_ (other.active_)
      {
        other.active_ = false;
      }

      stream_accumulator& operator= (stream_accumulator&&) = delete;

      ~stream_accumulator ()
      {
        if constexpr (L >= min_level)
        {
          if (active_)
          {
            quill::Logger* logger_ptr (logger ());

            if (logger_ptr != nullptr && logger_ptr->should_log_statement (L))
            {
              std::string message (stream_.str ());

              // We avoid pushing completely empty strings to the backend.
              //
              if (!message.empty ())
                quill::log (logger_ptr, "", L, "{}", location_, message);
            }
          }
        }
      }

      template <typename T>
      requires Streamable<T, decltype(stream_)>
      stream_accumulator&
      operator<< (T&& value)
      {
        if constexpr (L >= min_level)
          stream_ << std::forward<T> (value);

        return *this;
      }

      stream_accumulator&
      operator<< (std::ostream& (*manipulator) (std::ostream&))
      {
        if constexpr (L >= min_level)
          manipulator (stream_);

        return *this;
      }

      template <typename F>
      requires std::invocable<F, std::ostream&>
      stream_accumulator&
      operator<< (F&& func)
      {
        if constexpr (L >= min_level)
          std::forward<F> (func) (stream_);

        return *this;
      }
    };

    namespace detail
    {
      template <typename T, quill::LogLevel L>
      concept StreamableToAccumulator =
        requires (stream_accumulator<L>& accumulator,
                  std::remove_cvref_t<T> const& value)
      {
        { accumulator << value } -> std::same_as<stream_accumulator<L>&>;
      };

      // Capture the source location for the first operand of operator<<.
      //
      // Because C++ prohibits default arguments on overloaded operators, we
      // cannot inject std::source_location directly into the operator<<
      // signature. Instead, we capture the call site location during the
      // implicit conversion of the right-hand operand.
      //
      // That is, when evaluating 'log::error << "foo"', overload resolution selects
      // an operator<< that accepts this wrapper type. The compiler performs an
      // implicit conversion of the operand, invoking the wrapper's templated
      // constructor which specifies std::source_location::current () as a
      // default argument.
      //
      // Notice also that the payload is then type-erased via a function pointer
      // and forwarded to the stream accumulator.
      //
      template <quill::LogLevel L>
      struct first_arg
      {
        void const* payload_;

        void (*format_func_)  (stream_accumulator<L>&, void const*);
        void (*destroy_func_) (void*);

        alignas(std::max_align_t) std::byte storage_[128];

        quill::SourceLocation location_;

        template <typename T>
          requires StreamableToAccumulator<T, L>
        first_arg (T&& value,
                   quill::SourceLocation location =
                     quill::SourceLocation::current ()) noexcept
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

            new (&storage_) type (std::forward<T> (value));

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
            destroy_func_ =
              [] (void* p)
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
        first_arg& operator= (const first_arg&) = delete;

        first_arg (first_arg&&) = delete;
        first_arg& operator= (first_arg&&) = delete;
      };
    }

    template <quill::LogLevel L>
    struct severity
    {
      stream_accumulator<L>
      operator<< (detail::first_arg<L> arg) const
      {
        // Seed the accumulator with the source location and apply the first
        // argument's formatting function.
        //
        stream_accumulator<L> result (arg.location_);
        arg.format_func_ (result, arg.payload_);
        return result;
      }
    };

    inline constexpr severity<quill::LogLevel::TraceL3>  trace_l3 {};
    inline constexpr severity<quill::LogLevel::TraceL2>  trace_l2 {};
    inline constexpr severity<quill::LogLevel::TraceL1>  trace_l1 {};
    inline constexpr severity<quill::LogLevel::Debug>    debug {};
    inline constexpr severity<quill::LogLevel::Info>     info {};
    inline constexpr severity<quill::LogLevel::Notice>   notice {};
    inline constexpr severity<quill::LogLevel::Warning>  warning {};
    inline constexpr severity<quill::LogLevel::Error>    error {};
    inline constexpr severity<quill::LogLevel::Critical> critical {};

    struct rate_limiter
    {
      using clock      = std::chrono::steady_clock;
      using time_point = clock::time_point;
      using duration   = clock::duration;

      duration                interval_;
      std::atomic<time_point> last_time_;

      explicit
      rate_limiter (duration interval) noexcept
        : interval_ (interval),
          last_time_ (time_point::min ()) {}

      bool
      operator() () noexcept
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
