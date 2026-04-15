#pragma once

#include <optional>

#include <libiw4x/mod/oob/oob-envelope.hxx>
#include <libiw4x/mod/oob/oob-message.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      std::optional<oob_message>
      parse_message (const oob_envelope& envelope);
    }
  }
}
