#include <libiw4x/memory.hxx>

#include <array>
#include <cstdint>
#include <cstring>

using namespace std;

namespace iw4x
{
  // Intel architectural NOP sequences.
  //
  // See Intel 64 and IA-32 Architectures Software Developer's Manual, Volume
  // 2B, Instruction Set Reference N-Z, Table 4-12.
  //
  // We use these instead of repeated 0x90 to reduce the pressure on the
  // instruction decoder.
  //
  static constexpr array<array<uint8_t, 9>, 9> nops
  {{
    {{0x90}},                                                 // 1
    {{0x66, 0x90}},                                           // 2
    {{0x0F, 0x1F, 0x00}},                                     // 3
    {{0x0F, 0x1F, 0x40, 0x00}},                               // 4
    {{0x0F, 0x1F, 0x44, 0x00, 0x00}},                         // 5
    {{0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00}},                   // 6
    {{0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00}},             // 7
    {{0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00}},       // 8
    {{0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00}}  // 9
  }};

  void*
  memwrite (void* dst, int val, size_t size)
  {
    // Empty op is a no-op.
    //
    if (size == 0)
      return dst;

    // If this is a request for 0x90 (NOP) fill, we try to be smart and use
    // the optimal multi-byte sequences.
    //
    if (val == 0x90)
    {
      uint8_t* out (static_cast<uint8_t*> (dst));

      // We only have sequences up to 9 bytes.
      //
      size_t chunks (size / 9);
      size_t rem (size % 9);

      const uint8_t* seq (nops[8].data ());

      // To avoid the overhead of calling memcpy inside the loop (which creates
      // a massive stall for such small writes), we cast the sequence to a
      // 64-bit integer.
      //
      // We know the 9-byte sequence is effectively a uint64 + a uint8.
      //
      // Note: this assumes unaligned access is safe, which it is on x64.
      //
      uint64_t seq_u64 (*reinterpret_cast<const uint64_t*> (seq));
      uint8_t seq_u8 (seq[8]);

      for (size_t i (0); i != chunks; ++i)
      {
        *reinterpret_cast<uint64_t*> (out) = seq_u64;
        out[8] = seq_u8;
        out += 9;
      }

      // Handle the tail.
      //
      if (rem != 0)
        memcpy (out, nops[rem - 1].data (), rem);

      return dst;
    }

    // For all non-NOP cases, fall back to the ordinary byte blast.
    //
    return memset (dst, val, size);
  }

  void*
  memwrite (uintptr_t dst, int val, size_t size)
  {
    return memwrite (reinterpret_cast<void*> (dst), val, size);
  }

  void*
  memwrite (void* dst, const void* src, size_t size)
  {
    if (size == 0)
      return dst;

    uint8_t* out (static_cast<uint8_t*> (dst));
    const uint8_t* in (static_cast<const uint8_t*> (src));

    // Iterate over the source buffer. We are looking for runs of 0x90 which we
    // interpret as "placeholders for optimized NOPs".
    //
    // Note: We implicitly assume the source buffer represents code. If we are
    // copying raw data where 0x90 is a valid value (e.g., offsets or immediate
    // values), this logic will corrupt it.
    //
    for (size_t pos (0); pos < size;)
    {
      // Check if we hit a NOP placeholder.
      //
      if (in[pos] == 0x90)
      {
        size_t len (0);
        while (pos + len < size && in[pos + len] == 0x90)
          len++;

        // Delegate to the smart filler.
        //
        memwrite (out + pos, 0x90, len);
        pos += len;
      }
      else
      {
        // Otherwise, we want to find the next NOP or the end of the buffer as
        // quickly as possible.
        //
        // We use memchr here as it will use SIMD internally to scan for the
        // 0x90 byte much faster than a scalar loop would.
        //
        const void* match (memchr (in + pos, 0x90, size - pos));

        size_t len (match
                    ? static_cast<const uint8_t*> (match) - (in + pos)
                    : size - pos);

        // Bulk copy the non-NOP code.
        //
        memcpy (out + pos, in + pos, len);
        pos += len;
      }
    }

    return dst;
  }

  void*
  memwrite (uintptr_t dst, const void* src, size_t size)
  {
    return memwrite (reinterpret_cast<void*> (dst), src, size);
  }
}
