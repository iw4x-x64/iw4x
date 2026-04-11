#include <libiw4x/demonware/bd-auth.hxx>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <windows.h>
#    undef NOMINMAX
#  else
#    include <windows.h>
#  endif
#  undef WIN32_LEAN_AND_MEAN
#else
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <windows.h>
#    undef NOMINMAX
#  else
#    include <windows.h>
#  endif
#endif

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <ranges>
#include <algorithm>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    auth_ticket local_ticket {};

    auth::
    auth ()
    {
      local_ticket.title_id = 0x7DA; // IW4 (2010)

      // @wroyca: We need to think about how we want to handle unique id.
      //
      char hn[256] {};
      DWORD sz (sizeof (hn));
      GetComputerNameA (hn, &sz);

      hash<string> hs;
      local_ticket.user_id = hs (string (hn, sz));

      // Note that the session array name field is only 0x10 bytes. So we
      // truncate the hostname to 15 characters plus the null terminator to use
      // it as the player's display name.
      //
      // @wroyca: Check if this 15-character limit is actually sufficient.
      //          Between ANSI color codes, longer usernames, and multi-byte
      //          UTF-8 sequences for internationalized names, we might find
      //          ourselves truncating prematurely. We should probably keep an
      //          eye on this and see if we need a more dynamic approach or at
      //          least a more generous buffer.
      //
      //
      strncpy (local_ticket.username, hn, sizeof (local_ticket.username) - 1);
      local_ticket.username[sizeof (local_ticket.username) - 1] = '\0';

      // Fill the session key and ticket data with random bytes. Since these
      // values are opaque to the game and only inspected by the lobby
      // service (which we control), any random data will do.
      //
      random_device rd;
      seed_seq ss {rd (), rd (), rd (), rd ()};
      independent_bits_engine<mt19937_64, 8, unsigned char> r (ss);

      ranges::generate (local_ticket.session_key, ref (r));
      ranges::generate (local_ticket.ticket_data, ref (r));
    }

    const auth_ticket&
    auth::ticket ()
    {
      return local_ticket;
    }
  }
}
