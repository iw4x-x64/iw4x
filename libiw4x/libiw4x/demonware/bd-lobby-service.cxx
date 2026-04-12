#include <libiw4x/demonware/bd-lobby-service.hxx>

#include <cassert>
#include <cstdint>
#include <cstring>

#include <libiw4x/detour.hxx>
#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    namespace
    {
      alignas (16) bd_lobby_connection   lobby_connection;
      alignas (16) bd_lobby_service_impl lobby_service_impl;

      // bd_lobby_service_impl virtual methods (4-slot vtable).
      //
      // Note that these slots match the game's vtable exactly at 0x1403DBAC0:
      //   [0] destructor   (0x14031C4F0)
      //   [1] onConnect    (0x14031D090)
      //   [2] pump         (0x140326620)
      //   [3] onDisconnect (0x14031D320)

      [[gnu::ms_abi]] void
      lobby_service_impl_destructor (bd_lobby_service_impl*)
      {
        // Since our singleton is statically allocated, there is nothing
        // to actually free here.
      }

      [[gnu::ms_abi]] void
      lobby_service_impl_on_connect (bd_lobby_service_impl*)
      {
        // The connection is always established in our emulation, so we
        // can just no-op this.
      }

      [[gnu::ms_abi]] void
      lobby_service_impl_pump (bd_lobby_service_impl*)
      {
        // Task completion is handled in the startTask hook instead, so
        // pumping does nothing.
      }

      [[gnu::ms_abi]] void
      lobby_service_impl_on_disconnect (bd_lobby_service_impl*)
      {
        // We never disconnect.
      }

      void* lobby_service_impl_vtable[4]
      {
        reinterpret_cast<void*> (&lobby_service_impl_destructor),
        reinterpret_cast<void*> (&lobby_service_impl_on_connect),
        reinterpret_cast<void*> (&lobby_service_impl_pump),
        reinterpret_cast<void*> (&lobby_service_impl_on_disconnect)
      };

      // bd_lobby_connection virtual methods.
      //
      // The game do virtual dispatch on the connection object during
      // destruction. We provide a destructor along with safe defaults for any
      // unidentified virtual slots.

      [[gnu::ms_abi]] int64_t
      lobby_connection_destructor (bd_lobby_connection*)
      {
        return 0;
      }

      [[gnu::ms_abi]] int64_t
      lobby_connection_stub (void*)
      {
        return 0;
      }

      static constexpr size_t lobby_connection_vtable_slots (32);
      void* lobby_connection_vtable[lobby_connection_vtable_slots];

      void
      lobby_connection_constructor ()
      {
        auto stub (reinterpret_cast<void*> (&lobby_connection_stub));

        for (auto& e : lobby_connection_vtable)
          e = stub;

        lobby_connection_vtable[0] = reinterpret_cast<void*> (&lobby_connection_destructor);
        lobby_connection.vtable = lobby_connection_vtable;
        lobby_connection.refcount = 0x7FFFFFFF; // never reaches zero

        memset (lobby_connection.body, 0, sizeof (lobby_connection.body));
      }

      // bdStorage.
      //
      // The game reads 0x00 (status) and 0x08 (bdLobbyConnection*).
      //
      // Keep in mind this must be non-null so the task-fire path in
      // LiveStorage_StartGetPlaylists can actually reach startTask.
      //
      struct bd_storage_stub
      {
        int32_t status;     // 0x00
        int32_t pad;        // 0x04
        void*   connection; // 0x08
      };

      static_assert (sizeof (bd_storage_stub) == 0x10);

      alignas (16) bd_storage_stub bd_storage {};

      void
      lobby_service_impl_constructor ()
      {
        memset (&lobby_service_impl, 0, sizeof (lobby_service_impl));

        lobby_service_impl.vtable = lobby_service_impl_vtable;
        lobby_service_impl.connection = &lobby_connection;

        bd_storage.status = 0;
        bd_storage.connection = &lobby_connection;

        lobby_service_impl.storage = &bd_storage;
      }

      // dw_service (0x140136210).
      //
      // The game normally creates the singleton lazily at 0x140714A50.
      //
      // TODO: move this elsewhere?

      using dw_sevice_t = void* (*) ();
      inline dw_sevice_t dw_sevice_original =
        reinterpret_cast<dw_sevice_t> (0x140136210);

      [[gnu::ms_abi]] void*
      dw_service ()
      {
        return &lobby_service_impl;
      }

      // Non-virtual member function hooks.
      //

      using bdLobbyServiceImplConnect_t = bool (*) (void*, void*, void*, bool);
      bdLobbyServiceImplConnect_t bdLobbyServiceImplConnect (
        reinterpret_cast<bdLobbyServiceImplConnect_t> (0x14031c750));

      [[gnu::ms_abi]] bool
      lobby_service_impl_connect (void*, void*, void*, bool)
      {
        return true;
      }

      using bdLobbyServiceImplDisconnect_t = void (*) (void*, int);
      bdLobbyServiceImplDisconnect_t bdLobbyServiceImplDisconnect (
        reinterpret_cast<bdLobbyServiceImplDisconnect_t> (0x14031c950));

      [[gnu::ms_abi]] void
      lobby_service_impl_disconnect (void*, int reason)
      {
        // We do not have anything to tear down.
        //
      }

      using bdLobbyServiceImplGetStatus_t = int32_t (*) (void*);
      bdLobbyServiceImplGetStatus_t bdLobbyServiceImplGetStatus (
        reinterpret_cast<bdLobbyServiceImplGetStatus_t> (0x14031cb10));

      [[gnu::ms_abi]] int32_t
      lobby_service_impl_get_status (void*)
      {
        return 2; // always BD_ONLINE
      }

      using bdLobbyServiceImplGetMatchmaking_t = void* (*) (void*);
      bdLobbyServiceImplGetMatchmaking_t bdLobbyServiceImplGetMatchmaking (
        reinterpret_cast<bdLobbyServiceImplGetMatchmaking_t> (0x14031c990));

      [[gnu::ms_abi]] void*
      lobby_service_impl_get_matchmaking (void*)
      {
        return lobby_service_impl.matchmaking;
      }

      using bdLobbyServiceImplGetTaskMgr_t = void* (*) (void*);
      bdLobbyServiceImplGetTaskMgr_t bdLobbyServiceImplGetTaskMgr (
        reinterpret_cast<bdLobbyServiceImplGetTaskMgr_t> (0x14031CA30));

      [[gnu::ms_abi]] void*
      lobby_service_impl_get_task_mgr (void*)
      {
        return lobby_service_impl.task_mgr;
      }

      using bdLobbyServiceImplGetPerformance_t = void* (*) (void*);
      bdLobbyServiceImplGetPerformance_t bdLobbyServiceImplGetPerformance (
        reinterpret_cast<bdLobbyServiceImplGetPerformance_t> (0x14031ca70));

      [[gnu::ms_abi]] void*
      lobby_service_impl_get_performance (void*)
      {
        return lobby_service_impl.performance;
      }

      using bdLobbyServiceImplGetStorage_t = void* (*) (void*);
      bdLobbyServiceImplGetStorage_t bdLobbyServiceImplGetStorage (
        reinterpret_cast<bdLobbyServiceImplGetStorage_t> (0x14031cff0));

      [[gnu::ms_abi]] void*
      lobby_service_impl_get_storage (void*)
      {
        // Return the sub-service located at +0x60. This safely points to
        // store_impl which is guaranteed non-null.
        //
        return lobby_service_impl.storage;
      }
    }

    lobby_service::
    lobby_service ()
    {
      lobby_connection_constructor ();
      lobby_service_impl_constructor ();

      detour (dw_sevice_original,                &dw_service);

      detour (bdLobbyServiceImplGetMatchmaking,  &lobby_service_impl_get_matchmaking);
      detour (bdLobbyServiceImplGetPerformance,  &lobby_service_impl_get_performance);
      detour (bdLobbyServiceImplGetStatus,       &lobby_service_impl_get_status);
      detour (bdLobbyServiceImplGetStorage,      &lobby_service_impl_get_storage);
      detour (bdLobbyServiceImplGetTaskMgr,      &lobby_service_impl_get_task_mgr);
      detour (bdLobbyServiceImplConnect,         &lobby_service_impl_connect);
      detour (bdLobbyServiceImplDisconnect,      &lobby_service_impl_disconnect);
    }

    bd_lobby_service_impl&
    lobby_service::instance ()
    {
      return lobby_service_impl;
    }
  }
}
