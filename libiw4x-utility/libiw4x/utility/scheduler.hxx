#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <libiw4x/utility/export.hxx>

namespace iw4x
{
  namespace utility
  {
    class LIBIW4X_UTILITY_SYMEXPORT scheduler
    {
    public:
      scheduler ();
      ~scheduler ();

      scheduler (const scheduler&) = delete;
      scheduler& operator= (const scheduler&) = delete;

      // Register a new strand with the given name.
      //
      // Strand names must be unique. Throws std::invalid_argument if the
      // name is empty or already registered.
      //
      void
      register_strand (const std::string& name);

      // Unregister a strand, clearing any pending tasks.
      //
      // Throws std::invalid_argument if the strand is not registered.
      //
      void
      unregister_strand (const std::string& name);

      // Poll one strand to process pending tasks.
      //
      // This is non-blocking and processes all ready tasks. Throws
      // std::invalid_argument if the strand is not registered.
      //
      void
      poll (const std::string& name);

      // Post an immediate task to the specified strand.
      //
      // The task is enqueued and will execute when the strand is polled.
      // Throws std::invalid_argument if the strand is not registered.
      //
      template <typename F>
      void
      post (const std::string& strand_name, F&& function)
      {
        enqueue_task (strand_name,
                      std::function<void ()> (static_cast<F&&> (function)));
      }

      // Check if a strand has pending tasks.
      //
      // Throws std::invalid_argument if the strand is not registered.
      //
      bool
      has_pending (const std::string& name) const;

      // Check if a strand is registered.
      //
      bool
      is_registered (const std::string& name) const;

    private:
      // Strand adapter holds the task queue for a single strand.
      //
      struct strand
      {
        std::string name;
        std::vector<std::function<void ()>> tasks;
        mutable std::mutex mtx;

        explicit
        strand (const std::string& strand_name);

       ~strand ();

        strand (const strand&) = delete;
        strand& operator= (const strand&) = delete;

        void
        enqueue (std::function<void ()> task);

        std::vector<std::function<void ()>>
        extract_pending ();

        bool
        has_pending () const;

        void
        clear ();
      };

      std::unordered_map<std::uint64_t, std::unique_ptr<strand>> strands_;
      mutable std::mutex registry_mutex_;

      void
      enqueue_task (const std::string& name, std::function<void ()>);

      strand*
      find_strand (std::uint64_t hash) noexcept;

      const strand*
      find_strand (std::uint64_t hash) const noexcept;

      void
      process_tasks (strand&);
    };
  }
}
