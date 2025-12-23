#include <libiw4x/client/live-win.hxx>

namespace iw4x
{
  namespace client
  {
    bool
    live_start_signin_any (unsigned int)
    {
      return true;
    }

    LIBIW4X_SYMEXPORT const char*
    live_get_local_client_name (int)
    {
      const char* n (getenv("USERNAME"));
      return n ? n : "UnnamedPlayer";
    }
  }
}
