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
      alignas (16) bd_lobby_connection connection;
      alignas (16) bd_lobby_service lobby;

      // bd_lobby_service virtual methods (4-slot vtable).
      //
      // Note that these slots match the game's vtable exactly at 0x1403DBAC0:
      //   [0] destructor   (0x14031C4F0)
      //   [1] onConnect    (0x14031D090)
      //   [2] pump         (0x140326620)
      //   [3] onDisconnect (0x14031D320)

      [[gnu::ms_abi]] void
      lobby_destructor (bd_lobby_service*)
      {
        // Since our singleton is statically allocated, there is nothing
        // to actually free here.
      }

      [[gnu::ms_abi]] void
      lobby_on_connect (bd_lobby_service*)
      {
        // The connection is always established in our emulation, so we
        // can just no-op this.
      }

      [[gnu::ms_abi]] void
      lobby_pump (bd_lobby_service*)
      {
        // Task completion is handled in the startTask hook instead, so
        // pumping does nothing.
      }

      [[gnu::ms_abi]] void
      lobby_on_disconnect (bd_lobby_service*)
      {
        // We never disconnect.
      }

      void* lobby_vtable[4] {reinterpret_cast<void*> (&lobby_destructor),
                             reinterpret_cast<void*> (&lobby_on_connect),
                             reinterpret_cast<void*> (&lobby_pump),
                             reinterpret_cast<void*> (&lobby_on_disconnect)};


      // bd_lobby_connection virtual methods.
      //
      // The game do virtual dispatch on the connection object during
      // destruction. We provide a destructor along with safe defaults for any
      // unidentified virtual slots.

      [[gnu::ms_abi]] int64_t
      connection_destructor (bd_lobby_connection*)
      {
        return 0;
      }

      [[gnu::ms_abi]] int64_t
      connection_stub (void*)
      {
        return 0;
      }

      static constexpr size_t connection_vtable_slots (32);
      void* connection_vtable[connection_vtable_slots];

      void
      init_connection ()
      {
        auto stub (reinterpret_cast<void*> (&connection_stub));

        for (auto& e : connection_vtable)
          e = stub;

        connection_vtable[0] = reinterpret_cast<void*> (&connection_destructor);
        connection.vtable = connection_vtable;
        connection.refcount = 0x7FFFFFFF; // never reaches zero

        memset (connection.body, 0, sizeof (connection.body));
      }

      // Stub bdStorage.
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
      init_lobby ()
      {
        memset (&lobby, 0, sizeof (lobby));

        lobby.vtable = lobby_vtable;
        lobby.connection = &connection;

        bd_storage.status = 0;
        bd_storage.connection = &connection;

        lobby.storage = &bd_storage;
      }

      // dw_service (0x140136210).
      //
      // The game normally creates the singleton lazily at 0x140714A50.

      using  dw_sevice_t = void* (*) ();
      inline dw_sevice_t dw_sevice_original = reinterpret_cast<dw_sevice_t> (0x140136210);

      [[gnu::ms_abi]] void*
      dw_service ()
      {
        return &lobby;
      }

      // Non-virtual member function hooks.
      //

      // bdLobbyServiceImpl::connect.
      //
      using lobby_connect_t = bool (*) (void*, void*, void*, bool);
      lobby_connect_t lobby_connect_original (reinterpret_cast<lobby_connect_t> (0x14031c750));

      [[gnu::ms_abi]] bool
      lobby_connect (void*, void*, void*, bool)
      {
        return true;
      }

      // bdLobbyServiceImpl::disconnect.
      //
      using lobby_disconnect_t = void (*) (void*, int);
      lobby_disconnect_t lobby_disconnect_original (reinterpret_cast<lobby_disconnect_t> (0x14031c950));

      [[gnu::ms_abi]] void
      lobby_disconnect (void*, int reason)
      {
        // We do not have anything to tear down.
        //
      }

      // bdLobbyServiceImpl::getStatus.
      //
      using get_status_t = int32_t (*) (void*);
      get_status_t get_status_original (reinterpret_cast<get_status_t> (0x14031cb10));

      [[gnu::ms_abi]] int32_t
      get_status (void*)
      {
        return 2; // always BD_ONLINE
      }

      // bdLobbyServiceImpl::getMatchMaking.
      //
      using get_matchmaking_t = void* (*) (void*);
      get_matchmaking_t get_matchmaking_original (reinterpret_cast<get_matchmaking_t> (0x14031c990));

      [[gnu::ms_abi]] void*
      get_matchmaking (void*)
      {
        return lobby.matchmaking;
      }

      // bdLobbyServiceImpl::getRemoteTaskMgr.
      //
      using get_task_mgr_t = void* (*) (void*);
      get_task_mgr_t get_task_mgr_original (reinterpret_cast<get_task_mgr_t> (0x14031CA30));

      [[gnu::ms_abi]] void*
      get_task_mgr (void*)
      {
        return lobby.task_mgr;
      }

      // bdLobbyServiceImpl::getPerformance.
      //
      using get_performance_t = void* (*) (void*);
      get_performance_t get_performance_original (reinterpret_cast<get_performance_t> (0x14031ca70));

      [[gnu::ms_abi]] void*
      get_performance (void*)
      {
        return lobby.performance;
      }

      // bdLobbyServiceImpl::getStorage.
      //
      using get_storage_t = void* (*) (void*);
      get_storage_t get_storage_original (reinterpret_cast<get_storage_t> (0x14031cff0));

      [[gnu::ms_abi]] void*
      get_storage (void*)
      {
        // Return the sub-service located at +0x60. This safely points to
        // store_impl which is guaranteed non-null.
        //
        return lobby.storage;
      }
    }

    lobby_service::lobby_service ()
    {
      init_connection ();
      init_lobby ();

      detour (dw_sevice_original,        &dw_service);
      detour (get_matchmaking_original,  &get_matchmaking);
      detour (get_performance_original,  &get_performance);
      detour (get_status_original,       &get_status);
      detour (get_storage_original,      &get_storage);
      detour (get_task_mgr_original,     &get_task_mgr);
      detour (lobby_connect_original,    &lobby_connect);
      detour (lobby_disconnect_original, &lobby_disconnect);
    }

    bd_lobby_service&
    lobby_service::instance ()
    {
      return lobby;
    }
  }
}
