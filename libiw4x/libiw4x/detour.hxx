#pragma once

#include <libiw4x/export.hxx>

namespace iw4x
{
  void
  detour (void*& t, void* s);

  inline void
  detour (auto& t, auto s)
  {
    void* pt (reinterpret_cast<void*> (t));
    void* ps (reinterpret_cast<void*> (s));

    detour (pt, ps);

    t = reinterpret_cast<decltype (t)> (pt);
  }
}
