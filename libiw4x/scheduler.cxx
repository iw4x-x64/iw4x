#include <libiw4x/scheduler.hxx>

using namespace boost::asio;

namespace iw4x
{
  scheduler::scheduler ()
    : context (make_unique<io_context> ()),
      strands ()
  {
    // Intentionally left empty by design
    //
  }

  scheduler::~scheduler ()
  {
    // Intentionally left empty by design
    //
  }

  bool
  scheduler::create (const string& n)
  {
    auto r (strands.try_emplace (n, make_strand (*context)));
    return r.second;
  }

  bool
  scheduler::destroy (const string& n)
  {
    return strands.erase (n) != 0;
  }

  void
  scheduler::poll (const string& n)
  {
    strand_type* s (find (n));

    if (s != nullptr)
    {
      // Once all queued work has completed, io_context transitions to the
      // stopped state. Any subsequent call to poll(), run(), run_one(), or
      // poll_one() will then return immediately without processing newly added
      // tasks.
      //
      // To actually permit a second (or later) round of work submission, we
      // must first explicitly restart() the context before invoking poll()
      // again.
      //
      if (context->stopped ())
        context->restart ();
      context->poll ();
    }
  }

  bool
  scheduler::exists (const string& n) const
  {
    return find (n) != nullptr;
  }

  scheduler::strand_type*
  scheduler::find (const string& n)
  {
    auto i (strands.find (n));
    return i != strands.end () ? &i->second : nullptr;
  }

  const scheduler::strand_type*
  scheduler::find (const string& n) const
  {
    auto i (strands.find (n));
    return i != strands.end () ? &i->second : nullptr;
  }

  // Process-wide scheduler instance. See header for details.
  //
  scheduler* sched (nullptr);
}
