#pragma once

#include <variant>

#include <libiw4x/mod/oob/oob-types.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      struct oob_ping_message { oob_source_endpoint source; };
      struct oob_pong_message { oob_source_endpoint source; };

      using oob_message = std::variant<oob_ping_message, oob_pong_message>;
    }
  }
}
