#include <libiw4x/frame/init.hxx>

#include <iostream>

#include <libiw4x/detour.hxx>

using namespace std;

namespace iw4x
{
  namespace frame
  {
    namespace
    {
      scheduler* sched (nullptr);

      void
      com_frame_try_block_function ()
      {
        struct poll
        {
          ~poll ()
          {
            if (sched != nullptr)
            {
              // Note that we lie here. The engine's nominal frame boundary is
              // Com_Frame, but the actual control path is mediated by
              // Com_Frame_Try_Block_Function. We instrument the latter.
              //
              sched->poll ("com_frame");
            }
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

    void
    init (scheduler& s)
    {
      sched = &s;

      s.create ("com_frame");

      detour (Com_Frame_Try_Block_Function, &com_frame_try_block_function);
    }
  }
}
