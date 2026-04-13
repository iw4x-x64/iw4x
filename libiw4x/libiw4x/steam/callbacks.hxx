#pragma once

#include <functional>
#include <unordered_map>

#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace steam
  {
    // Represent a callback handler.
    //
    // Note that the void pointer points to the raw callback struct exactly as
    // defined by Steamworks. We rely on each handler to know its expected type
    // and cast the payload accordingly.
    //
    using callback_handler = std::function<void (void*)>;

    // Register a handler for a specific Steam callback ID.
    //
    LIBIW4X_SYMEXPORT void
    register_callback (int callback_id, callback_handler handler);

    // Unregister all handlers currently associated with the given id.
    //
    LIBIW4X_SYMEXPORT void
    unregister_callback (int callback_id);

    // Dispatch a callback to all registered handlers for this id.
    //
    // Note that we expect this to be called internally by the pump_callbacks
    // machinery. It is generally not intended to be invoked directly from the
    // outside.
    //
    LIBIW4X_SYMEXPORT void
    dispatch_callback (int callback_id, void* data);
  }
}
