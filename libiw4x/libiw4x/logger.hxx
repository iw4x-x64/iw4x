#pragma once

#include <chrono>
#include <sstream>
#include <type_traits>
#include <utility>

#include <quill/Backend.h>
#include <quill/LogFunctions.h>
#include <quill/Logger.h>

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
    namespace detail
    {
      inline quill::Logger*&
      logger () noexcept
      {
        static quill::Logger* p (nullptr);
        return p;
      }
    }

    inline quill::Logger*
    logger () noexcept
    {
      return detail::logger ();
    }

    // Minimum severity level.
    //
    // The idea here is that every log statement whose severity falls strictly
    // below this threshold is to be removed by the compiler entirely. That is,
    // the threshold is a compile-time constant so the optimizer can fold the
    // guarding if constexpr in each dispatch struct to a no-op.
    //
    // Note that development builds open the full trace range.
    //
  #if LIBIW4X_DEVELOP
    inline constexpr quill::LogLevel
      min_level (quill::LogLevel::TraceL3);
#else
    inline constexpr quill::LogLevel
      min_level (quill::LogLevel::Info);
#endif

    template <quill::LogLevel L>
    struct stream_accumulator;

    namespace detail
    {
      template <typename T, quill::LogLevel L>
      concept StreamableToAccumulator =
        requires (stream_accumulator<L>& a,
                  std::remove_reference_t<T> const& t)
      {
        {
          a << t
        };
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
      // constructor which specifies std::source_location::current() as a
      // default argument.
      //
      // Note also that the payload is then type-erased via a function pointer
      // and forwarded to the stream accumulator.
      //
      template <quill::LogLevel L>
      struct first_arg
      {
        void const* p_;
        void (*f_) (stream_accumulator<L>&, void const*);
        quill::SourceLocation l_;

        first_arg (
          StreamableToAccumulator<L> auto&& t,
          quill::SourceLocation l = quill::SourceLocation::current ()) noexcept
            : p_ (std::addressof (t)),
              f_ ([] (stream_accumulator<L>& a, void const* p)
          {
            a << *static_cast<std::remove_reference_t<decltype(t)> const*> (p);
          }), l_ (l) {}
      };
    }

    template <typename T, typename Stream>
    concept Streamable = requires (Stream& stream, T&& value)
    {
      {
        stream << std::forward<T> (value)
      };
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
      quill::SourceLocation l_;
      std::ostringstream    s_;
      bool                  a_ {true};

      stream_accumulator (quill::SourceLocation l)
        : l_ (l) {}

      // Notice the move constructor and the 'a_' (active) flag.
      //
      // As the compiler passes this temporary object down the operator chain,
      // it might move it, so we must clear the active flag on the moved-from
      // object, otherwise both the old and new objects will attempt to flush
      // the same log message when they are respectively destroyed.
      //
      stream_accumulator (stream_accumulator&& o) noexcept
        : l_ (o.l_), s_ (std::move (o.s_)), a_ (o.a_)
      {
        o.a_ = false;
      }

      ~stream_accumulator ()
      {
        if constexpr (L >= min_level)
        {
          if (a_)
          {
            quill::Logger* q (logger ());

            if (q != nullptr && q->should_log_statement (L))
            {
              std::string s (s_.str ());
              if (!s.empty ())
                quill::log (q, "", L, "{}", l_, s);
            }
          }
        }
      }

      stream_accumulator&
      operator << (auto&& t) requires Streamable<decltype(t), decltype(s_)>
      {
        if constexpr (L >= min_level)
          s_ << std::forward<decltype(t)> (t);
        return *this;
      }

      stream_accumulator&
      operator << (std::ostream& (*f)(std::ostream&))
      {
        if constexpr (L >= min_level)
          f (s_);
        return *this;
      }
    };

    template <quill::LogLevel L>
    struct severity
    {
      stream_accumulator<L>
      operator<< (detail::first_arg<L> a) const
      {
        stream_accumulator<L> r (a.l_);
        a.f_ (r, a.p_);
        return r;
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

      duration   i_; // interval
      time_point l_; // last

      explicit
      rate_limiter (duration d) noexcept
        : i_ (d), l_ (time_point::min ()) {}

      bool
      operator() () noexcept
      {
        time_point n (clock::now ());

        if (l_ == time_point::min () || n - l_ >= i_)
          return l_ = n, true;

        return false;
      }
    };
  }
}
