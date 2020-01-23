/* File: variable.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Angband variables */

#include "angband.h"

bool initialized = FALSE;

int game_mode = GAME_MODE_NORMAL;

/*
 * Hack -- Link a copyright message into the executable
 */
cptr copyright[5] =
{
    "Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
    "",
    "This software may be copied and distributed for educational, research,",
    "and not for profit purposes provided that this copyright and statement",
    "are included in all such copies."
};


int max_macrotrigger = 0;
cptr macro_template = NULL;
cptr macro_modifier_chr;
cptr macro_modifier_name[MAX_MACRO_MOD];
cptr macro_trigger_name[MAX_MACRO_TRIG];
cptr macro_trigger_keycode[2][MAX_MACRO_TRIG];

int level_up = 0;

/*
 *  List for auto-picker/destroyer entries
 */
int max_autopick = 0;
int max_max_autopick = 0;
autopick_type *autopick_list = NULL;

/*
 * Savefile version
 */
byte h_ver_major;       /* Savefile version for Hengband 1.1.1 and later */
byte h_ver_minor;
byte h_ver_patch;
byte h_ver_extra;

byte sf_extra;        /* Savefile's encoding key */

byte z_major;           /* Savefile version for Hengband */
byte z_minor;
char z_patch[16];

/*
 * Savefile information
 */
u32b sf_system;            /* Operating system info */
u32b sf_when;            /* Time when savefile created */
u16b sf_lives;            /* Number of past "lives" with this file */
u16b sf_saves;            /* Number of "saves" during this life */

/*
 * Run-time arguments
 */
bool arg_fiddle;            /* Command arg -- Request fiddle mode */
bool arg_wizard;            /* Command arg -- Request wizard mode */
bool arg_sound;                /* Command arg -- Request special sounds */
byte arg_graphics;            /* Command arg -- Request graphics mode */
bool arg_monochrome;        /* Command arg -- Request monochrome mode */
bool arg_force_original;    /* Command arg -- Request original keyset */
bool arg_force_roguelike;    /* Command arg -- Request roguelike keyset */
bool arg_bigtile = FALSE;    /* Command arg -- Request big tile mode */

/*
 * Various things
 */
bool character_generated;    /* The character exists */
bool character_dungeon;        /* The character has a dungeon */
bool character_loaded;        /* The character was loaded from a savefile */
bool character_saved;        /* The character was just saved to a savefile */

bool character_icky;        /* The game is in an icky full screen mode */
bool character_xtra;        /* The game is in an icky startup mode */

bool creating_savefile;        /* New savefile is currently created */

u32b seed_flavor;        /* Hack -- consistent object colors */
u32b seed_town;            /* Hack -- consistent town layout */
u32b chaotic_py_seed;    /* Seed for chaotic personality's random stats */

s16b command_cmd;        /* Current "Angband Command" */

s16b command_arg;        /* Gives argument of current command */
s16b command_rep;        /* Gives repetition of current command */
s16b command_dir;        /* Gives direction of current command */

s16b command_see;        /* See "object1.c" */
s16b command_wrk;        /* See "object1.c" */

s16b command_gap = 999;         /* See "object1.c" */

s16b command_new;        /* Command chaining from inven/equip view */

s16b energy_use;        /* Energy use this turn */

s16b running;            /* Current counter for running, if any */
s16b resting;            /* Current counter for resting, if any */

s16b cur_hgt;            /* Current dungeon height */
s16b cur_wid;            /* Current dungeon width */
s16b dun_level;            /* Current dungeon level */
byte room_object_origin = ORIGIN_SPECIAL;
s16b unique_count;
s16b num_repro;            /* Current reproducer count */
s16b num_repro_kill;
s16b object_level;        /* Current object creation level */
s16b monster_level;        /* Current monster creation level */
s16b base_level;        /* Base dungeon level */

s32b game_turn;                /* Current game turn */
s32b game_turn_limit;        /* Limit of game turn */
s32b dungeon_turn;            /* Game turn in dungeon */
s32b dungeon_turn_limit;    /* Limit of game turn in dungeon */
s32b old_turn;            /* Turn when level began */
s32b old_battle;
s32b player_turn;

bool use_sound;            /* The "sound" mode is enabled */
bool use_graphics;        /* The "graphics" mode is enabled */
bool use_bigtile = FALSE;

s16b signal_count;        /* Hack -- Count interupts */

bool inkey_base;        /* See the "inkey()" function */
bool inkey_xtra;        /* See the "inkey()" function */
bool inkey_scan;        /* See the "inkey()" function */
bool inkey_flag;        /* See the "inkey()" function */
bool get_com_no_macros = FALSE;    /* Expand macros in "get_com" or not */

s16b coin_type;            /* Hack -- force coin type */

bool opening_chest;        /* Hack -- prevent chest generation */

bool shimmer_monsters;    /* Hack -- optimize multi-hued monsters */
bool shimmer_objects;    /* Hack -- optimize multi-hued objects */

bool repair_monsters;    /* Hack -- optimize detect monsters */
bool repair_objects;    /* Hack -- optimize detect objects */

bool hack_mind;

s16b o_max = 1;            /* Number of allocated objects */
s16b o_cnt = 0;            /* Number of live objects */

s16b m_max = 1;            /* Number of allocated monsters */
s16b m_cnt = 0;            /* Number of live monsters */

s16b hack_m_idx = 0;    /* Hack -- see "process_monsters()" */
s16b hack_m_idx_ii = 0;
int  hack_max_m_dam = 0;
char summon_kin_type;   /* Hack, by Julian Lighton: summon 'relatives' */
s16b warning_hack_hp = 0;
s16b shuffling_hack_hp = 0;
byte poison_warning_hack = 0;
bool spawn_hack = FALSE;

int total_friends = 0;
s32b friend_align = 0;

bool reinit_wilderness = FALSE;
bool quest_reward_drop_hack = FALSE;
bool very_nice_summon_hack = FALSE;
bool predictable_energy_hack = FALSE;
byte delay_autopick_hack = 0;
bool obj_list_autopick_hack = FALSE;
bool monsters_damaged_hack = FALSE;
bool shop_exit_hack = FALSE;
bool no_karrot_hack = FALSE;
bool allow_special3_hack = FALSE;
byte troika_spell_hack = 0;
s16b vampirism_hack = 1000;
byte spell_problem = 0;
s16b run_count = 0;
byte autopick_inkey_hack = 0;

int current_flow_depth = 0;

/*
 * Software options (set via the '=' command). See "tables.c"
 */

/*** Input Options ***/

bool rogue_like_commands;    /* Rogue-like commands */
bool always_pickup;    /* Pick things up by default */
bool online_macros;    /* Disable Run in macros */
bool quick_messages;    /* Activate quick messages */
bool command_menu;    /* Enable command selection menu */
bool use_old_target;    /* Use old target by default */
bool auto_target;    /* Automatically target nearest monster */
bool always_repeat;    /* Repeat obvious commands */
bool confirm_destroy;    /* Prompt for destruction of known worthless items */
bool confirm_wear;    /* Confirm to wear/wield known cursed items */
bool prompt_on_failure; /* Display prompt on device/spell fail */
bool target_pet;    /* Allow targetting pets */

#ifdef ALLOW_EASY_OPEN
bool easy_open;    /* Automatically open doors */
#endif

#ifdef ALLOW_EASY_DISARM
bool easy_disarm;    /* Automatically disarm traps */
#endif

bool auto_get_ammo;
bool auto_get_objects;
bool auto_detect_traps;
bool auto_map_area;

bool numpad_as_cursorkey;    /* Use numpad keys as cursor key in editor mode */
bool use_pack_slots;


/*** Map Screen Options ***/

bool center_player;    /* Center map while walking (*slow*) */
bool center_running;    /* Centering even while running */
bool view_yellow_lite;    /* Use special colors for torch-lit grids */
bool view_bright_lite;    /* Use special colors for 'viewable' grids */
bool view_granite_lite;    /* Use special colors for wall grids (slow) */
bool view_special_lite;    /* Use special colors for floor grids (slow) */
bool view_perma_grids;    /* Map remembers all perma-lit grids */
bool view_torch_grids;    /* Map remembers all torch-lit grids */
bool view_unsafe_grids;    /* Map marked by detect traps */
bool fresh_before;    /* Flush output while continuous command */
bool fresh_after;    /* Flush output after monster's move */
bool fresh_message;    /* Flush output after every message */
bool hilite_player;    /* Hilite the player with the cursor */
bool display_path;    /* Display actual path before shooting */
bool square_delays;   /* Use delay factors based on squares */


/*** Text Display Options ***/

bool plain_descriptions;    /* Plain object descriptions */
bool always_show_list;    /* Always show list when choosing items */
bool depth_in_feet;    /* Show dungeon level in feet */
bool effective_speed;  /* Use Ighalli's speed display */
bool show_labels;    /* Show labels in object listings */
bool show_weights;    /* Show weights in object listings */
bool show_discounts;
bool show_item_graph;    /* Show items graphics */
bool equippy_chars;    /* Display 'equippy' chars */
bool display_food_bar;
bool display_hp_bar;
bool display_sp_bar;
bool compress_savefile;    /* Compress messages in savefiles */
bool abbrev_extra;    /* Describe obj's extra resistances by abbreviation */
bool abbrev_all;    /* Describe obj's all resistances by abbreviation */
bool exp_need;    /* Show the experience needed for next level */
bool ignore_unview;    /* Ignore whenever any monster does */
bool display_distance;
bool display_race; /* Display monster races with their racial char */
bool easy_mimics;  /* Use the 'x' glyph for monsters that look like things */
bool show_origins; /* Show origin in object description */
bool show_discovery; /* Show time of discovery in object description */
bool final_dump_origins; /* Show equipment origins in final dumps */
bool always_dump_origins; /* Show equipment origins in all dumps */
bool list_stairs;  /* Display stairs in object list */
bool display_skill_num; /* Give skills numerically in char sheet */
bool reforge_details; /* Show statistics before proceeding with reforge */
bool auto_sticky_labels; /* Automatically make power labels sticky */
bool show_power; /* Display device powers in inventory */
bool show_rogue_keys; /* Display roguelike keys if possible */
bool decimal_stats; /* Decimalized stat display */

/*** Game-Play Options ***/

bool stack_force_notes;    /* Merge inscriptions when stacking */
bool stack_force_costs;    /* Merge discounts when stacking */
bool expand_list;    /* Expand the power of the list commands */
bool empty_levels;    /* Allow empty 'arena' levels */
bool bound_walls_perm;    /* Boundary walls become 'permanent wall' */
bool delay_autopick;  /* Always use delayed autopick */
bool last_words;    /* Leave last words when your character dies */

#ifdef WORLD_SCORE
bool send_score;    /* Send score dump to the world score server */
#endif

bool allow_debug_opts;    /* Allow use of debug/cheat options */


/*** Disturbance Options ***/

bool find_ignore_stairs;    /* Run past stairs */
bool find_ignore_doors;    /* Run through open doors */
bool find_ignore_veins;  /* Run past gold veins */
bool find_cut;    /* Run past known corners */
bool travel_ignore_items;  /* Ignore identified items while travelling */
bool check_abort;    /* Check for user abort while continuous command */
bool flush_failure;    /* Flush input on various failures */
bool flush_disturb;    /* Flush input whenever disturbed */
bool disturb_move;    /* Disturb whenever any monster moves */
bool disturb_high;    /* Disturb whenever high-level monster moves */
bool disturb_near;    /* Disturb whenever viewable monster moves */
bool disturb_pets;    /* Disturb when visible pets move */
bool disturb_panel;    /* Disturb whenever map panel changes */
bool disturb_state;    /* Disturb whenever player state changes */
bool disturb_minor;    /* Disturb whenever boring things happen */
bool town_no_disturb;
bool ring_bell;    /* Audible bell (on errors, etc) */
bool disturb_trap_detect;    /* Disturb when leaving trap detected area */
bool alert_trap_detect;    /* Alert when leaving trap detected area */
bool alert_device_gone;  /* Alert when device is destroyed or stolen */
bool alert_insc_gone;    /* Alert when inscribed item is destroyed or stolen */
bool alert_wanted_kill;  /* Alert when killing a wanted monster */
bool alert_poison;   /* Alert on high poisoning */

/*** Birth Options ***/

byte coffee_break;   /* Coffee-break mode */
bool easy_id;        /* Easy Identify */
bool easy_lore;      /* Easy Monster Lore */
bool empty_lore;     /* Always start with empty item lore */
bool easy_damage;    /* Access wizard info about damage and monster health */
bool allow_spoilers;
bool power_tele;     /* Use old-style, non-fuzzy telepathy */
bool smart_learn;    /* Monsters learn from their mistakes (*) */
bool smart_cheat;    /* Monsters exploit players weaknesses (*) */
bool no_wilderness;
bool ironman_shops;    /* Stores are permanently closed (*) */
bool ironman_downward;    /* Disable recall and use of up stairs (*) */
bool ironman_empty_levels;    /* Always create empty 'arena' levels (*) */
bool ironman_nightmare;    /* Nightmare mode(it isn't even remotely fair!)(*) */
bool preserve_mode;    /* Preserve artifacts (*) */
bool allow_friendly_monster; /* Allow monsters friendly to player */
bool allow_hostile_monster; /* Allow monsters hostile to each other */
bool allow_pets; /* Allow pets: Note, this makes some classes unplayable. */
bool quest_unique; /* Random quests for unique monsters only */
bool random_artifacts;
byte random_artifact_pct = 100;
bool single_pantheon; /* Only use one pantheon */
byte game_pantheon = 0;
bool no_artifacts;
bool no_egos;
bool no_selling;
bool enable_virtues;
bool easy_thalos;
bool never_forget;
bool no_chris;
bool no_scrambling;
bool comp_mode;
bool reduce_uniques;
byte reduce_uniques_pct = 100; /* This is the pct of uniques to face */
bool always_small_levels;
byte small_level_type = 0;
bool increase_density;
bool even_proportions;
bool no_big_dungeons;
bool thrall_mode;
bool wacky_rooms;
bool melee_challenge;
bool no_melee_challenge;
bool no_wanted_points;

/*** Easy Object Auto-Destroyer ***/

bool destroy_items;    /* Use easy auto-destroyer */
bool destroy_debug;
bool destroy_feeling;    /* Apply auto-destroy as sense feeling */
bool destroy_identify;    /* Apply auto-destroy as identify an item */
bool leave_worth;    /* Auto-destroyer leaves known worthy items */
bool leave_equip;    /* Auto-destroyer leaves weapons and armour */
bool leave_chest;    /* Auto-destroyer leaves closed chests */
bool leave_wanted;    /* Auto-destroyer leaves wanted corpses */
bool leave_corpse;    /* Auto-destroyer leaves corpses and skeletons */
bool leave_junk;    /* Auto-destroyer leaves junk */
bool leave_special;    /* Auto-destroyer leaves items your race/class needs */
bool leave_mogaminator; /* Mogaminator reads "destroy" as "leave" */
bool no_mogaminator;   /* Deactivate the Mogaminator */


/* Cheating options */

bool cheat_peek;    /* Peek into object creation */
bool cheat_hear;    /* Peek into monster creation */
bool cheat_room;    /* Peek into dungeon creation */
bool cheat_xtra;    /* Peek into something else */
bool cheat_live;    /* Allow player to avoid death */
bool cheat_save;    /* Ask for saving death */


/* Special options */

byte hitpoint_warn = 5;    /* Hitpoint warning (0 to 9) */
byte mana_warn;    /* Mana color (0 to 9) */

byte delay_factor = 2;    /* Delay factor (0 to 9) */

bool autosave_l;    /* Autosave before entering new levels */
bool autosave_t;    /* Timed autosave */
s16b autosave_freq;     /* Autosave frequency */


/*
 * Dungeon variables
 */

bool closing_flag;        /* Dungeon is closing */


/*
 * Dungeon size info
 */
point_t viewport_origin;

/*
 * Player location in dungeon
 */
int py;
int px;

/*
 * Targetting variables
 */
s16b target_who;
s16b target_col;
s16b target_row;


/*
 * User info
 */
int player_uid;
int player_euid;
int player_egid;

/*
 * Current player's character name
 */
char player_name[32];

/*
 * Stripped version of "player_name"
 */
char player_base[32];

/*
 * Default pref file save name (except for the Mogaminator)
 */
char pref_save_base[32] = "";

/*
 * Buffer to hold the current savefile name
 * 'savefile' holds full path name. 'savefile_base' holds only base name.
 */
char savefile[1024];
char savefile_base[40];

/*
 * Array[RANDNAME_NUM_TYPES][num_names] of random names
 * Lifted from Vanilla 3.5.1
 */
const char ***name_sections;

/*
 * Array of grids lit by player lite (see "cave.c")
 */
s16b lite_n;
s16b lite_y[LITE_MAX];
s16b lite_x[LITE_MAX];

/*
 * Array of grids lit by player lite (see "cave.c")
 */
s16b mon_lite_n;
s16b mon_lite_y[MON_LITE_MAX];
s16b mon_lite_x[MON_LITE_MAX];

/*
 * Array of grids viewable to the player (see "cave.c")
 */
s16b view_n;
s16b view_y[VIEW_MAX];
s16b view_x[VIEW_MAX];

/*
 * Array of grids for use by various functions (see "cave.c")
 */
s16b temp_n;
s16b temp_y[TEMP_MAX];
s16b temp_x[TEMP_MAX];

/*
 * Array of grids for delayed visual updating (see "cave.c")
 */
s16b redraw_n = 0;
s16b redraw_y[REDRAW_MAX];
s16b redraw_x[REDRAW_MAX];


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
cptr *macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
cptr *macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool *macro__cmd;

/*
 * Current macro action [1024]
 */
char *macro__buf;


/*
 * The number of quarks
 */
s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
cptr *quark__str;

/*
 * The array of normal options
 */
u32b option_flag[8];
u32b option_mask[8];


/*
 * The array of window options
 */
u32b window_flag[8];
u32b window_mask[8];


/*
 * The array of window pointers
 */
term *angband_term[8];


/*
 * Standard window names
 */
char angband_term_name[8][16] =
{
    "Main",
    "Term-1",
    "Term-2",
    "Term-3",
    "Term-4",
    "Term-5",
    "Term-6",
    "Term-7"
};


/*
 * Global table of color definitions
 */
byte angband_color_table[256][4] =
{
    {0x00, 0x00, 0x00, 0x00},    /* TERM_DARK */
    {0x00, 0xFF, 0xFF, 0xFF},    /* TERM_WHITE */
    {0x00, 0x80, 0x80, 0x80},    /* TERM_SLATE */
    {0x00, 0xFF, 0x80, 0x00},    /* TERM_ORANGE */
    {0x00, 0xC0, 0x00, 0x00},    /* TERM_RED */
    {0x00, 0x00, 0x80, 0x40},    /* TERM_GREEN */
    {0x00, 0x00, 0x00, 0xFF},    /* TERM_BLUE */
    {0x00, 0x80, 0x40, 0x00},    /* TERM_UMBER */
    {0x00, 0x40, 0x40, 0x40},    /* TERM_L_DARK */
    {0x00, 0xC0, 0xC0, 0xC0},    /* TERM_L_WHITE */
    {0x00, 0xFF, 0x00, 0xFF},    /* TERM_VIOLET */
    {0x00, 0xFF, 0xFF, 0x00},    /* TERM_YELLOW */
    {0x00, 0xFF, 0x00, 0x00},    /* TERM_L_RED */
    {0x00, 0x00, 0xFF, 0x00},    /* TERM_L_GREEN */
    {0x00, 0x00, 0xFF, 0xFF},    /* TERM_L_BLUE */
    {0x00, 0xC0, 0x80, 0x40}    /* TERM_L_UMBER */
};


/*
 * Standard sound names
 */
char angband_sound_name[SOUND_MAX][16] =
{
    "",
    "hit",
    "miss",
    "flee",
    "drop",
    "kill",
    "level",
    "death",
    "study",
    "teleport",
    "shoot",
    "quaff",
    "zap",
    "walk",
    "tpother",
    "hitwall",
    "eat",
    "store1",
    "store2",
    "store3",
    "store4",
    "dig",
    "opendoor",
    "shutdoor",
    "tplevel",
    "scroll",
    "buy",
    "sell",
    "warn",
    "rocket",
    "n_kill",
    "u_kill",
    "quest",
    "heal",
    "x_heal",
    "bite",
    "claw",
    "m_spell",
    "summon",
    "breath",
    "ball",
    "m_heal",
    "atkspell",
    "evil",
    "touch",
    "sting",
    "crush",
    "slime",
    "wail",
    "winner",
    "fire",
    "acid",
    "elec",
    "cold",
    "illegal",
    "fail",
    "wakeup",
    "invuln",
    "fall",
    "pain",
    "destitem",
    "moan",
    "show",
    "unused",
    "explode",
    "glass",
};


/*
 * The array of "cave grids" [MAX_WID][MAX_HGT].
 * Not completely allocated, that would be inefficient
 * Not completely hardcoded, that would overflow memory
 */
cave_type *cave[MAX_HGT];


/*
 * The array of saved floors
 */
saved_floor_type saved_floors[MAX_SAVED_FLOORS];


/*
 * Number of floor_id used from birth
 */
s16b max_floor_id;


/*
 * Sign for current process used in temporal files.
 * Actually it is the start time of current process.
 */
u32b saved_floor_file_sign;


/*
 * The array of dungeon items [max_o_idx]
 */
object_type *o_list;

/*
 * The array of dungeon monsters [max_m_idx]
 */
monster_type *m_list;

pack_info_t *pack_info_list;
s16b max_pack_info_idx;
s16b pack_info_free_list;
s16b pack_info_count;


/*
 * The size of "alloc_kind_table" (at most max_k_idx * 4)
 */
s16b alloc_kind_size;

/*
 * The entries in the "kind allocator table"
 */
alloc_entry *alloc_kind_table;


/*
 * The size of "alloc_race_table" (at most max_r_idx)
 */
s16b alloc_race_size;

/*
 * The entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;


/*
 * Specify attr/char pairs for visual special effects
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte misc_to_attr[256];
char misc_to_char[256];


/*
 * Specify attr/char pairs for inventory items (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];
char tval_to_char[128];


/*
 * Keymaps for each "mode" associated with each keypress.
 */
cptr keymap_act[KEYMAP_MODES][256];



/*** Player information ***/

/*
 * Static player info record
 */
player_type p_body;

/*
 * Pointer to the player info
 */
player_type *p_ptr = &p_body;

/*
 * Pointer to the player tables
 * (sex, race, class, magic)
 */
player_magic *mp_ptr;


/*
 * The last character rolled,
 * holded for quick start
 */
birther previous_char;


/*
 * Room Templates (Vaults, Special Rooms, Wilderness Encounters)
 */
vec_ptr     room_info = NULL;
int_map_ptr room_letters = NULL;

/*
 * The magic info
 */
player_magic *m_info;

/*
 * The terrain feature arrays
 */
feature_type *f_info;
char *f_name;
char *f_tag;

/*
 * The object kind arrays
 */
object_kind *k_info;
char *k_name;
char *k_text;

/*
 * The artifact arrays
 */
artifact_type *a_info;
char *a_name;
char *a_text;

/*
 * The ego-item arrays
 */
ego_type *e_info;
char *e_name;
char *e_text;

/*
 * The monster race arrays
 */
monster_race *r_info;
char *r_name;
char *r_text;

/* Body Types for Player Monster Races */
equip_template_ptr b_info;
char *b_name;
char *b_tag;

/*
 * The dungeon arrays
 */
dungeon_info_type *d_info;
char *d_name;
char *d_text;

/*
 * Power labels
 */
char power_labels[MAX_POWER_LABEL][15];

/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Hack -- The special Angband "Keyboard Suffix"
 * This variable is used to choose an appropriate macro-trigger definition
 */
cptr ANGBAND_KEYBOARD = "0";

/*
 * Hack -- The special Angband "Graphics Suffix"
 * This variable is used to choose an appropriate "graf-xxx" file
 */
cptr ANGBAND_GRAF = "ascii";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
cptr ANGBAND_DIR;

/*
 * High score files (binary)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_APEX;

/*
 * Bone files for player ghosts (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_BONE;

/*
 * Binary image files for the "*_info" arrays (binary)
 * These files are not portable between platforms
 */
cptr ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_EDIT;

/*
 * Script files
 * These files are portable between platforms.
 */
cptr ANGBAND_DIR_SCRIPT;

/*
 * Various extra files (ascii)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_FILE;

/*
 * Help files (normal) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_HELP;

/*
 * Help files (spoilers) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_INFO;

/*
 * Default user "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_PREF;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_SAVE;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_USER;

/*
 * Various extra files (binary)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_XTRA;


/*
 * Current "comp" function for ang_sort()
 */
bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);


/*
 * Current "swap" function for ang_sort()
 */
void (*ang_sort_swap)(vptr u, vptr v, int a, int b);



/*
 * Hack -- function hooks to restrict "get_mon_num_prep()" function
 */
monster_hook_type get_mon_num_hook;
monster_hook_type get_mon_num2_hook;


/*
 * Hack -- function hook to restrict "get_obj_num_prep()" function
 */
bool (*get_obj_num_hook)(int k_idx);
int obj_drop_theme;


/* Hack, monk armour */
bool monk_armour_aux;
bool monk_notify_aux;

#ifdef ALLOW_EASY_OPEN /* TNB */
bool easy_open;
#endif /* ALLOW_EASY_OPEN -- TNB */

#ifdef ALLOW_EASY_DISARM /* TNB */
bool easy_disarm;
#endif /* ALLOW_EASY_DISARM -- TNB */

bool center_player;
bool center_running;

/* Auto-destruction options */
bool destroy_items;
bool destroy_feeling;
bool destroy_identify;
bool leave_worth;
bool leave_equip;
bool leave_wanted;
bool leave_corpse;
bool leave_junk;
bool leave_chest;
bool leave_special;

/*
 * Wilderness
 */
wilderness_type **wilderness;
u32b wilderness_seed;


/*
 * Buildings
 */
building_type building[MAX_BLDG];


/*
 * Maximum number of quests
 */
u16b max_quests;
byte num_random_quests;

/*
 * Maximum number of monsters in r_info.txt
 */
u16b max_r_idx;
u16b max_b_idx;

/*
 * Maximum number of items in k_info.txt
 */
u16b max_k_idx;

/*
 * Maximum number of vaults in v_info.txt
 */
u16b max_room_idx;

/*
 * Maximum number of terrain features in f_info.txt
 */
u16b max_f_idx;

/*
 * Maximum number of artifacts in a_info.txt
 */
u16b max_a_idx;

/*
 * Maximum number of ego-items in e_info.txt
 */
u16b max_e_idx;

/*
 * Maximum number of dungeon in d_info.txt
 */
u16b max_d_idx;

/*
 * Maximum number of objects in the level
 */
u16b max_o_idx;

/*
 * Maximum number of monsters in the level
 */
u16b max_m_idx;

/*
 * Maximum size of the wilderness
 */
s32b max_wild_x;
s32b max_wild_y;

/*
 * Default spell color table (quark index)
 */
s16b gf_color[GF_COUNT];

int mutant_regenerate_mod = 100;

bool can_save = FALSE;        /* Game can be saved */

s16b world_monster;
bool world_player;

int cap_mon;
int cap_mspeed;
int cap_hp;
int cap_maxhp;
u16b cap_nickname;

s16b battle_mon[4];
int sel_monster;
int battle_odds;
int kakekin;
u32b mon_odds[4];

int pet_t_m_idx;
int riding_t_m_idx;

s16b kubi_r_idx[MAX_KUBI];
s16b today_mon;

bool write_level;

u32b playtime;
time_t start_time;

int tsuri_dir;

bool sukekaku;
bool new_mane;

bool mon_fight;

bool generate_encounter;

cptr screen_dump = NULL;

/*** Terrain feature variables ***/

/* Nothing */
s16b feat_none;

/* Floor */
s16b feat_floor;

/* Objects */
s16b feat_glyph;
s16b feat_explosive_rune;
s16b feat_rogue_trap1;
s16b feat_rogue_trap2;
s16b feat_rogue_trap3;
s16b feat_semicolon;
s16b feat_mirror;
s16b feat_shadow_zap;

/* Doors */
door_type feat_door[MAX_DOOR_TYPES];

/* Stairs */
s16b feat_up_stair;
s16b feat_down_stair;
s16b feat_entrance;

/* Special traps */
s16b feat_trap_open;
s16b feat_trap_armageddon;
s16b feat_trap_piranha;

/* Rubble */
s16b feat_rubble;

/* Seams */
s16b feat_magma_vein;
s16b feat_quartz_vein;

/* Walls */
s16b feat_granite;
s16b feat_permanent;

/* Glass floor */
s16b feat_glass_floor;

/* Glass walls */
s16b feat_glass_wall;
s16b feat_permanent_glass_wall;

/* Pattern */
s16b feat_pattern_start;
s16b feat_pattern_1;
s16b feat_pattern_2;
s16b feat_pattern_3;
s16b feat_pattern_4;
s16b feat_pattern_end;
s16b feat_pattern_old;
s16b feat_pattern_exit;
s16b feat_pattern_corrupted;

/* Various */
s16b feat_black_market;
s16b feat_town;

/* Terrains */
s16b feat_deep_water;
s16b feat_shallow_water;
s16b feat_deep_lava;
s16b feat_shallow_lava;
s16b feat_dirt;
s16b feat_grass;
s16b feat_flower;
s16b feat_brake;
s16b feat_tree;
s16b feat_mountain;
s16b feat_swamp;
s16b feat_dark_pit;
s16b feat_web;

/* Unknown grid (not detected) */
s16b feat_undetected;

/*
 * Which dungeon ?
 */
byte dungeon_type;
s16b *max_dlv;
u32b *dungeon_flags;

s16b feat_wall_outer;
s16b feat_wall_inner;
s16b feat_wall_solid;
s16b floor_type[100], fill_type[100];

s32b now_turn;
bool use_menu;


travel_type travel;

/* for snipers */
bool reset_concent = FALSE;   /* Concentration reset flag */

/* Initialize pantheons
 * Note that pantheon flags should be RF3_something because code in
 * several places compares them to r_ptr->rflags3 */
pantheon_type pant_list[PANTHEON_MAX] =
{
    { 0, 0, 0, "BUGGY", "Bug", "Software Bugs"},
    { PANTHEON_OLYMPIAN, RF3_OLYMPIAN, RF3_OLYMPIAN2, "Olympian", "Oly", "Olympians"},
    { PANTHEON_EGYPTIAN, RF3_EGYPTIAN, RF3_EGYPTIAN2, "Egyptian", "Egy", "Egyptian gods"},
};

byte summon_pantheon_hack = 0;
