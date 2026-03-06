#include <libiw4x/mod/mod-scheduler.hxx>

#include <cstdint>

#include <libiw4x/detour.hxx>
#include <libiw4x/scheduler.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    namespace
    {
      int64_t
      com_frame_try_block_function ()
      {
        struct poll
        {
          ~poll ()
          {
            // Note that we lie here. The engine's nominal frame boundary is
            // Com_Frame, but the actual control path is mediated by
            // Com_Frame_Try_Block_Function. We instrument the latter.
            //
            scheduler::get<com_frame_domain> ().tick ();
          }
        };

        poll p;

        return Com_Frame_Try_Block_Function ();
      }
    }

    scheduler_module::
    scheduler_module ()
    {
      detour (Com_Frame_Try_Block_Function, &com_frame_try_block_function);
    }
  }
}
