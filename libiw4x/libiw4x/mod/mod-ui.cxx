#include <libiw4x/mod/mod-ui.hxx>

#include <algorithm>
#include <ranges>

#include <libiw4x/detour.hxx>
#include <libiw4x/scheduler.hxx>

using namespace std;

namespace iw4x
{
  namespace mod
  {
    namespace
    {
      menu_definition&
      find_menu (const char* name)
      {
        xasset_header h (DB_FindXAssetHeader (XASSET_TYPE_MENU, name));

        // FIXME: DB_FindXAssetHeader will deadlock if the
        //        asset can't be found.
        //
        return *h.menu;
      }

      item_definition&
      find_item (menu_definition& m, const char* name)
      {
        for (int i (0); i < m.item_count; ++i)
        {
          item_definition* d (m.items [i]);

          // The engine's UI structures provide zero pointer safety guarantees.
          // We have to validate the item and its text pointer before doing our
          // string comparison to avoid crashing on malformed assets.
          //
          if (d != nullptr && d->text != nullptr && strcmp (name, d->text) == 0)
            return *d;
        }

        // If we drop through the loop, the item isn't there. This usually means
        // the menu structure is out of sync with our expectations. It's also
        // possible the menu legitimately has no items, though why we'd search
        // an empty menu is another question.
        //
        exit (1);
      }

      menu_event_handler*
      make_event_handler (const char* script)
      {
        menu_event_handler* h (new menu_event_handler ());

        h->event_type = 0;
        h->event_data_union.unconditional_script = _strdup (script);

        return h;
      }

      menu_event_handler_set*
      make_event_handler_set (initializer_list<const char*> scripts)
      {
        menu_event_handler_set* hs (new menu_event_handler_set ());

        hs->event_handler_count = static_cast<int> (scripts.size ());
        hs->event_handlers = new menu_event_handler* [scripts.size ()];

        ranges::transform (scripts, hs->event_handlers, make_event_handler);

        return hs;
      }

      void
      set_action (const char* menu,
                  const char* item,
                  initializer_list<const char*> scripts)
      {
        menu_definition& m (find_menu (menu));
        item_definition& i (find_item (m, item));

        i.action = make_event_handler_set (scripts);
      }
    }

    ui_module::
    ui_module ()
    {
      scheduler::post (com_frame_domain {}, []
      {
        set_action
        (
          "main_text",
          "@PLATFORM_PLAY_ONLINE_CAPS",
          {
            "play mouse_click",
            "open menu_xboxlive"
          }
        );
      });
    }
  }
}
