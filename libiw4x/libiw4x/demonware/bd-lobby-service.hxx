#pragma once

#include <cstdint>

#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace demonware
  {
    // Layout-compatible bdLobbyConnection.
    //
    // The game holds a pointer to this at lobby_service +0x90 and reads the
    // reference count at +0x08. Note that all interaction is routed through
    // hooked lobby service accessors. We just need to make sure the connection
    // object exists as a valid target and its reference count never drops to
    // zero.
    //
    struct bd_lobby_connection
    {
      void*   vtable;     // 0x00
      int32_t refcount;   // 0x08
      uint8_t body[0xF4]; // 0x0C: zero-initialized
    };

    static_assert (sizeof (bd_lobby_connection) == 0x100);

    // Layout-compatible bdLobbyServiceImpl singleton.
    //
    // This is a 0x140-byte structure that mirrors the game's singleton at
    // 0x140714A50. The vtable at 0x00 has 4 entries (destructor, onConnect,
    // pump, onDisconnect). Notice the inline bdRemoteTaskManager at 0x18. The
    // sub-service pointers are scattered at 0x40, 0x60, and 0x80, while our
    // bdLobbyConnection pointer sits at 0x90.
    //
    struct bd_lobby_service
    {
      void*                vtable;            // 0x00: 4-slot vtable
      uint8_t              net[0x10];         // 0x08: network/socket data
      uint8_t              task_mgr[0x20];    // 0x18: bdRemoteTaskManager
      int32_t              flags;             // 0x38
      int32_t              pad0;              // 0x3C
      void*                matchmaking;       // 0x40: bdMatchMaking*
      uint8_t              mm_reserved[0x18]; // 0x48
      void*                storage;           // 0x60: bdStorage*
      uint8_t              st_reserved[0x18]; // 0x68
      void*                performance;       // 0x80: bdPerformance*
      int32_t              flags2;            // 0x88
      int32_t              pad1;              // 0x8C
      bd_lobby_connection* connection;        // 0x90: ref-counted
      uint8_t              tail[0xA4];        // 0x98
      int32_t              flags3;            // 0x13C
    };

    static_assert (sizeof (bd_lobby_service) == 0x140);

    // Emulated bdLobbyServiceImpl.
    //
    class LIBIW4X_SYMEXPORT lobby_service
    {
    public:
      lobby_service ();

      // Access the emulated singleton.
      //
      static bd_lobby_service&
      instance ();
    };
  }
}
