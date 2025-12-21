#include <libiw4x/types.hxx>
#include <libiw4x/utility.hxx>
#include <libiw4x/utility-win32.hxx>

#include <libiw4x/export.hxx>

namespace iw4x
{
  extern "C"
  {
    // MinGW and MSVC differ in how they treat exports for DLLs.
    //
    // MinGW (via GNU ld) allows producing a DLL with no explicitly exported
    // symbols. In that model, DllMain is treated as a special entry point
    // discovered by name and does not need to appear in the export table.
    //
    // MSVC's linker, on the other hand, requires at least one symbol to be
    // placed in the DLL export table. If no symbol is explicitly marked for
    // export (e.g. via __declspec(dllexport) or a .def file), MSVC refuses to
    // link the DLL and reports that no exported symbols exist.
    //
    // For this reason, we explicitly export DllMain when building with MSVC,
    // even though this is unnecessary for MinGW and has no functional effect at
    // runtime.
    //
    LIBIW4X_SYMEXPORT BOOL WINAPI
    DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
  }
}
