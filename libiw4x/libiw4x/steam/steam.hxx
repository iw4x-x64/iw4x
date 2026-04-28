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
    bool
    initialize ();

    // Shut down the runtime.
    //
    void
    shutdown ();

    // Pump the Steamworks callback machinery.
    //
    // We need to call this once per frame, typically from the com_frame
    // scheduler. It runs a single iteration of the manual dispatch loop to
    // ensure all pending callbacks and asynchronous results are delivered.
    //
    void
    pump_callbacks ();

    // Cached interface accessors.
    //
    ISteamUser*
    user () noexcept;

    ISteamFriends*
    friends () noexcept;

    ISteamMatchmaking*
    matchmaking () noexcept;

    ISteamUtils*
    utils () noexcept;

    HSteamPipe
    pipe () noexcept;

    // Local player's Steam ID.
    //
    steam_id
    local_id () noexcept;
  }
}
