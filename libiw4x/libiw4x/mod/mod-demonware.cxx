#include <libiw4x/mod/mod-demonware.hxx>

#include <algorithm>
#include <cassert>
#include <cstdint>

#include <libiw4x/detour.hxx>

#include <libiw4x/demonware/bd-auth-service.hxx>
#include <libiw4x/demonware/bd-bandwidth-test-client.hxx>
#include <libiw4x/demonware/bd-lobby-service.hxx>
#include <libiw4x/demonware/bd-remote-task.hxx>
#include <libiw4x/demonware/bd-remote-task-manager.hxx>
#include <libiw4x/demonware/bd-storage.hxx>



#include <libiw4x/demonware/bd-lobby-service.hxx>
#include <libiw4x/demonware/bd-platform-log.hxx>

using namespace std;
using namespace iw4x::demonware;

namespace iw4x
{
  namespace mod
  {
    namespace
    {
      auto& uk_state                (*reinterpret_cast<int32_t*>  (0x141646308));
      auto& uk_disable              (*reinterpret_cast<bool*>     (0x1416462E9));
      auto& uk_session_signin       (*reinterpret_cast<uint8_t*>  (0x141A4DA7C));
      auto& uk_session_signin_state (*reinterpret_cast<int32_t*>  (0x141A4dA84));
      auto& uk_xprivileg_state      (*reinterpret_cast<uint8_t*>  (0x141A4A990 + 0xC0));
      auto& uk_xprivileg_cache      (*reinterpret_cast<uint8_t*>  (0x141A4A990 + 0xC0 + 2));
      auto& uk_retry_count          (*reinterpret_cast<int32_t*>  (0x1416462C4));
      auto& uk_timestamp            (*reinterpret_cast<int32_t*>  (0x1416462C8));

      auto& dw_lobby_service        (*reinterpret_cast<void**>    (0x1416462D0));
      auto& dw_socket_router        (*reinterpret_cast<void**>    (0x1416462D8));
      auto& dw_session              (*reinterpret_cast<void**>    (0x141a4a998));
      auto& dw_session_context      (*reinterpret_cast<uint64_t*> (0x141a4a9a0));

      void
      iwnet_frame (int controller)
      {
        // Force the state variable to 0x08. It is not entirely clear what this
        // state actually represents, but this is the value the game expects
        // here, which might simply be another GDK peculiarity.
        //
        uk_state = 8;

        // Clear the disable flag. The game sets this flag to true immediately
        // prior to calling into the dw service, which effectively blocks the
        // service from progressing.
        //
        // Note that its exact semantics are unclear, but like above this
        // appears to be another GDK peculiarity.
        //
        uk_disable = false;

        // Force what appears to be a session sign-in byte on every tick. While
        // its exact mechanics are not entirely confirmed, we know that without
        // it, Live_Frame skips some important logic.
        //
        uk_session_signin = 1;

        // Resolve Xbox Live sign-in state at 0x141A4dA84.
        //
        // The issue here is that the connectivity gate at 0x1402A6F40 (check 2)
        // calls into 0x1401B5AA0, which expects this state to be 3 (signed in).
        //
        // Under normal circumstances, this is handled for us by GDK sign-in
        // flow at 0x1401b62E0. But since we've disabled GDK, that
        // initialization flow never actually runs. We therefore have to
        // manually spoof the signed-in state here in order to get past the
        // gate.
        //
        uk_session_signin_state = 3;

        // Multiplayer privilege cache entry for controller 0.
        //
        // The third check in the connectivity gate at 0x1401b5E90 verifies our
        // multiplayer privileges by indexing into the session array located at
        // 0x141A4A990. The index is calculated as (privilege_id + 3) * 12, which
        // means for privilege 13 (XPRIVILEGE_MULTIPLAYER_SESSIONS), we land
        // exactly at offset 0xC0.
        //
        // Note also that byte +2 acts as a "checked" flag.
        //
        uk_xprivileg_state = 1, uk_xprivileg_cache = 1;

        if (static bool connected (false); connected)
        {
          connected = true;

          // Disable the automated retry mechanism.
          //
          // We are now driving the pump manually within our own loop, so the
          // background retry system and its associated timestamp tracking are
          // completely redundant.
          //
          uk_retry_count = 0, uk_timestamp = 0;

          Uk_OnConnected (controller);

          // Populate the session name AFTER live_on_connected, which
          // resets the session array.
          //
          auto sn (reinterpret_cast<char*> (0x141A4A998 + 0x30D4));
          strncpy (sn, auth_service::ticket ().username, 16 - 1), sn[16 - 1] = '\0';
          assert  (uk_session_signin == 1);
        }

        // We're done here so pump the task completion array.
        //
        DW_SendPush ();
      }

      struct bd_router
      {
        void*   vtable;
        uint8_t body[0x1F8];
      };

      static_assert (sizeof (bd_router) == 0x200);

      alignas (16) bd_router router {};

      [[gnu::ms_abi]] int64_t
      router_stub (void*)
      {
        return 0;
      }

      static constexpr size_t router_vtable_slots (64);
      void* router_vtable[router_vtable_slots];

      void
      router_init ()
      {
        auto stub (reinterpret_cast<void*> (&router_stub));

        for (auto& e : router_vtable)
          e = stub;

        router.vtable = router_vtable;
      }

      // Stub session object to satisfy the session checks.
      //
      // The gate at 0x1401B5A50 insists that the session pointer is non-null,
      // and 0x1401B5BE3 will blindly dereference [ptr + 0x28], which then must
      // be either null (resulting in a graceful fail) or point to a valid
      // refcounted object.
      //
      alignas (16) uint8_t session_stub[0x30] {};
    }

    demonware_module::
    demonware_module ()
    {
      demonware::auth_service ();
      demonware::lobby_service ();
      demonware::remote_task_manager ();
      demonware::storage ();
      demonware::bandwidth_test_client ();

      router_init ();

      dw_socket_router = &router;
      dw_lobby_service = &lobby_service::instance ();

      uk_state = 8;
      uk_disable = false;
      dw_session = session_stub;
      dw_session_context = auth_service::ticket ().user_id;

      detour (bdLogMessage, &bd_log_message);
      detour (IWNet_Frame,  &iwnet_frame);
    }
  }
}
