#pragma once

#include <libiw4x/export.hxx>
#include <libiw4x/steam/types.hxx>

// Forward declare opaque Steamworks interface pointers.
//
// Notice that we strictly avoid including Steamworks headers in our public
// interface. Instead, all access goes through the flat C API functions that
// we load from steam_api64.dll.
//
struct ISteamUser;
struct ISteamFriends;
struct ISteamMatchmaking;
struct ISteamUtils;

using HSteamPipe = int;

namespace iw4x
{
  namespace steam
  {
    // Initialize the Steamworks runtime.
    //
    // We must call this from the main thread before invoking any other function
    // in this namespace. It returns true if SteamAPI_InitFlat() succeeds and we
    // are able to acquire all the necessary interface pointers.
    //
    LIBIW4X_SYMEXPORT bool
    initialize ();

    // Shut down the runtime.
    //
    LIBIW4X_SYMEXPORT void
    shutdown ();

    // Pump the Steamworks callback machinery.
    //
    // We need to call this once per frame, typically from the com_frame
    // scheduler. It runs a single iteration of the manual dispatch loop to
    // ensure all pending callbacks and asynchronous results are delivered.
    //
    LIBIW4X_SYMEXPORT void
    pump_callbacks ();

    // Cached interface accessors.
    //
    LIBIW4X_SYMEXPORT ISteamUser*
    user () noexcept;

    LIBIW4X_SYMEXPORT ISteamFriends*
    friends () noexcept;

    LIBIW4X_SYMEXPORT ISteamMatchmaking*
    matchmaking () noexcept;

    LIBIW4X_SYMEXPORT ISteamUtils*
    utils () noexcept;

    LIBIW4X_SYMEXPORT HSteamPipe
    pipe () noexcept;

    // Local player's Steam ID.
    //
    LIBIW4X_SYMEXPORT steam_id
    local_id () noexcept;
  }
}
