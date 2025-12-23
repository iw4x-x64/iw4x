#include <libiw4x/client/live-win.hxx>

namespace iw4x
{
  namespace client
  {
    bool
    live_require_user_to_play_online (int controllerIndex)
    {
      // Note that for now this unconditionally returns true. It will later
      // become the semantic gate for online participation and is where the
      // call path is expected to block until an online session is actually
      // established. In other words, this means driving the connection state
      // machine and only returning once the backend has acknowledged the
      // client as authenticated and ready, rather than merely assuming online
      // availability.
      //
      return true;
    }
  }
}
