#include <libiw4x/demonware/lobby/auth-service.hxx>

#include <algorithm>
#include <random>
#include <string>
#include <functional>

#include <libiw4x/utility-win32.hxx>

using namespace std;

namespace iw4x
{
  namespace demonware
  {
    bd_auth_ticket auth_ticket {};

    auth_service::
    auth_service ()
    {
      auth_ticket.title_id = 0x7DA; // IW4 (2010)

      // @wroyca: We need to think about how we want to handle unique id.
      //
      char hn[256] {};
      DWORD sz (sizeof (hn));
      GetComputerNameA (hn, &sz);

      hash<string> hs;
      auth_ticket.user_id = hs (string (hn, sz));

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
      strncpy (auth_ticket.username, hn, sizeof (auth_ticket.username) - 1);
      auth_ticket.username[sizeof (auth_ticket.username) - 1] = '\0';

      // Fill the session key and ticket data with random bytes. Since these
      // values are opaque to the game and only inspected by the lobby
      // service (which we control), any random data will do.
      //
      random_device rd;
      seed_seq ss {rd (), rd (), rd (), rd ()};
      independent_bits_engine<mt19937_64, 8, uint32_t> r (ss);

      ranges::generate (auth_ticket.session_key, std::ref (r));
      ranges::generate (auth_ticket.ticket_data, std::ref (r));
    }

    const bd_auth_ticket& auth_service::
    ticket ()
    {
      return auth_ticket;
    }
  }
}
