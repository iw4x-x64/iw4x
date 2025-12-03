#include <libiw4x/renderer/init.hxx>

namespace iw4x
{
  namespace renderer
  {
    namespace
    {
      vector<function<void (IDirect3DDevice9*)>> callbacks;

      using endscene_fn      = HRESULT (APIENTRY*) (IDirect3DDevice9*);
      using create_device_fn = HRESULT (APIENTRY*) (IDirect3D9*,
                                                     UINT,
                                                     D3DDEVTYPE,
                                                     HWND,
                                                     DWORD,
                                                     D3DPRESENT_PARAMETERS*,
                                                     IDirect3DDevice9**);

      endscene_fn      orig_endscene      (nullptr);
      create_device_fn orig_create_device (nullptr);

      void
      dispatch_frame (IDirect3DDevice9* d)
      {
        assert (d != nullptr);

        // Fire callbacks. If any callback throws, then bail out.
        //
        try
        {
          for (auto& cb : callbacks)
            cb (d);
        }
        catch (const std::exception& e)
        {
          ostringstream s;

          s << "exception in frame callback: "
            << e.what ();

          throw runtime_error (s.str ());
        }
        catch (...)
        {
          throw runtime_error ("unknown exception in frame callback");
        }
      }
    }

    void
    on_frame (function<void (IDirect3DDevice9*)> cb)
    {
      callbacks.push_back (std::move (cb));
    }

    void
    init ()
    {
      // Obtain d3d9 module handle. Note that at this point, d3d9.dll is already
      // loaded (either directly or transitively) for the current process.
      //
      HMODULE dm (GetModuleHandle ("d3d9"));
      if (dm == nullptr)
        throw runtime_error ("unable to retreive d3d9 module handle");

      FARPROC dc (GetProcAddress (dm, "Direct3DCreate9"));
      if (dc == nullptr)
        throw runtime_error ("unable to retreive Direct3DCreate9 address");

      // Relax d3d9 module's memory protection to permit writes to vtables.
      //
      MODULEINFO mi;
      if (GetModuleInformation (GetCurrentProcess (),
                                dm,
                                &mi,
                                sizeof (mi)))
      {
        DWORD o (0);
        if (!VirtualProtect (mi.lpBaseOfDll,
                             mi.SizeOfImage,
                             PAGE_EXECUTE_READWRITE,
                             &o))
          throw runtime_error ("unable to change d3d9 module protection");
      }
      else
        throw runtime_error ("unable to retrieve d3d9 module information");

      // Create a temporary Direct3D interface to extract its vtable.
      //
      using create_fn = IDirect3D9*(WINAPI*) (UINT);
      IDirect3D9* di (reinterpret_cast<create_fn> (dc) (D3D_SDK_VERSION));

      if (di == nullptr)
        throw runtime_error ("unable to create IDirect3D9 interface");

      // Extract the vtable pointer. In the COM ABI, the first member of any
      // interface object is the vtable pointer.
      //
      // Layout:
      //   [object] +0x00 -> vtable pointer
      //   [vtable] +0x00 -> QueryInterface
      //            +0x04 -> AddRef
      //            +0x08 -> Release
      //            ...
      //            +0x40 -> CreateDevice (vtable[16])
      //
      void** vt (*reinterpret_cast<void***> (di));

      // Hook CreateDevice (vtable entry 16).
      //
      // Save the original CreateDevice pointer.
      //
      orig_create_device = reinterpret_cast<create_device_fn> (vt [16]);

      // Install our hook.
      //
      vt [16] = reinterpret_cast<void*> (+[] (IDirect3D9* d3d,
                                              UINT a,
                                              D3DDEVTYPE dt,
                                              HWND hw,
                                              DWORD bf,
                                              D3DPRESENT_PARAMETERS* pp,
                                              IDirect3DDevice9** dv) -> HRESULT
      {
        // Call the original CreateDevice first.
        //
        HRESULT hr (orig_create_device (d3d, a, dt, hw, bf, pp, dv));

        if (SUCCEEDED (hr))
        {
          // Extract the device vtable and hook EndScene (entry 42).
          //
          void** dvt (*reinterpret_cast<void***> (*dv));

          orig_endscene = reinterpret_cast<endscene_fn> (dvt [42]);

          dvt [42] =
            reinterpret_cast<void*> (+[] (IDirect3DDevice9* d) -> HRESULT
          {
            // Dispatch frame notification before calling original.
            //
            dispatch_frame (d);

            return orig_endscene (d);
          });
        }

        return hr;
      });

      // Release the temporary interface.
      //
      di->Release ();
    }
  }
}
