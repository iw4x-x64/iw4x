#pragma once

#include <span>

#include <libiw4x/import.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      enum class oob_message_id : std::uint16_t
      {
        ping = 0x0100,
        pong = 0x0101,
      };

      struct oob_source_endpoint
      {
        network_address address;
      };

      // The disposition tells the pipeline what to do with an incoming
      // network message after we have inspected it.
      //
      enum class oob_disposition
      {
        consumed,          // We handled it, drop it.
        forward_to_engine, // Not for us, let the game engine process it.
        rejected           // Invalid or malformed, drop it.
      };

      using oob_raw_payload = std::span<const std::byte>;
    }
  }
}
