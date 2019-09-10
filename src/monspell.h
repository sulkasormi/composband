#ifndef INCLUDED_MON_SPELL_H
#define INCLUDED_MON_SPELL_H

/* Monster Spell System */

/* Monster Spell Type (MST_*)
 * The spell type is used to intelligently choose spells depending on
 * the current situation. For example, wounded monsters will prefer
 * to MST_HEAL or possibly MST_ESCAPE. If the player is not "projectable",
 * then the attack types are excluded, but we might keep MST_BREATH and
 * MST_BALL around if the player can be "splashed" by the effect. Etc. */
enum {
    MST_BREATH,
    MST_BALL,
    MST_BOLT,
    MST_BEAM,
    MST_CURSE,
    MST_BUFF,
    MST_BIFF,
    MST_ESCAPE,
    MST_ANNOY,
    MST_SUMMON,
    MST_HEAL,
    MST_TACTIC,
    MST_WEIRD,
    MST_POSSESSOR,
    MST_COUNT
};

/* Monster Spell Flags (MSF_*) */
#define MSF_INNATE 0x0001
#define MSF_BALL0  0x0002 /* hack for THROW = {MST_BALL, GF_ROCK} */
#define MSF_BALL4  0x0004 /* hack for Mana Storm, Starburst, et. al. */
#define MSF_TARGET 0x0008 /* spell requires a target (possessor) */
#define MSF_DIRECT 0x0010 /* non-projection; requires monster target, not location */

/* Every spell can be identified by (Type, Effect) pair */
typedef struct {
    byte type;
    s16b effect;
} mon_spell_id_t;

extern mon_spell_id_t mon_spell_id(int type, int effect);
extern int            mon_spell_hash(mon_spell_id_t id);

/* Every spell can be parameterized (MSP_*) */
enum {
    MSP_NONE = 0,
    MSP_DICE,     /* for damage or duration of timed effects */
    MSP_HP_PCT,   /* percentage of chp for damage up to a max */
};
typedef struct {  /* XdY+Z ... Note: Mana Bolt needs a very high ds */
    s16b dd;
    s16b ds;     
    s16b base;
} dice_t;
typedef struct {  /* MIN(m->chp*pct/100, max) */
    byte pct;
    s16b max;
} hp_pct_t;
typedef struct {
    union {
        dice_t   dice;
        hp_pct_t hp_pct;
    } v;
    byte tag;
} mon_spell_parm_t, *mon_spell_parm_ptr;

extern mon_spell_parm_t mon_spell_parm_dice(int dd, int ds, int base);
extern mon_spell_parm_t mon_spell_parm_hp_pct(int pct, int max);
extern mon_spell_parm_t mon_spell_parm_default(mon_spell_id_t id, int rlev);
extern errr             mon_spell_parm_parse(mon_spell_parm_ptr parm, char *token);
extern void             mon_spell_parm_print(mon_spell_parm_ptr parm, string_ptr s, mon_race_ptr race);

/* Casting information for a spell (idea from Vanilla) */
typedef struct {
    cptr name;
    byte color;
    cptr cast_msg;
    cptr blind_msg;
    cptr cast_mon_msg;
    cptr cast_plr_msg;
} mon_spell_display_t, *mon_spell_display_ptr;

/* A single monster spell */
typedef struct {
    mon_spell_id_t   id;
    mon_spell_parm_t parm;
    mon_spell_display_ptr
                     display;
    s16b             lore;
    byte             prob;
    byte             flags;
} mon_spell_t, *mon_spell_ptr;

extern errr mon_spell_parse(mon_spell_ptr spell, int rlev, char *token);
extern void mon_spell_print(mon_spell_ptr spell, string_ptr s);
extern void mon_spell_display(mon_spell_ptr spell, string_ptr s); /* helper for mon_display */
extern void mon_spell_doc(mon_spell_ptr spell, doc_ptr doc);
extern int  mon_spell_avg_dam(mon_spell_ptr spell, mon_race_ptr race, bool apply_resist);

/* A collection of related spells, grouped together for tactical purposes.
 * Each tactical group has a dynamic probability depending on the current
 * context (e.g., heal when wounded, no offense when player out of los, etc.) */
typedef struct {
    byte type;   /* MST_* for the group */
    byte prob;
    byte count;
    byte allocated;
    mon_spell_ptr spells; /* vec<mon_spell_t> */
} mon_spell_group_t, *mon_spell_group_ptr;

extern mon_spell_group_ptr mon_spell_group_alloc(void);
extern void mon_spell_group_free(mon_spell_group_ptr group);
extern void mon_spell_group_add(mon_spell_group_ptr group, mon_spell_ptr spell);
extern mon_spell_ptr mon_spell_group_find(mon_spell_group_ptr group, mon_spell_id_t id);

typedef struct {
    byte freq;
    u32b flags;
    mon_spell_group_ptr groups[MST_COUNT];
} mon_spells_t, *mon_spells_ptr;

extern mon_spells_ptr mon_spells_alloc(void);
extern void           mon_spells_free(mon_spells_ptr spells);
extern void           mon_spells_add(mon_spells_ptr spells, mon_spell_ptr spell);
extern errr           mon_spells_parse(mon_spells_ptr spells, int rlev, char *token);
extern vec_ptr        mon_spells_all(mon_spells_ptr spells);
extern mon_spell_ptr  mon_spells_find(mon_spells_ptr spells, mon_spell_id_t id);
extern mon_spell_ptr  mon_spells_random(mon_spells_ptr spells); /* stupid monsters */
extern void           mon_spells_load(mon_spells_ptr spells, savefile_ptr file);
extern void           mon_spells_save(mon_spells_ptr spells, savefile_ptr file);

/* Finally, it is time to cast a spell!
 * Note, this is slightly more complicated then I would like since monsters
 * may splash the player. We need not just a spell from the AI, but possibly
 * a target location as well. Also, monsters may cast spells at other monster
 * and certain players can use monster spells directly (e.g. Possessor).
 * We'll group everything we need to actually cast a spell into the following
 * complicated mon_spell_cast (MSC) struct: */
#define MSC_SRC_MONSTER  0x0001
#define MSC_SRC_PLAYER   0x0002  /* Players can cast monster spells: Possessor, Blue-Mage, Imitator */
#define MSC_DEST_MONSTER 0x0004
#define MSC_DEST_PLAYER  0x0008
#define MSC_DEST_SELF    0x0010
#define MSC_DIRECT       0x0020
#define MSC_SPLASH       0x0040
#define MSC_UNVIEW       0x0080
typedef struct {
    mon_ptr       mon;             /* Src monster or null if MSC_SRC_PLAYER */
    char          name[MAX_NLEN];
    mon_race_ptr  race;            /* race->spells contains accurate probabilities for debugging */
    mon_spell_ptr spell;
    point_t       src;
    point_t       dest;            /* Might be near MSC_DEST_* if MSC_SPLASH */
    mon_ptr       mon2;            /* Dest monster if MSC_DEST_MONSTER */
    char          name2[MAX_NLEN];
    u32b          flags;
} mon_spell_cast_t, *mon_spell_cast_ptr;

/* Allow clients to plug in a smarter/alternative AI */
typedef bool (*mon_spell_ai)(mon_spell_cast_ptr cast);
extern bool           mon_spell_ai_wizard(mon_spell_cast_ptr cast);

extern int            mon_spell_cost(mon_spell_ptr spell, mon_race_ptr race);
extern bool           mon_spell_cast(mon_ptr mon, mon_spell_ai ai);
extern bool           mon_spell_cast_mon(mon_ptr mon, mon_spell_ai ai);
extern void           mon_spell_wizard(mon_ptr mon, mon_spell_ai ai, doc_ptr doc);
extern mon_spell_ptr  mon_spell_find(mon_race_ptr race, mon_spell_id_t id);
extern bool           mon_spell_cast_possessor(mon_race_ptr race);

/* Some classes need to know what spell is being cast, and who is doing it: */
extern mon_spell_ptr  mon_spell_current(void);
extern mon_ptr        mon_current(void);

extern bool mon_could_splash(mon_ptr mon, point_t tgt);
extern bool mon_is_magical(mon_ptr mon);
extern bool mon_race_is_magical(mon_race_ptr race);
extern bool mon_has_attack_spell(mon_ptr mon);
extern bool mon_race_has_attack_spell(mon_race_ptr race);
extern bool mon_has_worthy_attack_spell(mon_ptr mon);
extern bool mon_race_has_worthy_attack_spell(mon_race_ptr race);
extern bool mon_has_innate_spell(mon_ptr mon);
extern bool mon_race_has_innate_spell(mon_race_ptr race);
extern bool mon_has_summon_spell(mon_ptr mon);
extern bool mon_race_has_summon_spell(mon_race_ptr race);
extern bool mon_has_spell_type(mon_ptr mon, int type);
extern bool mon_race_has_spell_type(mon_race_ptr race, int type);
extern int  mon_spell_freq(mon_ptr mon);
extern int  mon_race_spell_freq(mon_race_ptr race);
extern bool mon_race_has_invulnerability(mon_race_ptr race);
extern bool mon_race_has_healing(mon_race_ptr race);
extern bool mon_race_has_drain_mana(mon_race_ptr race);
extern bool mon_race_can_summon(mon_race_ptr race, int summon_type);
extern bool mon_race_can_teleport(mon_race_ptr race);
extern bool mon_race_has_lite_dark_spell(mon_race_ptr race);
extern bool mon_race_has_dispel(mon_race_ptr race);

/* Blue-Mage things that need local monspell.c stuff */
extern void list_spell_info(doc_ptr doc, mon_spell_ptr spell, mon_race_ptr race);
extern int  blue_mage_spell_fail_rate(mon_spell_ptr spell);
extern int  blue_mage_spell_order(byte type, s16b effect);
extern void blue_mage_learn_spell(void);
extern void blue_mage_learn_spell_aux(byte type, s16b effect, s16b lore, s16b seniority, bool noisy);
extern void blue_mage_update_parms(vec_ptr spells);
#endif

