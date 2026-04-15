#pragma once

#include <optional>

#include <libiw4x/mod/oob/oob-types.hxx>

#include <libiw4x/import.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      struct oob_envelope
      {
        oob_source_endpoint source;
        oob_message_id      id;
        oob_raw_payload     payload;
      };

      // Attempt to decode a raw OOB packet into a typed envelope.
      //
      // Note that encountering an unknown command word is not strictly an
      // error condition from the caller's perspective. Instead, we return
      // nullopt to signal the pipeline that this packet should simply be
      // forwarded to the engine.
      //
      std::optional<oob_envelope>
      parse_envelope (const network_address&, const message&);
    }
  }
}
