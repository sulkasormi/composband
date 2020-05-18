#include "angband.h"

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static bool _summon_aux(int num, bool pet, int y, int x, int lev, int type, u32b mode)
{
    int plev = p_ptr->lev;
    int who;
    int i;
    bool success = FALSE;

    /*if (!lev) lev = spell_power(plev) + randint1(spell_power(plev * 2 / 3));*/
    if (!lev) lev = MAX(plev, dun_level);

    if (pet)
    {
        mode |= PM_FORCE_PET;
        if (mode & PM_ALLOW_UNIQUE)
        {
            if (randint1(50 + plev) >= plev / 10)
                mode &= ~PM_ALLOW_UNIQUE;
        }
        who = -1;
    }
    else
    {
        mode |= PM_NO_PET;
        who = 0;
    }

    for (i = 0; i < num; i++)
    {
        if (summon_specific(who, y, x, lev, type, mode))
            success = TRUE;
    }

    if (!success)
        msg_print("Nobody answers your call for help.");

    return success;
}

static void _summon(int what, int num, bool fail)
{
    int x = px;
    int y = py;

    if (fail) /* Failing spells should not be insta-death ... */
    {
        num = MAX(1, num/4);
        /* I'm debating this ... with -10 speed, Q's just die if they fail early on! */
        return;
    }
    else
        num = spell_power(num);

    if (!fail && old_target_okay() && los(py, px, target_row, target_col) && !one_in_(3))
    {
        y = target_row;
        x = target_col;
    }
    if (_summon_aux(num, !fail, y, x, 0, what, PM_ALLOW_UNIQUE | PM_ALLOW_GROUP))
    {
        if (fail)
        {
            if (num == 1)
                msg_print("The summoned monster gets angry!");
            else
                msg_print("The summoned monsters get angry!");
        }
    }
}

/**********************************************************************
 * Spells
 **********************************************************************/
void _heal_monster_spell(int cmd, variant *res)
{
    int heal = spell_power(p_ptr->lev * 10 + 200);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Heal Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to heal a chosen monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, heal));
        break;
    case SPELL_CAST:
    {
        int dir;
        bool result;
        bool old_target_pet = target_pet;

        var_set_bool(res, FALSE);

        target_pet = TRUE;
        result = get_fire_dir(&dir);
        target_pet = old_target_pet;

        if (!result) return;

        heal_monster(dir, heal);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ancient_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ancient Dragon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon an ancient dragon for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ants");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon ants for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ANT, randint1(2), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_balrog_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Balrog");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a balrog for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_BALROG, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_clubber_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Clubber Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a clubber demon for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_CLUBBER_DEMON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dark_elf_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Dark Elf");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon dark elves for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DARK_ELF, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a demon for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_demon_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Demon Summoner");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a demon summoner for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Dragon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a dragon for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dragon_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Dragon Summoner");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a dragon summoner for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DRAGON_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_elemental_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Elemental");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon an elemental for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ELEMENTAL, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_giant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Giant");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a giant for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_GIANT, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_golem_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Golem");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a golem for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_GOLEM, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Greater Dragons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon greater dragons for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_DRAGON, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Greater Demons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon greater demons for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Greater Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon greater undead for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_UNDEAD, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_hounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Hounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon hounds for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HOUND, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_lich_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Lich");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a lich for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_LICH, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_mature_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Mature Dragon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a mature dragon for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_MATURE_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_orc_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Orcs");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon orcs for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ORC, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_spider_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Spiders");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon spiders for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_SPIDER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ultimate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ultimate");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon the most powerful monsters for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ULTIMATE, randint1(2), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon an undead for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_UNDEAD, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_undead_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Undead Summoner");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon an undead summoner for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_UNDEAD_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_wight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Wight");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a wight for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_WIGHT, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_yeek_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Yeeks");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon yeeks for assistance.");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_YEEK, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _baby_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    {  1,  2, 35, _summon_yeek_spell},
    {  2,  6, 35, _summon_spider_spell},
    {  3,  7, 35, _summon_ant_spell},
    {  4,  9, 35, _summon_orc_spell},
    {  8, 10, 35, _summon_dark_elf_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 15, 15, 45, _summon_giant_spell},
    { 18, 20, 45, _summon_golem_spell},
    { 20, 20, 45, _summon_elemental_spell},
    { 23, 25, 45, _summon_hounds_spell},
    { 25, 35, 55, _heal_monster_spell},
    { -1, -1, -1, NULL}
};

static spell_info _rotting_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_lich_spell},
    { 35, 23, 55, _summon_wight_spell},
    { 38, 30, 55, _summon_undead_summoner_spell},
    { 42, 45, 65, _summon_high_undead_spell},
    { -1, -1, -1, NULL}
};
static spell_info _draconic_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_mature_dragon_spell},
    { 38, 30, 55, _summon_ancient_dragon_spell},
    { 38, 30, 55, _summon_dragon_summoner_spell},
    { 42, 45, 65, _summon_high_dragon_spell},
    { -1, -1, -1, NULL}
};
static spell_info _demonic_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 45, _summon_clubber_demon_spell},
    { 38, 30, 55, _summon_balrog_spell},
    { 38, 30, 55, _summon_demon_summoner_spell},
    { 42, 45, 65, _summon_high_demon_spell},
    { -1, -1, -1, NULL}
};

static spell_info _master_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_lich_spell},
    { 33, 20, 45, _summon_clubber_demon_spell},
    { 35, 23, 55, _summon_wight_spell},
    { 38, 30, 55, _summon_balrog_spell},
    { 38, 30, 55, _summon_demon_summoner_spell},
    { 38, 30, 55, _summon_dragon_summoner_spell},
    { 38, 30, 55, _summon_undead_summoner_spell},
    { 42, 45, 65, _summon_high_demon_spell},
    { 42, 45, 65, _summon_high_dragon_spell},
    { 42, 45, 65, _summon_high_undead_spell},
    { 50, 50, 75, _summon_ultimate_spell},
    { -1, -1, -1, NULL}
};

static spell_info *_get_spells(void) 
{
    switch (p_ptr->current_r_idx)
    {
    case MON_ROTTING_QUYLTHULG:
    case MON_GREATER_ROTTING_QUYLTHULG:
        return _rotting_spells;
    case MON_DRACONIC_QUYLTHULG:
    case MON_GREATER_DRACONIC_QUYLTHULG:
        return _draconic_spells;
    case MON_DEMONIC_QUYLTHULG:
    case MON_GREATER_DEMONIC_QUYLTHULG:
        return _demonic_spells;
    case MON_MASTER_QUYLTHULG:
        return _master_spells;
    }

    return _baby_spells;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "quivering power";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        init = TRUE;
    }
    return &me;
}


/**********************************************************************
 * Bonuses
 **********************************************************************/
static void _calc_bonuses(void) 
{
    p_ptr->pspeed -= 10;
    p_ptr->pspeed += p_ptr->lev / 10;

    res_add(RES_FEAR);
    res_add(RES_BLIND);
    res_add(RES_CONF);
    p_ptr->regen += 100;
    p_ptr->slow_digest = TRUE;
    p_ptr->telepathy = TRUE;
    p_ptr->free_act++;
    p_ptr->see_inv++;
    p_ptr->sustain_chr = TRUE;

    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG)
        res_add(RES_NEXUS);

    if ( p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
    {
        res_add(RES_NETHER);
        p_ptr->hold_life++;
    }

    if ( p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG )
    {
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ACID);
        res_add(RES_ELEC);
        res_add(RES_POIS);
    }

    if ( p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG )
    {
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add(RES_CHAOS);
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_DEC_SPEED);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_RES_BLIND);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_REGEN);
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_TELEPATHY);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_SUST_CHR);

    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG)
        add_flag(flgs, OF_RES_NEXUS);

    if ( p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_HOLD_LIFE);
    }

    if ( p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
    }

    if ( p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_CHAOS);
    }
}

/**********************************************************************
 * Birth and Evolution
 **********************************************************************/
static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_QUYLTHULG;
    equip_on_change_race();
    
    object_prep(&forge, lookup_kind(TV_CAPTURE, 0));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_WHISTLE, 1));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_QUYLTHULG && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_NEXUS_QUYLTHULG;
        msg_print("You have evolved into a Nexus Quylthulg.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG && new_level >= 30)
    {
        int which = randint1(3);
        if (spoiler_hack) which = 1;
        switch (which)
        {
        case 1:
            p_ptr->current_r_idx = MON_ROTTING_QUYLTHULG;
            msg_print("You have evolved into a Rotting Quylthulg.");
            break;
        case 2:
            p_ptr->current_r_idx = MON_DRACONIC_QUYLTHULG;
            msg_print("You have evolved into a Draconic Quylthulg.");
            break;
        case 3:
            p_ptr->current_r_idx = MON_DEMONIC_QUYLTHULG;
            msg_print("You have evolved into a Demonic Quylthulg.");
            break;
        }
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_ROTTING_QUYLTHULG;
        msg_print("You have evolved into a Greater Rotting Quylthulg.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_DRACONIC_QUYLTHULG;
        msg_print("You have evolved into a Greater Draconic Quylthulg.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_DEMONIC_QUYLTHULG;
        msg_print("You have evolved into a Greater Demonic Quylthulg.");
        p_ptr->redraw |= PR_MAP;
    }
    if ( ( p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG 
        || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG
        || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
      && new_level >= 50 )
    {
        p_ptr->current_r_idx = MON_MASTER_QUYLTHULG;
        msg_print("You have evolved into a Master Quylthulg.");
        p_ptr->redraw |= PR_MAP;
    }
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_quylthulg_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;
    if (p_ptr->lev >= 50) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  45,  40,   7,  20,  30,   0,   0 };
    skills_t xs = {  7,  15,  12,   0,   0,   0,   0,   0 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Quylthulg";
        me.desc = "Quylthulgs are disgusting, quivering mounds of pulsating flesh, scarcely "
                    "able to move about. Physically, they are pathetic. However, their ability "
                    "to control others to do their bidding is legendary, and they are quite "
                    "capable of fleeing when the going gets tough. Quylthulgs have no physical "
                    "attacks.";

        me.infra = 5;
        me.exp = 150;
        me.base_hp = 0;
        me.shop_adjust = 120;

        me.get_spells_fn = _get_spells;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;
        me.pseudo_class_idx = CLASS_SORCERER;

        me.flags = RACE_IS_MONSTER;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    me.stats[A_STR] = -5 + rank;
    me.stats[A_INT] = -5 + rank;
    me.stats[A_WIS] = -5 + rank;
    me.stats[A_DEX] = -5 + rank;
    me.stats[A_CON] = -5 + rank;
    me.stats[A_CHR] =  6 + rank;
    me.life = 95;
    me.boss_r_idx = MON_EMPEROR_QUYLTHULG;

    me.equip_template = mon_get_equip_template();
    return &me;
}
