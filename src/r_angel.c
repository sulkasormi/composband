#include "angband.h"

static cptr _desc =
    "Angels are heavenly beings on a holy mission. Their wings allow them to "
    "fly over chasms and pits. They are rarely fooled by invisibility, and "
    "become resistant to the elements as they mature. Their stats are truly awe-inspiring, "
    "and their divine magic extremely strong, offering good offense combined with detection, "
    "healing, melee enhancement and utility. Angels are one of the most powerful "
    "races, but they also gain levels extremely slowly.\n \n"
    "Angels have a humanoid body type; they use the same equipment slots as most normal "
    "player races, and have no innate melee attacks. Wisdom is their primary spell stat.";

static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_ANGEL;

    object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_HEALING));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HAFTED, SV_MACE));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/******************************************************************************
 *              10           20        30        40        45          50
 * Angel: Angel -> Archangel -> Cherub -> Seraph -> Archon -> Planetar -> Solar
 ******************************************************************************/
static void _psycho_spear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Psycho-Spear");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of pure energy which penetrate the invulnerability barrier.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 3), spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_PSY_SPEAR, dir, spell_power(randint1(p_ptr->lev*3) + p_ptr->lev*3 + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _spells[] = {
    {  1,  1, 30, punishment_spell},
    {  2,  2, 30, bless_spell},
    {  3,  2, 30, confuse_spell},
    {  5,  3, 30, scare_spell},
    {  7,  4, 30, detect_monsters_spell},
    { 10,  7, 40, heroism_spell},
    { 15, 10, 40, magic_mapping_spell},
    { 20, 15, 50, mana_bolt_I_spell},
    { 23, 15, 50, brain_smash_spell},
    { 25, 17, 50, haste_self_spell},
    { 27, 20, 50, protection_from_evil_spell},
    { 29, 20, 60, dispel_evil_spell},
    { 31, 20, 60, teleport_other_spell},
    { 32, 20, 60, healing_I_spell},
    { 35, 25, 60, destruction_spell},
    { 37, 25, 60, summon_monsters_spell},
    { 40, 30, 65, _psycho_spear_spell},
    { 42, 30, 65, restoration_spell},
    { 45, 80, 65, clairvoyance_spell},
    { 47, 60, 70, summon_angel_spell},
    { 49, 90, 70, summon_hi_dragon_spell},
    { 50, 50, 65, starburst_II_spell},
    { -1, -1, -1, NULL}
};
static int _get_spells(spell_info* spells, int max) {
    return get_spells_aux(spells, max, _spells);
}

static void _calc_bonuses(void) {
    /* cf calc_torch in xtra1.c for the 'extra light' */

    p_ptr->align += 200;
    p_ptr->levitation = TRUE;
    res_add(RES_POIS);

    if (p_ptr->lev >= 10)
    {
        res_add(RES_FIRE);
        p_ptr->see_inv++;
    }
    if (p_ptr->lev >= 20)
    {
        p_ptr->pspeed += 1;
        res_add(RES_COLD);
    }
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 1;
        res_add(RES_ACID);
        res_add(RES_ELEC);
        res_add(RES_CONF);
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 3;
        p_ptr->reflect = TRUE;
        res_add(RES_TELEPORT);
        res_add(RES_LITE);
    }
    if (p_ptr->lev >= 45)
    {
        p_ptr->pspeed += 1;
    }
    if (p_ptr->lev >= 50)
    {
        p_ptr->pspeed += 1;
    }
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_RES_POIS);

    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_SEE_INVIS);
    }
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_COLD);
    }
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_CONF);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_REFLECT);
        add_flag(flgs, OF_RES_LITE);
    }
    if (p_ptr->lev >= 45)
    {
    }
    if (p_ptr->lev >= 50)
    {
    }
}
static void _gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_ANGEL && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_ARCHANGEL;
        msg_print("You have evolved into an Archangel.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ARCHANGEL && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_CHERUB;
        msg_print("You have evolved into a Cherub.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_CHERUB && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_SERAPH;
        msg_print("You have evolved into a Seraph.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_SERAPH && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ARCHON;
        msg_print("You have evolved into an Archon.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ARCHON && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_PLANETAR;
        msg_print("You have evolved into a Planetar.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_PLANETAR && new_level >= 50)
    {
        p_ptr->current_r_idx = MON_SOLAR;
        msg_print("You have evolved into a Solar.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_solar_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[7] =  {"Angel", "Archangel", "Cherub", "Seraph", "Archon", "Planetar", "Solar"};
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;
    if (p_ptr->lev >= 45) rank++;
    if (p_ptr->lev >= 50) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  35,  40,   3,  18,  12,  48,  35};
    skills_t xs = {  7,  13,  15,   0,   0,   0,  18,  13};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 3;
        me.exp = 325; /* 14.6 Mxp */
        me.base_hp = 26;

        me.get_spells = _get_spells;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  1 + rank/2;
    me.stats[A_INT] =  1 + rank/2;
    me.stats[A_WIS] =  4 + rank/2;
    me.stats[A_DEX] =  1 + rank/2;
    me.stats[A_CON] =  1 + rank/2;
    me.stats[A_CHR] =  1 + rank;
    me.life = 90 + 3*rank;

    return &me;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "divine power";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_angel_get_race(void)
{
    race_t *result = NULL;

    switch (p_ptr->psubrace)
    {
    /* TODO: Fallen Angel ? */
    default: /* Birth Menus */
        result = _solar_get_race_t();
    }

    result->name = "Angel";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->birth = _birth;
    result->caster_info = _caster_info;
    result->pseudo_class_idx = CLASS_PRIEST;
    result->shop_adjust = 90;

    result->boss_r_idx = MON_RAPHAEL;

    if (birth_hack || spoiler_hack)
    {
        result->subname = NULL;
        result->subdesc = NULL;
    }

    return result;
}
