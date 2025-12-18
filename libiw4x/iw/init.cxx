#include <libiw4x/iw/init.hxx>

namespace iw4x
{
  namespace iw
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
          memset (reinterpret_cast<void*> (address), value, size);
        });
    }
  }
}
