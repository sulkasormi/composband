/* File: types.h */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Purpose: global type declarations */


/*
 * This file should ONLY be included by "angband.h"
 */

/*
 * Note that "char" may or may not be signed, and that "signed char"
 * may or may not work on all machines.  So always use "s16b" or "s32b"
 * for signed values.  Also, note that unsigned values cause math problems
 * in many cases, so try to only use "u16b" and "u32b" for "bit flags",
 * unless you really need the extra bit of information, or you really
 * need to restrict yourself to a single byte for storage reasons.
 *
 * Also, if possible, attempt to restrict yourself to sub-fields of
 * known size (use "s16b" or "s32b" instead of "int", and "byte" instead
 * of "bool"), and attempt to align all fields along four-byte words, to
 * optimize storage issues on 32-bit machines.  Also, avoid "bit flags"
 * since these increase the code size and slow down execution.  When
 * you need to store bit flags, use one byte per flag, or, where space
 * is an issue, use a "byte" or "u16b" or "u32b", and add special code
 * to access the various bit flags.
 *
 * Many of these structures were developed to reduce the number of global
 * variables, facilitate structured program design, allow the use of ascii
 * template files, simplify access to indexed data, or facilitate efficient
 * clearing of many variables at once.
 *
 * Certain data is saved in multiple places for efficient access, currently,
 * this includes the tval/sval/weight fields in "object_type", various fields
 * in "header_type", and the "m_idx" and "o_idx" fields in "cave_type".  All
 * of these could be removed, but this would, in general, slow down the game
 * and increase the complexity of the code.
 */

typedef struct {
    cptr name;
    cptr desc;
} name_desc_t, *name_desc_ptr;

/*
 * Feature state structure
 *
 * - Action (FF_*)
 * - Result (f_info ID)
 */
typedef struct feature_state feature_state;

struct feature_state
{
    byte action;
    s16b result;
};


/*
 * Information about terrain "features"
 */

typedef struct feature_type feature_type;

struct feature_type
{
    u32b name;                /* Name (offset) */
    u32b text;                /* Text (offset) */
    s16b tag;                 /* Tag (offset) */

    s16b mimic;               /* Feature to mimic */

    u32b flags[FF_FLAG_SIZE]; /* Flags */

    u16b priority;            /* Map priority */
    s16b destroyed;           /* Default destroyed state */

    feature_state state[MAX_FEAT_STATES];

    byte subtype;
    byte power;

    byte d_attr[F_LIT_MAX];   /* Default feature attribute */
    byte d_char[F_LIT_MAX];   /* Default feature character */

    byte x_attr[F_LIT_MAX];   /* Desired feature attribute */
    byte x_char[F_LIT_MAX];   /* Desired feature character */
};

/* Effects are used for activations on objects (e.g. DSM) as well as devices (wand, rod, staff) */
struct effect_s
{
    s16b type;         /* See effect_e in defines.h. */
    byte power;        /* Power Level (1-100): Determines damage, etc */
    byte difficulty;   /* Difficulty Level (1-100): Determines fail rate. */
                       /* Think of power as the player level and difficulty as the spell level. */
    s16b cost;         /* Timeout for activations. SP for devices */
    s16b extra;        /* Optionally override normal damage calc. See do_effect for details on a per effect basis. */
};
typedef struct effect_s effect_t;

struct counts_s
{
    s32b generated;
    s32b found;
    s32b bought;
    s32b used;
    s32b destroyed;
};
typedef struct counts_s counts_t, *counts_ptr;

typedef struct {
    s32b found;
    s32b selling;
    s32b buying;
    s32b services;
    s32b winnings;
    s32b stolen;
} gold_counts_t;

/*
 * Information about object "kinds", including player knowledge.
 *
 */

typedef struct object_kind object_kind;

struct object_kind
{
    u32b idx;
    u32b name;            /* Name (offset) */
    u32b text;            /* Text (offset) */
    u32b flavor_name;        /* Flavor name (offset) */

    byte tval;            /* Object type */
    byte sval;            /* Object sub type */

    s16b pval;            /* Object extra info */

    s16b to_h;            /* Bonus to hit */
    s16b to_d;            /* Bonus to damage */
    s16b to_a;            /* Bonus to armor */

    s16b ac;            /* Base armor */

    byte dd, ds;        /* Damage dice/sides for melee weapons */
    s16b mult;          /* Damage multiplier (scaled by 100) for bows */

    s16b weight;        /* Weight */

    s32b cost;            /* Object "base cost" */

    u32b flags[OF_ARRAY_SIZE];    /* Flags */

    u32b gen_flags;        /* flags for generate */
    byte stack_chance;
    byte stack_dice, stack_sides;

    byte locale[4];        /* Allocation level(s) */
    byte chance[4];        /* Allocation chance(s) */

    byte level;            /* Level */
    byte extra;            /* Something */
    byte max_level;        /* Level */

    effect_t activation;
    u32b     activation_msg;

    byte d_attr;        /* Default object attribute */
    byte d_char;        /* Default object character */


    byte x_attr;        /* Desired object attribute */
    byte x_char;        /* Desired object character */


    s16b flavor;        /* Special object flavor (or zero) */

    bool easy_know;        /* This object is always known (if aware) */


    bool aware;            /* The player is "aware" of the item's effects */
    bool tried;            /* The player has "tried" one of the items */
    counts_t counts;
};



/*
 * Information about "artifacts".
 *
 * Note that the save-file only writes "cur_num" to the savefile.
 *
 * Note that "max_num" is always "1" (if that artifact "exists")
 */

typedef struct artifact_type artifact_type, *art_ptr;

struct artifact_type
{
    u32b name;            /* Name (offset) */
    u32b text;            /* Text (offset) */

    byte tval;            /* Artifact type */
    byte sval;            /* Artifact sub type */

    s16b pval;            /* Artifact extra info */

    s16b to_h;            /* Bonus to hit */
    s16b to_d;            /* Bonus to damage */
    s16b to_a;            /* Bonus to armor */

    s16b ac;            /* Base armor */

    byte dd, ds;        /* Damage when hits */
    s16b mult;          /* Damage multiplier (scaled by 100) for bows */

    s16b weight;        /* Weight */

    s32b cost;            /* Artifact "cost" */

    u32b     flags[OF_ARRAY_SIZE];       /* Artifact Flags */
    u32b     known_flags[OF_ARRAY_SIZE];

    effect_t activation;
    u32b     activation_msg;

    u32b gen_flags;        /* flags for generate */

    byte level;            /* Artifact level */
    byte rarity;        /* Artifact rarity */

    bool generated; /* Artifact has been created, but possibly yet to be discovered */
    bool found;     /* Player has found and identified this artifact */

    s16b floor_id;          /* Leaved on this location last time */
};


/*
 * Information about "ego-items".
 */

typedef struct ego_type ego_type, *ego_ptr;

struct ego_type
{
    u32b id;
    u32b name;            /* Name (offset) */
    u32b text;            /* Text (offset) */

    u32b type;            /* Type Flags (Bow, Weapon, Gloves, Helmet, Crown, Harp, etc)
                             Note: Due to object lore, it is very useful to share the same
                             ego type across different equipment slots. For example,
                             Elemental Protection can appear on multiple armor types. */

    byte level;           /* Minimum level */
    byte rarity;          /* Object rarity */
    byte max_level;       /* Maximum level. 0 => No restriction */

    s16b max_to_h;        /* Maximum to-hit bonus */
    s16b max_to_d;        /* Maximum to-dam bonus */
    s16b max_to_a;        /* Maximum to-ac bonus */

    byte max_pval;        /* Maximum pval */

    u32b flags[OF_ARRAY_SIZE];    /* Ego-Item Flags */
    u32b known_flags[OF_ARRAY_SIZE];
    u32b xtra_flags[OF_ARRAY_SIZE];

    u32b gen_flags;        /* flags for generate */
    effect_t activation;

    counts_t counts;
};

/*
 * Object information, for a specific object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis,
 * and care must be taken when handling such objects.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 *
 * Each cave grid points to one (or zero) objects via the "o_idx"
 * field (above).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a "stack" of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "hold_o_idx"
 * field (below).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a pile of objects held by the monster.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix=0" and "iy=0".
 */

typedef struct object_type object_type;
typedef bool (*object_p)(object_type *o_ptr);

struct obj_loc_s {
    byte where; /* INV_EQUIP, INV_PACK, INV_QUIVER, INV_SHOP or INV_FLOOR */
    byte x, y;  /* where == INV_FLOOR */
    int  slot;  /* o_idx if floor, slot otherwise */
};
typedef struct obj_loc_s obj_loc_t;

struct object_type
{
    s16b k_idx;            /* Kind index (zero if "dead") */

    byte tval;            /* Item type (from kind) */
    byte sval;            /* Item sub-type (from kind) */

    s16b pval;            /* Item extra-parameter */

    byte discount;        /* Discount (if any) */

    byte number;        /* Number of items */

    s16b weight;        /* Item weight in decipounds */

    s16b name1;            /* Artifact type, if any */
    s16b name2;            /* Ego-Item type, if any */
    s16b name3;         /* Random replacement for a fixed art */

    byte xtra1;            /* Extra info: Weaponsmith/Politician */
    byte xtra2;            /* Extra info index */
    byte xtra3;            /* Extra info: Chests and Weaponsmith. Device Power. */
    s16b xtra4;            /* Extra info: Lights, Capture, Quiver Capacity, Device MaxSP. */
    s32b xtra5;            /* Extra info: Device CSP */

    s16b to_h;            /* Plusses to hit */
    s16b to_d;            /* Plusses to damage */
    s16b to_a;            /* Plusses to AC */

    s16b ac;            /* Normal AC */

    byte dd, ds;        /* Damage dice/sides */
    s16b mult;          /* Damage multiplier (scaled by 100) for bows */

    s16b timeout;        /* Timeout Counter */

    byte ident;            /* Special flags  */

    u32b marked;        /* Object is marked */

    u16b inscription;    /* Inscription index */
    u16b art_name;      /* Artifact name (random artifacts) */

    byte feeling;          /* Game generated inscription number (eg, pseudo-id) */

    u32b flags[OF_ARRAY_SIZE];        /* Extra Flags for ego and artifacts */

    u32b curse_flags;        /* Flags for curse */

    u32b known_flags[OF_ARRAY_SIZE];
    u32b known_curse_flags;
    u32b known_xtra;

    u32b rune;

    s16b next_o_idx;    /* Next object in stack (if any) */

    s16b held_m_idx;    /* Monster holding us (if any) */
    effect_t activation;
    obj_loc_t loc;

    s16b level;         /* object_level on generation for my statistical pleasures */
    u16b origin_place;  /* dungeon and level of generation */ 
    byte origin_type;   /* type of generation */
    s16b origin_xtra;   /* extra information about the item's origin */
    byte mitze_type;    /* MITZE type */
    byte mitze_level;   /* level on MITZE */
    s32b mitze_turn;    /* turn on MITZE */
    u16b insured;       /* Item is part of insurance policy */
    int  scratch;
};
#define object_is_(O, T, S) ((O)->tval == (T) && (O)->sval == (S))
#define object_plural(O) (((O)->number != 1) || (have_flag((O)->flags, OF_PLURAL)))

/* Monster blows ... Redone. Note that changing any
 * of the following values will break savefiles (or
 * at least require special code to read old files). */
#define MAX_MON_BLOW_EFFECTS 4
#define MAX_MON_BLOWS        4
#define MAX_MON_AURAS        3
typedef struct {
    s16b effect; /* either RBE_* or GF_* */
    byte dd;
    byte ds;
    byte pct;    /* 0 => 100% (as does 100) */
    s16b lore;   /* monster lore: number of times this effect experienced */
} mon_effect_t, *mon_effect_ptr;

typedef struct {
    byte method;
    byte power;
    s16b lore;   /* monster lore: number of times this blow seen (hit or miss) */
    mon_effect_t effects[MAX_MON_BLOW_EFFECTS];
} mon_blow_t, *mon_blow_ptr;

/*
 * Monster "race" information, including racial memories
 *
 * Note that "d_attr" and "d_char" are used for MORE than "visual" stuff.
 *
 * Note that "x_attr" and "x_char" are used ONLY for "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Note that several of these fields, related to "recall", can be
 * scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc).  All of the "recall"
 * fields have a special prefix to aid in searching for them.
 */

typedef struct {
    s16b dis;            /* disarming */
    s16b dev;            /* magic devices */
    s16b sav;            /* saving throw */
    s16b stl;            /* stealth */
    s16b srh;            /* search ability */
    s16b fos;            /* search frequency */
    s16b thn;            /* combat (normal) */
    s16b thb;            /* combat (shooting) */
} skills_t, *skills_ptr;

#define SKILL_DESC_LEN 50
typedef struct {
    char dis[SKILL_DESC_LEN];
    char dev[SKILL_DESC_LEN];
    char sav[SKILL_DESC_LEN];
    char stl[SKILL_DESC_LEN];
    char srh[SKILL_DESC_LEN];
    char fos[SKILL_DESC_LEN];
    char thn[SKILL_DESC_LEN];
    char thb[SKILL_DESC_LEN];
} skills_desc_t, *skills_desc_ptr;

/* For the blows calculation, see calculate_base_blows in combat.c
 * This structure should be initialized in the appropriate calc_weapon_bonuses
 * cf calc_bonuses in xtra1.c */
typedef struct {
    int max;
    int wgt;
    int mult;
} _blows_calc_t;

struct monster_body_s
{
    s16b     stats[MAX_STATS];
    skills_t skills;
    skills_t extra_skills;
    s16b     life;
    s16b     infra;
    s16b     spell_stat;
    s16b     body_idx;
    s16b     class_idx;
    s16b     speed;
    _blows_calc_t blows_calc;
};
typedef struct monster_body_s monster_body_t;


typedef struct monster_race monster_race;

struct monster_race
{
    u32b name;                /* Name (offset) */
    u32b text;                /* Text (offset) */

    byte hdice;               /* Creatures hit dice count */
    byte hside;               /* Creatures hit dice sides */
    s16b ac;                  /* Armour Class: Always use mon_ac(mon) instead! */

    s16b sleep;               /* Inactive counter (base) */
    byte aaf;                 /* Area affect radius (1-100) */
    byte speed;               /* Speed (normally 110) */

    s32b mexp;                /* Exp value for kill */

    s16b weight;
    byte drop_theme;

    mon_spells_ptr spells;
    u32b flags1;              /* Flags 1 (general) */
    u32b flags2;              /* Flags 2 (abilities) */
    u32b flags3;              /* Flags 3 (race/resist) */
    u32b flags7;              /* Flags 7 (movement related abilities) */
    u32b flags8;              /* Flags 8 (wilderness info) */
    u32b flags9;              /* Flags 9 (drops info; possessor info) */
    u32b flagsr;              /* Flags R (resistances info) */
    u32b flagsx;              /* Temp Flags valid only for a single game. Written to savefile.
                                 For example, this unique is a questor. Or this
                                 unique is suppressed and won't appear in this game. */

    mon_blow_t blows[MAX_MON_BLOWS];
    mon_effect_t auras[MAX_MON_AURAS];

    s16b next_r_idx;          /* Evolution */
    u32b next_exp;

    byte pack_pct;           /* FRIENDS has become FRIENDS(XdY[,Z%]) if desired */
    byte pack_dice;
    byte pack_sides;

    byte level;               /* Level of creature */
    byte melee_level;
    byte save_level;
    byte rarity;              /* Rarity of creature */

    s16b max_level;
    s16b id;


    byte d_attr;              /* Default monster attribute */
    byte d_char;              /* Default monster character */
    byte x_attr;              /* Desired monster attribute */
    byte x_char;              /* Desired monster character */


    byte max_num;             /* Maximum population allowed per level */
    byte cur_num;             /* Monster population on current level */
    byte ball_num;            /* Monster population in capture balls */
    s16b floor_id;            /* Location of unique monster */


    s16b r_sights;            /* Count sightings of this monster */
    s16b r_deaths;            /* Count deaths from this monster */

    s16b r_pkills;            /* Count visible monsters killed in this life */
    s16b r_akills;            /* Count all monsters killed in this life */
    s16b r_tkills;            /* Count monsters killed in all lives */

    s16b r_skills;            /* Count all summons killed in this life */

    byte r_wake;              /* Number of times woken up (?) */
    byte r_ignore;            /* Number of times ignored (?) */

    byte r_xtra1;             /* Flags for Evolution and Possessor Body Info */
    byte r_xtra2;             /* Something (unused) */

    byte r_drop_gold;         /* Max number of gold dropped at once */
    byte r_drop_item;         /* Max number of item dropped at once */

    u32b r_spell_turns;       /* Number of spells cast or failed (so may exceed sum(spell->lore)) */
    u32b r_move_turns;        /* Includes attacking the player */
                              /* Now we can report accurate observed spell frequencies! */

    u32b r_flags1;            /* Observed racial flags */
    u32b r_flags2;            /* Observed racial flags */
    u32b r_flags3;            /* Observed racial flags */
    u32b r_flagsr;            /* Observed racial resistance flags */

    byte stolen_ct;           /* For uniques in this lifetime only. Prevents PickPocket scumming of excellent drop uniques */

    monster_body_t body;      /* For The Possessor */
};



/*
 * Information about "skill"
 */

typedef struct skill_table skill_table;

struct skill_table
{
    s16b w_start[5][64];      /* start weapon exp */
    s16b w_max[5][64];        /* max weapon exp */
    s16b s_start[10];      /* start skill */
    s16b s_max[10];           /* max skill */
};


/*
 * A single "grid" in a Cave
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * The "o_idx" and "m_idx" fields are very interesting.  There are
 * many places in the code where we need quick access to the actual
 * monster or object(s) in a given cave grid.  The easiest way to
 * do this is to simply keep the index of the monster and object
 * (if any) with the grid, but this takes 198*66*4 bytes of memory.
 * Several other methods come to mind, which require only half this
 * amound of memory, but they all seem rather complicated, and would
 * probably add enough code that the savings would be lost.  So for
 * these reasons, we simply store an index into the "o_list" and
 * "m_list" arrays, using "zero" when no monster/object is present.
 *
 * Note that "o_idx" is the index of the top object in a stack of
 * objects, using the "next_o_idx" field of objects (see below) to
 * create the singly linked list of objects.  If "o_idx" is zero
 * then there are no objects in the grid.
 *
 * Note the special fields for the "MONSTER_FLOW" code.
 */

typedef struct cave_type cave_type;

struct cave_type
{
    u32b info;        /* Hack -- cave flags */

    s16b feat;        /* Hack -- feature type */

    s16b o_idx;        /* Object in this grid */

    s16b m_idx;        /* Monster in this grid */

    s16b special;    /* Special cave info */

    s16b mimic;        /* Feature to mimic */

    byte cost;        /* Hack -- cost of flowing */
    byte dist;        /* Hack -- distance from player */
    byte when;        /* Hack -- when cost was computed */
};



/*
 * Simple structure to hold a map location
 */
typedef struct coord coord;

struct coord
{
    byte y;
    byte x;
};


/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "hold_o_idx" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */

typedef struct monster_type monster_type;
enum {
    SM_REFLECTION = RES_MAX, /* 18 */
    SM_FREE_ACTION,
    SM_GUARDIAN,
    SM_CLONED,
    SM_PET,
    SM_FRIENDLY,
    SM_SUMMONED,
    SM_MAX
};

enum {
    MTIMED_CSLEEP,
    MTIMED_FAST,
    MTIMED_SLOW,
    MTIMED_STUNNED,
    MTIMED_CONFUSED,
    MTIMED_MONFEAR,
    MTIMED_INVULNER,
    MTIMED_PARALYZED,
    MTIMED_COUNT
};

#define MON_CSLEEP(M_PTR)   ((M_PTR)->mtimed[MTIMED_CSLEEP])
#define MON_FAST(M_PTR)     ((M_PTR)->mtimed[MTIMED_FAST])
#define MON_SLOW(M_PTR)     ((M_PTR)->mtimed[MTIMED_SLOW])
#define MON_STUNNED(M_PTR)  ((M_PTR)->mtimed[MTIMED_STUNNED])
#define MON_CONFUSED(M_PTR) ((M_PTR)->mtimed[MTIMED_CONFUSED])
#define MON_MONFEAR(M_PTR)  ((M_PTR)->mtimed[MTIMED_MONFEAR])
#define MON_INVULNER(M_PTR) ((M_PTR)->mtimed[MTIMED_INVULNER])
#define MON_PARALYZED(M_PTR) ((M_PTR)->mtimed[MTIMED_PARALYZED])

struct monster_type
{
    int  id;
    s16b r_idx;        /* Monster race index */
    s16b ap_r_idx;        /* Monster race appearance index */
    byte sub_align;        /* Sub-alignment for a neutral monster */

    byte fy;        /* Y location on map */
    byte fx;        /* X location on map */

    s16b hp;        /* Current Hit points */
    s16b maxhp;        /* Max Hit points */
    s16b max_maxhp;        /* Max Max Hit points */
    s16b ac_adj;
    s16b mpower;    /* Monster power scales various things like melee skill, damage, AC, etc.
                       This field is a per mill value, just like p_ptr->clp */

    s16b mtimed[MTIMED_COUNT];    /* Timed status counter */

    byte mspeed;            /* Monster "speed" */
    s16b energy_need;    /* Monster "energy" */

    byte cdis;        /* Current dis from player */

    u32b mflag;        /* Extra monster flags. (Note: Not saved!) */
    u32b mflag2;        /* Extra monster flags. (This one *is* saved)  */

    bool ml;        /* Monster is "visible" */

    s16b hold_o_idx;    /* Object being held (if any) */

    s16b target_y;        /* Can attack !los player */
    s16b target_x;        /* Can attack !los player */

    u16b nickname;        /* Monster's Nickname */

    u32b exp;

    u32b smart;            /* Field for "smart_learn" */

    s16b parent_m_idx;
    s16b pack_idx;

    byte drop_ct;
    byte stolen_ct;

    byte ego_whip_ct;
    byte ego_whip_pow;
    byte anti_magic_ct;
    byte anger;
    byte minislow;
    s16b mana;

    s32b pexp;    /* player experience gained (x100). kept <= r_ptr->mexp */
};

enum {
    AI_SEEK = 0,
    AI_LURE = 1,
    AI_GUARD_MON = 2,
    AI_GUARD_POS = 3,
    AI_FEAR = 4,
    AI_SHOOT = 5,
    AI_MAINTAIN_DISTANCE = 6,
};

typedef struct {
    s16b pack_idx;        /* Set when allocated */
    s16b leader_idx;
    s16b count;
    s16b ai;            /* How is the pack behaving? */
    s16b guard_idx;     /* Pack is guarding another monster, perhaps the leader. Or, this is a dungeon entrance being guarded. */
    s16b guard_x;       /* Pack is defending a specific location */
    s16b guard_y;
    s16b distance;
    s16b next_idx;        /* Free list */
} pack_info_t;


/*
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */

typedef struct alloc_entry alloc_entry;

struct alloc_entry
{
    s16b index;        /* The actual index */

    s16b level;        /* Base dungeon level */
    s16b max_level;
    byte prob1;        /* Probability, pass 1 */
    byte prob2;        /* Probability, pass 2 */
    byte prob3;        /* Probability, pass 3 */

    u16b total;        /* Unused for now */
};



/*
 * Available "options"
 *
 *    - Address of actual option variable (or NULL)
 *
 *    - Normal Value (TRUE or FALSE)
 *
 *    - Option Page Number (or zero)
 *
 *    - Savefile Set (or zero)
 *    - Savefile Bit in that set
 *
 *    - Textual name (or NULL)
 *    - Textual description
 */

typedef struct option_type option_type;

struct option_type
{
    bool    *o_var;

    byte    o_norm;

    byte    o_page;

    byte    o_set;
    byte    o_bit;

    cptr    o_text;
    cptr    o_desc;
};


typedef struct magic_type magic_type;

struct magic_type
{
    byte realm;         /* Which realm the spell is in */
    byte idx;           /* Index of the spell */
    byte slevel;        /* Required level (to learn) */
    byte smana;            /* Required mana (to cast) */
    byte sfail;            /* Minimum chance of failure */
    byte sexp;            /* Encoded experience bonus */
};

/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */

typedef struct player_magic player_magic;

struct player_magic
{
    int spell_book;        /* Tval of spell books (if any) */
    int spell_xtra;        /* Something for later */

    int spell_stat;        /* Stat for spells (if any)  */
    int spell_type;        /* Spell type (mage/priest) */

    int spell_first;        /* Level of first spell */
    int spell_weight;        /* Weight that hurts spells */

    magic_type info[MAX_MAGIC][32];    /* The available spells */
};



/*
 * Player sex info
 */

typedef struct player_sex player_sex;

struct player_sex
{
    cptr title;            /* Type of sex */
    cptr winner;        /* Name of winner */
};

typedef struct player_pact player_pact;

struct player_pact
{
    cptr title;
    cptr alliance;
};

/*
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This structure contains several "blocks" of information.
 *   (1) the "permanent" info
 *   (2) the "variable" info
 *   (3) the "transient" info
 *
 * All of the "permanent" info, and most of the "variable" info,
 * is saved in the savefile.  The "transient" info is recomputed
 * whenever anything important changes.
 */
#define WIELD_NONE       0
#define WIELD_ONE_HAND   1
#define WIELD_TWO_HANDS  2

#define MAX_HANDS 6
#define MAX_ARMS  3
#define HAND_NONE -1

typedef struct {
    int  wield_how;
    bool omoi;   /* WIELD_TWO_HANDS but too heavy for WIELD_ONE_HAND */
    bool bare_hands; /* Monks and Forcetrainers */
    bool riding; /* Riding requires one hand to control your mount */
    int  slot;
    int  to_h;
    int  to_d;
    int  dis_to_h;
    int  dis_to_d;
    int  to_dd;
    int  to_ds;
    _blows_calc_t blows_calc; /* set in calc_weapon_bonuses (class_t or sometimes race_t) */
    int  base_blow;
    int  xtra_blow;
    bool genji;
    bool heavy_wield;
    bool icky_wield;
    bool riding_wield;
    int  giant_wield;
    int  dual_wield_pct; /* Scaled by 10 so 123 = 12.3%. Set to 1000 (ie 100%) if not dual wielding */
    u32b flags[OF_ARRAY_SIZE];
    u32b known_flags[OF_ARRAY_SIZE];
    byte info_attr;
    cptr info;
} weapon_info_t, *weapon_info_ptr;

#define NUM_BLOWS(h) (p_ptr->weapon_info[h].base_blow + p_ptr->weapon_info[h].xtra_blow)

typedef struct {
    int slot;
    int to_h;
    int to_d;
    int dis_to_h;
    int dis_to_d;
    int to_mult;
    int base_shot;
    int xtra_shot;
    byte tval_ammo;
    int breakage; /* pct of normal breakage odds ... default is 100 */
    bool heavy_shoot;
    u32b flags[OF_ARRAY_SIZE];
} shooter_info_t;

#define NUM_SHOTS (p_ptr->shooter_info.base_shot + p_ptr->shooter_info.xtra_shot)

typedef struct {
    int to_dd;
    int xtra_blow;
    u32b flags[OF_ARRAY_SIZE]; /* TODO */
} innate_attack_info_t;

typedef struct player_type player_type;

typedef struct {
    s16b type;
    s16b counter;
} counter_t;

#define MAX_WILD_COUNTERS  5
#define MAX_INNATE_EFFECTS 8

#define INNATE_SKIP        0x0001
#define INNATE_VORPAL      0x0002
#define INNATE_EXPLODE     0x0004
#define INNATE_NO_DAM      0x0008
#define INNATE_NO_CRIT     0x0010

typedef struct {
    int dd;
    int ds;
    int to_h;
    int to_d;
    int blows;
    int weight;
    int effect[MAX_INNATE_EFFECTS];
    int effect_chance[MAX_INNATE_EFFECTS];
    int flags;
    cptr msg;   /* "You bite %s.", "You hit %s with your horns.", etc. */
    cptr name;
} innate_attack_t, *innate_attack_ptr;
#define MAX_INNATE_ATTACKS 10

struct player_type
{
    s32b id;
    s16b oldpy;        /* Previous player location -KMW- */
    s16b oldpx;        /* Previous player location -KMW- */

    byte psex;            /* Sex index */
    byte prace;            /* Race index */
    byte pclass;        /* Class index */
    byte personality;        /* Seikaku index */
    byte realm1;        /* First magic realm */
    byte realm2;        /* Second magic realm */
    byte dragon_realm;
    byte psubclass;        /* e.g. Pacts on Warlocks. Type of Weaponmaster.*/
    byte psubrace;      /* e.g. Parentage on Demigods */
    s16b current_r_idx;


    u16b expfact;

    s32b au;            /* Current Gold */
    s16b fame;

    s32b max_max_exp;    /* Max max experience (only to calculate score) */
    s32b max_exp;        /* Max experience */
    s32b exp;            /* Cur experience */
    u32b exp_frac;        /* Cur exp frac (times 2^16) */

    s16b lev;            /* Level */

    u32b quest_seed;     /* Seed for random quests */

    s16b town_num;            /* Current town number */
    s16b arena_number;        /* monster number in arena -KMW- */
    bool inside_arena;        /* Is character inside arena? */
    bool inside_battle;        /* Is character inside tougijou? */

    s32b wilderness_x;    /* Coordinates in the wilderness */
    s32b wilderness_y;
    s16b wilderness_dx;   /* Offset of 4x4 viewport window */
    s16b wilderness_dy;
    bool wild_mode;

    s32b mmhp;           /* Max Max hit pts */
    s32b mhp;            /* Max hit pts */
    s32b chp;            /* Cur hit pts */
    u32b chp_frac;       /* Cur hit frac (times 2^16) */

    s32b msp;            /* Max mana pts */
    s32b csp;            /* Cur mana pts */
    u32b csp_frac;       /* Cur mana frac (times 2^16) */

    s16b clp;            /* Cur life pts (per mill) */

    s16b max_plv;        /* Max Player Level */

    s16b stat_max[6];    /* Current "maximal" stat values */
    s16b stat_max_max[6];    /* Maximal "maximal" stat values */
    s16b stat_cur[6];    /* Current "natural" stat values */

    s16b learned_spells;
    s16b add_spells;

    u32b count;

    s16b fast;        /* Timed -- Fast */
    s16b slow;        /* Timed -- Slow */
    s16b blind;        /* Timed -- Blindness */
    s16b paralyzed;        /* Timed -- Paralysis */
    s16b confused;        /* Timed -- Confusion */
    s16b afraid;        /* Timed -- Fear */
    s16b image;        /* Timed -- Hallucination */
    s16b poisoned;        /* Timed -- Poisoned */
    s16b cut;        /* Timed -- Cut */
    s16b stun;        /* Timed -- Stun */

    s16b protevil;        /* Timed -- Protection */
    s16b invuln;        /* Timed -- Invulnerable */
    s16b ult_res;        /* Timed -- Ultimate Resistance */
    s16b hero;        /* Timed -- Heroism */
    s16b shero;        /* Timed -- Super Heroism */
    s16b shield;        /* Timed -- Shield Spell */
    s16b blessed;        /* Timed -- Blessed */
    s16b tim_invis;        /* Timed -- See Invisible */
    s16b tim_infra;        /* Timed -- Infra Vision */
    s16b tsuyoshi;        /* Timed -- Tsuyoshi Special */
    s16b ele_attack;    /* Timed -- Elemental Attack */
    s16b ele_immune;    /* Timed -- Elemental Immune */

    s16b oppose_acid;    /* Timed -- oppose acid */
    s16b oppose_elec;    /* Timed -- oppose lightning */
    s16b oppose_fire;    /* Timed -- oppose heat */
    s16b oppose_cold;    /* Timed -- oppose cold */
    s16b oppose_pois;    /* Timed -- oppose poison */
    s16b spin;           /* Timed -- spinning */

    s16b tim_esp;       /* Timed ESP */
    s16b tim_esp_magical;
    s16b wraith_form;   /* Timed wraithform */

    s16b resist_magic;  /* Timed Resist Magic (later) */
    s16b tim_regen;
    s16b kabenuke;
    s16b tim_stealth;
    s16b tim_levitation;
    s16b tim_sh_touki;
    s16b lightspeed;
    s16b tsubureru;
    s16b magicdef;
    s16b tim_res_nether;
    s16b tim_res_time;
    s16b tim_res_disenchantment;
    s16b mimic_form;
    s16b tim_mimic;
    s16b tim_sh_fire;
    s16b tim_sh_elements;
    s16b tim_sh_shards;
    s16b tim_sh_domination;
    s16b tim_weaponmastery;
    s16b tim_sh_holy;
    s16b tim_eyeeye;

    s16b tim_spurt;

    s16b tim_blood_shield;
    s16b tim_blood_seek;
    s16b tim_blood_sight;
    s16b tim_blood_feast;
    s16b tim_blood_revenge;
    s16b tim_blood_rite;

    s16b tim_force;
    s16b tim_building_up;
    s16b tim_vicious_strike;
    s16b tim_enlarge_weapon;
    s16b tim_field;

    s16b tim_spell_reaction;
    s16b tim_resist_curses;
    s16b tim_armor_of_fury;
    s16b tim_spell_turning;
    bool spell_turned;

    s16b tim_dark_stalker;
    s16b tim_nimble_dodge;
    s16b tim_stealthy_snipe;

    s16b tim_killing_spree;
    s16b tim_slay_sentient;
    byte unwell; /* Never takes high values */
    bool maul_of_vice;
    bool uimapuku;
    bool upkeep_warning; /* Unsafe upkeep - pets may turn hostile */
    bool upset_okay;

    counter_t wild_counters[MAX_WILD_COUNTERS];    /* Wild Weapons */

    bool            innate_attack_lock;
    innate_attack_t innate_attacks[MAX_INNATE_ATTACKS];
    int             innate_attack_ct;

    bool sense_artifact;
    s16b duelist_target_idx;

    bool return_ammo;
    bool big_shot;
    bool painted_target;
    int  painted_target_idx;
    int  painted_target_ct;
    bool easy_2weapon;
    bool speciality_equip;
    bool sneak_attack;
    bool enhanced_crit;
    bool cleave;
    bool constant_hero;
    bool vorpal;
    bool whirlwind;
    s16b elaborate_defense;
    s16b cloak_of_shadows;
    bool lightning_reflexes;
    bool clear_mind;

    bool ambush;
    bool peerless_stealth;
    s16b open_terrain_ct;

    s16b entrench_x;
    s16b entrench_y;
    s16b entrench_ct;
    bool entrenched;
    bool inven_prot;
    bool quick_walk;
    bool filibuster;

    s16b tim_no_spells;     /* Blocking spell usage is a side effect of Empowered Blast, but will become an evil monster ability */
    s16b tim_no_device;        /* For a more powerful twist, this will block devices as well!  But that is really an evil death sentence :) */

    s16b tim_superstealth;

    bool fasting;
    s16b tim_sustain_str;
    s16b tim_sustain_int;
    s16b tim_sustain_wis;
    s16b tim_sustain_dex;
    s16b tim_sustain_con;
    s16b tim_sustain_chr;
    s16b tim_hold_life;
    s16b tim_transcendence;
    s16b tim_quick_walk;
    s16b tim_inven_prot;
    s16b tim_inven_prot2;
    s16b tim_device_power;
    s16b tim_sh_time;
    s16b free_turns;
    s16b tim_foresight;

    /* Rune Knight: Some Rune effects might become general game mechanics, like Magic Resistance
       and Magic Absorption.  Also, let's consolidate the White Aura mutation with the Rune of
       Good Fortune into a cached Good Luck value */
    s16b magic_resistance;
    bool good_luck;

    bool rune_elem_prot;

    /* for mirror master */
    s16b tim_reflect;       /* Timed -- Reflect */
    s16b multishadow;       /* Timed -- Multi-shadow */
    s16b dustrobe;          /* Timed -- Robe of dust */

    s16b chaos_patron;
    u32b muta[MUT_FLAG_SIZE];
    u32b muta_lock[MUT_FLAG_SIZE];   /* Mutations that can not be removed! */
    s16b demigod_power[MAX_DEMIGOD_POWERS];
    s16b draconian_power;

    s16b virtues[8];
    s16b vir_types[8];

    s16b word_recall;      /* Word of recall counter */
    s16b alter_reality;      /* Alter reality counter */
    byte recall_dungeon;      /* Dungeon set to be recalled */
    byte coffee_lv_revisits;  /* Count 99/100 spam by coffee-breakers */
    byte minislow;
    u16b mini_energy;
    byte py_summon_kills;
    s16b lv_kills;
    s16b pet_lv_kills;

    s16b energy_need;      /* Energy needed for next move */

    s16b food;          /* Current nutrition */

    u32b special_attack;      /* Special attack capacity -LM- */
    u32b special_defense;      /* Special block capacity -LM- */
    byte action;          /* Current action */

    u32b spell_learned1;      /* bit mask of spells learned */
    u32b spell_learned2;      /* bit mask of spells learned */
    u32b spell_worked1;      /* bit mask of spells tried and worked */
    u32b spell_worked2;      /* bit mask of spells tried and worked */
    u32b spell_forgotten1;      /* bit mask of spells learned but forgotten */
    u32b spell_forgotten2;      /* bit mask of spells learned but forgotten */
    byte spell_order[64];      /* order spells learned/remembered/forgotten */

    s16b spell_exp[64];       /* Proficiency of spells */
    s16b weapon_exp[5][64];   /* Proficiency of weapons */
    s16b skill_exp[10];       /* Proficiency of misc. skill */
    s16b spells_per_round;    /* 175 = 1.75 spells per round, etc. Calculated in calc_bonuses(). Only works for book casters (do_cmd_cast) at the moment. */

    s32b magic_num1[MAX_MAGIC_NUM];     /* Array for non-spellbook type magic */
    byte magic_num2[MAX_MAGIC_NUM];     /* Flags for non-spellbook type magics */

    s16b mane_spell[MAX_MANE];
    s16b mane_dam[MAX_MANE];
    s16b mane_num;

    s16b concent;      /* Sniper's concentration level */

    s16b player_hp[PY_MAX_LEVEL];
    char died_from[80];         /* What killed the player */
    cptr last_message;        /* Last message on death or retirement */

    u16b total_winner;      /* Total winner */
    u16b panic_save;      /* Panic save */

    u16b noscore;          /* Cheating flags */

    bool wait_report_score;   /* Waiting to report score */
    bool is_dead;          /* Player is dead */

    bool wizard;          /* Player is in wizard mode */

    s16b riding;              /* Riding on a monster of this index */
    byte knowledge;           /* Knowledge about yourself */

    byte start_race;          /* Race at birth */
    byte start_sex;           /* Sex at birth */
    s32b old_race1;           /* Record of race changes */
    s32b old_race2;           /* Record of race changes */
    s32b old_race3;           /* Record of race changes */
    s16b old_realm;           /* Record of realm changes */

    s16b pet_follow_distance; /* Length of the imaginary "leash" for pets */
    s16b pet_extra_flags;     /* Various flags for controling pets */

    s16b today_mon;           /* Wanted monster */

    s16b floor_id;            /* Current floor location */

    bool autopick_autoregister; /* auto register is in-use or not */

    byte feeling;        /* Most recent dungeon feeling */
    s32b feeling_turn;    /* The turn of the last dungeon feeling */


    /*** Temporary fields ***/

    bool playing;            /* True if player is playing */
    bool leaving;            /* True if player is leaving */
    int  leaving_method;

    byte exit_bldg;            /* Goal obtained in arena? -KMW- */

    byte leaving_dungeon;    /* Which dungeon the player is leaving if any */
    bool teleport_town;
    bool enter_dungeon;     /* Just enter the dungeon */

    s16b health_who;    /* Health bar trackee */

    s16b monster_race_idx;    /* Monster race trackee */

    s16b object_kind_idx;    /* Object kind trackee */

    s16b new_spells;    /* Number of spells available */
    s16b old_spells;

    s16b old_food_aux;    /* Old value of food */

    bool old_cumber_armor;
    bool old_cumber_glove;
    bool old_heavy_wield[MAX_HANDS];
    bool old_heavy_shoot;
    bool old_icky_wield[MAX_HANDS];
    bool old_riding_wield[MAX_HANDS];
    bool old_riding_ryoute;
    bool old_monlite;

    s16b old_lite;        /* Old radius of lite (if any) */

    bool cumber_armor;    /* Mana draining armor */
    int  cumber_armor_amt;
    bool cumber_glove;    /* Mana draining gloves */
    bool riding_ryoute;    /* Riding weapon */
    bool monlite;

    s16b cur_lite;        /* Radius of lite (if any) */


    u32b notice;        /* Special Updates (bit flags) */
    u32b update;        /* Pending Updates (bit flags) */
    u32b redraw;        /* Normal Redraws (bit flags) */
    u32b window;        /* Window Redraws (bit flags) */

    s16b stat_use[6];    /* Current modified stats */
    s16b stat_top[6];    /* Maximal modified stats */

    bool sutemi;
    bool counter;

    s32b align;                /* Good/evil/neutral */
    s16b run_py;
    s16b run_px;

    bool nice;


    /*** Extracted fields ***/
    s16b stat_add[6];    /* Modifiers to stat values */
    s16b stat_ind[6];    /* Indexes into stat tables */

    s16b resist[RES_MAX];
    s16b life;

    bool reflect;
    byte sh_fire;
    byte sh_elec;
    byte sh_cold;
    byte sh_shards;
    bool sh_retaliation;
    bool sh_fear;
    bool ignore_invuln;

    bool no_eldritch;
    bool no_stun;
    bool no_cut;
    bool no_slow;
    bool no_passwall_dam;
    bool no_charge_drain;
    bool melt_armor;

    bool anti_magic;    /* Anti-magic */
    bool anti_tele;     /* Prevent teleportation */
    bool anti_summon;

    bool sustain_str;    /* Keep strength */
    bool sustain_int;    /* Keep intelligence */
    bool sustain_wis;    /* Keep wisdom */
    bool sustain_dex;    /* Keep dexterity */
    bool sustain_con;    /* Keep constitution */
    bool sustain_chr;    /* Keep charisma */

    u32b cursed;         /* Player is cursed */

    bool can_swim;       /* No damage falling */
    bool levitation;     /* No damage falling */
    bool lite;           /* Permanent light */
    s16b free_act;       /* Resist paralysis; perhaps slowing */
    s16b see_inv;        /* Can see invisible */
    s16b regen;          /* Rate of regeneration: 100 = 100%, 200 = 200%, etc. */
    s16b hold_life;      /* Resist life draining */

    bool auto_id;
    bool auto_pseudo_id;
    bool munchkin_pseudo_id;
    int  auto_id_sp;
    bool cult_of_personality;
    bool fairy_stealth;

    bool telepathy;      /* Telepathy */
    bool esp_animal;
    bool esp_undead;
    bool esp_demon;
    bool esp_orc;
    bool esp_troll;
    bool esp_giant;
    bool esp_dragon;
    bool esp_human;
    bool esp_evil;
    bool esp_good;
    bool esp_nonliving;
	bool esp_living;
    bool esp_unique;
    bool esp_magical;

    bool slow_digest;    /* Slower digestion */
    bool pass_wall;     /* Permanent wraithform */
    bool kill_wall;
    bool dec_mana;
    s16b spell_power;
    s16b device_power;
    s16b spell_cap;
    bool easy_spell;
    bool heavy_spell;
    bool warning;
    bool mighty_throw;
    bool see_nocto;        /* Noctovision */
    bool easy_capture;

    byte easy_realm1;   /* Magic Stones give realm specific boosts */

    bool move_random;   /* Cyberdemons and Possessors ... */

    s16b monk_lvl;

    int           weapon_ct;
    weapon_info_t weapon_info[MAX_HANDS];
    innate_attack_info_t innate_attack_info;
    shooter_info_t shooter_info;

    s16b dis_to_a;        /* Known bonus to ac */

    s16b dis_ac;        /* Known base ac */

    s16b to_h_m;            /* Bonus to hit (misc) */
    s16b to_d_m;            /* Bonus to dam (misc) */
    s16b to_a;            /* Bonus to ac */

    s16b to_d_spell;

    s16b to_m_chance;        /* Minusses to cast chance */

    bool ryoute;
    bool migite;
    bool hidarite;

    bool no_flowed;

    int birth_mutation;

    s16b ac;            /* Base ac */

    s16b see_infra;        /* Infravision range */

    skills_t skills;

    s16b skill_tht;        /* Skill: To hit (throwing) */
    s16b skill_dig;        /* Skill: Digging */

    s16b pspeed;        /* Current speed */
};


/*
 * A structure to hold "rolled" information
 *
 * TODO: Dead fields are mis-aligned ... remove for 6.0
 */
typedef struct birther birther;

struct birther
{
    byte game_mode;
    byte coffee_break;
    byte psex;         /* Sex index */
    byte prace;        /* Race index */
    byte psubrace;
    byte pclass;       /* Class index */
    byte psubclass;       /* Subclass index */
    byte personality;     /* Seikaku index */
    byte realm1;       /* First magic realm */
    byte realm2;       /* Second magic realm */
    byte dragon_realm;

s16b age;
s16b ht;
s16b wt;
s16b sc;

    s32b au;

    s16b stat_max[6];        /* Current "maximal" stat values */
s16b stat_max_max[6];    /* Maximal "maximal" stat values */
s16b player_hp[PY_MAX_LEVEL]; /* Map (L-1)->Cumulative Percentage of Base HD */
                              /* See calc_hitpoints() in xtra1.c for details */
s16b chaos_patron;
int  mutation;

s16b vir_types[8];

    bool quick_ok;
};


/* For Monk martial arts */

typedef struct martial_arts martial_arts;

struct martial_arts
{
    cptr    name;
    cptr    desc;       /* A verbose attack description */
    int     min_level;  /* Minimum level to use */
    int     chance;     /* Chance of 'success' */
    int     dd;         /* Damage dice */
    int     ds;         /* Damage sides */
    int     effect;     /* Special effects */
};

typedef struct kamae kamae;

struct kamae
{
    cptr    desc;       /* A verbose kamae description */
    int     min_level;  /* Minimum level to use */
    cptr    info;
};

/* Mindcrafters */
typedef struct mind_type mind_type;
struct mind_type
{
    int     min_lev;
    int     mana_cost;
    int     fail;
    cptr    name;
};

typedef struct mind_power mind_power;
struct mind_power
{
    mind_type info[MAX_MIND_POWERS];
};

/* Imitator */

typedef struct monster_power monster_power;
struct monster_power
{
    int     level;
    int     smana;
    int     fail;
    int     manedam;
    int     manefail;
    int     use_stat;
    cptr    name;
};


/*
 * A structure to describe a building.
 * From Kamband
 */
typedef struct building_type building_type;

struct building_type
{
    char name[20];                  /* building name */
    char owner_name[25];            /* proprietor name */
    char owner_race[20];            /* proprietor race */

    char act_names[8][30];          /* action names */
    s32b member_costs[8];           /* Costs for class members of building */
    s32b other_costs[8];            /* Costs for nonguild members */
    char letters[8];                /* action letters */
    s16b actions[8];                /* action codes */
    s16b action_restr[8];           /* action restrictions */

    s16b member_class[MAX_CLASS];   /* which classes are part of guild */
    s16b member_race[MAX_RACES];    /* which races are part of guild */
    s16b member_realm[MAX_REALM+1]; /* which realms are part of guild */
};


/* Border */
typedef struct border_type border_type;
struct border_type
{
    s16b north[MAX_WID];
    s16b south[MAX_WID];
    s16b east[MAX_HGT];
    s16b west[MAX_HGT];
    s16b north_west;
    s16b north_east;
    s16b south_west;
    s16b south_east;
};


/*
 * A structure describing a wilderness area
 * with a terrain or a town
 */
typedef struct wilderness_type wilderness_type;
struct wilderness_type
{
    int         terrain;
    int         town;
    int         road;
    u32b        seed;
    s16b        level;
    byte        entrance;
};


/*
 * Sort-array element
 */
typedef struct tag_type tag_type;

struct tag_type
{
    int     tag;
    int     value;
};

typedef bool (*monster_hook_type)(int r_idx);


/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);


/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score
{
    char what[8];        /* Version info (string) */

    char pts[10];        /* Total Score (number) */

    char gold[10];        /* Total Gold (number) */

    char turns[10];        /* Turns Taken (number) */

    char day[10];        /* Time stamp (string) */

    char who[16];        /* Player Name (string) */

    char uid[8];        /* Player UID (number) */

    char sex[2];        /* Player Sex (string) */
    char p_r[3];        /* Player Race (number) */
    char p_c[3];        /* Player Class (number) */
    char p_a[3];        /* Player Seikaku (number) */

    char cur_lev[4];        /* Current Player Level (number) */
    char cur_dun[4];        /* Current Dungeon Level (number) */
    char max_lev[4];        /* Max Player Level (number) */
    char max_dun[4];        /* Max Dungeon Level (number) */

    char how[40];        /* Method of death (string) */
};


typedef struct
{
    s16b feat;    /* Feature tile */
    byte percent; /* Chance of type */
}
feat_prob;


/* A structure for the != dungeon types */
typedef struct dungeon_info_type dungeon_info_type;
struct dungeon_info_type {
    int  id;
    u32b name;        /* Name */
    u32b text;        /* Description */

    byte dy;
    byte dx;

    feat_prob floor[DUNGEON_FEAT_PROB_NUM]; /* Floor probability */
    feat_prob fill[DUNGEON_FEAT_PROB_NUM];  /* Cave wall probability */
    s16b outer_wall;                        /* Outer wall tile */
    s16b inner_wall;                        /* Inner wall tile */
    s16b stream1;                           /* stream tile */
    s16b stream2;                           /* stream tile */

    s16b mindepth;         /* Minimal depth */
    s16b maxdepth;         /* Maximal depth */
    byte min_plev;         /* Minimal plev needed to enter -- it's an anti-cheating mesure */
    s16b pit;
    s16b nest;
    byte mode;        /* Mode of combinaison of the monster flags */

    int min_m_alloc_level;    /* Minimal number of monsters per level */
    int max_m_alloc_chance;    /* There is a 1/max_m_alloc_chance chance per round of creating a new monster */

    u32b flags1;        /* Flags 1 */

    u32b mflags1;        /* The monster flags that are allowed */
    u32b mflags2;
    u32b mflags3;
    u32b mflags4;
    u32b mflags5;
    u32b mflags6;
    u32b mflags7;
    u32b mflags8;
    u32b mflags9;
    u32b mflagsr;

    char r_char[5];     /* Monster race allowed */
    int final_object;    /* The object you'll find at the bottom */
    int final_ego;       /* Ego type for final_object, or effect type for devices */
    int final_artifact;    /* The artifact you'll find at the bottom */
    int final_guardian;    /* The artifact's guardian. If an artifact is specified, then it's NEEDED */
    int initial_guardian;  /* Guarding the entrance */
    byte pantheon;       /* Pantheon associated with this dungeon */

    byte special_div;    /* % of monsters affected by the flags/races allowed, to add some variety */
    int tunnel_percent;
    int obj_great;
    int obj_good;
};


/*
 *  A structure type for entry of auto-picker/destroyer
 */
#define AUTOPICK_FLAG_SIZE 2

typedef struct {
    cptr name;          /* Items which have 'name' as part of its name match */
    cptr insc;          /* Items will be auto-inscribed as 'insc' */
    u32b flag[AUTOPICK_FLAG_SIZE]; /* Misc. keyword to be matched */
    byte action;        /* Auto-pickup or Destroy or Leave items */
    byte dice;          /* Weapons which have more than 'dice' dice match */
    byte bonus;         /* Items which have more than 'bonus' magical bonus match */
    byte level;
    byte weight;
    byte charges;
    int  value;
} autopick_type;


/*
 *  A structure type for the saved floor
 */
typedef struct
{
    s16b floor_id;        /* No recycle until 65536 IDs are all used */
    byte savefile_id;     /* ID for savefile (from 0 to MAX_SAVED_FLOOR) */
    s16b dun_level;
    s32b last_visit;      /* Time count of last visit. 0 for new floor. */
    u32b visit_mark;      /* Older has always smaller mark. */
    s16b upper_floor_id;  /* a floor connected with level teleportation */
    s16b lower_floor_id;  /* a floor connected with level tel. and trap door */
} saved_floor_type;


/*
 *  A structure type for terrain template of saving dungeon floor
 */
typedef struct
{
    u32b info;
    s16b feat;
    s16b mimic;
    s16b special;
    u16b occurrence;
} cave_template_type;


/*
 * A structure type for arena entry
 */
typedef struct
{
    s16b r_idx; /* Monster (0 means victory prizing) */
    byte tval;  /* tval of prize (0 means no prize) */
    int  sval;  /* sval of prize, or effect_e for devices */
} arena_type;


/*
 * A structure type for doors
 */
typedef struct
{
    s16b open;
    s16b broken;
    s16b closed;
    s16b locked[MAX_LJ_DOORS];
    s16b num_locked;
    s16b jammed[MAX_LJ_DOORS];
    s16b num_jammed;
} door_type;


/*
 *  A structure type for travel command
 */
#define TRAVEL_MODE_NORMAL   0
#define TRAVEL_MODE_AMMO     1
#define TRAVEL_MODE_AUTOPICK 2
typedef struct {
    int run;
    int cost[MAX_HGT][MAX_WID];
    int x;
    int y;
    int dir;
    int mode;
} travel_type;

typedef struct {
    int  id;
    cptr name;
    byte color;
    cptr desc;
    cptr parse;
    int  xtra;
} parse_tbl_t, *parse_tbl_ptr;

/*
 * A new spell system, and some half baked ideas for refactoring
 * Below here, is under construction, to be cleaned up later!
 */

typedef void (*ang_spell)(int cmd, variant *res);

typedef struct {
    int level;
    int cost;
    int fail;
    ang_spell fn;
} spell_info;

typedef void (*ang_spell_action)(const spell_info *spell);
typedef int (*calc_fail_fn)(int fail);

typedef struct {
int            stat;
spell_info    spell;
} power_info;

#define SPELL_FLAG_LEARNED   0x0001
#define SPELL_FLAG_FORGOTTEN 0x0002
#define SPELL_FLAG_NOTICED   0x0004
#define SPELL_FLAG_CONFIRM   0x0008
#define SPELL_FLAG_HIDE      0x0010

struct spell_stats_s
{
    int flags;
    int ct_cast;
    int ct_fail;
    int skill;      /* CASTER_GAIN_SKILL */
    int max_skill;
};

typedef struct spell_stats_s  spell_stats_t;
typedef spell_stats_t        *spell_stats_ptr;


/* TODO: This needs some work ... I just hacked this together for now.
   I'm shooting for a single unified interface for choosing, browsing
   and casting spells */

#define CASTER_ALLOW_DEC_MANA       0x0001 /* Wizardstaff and Mage Egos/Artifacts. cf equip.c for more details */
#define CASTER_GLOVE_ENCUMBRANCE    0x0002
#define CASTER_NO_SPELL_FAIL        0x0008
#define CASTER_USE_HP               0x0010
#define CASTER_GAIN_SKILL           0x0020
#define CASTER_USE_AU               0x0040 /* Leprechauns */
#define CASTER_SUPERCHARGE_MANA     0x0080
#define CASTER_USE_CONCENTRATION    0x0100 /* Sniper */

typedef struct {
    int max_wgt;    /* max weight before encumbrance */
    int weapon_pct; /* how much do melee weapons matter for encumbrance? */
    int enc_wgt;    /* how much weight over the max before 0sp */
} encumbrance_info;

typedef struct {
    cptr magic_desc;    /* spell, mindcraft, brutal power, ninjitsu, etc */
    int  min_fail;
    encumbrance_info encumbrance;
    int  which_stat;
    int  min_level;
    u32b options;
    ang_spell_action on_fail;    /* Hallucinate, Temporal Inversion, etc. */
    ang_spell_action on_cast;    /* Blood Knights take cuts, etc. */
} caster_info;

typedef void(*process_player_fn)(void);
typedef void(*move_player_fn)(void);
typedef void(*process_world_fn)(void);
typedef void(*move_monster_fn)(int m_idx);
typedef void(*calc_bonuses_fn)(void);
typedef void(*calc_innate_attacks_fn)(void);
typedef int(*calc_extra_weight_fn)(obj_p p);
typedef void(*birth_fn)(void);
typedef void(*calc_weapon_bonuses_fn)(object_type *o_ptr, weapon_info_t *info_ptr);
typedef void(*calc_shooter_bonuses_fn)(object_type *o_ptr, shooter_info_t *info_ptr);
typedef bool(*known_icky_fn)(object_type *o_ptr);
typedef caster_info*(*caster_info_fn)(void);
typedef int(*get_spells_fn)(spell_info* spells, int max);
typedef void(*gain_level_fn)(int new_level);
typedef void(*change_level_fn)(int old_level, int new_level);
typedef void(*character_dump_fn)(doc_ptr doc);
typedef void(*player_action_fn)(int energy_use);
typedef void(*flags_fn)(u32b flgs[OF_ARRAY_SIZE]);
typedef void(*stats_fn)(s16b stats[MAX_STATS]);
typedef void(*load_fn)(savefile_ptr file);
typedef void(*save_fn)(savefile_ptr file);
typedef int(*birth_ui_fn)(doc_ptr doc);

typedef struct {
    int                     id;
    int                     subid;
    cptr                    name;
    cptr                    subname;
    cptr                    desc;
    cptr                    subdesc;
    s16b                    stats[MAX_STATS];
    skills_t                base_skills;
    skills_t                extra_skills; /* Prorata every 10 levels */
    s16b                    life;
    s16b                    base_hp;
    s16b                    exp;
    byte                    pets;
    u32b                    flags;

    birth_fn                birth;          /* After py_birth() ... grant starting gear, etc */
    birth_ui_fn             birth_ui;       /* Used during py_birth() ... choose a subclass */ 
    process_player_fn       process_player; /* Called from process_player ... but player take 0 or more actions per call */
    player_action_fn        player_action;  /* Called once per player action, so long as the action consumes energy */
    move_player_fn          move_player;    /* Called every time the player actually moves */
    move_monster_fn         move_monster;    /* Called whenever a monster moves */
    calc_bonuses_fn         calc_bonuses;    /* Do flag related bonuses here ... */
    stats_fn                calc_stats;      /* ... and stat related stuff here */
    calc_weapon_bonuses_fn  calc_weapon_bonuses;
    calc_shooter_bonuses_fn calc_shooter_bonuses;
    calc_extra_weight_fn    calc_extra_weight;
    known_icky_fn           known_icky_object;
    caster_info_fn          caster_info;
    get_spells_fn           get_spells;
    get_spells_fn           get_powers;
    gain_level_fn           gain_level; /* Only ever called when a new max level is achieved */
    character_dump_fn       character_dump;
    flags_fn                get_flags;
    load_fn                 load_player;
    save_fn                 save_player;
    obj_p                   destroy_object;
    obj_f                   get_object;
    inv_ptr                 bonus_pack;
} class_t, *class_ptr;

struct equip_template_s;

typedef struct {
    int                     id;
    int                     subid;
    cptr                    name;
    cptr                    subname;
    cptr                    desc;
    cptr                    subdesc;
    s16b                    stats[MAX_STATS];
    skills_t                skills;
    skills_t                extra_skills; /* Prorata every 10 levels (Monster Races) */
    s16b                    life;
    s16b                    base_hp;
    s16b                    exp;
    s16b                    infra;
    birth_fn                birth; /* Note: If specified, give starting food and light as well
                                      See: py_birth_food() and py_birth_light() for defaults */
    calc_bonuses_fn         calc_bonuses;    /* Do flag related bonuses here ... */
    stats_fn                calc_stats;      /* ... and stat related stuff here */
    calc_weapon_bonuses_fn  calc_weapon_bonuses;
    calc_shooter_bonuses_fn calc_shooter_bonuses;
    calc_innate_attacks_fn  calc_innate_attacks;
    calc_extra_weight_fn    calc_extra_weight;
    caster_info_fn          caster_info;
    get_spells_fn           get_spells;
    get_spells_fn           get_powers;
    gain_level_fn           gain_level;
    change_level_fn         change_level;
    character_dump_fn       character_dump;
    flags_fn                get_flags;
    u32b                    flags;
    bool                    mimic;
    struct equip_template_s *equip_template;
    int                     boss_r_idx;
    player_action_fn        player_action;  /* Called once per player action, so long as the action consumes energy */
    move_player_fn          move_player;
    process_world_fn        process_world;  /* Called every 10 game turns */
    load_fn                 load_player;
    save_fn                 save_player;
    object_p                destroy_object;
    s16b                    pseudo_class_idx; /* For the "Monster" class ... */
    s16b                    shop_adjust;
    inv_ptr                 bonus_pack;
    inv_ptr                 bonus_pack2;
} race_t, *race_ptr;

typedef struct {
    int  type;
    s16b tag;
    int  hand;
} equip_slot_t;

typedef struct equip_template_s {
    int          max;
    u32b         name;
    equip_slot_t slots[EQUIP_MAX + 1];
} equip_template_t, *equip_template_ptr;

typedef struct {
    int                     id;
    cptr                    name;
    cptr                    desc;
    s16b                    stats[MAX_STATS];
    skills_t                skills;
    s16b                    life;
    s16b                    exp;
    s16b                    attack;
    s16b                    breath;
    s16b                    spell_stat;
} dragon_realm_t, *dragon_realm_ptr;

struct device_effect_info_s
{
    int      type;
    int      level;
    int      cost;
    int      rarity;
    int      max_depth;
    int      difficulty_base;
    int      difficulty_xtra;
    int      flags;
    counts_t counts;
    int      prob;
};

typedef struct device_effect_info_s  device_effect_info_t;
typedef struct device_effect_info_s *device_effect_info_ptr;

struct personality_s
{
    int             id;
    cptr            name;
    cptr            desc;
    s16b            stats[MAX_STATS];
    skills_t        skills;
    s16b            life;
    s16b            exp;
    int             flags;
    birth_fn        birth;
    calc_bonuses_fn calc_bonuses;
    calc_weapon_bonuses_fn  calc_weapon_bonuses;
    flags_fn        get_flags;
};

typedef struct personality_s personality_t, *personality_ptr;

typedef struct pantheon_type pantheon_type;

struct pantheon_type
{
    byte id;
    u32b flag;
    u32b flag2; /* for monsters associated with the pantheon,
                 * but not part of the pantheon proper */
    char name[20];
    char short_name[5];
    char plural[20];
};
