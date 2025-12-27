#include <libiw4x/client/init.hxx>

#include <libiw4x/client/common.hxx>

namespace iw4x
{
  namespace client
  {
    void
    init ()
    {
      memwrite (0x1402A91E5, "\xB0\x01");                                       // Suppress XGameRuntimeInitialize call in WinMain
      memwrite (0x1402A91E7, 0x90, 3);                                          // ^

      memwrite (0x1402A6A4B, 0x90, 5);                                          // Suppress XCurl call in Live_Init
      memwrite (0x1402A6368, 0x90, 5);                                          // Suppress XCurl call in Live_Frame

      memwrite (0x1402A8CFE, 0x90, 5);                                          // Suppress GDK shutdown in Com_Quit_f (avoids crash)

      memwrite (0x1402A92B3, 0x90, 13);                                         // Suppress Sys_CheckCrashOrRerun call in WinMain
      memwrite (0x1401FAC87, 0x90, 7);                                          // Suppress com_safemode checks in Com_Init.
      memwrite (0x1401FAC8E, 0xEB, 1);                                          // ^

      memwrite (0x1402864F0, "\xB0\x01\xC3");                                   // Patch Content_DoWeHaveContentPack to return true

      // Patch s_cpuCount with hardware concurrency in Sys_InitMainThread.
      //
      // Note that this may violate implicit engine assumptions about CPU
      // topology. Current behavior is stable, but should be monitored for
      // regressions.
      //
      *(uint32_t*) 0x14020DD06 = thread::hardware_concurrency ();

      // Experimental.
      //
      #if LIBIW4X_DEVELOP
        memwrite(0x1402A5F70, 0x90, 3); // xboxlive_signed
        memwrite(0x1402A5F73, 0x74, 1); //
        memwrite(0x1400F5B86, 0xEB, 1); //
        memwrite(0x1400F5BAC, 0xEB, 1); //
        memwrite(0x14010B332, 0xEB, 1); //
        memwrite(0x1401BA1FE, 0xEB, 1); //

        memwrite(0x140271ED0, 0xC3, 1); // playlist
        memwrite(0x1400F6BC4, 0x90, 2); //

        memwrite(0x1400FC833, 0xEB, 1); // configstring
        memwrite(0x1400D2AFC, 0x90, 2); //

        memwrite(0x1400E4DA0, 0x33, 1); // stats
        memwrite(0x1400E4DA1, 0xC0, 1); //
        memwrite(0x1400E4DA2, 0xC3, 1); //
      #endif

      // scheduler
      //
      ctx.sched.create ("com_frame");

      // common.hxx
      //
      detour (Com_Frame_Try_Block_Function, &com_frame_try_block_function);
    }
  }
}
