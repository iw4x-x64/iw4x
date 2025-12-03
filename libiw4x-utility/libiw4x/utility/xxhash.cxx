#include <libiw4x/utility/xxhash.hxx>

#include <xxhash.h>

using namespace std;

namespace iw4x
{
  namespace utility
  {
    uint64_t
    xxh64 (const string& in)
    {
      return XXH64 (in.data (), in.size (), 0);
    }

    uint64_t
    xxh64 (const string& in, uint64_t s)
    {
      return XXH64 (in.data (), in.size (), s);
    }

    uint64_t
    xxh64 (span<const byte> in)
    {
      return XXH64 (in.data (), in.size (), 0);
    }

    uint64_t
    xxh64 (span<const byte> in, uint64_t s)
    {
      return XXH64 (in.data (), in.size (), s);
    }

    // xxh64_hasher
    //

    namespace detail
    {
      struct xxh64_state
      {
        XXH64_state_t* st;

        xxh64_state ()
          : st (XXH64_createState ())
        {
        }

        ~xxh64_state ()
        {
          if (st != nullptr)
            XXH64_freeState (st);
        }
      };
    }

    xxh64_hasher::
    xxh64_hasher ()
      : state (make_unique<detail::xxh64_state> ())
    {
      XXH64_reset (state->st, 0);
    }

    xxh64_hasher::
    xxh64_hasher (uint64_t s)
      : state (make_unique<detail::xxh64_state> ())
    {
      XXH64_reset (state->st, s);
    }

    xxh64_hasher::
    ~xxh64_hasher () = default;

    void xxh64_hasher::
    reset ()
    {
      XXH64_reset (state->st, 0);
    }

    void xxh64_hasher::
    reset (uint64_t s)
    {
      XXH64_reset (state->st, s);
    }

    void xxh64_hasher::
    update (const string& in)
    {
      XXH64_update (state->st, in.data (), in.size ());
    }

    void xxh64_hasher::
    update (span<const byte> in)
    {
      XXH64_update (state->st, in.data (), in.size ());
    }

    uint64_t xxh64_hasher::
    digest () const
    {
      return XXH64_digest (state->st);
    }
  }
}
