#include <libiw4x/menu/init.hxx>

#include <string>
#include <vector>

using namespace std;

namespace iw4x
{
  namespace menu
  {
    namespace
    {
      menuDef_t&
      find_menu (const string& name)
      {
        XAssetHeader header (DB_FindXAssetHeader (ASSET_TYPE_MENU,
                                                  name.c_str()));

        // FIXME: DB_FindXAssetHeader will deadlock if the
        //        menu can't be found.
        //
        return *header.menu;
      }

      itemDef_s&
      find_item (menuDef_t& m, const string& name)
      {
        for (int i (0); i < m.itemCount; ++i)
        {
          itemDef_s* d (m.items [i]);

          // The engine does not guarantee invariants. Validate pointers before
          // comparing names.
          //
          if ((d != nullptr) && (d->text != nullptr) && name == d->text)
            return *d;
        }

        // At this point, either the menu has no items, or the requested item
        // was not found. It's unclear whether a menu is allowed to exist
        // without items (and if so, why find_item would be called), but let's
        // handle both cases explicitly.
        //
        if (m.itemCount == 0)
          throw runtime_error ("item count is empty");

        throw runtime_error ("unable to find item: " + name);
      }

      MenuEventHandler*
      make_handler (const string& cmd)
      {
        MenuEventHandler* h (new MenuEventHandler ());

        h->eventType = 0;
        h->eventData.unconditionalScript = strdup (cmd.c_str ());

        return h;
      }

      MenuEventHandlerSet*
      make_handler_set (const vector<string>& cmds)
      {
        MenuEventHandlerSet* hs (new MenuEventHandlerSet ());

        hs->eventHandlerCount = static_cast<int>  (cmds.size ());
        hs->eventHandlers = new MenuEventHandler* [cmds.size ()];

        for (size_t i (0); i != cmds.size (); ++i)
          hs->eventHandlers[i] = make_handler (cmds[i]);

        return hs;
      }
    }

    void
    init (scheduler& s)
    {
      // Must runs only after the menu graph is actually there to work with.
      //
      s.post ("com_frame", [] ()
      {
        ([] (auto&& action, auto&& expression)
          {
            action
            (
              "main_text",
              "@PLATFORM_PLAY_ONLINE_CAPS",
              {
                "play mouse_click",
                "open menu_xboxlive"
              }
            );

            action
            (
              "menu_xboxlive",
              "@MENU_PRIVATE_MATCH_CAPS",
              {
                "play mouse_click",
                "exec xcheckezpatch",
                "exec default_xboxlive.cfg",
                "exec xblive_rankedmatch 0",
                "exec ui_enumeratesaved exec xblive_privatematch 1",
                "exec onlinegame 0",
                "exec xblive_hostingprivateparty 1",
                "exec xblive_privatepartyclient 1",
                "exec xstartprivatematch",
                "open menu_xboxlive_privatelobby"
              }
            );

            expression ("menu_xboxlive_privatelobby", "@MENU_START_GAME_CAPS");
            expression ("menu_xboxlive_privatelobby", "@MENU_GAME_SETUP_CAPS");
          })

        // Action.
        //
        ([] (const string& menu_name, const string& item_name, const vector<string>& cmds)
          {
            menuDef_t& m (find_menu (menu_name));
            itemDef_s& i (find_item (m, item_name));

            i.action = make_handler_set (cmds);
          },

        // Expression.
        //
        ([] (const string& menu_name, const string& item_name)
          {
            menuDef_t& m (find_menu (menu_name));
            itemDef_s& i (find_item (m, item_name));

            i.disabledExp = nullptr;
          }));
      });
    }
  }
}
