#include "angband.h"

static cptr _desc =
    "The creations of some differently normal scientist who thought sentient "
    "blood-thirsty pumpkins would be a great idea, these creatures begin their lives "
    "as pumpkin-headed humanoids; over time, however, the pumpkin bloats and crushes "
    "the frail stick body underneath, leaving just a massive rolling fruit and an "
    "unquenchable desire to kill. Death pumpkins lack most normal equipment slots, "
    "but make up for it by the many lights they can store within their body. "
    "Also, the bites of the deadliest pumpkins are said to be vampiric...";

#define PUMP_LEVEL 40

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static int _rank(void)
{
    int r = 0;
    if (p_ptr->lev >= PUMP_LEVEL) r++;
    return r;
}

static void _birth(void) 
{ 
    object_type forge;

    p_ptr->current_r_idx = MON_PUMPKIN_MAN;
    skills_innate_init("Squash", WEAPON_EXP_BEGINNER, WEAPON_EXP_SKILLED);
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_DAGGER));
    py_birth_obj(&forge);
    object_prep(&forge, lookup_kind(TV_CLOAK, SV_CLOAK));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

static void _calc_innate_attacks(void)
{
    if (p_ptr->current_r_idx != MON_DEATH_PUMPKIN) return;
    {
        innate_attack_t a = {0};

        a.dd = 1 + (p_ptr->lev / 5);
        a.ds = 7 + (p_ptr->lev / 15);
        a.weight = 250;
        a.to_h = 5 + (p_ptr->lev / 2);
        a.to_d = 10;

        a.effect[1] = GF_STUN;
        a.effect_chance[1] = 20;

        a.blows = 100;
        a.msg = "You squash.";
        a.name = "Squash";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    {
        innate_attack_t a = {0};

        a.dd = 2 + (p_ptr->lev / 15);
        a.ds = 7 + (p_ptr->lev / 10);
        a.weight = 100;
        a.to_h = (p_ptr->lev / 3);
        a.to_d = 5;

        if (p_ptr->lev >= 40) a.effect[1] = GF_OLD_DRAIN;
        calc_innate_blows(&a, 450);
        a.msg = "You bite.";
        a.name = "Bite";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_PUMPKIN_MAN && new_level >= PUMP_LEVEL)
    {
        p_ptr->current_r_idx = MON_DEATH_PUMPKIN;
        msg_print("You have evolved into a Death pumpkin.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
}

static void _calc_bonuses(void) 
{
    res_add(RES_LITE);
    res_add(RES_CONF);
    res_add(RES_BLIND);
    if (p_ptr->lev >= PUMP_LEVEL)
    {
        int to_a = 50 + py_prorata_level(50);
        res_add(RES_NETHER);
        res_add(RES_LITE);
        res_add(RES_BLIND);
        res_add(RES_FIRE);
        res_add(RES_ELEC);
        res_add(RES_FEAR);
        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
        p_ptr->free_act++;
        p_ptr->hold_life++;
        p_ptr->see_inv++;
        p_ptr->pspeed += (p_ptr->lev / 5);
    }
    else
    {
        res_add_vuln(RES_FIRE);
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_RES_BLIND);
    if (p_ptr->lev < PUMP_LEVEL) add_flag(flgs, OF_VULN_FIRE);
    else
    {
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_FEAR);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_SEE_INVIS);
        add_flag(flgs, OF_BRAND_VAMP);
    }
}

/**********************************************************************
 * Powers
 **********************************************************************/
void _pumpkin_breathe_light_spell(int cmd, variant *res)
{
    int dam = p_ptr->lev + (p_ptr->chp / 18);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes bright light at your chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You breathe light.");
        fire_ball(GF_LITE, dir, dam, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _get_powers[] =
{
    { A_INT, {  5,  2, 20, scare_monster_spell} },
    { A_CON, { PUMP_LEVEL, 15,  0, _pumpkin_breathe_light_spell} },
    {    -1, { -1, -1, -1, NULL} }
};

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_pumpkin_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  19,  32,   1,  15,  10,  60,  30};
    skills_t xs = {  5,   9,  12,   0,   0,   0,  27,  12};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 2;
        me.shop_adjust = 110;

        me.name = "Pumpkin";
        me.desc = _desc;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.birth = _birth;
        me.gain_level = _gain_level;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        me.base_hp = 15;
        me.boss_r_idx = MON_JACK_LANTERN;
        me.pseudo_class_idx = CLASS_CHAOS_WARRIOR;

        init = TRUE;

    }

    me.subname = _mon_name(p_ptr->current_r_idx);

    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -2;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] =  0 - (rank * 2);
    me.stats[A_CON] =  1 + rank;
    me.stats[A_CHR] = -2;

    me.flags = (rank > 0) ? RACE_IS_MONSTER : (RACE_IS_MONSTER | RACE_IS_NONLIVING);

    me.life = 102;
    me.exp = 115;
    me.equip_template = mon_get_equip_template();

    return &me;
}

