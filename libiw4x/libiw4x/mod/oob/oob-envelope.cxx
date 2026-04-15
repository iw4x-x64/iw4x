#include <libiw4x/mod/oob/oob-envelope.hxx>

#include <string_view>
#include <unordered_map>

#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      namespace
      {
        // Central registry mapping OOB command strings to typed message ids. We
        // do this at the edge so the rest of the pipeline only deals with
        // integers, not strings.
        //
        const unordered_map<string_view, oob_message_id> cmds ({
          {"ping", oob_message_id::ping},
          {"pong", oob_message_id::pong},
        });

        // The maximum command length we are willing to parse. If the first
        // token is longer than this, it's bogus and we reject it immediately.
        //
        constexpr size_t max_cmd (32);

        // The four-byte OOB header. Every valid out-of-band packet must start
        // with four of these. https://www.jfedor.org/quake3/
        //
        constexpr unsigned char hdr (0xFF);

        // At a bare minimum we need four header bytes and a one-character
        // command.
        //
        constexpr int min_sz (5);
      }

      optional<oob_envelope>
      parse_envelope (const network_address& a, const message& m)
      {
        log::trace_l3 << "parsing packet for oob envelope";

        // Check if we even have enough bytes to bother parsing.
        //
        if (m.data == nullptr || m.current_size < min_sz)
        {
          log::trace_l3 << "dropping undersized or null packet";
          return nullopt;
        }

        const auto* const b (reinterpret_cast<const unsigned char*> (m.data));

        // Verify the OOB header.
        //
        if (b [0] != hdr || b [1] != hdr || b [2] != hdr || b [3] != hdr)
        {
          log::trace_l3 << "packet lacks oob header";
          return nullopt;
        }

        // Find the command word. It's the first token right after the header
        // and is delimited by whitespace or null.
        //
        const char* p (m.data + 4);
        const char* e (m.data + m.current_size);

        const char* const s (p);

        while (p < e && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
          ++p;

        const size_t n (static_cast<size_t> (p - s));

        if (n == 0 || n > max_cmd)
        {
          log::debug << "malformed oob command length: " << n;
          return nullopt;
        }

        // Try to identify the command. If it's unknown we just return nullopt.
        //
        // Note that this is not an error, that is, the engine might know what
        // to do with it so the pipeline will forward it.
        //
        const string_view c (s, n);
        const auto i (cmds.find (c));

        if (i == cmds.end ())
        {
          log::trace_l2 << "forwarding unknown oob command to engine: " << c;
          return nullopt;
        }

        // Eat any whitespace separating the command from its payload.
        //
        while (p < e && (*p == ' ' || *p == '\t'))
          ++p;

        // The rest of the buffer is our raw payload.
        //
        const oob_raw_payload r (reinterpret_cast<const std::byte*> (p),
                                 static_cast<size_t> (e - p));

        log::trace_l2 << "parsed oob envelope for command: " << c;

        return oob_envelope (oob_source_endpoint (a), i->second, r);
      }
    }
  }
}
