#pragma once

#include <libiw4x/import.hxx>
#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace client
  {
    LIBIW4X_SYMEXPORT bool
    live_xsync_with_cloud (int controllerIndex);

    LIBIW4X_SYMEXPORT bool
    live_xbase_game_license_check (int controllerIndex);
  }
}
