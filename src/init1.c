/* File: init1.c */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Initialization (part 1) -BEN- */

#include "angband.h"
#include "rooms.h"

#include <assert.h>
/*
 * This file is used to initialize various variables and arrays for the
 * Angband game. Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass. Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * Note that if "ALLOW_TEMPLATES" is not defined, then a lot of the code
 * in this file is compiled out, and the game will not run unless valid
 * "binary template files" already exist in "lib/data". Thus, one can
 * compile Angband with ALLOW_TEMPLATES defined, run once to create the
 * "*.raw" files in "lib/data", and then quit, and recompile without
 * defining ALLOW_TEMPLATES, which will both save 20K and prevent people
 * from changing the ascii template files in potentially dangerous ways.
 *
 * The code could actually be removed and placed into a "stand-alone"
 * program, but that feels a little silly, especially considering some
 * of the platforms that we currently support.
 */

#ifdef ALLOW_TEMPLATES

#include "init.h"


/*** Helper arrays for parsing ascii template files ***/
static int _get_gf_type(cptr which)
{
    gf_info_ptr gf = gf_parse_name(which);
    if (gf) return gf->id;
    return 0;
}

/*
 * Monster Blow Methods
 */
static cptr r_info_blow_method[] =
{
    "",
    "HIT",
    "TOUCH",
    "PUNCH",
    "KICK",
    "CLAW",
    "BITE",
    "STING",
    "SLASH",
    "BUTT",
    "CRUSH",
    "ENGULF",
    "CHARGE",
    "CRAWL",
    "DROOL",
    "SPIT",
    "EXPLODE",
    "GAZE",
    "WAIL",
    "SPORE",
    "XXX4",
    "BEG",
    "INSULT",
    "MOAN",
    "SHOW",
    "SHOOT",
    NULL
};

static int _get_r_blow_method(cptr name)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!r_info_blow_method[i]) return 0;
        if (streq(r_info_blow_method[i], name)) return i;
    }
}

static int _get_r_blow_effect(cptr which)
{
    int i;
    struct { cptr name; int id; } _table[] = {
        {"HURT", RBE_HURT},
        {"SHATTER", RBE_SHATTER},
        {"EAT_GOLD", RBE_EAT_GOLD},
        {"EAT_ITEM", RBE_EAT_ITEM},
        {"EAT_FOOD", RBE_EAT_FOOD},
        {"EAT_LITE", RBE_EAT_LITE},
        {"LOSE_STR", RBE_LOSE_STR},
        {"LOSE_INT", RBE_LOSE_INT},
        {"LOSE_WIS", RBE_LOSE_WIS},
        {"LOSE_DEX", RBE_LOSE_DEX},
        {"LOSE_CON", RBE_LOSE_CON},
        {"LOSE_CHR", RBE_LOSE_CHR},
        {"LOSE_ALL", RBE_LOSE_ALL},
        {"DISEASE", RBE_DISEASE},
        {"DRAIN_CHARGES", RBE_DRAIN_CHARGES},
        {"DRAIN_EXP", RBE_DRAIN_EXP},
        {"VAMP", RBE_VAMP},
        {"CUT", RBE_CUT},
        {0}};
    for (i = 0;; i++)
    {
        if (!_table[i].name) break;
        if (streq(_table[i].name, which)) return _table[i].id;
    }
    return _get_gf_type(which);
}


/*
 * Feature info flags
 */
static cptr f_info_flags[] =
{
    "LOS",
    "PROJECT",
    "MOVE",
    "PLACE",
    "DROP",
    "SECRET",
    "NOTICE",
    "REMEMBER",
    "OPEN",
    "CLOSE",
    "BASH",
    "SPIKE",
    "DISARM",
    "STORE",
    "TUNNEL",
    "MAY_HAVE_GOLD",
    "HAS_GOLD",
    "HAS_ITEM",
    "DOOR",
    "TRAP",
    "STAIRS",
    "GLYPH",
    "LESS",
    "MORE",
    "AVOID_RUN",
    "FLOOR",
    "WALL",
    "PERMANENT",
    "XXX00",
    "XXX01",
    "XXX02",
    "HIT_TRAP",

    "BRIDGE",
    "RIVER",
    "LAKE",
    "BRIDGED",
    "COVERED",
    "GLOW",
    "ENSECRET",
    "WATER",
    "LAVA",
    "SHALLOW",
    "DEEP",
    "FILLED",
    "HURT_ROCK",
    "HURT_FIRE",
    "HURT_COLD",
    "HURT_ACID",
    "ICE",
    "ACID",
    "OIL",
    "XXX04",
    "CAN_CLIMB",
    "CAN_FLY",
    "CAN_SWIM",
    "CAN_PASS",
    "CAN_OOZE",
    "CAN_DIG",
    "HIDE_ITEM",
    "HIDE_SNEAK",
    "HIDE_SWIM",
    "HIDE_DIG",
    "KILL_HUGE",
    "KILL_MOVE",

    "PICK_TRAP",
    "PICK_DOOR",
    "ALLOC",
    "CHEST",
    "DROP_1D2",
    "DROP_2D2",
    "DROP_GOOD",
    "DROP_GREAT",
    "HURT_POIS",
    "HURT_ELEC",
    "HURT_WATER",
    "HURT_BWATER",
    "USE_FEAT",
    "GET_FEAT",
    "GROUND",
    "OUTSIDE",
    "EASY_HIDE",
    "EASY_CLIMB",
    "MUST_CLIMB",
    "TREE",
    "NEED_TREE",
    "BLOOD",
    "DUST",
    "SLIME",
    "PLANT",
    "XXX2",
    "INSTANT",
    "EXPLODE",
    "TIMED",
    "ERUPT",
    "STRIKE",
    "SPREAD",

    "SPECIAL",
    "HURT_DISI",
    "QUEST_ENTER",
    "QUEST_EXIT",
    "QUEST",
    "SHAFT",
    "MOUNTAIN",
    "BLDG",
    "MON_TRAP",
    "PATTERN",
    "TOWN",
    "ENTRANCE",
    "MIRROR",
    "UNPERM",
    "TELEPORTABLE",
    "CONVERT",
    "GLASS",
    "ROGUE_TRAP_1",
    "ROGUE_TRAP_2",
    "ROGUE_TRAP_3",
    "WEB",
    "SEMI_PUN",
    "SHADOW_ZAP",
};


/*
 * Monster race flags
 */
static cptr r_info_flags1[] =
{
    "UNIQUE",
    "FIXED_UNIQUE",
    "MALE",
    "FEMALE",
    "CHAR_CLEAR",
    "SHAPECHANGER",
    "ATTR_CLEAR",
    "ATTR_MULTI",
    "FORCE_DEPTH",
    "FORCE_MAXHP",
    "FORCE_SLEEP",
    "FORCE_EXTRA",
    "ATTR_SEMIRAND",
    "FRIENDS",
    "ESCORT",
    "NO_SUMMON",
    "NEVER_BLOW",
    "NEVER_MOVE",
    "RAND_25",
    "RAND_50",
    "ONLY_GOLD",
    "ONLY_ITEM",
    "DROP_60",
    "DROP_90",
    "DROP_1D2",
    "DROP_2D2",
    "DROP_3D2",
    "DROP_4D2",
    "DROP_GOOD",
    "DROP_GREAT",
    "TRUMP",
    "NO_QUEST"
};

/*
 * Monster race flags
 */
static cptr r_info_flags2[] =
{
    "STUPID",
    "SMART",
    "CAN_SPEAK",
    "REFLECTING",
    "INVISIBLE",
    "COLD_BLOOD",
    "EMPTY_MIND",
    "WEIRD_MIND",
    "MULTIPLY",
    "REGENERATE",
    "CHAR_MULTI",
    "ATTR_ANY",
    "FOREST",
    "ELDRITCH_HORROR",
    "AURA_FIRE",
    "AURA_ELEC",
    "OPEN_DOOR",
    "BASH_DOOR",
    "PASS_WALL",
    "KILL_WALL",
    "MOVE_BODY",
    "KILL_BODY",
    "TAKE_ITEM",
    "KILL_ITEM",
    "AURA_REVENGE",
    "THIEF",
    "AURA_FEAR",
    "CAMELOT",
    "KNIGHT",
    "SOUTHERING",
    "HUMAN",
    "QUANTUM"
};

/*
 * Monster race flags
 */
static cptr r_info_flags3[] =
{
    "ORC",
    "TROLL",
    "GIANT",
    "DRAGON",
    "DEMON",
    "UNDEAD",
    "EVIL",
    "ANIMAL",
    "AMBERITE",
    "GOOD",
    "AURA_COLD",
    "NONLIVING",
    "HURT_LITE",
    "HURT_ROCK",
    "HURT_FIRE",
    "HURT_COLD",
    "OLYMPIAN",
    "EGYPTIAN",
    "EGYPTIAN2",
    "OLYMPIAN2",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "COMPOST",
    "XXX",
    "XXX",
    "CLEAR_HEAD",
    "NO_FEAR",
    "NO_STUN",
    "NO_CONF",
    "NO_SLEEP"
};

/*
 * Monster race flags
 */
static cptr r_info_flags7[] =
{
    "AQUATIC",
    "CAN_SWIM",
    "CAN_FLY",
    "FRIENDLY",
    "NAZGUL",
    "UNIQUE2",
    "RIDING",
    "KAGE",
    "HAS_LITE_1",
    "SELF_LITE_1",
    "HAS_LITE_2",
    "SELF_LITE_2",
    "GUARDIAN",
    "CHAMELEON",
    "KILL_EXP",
    "TANUKI",
    "HAS_DARK_1",
    "SELF_DARK_1",
    "HAS_DARK_2",
    "SELF_DARK_2",
    "CAN_CLIMB",
    "RANGED_MELEE",
    "NASTY_GLYPH",
    "SILVER",
    "XXX7X24",
    "XXX7X25",
    "XXX7X26",
    "XXX7X27",
    "XXX7X28",
    "XXX7X29",
    "XXX7X30",
    "XXX7X31",
};

/*
 * Monster race flags
 */
static cptr r_info_flags8[] =
{
    "WILD_ONLY",
    "WILD_TOWN",
    "XXX8X02",
    "WILD_SHORE",
    "WILD_OCEAN",
    "WILD_WASTE",
    "WILD_WOOD",
    "WILD_VOLCANO",
    "XXX8X08",
    "WILD_MOUNTAIN",
    "WILD_GRASS",
    "XXX8X11",
    "XXX8X12",
    "XXX8X13",
    "XXX8X14",
    "XXX8X15",
    "XXX8X16",
    "XXX8X17",
    "XXX8X18",
    "XXX8X19",
    "XXX8X20",
    "XXX8X21",
    "XXX8X22",
    "XXX8X23",
    "XXX8X24",
    "XXX8X25",
    "XXX8X26",
    "XXX8X27",
    "XXX8X28",
    "XXX8X29",
    "WILD_SWAMP",    /* ToDo: Implement Swamp */
    "WILD_ALL",
};


/*
 * Monster race flags - Drops
 */
static cptr r_info_flags9[] =
{
    "DROP_CORPSE",
    "DROP_SKELETON",
    "POS_GAIN_AC",
    "POS_TELEPATHY",
    "POS_SEE_INVIS",
    "POS_HOLD_LIFE",
    "POS_SUST_STR",
    "POS_SUST_INT",
    "POS_SUST_WIS",
    "POS_SUST_DEX",
    "POS_SUST_CON",
    "POS_SUST_CHR",
    "XXX13",
    "XXX14",
    "XXX15",
    "XXX16",
    "XXX17",
    "XXX18",
    "XXX19",
    "XXX20",
    "XXX21",
    "XXX22",
    "XXX23",
    "XXX24",
    "POS_BACKSTAB",
    "XXX26",
    "XXX27",
    "XXX28",
    "XXX29",
    "XXX30",
    "XXX31",
    "DEPRECATED",
};

static cptr r_drop_themes[R_DROP_MAX] =
{
    "NONE",

    "DROP_WARRIOR",
    "DROP_WARRIOR_SHOOT",
    "DROP_ARCHER",
    "DROP_MAGE",
    "DROP_PRIEST",
    "DROP_PRIEST_EVIL",
    "DROP_PALADIN",
    "DROP_PALADIN_EVIL",
    "DROP_SAMURAI",
    "DROP_NINJA",
    "DROP_ROGUE",

    "DROP_HOBBIT",
    "DROP_DWARF",

    "DROP_JUNK",
};

/*
 * Monster race flags - Resistances
 */
static cptr r_info_flagsr[] =
{
    "IM_ACID",
    "IM_ELEC",
    "IM_FIRE",
    "IM_COLD",
    "IM_POIS",
    "RES_LITE",
    "RES_DARK",
    "RES_NETH",
    "RES_WATE",
    "RES_PLAS",
    "RES_SHAR",
    "RES_SOUN",
    "RES_CHAO",
    "RES_NEXU",
    "RES_DISE",
    "RES_WALL",
    "RES_INER",
    "RES_TIME",
    "RES_GRAV",
    "RES_ALL",
    "RES_TELE",
    "XXX",
    "RES_ACID",
    "RES_ELEC",
    "RES_FIRE",
    "RES_COLD",
    "RES_POIS",
    "XXX27",
    "XXX28",
    "XXX29",
    "XXX30",
    "XXX31",
};


/*
 * Object flags (Keep in sync with obj_flags_e in defines.h)
 */
static cptr k_info_flags[OF_COUNT] =
{
    "HIDE_TYPE",
    "SHOW_MODS",
    "FULL_NAME",
    "FIXED_FLAVOR",

    /* Stats */
    "STR",
    "INT",
    "WIS",
    "DEX",
    "CON",
    "CHR",
    "DEC_STR",
    "DEC_INT",
    "DEC_WIS",
    "DEC_DEX",
    "DEC_CON",
    "DEC_CHR",
    "SUST_STR",
    "SUST_INT",
    "SUST_WIS",
    "SUST_DEX",
    "SUST_CON",
    "SUST_CHR",

    /* Skills/Bonuses */
    "SPEED",
    "STEALTH",
    "SEARCH",
    "INFRA",
    "TUNNEL",
    "MAGIC_MASTERY",
    "MAGIC_RESISTANCE",
    "SPELL_POWER",
    "SPELL_CAP",
    "DEVICE_POWER",
    "LIFE",

    "DEC_SPEED",
    "DEC_STEALTH",
    "DEC_MAGIC_MASTERY",
    "DEC_SPELL_POWER",
    "DEC_SPELL_CAP",
    "DEC_LIFE",

    /* Resists */
    "RES_ACID",
    "RES_ELEC",
    "RES_FIRE",
    "RES_COLD",
    "RES_POIS",
    "RES_LITE",
    "RES_DARK",
    "RES_CONF",
    "RES_NETHER",
    "RES_NEXUS",
    "RES_SOUND",
    "RES_SHARDS",
    "RES_CHAOS",
    "RES_DISEN",
    "RES_TIME",
    "RES_BLIND",
    "RES_FEAR",

    "IM_ACID",
    "IM_ELEC",
    "IM_FIRE",
    "IM_COLD",
    "IM_POIS",
    "IM_LITE",
    "IM_DARK",
    "IM_NETHER",
    "IM_BLIND",
    "IM_FEAR",

    "VULN_ACID",
    "VULN_ELEC",
    "VULN_FIRE",
    "VULN_COLD",
    "VULN_POIS",
    "VULN_LITE",
    "VULN_DARK",
    "VULN_CONF",
    "VULN_NETHER",
    "VULN_NEXUS",
    "VULN_SOUND",
    "VULN_SHARDS",
    "VULN_CHAOS",
    "VULN_DISEN",
    "VULN_BLIND",
    "VULN_FEAR",

    /* Abilities */
    "FREE_ACT",
    "SEE_INVIS",
    "REGEN",
    "HOLD_LIFE",
    "REFLECT",
    "LEVITATION",
    "SLOW_DIGEST",
    "WARNING",
    "NO_MAGIC",
    "NO_SUMMON",
    "NO_TELE",
    "NO_ENCHANT",
    "NO_REMOVE",
    "EASY_SPELL",
    "DEC_MANA",
    "LITE",
    "DARKNESS",
    "LORE1",
    "LORE2",

    "ACTIVATE",

    "IGNORE_ACID",
    "IGNORE_ELEC",
    "IGNORE_FIRE",
    "IGNORE_COLD",

    /* Auras */
    "AURA_ELEC",
    "AURA_FIRE",
    "AURA_COLD",
    "AURA_SHARDS",
    "AURA_REVENGE",
    "AURA_FEAR",

    /* Telepathy */
    "TELEPATHY",
    "ESP_EVIL",
    "ESP_GOOD",
    "ESP_NONLIVING",
	"ESP_LIVING",
    "ESP_UNIQUE",
    "ESP_DRAGON",
    "ESP_DEMON",
    "ESP_UNDEAD",
    "ESP_ANIMAL",
    "ESP_HUMAN",
    "ESP_ORC",
    "ESP_TROLL",
    "ESP_GIANT",

    /* Weapons */
    "SLAY_EVIL",
    "SLAY_GOOD",
    "SLAY_LIVING",
    "SLAY_DRAGON",
    "SLAY_DEMON",
    "SLAY_UNDEAD",
    "SLAY_ANIMAL",
    "SLAY_HUMAN",
    "SLAY_ORC",
    "SLAY_TROLL",
    "SLAY_GIANT",

    "KILL_EVIL",
	"KILL_GOOD",
	"KILL_LIVING",
    "KILL_DRAGON",
    "KILL_DEMON",
    "KILL_UNDEAD",
    "KILL_ANIMAL",
    "KILL_HUMAN",
    "KILL_ORC",
    "KILL_TROLL",
    "KILL_GIANT",

    "BRAND_ACID",
    "BRAND_ELEC",
    "BRAND_FIRE",
    "BRAND_COLD",
    "BRAND_POIS",
    "BRAND_CHAOS",
    "BRAND_VAMP",
    "BRAND_WILD",
    "BRAND_ORDER",
    "BRAND_MANA",
    "VORPAL",
    "VORPAL2",
    "IMPACT",
    "STUN",

    "BLESSED",
    "RIDING",
    "THROWING",

    "BLOWS",
    "DEC_BLOWS",
    "WEAPONMASTERY",
    "DUAL_WIELDING",

    /* Bows */
    "XTRA_MIGHT",
    "XTRA_SHOTS",

    /* Curses */
    "DRAIN_EXP",
    "TELEPORT",
    "AGGRAVATE",
    "TY_CURSE",

    /* Plural */
    "PLURAL",

    /* Ignore Invulnerability Spheres */
    "IGNORE_INVULN",

    /* Night Vision */
    "NIGHT_VISION",

    "BRAND_DARK",
};


static cptr k_info_gen_flags[] =
{
    "INSTA_ART",
    "QUESTITEM",
    "XTRA_POWER",
    "ONE_SUSTAIN",
    "XTRA_RES_OR_POWER",
    "XTRA_H_RES",
    "XTRA_E_RES",
    "XTRA_L_RES",
    "XTRA_D_RES",
    "XTRA_RES",
    "CURSED",
    "HEAVY_CURSE",
    "PERMA_CURSE",
    "RANDOM_CURSE0",
    "RANDOM_CURSE1",
    "RANDOM_CURSE2",
    "AWARE",
    "TOWN",
    "FIXED_ART",
    "FIXED_ACT",
    "NO_SHUFFLE",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
    "XXX",
};


/*
 * Dungeon flags
 */
static cptr d_info_flags1[] =
{
    "WINNER",
    "MAZE",
    "SMALLEST",
    "BEGINNER",
    "BIG",
    "NO_DOORS",
    "WATER_RIVER",
    "LAVA_RIVER",
    "CURTAIN",
    "GLASS_DOOR",
    "CAVE",
    "CAVERN",
    "RANDOM",
    "COFFEE",
    "XXX",
    "XXX",
    "FORGET",
    "LAKE_WATER",
    "LAKE_LAVA",
    "LAKE_RUBBLE",
    "LAKE_TREE",
    "NO_VAULT",
    "ARENA",
    "DESTROY",
    "GLASS_ROOM",
    "NO_CAVE",
    "NO_MAGIC",
    "NO_MELEE",
    "CHAMELEON",
    "DARKNESS",
    "ALL_SHAFTS",
    "XXX"
};

/*
 * Add a text to the text-storage and store offset to it.
 *
 * Returns FALSE when there isn't enough space available to store
 * the text.
 */
static bool add_text(u32b *offset, header *head, cptr buf, bool normal_text)
{
    /* Hack -- Verify space */
    if (head->text_size + strlen(buf) + 8 > FAKE_TEXT_SIZE)
        return (FALSE);

    /* New text? */
    if (*offset == 0)
    {
        /* Advance and save the text index */
        *offset = ++head->text_size;
    }

    /* Additional text */
    else if (normal_text)
    {
        /*
         * If neither the end of the last line nor
         * the beginning of current line is not a space,
         * fill up a space as a correct separator of two words.
         */
        if (head->text_size > 0 &&
            (*(head->text_ptr + head->text_size - 1) != ' ') &&
            (buf[0] != ' ')
            )
        {
            /* Append a space */
            *(head->text_ptr + head->text_size) = ' ';

            /* Advance the index */
            head->text_size++;
        }
    }

    /* Append chars to the text */
    strcpy(head->text_ptr + head->text_size, buf);

    /* Advance the index */
    head->text_size += strlen(buf);

    /* Success */
    return (TRUE);
}


/*
 * Add a name to the name-storage and return an offset to it.
 *
 * Returns FALSE when there isn't enough space available to store
 * the name.
 */
static bool add_name(u32b *offset, header *head, cptr buf)
{
    /* Hack -- Verify space */
    if (head->name_size + strlen(buf) + 8 > FAKE_NAME_SIZE)
        return (FALSE);

    /* New name? */
    if (*offset == 0)
    {
        /* Advance and save the name index */
        *offset = ++head->name_size;
    }

    /* Append chars to the names */
    strcpy(head->name_ptr + head->name_size, buf);

    /* Advance the index */
    head->name_size += strlen(buf);

    /* Success */
    return (TRUE);
}


/*
 * Add a tag to the tag-storage and return an offset to it.
 *
 * Returns FALSE when there isn't enough space available to store
 * the name.
 */
static bool add_tag(s16b *offset, header *head, cptr buf)
{
    u32b i;

    /* Search for an existing (fake) tag */
    for (i = 1; i < head->tag_size; i += strlen(&head->tag_ptr[i]) + 1)
    {
        /* Found it */
        if (streq(&head->tag_ptr[i], buf)) break;
    }

    /* There was no existing tag */
    if (i >= head->tag_size)
    {
        /* Hack -- Verify space */
        if (head->tag_size + strlen(buf) + 8 > FAKE_TAG_SIZE)
            return FALSE;

        /* Append chars to the tags */
        strcpy(head->tag_ptr + head->tag_size, buf);

        /* Point the new tag */
        i = head->tag_size;

        /* Advance the index */
        head->tag_size += strlen(buf) + 1;
    }

    /* Return offset of the tag */
    *offset = (s16b)i;

    /* Success */
    return TRUE;
}


/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
byte color_char_to_attr(char c)
{
    switch (c)
    {
        case 'd': return (TERM_DARK);
        case 'w': return (TERM_WHITE);
        case 's': return (TERM_SLATE);
        case 'o': return (TERM_ORANGE);
        case 'r': return (TERM_RED);
        case 'g': return (TERM_GREEN);
        case 'b': return (TERM_BLUE);
        case 'u': return (TERM_UMBER);

        case 'D': return (TERM_L_DARK);
        case 'W': return (TERM_L_WHITE);
        case 'v': return (TERM_VIOLET);
        case 'y': return (TERM_YELLOW);
        case 'R': return (TERM_L_RED);
        case 'G': return (TERM_L_GREEN);
        case 'B': return (TERM_L_BLUE);
        case 'U': return (TERM_L_UMBER);
    }

    return (255);
}



/*** Initialize from ascii template files ***/


/*
 * Initialize an "*_info" array, by parsing an ascii "template" file
 */
errr init_info_txt(FILE *fp, char *buf, header *head,
           parse_info_txt_func parse_info_txt_line)
{
    errr err;

    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = 0;


    /* Prepare the "fake" stuff */
    head->name_size = 0;
    head->text_size = 0;
    head->tag_size = 0;

    /* Parse */
    while (0 == my_fgets(fp, buf, 1024))
    {
        /* Advance the line number */
        error_line++;

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (PARSE_ERROR_GENERIC);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V')
        {
            /* ignore */
            continue;
        }

        /* Mega Hack -- Calculate Check Sum */
        if (buf[0] != 'N' && buf[0] != 'D')
        {
            int i;
            for (i = 0; buf[i]; i++)
            {
                head->v_extra += (byte)buf[i];
                head->v_extra ^= (1 << (i % 8));
            }
        }

        /* Parse the line */
        if ((err = (*parse_info_txt_line)(buf, head)) != 0)
            return (err);
    }


    /* Complete the "name" and "text" sizes */
    if (head->name_size) head->name_size++;
    if (head->text_size) head->text_size++;

    /* Success */
    return (0);
}

static bool _is_digit(char c)
{
    if ('0' <= c && c <= '9')
        return TRUE;
    return FALSE;
}

static bool _is_numeric(const char *token) /* [0-9]+ */
{
    const char *c = token;
    if (!*c) return FALSE;
    if (!_is_digit(*c)) return FALSE;
    for (c++; *c; c++)
    {
        if (!_is_digit(*c)) return FALSE;
    }
    return TRUE;
}

static bool _is_d_char(const char *token)
{
    int i;
    if (strlen(token) != 1) return FALSE;
    for (i = 0; i < max_r_idx; i++)
    {
        if (token[0] == r_info[i].d_char) return TRUE;
    }
    return FALSE;
}

/* this is almost strip_name_aux ... but I want to support
 * partial matches (e.g. EGO(speed) to match 'of Speed' and
 * EGO(pattern) to match '(Pattern)' (Note: EGO((Pattern)) 
 * would sadly confuse our parser ...) Also, MON(raal's)
 * rather than MON(raal's tome of destruction) will keep
 * my fingers happy. */
static void _prep_name_aux(char *dest, const char *src)
{
    char *t;

    while (*src == ' ' || *src == '&' || *src == '[' || *src == '(')
        src++;

    for (t = dest; *src; src++)
    {
        char c = *src;
        if (c != '~' && c != ']' && c != ')')
        {
            if (isupper(c)) c = tolower(c);
            *t++ = c;
        }
    }

    *t = '\0';
}

static void _prep_name(char *dest, const char *src)
{
    strcpy(dest, "^"); /* OBJ(^dagger) matches ^dagger$ not ^broken dagger$ */
    _prep_name_aux(dest + strlen(dest), src);
    strcat(dest, "$"); /* MON(ent$) matches ^ent$ not ^agent of the black market$ */
}

/* Same order as summon_specific_e in defines.h
   These are legal monster types for the MON() directive when
   specifying room_grid_t
 */
static parse_tbl_t _summon_type_tbl[] = {
    { SUMMON_MONSTER, "Monsters", TERM_WHITE, "", "MONSTER", 10 },
    { SUMMON_ANT, "Ants", TERM_WHITE, "", "ANT", 10 },
    { SUMMON_SPIDER, "Spiders", TERM_WHITE, "", "SPIDER", 10 },
    { SUMMON_HOUND, "Hounds", TERM_WHITE, "", "HOUND", 15 },
    { SUMMON_HYDRA, "Hydras", TERM_WHITE, "", "HYDRA", 20 },
    { SUMMON_ANGEL, "Angels", TERM_WHITE, "", "ANGEL", 60 },
    { SUMMON_DEMON, "Demons", TERM_WHITE, "", "DEMON", 40 },
    { SUMMON_UNDEAD, "Undead", TERM_WHITE, "", "UNDEAD", 40 },
    { SUMMON_DRAGON, "Dragons", TERM_WHITE, "", "DRAGON", 40 },
    { SUMMON_HI_UNDEAD, "Mighty Undead", TERM_L_DARK, "", "HI_UNDEAD", 60 },
    { SUMMON_HI_DRAGON, "Ancient Dragons", TERM_RED, "", "HI_DRAGON", 70 },
    { SUMMON_HI_DEMON, "Foul Demons", TERM_RED, "", "HI_DEMON", 66 },
    { SUMMON_AMBERITE, "Amberites", TERM_WHITE, "", "AMBERITE", 100 },
    { SUMMON_UNIQUE, "Uniques", TERM_VIOLET, "", "UNIQUE", 125 },
    { SUMMON_BIZARRE1, "Mold", TERM_WHITE, "", "BIZARRE1", 15 },
    { SUMMON_BIZARRE2, "Bats", TERM_WHITE, "", "BIZARRE2", 5 },
    { SUMMON_BIZARRE3, "Quylthulgs", TERM_WHITE, "", "BIZARRE3", 20 },
    { SUMMON_BIZARRE4, "Vortices", TERM_WHITE, "", "BIZARRE4", 20 },
    { SUMMON_BIZARRE5, "Creeping Coins", TERM_WHITE, "", "BIZARRE5", 5 },
    { SUMMON_BIZARRE6, "Mimics", TERM_WHITE, "", "BIZARRE6", 10 },
    { SUMMON_CYBER, "Cyberdemons", TERM_WHITE, "", "CYBER", 80 },
    { SUMMON_KIN, "Kin", TERM_WHITE, "", "KIN", 35 },
    { SUMMON_DAWN, "Warriors of the Dawn", TERM_WHITE, "", "DAWN", 30 },
    { SUMMON_ANIMAL, "Animals", TERM_WHITE, "", "ANIMAL", 15 },
    { SUMMON_ANIMAL_RANGER, "Animals", TERM_WHITE, "", "ANIMAL_RANGER", 15 },
    { SUMMON_PHANTOM, "Phantom Beasts", TERM_WHITE, "", "PHANTOM", 20 },
    { SUMMON_BLUE_HORROR, "Blue Horrors", TERM_WHITE, "", "BLUE_HORROR", 5 },
    { SUMMON_LIVING, "Life", TERM_WHITE, "", "LIVING", 10 },
    { SUMMON_HI_DRAGON_LIVING, "Dragons", TERM_WHITE, "", "HI_DRAGON_LIVING", 70 },
    { SUMMON_GOLEM, "Golems", TERM_WHITE, "", "GOLEM", 20 },
    { SUMMON_ELEMENTAL, "Elementals", TERM_WHITE, "", "ELEMENTAL", 15 },
    { SUMMON_VORTEX, "Vortices", TERM_WHITE, "", "VORTEX", 20 },
    { SUMMON_HYBRID, "Abominations", TERM_WHITE, "", "HYBRID", 15 },
    { SUMMON_BIRD, "Birds", TERM_WHITE, "", "BIRD", 20 },
    { SUMMON_KAMIKAZE, "Fanatics", TERM_WHITE, "", "KAMIKAZE", 10 },
    { SUMMON_KAMIKAZE_LIVING, "Fanatics", TERM_WHITE, "", "KAMIKAZE_LIVING", 15 },
    { SUMMON_MANES, "Manes", TERM_WHITE, "", "MANES", 5 },
    { SUMMON_LOUSE, "Lice", TERM_WHITE, "", "LOUSE", 1 },
    { SUMMON_GUARDIAN, "Dungeon Guardians", TERM_VIOLET, "", "GUARDIAN", 150 },
    { SUMMON_KNIGHT, "Knights", TERM_WHITE, "", "KNIGHT", 20 },
    { SUMMON_EAGLE, "Eagles", TERM_WHITE, "", "EAGLE", 25 },
    { SUMMON_PIRANHA, "Piranhas", TERM_WHITE, "", "PIRANHA", 5 },
    { SUMMON_ARMAGE_GOOD, "Holy Monsters", TERM_WHITE, "", "ARMAGE_GOOD", 40 },
    { SUMMON_ARMAGE_EVIL, "Foul Monsters", TERM_WHITE, "", "ARMAGE_EVIL", 40 },
    { SUMMON_SOFTWARE_BUG, "Software Bugs", TERM_WHITE, "", "SOFTWARE_BUG", 1 },
    { SUMMON_PANTHEON, "Gods", TERM_WHITE, "", "PANTHEON", 150 },
    { SUMMON_RAT, "Rats", TERM_WHITE, "", "RAT", 2 },
    { SUMMON_BAT, "Bats", TERM_WHITE, "", "BAT", 5 },
    { SUMMON_WOLF, "Wolves", TERM_WHITE, "", "WOLF", 7 },
    { SUMMON_DREAD, "Dread", TERM_WHITE, "", "DREAD", 20 },
    { SUMMON_ZOMBIE, "Zombies", TERM_WHITE, "", "ZOMBIE", 10 },
    { SUMMON_SKELETON, "Skeletons", TERM_WHITE, "", "SKELETON", 20 },
    { SUMMON_GHOST, "Ghosts", TERM_WHITE, "", "GHOST", 20 },
    { SUMMON_VAMPIRE, "Vampires", TERM_WHITE, "", "VAMPIRE", 25 },
    { SUMMON_WIGHT, "Wights", TERM_WHITE, "", "WIGHT", 25 },
    { SUMMON_LICH, "Liches", TERM_WHITE, "", "LICH", 30 },
    { SUMMON_KRAKEN, "Kraken", TERM_WHITE, "", "KRAKEN", 50 },
    { SUMMON_THIEF, "Thieves", TERM_WHITE, "", "THIEF", 15 },
    { SUMMON_ENT, "Ents", TERM_WHITE, "", "ENT", 50 },
    { SUMMON_CAMELOT, "Camelot Knights", TERM_WHITE, "", "CAMELOT", 25 },
    { SUMMON_NIGHTMARE, "Nightmares", TERM_WHITE, "", "NIGHTMARE", 20 },
    { SUMMON_YEEK, "Yeeks", TERM_WHITE, "", "YEEK", 2 },
    { SUMMON_ORC, "Orcs", TERM_WHITE, "", "ORC", 5 },
    { SUMMON_DARK_ELF, "Dark Elves", TERM_WHITE, "", "DARK_ELF", 15 },
    { SUMMON_GIANT, "Giants", TERM_WHITE, "", "GIANT", 20 },
    { SUMMON_UNDEAD_SUMMONER, "Undead Summoners", TERM_WHITE, "", "UNDEAD_SUMMONER", 25 },
    { SUMMON_MATURE_DRAGON, "Mature Dragons", TERM_WHITE, "", "MATURE_DRAGON", 15 },
    { SUMMON_DRAGON_SUMMONER, "Dragon Summoners", TERM_WHITE, "", "DRAGON_SUMMONER", 25 },
    { SUMMON_CLUBBER_DEMON, "Clubber Demons", TERM_WHITE, "", "CLUBBER_DEMON", 20 },
    { SUMMON_BALROG, "Balrogs", TERM_WHITE, "", "BALROG", 35 },
    { SUMMON_DEMON_SUMMONER, "Demon Summoners", TERM_WHITE, "", "DEMON_SUMMONER", 35 },
    { SUMMON_ULTIMATE, "Ultimate", TERM_WHITE, "", "ULTIMATE", 100 },
    { SUMMON_HUMAN, "Human", TERM_WHITE, "", "HUMAN", 10 },
    { SUMMON_HORSE, "Horsies", TERM_WHITE, "", "HORSE", 10 },
    { SUMMON_MAGICAL, "Magical Monsters", TERM_WHITE, "", "MAGICAL", 15 },
    { SUMMON_TROLL, "Trolls", TERM_WHITE, "", "TROLL", 10 },
    { SUMMON_CHAPEL_GOOD, "Good Monsters", TERM_WHITE, "", "CHAPEL_GOOD", 25 },
    { SUMMON_CHAPEL_EVIL, "Evil Monsters", TERM_WHITE, "", "CHAPEL_EVIL", 25 },
    { SUMMON_RING_BEARER, "Ring Bearers", TERM_WHITE, "", "RING_BEARER", 20 },
    { SUMMON_ARCHER, "Archers", TERM_WHITE, "", "ARCHER", 10 },
    { SUMMON_MONK, "Monks", TERM_WHITE, "", "MONK", 20 },
    { SUMMON_MAGE, "Mages", TERM_WHITE, "", "MAGE", 20 },
    { SUMMON_SPECIAL, "Special", TERM_WHITE, "", "SPECIAL", 30 },
    { SUMMON_REPTILE, "Reptiles", TERM_WHITE, "", "REPTILE", 30 },
    { 0 }
};

parse_tbl_ptr parse_tbl_parse(parse_tbl_ptr tbl, cptr token)
{
    int i;
    for (i = 0;; i++)
    {
        parse_tbl_ptr p = &tbl[i];
        if (!p->name) return NULL;
        if (strcmp(p->parse, token) == 0) return p;
    }
}

parse_tbl_ptr parse_tbl_lookup(parse_tbl_ptr tbl, int id)
{
    int i;
    for (i = 0;; i++)
    {
        parse_tbl_ptr p = &tbl[i];
        if (!p->name) return NULL;
        if (p->id == id) return p;
    }
}

parse_tbl_ptr summon_type_parse(cptr token)
{
    return parse_tbl_parse(_summon_type_tbl, token);
}
        
parse_tbl_ptr summon_type_lookup(int id)
{
    return parse_tbl_lookup(_summon_type_tbl, id);
}

int parse_lookup_monster(cptr name, int options)
{
    int i, paras = 0;
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        char buf[255];
        if (!r_ptr->name) continue;
        _prep_name(buf, r_name + r_ptr->name);
        if (strstr(buf, name))
        {
            if ((!paras) && (r_ptr->flags9 & RF9_DEPRECATED))
            {
                paras = i;
                continue;
            }
            if (trace_doc)
                doc_printf(trace_doc, "Mapping <color:B>%s</color> to <color:R>%s</color> (%d).\n", name, buf, i);
            return i;
        }
    }
    if (paras > 0)
    {
        monster_race *r_ptr = &r_info[paras];
        char buf[255];
        _prep_name(buf, r_name + r_ptr->name);
        if (trace_doc)
            doc_printf(trace_doc, "Mapping <color:B>%s</color> to <color:R>%s</color> (%d).\n", name, buf, paras);
        return paras;
    }
    return 0;
}

int parse_lookup_dungeon(cptr name, int options)
{
    int i;
    for (i = 1; i < max_d_idx; i++)
    {
        dungeon_info_type *d_ptr = &d_info[i];
        char buf[255];
        if (!d_ptr->name) continue;
        _prep_name(buf, d_name + d_ptr->name);
        if (strstr(buf, name))
        {
            if (trace_doc)
                doc_printf(trace_doc, "Mapping dungeon <color:B>%s</color> to <color:R>%s</color> (%d).\n", name, buf, i);
            return i;
        }
    }
    return 0;
}

/* MON(DRAGON, DEPTH+20)          Any dragon, 20 levels OoD
   MON(*, DEPTH+40)               Any monster, 40 levels OoD
   MON(ORC, NO_GROUP | HASTE)     A hasted orc loner at current depth
   MON(o, NO_GROUP | HASTE)       Ditto: You can use any valid d_char
   MON(black knight)              A Black Knight
   MON(^ent$)                     An Ent (not an Agent of the black market) */
static errr _parse_room_grid_monster(char **args, int arg_ct, room_grid_ptr grid, int options)
{
    if (arg_ct < 1 || arg_ct > 2)
    {
        msg_print("Invalid MON() directive: Syntax: MON(<which> [,<options>]).");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }

    /* Which monster? Can be random, by index, by display character, or by summoning type.*/
    if (streq(args[0], "*"))
    {
        grid->flags |= ROOM_GRID_MON_RANDOM;
    }
    else if (_is_numeric(args[0]))
    {
        grid->monster = atoi(args[0]);
        if (!grid->monster)
        {
            msg_format("Error: %d is not a valid monster index (See r_idx.txt).", grid->monster);
            return PARSE_ERROR_GENERIC;
        }
    }
    else if (_is_d_char(args[0]))
    {
        grid->flags |= ROOM_GRID_MON_CHAR;
        grid->monster = args[0][0];
    }
    else
    {
        parse_tbl_ptr p = summon_type_parse(args[0]);
        if (p)
        {
            grid->flags |= ROOM_GRID_MON_TYPE;
            grid->monster = p->id;
        }
        else
        {
            grid->monster = parse_lookup_monster(args[0], options);
            if (!grid->monster)
            {
                msg_format("Error: Invalid monster specifier %s.", args[0]);
                return PARSE_ERROR_GENERIC;
            }
        }
    }

    /* Options */
    if (arg_ct >= 2)
    {
        char *flags[10];
        int   flag_ct = z_string_split(args[1], flags, 10, "|");
        int   i, n;

        for (i = 0; i < flag_ct; i++)
        {
            char* flag = flags[i];
            if (streq(flag, "NO_GROUP"))
            {
                grid->flags |= ROOM_GRID_MON_NO_GROUP;
            }
            else if (streq(flag, "NO_SLEEP"))
            {
                grid->flags |= ROOM_GRID_MON_NO_SLEEP;
            }
            else if (streq(flag, "NO_UNIQUE"))
            {
                grid->flags |= ROOM_GRID_MON_NO_UNIQUE;
            }
            else if (streq(flag, "FRIENDLY"))
            {
                grid->flags |= ROOM_GRID_MON_FRIENDLY;
            }
            else if (streq(flag, "HASTE"))
            {
                grid->flags |= ROOM_GRID_MON_HASTE;
            }
            else if (streq(flag, "CLONE"))
            {
                grid->flags |= ROOM_GRID_MON_CLONED;
            }
            else if (sscanf(flag, "DEPTH+%d", &n) == 1)
            {
                grid->monster_level = n;
            }
            else if (sscanf(flag, "%d%%", &n) == 1)
            {
                grid->mon_pct = n;
            }
            else
            {
                msg_format("Error: Invalid monster option %s.", flag);
                return PARSE_ERROR_GENERIC;
            }
        }
    }
    return 0;
}

struct _object_type_s
{
    cptr name;
    int  type;
    int  ego_type;
};
typedef struct _object_type_s _object_type_t;

/* Use the following keywords in OBJ(), EGO() and ART() directives
   to force a random object of the indicated tval.
   Regexp: \#define TV_{[A-Z_0-9]+}[ ]+[0-9]+ -> { "\1", TV_\1 }, */
static _object_type_t _object_types[] =
{
    { "JUNK",               TV_JUNK },
    { "SKELETON",           TV_SKELETON },
    { "STATUE",             TV_STATUE },
    { "FIGURINE",           TV_FIGURINE },
    { "CHEST",              TV_CHEST },
    { "SHOT",               TV_SHOT, EGO_TYPE_AMMO },
    { "ARROW",              TV_ARROW, EGO_TYPE_AMMO },
    { "BOLT",               TV_BOLT, EGO_TYPE_AMMO },
    { "BOW",                TV_BOW, EGO_TYPE_BOW },
    { "DIGGING",            TV_DIGGING, EGO_TYPE_DIGGER },
    { "HAFTED",             TV_HAFTED, EGO_TYPE_WEAPON },
    { "POLEARM",            TV_POLEARM, EGO_TYPE_WEAPON },
    { "SWORD",              TV_SWORD, EGO_TYPE_WEAPON },
    { "BOOTS",              TV_BOOTS, EGO_TYPE_BOOTS },
    { "GLOVES",             TV_GLOVES, EGO_TYPE_GLOVES },
    { "HELM",               TV_HELM, EGO_TYPE_HELMET },
    { "CROWN",              TV_CROWN, EGO_TYPE_CROWN },
    { "SHIELD",             TV_SHIELD, EGO_TYPE_SHIELD },
    { "CLOAK",              TV_CLOAK, EGO_TYPE_CLOAK },
    { "SOFT_ARMOR",         TV_SOFT_ARMOR, EGO_TYPE_BODY_ARMOR },
    { "HARD_ARMOR",         TV_HARD_ARMOR, EGO_TYPE_BODY_ARMOR },
    { "DRAG_ARMOR",         TV_DRAG_ARMOR, EGO_TYPE_BODY_ARMOR },
    { "LITE",               TV_LITE, EGO_TYPE_LITE },
    { "AMULET",             TV_AMULET, EGO_TYPE_AMULET },
    { "RING",               TV_RING, EGO_TYPE_RING },
    { "STAFF",              TV_STAFF, EGO_TYPE_DEVICE },
    { "WAND",               TV_WAND, EGO_TYPE_DEVICE },
    { "ROD",                TV_ROD, EGO_TYPE_DEVICE },
    { "SCROLL",             TV_SCROLL },
    { "POTION",             TV_POTION },
    { "FOOD",               TV_FOOD },
    { "LIFE_BOOK",          TV_LIFE_BOOK },
    { "SORCERY_BOOK",       TV_SORCERY_BOOK },
    { "NATURE_BOOK",        TV_NATURE_BOOK },
    { "CHAOS_BOOK",         TV_CHAOS_BOOK },
    { "DEATH_BOOK",         TV_DEATH_BOOK },
    { "TRUMP_BOOK",         TV_TRUMP_BOOK },
    { "ARCANE_BOOK",        TV_ARCANE_BOOK },
    { "CRAFT_BOOK",         TV_CRAFT_BOOK },
    { "DAEMON_BOOK",        TV_DAEMON_BOOK },
    { "CRUSADE_BOOK",       TV_CRUSADE_BOOK },
    { "NECROMANCY_BOOK",    TV_NECROMANCY_BOOK },
    { "ARMAGEDDON_BOOK",    TV_ARMAGEDDON_BOOK },
    { "LAW_BOOK",           TV_LAW_BOOK },
    { "MUSIC_BOOK",         TV_MUSIC_BOOK },
    { "HISSATSU_BOOK",      TV_HISSATSU_BOOK },
    { "HEX_BOOK",           TV_HEX_BOOK },
    { "RAGE_BOOK",          TV_RAGE_BOOK },
    { "BURGLARY_BOOK",      TV_BURGLARY_BOOK },
    { "GOLD",               TV_GOLD },
    { "DEVICE",             OBJ_TYPE_DEVICE, EGO_TYPE_DEVICE },
    { "JEWELRY",            OBJ_TYPE_JEWELRY },
    { "BOOK",               OBJ_TYPE_BOOK },
    { "HI_BOOK",            OBJ_TYPE_HI_BOOK },
    { "BODY_ARMOR",         OBJ_TYPE_BODY_ARMOR, EGO_TYPE_BODY_ARMOR },
    { "OTHER_ARMOR",        OBJ_TYPE_OTHER_ARMOR },
    { "WEAPON",             OBJ_TYPE_WEAPON, EGO_TYPE_WEAPON },
    { "BOW_AMMO",           OBJ_TYPE_BOW_AMMO, EGO_TYPE_AMMO },
    { "MISC",               OBJ_TYPE_MISC },
    { 0, 0 }
};

static int _lookup_ego_type(int object)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_object_types[i].name) return EGO_TYPE_NONE;
        if (_object_types[i].type == object)
            return _object_types[i].ego_type;
    }
}

static int _lookup_ego(cptr name, int type, int options)
{
    int i;
    for (i = 1; i < max_e_idx; i++)
    {
        ego_type *e_ptr = &e_info[i];
        char      buf[255];
        if (!e_ptr->name) continue;
        if (type && !(e_ptr->type & type)) continue;
        _prep_name(buf, e_name + e_ptr->name);
        if (strstr(buf, name))
        {
            if (trace_doc)
                doc_printf(trace_doc, "Matching ego <color:B>%s</color> to <color:R>%s</color> (%d).\n", name, e_name + e_ptr->name, i);
            return i;
        }
    }
    return 0;
}

static int _lookup_kind(char *arg, int options)
{
    int i;
    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];
        char         buf[255];

        if (k_ptr->tval == TV_FOOD && k_ptr->sval <= SV_FOOD_MAX_MUSHROOM)
            strcpy(buf, "^mushroom of ");
        else if (k_ptr->tval == TV_POTION)
            strcpy(buf, "^potion of ");
        else if (k_ptr->tval == TV_SCROLL)
            strcpy(buf, "^scroll of ");
        else
            strcpy(buf, "^");

        _prep_name_aux(buf + strlen(buf), k_name + k_ptr->name);
        strcat(buf, "$");
        if (strstr(buf, arg))
        {
            if (trace_doc)
                doc_printf(trace_doc, "Mapping kind <color:B>%s</color> to <color:R>%s</color> (%d).\n", arg, buf, i);
            return i;
        }
    }
    return 0;
}

/* OBJ(WAND_ROCKET)      -> _parse_effect(TV_WAND, "ROCKET")     -> EFFECT_ROCKET
 * OBJ(STAFF_MANA_STORM) -> _parse_effect(TV_STAFF, "MANA_STORM")-> EFFECT_MANA_STORM */
static errr _parse_effect(int tval, cptr arg, room_grid_ptr grid, int options)
{
    int effect_id = effect_parse_type(arg);
    if (!effect_id)
    {
        msg_format("Unknown effect: %s", arg);
        return PARSE_ERROR_GENERIC;
    }
    if (!device_is_valid_effect(tval, effect_id))
    {
        msg_format("Invalid effect for this device type: %s (%d)", arg, effect_id);
        return PARSE_ERROR_GENERIC;
    }
    grid->object = lookup_kind(tval, SV_ANY);
    grid->extra = effect_id;
    grid->flags |= ROOM_GRID_OBJ_EFFECT;
    if (trace_doc)
    {
        char     name[255];
        effect_t e = {0};
        e.type = grid->extra;
        sprintf(name, "%s", do_effect(&e, SPELL_NAME, 0));
        doc_printf(trace_doc, "Mapping effect <color:B>%s</color> to <color:R>%s</color> (%d).\n",
            arg, name, grid->extra);
    }
    return 0;
}

/* OBJ(*)                       Any object
   OBJ(*, DEPTH+7)              Any object, 7 levels OoD
   OBJ(potion of healing)       Potion of Healing
   OBJ(POTION)                  Any potion
   OBJ(diamond edge, DEPTH+100) A really deep diamond edge
   OBJ(STAFF_MANA_STORM)        A staff with EFFECT_MANA_STORM
   OBJ(mushroom of cure serious wounds)
   OBJ(potion of cure serious wounds)   i.e. you must disambiguate!
*/
static errr _parse_room_grid_object(char **args, int arg_ct, room_grid_ptr grid, int options)
{
    switch (arg_ct)
    {
    case 2:
    {
        char *flags[10];
        int   flag_ct = z_string_split(args[1], flags, 10, "|");
        int   i, n, nn, nnn;
        char tyyppi[80] = "";

        for (i = 0; i < flag_ct; i++)
        {
            char* flag = flags[i];
            if (sscanf(flag, "DEPTH+%d", &n) == 1)
            {
                grid->object_level = n;
            }
            else if (sscanf(flag, "NUMBER=%d+%dd%d", &n, &nn, &nnn) == 3)
            {
                if ((n < 1) || (nn < 1) || (nnn < 1))
                {
                    msg_format("Error: Invalid object pile size specifier %s.", flag);
                    return PARSE_ERROR_GENERIC;
                }
                grid->extra2 = n + damroll(nn, nnn);
            }
            else if (sscanf(flag, "NUMBER=%dd%d", &n, &nn) == 2)
            {
                if ((n < 1) || (nn < 1))
                {
                    msg_format("Error: Invalid object pile size specifier %s.", flag);
                    return PARSE_ERROR_GENERIC;
                }
                grid->extra2 = damroll(n, nn);
            }
            else if (sscanf(flag, "NUMBER=%d", &n) == 1)
            {
                grid->extra2 = n;
            }
            else if (sscanf(flag, "TYPE=%d", &n) == 1)
            {
                /* Number and Type can share the same extra parameter,
                 * because Type is only used by devices (for ego generation),
                 * and statues (for marking the monster), and devices and
                 * statues are never generated in piles */
                grid->extra2 = n;
//                grid->flags |= ROOM_GRID_EGO;
            }
            else if (streq(flag, "TYPE=*"))
            {
                grid->extra2 = -1;
//                grid->flags |= ROOM_GRID_EGO_RANDOM;
            }
            else if (sscanf(flag, "TYPE=%s", tyyppi) == 1)
            {
                unsigned int j;
                for (j = 0; j < strlen(tyyppi); j++)
                {
                    tyyppi[j] = tolower(tyyppi[j]);
                }
                grid->extra2 = _lookup_ego(tyyppi, 0, options);
//                grid->flags |= ROOM_GRID_EGO;
            }
            else if (sscanf(flag, "%d%%", &n) == 1)
            {
                grid->obj_pct = n;
            }
            else
            {
                msg_format("Error: Invalid object option %s.", flag);
                return PARSE_ERROR_GENERIC;
            }
        }
        /* vvvvvvvvvvvvv Fall Through vvvvvvvvvvvvv */
    }
    case 1:
        if (streq(args[0], "*"))
        {
            grid->flags |= ROOM_GRID_OBJ_RANDOM;
        }
        else
        {
            int i;
            /* OBJ(WAND_ROCKET) ... note: OBJ(WAND) will fall thru to normal type handling */
            if (prefix(args[0], "WAND_"))
                return _parse_effect(TV_WAND, args[0] + strlen("WAND_"), grid, options);
            if (prefix(args[0], "ROD_"))
                return _parse_effect(TV_ROD, args[0] + strlen("ROD_"), grid, options);
            if (prefix(args[0], "STAFF_"))
                return _parse_effect(TV_STAFF, args[0] + strlen("STAFF_"), grid, options);
            /* OBJ(SWORD) */
            for (i = 0; ; i++)
            {
                if (!_object_types[i].name) break;
                if (streq(args[0], _object_types[i].name))
                {
                    grid->object = _object_types[i].type;
                    grid->flags |= ROOM_GRID_OBJ_TYPE;
                    return 0;
                }
            }
            /* OBJ(^dagger$) */
            grid->object = _lookup_kind(args[0], options);
            if (!grid->object)
            {
                /* OBJ(212) ... whatever that might be?? */
                grid->object = atoi(args[0]);
                if (!grid->object)
                {
                    msg_format("Invalid object: %s", args[0]);
                    return PARSE_ERROR_GENERIC;
                }
            }
        }
        break;
    default:
        msg_print("Error: Invalid OBJ() directive. Syntax: OBJ(<which> [,<flags>]).");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }
    return 0;
}

/* OBJ(RING):EGO(speed)          Ring of Speed
   OBJ(RING):EGO(*)              Any ego ring
   OBJ(CLOAK, DEPTH+20):EGO(*)   Any ego cloak generated 20 levels OoD
   OBJ(RING, DEPTH+50):EGO(speed) Ring of Speed generated 50 level OoD
   OBJ(SWORD):EGO(pattern)       A pattern blade
*/   
static errr _parse_room_grid_ego(char **args, int arg_ct, room_grid_ptr grid, int options)
{
    switch (arg_ct)
    {
    case 1:
        if (streq(args[0], "*"))
        {
            grid->flags |= ROOM_GRID_EGO_RANDOM;
        }
        else
        {
            if (_is_numeric(args[0]))
                grid->extra = atoi(args[0]);
            else if (grid->flags & ROOM_GRID_OBJ_TYPE)
            {
                int type = _lookup_ego_type(grid->object);
                grid->extra = _lookup_ego(args[0], type, options);
            }
            else if (grid->object && !(grid->flags & ROOM_GRID_OBJ_RANDOM))
            {
                int type = _lookup_ego_type(k_info[grid->object].tval);
                grid->extra = _lookup_ego(args[0], type, options);
            }
            else
                grid->extra = _lookup_ego(args[0], 0, options);
            if (!grid->extra)
            {
                msg_format("Error: Unknown Ego %s.", args[0]);
                return PARSE_ERROR_GENERIC;
            }
            grid->flags |= ROOM_GRID_OBJ_EGO;
        }
        break;

    default:
        msg_print("Error: Invalid EGO() directive. Syntax: EGO(<which>).");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }
    return 0;
}

int parse_lookup_artifact(cptr name, int options)
{
    int i;
    for (i = 1; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        char           buf[255];
        if (!a_ptr->name) continue;
        if (have_flag(a_ptr->flags, OF_FULL_NAME))
            _prep_name(buf, a_name + a_ptr->name);
        else /* ART(bow of bard) matches "long bow of bard" */
        {    /* not "black arrow of bard" or "soft leather boots of bard" */
            int          k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
            object_kind *k_ptr = &k_info[k_idx];
            strcpy(buf, "^");
            _prep_name_aux(buf + strlen(buf), k_name + k_ptr->name);
            strcat(buf, " ");
            _prep_name_aux(buf + strlen(buf), a_name + a_ptr->name);
            strcat(buf, "$");
        }
        if (strstr(buf, name))
        {
            if (trace_doc)
                doc_printf(trace_doc, "Matching artifact <color:B>%s</color> to <color:R>%s</color> (%d).\n", name, buf, i);
            return i;
        }
    }
    return 0;
}

/* OBJ(CLOAK, DEPTH+20):ART(*)   Rand-art cloak generated 20 levels OoD
   ART(dwarves)                  Necklace of the Dwarves */
static errr _parse_room_grid_artifact(char **args, int arg_ct, room_grid_ptr grid, int options)
{
    switch (arg_ct)
    {
    case 1:
        if (streq(args[0], "*"))
        {
            grid->flags |= ROOM_GRID_ART_RANDOM;
        }
        else
        {
            if (_is_numeric(args[0]))
                grid->object = atoi(args[0]);
            else
                grid->object = parse_lookup_artifact(args[0], options);
            if (!grid->object)
            {
                msg_format("Error: Unknown Artifact %s.", args[0]);
                return PARSE_ERROR_GENERIC;
            }
            grid->flags |= ROOM_GRID_OBJ_ARTIFACT;
        }
        break;

    default:
        msg_print("Error: Invalid ART() directive. Syntax: ART(<which>).");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }
    return 0;
}

static errr _parse_room_grid_trap(char **args, int arg_ct, room_grid_ptr grid)
{
    int n;
    switch (arg_ct)
    {
    case 2:
        if (sscanf(args[1], "%d%%", &n) == 1)
            grid->trap_pct = n;
        /* vvvvvvvvv Fall Through vvvvvvvvvvvv */
    case 1:
        if (streq(args[0], "*"))
        {
            grid->flags |= ROOM_GRID_TRAP_RANDOM;
        }
        else
        {
            s16b trap = f_tag_to_index(args[0]);
            if (trap < 0)
            {
                msg_format("Error: Unknown Trap %s.", args[0]);
                return PARSE_ERROR_GENERIC;
            }
            grid->cave_trap = trap;
        }
        break;

    default:
        msg_print("Error: Invalid TRAP() directive. Syntax: TRAP(<which> [,<pct odds>]).");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }
    return 0;
}

/* GRANITE
   FLOOR(ROOM | ICKY | GLOW) */
static errr _parse_room_grid_feature(char* name, char **args, int arg_ct, room_grid_ptr grid)
{
    s16b feat = f_tag_to_index(name);

    if (feat < 0)
    {
        msg_format("Error: Unknown Feature %s.", name);
        return PARSE_ERROR_GENERIC;
    }
    grid->cave_feat = feat;

    if (arg_ct > 2)
    {
        msg_print("Error: Invalid feature directive. Syntax: <Name>[(<flags> [,<special info>])].");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }
    if (arg_ct >= 2)
    {
        /* Extra is the dungeon type for wilderness random dungeon entrances. */
        grid->flags |= ROOM_GRID_SPECIAL;
        grid->extra = atoi(args[1]);
    }
    if (arg_ct >= 1)
    {
        char *flags[10];
        int   flag_ct = z_string_split(args[0], flags, 10, "|");
        int   i;

        for (i = 0; i < flag_ct; i++)
        {
            char* flag = flags[i];
            int n;

            if (streq(flag, "ROOM"))
                grid->cave_info |= CAVE_ROOM;
            else if (streq(flag, "ICKY"))
                grid->cave_info |= CAVE_ICKY;
            else if (streq(flag, "GLOW"))
                grid->cave_info |= CAVE_GLOW;
            else if (streq(flag, "MARK"))
                grid->cave_info |= CAVE_MARK | CAVE_AWARE;
            else if (streq(flag, "INNER"))
                grid->cave_info |= CAVE_INNER | CAVE_VAULT;
            else if (streq(flag, "OUTER"))
                grid->cave_info |= CAVE_OUTER | CAVE_VAULT;
            else if (_is_numeric(flag)) /* QUEST_ENTER(1) or QUEST_ENTER(GLOW, 1) */
            {
                grid->flags |= ROOM_GRID_SPECIAL;
                grid->extra = atoi(flag);
            }
            else if (sscanf(flag, "%d%%", &n) == 1)
                grid->feat_pct = n;
            else
            {
                msg_format("Error: Unknown Feature Option %s.", flag);
                return PARSE_ERROR_INVALID_FLAG;
            }
        }
    }
    return 0;
}

/*     v---------- buf
   L:.:FLOOR(ROOM|ICKY):MON(DRAGON, 20):EGO(*)
   R:ART(ringil)
     ^-----buf
*/
errr parse_room_grid(char *buf, room_grid_ptr grid, int options)
{
    errr  result = 0;
    char *commands[10];
    int   command_ct = z_string_split(buf, commands, 10, ":");
    int   i;
    bool  found_feature = FALSE;

    for (i = 0; i < command_ct; i++)
    {
        char *command = commands[i];
        char *name;
        char *args[10];
        int   arg_ct = parse_args(command, &name, args, 10);

        if (arg_ct < 0)
        {
            msg_format("Error: Malformed argument %s. Missing )?", name);
            return PARSE_ERROR_GENERIC;
        }

        if (streq(name, "MON"))
        {
            result = _parse_room_grid_monster(args, arg_ct, grid, options);
            if (result) break;
        }
        else if (streq(name, "OBJ"))
        {
            result = _parse_room_grid_object(args, arg_ct, grid, options);
            if (result) break;
        }
        else if (streq(name, "EGO"))
        {
            result = _parse_room_grid_ego(args, arg_ct, grid, options);
            if (result) break;
        }
        else if (streq(name, "ART"))
        {
            result = _parse_room_grid_artifact(args, arg_ct, grid, options);
            if (result) break;
        }
        else if (streq(name, "TRAP") || streq(name, "SECRET"))
        {
            result = _parse_room_grid_trap(args, arg_ct, grid);
            if (result) break;
        }
        else
        {
            if (found_feature)
            {
                msg_format("Error: Unknown %s directive.", name);
                return PARSE_ERROR_GENERIC;
            }

            result = _parse_room_grid_feature(name, args, arg_ct, grid);
            if (result) break;
            found_feature = TRUE;
        }
    }

    return result;
}

static errr _parse_room_flags(char* buf, room_ptr room)
{
    char *flags[10];
    int   flag_ct = z_string_split(buf, flags, 10, "|");
    int   i;

    for (i = 0; i < flag_ct; i++)
    {
        char* flag = flags[i];

        if (streq(flag, "GOOD"))
            room->flags |= ROOM_THEME_GOOD;
        else if (streq(flag, "EVIL"))
            room->flags |= ROOM_THEME_EVIL;
        else if (streq(flag, "FRIENDLY"))
            room->flags |= ROOM_THEME_FRIENDLY;
        else if (streq(flag, "NIGHT"))
            room->flags |= ROOM_THEME_NIGHT;
        else if (streq(flag, "DAY"))
            room->flags |= ROOM_THEME_DAY;
        else if (streq(flag, "SHOP"))
            room->flags |= ROOM_SHOP;
        else if (streq(flag, "DEBUG"))
            room->flags |= ROOM_DEBUG;
        else if (streq(flag, "NO_ROTATE"))
            room->flags |= ROOM_NO_ROTATE;
        else if (streq(flag, "FORMATION"))
            room->flags |= ROOM_THEME_FORMATION;
        else if (streq(flag, "THEME_OBJECT"))
            room->flags |= ROOM_THEME_OBJECT;
        else
        {
            msg_format("Error: Invalid room flag %s.", flag);
            return PARSE_ERROR_INVALID_FLAG;
        }
    }
    return 0;
}

static errr _parse_room_type(char *buf, room_ptr room)
{
    char *zz[10];
    int   num = tokenize(buf, 10, zz, 0);

    if (num < 2 || num > 3)
    {
        msg_print("Error: Invalid T: line. Syntax is T:<Type>:<Subtype>[:<Flags>].");
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;
    }

    if (streq(zz[0], "VAULT"))
    {
        room->type = ROOM_VAULT;
        if (streq(zz[1], "LESSER"))
            room->subtype = VAULT_LESSER;
        else if (streq(zz[1], "GREATER"))
            room->subtype = VAULT_GREATER;

        if (!room->subtype)
        {
            msg_format("Error: Unknown vault type %s.", zz[1]);
            return PARSE_ERROR_GENERIC;
        }
    }
    else if (streq(zz[0], "ROOM"))
    {
        room->type = ROOM_NORMAL;
        room->subtype = 0; /* TODO */
    }
    else if (streq(zz[0], "QUEST"))
    {
        room->type = ROOM_QUEST;
        room->subtype = 0; /* TODO */
    }
    else if (streq(zz[0], "TOWN"))
    {
        room->type = ROOM_TOWN;
        room->subtype = 0; /* TODO */
    }
    else if (streq(zz[0], "WILD") || streq(zz[0], "AMBUSH"))
    {
        static struct { cptr name; int type; } types[] = {
            {"WATER",    TERRAIN_DEEP_WATER}, /* TERRAIN_SHALLOW_WATER */
            {"SWAMP",    TERRAIN_SWAMP},
            {"GRASS",    TERRAIN_GRASS},      /* TERRAIN_DIRT, TERRAIN_DESERT */
            {"TREES",    TERRAIN_TREES},
            {"LAVA",     TERRAIN_DEEP_LAVA},  /* TERRAIN_SHALLOW_LAVA */
            {"MOUNTAIN", TERRAIN_MOUNTAIN},
            { 0, 0 }
        };
        int j;
        if (streq(zz[0], "AMBUSH"))
            room->type = ROOM_AMBUSH;
        else
            room->type = ROOM_WILDERNESS;
        for (j = 0; ; j++)
        {
            if (!types[j].name) break;
            if (streq(types[j].name, zz[1]))
            {
                room->subtype = types[j].type;
                break;
            }
        }

        if (!room->subtype)
        {
            msg_format("Error: Unknown wilderness type %s.", zz[1]);
            return PARSE_ERROR_GENERIC;
        }
    }

    if (!room->type)
    {
        msg_format("Error: Unknown room type %s.", zz[0]);
        return PARSE_ERROR_GENERIC;
    }

    if (num == 3)
    {
        errr rc = _parse_room_flags(zz[2], room);
        if (rc) return rc;
    }
    return 0;
}

static errr parse_v_info(char *buf, int options)
{
    /* Current entry */
    static room_ptr room = NULL;

    /* Default letters for all rooms and vaults */
    if (buf[0] == 'L' && buf[1] == ':' && !room)
    {
        int rc;
        room_grid_ptr letter = malloc(sizeof(room_grid_t));
        memset(letter, 0, sizeof(room_grid_t));
        letter->letter = buf[2];
        rc = parse_room_grid(buf + 4, letter, options);
        if (!rc) int_map_add(room_letters, letter->letter, letter);
        else free(letter);
        if (rc) return rc;
    }
    /* N:Name */
    else if (buf[0] == 'N')
    {
        char *zz[10];
        int   num = tokenize(buf + 2, 10, zz, 0);

        if (num != 1 || !*zz[0])
        {
            msg_print("Error: Invalid N: line. Syntax: N:<Name>.");
            return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        }

        error_idx = vec_length(room_info);
        if (options & INIT_DEBUG)
        {
            room_free(room);
            room = room_alloc(zz[0]);
        }
        else
        {
            room = room_alloc(zz[0]);
            vec_push(room_info, room);
        }
    }

    /* There better be a current room */
    else if (!room)
    {
        msg_print("Error: Missing N: line for new room template.");
        return PARSE_ERROR_MISSING_RECORD_HEADER;
    }
    else
        return parse_room_line(room, buf, options);

    /* Success */
    return 0;
}

errr init_v_info(int options)
{
    if (room_info) vec_free(room_info); /* double initialization?? */
    room_info = vec_alloc((vec_free_f)room_free);
    if (room_letters) int_map_free(room_letters);
    room_letters = int_map_alloc(free);
    return parse_edit_file("v_info.txt", parse_v_info, options);
}

/*
 * Initialize the "s_info" array, by parsing an ascii "template" file
 */
errr parse_s_info(char *buf, header *head)
{
    int i;

    /* Current entry */
    static skill_table *s_ptr = NULL;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Get the index */
        i = atoi(buf+2);

            /* Verify information */
        if (i <= error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        s_ptr = &s_info[i];
    }

    /* There better be a current s_ptr */
    else if (!s_ptr) return (3);

    /* Process 'W' for "Weapon exp" */
    else if (buf[0] == 'W')
    {
        int tval, sval, start, max;
        const s16b exp_conv_table[] =
        {
            WEAPON_EXP_UNSKILLED, WEAPON_EXP_BEGINNER, WEAPON_EXP_SKILLED,
            WEAPON_EXP_EXPERT, WEAPON_EXP_MASTER
        };

        /* Scan for the values */
        if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &tval, &sval, &start, &max)) return (1);

        if (start < EXP_LEVEL_UNSKILLED || start > EXP_LEVEL_MASTER
            || max < EXP_LEVEL_UNSKILLED || max > EXP_LEVEL_MASTER) return (8);

        /* Save the values */
        s_ptr->w_start[tval][sval] = exp_conv_table[start];
        s_ptr->w_max[tval][sval] = exp_conv_table[max];
    }

    /* Process 'S' for "Skill exp" */
    else if (buf[0] == 'S')
    {
        int num, start, max;

        /* Scan for the values */
        if (3 != sscanf(buf+2, "%d:%d:%d",
                &num, &start, &max)) return (1);

        if (start < WEAPON_EXP_UNSKILLED || start > WEAPON_EXP_MASTER
            || max < WEAPON_EXP_UNSKILLED || max > WEAPON_EXP_MASTER) return (8);

        /* Save the values */
        s_ptr->s_start[num] = start;
        s_ptr->s_max[num] = max;
    }


    /* Oops */
    else return (6);

    /* Success */
    return (0);
}


/*
 * Initialize the "m_info" array, by parsing an ascii "template" file
 */
errr parse_m_info(char *buf, header *head)
{
    int i;

    char *s;

    /* Current entry */
    static player_magic *m_ptr = NULL;

    /* ---Hack--- */
    static int realm, magic_idx = 0, readable = 0;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Get the index */
        i = atoi(buf+2);

            /* Verify information */
        if (i <= error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        m_ptr = &m_info[i];
    }

    /* There better be a current m_ptr */
    else if (!m_ptr) return (3);

    /* Process 'I' for "Info" (one line only) */
    else if (buf[0] == 'I')
    {
        char *book, *stat;
        int xtra, type, first, weight;

        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

        /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        book = buf+2;

        if (streq(book, "SORCERY")) m_ptr->spell_book = TV_SORCERY_BOOK;
        else if (streq(book, "LIFE")) m_ptr->spell_book = TV_LIFE_BOOK;
        else if (streq(book, "NECROMANCY")) m_ptr->spell_book = TV_NECROMANCY_BOOK;
        else if (streq(book, "MUSIC")) m_ptr->spell_book = TV_MUSIC_BOOK;
        else if (streq(book, "HISSATSU")) m_ptr->spell_book = TV_HISSATSU_BOOK;
        else if (streq(book, "RAGE")) m_ptr->spell_book = TV_RAGE_BOOK;
        else if (streq(book, "LAW")) m_ptr->spell_book = TV_LAW_BOOK;
        else if (streq(book, "NONE")) m_ptr->spell_book = 0;
        else return (5);

        stat = s;

        /* Find the colon before the name */
        s = my_strchr(s, ':');

        /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        if (streq(stat, "STR")) m_ptr->spell_stat = A_STR;
        else if (streq(stat, "INT")) m_ptr->spell_stat = A_INT;
        else if (streq(stat, "WIS")) m_ptr->spell_stat = A_WIS;
        else if (streq(stat, "DEX")) m_ptr->spell_stat = A_DEX;
        else if (streq(stat, "CON")) m_ptr->spell_stat = A_CON;
        else if (streq(stat, "CHR")) m_ptr->spell_stat = A_CHR;
        else return (5);


        /* Scan for the values */
        if (4 != sscanf(s, "%x:%d:%d:%d",
                (uint *)&xtra, &type, &first, &weight))    return (1);

        m_ptr->spell_xtra = xtra;
        m_ptr->spell_type = type;
        m_ptr->spell_first = first;
        m_ptr->spell_weight = weight;
    }


    /* Process 'R' for "Realm" (one line only) */
    else if (buf[0] == 'R')
    {
        /* Scan for the values */
        if (2 != sscanf(buf+2, "%d:%d",
                &realm, &readable)) return (1);

        magic_idx = 0;
    }

    else if (buf[0] == 'T')
    {
        int level, mana, fail, exp;

        if (!readable) return (1);
        /* Scan for the values */
        if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &level, &mana, &fail, &exp)) return (1);

        m_ptr->info[realm][magic_idx].realm = realm + 1; /* the realms are off by 1 */
        m_ptr->info[realm][magic_idx].idx = magic_idx;
        m_ptr->info[realm][magic_idx].slevel = level;
        m_ptr->info[realm][magic_idx].smana = mana;
        m_ptr->info[realm][magic_idx].sfail = fail;
        m_ptr->info[realm][magic_idx].sexp = exp;
        magic_idx ++;
    }


    /* Oops */
    else return (6);

    /* Success */
    return (0);
}


/*
 * Grab one flag from a textual string
 */
static errr grab_one_flag(u32b *flags, cptr names[], cptr what)
{
    int i;

    /* Check flags */
    for (i = 0; i < 32; i++)
    {
        if (streq(what, names[i]))
        {
            *flags |= (1L << i);
            return 0;
        }
    }

    return -1;
}


/*
 * Grab one flag in an feature_type from a textual string
 */
static errr grab_one_feat_flag(feature_type *f_ptr, cptr what)
{
    int i;

    /* Check flags */
    for (i = 0; i < FF_FLAG_MAX; i++)
    {
        if (streq(what, f_info_flags[i]))
        {
            add_flag(f_ptr->flags, i);
            return 0;
        }
    }

    /* Oops */
    msg_format("Unknown feature flag '%s'.", what);

    /* Error */
    return PARSE_ERROR_GENERIC;
}


/*
 * Grab an action in an feature_type from a textual string
 */
static errr grab_one_feat_action(feature_type *f_ptr, cptr what, int count)
{
    int i;

    /* Check flags */
    for (i = 0; i < FF_FLAG_MAX; i++)
    {
        if (streq(what, f_info_flags[i]))
        {
            f_ptr->state[count].action = i;
            return 0;
        }
    }

    /* Oops */
    msg_format("Unknown feature action '%s'.", what);

    /* Error */
    return PARSE_ERROR_GENERIC;
}


/*
 * Initialize the "f_info" array, by parsing an ascii "template" file
 */
errr parse_f_info(char *buf, header *head)
{
    int i;

    char *s, *t;

    /* Current entry */
    static feature_type *f_ptr = NULL;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

        if (s)
        {
            /* Nuke the colon, advance to the name */
            *s++ = '\0';
        }

        /* Get the index */
        i = atoi(buf+2);

        /* Verify information */
        if (i <= error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        f_ptr = &f_info[i];

        /* Tag name is given */
        if (s)
        {
            /* Store the tag */
            if (!add_tag(&f_ptr->tag, head, s)) return (7);
        }

        /* Default "mimic" */
        f_ptr->mimic = i;

        /* Default "destroyed state" -- if not specified */
        f_ptr->destroyed = i;

        /* Default "states" */
        for (i = 0; i < MAX_FEAT_STATES; i++) f_ptr->state[i].action = FF_FLAG_MAX;
    }

    /* There better be a current f_ptr */
    else if (!f_ptr) return (3);

    else if (buf[0] == 'E')
    {
        /* Acquire the Text */
        s = buf+2;

        /* Store the name */
        if (!add_name(&f_ptr->name, head, s)) return (7);
    }


    /* Process 'M' for "Mimic" (one line only) */
    else if (buf[0] == 'M')
    {
        s16b offset;

        if (!add_tag(&offset, head, buf + 2)) return PARSE_ERROR_OUT_OF_MEMORY;

        /* Record a fake tag index */
        f_ptr->mimic = -offset;
    }


    /* Process 'G' for "Graphics" (one line only) */
    else if (buf[0] == 'G')
    {
        int j;
        byte s_attr;
        char char_tmp[F_LIT_MAX];

        /* Paranoia */
        if (buf[1] != ':') return (1);
        if (!buf[2]) return (1);
        if (buf[3] != ':') return (1);
        if (!buf[4]) return (1);

        /* Extract the char */
        char_tmp[F_LIT_STANDARD] = buf[2];

        /* Extract the color */
        s_attr = color_char_to_attr(buf[4]);

        /* Paranoia */
        if (s_attr > 127) return (1);

        /* Save the standard values */
        f_ptr->d_attr[F_LIT_STANDARD] = s_attr;
        f_ptr->d_char[F_LIT_STANDARD] = char_tmp[F_LIT_STANDARD];

        /* Is this feature supports lighting? */
        if (buf[5] == ':')
        {
            /* G:c:a:LIT (default) */
            apply_default_feat_lighting(f_ptr->d_attr, f_ptr->d_char);

            /* G:c:a:lc:la:dc:da */
            if (!streq(buf + 6, "LIT"))
            {
                char attr_lite_tmp[F_LIT_MAX - F_LIT_NS_BEGIN];

                if ((F_LIT_MAX - F_LIT_NS_BEGIN) * 2 != sscanf(buf + 6, "%c:%c:%c:%c",
                    &char_tmp[F_LIT_LITE], &attr_lite_tmp[F_LIT_LITE - F_LIT_NS_BEGIN],
                    &char_tmp[F_LIT_DARK], &attr_lite_tmp[F_LIT_DARK - F_LIT_NS_BEGIN])) return 1;
                if (buf[F_LIT_MAX * 4 + 1]) return 1;

                for (j = F_LIT_NS_BEGIN; j < F_LIT_MAX; j++)
                {
                    switch (attr_lite_tmp[j - F_LIT_NS_BEGIN])
                    {
                    case '*':
                        /* Use default lighting */
                        break;
                    case '-':
                        /* No lighting support */
                        f_ptr->d_attr[j] = f_ptr->d_attr[F_LIT_STANDARD];
                        break;
                    default:
                        /* Extract the color */
                        f_ptr->d_attr[j] = color_char_to_attr(attr_lite_tmp[j - F_LIT_NS_BEGIN]);
                        if (f_ptr->d_attr[j] > 127) return 1;
                        break;
                    }
                    f_ptr->d_char[j] = char_tmp[j];
                }
            }
        }
        else if (!buf[5])
        {
            for (j = F_LIT_NS_BEGIN; j < F_LIT_MAX; j++)
            {
                f_ptr->d_attr[j] = s_attr;
                f_ptr->d_char[j] = char_tmp[F_LIT_STANDARD];
            }
        }
        else return 1;
    }

    /* Hack -- Process 'F' for flags */
    else if (buf[0] == 'F')
    {
        /* Parse every entry textually */
        for (s = buf + 2; *s; )
        {
            /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

            /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while (*t == ' ' || *t == '|') t++;
            }

            /* XXX XXX XXX Hack -- Read feature subtype */
            if (1 == sscanf(s, "SUBTYPE_%d", &i))
            {
                /* Extract a "subtype" */
                f_ptr->subtype =  i;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }

            /* XXX XXX XXX Hack -- Read feature power */
            if (1 == sscanf(s, "POWER_%d", &i))
            {
                /* Extract a "power" */
                f_ptr->power =  i;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }

            /* Parse this entry */
            if (0 != grab_one_feat_flag(f_ptr, s)) return (PARSE_ERROR_INVALID_FLAG);

            /* Start the next entry */
            s = t;
        }
    }

    /* Process 'W' for "More Info" (one line only) */
    else if (buf[0] == 'W')
    {
        int priority;

        /* Scan for the value */
        if (1 != sscanf(buf+2, "%d", &priority)) return (PARSE_ERROR_GENERIC);

        /* Save the value */
        f_ptr->priority = priority;
    }

    /* Process 'K' for "States" (up to four lines + default (which cannot be last)) */
    else if (buf[0] == 'K')
    {
        s16b offset;

        /* Find the next empty state slot (if any) */
        for (i = 0; i < MAX_FEAT_STATES; i++) if (f_ptr->state[i].action == FF_FLAG_MAX) break;

        /* Oops, no more slots */
        if (i == MAX_FEAT_STATES) return PARSE_ERROR_GENERIC;

        /* Analyze the first field */
        for (s = t = buf+2; *t && (*t != ':'); t++) /* loop */;

        /* Terminate the field (if necessary) */
        if (*t == ':') *t++ = '\0';

        /* Is this default entry? */
        if (streq(s, "DESTROYED"))
        {
            if (!add_tag(&offset, head, t)) return PARSE_ERROR_OUT_OF_MEMORY;

            /* Record a fake tag index */
            f_ptr->destroyed = -offset;
        }
        else
        {
            /* Reset */
            f_ptr->state[i].action = 0;

            /* Parse this entry */
            if (0 != grab_one_feat_action(f_ptr, s, i)) return PARSE_ERROR_INVALID_FLAG;

            if (!add_tag(&offset, head, t)) return PARSE_ERROR_OUT_OF_MEMORY;

            /* Record a fake tag index */
            f_ptr->state[i].result = -offset;
        }
    }

    /* Oops */
    else    return (6);

    /* Success */
    return (0);
}


/*
 * Convert a fake tag to a real feat index
 */
s16b f_tag_to_index(cptr str)
{
    u16b i;

    /* Search for real index corresponding to this fake tag */
    for (i = 0; i < f_head.info_num; i++)
    {
        if (streq(f_tag + f_info[i].tag, str))
        {
            /* Return the index */
            return (s16b)i;
        }
    }

    /* Not found */
    return -1;
}


/*
 * Search for real index corresponding to this fake tag
 */
static void search_real_feat(s16b *feat)
{
    int i;

    /* Don't convert non-fake tag */
    if (*feat >= 0) return;

    /* Search for real index corresponding to this fake tag */
    for (i = 0; i < f_head.info_num; i++)
    {
        if ((-(*feat)) == f_info[i].tag)
        {
            /* Record real index */
            *feat = (s16b)i;
            return;
        }
    }

    /* Undefined tag */
    msg_format("%s is undefined.", f_tag + (-(*feat)));
}


/*
 * Retouch fake tags of f_info
 */
void retouch_f_info(header *head)
{
    int i;

    /* Convert fake tags to real feat indices */
    for (i = 0; i < head->info_num; i++)
    {
        feature_type *f_ptr = &f_info[i];
        int j;

        search_real_feat(&f_ptr->mimic);

        search_real_feat(&f_ptr->destroyed);

        for (j = 0; j < MAX_FEAT_STATES; j++) search_real_feat(&f_ptr->state[j].result);
    }
}


/*
 * Grab one flag in an object_kind from a textual string
 */
static errr grab_one_kind_flag(object_kind *k_ptr, cptr what)
{
    int i;

    /* We really should check this someplace :) */
    assert((OF_COUNT + 31)/32 == OF_ARRAY_SIZE);

    /* Check flags */
    for (i = 0; i < OF_COUNT; i++)
    {
        if (streq(what, k_info_flags[i]))
        {
            add_flag(k_ptr->flags, i);
            return (0);
        }
    }

    if (grab_one_flag(&k_ptr->gen_flags, k_info_gen_flags, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown object flag '%s'.", what);


    /* Error */
    return (1);
}


/*
 * Initialize the "k_info" array, by parsing an ascii "template" file
 */
errr parse_k_info(char *buf, header *head)
{
    char *s, *t;

    /* Current entry */
    static object_kind *k_ptr = NULL;
    static int k_idx = 0;

    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        char *flavor;

        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

        /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        /* Get the index */
        if (strcmp(buf+2, "*") == 0)
            k_idx++;
        else
            k_idx = atoi(buf+2);

        /* Verify information */
        if (k_idx <= error_idx) return (4);

        /* Verify information */
        if (k_idx >= head->info_num) return (2);

        /* Save the index */
        error_idx = k_idx;

        /* Point at the "info" */
        k_ptr = &k_info[k_idx];
        k_ptr->idx = k_idx;

        /* Paranoia -- require a name */
        if (!*s) return (1);

        /* Find the colon before the flavor */
        flavor = my_strchr(s, ':');

        /* Verify that colon */
        if (flavor)
        {
            /* Nuke the colon, advance to the flavor */
            *flavor++ = '\0';

            /* Store the flavor */
            if (!add_name(&k_ptr->flavor_name, head, flavor)) return (7);
        }

        /* Store the name */
        if (!add_name(&k_ptr->name, head, s)) return (7);
    }

    /* There better be a current k_ptr */
    else if (!k_ptr) return (3);
    /* From Vanilla: M:P:XdY to control object stacks. P is
       the probabilty (1 to 100) for a stack and XdY is how
       many get rolled up. Replaces TRG_STACK and obj_make_pile(). */
    else if (buf[0] == 'M')
    {
        int p, x, y;

        if (3 != sscanf(buf+2, "%d:%dd%d",
                &p, &x, &y)) return (1);

        k_ptr->stack_chance = p;
        k_ptr->stack_dice = x;
        k_ptr->stack_sides = y;
    }
    else if (buf[0] == 'E')
    {
        /* First E: line is required and defines the activation. */
        if (!k_ptr->activation.type)
        {
            errr rc = effect_parse(buf + 2, &k_ptr->activation);
            if (rc) return rc;
            add_flag(k_ptr->flags, OF_ACTIVATE); /* for object lore */
        }
        /* Second E: line is optional and describes the activation. */
        else if (!k_ptr->activation_msg)
        {
            s = buf+2;
            if (!add_text(&k_ptr->activation_msg, head, s, FALSE)) return (7);
        }
        else
            return 1;
    }

    /* Process 'D' for "Description" */
    else if (buf[0] == 'D')
    {
        /* Acquire the text */
        s = buf+2;

        /* Store the text */
        if (!add_text(&k_ptr->text, head, s, TRUE)) return (7);
    }

    /* Process 'G' for "Graphics" (one line only) */
    else if (buf[0] == 'G')
    {
        char sym;
        byte tmp;

        /* Paranoia */
        if (buf[1] != ':') return (1);
        if (!buf[2]) return (1);
        if (buf[3] != ':') return (1);
        if (!buf[4]) return (1);

        /* Extract the char */
        sym = buf[2];

        /* Extract the attr */
        tmp = color_char_to_attr(buf[4]);

        /* Paranoia */
        if (tmp > 127) return (1);

        /* Save the values */
        k_ptr->d_attr = tmp;
        k_ptr->d_char = sym;
    }

    /* Process 'I' for "Info" (one line only) */
    else if (buf[0] == 'I')
    {
        int tval, sval, pval;

        /* Scan for the values */
        if (3 != sscanf(buf+2, "%d:%d:%d",
                &tval, &sval, &pval)) return (1);

        /* Save the values */
        k_ptr->tval = tval;
        k_ptr->sval = sval;
        k_ptr->pval = pval;
    }

    /* Process 'W' for "More Info" (one line only) */
    else if (buf[0] == 'W')
    {
        int level, extra, wgt, max_level;
        int cost;

        /* Scan for the values */
        if (5 != sscanf(buf+2, "%d:%d:%d:%d:%d",
                &level, &extra, &max_level, &wgt, &cost)) return (1);

        /* Save the values */
        k_ptr->level = level;
        k_ptr->extra = extra;
        k_ptr->max_level = max_level;
        k_ptr->weight = wgt;
        k_ptr->cost = cost;
    }

    /* Process 'A' for "Allocation" (one line only) */
    else if (buf[0] == 'A')
    {
        int i;

        /* XXX XXX XXX Simply read each number following a colon */
        for (i = 0, s = buf+1; s && (s[0] == ':') && s[1]; ++i)
        {
                /* Default chance */
            k_ptr->chance[i] = 1;

                /* Store the attack damage index */
            k_ptr->locale[i] = atoi(s+1);

                /* Find the slash */
            t = my_strchr(s+1, '/');

                /* Find the next colon */
            s = my_strchr(s+1, ':');

                /* If the slash is "nearby", use it */
            if (t && (!s || t < s))
            {
                int chance = atoi(t+1);
                if (chance > 0) k_ptr->chance[i] = chance;
            }
        }
    }

    /* Hack -- Process 'P' for "power" and such */
    else if (buf[0] == 'P')
    {
        int ac, hd1, hd2, th, td, ta, mult = 0;

        if (k_ptr->tval == TV_BOW)
        {
            if (6 != sscanf(buf+2, "%d:x%d.%d:%d:%d:%d",
                    &ac, &hd1, &hd2, &th, &td, &ta)) return (1);
            mult = hd1 * 100 + hd2; /* x3.25 -> 325 (alas, x3.2 -> 302 so use x3.20 instead) */
            hd1 = 0;
            hd2 = 0;
        }
        else
        {
            if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
                    &ac, &hd1, &hd2, &th, &td, &ta)) return (1);
        }
        k_ptr->ac = ac;
        k_ptr->dd = hd1;
        k_ptr->ds = hd2;
        k_ptr->mult = mult;
        k_ptr->to_h = th;
        k_ptr->to_d = td;
        k_ptr->to_a =  ta;
    }

    /* Hack -- Process 'F' for flags */
    else if (buf[0] == 'F')
    {
        /* Parse every entry textually */
        for (s = buf + 2; *s; )
        {
                /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

                /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while (*t == ' ' || *t == '|') t++;
            }

                /* Parse this entry */
            if (0 != grab_one_kind_flag(k_ptr, s)) return (5);

                /* Start the next entry */
            s = t;
        }
    }


    /* Oops */
    else return (6);


    /* Success */
    return (0);
}


/*
 * Grab one flag in an artifact_type from a textual string
 */
static errr grab_one_artifact_flag(artifact_type *a_ptr, cptr what)
{
    int i;

    /* Check flags */
    for (i = 0; i < OF_COUNT; i++)
    {
        if (streq(what, k_info_flags[i]))
        {
            add_flag(a_ptr->flags, i);
            return (0);
        }
    }

    if (grab_one_flag(&a_ptr->gen_flags, k_info_gen_flags, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown artifact flag '%s'.", what);


    /* Error */
    return (1);
}




/*
 * Initialize the "a_info" array, by parsing an ascii "template" file
 */
errr parse_a_info(char *buf, header *head)
{
    int i;

    char *s, *t;

    /* Current entry */
    static artifact_type *a_ptr = NULL;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

            /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        /* Paranoia -- require a name */
        if (!*s) return (1);

        /* Get the index */
        i = atoi(buf+2);

        /* Verify information */
        if (i < error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        a_ptr = &a_info[i];

        /* Ignore everything */
        add_flag(a_ptr->flags, OF_IGNORE_ACID);
        add_flag(a_ptr->flags, OF_IGNORE_ELEC);
        add_flag(a_ptr->flags, OF_IGNORE_FIRE);
        add_flag(a_ptr->flags, OF_IGNORE_COLD);

        /* Store the name */
        if (!add_name(&a_ptr->name, head, s)) return (7);
    }

    /* There better be a current a_ptr */
    else if (!a_ptr) return (3);

    else if (buf[0] == 'E')
    {
        /* First E: line is required and defines the activation. */
        if (!a_ptr->activation.type)
        {
            errr rc = effect_parse(buf + 2, &a_ptr->activation);
            if (rc) return rc;
            add_flag(a_ptr->flags, OF_ACTIVATE); /* for object lore */
        }
        /* Second E: line is optional and describes the activation. */
        else if (!a_ptr->activation_msg)
        {
            s = buf+2;
            if (!add_text(&a_ptr->activation_msg, head, s, FALSE)) return (7);
        }
        else
            return 1;
    }

    /* Process 'D' for "Description" */
    else if (buf[0] == 'D')
    {
        /* Acquire the text */
        s = buf+2;

        /* Store the text */
        if (!add_text(&a_ptr->text, head, s, TRUE)) return (7);
    }


    /* Process 'I' for "Info" (one line only) */
    else if (buf[0] == 'I')
    {
        int tval, sval, pval;

        /* Scan for the values */
        if (3 != sscanf(buf+2, "%d:%d:%d",
                &tval, &sval, &pval)) return (1);

        /* Save the values */
        a_ptr->tval = tval;
        a_ptr->sval = sval;
        a_ptr->pval = pval;
    }

    /* Process 'W' for "More Info" (one line only) */
    else if (buf[0] == 'W')
    {
        int level, rarity, wgt;
        int cost;

        /* Scan for the values */
        if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &level, &rarity, &wgt, &cost)) return (1);

        /* Save the values */
        a_ptr->level = level;
        a_ptr->rarity = rarity;
        a_ptr->weight = wgt;
        a_ptr->cost = cost;
    }

    /* Hack -- Process 'P' for "power" and such */
    else if (buf[0] == 'P')
    {
        int ac, hd1, hd2, th, td, ta, mult = 0;

        if (a_ptr->tval == TV_BOW)
        {
            if (6 != sscanf(buf+2, "%d:x%d.%d:%d:%d:%d",
                    &ac, &hd1, &hd2, &th, &td, &ta)) return (1);
            mult = hd1 * 100 + hd2; /* x3.25 -> 325 (alas, x3.2 -> 302 so use x3.20 instead) */
            hd1 = 0;
            hd2 = 0;
        }
        else
        {
            if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
                    &ac, &hd1, &hd2, &th, &td, &ta)) return (1);
        }
        a_ptr->ac = ac;
        a_ptr->dd = hd1;
        a_ptr->ds = hd2;
        a_ptr->mult = mult;
        a_ptr->to_h = th;
        a_ptr->to_d = td;
        a_ptr->to_a =  ta;
    }

    /* Hack -- Process 'F' for flags */
    else if (buf[0] == 'F')
    {
        /* Parse every entry textually */
        for (s = buf + 2; *s; )
        {
                /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

                /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while ((*t == ' ') || (*t == '|')) t++;
            }

                /* Parse this entry */
            if (0 != grab_one_artifact_flag(a_ptr, s)) return (5);

                /* Start the next entry */
            s = t;
        }
    }


    /* Oops */
    else return (6);


    /* Success */
    return (0);
}


/*
 * Grab one flag in a ego-item_type from a textual string
 */
static bool grab_one_ego_item_flag(ego_type *e_ptr, cptr what)
{
    int i;

    /* Check flags */
    for (i = 0; i < OF_COUNT; i++)
    {
        if (streq(what, k_info_flags[i]))
        {
            add_flag(e_ptr->flags, i);
            return (0);
        }
    }

    if (grab_one_flag(&e_ptr->gen_flags, k_info_gen_flags, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown ego-item flag '%s'.", what);


    /* Error */
    return (1);
}

static bool grab_one_ego_type_flag(ego_type *e_ptr, cptr what)
{
    if (streq(what, "AMMO")) e_ptr->type |= EGO_TYPE_AMMO;
    else if (streq(what, "WEAPON")) e_ptr->type |= EGO_TYPE_WEAPON;
    else if (streq(what, "SHIELD")) e_ptr->type |= EGO_TYPE_SHIELD;
    else if (streq(what, "BOW")) e_ptr->type |= EGO_TYPE_BOW;
    else if (streq(what, "RING")) e_ptr->type |= EGO_TYPE_RING;
    else if (streq(what, "AMULET")) e_ptr->type |= EGO_TYPE_AMULET;
    else if (streq(what, "LITE")) e_ptr->type |= EGO_TYPE_LITE;
    else if (streq(what, "BODY_ARMOR")) e_ptr->type |= EGO_TYPE_BODY_ARMOR;
    else if (streq(what, "CLOAK")) e_ptr->type |= EGO_TYPE_CLOAK;
    else if (streq(what, "HELMET")) e_ptr->type |= EGO_TYPE_HELMET;
    else if (streq(what, "GLOVES")) e_ptr->type |= EGO_TYPE_GLOVES;
    else if (streq(what, "BOOTS")) e_ptr->type |= EGO_TYPE_BOOTS;
    else if (streq(what, "DIGGER")) e_ptr->type |= EGO_TYPE_DIGGER;
    else if (streq(what, "CROWN")) e_ptr->type |= EGO_TYPE_CROWN;
    else if (streq(what, "HARP")) e_ptr->type |= EGO_TYPE_HARP;
    else if (streq(what, "ROBE")) e_ptr->type |= EGO_TYPE_ROBE;
    else if (streq(what, "SPECIAL")) e_ptr->type |= EGO_TYPE_SPECIAL;
    else if (streq(what, "DEVICE")) e_ptr->type |= EGO_TYPE_DEVICE;
    else if (streq(what, "DRAGON_ARMOR")) e_ptr->type |= EGO_TYPE_DRAGON_ARMOR;
    else if (streq(what, "QUIVER")) e_ptr->type |= EGO_TYPE_QUIVER;
    else
    {
        msg_format("Unknown ego type flag: '%s'.", what);
        return ERROR_UNKOWN_FAILURE;
    }
    return ERROR_SUCCESS;
}

/*
 * Initialize the "e_info" array, by parsing an ascii "template" file
 */
errr parse_e_info(char *buf, header *head)
{
    int i;

    char *s, *t;

    /* Current entry */
    static ego_type *e_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* N:1:of Free Action */
    if (buf[0] == 'N')
    {
        char *zz[3];
        int   num = tokenize(buf + 2, 2, zz, 0);

        if (num != 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

        /* Unique Index */
        i = atoi(zz[0]);
        if (i < error_idx) return 4;
        if (i >= head->info_num) return 2;

        error_idx = i;
        e_ptr = &e_info[i];
        e_ptr->id = i;

        /* Description */
        if (!add_name(&e_ptr->name, head, zz[1])) return 7;
    }

    /* There better be a current e_ptr */
    else if (!e_ptr) return (3);
    else if (buf[0] == 'E')
    {
        errr rc = effect_parse(buf + 2, &e_ptr->activation);
        if (rc) return rc;
        add_flag(e_ptr->flags, OF_ACTIVATE); /* for object lore */
    }
    /* W:MinDepth:MaxDepth:Rarity
       W:30:*:32                  */
    else if (buf[0] == 'W')
    {
        char *zz[4];
        int   num = tokenize(buf + 2, 4, zz, 0);

        if (num != 3) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

        e_ptr->level = atoi(zz[0]);
        if (strcmp(zz[1], "*") == 0)
            e_ptr->max_level = 0;
        else
            e_ptr->max_level = atoi(zz[1]);
        e_ptr->rarity = atoi(zz[2]);
    }

    /* Hack -- Process 'C' for "creation" */
    else if (buf[0] == 'C')
    {
        int th, td, ta, pv;

        /* Scan for the values */
        if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &th, &td, &ta, &pv)) return (1);

        e_ptr->max_to_h = th;
        e_ptr->max_to_d = td;
        e_ptr->max_to_a = ta;
        e_ptr->max_pval = pv;
    }
    /* T:HELMET | SHIELD | BODY_ARMOR | CLOAK
       T:WEAPON | AMMO
       T:RING etc. */
    else if (buf[0] == 'T')
    {
        /* Parse every entry textually */
        for (s = buf + 2; *s; )
        {
                /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

                /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while ((*t == ' ') || (*t == '|')) t++;
            }

                /* Parse this entry */
            if (0 != grab_one_ego_type_flag(e_ptr, s)) return (5);

                /* Start the next entry */
            s = t;
        }
    }

    /* Hack -- Process 'F' for flags */
    else if (buf[0] == 'F')
    {
        /* Parse every entry textually */
        for (s = buf + 2; *s; )
        {
                /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

                /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while ((*t == ' ') || (*t == '|')) t++;
            }

                /* Parse this entry */
            if (0 != grab_one_ego_item_flag(e_ptr, s)) return (5);

                /* Start the next entry */
            s = t;
        }
    }
    /* Process 'D' for "Description" */
    else if (buf[0] == 'D')
    {
        /* Acquire the text */
        s = buf+2;

        /* Store the text */
        if (!add_text(&e_ptr->text, head, s, TRUE)) return (7);
    }

    /* Oops */
    else return (6);

    /* Success */
    return (0);
}


/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_flag(monster_race *r_ptr, cptr what)
{
    if (grab_one_flag(&r_ptr->flags1, r_info_flags1, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flags2, r_info_flags2, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flags3, r_info_flags3, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flags7, r_info_flags7, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flags8, r_info_flags8, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flags9, r_info_flags9, what) == 0)
        return 0;

    if (grab_one_flag(&r_ptr->flagsr, r_info_flagsr, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown monster flag '%s'.", what);


    /* Failure */
    return (1);
}

/*
 * b_info for monster body types
 */
static cptr b_info_slots[] =
{
    "NONE",
    "GLOVES",
    "WEAPON_SHIELD",
    "RING",
    "BOW",
    "AMULET",
    "LITE",
    "BODY_ARMOR",
    "CLOAK",
    "BOOTS",
    "HELMET",
    "ANY",
    "WEAPON",
    "CAPTURE_BALL",
    "QUIVER",
};

errr parse_b_info(char *buf, header *head)
{
    int i;
    char *s, *t;

    /* Current entry */
    static equip_template_ptr b_ptr = NULL;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

        /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        /* Paranoia -- require a name */
        if (!*s) return (1);

        /* Get the index */
        i = atoi(buf+2);

        /* Verify information */
        if (i < error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        b_ptr = &b_info[i];

        /* Store the name */
        if (!add_name(&b_ptr->name, head, s)) return (7);
    }

    /* There better be a current r_ptr */
    else if (!b_ptr) return (3);

    else if (buf[0] == 'S')
    {
        int n1;

        i = ++b_ptr->max;
        if (i > EQUIP_MAX) return PARSE_ERROR_OUT_OF_BOUNDS;

        /* Analyze the first field */
        for (s = t = buf+2; *t && (*t != ':'); t++) /* loop */;

        /* Terminate the field (if necessary) */
        if (*t == ':') *t++ = '\0';

        /* Analyze the slot type */
        for (n1 = 0; n1 < EQUIP_SLOT_MAX; n1++)
        {
            if (streq(s, b_info_slots[n1]))
            {
                b_ptr->slots[i].type = n1;
                break;
            }
        }

        /* Invalid slot type */
        if (!b_ptr->slots[i].type) return (1);

        /* Analyze the second field */
        for (s = t; *t && (*t != ':'); t++) /* loop */;

        /* Terminate the field (if necessary) */
        if (*t == ':') *t++ = '\0';

        if (!add_tag(&b_ptr->slots[i].tag, head, s)) return (7);

        /* Analyze the third field */
        if (*t)
            b_ptr->slots[i].hand = atoi(t);
    }
    /* Oops */
    else return 6;


    /* Success */
    return 0;
}

/* BITE(60) or perhaps just BITE */
static errr parse_mon_blow_method(char *command, mon_blow_ptr blow)
{
    char *name;
    char *args[10];
    int   arg_ct = parse_args(command, &name, args, 10);

    if (arg_ct < 0)
    {
        msg_format("Error: Malformed argument %s. Missing )?", name);
        return PARSE_ERROR_GENERIC;
    }

    blow->method = _get_r_blow_method(name);
    if (!blow->method)
    {
        msg_format("Error: Unknown monster blow method %s.", name);
        return PARSE_ERROR_UNDEFINED_DIRECTIVE;
    }

    if (arg_ct > 1)
        return PARSE_ERROR_TOO_FEW_ARGUMENTS; /* too many, actually */

    if (arg_ct)
    {
        cptr arg = args[0];
        blow->power = atoi(arg);
    }
    return 0;
}

/* historically in mbe_info ... */
static int _default_blow_power(int effect)
{
    switch (effect)
    {
    case RBE_HURT:
    case RBE_SHATTER:
        return 60;
    case RBE_LOSE_STR:
    case RBE_LOSE_INT:
    case RBE_LOSE_WIS:
    case RBE_LOSE_DEX:
    case RBE_LOSE_CON:
    case RBE_LOSE_CHR:
    case GF_ACID:
        return 0;
    case GF_ELEC:
    case GF_FIRE:
    case GF_COLD:
        return 10;
    case RBE_LOSE_ALL:
        return 2;
    case GF_DISENCHANT:
        return 20;
    case RBE_DRAIN_CHARGES:
        return 15;
    }
    return 5;
}

/*   V------------------- buf
 * B:BITE:SUPERHURT:15d10   <===== The old syntax (no longer supported)
 * B:BITE(60):HURT(15d10):HURT(15d10, 20%):STUN(5d5, 10%)  <=== New syntax, multiple effects
 *   ^------------------- buf
 */
static errr parse_mon_blow(char *buf, mon_blow_ptr blow)
{
    errr  rc = 0;
    char *commands[10];
    int   command_ct = z_string_split(buf, commands, 10, ":");
    int   i, j, dd, ds, pct; /* sscanf probably wants int*, not byte* */

    if (command_ct < 1)
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;

    rc = parse_mon_blow_method(commands[0], blow);
    if (rc) return rc;

    if (command_ct - 1 > MAX_MON_BLOW_EFFECTS)
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;

    for (i = 1; i < command_ct; i++)
    {
        char *command = commands[i];
        char *name;
        char *args[10];
        int   arg_ct = parse_args(command, &name, args, 10);
        mon_effect_ptr effect = &blow->effects[i-1];

        if (arg_ct < 0)
        {
            msg_format("Error: Malformed argument %s. Missing )?", name);
            return PARSE_ERROR_GENERIC;
        }

        effect->effect = _get_r_blow_effect(name);
        if (!effect->effect)
        {
            msg_format("Error: Unknown monster blow effect %s.", name);
            return PARSE_ERROR_UNDEFINED_DIRECTIVE;
        }
        if (arg_ct > 2)
            return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        for (j = 0; j < arg_ct; j++)
        {
            char arg[100], sentinel = '~', check;
            /* sscanf tricks learned from vanilla ... */
            sprintf(arg, "%s%c", args[j], sentinel);
            
            if (2 == sscanf(arg, "%d%%%c", &pct, &check) && check == sentinel)
                effect->pct = MAX(0, MIN(100, pct));
            else if (3 == sscanf(arg, "%dd%d%c", &dd, &ds, &check) && check == sentinel)
            {
                effect->dd = MAX(0, MIN(100, dd)); /* 100d100 max */
                effect->ds = MAX(0, MIN(100, ds));
            }
            else
            {
                msg_format("Error: Unknown argument %s.", args[j]);
                return PARSE_ERROR_GENERIC;
            }
        }
        /* For convenience, use the implied power on the first effect for the
         * overall blow power. For example: B:BITE:HURT(7d7) == B:BITE(60):HURT(7d7) */
        if (i == 1 && !blow->power)
            blow->power = _default_blow_power(effect->effect);
    }

    return rc;
}
/*   V---buf
 * A:INERT(3d4, 20%):FIRE(3d4):GRAVITY(3d4, 25%)
 * Multiple auras on a single line. Assume all only a single A: line per monster.
 * Up to MAX_MON_AURAS allowed. Effects are GF_**** (see list above).
 */
static errr parse_mon_auras(char *buf, mon_race_ptr r_ptr)
{
    errr  rc = 0;
    char *commands[10];
    int   command_ct = z_string_split(buf, commands, 10, ":");
    int   i, j, dd, ds, pct; /* sscanf probably wants int*, not byte* */

    if (command_ct > MAX_MON_AURAS)
        return PARSE_ERROR_TOO_FEW_ARGUMENTS;

    for (i = 0; i < command_ct; i++)
    {
        char *command = commands[i];
        char *name;
        char *args[10];
        int   arg_ct = parse_args(command, &name, args, 10);
        mon_effect_ptr aura = &r_ptr->auras[i];

        if (arg_ct < 0)
        {
            msg_format("Error: Malformed argument %s. Missing )?", name);
            return PARSE_ERROR_GENERIC;
        }
        if (aura->effect)
        {
            msg_print("Duplicate A: line. Put all auras on a single line, please!");
            return PARSE_ERROR_GENERIC;
        }

        aura->effect = _get_gf_type(name);
        if (!aura->effect)
        {
            msg_format("Error: Unknown monster aura effect %s.", name);
            return PARSE_ERROR_UNDEFINED_DIRECTIVE;
        }
        if (arg_ct > 2)
            return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        for (j = 0; j < arg_ct; j++)
        {
            char arg[100], sentinel = '~', check;
            /* sscanf tricks learned from vanilla ... */
            sprintf(arg, "%s%c", args[j], sentinel);
            
            if (2 == sscanf(arg, "%d%%%c", &pct, &check) && check == sentinel)
                aura->pct = MAX(0, MIN(100, pct));
            else if (3 == sscanf(arg, "%dd%d%c", &dd, &ds, &check) && check == sentinel)
            {
                aura->dd = MAX(0, MIN(100, dd)); /* 100d100 max */
                aura->ds = MAX(0, MIN(100, ds));
            }
            else
            {
                msg_format("Error: Unknown argument %s.", args[j]);
                return PARSE_ERROR_GENERIC;
            }
        }
    }

    return rc;
}
static errr parse_mon_flags(char *buf, mon_race_ptr race)
{
    errr  rc = 0;
    char *tokens[20];
    int   token_ct = z_string_split(buf, tokens, 20, "|");
    int   i, j;

    for (i = 0; i < token_ct; i++)
    {
        char *token = tokens[i];

        if (!strlen(token)) continue;
        if (prefix(token, "FRIENDS"))
        {
            char *name;
            char *args[10];
            int   arg_ct = parse_args(token, &name, args, 10);
            race->flags1 |= RF1_FRIENDS;
            if (!streq(name, "FRIENDS")) return 5; /* eg FRIENDS_I_LIKE s/b an error */
            if (arg_ct > 2)
                return PARSE_ERROR_TOO_FEW_ARGUMENTS; /* s/FEW/MANY */
            for (j = 0; j < arg_ct; j++)  /* XXX parsing logic duplicated above ... */
            {
                char arg[100], sentinel = '~', check;
                int  dd, ds, pct;
                sprintf(arg, "%s%c", args[j], sentinel);
                
                if (2 == sscanf(arg, "%d%%%c", &pct, &check) && check == sentinel)
                    race->pack_pct = MAX(0, MIN(100, pct));
                else if (3 == sscanf(arg, "%dd%d%c", &dd, &ds, &check) && check == sentinel)
                {
                    race->pack_dice = MAX(0, MIN(100, dd)); /* 100d100 max */
                    race->pack_sides = MAX(0, MIN(100, ds));
                }
                else
                {
                    msg_format("Error: Unknown argument %s.", args[j]);
                    return PARSE_ERROR_GENERIC;
                }
            }
        }
        else if (0 != grab_one_basic_flag(race, token)) return 5;
    }
    return rc;
}
static errr parse_mon_spells(char *buf, mon_race_ptr race)
{
    errr  rc = 0;
    char *tokens[10];
    int   token_ct = z_string_split(buf, tokens, 10, "|");
    int   i, n;

    assert(race->spells);

    for (i = 0; i < token_ct; i++)
    {
        char *token = tokens[i];

        if (!strlen(token)) continue;

        if (1 == sscanf(token, "1_IN_%d", &n))
            race->spells->freq = 100 / n;
        else if (1 == sscanf(token, "FREQ_%d", &n))
            race->spells->freq = n;
        else
        {
            rc = mon_spells_parse(race->spells, race->level, token);
            if (rc)
                return rc;
        }
    }
    return rc;
}
/*
 * Initialize the "r_info" array, by parsing an ascii "template" file
 */
errr parse_r_info(char *buf, header *head)
{
    int i;

    char *s;

    /* Current entry */
    static monster_race *r_ptr = NULL;


    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

            /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        /* Paranoia -- require a name */
        if (!*s) return (1);

        /* Get the index */
        i = atoi(buf+2);

        /* Verify information */
        if (i < error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        r_ptr = &r_info[i];
        r_ptr->id = i;

        /* Store the name */
        if (!add_name(&r_ptr->name, head, s)) return (7);
    }

    /* There better be a current r_ptr */
    else if (!r_ptr) return (3);

    /* Process 'D' for "Description" */
    else if (buf[0] == 'D')
    {
        /* Acquire the text */
        s = buf+2;

        /* Store the text */
        if (!add_text(&r_ptr->text, head, s, TRUE)) return (7);
    }

    /* Process 'G' for "Graphics" (one line only) */
    else if (buf[0] == 'G')
    {
        char sym;
        byte tmp;

        /* Paranoia */
        if (buf[1] != ':') return (1);
        if (!buf[2]) return (1);
        if (buf[3] != ':') return (1);
        if (!buf[4]) return (1);

        /* Extract the char */
        sym = buf[2];

        /* Extract the attr */
        tmp = color_char_to_attr(buf[4]);

        /* Paranoia */
        if (tmp > 127) return (1);

        /* Save the values */
        r_ptr->d_char = sym;
        r_ptr->d_attr = tmp;
    }

    /* Process 'I' for "Info" (one line only) */
    else if (buf[0] == 'I')
    {
        int spd, hp1, hp2, aaf, ac, slp, wgt;

        /* Scan for the other values */
        if (7 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d:%d",
                &spd, &hp1, &hp2, &aaf, &ac, &slp, &wgt)) return (1);

        /* Save the values */
        r_ptr->speed = spd;
        r_ptr->hdice = MAX(hp1, 1);
        r_ptr->hside = MAX(hp2, 1);
        r_ptr->aaf = aaf;
        r_ptr->ac = ac;
        r_ptr->sleep = slp;
        r_ptr->weight = wgt;
    }
    else if (buf[0] == 'M')
    {
        int m,s;

        /* Scan for the other values */
        if (2 != sscanf(buf+2, "%d:%d", &m, &s)) return (1);

        /* Save the values */
        r_ptr->melee_level = m;
        r_ptr->save_level = s;
    }
    /* P:<Code>:... for The Possessor */
    else if (buf[0] == 'P')
    {
        char *zz[33];
        int   num = tokenize(buf + 2, 33, zz, 0);

        if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

        if (strcmp(zz[0], "Copy") == 0)
        {
            int idx, i;
            if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

            /* P:Copy:<r_idx> */
            if (num == 2)
            {
                idx = atoi(zz[1]);
                r_ptr->body = r_info[idx].body;
            }
            /* P:Copy:Yeek:Tourist */
            else if (num == 3)
            {
                idx = get_race_idx(zz[1]);
                if (idx >= 0)
                {
                    race_t *race_ptr = get_race_aux(idx, 0);
                    for (i = 0; i < MAX_STATS; i++)
                        r_ptr->body.stats[i] = race_ptr->stats[i];
                    r_ptr->body.skills = race_ptr->skills;
                    r_ptr->body.extra_skills = race_ptr->extra_skills;
                    r_ptr->body.life = race_ptr->life;
                    r_ptr->body.infra = race_ptr->infra;
                }
                else
                    return PARSE_ERROR_OUT_OF_BOUNDS;

                idx = lookup_class_idx(zz[2]);
                if (idx >= 0)
                {
                    class_t *class_ptr = get_class_aux(idx, 0);
                    r_ptr->body.class_idx = idx;
                    for (i = 0; i < MAX_STATS; i++)
                        r_ptr->body.stats[i] += class_ptr->stats[i];
                    skills_add(&r_ptr->body.skills, &class_ptr->base_skills);
                    skills_add(&r_ptr->body.extra_skills, &class_ptr->extra_skills);
                    r_ptr->body.life = r_ptr->body.life * class_ptr->life / 100;
                    if (class_ptr->caster_info)
                        r_ptr->body.spell_stat = (s16b)class_ptr->caster_info()->which_stat;
                    else
                        r_ptr->body.spell_stat = A_NONE;
                }
                else
                    return PARSE_ERROR_OUT_OF_BOUNDS;
            }
            else
                return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        }
        else if (strcmp(zz[0], "Speed") == 0)
        {
            int sp = atoi(zz[1]);
            if (num != 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            if (sp > 90) /* Hack: 110 -> +0, 100 -> -10, etc. This is old school.*/
                sp = sp - 110;
            r_ptr->body.speed = sp;
        }
        else if (strcmp(zz[0], "SpellStat") == 0)
        {
            if (num != 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            if (streq(zz[1], "Str")) r_ptr->body.spell_stat = A_STR;
            else if (streq(zz[1], "Int")) r_ptr->body.spell_stat = A_INT;
            else if (streq(zz[1], "Wis")) r_ptr->body.spell_stat = A_WIS;
            else if (streq(zz[1], "Dex")) r_ptr->body.spell_stat = A_DEX;
            else if (streq(zz[1], "Con")) r_ptr->body.spell_stat = A_CON;
            else if (streq(zz[1], "Chr")) r_ptr->body.spell_stat = A_CHR;
            else return PARSE_ERROR_OUT_OF_BOUNDS;
        }
        /* P:Stats:<Str>:<Int>:<Wis>:<Dex>:<Con>:<Chr> */
        else if (strcmp(zz[0], "Stats") == 0)
        {
            if (num < 7) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            r_ptr->body.stats[A_STR] += atoi(zz[1]);
            r_ptr->body.stats[A_INT] += atoi(zz[2]);
            r_ptr->body.stats[A_WIS] += atoi(zz[3]);
            r_ptr->body.stats[A_DEX] += atoi(zz[4]);
            r_ptr->body.stats[A_CON] += atoi(zz[5]);
            r_ptr->body.stats[A_CHR] += atoi(zz[6]);
        }
        /* P:Skills:<Dis>:<Dev>:<Sav>:<Stl>:<Srh>:<Fos>:<Thn>:<Thb> */
        else if (strcmp(zz[0], "Skills") == 0)
        {
            if (num < 9) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            r_ptr->body.skills.dis += atoi(zz[1]);
            r_ptr->body.skills.dev += atoi(zz[2]);
            r_ptr->body.skills.sav += atoi(zz[3]);
            r_ptr->body.skills.stl += atoi(zz[4]);
            r_ptr->body.skills.srh += atoi(zz[5]);
            r_ptr->body.skills.fos += atoi(zz[6]);
            r_ptr->body.skills.thn += atoi(zz[7]);
            r_ptr->body.skills.thb += atoi(zz[8]);
        }
        else if (strcmp(zz[0], "ExtraSkills") == 0)
        {
            if (num < 9) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            r_ptr->body.extra_skills.dis += atoi(zz[1]);
            r_ptr->body.extra_skills.dev += atoi(zz[2]);
            r_ptr->body.extra_skills.sav += atoi(zz[3]);
            r_ptr->body.extra_skills.stl += atoi(zz[4]);
            r_ptr->body.extra_skills.srh += atoi(zz[5]);
            r_ptr->body.extra_skills.fos += atoi(zz[6]);
            r_ptr->body.extra_skills.thn += atoi(zz[7]);
            r_ptr->body.extra_skills.thb += atoi(zz[8]);
        }
        else if (strcmp(zz[0], "Life") == 0)
        {
            int life;
            if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            life = atoi(zz[1]);
            if (!r_ptr->body.life)
                r_ptr->body.life = life;
            else
                r_ptr->body.life = r_ptr->body.life * life / 100;
        }
        else if (strcmp(zz[0], "Infra") == 0)
        {
            if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            r_ptr->body.infra += atoi(zz[1]);
        }
        else if (strcmp(zz[0], "Body") == 0)
        {
            int i;
            bool found = FALSE;
            if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

            for (i = 0; i < max_b_idx; i++)
            {
                if (streq(b_name + b_info[i].name, zz[1]))
                {
                    r_ptr->body.body_idx = i;
                    found = TRUE;
                    break;
                }
            }

            if (!found) return PARSE_ERROR_OUT_OF_BOUNDS;
        }
        else if (strcmp(zz[0], "Blows") == 0)
        {
            if (num < 2) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
            if (num > 4) return PARSE_ERROR_TOO_FEW_ARGUMENTS; /* s/FEW/MANY */
            switch (num)
            {
            case 4: r_ptr->body.blows_calc.mult = atoi(zz[3]);
            case 3: r_ptr->body.blows_calc.wgt = atoi(zz[2]);
            case 2: r_ptr->body.blows_calc.max = atoi(zz[1]);
            }
        }
        else
            return PARSE_ERROR_UNDEFINED_DIRECTIVE;
    }
    /* Process 'W' for "More Info" (one line only) */
    else if (buf[0] == 'W')
    {
        int lev, rar, max_lev;
        int exp;
        int nextexp;
        int nextmon;

        /* Scan for the values */
        if (6 != sscanf(buf+2, "%d:%d:%d:%d:%d:%d",
                &lev, &rar, &max_lev, &exp, &nextexp, &nextmon)) return (1);

        /* Save the values */
        r_ptr->level = lev;
        r_ptr->max_level = max_lev;

        r_ptr->rarity = rar;
        r_ptr->mexp = exp;
        r_ptr->next_exp = nextexp;
        r_ptr->next_r_idx = nextmon;
    }

    /* Process 'B' for "Blows" (up to MAX_MON_BLOWS lines) */
    else if (buf[0] == 'B')
    {
        errr rc = 0;

        /* Find the next empty blow slot (if any) */
        for (i = 0; i < MAX_MON_BLOWS; i++) if (!r_ptr->blows[i].method) break;
        if (i == MAX_MON_BLOWS) return (1);

        rc = parse_mon_blow(buf + 2, &r_ptr->blows[i]);
        if (rc) return rc;
    }

    /* Process 'A' for "Auras" (up to MAX_MON_AURAS, all on a single line) */
    else if (buf[0] == 'A')
    {
        return parse_mon_auras(buf + 2, r_ptr);
    }
    /* Process 'F' for "Basic Flags" (multiple lines) */
    else if (buf[0] == 'F')
    {
        return parse_mon_flags(buf + 2, r_ptr);
    }

    /* Process 'S' for "Spell Flags" (multiple lines) */
    else if (buf[0] == 'S')
    {
        if (!r_ptr->spells)
            r_ptr->spells = mon_spells_alloc();
        return parse_mon_spells(buf + 2, r_ptr);
    }
    /* O:DROP_WARRIOR */
    else if (buf[0] == 'O')
    {
        /* Acquire the text */
        s = buf+2;

        if (r_ptr->drop_theme)
            return PARSE_ERROR_GENERIC; /* Only one theme allowed */

        for (i = 0; i < R_DROP_MAX; i++)
        {
            if (streq(s, r_drop_themes[i]))
            {
                r_ptr->drop_theme = i;
                break;
            }
        }
        if (!r_ptr->drop_theme)
            return PARSE_ERROR_INVALID_FLAG; /* Not a valid theme */
    }

    /* Oops */
    else return PARSE_ERROR_UNDEFINED_DIRECTIVE;


    /* Success */
    return (0);
}


/*
 * Grab one flag for a dungeon type from a textual string
 */
static errr grab_one_dungeon_flag(dungeon_info_type *d_ptr, cptr what)
{
    if (grab_one_flag(&d_ptr->flags1, d_info_flags1, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown dungeon type flag '%s'.", what);

    /* Failure */
    return (1);
}

/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_monster_flag(dungeon_info_type *d_ptr, cptr what)
{
    if (grab_one_flag(&d_ptr->mflags1, r_info_flags1, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflags2, r_info_flags2, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflags3, r_info_flags3, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflags7, r_info_flags7, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflags8, r_info_flags8, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflags9, r_info_flags9, what) == 0)
        return 0;

    if (grab_one_flag(&d_ptr->mflagsr, r_info_flagsr, what) == 0)
        return 0;

    /* Oops */
    msg_format("Unknown monster flag '%s'.", what);
    /* Failure */
    return (1);
}


/*
 * Initialize the "d_info" array, by parsing an ascii "template" file
 */
errr parse_d_info(char *buf, header *head)
{
    int i;

    char *s, *t;

    /* Current entry */
    static dungeon_info_type *d_ptr = NULL;

    /* Process 'N' for "New/Number/Name" */
    if (buf[0] == 'N')
    {
        /* Find the colon before the name */
        s = my_strchr(buf+2, ':');

        /* Verify that colon */
        if (!s) return (1);

        /* Nuke the colon, advance to the name */
        *s++ = '\0';

        /* Paranoia -- require a name */
        if (!*s) return (1);

        /* Get the index */
        i = atoi(buf+2);

        /* Verify information */
        if (i < error_idx) return (4);

        /* Verify information */
        if (i >= head->info_num) return (2);

        /* Save the index */
        error_idx = i;

        /* Point at the "info" */
        d_ptr = &d_info[i];
        d_ptr->id = i;

        /* Store the name */
        if (!add_name(&d_ptr->name, head, s)) return (7);
    }

    /* Process 'D' for "Description */
    else if (buf[0] == 'D')
    {
        /* Acquire the text */
        s = buf+2;

        /* Store the text */
        if (!add_text(&d_ptr->text, head, s, TRUE)) return (7);
    }

    /* Process 'W' for "More Info" (one line only) */
    else if (buf[0] == 'W')
    {
        int min_lev, max_lev;
        int min_plev, mode;
        int min_alloc, max_chance;
        int obj_good, obj_great;
        int pit, nest;

        /* Scan for the values */
        if (10 != sscanf(buf+2, "%d:%d:%d:%d:%d:%d:%d:%d:%x:%x",
                 &min_lev, &max_lev, &min_plev, &mode, &min_alloc, &max_chance, &obj_good, &obj_great, (unsigned int *)&pit, (unsigned int *)&nest)) return (1);

        /* Save the values */
        d_ptr->mindepth = min_lev;
        d_ptr->maxdepth = max_lev;
        d_ptr->min_plev = min_plev;
        d_ptr->mode = mode;
        d_ptr->min_m_alloc_level = min_alloc;
        d_ptr->max_m_alloc_chance = max_chance;
        d_ptr->obj_good = obj_good;
        d_ptr->obj_great = obj_great;
        d_ptr->pit = pit;
        d_ptr->nest = nest;
    }

    /* Process 'P' for "Place Info" */
    else if (buf[0] == 'P')
    {
        int dy, dx;

        /* Scan for the values */
        if (2 != sscanf(buf+2, "%d:%d", &dy, &dx)) return (1);

        /* Save the values */
        d_ptr->dy = dy;
        d_ptr->dx = dx;
    }

    /* Process 'L' for "fLoor type" (one line only) */
    else if (buf[0] == 'L')
    {
        char *zz[16];

        /* Scan for the values */
        if (tokenize(buf+2, DUNGEON_FEAT_PROB_NUM * 2 + 1, zz, 0) != (DUNGEON_FEAT_PROB_NUM * 2 + 1)) return (1);

        /* Save the values */
        for (i = 0; i < DUNGEON_FEAT_PROB_NUM; i++)
        {
            d_ptr->floor[i].feat = f_tag_to_index(zz[i * 2]);
            if (d_ptr->floor[i].feat < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;

            d_ptr->floor[i].percent = atoi(zz[i * 2 + 1]);
        }
        d_ptr->tunnel_percent = atoi(zz[DUNGEON_FEAT_PROB_NUM * 2]);
    }

    /* Process 'A' for "wAll type" (one line only) */
    else if (buf[0] == 'A')
    {
        char *zz[16];

        /* Scan for the values */
        if (tokenize(buf+2, DUNGEON_FEAT_PROB_NUM * 2 + 4, zz, 0) != (DUNGEON_FEAT_PROB_NUM * 2 + 4)) return (1);

        /* Save the values */
        for (i = 0; i < DUNGEON_FEAT_PROB_NUM; i++)
        {
            d_ptr->fill[i].feat = f_tag_to_index(zz[i * 2]);
            if (d_ptr->fill[i].feat < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;

            d_ptr->fill[i].percent = atoi(zz[i * 2 + 1]);
        }

        d_ptr->outer_wall = f_tag_to_index(zz[DUNGEON_FEAT_PROB_NUM * 2]);
        if (d_ptr->outer_wall < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;

        d_ptr->inner_wall = f_tag_to_index(zz[DUNGEON_FEAT_PROB_NUM * 2 + 1]);
        if (d_ptr->inner_wall < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;

        d_ptr->stream1 = f_tag_to_index(zz[DUNGEON_FEAT_PROB_NUM * 2 + 2]);
        if (d_ptr->stream1 < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;

        d_ptr->stream2 = f_tag_to_index(zz[DUNGEON_FEAT_PROB_NUM * 2 + 3]);
        if (d_ptr->stream2 < 0) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;
    }

    /* Process 'F' for "Dungeon Flags" (multiple lines) */
    else if (buf[0] == 'F')
    {
        int artif = 0, monst = 0, tval = 0, sval = 0, pant = 0;

        /* Parse every entry */
        for (s = buf + 2; *s; )
        {
            /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

            /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while (*t == ' ' || *t == '|') t++;
            }

            /* XXX XXX XXX Hack -- Read Final Artifact */
            if (1 == sscanf(s, "FINAL_ARTIFACT_%d", &artif))
            {
                /* Extract a "Final Artifact" */
                d_ptr->final_artifact = artif;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }

            /* XXX XXX XXX Hack -- Read Final Object */
            if (2 == sscanf(s, "FINAL_OBJECT_%d_%d", &tval, &sval))
            {
                int k_idx = lookup_kind(tval, sval);
                if (!k_idx) return PARSE_ERROR_UNDEFINED_TERRAIN_TAG;
                d_ptr->final_object = k_idx;
                s = t;
                continue;
            }

            if (1 == sscanf(s, "FINAL_EGO_%d", &artif))
            {
                d_ptr->final_ego = artif;
                s = t;
                continue;
            }

            /* XXX XXX XXX Hack -- Read Final Guardian */
            if (1 == sscanf(s, "FINAL_GUARDIAN_%d", &monst))
            {
                /* Extract final guardian */
                d_ptr->final_guardian = monst;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }
            if (1 == sscanf(s, "INITIAL_GUARDIAN_%d", &monst))
            {
                d_ptr->initial_guardian = monst;
                s = t;
                continue;
            }

            /* XXX XXX XXX Hack -- Read Associated Pantheon */
            if (1 == sscanf(s, "PANTHEON_%d", &pant))
            {
                /* Extract pantheon */
                d_ptr->pantheon = pant;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }


            /* XXX XXX XXX Hack -- Read Special Percentage */
            if (1 == sscanf(s, "MONSTER_DIV_%d", &monst))
            {
                /* Extract a "Special %" */
                d_ptr->special_div = monst;

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }

            /* Parse this entry */
            if (0 != grab_one_dungeon_flag(d_ptr, s)) return (5);

            /* Start the next entry */
            s = t;
        }
    }

    /* Process 'M' for "Basic Flags" (multiple lines) */
    else if (buf[0] == 'M')
    {
        /* Parse every entry */
        for (s = buf + 2; *s; )
        {
            /* Find the end of this entry */
            for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

            /* Nuke and skip any dividers */
            if (*t)
            {
                *t++ = '\0';
                while (*t == ' ' || *t == '|') t++;
            }

            /* Hack -- Read monster symbols */
            if (!strncmp(s, "R_CHAR_", 7))
            {
                /* Skip "R_CHAR_" */
                s += 7;

                /* Read a string */
                strncpy(d_ptr->r_char, s, sizeof(d_ptr->r_char));

                /* Start at next entry */
                s = t;

                /* Continue */
                continue;
            }

            /* Parse this entry */
            if (0 != grab_one_basic_monster_flag(d_ptr, s)) return (5);

            /* Start the next entry */
            s = t;
        }
    }


    /* Oops */
    else return (6);

    /* Success */
    return (0);
}

#else    /* ALLOW_TEMPLATES */

#ifdef MACINTOSH
static int i = 0;
#endif

#endif    /* ALLOW_TEMPLATES */

/*
 * Place the object j_ptr to a grid
 */
void drop_here(object_type *j_ptr, int y, int x)
{
    cave_type *c_ptr = &cave[y][x];
    object_type *o_ptr;

    /* Get new object */
    s16b o_idx = o_pop();

    /* Access new object */
    o_ptr = &o_list[o_idx];

    /* Structure copy */
    object_copy(o_ptr, j_ptr);


    /* Locate */
    o_ptr->loc.where = INV_FLOOR;
    o_ptr->loc.y = y;
    o_ptr->loc.x = x;
    o_ptr->loc.slot = o_idx;

    /* No monster */
    o_ptr->held_m_idx = 0;

    /* Build a stack */
    o_ptr->next_o_idx = c_ptr->o_idx;

    /* Place the object */
    c_ptr->o_idx = o_idx;

    p_ptr->window |= PW_OBJECT_LIST;
}

errr parse_room_line(room_ptr room, char *line, int options)
{
    /* T:Type:Subtype[:Flags] */
    if (line[0] == 'T' && line[1] == ':')
        return _parse_room_type(line + 2, room);

    /* W:Level:MaxLevel:Rarity */
    else if (line[0] == 'W' && line[1] == ':')
    {
        char *zz[10];
        int   num = tokenize(line + 2, 10, zz, 0);
        int   tmp;

        if (num != 3)
        {
            msg_print("Error: Invalid W: line. Syntax: W:<Level>:<MaxLevel>:<Rarity>.");
            return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        }
        room->level = atoi(zz[0]);
        if (streq(zz[1], "*"))
            room->max_level = 0;
        else
            room->max_level = atoi(zz[1]);
        tmp = atoi(zz[2]);
        if (tmp < 0 || tmp > 255) /* room_t.rarity is a byte */
        {
            msg_format("Error: Invalid rarity %d. Enter a value between 1 and 255.", tmp);
            return PARSE_ERROR_OUT_OF_BOUNDS;
        }
        room->rarity = tmp;
    }

    /* L:X:... */
    else if (line[0] == 'L' && line[1] == ':')
    {
        int rc;
        room_grid_ptr letter = malloc(sizeof(room_grid_t));
        memset(letter, 0, sizeof(room_grid_t));
        letter->letter = line[2];
        rc = parse_room_grid(line + 4, letter, options);
        if (!rc) 
        {
        /* Check for the following case: a quest line is edited during the game, 
         * and the player has already requested or completed a quest that happens
         * later in the new line without having requested, completed or received a
         * reward for an earlier quest. This can make either a quest reward or an
         * entire quest inaccessible. To avoid this, we use a very ugly hack:
         * buildings set to an Untaken quest or a Completed quest (meaning an
         * unrewarded one) can only be overwritten by setting them to another 
         * Completed quest. (We allow that so that a player who completes the
         * later quest first can receive the reward immediately, without having to
         * take the earlier quest.) */
            if ((letter->cave_feat >= 128) && (letter->cave_feat < 160))
            {
                room_grid_ptr old_grid = int_map_find(room->letters, letter->letter);
                while ((old_grid) && (old_grid->extra > 0) && (old_grid->cave_feat == letter->cave_feat) 
                      && (old_grid->extra != letter->extra))
                {
                    quest_ptr q = quests_get(old_grid->extra);
                    if (!q || !q->id) break;
                    if ((q->status == QS_UNTAKEN) || (q->status == QS_COMPLETED))
                    {
                        if (letter->extra < 1) letter->extra = old_grid->extra;
                        else {
                            quest_ptr q2 = quests_get(letter->extra);   
                            if (!q2 || !q2->id) letter->extra = old_grid->extra;
                            else if (q2->status != QS_COMPLETED) letter->extra = old_grid->extra;
                        }
                    }
                    break;
                }
            }
            int_map_add(room->letters, letter->letter, letter);
        }
        else free(letter);
        if (rc) return rc;
    }

    /* Process 'M'ap lines */
    else if (line[0] == 'M' && line[1] == ':')
    {
        /* Acquire the text */
        char *s = line+2;

        /* Calculate room dimensions automagically */
        room->height++;
        if (!room->width)
            room->width = strlen(s);
        else if (strlen(s) != room->width)
        {
            msg_format(
                "Error: Inconsistent map widths. Room width auto-calculated to %d but "
                "current line is %d. Please make all map lines the same length.",
                room->width, strlen(s)
            );
            return PARSE_ERROR_GENERIC;
        }
        vec_push(room->map, (vptr)z_string_make(s));
    }
    /* !:SCRAMBLE(a,b,c,d) */
    else if (line[0] == '!' && line[1] == ':')
    {
        char *name;
        char *args[10];
        int   arg_ct = parse_args(line + 2, &name, args, 10);

        if (arg_ct < 0)
        {
            msg_format("Error: Malformed argument %s. Missing )?", name);
            return PARSE_ERROR_GENERIC;
        }

        if (streq(name, "SCRAMBLE") || streq(name, "SHUFFLE"))
        {
            #define _max_scramble 20 /* Windows */
            char letters[_max_scramble], scrambles[_max_scramble];
            int  i;
            if (arg_ct > _max_scramble)
            {
                msg_format("I can only scramble %d letters.", _max_scramble);
                return PARSE_ERROR_GENERIC;
            }
            for (i = 0; i < arg_ct; i++)
            {
                letters[i] = args[i][0];
                scrambles[i] = letters[i];
            }
            for (i = 0; i < arg_ct - 1; i++) /* Skiena: _Algorithm_Design_Manual_ p248 */
            {
                int j = rand_range(i, arg_ct - 1);
                char t = scrambles[i];
                scrambles[i] = scrambles[j];
                scrambles[j] = t;
            }
            for (i = 0; i < arg_ct; i++)
            {
                char letter = letters[i];
                char scramble = scrambles[i];
                room_grid_ptr grid = int_map_find(room->letters, letter);
                if (!grid)
                {
                    msg_format("Error: Undefined letter: %c", letter);
                    return PARSE_ERROR_GENERIC;
                }
                grid->scramble = scramble; /* cf _find_room_grid (rooms.c) */
                grid = int_map_find(room->letters, scramble);
                if (!grid)
                {
                    msg_format("Error: Undefined scramble: %c", scramble);
                    return PARSE_ERROR_GENERIC;
                }
                if (trace_doc)
                    doc_printf(trace_doc, "Scrambled <color:R>%c</color> to <color:B>%c</color>.\n", letter, scramble);
            }
        }
        else
        {
            msg_format("Error: Unknown directive: %s", name);
            return PARSE_ERROR_GENERIC;
        }
        return 0;
    }
    return 0;
}

/*
 * Parse a sub-file of the "extra info"
 */
static errr process_dungeon_file_aux(char *buf, int options)
{
    /* Skip "empty" lines */
    if (!buf[0]) return (0);

    /* Skip "blank" lines */
    if (isspace(buf[0])) return (0);

    /* Skip comments */
    if (buf[0] == '#') return (0);

    /* Require "?:*" format */
    if (buf[1] != ':') return (1);


    /* Process "%:<fname>" */
    if (buf[0] == '%')
    {
        return process_dungeon_file(buf + 2, options);
    }

    /* Process "W:<command>: ..." -- info for the wilderness */
    else if (buf[0] == 'W')
    {
        return parse_line_wilderness(buf, options);
    }

    /* Failure */
    return (1);
}


static char tmp[255];
static cptr variant_name = "FROGCOMPOSBAND";

/*
 * Helper function for "process_dungeon_file()"
 */
static cptr process_dungeon_file_expr(char **sp, char *fp)
{
    cptr v;

    char *b;
    char *s;

    char b1 = '[';
    char b2 = ']';

    char f = ' ';

    /* Initial */
    s = (*sp);

    /* Skip spaces */
    while (isspace(*s)) s++;

    /* Save start */
    b = s;

    /* Default */
    v = "?o?o?";

    /* Analyze */
    if (*s == b1)
    {
        const char *p;
        const char *t;

        /* Skip b1 */
        s++;

        /* First */
        t = process_dungeon_file_expr(&s, &f);

        /* Oops */
        if (!*t)
        {
            /* Nothing */
        }

        /* Function: IOR */
        else if (streq(t, "IOR") || streq(t, "OR"))
        {
            v = "0";
            while (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
                if (*t && !streq(t, "0")) v = "1";
            }
        }

        /* Function: AND */
        else if (streq(t, "AND"))
        {
            v = "1";
            while (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
                if (*t && streq(t, "0")) v = "0";
            }
        }

        /* Function: NOT */
        else if (streq(t, "NOT"))
        {
            v = "1";
            while (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
                if (*t && streq(t, "1")) v = "0";
            }
        }

        /* Function: EQU */
        else if (streq(t, "EQU"))
        {
            v = "0";
            if (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = process_dungeon_file_expr(&s, &f);
                if (streq(t, p)) v = "1";
            }
        }

        /* Function: NEQ */
        else if (streq(t, "NEQ"))
        {
            v = "1";
            if (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = process_dungeon_file_expr(&s, &f);
                if (streq(t, p)) v = "0";
            }
        }

        /* Function: MOD */
        else if (streq(t, "MOD"))
        {
            int x = 0;
            int y = 0;
            v = "0";
            if (*s && (f != b2))
            {
                x = atoi(process_dungeon_file_expr(&s, &f));
            }
            if(*s && (f != b2))
            {
                y = atoi(process_dungeon_file_expr(&s, &f));
                sprintf(tmp, "%d", x%y);
                v = tmp;
            }
        }

        /* Function: LEQ */
        else if (streq(t, "LEQ"))
        {
            v = "1";
            if (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = t;
                t = process_dungeon_file_expr(&s, &f);
                if (*t && atoi(p) > atoi(t)) v = "0";
            }
        }

        /* Function: GEQ */
        else if (streq(t, "GEQ"))
        {
            v = "1";
            if (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = t;
                t = process_dungeon_file_expr(&s, &f);

                /* Compare two numbers instead of string */
                if (*t && atoi(p) < atoi(t)) v = "0";
            }
        }

        /* Oops */
        else
        {
            while (*s && (f != b2))
            {
                t = process_dungeon_file_expr(&s, &f);
            }
        }

        /* Verify ending */
        if (f != b2) v = "?x?x?";

        /* Extract final and Terminate */
        if ((f = *s) != '\0') *s++ = '\0';
    }

    /* Other */
    else
    {
        /* Accept all printables except spaces and brackets */
        while (isprint(*s) && !my_strchr(" []", *s)) ++s;

        /* Extract final and Terminate */
        if ((f = *s) != '\0') *s++ = '\0';

        /* Variable */
        if (*b == '$')
        {
            /* System */
            if (streq(b+1, "SYS"))
            {
                v = ANGBAND_SYS;
            }

            /* Graphics */
            else if (streq(b+1, "GRAF"))
            {
                v = ANGBAND_GRAF;
            }
            else if (streq(b+1, "MONOCHROME"))
            {
                if (arg_monochrome)
                    v = "ON";
                else
                    v = "OFF";
            }
            /* Race */
            else if (streq(b+1, "RACE"))
            {
                v = get_true_race()->name;
                while (1)
                {
                    unsigned int paikka = strpos(" ", v);
                    if (!paikka) break;
                    sprintf(tmp, v);
                    tmp[paikka - 1] = '-';
                    v = tmp;
                }
            }
            else if (streq(b+1, "SUBRACE"))
            {
                v = get_true_race()->subname;
            }
            /* Class */
            else if (streq(b+1, "CLASS"))
            {
                v = get_class()->name;
            }
            else if (streq(b+1, "SUBCLASS"))
            {
                v = get_class()->subname;
                if (!v) v = "why are we here";
            }
            /* Realms */
            else if (streq(b+1, "REALM1"))
            {
                v = realm_names[p_ptr->realm1];
            }
            else if (streq(b+1, "REALM2"))
            {
                v = realm_names[p_ptr->realm2];
            }
            /* Player name */
            else if (streq(b+1, "PLAYER"))
            {
                static char tmp_player_name[32];
                char *pn, *tpn;
                for (pn = player_name, tpn = tmp_player_name; *pn; pn++, tpn++)
                {
                    *tpn = my_strchr(" []", *pn) ? '_' : *pn;
                }
                *tpn = '\0';
                v = tmp_player_name;
            }

            /* Town */
            else if (streq(b+1, "TOWN"))
            {
                sprintf(tmp, "%d", p_ptr->town_num);
                v = tmp;
            }

            /* Level */
            else if (streq(b+1, "LEVEL"))
            {
                sprintf(tmp, "%d", p_ptr->lev);
                v = tmp;
            }

            /* Quest status */
            else if (prefix(b+1, "QUEST"))
            {
                cptr _status[] = { "Untaken", "Taken", "InProgress", "Completed", "Finished", "Failed", "FailedDone" };
                int  which = atoi(b+6);
                quest_ptr q = quests_get(which);
                if (q) sprintf(tmp, "%s", _status[q->status]);
                else sprintf(tmp, "Unknown");
                v = tmp;
            }

            /* Random
               OLD: $RANDOM7 evaluated as seed_town % 7
               NEW: [MOD $RANDOM37 7] where 37 refers to the seed
               for quest 37. Every quest gets assigned a new seed
               when it is accepted in bldg.c.
            */
            else if (prefix(b+1, "RANDOM"))
            {
                int q_idx = atoi(b+7);
                /* "RANDOM" uses a special parameter to determine the number of the quest */
                if (q_idx == 0) sprintf(tmp, "%d", p_ptr->quest_seed);
                else sprintf(tmp, "%d", quests_get(q_idx)->seed);
                v = tmp;
            }

            /* Variant name */
            else if (streq(b+1, "VARIANT"))
            {
                v = variant_name;
            }

            /* Wilderness */
            else if (streq(b+1, "WILDERNESS"))
            {
                if (no_wilderness)
                    sprintf(tmp, "NONE");
                else
                    sprintf(tmp, "NORMAL");
                v = tmp;
            }

            else if (streq(b+1, "SPECIALITY"))
            {
                if (p_ptr->pclass == CLASS_WEAPONMASTER)
                    sprintf(tmp, "%s", weaponmaster_speciality_name(p_ptr->psubclass));
                else if (p_ptr->pclass == CLASS_DEVICEMASTER)
                    sprintf(tmp, "%s", devicemaster_speciality_name(p_ptr->psubclass));
                else
                    sprintf(tmp, "None");
                v = tmp;
            }
        }

        /* Constant */
        else
        {
            v = b;
        }
    }

    /* Save */
    (*fp) = f;

    /* Save */
    (*sp) = s;

    /* Result */
    return (v);
}

/* XXX Replace with parse_edit_file ... */
errr process_dungeon_file(cptr name, int options)
{
    FILE      *fp;
    char       buf[1024];
    int        num = 0;
    errr       err = 0;
    bool       bypass = FALSE;

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, name);

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* No such file */
    if (!fp) return (-1);

    /* Process the file */
    while (0 == my_fgets(fp, buf, sizeof(buf)))
    {
        /* Count lines */
        num++;


        /* Skip "empty" lines */
        if (!buf[0]) continue;

        /* Skip "blank" lines */
        if (isspace(buf[0])) continue;

        /* Skip comments */
        if (buf[0] == '#') continue;


        /* Process "?:<expr>" */
        if ((buf[0] == '?') && (buf[1] == ':'))
        {
            char f;
            cptr v;
            char *s;

            /* Start */
            s = buf + 2;

            /* Parse the expr */
            v = process_dungeon_file_expr(&s, &f);

            /* Set flag */
            bypass = (streq(v, "0") ? TRUE : FALSE);

            /* Continue */
            continue;
        }

        /* Apply conditionals ... INIT_DEBUG is for testing purposes */
        if (!(options & INIT_DEBUG) && bypass) continue;

        err = process_dungeon_file_aux(buf, options);

        /* Oops */
        if (err) break;
    }

    /* Errors */
    if (err)
    {
        cptr oops;

        /* Error string */
        oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

        /* Oops */
        msg_boundary();
        msg_format("<color:v>Error</color> %d (%s) at line %d of '%s'.", err, oops, num, name);
        msg_format("Parsing '%s'.", buf);

        msg_print(NULL);
    }

    /* Close the file */
    my_fclose(fp);

    /* Result */
    return (err);
}

errr parse_edit_file(cptr name, parser_f parser, int options)
{
    FILE *fp;
    char  buf[1024];
    int   line_num = 0;
    errr  err = 0;
    bool  bypass = FALSE;

    assert(parser);

    path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, name);
    fp = my_fopen(buf, "r");
    if (!fp) return (-1);

    while (!err && 0 == my_fgets(fp, buf, sizeof(buf)))
    {
        line_num++;

        if (!buf[0]) continue; /* empty line */
        if (isspace(buf[0])) continue; /* blank line */
        if (buf[0] == '#') continue; /* comment */
        if (buf[0] == '?' && buf[1] == ':') /* ?:<exp> conditional controls subsequent parsing */
        {
            char f;
            cptr v;
            char *s;

            s = buf + 2;
            v = process_dungeon_file_expr(&s, &f);
            bypass = streq(v, "0"); /* skip until subsequent ?: returns true */
            continue;
        }
        if (!(options & INIT_DEBUG) && bypass) continue; /* apply skip unless debugging */

        if (buf[0] == '%' && buf[1] == ':') /* %:file.txt */
            err = parse_edit_file(buf + 2, parser, options);
        else
        {
            if (trace_doc && (buf[0] == 'L' || buf[0] == 'R'))
            {
                doc_printf(trace_doc, "<color:R>%s:%d</color> <indent>%s</indent>\n",
                    name, line_num, buf);
            }
            err = parser(buf, options);
            if ((err) && (!(options & INIT_SILENT))) /* report now for recursion */
            {
                cptr oops = (err > 0 && err < PARSE_ERROR_MAX) ? err_str[err] : "unknown";

                msg_boundary();
                msg_format("<color:v>Error</color> %d (%s) at line %d of '%s'.", err, oops, line_num, name);
                msg_format("Parsing '%s'.", buf);
                msg_print(NULL); /* quit() during initialization ... */
            }
        }
    }

    my_fclose(fp);
    return err;
}


