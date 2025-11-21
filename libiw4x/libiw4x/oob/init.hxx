#pragma once

#include <libiw4x/import.hxx>
#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace oob
  {
    // Command handler function signature.
    //
    // Handlers receive the sender's network address and a vector of parsed
    // command-line arguments (similar to argc/argv). The first argument is the
    // command name itself.
    //
    using command_handler =
      std::function<void (const netadr_t& sender_address,
                          const std::vector<std::string>& arguments)>;

    // Register handler for an out-of-band command.
    //
    // When an OOB packet arrives with the specified command name as its first
    // token, the registered handler will be invoked with the sender address
    // and parsed arguments.
    //
    // Throws std::invalid_argument if the command name is empty.
    // Throws std::invalid_argument if a handler for this command already
    // exists.
    //
    LIBIW4X_SYMEXPORT void
    register_command_handler (const std::string& command_name,
                              command_handler handler);

    // Initialize out-of-band packet handling subsystem.
    //
    LIBIW4X_SYMEXPORT void
    init ();
  }
}
