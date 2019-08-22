#include "angband.h"

#define MON_CHEERFUL_LEPRECHAUN 258
#define MON_MALICIOUS_LEPRECHAUN 529
#define MON_DEATH_LEPRECHAUN 680
#define MON_LEPRECHAUN_FANATIC 700

static cptr _desc =
    "Leprechauns are small, mischievous creatures, always scheming for gold. "
    "They are weak, but quick and stealthy. For combat, they prefer not to use "
    "weapons at all. Rather, by keeping their weapon hand free, they may pilfer "
    "objects or gold from their foes and flee to safety, all in a single action. However, "
    "they are capable of fighting with normal weapons should the need arise, but "
    "one seldom hears of leprechauns inspiring fear in combat!\n \n"
    "Leprechauns value riches above all else and even begin the game with their "
    "legendary pot of gold. Leprechauns also have the luck of the Irish, and good "
    "drops are more common for them. For magic, the leprechaun has a few talents "
    "and even a bit of cursing offense (Malicious and Death Leprechauns are vile, "
    "evil creatures, after all). They are masters of teleportation and detection. And while "
    "one never fears their combat (though one despises their thievery, of course), they "
    "are great masters of bow and device alike. At high levels, they move with incredible "
    "speed and seem to be in multiple places at once. Dexterity determines the leprechaun's "
    "skill with their magic.\n \n"
    "The leprechaun uses gold for spell casting the way other players use mana. Gold is "
    "also used to pay for recharging devices or to power mana branded weapons. In addition, "
    "increasing the amount of gold on hand increases the power of the leprechaun, granting "
    "extra blows per round, extra shots per round, increased AC, life rating and even "
    "increasing their power with magical devices. To succeed in life, greed is obviously "
    "the primary virtue of the leprechaun!";

static void _birth(void)
{
    p_ptr->current_r_idx = MON_CHEERFUL_LEPRECHAUN;
    skills_innate_init("Greedy Hands", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    msg_print("You feel the luck of the Irish!");
    mut_gain(MUT_GOOD_LUCK);
    mut_lock(MUT_GOOD_LUCK);

    p_ptr->au = 50000;

    py_birth_food();
    py_birth_light();
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    return result;
}

/**********************************************************************
 * Innate Attacks
 **********************************************************************/
static void _calc_innate_attacks(void)
{
    if (p_ptr->weapon_ct == 0 && equip_find_empty_hand())
    {
        innate_attack_t    a = {0};
        int l = p_ptr->lev;

        a.dd = 1;
        a.ds = 3 + l / 15;
        a.weight = 2;
        a.to_h = p_ptr->lev/5;

        a.effect[1] = GF_STEAL;

        calc_innate_blows(&a, 300);

        a.msg = "You pilfer.";
        a.name = "Greedy Hands";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/****************************************************************
 * Spells
 ****************************************************************/
static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void blink_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blinking Death");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you will execute a short range, line of sight teleport after every action.");
        break;
    default:
        _toggle_spell(LEPRECHAUN_TOGGLE_BLINK, cmd, res);
        break;
    }
}

void _hoarding_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hoarding");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, any items you 'destroy' will be automatically converted into gold.");
        break;
    default:
        _toggle_spell(LEPRECHAUN_TOGGLE_HOARDING, cmd, res);
        break;
    }
}

/**********************************************************************
 * Leprechaun Spells and Abilities
 **********************************************************************/
void _fanaticism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fanaticism");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon many devoted leprechaun servants at chosen foe.");
        break;
    case SPELL_CAST:
    {
        int x, y, i;

        var_set_bool(res, FALSE);
        if (!target_set(TARGET_KILL)) return;
        x = target_col;
        y = target_row;

        for (i = 0; i < 8; i++)
            summon_named_creature(-1, y, x, MON_LEPRECHAUN_FANATIC, PM_FORCE_PET);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _spells[] =
{ /* Lvl    Au  Fail */
    {  1,    5,   20, phase_door_spell},
    {  1,   10,   20, detect_treasure_spell},
    { 10,   25,   30, detection_spell},
    { 15,   25,   30, cause_wounds_I_spell},
    { 15,   20,   30, teleport_spell},
    { 15,  100,   40, teleport_to_spell},
    { 20,  250,   40, telekinesis_spell},
    { 30,  200,   50, cause_wounds_III_spell},
    { 35,  500,   60, animate_dead_spell},
    { 37, 1000,   60, recharging_spell},
    { 40,    0,    0, blink_toggle_spell},
    { 42,    0,    0, _hoarding_toggle_spell},
    { 45, 1500,   60, _fanaticism_spell},
    { -1,   -1,   -1, NULL}
};

static int _get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, max, _spells);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "greedy power";
        me.which_stat = A_DEX;
        me.options = CASTER_USE_AU;
        init = TRUE;
    }
    return &me;
}

static void _calc_bonuses(void)
{
    int ac = MIN(p_ptr->au / 250000, 25);

    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;

    if (p_ptr->au >= 10 * 1000 * 1000)
    {
        p_ptr->device_power += 2;
    }
    else if (p_ptr->au >= 5 * 1000 * 1000)
    {
        p_ptr->device_power += 1;
    }

    p_ptr->skills.thb += MIN(p_ptr->au / 100000, 100);

    switch (p_ptr->current_r_idx)
    {
    case MON_CHEERFUL_LEPRECHAUN:
        p_ptr->align += 200;
        p_ptr->pspeed += 5;
        break;
    case MON_MALICIOUS_LEPRECHAUN:
        p_ptr->align -= 200;
        p_ptr->pspeed += 7;
        p_ptr->levitation = TRUE;
        res_add_vuln(RES_LITE);
        break;
    case MON_DEATH_LEPRECHAUN:
        p_ptr->align -= 200;
        p_ptr->pspeed += 10;
        p_ptr->levitation = TRUE;
        res_add_vuln(RES_LITE);
        res_add(RES_NETHER);
        break;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->au >= 5 * 1000 * 1000)
        add_flag(flgs, OF_DEVICE_POWER);

    switch (p_ptr->current_r_idx)
    {
    case MON_CHEERFUL_LEPRECHAUN:
        add_flag(flgs, OF_SPEED);
        break;
    case MON_MALICIOUS_LEPRECHAUN:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_VULN_LITE);
        break;
    case MON_DEATH_LEPRECHAUN:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_VULN_LITE);
        break;
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    info_ptr->xtra_blow += MIN(p_ptr->au / 100000, 100);
}

static void _player_action(int energy_use)
{
    if (_get_toggle() == LEPRECHAUN_TOGGLE_BLINK)
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);
}

/**********************************************************************
 * Leprechaun Evolution
 **********************************************************************/
static void _gain_level(int new_level)
{
    if (p_ptr->current_r_idx == MON_CHEERFUL_LEPRECHAUN && new_level >= 15)
    {
        p_ptr->current_r_idx = MON_MALICIOUS_LEPRECHAUN;
        msg_print("You have evolved into a Malicious Leprechaun.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MALICIOUS_LEPRECHAUN && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_DEATH_LEPRECHAUN;
        msg_print("You have evolved into a Death Leprechaun.");
        p_ptr->redraw |= PR_MAP;
    }
}

/**********************************************************************
 * Public Interface
 **********************************************************************/
int leprechaun_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->prace == RACE_MON_LEPRECHAUN)
        result = _get_toggle();
    return result;
}

bool leprechaun_steal(int m_idx)
{
    bool result = FALSE;
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    if ( !mon_save_p(m_ptr->r_idx, A_DEX)
      || (MON_CSLEEP(m_ptr) && !mon_save_p(m_ptr->r_idx, A_DEX)))
    {
        object_type loot = {0};

        if (m_ptr->hold_o_idx && one_in_(2))
        {
            object_copy(&loot, &o_list[m_ptr->hold_o_idx]);
            delete_object_idx(m_ptr->hold_o_idx);
            loot.held_m_idx = 0;
        }
        else if (m_ptr->drop_ct > m_ptr->stolen_ct)
        {
            if (get_monster_drop(m_idx, &loot))
            {
                m_ptr->stolen_ct++;
                if (r_ptr->flags1 & RF1_UNIQUE)
                    r_ptr->stolen_ct++;
            }
        }

        if (!loot.k_idx)
        {
            msg_print("There is nothing to steal!");
        }
        else
        {
            char o_name[MAX_NLEN];

            result = TRUE;
            object_desc(o_name, &loot, 0);
            if (mon_save_p(m_ptr->r_idx, A_DEX))
            {
                msg_format("Oops! You drop %s.", o_name);
                drop_near(&loot, -1, py, px);
            }
            else if (loot.tval == TV_GOLD)
            {
                msg_format("You steal %d gold pieces' worth of %s.", (int)loot.pval, o_name);
                sound(SOUND_SELL);
                p_ptr->au += loot.pval;
                stats_on_gold_find(loot.pval);
                p_ptr->redraw |= (PR_GOLD);
                if (prace_is_(RACE_MON_LEPRECHAUN))
                    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
            }
            else
            {
                pack_carry(&loot);
                msg_format("You steal %s.", o_name);
            }
        }
    }
    return result;
}

bool _destroy_object(object_type *o_ptr)
{
    if (_get_toggle() == LEPRECHAUN_TOGGLE_HOARDING)
    {
        int amt = obj_value_real(o_ptr) * o_ptr->number / 3;

        if (amt > 0)
        {
            char o_name[MAX_NLEN];

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            msg_format("You turn %s to %d coins' worth of gold.", o_name, amt);

            p_ptr->au += amt;
            stats_on_gold_selling(amt); /* ? */

            p_ptr->redraw |= (PR_GOLD);
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

            return TRUE;
        }
    }
    return FALSE;
}

race_t *mon_leprechaun_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Cheerful Leprechaun", "Malicious Leprechaun", "Death Leprechaun"};
    int           rank = 0;

    if (p_ptr->lev >= 15) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  45,  38,  10,  24,  16,  48,  50 };
    skills_t xs = { 12,  18,  11,   1,   0,   0,  13,  10 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Leprechaun";
        me.desc = _desc;

        me.infra = 5;
        me.exp = 150;
        me.base_hp = 15;
        me.shop_adjust = 85;

        me.get_spells = _get_spells;
        me.caster_info = _caster_info;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;
        me.player_action = _player_action;
        me.destroy_object = _destroy_object;
        me.pseudo_class_idx = CLASS_ROGUE;
        me.boss_r_idx = MON_GRAGOMANI;

        me.flags = RACE_IS_MONSTER;
        init = TRUE;
    }

    me.life = 80;
    if (!spoiler_hack)
        me.life += MIN(p_ptr->au / 500000, 20);

    if (!birth_hack && !spoiler_hack)
        me.subname = titles[rank];
    me.stats[A_STR] = -2 - 2*rank;
    me.stats[A_INT] = 1;
    me.stats[A_WIS] = 1;
    me.stats[A_DEX] = 3 + 2*rank;
    me.stats[A_CON] = -2;
    me.stats[A_CHR] = -2;

    return &me;
}
