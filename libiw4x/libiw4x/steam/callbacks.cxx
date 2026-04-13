#include <libiw4x/steam/callbacks.hxx>

#include <mutex>
#include <vector>

#include <libiw4x/logger.hxx>

using namespace std;

namespace iw4x
{
  namespace steam
  {
    namespace
    {
      // The registry maps a callback id to a list of handlers.
      //
      // Keep in mind that handlers can be registered from non-frame threads
      // (for example, during early module initialization). On the other hand,
      // dispatch always happens on the frame thread. So we have to protect
      // access with a mutex. It's heavy, but registry modifications are rare
      // enough in practice.
      //
      struct registry
      {
        mutex mtx;
        unordered_map<int, vector<callback_handler>> handlers;
      };

      registry&
      get_registry ()
      {
        static registry r;
        return r;
      }
    }

    void
    register_callback (int callback_id, callback_handler handler)
    {
      auto& r (get_registry ());
      lock_guard g (r.mtx);

      // We just move the handler into the vector. If the vector doesn't
      // exist yet, the map will default-construct it for us.
      //
      r.handlers[callback_id].push_back (move (handler));
    }

    void
    unregister_callback (int callback_id)
    {
      auto& r (get_registry ());
      lock_guard g (r.mtx);
      r.handlers.erase (callback_id);
    }

    void
    dispatch_callback (int callback_id, void* data)
    {
      auto& r (get_registry ());

      // We should not hold the mutex while dispatching. A handler might take a
      // while, or worse, try to register or unregister another callback and
      // deadlock us. So we just copy the handler list under the lock. Yes, it's
      // an allocation, but callbacks aren't typically dispatched millions of
      // times per frame.
      //
      vector<callback_handler> local;
      {
        lock_guard g (r.mtx);
        auto it (r.handlers.find (callback_id));
        if (it == r.handlers.end ())
          return;
        local = it->second;
      }

      for (auto& h : local)
      {
        try
        {
          h (data);
        }
        catch (const exception& e)
        {
          // We swallow the exception here. If a handler throws, it shouldn't
          // bring down the entire steam callback pump. Just log it and move on.
          //
          log::error << "steam: callback " << callback_id
                     << " handler threw: " << e.what ();
        }
      }
    }
  }
}
