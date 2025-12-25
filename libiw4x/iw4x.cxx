#include <libiw4x/iw4x.hxx>

#include <libiw4x/frame/init.hxx>
#include <libiw4x/menu/init.hxx>
#include <libiw4x/client/init.hxx>

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
        intptr_t stdout_handle (_get_osfhandle (_fileno (stdout)));
        intptr_t stderr_handle (_get_osfhandle (_fileno (stderr)));

        if (stdout_handle >= 0 || stderr_handle >= 0)
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
        bool stdout_rebound (false);
        bool stderr_rebound (false);

        if (freopen ("CONOUT$", "w", stdout) != nullptr &&
            _dup2 (_fileno (stdout), 1) != -1)
          stdout_rebound = true;

        if (freopen ("CONOUT$", "w", stderr) != nullptr &&
            _dup2 (_fileno (stderr), 2) != -1)
          stderr_rebound = true;

        // If stream were rebound, realign iostream objects (`cout`, `cerr`,
        // etc.) with C FILE streams.
        //
        if (stdout_rebound && stderr_rebound)
          ios::sync_with_stdio ();
      }
    }
  }

  extern "C"
  {
    BOOL WINAPI
    DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
    {
      if (fdwReason != DLL_PROCESS_ATTACH)
        return TRUE;

      // DllMain executes while the process loader lock is held, so we defer
      // initialization to IW4's main thread to avoid deadlocks and ordering
      // violation.
      //
      uintptr_t target (0x140358EBC);
      uintptr_t source (reinterpret_cast<uintptr_t> (+[] ()
      {
        // __security_init_cookie
        //
        reinterpret_cast<void (*) ()> (0x1403598CC) ();

        // IW4's binary is built with subsystem:windows and does not receive a
        // console by default. Attach to an existing one when available so that
        // diagnostic output become visible in interactive use.
        //
        attach_console ();

        // Under normal circumstances, a DLL is unloaded via FreeLibrary once
        // its reference count reaches zero. This is acceptable for auxiliary
        // libraries but unsuitable for modules like ours, which embed deeply
        // into the host process.
        //
        HMODULE m (nullptr);
        if (!GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_PIN |
                                  GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                reinterpret_cast<LPCTSTR> (DllMain),
                                &m))
        {
          cerr << "error: unable to mark module as permanent";
          exit (1);
        }

        // By default, the process inherits its working directory from whatever
        // environment or launcher invoked it, which may vary across setups and
        // lead to unpredictable relative path resolution.
        //
        // The strategy here is to explicitly realign the working directory to
        // the DLL's own location. That is, we effectively makes all relative
        // file operations resolve against the module's directory when the
        // process is hosted or started indirectly.
        //
        char p [MAX_PATH];
        if (GetModuleFileName (m, p, MAX_PATH))
        {
          string s (p);
          size_t i (s.rfind ('\\'));

          if (i == string::npos ||
              !SetCurrentDirectory (s.substr (0, i).c_str ()))
          {
            cerr << "error: unable to set module current directory";
            exit (1);
          }
        }
        else
        {
          cerr << "error: unable to retrieve module location";
          exit (1);
        }

        // Relax module's memory protection to permit writes to segments that
        // are otherwise read-only.
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
            cerr << "error: unable to change module protection";
            exit (1);
          }
        }
        else
        {
          cerr << "error: unable to retrieve module information";
          exit (1);
        }

        // Patch Game Development Kit (GDK) runtime
        //
        ([] (void (*_) (uintptr_t, int, size_t))
          {
            _(0x1401B2FCA, 0x31, 1); // Bypass XGameRuntimeInitialize
            _(0x1401B2FCB, 0xC0, 1); //
            _(0x1401B2FCC, 0x90, 3); //
            _(0x1401B308F, 0x31, 1); //
            _(0x1401B3090, 0xC0, 1); //
            _(0x1401B3091, 0x90, 3); //

            _(0x1402A6A4B, 0x90, 5); // Bypass xCurl
            _(0x1402A6368, 0x90, 5); //

            _(0x1402A5F70, 0x90, 3); // Bypass xboxlive_signed
            _(0x1402A5F73, 0x74, 1); //
            _(0x1400F5B86, 0xEB, 1); //
            _(0x1400F5BAC, 0xEB, 1); //
            _(0x14010B332, 0xEB, 1); //
            _(0x1401BA1FE, 0xEB, 1); //

            _(0x140271ED0, 0xC3, 1); // Bypass playlist
            _(0x1400F6BC4, 0x90, 2); //

            _(0x1400FC833, 0xEB, 1); // Bypass configstring mismatch
            _(0x1400D2AFC, 0x90, 2); //

            _(0x1400E4DA0, 0x33, 1); // Bypass stats mismatch
            _(0x1400E4DA1, 0xC0, 1); //
            _(0x1400E4DA2, 0xC3, 1); //
          })
        ([] (uintptr_t address, int value, size_t size)
          {
            memwrite (reinterpret_cast<void*> (address), value, size);
          });

        scheduler s;
        sched = &s;

        frame::init ();
        menu::init ();
        client::init ();

        // __scrt_common_main_seh
        //
        return reinterpret_cast<int (*) ()> (0x140358D48) ();
      }));

      // Build a 64-bit jump using a 6-byte RIP-relative opcode plus 8-byte
      // address, since x86-64 lacks a single-instruction that takes a 64-bit
      // immediate.
      //
      array<unsigned char, 14> seq
      ({
        static_cast<unsigned char> (0xFF),
        static_cast<unsigned char> (0x25),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (source       & 0xFF),
        static_cast<unsigned char> (source >> 8  & 0xFF),
        static_cast<unsigned char> (source >> 16 & 0xFF),
        static_cast<unsigned char> (source >> 24 & 0xFF),
        static_cast<unsigned char> (source >> 32 & 0xFF),
        static_cast<unsigned char> (source >> 40 & 0xFF),
        static_cast<unsigned char> (source >> 48 & 0xFF),
        static_cast<unsigned char> (source >> 56 & 0xFF)
      });

      DWORD o (0);

      if (VirtualProtect (reinterpret_cast<void*> (target),
                          seq.size (),
                          PAGE_EXECUTE_READWRITE,
                          &o) != 0)
      {
        memmove (reinterpret_cast<void*> (target), seq.data (), seq.size ());

        // Flushes the instruction cache.
        //
        // Note that on x86 and x86-64 architectures, flushing the
        // instruction cache is generally unnecessary provided that
        // both code modification and execution occur via the same
        // linear address. In practice, it's recommended to always
        // invoke FlushInstructionCache and allow the operating system
        // to determine whether any action is required.
        //
        // Moreover, according to Microsoft documentation, failing to
        // call FlushInstructionCache after modifying a region of
        // memory that will become executable is considered "undefined
        // behavior".
        //
        // For reference, see Intel(R) 64 and IA-32 Architectures
        // Software Developer's Manual, Volume 3A: System Programming
        // Guide, Part 1, Section 11.6: "Self-Modifying Code".
        //
        FlushInstructionCache (GetCurrentProcess (),
                               reinterpret_cast<const void*> (target),
                               seq.size ());
      }
      else
        return FALSE;

      // Successful DLL_PROCESS_ATTACH.
      //
      return TRUE;
    }
  }
}
