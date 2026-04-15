#include <libiw4x/logger.hxx>

#include <quill/Frontend.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/RotatingFileSink.h>

using namespace std;
using namespace quill;

namespace iw4x
{
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
      "%(time) [%(log_level_short_code)] %(logger:<16) %(caller_function:<32) "
      "%(short_source_location:<24) %(message)",
      "%H:%M:%S.%Qms",
      Timezone::LocalTime);

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

    log::detail::logger ().store (l, std::memory_order_release);
  }

  logger::
  ~logger ()
  {
    // Wipe the cached logger pointer to prevent dangling references.
    //
    log::detail::logger ().store (nullptr, std::memory_order_release);

    // Halt the asynchronous logging worker thread safely.
    //
    Backend::stop ();
  }
}
