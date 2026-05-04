#pragma once

#include <cstdint>

#include <libiw4x/export.hxx>

namespace iw4x
{
  class memory
  {
  public:
    // Fill memory with a value.
    //
    // Similar to memset, but if the value is 0x90 (NOP), we try to replace
    // runs of single-byte NOPs with optimal multi-byte architectural NOP
    // sequences to relieve the instruction decoder.
    //
    static void*
    write (void* destination, int value, size_t size);

    static void*
    write (uintptr_t destination, int value, size_t size);

    // Copy memory.
    //
    // Similar to memcpy, but with the same NOP-optimization logic: if the
    // source buffer contains runs of 0x90, we expand them into multi-byte
    // NOPs in the destination.
    //
    static void*
    write (void* destination, const void* source, size_t size);

    static void*
    write (uintptr_t destination, const void* source, size_t size);

    // Convenience wrappers for static arrays.
    //
    template <size_t N>
    static inline void*
    write (uintptr_t destination, const uint8_t (&source)[N])
    {
      return write (destination, source, N);
    }

    template <size_t N>
    static inline void*
    write (uintptr_t destination, const char (&source)[N])
    {
      // If we are passing a string literal (which is the 99% use case here),
      // N includes the null terminator. Since we are patching code, we almost
      // never want to write that trailing null byte.
      //
      // So we strip it. If we really want to write a null byte, we can use the
      // raw pointer overload or a uint8_t array.
      //
      return write (destination, source, N - 1);
    }

    // Address literal helpers.
    //

    // Produce a typed reference from a raw memory address.
    //
    // Note that we assume the caller has already verified the address falls
    // within the mapped memory boundaries and is properly aligned for type T.
    // If it's not, we are going to have a bad time when we actually try to
    // dereference it.
    //
    template <typename T>
    inline T&
    iref (uintptr_t address) noexcept
    {
      T* p (reinterpret_cast<T*> (address));
      return *p;
    }
  };
}
