#pragma once

#include <libiw4x/utility/export.hxx>

namespace iw4x
{
  namespace utility
  {
    LIBIW4X_UTILITY_SYMEXPORT void
    detour (void*& target, void* source);

    inline void
    detour (auto& target, auto source)
    {
      void* t = reinterpret_cast<void*> (target);
      void* s = reinterpret_cast<void*> (source);

      detour (t, s);

      target = reinterpret_cast<decltype (target)> (t);
    }
  }
}
