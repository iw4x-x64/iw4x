#include <libiw4x/client/common.hxx>

namespace iw4x
{
  namespace client
  {
    void
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
          ctx.sched.poll ("com_frame");
        }
      };

      // Scheduler poll must run after the frame boundary.
      //
      poll p;

      // Call original Com_Frame_Try_Block_Function.
      //
      Com_Frame_Try_Block_Function ();
    }
  }
}
