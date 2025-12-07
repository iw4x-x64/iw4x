#pragma once

#include <string>

#include <libiw4x/export.hxx>

namespace iw4x
{
  LIBIW4X_SYMEXPORT bool
  load_module (const std::string& name);
}
