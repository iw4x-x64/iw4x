#pragma once

#include <libiw4x/import.hxx>
#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace client
  {
    LIBIW4X_SYMEXPORT bool
    live_require_user_to_play_online (int controllerIndex);
  }
}
