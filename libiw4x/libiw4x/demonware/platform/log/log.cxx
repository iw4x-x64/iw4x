#include <libiw4x/demonware/platform/log/log.hxx>

#include <cstdarg>
#include <cstdio>
#include <cassert>

#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    void
    bd_log_message (int type,
                    const char* base_channel,
                    const char* channel,
                    const char* file,
                    const char* function,
                    int line,
                    const char* fmt,
                    ...)
    {
      va_list ap;
      va_start (ap, fmt);

      char b[512];
      va_list ac;
      va_copy (ac, ap);

      // Note that we must copy the argument list since vsnprintf will likely
      // consume it during our initial sizing pass.
      //
      const int n (vsnprintf (b, sizeof (b), fmt, ac));
      va_end (ac);

      assert (n >= 0);

      string s;
      string_view m;

      // See if the message fits into our stack buffer. If it does, we simply
      // point the view directly to it. Otherwise, vsnprintf has told us exactly
      // how much space we need. In this case, we fall back to a heap allocation
      // and perform a second pass.
      //
      if (static_cast<size_t> (n) < sizeof (b))
      {
        m = string_view (b, static_cast<size_t> (n));
      }
      else
      {
        s.resize (static_cast<size_t> (n));
        vsnprintf (s.data (), static_cast<size_t> (n) + 1, fmt, ap);
        m = s;
      }

      auto c ([] (const char* p) {return p ? p : "";});
      log::trace_l3 << format ("[{} {}] {}", c (base_channel), c (channel), m);

      va_end (ap);
    }
  }
}
