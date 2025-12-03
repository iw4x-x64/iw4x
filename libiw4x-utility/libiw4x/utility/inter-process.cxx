#include <libiw4x/utility/inter-process.hxx>

#include <cstdint>
#include <system_error>

#include <libiw4x/utility/utility-win32.hxx>

using namespace std;

namespace iw4x
{
  namespace utility
  {
    void
    inter_process::create (string n)
    {
      close ();

      n.insert (0, "\\\\.\\pipe\\");

      SECURITY_ATTRIBUTES sa;
      sa.nLength = sizeof (SECURITY_ATTRIBUTES);
      sa.bInheritHandle = FALSE;
      sa.lpSecurityDescriptor = nullptr;

      handle =
        CreateNamedPipeA (n.c_str (),
                          PIPE_ACCESS_DUPLEX,       // Read/write access
                          PIPE_TYPE_BYTE |          // Byte-type pipe
                          PIPE_READMODE_BYTE |      // Byte-read mode
                          PIPE_WAIT,                // Blocking mode
                          PIPE_UNLIMITED_INSTANCES, // Allow reconnections
                          4096,                     // Output buffer size
                          4096,                     // Input buffer size
                          0,                        // Default timeout
                          &sa);

      if (handle == INVALID_HANDLE_VALUE)
        throw system_error (GetLastError (),
                            system_category (),
                            "unable to create pipe: " + n);

      name = move (n);
      server = true;
    }

    void
    inter_process::connect ()
    {
      // Wait for a client to connect.
      //
      if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        throw system_error (EINVAL, system_category (), "pipe not created");

      if (!ConnectNamedPipe (handle, nullptr))
      {
        DWORD e (GetLastError ());

        // ERROR_PIPE_CONNECTED means a client connected before we called
        // ConnectNamedPipe, which is fine.
        //
        if (e != ERROR_PIPE_CONNECTED)
          throw system_error (e,
                              system_category (),
                              "unable to accept connection: " + name);
      }

      connected = true;
    }

    void
    inter_process::open (string n)
    {
      close ();

      n.insert (0, "\\\\.\\pipe\\");

      // Wait for the pipe to become available.
      //
      if (!WaitNamedPipeA (n.c_str (), NMPWAIT_WAIT_FOREVER))
        throw system_error (GetLastError (),
                            system_category (),
                            "pipe not available: " + n);

      handle = CreateFileA (n.c_str (),
                            GENERIC_READ | GENERIC_WRITE, // Read/write access
                            0,                            // No sharing
                            nullptr,                      // Default security
                            OPEN_EXISTING,                // Open existing pipe
                            0,                            // Default attributes
                            nullptr);                     // No template

      if (handle == INVALID_HANDLE_VALUE)
        throw system_error (GetLastError (),
                            system_category (),
                            "unable to open pipe: " + n);

      name = move (n);
      server = false;
      connected = true;
    }

    void
    inter_process::send (const string& m)
    {
      if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        throw system_error (EINVAL, system_category (), "pipe not open");

      // Send message length first (as 32-bit unsigned integer in native byte
      // order since both processes are on the same machine).
      //
      uint32_t l (static_cast<uint32_t> (m.size ()));
      write_bytes (&l, sizeof (l));

      // Send message data.
      //
      if (l > 0)
        write_bytes (m.data (), l);
    }

    string
    inter_process::receive ()
    {
      if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        throw system_error (EINVAL, system_category (), "pipe not open");

      // Read message length.
      //
      uint32_t l;
      read_bytes (&l, sizeof (l));

      // Read message data.
      //
      if (l == 0)
        return string ();

      string m;
      m.resize (l);
      read_bytes (&m [0], l);

      return m;
    }

    void
    inter_process::disconnect ()
    {
      if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
      {
        if (server)
        {
          DisconnectNamedPipe (handle);
          // Keep handle open for potential reconnection.
        }
        else
        {
          CloseHandle (handle);
          handle = nullptr;
        }

        connected = false;
      }
    }

    bool
    inter_process::is_connected () const
    {
      return connected;
    }

    void
    inter_process::close ()
    {
      if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
      {
        if (server)
          DisconnectNamedPipe (handle);

        CloseHandle (handle);
        handle = nullptr;
      }

      server = false;
      connected = false;
      name.clear ();
    }

    inter_process::~inter_process ()
    {
      close ();
    }

    void
    inter_process::write_bytes (const void* buf, size_t n)
    {
      const char* b (static_cast<const char*> (buf));
      size_t written (0);

      while (written < n)
      {
        DWORD w;
        if (!WriteFile (handle,
                        b + written,
                        static_cast<DWORD> (n - written),
                        &w,
                        nullptr))
          throw system_error (GetLastError (),
                              system_category (),
                              "unable to write to pipe");

        written += static_cast<size_t> (w);
      }
    }

    void
    inter_process::read_bytes (void* buf, size_t n)
    {
      char* b (static_cast<char*> (buf));
      size_t nread (0);

      while (nread < n)
      {
        DWORD w;
        if (!ReadFile (handle,
                       b + nread,
                       static_cast<DWORD> (n - nread),
                       &w,
                       nullptr))
          throw system_error (GetLastError (),
                              system_category (),
                              "unable to read from pipe");

        if (w == 0)
          throw system_error (0, system_category (), "pipe closed");

        nread += static_cast<size_t> (w);
      }
    }
  }
}
