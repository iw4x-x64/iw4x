#pragma once

#include <libiw4x/import.hxx>
#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace client
  {
    LIBIW4X_SYMEXPORT bool
    live_start_signin_any (unsigned int);

    LIBIW4X_SYMEXPORT const char*
    live_get_local_client_name (int);
  }
}
