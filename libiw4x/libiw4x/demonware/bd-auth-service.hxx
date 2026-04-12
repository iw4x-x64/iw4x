#pragma once

#include <libiw4x/demonware/bd-auth-ticket.hxx>

#include <libiw4x/export.hxx>

namespace iw4x
{
  namespace demonware
  {
    class LIBIW4X_SYMEXPORT auth_service
    {
    public:
      auth_service ();

      static const bd_auth_ticket&
      ticket ();
    };
  }
}
