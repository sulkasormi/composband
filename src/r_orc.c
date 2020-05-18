#include "angband.h"

static cptr _desc = 
    "Orcs are Morgoth's creation and the backbone of his armies. They are "
    "accustomed to caves and tunnels, and resist darkness but tend to be "
    "vulnerable to light. Orcs are ugly, brutish pillagers by reputation "
    "and should not expect much love from shopkeepers. Morgoth's spirit "
    "lives in orcs and drives them forward, making them hard to frighten. "
    "What orcs lack in stealth and subtlety they make up for with their sheer "
    "brute strength, and generally have little trouble brushing aside weak "
    "enemies like kobolds; yet in the deep dungeons, where the strongest "
    "fighters and most powerful magicians reside, orcs are rarely seen.";

static void _calc_bonuses(void) 
{
    res_add(RES_DARK);
    if (r_info[MON_MORGOTH].max_num) res_add(RES_FEAR);
    if (p_ptr->psubrace == ORC_FIGHTER)
    {
        p_ptr->skill_dig += 50;
        if (p_ptr->lev <= 20) res_add_vuln(RES_LITE);
    }
    else if (p_ptr->lev <= 40) res_add_vuln(RES_LITE);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_RES_DARK);
    if (r_info[MON_MORGOTH].max_num) add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->psubrace == ORC_FIGHTER)
    {
        add_flag(flgs, OF_TUNNEL);
        if (p_ptr->lev <= 20) add_flag(flgs, OF_VULN_LITE);
    }
    else if (p_ptr->lev <= 40) add_flag(flgs, OF_VULN_LITE);
}

static void _fighter_birth(void)
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_SWORD, SV_RAPIER));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SHIELD, SV_SMALL_LEATHER_SHIELD));
    forge.name2 = EGO_SHIELD_ORCISH;
    forge.to_h = 2;
    forge.to_d = 2;
    forge.to_a = 10;
    forge.pval = 1;
    add_flag(forge.flags, OF_VULN_LITE);
    add_flag(forge.flags, OF_RES_DARK);
    add_flag(forge.flags, OF_STR);
    add_flag(forge.flags, OF_DEC_INT);
    add_flag(forge.flags, OF_DEC_STEALTH);
    add_flag(forge.flags, OF_IGNORE_ACID);
    add_flag(forge.flags, OF_IGNORE_ELEC);
    add_flag(forge.flags, OF_IGNORE_FIRE);
    add_flag(forge.flags, OF_IGNORE_COLD);
    switch (randint0(4))
    {
        case 0: add_flag(forge.flags, OF_RES_COLD); break;
        case 1: add_flag(forge.flags, OF_RES_ELEC); break;
        case 2: add_flag(forge.flags, OF_RES_FIRE); break;
        default: add_flag(forge.flags, OF_RES_ACID); break;
    }
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_CURE_LIGHT));
    forge.number = 4 + randint0(4);
    py_birth_obj(&forge);

    py_birth_light();

    p_ptr->current_r_idx = MON_SNAGA;
}

static void _summon_orcs_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Orcs");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summons one or more orcs to your aid.");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (summon_kin_player(p_ptr->lev, py, px, (PM_FORCE_PET | PM_ALLOW_GROUP))) var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _fighter_get_powers[] =
{
    { A_STR, { 27, 15, 40, _summon_orcs_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

static void _fighter_gain_level(int new_level)
{
    if (p_ptr->current_r_idx == MON_SNAGA && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_CAVE_ORC;
        msg_print("You have evolved into a <color:G>Cave orc</color>.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_CAVE_ORC && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_URUK;
        msg_print("You have evolved into an Uruk.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_URUK && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ORC_WARLORD;
        msg_print("You have evolved into an <color:g>Orc warlord</color>.");
        p_ptr->redraw |= PR_MAP;
    }

}

static race_t *_fighter_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Snaga", "Cave orc", "Uruk", "Orc warlord"};
    int           rank = 0;

    if (p_ptr->max_plv >= 10) rank = MIN(3, p_ptr->max_plv / 10);

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  30,   2,  16,   4,  72,  54};
    skills_t xs = { 11,   6,  10,   0,   0,   0,  18,  18};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 3;
        me.exp = 100;
        me.life = 99;
        me.base_hp = 30;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _fighter_birth;
        me.get_powers = _fighter_get_powers;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _fighter_gain_level;
        me.pseudo_class_idx = CLASS_WARRIOR;
        me.subdesc = "The most common and familiar type of orc, fighter orcs "
                       "are feared around the world for their strength, endurance, "
                       "brutality and low cunning. They start their careers as a "
                       "lowly Snaga, but will eventually develop into Uruk-hai or "
                       "even mighty warlords, capable of summoning and controlling "
                       "armies of lesser orcs.";
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] = -1 + rank;
    me.stats[A_INT] = -4 + rank;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] = 0;
    me.stats[A_CON] = rank;
    me.stats[A_CHR] = -5;
    me.boss_r_idx = MON_OTHROD;
    if (birth_hack || spoiler_hack) me.subname = "Orc fighter";

    return &me;
}

static void _shaman_birth(void)
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_POLEARM, SV_SPEAR));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR));
    py_birth_obj(&forge);

    py_birth_light();

    p_ptr->current_r_idx = MON_ORC_SHAMAN;
}

void _orcish_curse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Orcish Curse");
        break;
    case SPELL_DESC:
        var_set_string(res, "Curses a single monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(3 + ((p_ptr->lev - 1) / 5)), 9 + (p_ptr->lev / 14), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dice = 3 + (p_ptr->lev - 1) / 5;
        int sides = 9 + (p_ptr->lev / 14);
        int dir = 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_CAUSE_1, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _evil_curse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Curse of Morgul");
        break;
    case SPELL_DESC:
        var_set_string(res, "Powerfully curses a single monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(((p_ptr->lev + 59) / 5)), 9 + (p_ptr->lev / 14), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dice = (p_ptr->lev + 59) / 5;
        int sides = 9 + (p_ptr->lev / 14);
        int dir = 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_CAUSE_2, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _sleep_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sleep Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to put a single monster to sleep.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 5 / 3));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        sleep_monster(dir, p_ptr->lev * 5 / 3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _confuse_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confuse Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to confuse a single monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 5 / 3));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        confuse_monster(dir, p_ptr->lev * 5 / 3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _shaman_get_spells[] =
{
    { 1, 1, 15, magic_missile_spell},
    { 5, 4, 20, phase_door_spell},
    { 10, 6, 25, _orcish_curse_spell},
    { 12, 6, 25, detect_monsters_spell},
    { 13, 8, 25, _sleep_monster_spell},
    { 15, 8, 25, detect_traps_spell},
    { 21, 12, 30, mind_blast_spell},
    { 23, 14, 40, _confuse_monster_spell},
    { 25, 17, 40, nether_bolt_spell},
    { 32, 22, 50, _evil_curse_spell},
    { 40, 32, 65, animate_dead_spell},
    { -1, -1, -1, NULL}
};

static caster_info * _shaman_caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.options = CASTER_USE_HP;
        me.which_stat = A_INT;
        init = TRUE;
    }
    return &me;
}

static void _shaman_gain_level(int new_level)
{
    if (p_ptr->current_r_idx == MON_ORC_SHAMAN && new_level >= 23)
    {
        p_ptr->current_r_idx = MON_ORC_WARLOCK;
        msg_print("You have evolved into an <color:v>Orc warlock</color>.");
        p_ptr->redraw |= PR_MAP;
    }
}

static race_t *_shaman_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Orc shaman", "Orc warlock"};
    int           rank = 0;

    if (p_ptr->max_plv >= 23) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  24,  40,   2,  24,  12,  60,  48};
    skills_t xs = { 11,   8,   9,   0,   0,   0,  15,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 3;
        me.exp = 100;
        me.life = 92;
        me.base_hp = 28;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _shaman_birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _shaman_caster_info;
        me.get_flags = _get_flags;
        me.gain_level = _shaman_gain_level;
        me.pseudo_class_idx = CLASS_WARRIOR_MAGE;
        me.get_spells = _shaman_get_spells;
        me.character_dump = py_dump_spells;
        me.subdesc = "Orc shamans and warlocks are more cunning than the average orc and "
                       "have learned rudimentary magic from their black masters. "
                       "They can detect threats, blink out of dangerous situations "
                       "and easily dispatch weak enemies with their offensive spells, "
                       "and are not quite as hopeless with magic devices as their "
                       "fighter kin, but like all orcs they tend to have trouble "
                       "when the going really gets tough.";
                       
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] = -1 + rank;
    me.stats[A_INT] = -1 + rank;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] = -1;
    me.stats[A_CON] = -1;
    me.stats[A_CHR] = -5;
    me.boss_r_idx = MON_OTHROD;
    if (birth_hack || spoiler_hack) me.subname = "Orc shaman";

    return &me;
}


/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_orc_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case ORC_FIGHTER:
        result = _fighter_get_race_t();
        break;
    case ORC_WARLOCK:
        result = _shaman_get_race_t();
        break;
    default:
        result = _fighter_get_race_t();
        break;
    }

    result->name = "Orc";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->shop_adjust = 140;

    return result;
}


