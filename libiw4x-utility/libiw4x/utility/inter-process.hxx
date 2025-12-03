#pragma once

#include <string>

#include <libiw4x/utility/export.hxx>

namespace iw4x
{
  namespace utility
  {
    class LIBIW4X_UTILITY_SYMEXPORT inter_process
    {
    public:
      inter_process () = default;
      ~inter_process ();

      // Create a new named pipe and prepare to accept a connection. This should
      // be called by the process that creates the pipe (typically the test
      // driver or server).
      //
      // Throw std::system_error on failure.
      //
      void
      create (std::string name);

      // Wait for and accept a connection from another process. This should be
      // called after create () and after the other process is ready to connect.
      //
      // Throw std::system_error on failure.
      //
      void
      connect ();

      // Open an existing named pipe for communication. This should be called
      // by the process that connects to an existing pipe (typically the test
      // executable or client).
      //
      // Throw std::system_error on failure.
      //
      void
      open (std::string name);

      // Send a message to the other end of the channel. The message size is
      // automatically prepended.
      //
      // Throw std::system_error on I/O failure.
      //
      void
      send (const std::string& message);

      // Receive a message from the other end of the channel. Blocks until a
      // complete message is available.
      //
      // Throw std::system_error on I/O failure or if the channel is closed.
      //
      std::string
      receive ();

      // Close the connection but keep the pipe for potential reconnection
      // (primarily useful for the creating process).
      //
      void
      disconnect ();

      // Check if the communication channel is in a valid state (i.e., has been
      // opened or connected).
      //
      bool
      is_connected () const;

      // Close all resources and clean up. Called automatically by destructor.
      //
      void
      close ();

    private:
      std::string name;         // Pipe name.
      void* handle = nullptr;   // HANDLE for Windows named pipe.
      bool server = false;      // True if we created the pipe (server side).
      bool connected = false;   // True if client/server are connected.

      // Write the specified buffer to the pipe.
      //
      void
      write_bytes (const void* buffer, std::size_t size);

      // Read exactly the specified number of bytes from the pipe into the
      // buffer.
      //
      // Throw std::system_error if EOF is encountered before reading all bytes.
      //
      void
      read_bytes (void* buffer, std::size_t size);
    };
  }
}
