#pragma once

#include <array>
#include <cstdint>

#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace demonware
  {
    struct auth_ticket
    {
      uint64_t title_id;
      uint64_t user_id;

      char username[16];

      std::array<uint8_t, 128> session_key;
      std::array<uint8_t, 256> ticket_data;
    };

    // Local authentication service.
    //
    // Generates a fake auth ticket without contacting auth3.prod.demonware.net.
    // The ticket is structurally valid but contains locally-generated key
    // material.
    //
    class LIBIW4X_SYMEXPORT auth
    {
    public:
      auth ();

      static const auth_ticket&
      ticket ();
    };
  }
}
