#include <libiw4x/mod/oob/oob-parser.hxx>

#include <cassert>
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
        // Parse individual message types.
        //
        // We pass the decoded envelope to each of these so they can produce a
        // typed message.

        optional<oob_message>
        parse_ping (const oob_envelope& e)
        {
          log::trace_l3 << "parsing ping message payload";
          return oob_ping_message (e.source);
        }

        optional<oob_message>
        parse_pong (const oob_envelope& e)
        {
          log::trace_l3 << "parsing pong message payload";
          return oob_pong_message (e.source);
        }

        // Map message ids to their parse functions. Note that every registered
        // oob_message_id must have a corresponding entry here.
        //
        using pfn = optional<oob_message> (*) (const oob_envelope&);

        const unordered_map<oob_message_id, pfn> tab {
          {oob_message_id::ping, parse_ping},
          {oob_message_id::pong, parse_pong}};
      }

      optional<oob_message>
      parse_message (const oob_envelope& e)
      {
        const auto i (tab.find (e.id));

        // Any id that reaches us here was already vetted by parse_envelope (),
        // which means it is definitely present in the command registry. If we
        // don't find it in our table, someone forgot to update it when adding a
        // new message type.
        //
        assert (i != tab.end ());

        const auto r (i->second (e));

        if (!r)
          log::debug << "type-specific parser rejected oob payload";
        else
          log::trace_l2 << "parsed typed oob message";

        return r;
      }
    }
  }
}
