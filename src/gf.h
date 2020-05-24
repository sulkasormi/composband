#ifndef INCLUDED_GF_H
#define INCLUDED_GF_H

/* Player and monster effects. GF_ is a mysterious historical abbreviation
 * Note: I reordered constants for the mon_spell system to provide sensible
 *       sorting (cf Possessor Spell UI).
 * Note: These constants are used in savefiles for purposes of monster lore.
 *       Any changes or reordering should be avoided (add to end instead).*/
enum {
    GF_NONE = 0,  /* if (type) project(..., type,...) */

    /* Elemental Effects */
    GF_ACID,
    GF_ELEC,
    GF_FIRE,
    GF_COLD,
    GF_POIS,
    GF_LITE,
    GF_DARK,
    GF_CONFUSION,
    GF_NETHER,
    GF_NEXUS,
    GF_SOUND,
    GF_SHARDS,
    GF_CHAOS,
    GF_DISENCHANT,
    GF_TIME,

    GF_MANA,
    GF_GRAVITY,
    GF_INERT,
    GF_PLASMA,
    GF_FORCE,
    GF_NUKE,
    GF_DISINTEGRATE,
    GF_STORM,
    GF_HOLY_FIRE,
    GF_HELL_FIRE,
    GF_ICE,
    GF_WATER,
    GF_ROCKET,
    GF_METEOR,
    GF_ROCK,
    GF_ARROW,
    GF_MISSILE,

    /* Curses */
    GF_CAUSE_1,
    GF_CAUSE_2,
    GF_CAUSE_3,
    GF_CAUSE_4,
    GF_HAND_DOOM,
    GF_BLOOD_CURSE,

    /* Mental Attacks */
    GF_PSY_SPEAR,
    GF_PSI,
    GF_PSI_DRAIN,
    GF_PSI_EGO_WHIP,
    GF_PSI_BRAIN_SMASH,
    GF_PSI_STORM,
    GF_TELEKINESIS,
    GF_DOMINATION,
    GF_SUBJUGATION,
    GF_DRAIN_MANA,
    GF_MIND_BLAST,
    GF_BRAIN_SMASH,
    GF_AMNESIA,

    /* Status Effects */
    GF_BLIND,
    GF_OLD_CLONE,
    GF_OLD_POLY,
    GF_OLD_HEAL,
    GF_STAR_HEAL,  /* smash potion effect */
    GF_OLD_SPEED,
    GF_OLD_SLOW,
    GF_OLD_CONF,
    GF_OLD_SLEEP,
    GF_OLD_DRAIN,
    GF_STASIS,
    GF_STASIS_EVIL,
    GF_PARALYSIS,
    GF_STUN,
    GF_ELDRITCH,
    GF_ANTIMAGIC,
    GF_CRUSADE,
    GF_UNHOLY_WORD,
    GF_UNLIFE,

    /* Terrain Effects */
    GF_LITE_WEAK,
    GF_DARK_WEAK,
    GF_KILL_WALL,
    GF_KILL_DOOR,
    GF_KILL_TRAP,
    GF_REMOVE_OBSTACLE,
    GF_MAKE_DOOR,
    GF_MAKE_TRAP,
    GF_MAKE_TREE,
    GF_MAKE_GLYPH,
    GF_MAKE_WALL,
    GF_JAM_DOOR,
    GF_WATER_FLOW,
    GF_WATER2,
    GF_LAVA_FLOW,
    GF_WEB,
    GF_QUAKE,

    /* Turning, Dispelling, Controlling, etc */
    GF_AWAY_UNDEAD,
    GF_AWAY_EVIL,
    GF_AWAY_ALL,
    GF_ISOLATION,
    GF_TURN_UNDEAD,
    GF_TURN_EVIL,
    GF_TURN_ALL,
    GF_DISP_UNDEAD,
    GF_DISP_EVIL,
    GF_DISP_GOOD,
    GF_DISP_DEMON,
    GF_DISP_LIVING,
    GF_DISP_ALL,
    GF_CONTROL_UNDEAD,
    GF_CONTROL_DEMON,
    GF_CONTROL_ANIMAL,
    GF_CONTROL_LIVING,
    GF_CONTROL_PACT_MONSTER,
    GF_CHARM,
    GF_CHARM_RING_BEARER,
    GF_CAPTURE,
    GF_ANIM_DEAD,
    GF_DEATH_RAY,
    GF_GENOCIDE,

    /* Object Effects */
    GF_IDENTIFY,

    /* Class Specific */
    GF_PHOTO,
    GF_ATTACK,
    GF_ENGETSU,
    GF_SEEKER,
    GF_SUPER_RAY,
    GF_BLOOD,
    GF_ELDRITCH_STUN,
    GF_ELDRITCH_DRAIN,
    GF_ELDRITCH_DISPEL,
    GF_ELDRITCH_CONFUSE,
    GF_ELDRITCH_HOWL,
    GF_ENTOMB,
    GF_MANA_CLASH,
    GF_PHARAOHS_CURSE,
    GF_DRAINING_TOUCH,
    GF_DEATH_TOUCH,
    GF_STEAL,

    /* New Stuff (unsorted) */
    /* Add new effects here. Reorganize later for next major version upgrade. */
    GF_SLOW,
    GF_CHICKEN,
    GF_BOMB,
    GF_AIR,
    GF_BABY_SLOW,

    GF_COUNT  /* enumerate 0 <= i < GF_COUNT */
};            /* allocate gf[GF_COUNT] */

typedef struct {
    int  id;
    cptr name;
    byte color;
    int  resist;
    cptr parse;
    byte flags;
} gf_info_t, *gf_info_ptr;

#define GFF_ATTACK 0x01
#define GFF_STATUS 0x02
#define GFF_TERRAIN 0x04
#define GFF_UTILITY 0x08

extern gf_info_ptr gf_parse_name(cptr token);
extern gf_info_ptr gf_lookup(int id);

/* Directly damage the player or a monster with a GF_* attack type, 
 * bypassing the project() code. This is needed for monster melee,
 * monster auras, and player innate attacks where project() is overly
 * complicated and error prone if flg is incorrectly set.
 *
 * We still need a "who" done it parameter. This is either the player (0)
 * or a monster index (m_idx > 0). Occasionally, it might be a special
 * negative code when using project(), but not for the gf_affect_* routines.
 */
#define GF_WHO_PLAYER      0  /* same as PROJECT_WHO_PLAYER */
#define GF_WHO_TRAP       -3  /* same as PROJECT_WHO_TRAP */

/* We also need information on whether the effect is spell/breath based,
 * or whether it is the result of melee contact. Mostly, this if for message
 * purposes, but, occasionally, it has gameplay effects. For example, GF_DISENCHANT
 * will usually dispel_player() on a touch effect (GF_AFFECT_ATTACK or _AURA)
 * while it always apply_disenchant()s on a breath (GF_AFFECT_SPELL).
 * The flags param only contains the following GF_AFFECT_* flags, and never
 * needs any of the PROJECT_* stuff. At the moment, using bits is inappropriate
 * as the options are mutually exclusive, but other options may follow in the future.
 */
#define GF_AFFECT_SPELL  0x01 /* Monster spell or breath from a project() */
#define GF_AFFECT_ATTACK 0x02 /* Monster melee B:HIT:HURT(10d10):DISENCHANT */
#define GF_AFFECT_AURA   0x04 /* Monster aura  A:DISENCHANT(3d5) */
#define GF_AFFECT_TRAP   0x08
#define GF_NO_PAIN       0x10 /* Hide pain messages */
extern int gf_affect_p(int who, int type, int dam, int flags);
extern bool gf_affect_m(int who, mon_ptr mon, int type, int dam, int flags);

/* exposed for the sake of wizard commands: calculate damage based
 * on the player's alignment */
extern int gf_holy_dam(int dam);
extern int gf_hell_dam(int dam);

/* XXX Remove these ... */
extern int gf_distance_hack;
#endif
