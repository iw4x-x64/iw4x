#include <libiw4x/mod/mod-oob.hxx>

#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <libiw4x/logger.hxx>
#include <libiw4x/scheduler.hxx>
#include <libiw4x/mod/oob/oob-pipeline.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    oob::oob_pipeline* active_pipeline (nullptr);

    extern "C"
    {
      bool
      oob_dispatch (const network_address* a, const message* m)
      {
        const oob::oob_disposition d (active_pipeline->process (*a, *m));
        return d != oob::oob_disposition::forward_to_engine;
      }

      void
      oob_dispatch_stub ();
    }

    oob_module::
    oob_module ()
    {
      static mod::oob::oob_dispatcher oob_dispatcher;
      static mod::oob::oob_pipeline   oob_pipeline (oob_dispatcher);

      active_pipeline = &oob_pipeline;

      // Install the OOB packet interception loop.
      //
      // Note that we bypass our standard detour utility for this hook. The
      // target site (CL_DispatchConnectionlessPacket) doesn't have a
      // conventional function prologue, and the dispatch path needs complete
      // control over the call frame.
      //
      {
        uintptr_t t (0x1400F6040);
        uintptr_t h (reinterpret_cast<uintptr_t> (&oob_dispatch_stub));

        array<unsigned char, 14> sequence
        {{
          static_cast<unsigned char> (0xFF),
          static_cast<unsigned char> (0x25),
          static_cast<unsigned char> (0x00),
          static_cast<unsigned char> (0x00),
          static_cast<unsigned char> (0x00),
          static_cast<unsigned char> (0x00),
          static_cast<unsigned char> (h       & 0xFF),
          static_cast<unsigned char> (h >>  8 & 0xFF),
          static_cast<unsigned char> (h >> 16 & 0xFF),
          static_cast<unsigned char> (h >> 24 & 0xFF),
          static_cast<unsigned char> (h >> 32 & 0xFF),
          static_cast<unsigned char> (h >> 40 & 0xFF),
          static_cast<unsigned char> (h >> 48 & 0xFF),
          static_cast<unsigned char> (h >> 56 & 0xFF)
        }};

        memmove (reinterpret_cast<void*> (t),
                 sequence.data (), sequence.size ());
      }
    }
  }
}
