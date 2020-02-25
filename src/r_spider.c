#include "angband.h"

#define MON_ARANEA       277 /* TODO: This is actually a Mirkwood spider ... */
#define MON_ELDER_ARANEA 809 /* TODO: This is actually Atlach-Nacha ... */

/**********************************************************************
 * Spider: Cave Spider -> Giant Spider -> Phase Spider 
 *          "     "    ->  "      "    -> Aranea       -> Elder Aranea
 **********************************************************************/

static cptr _desc = 
    "Spiders are stealthy monsters that prefer to wait for their prey. "
    "They have poisonous attacks and can weave sticky webs to entangle "
    "their enemies. They have eight legs, enough for four rings and "
    "two pairs of boots; they may also wear a helmet, an amulet, a cloak and "
    "even a suit of body armor (after slight adjustments, of course), but "
    "cannot wield a weapon or a bow. They also cannot equip light sources, "
    "but this is not an issue, as spiders are accustomed to the dark.\n \n"
    "There are two paths of evolution for the spider: the Phase Spider and "
    "the Aranea. Araneas have greater fortitude and better stats, as well as "
    "powerful melee capable of paralyzing an opponent; but Phase Spiders "
    "compensate for their relative fragility with unsurpassed powers of teleportation.\n \n"
    "All spiders begin in the very weak form of the Cave Spider. Lacking "
    "any special talents, the cave spider must play carefully if it is to "
    "survive.";

static void _detect_prey_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Prey");
        break;
    case SPELL_DESC:
        var_set_string(res, "Looks for a nearby meal.");
        break;
    case SPELL_CAST:
        detect_monsters_living(DETECT_RAD_DEFAULT, "You sense potential prey!");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _place_web(int y, int x)
{
    if (!in_bounds(y, x))
        return FALSE;
    if (!cave_clean_bold(y,x) || cave[y][x].m_idx)
        return FALSE;
    cave_set_feat(y, x, feat_web);
    return TRUE;
}

static int _web_attempts(void)
{
    return 8 * (p_ptr->lev - 25) / 25;
}

void spider_web_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spider Web");
        break;
    case SPELL_DESC:
        var_set_string(res, "Weaves a fine, silky web which obstructs the movement of all save spiders.");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 50)
            project(0, 1, py, px, 0, GF_WEB, PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE);
        else
        {
            int attempts = 0;
            int max_attempts = _web_attempts();
            int x, y, dir;

            _place_web(py, px);

            for (;;)
            {
                if (attempts >= max_attempts)
                    break;

                dir = randint0(9);
                if (dir == 5) continue;

                attempts++;
                y = py + ddy[dir];
                x = px + ddx[dir];
                _place_web(y, x);
            }
        }
        p_ptr->update |= PU_FLOW;
        p_ptr->redraw |= PR_MAP;
        var_set_bool(res, TRUE);
        break;
    case SPELL_COST_EXTRA:
        if (p_ptr->lev >= 50)
            var_set_int(res, 20);
        else
            var_set_int(res, _web_attempts() * 2);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/* Cave Spider */
static power_info _cave_spider_powers[] = {
    { A_DEX, {  1,  1, 30, _detect_prey_spell } },
    {    -1, { -1, -1, -1, NULL } }
};
static int _cave_spider_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _cave_spider_powers);
}
static void _cave_spider_calc_innate_attacks(void)
{
    innate_attack_t    a = {0};

    a.dd = 1;
    a.ds = 5;
    a.weight = 70;
    calc_innate_blows(&a, 200);
    a.msg = "You bite.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}
static void _cave_spider_calc_bonuses(void)
{
    p_ptr->pspeed += 3;
    res_add(RES_POIS);
    res_add(RES_DARK);
    res_add_vuln(RES_LITE);
    p_ptr->see_nocto = TRUE;
}
static void _cave_spider_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_VULN_LITE);
}
race_t *_cave_spider_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  40,  38,   6,  20,  15,  56,  30};
    skills_t xs = {  8,  15,  10,   0,   0,   0,  18,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Cave Spider";

        me.stats[A_STR] = -2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -3;
        
        me.life = 70;
        me.infra = 5;

        me.get_powers = _cave_spider_get_powers;
        me.calc_bonuses = _cave_spider_calc_bonuses;
        me.get_flags = _cave_spider_get_flags;
        me.calc_innate_attacks = _cave_spider_calc_innate_attacks;
        init = TRUE;
    }
    return &me;
}

/* Giant Spider */
static void _giant_spider_calc_innate_attacks(void)
{
    innate_attack_t    a = {0};

    a.dd = 1;
    a.ds = 10;
    a.to_d = p_ptr->lev / 5;
    a.weight = 70;
    a.effect[0] = GF_MISSILE;
    a.effect[1] = GF_POIS;
    calc_innate_blows(&a, 400);
    a.msg = "You bite.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

static void _giant_spider_calc_bonuses(void)
{
    res_add(RES_POIS);
    p_ptr->see_nocto = TRUE;
}
static void _giant_spider_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
}
race_t *_giant_spider_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  40,  38,   7,  20,  15,  56,  30};
    skills_t xs = {  8,  15,  10,   0,   0,   0,  18,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Giant Spider";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -1;
        
        me.life = 100;
        me.infra = 5;

        me.get_powers = _cave_spider_get_powers;
        me.calc_bonuses = _giant_spider_calc_bonuses;
        me.get_flags = _giant_spider_get_flags;
        me.calc_innate_attacks = _giant_spider_calc_innate_attacks;
        init = TRUE;
    }
    return &me;
}

/* Phase Spider */
static void _phase_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Phase Shield");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleport as you receive an attack, potentially escaping damage altogether.");
        break;
    case SPELL_CAST:
        if (!(p_ptr->special_defense & NINJA_KAWARIMI))
        {
            msg_print("You prepare to evade any attacks.");
            p_ptr->special_defense |= NINJA_KAWARIMI;
            p_ptr->redraw |= PR_STATUS;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _phase_spider_powers[] = {
    { A_DEX, {  1,  1, 30, _detect_prey_spell } },
    { A_DEX, {  5,  1, 30, phase_door_spell } },
    { A_DEX, { 10,  5, 30, teleport_spell } },
    { A_DEX, { 12, 10, 60, spider_web_spell } },
    { A_DEX, { 25, 10, 30, teleport_to_spell } },
    { A_DEX, { 30, 15, 50, teleport_level_spell } },
    { A_DEX, { 35, 20, 55, teleport_other_spell } },
    { A_DEX, { 40, 25, 60, _phase_shield_spell } },
    { A_DEX, { 50, 30, 60, dimension_door_spell } },
    {    -1, { -1, -1, -1, NULL } }
};
static int _phase_spider_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _phase_spider_powers);
}
static void _phase_spider_calc_innate_attacks(void)
{
    innate_attack_t    a = {0};

    a.dd = 2 + p_ptr->lev / 15;
    a.ds = 5 + p_ptr->lev / 16;
    a.to_h = p_ptr->lev/5;
    a.weight = 70;
    a.effect[0] = GF_MISSILE;
    a.effect[1] = GF_POIS;
    a.effect[2] = GF_OLD_SLEEP;
    calc_innate_blows(&a, 500);
    a.msg = "You bite.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

static void _phase_spider_calc_bonuses(void)
{
    p_ptr->pspeed += 5 + (p_ptr->lev - 25)/5;
    if (p_ptr->lev >= 50)
        res_add_immune(RES_POIS);
    else
    {
        res_add(RES_POIS);
        res_add(RES_POIS);
    }
    res_add(RES_NEXUS);
    res_add(RES_CONF);
    res_add_immune(RES_TELEPORT);
    p_ptr->free_act++;
    p_ptr->see_nocto = TRUE;
}
static void _phase_spider_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_RES_NEXUS);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_FREE_ACT);

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_POIS);
    else
        add_flag(flgs, OF_RES_POIS);
}
race_t *_phase_spider_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  40,  38,  10,  25,  15,  56,  30};
    skills_t xs = {  8,  15,  10,   0,   0,   0,  18,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Phase Spider";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -1;
        
        me.life =  90;
        me.infra =  5;

        me.get_powers = _phase_spider_get_powers;
        me.calc_bonuses = _phase_spider_calc_bonuses;
        me.get_flags = _phase_spider_get_flags;
        me.calc_innate_attacks = _phase_spider_calc_innate_attacks;
        init = TRUE;
    }
    return &me;
}

/* Aranea */
static power_info _aranea_powers[] = {
    { A_DEX, {  1,  1, 30, _detect_prey_spell } },
    { A_DEX, { 12, 10, 60, spider_web_spell } },
    {    -1, { -1, -1, -1, NULL } }
};
static int _aranea_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _aranea_powers);
}
static void _aranea_calc_innate_attacks(void)
{
    innate_attack_t    a = {0};

    a.dd = 2 + p_ptr->lev / 14;
    a.ds = 7 + p_ptr->lev / 16;
    a.to_h = p_ptr->lev/3;
    a.to_d = p_ptr->lev/5;
    a.weight = 70;
    a.effect[0] = GF_MISSILE;
    a.effect[1] = GF_POIS;
    a.effect[2] = GF_OLD_SLEEP;
    calc_innate_blows(&a, 500);
    a.msg = "You bite.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

static void _aranea_calc_bonuses(void)
{
    p_ptr->pspeed += 5;
    if (p_ptr->lev >= 50)
        res_add_immune(RES_POIS);
    else
    {
        res_add(RES_POIS);
        res_add(RES_POIS);
    }
    res_add(RES_DARK);
    res_add(RES_CONF);
    res_add(RES_FEAR);

    p_ptr->free_act++;
    p_ptr->see_nocto = TRUE;
}
static void _aranea_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_FREE_ACT);

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_POIS);
    else
        add_flag(flgs, OF_RES_POIS);
}
race_t *_aranea_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  30,  35,   6,  20,  15,  65,  30};
    skills_t xs = {  8,  10,  10,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Aranea";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;

        me.life = 100;
        me.infra =  5;

        me.get_powers = _aranea_get_powers;
        me.calc_bonuses = _aranea_calc_bonuses;
        me.get_flags = _aranea_get_flags;
        me.calc_innate_attacks = _aranea_calc_innate_attacks;
        init = TRUE;
    }
    return &me;
}

/* Elder Aranea */
static void _elder_aranea_calc_innate_attacks(void)
{
    innate_attack_t    a = {0};

    a.dd = 2 + p_ptr->lev / 14;
    a.ds = 7 + p_ptr->lev / 16;
    a.to_h = p_ptr->lev/2;
    a.to_d = p_ptr->lev/5;
    a.weight = 200;
    a.effect[0] = GF_MISSILE;
    a.effect[1] = GF_POIS;
    a.effect[2] = GF_PARALYSIS;
    a.effect_chance[2] = 25 + 5*(p_ptr->lev - 40);
    calc_innate_blows(&a, 550);
    a.msg = "You bite.";
    a.name = "Bite";
    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

static void _elder_aranea_calc_bonuses(void)
{
    p_ptr->pspeed += 10;

    if (p_ptr->lev >= 50)
        res_add_immune(RES_POIS);
    else
    {
        res_add(RES_POIS);
        res_add(RES_POIS);
    }

    res_add(RES_FIRE);
    res_add(RES_NETHER);
    res_add(RES_DISEN);
    res_add(RES_DARK);
    res_add(RES_CONF);
    res_add(RES_FEAR);

    res_add_vuln(RES_LITE);

    p_ptr->free_act++;
    p_ptr->see_inv++;
    p_ptr->regen += 100;
    p_ptr->see_nocto = TRUE;
}
static void _elder_aranea_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_RES_DISEN);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_RES_FEAR);

    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_REGEN);

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_POIS);
    else
        add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_VULN_LITE);
}
race_t *_elder_aranea_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  30,  35,   6,  20,  15,  65,  30};
    skills_t xs = {  8,  10,  10,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.subname = "Elder Aranea";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  1;
        
        me.life = 100;
        me.infra =  5;

        me.get_powers = _aranea_get_powers;
        me.calc_bonuses = _elder_aranea_calc_bonuses;
        me.get_flags = _elder_aranea_get_flags;
        me.calc_innate_attacks = _elder_aranea_calc_innate_attacks;
        init = TRUE;
    }
    return &me;
}

static void _gain_level(int new_level)
{
    if ( p_ptr->current_r_idx == MON_CAVE_SPIDER
      && new_level >= 10 )
    {
        p_ptr->current_r_idx = MON_GIANT_SPIDER;
        msg_print("You have evolved into a Giant Spider.");
        p_ptr->redraw |= PR_MAP;
    }
    else if ( p_ptr->current_r_idx == MON_GIANT_SPIDER
           && new_level >= 25 )
    {
        if (p_ptr->psubrace == SPIDER_PHASE)
        {
            p_ptr->current_r_idx = MON_PHASE_SPIDER;
            msg_print("You have evolved into a Phase Spider.");
            p_ptr->redraw |= PR_MAP;
        }
        else
        {
            p_ptr->current_r_idx = MON_ARANEA;
            msg_print("You have evolved into an Aranea.");
            p_ptr->redraw |= PR_MAP;
        }
    }
    else if ( p_ptr->current_r_idx == MON_ARANEA
           && new_level >= 40 )
    {
        p_ptr->current_r_idx = MON_ELDER_ARANEA;
        msg_print("You have evolved into an Elder Aranea.");
        p_ptr->redraw |= PR_MAP;
    }
}

static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_CAVE_SPIDER;
    equip_on_change_race();
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_h = 7;
    forge.to_d = 2;
    forge.pval = 1;
    add_flag(forge.flags, OF_DEX);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    py_birth_food();
}

static name_desc_t _info[SPIDER_MAX] = {
    { "Phase Spider", "Phase Spiders have unsurpassed powers of teleportation and average offense." },
    { "Aranea", "Aranea are stronger in melee than Phase Spiders, but lack special powers except for their Spider Web." },
};

race_t *mon_spider_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (p_ptr->current_r_idx)
    {
    case MON_CAVE_SPIDER:
        result = _cave_spider_get_race_t();
        break;
    case MON_GIANT_SPIDER:
        result = _giant_spider_get_race_t();
        break;
    case MON_PHASE_SPIDER:
        result = _phase_spider_get_race_t();
        break;
    case MON_ARANEA:
        result = _aranea_get_race_t();
        break;
    case MON_ELDER_ARANEA:
        result = _elder_aranea_get_race_t();
        break;
    default: /* Birth and High Scores */
        result = (spoiler_hack ? _aranea_get_race_t() : _cave_spider_get_race_t());
    }

    result->name = "Spider";
    result->desc = _desc;
    result->exp = 200;
    result->equip_template = mon_get_equip_template();
    result->flags = RACE_IS_MONSTER;
    result->gain_level = _gain_level;
    result->birth = _birth;
    result->base_hp = 25;
    result->pseudo_class_idx = CLASS_ROGUE;
    result->shop_adjust = 115;

    result->boss_r_idx = MON_UNGOLIANT;

    if (birth_hack || spoiler_hack)
    {
        result->subname = _info[psubrace].name;
        result->subdesc = _info[psubrace].desc;
    }

    return result;
}
