#pragma once

#include <libiw4x/export.hxx>

namespace iw4x
{
  // Forward declarations
  //
  class scheduler;

  // Process-wide execution context.
  //
  // Note that context should always have reference semantics. That is, it must
  // never own the lifecycle of subsystems it points to.
  //
  struct LIBIW4X_SYMEXPORT context
  {
    scheduler& sched;

    explicit
    context (scheduler&);
  };

  // `context` is required by a large portion of the detour surface and
  // therefore must be reachable where parameter injection is not possible. The
  // detour mechanism does not (yet) support passing user-defined state, and
  // introducing per-detour wrappers solely to thread this dependency would be
  // structurally redundant and unmaintainable.
  //
  // Note that ctx instance is registered during `DllMain` execution, from
  // within the game entry-point detour. Its lifetime is bound to the process
  // and is expected to remain valid for the entire duration of the
  // application.
  //
  extern LIBIW4X_SYMEXPORT context& ctx;
}
