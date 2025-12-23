#include <libiw4x/client/init.hxx>

#include <libiw4x/client/live.hxx>
#include <libiw4x/client/live-gdk.hxx>
#include <libiw4x/client/live-win.hxx>

namespace iw4x
{
  namespace client
  {
    void
    init ()
    {
      ([] (void (*_) (uintptr_t, int, size_t))
        {
          // Disable Sys_CheckCrashOrRerun in WinMain.
          //
          _ (0x1402A92B3, 0x90, 13);

          // Skip com_safemode checks in Com_Init.
          //
          // com_safemode is only ever set to true by Sys_CheckCrashOrRerun.
          //
          _ (0x1401FAC87, 0x90, 7);
          _ (0x1401FAC8E, 0xEB, 1);
        })
      ([] (uintptr_t address, int value, size_t size)
        {
          memwrite (reinterpret_cast<void*> (address), value, size);
        });

      // Patch s_cpuCount initialization to use hardware concurrency in
      // Sys_InitMainThread. (Dynamic)
      //
      *(uint32_t*) 0x14020DD06 = thread::hardware_concurrency ();

      // Patch Content_DoWeHaveContentPack
      //
      memcpy ((void*) 0x1402864F0, "\xB0\x01\xC3", 3);

      // Patch Live_IsSignedIn to always return true.
      //
      memcpy ((void*) 0x1402A6E40, "\x33\xC0\xFF\xC0\xC3", 5);

      // live.hxx
      //
      detour (Live_StartSigninAny, &live_start_signin_any);
      detour (Live_GetLocalClientName, &live_get_local_client_name);

      // live-gdk.hxx
      //
      detour (Live_XSyncWithCloud, &live_xsync_with_cloud);
      detour (Live_XBaseGameLicenseCheck, &live_xbase_game_license_check);

      // live-win.hxx
      //
      detour (Live_RequireUserToPlayOnline, &live_require_user_to_play_online);
    }
  }
}
