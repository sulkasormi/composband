#ifndef INCLUDED_ROOMS_H
#define INCLUDED_ROOMS_H
/*
 * File: rooms.h
 * Purpose: Header file for rooms.c, used only in generate.c
 */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */


/* Number of rooms to attempt (was 50) */
#define DUN_ROOMS_MAX    40


/* Room types for generate_lake() */
#define LAKE_T_LAVA        1
#define LAKE_T_WATER       2
#define LAKE_T_CAVE        3
#define LAKE_T_EARTH_VAULT 4
#define LAKE_T_AIR_VAULT   5
#define LAKE_T_WATER_VAULT 6
#define LAKE_T_FIRE_VAULT  7


/* Room types for room_build() */
enum {
    ROOM_T_NORMAL,            /* Simple (33x11) */
    ROOM_T_LESSER_VAULT,      /* Lesser vault (33x22) */
    ROOM_T_GREATER_VAULT,     /* Greater vault (66x44) */
    ROOM_T_FRACAVE,           /* Fractal cave (42x24) */
    ROOM_T_CRYPT,             /* Crypts (22x22) */
    ROOM_T_TRAP,              /* Piranha/Armageddon trap room */
    ROOM_T_TEMPLATE,          /* ../lib/edit/rooms.txt */
    ROOM_T_MAX
};

/* Constants for room_template_t types and subtypes. I would use
   ROOM_T_* above except the code is unable to handle addition of subtypes.
   Currently, choose_room_template uses these to pick an appropriate
   template as specified in v_info.txt. */
enum room_type_e {
    ROOM_UNKNOWN,
    ROOM_VAULT,
    ROOM_NORMAL,
    ROOM_WILDERNESS,
    ROOM_AMBUSH,
    ROOM_QUEST,
    ROOM_TOWN,
};

enum vault_type_e {
    VAULT_UNKNOWN,
    VAULT_LESSER,
    VAULT_GREATER,
};

/* ROOM_WILDERNESS uses TERRAIN_* values as the subtype */

/*
 * Room type information
 */
typedef struct room_info_type room_info_type;

struct room_info_type
{
        /* Allocation information. */
        s16b prob[11];

        /* Minimum level on which room can appear. */
        int min_level;
};

/*
 * Generating rooms from templates
 * This includes support for user defined "letters" in the template file
 * as well as built in predefined "letters" (for historical reasons).
 *
 * See lib/edit/readme.txt for parser syntax.
 * See lib/edit/v_info.txt for sample rooms and vaults.
 * See lib/edit/q_*.txt for sample quests.
 * See lib/edit/t_*.txt for sample towns.
 */

#define ROOM_GRID_MON_TYPE      0x00000001  /* monster is SUMMON_* rather than a specific r_idx */
#define ROOM_GRID_MON_CHAR      0x00000002  /* monster is a "d_char" rather than a specific r_idx */
#define ROOM_GRID_MON_RANDOM    0x00000004
#define ROOM_GRID_MON_NO_GROUP  0x00000008
#define ROOM_GRID_MON_NO_SLEEP  0x00000010
#define ROOM_GRID_MON_NO_UNIQUE 0x00000020
#define ROOM_GRID_MON_FRIENDLY  0x00000040
#define ROOM_GRID_MON_HASTE     0x00000080
#define ROOM_GRID_MON_CLONED    0x00000100  /* hack for The Cloning Pits */

#define ROOM_GRID_OBJ_TYPE      0x00010000  /* object is TV_* or OBJ_TYPE_* rather than a specific k_idx */
#define ROOM_GRID_OBJ_ARTIFACT  0x00020000  /* object is a_idx (which implies k_idx) */
#define ROOM_GRID_OBJ_EGO       0x00040000  /* named ego using extra for type */
#define ROOM_GRID_OBJ_RANDOM    0x00080000  /* object is completely random */
#define ROOM_GRID_EGO_RANDOM    0x00100000  /* object is either k_idx or tval, but make it an ego */
#define ROOM_GRID_ART_RANDOM    0x00200000  /* object is either k_idx or tval, but make it a rand art */
#define ROOM_GRID_OBJ_EFFECT    0x00400000  /* object is a device and extra is the EFFECT_* code */

#define ROOM_GRID_TRAP_RANDOM   0x10000000  /* this may override object info */
#define ROOM_GRID_SPECIAL       0x20000000  /* use extra for cave.special field */


#define ROOM_THEME_GOOD        0x0001
#define ROOM_THEME_EVIL        0x0002
#define ROOM_THEME_FRIENDLY    0x0004
#define ROOM_THEME_NIGHT       0x0008  /* Useful for wilderness graveyards where monsters only spawn at night */
#define ROOM_THEME_DAY         0x0010
#define ROOM_THEME_FORMATION   0x0020  /* Hack (see source for details): Allows monster formations. */
#define ROOM_THEME_OBJECT      0x0040  /* Hack: All objects similar (e.g. Potions, Devices, ...) */
#define ROOM_SHOP              0x2000  /* Room is a shop ... NO_TOWN means multiple shops on same level
                                          would all stock the same stuff. This is still a wilderness problem, though */
#define ROOM_DEBUG             0x4000  /* For debugging ... force this template to always be chosen */
#define ROOM_NO_ROTATE         0x8000

enum obj_types_e                           /* OBJ(DEVICE), etc */
{
    OBJ_TYPE_TVAL_MAX = 255,
    OBJ_TYPE_DEVICE,
    OBJ_TYPE_JEWELRY,
    OBJ_TYPE_BOOK,
    OBJ_TYPE_HI_BOOK,
    OBJ_TYPE_BODY_ARMOR,
    OBJ_TYPE_OTHER_ARMOR,
    OBJ_TYPE_WEAPON,
    OBJ_TYPE_BOW_AMMO,
    OBJ_TYPE_MISC,
};

struct room_grid_s
{
    s16b cave_feat;
    s16b cave_trap; /* This could also be a secret door ... */

    u16b cave_info;
    s16b monster;

    s16b object;
    s16b extra;
    s16b extra2;

    u32b flags;

    byte letter;
    byte scramble; /* useful in randomizing quests (e.g. Vault and Cloning Pits) */
    byte monster_level;
    byte object_level;

    byte trap_pct;
    byte mon_pct;
    byte obj_pct;
    byte feat_pct;
};
typedef struct room_grid_s room_grid_t, *room_grid_ptr;
extern obj_ptr room_grid_make_obj(room_grid_ptr grid, int level);

struct room_s
{
    cptr name;

    byte level;
    byte max_level;
    byte rarity;
    byte type;

    u16b subtype;
    u16b flags;

    byte height;
    byte width;

    vec_ptr map;
    int_map_ptr letters;
};
typedef struct room_s room_t, *room_ptr;

extern room_ptr room_alloc(cptr name);
extern void     room_free(room_ptr room);
extern room_ptr choose_room_template(int type, int subtype); /* from v_info.txt */

/* Coordinate Transformations allow rooms to be rotated */
struct transform_s
{
    int     which;
    rect_t  src;
    rect_t  dest;
    point_t fudge;
};
typedef struct transform_s transform_t, *transform_ptr;

transform_ptr transform_alloc(int which, rect_t src);
transform_ptr transform_alloc_random(rect_t src, point_t max_size);
transform_ptr transform_alloc_room(room_ptr room, point_t max_size);
void          transform_free(transform_ptr x);
point_t       transform_point(transform_ptr x, point_t p);


/* Wilderness Scrolling allows towns to be partially generated */
struct wild_scroll_s
{
    point_t scroll;
    rect_t  exclude;
    int     flags;
};
typedef struct wild_scroll_s wild_scroll_t, *wild_scroll_ptr;


/* Generate the room from the template, apply the indicated transformation.
 * The wilderness scroll info is optional (cf wild.c) */
extern void build_room_template_aux(room_ptr room, transform_ptr xform, wild_scroll_ptr wild);


/* Other stuff ... */
extern void build_lake(int type);
extern void build_cavern(void);

extern bool generate_rooms(void);
extern void build_maze_vault(int x0, int y0, int xsize, int ysize, bool is_vault);
extern void coord_trans(int *x, int *y, int xoffset, int yoffset, int transno);
extern bool vault_aux_chapel_g(int r_idx);
extern bool vault_aux_chapel_e(int r_idx);

#endif
