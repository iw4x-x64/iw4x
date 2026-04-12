#pragma once

#include <cstdint>
#include <array>

namespace iw4x
{
  namespace demonware
  {
    struct bd_auth_ticket
    {
      uint64_t title_id, user_id;

      char username[16];

      std::array<uint8_t, 128> session_key;
      std::array<uint8_t, 256> ticket_data; // ?
    };
  }
}
