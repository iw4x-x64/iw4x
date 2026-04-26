#include <libiw4x/mod/mod-menu.hxx>

#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <libiw4x/detour.hxx>
#include <libiw4x/logger.hxx>
#include <libiw4x/scheduler.hxx>
#include <libiw4x/shim/open-asset-tools/open-asset-tools.hxx>

namespace iw4x
{
  namespace mod
  {
    namespace
    {
      constexpr int max_context_menus (640);
      constexpr int max_menu_stack (16);

      using menu_key = std::string;

      enum class reload_pass
      {
        ui_initialization,
        cgame_initialization
      };

      enum class reload_stage
      {
        wait_for_ui_initialization,
        wait_for_cgame_initialization,
        complete
      };

      struct live_menu_slot
      {
        ui_context*      context;
        int              index;
        menu_definition* original;
        menu_definition* replacement;
        bool             stack_slot;   // True: menu_stack[], false: menus[].
      };

      struct appended_menu
      {
        ui_context* context;
        menu_definition* menu;
      };

      struct owned_menu_list
      {
        std::unique_ptr<menu_list>           asset;
        std::unique_ptr<menu_definition*[]>  menus;
      };

      struct menu_override_store
      {
        std::vector<std::shared_ptr<void>>              keepalive;
        std::vector<owned_menu_list>                    owned_menu_lists;

        std::unordered_map<menu_key, menu_definition*>  menus_by_name;
        std::unordered_map<menu_key, bool>              appendable_by_name;
        std::unordered_map<menu_key, menu_list*>        menu_lists_by_path;

        std::vector<live_menu_slot>                     replaced_slots;
        std::vector<appended_menu>                      appended_menus;
      };

      std::atomic<std::shared_ptr<menu_override_store>> active_store;
      reload_stage
        startup_reload_stage (reload_stage::wait_for_ui_initialization);
      thread_local bool db_find_xasset_header_bypass (false);

      menu_key
      normalize_key (std::string_view input)
      {
        menu_key out;
        out.reserve (input.size ());

        for (unsigned char c : input)
        {
          if (c == '\\')
            out.push_back ('/');
          else
            out.push_back (static_cast<char> (std::tolower (c)));
        }

        return out;
      }

      menu_key
      menu_name_key (const menu_definition* menu)
      {
        if (menu == nullptr || menu->window.name == nullptr)
          return {};

        return normalize_key (menu->window.name);
      }

      int
      bounded_menu_count (const ui_context* c)
      {
        if (c == nullptr)
          return 0;

        return std::clamp (c->menu_count, 0, max_context_menus);
      }

      int
      bounded_open_menu_count (const ui_context* c)
      {
        if (c == nullptr)
          return 0;

        return std::clamp (c->open_menu_count, 0, max_menu_stack);
      }

      std::array<ui_context*, 2>
      ui_contexts ()
      {
        return {UI_GetFrontendContext (), UI_GetIngameContext ()};
      }

      bool
      has_menu_in_live_contexts (const menu_key& name_key)
      {
        for (ui_context* c : ui_contexts ())
        {
          const int n (bounded_menu_count (c));

          for (int i (0); i < n; ++i)
          {
            const menu_definition* m (c->menus [i]);

            if (m == nullptr || m->window.name == nullptr)
              continue;

            if (normalize_key (m->window.name) == name_key)
              return true;
          }
        }

        return false;
      }

      void
      remove_menu_from_context (ui_context* context, menu_definition* menu)
      {
        if (context == nullptr || menu == nullptr)
          return;

        int write (0);
        const int n (bounded_menu_count (context));

        for (int read (0); read < n; ++read)
        {
          menu_definition* m (context->menus [read]);

          if (m != menu)
            context->menus [write++] = m;
        }

        for (int i (write); i < n; ++i)
          context->menus [i] = nullptr;

        context->menu_count = write;
      }

      void
      restore_live_store (menu_override_store& store)
      {
        for (const live_menu_slot& slot : store.replaced_slots)
        {
          if (slot.context == nullptr || slot.index < 0)
            continue;

          if (slot.stack_slot)
          {
            if (slot.index >= bounded_open_menu_count (slot.context))
              continue;

            if (slot.context->menu_stack [slot.index] == slot.replacement)
              slot.context->menu_stack [slot.index] = slot.original;
          }
          else
          {
            if (slot.index >= bounded_menu_count (slot.context))
              continue;

            if (slot.context->menus [slot.index] == slot.replacement)
              slot.context->menus [slot.index] = slot.original;
          }
        }

        // Clean up any entirely new menus we appended so they don't leak into
        // the subsequent generations.
        for (const appended_menu& a : store.appended_menus)
          remove_menu_from_context (a.context, a.menu);
      }

      bool
      try_replace_slot (menu_override_store& store,
                        ui_context* context,
                        int index,
                        menu_definition* current,
                        bool stack_slot,
                        std::unordered_set<menu_key>& matched)
      {
        if (current == nullptr || current->window.name == nullptr)
          return false;

        const menu_key key (normalize_key (current->window.name));
        const auto it (store.menus_by_name.find (key));

        if (it == store.menus_by_name.end () || it->second == current)
          return false;

        store.replaced_slots.push_back (
          live_menu_slot {context, index, current, it->second, stack_slot});

        if (stack_slot)
          context->menu_stack [index] = it->second;
        else
          context->menus [index] = it->second;

        matched.insert (key);
        return true;
      }

      void
      append_unmatched_menus (menu_override_store& store,
                              const std::unordered_set<menu_key>& matched)
      {
        ui_context* frontend (UI_GetFrontendContext ());

        for (const auto& [name_key, menu] : store.menus_by_name)
        {
          if (matched.contains (name_key))
            continue;

          const auto it (store.appendable_by_name.find (name_key));

          if (it == store.appendable_by_name.end () || !it->second)
            continue;

          if (frontend == nullptr ||
              bounded_menu_count (frontend) >= max_context_menus)
          {
            log::warning << "menu override could not append new menu \""
                         << name_key << "\"";
            continue;
          }

          frontend->menus [frontend->menu_count++] = menu;
          store.appended_menus.push_back ({frontend, menu});
        }
      }

      void
      apply_live_store (menu_override_store& store)
      {
        std::unordered_set<menu_key> matched;

        for (ui_context* c : ui_contexts ())
        {
          const int n_menus (bounded_menu_count (c));
          const int n_stack (bounded_open_menu_count (c));

          for (int i (0); i < n_menus; ++i)
            try_replace_slot (store,
                              c,
                              i,
                              c->menus [i],
                              /* stack_slot */ false,
                              matched);

          for (int i (0); i < n_stack; ++i)
            try_replace_slot (store,
                              c,
                              i,
                              c->menu_stack [i],
                              /* stack_slot */ true,
                              matched);
        }

        append_unmatched_menus (store, matched);
      }

      std::optional<std::vector<std::string>>
      scan_relative_files (const std::filesystem::path& directory,
                           std::string_view extension)
      {
        namespace fs = std::filesystem;

        if (!fs::exists (directory) || !fs::is_directory (directory))
          return std::vector<std::string> ();

        std::vector<std::string> files;

        for (const fs::directory_entry& e : fs::directory_iterator (directory))
        {
          if (!e.is_regular_file ())
            continue;

          if (normalize_key (e.path ().extension ().string ()) != extension)
            continue;

          files.emplace_back (normalize_key (
            fs::relative (e.path (), fs::current_path ()).generic_string ()));
        }

        std::sort (files.begin (), files.end ());
        return files;
      }

      struct source_file
      {
        std::string path;
        bool allow_new_menus;
      };

      std::vector<source_file>
      enumerate_source_files (reload_pass pass)
      {
        std::vector<source_file> files;

        const bool cgame (pass == reload_pass::cgame_initialization);

        auto root (scan_relative_files ("ui_mp", ".menu"));
        if (root)
        {
          for (const std::string& f : *root)
            files.push_back ({f, cgame});
        }

        if (cgame)
        {
          auto script (scan_relative_files ("ui_mp/scriptmenus", ".menu"));
          if (script)
          {
            for (const std::string& f : *script)
              files.push_back ({f, true});
          }
        }

        auto lists (scan_relative_files ("ui_mp", ".txt"));
        if (lists)
        {
          for (const std::string& f : *lists)
            files.push_back ({f, true});
        }

        return files;
      }

      owned_menu_list
      make_owned_menu_list (const menu_list* template_asset,
                            const std::vector<menu_definition*>& accepted)
      {
        assert (template_asset != nullptr);

        owned_menu_list out;
        out.asset = std::make_unique<menu_list> ();
        out.menus = std::make_unique<menu_definition*[]> (accepted.size ());

        for (std::size_t i (0); i < accepted.size (); ++i)
          out.menus [i] = accepted [i];

        out.asset->name = template_asset->name;
        out.asset->menu_count = static_cast<int> (accepted.size ());
        out.asset->menus = out.menus.get ();

        return out;
      }

      std::shared_ptr<menu_override_store>
      build_generation (reload_pass pass)
      {
        auto store (std::make_shared<menu_override_store> ());

        const std::vector<source_file> files (enumerate_source_files (pass));

        for (const source_file& src : files)
        {
          auto parsed (menu_loader::load_menu_file (src.path));

          if (!parsed)
            continue;

          store->keepalive.emplace_back (parsed->keepalive);

          std::vector<menu_definition*> accepted;
          accepted.reserve (parsed->menus.size ());

          for (menu_definition* m : parsed->menus)
          {
            const menu_key key (menu_name_key (m));
            const bool exists (has_menu_in_live_contexts (key));

            if (!exists && !src.allow_new_menus)
              continue;

            accepted.push_back (m);
            store->menus_by_name [key] = m;
            store->appendable_by_name [key] = !exists && src.allow_new_menus;
          }

          const bool is_menu_list (
            std::filesystem::path (src.path).extension () == ".txt");

          if (accepted.empty () && !is_menu_list && !src.allow_new_menus)
            continue;

          menu_list* final_list (parsed->menu_list_asset);

          if (accepted.size () != parsed->menus.size ())
          {
            owned_menu_list owned (
              make_owned_menu_list (parsed->menu_list_asset, accepted));

            final_list = owned.asset.get ();
            store->owned_menu_lists.push_back (std::move (owned));
          }

          store->menu_lists_by_path [normalize_key (src.path)] = final_list;
        }

        return store;
      }

      class bypass_guard
      {
      public:
        bypass_guard () noexcept
        {
          assert (!db_find_xasset_header_bypass);
          db_find_xasset_header_bypass = true;
        }

        ~bypass_guard ()
        {
          db_find_xasset_header_bypass = false;
        }

        bypass_guard (const bypass_guard&) = delete;
        bypass_guard& operator = (const bypass_guard&) = delete;
      };

      xasset_header
      find_original_asset (xasset_type type, const char* name)
      {
        bypass_guard g;
        return DB_FindXAssetHeader (type, name);
      }

      xasset_header
      db_find_xasset_header (xasset_type type, const char* name)
      {
        if (db_find_xasset_header_bypass || name == nullptr)
          return DB_FindXAssetHeader (type, name);

        const std::shared_ptr<menu_override_store> store (active_store.load ());

        if (store != nullptr)
        {
          const menu_key key (normalize_key (name));

          if (type == XASSET_TYPE_MENU)
          {
            const auto it (store->menus_by_name.find (key));

            if (it != store->menus_by_name.end ())
            {
              xasset_header h {};
              h.menu = it->second;
              return h;
            }
          }
          else if (type == XASSET_TYPE_MENU_LIST)
          {
            const auto it (store->menu_lists_by_path.find (key));

            if (it != store->menu_lists_by_path.end ())
            {
              xasset_header h {};
              h.menu_list_pointer = it->second;
              return h;
            }
          }
        }

        return find_original_asset (type, name);
      }

      void
      reload_disk_menus (reload_pass pass)
      {
        const std::shared_ptr<menu_override_store> previous (
          active_store.load ());

        if (previous != nullptr)
        {
          restore_live_store (*previous);
          active_store.store (nullptr);
        }

        std::shared_ptr<menu_override_store> next (build_generation (pass));

        if (next == nullptr)
        {
          if (previous != nullptr)
          {
            apply_live_store (*previous);
            active_store.store (previous);
          }

          return;
        }

        apply_live_store (*next);
        active_store.store (std::move (next));
      }

      void
      poll_startup_reload_pipeline ()
      {
        switch (startup_reload_stage)
        {
          case reload_stage::wait_for_ui_initialization:
          {
            if (bounded_menu_count (UI_GetFrontendContext ()) > 0)
            {
              reload_disk_menus (reload_pass::ui_initialization);
              startup_reload_stage =
                reload_stage::wait_for_cgame_initialization;
            }
            break;
          }
          case reload_stage::wait_for_cgame_initialization:
          {
            if (bounded_menu_count (UI_GetIngameContext ()) > 0)
            {
              reload_disk_menus (reload_pass::cgame_initialization);
              startup_reload_stage = reload_stage::complete;
            }
            break;
          }
          case reload_stage::complete:
            break;
        }
      }
    }

    menu_module::
    menu_module ()
    {
      detour (DB_FindXAssetHeader, &db_find_xasset_header);

      scheduler::post (com_frame_domain,
                       [] ()
      {
        poll_startup_reload_pipeline ();
      },

      repeat_until_predicate ([] ()
      {
        return startup_reload_stage == reload_stage::complete;
      }));
    }
  }
}
