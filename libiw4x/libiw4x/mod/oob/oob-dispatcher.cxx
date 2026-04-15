#include <libiw4x/mod/oob/oob-dispatcher.hxx>

#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      void oob_dispatcher::
      on_ping (move_only_function<void (const oob_ping_message&)> h)
      {
        log::trace_l1 << "registering oob ping handler";
        ping_handler_ = move (h);
      }

      void oob_dispatcher::
      on_pong (move_only_function<void (const oob_pong_message&)> h)
      {
        log::trace_l1 << "registering oob pong handler";
        pong_handler_ = move (h);
      }

      void oob_dispatcher::
      dispatch (const oob_message& m)
      {
        log::trace_l2 << "dispatching oob message";

        // Map the variant types to their respective handlers. We use a local
        // visitor struct rather than a generic overloaded lambda to keep the
        // routing logic explicitly tied to our class state.
        //
        // Note that std::visit will resolve the correct overload at compile
        // time. If a handler isn't registered for a given type, it is not
        // considered a fatal error or a resource leak, that is, we simply drop
        // the message which is a fairly common occurrence during the boundary
        // phases of connection setups and teardowns.
        //
        struct visitor
        {
          oob_dispatcher& self;

          void
          operator () (const oob_ping_message& m) const
          {
            if (self.ping_handler_)
            {
              log::trace_l1 << "handling oob ping message";
              self.ping_handler_ (m);
            }
            else
              log::debug << "unhandled oob ping message dropped";
          }

          void
          operator () (const oob_pong_message& m) const
          {
            if (self.pong_handler_)
            {
              log::trace_l1 << "handling oob pong message";
              self.pong_handler_ (m);
            }
            else
              log::debug << "unhandled oob pong message dropped";
          }
        };

        // Dispatch the message variant against our current state.
        //
        visit (visitor {*this}, m);
      }
    }
  }
}
