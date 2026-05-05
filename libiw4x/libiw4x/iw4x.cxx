#include <libiw4x/iw4x.hxx>

#include <io.h>

#include <array>
#include <cstdint>
#include <ios>
#include <string>

#include <libiw4x/logger.hxx>
#include <libiw4x/memory.hxx>

#include <libiw4x/mod/oob/oob-pipeline.hxx>

#include <libiw4x/mod/mod-demonware.hxx>
#include <libiw4x/mod/mod-network.hxx>
#include <libiw4x/mod/mod-oob.hxx>
#include <libiw4x/mod/mod-party.hxx>
#include <libiw4x/mod/mod-scheduler.hxx>
#include <libiw4x/mod/mod-ui.hxx>
#include <libiw4x/mod/mod-menu.hxx>

using namespace std;

namespace iw4x
{
  namespace
  {
    void
    attach_console ()
    {
      // The subtlety here is that Windows has many ways to end up with stdout
      // and stderr pointing *somewhere* (sometimes to an actual console,
      // sometimes to a pipe, sometimes to a completely invalid handle). We do
      // not want to attach to `CONOUT$` in cases where the existing handles are
      // already valid and intentional, as this would silently discard the real
      // output sink.
      //
      // Instead, we first check `_fileno(stdout)` and `_fileno(stderr)`. That
      // is, MSVCRT sets these up at startup and will return `-2`
      // (`_NO_CONSOLE_FILENO`) if the file is invalid. This check is more
      // trustworthy than calling `GetStdHandle()`, which can return stale
      // handle IDs that may already be reused for unrelated objects by the time
      // we run.
      //
      if (_fileno (stdout) >= 0 || _fileno (stderr) >= 0)
      {
        // If either `_fileno()` is valid, we go one step further: `_fileno()`
        // itself had a bug (http://crbug.com/358267) in SUBSYSTEM:WINDOWS
        // builds for certain MSVC versions (VS2010 to VS2013), so we
        // double-check by calling `_get_osfhandle()` to confirm that the
        // underlying OS handle is valid. Only if both streams are invalid do
        // we attempt to attach a console.
        //
        intptr_t o (_get_osfhandle (_fileno (stdout)));
        intptr_t e (_get_osfhandle (_fileno (stderr)));

        if (o >= 0 || e >= 0)
          return;
      }

      // At this point, both standard streams appear invalid, so we attempt to
      // attach to the parent's console. Note that this call may fail for
      // expected reasons such as being already attached or the parent having
      // exited, and in all such cases the failure is non-fatal and we simply
      // bail out.
      //
      if (AttachConsole (ATTACH_PARENT_PROCESS) != 0)
      {
        // Once attached, rebind `stdout` and `stderr` to `CONOUT$` using
        // `freopen()`. Also duplicate their low-level descriptors (1 for
        // stdout, 2 for stderr) so that code using the raw file descriptor API
        // observes the same handles.
        //
        // Note that failure to rebind is non-fatal. Console output is
        // diagnostic-only and has no bearing on core functionality. We
        // therefore avoid exceptions and suppress all errors unconditionally.
        //
        bool o (freopen ("CONOUT$", "w", stdout) != nullptr &&
                _dup2 (_fileno (stdout), 1) != -1);

        bool e (freopen ("CONOUT$", "w", stderr) != nullptr &&
                _dup2 (_fileno (stderr), 2) != -1);

        // If stream were rebound, realign iostream objects (`cout`, `cerr`,
        // etc.) with C FILE streams.
        //
        if (o && e)
          ios::sync_with_stdio ();
      }
    }
  }

  extern "C"
  {
    BOOL WINAPI
    DllMain (HINSTANCE, DWORD r, LPVOID)
    {
      // We are only interested in the process attach event. That is, thread
      // notifications are just noise for our use case, and we handle cleanup
      // via static destructors rather than manual intervention on process
      // detach.
      //
      if (r != DLL_PROCESS_ATTACH)
        return TRUE;

      // DllMain executes while the loader lock is held, so we defer IW4x
      // initialization to the process's main thread to avoid ordering
      // violation.
      //
      uintptr_t t (0x140358EBC);
      uintptr_t s (reinterpret_cast<uintptr_t> (+[] ()
      {
        // __security_init_cookie
        //
        reinterpret_cast<void (*) ()> (0x1403598CC) ();

        // Under normal circumstances, a DLL is unloaded via FreeLibrary once
        // its reference count reaches zero. This is acceptable for auxiliary
        // libraries but unsuitable for IW4x, which embed deeply into the host
        // process.
        //
        HMODULE m;
        if (!GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_PIN |
                                  GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                reinterpret_cast<LPCTSTR> (DllMain),
                                &m))
        {
          MessageBox (nullptr,
                      "unable to mark module as permanent",
                      "error",
                      MB_ICONERROR);
          exit (1);
        }

        // By default, the process inherits its working directory from whatever
        // environment invoked it, which may vary across setups and lead to
        // unpredictable relative path resolution.
        //
        // The strategy here is to explicitly realign the process's working
        // directory to this DLL's location. Because the current working
        // directory is a process-wide state, this effectively makes all
        // relative file operations resolve against the DLL's directory even
        // when the DLL is hosted or started indirectly by a separate launcher.
        //
        // Note that we don't pass NULL to GetModuleFileNameW() as it
        // returns the path of the host process executable. If we are running
        // under a generic launcher, that would be the launcher's path, not
        // ours, and any relative path resolution based on it would be incorrect
        // (i.e., we would look for configuration files next to the launcher).
        //
        // Instead, we use the __ImageBase MSVC linker pseudo-variable. Its
        // address coincides with this specific module's base address (HMODULE),
        // which in turn can be used to query the DLL's path regardless of who
        // the hosting process is.
        //
        wchar_t p [MAX_PATH];
        if (GetModuleFileNameW (reinterpret_cast<HMODULE> (&__ImageBase),
                                p,
                                MAX_PATH))
        {
          wstring s (p);
          size_t i (s.rfind ('\\'));

          if (i == wstring::npos ||
              !SetCurrentDirectoryW (s.substr (0, i).c_str ()))
          {
            MessageBox (nullptr,
                        "unable to set process current directory",
                        "error",
                        MB_ICONERROR);
            exit (1);
          }
        }
        else
        {
          MessageBox (nullptr,
                      "unable to retrieve module location",
                      "error",
                      MB_ICONERROR);
          exit (1);
        }

        // Mark process as writable and executable.
        //
        // Note that we unprotect the whole image (SizeOfImage) rather than
        // parsing headers to locate its code sections. That is, we want to
        // avoid dealing with PE section alignment nuances.
        //
        MODULEINFO mi;
        if (GetModuleInformation (GetCurrentProcess (),
                                  GetModuleHandle (nullptr),
                                  &mi,
                                  sizeof (mi)))
        {
          DWORD o (0);
          if (!VirtualProtect (mi.lpBaseOfDll,
                               mi.SizeOfImage,
                               PAGE_EXECUTE_READWRITE,
                               &o))
          {
            MessageBox (nullptr,
                        "unable to change process protection",
                        "error",
                        MB_ICONERROR);
            exit (1);
          }
        }
        else
        {
          MessageBox (nullptr,
                      "unable to retrieve module information",
                      "error",
                      MB_ICONERROR);
          exit (1);
        }

        // Initialize log.
        //
        attach_console (), logger = new class logger;

        // Built-in patches.
        //
        memory::write (0x1402A91E5, "\xB0\x01");                                // Suppress XGameRuntimeInitialize call in WinMain
        memory::write (0x1402A91E7, 0x90, 3);                                   // ^
        memory::write (0x1402A6A4B, 0x90, 5);                                   // Suppress XCurl call in Live_Init
        memory::write (0x1402A6368, 0x90, 5);                                   // Suppress XCurl call in Live_Frame
        memory::write (0x1402A8CFE, 0x90, 5);                                   // Suppress GDK shutdown in Com_Quit_f (avoids crash)
        memory::write (0x1402A92B3, 0x90, 13);                                  // Suppress Sys_CheckCrashOrRerun call in WinMain
        memory::write (0x1401FAC87, 0x90, 7);                                   // Suppress com_safemode checks in Com_Init
        memory::write (0x1401FAC8E, 0xEB, 1);                                   // ^
        memory::write (0x1403B30B8, "// generated by IW4x, do not modify\n");   // Rename "Infinity Ward" to "IW4x"
        memory::write (0x1403B30DC, 0x00, 9);                                   // ^
        memory::write (0x1402864F0, "\xB0\x01\xC3");                            // Suppress Content_DoWeHaveContentPack
        memory::write (0x1400F85F8, 0x90, 26);                                  // Suppress CL_Live_BaseGameLicenseCheck
        memory::write (0x14012F720, "\xB8\x02\x00\x00\x00\xC3", 6);             // Suppress GDK connection state
        memory::write (0x14023D0F1, 0xEB, 1);                                   // Suppress SV_SendMessageToClient demonware fingerprint
        memory::write (0x140248b4C, 0x90, 5);                                   // Suppress SV_SendClientDatagram demonware relay notification
        memory::write (0x14023D064, 0x90, 6);                                   // Suppress playlist dvar check (?)

        // Built-in modules.
        //
        mod::scheduler_module ();
        mod::ui_module ();
        mod::demonware_module ();
        mod::party_module ();
        mod::oob_module ();
        mod::network_module ();
        mod::menu_module ();

        // __scrt_common_main_seh
        //
        return reinterpret_cast<int (*) ()> (0x140358D48) ();
      }));

      // clang-format off

      // Construct a 64-bit absolute jump. x64 has no single instruction for
      // this, so we use `FF 25` (JMP [RIP+0]) followed by the 64-bit address.
      //
      array<unsigned char, 14> seq
      ({
        static_cast<unsigned char> (0xFF),
        static_cast<unsigned char> (0x25),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (s       & 0xFF),
        static_cast<unsigned char> (s >> 8  & 0xFF),
        static_cast<unsigned char> (s >> 16 & 0xFF),
        static_cast<unsigned char> (s >> 24 & 0xFF),
        static_cast<unsigned char> (s >> 32 & 0xFF),
        static_cast<unsigned char> (s >> 40 & 0xFF),
        static_cast<unsigned char> (s >> 48 & 0xFF),
        static_cast<unsigned char> (s >> 56 & 0xFF)
      });

      DWORD o (0);
      if (VirtualProtect (reinterpret_cast<void*> (t),
                          seq.size (),
                          PAGE_EXECUTE_READWRITE,
                          &o) != 0)
      {
        memmove (reinterpret_cast<void*> (t), seq.data (), seq.size ());
      }
      else
        return FALSE;

      // If we made it here, we are attached to the process.
      //
      return TRUE;
    }
  }
}
