#include <libiw4x/context.hxx>
#include <libiw4x/scheduler.hxx>

namespace iw4x
{
  context::context (scheduler& s)
    : sched (s)
  {
    // Intentionally left empty by design
    //
  }
}
