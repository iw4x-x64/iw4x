#include <libiw4x/utility/scheduler.hxx>

#include <stdexcept>
#include <utility>

#include <libiw4x/utility/xxhash.hxx>

using namespace std;

namespace iw4x
{
  namespace utility
  {
    // scheduler::strand
    //

    scheduler::strand::
    strand (const string& n)
        : name (n), tasks (), mtx ()
    {
    }

    scheduler::strand::
    ~strand ()
    {
    }

    void scheduler::strand::
    enqueue (function<void ()> t)
    {
      lock_guard<mutex> lck (mtx);
      tasks.push_back (move (t));
    }

    vector<function<void ()>> scheduler::strand::
    extract_pending ()
    {
      vector<function<void ()>> pnd;

      {
        lock_guard<mutex> lck (mtx);
        pnd.swap (tasks);
      }

      return pnd;
    }

    bool scheduler::strand::
    has_pending () const
    {
      lock_guard<mutex> lck (mtx);
      return !tasks.empty ();
    }

    void scheduler::strand::
    clear ()
    {
      lock_guard<mutex> lck (mtx);
      tasks.clear ();
    }

    // scheduler
    //

    scheduler::
    scheduler ()
      : strands_ (), registry_mutex_ ()
    {
    }

    scheduler::
    ~scheduler ()
    {
      lock_guard<mutex> lck (registry_mutex_);
      strands_.clear ();
    }

    void scheduler::
    register_strand (const string& n)
    {
      if (n.empty ())
        throw invalid_argument ("strand name cannot be empty");

      uint64_t h (xxh64 (n));
      lock_guard<mutex> lck (registry_mutex_);

      // Check for duplicate names.
      //
      if (strands_.find (h) != strands_.end ())
        throw invalid_argument ("strand name already registered: " + n);

      strands_.emplace (h, make_unique<strand> (n));
    }

    void scheduler::
    unregister_strand (const string& n)
    {
      if (n.empty ())
        throw invalid_argument ("strand name cannot be empty");

      uint64_t h (xxh64 (n));
      lock_guard<mutex> lck (registry_mutex_);

      if (strands_.find (h) == strands_.end ())
        throw invalid_argument ("strand not registered: " + n);

      strands_.erase (h);
    }

    void scheduler::
    poll (const string& n)
    {
      uint64_t h (xxh64 (n));
      strand* ctx (find_strand (h));

      if (ctx == nullptr)
        throw invalid_argument ("strand not registered: " + n);

      process_tasks (*ctx);
    }

    void scheduler::
    enqueue_task (const string& n, function<void ()> f)
    {
      uint64_t h (xxh64 (n));
      strand* ctx (find_strand (h));

      if (ctx == nullptr)
        throw invalid_argument ("strand not registered: " + n);

      ctx->enqueue (move (f));
    }

    bool scheduler::
    has_pending (const string& n) const
    {
      uint64_t h (xxh64 (n));
      const strand* ctx (find_strand (h));

      if (ctx == nullptr)
        throw invalid_argument ("strand not registered: " + n);

      return ctx->has_pending ();
    }

    bool scheduler::
    is_registered (const string& n) const
    {
      uint64_t h (xxh64 (n));
      lock_guard<mutex> lck (registry_mutex_);
      return strands_.find (h) != strands_.end ();
    }

    scheduler::strand* scheduler::
    find_strand (uint64_t h) noexcept
    {
      lock_guard<mutex> lck (registry_mutex_);

      unordered_map<uint64_t, unique_ptr<strand>>::iterator it (
        strands_.find (h));

      return it != strands_.end () ? it->second.get () : nullptr;
    }

    const scheduler::strand* scheduler::
    find_strand (uint64_t h) const noexcept
    {
      lock_guard<mutex> lck (registry_mutex_);

      unordered_map<uint64_t, unique_ptr<strand>>::const_iterator it (
        strands_.find (h));

      return it != strands_.end () ? it->second.get () : nullptr;
    }

    void scheduler::
    process_tasks (strand& ctx)
    {
      vector<function<void ()>> tsks (ctx.extract_pending ());

      // Execute tasks outside the lock.
      //
      for (function<void ()>& tsk : tsks)
        tsk ();
    }
  }
}
