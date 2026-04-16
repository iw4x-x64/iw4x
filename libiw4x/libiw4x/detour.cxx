#include <libiw4x/detour.hxx>

#include <Zydis/Zydis.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <libiw4x/utility-win32.hxx>

using namespace std;

namespace iw4x
{
  constexpr int32_t min_hook_size (14);
  constexpr int32_t max_hook_size (32);
  constexpr int64_t max_hook_disp (INT32_MAX);

  struct prologue_data
  {
    size_t ds;
    size_t di;
    ZydisDecodedInstruction ins[max_hook_size];
    ZydisDecodedOperand ops[max_hook_size][ZYDIS_MAX_OPERAND_COUNT];

    // Zero out the sizes. The arrays will be overwritten as we decode, so no
    // need to waste cycles clearing them.
    //
    prologue_data ()
      : ds (0), di (0) {}
  };

  struct frame_data
  {
    prologue_data p;
    void* f;

    frame_data (prologue_data const& p_, void* f_)
      : p (p_), f (f_) {}
  };

  struct relocation_data
  {
    void* f;
    size_t ds;
    size_t rd;

    relocation_data (void* f_, size_t ds_, size_t rd_)
      : f (f_), ds (ds_), rd (rd_) {}
  };

  void
  detour (void*& t, void* s)
  {
    auto
    decode ([] (void* t) -> prologue_data
    {
      ZydisDecoder d {};
      ZydisDecoderInit (&d, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

      prologue_data p {};

      // Keep decoding until we have at least 14 bytes for our absolute jump.
      // Note that we cannot split instructions, so we might pull in a bit more
      // than the strict minimum.
      //
      while (p.ds < min_hook_size && p.di < max_hook_size)
      {
        if (ZYAN_FAILED (
              ZydisDecoderDecodeFull (&d,
                                      static_cast<uint8_t*> (t) + p.ds,
                                      ZYDIS_MAX_INSTRUCTION_LENGTH,
                                      &p.ins[p.di],
                                      p.ops[p.di])))
          throw runtime_error ("unable to decode instruction");

        p.ds += p.ins[p.di].length;
        p.di++;
      }

      return p;
    });

    auto
    allocate ([] (void* t, prologue_data& p) -> frame_data
    {
      SYSTEM_INFO si;
      GetSystemInfo (&si);

      size_t ag (si.dwAllocationGranularity);
      size_t as (p.ds + min_hook_size);

      auto ta (reinterpret_cast<uint64_t> (t));
      uintptr_t min_a (ta > max_hook_disp ? ta - max_hook_disp : 0);
      uintptr_t max_a (ta < UINTPTR_MAX - max_hook_disp ? ta + max_hook_disp
                                                        : UINTPTR_MAX);

      // Find an isolated memory frame close to the target function. We need to
      // stay within 2GB for RIP-relative addressing to remain valid after
      // relocation. Walk the page tables up and down.
      //
      auto
      probe ([as, ag, min_a, max_a] (uintptr_t addr, bool down) -> void*
      {
        MEMORY_BASIC_INFORMATION mi;

        while (addr >= min_a && addr <= max_a &&
               VirtualQuery (reinterpret_cast<void*> (addr), &mi, sizeof (mi)))
        {
          uintptr_t base (reinterpret_cast<uintptr_t> (mi.BaseAddress));
          uintptr_t size (mi.RegionSize);

          if (mi.State == MEM_FREE && size >= as)
          {
            uintptr_t a (down ? (base + size - as) & ~(ag - 1)
                              : (base + ag - 1) & ~(ag - 1));

            if (a >= base && a + as <= base + size && a >= min_a && a <= max_a)
            {
              if (void* m = VirtualAlloc (reinterpret_cast<void*> (a),
                                          as,
                                          MEM_COMMIT | MEM_RESERVE,
                                          PAGE_EXECUTE_READWRITE))
                return m;
            }
          }

          if (down)
          {
            if (base == 0) break;
            addr = base - 1;
          }
          else
          {
            addr = base + size;
            if (addr < base) break;
          }
        }

        return nullptr;
      });

      if (void* f = probe (ta, true))
        return frame_data (p, f);

      if (void* f = probe (ta, false))
        return frame_data (p, f);

      throw runtime_error ("unable to allocate isolated address frame");
    });

    auto
    relocate ([] (void* t, frame_data& f) -> relocation_data
    {
      auto fo (reinterpret_cast<uint8_t*> (f.f));
      auto fa (reinterpret_cast<uint64_t> (f.f));
      auto ta (reinterpret_cast<uint64_t> (t));

      size_t rd (0);
      uint64_t ra (ta);

      // Re-encode the stolen prologue instructions into our new frame. Any
      // RIP-relative memory access needs to be adjusted because its distance
      // to the target data has changed.
      //
      for (size_t i (0); i < f.p.di; ++i)
      {
        ZydisEncoderRequest r {};
        ZydisDecodedInstruction* ri (&f.p.ins[i]);
        ZydisDecodedOperand* ro (f.p.ops[i]);
        ZyanU8 rv (ri->operand_count_visible);

        if (ZYAN_FAILED (
              ZydisEncoderDecodedInstructionToEncoderRequest (ri, ro, rv, &r)))
          throw runtime_error ("unable to create encoder request");

        for (ZyanU8 n (0); n < rv; ++n)
        {
          if (ro[n].type == ZYDIS_OPERAND_TYPE_MEMORY &&
              ro[n].mem.base == ZYDIS_REGISTER_RIP)
          {
            int64_t dv (ro[n].mem.disp.value);
            int64_t at (static_cast<int64_t> (ra + ri->length + dv));
            int64_t dr (static_cast<int64_t> (at - (fa + rd + ri->length)));

            // If it falls outside the 32-bit window, the target is out of reach
            // from our newly allocated trampoline page.
            //
            if (dr < INT32_MIN || dr > INT32_MAX)
              throw runtime_error ("RIP-relative displacement out of range");

            r.operands[n].mem.displacement = dr;
          }
        }

        ZyanUSize rl (f.p.ds + min_hook_size - rd);

        if (ZYAN_FAILED (ZydisEncoderEncodeInstruction (&r, fo + rd, &rl)))
          throw runtime_error ("unable to encode relocated instruction");

        rd += rl;
        ra += ri->length;
      }

      return relocation_data (f.f, f.p.ds, rd);
    });

    auto
    apply ([] (void*& t, void* s, relocation_data& r) -> void
    {
      auto
      commit ([] (uint8_t* b, void* src)
      {
        ZydisEncoderRequest req {};
        req.mnemonic = ZYDIS_MNEMONIC_JMP;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 1;

        ZydisEncoderOperand* o (&req.operands[0]);
        o->type = ZYDIS_OPERAND_TYPE_MEMORY;
        o->mem.base = ZYDIS_REGISTER_RIP;
        o->mem.displacement = 0;
        o->mem.size = sizeof (ZyanU64);

        ZyanU8 in[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize il (sizeof (in));

        if (ZYAN_FAILED (ZydisEncoderEncodeInstruction (&req, in, &il)))
          throw runtime_error ("unable to encode absolute jump");

        memmove (b, in, il);
        memmove (b + il, static_cast<const void*> (&src), sizeof (src));
      });

      auto to (reinterpret_cast<uint8_t*> (t));
      auto fo (reinterpret_cast<uint8_t*> (r.f));

      // Stitch it all together. Reintegrate trampoline to original function
      // tail.
      //
      commit (fo + r.rd, to + r.ds);

      // Update entry point.
      //
      t = fo;

      // Overwrite original prologue with detour branch.
      //
      commit (to, s);
    });

    prologue_data p (decode (t));
    frame_data f (allocate (t, p));
    relocation_data r (relocate (t, f));

    apply (t, s, r);
  }
}
