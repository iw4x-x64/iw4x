#include <libiw4x/iw4x.hxx>

#include <array>
#include <filesystem>
#include <iostream>

extern "C"
{
  #include <io.h>
}

#if LIBIW4X_CPPTRACE
#  include <cpptrace/cpptrace.hpp>
#  include <cpptrace/formatting.hpp>
#endif

#include <boost/dll/shared_library.hpp>

#include <libiw4x/frame/init.hxx>
#include <libiw4x/imgui/init.hxx>
#include <libiw4x/menu/init.hxx>
#include <libiw4x/network/init.hxx>
#include <libiw4x/oob/init.hxx>
#include <libiw4x/renderer/init.hxx>

#include <libiw4x/version.hxx>
#include <libiw4x/iw4x-options.hxx>

using namespace std;
using namespace std::filesystem;
using namespace boost::dll;

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

#if LIBIW4X_CPPTRACE
    void
    setup_cpptrace ()
    {
      using namespace cpptrace;

      using formatter        = cpptrace::formatter;
      using address_mode     = cpptrace::formatter::address_mode;
      using colors_mode      = cpptrace::formatter::color_mode;
      using symbol_mode      = cpptrace::formatter::symbol_mode;
      using stacktrace_frame = cpptrace::stacktrace_frame;

      // Configure the global cpptrace formatter to produce actionable stack
      // traces for diagnostics (particularly from terminate handlers).
      //
      // Note that the default formatter is returned as a const reference, so we
      // use const_cast to modify it. This is generally unsafe but acceptable
      // here since we're modifying a static object before any traces are
      // generated.
      //
      const_cast<formatter&> (get_default_formatter ())
        .addresses (address_mode::object)
        // Note that color may not survive terminal line wrapping, since the
        // terminal handles the wrap itself. Also, the automatic mode tends to
        // misbehave under Wine, presumably because the console detection does
        // not recognize the attached terminal.
        //
        .colors (colors_mode::always)
        .snippets (true)
        .symbols (symbol_mode::pretty)
        .filter ([] (const stacktrace_frame& frame)
      {
        const string& s (frame.symbol);
        const string& n (frame.filename);

        // Performs an unconstrained search since symbol names are often
        // decorated and may embed the target substring without any stable
        // boundary.
        //
        auto
        contains ([&] (const string& str, const string& substr)
        {
          return str.find (substr) != string::npos;
        });

        // Filter out known runtime and system frames that are unhelpful in
        // stack traces.
        //
        return (contains (s, "terminate_handler")                ||
                contains (s, "__terminate")                      ||
                contains (s, "std::terminate")                   ||
                contains (s, "__cxa_rethrow")                    ||
                contains (s, "__tmainCRTStartup")                ||
                contains (s, "WinMainCRTStartup")                ||
                contains (s, "BaseThreadInitThunk")              ||
                contains (s, "dispatch_exception")               ||
                contains (s, "call_seh_handlers")                ||
                contains (s, "call_unhandled_exception_handler") ||
                contains (s, "call_unhandled_exception_filter")  ||
                contains (s, "UnhandledExceptionFilter")         ||
                contains (s, "boost::asio::detail")              ||
                contains (s, "boost::asio::io_context")          ||
                contains (n, "ntdll.dll")                        ||
                contains (n, "kernelbase")                       ||
                contains (n, "mingw/include")                    ||
                contains (n, "libboost-asio"))                   == false;
      });

      // Register cpptrace terminate handler.
      //
      register_terminate_handler ();

      // Register Windows unhandled exception filter to catch exceptions that
      // escape the normal C++ exception handling mechanism.
      //
      SetUnhandledExceptionFilter ([] (EXCEPTION_POINTERS* ep) -> LONG
      {
        cerr << "unhandled Windows exception at 0x" << hex
             << ep->ExceptionRecord->ExceptionAddress << dec << endl;

        generate_trace (1);

        return EXCEPTION_EXECUTE_HANDLER;
      });
    }
#endif
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

        // Setup cpptrace to generate stack trace from terminate handler.
        //
#if LIBIW4X_CPPTRACE
        setup_cpptrace ();
#endif

        HMODULE m (nullptr);

        // Under normal circumstances, a DLL is unloaded via FreeLibrary once
        // its reference count reaches zero. This is acceptable for auxiliary
        // libraries but unsuitable for modules like ours, which embed deeply
        // into the host process.
        //
        auto
        pin ([&m] (void* fp, const string& name)
        {
          if (!GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_PIN |
                                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                  reinterpret_cast<LPCTSTR> (fp),
                                  &m))
          {
            cerr << "error: unable to mark '" << name << "' as permanent";
            exit (1);
          }
        });

        pin (reinterpret_cast<void*> (DllMain), "module"), assert (m);

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

        // Parse the command line arguments.
        //
        // Note that when running in a context without an active output stream,
        // any attempts to write to 'cout' through our handlers will have no
        // effect.
        //
        // Note also that in cases where a handler is intended to terminate the
        // program after printing (e.g., --help or --version), it will still
        // exit as intended. That is, no special handling is performed for the
        // absence of an active stream.
        //
        options opt (__argc, __argv);

        // Handle --version.
        //
        if (opt.version ())
        {
          cout << "IW4x " << LIBIW4X_VERSION_ID << "\n";

          exit (0);
        }

        // Handle --help.
        //
        if (opt.help ())
        {
          cout << "usage: iw4x [options] <names>" << "\n"
               << "options:"                      << "\n";

          opt.print_usage (cout);

          exit (0);
        }

        // Patch runtime checks and service initialization.
        //
        // Removes references to Xbox Live and XGameRuntime and disables
        // related handling paths.
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
            memset (reinterpret_cast<void*> (address), value, size);
          });

        scheduler s;
        frame::init (s);
        menu::init (s);
        renderer::init ();
        imgui::init ();
        network::init (s);
        oob::init ();

        // Load external modules discovered under the `module` directory.
        //
        // We treat this directory as an optional extension point: any DLL
        // matching `libiw4x-*.dll` naming pattern is considered a candidate
        // module.
        //
        if (path p ("module"); exists (p) && is_directory (p))
        {
          for (const auto& e : directory_iterator (p))
          {
            // Ignore non-regular entries (directories, symlinks, etc.).
            //
            if (!e.is_regular_file ())
              continue;

            auto m (e.path ());
            auto n (m.filename ().string ());

            // Only consider DLLs matching `libiw4x-*.dll` naming pattern
            //
            if (n.size () < 13 ||
                n.substr (0, 8) != "libiw4x-" ||
                n.substr (n.size () - 4) != ".dll")
              continue;

            try
            {
              shared_library lib (m.string (),
                                  load_mode::rtld_now |
                                    load_mode::rtld_global);

              // Resolve the module's canonical entry point, if present.
              //
              if (string s ("init"); lib.has (s))
              {
                // At this point we would normally just invoke the function
                // returned by `lib.get()`, but in our case we also need to pin
                // the module to the application lifetime.
                //
                void (*fp) () (lib.get<void ()> (s)); fp ();

                // Mark the module as permanent. See `pin()` for the rationale.
                //
                pin (reinterpret_cast<void*> (fp), n);
              }
            }
            catch (...)
            {
              // Consider failure non-fatal. A single misbehaving DLL should not
              // compromise startup. The intent is to maintain a best-effort
              // module environment rather than enforce an all-or-nothing
              // policy.
              //
              // In IW4x this is sufficient: the core provides all required
              // functionality, and external modules are strictly optional.
              //
              cout << "warn: unable to load module '" << n << "'" << endl;
            }
          }
        }

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
      }
      else
        return FALSE;

      // Successful DLL_PROCESS_ATTACH.
      //
      return TRUE;
    }
  }
}
