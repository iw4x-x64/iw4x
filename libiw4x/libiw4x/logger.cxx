#include <libiw4x/logger.hxx>

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogFunctions.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/RotatingFileSink.h>

using namespace std;
using namespace quill;

namespace iw4x
{
  namespace log::detail
  {
    inline atomic<quill::Logger*>&
    logger () noexcept
    {
      static atomic<quill::Logger*> instance (nullptr);
      return instance;
    }

    static constexpr quill::LogLevel
    to_quill_level (level l) noexcept
    {
      switch (l)
      {
        case level::trace_l3: return quill::LogLevel::TraceL3;
        case level::trace_l2: return quill::LogLevel::TraceL2;
        case level::trace_l1: return quill::LogLevel::TraceL1;
        case level::debug:    return quill::LogLevel::Debug;
        case level::info:     return quill::LogLevel::Info;
        case level::notice:   return quill::LogLevel::Notice;
        case level::warning:  return quill::LogLevel::Warning;
        case level::error:    return quill::LogLevel::Error;
        case level::critical: return quill::LogLevel::Critical;
        default:              return quill::LogLevel::Info;
      }
    }

    bool
    should_log_statement (level l) noexcept
    {
      quill::Logger* logger_ptr (logger ().load (memory_order_acquire));

      return logger_ptr != nullptr &&
             logger_ptr->should_log_statement (to_quill_level (l));
    }

    void
    emit (level l, const source_location& loc, const string& msg)
    {
      quill::Logger* logger_ptr (logger ().load (memory_order_acquire));

      if (logger_ptr != nullptr)
      {
        quill::SourceLocation quill_loc {loc.file_name (),
                                         loc.function_name (),
                                         static_cast<uint32_t> (loc.line ())};

        quill::log (logger_ptr, "", to_quill_level (l), "{}", quill_loc, msg);
      }
    }
  }

  class logger* logger (nullptr);

  logger::
  logger ()
  {
    Backend::start ({
      .enable_yield_when_idle               = true,
      .sleep_duration                       = 0ns,
      .wait_for_queues_to_empty_before_exit = false,
      .check_printable_char                 = {},
      .log_level_short_codes                =
      {
        "3", "2", "1", "D", "I", "N", "W", "E", "C", "B", "_"
      }
    });

    ConsoleSinkConfig c;
    c.set_colour_mode (ConsoleSinkConfig::ColourMode::Always);

    FileSinkConfig r;
    r.set_filename_append_option (FilenameAppendOption::StartDateTime);

    auto cs (Frontend::create_or_get_sink<ConsoleSink> ("cs",       c));
    auto fs (Frontend::create_or_get_sink<FileSink>    ("iw4x.log", r));

    PatternFormatterOptions pf (
      "[%(log_level_short_code)] %(short_source_location:<24) %(message)");

    Logger* l (Frontend::create_or_get_logger ("iw4x", {cs, fs}, pf));

    l->set_log_level (LogLevel::Info);

    // In development builds, we blow the doors wide open and allow all trace
    // statements through so internals are visible. The compile-time minimum
    // level already permits this in LIBIW4X_DEVELOP mode, so we just need to
    // drop the runtime threshold here so they actually hit the sinks.
    //
#if LIBIW4X_DEVELOP
    l->set_log_level (LogLevel::TraceL3);
#endif

    log::detail::logger ().store (l, memory_order_release);
  }

  logger::
  ~logger ()
  {
    // Wipe the cached logger pointer to prevent dangling references.
    //
    log::detail::logger ().store (nullptr, memory_order_release);

    // Halt the asynchronous logging worker thread safely.
    //
    Backend::stop ();
  }
}
