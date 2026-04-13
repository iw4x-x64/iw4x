#pragma once

#include <cstdint>
#include <compare>

namespace iw4x
{
  namespace steam
  {
    // Wrap the raw Steam user ID.
    //
    // The Steamworks API relies on raw uint64_t for both user and lobby
    // identifiers. We differentiate them here at the type level so we don't
    // accidentally mix them up and pass a lobby ID where a user ID is expected.
    //
    struct steam_id
    {
      uint64_t value;

      constexpr steam_id ()
        : value (0) {}

      constexpr explicit steam_id (uint64_t v)
        : value (v) {}

      constexpr explicit operator bool () const noexcept
      {
        return value != 0;
      }

      constexpr auto operator <=> (const steam_id&) const = default;
    };

    // Wrap the raw Steam lobby ID.
    //
    // Same reasoning as above, but for lobbies.
    //
    struct lobby_id
    {
      uint64_t value;

      constexpr lobby_id ()
        : value (0) {}

      constexpr explicit lobby_id (uint64_t v)
        : value (v) {}

      constexpr explicit operator bool () const noexcept
      {
        return value != 0;
      }

      constexpr auto operator <=> (const lobby_id&) const = default;
    };

    // Lobby type.
    //
    // This maps directly to ELobbyType from the Steamworks SDK. We define our
    // own enum class here so we don't have to drag the massive Steamworks
    // headers into every file that needs to know about lobby visibility.
    //
    enum class lobby_type : int
    {
      private_lobby  = 0,
      friends_only   = 1,
      public_lobby   = 2,
      invisible      = 3,
      private_unique = 4,
    };

    // Lobby state machine.
    //
    enum class lobby_state
    {
      none,
      creating,
      joining,
      active,
      leaving,
    };

    // Steam callback IDs.
    //
    // These must exactly match the k_iCallback values from the Steamworks
    // headers. For reference, k_iSteamMatchmakingCallbacks is 514 and
    // k_iSteamFriendsCallbacks is 331.
    //
    inline constexpr int k_LobbyCreated         = 514 + 13;  // 527
    inline constexpr int k_LobbyEnter            = 514 + 4;   // 518
    inline constexpr int k_LobbyInvite           = 514 + 3;   // 517
    inline constexpr int k_LobbyDataUpdate       = 514 + 5;   // 519
    inline constexpr int k_LobbyChatUpdate        = 514 + 6;   // 520
    inline constexpr int k_GameLobbyJoinRequested = 331 + 33;  // 364
    inline constexpr int k_SteamAPICallCompleted  = 703 + 3;   // 706

    // Maximum players allowed in a party.
    //
    // The engine enforces a hard upper bound of 18 players via the
    // party_maxplayers dvar. We just mirror this limit here to cap our Steam
    // lobby creation.
    //
    inline constexpr int max_party_members = 18;
  }
}
