#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <libiw4x/import.hxx>

namespace iw4x
{
  namespace menu_loader
  {
    struct parsed_menu_file
    {
      std::string source_path;
      menu_list* menu_list_asset;
      std::vector<menu_definition*> menus;
      std::shared_ptr<void> keepalive;
    };

    std::optional<parsed_menu_file>
    load_menu_file (const std::string& source_path);
  }
}
