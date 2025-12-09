#include <libiw4x/detour.hxx>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include <Zydis/Zydis.h>

#include <libiw4x/windows/windows.hxx>

using namespace std;

namespace iw4x
{
  constexpr int32_t min_hook_size (14);
  constexpr int32_t max_hook_size (32);
  constexpr int64_t max_hook_disp (INT32_MAX);

  void
  detour (void*& t, void* s)
  {
    ZydisDecoder d {};
    ZydisDecoderInit (&d, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    ZydisDecodedInstruction ins[max_hook_size];
    ZydisDecodedOperand ops[max_hook_size][ZYDIS_MAX_OPERAND_COUNT];

    // Determine how many whole instructions the entry contains before we
    // patch anything. That is, x86-64 doesn't guarantee alignment or
    // structural boundaries, so a jump inserted mid-instruction breaks the
    // architectural contract instantly.
    //
    size_t ds (0), di (0);
    while (ds < min_hook_size && di < max_hook_size)
    {
      if (ZYAN_FAILED (ZydisDecoderDecodeFull (&d,
                                                static_cast<uint8_t*> (t) + ds,
                                                ZYDIS_MAX_INSTRUCTION_LENGTH,
                                                &ins[di],
                                                ops[di])))
        throw runtime_error ("unable to decode instruction");

      ds += ins[di].length, ++di;
    }

    // Derive the target representations needed for relocation
    //
    auto to (reinterpret_cast<uint8_t*> (t));
    auto ta (reinterpret_cast<uint64_t> (t));

    // Allocate an isolated address frame so that PC-relative adjustments
    // become well-defined. Note that our allocation must tolerate both the
    // relocated slice and the jump that reintegrates us with the unmodified
    // tail of the function.
    //
    // Note also that ISA restricts displacement to a signed 32-bit field, so
    // our frame allocation must remain representable within that range.
    //
    // For reference:
    // llvm-project/compiler-rt/lib/interception/interception_win.cpp
    //
    auto
    alloc_page = [t, ds, ta] () -> void*
    {
      SYSTEM_INFO si;
      GetSystemInfo (&si);

      size_t ag (si.dwAllocationGranularity);
      size_t as (ds + min_hook_size);

      auto
      bounded = [] (uintptr_t addr, size_t disp, bool above) -> uintptr_t
      {
        if (above)
          return addr < UINTPTR_MAX - disp ? addr + disp : UINTPTR_MAX;
        else
          return addr > disp ? addr - disp : 0;
      };

      uintptr_t lower_bound (bounded (ta, max_hook_disp, false));
      uintptr_t upper_bound (bounded (ta, max_hook_disp, true));

      // Probe on allocation-granularity boundaries, expanding outward from
      // the target.
      //
      // Note that ISA imposes no guarantee that a suitable page exists
      // "above" the target. In practice, fragmentation often make the nearest
      // reachable frame land below the symbol.
      //
      for (size_t offset (0); offset < max_hook_disp; offset += ag)
      {
        // Upward.
        //
        if (ta + offset <= upper_bound)
        {
          uintptr_t page ((ta + offset + ag - 1) & ~(ag - 1));

          void* frame (VirtualAlloc (reinterpret_cast<void*> (page),
                                      as,
                                      MEM_COMMIT | MEM_RESERVE,
                                      PAGE_EXECUTE_READWRITE));
          if (frame != nullptr)
          {
            auto fa (reinterpret_cast<uintptr_t> (frame));
            auto fd (static_cast<int64_t> (fa) - static_cast<int64_t> (ta));

            if (fd >= -max_hook_disp && fd <= max_hook_disp)
              return frame;

            VirtualFree (frame, 0, MEM_RELEASE);
          }
        }

        // Downward.
        //
        // Mirror the upward case while additionally guarding against
        // underflow when forming the request. The same displacement
        // validation applies. (see above for details)
        //
        if (offset > 0 && ta >= offset && ta - offset >= lower_bound)
        {
          uintptr_t page ((ta - offset) & ~(ag - 1));

          void* frame (VirtualAlloc (reinterpret_cast<void*> (page),
                                      as,
                                      MEM_COMMIT | MEM_RESERVE,
                                      PAGE_EXECUTE_READWRITE));
          if (frame != nullptr)
          {
            auto fa (reinterpret_cast<uintptr_t> (frame));
            auto fd (static_cast<int64_t> (fa) - static_cast<int64_t> (ta));

            if (fd >= -max_hook_disp && fd <= max_hook_disp)
              return frame;

            VirtualFree (frame, 0, MEM_RELEASE);
          }
        }
      }

      // If control reaches this point, the search radius has collapsed
      // without a single admissible page.
      //
      // TODO: The fallback path would require constructing an absolute
      // control-transfer sequence by rewriting PC-relative operands into
      // fully resolved absolute forms. This is technically feasible but
      // significantly more complex than maintaining a near-jump within the ~2
      // GB window.
      //
      return nullptr;
    };

    void* frame (alloc_page ());
    if (frame == nullptr)
      throw runtime_error ("unable to allocate isolated address frame");

    // Derive the frame representations needed for relocation.
    //
    auto fo (reinterpret_cast<uint8_t*> (frame));
    auto fa (reinterpret_cast<uint64_t> (frame));

    // Track the relocation progress within the isolated frame.
    //
    size_t rd (0);
    uint64_t ra (ta);

    // Relocate each decoded instruction into the trampoline. Note that
    // RIP-relative operands encode displacement from the instruction pointer
    // rather than an absolute address. Relocation therefore requires
    // re-deriving these displacements so that the semantic target remains
    // stable even though the instruction's physical address changes.
    //
    for (size_t i (0); i < di; ++i)
    {
      ZydisEncoderRequest r {};
      ZydisDecodedInstruction* ri (&ins [i]);
      ZydisDecodedOperand* ro (ops [i]);
      ZyanU8 rv (ri->operand_count_visible);

      // Convert the decoded instruction into an encoder request so that we
      // may adjust operands before re-encoding.
      //
      if (ZYAN_FAILED (ZydisEncoderDecodedInstructionToEncoderRequest (ri,
                                                                        ro,
                                                                        rv,
                                                                        &r)))
        throw runtime_error ("unable to create encoder request");

      // RIP-relative operands encode their target as a displacement from the
      // instruction pointer, meaning the numeric value is meaningful only at
      // the instruction's original address.
      //
      // Relocation therefore requires reconstructing the intended target
      // address from the original encoding, then recalculating the
      // displacement that would reach the same target when the instruction is
      // executed inside the trampoline.
      //
      for (ZyanU8 n (0); n < rv; ++n)
      {
        if (ro[n].type == ZYDIS_OPERAND_TYPE_MEMORY &&
            ro[n].mem.base == ZYDIS_REGISTER_RIP)
        {
          // At this point, if you squint hard enough, you'll realize
          // that it's really just an elaborate swap.
          //
          int64_t dv (ro [n].mem.disp.value);
          int64_t dr (ra + ri->length + dv - fa + rd - ri->length);

          if (in_range<int32_t> (dr))
            r.operands [n].mem.displacement = dr;
          else
            throw out_of_range ("RIP-relative displacement out of range");
        }
      }

      ZyanUSize relocation (ds + min_hook_size - rd);

      // After operand adjustment, we encode the relocated instruction into
      // the trampoline.
      //
      if (ZYAN_FAILED (ZydisEncoderEncodeInstruction (&r,
                                                      fo + rd,
                                                      &relocation)))
        throw runtime_error ("unable to encode relocated instruction");

      rd += relocation;
      ra += ri->length;
    }

    auto
    commit ([] (uint8_t* b, const void* source)
    {
      ZydisEncoderRequest r {};
      r.mnemonic = ZYDIS_MNEMONIC_JMP;
      r.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
      r.operand_count = 1;

      ZydisEncoderOperand* o (&r.operands [0]);
      o->type = ZYDIS_OPERAND_TYPE_MEMORY;
      o->mem.base = ZYDIS_REGISTER_RIP;
      o->mem.displacement = 0;
      o->mem.size = sizeof (ZyanU64);

      ZyanU8 instruction [ZYDIS_MAX_INSTRUCTION_LENGTH];
      ZyanUSize instruction_length (sizeof (instruction));

      if (ZYAN_FAILED (ZydisEncoderEncodeInstruction (&r,
                                                      instruction,
                                                      &instruction_length)))
        throw runtime_error ("unable to encode instruction");

      // Note that x86-64 lacks a single-instruction that takes a 64-bit
      // immediate, so we must manually append the address.
      //
      memmove (b, instruction, instruction_length);
      memmove (b + instruction_length, &source, sizeof (source));
    });

    // Returns control to the original function after the displaced slice.
    // Note that we also expose the trampoline as the canonical entry for
    // executing the preserved prologue.
    //
    commit (fo + rd, to + ds), t = fo;

    // Redirects the original function to the detour.
    //
    commit (to, s);
  }
}
