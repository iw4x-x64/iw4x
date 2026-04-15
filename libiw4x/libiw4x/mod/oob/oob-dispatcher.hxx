#pragma once

#include <functional>

#include <libiw4x/mod/oob/oob-message.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      class oob_dispatcher
      {
      public:
        oob_dispatcher () = default;

        oob_dispatcher (const oob_dispatcher&) = delete;
        oob_dispatcher& operator = (const oob_dispatcher&) = delete;

        // Register a handler for ping messages.
        //
        void
        on_ping (
          std::move_only_function<void (const oob_ping_message&)> handler);

        // Register a handler for pong messages.
        //
        void
        on_pong (
          std::move_only_function<void (const oob_pong_message&)> handler);

        // Dispatch a generic OOB message.
        //
        void
        dispatch (const oob_message&);

      private:
        std::move_only_function<void (const oob_ping_message&)> ping_handler_;
        std::move_only_function<void (const oob_pong_message&)> pong_handler_;
      };
    }
  }
}
