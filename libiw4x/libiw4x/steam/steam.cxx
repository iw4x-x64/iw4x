#include <libiw4x/steam/steam.hxx>

#include <steam/steam_api.h>
#include <steam/steam_api_flat.h>

#include <libiw4x/steam/callbacks.hxx>

#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace steam
  {
    namespace
    {
      ISteamUser*         g_user         = nullptr;
      ISteamFriends*      g_friends      = nullptr;
      ISteamMatchmaking*  g_matchmaking  = nullptr;
      ISteamUtils*        g_utils        = nullptr;
      HSteamPipe          g_pipe         = 0;
      steam_id            g_local_id;
      bool                g_initialized  = false;
    }

    bool
    initialize ()
    {
      if (g_initialized)
        return true;

      SteamErrMsg err_msg {};
      ESteamAPIInitResult r (SteamAPI_InitFlat (&err_msg));

      if (r != k_ESteamAPIInitResult_OK)
      {
        log::error << "steam: SteamAPI_InitFlat failed: "
                   << static_cast<const char*> (err_msg);
        return false;
      }

      // Switch to manual dispatch. We want to be in the driver's seat when it
      // comes to when callbacks fire, rather than having them implicitly
      // triggered under the hood by some unrelated API call.
      //
      SteamAPI_ManualDispatch_Init ();

      g_user        = SteamAPI_SteamUser ();
      g_friends     = SteamAPI_SteamFriends ();
      g_matchmaking = SteamAPI_SteamMatchmaking ();
      g_utils       = SteamAPI_SteamUtils ();
      g_pipe        = SteamAPI_GetHSteamPipe ();

      if (!g_user || !g_friends || !g_matchmaking || !g_utils || !g_pipe)
      {
        log::error << "steam: failed to acquire one or more interface pointers";
        SteamAPI_Shutdown ();
        return false;
      }

      g_local_id = steam_id (SteamAPI_ISteamUser_GetSteamID (g_user));
      g_initialized = true;

      log::info << "steam: initialized (user " << g_local_id.value << ")";
      return true;
    }

    void
    shutdown ()
    {
      if (!g_initialized)
        return;

      g_initialized = false;
      g_user        = nullptr;
      g_friends     = nullptr;
      g_matchmaking = nullptr;
      g_utils       = nullptr;
      g_pipe        = 0;
      g_local_id    = {};

      SteamAPI_Shutdown ();

      log::info << "steam: shut down";
    }

    void
    pump_callbacks ()
    {
      if (!g_initialized)
        return;

      SteamAPI_ManualDispatch_RunFrame (g_pipe);

      CallbackMsg_t msg {};

      while (SteamAPI_ManualDispatch_GetNextCallback (g_pipe, &msg))
      {
        // See if this is an async call completion. If so, we need to extract
        // the real result. The API gives us a generic completion message, so we
        // allocate a small buffer on the stack to hold the actual payload
        // before handing it off to the dispatcher.
        //
        if (msg.m_iCallback == k_SteamAPICallCompleted)
        {
          auto completed (
            reinterpret_cast<SteamAPICallCompleted_t*> (msg.m_pubParam));

          auto buf (alloca (completed->m_cubParam));
          bool failed (false);

          if (SteamAPI_ManualDispatch_GetAPICallResult (g_pipe,
                                                        completed->m_hAsyncCall,
                                                        buf,
                                                        completed->m_cubParam,
                                                        completed->m_iCallback,
                                                        &failed))
          {
            if (!failed)
              dispatch_callback (completed->m_iCallback, buf);
          }
        }
        else
        {
          // Standard callback. Just dispatch it directly.
          //
          dispatch_callback (msg.m_iCallback, msg.m_pubParam);
        }

        SteamAPI_ManualDispatch_FreeLastCallback (g_pipe);
      }
    }

    ISteamUser*        user ()        noexcept { return g_user; }
    ISteamFriends*     friends ()     noexcept { return g_friends; }
    ISteamMatchmaking* matchmaking () noexcept { return g_matchmaking; }
    ISteamUtils*       utils ()       noexcept { return g_utils; }
    HSteamPipe         pipe ()        noexcept { return g_pipe; }
    steam_id           local_id ()    noexcept { return g_local_id; }
  }
}
