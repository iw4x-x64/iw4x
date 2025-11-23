#include <cstring>
#include <span>
#include <string>

#include <libiw4x/utility/xxhash.hxx>

#undef NDEBUG
#include <cassert>

using namespace std;
using namespace iw4x::utility;

namespace
{
  // Test xxh64() with string and default seed
  //
  void
  test_xxh64_string_default_seed ()
  {
    uint64_t h (xxh64 ("hello world"));
    assert (h != 0);

    // Same input should produce same hash
    //
    uint64_t h2 (xxh64 ("hello world"));
    assert (h == h2);

    // Different input should produce different hash
    //
    uint64_t h3 (xxh64 ("hello world!"));
    assert (h != h3);
  }

  // Test xxh64() with string and custom seed
  //
  void
  test_xxh64_string_custom_seed ()
  {
    uint64_t h1 (xxh64 ("hello world", 0));
    uint64_t h2 (xxh64 ("hello world", 42));
    uint64_t h3 (xxh64 ("hello world", 0));

    // Different seeds should produce different hashes
    //
    assert (h1 != h2);

    // Same seed should produce same hash
    //
    assert (h1 == h3);
  }

  // Test xxh64() with empty string
  //
  void
  test_xxh64_empty_string ()
  {
    uint64_t h1 (xxh64 (""));
    uint64_t h2 (xxh64 (""));
    assert (h1 == h2);

    // Empty string hash should differ from non-empty
    //
    uint64_t h3 (xxh64 ("x"));
    assert (h1 != h3);
  }

  // Test xxh64() with binary data and default seed
  //
  void
  test_xxh64_binary_default_seed ()
  {
    const byte data [] = {byte {0x01}, byte {0x02}, byte {0x03}, byte {0x04}};
    uint64_t h (xxh64 (span {data}));
    assert (h != 0);

    // Same data should produce same hash
    //
    uint64_t h2 (xxh64 (span {data}));
    assert (h == h2);

    // Different length should produce different hash
    //
    uint64_t h3 (xxh64 (span {data, 3}));
    assert (h != h3);
  }

  // Test xxh64() with binary data and custom seed
  //
  void
  test_xxh64_binary_custom_seed ()
  {
    const byte data [] = {byte {0x01}, byte {0x02}, byte {0x03}, byte {0x04}};
    uint64_t h1 (xxh64 (span {data}, 0));
    uint64_t h2 (xxh64 (span {data}, 123));

    // Different seeds should produce different hashes
    //
    assert (h1 != h2);
  }

  // Test xxh64() with null-terminated string as binary data
  //
  void
  test_xxh64_string_as_binary ()
  {
    const char* str ("test");
    span<const byte> sp (reinterpret_cast<const byte*> (str), strlen (str));
    uint64_t h1 (xxh64 (sp));
    uint64_t h2 (xxh64 (string ("test")));

    // String and binary data hash should match
    //
    assert (h1 == h2);
  }

  // Test xxh64_hasher with default seed
  //
  void
  test_xxh64_hasher_default_seed ()
  {
    xxh64_hasher hasher;
    hasher.update ("hello ");
    hasher.update ("world");
    uint64_t h (hasher.digest ());

    // Should match single-shot hash
    //
    uint64_t h2 (xxh64 ("hello world"));
    assert (h == h2);
  }

  // Test xxh64_hasher with custom seed
  //
  void
  test_xxh64_hasher_custom_seed ()
  {
    xxh64_hasher hasher (42);
    hasher.update ("hello world");
    uint64_t h (hasher.digest ());

    // Should match single-shot hash with same seed
    //
    uint64_t h2 (xxh64 ("hello world", 42));
    assert (h == h2);
  }

  // Test xxh64_hasher with binary data
  //
  void
  test_xxh64_hasher_binary ()
  {
    xxh64_hasher hasher;
    const byte data1 [] = {byte {0x01}, byte {0x02}};
    const byte data2 [] = {byte {0x03}, byte {0x04}};
    hasher.update (span {data1});
    hasher.update (span {data2});
    uint64_t h (hasher.digest ());

    // Should match single-shot hash of combined data
    //
    const byte combined [] = {byte {0x01},
                              byte {0x02},
                              byte {0x03},
                              byte {0x04}};
    uint64_t h2 (xxh64 (span {combined}));
    assert (h == h2);
  }

  // Test xxh64_hasher reset with default seed
  //
  void
  test_xxh64_hasher_reset_default ()
  {
    xxh64_hasher hasher;
    hasher.update ("first");
    hasher.reset ();
    hasher.update ("second");
    uint64_t h (hasher.digest ());

    // Should match hash of "second" only
    //
    uint64_t h2 (xxh64 ("second"));
    assert (h == h2);
  }

  // Test xxh64_hasher reset with custom seed
  //
  void
  test_xxh64_hasher_reset_custom ()
  {
    xxh64_hasher hasher (0);
    hasher.update ("first");
    hasher.reset (100);
    hasher.update ("second");
    uint64_t h (hasher.digest ());

    // Should match hash of "second" with seed 100
    //
    uint64_t h2 (xxh64 ("second", 100));
    assert (h == h2);
  }

  // Test xxh64_hasher can continue after digest
  //
  void
  test_xxh64_hasher_continue_after_digest ()
  {
    xxh64_hasher hasher;
    hasher.update ("hello");
    uint64_t h1 (hasher.digest ());

    hasher.update (" world");
    uint64_t h2 (hasher.digest ());

    // Second digest should be hash of "hello world"
    //
    uint64_t h3 (xxh64 ("hello world"));
    assert (h2 == h3);

    // First digest should be hash of "hello"
    //
    uint64_t h4 (xxh64 ("hello"));
    assert (h1 == h4);
  }

  // Test xxh64_hasher with empty updates
  //
  void
  test_xxh64_hasher_empty_updates ()
  {
    xxh64_hasher hasher;
    hasher.update ("");
    hasher.update ("test");
    hasher.update ("");
    uint64_t h (hasher.digest ());

    // Should match hash of "test"
    //
    uint64_t h2 (xxh64 ("test"));
    assert (h == h2);
  }

  // Test xxh64_hasher with multiple small updates
  //
  void
  test_xxh64_hasher_multiple_small_updates ()
  {
    xxh64_hasher hasher;
    string input ("abcdefghijklmnopqrstuvwxyz");

    for (char c : input)
    {
      string s (1, c);
      hasher.update (s);
    }

    uint64_t h (hasher.digest ());

    // Should match single-shot hash
    //
    uint64_t h2 (xxh64 (input));
    assert (h == h2);
  }

  // Test xxh64_hasher with large data
  //
  void
  test_xxh64_hasher_large_data ()
  {
    xxh64_hasher hasher;
    string large (10000, 'x');
    hasher.update (large);
    uint64_t h (hasher.digest ());

    // Should match single-shot hash
    //
    uint64_t h2 (xxh64 (large));
    assert (h == h2);
  }

  // Test seed value 0 equivalence
  //
  void
  test_seed_zero_equivalence ()
  {
    // Default seed should be 0
    //
    uint64_t h1 (xxh64 ("test"));
    uint64_t h2 (xxh64 ("test", 0));
    assert (h1 == h2);

    xxh64_hasher hasher1;
    xxh64_hasher hasher2 (0);
    hasher1.update ("test");
    hasher2.update ("test");
    assert (hasher1.digest () == hasher2.digest ());
  }

  // Test binary data with embedded nulls
  //
  void
  test_binary_embedded_nulls ()
  {
    const byte data [] = {byte {'a'},
                          byte {'\0'},
                          byte {'b'},
                          byte {'\0'},
                          byte {'c'}};
    uint64_t h1 (xxh64 (span {data}));

    xxh64_hasher hasher;
    hasher.update (span {data});
    uint64_t h2 (hasher.digest ());

    assert (h1 == h2);
  }
}

int
main ()
{
  test_xxh64_string_default_seed ();
  test_xxh64_string_custom_seed ();
  test_xxh64_empty_string ();
  test_xxh64_binary_default_seed ();
  test_xxh64_binary_custom_seed ();
  test_xxh64_string_as_binary ();
  test_xxh64_hasher_default_seed ();
  test_xxh64_hasher_custom_seed ();
  test_xxh64_hasher_binary ();
  test_xxh64_hasher_reset_default ();
  test_xxh64_hasher_reset_custom ();
  test_xxh64_hasher_continue_after_digest ();
  test_xxh64_hasher_empty_updates ();
  test_xxh64_hasher_multiple_small_updates ();
  test_xxh64_hasher_large_data ();
  test_seed_zero_equivalence ();
  test_binary_embedded_nulls ();
}
