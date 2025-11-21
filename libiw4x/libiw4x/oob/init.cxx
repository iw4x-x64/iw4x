#include <libiw4x/oob/init.hxx>

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

using namespace std;

namespace iw4x
{
  namespace oob
  {
    namespace
    {
      // Command handler registry.
      //
      // Maps command names (case-sensitive) to their handler functions.
      //
      unordered_map<string, command_handler> command_handlers;

      // Parse an OOB message payload into command-line arguments.
      //
      // The message data is expected to follow Quake 3 connectionless packet
      // format: four 0xFF bytes followed by a command string.
      //
      // https://www.jfedor.org/quake3/
      // https://github.com/iw4x-x64/packet-quake3
      //
      vector<string>
      parse_command_line (const msg_t& message)
      {
        // OOB packets must have at least the 4-byte marker plus a command.
        //
        if (message.data == nullptr || message.cursize <= 4)
          return {};

        vector<string> arguments;
        arguments.reserve (8); // Typical command has fewer than 8 args.

        const char* current_position (message.data + 4);
        const char* message_end (message.data + message.cursize);

        string current_token;
        current_token.reserve (64); // Reasonable default for most tokens.

        bool inside_quotes (false);

        for (; current_position < message_end; ++current_position)
        {
          const char character (*current_position);

          // Null terminator or newline ends the command line.
          //
          if (character == '\0' || character == '\n')
            break;

          // Quote character toggles quoted string mode.
          //
          if (character == '"')
          {
            inside_quotes = !inside_quotes;
            continue;
          }

          // Outside quotes, whitespace delimits tokens.
          //
          if (!inside_quotes &&
              isspace (static_cast<unsigned char> (character)))
          {
            if (!current_token.empty ())
            {
              arguments.push_back (move (current_token));
              current_token.clear ();
            }
            continue;
          }

          // Accumulate character into the current token.
          //
          current_token += character;
        }

        // Finalize the last token if present.
        //
        if (!current_token.empty ())
          arguments.push_back (move (current_token));

        return arguments;
      }

      // Dispatch an OOB packet to the appropriate registered handler.
      //
      // Returns true if a handler was found and properly invoked, false
      // if no handler exists for the parsed command.
      //
      // Throws if parsing fails (invalid message format) or if the handler
      // throws an exception. Handler exceptions are not caught and are
      // propagated to the caller as fatal errors.
      //
      bool
      dispatch_oob_packet (const netadr_t& sender_address,
                           const msg_t& message)
      {
        vector<string> arguments (parse_command_line (message));

        // Command name must be present.
        //
        if (arguments.empty ())
          return false;

        const string& command_name (arguments[0]);

#if LIBIW4X_DEVELOP
        cout << "info: dispatching OOB command '" << command_name << "' with "
             << (arguments.size () - 1) << " argument(s)";

        if (arguments.size () > 1)
        {
          cout << ": [";
          for (size_t i (1); i < arguments.size (); ++i)
          {
            if (i > 1) cout << ", ";
            cout << "'" << arguments[i] << "'";
          }
          cout << "]";
        }

        cout << endl;
#endif

        // Lookup handler for this command.
        //
        auto handler_iterator (command_handlers.find (command_name));
        if (handler_iterator == command_handlers.end ())
          return false;

        // Invoke the handler, catching any exceptions it throws.
        //
        try
        {
          handler_iterator->second (sender_address, arguments);
          return true;
        }
        catch (const exception& error)
        {
          cerr << "error: OOB command '" << command_name << "' handler threw "
               << "exception: " << error.what () << endl;

          exit (1);
        }
        catch (...)
        {
          cerr << "error: OOB command '" << command_name << "' handler threw "
               << "unknown exception" << endl;

          exit (1);
        }
      }
    }

    // External assembly stub defined in init.asm.
    //
    // Calls the C interface dispatcher, and either returns to the caller (if
    // packet was handled) or resumes the original engine control flow (if
    // packet was not handled).
    //
    extern "C" void cl_dispatch_connectionless_packet_stub ();

    // C interface for the packet dispatcher, called from assembly stub.
    //
    // Its role is strictly to adapt the assembly-level call into a C++
    // invocation with properly typed arguments.
    //
    extern "C"
    {
      bool
      cl_dispatch_connectionless_packet (const netadr_t* sender_address,
                                         const msg_t* message)
      {
        if (sender_address == nullptr || message == nullptr)
          return false;

        return dispatch_oob_packet (*sender_address, *message);
      }
    }

    void
    register_command_handler (const string& command_name,
                              command_handler handler)
    {
      if (command_name.empty ())
        throw invalid_argument ("command name cannot be empty");

      // Check for duplicate registration.
      //
      if (command_handlers.find (command_name) != command_handlers.end ())
        throw invalid_argument ("command handler already registered for '" +
                                command_name + "'");

      command_handlers[command_name] = move (handler);
    }

    void
    init ()
    {
      uintptr_t t (0x1400F6040);
      uintptr_t s (reinterpret_cast<uintptr_t> (&cl_dispatch_connectionless_packet_stub));

      // Don't use minhook. Target site is not a conventional function prologue
      // and the dispatch path requires full control over the call frame.
      //
      array<unsigned char, 14> seq
      {
        static_cast<unsigned char> (0xFF),
        static_cast<unsigned char> (0x25),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (0x00),
        static_cast<unsigned char> (s       & 0xFF),
        static_cast<unsigned char> (s >>  8 & 0xFF),
        static_cast<unsigned char> (s >> 16 & 0xFF),
        static_cast<unsigned char> (s >> 24 & 0xFF),
        static_cast<unsigned char> (s >> 32 & 0xFF),
        static_cast<unsigned char> (s >> 40 & 0xFF),
        static_cast<unsigned char> (s >> 48 & 0xFF),
        static_cast<unsigned char> (s >> 56 & 0xFF)
      };

      memmove (reinterpret_cast<void*> (t), seq.data (), seq.size ());
    }
  }
}
