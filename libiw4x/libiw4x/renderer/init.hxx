#pragma once

#include <libiw4x/import.hxx>
#include <libiw4x/export.hxx>

// Forward declaration for D3D9.
//
struct IDirect3DDevice9;

namespace iw4x
{
  namespace renderer
  {
    LIBIW4X_SYMEXPORT void
    init ();

    // Register a callback to be invoked on each frame.
    //
    LIBIW4X_SYMEXPORT void
    on_frame (function<void (IDirect3DDevice9*)>);
  }
}
