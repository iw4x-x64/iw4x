#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <libiw4x/iw4x.hxx>
#include <libiw4x/export.hxx>

namespace iw4x
{
  // Strand-based task scheduler for serialized execution contexts.
  //
  class LIBIW4X_SYMEXPORT scheduler
  {
  public:
    using strand_type =
      boost::asio::strand<boost::asio::io_context::executor_type>;

    scheduler ();
    ~scheduler ();

    scheduler (const scheduler&) = delete;
    scheduler& operator= (const scheduler&) = delete;

    // Create a named strand.
    //
    // Return false if the name already exists.
    //
    bool
    create (const string& name);

    // Destroy a named strand.
    //
    // Return false if the name does not exist.
    //
    bool
    destroy (const string& name);

    // Post work to a named strand.
    //
    // Return false if the strand does not exist.
    //
    template <typename F> bool
    post (const string& name, F&& work)
    {
      strand_type* s (find (name));

      if (s == nullptr)
        return false;

      boost::asio::post (*s, forward<F> (work));
      return true;
    }

    // Poll the io_context, executing ready handlers.
    //
    // If name is provided, poll only that strand's context.
    // Otherwise, poll the entire io_context.
    //
    void
    poll (const string& name);

    // Check if a named strand exists.
    //
    bool
    exists (const string& name) const;

  private:
    unique_ptr<boost::asio::io_context> context;
    unordered_map<string, strand_type> strands;

    strand_type*
    find (const string& name);

    const strand_type*
    find (const string& name) const;
  };
}
