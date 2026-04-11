#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <winsock2.h>
#    include <d3d9.h>
#    undef NOMINMAX
#  else
#    include <winsock2.h>
#    include <d3d9.h>
#  endif
#  undef WIN32_LEAN_AND_MEAN
#else
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <winsock2.h>
#    include <d3d9.h>
#    undef NOMINMAX
#  else
#    include <winsock2.h>
#    include <d3d9.h>
#  endif
#endif

#include <cstdint>

namespace iw4x
{
  struct expression_entry;
  struct expression_supporting_data;
  struct effects_element_definition;
  struct graphics_portal;
  struct item_definition;
  struct material_technique;
  struct menu_event_handler;
  struct path_node;
  struct path_node_tree;
  struct statement;

  // 29
  //
  enum network_source
  {
    NETWORK_SOURCE_CLIENT_1 = 0x0,
    NETWORK_SOURCE_SERVER = 0x1,
    NETWORK_SOURCE_MAXIMUM_CLIENTS = 0x1,
    NETWORK_SOURCE_PACKET = 0x2,
  };

  // 38
  //
  enum xasset_type
  {
    XASSET_TYPE_PHYSICS_PRESET = 0x0,
    XASSET_TYPE_PHYSICS_COLLISION_MAP = 0x1,
    XASSET_TYPE_XANIMATION_PARTS = 0x2,
    XASSET_TYPE_XMODEL_SURFACES = 0x3,
    XASSET_TYPE_XMODEL = 0x4,
    XASSET_TYPE_MATERIAL = 0x5,
    XASSET_TYPE_PIXEL_SHADER = 0x6,
    XASSET_TYPE_VERTEX_SHADER = 0x7,
    XASSET_TYPE_VERTEX_DECLARATION = 0x8,
    XASSET_TYPE_TECHNIQUE_SET = 0x9,
    XASSET_TYPE_IMAGE = 0xA,
    XASSET_TYPE_SOUND = 0xB,
    XASSET_TYPE_SOUND_CURVE = 0xC,
    XASSET_TYPE_LOADED_SOUND = 0xD,
    XASSET_TYPE_CLIP_MAP_SINGLE_PLAYER = 0xE,
    XASSET_TYPE_CLIP_MAP_MULTI_PLAYER = 0xF,
    XASSET_TYPE_COMMON_WORLD = 0x10,
    XASSET_TYPE_GAME_WORLD_SINGLE_PLAYER = 0x11,
    XASSET_TYPE_GAME_WORLD_MULTI_PLAYER = 0x12,
    XASSET_TYPE_MAP_ENTITIES = 0x13,
    XASSET_TYPE_EFFECTS_WORLD = 0x14,
    XASSET_TYPE_GRAPHICS_WORLD = 0x15,
    XASSET_TYPE_LIGHT_DEFINITION = 0x16,
    XASSET_TYPE_USER_INTERFACE_MAP = 0x17,
    XASSET_TYPE_FONT = 0x18,
    XASSET_TYPE_MENU_LIST = 0x19,
    XASSET_TYPE_MENU = 0x1A,
    XASSET_TYPE_LOCALIZE_ENTRY = 0x1B,
    XASSET_TYPE_WEAPON = 0x1C,
    XASSET_TYPE_SOUND_DRIVER_GLOBALS = 0x1D,
    XASSET_TYPE_EFFECTS = 0x1E,
    XASSET_TYPE_IMPACT_EFFECTS = 0x1F,
    XASSET_TYPE_ARTIFICIAL_INTELLIGENCE_TYPE = 0x20,
    XASSET_TYPE_MULTI_PLAYER_TYPE = 0x21,
    XASSET_TYPE_CHARACTER = 0x22,
    XASSET_TYPE_XMODEL_ALIAS = 0x23,
    XASSET_TYPE_RAW_FILE = 0x24,
    XASSET_TYPE_STRING_TABLE = 0x25,
    XASSET_TYPE_LEADERBOARD = 0x26,
    XASSET_TYPE_STRUCTURED_DATA_DEFINITION = 0x27,
    XASSET_TYPE_TRACER = 0x28,
    XASSET_TYPE_VEHICLE = 0x29,
    XASSET_TYPE_ADDON_MAP_ENTITIES = 0x2A,
    XASSET_TYPE_COUNT = 0x2B,
    XASSET_TYPE_STRING = 0x2B,
    XASSET_TYPE_ASSET_LIST = 0x2C,
  };

  // 68
  //
  enum client_migration_state
  {
    CLIENT_MIGRATION_STATE_INACTIVE = 0x0,
    CLIENT_MIGRATION_STATE_OLD_HOST_LEAVING = 0x1,
    CLIENT_MIGRATION_STATE_LIMBO = 0x2,
    CLIENT_MIGRATION_STATE_NEW_HOST_CONNECT = 0x3,
    CLIENT_MIGRATION_STATE_COUNT = 0x4,
  };

  // 77
  //
  enum weapon_fire_type
  {
    WEAPON_FIRE_TYPE_FULL_AUTO = 0x0,
    WEAPON_FIRE_TYPE_SINGLE_SHOT = 0x1,
    WEAPON_FIRE_TYPE_BURST_FIRE_2 = 0x2,
    WEAPON_FIRE_TYPE_BURST_FIRE_3 = 0x3,
    WEAPON_FIRE_TYPE_BURST_FIRE_4 = 0x4,
    WEAPON_FIRE_TYPE_DOUBLE_BARREL = 0x5,
    WEAPON_FIRE_TYPE_COUNT = 0x6,
    WEAPON_FIRE_TYPE_BURST_FIRE_FIRST = 0x2,
    WEAPON_FIRE_TYPE_BURST_FIRE_LAST = 0x4,
  };

  // 79
  //
  enum offhand_class
  {
    OFFHAND_CLASS_NONE = 0x0,
    OFFHAND_CLASS_FRAGMENTATION_GRENADE = 0x1,
    OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
    OFFHAND_CLASS_FLASH_GRENADE = 0x3,
    OFFHAND_CLASS_THROWING_KNIFE = 0x4,
    OFFHAND_CLASS_OTHER = 0x5,
    OFFHAND_CLASS_COUNT = 0x6,
  };

  // 82
  //
  enum connection_state
  {
    CONNECTION_STATE_DISCONNECTED = 0x0,
    CONNECTION_STATE_CINEMATIC = 0x1,
    CONNECTION_STATE_LOGO = 0x2,
    CONNECTION_STATE_CONNECTING = 0x3,
    CONNECTION_STATE_CHALLENGING = 0x4,
    CONNECTION_STATE_CONNECTED = 0x5,
    CONNECTION_STATE_SENDING_STATISTICS = 0x6,
    CONNECTION_STATE_LOADING = 0x7,
    CONNECTION_STATE_PRIMED = 0x8,
    CONNECTION_STATE_ACTIVE = 0x9,
  };

  // 83
  //
  enum weapon_stickiness_type
  {
    WEAPON_STICKINESS_NONE = 0x0,
    WEAPON_STICKINESS_ALL = 0x1,
    WEAPON_STICKINESS_ALL_ORIENTED = 0x2,
    WEAPON_STICKINESS_GROUND = 0x3,
    WEAPON_STICKINESS_GROUND_WITH_YAW = 0x4,
    WEAPON_STICKINESS_KNIFE = 0x5,
    WEAPON_STICKINESS_COUNT = 0x6,
  };

  // 93
  //
  enum material_technique_type
  {
    TECHNIQUE_DEPTH_PREPASS = 0x0,
    TECHNIQUE_BUILD_FLOAT_Z = 0x1,
    TECHNIQUE_BUILD_SHADOW_MAP_DEPTH = 0x2,
    TECHNIQUE_BUILD_SHADOW_MAP_COLOR = 0x3,
    TECHNIQUE_UNLIT = 0x4,
    TECHNIQUE_EMISSIVE = 0x5,
    TECHNIQUE_EMISSIVE_DEPTH_FOG = 0x6,
    TECHNIQUE_EMISSIVE_SHADOW = 0x7,
    TECHNIQUE_EMISSIVE_SHADOW_DEPTH_FOG = 0x8,
    TECHNIQUE_LIT_BEGIN = 0x9,
    TECHNIQUE_LIT = 0x9,
    TECHNIQUE_LIT_DEPTH_FOG = 0xA,
    TECHNIQUE_LIT_SUN = 0xB,
    TECHNIQUE_LIT_SUN_DEPTH_FOG = 0xC,
    TECHNIQUE_LIT_SUN_SHADOW = 0xD,
    TECHNIQUE_LIT_SUN_SHADOW_DEPTH_FOG = 0xE,
    TECHNIQUE_LIT_SPOT = 0xF,
    TECHNIQUE_LIT_SPOT_DEPTH_FOG = 0x10,
    TECHNIQUE_LIT_SPOT_SHADOW = 0x11,
    TECHNIQUE_LIT_SPOT_SHADOW_DEPTH_FOG = 0x12,
    TECHNIQUE_LIT_OMNI = 0x13,
    TECHNIQUE_LIT_OMNI_DEPTH_FOG = 0x14,
    TECHNIQUE_LIT_OMNI_SHADOW = 0x15,
    TECHNIQUE_LIT_OMNI_SHADOW_DEPTH_FOG = 0x16,
    TECHNIQUE_LIT_INSTANCED = 0x17,
    TECHNIQUE_LIT_INSTANCED_DEPTH_FOG = 0x18,
    TECHNIQUE_LIT_INSTANCED_SUN = 0x19,
    TECHNIQUE_LIT_INSTANCED_SUN_DEPTH_FOG = 0x1A,
    TECHNIQUE_LIT_INSTANCED_SUN_SHADOW = 0x1B,
    TECHNIQUE_LIT_INSTANCED_SUN_SHADOW_DEPTH_FOG = 0x1C,
    TECHNIQUE_LIT_INSTANCED_SPOT = 0x1D,
    TECHNIQUE_LIT_INSTANCED_SPOT_DEPTH_FOG = 0x1E,
    TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW = 0x1F,
    TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW_DEPTH_FOG = 0x20,
    TECHNIQUE_LIT_INSTANCED_OMNI = 0x21,
    TECHNIQUE_LIT_INSTANCED_OMNI_DEPTH_FOG = 0x22,
    TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW = 0x23,
    TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW_DEPTH_FOG = 0x24,
    TECHNIQUE_LIT_END = 0x25,
    TECHNIQUE_LIGHT_SPOT = 0x25,
    TECHNIQUE_LIGHT_OMNI = 0x26,
    TECHNIQUE_LIGHT_SPOT_SHADOW = 0x27,
    TECHNIQUE_FAKE_LIGHT_NORMAL = 0x28,
    TECHNIQUE_FAKE_LIGHT_VIEW = 0x29,
    TECHNIQUE_SUN_LIGHT_PREVIEW = 0x2A,
    TECHNIQUE_CASE_TEXTURE = 0x2B,
    TECHNIQUE_WIREFRAME_SOLID = 0x2C,
    TECHNIQUE_WIREFRAME_SHADED = 0x2D,
    TECHNIQUE_DEBUG_BUMP_MAP = 0x2E,
    TECHNIQUE_DEBUG_BUMP_MAP_INSTANCED = 0x2F,
    TECHNIQUE_COUNT = 0x30,
    TECHNIQUE_TOTAL_COUNT = 0x31,
    TECHNIQUE_NONE = 0x32,
  };

  // 97
  //
  enum migration_verbose_state
  {
    MIGRATION_VERBOSE_STATE_INACTIVE = 0x0,
    MIGRATION_VERBOSE_STATE_WAITING = 0x1,
    MIGRATION_VERBOSE_STATE_RATING = 0x2,
    MIGRATION_VERBOSE_STATE_SENDING = 0x3,
    MIGRATION_VERBOSE_STATE_MIGRATING = 0x4,
    MIGRATION_VERBOSE_STATE_COUNT = 0x5,
  };

  // 98
  //
  enum stance_state
  {
    CLIENT_STANCE_STAND = 0x0,
    CLIENT_STANCE_CROUCH = 0x1,
    CLIENT_STANCE_PRONE = 0x2,
  };

  // 99
  //
  enum impact_type
  {
    IMPACT_TYPE_NONE = 0x0,
    IMPACT_TYPE_BULLET_SMALL = 0x1,
    IMPACT_TYPE_BULLET_LARGE = 0x2,
    IMPACT_TYPE_BULLET_ARMOR_PIERCING = 0x3,
    IMPACT_TYPE_BULLET_EXPLODE = 0x4,
    IMPACT_TYPE_SHOTGUN = 0x5,
    IMPACT_TYPE_SHOTGUN_EXPLODE = 0x6,
    IMPACT_TYPE_GRENADE_BOUNCE = 0x7,
    IMPACT_TYPE_GRENADE_EXPLODE = 0x8,
    IMPACT_TYPE_ROCKET_EXPLODE = 0x9,
    IMPACT_TYPE_PROJECTILE_DUD = 0xA,
    IMPACT_TYPE_COUNT = 0xB,
  };

  // 100
  //
  enum network_address_type
  {
    NETWORK_ADDRESS_BOT = 0x0,
    NETWORK_ADDRESS_BAD = 0x1,
    NETWORK_ADDRESS_LOOPBACK = 0x2,
    NETWORK_ADDRESS_BROADCAST = 0x3,
    NETWORK_ADDRESS_IP = 0x4,
    NETWORK_ADDRESS_IPX = 0x5,
    NETWORK_ADDRESS_BROADCAST_IPX = 0x6,
  };

  // 102
  //
  enum weapon_type
  {
    WEAPON_TYPE_BULLET = 0x0,
    WEAPON_TYPE_GRENADE = 0x1,
    WEAPON_TYPE_PROJECTILE = 0x2,
    WEAPON_TYPE_RIOT_SHIELD = 0x3,
    WEAPON_TYPE_NUMBER = 0x4,
  };

  // 103
  //
  enum weapon_overlay_interface
  {
    WEAPON_OVERLAY_INTERFACE_NONE = 0x0,
    WEAPON_OVERLAY_INTERFACE_JAVELIN = 0x1,
    WEAPON_OVERLAY_INTERFACE_TURRET_SCOPE = 0x2,
    WEAPON_OVERLAY_INTERFACE_COUNT = 0x3,
  };

  // 105
  //
  enum weapon_class
  {
    WEAPON_CLASS_RIFLE = 0x0,
    WEAPON_CLASS_SNIPER = 0x1,
    WEAPON_CLASS_MACHINE_GUN = 0x2,
    WEAPON_CLASS_SUB_MACHINE_GUN = 0x3,
    WEAPON_CLASS_SPREAD = 0x4,
    WEAPON_CLASS_PISTOL = 0x5,
    WEAPON_CLASS_GRENADE = 0x6,
    WEAPON_CLASS_ROCKET_LAUNCHER = 0x7,
    WEAPON_CLASS_TURRET = 0x8,
    WEAPON_CLASS_THROWING_KNIFE = 0x9,
    WEAPON_CLASS_NON_PLAYER = 0xA,
    WEAPON_CLASS_ITEM = 0xB,
    WEAPON_CLASS_NUMBER = 0xC,
  };

  // 106
  //
  enum weapon_overlay_reticle
  {
    WEAPON_OVERLAY_RETICLE_NONE = 0x0,
    WEAPON_OVERLAY_RETICLE_CROSSHAIR = 0x1,
    WEAPON_OVERLAY_RETICLE_NUMBER = 0x2,
  };

  // 108
  //
  enum weapon_stance
  {
    WEAPON_STANCE_STAND = 0x0,
    WEAPON_STANCE_DUCK = 0x1,
    WEAPON_STANCE_PRONE = 0x2,
    WEAPON_STANCE_NUMBER = 0x3,
  };

  // 109
  //
  enum weapon_icon_ratio_type
  {
    WEAPON_ICON_RATIO_1_TO_1 = 0x0,
    WEAPON_ICON_RATIO_2_TO_1 = 0x1,
    WEAPON_ICON_RATIO_4_TO_1 = 0x2,
    WEAPON_ICON_RATIO_COUNT = 0x3,
  };

  // 111
  //
  enum penetration_type
  {
    PENETRATION_TYPE_NONE = 0x0,
    PENETRATION_TYPE_SMALL = 0x1,
    PENETRATION_TYPE_MEDIUM = 0x2,
    PENETRATION_TYPE_LARGE = 0x3,
    PENETRATION_TYPE_COUNT = 0x4,
  };

  // 112
  //
  enum weapon_inventory_type
  {
    WEAPON_INVENTORY_PRIMARY = 0x0,
    WEAPON_INVENTORY_OFFHAND = 0x1,
    WEAPON_INVENTORY_ITEM = 0x2,
    WEAPON_INVENTORY_ALTERNATE_MODE = 0x3,
    WEAPON_INVENTORY_EXCLUSIVE = 0x4,
    WEAPON_INVENTORY_SCAVENGER = 0x5,
    WEAPON_INVENTORY_COUNT = 0x6,
  };

  // 113
  //
  enum active_reticle_type
  {
    VEHICLE_ACTIVE_RETICLE_NONE = 0x0,
    VEHICLE_ACTIVE_RETICLE_PICTURE_IN_PICTURE_ON_A_STICK = 0x1,
    VEHICLE_ACTIVE_RETICLE_BOUNCING_DIAMOND = 0x2,
    VEHICLE_ACTIVE_RETICLE_COUNT = 0x3,
  };

  // 114
  //
  enum ammunition_counter_clip_type
  {
    AMMUNITION_COUNTER_CLIP_NONE = 0x0,
    AMMUNITION_COUNTER_CLIP_MAGAZINE = 0x1,
    AMMUNITION_COUNTER_CLIP_SHORT_MAGAZINE = 0x2,
    AMMUNITION_COUNTER_CLIP_SHOTGUN = 0x3,
    AMMUNITION_COUNTER_CLIP_ROCKET = 0x4,
    AMMUNITION_COUNTER_CLIP_BELT_FED = 0x5,
    AMMUNITION_COUNTER_CLIP_ALTERNATE_WEAPON = 0x6,
    AMMUNITION_COUNTER_CLIP_COUNT = 0x7,
  };

  // 115
  //
  enum weapon_projectile_explosion
  {
    WEAPON_PROJECTILE_EXPLOSION_GRENADE = 0x0,
    WEAPON_PROJECTILE_EXPLOSION_ROCKET = 0x1,
    WEAPON_PROJECTILE_EXPLOSION_FLASHBANG = 0x2,
    WEAPON_PROJECTILE_EXPLOSION_NONE = 0x3,
    WEAPON_PROJECTILE_EXPLOSION_DUD = 0x4,
    WEAPON_PROJECTILE_EXPLOSION_SMOKE = 0x5,
    WEAPON_PROJECTILE_EXPLOSION_HEAVY = 0x6,
    WEAPON_PROJECTILE_EXPLOSION_NUMBER = 0x7,
  };

  // 116
  //
  enum guided_missile_type
  {
    MISSILE_GUIDANCE_NONE = 0x0,
    MISSILE_GUIDANCE_SIDEWINDER = 0x1,
    MISSILE_GUIDANCE_HELLFIRE = 0x2,
    MISSILE_GUIDANCE_JAVELIN = 0x3,
    MISSILE_GUIDANCE_COUNT = 0x4,
  };

  // 165
  //
  enum expression_data_type
  {
    VALUE_INTEGER = 0x0,
    VALUE_FLOAT = 0x1,
    VALUE_STRING = 0x2,
    NUMBER_OF_INTERNAL_DATA_TYPES = 0x3,
    VALUE_FUNCTION = 0x3,
    NUMBER_OF_DATA_TYPES = 0x4,
  };

  // 249
  //
  enum leaderboard_column_type
  {
    LEADERBOARD_COLUMN_TYPE_NUMBER = 0x0,
    LEADERBOARD_COLUMN_TYPE_TIME = 0x1,
    LEADERBOARD_COLUMN_TYPE_LEVEL_EXPERIENCE = 0x2,
    LEADERBOARD_COLUMN_TYPE_PRESTIGE = 0x3,
    LEADERBOARD_COLUMN_TYPE_BIG_NUMBER = 0x4,
    LEADERBOARD_COLUMN_TYPE_PERCENTAGE = 0x5,
    LEADERBOARD_COLUMN_TYPE_COUNT = 0x6,
  };

  // 250
  //
  enum leaderboard_aggregation_type
  {
    LEADERBOARD_AGGREGATION_TYPE_MINIMUM = 0x0,
    LEADERBOARD_AGGREGATION_TYPE_MAXIMUM = 0x1,
    LEADERBOARD_AGGREGATION_TYPE_SUM = 0x2,
    LEADERBOARD_AGGREGATION_TYPE_LAST = 0x3,
    LEADERBOARD_AGGREGATION_TYPE_COUNT = 0x4,
  };

  // 261
  //
  enum vehicle_axle_type
  {
    VEHICLE_AXLE_FRONT = 0x0,
    VEHICLE_AXLE_REAR = 0x1,
    VEHICLE_AXLE_ALL = 0x2,
    VEHICLE_AXLE_COUNT = 0x3,
  };

  // 262
  //
  enum vehicle_type
  {
    VEHICLE_WHEELS_4 = 0x0,
    VEHICLE_TANK = 0x1,
    VEHICLE_PLANE = 0x2,
    VEHICLE_BOAT = 0x3,
    VEHICLE_ARTILLERY = 0x4,
    VEHICLE_HELICOPTER = 0x5,
    VEHICLE_SNOWMOBILE = 0x6,
    VEHICLE_TYPE_COUNT = 0x7,
  };

  // 313
  //
  enum dynamic_entity_type
  {
    DYNAMIC_ENTITY_TYPE_INVALID = 0x0,
    DYNAMIC_ENTITY_TYPE_CLUTTER = 0x1,
    DYNAMIC_ENTITY_TYPE_DESTRUCTIBLE = 0x2,
    DYNAMIC_ENTITY_TYPE_COUNT = 0x3,
  };

  // 366
  //
  enum dvar_type
  {
    DVAR_TYPE_BOOLEAN = 0x0,
    DVAR_TYPE_FLOAT = 0x1,
    DVAR_TYPE_FLOAT_2 = 0x2,
    DVAR_TYPE_FLOAT_3 = 0x3,
    DVAR_TYPE_FLOAT_4 = 0x4,
    DVAR_TYPE_INTEGER = 0x5,
    DVAR_TYPE_ENUMERATION = 0x6,
    DVAR_TYPE_STRING = 0x7,
    DVAR_TYPE_COLOR = 0x8,
    DVAR_TYPE_FLOAT_3_COLOR = 0x9,
    DVAR_TYPE_COUNT = 0xA,
  };

  // 368
  //
  enum structured_data_type_category
  {
    DATA_INTEGER = 0x0,
    DATA_BYTE = 0x1,
    DATA_BOOLEAN = 0x2,
    DATA_STRING = 0x3,
    DATA_ENUMERATION = 0x4,
    DATA_STRUCTURE = 0x5,
    DATA_INDEXED_ARRAY = 0x6,
    DATA_ENUMERATION_ARRAY = 0x7,
    DATA_FLOAT = 0x8,
    DATA_SHORT = 0x9,
    DATA_COUNT = 0xA,
  };

  // 548
  //
  enum node_type
  {
    NODE_ERROR = 0x0,
    NODE_PATH_NODE = 0x1,
    NODE_COVER_STAND = 0x2,
    NODE_COVER_CROUCH = 0x3,
    NODE_COVER_CROUCH_WINDOW = 0x4,
    NODE_COVER_PRONE = 0x5,
    NODE_COVER_RIGHT = 0x6,
    NODE_COVER_LEFT = 0x7,
    NODE_AMBUSH = 0x8,
    NODE_EXPOSED = 0x9,
    NODE_CONCEALMENT_STAND = 0xA,
    NODE_CONCEALMENT_CROUCH = 0xB,
    NODE_CONCEALMENT_PRONE = 0xC,
    NODE_DOOR = 0xD,
    NODE_DOOR_INTERIOR = 0xE,
    NODE_SCRIPTED = 0xF,
    NODE_NEGOTIATION_BEGIN = 0x10,
    NODE_NEGOTIATION_END = 0x11,
    NODE_TURRET = 0x12,
    NODE_GUARD = 0x13,
    NODE_NUMBER_OF_TYPES = 0x14,
    NODE_DO_NOT_LINK = 0x14,
  };

  // 551
  //
  enum path_node_error_code
  {
    PATH_NODE_ERROR_NONE = 0x0,
    PATH_NODE_ERROR_IN_SOLID = 0x1,
    PATH_NODE_ERROR_FLOATING = 0x2,
    PATH_NODE_ERROR_NO_LINK = 0x3,
    PATH_NODE_ERROR_DUPLICATE = 0x4,
    PATH_NODE_ERROR_NO_STANCE = 0x5,
    PATH_NODE_ERROR_INVALID_DOOR = 0x6,
    PATH_NODE_ERROR_NO_ANGLES = 0x7,
    PATH_NODE_ERROR_BAD_PLACEMENT = 0x8,
    NUMBER_OF_PATH_NODE_ERRORS = 0x9,
  };

  // 771
  //
  struct sound_information
  {
    int format;
    const void* data_pointer;
    unsigned int data_length;
    unsigned int rate;
    int bits;
    int channels;
    unsigned int samples;
    unsigned int block_size;
    const void* initial_pointer;
  };

  // 772
  //
  struct miles_sound_system_sound
  {
    sound_information information;
    char* data;
  };

  // 773
  //
  struct loaded_sound
  {
    const char* name;
    miles_sound_system_sound sound;
  };

  // 774
  //
  struct stream_file_name_raw
  {
    const char* directory;
    const char* name;
  };

  // 775
  //
  union stream_file_information
  {
    stream_file_name_raw raw;
  };

  // 776
  //
  struct stream_file_name
  {
    stream_file_information information;
  };

  // 777
  //
  struct streamed_sound
  {
    stream_file_name file_name;
  };

  // 778
  //
  union sound_file_reference
  {
    loaded_sound* loaded_sound_pointer;
    streamed_sound streamed_sound_pointer;
  };

  // 779
  //
  struct sound_file
  {
    char type;
    char exists;
    sound_file_reference union_data;
  };

  // 780
  //
  struct sound_curve
  {
    const char* file_name;
    unsigned short knot_count;
    float knots[16][2];
  };

  // 781
  //
  struct miles_sound_system_speaker_levels
  {
    int speaker;
    int number_of_levels;
    float levels[2];
  };

  // 782
  //
  struct miles_sound_system_channel_map
  {
    int speaker_count;
    miles_sound_system_speaker_levels speakers[6];
  };

  // 783
  //
  struct speaker_map
  {
    bool is_default;
    const char* name;
    miles_sound_system_channel_map channel_maps[2][2];
  };

  // 785
  //
  struct sound_alias
  {
    const char* alias_name;
    const char* subtitle;
    const char* secondary_alias_name;
    const char* chain_alias_name;
    const char* mixer_group;
    sound_file* sound_file_pointer;
    int sequence;
    float minimum_volume;
    float maximum_volume;
    float minimum_pitch;
    float maximum_pitch;
    float minimum_distance;
    float maximum_distance;
    float minimum_velocity;
    int flags;

    union
    {
      float slave_percentage;
      float master_percentage;
    };

    float probability;
    float low_frequency_effect_percentage;
    float center_percentage;
    int start_delay;
    sound_curve* volume_falloff_curve;
    float minimum_envelop;
    float maximum_envelop;
    float envelop_percentage;
    speaker_map* speaker_map_pointer;
  };

  // 791
  //
  union dvar_value
  {
    bool enabled;
    int integer;
    unsigned int unsigned_integer;
    float value;
    float vector[4];
    const char* string;
    char color[4];
  };

  // 795
  //
  union dvar_limits
  {
    struct
    {
      int string_count;
      const char** strings;
    } enumeration;

    struct
    {
      int minimum;
      int maximum;
    } integer;

    struct
    {
      float minimum;
      float maximum;
    } value;

    struct
    {
      float minimum;
      float maximum;
    } vector;
  };

  // 796
  //
  struct dvar
  {
    const char* name;
    unsigned int flags;
    dvar_type type;
    bool is_modified;
    dvar_value current_value;
    dvar_value latched_value;
    dvar_value reset_value;
    dvar_limits domain;
    bool (__cdecl* domain_function) (dvar*, dvar_value);
    dvar* hash_next;
  };

  struct sound_alias_list
  {
    const char* alias_name;
    sound_alias* head;
    int count;
  };

  // 962
  //
  struct graphics_draw_surface_fields
  {
    unsigned long long object_identifier : 16;
    unsigned long long reflection_probe_index : 8;
    unsigned long long has_graphics_entity_index : 1;
    unsigned long long custom_index : 5;
    unsigned long long material_sorted_index : 12;
    unsigned long long prepass : 2;
    unsigned long long use_hero_lighting : 1;
    unsigned long long scene_light_index : 8;
    unsigned long long surface_type : 4;
    unsigned long long primary_sort_key : 6;
    unsigned long long unused : 1;
  };

  // 963
  //
  union graphics_draw_surface
  {
    graphics_draw_surface_fields fields;
    unsigned long long packed;
  };

  // 964
  //
  struct material_information
  {
    const char* name;
    char game_flags;
    char sort_key;
    char texture_atlas_row_count;
    char texture_atlas_column_count;
    graphics_draw_surface draw_surface;
    unsigned int surface_type_bits;
    unsigned short hash_index;
  };

  // 965
  //
  struct material_technique_set
  {
    const char* name;
    char world_vertex_format;
    bool has_been_uploaded;
    char unused[1];
    material_technique_set* remapped_technique_set;
    material_technique* techniques[48];
  };

  // 966
  //
  struct material_stream_routing
  {
    char source;
    char destination;
  };

  // 968
  //
  struct material_vertex_stream_routing
  {
    material_stream_routing data[13];
    IDirect3DVertexDeclaration9* declaration[16];
  };

  // 969
  //
  struct material_vertex_declaration
  {
    const char* name;
    char stream_count;
    bool has_optional_source;
    material_vertex_stream_routing routing;
  };

  // 971
  //
  struct graphics_vertex_shader_load_definition
  {
    unsigned int* program;
    unsigned short program_size;
    unsigned short load_for_renderer;
  };

  // 972
  //
  struct material_vertex_shader_program
  {
    IDirect3DVertexShader9* vertex_shader;
    graphics_vertex_shader_load_definition load_definition;
  };

  // 973
  //
  struct material_vertex_shader
  {
    const char* name;
    material_vertex_shader_program program;
  };

  // 975
  //
  struct graphics_pixel_shader_load_definition
  {
    unsigned int* program;
    unsigned short program_size;
    unsigned short load_for_renderer;
  };

  // 976
  //
  struct material_pixel_shader_program
  {
    IDirect3DPixelShader9* pixel_shader;
    graphics_pixel_shader_load_definition load_definition;
  };

  // 977
  //
  struct material_pixel_shader
  {
    const char* name;
    material_pixel_shader_program program;
  };

  // 978
  //
  struct material_argument_code_constant
  {
    unsigned short index;
    char first_row;
    char row_count;
  };

  // 979
  //
  union material_argument_definition
  {
    const float* literal_constant;
    material_argument_code_constant code_constant;
    unsigned int code_sampler;
    unsigned int name_hash;
  };

  // 980
  //
  struct material_shader_argument
  {
    unsigned short type;
    unsigned short destination;
    material_argument_definition union_data;
  };

  // 981
  //
  struct material_pass
  {
    material_vertex_declaration* vertex_declaration;
    material_vertex_shader* vertex_shader;
    material_pixel_shader* pixel_shader;
    char per_primitive_argument_count;
    char per_object_argument_count;
    char stable_argument_count;
    char custom_sampler_flags;
    material_shader_argument* arguments;
  };

  // 982
  //
  struct material_technique
  {
    const char* name;
    unsigned short flags;
    unsigned short pass_count;
    material_pass pass_array[1];
  };

  // 988
  //
  struct graphics_image_load_definition
  {
    char level_count;
    char padding[3];
    int flags;
    int format;
    int resource_size;
    char data[1];
  };

  // 989
  //
  union graphics_texture
  {
    IDirect3DBaseTexture9* base_map;
    IDirect3DTexture9* map;
    IDirect3DVolumeTexture9* volume_map;
    IDirect3DCubeTexture9* cube_map;
    graphics_image_load_definition* load_definition;
  };

  // 990
  //
  struct picture_mipmap
  {
    char platform[2];
  };

  // 991
  //
  struct card_memory
  {
    int platform[2];
  };

  // 992
  //
  struct graphics_image
  {
    graphics_texture texture;
    char map_type;
    char semantic;
    char category;
    bool use_standard_rgb_reads;
    picture_mipmap picture_mipmap_data;
    bool no_picture_mipmap;
    char track;
    card_memory card_memory_data;
    unsigned short width;
    unsigned short height;
    unsigned short depth;
    bool delay_load_pixels;
    const char* name;
  };

  // 993
  //
  struct water_writable
  {
    float float_time;
  };

  // 994
  //
  struct complex
  {
    float real;
    float imaginary;
  };

  // 995
  //
  struct water
  {
    water_writable writable;
    complex* h0;
    float* w_term;
    int m;
    int n;
    float length_x;
    float length_z;
    float gravity;
    float wind_velocity;
    float wind_direction[2];
    float amplitude;
    float code_constant[4];
    graphics_image* image;
  };

  // 996
  //
  union material_texture_definition_information
  {
    graphics_image* image;
    water* water_pointer;
  };

  // 997
  //
  struct material_texture_definition
  {
    unsigned int name_hash;
    char name_start;
    char name_end;
    char sampler_state;
    char semantic;
    material_texture_definition_information union_data;
  };

  // 998
  //
  struct material_constant_definition
  {
    unsigned int name_hash;
    char name[12];
    float literal[4];
  };

  // 999
  //
  struct graphics_state_bits
  {
    unsigned int load_bits[2];
  };

  // 1000
  //
  struct material
  {
    material_information information;
    char state_bits_entry[48];
    char texture_count;
    char constant_count;
    char state_bits_count;
    char state_flags;
    char camera_region;
    material_technique_set* technique_set;
    material_texture_definition* texture_table;
    material_constant_definition* constant_table;
    graphics_state_bits* state_bits_table;
  };

  // 1002
  //
  struct glyph
  {
    unsigned short letter;
    char x0;
    char y0;
    char delta_x;
    char pixel_width;
    char pixel_height;
    float s0;
    float t0;
    float s1;
    float t1;
  };

  // 1003
  //
  struct font
  {
    const char* font_name;
    int pixel_height;
    int glyph_count;
    material* material_pointer;
    material* glow_material_pointer;
    glyph* glyphs;
  };

  // 1015
  struct xnetwork_address
  {
    in_addr internet_address;
    in_addr internet_address_online;
    unsigned short port;
    unsigned short port_online;
    char enet_address_bytes[4];
    char online_address_bytes[20];
  };

  // 1060
  //
  struct collision_plane
  {
    float normal[3];
    float distance;
    char type;
    char padding[3];
  };

  // 1061
  //
  struct collision_brush_side
  {
    collision_plane* plane;
    unsigned short material_number;
    char first_adjacent_side_offset;
    char edge_count;
  };

  // 1062
  //
  struct collision_brush
  {
    unsigned short number_of_sides;
    unsigned short glass_piece_index;
    collision_brush_side* sides;
    char* base_adjacent_side;
    short axial_material_number[2][3];
    char first_adjacent_side_offsets[2][3];
    char edge_count[2][3];
  };

  // 1063
  //
  struct profile_script_writable
  {
    int reference_count;
    unsigned int start_time;
    unsigned int total_time;
  };

  // 1064
  //
  struct profile_script
  {
    profile_script_writable write[40];
    volatile unsigned int total_time[40];
    volatile unsigned int average_time[40];
    volatile unsigned int maximum_time[40];
    volatile float cumulative[40];
    char profile_script_names[40][20];
    int script_source_buffer_index[32];
    unsigned int source_total;
    unsigned int source_average_time;
    unsigned int source_maximum_time;
  };

  // 1065
  //
  struct bounds
  {
    float mid_point[3];
    float half_size[3];
  };

  // 1066
  //
  struct brush_wrapper
  {
    bounds bounds_data;
    collision_brush brush;
    int total_edge_count;
    collision_plane* planes;
  };

  // 1067
  //
  struct xmodel_collision_triangle
  {
    float plane[4];
    float s_vector[4];
    float t_vector[4];
  };

  // 1068
  //
  struct trigger_slab
  {
    float direction[3];
    float mid_point;
    float half_size;
  };

  // 1069
  //
  struct physics_mass
  {
    float center_of_mass[3];
    float moments_of_inertia[3];
    float products_of_inertia[3];
  };

  // 1070
  //
  struct trigger_hull
  {
    bounds bounds_data;
    int contents;
    unsigned short slab_count;
    unsigned short first_slab;
  };

  // 1071
  //
  struct effects_spawn_definition_looping
  {
    int interval_milliseconds;
    int count;
  };

  // 1072
  //
  struct effects_integer_range
  {
    int base;
    int amplitude;
  };

  // 1073
  //
  struct effects_spawn_definition_one_shot
  {
    effects_integer_range count;
  };

  // 1074
  //
  union effects_spawn_definition
  {
    effects_spawn_definition_looping looping;
    effects_spawn_definition_one_shot one_shot;
  };

  // 1075
  //
  struct effects_float_range
  {
    float base;
    float amplitude;
  };

  // 1076
  //
  struct effects_element_atlas
  {
    char behavior;
    char index;
    char frames_per_second;
    char loop_count;
    char column_index_bits;
    char row_index_bits;
    short entry_count;
  };

  // 1077
  //
  struct effects_element_vector3_range
  {
    float base[3];
    float amplitude[3];
  };

  // 1078
  //
  struct effects_element_velocity_state_in_frame
  {
    effects_element_vector3_range velocity;
    effects_element_vector3_range total_delta;
  };

  // 1079
  //
  struct effects_element_velocity_state_sample
  {
    effects_element_velocity_state_in_frame local;
    effects_element_velocity_state_in_frame world;
  };

  // 1080
  //
  struct effects_element_visual_state
  {
    char color[4];
    float rotation_delta;
    float rotation_total;
    float size[2];
    float scale;
  };

  // 1081
  //
  struct effects_element_visual_state_sample
  {
    effects_element_visual_state base;
    effects_element_visual_state amplitude;
  };

  // 1082
  //
  struct effects_element_mark_visuals
  {
    material* materials[2];
  };

  // 1083
  //
  struct dynamic_object_animation_matrix
  {
    float quaternion[4];
    float translation[3];
    float translation_weight;
  };

  // 1084
  //
  struct xsurface_vertex_information
  {
    short vertex_count[4];
    unsigned short* vertices_blend;
  };

  // 1085
  //
  union graphics_color
  {
    unsigned int packed;
    char array[4];
  };

  // 1086
  //
  union packed_texture_coordinates
  {
    unsigned int packed;
  };

  // 1087
  //
  union packed_unit_vector
  {
    unsigned int packed;
    char array[4];
  };

  // 1088
  //
  struct graphics_packed_vertex
  {
    float coordinates[3];
    float binormal_sign;
    graphics_color color;
    packed_texture_coordinates texture_coordinate;
    packed_unit_vector normal;
    packed_unit_vector tangent;
  };

  // 1089
  //
  struct xsurface_collision_axis_aligned_bounding_box
  {
    unsigned short minimums[3];
    unsigned short maximums[3];
  };

  // 1090
  //
  struct xsurface_collision_node
  {
    xsurface_collision_axis_aligned_bounding_box bounding_box;
    unsigned short child_begin_index;
    unsigned short child_count;
  };

  // 1091
  //
  struct xsurface_collision_leaf
  {
    unsigned short triangle_begin_index;
  };

  // 1092
  //
  struct xsurface_collision_tree
  {
    float translation[3];
    float scale[3];
    unsigned int node_count;
    xsurface_collision_node* nodes;
    unsigned int leaf_count;
    xsurface_collision_leaf* leafs;
  };

  // 1093
  //
  struct xrigid_vertex_list
  {
    unsigned short bone_offset;
    unsigned short vertex_count;
    unsigned short triangle_offset;
    unsigned short triangle_count;
    xsurface_collision_tree* collision_tree;
  };

  // 1094
  //
  struct xsurface
  {
    char tile_mode;
    bool deformed;
    unsigned short vertex_count;
    unsigned short triangle_count;
    char zone_handle;
    unsigned short base_triangle_index;
    unsigned short base_vertex_index;
    unsigned short* triangle_indices;
    xsurface_vertex_information vertex_information;
    graphics_packed_vertex* vertices_0;
    unsigned int vertex_list_count;
    xrigid_vertex_list* vertex_list;
    int part_bits[6];
  };

  // 1095
  //
  struct xmodel_surfaces
  {
    const char* name;
    xsurface* surfaces;
    unsigned short number_of_surfaces;
    int part_bits[6];
  };

  // 1096
  //
  struct xmodel_level_of_detail_information
  {
    float distance;
    unsigned short number_of_surfaces;
    unsigned short surface_index;
    xmodel_surfaces* model_surfaces;
    int part_bits[6];
    xsurface* surfaces;
    char level_of_detail;
    char sub_model_collection_base_index_plus_one;
    char sub_model_collection_sub_index_mask;
    char sub_model_collection_bucket;
  };

  // 1097
  //
  struct xmodel_collision_surface
  {
    xmodel_collision_triangle* collision_triangles;
    int number_of_collision_triangles;
    bounds bounds_data;
    int bone_index;
    int contents;
    int surface_flags;
  };

  // 1098
  //
  struct xbone_information
  {
    bounds bounds_data;
    float radius_squared;
  };

  // 1099
  //
  struct physics_preset
  {
    const char* name;
    int type;
    float mass;
    float bounce;
    float friction;
    float bullet_force_scale;
    float explosive_force_scale;
    const char* sound_alias_prefix;
    float pieces_spread_fraction;
    float pieces_upward_velocity;
    bool temporary_default_to_cylinder;
    bool per_surface_sound_alias;
  };

  // 1100
  //
  struct physics_geometry_information
  {
    brush_wrapper* brush_wrapper_pointer;
    int type;
    float orientation[3][3];
    bounds bounds_data;
  };

  // 1101
  //
  struct physics_collision_map
  {
    const char* name;
    unsigned int count;
    physics_geometry_information* geometries;
    physics_mass mass;
    bounds bounds_data;
  };

  // 1102
  //
  struct xmodel
  {
    const char* name;
    char number_of_bones;
    char number_of_root_bones;
    char number_of_surfaces;
    char level_of_detail_ramp_type;
    float scale;
    unsigned int no_scale_part_bits[6];
    unsigned short* bone_names;
    char* parent_list;
    short* quaternions;
    float* translations;
    char* part_classification;
    dynamic_object_animation_matrix* base_matrix;
    material** material_handles;
    xmodel_level_of_detail_information level_of_detail_information[4];
    char maximum_loaded_level_of_detail;
    char number_of_levels_of_detail;
    char collision_level_of_detail;
    char flags;
    xmodel_collision_surface* collision_surfaces;
    int number_of_collision_surfaces;
    int contents;
    xbone_information* bone_information;
    float radius;
    bounds bounds_data;
    int memory_usage;
    bool bad;
    physics_preset* physics_preset_pointer;
    physics_collision_map* physics_collision_map_pointer;
  };

  // 1103
  //
  struct effects_effect_definition
  {
    const char* name;
    int flags;
    int total_size;
    int millisecond_looping_life;
    int element_definition_count_looping;
    int element_definition_count_one_shot;
    int element_definition_count_emission;
    effects_element_definition* element_definitions;
  };

  // 1104
  //
  union effects_effect_definition_reference
  {
    effects_effect_definition* handle;
    const char* name;
  };

  // 1105
  //
  union effects_element_visuals
  {
    const void* anonymous;
    material* material_pointer;
    xmodel* model;
    effects_effect_definition_reference effect_definition;
    const char* sound_name;
  };

  // 1106
  //
  union effects_element_definition_visuals
  {
    effects_element_mark_visuals* mark_array;
    effects_element_visuals* array;
    effects_element_visuals instance;
  };

  // 1107
  //
  struct effects_trail_vertex
  {
    float position[2];
    float normal[2];
    float texture_coordinate;
  };

  // 1108
  //
  struct effects_trail_definition
  {
    int scroll_time_milliseconds;
    int repeat_distance;
    float inverse_split_distance;
    float inverse_split_arc_distance;
    float inverse_split_time;
    int vertex_count;
    effects_trail_vertex* vertices;
    int index_count;
    unsigned short* indices;
  };

  // 1109
  //
  struct effects_spark_fountain_definition
  {
    float gravity;
    float bounce_fraction;
    float bounce_randomness;
    float spark_spacing;
    float spark_length;
    int spark_count;
    float loop_time;
    float velocity_minimum;
    float velocity_maximum;
    float velocity_cone_fraction;
    float rest_speed;
    float boost_time;
    float boost_factor;
  };

  // 1110
  //
  union effects_element_extended_definition_pointer
  {
    effects_trail_definition* trail_definition;
    effects_spark_fountain_definition* spark_fountain_definition;
    void* unknown_definition;
  };

  // 1111
  //
  struct effects_element_definition
  {
    int flags;
    effects_spawn_definition spawn;
    effects_float_range spawn_range;
    effects_float_range fade_in_range;
    effects_float_range fade_out_range;
    float spawn_frustum_cull_radius;
    effects_integer_range spawn_delay_milliseconds;
    effects_integer_range life_span_milliseconds;
    effects_float_range spawn_origin[3];
    effects_float_range spawn_offset_radius;
    effects_float_range spawn_offset_height;
    effects_float_range spawn_angles[3];
    effects_float_range angular_velocity[3];
    effects_float_range initial_rotation;
    effects_float_range gravity;
    effects_float_range reflection_factor;
    effects_element_atlas atlas;
    char element_type;
    char visual_count;
    char velocity_interval_count;
    char visual_state_interval_count;
    effects_element_velocity_state_sample* velocity_samples;
    effects_element_visual_state_sample* visual_samples;
    effects_element_definition_visuals visuals;
    bounds collision_bounds;
    effects_effect_definition_reference effect_on_impact;
    effects_effect_definition_reference effect_on_death;
    effects_effect_definition_reference effect_emitted;
    effects_float_range emission_distance;
    effects_float_range emission_distance_variance;
    effects_element_extended_definition_pointer extended;
    char sort_order;
    char lighting_fraction;
    char use_item_clip;
    char fade_information;
  };

  // 1112
  //
  struct effects_impact_entry
  {
    effects_effect_definition* non_flesh[31];
    effects_effect_definition* flesh[4];
  };

  // 1113
  //
  struct migration_persistence
  {
    int time;
    bool stance_held;
    stance_state stance;
    stance_state stance_position;
    int stance_time;
    int client_game_user_command_weapon;
    int client_game_user_command_offhand_index;
    unsigned int weapon_select;
    int weapon_select_time;
    int weapon_forced_select_time;
    unsigned int weapon_latest_primary_index;
    unsigned short primary_weapon_for_alternate[1400];
    int hold_breath_time;
    int hold_breath_in_time;
    int hold_breath_delay;
    float hold_breath_fraction;
  };

  // 1114
  //
  struct client_user_interface_active
  {
    bool active;
    bool is_running;
    bool client_game_initialized;
    bool client_game_initialize_called;
    bool map_preloaded;
    client_migration_state migration_state;
    migration_persistence migration_persistence_data;
    migration_verbose_state verbose_migration_state;
    int verbose_migration_data;
    int key_catchers;
    bool display_head_up_display_with_keycatcher_user_interface;
    connection_state connection_state_status;
    bool invited;
    char items_unlocked[256];
    bool items_unlocked_initialized;
    bool items_unlocked_last_game_dirty;
    unsigned short items_unlocked_last_game[16];
    int items_unlocked_last_game_count;
    char* items_unlocked_buffer;
    int items_unlocked_local_client_number;
    int items_unlocked_controller_index;
    int items_unlocked_statistics_source;
  };

  // 1115
  //
  struct network_profile_packet
  {
    int time;
    int size;
    int is_fragment;
  };

  // 1116
  //
  struct network_profile_stream
  {
    network_profile_packet packets[60];
    int current_packet;
    int bytes_per_second;
    int last_bytes_per_second_calculation_time;
    int counted_packets;
    int counted_fragments;
    int fragment_percentage;
    int largest_packet;
    int smallest_packet;
  };

  // 1117
  //
  struct client_voice_packet
  {
    char data[256];
    int data_size;
  };

  // 1118
  //
  struct voice_communication
  {
    client_voice_packet voice_packets[10];
    int voice_packet_count;
    int voice_packet_last_transmit;
    int packets_per_second;
    int packets_per_second_start;
  };

  // 1119
  //
  struct material_information_raw
  {
    unsigned int name_offset;
    unsigned int reference_image_name_offset;
    char game_flags;
    char padding[1];
    char texture_atlas_row_count;
    char texture_atlas_column_count;
    unsigned int sort_key_name_offset;
    float maximum_deform_move;
    char deform_flags;
    char usage;
    unsigned short tool_flags;
    unsigned int locale;
    unsigned short auto_texture_scale_width;
    unsigned short auto_texture_scale_height;
    float tessellation_size;
    int surface_flags;
    int contents;
  };

  // 1120
  //
  struct profile_atom
  {
    unsigned int value[1];
  };

  // 1121
  //
  struct profile_writable
  {
    int nesting;
    unsigned int hits;
    profile_atom start[3];
    profile_atom total;
    profile_atom child;
  };

  // 1122
  //
  struct profile_readable
  {
    unsigned int hits;
    profile_atom total;
    profile_atom self;
  };

  // 1123
  //
  struct profile
  {
    profile_writable write;
    profile_readable read;
  };

  // 1124
  //
  struct profile_guard
  {
    int identifier;
    profile** pointer_to_stack;
  };

  // 1125
  //
  struct profile_stack
  {
    profile profile_root;
    profile* profile_pointer_stack[16384];
    profile** profile_pointer_to_stack;
    profile profile_array[443];
    profile_atom profile_overhead_internal;
    profile_atom profile_overhead_inlineal;
    profile_guard profile_guard_stack[32];
    int profile_guard_position;
    float profile_time_scale;
  };

  // 1126
  //
  struct trigger_model
  {
    int contents;
    unsigned short hull_count;
    unsigned short first_hull;
  };

  // 1127
  //
  struct effects_impact_table
  {
    const char* name;
    effects_impact_entry* table;
  };

  // 1128
  //
  union xanimation_indices
  {
    char* byte_data;
    unsigned short* short_data;
    void* data;
  };

  // 1129
  //
  struct xanimation_notify_information
  {
    unsigned short name;
    float time;
  };

  // 1130
  //
  union xanimation_dynamic_frames
  {
    char (*byte_frames)[3];
    unsigned short (*short_frames)[3];
  };

  // 1131
  //
  union xanimation_dynamic_indices
  {
    char byte_indices[1];
    unsigned short short_indices[1];
  };

  // 1132
  //
  struct xanimation_part_translation_frames
  {
    float minimums[3];
    float size[3];
    xanimation_dynamic_frames frames;
    xanimation_dynamic_indices indices;
  };

  // 1133
  //
  union xanimation_part_translation_data
  {
    xanimation_part_translation_frames frames;
    float frame_0[3];
  };

  // 1134
  //
  struct xanimation_part_translation
  {
    unsigned short size;
    char small_translation;
    xanimation_part_translation_data union_data;
  };

  // 1135
  //
  struct xanimation_delta_part_quaternion_data_frames_2
  {
    short (*frames)[2];
    xanimation_dynamic_indices indices;
  };

  // 1136
  //
  union xanimation_delta_part_quaternion_data_2
  {
    xanimation_delta_part_quaternion_data_frames_2 frames;
    short frame_0[2];
  };

  // 1137
  //
  struct xanimation_delta_part_quaternion_2
  {
    unsigned short size;
    xanimation_delta_part_quaternion_data_2 union_data;
  };

  // 1138
  //
  struct xanimation_delta_part_quaternion_data_frames
  {
    short (*frames)[4];
    xanimation_dynamic_indices indices;
  };

  // 1139
  //
  union xanimation_delta_part_quaternion_data
  {
    xanimation_delta_part_quaternion_data_frames frames;
    short frame_0[4];
  };

  // 1140
  //
  struct xanimation_delta_part_quaternion
  {
    unsigned short size;
    xanimation_delta_part_quaternion_data union_data;
  };

  // 1141
  //
  struct xanimation_delta_part
  {
    xanimation_part_translation* translation;
    xanimation_delta_part_quaternion_2* quaternion_2;
    xanimation_delta_part_quaternion* quaternion;
  };

  // 1142
  //
  struct xanimation_parts
  {
    const char* name;
    unsigned short data_byte_count;
    unsigned short data_short_count;
    unsigned short data_integer_count;
    unsigned short random_data_byte_count;
    unsigned short random_data_integer_count;
    unsigned short number_of_frames;
    char flags;
    char bone_count[10];
    char notify_count;
    char asset_type;
    bool is_default;
    unsigned int random_data_short_count;
    unsigned int index_count;
    float frame_rate;
    float frequency;
    unsigned short* names;
    char* data_byte;
    short* data_short;
    int* data_integer;
    short* random_data_short;
    char* random_data_byte;
    int* random_data_integer;
    xanimation_indices indices;
    xanimation_notify_information* notify;
    xanimation_delta_part* delta_part;
  };

  // 1143
  //
  struct collision_static_model
  {
    xmodel* xmodel_pointer;
    float origin[3];
    float inverse_scaled_axis[3][3];
    bounds absolute_bounds;
  };

  // 1144
  //
  struct clip_material
  {
    const char* name;
    int surface_flags;
    int contents;
  };

  // 1145
  //
  struct collision_node
  {
    collision_plane* plane;
    short children[2];
  };

  // 1146
  //
  struct collision_leaf
  {
    unsigned short first_collision_axis_aligned_bounding_box_index;
    unsigned short collision_axis_aligned_bounding_box_count;
    int brush_contents;
    int terrain_contents;
    bounds bounds_data;
    int leaf_brush_node;
  };

  // 1147
  //
  struct collision_leaf_brush_node_leaf
  {
    unsigned short* brushes;
  };

  // 1148
  //
  struct collision_leaf_brush_node_children
  {
    float distance;
    float range;
    unsigned short child_offset[2];
  };

  // 1149
  //
  union collision_leaf_brush_node_data
  {
    collision_leaf_brush_node_leaf leaf;
    collision_leaf_brush_node_children children;
  };

  // 1150
  //
  struct collision_leaf_brush_node
  {
    char axis;
    short leaf_brush_count;
    int contents;
    collision_leaf_brush_node_data data;
  };

  // 1151
  //
  struct collision_border
  {
    float distance_equation[3];
    float z_base;
    float z_slope;
    float start;
    float length;
  };

  // 1152
  //
  struct collision_partition
  {
    char triangle_count;
    char border_count;
    char first_vertex_segment;
    int first_triangle;
    collision_border* borders;
  };

  // 1153
  //
  union collision_axis_aligned_bounding_box_tree_index
  {
    int first_child_index;
    int partition_index;
  };

  // 1154
  //
  struct collision_axis_aligned_bounding_box_tree
  {
    float mid_point[3];
    unsigned short material_index;
    unsigned short child_count;
    float half_size[3];
    collision_axis_aligned_bounding_box_tree_index union_data;
  };

  // 1155
  //
  struct collision_model
  {
    bounds bounds_data;
    float radius;
    collision_leaf leaf;
  };

  // 1156
  //
  struct map_triggers
  {
    unsigned int count;
    trigger_model* models;
    unsigned int hull_count;
    trigger_hull* hulls;
    unsigned int slab_count;
    trigger_slab* slabs;
  };

  // 1157
  //
  struct stage
  {
    const char* name;
    float origin[3];
    unsigned short trigger_index;
    char sun_primary_light_index;
  };

  // 1158
  //
  struct map_entities
  {
    const char* name;
    char* entity_string;
    int number_of_entity_characters;
    map_triggers trigger;
    stage* stages;
    char stage_count;
  };

  // 1159
  //
  struct static_model_axis_aligned_bounding_box_node
  {
    bounds bounds_data;
    unsigned short first_child;
    unsigned short child_count;
  };

  // 1160
  //
  struct graphics_placement
  {
    float quaternion[4];
    float origin[3];
  };

  // 1161
  //
  struct dynamic_entity_definition
  {
    dynamic_entity_type type;
    graphics_placement pose;
    xmodel* xmodel_pointer;
    unsigned short brush_model;
    unsigned short physics_brush_model;
    effects_effect_definition* destroy_effects;
    physics_preset* physics_preset_pointer;
    int health;
    physics_mass mass;
    int contents;
  };

  // 1162
  //
  struct dynamic_entity_pose
  {
    graphics_placement pose;
    float radius;
  };

  // 1163
  //
  struct dynamic_entity_client
  {
    int physics_object_identifier;
    unsigned short flags;
    unsigned short lighting_handle;
    int health;
  };

  // 1164
  //
  struct dynamic_entity_collision
  {
    unsigned short sector;
    unsigned short next_entity_in_sector;
    float link_minimums[2];
    float link_maximums[2];
  };

  // 1165
  //
  struct collision_clip_map
  {
    const char* name;
    int is_in_use;
    int plane_count;
    collision_plane* planes;
    unsigned int number_of_static_models;
    collision_static_model* static_model_list;
    unsigned int number_of_materials;
    clip_material* materials;
    unsigned int number_of_brush_sides;
    collision_brush_side* brush_sides;
    unsigned int number_of_brush_edges;
    char* brush_edges;
    unsigned int number_of_nodes;
    collision_node* nodes;
    unsigned int number_of_leafs;
    collision_leaf* leafs;
    unsigned int leaf_brush_nodes_count;
    collision_leaf_brush_node* leaf_brush_nodes;
    unsigned int number_of_leaf_brushes;
    unsigned short* leaf_brushes;
    unsigned int number_of_leaf_surfaces;
    unsigned int* leaf_surfaces;
    unsigned int vertex_count;
    float (*vertices)[3];
    int triangle_count;
    unsigned short* triangle_indices;
    char* triangle_edge_is_walkable;
    int border_count;
    collision_border* borders;
    int partition_count;
    collision_partition* partitions;
    int axis_aligned_bounding_box_tree_count;
    collision_axis_aligned_bounding_box_tree* axis_aligned_bounding_box_trees;
    unsigned int number_of_sub_models;
    collision_model* collision_models;
    unsigned short number_of_brushes;
    collision_brush* brushes;
    bounds* brush_bounds;
    int* brush_contents;
    map_entities* map_entities_pointer;
    unsigned short static_model_node_count;
    static_model_axis_aligned_bounding_box_node* static_model_nodes;
    unsigned short dynamic_entity_count[2];
    dynamic_entity_definition* dynamic_entity_definition_list[2];
    dynamic_entity_pose* dynamic_entity_pose_list[2];
    dynamic_entity_client* dynamic_entity_client_list[2];
    dynamic_entity_collision* dynamic_entity_collision_list[2];
    unsigned int checksum;
  };

  // 1166
  //
  struct common_primary_light
  {
    char type;
    char can_use_shadow_map;
    char exponent;
    char unused;
    float color[3];
    float direction[3];
    float origin[3];
    float radius;
    float cosine_half_field_of_view_outer;
    float cosine_half_field_of_view_inner;
    float cosine_half_field_of_view_expanded;
    float rotation_limit;
    float translation_limit;
    const char* definition_name;
  };

  // 1167
  //
  struct common_world
  {
    const char* name;
    int is_in_use;
    unsigned int primary_light_count;
    common_primary_light* primary_lights;
  };

  // 1168
  //
  struct path_link
  {
    float distance;
    unsigned short node_number;
    char disconnect_count;
    char negotiation_link;
    char flags;
    char bad_place_count[3];
  };

  // 1170
  //
  struct path_node_constant
  {
    node_type type;
    unsigned short spawn_flags;
    unsigned short target_name;
    unsigned short script_link_name;
    unsigned short script_noteworthy;
    unsigned short target;
    unsigned short animation_script;
    int animation_script_function;
    float origin[3];
    float angle;
    float forward[2];
    float radius;

    union
    {
      float minimum_use_distance_squared;
      path_node_error_code error;
    };

    short overlap_node[2];
    unsigned short total_link_count;
    path_link* links;
  };

  // 1171
  //
  struct path_node_dynamic
  {
    void* owner;
    int free_time;
    int valid_time[3];
    int dangerous_node_time[3];
    int in_player_line_of_sight_time;
    short link_count;
    short overlap_count;
    short turret_entity_number;
    char user_count;
    bool has_bad_place_link;
  };

  // 1174
  //
  struct path_node_transient
  {
    int search_frame;
    path_node* next_open;
    path_node* previous_open;
    path_node* parent;
    float cost;
    float heuristic;

    union
    {
      float node_cost;
      int link_index;
    };
  };

  // 1172
  //
  struct path_node
  {
    path_node_constant constant;
    path_node_dynamic dynamic;
    path_node_transient transient;
  };

  // 1175
  //
  struct path_base_node
  {
    float origin[3];
    unsigned int type;
  };

  // 1177
  //
  struct path_node_tree_nodes
  {
    int node_count;
    unsigned short* nodes;
  };

  // 1178
  //
  union path_node_tree_information
  {
    path_node_tree* child[2];
    path_node_tree_nodes struct_data;
  };

  // 1176
  //
  struct path_node_tree
  {
    int axis;
    float distance;
    path_node_tree_information union_data;
  };

  // 1179
  //
  struct path_data
  {
    unsigned int node_count;
    path_node* nodes;
    path_base_node* base_nodes;
    unsigned int chain_node_count;
    unsigned short* chain_node_for_node;
    unsigned short* node_for_chain_node;
    int visibility_bytes;
    char* path_visibility;
    int node_tree_count;
    path_node_tree* node_tree;
  };

  // 1180
  //
  struct vehicle_track_obstacle
  {
    float origin[2];
    float radius;
  };

  // 1181
  //
  struct vehicle_track_sector
  {
    float start_edge_direction[2];
    float start_edge_distance;
    float left_edge_direction[2];
    float left_edge_distance;
    float right_edge_direction[2];
    float right_edge_distance;
    float sector_length;
    float sector_width;
    float total_prior_length;
    float total_following_length;
    vehicle_track_obstacle* obstacles;
    unsigned int obstacle_count;
  };

  // 1182
  //
  struct vehicle_track_segment
  {
    const char* target_name;
    vehicle_track_sector* sectors;
    unsigned int sector_count;
    vehicle_track_segment** next_branches;
    unsigned int next_branches_count;
    vehicle_track_segment** previous_branches;
    unsigned int previous_branches_count;
    float end_edge_direction[2];
    float end_edge_distance;
    float total_length;
  };

  // 1183
  //
  struct vehicle_track
  {
    vehicle_track_segment* segments;
    unsigned int segment_count;
  };

  // 1184
  //
  struct game_glass_piece
  {
    unsigned short damage_taken;
    unsigned short collapse_time;
    int last_state_change_time;
    char impact_direction;
    char impact_position[2];
  };

  // 1185
  //
  struct game_glass_name
  {
    char* name_string;
    unsigned short name;
    unsigned short piece_count;
    unsigned short* piece_indices;
  };

  // 1186
  //
  struct game_glass_data
  {
    game_glass_piece* glass_pieces;
    unsigned int piece_count;
    unsigned short damage_to_weaken;
    unsigned short damage_to_destroy;
    unsigned int glass_name_count;
    game_glass_name* glass_names;
    char padding[108];
  };

  // 1187
  //
  struct game_world_single_player
  {
    const char* name;
    path_data path;
    vehicle_track vehicle_track_data;
    game_glass_data* game_glass_data_pointer;
  };

  // 1188
  //
  struct game_world_multi_player
  {
    const char* name;
    game_glass_data* game_glass_data_pointer;
  };

  // 1189
  //
  struct effects_glass_definition
  {
    float half_thickness;
    float texture_vectors[2][2];
    graphics_color color;
    material* material_pointer;
    material* material_shattered;
    physics_preset* physics_preset_pointer;
  };

  // 1190
  //
  struct effects_spatial_frame
  {
    float quaternion[4];
    float origin[3];
  };

  // 1192
  //
  union effects_glass_piece_place
  {
    // 1191
    //
    struct
    {
      effects_spatial_frame frame;
      float radius;
    };

    unsigned int next_free;
  };

  // 1193
  //
  struct effects_glass_piece_state
  {
    float texture_coordinate_origin[2];
    unsigned int support_mask;
    unsigned short initial_index;
    unsigned short geometry_data_start;
    char definition_index;
    char padding[5];
    char vertex_count;
    char hole_data_count;
    char crack_data_count;
    char fan_data_count;
    unsigned short flags;
    float area_times_two;
  };

  // 1194
  //
  struct effects_glass_piece_dynamics
  {
    int fall_time;
    int physics_object_identifier;
    int physics_joint_identifier;
    float velocity[3];
    float angular_velocity[3];
  };

  // 1195
  //
  struct effects_glass_vertex
  {
    short x;
    short y;
  };

  // 1196
  //
  struct effects_glass_hole_header
  {
    unsigned short unique_vertex_count;
    char touch_vertex;
    char padding[1];
  };

  // 1197
  //
  struct effects_glass_crack_header
  {
    unsigned short unique_vertex_count;
    char begin_vertex_index;
    char end_vertex_index;
  };

  // 1198
  //
  union effects_glass_geometry_data
  {
    effects_glass_vertex vertex;
    effects_glass_hole_header hole;
    effects_glass_crack_header crack;
    char as_bytes[4];
    short anonymous[2];
  };

  // 1199
  //
  struct effects_glass_initial_piece_state
  {
    effects_spatial_frame frame;
    float radius;
    float texture_coordinate_origin[2];
    unsigned int support_mask;
    float area_times_two;
    char definition_index;
    char vertex_count;
    char fan_data_count;
    char padding[1];
  };

  // 1200
  //
  struct effects_glass_system
  {
    int time;
    int previous_time;
    unsigned int definition_count;
    unsigned int piece_limit;
    unsigned int piece_word_count;
    unsigned int initial_piece_count;
    unsigned int cell_count;
    unsigned int active_piece_count;
    unsigned int first_free_piece;
    unsigned int geometry_data_limit;
    unsigned int geometry_data_count;
    unsigned int initial_geometry_data_count;
    effects_glass_definition* definitions;
    effects_glass_piece_place* piece_places;
    effects_glass_piece_state* piece_states;
    effects_glass_piece_dynamics* piece_dynamics;
    effects_glass_geometry_data* geometry_data;
    unsigned int* is_in_use;
    unsigned int* cell_bits;
    char* visibility_data;
    float (*link_origin)[3];
    float* half_thickness;
    unsigned short* lighting_handles;
    effects_glass_initial_piece_state* initial_piece_states;
    effects_glass_geometry_data* initial_geometry_data;
    bool need_to_compact_data;
    char initial_count;
    float effect_chance_accumulator;
    int last_piece_deletion_time;
  };

  // 1201
  //
  struct effects_world
  {
    const char* name;
    effects_glass_system glass_system;
  };

  // 1202
  //
  struct graphics_sky
  {
    int sky_surface_count;
    int* sky_start_surfaces;
    graphics_image* sky_image;
    char sky_sampler_state;
  };

  // 1203
  //
  struct graphics_world_dynamic_potential_visibility_set_planes
  {
    int cell_count;
    collision_plane* planes;
    unsigned short* nodes;
    unsigned int* scene_entity_cell_bits;
  };

  // 1204
  //
  struct graphics_cell_tree_count
  {
    int axis_aligned_bounding_box_tree_count;
  };

  // 1205
  //
  struct graphics_axis_aligned_bounding_box_tree
  {
    bounds bounds_data;
    unsigned short child_count;
    unsigned short surface_count;
    unsigned short start_surface_index;
    unsigned short surface_count_no_decal;
    unsigned short start_surface_index_no_decal;
    unsigned short static_model_index_count;
    unsigned short* static_model_indices;
    int children_offset;
  };

  // 1206
  //
  struct graphics_cell_tree
  {
    graphics_axis_aligned_bounding_box_tree* axis_aligned_bounding_box_tree;
  };

  // 1208
  //
  struct graphics_portal_writable
  {
    bool is_queued;
    bool is_ancestor;
    char recursion_depth;
    char hull_point_count;
    float (*hull_points)[2];
    graphics_portal* queued_parent;
  };

  // 1209
  //
  struct dynamic_potential_visibility_set_plane
  {
    float coefficients[4];
  };

  // 1207
  //
  struct graphics_portal
  {
    graphics_portal_writable writable;
    dynamic_potential_visibility_set_plane plane;
    float (*vertices)[3];
    unsigned short cell_index;
    char vertex_count;
    float hull_axis[2][3];
  };

  // 1210
  //
  struct graphics_cell
  {
    bounds bounds_data;
    int portal_count;
    graphics_portal* portals;
    char reflection_probe_count;
    char* reflection_probes;
  };

  // 1211
  //
  struct graphics_reflection_probe
  {
    float origin[3];
  };

  // 1212
  //
  struct graphics_lightmap_array
  {
    graphics_image* primary;
    graphics_image* secondary;
  };

  // 1213
  //
  struct graphics_world_vertex
  {
    float coordinates[3];
    float binormal_sign;
    graphics_color color;
    float texture_coordinate[2];
    float lightmap_coordinate[2];
    packed_unit_vector normal;
    packed_unit_vector tangent;
  };

  // 1215
  //
  struct graphics_world_vertex_data
  {
    graphics_world_vertex* vertices;
    IDirect3DVertexBuffer9* world_vertex_buffer;
  };

  // 1216
  //
  struct graphics_world_vertex_layer_data
  {
    char* data;
    IDirect3DVertexBuffer9* layer_vertex_buffer;
  };

  // 1217
  //
  struct graphics_world_draw
  {
    unsigned int reflection_probe_count;
    graphics_image** reflection_probes;
    graphics_reflection_probe* reflection_probe_origins;
    graphics_texture* reflection_probe_textures;
    int lightmap_count;
    graphics_lightmap_array* lightmaps;
    graphics_texture* lightmap_primary_textures;
    graphics_texture* lightmap_secondary_textures;
    graphics_image* lightmap_override_primary;
    graphics_image* lightmap_override_secondary;
    unsigned int vertex_count;
    graphics_world_vertex_data vertex_data;
    unsigned int vertex_layer_data_size;
    graphics_world_vertex_layer_data vertex_layer_data;
    unsigned int index_count;
    unsigned short* indices;
  };

  // 1218
  //
  struct graphics_light_grid_entry
  {
    unsigned short colors_index;
    char primary_light_index;
    char needs_trace;
  };

  // 1219
  //
  struct graphics_light_grid_colors
  {
    char rgb[56][3];
  };

  // 1220
  //
  struct graphics_light_grid
  {
    bool has_light_regions;
    unsigned int last_sun_primary_light_index;
    unsigned short minimums[3];
    unsigned short maximums[3];
    unsigned int row_axis;
    unsigned int column_axis;
    unsigned short* row_data_start;
    unsigned int raw_row_data_size;
    char* raw_row_data;
    unsigned int entry_count;
    graphics_light_grid_entry* entries;
    unsigned int color_count;
    graphics_light_grid_colors* colors;
  };

  // 1221
  //
  struct graphics_brush_model_writable
  {
    bounds bounds_data;
  };

  // 1222
  //
  struct graphics_brush_model
  {
    graphics_brush_model_writable writable;
    bounds bounds_data;
    float radius;
    unsigned short surface_count;
    unsigned short start_surface_index;
    unsigned short surface_count_no_decal;
  };

  // 1223
  //
  struct material_memory
  {
    material* material_pointer;
    int memory;
  };

  // 1224
  //
  struct sun_flare
  {
    bool has_valid_data;
    material* sprite_material;
    material* flare_material;
    float sprite_size;
    float flare_minimum_size;
    float flare_minimum_dot;
    float flare_maximum_size;
    float flare_maximum_dot;
    float flare_maximum_alpha;
    int flare_fade_in_time;
    int flare_fade_out_time;
    float blind_minimum_dot;
    float blind_maximum_dot;
    float blind_maximum_darken;
    int blind_fade_in_time;
    int blind_fade_out_time;
    float glare_minimum_dot;
    float glare_maximum_dot;
    float glare_maximum_lighten;
    int glare_fade_in_time;
    int glare_fade_out_time;
    float sun_effects_position[3];
  };

  // 1225
  //
  struct xmodel_draw_information
  {
    char has_graphics_entity_index;
    char level_of_detail;
    unsigned short surface_identifier;
  };

  // 1226
  //
  struct graphics_scene_dynamic_model
  {
    xmodel_draw_information information;
    unsigned short dynamic_entity_identifier;
  };

  // 1227
  //
  struct brush_model_draw_information
  {
    unsigned short surface_identifier;
  };

  // 1228
  //
  struct graphics_scene_dynamic_brush
  {
    brush_model_draw_information information;
    unsigned short dynamic_entity_identifier;
  };

  // 1229
  //
  struct graphics_shadow_geometry
  {
    unsigned short surface_count;
    unsigned short static_model_count;
    unsigned short* sorted_surface_index;
    unsigned short* static_model_index;
  };

  // 1230
  //
  struct graphics_light_region_axis
  {
    float direction[3];
    float mid_point;
    float half_size;
  };

  // 1231
  //
  struct graphics_light_region_hull
  {
    float k_discrete_oriented_polytope_mid_point[9];
    float k_discrete_oriented_polytope_half_size[9];
    unsigned int axis_count;
    graphics_light_region_axis* axis;
  };

  // 1232
  //
  struct graphics_light_region
  {
    unsigned int hull_count;
    graphics_light_region_hull* hulls;
  };

  // 1233
  //
  struct graphics_static_model_instance
  {
    bounds bounds_data;
    float lighting_origin[3];
  };

  // 1234
  //
  struct surface_triangles
  {
    unsigned int vertex_layer_data;
    unsigned int first_vertex;
    unsigned short vertex_count;
    unsigned short triangle_count;
    unsigned int base_index;
  };

  // 1235
  //
  struct graphics_surface_lighting_and_flags_fields
  {
    char lightmap_index;
    char reflection_probe_index;
    char primary_light_index;
    char flags;
  };

  // 1236
  //
  union graphics_surface_lighting_and_flags
  {
    graphics_surface_lighting_and_flags_fields fields;
    unsigned int packed;
  };

  // 1237
  //
  struct graphics_surface
  {
    surface_triangles triangles;
    material* material_pointer;
    graphics_surface_lighting_and_flags lighting_and_flags;
  };

  // 1238
  //
  struct graphics_surface_bounds
  {
    bounds bounds_data;
  };

  // 1239
  //
  struct graphics_packed_placement
  {
    float origin[3];
    float axis[3][3];
    float scale;
  };

  // 1240
  //
  struct graphics_static_model_draw_instance
  {
    graphics_packed_placement placement;
    xmodel* model;
    unsigned short cull_distance;
    unsigned short lighting_handle;
    char reflection_probe_index;
    char primary_light_index;
    char flags;
    char first_material_skin_index;
    graphics_color ground_lighting;
    unsigned short cache_identifier[4];
  };

  // 1241
  //
  struct graphics_world_dynamic_potential_visibility_set_static
  {
    unsigned int static_model_count;
    unsigned int static_surface_count;
    unsigned int static_surface_count_no_decal;
    unsigned int lit_opaque_surfaces_begin;
    unsigned int lit_opaque_surfaces_end;
    unsigned int lit_transparent_surfaces_begin;
    unsigned int lit_transparent_surfaces_end;
    unsigned int shadow_caster_surfaces_begin;
    unsigned int shadow_caster_surfaces_end;
    unsigned int emissive_surfaces_begin;
    unsigned int emissive_surfaces_end;
    unsigned int static_model_visibility_data_count;
    unsigned int surface_visibility_data_count;
    char* static_model_visibility_data[3];
    char* surface_visibility_data[3];
    unsigned short* sorted_surface_index;
    graphics_static_model_instance* static_model_instances;
    graphics_surface* surfaces;
    graphics_surface_bounds* surfaces_bounds;
    graphics_static_model_draw_instance* static_model_draw_instances;
    graphics_draw_surface* surface_materials;
    unsigned int* surface_casts_sun_shadow;
    volatile int usage_count;
  };

  // 1242
  //
  struct graphics_world_dynamic_potential_visibility_set_dynamic
  {
    unsigned int dynamic_entity_client_word_count[2];
    unsigned int dynamic_entity_client_count[2];
    unsigned int* dynamic_entity_cell_bits[2];
    char* dynamic_entity_visibility_data[2][3];
  };

  // 1243
  //
  struct graphics_hero_only_light
  {
    char type;
    char unused[3];
    float color[3];
    float direction[3];
    float origin[3];
    float radius;
    float cosine_half_field_of_view_outer;
    float cosine_half_field_of_view_inner;
    int exponent;
  };

  // 1244
  //
  struct graphics_world
  {
    const char* name;
    const char* base_name;
    int plane_count;
    int node_count;
    unsigned int surface_count;
    int sky_count;
    graphics_sky* skies;
    unsigned int last_sun_primary_light_index;
    unsigned int primary_light_count;
    unsigned int sort_key_lit_decal;
    unsigned int sort_key_effect_decal;
    unsigned int sort_key_effect_auto;
    unsigned int sort_key_distortion;
    graphics_world_dynamic_potential_visibility_set_planes
      dynamic_potential_visibility_set_planes;
    graphics_cell_tree_count* axis_aligned_bounding_box_tree_counts;
    graphics_cell_tree* axis_aligned_bounding_box_trees;
    graphics_cell* cells;
    graphics_world_draw draw;
    graphics_light_grid light_grid;
    int model_count;
    graphics_brush_model* models;
    bounds bounds_data;
    unsigned int checksum;
    int material_memory_count;
    material_memory* material_memory_pointer;
    sun_flare sun;
    float outdoor_lookup_matrix[4][4];
    graphics_image* outdoor_image;
    unsigned int* cell_caster_bits;
    unsigned int* cell_has_sun_lit_surfaces_bits;
    graphics_scene_dynamic_model* scene_dynamic_model;
    graphics_scene_dynamic_brush* scene_dynamic_brush;
    unsigned int* primary_light_entity_shadow_visibility;
    unsigned int* primary_light_dynamic_entity_shadow_visibility[2];
    char* non_sun_primary_light_for_model_dynamic_entity;
    graphics_shadow_geometry* shadow_geometry;
    graphics_light_region* light_region;
    graphics_world_dynamic_potential_visibility_set_static
      dynamic_potential_visibility_set_static_data;
    graphics_world_dynamic_potential_visibility_set_dynamic
      dynamic_potential_visibility_set_dynamic_data;
    unsigned int map_vertex_checksum;
    unsigned int hero_only_light_count;
    graphics_hero_only_light* hero_only_lights;
    char fog_types_allowed;
  };

  // 1245
  //
  struct graphics_light_image
  {
    graphics_image* image;
    char sampler_state;
  };

  // 1246
  //
  struct graphics_light_definition
  {
    const char* name;
    graphics_light_image attenuation;
    int lightmap_lookup_start;
  };

  // 1247
  //
  struct rectangle_definition
  {
    float x;
    float y;
    float width;
    float height;
    char horizontal_alignment;
    char vertical_alignment;
  };

  // 1248
  //
  struct window_definition
  {
    const char* name;
    rectangle_definition rectangle;
    rectangle_definition rectangle_client;
    const char* group;
    int style;
    int border;
    int owner_draw;
    int owner_draw_flags;
    float border_size;
    int static_flags;
    int dynamic_flags[1];
    int next_time;
    float foreground_color[4];
    float background_color[4];
    float border_color[4];
    float outline_color[4];
    float disable_color[4];
    material* background;
  };

  // 1249
  //
  struct menu_event_handler_set
  {
    int event_handler_count;
    menu_event_handler** event_handlers;
  };

  // 1250
  //
  struct expression_string
  {
    const char* string;
  };

  // 1252
  //
  union operand_internal_data_union
  {
    int integer_value;
    float float_value;
    expression_string string_value;
    statement* function;
  };

  // 1253
  //
  struct operand
  {
    expression_data_type data_type;
    operand_internal_data_union internals;
  };

  // 1251
  //
  struct statement
  {
    int number_of_entries;
    expression_entry* entries;
    expression_supporting_data* supporting_data;
    int last_execute_time;
    operand last_result;
  };

  // 1254
  //
  union entry_internal_data
  {
    int operation;
    operand operand_data;
  };

  // 1255
  //
  struct expression_entry
  {
    int type;
    entry_internal_data data;
  };

  // 1256
  //
  struct user_interface_function_list
  {
    int total_functions;
    statement** functions;
  };

  // 1257
  //
  struct static_dvar
  {
    dvar* dvar_pointer;
    char* dvar_name;
  };

  // 1258
  //
  struct static_dvar_list
  {
    int number_of_static_dvars;
    static_dvar** static_dvars;
  };

  // 1259
  //
  struct string_list
  {
    int total_strings;
    const char** strings;
  };

  // 1260
  //
  struct expression_supporting_data
  {
    user_interface_function_list user_interface_functions;
    static_dvar_list static_dvar_list_data;
    string_list user_interface_strings;
  };

  // 1261
  //
  struct conditional_script
  {
    menu_event_handler_set* event_handler_set;
    statement* event_expression;
  };

  // 1262
  //
  struct set_local_variable_data
  {
    const char* local_variable_name;
    statement* expression;
  };

  // 1263
  //
  union event_data
  {
    const char* unconditional_script;
    conditional_script* conditional_script_pointer;
    menu_event_handler_set* else_script;
    set_local_variable_data* set_local_variable_data_pointer;
  };

  // 1264
  //
  struct menu_event_handler
  {
    event_data event_data_union;
    char event_type;
  };

  // 1265
  //
  struct item_key_handler
  {
    int key;
    menu_event_handler_set* action;
    item_key_handler* next;
  };

  // 1276
  //
  struct menu_transition
  {
    int transition_type;
    int target_field;
    int start_time;
    float start_value;
    float end_value;
    float time;
    int end_trigger_type;
  };

  // 1266
  //
  struct menu_definition
  {
    window_definition window;
    const char* font;
    int full_screen;
    int item_count;
    int font_index;
    int cursor_item[1];
    int fade_cycle;
    float fade_clamp;
    float fade_amount;
    float fade_in_amount;
    float blur_radius;
    menu_event_handler_set* on_open;
    menu_event_handler_set* on_close_request;
    menu_event_handler_set* on_close;
    menu_event_handler_set* on_escape;
    item_key_handler* on_key;
    statement* visible_expression;
    const char* allowed_binding;
    const char* sound_name;
    int image_track;
    float focus_color[4];
    statement* rectangle_x_expression;
    statement* rectangle_y_expression;
    statement* rectangle_width_expression;
    statement* rectangle_height_expression;
    statement* open_sound_expression;
    statement* close_sound_expression;
    item_definition** items;
    menu_transition scale_transition[1];
    menu_transition alpha_transition[1];
    menu_transition x_transition[1];
    menu_transition y_transition[1];
    expression_supporting_data* expression_data;
  };

  // 1267
  //
  struct column_information
  {
    int position;
    int width;
    int maximum_characters;
    int alignment;
  };

  // 1268
  //
  struct list_box_definition
  {
    int mouse_position;
    int start_position[1];
    int end_position[1];
    int draw_padding;
    float element_width;
    float element_height;
    int element_style;
    int number_of_columns;
    column_information column_information_data[16];
    menu_event_handler_set* on_double_click;
    int not_selectable;
    int no_scroll_bars;
    int use_paging;
    float select_border[4];
    material* select_icon;
  };

  // 1269
  //
  struct edit_field_definition
  {
    float minimum_value;
    float maximum_value;
    float default_value;
    float range;
    int maximum_characters;
    int maximum_characters_goto_next;
    int maximum_paint_characters;
    int paint_offset;
  };

  // 1270
  //
  struct multiple_definition
  {
    const char* dvar_list[32];
    const char* dvar_string[32];
    float dvar_value[32];
    int count;
    int string_definition;
  };

  // 1271
  //
  struct news_ticker_definition
  {
    int feed_identifier;
    int speed;
    int spacing;
    int last_time;
    int start;
    int end;
    float x;
  };

  // 1272
  //
  struct text_scroll_definition
  {
    int start_time;
  };

  // 1273
  //
  union item_definition_data
  {
    list_box_definition* list_box;
    edit_field_definition* edit_field;
    multiple_definition* multiple;
    const char* enumeration_dvar_name;
    news_ticker_definition* ticker;
    text_scroll_definition* scroll;
    void* data;
  };

  // 1274
  //
  struct item_float_expression
  {
    int target;
    statement* expression;
  };

  // 1275
  //
  struct item_definition
  {
    window_definition window;
    rectangle_definition text_rectangle[1];
    int type;
    int data_type;
    int alignment;
    int font_enumeration;
    int text_align_mode;
    float text_align_x;
    float text_align_y;
    float text_scale;
    int text_style;
    int game_message_window_index;
    int game_message_window_mode;
    const char* text;
    int item_flags;
    menu_definition* parent;
    menu_event_handler_set* mouse_enter_text;
    menu_event_handler_set* mouse_exit_text;
    menu_event_handler_set* mouse_enter;
    menu_event_handler_set* mouse_exit;
    menu_event_handler_set* action;
    menu_event_handler_set* accept;
    menu_event_handler_set* on_focus;
    menu_event_handler_set* leave_focus;
    const char* dvar_name;
    const char* dvar_test;
    item_key_handler* on_key;
    const char* enable_dvar;
    const char* local_variable;
    int dvar_flags;
    sound_alias_list* focus_sound;
    float special;
    int cursor_position[1];
    item_definition_data type_data;
    int image_track;
    int float_expression_count;
    item_float_expression* float_expressions;
    statement* visible_expression;
    statement* disabled_expression;
    statement* text_expression;
    statement* material_expression;
    float glow_color[4];
    bool decay_active;
    int effects_birth_time;
    int effects_letter_time;
    int effects_decay_start_time;
    int effects_decay_duration;
    int last_sound_played_time;
  };

  // 1277
  //
  struct menu_list
  {
    const char* name;
    int menu_count;
    menu_definition** menus;
  };

  // 1278
  //
  struct localize_entry
  {
    const char* value;
    const char* name;
  };

  // 1279
  //
  struct tracer_definition
  {
    const char* name;
    material* material_pointer;
    unsigned int draw_interval;
    float speed;
    float beam_length;
    float beam_width;
    float screw_radius;
    float screw_distance;
    float colors[5][4];
  };

  // 1280
  //
  struct weapon_definition
  {
    const char* overlay_name;
    xmodel** gun_xmodel;
    xmodel* hand_xmodel;
    const char** xanimations_right_handed;
    const char** xanimations_left_handed;
    const char* mode_name;
    unsigned short* note_track_sound_map_keys;
    unsigned short* note_track_sound_map_values;
    unsigned short* note_track_rumble_map_keys;
    unsigned short* note_track_rumble_map_values;
    int player_animation_type;
    weapon_type type_of_weapon;
    weapon_class class_of_weapon;
    penetration_type type_of_penetration;
    weapon_inventory_type inventory_type;
    weapon_fire_type fire_type;
    offhand_class class_of_offhand;
    weapon_stance stance;
    effects_effect_definition* view_flash_effect;
    effects_effect_definition* world_flash_effect;
    sound_alias_list* pickup_sound;
    sound_alias_list* pickup_sound_player;
    sound_alias_list* ammunition_pickup_sound;
    sound_alias_list* ammunition_pickup_sound_player;
    sound_alias_list* projectile_sound;
    sound_alias_list* pullback_sound;
    sound_alias_list* pullback_sound_player;
    sound_alias_list* fire_sound;
    sound_alias_list* fire_sound_player;
    sound_alias_list* fire_sound_player_akimbo;
    sound_alias_list* fire_loop_sound;
    sound_alias_list* fire_loop_sound_player;
    sound_alias_list* fire_stop_sound;
    sound_alias_list* fire_stop_sound_player;
    sound_alias_list* fire_last_sound;
    sound_alias_list* fire_last_sound_player;
    sound_alias_list* empty_fire_sound;
    sound_alias_list* empty_fire_sound_player;
    sound_alias_list* melee_swipe_sound;
    sound_alias_list* melee_swipe_sound_player;
    sound_alias_list* melee_hit_sound;
    sound_alias_list* melee_miss_sound;
    sound_alias_list* rechamber_sound;
    sound_alias_list* rechamber_sound_player;
    sound_alias_list* reload_sound;
    sound_alias_list* reload_sound_player;
    sound_alias_list* reload_empty_sound;
    sound_alias_list* reload_empty_sound_player;
    sound_alias_list* reload_start_sound;
    sound_alias_list* reload_start_sound_player;
    sound_alias_list* reload_end_sound;
    sound_alias_list* reload_end_sound_player;
    sound_alias_list* detonate_sound;
    sound_alias_list* detonate_sound_player;
    sound_alias_list* night_vision_wear_sound;
    sound_alias_list* night_vision_wear_sound_player;
    sound_alias_list* night_vision_remove_sound;
    sound_alias_list* night_vision_remove_sound_player;
    sound_alias_list* alternate_switch_sound;
    sound_alias_list* alternate_switch_sound_player;
    sound_alias_list* raise_sound;
    sound_alias_list* raise_sound_player;
    sound_alias_list* first_raise_sound;
    sound_alias_list* first_raise_sound_player;
    sound_alias_list* put_away_sound;
    sound_alias_list* put_away_sound_player;
    sound_alias_list* scan_sound;
    sound_alias_list** bounce_sound;
    effects_effect_definition* view_shell_eject_effect;
    effects_effect_definition* world_shell_eject_effect;
    effects_effect_definition* view_last_shot_eject_effect;
    effects_effect_definition* world_last_shot_eject_effect;
    material* reticle_center;
    material* reticle_side;
    int reticle_center_size;
    int reticle_side_size;
    int reticle_minimum_offset;
    active_reticle_type active_reticle;
    float stand_movement[3];
    float stand_rotation[3];
    float strafe_movement[3];
    float strafe_rotation[3];
    float ducked_offset[3];
    float ducked_movement[3];
    float ducked_rotation[3];
    float prone_offset[3];
    float prone_movement[3];
    float prone_rotation[3];
    float position_movement_rate;
    float position_prone_movement_rate;
    float stand_movement_minimum_speed;
    float ducked_movement_minimum_speed;
    float prone_movement_minimum_speed;
    float position_rotation_rate;
    float position_prone_rotation_rate;
    float stand_rotation_minimum_speed;
    float ducked_rotation_minimum_speed;
    float prone_rotation_minimum_speed;
    xmodel** world_model;
    xmodel* world_clip_model;
    xmodel* rocket_model;
    xmodel* knife_model;
    xmodel* world_knife_model;
    material* head_up_display_icon;
    weapon_icon_ratio_type head_up_display_icon_ratio;
    material* pickup_icon;
    weapon_icon_ratio_type pickup_icon_ratio;
    material* ammunition_counter_icon;
    weapon_icon_ratio_type ammunition_counter_icon_ratio;
    ammunition_counter_clip_type ammunition_counter_clip;
    int start_ammunition;
    const char* ammunition_name;
    int ammunition_index;
    const char* clip_name;
    int clip_index;
    int maximum_ammunition;
    int shot_count;
    const char* shared_ammunition_capacity_name;
    int shared_ammunition_capacity_index;
    int shared_ammunition_capacity;
    int damage;
    int player_damage;
    int melee_damage;
    int damage_type;
    int fire_delay;
    int melee_delay;
    int melee_charge_delay;
    int detonate_delay;
    int rechamber_time;
    int rechamber_time_one_handed;
    int rechamber_bolt_time;
    int hold_fire_time;
    int detonate_time;
    int melee_time;
    int melee_charge_time;
    int reload_time;
    int reload_show_rocket_time;
    int reload_empty_time;
    int reload_add_time;
    int reload_start_time;
    int reload_start_add_time;
    int reload_end_time;
    int drop_time;
    int raise_time;
    int alternate_drop_time;
    int quick_drop_time;
    int quick_raise_time;
    int breach_raise_time;
    int empty_raise_time;
    int empty_drop_time;
    int sprint_in_time;
    int sprint_loop_time;
    int sprint_out_time;
    int stunned_time_begin;
    int stunned_time_loop;
    int stunned_time_end;
    int night_vision_wear_time;
    int night_vision_wear_time_fade_out_end;
    int night_vision_wear_time_power_up;
    int night_vision_remove_time;
    int night_vision_remove_time_power_down;
    int night_vision_remove_time_fade_in_start;
    int fuse_time;
    int artificial_intelligence_fuse_time;
    float automatic_aim_range;
    float aim_assist_range;
    float aim_assist_range_aim_down_sight;
    float aim_padding;
    float enemy_crosshair_range;
    float movement_speed_scale;
    float aim_down_sight_movement_speed_scale;
    float sprint_duration_scale;
    float aim_down_sight_zoom_in_fraction;
    float aim_down_sight_zoom_out_fraction;
    material* overlay_material;
    material* overlay_material_low_resolution;
    material* overlay_material_electromagnetic_pulse;
    material* overlay_material_electromagnetic_pulse_low_resolution;
    weapon_overlay_reticle overlay_reticle;
    weapon_overlay_interface overlay_interface_type;
    float overlay_width;
    float overlay_height;
    float overlay_width_split_screen;
    float overlay_height_split_screen;
    float aim_down_sight_bob_factor;
    float aim_down_sight_view_bob_multiplier;
    float hip_spread_stand_minimum;
    float hip_spread_ducked_minimum;
    float hip_spread_prone_minimum;
    float hip_spread_stand_maximum;
    float hip_spread_ducked_maximum;
    float hip_spread_prone_maximum;
    float hip_spread_decay_rate;
    float hip_spread_fire_add;
    float hip_spread_turn_add;
    float hip_spread_movement_add;
    float hip_spread_ducked_decay;
    float hip_spread_prone_decay;
    float hip_reticle_side_position;
    float aim_down_sight_idle_amount;
    float hip_idle_amount;
    float aim_down_sight_idle_speed;
    float hip_idle_speed;
    float idle_crouch_factor;
    float idle_prone_factor;
    float gun_maximum_pitch;
    float gun_maximum_yaw;
    float sway_maximum_angle;
    float sway_linear_interpolation_speed;
    float sway_pitch_scale;
    float sway_yaw_scale;
    float sway_horizontal_scale;
    float sway_vertical_scale;
    float sway_shell_shock_scale;
    float aim_down_sight_sway_maximum_angle;
    float aim_down_sight_sway_linear_interpolation_speed;
    float aim_down_sight_sway_pitch_scale;
    float aim_down_sight_sway_yaw_scale;
    float aim_down_sight_sway_horizontal_scale;
    float aim_down_sight_sway_vertical_scale;
    float aim_down_sight_view_error_minimum;
    float aim_down_sight_view_error_maximum;
    physics_collision_map* physics_collision_map_pointer;
    float dual_wield_view_model_offset;
    weapon_icon_ratio_type kill_icon_ratio;
    int reload_ammunition_add;
    int reload_start_add;
    int ammunition_drop_stock_minimum;
    int ammunition_drop_clip_percentage_minimum;
    int ammunition_drop_clip_percentage_maximum;
    int explosion_radius;
    int explosion_radius_minimum;
    int explosion_inner_damage;
    int explosion_outer_damage;
    float damage_cone_angle;
    float bullet_explosion_damage_multiplier;
    float bullet_explosion_radius_multiplier;
    int projectile_speed;
    int projectile_speed_up;
    int projectile_speed_forward;
    int projectile_activate_distance;
    float projectile_lifetime;
    float time_to_accelerate;
    float projectile_curvature;
    xmodel* projectile_model;
    weapon_projectile_explosion projectile_explosion_type;
    effects_effect_definition* projectile_explosion_effect;
    effects_effect_definition* projectile_dud_effect;
    sound_alias_list* projectile_explosion_sound;
    sound_alias_list* projectile_dud_sound;
    weapon_stickiness_type stickiness;
    float low_ammunition_warning_threshold;
    float ricochet_chance;
    float* parallel_bounce;
    float* perpendicular_bounce;
    effects_effect_definition* projectile_trail_effect;
    effects_effect_definition* projectile_beacon_effect;
    float projectile_color[3];
    guided_missile_type type_of_guided_missile;
    float maximum_steering_acceleration;
    int projectile_ignition_delay;
    effects_effect_definition* projectile_ignition_effect;
    sound_alias_list* projectile_ignition_sound;
    float aim_down_sight_aim_pitch;
    float aim_down_sight_crosshair_in_fraction;
    float aim_down_sight_crosshair_out_fraction;
    int aim_down_sight_gun_kick_reduced_kick_bullets;
    float aim_down_sight_gun_kick_reduced_kick_percentage;
    float aim_down_sight_gun_kick_pitch_minimum;
    float aim_down_sight_gun_kick_pitch_maximum;
    float aim_down_sight_gun_kick_yaw_minimum;
    float aim_down_sight_gun_kick_yaw_maximum;
    float aim_down_sight_gun_kick_acceleration;
    float aim_down_sight_gun_kick_speed_maximum;
    float aim_down_sight_gun_kick_speed_decay;
    float aim_down_sight_gun_kick_static_decay;
    float aim_down_sight_view_kick_pitch_minimum;
    float aim_down_sight_view_kick_pitch_maximum;
    float aim_down_sight_view_kick_yaw_minimum;
    float aim_down_sight_view_kick_yaw_maximum;
    float aim_down_sight_view_scatter_minimum;
    float aim_down_sight_view_scatter_maximum;
    float aim_down_sight_spread;
    int hip_gun_kick_reduced_kick_bullets;
    float hip_gun_kick_reduced_kick_percentage;
    float hip_gun_kick_pitch_minimum;
    float hip_gun_kick_pitch_maximum;
    float hip_gun_kick_yaw_minimum;
    float hip_gun_kick_yaw_maximum;
    float hip_gun_kick_acceleration;
    float hip_gun_kick_speed_maximum;
    float hip_gun_kick_speed_decay;
    float hip_gun_kick_static_decay;
    float hip_view_kick_pitch_minimum;
    float hip_view_kick_pitch_maximum;
    float hip_view_kick_yaw_minimum;
    float hip_view_kick_yaw_maximum;
    float hip_view_scatter_minimum;
    float hip_view_scatter_maximum;
    float fight_distance;
    float maximum_distance;
    const char* accuracy_graph_name[2];
    float (*original_accuracy_graph_knots[2])[2];
    unsigned short original_accuracy_graph_knot_count[2];
    int position_reload_transition_time;
    float left_arc;
    float right_arc;
    float top_arc;
    float bottom_arc;
    float accuracy;
    float artificial_intelligence_spread;
    float player_spread;
    float minimum_turn_speed[2];
    float maximum_turn_speed[2];
    float pitch_convergence_time;
    float yaw_convergence_time;
    float suppress_time;
    float maximum_range;
    float animation_horizontal_rotation_increment;
    float player_position_distance;
    const char* use_hint_string;
    const char* drop_hint_string;
    int use_hint_string_index;
    int drop_hint_string_index;
    float horizontal_view_jitter;
    float vertical_view_jitter;
    float scan_speed;
    float scan_acceleration;
    int scan_pause_time;
    const char* script;
    float out_of_position_animation_length[2];
    int minimum_damage;
    int minimum_player_damage;
    float maximum_damage_range;
    float minimum_damage_range;
    float destabilization_rate_time;
    float destabilization_curvature_maximum;
    int destabilize_distance;
    float* location_damage_multipliers;
    const char* fire_rumble;
    const char* melee_impact_rumble;
    tracer_definition* tracer_type;
    float turret_scope_zoom_rate;
    float turret_scope_zoom_minimum;
    float turret_scope_zoom_maximum;
    float turret_overheat_up_rate;
    float turret_overheat_down_rate;
    float turret_overheat_penalty;
    sound_alias_list* turret_overheat_sound;
    effects_effect_definition* turret_overheat_effect;
    const char* turret_barrel_spin_rumble;
    float turret_barrel_spin_speed;
    float turret_barrel_spin_up_time;
    float turret_barrel_spin_down_time;
    sound_alias_list* turret_barrel_spin_maximum_sound;
    sound_alias_list* turret_barrel_spin_up_sound[4];
    sound_alias_list* turret_barrel_spin_down_sound[4];
    sound_alias_list* missile_cone_sound_alias;
    sound_alias_list* missile_cone_sound_alias_at_base;
    float missile_cone_sound_radius_at_top;
    float missile_cone_sound_radius_at_base;
    float missile_cone_sound_height;
    float missile_cone_sound_origin_offset;
    float missile_cone_sound_volume_scale_at_core;
    float missile_cone_sound_volume_scale_at_edge;
    float missile_cone_sound_volume_scale_core_size;
    float missile_cone_sound_pitch_at_top;
    float missile_cone_sound_pitch_at_bottom;
    float missile_cone_sound_pitch_top_size;
    float missile_cone_sound_pitch_bottom_size;
    float missile_cone_sound_crossfade_top_size;
    float missile_cone_sound_crossfade_bottom_size;
    bool shared_ammunition;
    bool lock_on_supported;
    bool require_lock_on_to_fire;
    bool big_explosion;
    bool no_aim_down_sight_when_magazine_empty;
    bool avoid_drop_cleanup;
    bool inherits_perks;
    bool crosshair_color_change;
    bool is_rifle_bullet;
    bool armor_piercing;
    bool is_bolt_action;
    bool aim_down_sight;
    bool rechamber_while_aim_down_sight;
    bool bullet_explosive_damage;
    bool cook_off_hold;
    bool is_clip_only;
    bool no_ammunition_pickup;
    bool aim_down_sight_fire_only;
    bool cancel_auto_holster_when_empty;
    bool disable_switch_to_when_empty;
    bool suppress_ammunition_reserve_display;
    bool laser_sight_during_night_vision;
    bool markable_view_model;
    bool no_dual_wield;
    bool flip_kill_icon;
    bool no_partial_reload;
    bool segmented_reload;
    bool blocks_prone;
    bool silenced;
    bool is_rolling_grenade;
    bool projectile_explosion_effect_force_normal_up;
    bool projectile_impact_explode;
    bool stick_to_players;
    bool has_detonator;
    bool disable_firing;
    bool timed_detonation;
    bool rotate;
    bool hold_button_to_throw;
    bool freeze_movement_when_firing;
    bool thermal_scope;
    bool alternate_mode_same_weapon;
    bool turret_barrel_spin_enabled;
    bool missile_cone_sound_enabled;
    bool missile_cone_sound_pitch_shift_enabled;
    bool missile_cone_sound_crossfade_enabled;
    bool offhand_hold_is_cancelable;
  };

  // 1281
  //
  struct weapon_complete_definition
  {
    const char* internal_name;
    weapon_definition* weapon_definition_pointer;
    const char* display_name;
    unsigned short* hide_tags;
    const char** xanimations;
    float aim_down_sight_zoom_field_of_view;
    int aim_down_sight_transition_in_time;
    int aim_down_sight_transition_out_time;
    int clip_size;
    impact_type type_of_impact;
    int fire_time;
    weapon_icon_ratio_type directional_pad_icon_ratio;
    float penetrate_multiplier;
    float aim_down_sight_view_kick_center_speed;
    float hip_view_kick_center_speed;
    const char* alternate_weapon_name;
    unsigned int alternate_weapon_index;
    int alternate_raise_time;
    material* kill_icon;
    material* directional_pad_icon;
    int fire_animation_length;
    int first_raise_time;
    int ammunition_drop_stock_maximum;
    float aim_down_sight_depth_of_field_start;
    float aim_down_sight_depth_of_field_end;
    unsigned short accuracy_graph_knot_count[2];
    float (*accuracy_graph_knots[2])[2];
    bool motion_tracker;
    bool enhanced;
    bool directional_pad_icon_shows_ammunition;
  };

  // 1282
  //
  struct sound_driver_globals
  {
    const char* name;
  };

  // 1283
  //
  struct raw_file
  {
    const char* name;
    int compressed_length;
    int length;
    const char* buffer;
  };

  // 1284
  //
  struct string_table_cell
  {
    const char* string;
    int hash;
  };

  // 1285
  //
  struct string_table
  {
    const char* name;
    int column_count;
    int row_count;
    string_table_cell* values;
  };

  // 1286
  //
  struct leaderboard_column_definition
  {
    const char* name;
    int identifier;
    int property_identifier;
    bool hidden;
    const char* statistic_name;
    leaderboard_column_type type;
    int precision;
    leaderboard_aggregation_type aggregation;
  };

  // 1287
  //
  struct leaderboard_definition
  {
    const char* name;
    int identifier;
    int column_count;
    int experience_column_identifier;
    int prestige_column_identifier;
    leaderboard_column_definition* columns;
  };

  // 1288
  //
  struct structured_data_enumeration_entry
  {
    const char* string;
    unsigned short index;
  };

  // 1289
  //
  struct structured_data_enumeration
  {
    int entry_count;
    int reserved_entry_count;
    structured_data_enumeration_entry* entries;
  };

  // 1290
  //
  union structured_data_type_union
  {
    unsigned int string_data_length;
    int enumeration_index;
    int structure_index;
    int indexed_array_index;
    int enumerated_array_index;
  };

  // 1291
  //
  struct structured_data_type
  {
    structured_data_type_category type;
    structured_data_type_union union_data;
  };

  // 1292
  //
  struct structured_data_structure_property
  {
    const char* name;
    structured_data_type type;
    unsigned int offset;
  };

  // 1293
  //
  struct structured_data_structure
  {
    int property_count;
    structured_data_structure_property* properties;
    int size;
    unsigned int bit_offset;
  };

  // 1294
  //
  struct structured_data_indexed_array
  {
    int array_size;
    structured_data_type element_type;
    unsigned int element_size;
  };

  // 1295
  //
  struct structured_data_enumerated_array
  {
    int enumeration_index;
    structured_data_type element_type;
    unsigned int element_size;
  };

  // 1296
  //
  struct structured_data_definition
  {
    int version;
    unsigned int format_checksum;
    int enumeration_count;
    structured_data_enumeration* enumerations;
    int structure_count;
    structured_data_structure* structures;
    int indexed_array_count;
    structured_data_indexed_array* indexed_arrays;
    int enumerated_array_count;
    structured_data_enumerated_array* enumerated_arrays;
    structured_data_type root_type;
    unsigned int size;
  };

  // 1297
  //
  struct structured_data_definition_set
  {
    const char* name;
    unsigned int definition_count;
    structured_data_definition* definitions;
  };

  // 1298
  //
  struct vehicle_physics_definition
  {
    int physics_enabled;
    const char* physics_preset_name;
    physics_preset* physics_preset_pointer;
    const char* acceleration_graph_name;
    vehicle_axle_type steering_axle;
    vehicle_axle_type power_axle;
    vehicle_axle_type braking_axle;
    float top_speed;
    float reverse_speed;
    float maximum_velocity;
    float maximum_pitch;
    float maximum_roll;
    float suspension_travel_front;
    float suspension_travel_rear;
    float suspension_strength_front;
    float suspension_damping_front;
    float suspension_strength_rear;
    float suspension_damping_rear;
    float friction_braking;
    float friction_coasting;
    float friction_top_speed;
    float friction_side;
    float friction_side_rear;
    float velocity_dependent_slip;
    float roll_stability;
    float roll_resistance;
    float pitch_resistance;
    float yaw_resistance;
    float upright_strength_pitch;
    float upright_strength_roll;
    float target_air_pitch;
    float air_yaw_torque;
    float air_pitch_torque;
    float minimum_momentum_for_collision;
    float collision_launch_force_scale;
    float wrecked_mass_scale;
    float wrecked_body_friction;
    float minimum_jolt_for_notify;
    float slip_threshold_front;
    float slip_threshold_rear;
    float slip_friction_scale_front;
    float slip_friction_scale_rear;
    float slip_friction_rate_front;
    float slip_friction_rate_rear;
    float slip_yaw_torque;
  };

  // 1299
  //
  struct vehicle_definition
  {
    const char* name;
    vehicle_type type;
    const char* use_hint_string;
    int health;
    int quad_barrel;
    float texture_scroll_scale;
    float top_speed;
    float acceleration;
    float rotation_rate;
    float rotation_acceleration;
    float maximum_body_pitch;
    float maximum_body_roll;
    float fake_body_acceleration_pitch;
    float fake_body_acceleration_roll;
    float fake_body_velocity_pitch;
    float fake_body_velocity_roll;
    float fake_body_side_velocity_pitch;
    float fake_body_pitch_strength;
    float fake_body_roll_strength;
    float fake_body_pitch_dampening;
    float fake_body_roll_dampening;
    float fake_body_boat_rocking_amplitude;
    float fake_body_boat_rocking_period;
    float fake_body_boat_rocking_rotation_period;
    float fake_body_boat_rocking_fade_out_speed;
    float boat_bouncing_minimum_force;
    float boat_bouncing_maximum_force;
    float boat_bouncing_rate;
    float boat_bouncing_fade_in_speed;
    float boat_bouncing_fade_out_steering_angle;
    float collision_damage;
    float collision_speed;
    float kill_camera_offset[3];
    int player_protected;
    int bullet_damage;
    int armor_piercing_damage;
    int grenade_damage;
    int projectile_damage;
    int projectile_splash_damage;
    int heavy_explosive_damage;
    vehicle_physics_definition vehicle_physics_definition_data;
    float boost_duration;
    float boost_recharge_time;
    float boost_acceleration;
    float suspension_travel;
    float maximum_steering_angle;
    float steering_linear_interpolation;
    float minimum_steering_scale;
    float minimum_steering_speed;
    int camera_look_enabled;
    float camera_linear_interpolation;
    float camera_pitch_influence;
    float camera_roll_influence;
    float camera_field_of_view_increase;
    float camera_field_of_view_offset;
    float camera_field_of_view_speed;
    const char* turret_weapon_name;
    weapon_complete_definition* turret_weapon;
    float turret_horizontal_span_left;
    float turret_horizontal_span_right;
    float turret_vertical_span_up;
    float turret_vertical_span_down;
    float turret_rotation_rate;
    sound_alias_list* turret_spin_sound;
    sound_alias_list* turret_stop_sound;
    int trophy_enabled;
    float trophy_radius;
    float trophy_inactive_radius;
    int trophy_ammunition_count;
    float trophy_reload_time;
    unsigned short trophy_tags[4];
    material* compass_friendly_icon;
    material* compass_enemy_icon;
    int compass_icon_width;
    int compass_icon_height;
    sound_alias_list* idle_low_sound;
    sound_alias_list* idle_high_sound;
    sound_alias_list* engine_low_sound;
    sound_alias_list* engine_high_sound;
    float engine_sound_speed;
    sound_alias_list* engine_start_up_sound;
    int engine_start_up_length;
    sound_alias_list* engine_shutdown_sound;
    sound_alias_list* engine_idle_sound;
    sound_alias_list* engine_sustain_sound;
    sound_alias_list* engine_ramp_up_sound;
    int engine_ramp_up_length;
    sound_alias_list* engine_ramp_down_sound;
    int engine_ramp_down_length;
    sound_alias_list* suspension_soft_sound;
    float suspension_soft_compression;
    sound_alias_list* suspension_hard_sound;
    float suspension_hard_compression;
    sound_alias_list* collision_sound;
    float collision_blend_speed;
    sound_alias_list* speed_sound;
    float speed_sound_blend_speed;
    const char* surface_sound_prefix;
    sound_alias_list* surface_sounds[31];
    float surface_sound_blend_speed;
    float slide_volume;
    float slide_blend_speed;
    float in_air_pitch;
  };

  // 1300
  //
  struct addon_map_entities
  {
    const char* name;
    char* entity_string;
    int number_of_entity_characters;
    map_triggers trigger;
  };

  // 1301
  //
  union xasset_header
  {
    physics_preset* physics_preset_pointer;
    physics_collision_map* physics_collision_map_pointer;
    xanimation_parts* parts;
    xmodel_surfaces* model_surfaces;
    xmodel* model;
    material* material_pointer;
    material_pixel_shader* pixel_shader;
    material_vertex_shader* vertex_shader;
    material_vertex_declaration* vertex_declaration;
    material_technique_set* technique_set;
    graphics_image* image;
    sound_alias_list* sound;
    sound_curve* sound_curve_pointer;
    loaded_sound* loaded_sound_pointer;
    collision_clip_map* clip_map;
    common_world* common_world_pointer;
    game_world_single_player* game_world_single_player_pointer;
    game_world_multi_player* game_world_multi_player_pointer;
    map_entities* map_entities_pointer;
    effects_world* effects_world_pointer;
    graphics_world* graphics_world_pointer;
    graphics_light_definition* light_definition;
    font* font_pointer;
    menu_list* menu_list_pointer;
    menu_definition* menu;
    localize_entry* localize;
    weapon_complete_definition* weapon;
    sound_driver_globals* sound_driver_globals_pointer;
    effects_effect_definition* effects;
    effects_impact_table* impact_effects;
    raw_file* raw_file_pointer;
    string_table* string_table_pointer;
    leaderboard_definition* leaderboard_definition_pointer;
    structured_data_definition_set* structured_data_definition_set_pointer;
    tracer_definition* tracer_definition_pointer;
    vehicle_definition* vehicle_definition_pointer;
    addon_map_entities* addon_map_entities_pointer;
    void* data;
  };

  // 1315
  //
  struct network_address
  {
    network_address_type type;
    char ip[4];
    unsigned short port;
    char ipx[10];
  };

  // 1362
  //
  struct xnetwork_key_identifier
  {
    char bytes[8];
  };

  // 1363
  //
  struct xnetwork_key
  {
    char bytes[16];
  };

  // 1364
  //
  struct xsession_information
  {
    xnetwork_key_identifier session_identifier;
    xnetwork_address host_address;
    xnetwork_key key_exchange_key;
  };

  // 1474
  //
  struct message
  {
    int overflowed;
    int read_only;
    char* data;
    char* split_data;
    int maximum_size;
    int current_size;
    int split_size;
    int read_count;
    int bit;
    int last_entity_reference;
  };

  // 2096
  //
  struct script_string_list
  {
    int count;
    const char** strings;
  };

  // 2686
  //
  struct xasset
  {
    xasset_type type;
    xasset_header header;
  };

  // 2687
  //
  struct xasset_list
  {
    script_string_list string_list;
    int asset_count;
    xasset* assets;
  };

  // 2795
  //
  struct xasset_entry
  {
    xasset asset;
    char zone_index;
    volatile char in_use_mask;
    bool printed_missing_asset;
    unsigned short next_hash;
    unsigned short next_override;
  };

  // 2796
  //
  union xasset_entry_pool_entry
  {
    xasset_entry entry;
    xasset_entry_pool_entry* next;
  };

  // Internal symbols
  //
  using  Com_Frame_Try_Block_Function_t = int64_t (*) ();
  inline Com_Frame_Try_Block_Function_t Com_Frame_Try_Block_Function = reinterpret_cast<Com_Frame_Try_Block_Function_t> (0x1401F9930);

  using  CL_LocalClientNumFromControllerIndex_t = __int64 (*) (void);
  inline CL_LocalClientNumFromControllerIndex_t CL_LocalClientNumFromControllerIndex = reinterpret_cast<CL_LocalClientNumFromControllerIndex_t> (0x1400EF8F0);

  using  DB_FindXAssetHeader_t = xasset_header (*) (xasset_type, const char* name);
  inline DB_FindXAssetHeader_t DB_FindXAssetHeader = reinterpret_cast<DB_FindXAssetHeader_t> (0x140129220);

  using  LiveStorage_IsWaitingOnPlaylists_t = bool (*) ();
  inline LiveStorage_IsWaitingOnPlaylists_t LiveStorage_IsWaitingOnPlaylists = reinterpret_cast<LiveStorage_IsWaitingOnPlaylists_t> (0x1402AA9B0);

  using  LiveStorage_FetchPlaylists_t = void (*) (int controller_index);
  inline LiveStorage_FetchPlaylists_t LiveStorage_FetchPlaylists = reinterpret_cast<LiveStorage_FetchPlaylists_t> (0x1402AA840);

  using  Playlist_ParsePlaylists_t = int64_t (*) (uint8_t*);
  inline Playlist_ParsePlaylists_t Playlist_ParsePlaylists = reinterpret_cast<Playlist_ParsePlaylists_t> (0x14025D460);

  using  Playlist_ValidatePlaylistNum_t = void (*) (void);
  inline Playlist_ValidatePlaylistNum_t Playlist_ValidatePlaylistNum = reinterpret_cast<Playlist_ValidatePlaylistNum_t> (0x14025E250);

  using  Live_SetPlaylistVersion_t = int64_t (*) (void);
  inline Live_SetPlaylistVersion_t Live_SetPlaylistVersion = reinterpret_cast<Live_SetPlaylistVersion_t> (0x1402A7300);

  using  LiveStorage_IsWaitingOnStats_t = char (*) (__int64);
  inline LiveStorage_IsWaitingOnStats_t LiveStorage_IsWaitingOnStats = reinterpret_cast<LiveStorage_IsWaitingOnStats_t> (0x1401FE130);

  using  Cbuf_ExecuteBuffer_t = void (*) (unsigned int, unsigned int, uint8_t *);
  inline Cbuf_ExecuteBuffer_t Cbuf_ExecuteBuffer = reinterpret_cast<Cbuf_ExecuteBuffer_t> (0x1401EC780);

  using  Com_Error_t = void (*) (int, const char *, ...);
  inline Com_Error_t Com_Error = reinterpret_cast<Com_Error_t> (0x1401F8FD0);

  using  Com_ErrorEntered_t = bool (*) (void);
  inline Com_ErrorEntered_t Com_ErrorEntered = reinterpret_cast<Com_ErrorEntered_t> (0x1401F9620);

  using  Live_ThrowError_t = void (*) (int, const char *);
  inline Live_ThrowError_t Live_ThrowError = reinterpret_cast<Live_ThrowError_t> (0x1402A7600);

  using  bdLogMessage_t = void (*) (int, const char*, const char*, const char*, const char*, int, const char*, ...);
  inline bdLogMessage_t bdLogMessage = reinterpret_cast<bdLogMessage_t> (0x140331F60);

  using  IWNet_Frame_t = void (*) (int);
  inline IWNet_Frame_t IWNet_Frame (reinterpret_cast<IWNet_Frame_t> (0x14012df70));
}
