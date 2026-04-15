#pragma once

#include <mutex>

#include <libiw4x/mod/oob/oob-dispatcher.hxx>
#include <libiw4x/mod/oob/oob-types.hxx>

#include <libiw4x/import.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace oob
    {
      class oob_pipeline
      {
      public:
        explicit
        oob_pipeline (oob_dispatcher& dispatcher);

        oob_pipeline (const oob_pipeline&) = delete;
        oob_pipeline& operator = (const oob_pipeline&) = delete;

        oob_disposition
        process (const network_address&, const message&);

        void
        tick ();

      private:
        oob_dispatcher& dispatcher_;
        std::mutex mutex_;
        std::vector<oob_message> pending_;
      };
    }
  }
}
