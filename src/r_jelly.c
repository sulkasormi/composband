#include "angband.h"

/**********************************************************************
 * Jelly: black ooze -> gelatinous cube -> acidic cytoplasm -> shoggoth
 **********************************************************************/

static cptr _desc = 
  "Jellies are acidic creatures, quietly patrolling their surroundings in search "
  "of objects to absorb. They are quite stealthy for most of their lives. They "
  "don't wear equipment on their bodies the way normal races do; instead, they "
  "are capable of absorbing objects into their gelatinous frames. In fact, each of their "
  "equipment slots (except their pseudopod) is capable of holding any type of "
  "equipment whatsoever, and the number of slots increases with experience. "
  "For attacking, jellies are capable of forming a single pseudopod which they "
  "may either use for grasping a normal melee weapon, or leave empty to use their "
  "innate acid-based attacks.\n \n"
  "Rather than relying on sight, jellies use a little-understood sense of their "
  "surroundings which allows them to see in the dark with neither light nor infravision. "
  "So while jellies can equip a torch or lantern, there is little reason for them to do so.\n \n"
  "Jellies feed off the objects they find, gaining nutrition in proportion to the "
  "weight of the object absorbed. They can eat rations of food if desired, but these "
  "are so light as to offer very little nutritional value. Jellies prefer to eat heavy "
  "armors and weapons. For convenience, destroyed objects are automatically eaten; "
  "and jellies never become bloated, their appetite being insatiable.\n \n"
  "Jellies begin life as a small puddle of black ooze, with very little carrying capacity for equipment. "
  "However, they quickly evolve into a Gelatinous Cube and then into an Acidic Cytoplasm. "
  "Jellies take one additional, powerful evolutionary step late in life.";

static void _divide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Divide");
        break;
    case SPELL_DESC:
        var_set_string(res, "Replicate yourself.");
        break;
    case SPELL_CAST:
    {
        summon_named_creature(-1, py, px, p_ptr->current_r_idx, PM_FORCE_PET);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _jelly_get_powers[] = {
    { A_CON, { 1, 10, 0, _divide_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _jelly_calc_innate_attacks(void)
{
    if (equip_is_empty_hand(0))
    {
        innate_attack_t    a = {0};
        int l = p_ptr->lev;

        a.dd = 2 + l / 10;
        a.ds = 6 + l / 12;
        a.to_h += l/2;
        a.to_d += l/5;
        a.to_d += l*l/250;
        a.weight = 100;
        a.effect[0] = GF_ACID;
        calc_innate_blows(&a, 600);
        a.msg = "You shoot acid.";
        a.name = "Pseudopod";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _black_ooze_calc_bonuses(void)
{
    res_add(RES_ACID);
    res_add(RES_POIS);
    res_add_immune(RES_BLIND);
    p_ptr->see_nocto = TRUE;
    p_ptr->no_stun = TRUE;
    p_ptr->no_cut = TRUE;
}
static void _black_ooze_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_ACID);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_IM_BLIND);
    add_flag(flgs, OF_NIGHT_VISION);
}
race_t *_black_ooze_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  37,   8,  14,   7,  70,  30};
    skills_t xs = { 12,   7,  11,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Black Ooze";

        me.stats[A_STR] =  1;
        me.stats[A_INT] = -5;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -2;

        me.life = 90;
        me.infra = 0;

        me.calc_bonuses = _black_ooze_calc_bonuses;
        me.get_flags = _black_ooze_get_flags;

        init = TRUE;
    }
    return &me;
}

static void _gelatinous_cube_calc_bonuses(void)
{
    p_ptr->to_a += p_ptr->lev/3;
    p_ptr->dis_to_a += p_ptr->lev/3;
    res_add(RES_ACID);
    res_add(RES_FIRE);
    res_add(RES_COLD);
    res_add(RES_ELEC);
    _black_ooze_calc_bonuses();
}
static void _gelatinous_cube_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_ACID);
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_ELEC);
    _black_ooze_get_flags(flgs);
}
race_t *_gelatinous_cube_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  37,   8,  14,   7,  70,  30};
    skills_t xs = { 12,   7,  11,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Gelatinous Cube";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -10;
        me.stats[A_WIS] = -10;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] = -2;
        
        me.life = 100;
        me.infra = 0;

        me.calc_bonuses = _gelatinous_cube_calc_bonuses;
        me.get_flags = _gelatinous_cube_get_flags;

        init = TRUE;
    }
    return &me;
}

static void _acidic_cytoplasm_calc_bonuses(void)
{
    p_ptr->pspeed += 3;
    p_ptr->free_act++;
    p_ptr->to_a += p_ptr->lev/6;
    p_ptr->dis_to_a += p_ptr->lev/6;
    res_add_immune(RES_ACID);
    res_add(RES_CONF);

    add_flag(p_ptr->weapon_info[0].flags, OF_BRAND_ACID);

    _gelatinous_cube_calc_bonuses();
}
static void _acidic_cytoplasm_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_BRAND_ACID);

    add_flag(flgs, OF_IM_ACID);

    _gelatinous_cube_get_flags(flgs);
}

race_t *_acidic_cytoplasm_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  37,   7,  14,   7,  70,  30};
    skills_t xs = { 12,   7,  11,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Acidic Cytoplasm";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -7;
        me.stats[A_WIS] = -7;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] = -1;
        
        me.life = 102;
        me.infra = 0;

        me.calc_bonuses = _acidic_cytoplasm_calc_bonuses;
        me.get_flags = _acidic_cytoplasm_get_flags;

        init = TRUE;
    }
    me.life = 82 + (p_ptr->lev / 2);
    return &me;
}

static void _shoggoth_calc_bonuses(void)
{
    p_ptr->pspeed += 2;
    p_ptr->regen += 100;
    p_ptr->to_a += p_ptr->lev/6;
    p_ptr->dis_to_a += p_ptr->lev/6;
    p_ptr->no_eldritch = TRUE;

    res_add(RES_TELEPORT);

    _acidic_cytoplasm_calc_bonuses();
}
static void _shoggoth_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_REGEN);
    _acidic_cytoplasm_get_flags(flgs);
}
race_t *_shoggoth_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  37,   2,  14,   7,  70,  30};
    skills_t xs = { 12,   7,  11,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Shoggoth";

        me.stats[A_STR] =  5;
        me.stats[A_INT] = -5;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] =  0;
        
        me.life = 103;
        me.infra = 0;

        me.calc_bonuses = _shoggoth_calc_bonuses;
        me.get_flags = _shoggoth_get_flags;

        init = TRUE;
    }
    return &me;
}

static void _gain_level(int new_level) 
{
    if ( p_ptr->current_r_idx == MON_BLACK_OOZE
      && new_level >= 14 )
    {
        p_ptr->current_r_idx = MON_GELATINOUS_CUBE;
        msg_print("You have evolved into a Gelatinous Cube.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    else if ( p_ptr->current_r_idx == MON_GELATINOUS_CUBE
           && new_level >= 28 )
    {
        p_ptr->current_r_idx = MON_ACIDIC_CYTOPLASM;
        msg_print("You have evolved into an Acidic Cytoplasm.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    else if ( p_ptr->current_r_idx == MON_ACIDIC_CYTOPLASM
           && new_level >= 42 )
    {
        p_ptr->current_r_idx = MON_SHOGGOTH;
        msg_print("You have evolved into a Shoggoth.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
}


static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_BLACK_OOZE;
    equip_on_change_race();
    skills_innate_init("Pseudopod", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.pval = 1;
    forge.to_d = 3;
    add_flag(forge.flags, OF_STR);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);
}

race_t *mon_jelly_get_race(void)
{
    race_t *result = NULL;

    switch (p_ptr->current_r_idx)
    {
    case MON_BLACK_OOZE:
        result = _black_ooze_get_race_t();
        break;
    case MON_GELATINOUS_CUBE:
        result = _gelatinous_cube_get_race_t();
        break;
    case MON_ACIDIC_CYTOPLASM:
        result = _acidic_cytoplasm_get_race_t();
        break;
    case MON_SHOGGOTH:
        result = _shoggoth_get_race_t();
        break;
    default: /* Birth code? Startup? Restart from Old Savefile? */
        result = (spoiler_hack ? _acidic_cytoplasm_get_race_t() : _black_ooze_get_race_t());
    }

    result->name = "Jelly";
    result->desc = _desc;
    result->exp = 150;
    result->flags = RACE_IS_MONSTER /* | RACE_IS_ILLITERATE */;
    result->gain_level = _gain_level;
    result->get_powers = _jelly_get_powers;
    result->calc_innate_attacks = _jelly_calc_innate_attacks;
    result->birth = _birth;
    result->base_hp = 40;
    result->equip_template = mon_get_equip_template();
    result->pseudo_class_idx = CLASS_WARRIOR;
    result->shop_adjust = 125;
    result->destroy_object = jelly_eat_object;

    result->boss_r_idx = MON_UBBO_SATHLA;

    if (birth_hack || spoiler_hack)
    {
        result->subname = NULL;
        result->subdesc = NULL;
    }

    return result;
}

bool jelly_eat_object(object_type *o_ptr)
{
    char o_name[MAX_NLEN];
    object_desc(o_name, o_ptr, OD_COLOR_CODED);
    set_food(MIN(PY_FOOD_FULL - 1, p_ptr->food + o_ptr->weight * 50 * o_ptr->number));
    msg_format("You assimilate %s into your gelatinous frame.", o_name);
    /* TODO: Consider giving timed benefits based on what is absorbed.
       For example, TR_RES_FIRE might give temp fire resistance and 
       TR_SPEED might give temporary haste.
    */
    return TRUE;
}

