#include <libiw4x/mod/oob/oob-pipeline.hxx>

#include <libiw4x/logger.hxx>

#include <libiw4x/mod/oob/oob-envelope.hxx>
#include <libiw4x/mod/oob/oob-parser.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      oob_pipeline::
      oob_pipeline (oob_dispatcher& d)
        : dispatcher_ (d)
      {
      }

      oob_disposition oob_pipeline::
      process (const network_address& a, const message& m)
      {
        // First, see if this is even an OOB message we care about. If not, let
        // the engine deal with it.
        //
        const auto e (parse_envelope (a, m));

        if (!e)
        {
          log::trace_l3 << "forwarding unhandled packet to engine";
          return oob_disposition::forward_to_engine;
        }

        // Now try to extract the actual payload. If it's malformed, we just
        // drop it. There is no point in propagating garbage through the system.
        //
        const auto p (parse_message (*e));

        if (!p)
        {
          log::debug << "rejected malformed oob message payload";
          return oob_disposition::rejected;
        }

        // Queue it up. We do this to avoid tying up the network thread and to
        // process all messages in a predictable batch during the next tick.
        //
        {
          const lock_guard<mutex> l (mutex_);
          pending_.push_back (*p);
        }

        log::trace_l2 << "queued oob message for dispatch";
        return oob_disposition::consumed;
      }

      void oob_pipeline::
      tick ()
      {
        vector<oob_message> b;

        {
          const lock_guard<mutex> l (mutex_);
          b.swap (pending_);
        }

        if (!b.empty ())
          log::trace_l2 << "dispatching batch of " << b.size () << " messages";

        for (const oob_message& m : b)
          dispatcher_.dispatch (m);
      }
    }
  }
}
