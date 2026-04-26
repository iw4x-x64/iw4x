#include <libiw4x/shim/open-asset-tools/open-asset-tools.hxx>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <libiw4x/logger.hxx>

#ifdef ERROR
#  undef ERROR
#endif

#include <Pool/GlobalAssetPool.h>

#include <Game/IGame.h>
#include <Game/IW4/GameIW4.h>

#include <Utils/Logging/Log.h>

#include <Asset/AssetCreationContext.h>
#include <Asset/AssetCreationResult.h>
#include <Asset/AssetCreatorCollection.h>
#include <Asset/IAssetCreator.h>

#include <Game/IW4/IW4.h>
#include <Game/IW4/Menu/LoaderMenuListIW4.h>

#include <SearchPath/SearchPathFilesystem.h>

#include <Zone/Zone.h>

void
GlobalAssetPool::LinkAssetPool (AssetPool* assetPool,
                                const zone_priority_t priority)
{
  auto link (std::make_unique<LinkedAssetPool> ());

  link->m_asset_pool = assetPool;
  link->m_priority = priority;

  auto* const link_ptr (link.get ());

  m_linked_asset_pools.emplace_back (std::move (link));
  SortLinkedAssetPools ();

  for (auto* asset : *assetPool)
  {
    const auto normalized_name (
      XAssetInfoGeneric::NormalizeAssetName (asset->m_name));

    LinkAsset (link_ptr, normalized_name, asset);
  }
}

void
GlobalAssetPool::LinkAsset (const AssetPool* assetPool,
                            const std::string& normalizedAssetName,
                            XAssetInfoGeneric* asset)
{
  LinkedAssetPool* link (nullptr);

  for (const auto& entry : m_linked_asset_pools)
  {
    if (entry->m_asset_pool == assetPool)
    {
      link = entry.get ();
      break;
    }
  }

  assert (link != nullptr);

  if (link == nullptr)
    return;

  LinkAsset (link, normalizedAssetName, asset);
}

void
GlobalAssetPool::UnlinkAssetPool (const AssetPool* assetPool)
{
  auto it (m_linked_asset_pools.begin ());

  for (; it != m_linked_asset_pools.end (); ++it)
  {
    if ((*it)->m_asset_pool == assetPool)
      break;
  }

  assert (it != m_linked_asset_pools.end ());

  if (it == m_linked_asset_pools.end ())
    return;

  const auto pool_to_unlink (std::move (*it));

  m_linked_asset_pools.erase (it);

  for (auto entry_it (m_assets.begin ()); entry_it != m_assets.end ();)
  {
    auto& entry (entry_it->second);

    if (entry.m_asset_pool != pool_to_unlink.get ())
    {
      ++entry_it;
      continue;
    }

    if (entry.m_duplicate && ReplaceAssetPoolEntry (entry))
    {
      ++entry_it;
      continue;
    }

    entry_it = m_assets.erase (entry_it);
  }
}

XAssetInfoGeneric*
GlobalAssetPool::GetAsset (const std::string& name)
{
  const auto normalized_name (XAssetInfoGeneric::NormalizeAssetName (name));
  const auto found (m_assets.find (normalized_name));

  if (found == m_assets.end ())
    return nullptr;

  return found->second.m_asset;
}

void
GlobalAssetPool::SortLinkedAssetPools ()
{
  std::ranges::sort (m_linked_asset_pools,
                     [] (const std::unique_ptr<LinkedAssetPool>& a,
                         const std::unique_ptr<LinkedAssetPool>& b) -> bool
  {
    return a->m_priority < b->m_priority;
  });
}

bool
GlobalAssetPool::ReplaceAssetPoolEntry (GameAssetPoolEntry& assetEntry) const
{
  bool found_first (false);

  for (const auto& linked_pool : m_linked_asset_pools)
  {
    auto* const found_asset (
      linked_pool->m_asset_pool->GetAsset (assetEntry.m_asset->m_name));

    if (found_asset == nullptr)
      continue;

    if (!found_first)
    {
      assetEntry.m_asset = found_asset;
      assetEntry.m_duplicate = false;
      assetEntry.m_asset_pool = linked_pool.get ();
      found_first = true;
    }
    else
    {
      assetEntry.m_duplicate = true;
      break;
    }
  }

  return found_first;
}

void
GlobalAssetPool::LinkAsset (LinkedAssetPool* link,
                            const std::string& normalizedAssetName,
                            XAssetInfoGeneric* asset)
{
  const auto existing (m_assets.find (normalizedAssetName));

  if (existing == m_assets.end ())
  {
    const GameAssetPoolEntry entry {
      .m_asset = asset,
      .m_asset_pool = link,
      .m_duplicate = false,
    };

    m_assets [normalizedAssetName] = entry;
  }
  else
  {
    auto& existing_entry (existing->second);

    existing_entry.m_duplicate = true;

    if (existing_entry.m_asset_pool->m_priority < link->m_priority)
    {
      existing_entry.m_asset_pool = link;
      existing_entry.m_asset = asset;
    }
  }
}

GameGlobalAssetPools::GameGlobalAssetPools (const GameId gameId)
{
  const auto asset_type_count (
    IGame::GetGameById (gameId)->GetAssetTypeCount ());

  m_global_asset_pools.resize (asset_type_count);

  for (auto i (0u); i < asset_type_count; ++i)
    m_global_asset_pools [i] = std::make_unique<GlobalAssetPool> ();
}

void
GameGlobalAssetPools::LinkAssetPool (const asset_type_t assetType,
                                     AssetPool* assetPool,
                                     const zone_priority_t priority) const
{
  assert (assetType < m_global_asset_pools.size ());
  m_global_asset_pools [assetType]->LinkAssetPool (assetPool, priority);
}

void
GameGlobalAssetPools::LinkAsset (const asset_type_t assetType,
                                 const AssetPool* assetPool,
                                 const std::string& normalizedAssetName,
                                 XAssetInfoGeneric* asset) const
{
  assert (assetType < m_global_asset_pools.size ());
  m_global_asset_pools [assetType]->LinkAsset (assetPool,
                                               normalizedAssetName,
                                               asset);
}

void
GameGlobalAssetPools::UnlinkAssetPool (const asset_type_t assetType,
                                       const AssetPool* assetPool) const
{
  assert (assetType < m_global_asset_pools.size ());
  m_global_asset_pools [assetType]->UnlinkAssetPool (assetPool);
}

XAssetInfoGeneric*
GameGlobalAssetPools::GetAsset (const asset_type_t assetType,
                                const std::string& name) const
{
  assert (assetType < m_global_asset_pools.size ());
  return m_global_asset_pools [assetType]->GetAsset (name);
}

GameGlobalAssetPools*
GameGlobalAssetPools::GetGlobalPoolsForGame (GameId gameId)
{
  // Hardcoded to IW4: this shim only supports a single game.
  //
  static GameGlobalAssetPools pools (GameId::IW4);

  assert (gameId == GameId::IW4);
  (void) gameId;

  return &pools;
}

IGame*
IGame::GetGameById (GameId game_id)
{
  static IW4::Game game;

  assert (game_id == GameId::IW4);
  (void) game_id;

  return &game;
}

namespace con
{
  LogLevel _globalLogLevel = LogLevel::INFO;
  std::atomic_size_t _warningCount (0);
  std::atomic_size_t _errorCount (0);

  void
  init ()
  {
  }

  void
  set_log_level (LogLevel value)
  {
    _globalLogLevel = value;
  }

  void
  set_use_color (bool)
  {
  }

  void
  reset_counts ()
  {
    _warningCount.store (0, std::memory_order_relaxed);
    _errorCount.store (0, std::memory_order_relaxed);
  }

  size_t
  warning_count ()
  {
    return _warningCount.load (std::memory_order_relaxed);
  }

  size_t
  error_count ()
  {
    return _errorCount.load (std::memory_order_relaxed);
  }

  void
  _debug_internal (const std::string& str)
  {
    iw4x::log::debug << "[oat] " << str;
  }

  void
  _info_internal (const std::string& str)
  {
    iw4x::log::info << "[oat] " << str;
  }

  void
  _warn_internal (const std::string& str)
  {
    iw4x::log::warning << "[oat] " << str;
  }

  void
  _error_internal (const std::string& str)
  {
    iw4x::log::error << "[oat] " << str;
  }
}

static_assert (sizeof (iw4x::menu_definition) == sizeof (IW4::menuDef_t));
static_assert (sizeof (iw4x::menu_list) == sizeof (IW4::MenuList));
static_assert (sizeof (iw4x::item_definition) == sizeof (IW4::itemDef_s));
static_assert (sizeof (iw4x::statement) == sizeof (IW4::Statement_s));
static_assert (sizeof (iw4x::expression_supporting_data) ==
               sizeof (IW4::ExpressionSupportingData));
static_assert (sizeof (iw4x::menu_event_handler_set) ==
               sizeof (IW4::MenuEventHandlerSet));

namespace iw4x
{
  namespace menu_loader
  {
    namespace
    {
      template <typename OatAssetType, xasset_type engine_type, auto member>
      class engine_asset_creator final : public AssetCreator<OatAssetType>
      {
      public:
        AssetCreationResult
        CreateAsset (const std::string& asset_name,
                     AssetCreationContext& context) override
        {
          const xasset_header header (
            DB_FindXAssetHeader (engine_type, asset_name.c_str ()));

          auto* const engine_ptr (header.*member);

          if (engine_ptr == nullptr)
            return AssetCreationResult::NoAction ();

          using oat_ptr_type = typename OatAssetType::Type*;

          return AssetCreationResult::Success (context.AddAsset<OatAssetType> (
            asset_name,
            reinterpret_cast<oat_ptr_type> (engine_ptr)));
        }
      };

      using material_creator =
        engine_asset_creator<IW4::AssetMaterial,
                             XASSET_TYPE_MATERIAL,
                             &xasset_header::material_pointer>;

      using sound_creator = engine_asset_creator<IW4::AssetSound,
                                                 XASSET_TYPE_SOUND,
                                                 &xasset_header::sound>;

      // Repair parent back-pointers in menu item definitions.
      //
      // OAT's parser does not set the item->parent field because it is a
      // runtime-only link. The engine expects it to be valid, so we patch
      // it after parsing.
      //
      void
      repair_menu_runtime_links (menu_definition* menu)
      {
        if (menu == nullptr)
          return;

        assert (menu->item_count >= 0);

        if (menu->item_count <= 0 || menu->items == nullptr)
          return;

        for (int i (0); i < menu->item_count; ++i)
        {
          auto* const item (menu->items [i]);

          if (item != nullptr)
            item->parent = menu;
        }
      }

      struct zone_context
      {
        std::shared_ptr<Zone> zone;
        SearchPathFilesystem search_path;

        zone_context (const std::string& name)
          : zone (
              std::make_shared<Zone> (name, 0, GameId::IW4, GamePlatform::PC)),
            search_path (std::filesystem::current_path ().string ())
        {
        }
      };

      std::optional<AssetCreationResult>
      parse_menu_list (zone_context& ctx, const std::string& source_path)
      {
        AssetCreatorCollection creators (*ctx.zone);
        IgnoredAssetLookup ignored_assets;

        creators.AddAssetCreator (std::make_unique<material_creator> ());
        creators.AddAssetCreator (std::make_unique<sound_creator> ());
        creators.AddAssetCreator (
          menu::CreateMenuListLoaderIW4 (ctx.zone->Memory (), ctx.search_path));

        AssetCreationContext creation_ctx (*ctx.zone,
                                           &creators,
                                           &ignored_assets);

        AssetCreationResult result (
          creators.CreateAsset (IW4::AssetMenuList::EnumEntry,
                                source_path,
                                creation_ctx));

        if (!result.HasTakenAction ())
        {
          log::warning << "menu loader: no action for \"" << source_path
                       << "\" "
                       << "(file not found or not applicable)";
          return std::nullopt;
        }

        if (result.HasFailed ())
        {
          log::warning << "menu loader: parse error for \"" << source_path
                       << "\"";
          return std::nullopt;
        }

        creators.FinalizeZone (creation_ctx);
        return result;
      }

      std::optional<parsed_menu_file>
      extract_menus (const std::string& source_path,
                     AssetCreationResult& result,
                     std::shared_ptr<Zone> zone)
      {
        auto* const list_info (
          static_cast<XAssetInfo<IW4::MenuList>*> (result.GetAssetInfo ()));

        if (list_info == nullptr || list_info->Asset () == nullptr)
        {
          log::warning << "menu loader: no menu list produced for \""
                       << source_path << "\"";
          return std::nullopt;
        }

        auto* const raw_list (
          reinterpret_cast<menu_list*> (list_info->Asset ()));

        std::vector<menu_definition*> menus;

        if (raw_list->menu_count > 0 && raw_list->menus != nullptr)
        {
          menus.reserve (raw_list->menu_count);

          for (int i (0); i < raw_list->menu_count; ++i)
          {
            auto* const menu (raw_list->menus [i]);

            if (menu == nullptr)
              continue;

            repair_menu_runtime_links (menu);
            menus.emplace_back (menu);
          }
        }

        return parsed_menu_file {
          .source_path = source_path,
          .menu_list_asset = raw_list,
          .menus = std::move (menus),
          .keepalive = std::move (zone),
        };
      }
    }

    std::optional<parsed_menu_file>
    load_menu_file (const std::string& source_path)
    {
      zone_context ctx (source_path);

      auto result (parse_menu_list (ctx, source_path));

      if (!result)
        return std::nullopt;

      return extract_menus (source_path, *result, std::move (ctx.zone));
    }
  }
}
