#include "angband.h"
#include "gf.h"

#include <assert.h>

static gf_info_t _gf_tbl[GF_COUNT] = {
    { GF_NONE, "None", TERM_L_DARK, RES_INVALID, "NONE"},

    /* Elemental Effects */
    { GF_ACID, "Acid", TERM_GREEN, RES_ACID, "ACID" },
    { GF_ELEC, "Lightning", TERM_BLUE, RES_ELEC, "ELEC" },
    { GF_FIRE, "Fire", TERM_RED, RES_FIRE, "FIRE" },
    { GF_COLD, "Frost", TERM_L_WHITE, RES_COLD, "COLD" },
    { GF_POIS, "Poison", TERM_L_GREEN, RES_POIS, "POIS" },
    { GF_LITE, "Light", TERM_YELLOW, RES_LITE, "LITE" },
    { GF_DARK, "Darkness", TERM_L_DARK, RES_DARK, "DARK" },
    { GF_CONFUSION, "Confusion", TERM_L_UMBER, RES_CONF, "CONFUSION" },
    { GF_NETHER, "Nether", TERM_L_DARK, RES_NETHER, "NETHER" },
    { GF_NEXUS, "Nexus", TERM_VIOLET, RES_NEXUS, "NEXUS" },
    { GF_SOUND, "Sound", TERM_ORANGE, RES_SOUND, "SOUND" },
    { GF_SHARDS, "Shards", TERM_L_UMBER, RES_SHARDS, "SHARDS" },
    { GF_CHAOS, "Chaos", TERM_VIOLET, RES_CHAOS, "CHAOS" },
    { GF_DISENCHANT, "Disenchantment", TERM_VIOLET, RES_DISEN, "DISENCHANT" },
    { GF_TIME, "Time", TERM_L_BLUE, RES_TIME, "TIME" },

    { GF_MANA, "Mana", TERM_L_BLUE, RES_INVALID, "MANA" },
    { GF_GRAVITY, "Gravity", TERM_L_UMBER, RES_INVALID, "GRAVITY" },
    { GF_INERT, "Inertia", TERM_L_UMBER, RES_INVALID, "INERTIA" },
    { GF_PLASMA, "Plasma", TERM_L_RED, RES_INVALID, "PLASMA" },
    { GF_FORCE, "Force", TERM_L_BLUE, RES_INVALID, "FORCE" },
    { GF_NUKE, "Toxic Waste", TERM_L_GREEN, RES_POIS, "NUKE" },
    { GF_DISINTEGRATE, "Disintegration", TERM_L_DARK, RES_INVALID, "DISINTEGRATE" },
    { GF_STORM, "Storm Winds", TERM_BLUE, RES_INVALID, "STORM" },
    { GF_HOLY_FIRE, "Holy Fire", TERM_YELLOW, RES_INVALID, "HOLY_FIRE" },
    { GF_HELL_FIRE, "Hell Fire", TERM_L_DARK, RES_INVALID, "HELL_FIRE" },
    { GF_ICE, "Ice", TERM_L_WHITE, RES_COLD, "ICE" },
    { GF_WATER, "Water", TERM_L_BLUE, RES_INVALID, "WATER" },
    { GF_ROCKET, "Rocket", TERM_RED, RES_SHARDS, "ROCKET" },
    { GF_METEOR, "Meteor", TERM_RED, RES_INVALID, "METEOR" },
    { GF_ROCK, "Rock", TERM_L_UMBER, RES_INVALID, "ROCK" },
    { GF_ARROW, "Arrow", TERM_L_UMBER, RES_INVALID, "ARROW" },
    { GF_MISSILE, "Missile", TERM_L_UMBER, RES_INVALID, "MISSILE" },

    /* Curses */
    { GF_CAUSE_1, "Cause Light Wounds", TERM_RED, RES_INVALID, "CAUSE_1" },
    { GF_CAUSE_2, "Cause Serious Wounds", TERM_RED, RES_INVALID, "CAUSE_2" },
    { GF_CAUSE_3, "Cause Critical Wounds", TERM_RED, RES_INVALID, "CAUSE_3" },
    { GF_CAUSE_4, "Cause Mortal Wounds", TERM_RED, RES_INVALID, "CAUSE_4" },
    { GF_HAND_DOOM, "Hand of Doom", TERM_VIOLET, RES_INVALID, "HAND_DOOM" },
    { GF_BLOOD_CURSE, "Blood Curse", TERM_VIOLET, RES_INVALID, "BLOOD_CURSE" },

    /* Mental Attacks */
    { GF_PSY_SPEAR, "Psycho-Spear", TERM_L_BLUE, RES_INVALID, "PSY_SPEAR" },
    { GF_PSI, "Psionics", TERM_L_BLUE, RES_INVALID, "PSI" },
    { GF_PSI_DRAIN, "Psionic Drain", TERM_L_BLUE, RES_INVALID, "PSI_DRAIN" },
    { GF_PSI_EGO_WHIP, "Ego Whip", TERM_L_BLUE, RES_INVALID, "PSI_EGO_WHIP" },
    { GF_PSI_BRAIN_SMASH, "Brain Smash", TERM_L_BLUE, RES_INVALID, "PSI_BRAIN_SMASH" },
    { GF_PSI_STORM, "Psycho-Storm", TERM_L_BLUE, RES_INVALID, "PSI_STORM" },
    { GF_TELEKINESIS, "Pulverise", TERM_L_BLUE, RES_INVALID, "TELEKINESIS" },
    { GF_DOMINATION, "Domination", TERM_RED, RES_INVALID, "DOMINATION" },
    { GF_SUBJUGATION, "Subjugation", TERM_RED, RES_INVALID, "SUBJUGATION" },
    { GF_DRAIN_MANA, "Drain Mana", TERM_L_BLUE, RES_INVALID, "DRAIN_MANA" },
    { GF_MIND_BLAST, "Mind Blast", TERM_L_BLUE, RES_INVALID, "MIND_BLAST" },
    { GF_BRAIN_SMASH, "Brain Smash", TERM_L_BLUE, RES_INVALID, "BRAIN_SMASH" },
    { GF_AMNESIA, "Amnesia", TERM_L_DARK, RES_INVALID, "AMNESIA" },

    /* Status Effects */
    { GF_BLIND, "Blind", TERM_L_DARK, RES_INVALID, "BLIND" },
    { GF_OLD_CLONE, "Clone", TERM_RED, RES_INVALID, "OLD_CLONE" },
    { GF_OLD_POLY, "Polymorph", TERM_RED, RES_INVALID, "OLD_POLY" },
    { GF_OLD_HEAL, "Heal", TERM_WHITE, RES_INVALID, "OLD_HEAL" },
    { GF_STAR_HEAL, "Heal", TERM_WHITE, RES_INVALID, "STAR_HEAL" },
    { GF_OLD_SPEED, "Haste", TERM_L_RED, RES_INVALID, "OLD_SPEED" },
    { GF_OLD_SLOW, "Slow", TERM_L_UMBER, RES_INVALID, "OLD_SLOW" },
    { GF_OLD_CONF, "Confuse", TERM_L_UMBER, RES_INVALID, "OLD_CONF" },
    { GF_OLD_SLEEP, "Sleep", TERM_BLUE, RES_INVALID, "OLD_SLEEP" },
    { GF_OLD_DRAIN, "Drain", TERM_L_DARK, RES_INVALID, "OLD_DRAIN" },
    { GF_STASIS, "Freeze", TERM_BLUE, RES_INVALID, "STASIS" },
    { GF_STASIS_EVIL, "Freeze Evil", TERM_BLUE, RES_INVALID, "STASIS_EVIL" },
    { GF_PARALYSIS, "Paralyze", TERM_VIOLET, RES_INVALID, "PARALYZE" },
    { GF_STUN, "Stun", TERM_L_BLUE, RES_INVALID, "STUN" },
    { GF_ELDRITCH, "Eldritch Horror", TERM_VIOLET, RES_INVALID, "ELDRITCH" },
    { GF_ANTIMAGIC, "Anti-magic", TERM_RED, RES_INVALID, "ANTIMAGIC" },
    { GF_CRUSADE, "Crusade", TERM_WHITE, RES_INVALID, "CRUSADE" },
    { GF_UNHOLY_WORD, "Unholy Word", TERM_L_DARK, RES_INVALID, "UNHOLY_WORD" },
    { GF_UNLIFE, "Unlife", TERM_L_DARK, RES_INVALID, "UNLIFE" },

    /* Terrain Effects */
    { GF_LITE_WEAK, "Light", TERM_YELLOW, RES_INVALID, "LITE_WEAK" },
    { GF_DARK_WEAK, "Dark", TERM_L_DARK, RES_INVALID, "DARK_WEAK" },
    { GF_KILL_WALL, "Stone to Mud", TERM_L_UMBER, RES_INVALID, "KILL_WALL" },
    { GF_KILL_DOOR, "Door Destruction", TERM_RED, RES_INVALID, "KILL_DOOR" },
    { GF_KILL_TRAP, "Trap Destruction", TERM_RED, RES_INVALID, "KILL_TRAP" },
    { GF_REMOVE_OBSTACLE, "Remove Obstacle", TERM_RED, RES_INVALID, "REMOVE_OBSTACLE" },
    { GF_MAKE_DOOR, "Door Creation", TERM_L_BLUE, RES_INVALID, "MAKE_DOOR" },
    { GF_MAKE_TRAP, "Trap Creation", TERM_L_RED, RES_INVALID, "MAKE_TRAP" },
    { GF_MAKE_TREE, "Forest Creation", TERM_L_GREEN, RES_INVALID, "MAKE_TREE" },
    { GF_MAKE_GLYPH, "Glyph of Warding", TERM_L_BLUE, RES_INVALID, "MAKE_GLYPH" },
    { GF_MAKE_WALL, "Wall Creation", TERM_SLATE, RES_INVALID, "MAKE_WALL" },
    { GF_JAM_DOOR, "Wizard Lock", TERM_RED, RES_INVALID, "JAM_DOOR" },
    { GF_WATER_FLOW, "Flow of Water", TERM_BLUE, RES_INVALID, "WATER_FLOW" },
    { GF_WATER2, "Flow of Water", TERM_BLUE, RES_INVALID, "WATER2" },
    { GF_LAVA_FLOW, "Flow of Lava", TERM_RED, RES_INVALID, "LAVA_FLOW" },
    { GF_WEB, "Web Spinning", TERM_SLATE, RES_INVALID, "WEB" },
    { GF_QUAKE, "Earthquake", TERM_L_UMBER, RES_INVALID, "QUAKE" },

    /* Turning, Dispelling, Controlling, etc */
    { GF_AWAY_UNDEAD, "Banish Undead", TERM_L_BLUE, RES_INVALID, "AWAY_UNDEAD" },
    { GF_AWAY_EVIL, "Banish Evil", TERM_L_BLUE, RES_INVALID, "AWAY_EVIL" },
    { GF_AWAY_ALL, "Banishment", TERM_L_BLUE, RES_INVALID, "AWAY_ALL" },
    { GF_ISOLATION, "Isolation", TERM_L_BLUE, RES_INVALID, "ISOLATION" },
    { GF_TURN_UNDEAD, "Turn Undead", TERM_RED, RES_INVALID, "TURN_UNDEAD" },
    { GF_TURN_EVIL, "Turn Evil", TERM_RED, RES_INVALID, "TURN_EVIL" },
    { GF_TURN_ALL, "Turn Monsters", TERM_RED, RES_INVALID, "TURN_ALL" },
    { GF_DISP_UNDEAD, "Dispel Undead", TERM_L_RED, RES_INVALID, "DISP_UNDEAD" },
    { GF_DISP_EVIL, "Dispel Evil", TERM_L_RED, RES_INVALID, "DISP_EVIL" },
    { GF_DISP_GOOD, "Dispel Good", TERM_L_RED, RES_INVALID, "DISP_GOOD" },
    { GF_DISP_DEMON, "Dispel Demon", TERM_L_RED, RES_INVALID, "DISP_DEMON" },
    { GF_DISP_LIVING, "Dispel Living", TERM_L_RED, RES_INVALID, "DISP_LIVING" },
    { GF_DISP_ALL, "Dispel Monsters", TERM_L_RED, RES_INVALID, "DISP_ALL" },
    { GF_CONTROL_UNDEAD, "Enslave Undead", TERM_L_BLUE, RES_INVALID, "CONTROL_UNDEAD" },
    { GF_CONTROL_DEMON, "Dominate Demon", TERM_L_BLUE, RES_INVALID, "CONTROL_DEMON" },
    { GF_CONTROL_ANIMAL, "Charm Animal", TERM_L_BLUE, RES_INVALID, "CONTROL_ANIMAL" },
    { GF_CONTROL_LIVING, "Charm Living", TERM_L_BLUE, RES_INVALID, "CONTROL_LIVING" },
    { GF_CONTROL_PACT_MONSTER, "Control Pact Monster", TERM_L_BLUE, RES_INVALID, "CONTROL_PACT_MONSTER" },
    { GF_CHARM, "Charm Monster", TERM_L_BLUE, RES_INVALID, "CHARM" },
    { GF_CHARM_RING_BEARER, "Charm Ring Bearer", TERM_L_BLUE, RES_INVALID, "CHARM_RING_BEARER" },
    { GF_CAPTURE, "Capture Monster", TERM_L_BLUE, RES_INVALID, "CAPTURE" },
    { GF_ANIM_DEAD, "Raise Dead", TERM_L_DARK, RES_INVALID, "ANIM_DEAD" },
    { GF_DEATH_RAY, "Death Ray", TERM_L_DARK, RES_INVALID, "DEATH_RAY" },
    { GF_GENOCIDE, "Genocide", TERM_L_DARK, RES_INVALID, "GENOCIDE" },

    /* Object Effects */
    { GF_IDENTIFY, "Identify", TERM_L_BLUE, RES_INVALID, "IDENTIFY" },

    /* Class Specific */
    { GF_PHOTO, "Photograph", TERM_YELLOW, RES_INVALID, "PHOTO" },
    { GF_ATTACK, "Attack", TERM_RED, RES_INVALID, "ATTACK" },
    { GF_ENGETSU, "Moon Dazzling", TERM_YELLOW, RES_INVALID, "ENGETSU" },
    { GF_SEEKER, "Seeker Ray", TERM_YELLOW, RES_INVALID, "SEEKER" },
    { GF_SUPER_RAY, "Super Ray", TERM_YELLOW, RES_INVALID, "SUPER_RAY" },
    { GF_BLOOD, "Blood", TERM_RED, RES_INVALID, "BLOOD" },
    { GF_ELDRITCH_STUN, "Eldritch Stun", TERM_L_BLUE, RES_INVALID, "ELDRITCH_STUN" },
    { GF_ELDRITCH_DRAIN, "Eldritch Drain", TERM_L_DARK, RES_INVALID, "ELDRITCH_DRAIN" },
    { GF_ELDRITCH_DISPEL, "Eldritch Dispel", TERM_L_RED, RES_INVALID, "ELDRITCH_DISPEL" },
    { GF_ELDRITCH_CONFUSE, "Eldritch Confuse", TERM_L_UMBER, RES_INVALID, "ELDRITCH_CONFUSE" },
    { GF_ELDRITCH_HOWL, "Eldritch Howl", TERM_L_DARK, RES_INVALID, "ELDRITCH_HOWL" },
    { GF_ENTOMB, "Entomb", TERM_L_UMBER, RES_INVALID, "ENTOMB" },
    { GF_MANA_CLASH, "Mana Clash", TERM_L_BLUE, RES_INVALID, "MANA_CLASH" },
    { GF_PHARAOHS_CURSE, "Pharaoh's Curse", TERM_VIOLET, RES_INVALID, "PHARAOHS_CURSE" },
    { GF_DRAINING_TOUCH, "Draining Touch", TERM_L_DARK, RES_INVALID, "DRAINING_TOUCH" },
    { GF_DEATH_TOUCH, "Touch of Death", TERM_L_DARK, RES_INVALID, "DEATH_TOUCH" },
    { GF_STEAL, "Steal", TERM_WHITE, RES_INVALID, "STEAL" },

    /* New */
    { GF_SLOW, "Slow", TERM_ORANGE, RES_INVALID, "SLOW" },
    { GF_CHICKEN, "Chicken", TERM_YELLOW, RES_INVALID, "CHICKEN" },
    { GF_BOMB, "Bomb", TERM_SLATE, RES_INVALID, "BOMB" },
};

typedef struct {
    cptr parse;
    int id;
} _alias_t, *_alias_ptr;
static _alias_t _aliases[] = {
    { "DAM", GF_MISSILE },
    { "POISON", GF_POIS },
    { "LIGHT", GF_LITE },
    { "CONF", GF_CONFUSION },
    { "CONFUSE", GF_CONFUSION },
    { "DISENCHANTMENT", GF_DISENCHANT },
    { "PULVERISE", GF_TELEKINESIS },
    { "TERRIFY", GF_TURN_ALL },
    { "POLYMORPH", GF_OLD_POLY },
    { "BIG_SLOW", GF_OLD_SLOW },
    { "SLEEP", GF_OLD_SLEEP },
    { 0 }
};

gf_info_ptr gf_parse_name(cptr token)
{
    static str_map_ptr _map = NULL;
    if (!_map)
    {
        int i;
        _map = str_map_alloc(NULL);
        for (i = 0; i < GF_COUNT; i++)
        {
            gf_info_ptr info = &_gf_tbl[i];
            str_map_add(_map, info->parse, info);
        }
        for (i = 0; ; i++)
        {
            _alias_ptr alias = &_aliases[i];
            if (!alias->parse) break;
            str_map_add(_map, alias->parse, gf_lookup(alias->id));
        }
    }
    return str_map_find(_map, token);
}

gf_info_ptr gf_lookup(int id)
{
    gf_info_ptr gf = NULL;
    if (0 < id && id < GF_COUNT)
    {
        gf = &_gf_tbl[id];
        assert(gf->id == id);
    }
    return gf;
}

#define HURT_CHANCE 16

/* Stop using project() for direct damage effects! For one thing, monster auras
 * and attacks are not spells, so there is no learning/absorption/range damage
 * reduction, etc. For another, trying to get project() to actually work is extremely
 * obtuse ... You need to specify a half dozen flags to get it to actually work.
 * (The unexpected fact is that PROJECT_PLAYER still requires PROJECT_KILL to work).
 *
 * Cf monster melee, monster auras, monsters battling other monsters. There is 
 * absolutely no need for projection ... it is stupid! All we need is a way to 
 * directly damage player/monster with a specific attack type.
 * BTW, I have no idea what GF_ stands for ... 

   OLD: project(m_idx, 0, py, px, dam, aura->effect,
            PROJECT_KILL | PROJECT_PLAYER | PROJECT_HIDE |
            PROJECT_AIMED | PROJECT_JUMP | PROJECT_AURA, -1);  <== 13 parameters 
   NEW: gf_affect_p(m_idx, aura->effect, dam, GF_AFFECT_AURA); <==  4 parameters
 */
static int _rlev(int m_idx)
{
    if (m_idx > 0)
    {
        mon_ptr mon = &m_list[m_idx];
        mon_race_ptr race = &r_info[mon->/*ap_*/r_idx]; /* XXX */
        return race->level;
    }
    return 0;
}
static int _plr_save_odds(int m_idx, int boost)
{
    int rlev = _rlev(m_idx);
    int roll = 100 + rlev/2 + boost;
    int sav = duelist_skill_sav(m_idx);
    int odds = sav * 100 / roll;
    return odds;
}
static bool _plr_save(int m_idx, int boost)
{
    int odds = _plr_save_odds(m_idx, boost);
    return randint0(100) < odds;
}
static int _align_dam_pct(int align)
{
    static point_t tbl[6] = { {-150, 200}, {-50, 150}, {-10, 125},
                              {10, 80}, {50, 66}, {150, 50} };

    return interpolate(align, tbl, 6);
}
int gf_holy_dam(int dam)
{
    if ((prace_is_(RACE_MON_DEMON)) || (prace_is_(RACE_BALROG)) || (prace_is_(MIMIC_DRAGON))) dam += MIN(dam * 2 / 3, 30);
    return dam * _align_dam_pct(p_ptr->align) / 100;
}
int gf_hell_dam(int dam)
{
    if (((prace_is_(RACE_MON_ANGEL)) || (prace_is_(RACE_ARCHON))) && (p_ptr->align >= 0)) dam += MIN(dam * 2 / 3, 30);
    return dam * _align_dam_pct(-p_ptr->align) / 100;
}
static void _bomb_calc_dam(int *dam, int *shard_dam, int *sound_dam, int kuka)
{
    int rr;
    /* Bomb damage is weird because a) we need to calculate two types of damage,
     * shard damage and sound damage and b) they dissipate in different ways.
     * Code adapted from Frogspawn */
    *sound_dam = ((*dam) * 2 + 2) / 3;
    *shard_dam = ((*dam) - (*sound_dam));
    for (rr = 0; rr < gf_distance_hack; rr++)
    {
        *shard_dam -= ((*shard_dam) / 5);
    }
    *sound_dam = (((*sound_dam) + gf_distance_hack) / (gf_distance_hack + 1));
    if (kuka < 0)
    {
        *shard_dam = res_calc_dam(RES_SHARDS, *shard_dam);
        *sound_dam = res_calc_dam(RES_SOUND, *sound_dam);
    }
    else
    {
        monster_type *m_ptr = &m_list[kuka];
        if ((!m_ptr) || (!m_ptr->r_idx)) {} /* paranoia */
        else
        {
            monster_race *race = mon_race(m_ptr);
            if (race->flagsr & RFR_RES_SHAR)
            {
                (*shard_dam) *= 3; (*shard_dam) /= randint1(6) + 6;
            }
            if (race->flagsr & RFR_RES_SOUN)
            {
                (*sound_dam) *= 2; (*sound_dam) /= randint1(6) + 6;
            }
        }
    }
    *dam = (*sound_dam) + (*shard_dam);
}

static bool _failed_charm_nopet_chance(mon_ptr mon)
{
    if ((one_in_((p_ptr->spin > 0) ? 10 : 5)) && (!p_ptr->uimapuku) && (!is_friendly(mon))) return TRUE;
    return FALSE;
}

bool player_obviously_poly_immune(void)
{
    if ((prace_is_(RACE_ANDROID))
       || (p_ptr->pclass == CLASS_MONSTER)
       || (p_ptr->prace == RACE_DOPPELGANGER)
       || (p_ptr->prace == RACE_WEREWOLF)
       || (get_race()->flags & RACE_NO_POLY))
       return TRUE;
    return FALSE;
}

/* Water elementals have non-drainable fake mana */
bool player_mana_drainable(void)
{
    if (elemental_is_(ELEMENTAL_WATER)) return FALSE;
    if (mut_present(MUT_STRONG_MIND)) return FALSE;
    return TRUE;
}

static bool _mana_loss_save(monster_race *r_ptr)
{
    if (mut_present(MUT_STRONG_MIND)) return TRUE;
    if ( prace_is_(RACE_DEMIGOD)
        && p_ptr->psubrace == DEMIGOD_HERA
        && randint1(100) > r_ptr->level - 2*(p_ptr->stat_ind[A_WIS] + 3)) return TRUE;
    return FALSE;
}

int gf_affect_p(int who, int type, int dam, int flags)
{
    int          result = 0;
    mon_ptr      m_ptr = NULL;
    mon_race_ptr r_ptr = NULL;
    int          rlev = 1;
    char         m_name[MAX_NLEN], m_name_subject[MAX_NLEN], m_name_real[MAX_NLEN];
    bool         aura = BOOL(flags & GF_AFFECT_AURA);
    bool         touch = BOOL(flags & (GF_AFFECT_AURA | GF_AFFECT_ATTACK));
    bool         fuzzy = p_ptr->blind && (flags & GF_AFFECT_SPELL);
    int          damage_type = aura ? DAMAGE_NOESCAPE : DAMAGE_ATTACK;
    int          stat_drain_odds = aura ? 3 * HURT_CHANCE : HURT_CHANCE;

    if (who > 0)
    {
        m_ptr = &m_list[who];
        r_ptr = &r_info[m_ptr->r_idx];
        rlev = MAX(1, r_ptr->level*m_ptr->mpower/1000);

        monster_desc(m_name, m_ptr, 0);
        monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE);
        monster_desc(m_name_real, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
    }
    else
    {
        switch (who)
        {
        case PROJECT_WHO_UNCTRL_POWER:
            strcpy(m_name_real, "uncontrollable power storm");
            break;

        case PROJECT_WHO_GLASS_SHARDS:
            strcpy(m_name_real, "shards of glass");
            break;

        case PROJECT_WHO_MIRROR:
            strcpy(m_name_real, "mirror shards");
            break;

        case GF_WHO_TRAP:
        default:
            strcpy(m_name_real, "a trap");
            break;
        }
    }

    /* Analyze the damage */
    switch (type)
    {
    case GF_ACID:
        dam = res_calc_dam(RES_ACID, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:G>dissolved</color>!");
            else if (fuzzy) msg_print("You are hit by acid!");
            if (aura || !CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_ACID) && one_in_(stat_drain_odds))
                    do_dec_stat(A_CHR);
                if (minus_ac()) dam = (dam + 1) / 2;
            }
            result = take_hit(damage_type, dam, m_name_real);
            inven_damage(who, set_acid_destroy, 3, RES_ACID);
        }
        update_smart_learn(who, RES_ACID);
        break;
    case GF_FIRE:
        dam = res_calc_dam(RES_FIRE, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:r>burned</color>!");
            else if (fuzzy) msg_print("You are hit by fire!");
            if (aura || !CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_FIRE) && one_in_(stat_drain_odds))
                    do_dec_stat(A_STR);
            }

            result = take_hit(damage_type, dam, m_name_real);
            inven_damage(who, set_fire_destroy, 3, RES_FIRE);
        }
        update_smart_learn(who, RES_FIRE);
        break;
    case GF_COLD:
        dam = res_calc_dam(RES_COLD, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:W>frozen</color>!");
            else if (fuzzy) msg_print("You are hit by cold!");
            if (aura || !CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_COLD) && one_in_(stat_drain_odds))
                    do_dec_stat(A_STR);
            }
            result = take_hit(damage_type, dam, m_name_real);
            inven_damage(who, set_cold_destroy, 3, RES_COLD);
        }
        update_smart_learn(who, RES_COLD);
        break;
    case GF_ELEC:
        dam = res_calc_dam(RES_ELEC, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:b>shocked</color>!");
            else if (fuzzy) msg_print("You are hit by lightning!");
            if (aura || !CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_ELEC) && one_in_(stat_drain_odds))
                    do_dec_stat(A_DEX);
            }
            result = take_hit(damage_type, dam, m_name_real);
            inven_damage(who, set_elec_destroy, 3, RES_ELEC);
        }
        update_smart_learn(who, RES_ELEC);
        break;
    case GF_POIS:
        if (!aura && CHECK_MULTISHADOW())
        {
            msg_print("The attack hits your Shadow. You are unharmed!");
            break;
        }
        dam = res_calc_dam(RES_POIS, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:G>poisoned</color>!");
            else if (fuzzy) msg_print("You are hit by poison!");
            /* Moving damage from immediate to delayed can't simply leave the
             * value unchanged, else this is a monster nerf! We can scale everything
             * in r_info and BR_POIS, but that is tedious and I'm unsure what a good
             * scale factor is without some playtesting. cf GF_NUKE below. */
            dam = dam*7/4;
            set_poisoned(p_ptr->poisoned + dam, FALSE);
            if (!res_save_default(RES_POIS) && one_in_(stat_drain_odds))
                do_dec_stat(A_CON);
        }
        update_smart_learn(who, RES_POIS);
        break;
    case GF_NUKE:
        if (!aura && CHECK_MULTISHADOW())
        {
            msg_print("The attack hits your Shadow. You are unharmed!");
            break;
        }
        dam = res_calc_dam(RES_POIS, dam);
        if (dam)
        {
            if (touch) msg_print("You are <color:G>irradiated</color>!");
            else if (fuzzy) msg_print("You are hit by radiation!");
            dam = dam*7/4;
            set_poisoned(p_ptr->poisoned + dam, FALSE);
            if (!res_save_default(RES_POIS))
            {
                if (one_in_(5))
                {
                    msg_print("You undergo a freakish metamorphosis!");
                    if (one_in_(4))
                        do_poly_self();
                    else
                        mutate_player();
                }
                inven_damage(who, set_acid_destroy, 2, RES_POIS);
            }
        }
        update_smart_learn(who, RES_POIS);
        break;
    case GF_MISSILE:
    case GF_BLOOD:  /* Monsters can't do this ... */
        if (fuzzy) msg_print("You are hit by something!");
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_HOLY_FIRE:
        if (touch) msg_format("You are <color:y>%s</color>!", p_ptr->align < -10 ? "*immolated*" : "immolated");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = gf_holy_dam(dam);
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_HELL_FIRE:
        if (touch) msg_format("You are <color:D>%s</color>!", p_ptr->align > 10 ? "*bedeviled*" : "bedeviled");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = gf_hell_dam(dam);
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_ARROW:
        if (fuzzy) msg_print("You are hit by something sharp!");
        else if (equip_find_art(ART_ZANTETSU))
        {
            msg_print("You cut down the arrow!");
            break;
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_PLASMA:
        if (touch) msg_print("You are <color:R>burned</color>!");
        else if (fuzzy) msg_print("You are hit by something *HOT*!");

        /* Resist hack */
        if (prace_is_(RACE_MON_VORTEX) && p_ptr->current_r_idx == MON_PLASMA_VORTEX)
        {
            dam /= 3;
        }
        result = take_hit(damage_type, dam, m_name_real);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
        {
            int k = (randint1((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
            set_stun(p_ptr->stun + k, FALSE);
        }

        if (!touch) inven_damage(who, set_acid_destroy, 3, RES_FIRE);
        break;
    case GF_UNLIFE:
        if (!(get_race()->flags & RACE_IS_NONLIVING) && !life_save_p(rlev))
        {
            lp_player(-dam);
            if (touch && m_ptr)
            {
                m_ptr->mpower += dam;
                msg_format("<color:R>%^s grows more powerful!</color>", m_name_subject);
            }
        }
        break;
    case GF_NETHER: {
        int unlife;
        if (touch) msg_print("You are <color:D>drained</color>!");
        else if (fuzzy) msg_print("You are hit by nether forces!");
        dam = res_calc_dam(RES_NETHER, dam);
        unlife = dam*15/100;
        if (unlife)
        {
            gf_affect_p(who, GF_UNLIFE, unlife, flags);
            dam -= unlife;
        }
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_NETHER);
        break; }
    case GF_WATER:
    case GF_WATER2:
        if (fuzzy) msg_print("You are hit by something wet!");
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_SOUND))
                set_stun(p_ptr->stun + randint1(40), FALSE);
            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint1(5) + 5, FALSE);
            inven_damage(who, set_cold_destroy, 3, RES_SOUND);
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_CHAOS:
        if (touch) msg_print("You are <color:v>unmade</color>!");
        else if (fuzzy) msg_print("You are hit by a wave of anarchy!");
        dam = res_calc_dam(RES_CHAOS, dam);
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(20) + 10, FALSE);
            if (!res_save_default(RES_CHAOS))
            {
                int count = mut_count(mut_unlocked_pred);
                if (prace_is_(RACE_BEASTMAN)) count = 0;
                if (one_in_(3 + count*count))
                {
                    msg_print("Your body is twisted by chaos!");
                    mut_gain_random(NULL);
                }
                if (p_ptr->pclass == CLASS_WILD_TALENT && one_in_(7))
                    wild_talent_scramble();

                set_image(p_ptr->image + randint1(10), FALSE);
            }
            if (!res_save_default(RES_NETHER) && !res_save_default(RES_CHAOS))
                drain_exp(5000 + (p_ptr->exp / 100), 500 + (p_ptr->exp / 1000), 75);

            if (!touch)
            {
                inven_damage(who, set_elec_destroy, 2, RES_CHAOS);
                inven_damage(who, set_fire_destroy, 2, RES_CHAOS);
            }
        }
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_CHAOS);
        break;
    case GF_ROCK:
        if (fuzzy) msg_print("You are hit by something solid!");
        if (one_in_(2))
        {
            if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
                set_cut(p_ptr->cut + dam/2, FALSE);
            inven_damage(who, set_cold_destroy, 2, RES_SHARDS);
        }
        else
        {
            if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
            inven_damage(who, set_cold_destroy, 2, RES_SOUND);
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_SHARDS:
        if (touch) msg_print("You are <color:U>shredded</color>!");
        else if (fuzzy) msg_print("You are hit by something sharp!");
        dam = res_calc_dam(RES_SHARDS, dam);
        if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
            set_cut(p_ptr->cut + dam, FALSE);
        if (!touch) inven_damage(who, set_cold_destroy, 2, RES_SHARDS);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_SHARDS);
        break;
    case GF_SOUND:
        if (!touch && fuzzy) msg_print("You are hit by a loud noise!");
        /*if (touch) ... */
        dam = res_calc_dam(RES_SOUND, dam);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
        {
            int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
            set_stun(p_ptr->stun + k, FALSE);
        }
        if (!touch) inven_damage(who, set_cold_destroy, 2, RES_SOUND);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_SOUND);
        break;
    case GF_BOMB: /* combined sound and shards, but damage is weird */
    {
        int shard_dam, sound_dam;
        if (touch) msg_print("You are <color:s>blasted</color>!");
        else if (fuzzy) msg_print("There is an explosion! You are hit by shrapnel!"); /* If you want to complain about this terminology, write your own variant */
        _bomb_calc_dam(&dam, &shard_dam, &sound_dam, -1);
        if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
            set_cut(p_ptr->cut + shard_dam, FALSE);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
        {
            int k = (randint1((sound_dam > 90) ? 35 : (sound_dam / 3 + 5)));
            set_stun(p_ptr->stun + k, FALSE);
        }
        if (!touch)
        {
            inven_damage(who, set_cold_destroy, 2, RES_SHARDS);
            inven_damage(who, set_cold_destroy, 2, RES_SOUND);
        }
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_SHARDS);
        update_smart_learn(who, RES_SOUND);
        break;
    }
    case GF_CONFUSION:
        if (!touch && fuzzy) msg_print("You are hit by something puzzling!");
        /*if (touch) ... */
        dam = res_calc_dam(RES_CONF, dam);
        if (!res_save_default(RES_CONF) && !CHECK_MULTISHADOW())
            set_confused(p_ptr->confused + randint1(20) + 10, FALSE);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_CONF);
        break;
    case GF_DISENCHANT:
        if (touch) msg_print("You are <color:v>disenchanted</color>!");
        else if (fuzzy) msg_print("You are hit by something static!");
        dam = res_calc_dam(RES_DISEN, dam);
        if (!(flags & GF_AFFECT_SPELL) && !one_in_(5) && !CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_DISEN))
                disenchant_player();
        }
        else if (!res_save(RES_DISEN, 31) && !CHECK_MULTISHADOW())
            apply_disenchant(0);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_DISEN);
        break;
    case GF_NEXUS:
        if (touch) msg_print("You are <color:v>scrambled</color>!");
        else if (fuzzy) msg_print("You are hit by something strange!");
        dam = res_calc_dam(RES_NEXUS, dam);
        if (!res_save_default(RES_NEXUS) && !CHECK_MULTISHADOW())
            apply_nexus(m_ptr);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_NEXUS);
        break;
    case GF_FORCE:
        if (fuzzy) msg_print("You are hit by kinetic force!");
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            set_stun(p_ptr->stun + randint1(20), FALSE);
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_CHICKEN:
        if (fuzzy) msg_print("You are hit by a chicken!");
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            set_stun(p_ptr->stun + randint1(20), FALSE);
        if (!touch) inven_damage(who, set_cold_destroy, 2, RES_SOUND);
        result = take_hit(damage_type, dam, m_name_real);
        fear_scare_p(m_ptr);
        update_smart_learn(who, RES_FEAR);
        if (!touch) update_smart_learn(who, RES_SOUND);
        break;
    case GF_ROCKET:
        if (fuzzy) msg_print("There is an explosion!");
        dam = res_calc_dam(RES_SHARDS, dam);
        if (!res_save_default(RES_SOUND) && !CHECK_MULTISHADOW())
            set_stun(p_ptr->stun + randint1(20), FALSE);
        if (!res_save_default(RES_SHARDS) && !CHECK_MULTISHADOW())
            set_cut(p_ptr->cut + (dam / 2), FALSE);
        inven_damage(who, set_cold_destroy, 3, RES_SHARDS);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_SHARDS);
        break;
    case GF_INERT:
        if (touch) msg_print("You are <color:W>decelerated</color>!");
        else if (fuzzy) msg_print("You are hit by something slow!");
        /*if (touch) ... */
        if (!CHECK_MULTISHADOW())
        {
            if (!free_act_save_p(MAX(rlev, dam)))
                (void)p_inc_minislow(5);
            else (void)p_inc_minislow(1);
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_SLOW:
        if (touch) msg_print("You are <color:W>slowed</color>!");
        else if (fuzzy) msg_print("You are hit by something exhausting!");
        if (!CHECK_MULTISHADOW())
            (void)p_inc_minislow(1);
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_LITE:
        if (touch) msg_print("You are <color:y>dazzled</color>!");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = res_calc_dam(RES_LITE, dam);
        if (!p_ptr->blind && !res_save_default(RES_LITE) && !res_save_default(RES_BLIND) && !CHECK_MULTISHADOW())
            set_blind(p_ptr->blind + randint1(5) + 2, FALSE);

        result = take_hit(damage_type, dam, m_name_real);
        if (prace_is_(RACE_MON_VAMPIRE))
            vampire_take_light_damage(dam);

        if (IS_WRAITH() && !CHECK_MULTISHADOW() && !touch)
        {
            p_ptr->wraith_form = 0;
            wild_reset_counter(WILD_WRAITH);
            msg_print("The light forces you out of your incorporeal shadow form.");
            p_ptr->redraw |= PR_MAP;
            p_ptr->update |= (PU_MONSTERS);
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
            p_ptr->redraw |= (PR_STATUS);
        }
        update_smart_learn(who, RES_LITE);
        break;
    case GF_DARK:
        if (touch) msg_print("You are <color:D>benighted</color>!");
        else if (fuzzy) msg_print("You are hit by something!");
        dam = res_calc_dam(RES_DARK, dam);
        if (!p_ptr->blind && !res_save_default(RES_DARK) && !res_save_default(RES_BLIND) && !CHECK_MULTISHADOW())
            set_blind(p_ptr->blind + randint1(5) + 2, FALSE);
        result = take_hit(damage_type, dam, m_name_real);
        if (prace_is_(RACE_MON_VAMPIRE))
            vampire_take_dark_damage(dam);
        update_smart_learn(who, RES_DARK);
        break;
    case GF_ELDRITCH:
        if (touch && m_ptr)
            sanity_blast(m_ptr, FALSE);
        break;
    case GF_STUN:
        if (p_ptr->stun < STUN_KNOCKED_OUT)
            set_stun(p_ptr->stun + dam, FALSE);
        break;
    case GF_AMNESIA:
        if (_plr_save(who, 0))
        {
            if (!touch) msg_print("You resist the effects!");
        }
        else if (lose_all_info())
        {
            msg_print("Your memories fade away.");
        }
        break;
    case GF_TIME:
        if (touch) msg_print("You are <color:B>chronosmashed</color>!");
        else if (fuzzy) msg_print("You are hit by a blast from the past!");
        if (!res_save_default(RES_TIME) && !CHECK_MULTISHADOW())
        {
            int k;
            cptr act;
            switch (randint1(10))
            {
            case 1: case 2: case 3: case 4: case 5:
                if (p_ptr->prace == RACE_ANDROID) break;
                msg_print("You feel life has clocked back.");
                lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
                break;

            case 6: case 7: case 8: case 9:
                switch (randint1(6))
                {
                    case 1: k = A_STR; act = "strong"; break;
                    case 2: k = A_INT; act = "bright"; break;
                    case 3: k = A_WIS; act = "wise"; break;
                    case 4: k = A_DEX; act = "agile"; break;
                    case 5: k = A_CON; act = "hale"; break;
                    case 6: k = A_CHR; act = "confident"; break;
                }
                msg_format("You're not as %s as you used to be...", act);
                p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
                if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                p_ptr->update |= (PU_BONUS);
                break;
            case 10:
                msg_print("You're not as powerful as you used to be...");
                for (k = 0; k < 6; k++)
                {
                    p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 7) / 8;
                    if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                }
                p_ptr->update |= (PU_BONUS);
                break;
            }
        }
        dam = res_calc_dam(RES_TIME, dam);
        result = take_hit(damage_type, dam, m_name_real);
        update_smart_learn(who, RES_TIME);
        break;
    case GF_STORM:
        msg_print("You are hit by gale force winds!");
        if (!CHECK_MULTISHADOW())
        {
            teleport_player(5, TELEPORT_PASSIVE);
            if (!p_ptr->levitation)
                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
            if (!(res_save_default(RES_SOUND) || p_ptr->levitation))
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_GRAVITY:
        if (touch) msg_print("You are <color:W>warped</color>!");
        else if (fuzzy) msg_print("You are hit by something heavy!");
        /*if (touch) ... */
        msg_print("Gravity warps around you.");
        if (!CHECK_MULTISHADOW())
        {
            teleport_player(5, TELEPORT_PASSIVE);
            if (!p_ptr->levitation)
                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
            if (!(res_save_default(RES_SOUND) || p_ptr->levitation))
            {
                int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
                set_stun(p_ptr->stun + k, FALSE);
            }
        }
        if (p_ptr->levitation)
        {
            dam = (dam * 2) / 3;
        }
        inven_damage(who, set_cold_destroy, 2, RES_SOUND);
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_DISINTEGRATE:
        if (fuzzy) msg_print("You are hit by pure energy!");
        /* Resist hack */
        if (prace_is_(RACE_MON_VORTEX) && p_ptr->current_r_idx == MON_DISINTEGRATE_VORTEX)
        {
            dam /= 3;
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_OLD_HEAL:
        if (fuzzy) msg_print("You are hit by something invigorating!");

        hp_player(dam);
        break;
    case GF_OLD_SPEED:
        if (fuzzy) msg_print("You are hit by something!");

        set_fast(p_ptr->fast + randint1(5), FALSE);
        break;
    case GF_OLD_SLOW:
        if (fuzzy) msg_print("You are hit by something slow!");
        if (!free_act_save_p(MAX(rlev, dam)))
            set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
        break;
    case GF_OLD_SLEEP:
        if (!free_act_save_p(rlev))
        {
            if (fuzzy) msg_print("You fall asleep!");
            if (ironman_nightmare)
            {
                msg_print("A horrible vision enters your mind.");

                /* Pick a nightmare */
                get_mon_num_prep(get_nightmare, NULL);

                /* Have some nightmares */
                have_nightmare(get_mon_num(MAX_DEPTH));

                /* Remove the monster restriction */
                get_mon_num_prep(NULL, NULL);
            }
            set_paralyzed(dam, FALSE);
        }
        break;
    case GF_BLIND:
        if (res_save_default(RES_BLIND) || (!touch && _plr_save(who, 0)))
            msg_print("You resist the effects!");
        else
            set_blind(12 + randint0(4), FALSE);
        update_smart_learn(who, RES_BLIND);
        break;
    case GF_OLD_CONF:
        if (res_save_default(RES_CONF) || _plr_save(who, 0))
            msg_print("You disbelieve the feeble spell.");
        else
            set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
        update_smart_learn(who, RES_CONF);
        break;
    case GF_TURN_ALL:
        if (fuzzy) msg_print("Your will is shaken!");
        fear_scare_p(m_ptr);
        update_smart_learn(who, RES_FEAR);
        break;
    case GF_PARALYSIS:
        if (free_act_save_p(rlev) || (!touch && _plr_save(who, dam)))
            msg_print("You resist the effects!");
        else
            set_paralyzed(randint1(3), FALSE);
        update_smart_learn(who, SM_FREE_ACTION);
        break;
    case GF_MANA:
    case GF_SEEKER:
    case GF_SUPER_RAY:
        if (fuzzy) msg_print("You are hit by pure magic!");
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_PSY_SPEAR:
        if (fuzzy) msg_print("You are hit by an energy!");
        result = take_hit(DAMAGE_FORCE, dam, m_name_real);
        break;
    case GF_METEOR:
        if (fuzzy) msg_print("Something falls from the sky on you!");
        result = take_hit(damage_type, dam, m_name_real);
        inven_damage(who, set_fire_destroy, 2, RES_FIRE);
        inven_damage(who, set_cold_destroy, 2, RES_SHARDS);
        break;
    case GF_ICE:
        if (touch) msg_print("You are <color:W>frozen</color>!");
        else if (fuzzy) msg_print("You are hit by something sharp and cold!");
        result = gf_affect_p(who, GF_COLD, dam, 0);
        if (!CHECK_MULTISHADOW())
        {
            if (!res_save_default(RES_SHARDS))
                set_cut(p_ptr->cut + (touch ? damroll(3, 5) : damroll(5, 8)), FALSE);
            if (!res_save_default(RES_SOUND))
                set_stun(p_ptr->stun + (touch ? randint1(7) : randint1(15)), FALSE);
            inven_damage(who, set_cold_destroy, 3, RES_COLD);
        }
        update_smart_learn(who, RES_COLD);
        break;
    case GF_DEATH_RAY:
        if (fuzzy) msg_print("You are hit by something extremely cold!");
        if (!(get_race()->flags & RACE_IS_NONLIVING))
            result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_DRAIN_MANA:
        if (CHECK_MULTISHADOW())
        {
            if (!touch) msg_print("The attack hits Shadow, you are unharmed!");
        }
        else if (psion_mental_fortress())
        {
            if (!touch) msg_print("Your mental fortress is impenetrable!");
        }
        else if (_mana_loss_save(r_ptr))
        {
            if (!touch) msg_print("You keep your wits about you!");
        }
        else if ((p_ptr->csp) && (player_mana_drainable()))
        {
            if (who > 0) msg_format("%^s draws psychic energy from you!", m_name);
            else msg_print("Your psychic energy is drawn!");
            if (dam >= p_ptr->csp)
            {
                dam = p_ptr->csp;
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }
            else
                p_ptr->csp -= dam;

            p_ptr->redraw |= (PR_MANA);
            p_ptr->window |= (PW_SPELL);

            if (who > 0 && m_ptr->hp < m_ptr->maxhp)
            {
                m_ptr->hp += (6 * dam);
                if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
                check_mon_health_redraw(who);
                if (m_ptr->ml)
                    msg_format("%^s appears healthier.", m_name);
            }
        }
        break;
    case GF_MIND_BLAST:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else if (!CHECK_MULTISHADOW())
        {
            msg_print("Your mind is blasted by psionic energy.");

            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(4) + 4, FALSE);

            if (!res_save_default(RES_CHAOS) && one_in_(3))
                set_image(p_ptr->image + randint0(25) + 15, FALSE);

            if (player_mana_drainable())
            {
                p_ptr->csp -= touch ? 10 : 50;
                if (p_ptr->csp < 0)
                {
                    p_ptr->csp = 0;
                    p_ptr->csp_frac = 0;
                }
                p_ptr->redraw |= PR_MANA;
            }
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_BRAIN_SMASH:
        if (_plr_save(who, dam/3) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            if (_mana_loss_save(r_ptr))
            {
                if (!touch) msg_print("You keep your wits about you!");
            }
            else if ((!CHECK_MULTISHADOW()) && (player_mana_drainable()))
            {
                msg_print("Your mind is blasted by psionic energy.");
                if (touch)
                    p_ptr->csp -= 10;
                else
                    p_ptr->csp -= 100;
                if (p_ptr->csp < 0)
                {
                    p_ptr->csp = 0;
                    p_ptr->csp_frac = 0;
                }
                p_ptr->redraw |= PR_MANA;
            }
            result = take_hit(damage_type, dam, m_name_real);
            if (!CHECK_MULTISHADOW())
            {
                if (!res_save_default(RES_BLIND))
                    set_blind(p_ptr->blind + 8 + randint0(8), FALSE);
                if (!res_save_default(RES_CONF))
                    set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
                if (!free_act_save_p(rlev))
                    set_paralyzed(randint1(4), FALSE);

                set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
                set_stun(p_ptr->stun + pienempi(50, dam/6 + randint1(dam/6)), FALSE);

                while ((!_plr_save(who, 0)) && (p_ptr->stat_cur[A_INT] > 3))
                {
                    (void)do_dec_stat(A_INT);
                    if (p_ptr->sustain_int) break;
                }
                while ((!_plr_save(who, 0)) && (p_ptr->stat_cur[A_WIS] > 3))
                {
                    (void)do_dec_stat(A_WIS);
                    if (p_ptr->sustain_wis) break;
                }

                if (!res_save_default(RES_CHAOS))
                    set_image(p_ptr->image + randint0(25) + 15, FALSE);
            }
        }
        break;
    case GF_TELEKINESIS:
        if (!CHECK_MULTISHADOW())
        {
            if (one_in_(4))
                teleport_player(5, TELEPORT_PASSIVE);
            if (!_plr_save(who, dam/5))
                set_stun(p_ptr->stun + pienempi(25, dam/6 + randint1(dam/6)), FALSE);
        }
        result = take_hit(damage_type, dam, m_name_real);
        break;
    case GF_CAUSE_1:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(15, 0);
            result = take_hit(damage_type, dam, m_name_real);
        }
        break;
    case GF_CAUSE_2:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(25, MIN(rlev / 2 - 15, 5) );
            result = take_hit(damage_type, dam, m_name_real);
        }
        break;
    case GF_CAUSE_3:
        if (_plr_save(who, dam/5) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW()) curse_equipment(33, MIN(rlev / 2 - 15, 15));
            result = take_hit(damage_type, dam, m_name_real);
        }
        break;
    case GF_CAUSE_4:
        if (_plr_save(who, dam/5) && m_ptr->r_idx != MON_KENSHIROU && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            result = take_hit(damage_type, dam, m_name_real);
            if (!CHECK_MULTISHADOW()) set_cut(p_ptr->cut + damroll(10, 10), FALSE);
        }
        break;
    case GF_HAND_DOOM:
        if (_plr_save(who, 0) && !CHECK_MULTISHADOW())
        {
            if (!touch)
            {
                msg_print("You resist the effects!");
            }
        }
        else
        {
            if (!CHECK_MULTISHADOW())
            {
                msg_print("You feel your life fade away!");
                curse_equipment(40, 20);
            }

            result = take_hit(damage_type, dam, m_name_real);

            if (p_ptr->chp < 1) p_ptr->chp = 1; /* Paranoia */
        }
        break;
    case GF_OLD_POLY:
        if ( player_obviously_poly_immune()
          || mut_present(MUT_DRACONIAN_METAMORPHOSIS) )
        {
            if (flags & GF_AFFECT_SPELL)
                msg_print("You are unaffected!");
        }
        else if (_plr_save(who, 0))
        {
            if (flags & GF_AFFECT_SPELL)
                msg_print("You resist the effects!");
        }
        else
        {
            int which;
            switch(randint1(5))
            {
            case 1:
                if (p_ptr->prace != RACE_SNOTLING)
                {
                    which = RACE_SNOTLING;
                    break;
                }
            case 2:
                if (p_ptr->prace != RACE_YEEK)
                {
                    which = RACE_YEEK;
                    break;
                }
            case 3:
                which = MIMIC_SMALL_KOBOLD;
                break;
            case 4:
                which = MIMIC_MANGY_LEPER;
                break;
            default:
                for (;;)
                {
                    which = randint0(MAX_RACES);
                    if ( which != RACE_HUMAN
                      && which != RACE_DEMIGOD
                      && which != RACE_DRACONIAN
                      && which != RACE_ANDROID
                      && which != RACE_WEREWOLF
                      && p_ptr->prace != which
                      && !(get_race_aux(which, 0)->flags & RACE_IS_MONSTER)
                      && !(get_race_aux(which, 0)->flags & RACE_NO_POLY) )
                    {
                        break;
                    }
                }
            }
            set_mimic(50 + randint1(50), which, FALSE);
        }
        break;
    case GF_ATTACK:
        make_attack_normal(who);
        break;
    }
    if ((p_ptr->action == ACTION_LEARN) && (who > 0) && (flags & GF_AFFECT_SPELL) && (type != GF_ATTACK))
        blue_mage_learn_spell();
    return result;
}
int gf_distance_hack = 1;
static int _gf_distance_mod(int n)
{
    if (!gf_distance_hack) return n;
    return (n + gf_distance_hack) / (gf_distance_hack + 1);
}
static bool _is_mental_attack(int type)
{
    switch (type)
    {
    case GF_PSI:
    case GF_PSI_BRAIN_SMASH:
    case GF_BRAIN_SMASH:
    case GF_PSI_STORM:
    case GF_ELDRITCH_STUN:
    case GF_DOMINATION:
    case GF_SUBJUGATION:
    case GF_STUN: /* XXX */
        return TRUE;
    }
    return FALSE;
}

int charm_pow_modify(int dam, monster_type *mon)
{
    monster_race *race = &r_info[mon->r_idx];
    dam += (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1);
    if (p_ptr->pclass != CLASS_POLITICIAN)
    {
        dam += virtue_current(VIRTUE_HARMONY)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;
    }
    else
    {
        switch (politician_get_toggle())
        {
            case POLLY_TOGGLE_AUCAST:
            if (race->flags3 & RF3_EVIL) dam += (dam / 10);
                break;
            case POLLY_TOGGLE_XPCAST:
            if (!(race->flags3 & RF3_EVIL) && !(race->flags3 & RF3_GOOD)) dam += (dam / 10);
                break;
            default:
            if (race->flags3 & RF3_GOOD) dam += (dam / 10);
                break;
        }
    }
    if (p_ptr->spin > 0) dam += MAX(25, dam * 2 / 5);
    if (p_ptr->uimapuku) dam = dam * 3 / 2;
    if ((race->flags1 & RF1_UNIQUE) || (race->flags7 & RF7_NAZGUL) ||
        (mon->mflag2 & MFLAG2_QUESTOR))
    {
        if (p_ptr->uimapuku) dam = dam * 18 / 25; /* Fine-tuned to give a maxed-out mode-bonus polly a 1 in 90 chance of charming a level 100 unique */
        else dam = dam * 2 / 3;
    }
    return dam;
}

#define _BABBLE_HACK() \
            if (race->flagsr & RFR_RES_ALL) \
            { \
                note = " is immune."; \
                dam = 0; \
                mon_lore_r(mon, RFR_RES_ALL); \
                break; \
            }

bool gf_affect_m(int who, mon_ptr mon, int type, int dam, int flags)
{
    int tmp;

    point_t       where = point(mon->fx, mon->fy);
    monster_type *caster_ptr = (who > 0) ? &m_list[who] : NULL;
    bool          touch = BOOL(flags & (GF_AFFECT_AURA | GF_AFFECT_ATTACK));
    bool          no_harm = ((melee_challenge) && ((!touch) || (who)) && (!is_pet(mon)));

    monster_race *race = mon_race(mon);

    char killer[80];

    /* Is the monster "seen"? Note: mon_show_msg() requires
     * the monster square to be lit if ignore_unview is on. This
     * means that if the player is lobbing fireballs at a telepathically
     * seen monster, they would miss the resistance message. So, if
     * the player is doing this, then we just use ml instead. If the
     * player is not doing this, then we better respect ignore_unview
     * or we'll turn the message spammer back on full blast! */
    bool seen = mon->ml;
    bool seen_msg = (who == GF_WHO_PLAYER) ? mon->ml : mon_show_msg(mon);

    bool slept = BOOL(MON_CSLEEP(mon));

    /* Were the effects "obvious" (if seen)? */
    bool obvious = FALSE;

    /* Can the player know about this effect? */
    bool known = ((mon->cdis <= MAX_SIGHT) || p_ptr->inside_battle);

    /* Were the effects "irrelevant"? */
    bool skipped = FALSE;

    /* Gets the monster angry at the source of the effect? */
    bool get_angry = FALSE;

    /* Polymorph setting (true or false) */
    bool do_poly = FALSE;

    /* Teleport setting (max distance) */
    int do_dist = 0;

    /* Confusion setting (amount to confuse) */
    int do_conf = 0;

    /* Stunning setting (amount to stun) */
    int do_stun = 0;

    /* Sleep amount (amount to sleep) */
    int do_sleep = 0;
    int do_paralyzed = 0;

    /* Fear amount (amount to fear) */
    int do_fear = 0;

    /* Time amount (amount to time) */
    int do_time = 0;

    bool heal_leper = FALSE;

    /* Hold the monster name */
    char m_name[MAX_NLEN];
    char m_name_object[MAX_NLEN];

    char m_poss[80];

    int photo = 0;

    /* Assume no note */
    cptr note = NULL;

    /* Assume a default death */
    cptr note_dies = extract_note_dies(real_r_ptr(mon));

    int ty = mon->fy;
    int tx = mon->fx;

    int caster_lev = (who > 0) ? r_info[caster_ptr->r_idx].level : spell_power(p_ptr->lev * 2);

    bool who_is_pet = FALSE;
    if (who > 0 && is_pet(&m_list[who]))
        who_is_pet = TRUE;

    /* Never affect projector */
    if (mon->id == who) return FALSE;

    if (mon->id == p_ptr->riding && who == PROJECT_WHO_PLAYER)
    {
        switch (type)
        {
        case GF_OLD_HEAL:
        case GF_OLD_SPEED:
        case GF_STAR_HEAL:
        case GF_CRUSADE:
        case GF_UNHOLY_WORD:
            break;
        default:
            return FALSE;
        }
    }

    if (sukekaku && ((mon->r_idx == MON_SUKE) || (mon->r_idx == MON_KAKU))) return FALSE;

    /* Don't affect already dead monsters */
    /* Prevents problems with chain reactions of exploding monsters */
    if (mon->hp < 0) return (FALSE);

    /* Big hack here ... Jump Spiders are immune to the jumps of their kin
     * (They come in packs, and it would be silly for them to destroy each other) */
    if (mon_spell_current() && race->spells)
    {
        mon_spell_ptr spell = mon_spell_current();
        mon_spell_group_ptr group = race->spells->groups[MST_TACTIC];
        if (spell->id.type == MST_TACTIC && group && mon_spell_group_find(group, spell->id))
        {
            set_monster_csleep(mon->id, 0);
            return FALSE;
        }
    }

    /* Get the monster name (BEFORE polymorphing) */
    if (flags & GF_AFFECT_SPELL)
    {
        monster_desc(m_name, mon, 0);
        monster_desc(m_name_object, mon, 0);
    }
    else
    {
        monster_desc(m_name, mon, MD_PRON_VISIBLE);
        monster_desc(m_name_object, mon, MD_PRON_VISIBLE | MD_OBJECTIVE);
    }
    /* Get the monster possessive ("his"/"her"/"its") */
    monster_desc(m_poss, mon, MD_PRON_VISIBLE | MD_POSSESSIVE);


    if (p_ptr->riding && mon->id == p_ptr->riding) disturb(1, 0);

    /* Analyze the damage type */
    switch (type)
    {
    case GF_MISSILE:
    case GF_BLOOD:
    case GF_ELDRITCH:
    case GF_ELDRITCH_DRAIN:  /* Lazy ... I'll give back hp later */
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_MANA_CLASH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((!race->spells) || (!race->spells->freq))
        {
            note = " is immune.";
            dam = 0;
        }
        else /* 900 max dam coming in ... ~600 max dam going out */
            dam = dam * MIN(66, race->spells->freq) / 100;
        break;
    case GF_ACID:
        if (touch && seen_msg) msg_format("%^s is <color:G>dissolved</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_ACID)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_ACID);
        }
        else if (race->flagsr & RFR_RES_ACID)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(mon, RFR_RES_ACID);
        }
        break;
    case GF_ELEC:
        if (touch && seen_msg) msg_format("%^s is <color:b>shocked</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_ELEC)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_ELEC);
        }
        else if (race->flagsr & RFR_RES_ELEC)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(mon, RFR_RES_ELEC);
        }
        break;
    case GF_FIRE:
        if (touch && seen_msg) msg_format("%^s is <color:r>burned</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_FIRE)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_FIRE);
        }
        else if (race->flagsr & RFR_RES_FIRE)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(mon, RFR_RES_FIRE);
        }
        else if (race->flags3 & RF3_HURT_FIRE)
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(mon, RF3_HURT_FIRE);
        }
        break;
    case GF_COLD:
        if (touch && seen_msg) msg_format("%^s is <color:W>frozen</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_COLD)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_COLD);
        }
        else if (race->flagsr & RFR_RES_COLD)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(mon, RFR_RES_COLD);
        }
        else if (race->flags3 & RF3_HURT_COLD)
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(mon, RF3_HURT_COLD);
        }
        break;
    case GF_POIS:
        if (touch && seen_msg && !(race->flagsr & RFR_IM_POIS))
            msg_format("%^s is <color:G>poisoned</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_POIS)
        {
            if (!touch)
                note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_POIS);
        }
        else if (race->flagsr & RFR_RES_POIS)
        {
            note = " resists.";
            dam /= 3;
            mon_lore_r(mon, RFR_RES_POIS);
        }
        break;
    case GF_NUKE:
        if (touch && seen_msg) msg_format("%^s is <color:G>irradiated</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_IM_POIS)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_POIS);
        }
        else if (race->flagsr & RFR_RES_POIS)
        {
            note = " resists.";
            dam /= 2;
            mon_lore_r(mon, RFR_RES_POIS);
        }
        else if (one_in_(3)) do_poly = TRUE;
        break;
    case GF_HELL_FIRE:
        if (touch && seen_msg) msg_format("%^s is <color:D>%s</color>!", m_name, (race->flags3 & RF3_GOOD) ? "*burned*" : "burned");
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_GOOD)
        {
            dam *= 2;
            if (!touch) note = " is hit hard.";
            mon_lore_3(mon, RF3_GOOD);
        }
        break;
    case GF_HOLY_FIRE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_GOOD)
        {
            dam = 0;
            if (!touch) note = " is immune.";
            mon_lore_3(mon, RF3_GOOD);
        }
        else if (race->flags3 & RF3_EVIL)
        {
            if (touch && seen_msg) msg_format("%^s is <color:D>*burned*</color>!", m_name);
            dam *= 2;
            if (!touch) note = " is hit hard.";
            mon_lore_3(mon, RF3_EVIL);
        }
        else
        {
            if (touch && seen_msg) msg_format("%^s is <color:D>burned</color>!", m_name);
            dam *= 3; dam /= randint1(6) + 6;
        }
        break;
    case GF_ARROW:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_PLASMA:
        if (touch && seen_msg) msg_format("%^s is <color:R>burned</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_PLAS)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_PLAS);
        }
        else if (who == GF_WHO_PLAYER && mon_stun_save(race->level, dam))
        {
            note = " resists stunning.";
        }
        else do_stun = mon_stun_amount(dam);
        break;
    case GF_UNLIFE:
        if (monster_living(race) /* && some sort of save */)
        {
            mon->mpower = MAX(MIN(100, mon->mpower), mon->mpower - dam);
            if (seen_msg) msg_format("%^s grows less powerful.", m_name);
            if (!(flags & GF_AFFECT_SPELL) && who == GF_WHO_PLAYER)
                lp_player(dam);
            return TRUE; /* using note causes messages out of order */
        }
        return FALSE;
    case GF_NETHER:
        if (touch && seen_msg) msg_format("%^s is <color:D>drained</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((race->flagsr & RFR_RES_NETH) || (race->flags3 & RF3_UNDEAD))
        {
            if (race->flags3 & RF3_UNDEAD)
            {
                note = " is immune.";
                dam = 0;
                mon_lore_3(mon, RF3_UNDEAD);
            }
            else
            {
                note = " resists.";
                dam *= 3; dam /= randint1(6) + 6;
            }
            mon_lore_r(mon, RFR_RES_NETH);
        }
        break;
    case GF_WATER:
    case GF_WATER2:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_WATE)
        {
            if ((mon->r_idx == MON_WATER_ELEM) || (mon->r_idx == MON_UNMAKER))
            {
                note = " is immune.";
                dam = 0;
            }
            else
            {
                note = " resists.";
                dam *= 3; dam /= randint1(6) + 6;
            }
            mon_lore_r(mon, RFR_RES_WATE);
        }
        else if (who == GF_WHO_PLAYER && mon_stun_save(race->level, dam))
        {
            note = " resists stunning.";
        }
        else do_stun = mon_stun_amount(dam);
        break;
    case GF_CHAOS:
        if (touch && seen_msg) msg_format("%^s is <color:v>unmade</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_CHAO)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_CHAO);
        }
        else if ((race->flags3 & RF3_DEMON) && one_in_(3))
        {
            note = " resists somewhat.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_3(mon, RF3_DEMON);
        }
        else
        {
            do_poly = TRUE;
            /* Try to make the Chaos Vortex more playable. With too frequent
             * polymorphing, you always seem to get stuck on a chaos resistant
             * foe eventually. */
            if (p_ptr->current_r_idx == MON_CHAOS_VORTEX && !one_in_(5))
                do_poly = FALSE;
            do_conf = _gf_distance_mod(5 + randint1(11));
        }
        break;
    case GF_SHARDS:
        if (touch && seen_msg) msg_format("%^s is <color:U>shredded</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_SHAR)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_SHAR);
        }
        break;
    case GF_BOMB:
    {
        int sound_dam = 0, shard_dam = 0;
        if (touch && seen_msg) msg_format("%^s is <color:s>blasted</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        _bomb_calc_dam(&dam, &sound_dam, &shard_dam, mon->id);
        if (race->flagsr & RFR_RES_SHAR)
        {
            note = " resists somewhat.";
            mon_lore_r(mon, RFR_RES_SHAR);
        }
        if (race->flagsr & RFR_RES_SOUN)
        {
            note = (race->flagsr & RFR_RES_SHAR) ? " resists." : " resists somewhat.";
            mon_lore_r(mon, RFR_RES_SOUN);
        }
        else if (who == GF_WHO_PLAYER && mon_stun_save(race->level, sound_dam))
            note = " resists stunning.";
        else if (sound_dam > 0)
            do_stun = mon_stun_amount(sound_dam);
        break;
    }
    case GF_ROCKET:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_SHAR)
        {
            note = " resists somewhat.";
            dam /= 2;
            mon_lore_r(mon, RFR_RES_SHAR);
        }
        break;
    case GF_ELDRITCH_STUN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_NO_STUN)
            mon_lore_3(mon, RF3_NO_STUN);
        else if (mon_save_p(mon->r_idx, A_CHR)
              || ((race->flags1 & RF1_UNIQUE) && mon_save_p(mon->r_idx, A_CHR)) )
        {
            note = " resists stunning.";
        }
        else
            do_stun = mon_stun_amount(dam);
        break;
    case GF_ROCK:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!(race->flagsr & RFR_RES_SOUN))
        {
            if (who == GF_WHO_PLAYER  && mon_stun_save(race->level, dam))
                note = " resists stunning.";
            else
                do_stun = mon_stun_amount(dam);
        }
        break;
    case GF_SOUND:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_SOUN)
        {
            note = " resists.";
            dam *= 2; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_SOUN);
        }
        else if (who == GF_WHO_PLAYER && mon_stun_save(race->level, dam))
            note = " resists stunning.";
        else
            do_stun = mon_stun_amount(dam);
        break;
    case GF_ELDRITCH_DISPEL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        dispel_monster_status(mon->id);

        /* More Powerful vs. Evil */
        if ((race->flags3 & RF3_EVIL) && one_in_(5))
        {
            msg_print("You blast the forces of evil with great power!");
            dam = dam * 2;

            /* Attempt a Powerful Slow */
            if (race->level > randint1(p_ptr->lev * 2))
            {
                note = " resists being slowed!";
            }
            else
            {
                if (set_monster_slow(mon->id, MON_SLOW(mon) + 50))
                    note = " starts moving slower.";
            }
        }
        break;
    case GF_PHARAOHS_CURSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        dam = spell_power(MIN(mon->hp*8/100 + dam, 400));
        if (p_ptr->lev >= 50)
            dam += 50;
        note = " is cursed by ancient pharaohs of long ago!";
        break;
    case GF_ELDRITCH_CONFUSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* 593 monsters have NO_CONF ... it is more widely resisted than fire ...
           Eldritch confusion will bypass this check, but that is hacked below */
        if (MON_CONFUSED(mon))
            note = " is already confused.";
        else if ((race->flags3 & RF3_NO_CONF) && race->level + randint1(100) > p_ptr->lev*2 + (p_ptr->stat_ind[A_CHR] + 3) - 10)
            note = " resists confusion.";
        else
        {
            /* Recovery is randint1(r_info[mon->r_idx].level / 20 + 1) */
            do_conf = 3 + randint0(5);
        }
        break;
    case GF_CONFUSION:
        if (touch && seen_msg) msg_format("%^s is <color:U>baffled</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_NO_CONF)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_3(mon, RF3_NO_CONF);
        }
        else if (randint1(race->level) >= randint1(caster_lev))
        {
            do_conf = 0;
            dam -= (dam / 10);
            note = " resists the effects!";
        }

        else do_conf = _gf_distance_mod(10 + randint1(15));
        break;
    case GF_DISENCHANT:
        if (touch && seen_msg) msg_format("%^s is <color:v>disenchanted</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_DISE)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_DISE);
        }
        break;
    case GF_NEXUS:
        if (touch && seen_msg) msg_format("%^s is <color:v>scrambled</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_NEXU)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_NEXU);
        }
        break;
    case GF_FORCE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_WALL)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_WALL);
        }
        else
            do_stun = mon_stun_amount(dam);
        break;
    case GF_INERT:
        if (touch && seen_msg) msg_format("%^s is <color:W>decelerated</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_INER)
        {
            note = " resists.";

            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_INER);
        }
        else
        {
            /* Powerful monsters can resist */
            if ((race->flags1 & (RF1_UNIQUE)) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                obvious = FALSE;
            }
            /* Normal monsters slow down */
            else if (m_inc_minislow(mon, 5))
                note = " starts moving slower.";
        }
        break;
    case GF_SLOW:
        if (touch && seen_msg) msg_format("%^s is <color:W>slowed</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_INER)
        {
            note = " resists.";

            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_INER);
        }
        else if ((race->flags1 & (RF1_UNIQUE)) || (race->flags7 & (RF7_NAZGUL)) ||
                 (randint1(race->level) > randint1(MAX(62, dam))))
        {
            obvious = FALSE;
        }
        else if (m_inc_minislow(mon, 1))
            note = " starts moving slower.";
        break;
    case GF_TIME:
        if (touch && seen_msg) msg_format("%^s is <color:B>chronosmashed</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_TIME)
        {
            note = " resists.";
            dam *= 3; dam /= randint1(6) + 6;
            mon_lore_r(mon, RFR_RES_TIME);
        }
        else if (!who)
        {
            int ml = race->level;
            int pl = p_ptr->lev;
            bool unique = FALSE;

            if (race->flags1 & RF1_UNIQUE)
                unique = TRUE;

            if (unique)
                ml += 10;

            if (p_ptr->lev > 40 && p_ptr->current_r_idx != MON_HOUND_OF_TINDALOS)
                pl += (p_ptr->lev - 40) * 2;

            if (randint1(pl) <= randint1(ml))
            {
                #ifdef _DEBUG
                note = " resists the ravages of time.";
                #endif
            }
            else
            {
                int which = randint1(100);
                if (which <= 15)
                {
                    if (unique)
                    {
                        #ifdef _DEBUG
                        note = " cannot be slowed.";
                        #endif
                    }
                    else if(set_monster_slow(mon->id, MON_SLOW(mon) + 50))
                    {
                        #ifdef _DEBUG
                        note = " starts moving slower.";
                        #endif
                    }
                    else
                    {
                        #ifdef _DEBUG
                        note = " is already slow.";
                        #endif
                    }
                }
                else if (which <= 20)
                {
                    if (devolve_monster(mon->id, p_ptr->wizard))
                        return TRUE;
                }
                else if (which <= 25)
                {
                    if (evolve_monster(mon->id, p_ptr->wizard))
                        return TRUE;
                }
                else if (which <= 40)
                {
                    if (!unique)
                    {
                        note = " is suspended!";
                        do_paralyzed = 2;
                    }
                    else
                    {
                        #ifdef _DEBUG
                        note = " cannot be suspended.";
                        #endif
                    }
                }
                else if (which <= 50)
                {
                    mon->ac_adj -= randint1(10);
                    note = " is more exposed!";
                }
                else if (which <= 60)
                {
                    do_time = (dam + 1) / 2;
                }
                else if (which <= 70)
                {
                    mon->mspeed -= randint1(2);
                    note = " is permanently slowed!";
                }
                else if (which <= 90)
                {
                    mon->mpower = isompi(300, mon->mpower * (850 + randint0(100)) / 1000);
                    note = " shrinks!";
                }
                else
                {
                    note = " is suddenly sluggish.";
                    mon->energy_need += ENERGY_NEED();
                }
            }
        }
        else
        {
            do_time = (dam + 1) / 2;
        }
        break;
    case GF_STORM: /* TODO */
    case GF_GRAVITY: {
        bool resist_tele = FALSE;

        if (touch && seen_msg && type == GF_GRAVITY) msg_format("%^s is <color:W>warped</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_TELE)
        {
            if (race->flags1 & (RF1_UNIQUE))
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " is unaffected!";
                resist_tele = TRUE;
            }
            else if (race->level > randint1(100))
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " resists!";
                resist_tele = TRUE;
            }
        }

        if (!resist_tele) do_dist = 10;
        else do_dist = 0;
        if (p_ptr->riding && (mon->id == p_ptr->riding)) do_dist = 0;

        if (type == GF_GRAVITY && (race->flagsr & RFR_RES_GRAV))
        {
            note = " resists.";

            dam *= 3; dam /= randint1(6) + 6;
            do_dist = 0;
            mon_lore_r(mon, RFR_RES_GRAV);
        }
        else
        {
            /* 1. slowness */
            /* Powerful monsters can resist */
            if ((race->flags1 & (RF1_UNIQUE)) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                obvious = FALSE;
            }
            /* Normal monsters slow down */
            else
            {
                if (set_monster_slow(mon->id, MON_SLOW(mon) + 50))
                {
                    note = " starts moving slower.";
                }
            }

            /* 2. stun */
            do_stun = mon_stun_amount(dam);

            /* Attempt a saving throw */
            if ((race->flags1 & (RF1_UNIQUE)) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Resist */
                do_stun = 0;
                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
        }
        break; }
    case GF_MANA:
    case GF_SEEKER:
    case GF_SUPER_RAY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_DISINTEGRATE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_HURT_ROCK)
        {
            mon_lore_3(mon, RF3_HURT_ROCK);
            note = " loses some skin!";
            note_dies = " evaporates!";
            dam *= 2;
        }
        break;
    case GF_PSI_STORM:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";
            mon_lore_2(mon, RF2_EMPTY_MIND);
            break;

        }
        else if (race->flags2 & (RF2_STUPID | RF2_WEIRD_MIND))
        {
            dam /= 3;
            note = " resists.";
            break;
        }
        if (one_in_(4))
        {
            if (p_ptr->riding && (mon->id == p_ptr->riding)) do_dist = 0;
            else do_dist = 7;
        }
        if (one_in_(2))
        {
            int mult = 1;

            do_stun = mon_stun_amount(dam * 2 / 3);
            if (race->flags1 & RF1_UNIQUE)
                mult++;

            if (mult*race->level > 5 + randint1(dam))
            {
                do_stun = 0;
                obvious = FALSE;
            }
        }
        if (one_in_(4))
        {
            switch (randint1(3))
            {
                case 1:
                    do_conf = (race->flags3 & RF3_NO_CONF) ? 2 : 3 + randint1((dam + 9) / 10);
                    break;
                case 2:
                    if ((race->flags3 & RF3_NO_FEAR) && (one_in_(2))) break;
                    do_fear = 3 + randint1(dam);
                    break;
                case 3:
                    if ((race->flags3 & RF3_NO_SLEEP) && (one_in_(2))) break;
                    note = " falls asleep!";
                    do_sleep = 3 + randint1(dam);
                    break;
            }
        }
        note_dies = " collapses, a mindless husk.";
        break;
    case GF_PSI:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* PSI only works if the monster can see you! -- RG */
        if (!(los(mon->fy, mon->fx, py, px)))
        {
            if (seen_msg) msg_format("%^s can't see you, and isn't affected!", m_name);
            skipped = TRUE;
            break;
        }
        if (race->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";
            mon_lore_2(mon, RF2_EMPTY_MIND);

        }
        else if ((race->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
                 (race->flags3 & RF3_ANIMAL) ||
                 (race->level > randint1(3 * dam)))
        {
            dam /= 3;
            note = " resists.";


            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((race->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                (race->level > p_ptr->lev / 2) &&
                one_in_(2))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if ((randint0(100 + race->level / 2) < p_ptr->skills.sav) && !CHECK_MULTISHADOW())
                {
                    msg_print("You resist the effects!");

                }
                else
                {
                    /* Injure +/- confusion */
                    monster_desc(killer, mon, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                    take_hit(DAMAGE_ATTACK, dam, killer);  /* has already been /3 */
                    if (one_in_(4) && !CHECK_MULTISHADOW())
                    {
                        switch (randint1(4))
                        {
                        case 1:
                            set_confused(p_ptr->confused + 3 + randint1(dam), FALSE);
                            break;
                        case 2:
                            set_stun(p_ptr->stun + randint1(dam), FALSE);
                            break;
                        case 3:
                            if (race->flags3 & RF3_NO_FEAR)
                                note = " is unaffected.";
                            else
                                fear_add_p(FEAR_SCARED);
                            break;
                        default:
                            if (!free_act_save_p(race->level))
                                set_paralyzed(randint1(dam), FALSE);
                        }
                    }
                }
                dam = 0;
            }
        }

        if ((dam > 0) && one_in_(4))
        {
            switch (randint1(4))
            {
                case 1:
                    do_conf = 3 + randint1(dam);
                    break;
                case 2:
                    do_stun = 3 + randint1(dam);
                    break;
                case 3:
                    do_fear = 3 + randint1(dam);
                    break;
                default:
                    note = " falls asleep!";

                    do_sleep = 3 + randint1(dam);
                    break;
            }
        }

        note_dies = " collapses, a mindless husk.";
        break;
    case GF_PSI_DRAIN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags2 & RF2_EMPTY_MIND)
        {
            dam = 0;
            note = " is immune!";

        }
        else if ((race->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
                 (race->flags3 & RF3_ANIMAL) ||
                 (race->level > randint1(3 * dam)))
        {
            dam /= 3;
            note = " resists.";


            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((race->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                 (race->level > p_ptr->lev / 2) &&
                 (one_in_(2)))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if ((randint0(100 + race->level / 2) < p_ptr->skills.sav) && !CHECK_MULTISHADOW())
                {
                    msg_print("You resist the effects!");
                }
                else
                {
                    /* Injure + mana drain */
                    monster_desc(killer, mon, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
                    if ((!CHECK_MULTISHADOW()) && (player_mana_drainable()))
                    {
                        msg_print("Your psychic energy is drained!");

                        p_ptr->csp -= damroll(5, dam) / 2;
                        if (p_ptr->csp < 0) p_ptr->csp = 0;
                        p_ptr->redraw |= PR_MANA;
                        p_ptr->window |= (PW_SPELL);
                    }
                    take_hit(DAMAGE_ATTACK, dam, killer);  /* has already been /3 */
                }
                dam = 0;
            }
        }
        else if (dam > 0)
        {
            int b = damroll(5, dam) / 4;
            cptr str = (p_ptr->pclass == CLASS_MINDCRAFTER) ? "psychic energy" : "mana";
            msg_format("You convert %s%s pain into %s!",
                m_name, (seen ? "'s" : "s"), str);

            b = MIN(p_ptr->msp, p_ptr->csp + b);
            p_ptr->csp = b;
            p_ptr->redraw |= PR_MANA;
            p_ptr->window |= (PW_SPELL);
        }

        note_dies = " collapses, a mindless husk.";
        break;
    case GF_TELEKINESIS: {
        int mult = 1;
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (one_in_(4))
        {
            if (p_ptr->riding && (mon->id == p_ptr->riding)) do_dist = 0;
            else do_dist = 7;
        }

        do_stun = mon_stun_amount(dam);
        if (race->flags1 & RF1_UNIQUE)
            mult++;

        if (mult*race->level > 5 + randint1(dam))
        {
            do_stun = 0;
            obvious = FALSE;
        }
        break; }
    case GF_PSY_SPEAR:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_METEOR:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_DOMINATION: {
        int power = dam;

        /* No "real" damage */
        dam = 0;

        if (!is_hostile(mon)) break;
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        /* Attempt a saving throw */
        if (randint1(race->level) > randint1(power))
        {
            /*
             * Powerful demons & undead can turn a mindcrafter's
             * attacks back on them
             */
            if ((race->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
                (race->level > p_ptr->lev / 2) &&
                (one_in_(2)))
            {
                note = NULL;
                msg_format("%^s%s corrupted mind backlashes your attack!",
                    m_name, (seen ? "'s" : "s"));

                /* Saving throw */
                if (randint0(100 + race->level/2) < p_ptr->skills.sav)
                {
                    msg_print("You resist the effects!");

                }
                else
                {
                    /* Confuse, stun, terrify */
                    switch (randint1(4))
                    {
                        case 1:
                            set_stun(p_ptr->stun + power / 2, FALSE);
                            break;
                        case 2:
                            set_confused(p_ptr->confused + power / 2, FALSE);
                            break;
                        default:
                        {
                            if (race->flags3 & RF3_NO_FEAR)
                                note = " is unaffected.";

                            else
                                fear_add_p(power);
                        }
                    }
                }
            }
            else
            {
                note = " is unaffected!";
                obvious = FALSE;
            }
        }
        else
        {
            bool unique = BOOL(race->flags1 & RF1_UNIQUE);

            if (!unique && !(mon->mflag2 & MFLAG2_QUESTOR) && power > 29 && randint1(100) < power)
            {
                note = " is in your thrall!";
                set_pet(mon);
            }
            else
            {
                switch (randint1(4))
                {
                case 1:
                    do_stun = power / 2;
                    break;
                case 2:
                    if (race->flags3 & RF3_NO_CONF)
                    {
                        mon_lore_3(mon, RF3_NO_CONF);
                        note = " is unaffected.";
                        break;
                    }
                    else if (!unique)
                    {
                        do_conf = power / 2;
                        break;
                    }
                default:
                    if (prace_is_(RACE_MON_VAMPIRE))
                    {
                        if (!unique && randint1(race->level) < randint1(power))
                        {
                            note = " is frozen in terror!";
                            do_paralyzed = randint1(3);
                        }
                        break;
                    }
                    do_fear = power;
                }
            }
        }
        break; }
    case GF_ICE:
        if (touch && seen_msg) msg_format("%^s is <color:W>frozen</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (who || !mon_save_p(mon->r_idx, A_NONE))
            do_stun = _gf_distance_mod(randint1(15));
        if (race->flagsr & RFR_IM_COLD)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_IM_COLD);
        }
        else if (race->flagsr & RFR_RES_COLD)
        {
            note = " resists.";
            dam /= 2;
            mon_lore_r(mon, RFR_RES_COLD);
        }
        else if (race->flags3 & (RF3_HURT_COLD))
        {
            note = " is hit hard.";
            dam *= 2;
            mon_lore_3(mon, RF3_HURT_COLD);
        }
        break;
    case GF_OLD_DRAIN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(race))
        {
            mon_lore_3(mon, RF3_DEMON);
            mon_lore_3(mon, RF3_UNDEAD);
            mon_lore_3(mon, RF3_NONLIVING);
            note = " is unaffected!";
            obvious = FALSE;
            dam = 0;
        }
        else
        {
            if (dragon_vamp_hack)
                dragon_vamp_amt += dam;
            vampirism_hack = mon->hp;
            do_time = (dam+7)/8;
        }
        break;
    case GF_DEATH_TOUCH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(race))
        {
            mon_lore_3(mon, RF3_DEMON);
            mon_lore_3(mon, RF3_UNDEAD);
            mon_lore_3(mon, RF3_NONLIVING);
            note = " is immune.";
            obvious = FALSE;
            dam = 0;
        }
        else if (((race->flags1 & RF1_UNIQUE) && randint1(888) != 666)
                || mon_save_p(mon->r_idx, A_INT)
                || mon_save_p(mon->r_idx, A_INT) )
        {
            note = " resists!";
            obvious = FALSE;
            dam = 0;
        }
        break;
    case GF_DEATH_RAY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (!monster_living(race))
        {
            mon_lore_3(mon, RF3_DEMON);
            mon_lore_3(mon, RF3_UNDEAD);
            mon_lore_3(mon, RF3_NONLIVING);
            note = " is immune.";
            obvious = FALSE;
            dam = 0;
        }
        else if ( ((race->flags1 & RF1_UNIQUE) && randint1(888) != 666)
               || (race->level + randint1(20) > randint1(caster_lev)))
        {
            note = " resists!";
            obvious = FALSE;
            dam = 0;
        }

        break;
    case GF_OLD_POLY:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_poly = TRUE;
        if ((race->flags1 & RF1_UNIQUE) || (race->flags7 & RF7_NAZGUL) ||
            (race->flags7 & RF7_UNIQUE2) || (mon->mflag2 & MFLAG2_QUESTOR) ||
            (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            note = " is unaffected!";

            do_poly = FALSE;
            obvious = FALSE;
        }
        dam = 0;
        break;
    case GF_OLD_CLONE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        if ( p_ptr->inside_arena
          || is_pet(mon)
          || (race->flags1 & RF1_UNIQUE)
          || (race->flags7 & (RF7_NAZGUL | RF7_UNIQUE2))
          || (mon->mflag2 & MFLAG2_QUESTOR) )
        {
            note = " is unaffected!";
        }
        else
        {
            mon->hp = mon->maxhp;
            if (multiply_monster(mon->id, TRUE, 0L))
                note = " spawns!";
        }
        dam = 0;
        break;
    case GF_STAR_HEAL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        (void)set_monster_csleep(mon->id, 0);

        if (mon->maxhp < mon->max_maxhp)
        {
            if (seen_msg) msg_format("%^s recovers %s vitality.", m_name, m_poss);
            mon->maxhp = mon->max_maxhp;
        }

        if (!dam)
        {
            /* Redraw (later) if needed */
            check_mon_health_redraw(mon->id);
            break;
        }

        /* Fall through */
    case GF_OLD_HEAL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        (void)set_monster_csleep(mon->id, 0);
        if (MON_STUNNED(mon))
        {
            if (seen_msg) msg_format("%^s is no longer stunned.", m_name);
            (void)set_monster_stunned(mon->id, 0);
        }
        if (MON_CONFUSED(mon))
        {
            if (seen_msg) msg_format("%^s is no longer confused.", m_name);
            (void)set_monster_confused(mon->id, 0);
        }
        if (MON_MONFEAR(mon))
        {
            if (seen_msg) msg_format("%^s recovers %s courage.", m_name, m_poss);
            (void)set_monster_monfear(mon->id, 0);
        }

        if (!who && !(race->flags3 & RF3_EVIL))
            dam = dam * (625 + virtue_current(VIRTUE_COMPASSION))/625;

        /* Heal */
        if (mon->hp < 30000) mon->hp += dam;

        /* No overflow */
        if (mon->hp > mon->maxhp) mon->hp = mon->maxhp;

        if (!who)
        {
            virtue_add(VIRTUE_VITALITY, 1);

            if (race->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_INDIVIDUALISM, 1);

            if (is_friendly(mon))
                virtue_add(VIRTUE_HONOUR, 1);
            else if (!(race->flags3 & RF3_EVIL))
            {
                if (race->flags3 & RF3_GOOD)
                    virtue_add(VIRTUE_COMPASSION, 2);
                else
                    virtue_add(VIRTUE_COMPASSION, 1);
            }

            if (race->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }

        if (mon->r_idx == MON_LEPER)
        {
            heal_leper = TRUE;
            if (!who) virtue_add(VIRTUE_COMPASSION, 5);
        }

        /* Redraw (later) if needed */
        check_mon_health_redraw(mon->id);

        /* Message */
        note = " looks healthier.";

        /* No "real" damage */
        dam = 0;
        break;
    case GF_OLD_SPEED:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        /* Speed up */
        if (set_monster_fast(mon->id, MON_FAST(mon) + 100))
        {
            note = " starts moving faster.";
        }

        if (!who)
        {
            if (race->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_INDIVIDUALISM, 1);
            if (is_friendly(mon))
                virtue_add(VIRTUE_HONOUR, 1);
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_OLD_SLOW:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()

        if ( (race->flags1 & RF1_UNIQUE)
          || race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10 )
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            if (set_monster_slow(mon->id, MON_SLOW(mon) + 50))
                note = " starts moving slower.";
        }
        dam = 0;
        break;
    case GF_UNHOLY_WORD:
        if (is_pet(mon) && (race->flags3 & RF3_EVIL))
        {
            if (seen) obvious = TRUE;

            set_monster_csleep(mon->id, 0);
            if (MON_STUNNED(mon))
            {
                if (seen_msg) msg_format("%^s is no longer stunned.", m_name);
                set_monster_stunned(mon->id, 0);
            }
            if (MON_CONFUSED(mon))
            {
                if (seen_msg) msg_format("%^s is no longer confused.", m_name);
                set_monster_confused(mon->id, 0);
            }
            if (MON_MONFEAR(mon))
            {
                if (seen_msg) msg_format("%^s recovers %s courage.", m_name, m_poss);
                set_monster_monfear(mon->id, 0);
            }

            if (mon->hp < 30000) mon->hp += dam;
            if (mon->hp > mon->maxhp) mon->hp = mon->maxhp;
            set_monster_fast(mon->id, MON_FAST(mon) + 100);
            note = " fights with renewed vigor!";
        }
        dam = 0;
        break;
    case GF_STASIS_EVIL:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((race->flags1 & RF1_UNIQUE) ||
            !(race->flags3 & RF3_EVIL) ||
            (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            note = " is unaffected!";

            obvious = FALSE;
        }
        else
        {
            note = " is suspended!";
            do_paralyzed = 3;
        }
        dam = 0;
        break;
    case GF_PARALYSIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((race->flags1 & RF1_UNIQUE) || race->level > randint1(dam))
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            note = " is paralyzed!";
            do_paralyzed = randint1(3);
        }
        dam = 0;
        break;
    case GF_STASIS:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if ((race->flags1 & RF1_UNIQUE) || race->level > randint1(dam))
        {
            note = " is unaffected!";
            obvious = FALSE;
        }
        else
        {
            note = " is suspended!";
            do_paralyzed = 2;
            if (one_in_(15)) do_paralyzed++;
        }
        dam = 0;
        break;
    case GF_SUBJUGATION: {
        bool unique = BOOL(race->flags1 & RF1_UNIQUE);
        int  attempts = randint1(1 + p_ptr->lev/50);
        int  ct = 0;

        if (seen) obvious = TRUE;
        set_monster_csleep(mon->id, 0);
        _BABBLE_HACK()
        if (is_pet(mon))
            return FALSE;
        while (attempts--)
        {
            switch (randint1(5))
            {
            case 1:
                if (randint1(race->level) <= randint1(dam))
                {
                    do_stun = dam / 2;
                    ct++;
                }
                break;
            case 2:
                if (!(race->flags3 & RF3_NO_CONF) && randint1(race->level) <= randint1(dam))
                {
                    do_conf = dam / 2;
                    ct++;
                }
                break;
            case 3:
                if (!unique && !(mon->mflag2 & MFLAG2_QUESTOR) && randint1(race->level) <= randint1(dam))
                {
                    note = " is frozen in terror!";
                    do_paralyzed = randint1(3);
                    attempts = 0;
                    ct++;
                }
                break;
            case 4:
                if (randint1(race->level) <= randint1(dam))
                {
                    do_fear = dam;
                    ct++;
                }
                break;
            default:
                if (unique || (mon->mflag2 & MFLAG2_QUESTOR) || p_ptr->inside_arena)
                {
                }
                else if ((mon->mflag2 & MFLAG2_NOPET) || randint1(race->level) > randint1(dam))
                {
                    if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
                }
                else
                {
                    msg_format("%s bows to your will!", m_name);

                    set_pet(mon);
                    attempts = 0;
                    ct++;

                    virtue_add(VIRTUE_INDIVIDUALISM, -1);
                    if (race->flags3 & RF3_ANIMAL)
                        virtue_add(VIRTUE_NATURE, 1);

                    /* Ignore any prior effects */
                    return TRUE;
                }
            }
        }
        if (!ct)
            note = " resists";
        dam = 0;
        break; }
    case GF_ELDRITCH_HOWL:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (seen) obvious = TRUE;

        do_fear = damroll(3, (dam / 2)) + 1;

        if ((race->flags1 & (RF1_UNIQUE)) ||
            (race->flags3 & (RF3_NO_FEAR)))
        {
            note = " is unaffected!";
            obvious = FALSE;
            do_fear = 0;
        }
        else if (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
        {
            note = " resists!";
            obvious = FALSE;
            do_fear = 0;
        }
        else if (race->level <= randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
        {
            note = " is frozen with terror!";
            do_paralyzed = randint1(3);
        }
        dam = 0;
        break;
    case GF_CHARM:
    case GF_CHARM_RING_BEARER:
    {
        int voima, taso = race->level, mon_difficulty = 0;
        bool is_friend = is_friendly(mon);
        if ((type == GF_CHARM) && (is_pet(mon))) return (obvious);
        if (!allow_pets) return TRUE;

        if (seen) obvious = TRUE;

        if (type == GF_CHARM_RING_BEARER)
        {
            if (!mon_is_type(mon->r_idx, SUMMON_RING_BEARER))
            {
                note = " is not a suitable ring bearer.";
                dam = 0;
                break;
            }
        }
        else
        {
            dam = charm_pow_modify(dam, mon);
        }
        if (race->flags1 & RF1_UNIQUE) mon_difficulty = 25;
        if ((dungeon_type) && (d_info[dungeon_type].final_guardian == mon->r_idx)) mon_difficulty = 50;
        if (mon->mflag2 & MFLAG2_QUESTOR) mon_difficulty = 50;
        taso = MAX(taso, mon_difficulty);
        voima = randint1(dam);

        if (p_ptr->wizard) msg_format("Charm power: %d/%d", voima, dam);

        if ((race->flagsr & RFR_RES_ALL) || (mon->mflag2 & MFLAG2_NOPET) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_RES_ALL);
            break;
        }
        else if ((mon->mflag2 & MFLAG2_QUESTOR) && (!p_ptr->uimapuku))
        {
            if (!is_friend) note = " is unaffected!";
            obvious = FALSE;
        }
        else if (taso > voima)
        {
            if (!is_friend) note = " resists!";
            obvious = FALSE;
            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else if ((p_ptr->cursed & OFC_AGGRAVATE) && ((!p_ptr->uimapuku) || (!one_in_(3))))
        {
            if (!is_friend) note = " hates you too much!";
            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            bool upgrade = !is_friend;

            if ((!mon_difficulty) || (dam > taso * 2) || (one_in_(10)))
            {
                set_pet(mon);
                upgrade = TRUE;
            }
            else set_friendly_ingame(mon);

            if (upgrade)
            {
                virtue_add(VIRTUE_INDIVIDUALISM, -1);
                if (race->flags3 & RF3_ANIMAL)
                    virtue_add(VIRTUE_NATURE, 1);
                if (is_friend) note = " suddenly seems even more friendly!";
                else note = " suddenly seems friendly!";
            }
        }
        dam = 0;
        break;
    }
    case GF_CONTROL_UNDEAD:
    case GF_CONTROL_DEMON:
    case GF_CONTROL_ANIMAL:
        if (is_pet(mon)) return (obvious);
        if (!allow_pets) return TRUE;

        if (seen) obvious = TRUE;

        if (type == GF_CONTROL_ANIMAL) dam += virtue_current(VIRTUE_NATURE)/10;
        else dam += virtue_current(VIRTUE_UNLIFE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if (p_ptr->spin > 0) dam += MAX(25, dam * 2 / 5);
        if (p_ptr->uimapuku) dam = dam * 3 / 2;
            
        if ((race->flagsr & RFR_RES_ALL) || (mon->mflag2 & MFLAG2_NOPET) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_RES_ALL);
            break;
        }

        if ((race->flags1 & RF1_UNIQUE) || (race->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((mon->mflag2 & MFLAG2_QUESTOR) ||
            ((type == GF_CONTROL_UNDEAD) && (!(race->flags3 & RF3_UNDEAD))) ||
            ((type == GF_CONTROL_DEMON) && (!(race->flags3 & RF3_DEMON))) ||
            ((type == GF_CONTROL_ANIMAL) && (!(race->flags3 & RF3_ANIMAL))) ||
            ((type == GF_CONTROL_ANIMAL) && (race->flags3 & RF3_NO_CONF)) ||
            (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
        {
            /* Memorize a flag */
            if ((type == GF_CONTROL_ANIMAL) && (race->flags3 & RF3_NO_CONF))
            {
                mon_lore_3(mon, RF3_NO_CONF);
            }

            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            if (type == GF_CONTROL_ANIMAL) note = " is tamed!";
            else note = " is in your thrall!";

            set_pet(mon);
            if (race->flags3 & RF3_ANIMAL) virtue_add(VIRTUE_NATURE, 1);
        }
        /* No "real" damage */
        dam = 0;
        break;
    case GF_CONTROL_PACT_MONSTER:
        if (!allow_pets) return TRUE;
        if (warlock_is_pact_monster(race) && !is_pet(mon))
        {
            if (seen) obvious = TRUE;

            /* Attempt a saving throw */
            if ( (mon->mflag2 & MFLAG2_QUESTOR)
              || (mon->mflag2 & MFLAG2_NOPET)
              || mon_save_p(mon->r_idx, A_CHR)
              || ((race->flags1 & RF1_UNIQUE) && mon_save_p(mon->r_idx, A_CHR)) )
            {
                if (warlock_is_(WARLOCK_HOUNDS))
                    note = " growls at you in defiance!";
                else
                    note = " resists your control.";
                obvious = FALSE;
                if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
            }
            else if (p_ptr->cursed & OFC_AGGRAVATE)
            {
                note = " finds you very aggravating!";
                if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
            }
            else
            {
                if (warlock_is_(WARLOCK_HOUNDS))
                    note = " rolls on its back in submission.";
                else
                    note = " obeys your will.";
                set_pet(mon);
            }
        }

        /* No "real" damage */
        dam = 0;
        break;
    case GF_CONTROL_LIVING:
        if (is_pet(mon)) return (obvious);
        if (!allow_pets) return TRUE;
        if (seen) obvious = TRUE;

        dam += (adj_chr_chm[p_ptr->stat_ind[A_CHR]]);
        dam -= virtue_current(VIRTUE_UNLIFE)/10;
        dam -= virtue_current(VIRTUE_INDIVIDUALISM)/20;

        if (race->flags3 & (RF3_NO_CONF)) dam -= 30;
        if (p_ptr->spin > 0) dam += MAX(25, dam * 2 / 5);
        if (p_ptr->uimapuku) dam = dam * 3 / 2;
        if (dam < 1) dam = 1;
        msg_format("You stare into %s.", m_name_object);
        if ((race->flagsr & RFR_RES_ALL) || (mon->mflag2 & MFLAG2_NOPET) || p_ptr->inside_arena)
        {
            note = " is immune.";
            dam = 0;
            mon_lore_r(mon, RFR_RES_ALL);
            break;
        }

        if ((race->flags1 & RF1_UNIQUE) || (race->flags7 & RF7_NAZGUL))
            dam = dam * 2 / 3;

        /* Attempt a saving throw */
        if ((mon->mflag2 & MFLAG2_QUESTOR) ||
             !monster_living(race) ||
             ((race->level+10) > randint1(dam)))
        {
            /* Resist */
            /* No obvious effect */
            note = " is unaffected!";

            obvious = FALSE;
            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else if (p_ptr->cursed & OFC_AGGRAVATE)
        {
            note = " hates you too much!";

            if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
        }
        else
        {
            note = " is tamed!";

            set_pet(mon);

            if (race->flags3 & RF3_ANIMAL)
                virtue_add(VIRTUE_NATURE, 1);
        }
        dam = 0;
        break;
    case GF_OLD_SLEEP: {
        int ml = race->level;
        int pl = dam;

        if (race->flags1 & RF1_UNIQUE) ml += 3;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags3 & RF3_NO_SLEEP)
        {
            note = " is immune!";
            mon_lore_3(mon, RF3_NO_SLEEP);
            obvious = FALSE;
        }
        else if (race->flags1 & RF1_UNIQUE) /* Historical ... I'd like to remove this. */
        {
            note = " is immune!";
            obvious = FALSE;
        }
        else if (randint1(ml) >= randint1(pl))
        {
            note = " resists!";
            obvious = FALSE;
        }
        else
        {
            note = " falls asleep!";
            do_sleep = 500;
        }
        dam = 0;
        break; }
    case GF_BLIND:
    case GF_OLD_CONF: {
        int ml = race->level;
        int pl = caster_lev;

        if (race->flags1 & RF1_UNIQUE) ml += 3;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_conf = damroll(3, (pl / 2)) + 1;
        if (race->flags3 & RF3_NO_CONF)
        {
            do_conf = 0;
            note = " is immune!";
            mon_lore_3(mon, RF3_NO_CONF);
            obvious = FALSE;
        }
        else if (randint1(ml) >= randint1(pl))
        {
            do_conf = 0;
            note = " resists!";
            obvious = FALSE;
        }

        /* No "real" damage */
        dam = 0;
        break; }
    case GF_STUN:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        do_stun = dam;
        if (race->flags3 & RF3_NO_STUN)
        {
            do_stun = 0;
            note = " is unaffected!";
            mon_lore_3(mon, RF3_NO_STUN);
            obvious = FALSE;
        }
        dam = 0;
        break;
    case GF_LITE_WEAK:
        if (!dam)
        {
            skipped = TRUE;
            break;
        }
        if (race->flagsr & RFR_RES_ALL)
        {
            dam = 0;
            break;
        }
        /* Hurt by light */
        if ((race->flags3 & (RF3_HURT_LITE)) && (!no_harm))
        {
            /* Obvious effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            mon_lore_3(mon, RF3_HURT_LITE);

            /* Special effect */
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";

        }
        /* Normally no damage */
        else dam = 0;
        break;
    case GF_LITE:
        if (touch && seen_msg) msg_format("%^s is <color:y>dazzled</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_LITE)
        {
            note = " resists.";

            dam *= 2; dam /= (randint1(6)+6);
            mon_lore_r(mon, RFR_RES_LITE);
        }
        else if ((race->flags3 & (RF3_HURT_LITE)) && (!no_harm))
        {
            mon_lore_3(mon, RF3_HURT_LITE);
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";

            dam *= 2;
        }
        break;
    case GF_DARK:
        if (touch && seen_msg) msg_format("%^s is <color:D>benighted</color>!", m_name);
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flagsr & RFR_RES_DARK)
        {
            note = " resists.";
            dam *= 2; dam /= (randint1(6)+6);
            mon_lore_r(mon, RFR_RES_DARK);
        }
        break;
    case GF_KILL_WALL: /* e.g. Stone to Mud */
        if (race->flagsr & RFR_RES_ALL)
        {
            dam = 0;
            break;
        }
        if (race->flags3 & (RF3_HURT_ROCK))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_HURT_ROCK);
            note = " loses some skin!";
            note_dies = " dissolves!";
        }
        /* Usually, ignore the effects */
        else dam = 0;
        break;
    case GF_AWAY_UNDEAD:
        if (race->flags3 & (RF3_UNDEAD))
        {
            bool resists_tele = FALSE;
            if (race->flagsr & RFR_RES_TELE)
            {
                if ((race->flags1 & (RF1_UNIQUE)) || (race->flagsr & RFR_RES_ALL))
                {
                    mon_lore_r(mon, RFR_RES_TELE);
                    note = " is unaffected!";

                    resists_tele = TRUE;
                }
                else if (race->level > randint1(100))
                {
                    mon_lore_r(mon, RFR_RES_TELE);
                    note = " resists!";

                    resists_tele = TRUE;
                }
            }
            if (!resists_tele)
            {
                if (seen) obvious = TRUE;
                mon_lore_3(mon, RF3_UNDEAD);
                do_dist = dam;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_AWAY_EVIL:
        if (race->flags3 & (RF3_EVIL))
        {
            bool resists_tele = FALSE;
            if (race->flagsr & RFR_RES_TELE)
            {
                if ((race->flags1 & (RF1_UNIQUE)) || (race->flagsr & RFR_RES_ALL))
                {
                    mon_lore_r(mon, RFR_RES_TELE);
                    note = " is unaffected!";

                    resists_tele = TRUE;
                }
                else if (race->level > randint1(100))
                {
                    mon_lore_r(mon, RFR_RES_TELE);
                    note = " resists!";

                    resists_tele = TRUE;
                }
            }
            if (!resists_tele)
            {
                if (seen) obvious = TRUE;
                mon_lore_3(mon, RF3_EVIL);
                do_dist = dam;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_ISOLATION: {
        bool resists_tele = FALSE;
        if (mon->id == p_ptr->duelist_target_idx)
        {
            dam = 0;
            return TRUE;
        }
        if (race->flagsr & RFR_RES_TELE)
        {
            if ((race->flagsr & RFR_RES_ALL) || (race->flags1 & RF1_UNIQUE))
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " is unaffected!";
                resists_tele = TRUE;
            }
            else if (mon_save_p(mon->r_idx, A_DEX))
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " resists!";
                resists_tele = TRUE;
            }
        }
        if (!resists_tele)
        {
            if (seen) obvious = TRUE;
            do_dist = dam;
        }
        dam = 0;
        break; }
    case GF_AWAY_ALL: {
        bool resists_tele = FALSE;
        if (race->flagsr & RFR_RES_TELE)
        {
            if ( (race->flags1 & RF1_UNIQUE)
              || (race->flagsr & RFR_RES_ALL) 
              || (mon->smart & (1U << SM_GUARDIAN)) )
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " is unaffected!";
                resists_tele = TRUE;
            }
            else if (race->level > randint1(100))
            {
                mon_lore_r(mon, RFR_RES_TELE);
                note = " resists!";
                resists_tele = TRUE;
            }
        }
        else if (mon->smart & (1U << SM_GUARDIAN))
        {
            note = " is unaffected!";
            resists_tele = TRUE;
        }
        if (!resists_tele)
        {
            if (seen) obvious = TRUE;
            do_dist = dam;
        }
        dam = 0;
        break; }
    case GF_TURN_UNDEAD:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (race->flags3 & (RF3_UNDEAD))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_UNDEAD);

            do_fear = damroll(3, (dam / 2)) + 1;
            if (fear_save_m(mon))
            {
                note = " is unaffected!";
                obvious = FALSE;
                do_fear = 0;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_TURN_EVIL:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (race->flags3 & (RF3_EVIL))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_EVIL);

            do_fear = damroll(3, (dam / 2)) + 1;

            if (fear_save_m(mon))
            {
                note = " is unaffected!";
                obvious = FALSE;
                do_fear = 0;
            }
        }
        else skipped = TRUE;
        dam = 0;
        break;
    case GF_TURN_ALL:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            break;
        }
        if (seen) obvious = TRUE;
        do_fear = damroll(3, (dam / 2)) + 1;
        if ((race->flags3 & RF3_NO_FEAR) || fear_save_m(mon))
        {
            if (!touch) note = " is unaffected!";
            obvious = FALSE;
            do_fear = 0;
        }
        dam = 0;
        break;
    case GF_DISP_UNDEAD:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (race->flags3 & (RF3_UNDEAD))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_UNDEAD);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_EVIL:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (race->flags3 & RF3_EVIL)
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_EVIL);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_GOOD:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (race->flags3 & RF3_GOOD)
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_GOOD);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_LIVING:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (monster_living(race))
        {
            if (seen) obvious = TRUE;
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_DEMON:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (race->flags3 & (RF3_DEMON))
        {
            if (seen) obvious = TRUE;
            mon_lore_3(mon, RF3_DEMON);
            note = " shudders.";
            note_dies = " dissolves!";
        }
        else
        {
            skipped = TRUE;
            dam = 0;
        }
        break;
    case GF_DISP_ALL:
        if (race->flagsr & RFR_RES_ALL)
        {
            skipped = TRUE;
            dam = 0;
            break;
        }
        if (seen) obvious = TRUE;
        note = " shudders.";
        note_dies = " dissolves!";
        break;
    case GF_DRAINING_TOUCH:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (mon_is_magical(mon))
        {
            /*msg_format("You draw psychic energy from %s.", m_name);*/
            p_ptr->csp += dam;
            if (p_ptr->csp > p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }
            p_ptr->redraw |= PR_MANA;
        }
        else
        {
            msg_format("%^s is unaffected.", m_name);
            dam = 0;
        }
        break;
    case GF_DRAIN_MANA:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (mon_is_magical(mon))
        {
            if (who > 0)
            {
                if (caster_ptr->hp < caster_ptr->maxhp)
                {
                    caster_ptr->hp += 6 * dam;
                    if (caster_ptr->hp > caster_ptr->maxhp) caster_ptr->hp = caster_ptr->maxhp;
                    check_mon_health_redraw(who);
                    monster_desc(killer, caster_ptr, 0);
                    msg_format("%^s appears healthier.", killer);
                }
            }
            else if (!no_harm)
            {
                msg_format("You draw psychic energy from %s.", m_name_object);
                hp_player(dam);
            }
        }
        else if (seen_msg) msg_format("%^s is unaffected.", m_name);
        dam = 0;
        break;
    case GF_ANTIMAGIC: { /* Formerly restricted to rage-mage. But now, mon vs mon */
        bool save = FALSE;/* and the possessor can use the ANTIMAGIC monster spell */
        if (seen) obvious = TRUE;
        _BABBLE_HACK()            /*v---Possessor uses monster level (dam) for the save */
        if (who == GF_WHO_PLAYER && !mon_spell_current())
        {
            int stat = A_CHR;
            int rolls = 1, made = 0;
            int i;
            if (p_ptr->pclass == CLASS_RAGE_MAGE)
            {
                stat = A_STR;
                if (p_ptr->shero) rolls++;
            }
            for (i = 0; i < rolls; i++)
            {
                if (mon_save_p(mon->r_idx, stat))
                    made++;
            }
            save = (made == rolls);
        }
        else
        {
            save = mon_save_aux(mon->r_idx, dam);
        }
        if (save)
        {
            if (seen_msg) msg_format("%^s resists!", m_name);
            dam = 0;
            mon->anti_magic_ct = 0;
            return TRUE;
        }
        else
        {
            int dur = 2 + randint1(2);
            mon->anti_magic_ct = dur;
            if (seen_msg) msg_format("%^s can no longer cast spells!", m_name);
            dam = 0;
            return TRUE;
        }
        break; }
    case GF_PSI_EGO_WHIP:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        /* Hack: No damage now ... simply latch the whip on and let process_monster()
           do the dirty work for us */
        (void)set_monster_csleep(mon->id, 0);
        mon->ego_whip_ct = 5;
        mon->ego_whip_pow = dam;
        p_ptr->redraw |= PR_HEALTH_BARS;
        return TRUE;
    case GF_PSI_BRAIN_SMASH: /* dam is the power of the effect (1-5) */
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(mon, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (psion_mon_save_p(mon->r_idx, dam))
        {
            note = " resists!";
            dam = 0;
        }
        else if (race->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(mon, RF2_WEIRD_MIND);
            note = " resists somewhat.";
            dam /= 2;
            if (dam == 0) dam = 1;
        }
        if (dam)
        {
            note = " is blasted by psionic energy.";
            if (race->flags3 & RF3_NO_CONF)
                mon_lore_3(mon, RF3_NO_CONF);
            else
                do_conf = 2*dam;
            if (race->flags3 & RF3_NO_STUN)
                mon_lore_3(mon, RF3_NO_STUN);
            else
                do_stun = 2*dam;
            m_inc_minislow(mon, dam);
            dam = 0;
        }
        break;
    case GF_MIND_BLAST:
        if (seen) obvious = TRUE;
        if (who == GF_WHO_PLAYER) msg_format("You gaze intently at %s.", m_name_object);
        _BABBLE_HACK()
        if ((race->flags1 & RF1_UNIQUE) ||
             (race->flags3 & RF3_NO_CONF) ||
             (race->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
        {
            if (race->flags3 & (RF3_NO_CONF))
                mon_lore_3(mon, RF3_NO_CONF);
            note = " is unaffected!";
            dam = 0;
        }
        else if (race->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(mon, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (race->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(mon, RF2_WEIRD_MIND);
            note = " resists.";
            dam /= 3;
        }
        else
        {
            note = " is blasted by psionic energy.";
            note_dies = " collapses, a mindless husk.";

            if (who > 0) do_conf = randint0(4) + 4;
            else do_conf = randint0(8) + 8;
        }
        break;
    case GF_BRAIN_SMASH:
        if (seen) obvious = TRUE;
        if (who == GF_WHO_PLAYER) msg_format("You gaze intently at %s.", m_name_object);
        _BABBLE_HACK()
        if ((race->flags1 & RF1_UNIQUE) ||
            /* (race->flags3 & RF3_NO_CONF) || */
             (race->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
        {
            if (race->flags3 & (RF3_NO_CONF))
                mon_lore_3(mon, RF3_NO_CONF);
            note = " is unaffected!";
            dam = 0;
        }
        else if (race->flags2 & RF2_EMPTY_MIND)
        {
            mon_lore_2(mon, RF2_EMPTY_MIND);
            note = " is immune!";
            dam = 0;
        }
        else if (race->flags2 & RF2_WEIRD_MIND)
        {
            mon_lore_2(mon, RF2_WEIRD_MIND);
            note = " resists.";
            dam /= 3;
        }
        else
        {
            note = " is blasted by psionic energy.";
            note_dies = " collapses, a mindless husk.";

            if (who > 0)
            {
                do_conf = randint0(4) + 4;
                do_stun = randint0(4) + 4;
            }
            else
            {
                do_conf = randint0(8) + 8;
                do_stun = randint0(8) + 8;
            }
            set_monster_slow(mon->id, MON_SLOW(mon) + 10);
            if (race->flags3 & RF3_NO_CONF) do_conf = 0;
        }
        break;
    case GF_CAUSE_1:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and curse.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (race->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_2:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and curse horribly.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (race->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_3:
        if (seen) obvious = TRUE;
        if (!who) msg_format("You point at %s, incanting terribly!", m_name);
        _BABBLE_HACK()
        if (randint0(100 + (caster_lev / 2)) < (race->level + 35))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break;
    case GF_CAUSE_4: {
        bool save = FALSE;
        if (seen) obvious = TRUE;
        if (!who) msg_format("You %s at %s and scream the word, 'DIE!'.", prace_is_(RACE_MON_BEHOLDER) ? "gaze" : "point", m_name);
        _BABBLE_HACK()
        if (who == GF_WHO_PLAYER)
        {
            save = p_ptr->current_r_idx != MON_KENSHIROU && mon_save_p(mon->r_idx, A_WIS);
        }
        else
        {
            save = ((randint0(100 + (caster_lev / 2)) < (race->level + 35))
                && ((who <= 0) || (caster_ptr->r_idx != MON_KENSHIROU)));
        }
        if ((save) || (no_harm))
        {
            note = " is unaffected!";
            dam = 0;
        }
        break; }
    case GF_HAND_DOOM:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags1 & RF1_UNIQUE)
        {
            note = " is unaffected!";
            dam = 0;
        }
        else
        {
            if (randint1(dam) >= race->level + randint1(20))
            {
                dam = ((40 + randint1(20)) * mon->hp) / 100;
                if (mon->hp < dam) dam = mon->hp - 1;
            }
            else
            {
                note = " resists!";
                dam = 0;
            }
        }
        break;
    case GF_CAPTURE: {
        int nokori_hp;
        if (!quests_allow_all_spells() && !is_pet(mon))
        {
            msg_format("%^s is unaffected.", m_name);
            skipped = TRUE;
            break;
        }
        if ( (race->flags7 & RF7_UNIQUE2)
          || (mon->mflag2 & MFLAG2_QUESTOR)
          || (mon->r_idx == MON_BANOR)
          || (mon->r_idx == MON_LUPART)
          || (mon->r_idx == MON_BANORLUPART)
          || mon->parent_m_idx )
        {
            msg_format("%^s is unaffected.", m_name);
            skipped = TRUE;
            break;
        }
        if ( ((race->flags1 & RF1_UNIQUE)
          || (race->flags7 & RF7_NAZGUL))
          && (!is_pet(mon)))
        {
            msg_format("%^s is unaffected.", m_name);
            skipped = TRUE;
            break;
        }
        if (is_pet(mon)) nokori_hp = mon->maxhp * 4;
        else if ((p_ptr->pclass == CLASS_BEASTMASTER) && monster_living(race))
            nokori_hp = mon->maxhp * 3 / 10;
        else if (p_ptr->easy_capture)
            nokori_hp = mon->maxhp * 3 / 10;
        else if (warlock_is_(WARLOCK_DRAGONS) && (race->flags3 & RF3_DRAGON))
            nokori_hp = mon->maxhp * 3 / 15;
        else
            nokori_hp = mon->maxhp * 3 / 20;

        if (mon->hp >= nokori_hp)
        {
            msg_format("You need to weaken %s more.", m_name);
            skipped = TRUE;
        }
        else if (mon->hp < randint0(nokori_hp))
        {
            if (mon->mflag2 & MFLAG2_CHAMELEON) choose_new_monster(mon->id, FALSE, MON_CHAMELEON);
            msg_format("You capture %^s!", m_name);
            quests_on_kill_mon(mon);
            cap_mon = mon->r_idx;
            cap_mspeed = mon->mspeed;
            cap_hp = mon->hp;
            cap_maxhp = mon->max_maxhp;
            cap_nickname = mon->nickname;
            if (mon->id == p_ptr->riding)
            {
                if (rakuba(-1, FALSE))
                {
                    msg_format("You have fallen from %s.", m_name);
                }
            }
            delete_monster_idx(mon->id);
            return TRUE;
        }
        else
        {
            msg_format("You failed to capture %s.", m_name);
            skipped = TRUE;
        }
        break; }
    case GF_ATTACK: /* dam = py_attack_mode */
        if (who > 0)
        {
            return mon_attack_mon(who, mon->id);
        }
        if (dam == BEHOLDER_GAZE && !los(mon->fy, mon->fx, py, px))
        {
            if (seen_msg) msg_format("%^s can't see you, and isn't affected!", m_name);
            skipped = TRUE;
            break;
        }

        /* Return this monster's death */
        return py_attack(mon->fy, mon->fx, dam);
    case GF_ENGETSU: {
        int effect = 0;
        bool done = TRUE;

        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (race->flags2 & RF2_EMPTY_MIND)
        {
            note = " is immune!";
            dam = 0;
            skipped = TRUE;
            mon_lore_2(mon, RF2_EMPTY_MIND);
            break;
        }
        if (MON_CSLEEP(mon))
        {
            note = " is immune!";
            dam = 0;
            skipped = TRUE;
            break;
        }

        if (one_in_(5)) effect = 1;
        else if (one_in_(4)) effect = 2;
        else if (one_in_(3)) effect = 3;
        else done = FALSE;

        if (effect == 1)
        {
            /* Powerful monsters can resist */
            if ((race->flags1 & RF1_UNIQUE) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                note = " is unaffected!";

                obvious = FALSE;
            }

            /* Normal monsters slow down */
            else
            {
                if (set_monster_slow(mon->id, MON_SLOW(mon) + 50))
                {
                    note = " starts moving slower.";
                }
            }
        }

        else if (effect == 2)
        {
            do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

            /* Attempt a saving throw */
            if ((race->flags1 & (RF1_UNIQUE)) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Resist */
                do_stun = 0;

                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
        }

        else if (effect == 3)
        {
            /* Attempt a saving throw */
            if ((race->flags1 & RF1_UNIQUE) ||
                (race->flags3 & RF3_NO_SLEEP) ||
                (race->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
            {
                /* Memorize a flag */
                if (race->flags3 & RF3_NO_SLEEP)
                {
                    mon_lore_3(mon, RF3_NO_SLEEP);
                }

                /* No obvious effect */
                note = " is unaffected!";

                obvious = FALSE;
            }
            else
            {
                /* Go to sleep (much) later */
                note = " falls asleep!";
                do_paralyzed = 3;
            }
        }

        if (!done)
        {
            note = " is immune!";
        }
        dam = 0;
        break; }
    case GF_ENTOMB: {
        int dir, x, y;

        if (is_pet(mon) || is_friendly(mon))
        {
            msg_print("Failed!");
            return FALSE;
        }

        for (dir = 0; dir < 8; dir++)
        {
            y = mon->fy + ddy_ddd[dir];
            x = mon->fx + ddx_ddd[dir];

            if (!cave_naked_bold(y, x)) continue;
            if (p_ptr->lev < 45)
                cave_set_feat(y, x, feat_rubble);
            else
                cave_set_feat(y, x, feat_granite);
        }
        return TRUE; }
    case GF_GENOCIDE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        if (genocide_aux(mon->id, dam, !who, (race->level + 1) / 2, "Genocide One"))
        {
            if (seen_msg)
            {
                if (dam == 666) /* Hack for Daemon flavored message */
                    msg_format("%^s is sent directly to hell!", m_name);
                else
                    msg_format("%^s disappeared!", m_name);
            }

            virtue_add(VIRTUE_VITALITY, -1);
            return TRUE;
        }
        skipped = TRUE;
        break;
    case GF_PHOTO:
        if (!who) msg_format("You take a photograph of %s.", m_name);
        /* Hurt by light */
        if ((race->flags3 & (RF3_HURT_LITE)) && (!no_harm))
        {
            /* Obvious effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            mon_lore_3(mon, RF3_HURT_LITE);

            /* Special effect */
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";
        }
        /* Normally no damage */
        else dam = 0;
        photo = mon->r_idx;
        break;
    case GF_BLOOD_CURSE:
        if (seen) obvious = TRUE;
        _BABBLE_HACK()
        break;
    case GF_CRUSADE: {
        bool success = FALSE;
        if (seen) obvious = TRUE;

        if ((race->flags3 & (RF3_GOOD)) && !p_ptr->inside_arena)
        {
            if (race->flags3 & (RF3_NO_CONF)) dam -= 50;
            if (dam < 1) dam = 1;

            /* No need to tame your pet */
            if (is_pet(mon))
            {
                note = " starts moving faster.";

                (void)set_monster_fast(mon->id, MON_FAST(mon) + 100);
                success = TRUE;
            }

            /* Attempt a saving throw */
            else if ((mon->mflag2 & MFLAG2_QUESTOR) ||
                (race->flags1 & (RF1_UNIQUE)) ||
                (mon->mflag2 & MFLAG2_NOPET) ||
                (p_ptr->cursed & OFC_AGGRAVATE) ||
                 ((race->level+10) > randint1(dam)))
            {
                /* Resist */
                if (_failed_charm_nopet_chance(mon)) mon->mflag2 |= MFLAG2_NOPET;
            }
            else
            {
                note = " is tamed!";

                set_pet(mon);
                (void)set_monster_fast(mon->id, MON_FAST(mon) + 100);

                /* Learn about type */
                mon_lore_3(mon, RF3_GOOD);
                success = TRUE;
            }
        }

        if (!success)
        {
            if (!(race->flags3 & RF3_NO_FEAR))
            {
                do_fear = randint1(90)+10;
            }
            else
            {
                mon_lore_3(mon, RF3_NO_FEAR);
            }
        }
        dam = 0;
        break; }
    default:
        skipped = TRUE;
        dam = 0;
    }

    /* Absolutely no effect */
    if (skipped) return (FALSE);

    /* "Unique" monsters cannot be polymorphed */
    if (race->flags1 & (RF1_UNIQUE)) do_poly = FALSE;
    if (race->flags7 & (RF7_NAZGUL)) do_poly = FALSE;
    if (race->flags7 & (RF7_UNIQUE2)) do_poly = FALSE;

    /* Quest monsters cannot be polymorphed */
    if (mon->mflag2 & MFLAG2_QUESTOR) do_poly = FALSE;

    if (p_ptr->riding && (mon->id == p_ptr->riding)) do_poly = FALSE;

    /* "Unique" and "quest" monsters can only be "killed" by the player. */
    if (((race->flags1 & RF1_UNIQUE) || (race->flags7 & RF7_NAZGUL) || (mon->mflag2 & MFLAG2_QUESTOR))
      && !p_ptr->inside_battle
      && !prace_is_(RACE_MON_QUYLTHULG))
    {
        if (who && (dam > mon->hp)) dam = mon->hp;
    }

    if (!who && slept)
    {
        if (!(race->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_COMPASSION, -1);
        if (!(race->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_HONOUR, -1);
    }

    /* Modify the damage */
    tmp = dam;
    if (dam)
    {
        if (who > 0)
            dam = mon_damage_mod_mon(mon, dam, type == GF_PSY_SPEAR);
        else
            dam = mon_damage_mod(mon, dam, type == GF_PSY_SPEAR);
    }
    if (tmp > 0 && dam == 0)
        note = " is unharmed.";

    /* Check for death */
    if ((dam > mon->hp) && (!no_harm))
    {
        /* Extract method of death */
        note = note_dies;
    }
    else
    {
        /* Sound and Impact resisters never stun
         * XXX Why should RES_SOUND protect from mental attacks?! */
        if ( do_stun
          && (_is_mental_attack(type) || !(race->flagsr & (RFR_RES_SOUN | RFR_RES_WALL)))
          && !(race->flags3 & RF3_NO_STUN) )
        {
            if (mon_stun(mon, do_stun))
                note = " is <color:B>dazed</color>.";
            else
                note = " is <color:B>more dazed</color>.";

            if (seen) obvious = TRUE;
            get_angry = TRUE;
        }

        if (do_conf && (type == GF_ELDRITCH_CONFUSE || !(race->flags3 & RF3_NO_CONF)))
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            /* Already partially confused */
            if (MON_CONFUSED(mon))
            {
                if (type == GF_BLIND)
                    note = " is more blinded.";
                else
                    note = " looks more <color:U>confused</color>.";
                tmp = MON_CONFUSED(mon) + (do_conf / 2);
            }

            /* Was not confused */
            else
            {
                if (type == GF_BLIND)
                    note = " is blinded.";
                else
                    note = " looks <color:U>confused</color>.";
                tmp = do_conf;
            }

            /* Apply confusion */
            (void)set_monster_confused(mon->id, tmp);

            /* Get angry */
            get_angry = TRUE;
        }

        if (do_time)
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            if (do_time >= mon->maxhp) do_time = mon->maxhp - 1;

            if (do_time)
            {
                if (flags & GF_AFFECT_SPELL)
                    note = " seems weakened.";
                mon->maxhp -= do_time;
                if ((mon->hp - dam) > mon->maxhp) dam = mon->hp - mon->maxhp;
            }
            get_angry = TRUE;
        }

        /* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
        if (do_poly && (randint1(90) > race->level))
        {
            if (polymorph_monster(mon))
            {
                /* Obvious */
                if (seen) obvious = TRUE;
                /* Why, in the name of all sanity, would you do this??!
                 * dam = 0;*/

                /* Update names */
                if (flags & GF_AFFECT_SPELL)
                {
                    monster_desc(m_name, mon, 0);
                    monster_desc(m_name_object, mon, 0);
                }
                else
                {
                    monster_desc(m_name, mon, MD_PRON_VISIBLE);
                    monster_desc(m_name_object, mon, MD_PRON_VISIBLE | MD_OBJECTIVE);
                }
                /* Get the monster possessive ("his"/"her"/"its") */
                monster_desc(m_poss, mon, MD_PRON_VISIBLE | MD_POSSESSIVE);
            }
            else
            {
                /* No polymorph */
                note = " is unaffected!";
            }

            /* Hack -- Get new monster */
            mon = &m_list[mon->id];

            /* Hack -- Get new race */
            race = &r_info[mon->r_idx];
        }

        /* Handle "teleport" ... double check the Guardian. It should be
         * handled above for nicer messaging, but I didn't get all the cases. */
        if (do_dist && !(mon->smart & (1U << SM_GUARDIAN)))
        {
            /* Obvious */
            if (seen) obvious = TRUE;

            /* Message */
            note = " disappears!";
            if (!who) virtue_add(VIRTUE_VALOUR, -1);

            /* Teleport */
            teleport_away(mon->id, do_dist,
                        (!who ? TELEPORT_DEC_VALOUR : 0L) | TELEPORT_PASSIVE);
            where = point(mon->fx, mon->fy);
        }

        /* Fear */
        if (do_fear)
        {
            /* Set fear */
            (void)set_monster_monfear(mon->id, MON_MONFEAR(mon) + do_fear);

            /* Get angry */
            get_angry = TRUE;
        }
    }

    if (type == GF_DRAIN_MANA)
    {
        /* Drain mana does nothing */
    }
    /* If another monster did the damage, hurt the monster by hand */
    else if (who)
    {
        /* Redraw (later) if needed */
        check_mon_health_redraw(mon->id);

        /* Wake the monster up */
        (void)set_monster_csleep(mon->id, 0);

        if ((melee_challenge) && (!is_pet(mon))) dam = 0;

        /* Hurt the monster */
        mon->hp -= dam;

        /* Dead monster */
        if (mon->hp < 0)
        {
            bool sad = FALSE;

            if (is_pet(mon) && !(mon->ml))
                sad = TRUE;

            /* Give detailed messages if destroyed */
            if (known && note && seen_msg)
            {
                monster_desc(m_name, mon, MD_TRUE_NAME);
                msg_format("%^s%s", m_name, note);
            }

            if (who > 0) monster_gain_exp(who, mon->id);

            pack_on_slay_monster(mon->id);

            mon_check_kill_unique(mon->id);

            /* Generate treasure, etc */
            monster_death(mon->id, who_is_pet);

            /* Delete the monster */
            delete_monster_idx(mon->id);

            if (sad)
            {
                msg_print("You feel sad for a moment.");
            }
        }

        /* Damaged monster */
        else
        {
            /* Give detailed messages if visible or destroyed */
            if (note && seen_msg) msg_format("%^s%s", m_name, note);

            /* Hack -- Pain message */
            else
            {
                mon_fight = TRUE;
            }

            /* Hack -- handle sleep */
            if (do_sleep) (void)set_monster_csleep(mon->id, do_sleep);
            if (do_paralyzed && !MON_PARALYZED(mon))
                set_monster_paralyzed(mon->id, do_paralyzed);
        }
    }
    else if (heal_leper)
    {
        if (seen_msg) msg_print("The Mangy looking leper is healed!");
        delete_monster_idx(mon->id);
    }
    /* If the player did it, give him experience, check fear */
    else
    {
        bool fear = FALSE;

        /* Hacks  ... these effects probably belong in the gargantuan switch statement above ... sigh */
        /* Hack: The Draining Blast power gives hitpoints back. */
        if (type == GF_ELDRITCH_DRAIN && monster_living(race) && !no_harm)
        {
            int heal = dam;
            if (heal > mon->hp)
                heal = mon->hp;
            heal /= 2;
            hp_player(heal);
        }

        /* Hack for device lore: Keep track of max damage experienced by any single
         * monster for this effect. */
        if (dam)
        {
            int lore_dam = MIN(dam, mon->hp);
            hack_max_m_dam = MAX(hack_max_m_dam, lore_dam);
        }

        /* Hurt the monster, check for fear and death
              v---- Revert 525c2ace: Warlocks: Playtesting Dragon Pact. Massive problems with project()
                    The problem here is that many attacks, like Slow Monster, do no physical damage, but
                    rely on mon_take_hit to do other things, like wake up sleeping monsters (or reveal
                    player ring mimics). This code needs refactoring ...*/
        if (/*dam &&*/ mon_take_hit(mon->id, dam, ((no_harm) || (!melee_challenge)) ? DAM_TYPE_SPELL : DAM_TYPE_MELEE, &fear, note_dies))
        {
            /* Dead monster */
        }

        /* Damaged monster */
        else
        {
            if (note == note_dies) /* Hack around crap code design ... Above we assumed monster would die but alas, we were wrong! */
                note = NULL;

            if ((note) && (no_harm)) /* Hack */
            {
                if (streq(" resists.", note)) note = NULL;
                else if (streq(" shudders.", note)) note = NULL;
                else if (streq(" is hit hard.", note)) note = NULL;
            }

            /* HACK - anger the monster before showing the sleep message */
            if (do_sleep) anger_monster(mon);

            /* HACK - tick off smart monsters whenever damaged by the player
               from a distance. */
            if (who == 0 && dam > 0 && mon->cdis > 1)
            {
                bool splashed = !projectable(py, px, mon->fy, mon->fx);
                if (allow_ticked_off(race))
                {
                    if (!mut_present(MUT_SUBTLE_CASTING))
                        mon_anger_spell(mon, dam);
                    if (splashed)
                        mon_anger(mon);
                    /* Attempt to deal with Dungeon Guardians splash exploit.
                       Dungeon guardians use AI_GUARD_POS, so cannot be lured
                       away from the dungeon entrance. Attempting this exploit
                       makes them really mad, and if they are mad enough, then
                       they will actually pursue the player (cf get_moves in melee2.c) */
                    if (splashed && mon->cdis > MAX_RANGE)
                    {
                        pack_info_t *pack_ptr = pack_info_ptr(mon->id);
                        if (pack_ptr && pack_ptr->ai == AI_GUARD_POS)
                            mon->anger = 100;
                        else
                            mon_anger(mon);
                    }
                }
                /* Splashing Uniques out of LOS makes them rethink their approach */
                if (splashed && (race->flags1 & RF1_UNIQUE))
                {
                    pack_info_t *pack_ptr = pack_info_ptr(mon->id);

                    if (pack_ptr)
                    {
                        int odds = MAX(1, 7 - mon->anger/10);

                        if (!allow_ticked_off(race)) /* Paranoia ... These should already be seeking! */
                            odds = 1;

                        switch (pack_ptr->ai)
                        {
                        case AI_MAINTAIN_DISTANCE:
                            if (one_in_(odds))
                                pack_ptr->ai = AI_SEEK;
                            else
                                pack_ptr->distance += 2;
                            break;

                        case AI_SHOOT:
                        case AI_LURE:
                            if (one_in_(odds))
                                pack_ptr->ai = AI_SEEK;
                            break;
                        }
                    }
                }
            }

            /* Give detailed messages if visible or destroyed */
            if (seen_msg)
            {
                if (note)
                    msg_format("%^s%s", m_name, note);

                /* Hack -- Pain message */
                else if (known && dam && (flags & GF_AFFECT_SPELL))
                {
                    message_pain(mon->id, dam);
                }
            }

            /* Anger monsters */
            if (((dam > 0) || get_angry) && !do_sleep)
                anger_monster(mon);

            /* Take note */
            if ((fear || do_fear) && seen)
            {
                /* Sound */
                sound(SOUND_FLEE);

                /* Message */
                if (seen_msg)
                    msg_format("%^s flees in terror!", m_name);
            }

            /* Hack -- handle sleep */
            if (do_sleep) (void)set_monster_csleep(mon->id, do_sleep);
            if (do_paralyzed && !MON_PARALYZED(mon))
                set_monster_paralyzed(mon->id, do_paralyzed);
        }
    }

    if ((type == GF_BLOOD_CURSE) && one_in_(4))
    {
        int curse_flg = (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
        int count = 0;
        do
        {
            switch (randint1(28))
            {
            case 1: case 2:
                if (!count)
                {
                    msg_print("The ground trembles...");

                    earthquake(ty, tx, 4 + randint0(4));
                    if (!one_in_(6)) break;
                }
            case 3: case 4: case 5: case 6: case 7: case 8:
                if (!count)
                {
                    int dam = damroll(10, 10);
                    msg_print("A portal opens to a plane of raw mana!");

                    project(0, 8, ty,tx, dam, GF_MANA, curse_flg);
                    if (!one_in_(6)) break;
                }
            case 9: case 10: case 11:
                if (!count)
                {
                    msg_print("Space warps about it!");

                    if (mon->r_idx) teleport_away(mon->id, damroll(10, 10), TELEPORT_PASSIVE);
                    if (one_in_(13)) count += activate_hi_summon(ty, tx, TRUE);
                    if (!one_in_(6)) break;
                }
            case 12: case 13: case 14: case 15: case 16:
                msg_print("It feels a surge of energy!");

                project(0, 7, ty, tx, 50, GF_DISINTEGRATE, curse_flg);
                if (!one_in_(6)) break;
            case 17: case 18: case 19:
                aggravate_monsters(0);
                if (!one_in_(6)) break;
            case 20: case 21:
                count += activate_hi_summon(ty, tx, TRUE);
                if (!one_in_(6)) break;
            case 22: case 23: case 24: case 25: case 26:
            {
                bool pet = !one_in_(3);
                u32b mode = PM_ALLOW_GROUP;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= (PM_NO_PET | PM_FORCE_FRIENDLY);

                count += summon_specific((pet ? -1 : 0), py, px, (pet ? p_ptr->lev*2/3+randint1(p_ptr->lev/2) : dun_level), 0, mode);
                if (!one_in_(6)) break;
            }
            case 27:
                if (p_ptr->hold_life && (randint0(100) < 75)) break;
                msg_print("You feel your life draining away...");

                if (p_ptr->hold_life) lose_exp(p_ptr->exp / 160);
                else lose_exp(p_ptr->exp / 16);
                if (!one_in_(6)) break;
            case 28:
            {
                int i = 0;
                if (one_in_(13))
                {
                    while (i < 6)
                    {
                        do
                        {
                            (void)do_dec_stat(i);
                        }
                        while (one_in_(2));

                        i++;
                    }
                }
                else
                {
                    (void)do_dec_stat(randint0(6));
                }
                break;
            }
            }
        }
        while (one_in_(5));
    }

    if (p_ptr->inside_battle)
    {
        p_ptr->health_who = mon->id;
        p_ptr->redraw |= PR_HEALTH_BARS;
        redraw_stuff();
    }

    /* XXX XXX XXX Verify this code */

    /* Update the monster */
    if (mon->r_idx) update_mon(mon->id, FALSE);

    /* Redraw the monster grid */
    lite_spot(where.y, where.x);


    /* Update monster recall window */
    if ((p_ptr->monster_race_idx == mon->r_idx) && (seen || !mon->r_idx))
    {
        /* Window stuff */
        p_ptr->window |= (PW_MONSTER);
    }

    if ((dam > 0) && !is_pet(mon) && !is_friendly(mon))
    {
        if (who == GF_WHO_PLAYER)
        {
            assert(GF_WHO_PLAYER == PROJECT_WHO_PLAYER);
            /* FYI: monster_target_* is generally the player, but
             * it gets changed for various mirror master effects. I
             * suppose this is a subtle touch to confuse monsters
             * but I never noticed this when I played one ... */
            set_target(mon, monster_target_y, monster_target_x);
        }
        else if ((who > 0) && (!is_hostile(caster_ptr)) && (!player_bold(mon->target_y, mon->target_x)) && (mon->cdis > 1))
        {
            set_target(mon, caster_ptr->fy, caster_ptr->fx);
        }
    }

    if (p_ptr->riding && (p_ptr->riding == mon->id) && (dam > 0))
    {
        if (mon->hp > mon->maxhp/3) dam = (dam + 1) / 2;
        rakubadam_m = (dam > 200) ? 200 : dam;
    }

    if (photo)
    {
        object_type *q_ptr;
        object_type forge;

        /* Get local object */
        q_ptr = &forge;

        /* Prepare to make a photograph */
        object_prep(q_ptr, lookup_kind(TV_STATUE, SV_PHOTO));

        q_ptr->pval = photo;
        object_origins(q_ptr, ORIGIN_PHOTO);

        /* Mark the item as fully known */
        q_ptr->ident |= (IDENT_KNOWN);

        /* Drop it in the dungeon */
        pack_carry(q_ptr);
        obj_release(q_ptr, OBJ_RELEASE_QUIET);
    }

    /* Return "Anything seen?" */
    return (obvious);
}

