#include "angband.h"

#define _MAX_POWER  5

static int _spell_stat(void)
{
    int result = A_INT;
    int max = p_ptr->stat_ind[A_INT];
    
    if (p_ptr->stat_ind[A_WIS] > max)
    {
        result = A_WIS;
        max = p_ptr->stat_ind[A_WIS];
    }

    if (p_ptr->stat_ind[A_CHR] > max)
    {
        result = A_CHR;
        max = p_ptr->stat_ind[A_CHR];
    }
    return result;
}

static int _spell_stat_idx(void)
{
    return p_ptr->stat_ind[_spell_stat()];
}

/* Magic Number Indices 
    p_ptr->magic_num1 functions as a timer for the effect.
    p_ptr->magic_num2 remembers the power of the effect.
*/
#define _WEAPON_GRAFT 0
#define _CLARITY      1
#define _BLENDING     2
#define _SHIELDING    3
#define _COMBAT       4
#define _SPEED        5
#define _BACKLASH     6
#define _FORTRESS     7
#define _MINDSPRING   8
#define _FORESIGHT    9
#define _ARCHERY     10
#define _DISRUPTION  11
#define _DRAIN       12

bool psion_weapon_graft(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_WEAPON_GRAFT] > 0) return TRUE;
    return FALSE;
}

bool psion_can_wield(object_type *o_ptr)
{
    if ( object_is_melee_weapon(o_ptr) 
      && p_ptr->pclass == CLASS_PSION
      && psion_weapon_graft() )
    {
        msg_print("Failed! Your weapon is currently grafted to your arm!");
        return FALSE;
    }
    return TRUE;
}

bool psion_check_dispel(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_SPEED] > 0 && p_ptr->pspeed < 145 && p_ptr->magic_num2[_SPEED] > 2) return TRUE;
    if (p_ptr->magic_num1[_SHIELDING] > 0) return TRUE;
    if (p_ptr->magic_num1[_FORTRESS] > 0) return TRUE;
    /*if (p_ptr->magic_num1[_MINDSPRING] > 0) return TRUE;*/
    if (p_ptr->magic_num1[_FORESIGHT] > 0) return TRUE;
    if (p_ptr->magic_num1[_DISRUPTION] > 0) return TRUE;
    if (p_ptr->magic_num1[_DRAIN] > 0) return TRUE;
    return FALSE;
}

bool psion_clarity(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_CLARITY] > 0) return TRUE;
    return FALSE;
}

bool psion_blending(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_BLENDING] > 0) return TRUE;
    return FALSE;
}

bool psion_shielding(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_SHIELDING] > 0) return TRUE;
    return FALSE;
}

bool psion_combat(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_COMBAT] > 0) return TRUE;
    return FALSE;
}

/* Mega-Hack - no weapon, so the extra blows go to innate attacks
 * We don't use the normal innate attack extra blow code because that only
 * gives extra blows to one innate attack per round */
void psion_combat_innate_blows(innate_attack_t *a_ptr)
{
    a_ptr->blows += 20*p_ptr->magic_num2[_COMBAT];
}

bool psion_archery(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_ARCHERY] > 0) return TRUE;
    return FALSE;
}

bool psion_speed(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_SPEED] > 0) return TRUE;
    return FALSE;
}

bool psion_backlash(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_BACKLASH] > 0) return TRUE;
    return FALSE;
}

int psion_backlash_dam(int dam)
{
    if (psion_backlash())
        dam = dam * (25 + 25*p_ptr->magic_num2[_BACKLASH]) / 100;
    return dam;
}

bool psion_mental_fortress(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_FORTRESS] > 0) return TRUE;
    return FALSE;
}

bool psion_mindspring(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_MINDSPRING] > 0) return TRUE;
    return FALSE;
}

void psion_do_mindspring(int energy)
{
    if (!psion_mindspring()) return;
    p_ptr->csp += 16*p_ptr->magic_num2[_MINDSPRING] * energy / 100;
    if (p_ptr->csp >= p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    p_ptr->redraw |= PR_MANA;
}

bool psion_disruption(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_DISRUPTION] > 0) return TRUE;
    return FALSE;
}

bool psion_check_disruption(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    return psion_check_disruption_aux(m_ptr);
}
bool psion_check_disruption_aux(mon_ptr m_ptr)
{
    if (psion_disruption())
    {
        monster_race *r_ptr = &r_info[m_ptr->r_idx];
        int           pl = p_ptr->lev + 8*p_ptr->magic_num2[_DISRUPTION];

        if (randint0(r_ptr->level) < pl) 
            return TRUE;
    }
    return FALSE;
}

bool psion_drain(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_DRAIN] > 0) return TRUE;
    return FALSE;
}

int psion_do_drain(int dam)
{
    int result = dam, drain;
    mon_spell_ptr spell = mon_spell_current();

    if (!psion_drain()) return result;
    if (!spell) return result;
    if (spell->flags & MSF_INNATE) return result;

    drain = dam * 5 * p_ptr->magic_num2[_DRAIN] / 100;
    result -= drain;
    sp_player(MAX(drain, 3 * p_ptr->magic_num2[_DRAIN]));
    if (disturb_minor)
        msg_print("You draw power from the magics around you!");
    return result;
}

bool psion_foresight(void)
{
    if (p_ptr->pclass != CLASS_PSION) return FALSE;
    if (p_ptr->magic_num1[_FORESIGHT] > 0) return TRUE;
    return FALSE;
}

bool psion_check_foresight(void)
{
    if (!psion_foresight()) return FALSE;
    if (randint1(100) <= 12*p_ptr->magic_num2[_FORESIGHT] + 7)
    {
        msg_print("You saw that one coming!");
        return TRUE;
    }
    return FALSE;
}

bool psion_mon_save_p(int r_idx, int power)
{
    int pl = p_ptr->lev;
    int ml = r_info[r_idx].level;
    int s = _spell_stat_idx() + 3;

    if (ml + randint1(100) > pl + s + power*14) return TRUE;
    return FALSE;
}

bool psion_process_monster(int m_idx)
{
    bool result = FALSE;
    bool fear = FALSE;
    monster_type *m_ptr = &m_list[m_idx];
    if (m_ptr->ego_whip_ct)
    {
        char m_name[255];

        monster_desc(m_name, m_ptr, 0);
        anger_monster(m_ptr);

        if (psion_mon_save_p(m_ptr->r_idx, m_ptr->ego_whip_pow))
        {
            msg_format("%^s shakes off your ego whip!", m_name);
            p_ptr->redraw |= PR_HEALTH_BARS;
            m_ptr->ego_whip_ct = 0;
            m_ptr->ego_whip_pow = 0;
        }
        else
        {
            msg_format("Your ego whip lashes %s!", m_name);
            result = mon_take_hit(m_idx, spell_power(30*m_ptr->ego_whip_pow), DAM_TYPE_SPELL, &fear, NULL);
            m_ptr->ego_whip_ct--;
            if (!projectable(py, px, m_ptr->fy, m_ptr->fx))
                mon_anger(m_ptr);
            if (!m_ptr->ego_whip_ct)
            {
                msg_format("Your ego whip on %s disappears.", m_name);
                p_ptr->redraw |= PR_HEALTH_BARS;
            }
        }
    }
    return result;
}

/***************************************************************************
   For spells, I would prefer to choose the spell first and be presented
   with a sublist for all available power options, rather than choosing the
   power level (blindly) up front. This is a bit harder to implement as
   it requires 5 separate spell functions for each psionic power.
   For example, I know I want to blast an enemy with a Mana Thrust, but
   I don't know how much I can afford or what the various damage levels
   are.
 ***************************************************************************/

static cptr _roman_numeral[_MAX_POWER + 1] = { "", "I", "II", "III", "IV", "V" };

/* Archery Transformation */
static void _clear_counter(int which, cptr off);
static void _archery_transformation_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Archery %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, you focus your mental powers on effective shooting.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("+%d Archery skill", power*20));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_ARCHERY])
        {
            msg_print("You are already transformed into a shooting machine.");
            return;
        }
        _clear_counter(_COMBAT, "Your combat transformation expires.");    
        msg_print("You transform into a shooting machine!");
        p_ptr->magic_num1[_ARCHERY] = spell_power(power*8 + 20);
        p_ptr->magic_num2[_ARCHERY] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _archery_transformation1_spell(int cmd, variant *res) { _archery_transformation_spell(1, cmd, res); }
static void _archery_transformation2_spell(int cmd, variant *res) { _archery_transformation_spell(2, cmd, res); }
static void _archery_transformation3_spell(int cmd, variant *res) { _archery_transformation_spell(3, cmd, res); }
static void _archery_transformation4_spell(int cmd, variant *res) { _archery_transformation_spell(4, cmd, res); }
static void _archery_transformation5_spell(int cmd, variant *res) { _archery_transformation_spell(5, cmd, res); }

/* Brain Smash */
static void _brain_smash_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Brain Smash %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Pummel the minds of your foes.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_radius(2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(
            GF_PSI_BRAIN_SMASH,
            dir, 
            power,
            2
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _brain_smash1_spell(int cmd, variant *res) { _brain_smash_spell(1, cmd, res); }
static void _brain_smash2_spell(int cmd, variant *res) { _brain_smash_spell(2, cmd, res); }
static void _brain_smash3_spell(int cmd, variant *res) { _brain_smash_spell(3, cmd, res); }
static void _brain_smash4_spell(int cmd, variant *res) { _brain_smash_spell(4, cmd, res); }
static void _brain_smash5_spell(int cmd, variant *res) { _brain_smash_spell(5, cmd, res); }

/* Combat Transformation */
static void _combat_transformation_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Combat %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, you focus your mental powers on effective combat.");
        break;
    case SPELL_INFO:
        if (prace_is_(RACE_TONBERRY)) var_set_string(res, format("Blows: +%d.%2.2d", power / 4, (power * 25) % 100));
        else var_set_string(res, format("Blows: +%d.%d", power * 2 / 5, (power * 4) % 10));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_COMBAT])
        {
            msg_print("You are already transformed into a fighting machine.");
            return;
        }
        _clear_counter(_ARCHERY, "Your archery transformation expires.");    
        msg_print("You transform into a fighting machine!");
        p_ptr->magic_num1[_COMBAT] = spell_power(power*8 + 20);
        p_ptr->magic_num2[_COMBAT] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _combat_transformation1_spell(int cmd, variant *res) { _combat_transformation_spell(1, cmd, res); }
static void _combat_transformation2_spell(int cmd, variant *res) { _combat_transformation_spell(2, cmd, res); }
static void _combat_transformation3_spell(int cmd, variant *res) { _combat_transformation_spell(3, cmd, res); }
static void _combat_transformation4_spell(int cmd, variant *res) { _combat_transformation_spell(4, cmd, res); }
static void _combat_transformation5_spell(int cmd, variant *res) { _combat_transformation_spell(5, cmd, res); }

/* Ego Whip */
static void _ego_whip_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Ego Whip %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Lash out against a single monster with psychic energy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(power*40)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(
            GF_PSI_EGO_WHIP, 
            dir, 
            power,
            0
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _ego_whip1_spell(int cmd, variant *res) { _ego_whip_spell(1, cmd, res); }
static void _ego_whip2_spell(int cmd, variant *res) { _ego_whip_spell(2, cmd, res); }
static void _ego_whip3_spell(int cmd, variant *res) { _ego_whip_spell(3, cmd, res); }
static void _ego_whip4_spell(int cmd, variant *res) { _ego_whip_spell(4, cmd, res); }
static void _ego_whip5_spell(int cmd, variant *res) { _ego_whip_spell(5, cmd, res); }

/* Energy Blast */
typedef struct {
    cptr name;
    int type;
} _blast_t;
static _blast_t _blasts[_MAX_POWER] = {
    {"Fire", GF_FIRE},
    {"Cold", GF_COLD},
    {"Poison", GF_POIS},
    {"Acid", GF_ACID},
    {"Lightning", GF_ELEC},
};
static void _energy_blast_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_KEY:
        var_set_int(res, _blasts[which].name[0]);
        break;
    case MENU_TEXT:
        var_set_string(res, _blasts[which].name);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static int _get_energy_blast_type(int power)
{
    if (power == 1) return GF_FIRE;
    else
    { 
        int i;
        menu_t menu = { "Choose which effect?", NULL, NULL,
                        _energy_blast_menu_fn, NULL, power, 0};
        
        i = menu_choose(&menu);
        if (i >= 0)
            i = _blasts[i].type;
        return i;
    }
}

static void _energy_blast_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Blast %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an elemental ball.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(4*power), spell_power(4*power), 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int type = _get_energy_blast_type(power);
        
        var_set_bool(res, FALSE);
        
        if (type < 0) return;
        if (!get_fire_dir(&dir)) return;

        fire_ball_aux(
            type, 
            dir, 
            spell_power(damroll(4*power, 4*power)),
            spell_power((1 + power)/2),
            PROJECT_FULL_DAM
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _energy_blast1_spell(int cmd, variant *res) { _energy_blast_spell(1, cmd, res); }
static void _energy_blast2_spell(int cmd, variant *res) { _energy_blast_spell(2, cmd, res); }
static void _energy_blast3_spell(int cmd, variant *res) { _energy_blast_spell(3, cmd, res); }
static void _energy_blast4_spell(int cmd, variant *res) { _energy_blast_spell(4, cmd, res); }
static void _energy_blast5_spell(int cmd, variant *res) { _energy_blast_spell(5, cmd, res); }

/* Graft Weapon */
static void _graft_weapon_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Graft Weapon %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fuses your melee weapon to your arms and gain combat bonuses. For the duration of this power, you cannot unequip/swap your weapon.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("(+%2d,+%2d) melee", 6*power, 4*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_WEAPON_GRAFT])
        {
            msg_print("Your weapon is already grafted!");
            return;
        }
        if (!equip_find_first(object_is_melee_weapon))
        {
            msg_print("You are not wielding a weapon!");
            return;
        }
        msg_print("Your weapon fuses to your arm!");
        p_ptr->magic_num1[_WEAPON_GRAFT] = spell_power(8*power + 20);
        p_ptr->magic_num2[_WEAPON_GRAFT] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _graft_weapon1_spell(int cmd, variant *res) { _graft_weapon_spell(1, cmd, res); }
static void _graft_weapon2_spell(int cmd, variant *res) { _graft_weapon_spell(2, cmd, res); }
static void _graft_weapon3_spell(int cmd, variant *res) { _graft_weapon_spell(3, cmd, res); }
static void _graft_weapon4_spell(int cmd, variant *res) { _graft_weapon_spell(4, cmd, res); }
static void _graft_weapon5_spell(int cmd, variant *res) { _graft_weapon_spell(5, cmd, res); }

/* Mana Thrust */
static void _mana_thrust_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Mana Thrust %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of pure mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(4*power), spell_power(4*power), 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_MANA, dir, spell_power(damroll(4*power, 4*power)));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_thrust1_spell(int cmd, variant *res) { _mana_thrust_spell(1, cmd, res); }
static void _mana_thrust2_spell(int cmd, variant *res) { _mana_thrust_spell(2, cmd, res); }
static void _mana_thrust3_spell(int cmd, variant *res) { _mana_thrust_spell(3, cmd, res); }
static void _mana_thrust4_spell(int cmd, variant *res) { _mana_thrust_spell(4, cmd, res); }
static void _mana_thrust5_spell(int cmd, variant *res) { _mana_thrust_spell(5, cmd, res); }

/* Mental Fortress */
static void _mental_fortress_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Mental Fortress %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short time, you become resistant to anti-magic, dispel magic and any attack that drains mana.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Spell Power: +%d%%", spell_power_aux(100, power) - 100));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_FORTRESS])
        {
            msg_print("You already have a mental fortress.");
            return;
        }
        msg_print("You erect a mental fortress.");
        p_ptr->magic_num1[_FORTRESS] = spell_power(power + 3);
        p_ptr->magic_num2[_FORTRESS] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _mental_fortress1_spell(int cmd, variant *res) { _mental_fortress_spell(1, cmd, res); }
static void _mental_fortress2_spell(int cmd, variant *res) { _mental_fortress_spell(2, cmd, res); }
static void _mental_fortress3_spell(int cmd, variant *res) { _mental_fortress_spell(3, cmd, res); }
static void _mental_fortress4_spell(int cmd, variant *res) { _mental_fortress_spell(4, cmd, res); }
static void _mental_fortress5_spell(int cmd, variant *res) { _mental_fortress_spell(5, cmd, res); }

/* Mindspring */
static void _mindspring_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Mindspring %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short time, you regain mana with every action.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Recover %d sp/rnd", 16*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_MINDSPRING])
        {
            msg_print("Your mindspring is already flowing.");
            return;
        }
        msg_print("Your mindspring flows.");
        p_ptr->magic_num1[_MINDSPRING] = spell_power(power*2 + 3);
        p_ptr->magic_num2[_MINDSPRING] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _mindspring1_spell(int cmd, variant *res) { _mindspring_spell(1, cmd, res); }
static void _mindspring2_spell(int cmd, variant *res) { _mindspring_spell(2, cmd, res); }
static void _mindspring3_spell(int cmd, variant *res) { _mindspring_spell(3, cmd, res); }
static void _mindspring4_spell(int cmd, variant *res) { _mindspring_spell(4, cmd, res); }
static void _mindspring5_spell(int cmd, variant *res) { _mindspring_spell(5, cmd, res); }

/* Psionic Backlash */
static void _psionic_backlash_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Backlash %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, monsters are damaged whenever they hurt you.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Revenge: %d%%", 25 + 25*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_BACKLASH])
        {
            msg_print("Your psionic revenge is already active.");
            return;
        }
        msg_print("You contemplate revenge!");
        p_ptr->magic_num1[_BACKLASH] = spell_power(power*5 + 5);
        p_ptr->magic_num2[_BACKLASH] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_backlash1_spell(int cmd, variant *res) { _psionic_backlash_spell(1, cmd, res); }
static void _psionic_backlash2_spell(int cmd, variant *res) { _psionic_backlash_spell(2, cmd, res); }
static void _psionic_backlash3_spell(int cmd, variant *res) { _psionic_backlash_spell(3, cmd, res); }
static void _psionic_backlash4_spell(int cmd, variant *res) { _psionic_backlash_spell(4, cmd, res); }
static void _psionic_backlash5_spell(int cmd, variant *res) { _psionic_backlash_spell(5, cmd, res); }

/* Psionic Blending */
static void _psionic_blending_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Blending %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "You will temporarily blend into your surroundings, gaining increased stealth.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("+%d stealth", 3*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_BLENDING])
        {
            msg_print("You are already blending into your surroundings.");
            return;
        }
        msg_print("You blend into your surroundings.");
        p_ptr->magic_num1[_BLENDING] = spell_power(power*25 + 50);
        p_ptr->magic_num2[_BLENDING] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_blending1_spell(int cmd, variant *res) { _psionic_blending_spell(1, cmd, res); }
static void _psionic_blending2_spell(int cmd, variant *res) { _psionic_blending_spell(2, cmd, res); }
static void _psionic_blending3_spell(int cmd, variant *res) { _psionic_blending_spell(3, cmd, res); }
static void _psionic_blending4_spell(int cmd, variant *res) { _psionic_blending_spell(4, cmd, res); }
static void _psionic_blending5_spell(int cmd, variant *res) { _psionic_blending_spell(5, cmd, res); }

/* Psionic Clarity */
static void _psionic_clarity_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Clarity %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For the duration of this power, you gain increased mental focus. Your psionic powers become cheaper to cast.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Costs: %d%%", 85-7*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_CLARITY])
        {
            msg_print("Your mind is already focused.");
            return;
        }
        msg_print("You focus your mind.");
        p_ptr->magic_num1[_CLARITY] = spell_power(2*power + 5);
        p_ptr->magic_num2[_CLARITY] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_clarity1_spell(int cmd, variant *res) { _psionic_clarity_spell(1, cmd, res); }
static void _psionic_clarity2_spell(int cmd, variant *res) { _psionic_clarity_spell(2, cmd, res); }
static void _psionic_clarity3_spell(int cmd, variant *res) { _psionic_clarity_spell(3, cmd, res); }
static void _psionic_clarity4_spell(int cmd, variant *res) { _psionic_clarity_spell(4, cmd, res); }
static void _psionic_clarity5_spell(int cmd, variant *res) { _psionic_clarity_spell(5, cmd, res); }

/* Psionic Crafting */
static int _enchant_power = 0;
int psion_enchant_power(void) { 
    if (p_ptr->pclass == CLASS_PSION)
        return _enchant_power;
    return 0;
}
void _psionic_crafting_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Crafting %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to enchant a weapon, ammo or armor.");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        bool         okay = FALSE;
        char         o_name[MAX_NLEN];

        var_set_bool(res, FALSE);

        prompt.prompt = "Enchant which item?";
        prompt.error = "You have nothing to enchant.";
        prompt.filter = object_is_weapon_armour_ammo;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_EQUIP;
        prompt.where[2] = INV_QUIVER;
        prompt.where[3] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;

        object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        _enchant_power = power; /* Hack for enchant(), which I'm too lazy to rewrite ... */
        if (power == 5 && object_is_nameless(prompt.obj) && prompt.obj->number == 1)
        {
            if (object_is_weapon(prompt.obj))
            {
                if (brand_weapon_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
            else if (object_is_armour(prompt.obj))
            {
                if (brand_armour_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
        }

        if (!okay)
        {
            if (object_is_weapon_ammo(prompt.obj))
            {
                if (enchant(prompt.obj, randint0(4) + 1, ENCH_TOHIT | ENCH_PSI_HACK)) okay = TRUE;
                if (enchant(prompt.obj, randint0(4) + 1, ENCH_TODAM | ENCH_PSI_HACK)) okay = TRUE;
            }
            else
            {
                if (enchant(prompt.obj, randint0(3) + 2, ENCH_TOAC | ENCH_PSI_HACK)) okay = TRUE;            
            }
        }

        msg_format("The %s glow%s brightly!", o_name,
                ((prompt.obj->number > 1) ? "" : "s"));

        if (!okay)
        {
            if (flush_failure) flush();
            msg_print("The enchantment failed.");
            if (one_in_(3)) virtue_add(VIRTUE_ENCHANTMENT, -1);
        }
        else
        {
            virtue_add(VIRTUE_ENCHANTMENT, 1);
            android_calc_exp();
        }
        _enchant_power = 0;
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _psionic_crafting1_spell(int cmd, variant *res) { _psionic_crafting_spell(1, cmd, res); }
void _psionic_crafting2_spell(int cmd, variant *res) { _psionic_crafting_spell(2, cmd, res); }
void _psionic_crafting3_spell(int cmd, variant *res) { _psionic_crafting_spell(3, cmd, res); }
void _psionic_crafting4_spell(int cmd, variant *res) { _psionic_crafting_spell(4, cmd, res); }
void _psionic_crafting5_spell(int cmd, variant *res) { _psionic_crafting_spell(5, cmd, res); }

/* Psionic Disruption */
static void _psionic_disruption_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Disruption %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, your mental focus will disrupt the minds of others.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Power: %d", p_ptr->lev + 8*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_DISRUPTION])
        {
            msg_print("Your disruption is already active.");
            return;
        }
        msg_print("You project disrupting thoughts!");
        p_ptr->magic_num1[_DISRUPTION] = spell_power(power*2 + 3);
        p_ptr->magic_num2[_DISRUPTION] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_disruption1_spell(int cmd, variant *res) { _psionic_disruption_spell(1, cmd, res); }
static void _psionic_disruption2_spell(int cmd, variant *res) { _psionic_disruption_spell(2, cmd, res); }
static void _psionic_disruption3_spell(int cmd, variant *res) { _psionic_disruption_spell(3, cmd, res); }
static void _psionic_disruption4_spell(int cmd, variant *res) { _psionic_disruption_spell(4, cmd, res); }
static void _psionic_disruption5_spell(int cmd, variant *res) { _psionic_disruption_spell(5, cmd, res); }

/* Psionic Drain */
static void _psionic_drain_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Drain %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while you will draw mental energy from enemy magic spells, reducing their damage in the process.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Drain: %d%%", 5*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_DRAIN])
        {
            msg_print("Your drain is already active.");
            return;
        }
        msg_print("You prepare to draw power from surrounding magics.");
        p_ptr->magic_num1[_DRAIN] = spell_power(power*5 + 10);
        p_ptr->magic_num2[_DRAIN] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_drain1_spell(int cmd, variant *res) { _psionic_drain_spell(1, cmd, res); }
static void _psionic_drain2_spell(int cmd, variant *res) { _psionic_drain_spell(2, cmd, res); }
static void _psionic_drain3_spell(int cmd, variant *res) { _psionic_drain_spell(3, cmd, res); }
static void _psionic_drain4_spell(int cmd, variant *res) { _psionic_drain_spell(4, cmd, res); }
static void _psionic_drain5_spell(int cmd, variant *res) { _psionic_drain_spell(5, cmd, res); }

/* Psionic Foresight */
static void _psionic_foresight_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Foresight %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, you can see the future and may be able to avoid damage.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Avoidance: %d%%", 7 + 12*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_FORESIGHT])
        {
            msg_print("Your foresight is already active.");
            return;
        }
        msg_print("You see the future!");
        p_ptr->magic_num1[_FORESIGHT] = spell_power(power*2 + 3);
        p_ptr->magic_num2[_FORESIGHT] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_foresight1_spell(int cmd, variant *res) { _psionic_foresight_spell(1, cmd, res); }
static void _psionic_foresight2_spell(int cmd, variant *res) { _psionic_foresight_spell(2, cmd, res); }
static void _psionic_foresight3_spell(int cmd, variant *res) { _psionic_foresight_spell(3, cmd, res); }
static void _psionic_foresight4_spell(int cmd, variant *res) { _psionic_foresight_spell(4, cmd, res); }
static void _psionic_foresight5_spell(int cmd, variant *res) { _psionic_foresight_spell(5, cmd, res); }

/* Psionic Healing */
static void _psionic_healing_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Healing %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Use your mental powers to repair your body.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(79*power - (MAX(32, 5 * power +15)))));
        break;
    case SPELL_CAST:
        hp_player(spell_power(79*power - (MAX(32, 5 * power +15))));
        
        set_blind(0, TRUE);
        set_confused(0, TRUE); /* Probably, @ can't cast this while confused! */
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_shero(0,TRUE);

        if (power >= 3)
            set_image(0, TRUE);

        if (power == 5)
        {
            do_res_stat(A_STR);
            do_res_stat(A_INT);
            do_res_stat(A_WIS);
            do_res_stat(A_DEX);
            do_res_stat(A_CON);
            do_res_stat(A_CHR);
            restore_level();
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_healing1_spell(int cmd, variant *res) { _psionic_healing_spell(1, cmd, res); }
static void _psionic_healing2_spell(int cmd, variant *res) { _psionic_healing_spell(2, cmd, res); }
static void _psionic_healing3_spell(int cmd, variant *res) { _psionic_healing_spell(3, cmd, res); }
static void _psionic_healing4_spell(int cmd, variant *res) { _psionic_healing_spell(4, cmd, res); }
static void _psionic_healing5_spell(int cmd, variant *res) { _psionic_healing_spell(5, cmd, res); }

/* Psionic Protection */
static void _psionic_protection_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
    {
        const cptr _names[_MAX_POWER] = {
            "Resist Fire and Cold", "Resist Environment", "Resistance", "Elemental Protection", "Immunity"};
        var_set_string(res, _names[power-1]);
        break;
    }
    case SPELL_DESC:
    {
        const cptr _descriptions[_MAX_POWER] = {
            "Gain temporary resistance to fire and cold.",
            "Gain temporary resistance to fire, cold and lightning.",
            "Gain temporary resistance to fire, cold, lightning, acid and poison.",
            "Gain temporary resistance to fire, cold, lightning, acid and poison. Gain temporary elemental auras.",
            "Gain temporary immunity to the element of your choice."
            };
        var_set_string(res, _descriptions[power-1]);
        break;
    }
    case SPELL_CAST:
    {
        int dur = spell_power(10*power + 25);
        var_set_bool(res, FALSE);
        if (power >= 5)
        {
            if (!choose_ele_immune(dur)) return;
        }
        else
        {
            set_oppose_fire(dur, FALSE);
            set_oppose_cold(dur, FALSE);
            if (power >= 2) 
                set_oppose_elec(dur, FALSE);
            if (power >= 3) 
            {
                set_oppose_acid(dur, FALSE);
                set_oppose_pois(dur, FALSE);
            }
            if (power >= 4) 
                set_tim_sh_elements(dur, FALSE);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_protection1_spell(int cmd, variant *res) { _psionic_protection_spell(1, cmd, res); }
static void _psionic_protection2_spell(int cmd, variant *res) { _psionic_protection_spell(2, cmd, res); }
static void _psionic_protection3_spell(int cmd, variant *res) { _psionic_protection_spell(3, cmd, res); }
static void _psionic_protection4_spell(int cmd, variant *res) { _psionic_protection_spell(4, cmd, res); }
static void _psionic_protection5_spell(int cmd, variant *res) { _psionic_protection_spell(5, cmd, res); }

/* Psionic Seeing */
static void _psionic_seeing_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
    {
        const cptr _names[_MAX_POWER] = {
            "Detect Monsters", "and Traps, Objects", "and Surroundings", "and Telepathy", "and Clairvoyance"};
        var_set_string(res, _names[power-1]);
        break;
    }
    case SPELL_DESC:
    {
        const cptr _descriptions[_MAX_POWER] = {
            "Detects monsters.", 
            "Detects monsters, doors, stairs, traps and objects.",
            "Detects monsters, doors, stairs, traps and objects. Maps nearby area.",
            "Detects monsters, doors, stairs, traps and objects. Maps nearby area and grants temporary telepathy.",
            "Detects monsters, doors, stairs, traps and objects. Maps entire level and grants temporary telepathy.",
            };
        var_set_string(res, _descriptions[power-1]);
        break;
    }
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (power >= 4)
            set_tim_esp(spell_power(randint1(30) + 25), FALSE);

        if (power >= 5)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
        }
        else if (power >= 3)
            map_area(DETECT_RAD_MAP);

        if (power >= 2)
        {
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            detect_objects_normal(DETECT_RAD_DEFAULT);
        }

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_seeing1_spell(int cmd, variant *res) { _psionic_seeing_spell(1, cmd, res); }
static void _psionic_seeing2_spell(int cmd, variant *res) { _psionic_seeing_spell(2, cmd, res); }
static void _psionic_seeing3_spell(int cmd, variant *res) { _psionic_seeing_spell(3, cmd, res); }
static void _psionic_seeing4_spell(int cmd, variant *res) { _psionic_seeing_spell(4, cmd, res); }
static void _psionic_seeing5_spell(int cmd, variant *res) { _psionic_seeing_spell(5, cmd, res); }

/* Psionic Shielding */
static void _psionic_shielding_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Shielding %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "You gain physical protection from your mental fortitude.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("AC: +%d", 15*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_SHIELDING])
        {
            msg_print("You already have a psionic shield.");
            return;
        }
        msg_print("You create a psionic shield.");
        p_ptr->magic_num1[_SHIELDING] = spell_power(power*8 + 20);
        p_ptr->magic_num2[_SHIELDING] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_shielding1_spell(int cmd, variant *res) { _psionic_shielding_spell(1, cmd, res); }
static void _psionic_shielding2_spell(int cmd, variant *res) { _psionic_shielding_spell(2, cmd, res); }
static void _psionic_shielding3_spell(int cmd, variant *res) { _psionic_shielding_spell(3, cmd, res); }
static void _psionic_shielding4_spell(int cmd, variant *res) { _psionic_shielding_spell(4, cmd, res); }
static void _psionic_shielding5_spell(int cmd, variant *res) { _psionic_shielding_spell(5, cmd, res); }

/* Psionic Speed */
static void _psionic_speed_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Speed %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "You focus your mental energy on quickness of motion.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Speed: +%d", 4*power));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->magic_num1[_SPEED])
        {
            msg_print("You are already fast.");
            return;
        }
        msg_print("You gain psionic speed.");
        p_ptr->magic_num1[_SPEED] = spell_power(power*10 + 20);
        p_ptr->magic_num2[_SPEED] = power;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_speed1_spell(int cmd, variant *res) { _psionic_speed_spell(1, cmd, res); }
static void _psionic_speed2_spell(int cmd, variant *res) { _psionic_speed_spell(2, cmd, res); }
static void _psionic_speed3_spell(int cmd, variant *res) { _psionic_speed_spell(3, cmd, res); }
static void _psionic_speed4_spell(int cmd, variant *res) { _psionic_speed_spell(4, cmd, res); }
static void _psionic_speed5_spell(int cmd, variant *res) { _psionic_speed_spell(5, cmd, res); }

/* Psionic Storm */
static void _psionic_storm_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Storm %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of psionic energy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(power*96 + 4)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball_aux(
            GF_PSI_STORM, 
            dir, 
            spell_power(power*96 + 4),
            2 + power/5,
            0
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_storm1_spell(int cmd, variant *res) { _psionic_storm_spell(1, cmd, res); }
static void _psionic_storm2_spell(int cmd, variant *res) { _psionic_storm_spell(2, cmd, res); }
static void _psionic_storm3_spell(int cmd, variant *res) { _psionic_storm_spell(3, cmd, res); }
static void _psionic_storm4_spell(int cmd, variant *res) { _psionic_storm_spell(4, cmd, res); }
static void _psionic_storm5_spell(int cmd, variant *res) { _psionic_storm_spell(5, cmd, res); }

/* Psionic Travel */
static void _psionic_travel_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
    {
        const cptr _names[_MAX_POWER] = {
            "Phase Door", "Portal", "Teleport", "Dimension Door", "Psionic Portal"};
        var_set_string(res, _names[power-1]);
        break;
    }
    case SPELL_DESC:
    {
        const cptr _descriptions[_MAX_POWER] = {
            "Short-range teleport.", "Medium-range teleport.", "Long-range teleport.",
            "Teleport to specified location.", "Teleport to specified location."};
        var_set_string(res, _descriptions[power-1]);
        break;
    }
    case SPELL_INFO:
        if (power == 1)
            var_set_string(res, info_range(10));
        else if (power == 2)
            var_set_string(res, info_range(25 + p_ptr->lev / 2));
        else if (power == 3)
            var_set_string(res, info_range(p_ptr->lev * 4));
        else
            var_set_string(res, info_range(15*(power - 3)));
        break;
    case SPELL_CAST:
        var_set_bool(res, TRUE);

        if (power == 1)
            teleport_player(10, 0L);
        else if (power == 2)
            teleport_player(25 + p_ptr->lev/2, 0L);
        else if (power == 3)
            teleport_player(p_ptr->lev * 4, 0L);
        else
            var_set_bool(res, dimension_door(15*(power-3)));

        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
            var_set_int(res, 30);
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_travel1_spell(int cmd, variant *res) { _psionic_travel_spell(1, cmd, res); }
static void _psionic_travel2_spell(int cmd, variant *res) { _psionic_travel_spell(2, cmd, res); }
static void _psionic_travel3_spell(int cmd, variant *res) { _psionic_travel_spell(3, cmd, res); }
static void _psionic_travel4_spell(int cmd, variant *res) { _psionic_travel_spell(4, cmd, res); }
static void _psionic_travel5_spell(int cmd, variant *res) { _psionic_travel_spell(5, cmd, res); }

/* Psionic Wave */
static void _psionic_wave_spell(int power, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, format("Mind Wave %s", _roman_numeral[power]));
        break;
    case SPELL_DESC:
        var_set_string(res, "Inflict mental damage on all visible monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(power*42)));
        break;
    case SPELL_CAST:
        project_hack(GF_PSI_STORM, spell_power(power*42));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _psionic_wave1_spell(int cmd, variant *res) { _psionic_wave_spell(1, cmd, res); }
static void _psionic_wave2_spell(int cmd, variant *res) { _psionic_wave_spell(2, cmd, res); }
static void _psionic_wave3_spell(int cmd, variant *res) { _psionic_wave_spell(3, cmd, res); }
static void _psionic_wave4_spell(int cmd, variant *res) { _psionic_wave_spell(4, cmd, res); }
static void _psionic_wave5_spell(int cmd, variant *res) { _psionic_wave_spell(5, cmd, res); }

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/

typedef struct {
    int cost;
    int fail;
    ang_spell fn;
} _spell_info_t;

typedef struct {
    cptr name;
    int id;
    int level;
    _spell_info_t info[_MAX_POWER];
    cptr desc;
} _spell_t, *_spell_ptr;

/* Here are the unique spell ids, which can never change */
enum {
    _PSION_MANA_THRUST = 0,
    _PSION_ENERGY_BLAST,
    _PSION_SEEING,
    _PSION_GRAFT_WEAPON,
    _PSION_CLARITY,
    _PSION_BLENDING,
    _PSION_SHIELDING,
    _PSION_TRAVEL,
    _PSION_PROTECTION,
    _PSION_COMBAT_TRANSFORMATION,
    _PSION_ARCHERY_TRANSFORMATION,
    _PSION_EGO_WHIP,
    _PSION_SPEED,
    _PSION_HEALING,
    _PSION_BRAIN_SMASH,
    _PSION_CRAFTING,
    _PSION_STORM,
    _PSION_BACKLASH,
    _PSION_FORTRESS,
    _PSION_MINDSPRING,
    _PSION_FORESIGHT,
    _PSION_DISRUPTION,
    _PSION_DRAIN,
    _PSION_WAVE,
};

/* Here are the spells: Use _get_spell(id) to find the correct spell. */
static _spell_t __spells[] = 
{
    { "Mana Thrust", _PSION_MANA_THRUST, 1, {  
        {  1,  20, _mana_thrust1_spell },
        {  5,  70, _mana_thrust2_spell },
        { 13, 120, _mana_thrust3_spell },
        { 25, 155, _mana_thrust4_spell },
        { 40, 180, _mana_thrust5_spell }},
        "Mana Thrust grants you an offensive ranged attack. "
          "With this power, you will have good early offense as well as the "
          "ability to scale the damage of the mana thrust quite considerably. "
          "No monster can resist the Mana Thrust; but unlike Energy Blast this "
          "attack is a bolt, so it only affects a single monster at a time and "
          "some monsters might reflect the spell back."
    },
    { "Energy Blast", _PSION_ENERGY_BLAST, 1, {  
        {  1,  20, _energy_blast1_spell },
        {  5,  70, _energy_blast2_spell },
        { 13, 120, _energy_blast3_spell },
        { 25, 155, _energy_blast4_spell },
        { 40, 180, _energy_blast5_spell }},
        "Energy Blast grants you an offensive ranged attack. "
          "With this power, you will have good early offense as well as the "
          "ability to scale the damage of the blast quite considerably. "
          "The blast will produce an elemental ball whose type is of your "
          "choosing, although the range of choices will depend on how "
          "much focus you invest in the blast. Unlike most ball spells, "
          "this spell deals full damage to all monsters in its area of effect "
          "unless resisted, without any reduction away from the epicenter."
    },
    { "Psionic Seeing", _PSION_SEEING, 1, {  
        {  1,  20, _psionic_seeing1_spell },
        {  7,  50, _psionic_seeing2_spell },
        { 15, 100, _psionic_seeing3_spell },
        { 25, 130, _psionic_seeing4_spell },
        { 50, 180, _psionic_seeing5_spell }},
        "Psionic Seeing grants you considerable powers of detection. "
          "Depending on how hard you focus, you may detect monsters, traps, "
          "doors, stairs, and objects; with enough concentration you can "
          "map your surroundings, gain temporary powers of telepathy, or even "
          "map the entire level!"
    },
    { "Graft Weapon", _PSION_GRAFT_WEAPON, 1, {  
        {  5,  20, _graft_weapon1_spell },
        { 15,  70, _graft_weapon2_spell },
        { 30, 120, _graft_weapon3_spell },
        { 50, 155, _graft_weapon4_spell },
        { 75, 180, _graft_weapon5_spell }},
        "Weapon Grafting is a power unique to the psion. Invoking this power "
        "fuses your current weapon to your arm for a limited time; in a sense, "
        "your weapon becomes an extension of you and may be wielded much more "
        "effectively. However, while this spell is active, you will not be "
        "able to remove your weapon."
    },
    { "Psionic Clarity", _PSION_CLARITY, 1, {  
        {  6,  50, _psionic_clarity1_spell },
        { 18,  70, _psionic_clarity2_spell },
        { 36, 120, _psionic_clarity3_spell },
        { 60, 160, _psionic_clarity4_spell },
        { 90, 200, _psionic_clarity5_spell }},
        "Psionic Clarity focuses the mind of the psion. While only active for "
        "a short while, this power lowers the casting costs of all other psionic "
        "powers, which can be quite useful later in the game; take it as an early "
        "option if you already have an eye on the endgame and have no need for more "
        "immediate returns."
    },
    { "Psionic Blending", _PSION_BLENDING, 10, {  
        {  4,  40, _psionic_blending1_spell },
        { 12,  55, _psionic_blending2_spell },
        { 24,  70, _psionic_blending3_spell },
        { 40,  85, _psionic_blending4_spell },
        { 60, 100, _psionic_blending5_spell }},
        "Psionic Blending grants great powers of stealth. While active, you "
        "will be able to blend in with your surroundings and sneak up on many foes. "
        "With maximal focus, you may even suppress aggravation (from your equipment), "
        "though your stealth will still be somewhat disrupted."
    },
    { "Psionic Shielding", _PSION_SHIELDING, 10, {  
        {  7,  40, _psionic_shielding1_spell },
        { 21,  60, _psionic_shielding2_spell },
        { 42,  80, _psionic_shielding3_spell },
        { 70, 100, _psionic_shielding4_spell },
        {105, 125, _psionic_shielding5_spell }},
        "Psionic Shielding is a defensive power. While active you will gain Free Action "
          "as well as increased Armor Class; Free Action provides resistance to deadly "
          "paralysation attacks, while Armor Class makes monster melee attacks much "
          "more likely to miss and also reduces damage from some melee attack hits."
    },
    { "Psionic Travel", _PSION_TRAVEL, 10, {  
        {  2,  25, _psionic_travel1_spell },
        {  8,  35, _psionic_travel2_spell },
        { 10,  50, _psionic_travel3_spell },
        { 42, 128, _psionic_travel4_spell },
        { 54, 170, _psionic_travel5_spell }},
        "Psionic Travel grants a wide array of teleportation powers. From inexpensive short-range "
          "'blinking' to long-range escapes, you will have it all. Indeed, with great "
          "focus you will even be able to control your teleportation and choose where you land!"
    },
    { "Psionic Protection", _PSION_PROTECTION, 20, {  
        {  5,  25, _psionic_protection1_spell },
        { 10,  35, _psionic_protection2_spell },
        { 20,  55, _psionic_protection3_spell },
        { 30,  75, _psionic_protection4_spell },
        { 70, 125, _psionic_protection5_spell }},
        "Psionic Protection is a defensive power. While active you will gain resistance to "
          "the elements (Fire, Cold, Lightning, Acid and Poison). The more of your mental "
          "focus you devote to this power, the more of these elements you will resist."
    },
    { "Combat Transformation", _PSION_COMBAT_TRANSFORMATION, 20, {  
        { 13,  50, _combat_transformation1_spell },
        { 47,  65, _combat_transformation2_spell },
        { 86,  80, _combat_transformation3_spell },
        {130,  95, _combat_transformation4_spell },
        {195, 110, _combat_transformation5_spell }},
        "Combat Transformation is an offensive power, channelling your mental focus into "
          "enhanced melee fighting. While active, your skills in combat will improve, affecting "
          "the accuracy of your blows; also, your reflexes will quicken in line with your mental "
          "acuity, affecting the speed of your attacks. This power requires your constant "
          "focus, and so increases the casting costs of all other psionic powers."
    },
    { "Archery Transformation", _PSION_ARCHERY_TRANSFORMATION, 20, {  
        { 13,  50, _archery_transformation1_spell },
        { 49,  65, _archery_transformation2_spell },
        { 78,  80, _archery_transformation3_spell },
        {130,  95, _archery_transformation4_spell },
        {195, 110, _archery_transformation5_spell }},
        "Archery Transformation is an offensive power, channelling your mental focus into "
          "enhanced shooting. While active, your skills with all missile weapons will improve, "
          "affecting the accuracy of your shots; sufficient skill also allows you to shoot "
          "with increased speed. This power requires your constant focus, and thus "
          "increases the casting costs of all other psionic powers."
    },
    { "Ego Whip", _PSION_EGO_WHIP, 20, {  
        {  6,  40, _ego_whip1_spell },
        { 18,  65, _ego_whip2_spell },
        { 36,  80, _ego_whip3_spell },
        { 60,  95, _ego_whip4_spell },
        { 90, 110, _ego_whip5_spell }},
        "Ego Whip is an offensive power which lashes a targetted monster repeatedly with your "
          "psychic energy; the effect lasts for multiple rounds, during which time you may "
          "perform other separate actions. However, monsters do get a saving throw against "
          "the ego whip every turn, and if they make a save, they are able to shake off the "
          "whip completely."
    },
    { "Psionic Speed", _PSION_SPEED, 30, {  
        {  6,  40, _psionic_speed1_spell },
        { 18,  65, _psionic_speed2_spell },
        { 36,  80, _psionic_speed3_spell },
        { 60,  95, _psionic_speed4_spell },
        { 90, 110, _psionic_speed5_spell }},
        "Psionic Speed channels your mental energy into speed of motion, granting great "
          "powers of haste. With increased focus comes increased speed, and the total "
          "amount of haste can greatly exceed what is possible for other classes."
    },
    { "Psionic Healing", _PSION_HEALING, 30, {  
        {  7,  40, _psionic_healing1_spell }, /*  70hp */
        { 21,  60, _psionic_healing2_spell }, /* 190hp */
        { 42,  80, _psionic_healing3_spell }, /* 310hp */
        { 69, 100, _psionic_healing4_spell }, /* 430hp */
        {102, 125, _psionic_healing5_spell }},/* 550hp */
        "Psionic Healing is a recovery spell. By focusing your mind, you will be able "
          "to heal your wounds, cuts, stunning and poisoning; with "
          "total focus, you can even restore your stats."
    },
    { "Brain Smash", _PSION_BRAIN_SMASH, 30, {  
        { 10,  60, _brain_smash1_spell },
        { 20,  75, _brain_smash2_spell },
        { 40,  90, _brain_smash3_spell },
        { 70, 105, _brain_smash4_spell },
        {100, 120, _brain_smash5_spell }},
        "Brain Smash is an offensive spell. Although it does no physical damage, it inflicts "
          "a powerful mental attack on your foes which may confuse, stun or slow them."
    },
    { "Mind Wave", _PSION_WAVE, 30, {  
        { 10,  60, _psionic_wave1_spell }, /*  42hp */
        { 20,  75, _psionic_wave2_spell }, /*  84hp */
        { 40,  90, _psionic_wave3_spell }, /* 126hp */
        { 70, 105, _psionic_wave4_spell }, /* 168hp */
        {100, 125, _psionic_wave5_spell }},/* 210hp */
        "Mind Wave unleashes the effects of your mental focus on all visible monsters as psionic energy, "
        "dealing unresistable raw damage and potentially stunning, confusing, frightening and paralysing "
        "monsters; as the attack operates directly against monsters' minds, these side effects can sometimes bypass "
        "normal resistances, but mindless monsters are not affected at all. This spell does not pack quite as "
        "much raw power as Psionic Storm, but the lower cost and ability to affect more monsters compensate for this."
    },
    { "Psionic Crafting", _PSION_CRAFTING, 40, {  
        { 10,  50, _psionic_crafting1_spell },
        { 30,  65, _psionic_crafting2_spell },
        { 60,  80, _psionic_crafting3_spell },
        {100,  95, _psionic_crafting4_spell },
        {150, 110, _psionic_crafting5_spell }},
        "Psionic Crafting channels your mental focus into an object, enchanting it in the "
          "process. With maximal focus, you can even craft excellent items!"
    },
    { "Psionic Storm", _PSION_STORM, 40, {  
        { 12,  50, _psionic_storm1_spell }, /* 100hp */
        { 35,  65, _psionic_storm2_spell }, /* 196hp */
        { 65,  80, _psionic_storm3_spell }, /* 292hp */
        {100,  95, _psionic_storm4_spell }, /* 388hp */
        {135, 110, _psionic_storm5_spell }},/* 484hp */
        "Psionic Storm unleashes your mental focus in a large, powerful ball of energy; "
        "this psionic energy not only deals unresistable damage, but can stun, confuse, "
        "frighten and even paralyze monsters, with a chance to bypass normal resistances. "
        "Mindless monsters are not affected."
    },
    { "Psionic Backlash", _PSION_BACKLASH, 40, {  
        { 24,  50, _psionic_backlash1_spell },
        { 40,  65, _psionic_backlash2_spell },
        { 60,  80, _psionic_backlash3_spell },
        { 90,  95, _psionic_backlash4_spell },
        {130, 110, _psionic_backlash5_spell }},
        "Psionic Backlash is a defensive spell. While active, any enemy that damages you will "
          "take a proportional amount of damage in retaliation; the greater your focus, the "
          "greater the retaliatory damage."
    },
    { "Psychic Drain", _PSION_DRAIN, 40, {  
        { 24,  50, _psionic_drain1_spell },
        { 40,  65, _psionic_drain2_spell },
        { 60,  80, _psionic_drain3_spell },
        { 90,  95, _psionic_drain4_spell },
        {130, 110, _psionic_drain5_spell }},
        "Psychic Drain allows you to draw mental energy and focus from the magic around you. "
        "Whenever you are hit by a magic spell you will convert some of the damage into mana. This "
        "power has no effect on non-magical damage like breaths, rockets or melee."
    },
    { "Psionic Disruption", _PSION_DISRUPTION, 50, {  
        { 40,  40, _psionic_disruption1_spell },
        {120,  55, _psionic_disruption2_spell },
        {240,  70, _psionic_disruption3_spell },
        {400,  85, _psionic_disruption4_spell },
        {600, 100, _psionic_disruption5_spell }},
        "Psionic Disruption allows you to block the minds of others, hindering their ability "
        "to cast spells. But be warned: innate monster attacks (such as breaths, rockets and "
        "boulder throws) will not be affected!"
    },
    { "Mental Fortress", _PSION_FORTRESS, 50, {  
        { 40,  40, _mental_fortress1_spell },
        {120,  55, _mental_fortress2_spell },
        {240,  70, _mental_fortress3_spell },
        {400,  85, _mental_fortress4_spell },
        {600, 100, _mental_fortress5_spell }},
        "Mental Fortress grants immunity to Dispel Magic and Anti-Magic. In addition, it "
          "increases the power of your spells."
    },
    { "Mindspring", _PSION_MINDSPRING, 50, {  
        { 40,  40, _mindspring1_spell },
        {120,  55, _mindspring2_spell },
        {240,  70, _mindspring3_spell },
        {400,  85, _mindspring4_spell },
        {600, 100, _mindspring5_spell }},
        "Mindspring greatly enhances your mana recovery."
    },
    { "Psionic Foresight", _PSION_FORESIGHT, 50, {  
        { 40,  40, _psionic_foresight1_spell },
        {120,  55, _psionic_foresight2_spell },
        {240,  70, _psionic_foresight3_spell },
        {400,  85, _psionic_foresight4_spell },
        {600, 100, _psionic_foresight5_spell }},
        "Psionic Foresight allows you to see into the future. With knowledge "
          "of events before they take place, you will be able to avoid many attacks "
          "altogether!"
    },
    { 0 }
};

static _spell_ptr _get_spell(int id)
{
    int i;
    for (i = 0; ; i++)
    {
        _spell_ptr current = &__spells[i];
        if (!current->level)
            break;
        if (current->id == id)
            return current;
    }
    msg_format("Software Bug: Invalid psionic spell id = %d.", id);
    return &__spells[0];
}

static int _num_spells_learned(void)
{
    int i;
    for (i = 0; i < 64; i++) 
    {
        if (p_ptr->spell_order[i] == 99) break;
    }
    return i;
}

static bool _spell_is_known(int idx)
{
    int i;
    for (i = 0; i < 64; i++) 
    {
        if (p_ptr->spell_order[i] == idx) return TRUE;
        if (p_ptr->spell_order[i] == 99) break;
    }
    return FALSE;
}

typedef struct {
    int lvl;
    int color;
} _spell_rank_t;
static _spell_rank_t _spell_ranks[] = {
    {  1, TERM_WHITE },
    { 10, TERM_L_WHITE },
    { 15, TERM_L_UMBER },
    { 20, TERM_YELLOW },
    { 30, TERM_ORANGE },
    { 35, TERM_L_RED },
    { 40, TERM_RED },
    { 50, TERM_VIOLET },
    {  0, TERM_WHITE },
};

static int _num_spells_allowed(void)
{
    int ct = 0, i;
    for (i = 0; ; i++)
    {
        if (_spell_ranks[i].lvl <= 0) break;
        if (p_ptr->lev >= _spell_ranks[i].lvl)
            ct++;
    }
    return ct;
}

bool _can_study(void)
{
    int num = _num_spells_allowed() - _num_spells_learned();
    if (num <= 0) return FALSE;
    return TRUE;
}

static void _study_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    int id = ((int*)cookie)[which];
    _spell_ptr spell = _get_spell(id);
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, spell->name);
        break;
    case MENU_HELP:
        var_set_string(res, spell->desc);
        break;
    case MENU_COLOR:
    {
        int lvl = spell->level;
        int i;
        var_set_int(res, TERM_WHITE);
        for (i = 0; ; i++)
        {
            if (_spell_ranks[i].lvl <= 0) break;
            if (lvl == _spell_ranks[i].lvl)
            {
                var_set_int(res, _spell_ranks[i].color);
                break;
            }
        }
        break;
    }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static void _study(int level)
{
    int choices[100];
    int i;
    int ct = 0;
    menu_t menu = { "Gain which power?", "Browse which power?", NULL,
                    _study_menu_fn, choices, 0, 0};

    for (i = 0; ; i++)
    {
        _spell_t *spell = &__spells[i];
        if (!spell->level) break;
        if (spell->level <= level && !_spell_is_known(spell->id))
        {
            choices[ct] = spell->id;
            ct++;
        }
    }

    menu.count = ct;
    for (;;)
    {
        i = menu_choose(&menu);
        if (i >= 0)
        {
            char prompt[1024];
            char desc[255*10];
            int id = choices[i];
            _spell_ptr spell = _get_spell(id);
            int j;
            cptr t;

            screen_save();
            for (j = 0; j < 10+1; j++)
                Term_erase(13, 1+j, 255);
            
            roff_to_buf(spell->desc, 80-13, desc, sizeof(desc));
            for (t = desc, j = 0; t[0]; t += strlen(t) + 1, j++)
                prt(t, 2+j, 13);

            sprintf(prompt, "You will learn %s. Are you sure?", spell->name);
            if (get_check(prompt))
            {
                screen_load();
                p_ptr->spell_order[_num_spells_learned()] = spell->id;
                p_ptr->redraw |= PR_EFFECTS;
                msg_format("You have gained %s.", spell->name);
                break;
            }
            screen_load();
        }
        msg_print("Please make a choice!");
    }
}

static void _gain_level(int new_level)
{
    while (_can_study())
        _study(new_level);
}

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    spell_info* spell = &spells[ct++];
    spell->level = 15;
    spell->cost = 0;
    spell->fail = calculate_fail_rate(spell->level, 30, _spell_stat_idx());
    spell->fn = clear_mind_spell;

    return ct;
}

static void _choose_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    _spell_ptr spell = _get_spell(p_ptr->spell_order[which]);
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, spell->name);
        break;
    case MENU_HELP:
        spell->info[0].fn(SPELL_DESC, res);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static int _choose_spell(void)
{
    int i;
    menu_t menu = { "Use which power?", "Browse which power?", NULL,
                    _choose_menu_fn, NULL, _num_spells_learned(), 0};

    i = menu_choose(&menu);
    if (i >= 0)
        i = p_ptr->spell_order[i];
    return i;
}

static int _get_spells(spell_info* spells, int max)
{
    int       i, id, stat, ct = 0;
    _spell_t *base;

    /* First Choose which Psionic Spell to use */
    id = _choose_spell();
    if (id < 0) return 0;

    stat = _spell_stat_idx();
    base = _get_spell(id);

    /* Then Choose which power level of that spell to use */
    for (i = 0; i < _MAX_POWER; i++)
    {
        if (ct >= max) break;
        if (base->level <= p_ptr->lev)
        {
            spell_info* current = &spells[ct];
            int fail = base->info[i].fail;
            int cost = base->info[i].cost;

            current->level = base->level;
            current->fn = base->info[i].fn;

            if (p_ptr->magic_num1[_CLARITY])
            {
                cost = cost * (85 - 7 * p_ptr->magic_num2[_CLARITY]) / 100;
                if (cost < 1)
                    cost = 1;
            }

            if (p_ptr->magic_num1[_COMBAT] || p_ptr->magic_num1[_ARCHERY])
            {
                cost = cost * 3 / 2;
            }

            current->cost = cost;
            current->fail = calculate_fail_rate(base->level, fail, stat);
            ct++;
        }
    }
    return ct;
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 15)
        p_ptr->clear_mind = TRUE;

    if (p_ptr->magic_num1[_BLENDING])
    {
        p_ptr->skills.stl += 3 * p_ptr->magic_num2[_BLENDING];
        if ((p_ptr->cursed & OFC_AGGRAVATE) && p_ptr->magic_num2[_BLENDING] == 5)
        {
            p_ptr->cursed &= ~(OFC_AGGRAVATE);
            p_ptr->skills.stl = MIN(p_ptr->skills.stl - 3, (p_ptr->skills.stl + 2) / 2);
        }
    }

    if (p_ptr->magic_num1[_SHIELDING])
    {
        p_ptr->free_act++;
        if (!p_ptr->shield)
        {
            p_ptr->to_a += 15 * p_ptr->magic_num2[_SHIELDING];
            p_ptr->dis_to_a += 15 * p_ptr->magic_num2[_SHIELDING];
        }
    }

    if (p_ptr->magic_num1[_COMBAT])
    {
        p_ptr->skills.thn += 15*p_ptr->magic_num2[_COMBAT];
    }

    if (p_ptr->magic_num1[_ARCHERY])
    {
        /* Note: This also increases shots per round ... cf calc_bonuses in xtra1.c */
        p_ptr->skills.thb += 20*p_ptr->magic_num2[_ARCHERY];
    }

    if (p_ptr->magic_num1[_SPEED])
    {
        if (IS_FAST())
            p_ptr->pspeed += MAX(4*p_ptr->magic_num2[_SPEED] - 10, 0);
        else
            p_ptr->pspeed += 4*p_ptr->magic_num2[_SPEED];
    }
    if (p_ptr->magic_num1[_FORTRESS])
    {
        p_ptr->spell_power += p_ptr->magic_num2[_FORTRESS];
        res_add(RES_TIME);
        p_ptr->sustain_str = TRUE;
        p_ptr->sustain_int = TRUE;
        p_ptr->sustain_wis = TRUE;
        p_ptr->sustain_dex = TRUE;
        p_ptr->sustain_con = TRUE;
        p_ptr->sustain_chr = TRUE;
        p_ptr->hold_life++;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->magic_num1[_BLENDING])
        add_flag(flgs, OF_STEALTH);
    if (p_ptr->magic_num1[_SHIELDING])
        add_flag(flgs, OF_FREE_ACT);
    if (p_ptr->magic_num1[_SPEED])
        add_flag(flgs, OF_SPEED);
    if (p_ptr->magic_num1[_FORTRESS])
    {
        add_flag(flgs, OF_SPELL_POWER);
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_SUST_STR);
        add_flag(flgs, OF_SUST_INT);
        add_flag(flgs, OF_SUST_WIS);
        add_flag(flgs, OF_SUST_DEX);
        add_flag(flgs, OF_SUST_CON);
        add_flag(flgs, OF_SUST_CHR);
        add_flag(flgs, OF_HOLD_LIFE);
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (p_ptr->magic_num1[_WEAPON_GRAFT])
    {
        info_ptr->to_h += p_ptr->magic_num2[_WEAPON_GRAFT] * 6;
        info_ptr->dis_to_h += p_ptr->magic_num2[_WEAPON_GRAFT] * 6;

        info_ptr->to_d += p_ptr->magic_num2[_WEAPON_GRAFT] * 4;
        info_ptr->dis_to_d += p_ptr->magic_num2[_WEAPON_GRAFT] * 4;
    }

    if (p_ptr->magic_num1[_COMBAT])
    {
        info_ptr->xtra_blow += p_ptr->magic_num2[_COMBAT] * (prace_is_(RACE_TONBERRY) ? 25 : 40);
    }
}

static void _decrement_counter(int which, cptr off)
{
    if (p_ptr->magic_num1[which])
    {
        p_ptr->magic_num1[which]--;
        if (!p_ptr->magic_num1[which])
        {
            p_ptr->magic_num2[which] = 0;
            msg_print(off);
            if (disturb_state) disturb(0, 0);
            p_ptr->update |= PU_BONUS;
            p_ptr->redraw |= PR_STATUS;
        }
    }
}

void psion_decrement_counters(void)
{
    if (p_ptr->pclass != CLASS_PSION) return;

    _decrement_counter(_WEAPON_GRAFT, "Your melee weapon is no longer fused to your arm.");    
    _decrement_counter(_CLARITY, "You lose your mental focus.");    
    _decrement_counter(_BLENDING, "You no longer blend into your surroundings.");    
    _decrement_counter(_SHIELDING, "Your psionic shield disappears.");    
    _decrement_counter(_COMBAT, "Your combat transformation expires.");    
    _decrement_counter(_ARCHERY, "Your archery transformation expires.");    
    _decrement_counter(_SPEED, "Your psionic speed fades.");    
    _decrement_counter(_BACKLASH, "Your mental revenge abates.");    
    _decrement_counter(_FORTRESS, "Your mental fortress collapses.");    
    _decrement_counter(_MINDSPRING, "Your mindspring dries up.");    
    _decrement_counter(_FORESIGHT, "Your foresight fades.");    
    _decrement_counter(_DISRUPTION, "Your mental disruption vanishes.");
    _decrement_counter(_DRAIN, "You no longer drain power from surrounding magics.");
}

static void _clear_counter(int which, cptr off)
{
    if (p_ptr->magic_num1[which])
    {
        p_ptr->magic_num1[which] = 0;
        p_ptr->magic_num2[which] = 0;
        msg_print(off);
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
    }
}

void psion_dispel_player(void)
{
    if (p_ptr->pclass != CLASS_PSION) return;

    _clear_counter(_WEAPON_GRAFT, "Your melee weapon is no longer fused to your arm.");    
    _clear_counter(_CLARITY, "You lose your mental focus.");    
    _clear_counter(_BLENDING, "You no longer blend into your surroundings.");    
    _clear_counter(_SHIELDING, "Your psionic shield disappears.");    
    _clear_counter(_COMBAT, "Your combat transformation expires.");    
    _clear_counter(_ARCHERY, "Your archery transformation expires.");    
    _clear_counter(_SPEED, "Your psionic speed fades.");    
    _clear_counter(_BACKLASH, "Your mental revenge abates.");    
    _clear_counter(_FORTRESS, "Your mental fortress collapses.");    
    /*_clear_counter(_MINDSPRING, "Your mindspring dries up.");    */
    _clear_counter(_FORESIGHT, "Your foresight fades.");    
    _clear_counter(_DISRUPTION, "Your mental disruption is calmed.");    
    _clear_counter(_DRAIN, "You no longer drain power from surrounding magics.");
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "focus";
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    me.which_stat = _spell_stat();
    return &me;
}

static void _character_dump(doc_ptr doc)
{
    int     i, j;
    int     stat = _spell_stat_idx();
    int     num_learned = _num_spells_learned();
    variant name, info;

    var_init(&name);
    var_init(&info);

    doc_printf(doc, "<topic:Spells>=================================== <color:keypress>S</color>pells ====================================\n");

    for (i = 0; i < num_learned; i++)
    {
        _spell_t *power = _get_spell(p_ptr->spell_order[i]);

        doc_printf(doc, "\n<color:G>%-23.23s Cost Fail %-15.15s Cast Fail</color>\n", power->name, "Info");
        for (j = 0; j < _MAX_POWER; j++)
        {
            _spell_info_t  *spell = &power->info[j];
            int             fail = spell->fail;
            int             cost = spell->cost;
            spell_stats_ptr stats = NULL;

            if (p_ptr->magic_num1[_CLARITY])
            {
                cost = cost * (85 - 7 * p_ptr->magic_num2[_CLARITY]) / 100;
                if (cost < 1)
                    cost = 1;
            }

            if (p_ptr->magic_num1[_COMBAT] || p_ptr->magic_num1[_ARCHERY])
            {
                cost = cost * 3 / 2;
            }

            fail = calculate_fail_rate(power->level, fail, stat);

            (spell->fn)(SPELL_NAME, &name);
            stats = spell_stats_aux(var_get_string(&name));

            (spell->fn)(SPELL_INFO, &info);
            doc_printf(doc, "%-23.23s %4d %3d%% %-15.15s %4d %4d %3d%%\n",
                            var_get_string(&name),
                            cost,
                            fail,
                            var_get_string(&info),
                            stats->ct_cast, stats->ct_fail,
                            spell_stats_fail(stats)
            );

        }
    }

    var_clear(&name);
    var_clear(&info);
    doc_newline(doc);
}

static void _player_action(int energy_use)
{
    psion_do_mindspring(energy_use);
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SMALL_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CLARITY, rand_range(5, 10));
}

class_t *psion_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  35,  40,   2,  16,   8,  48,  35};
    skills_t xs = {  7,  11,  12,   0,   0,   0,  13,  11};

        me.name = "Psion";
        me.desc = "The Psion is like a Mindcrafter, and uses innate mental powers. "
                    "Unlike the Mindcrafter, however, Psions have the freedom to learn "
                    "powers that enforce their own styles. They learn very few powers, "
                    "but they can scale their powers to determine the SP cost and the "
                    "powers' potency. Psionic powers require great concentration, however, "
                    "and psions do not have the mind to spare to care for others. "
                    "The Psion gains powers at the following levels: 1, 10, 15, 20, 30, 35, 40 and 50. "
                    "The Psion uses Intelligence, Wisdom or Charisma as their primary "
                    "spell stat, which ever is currently highest. In this respect, Psions "
                    "are truly unique!";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 96;
        me.base_hp = 4;
        me.exp = 150;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        me.gain_level = _gain_level;
        me.player_action = _player_action;
        init = TRUE;
    }

    return &me;
}

void psion_relearn_powers(void)
{
    int i;
    for (i = 0; i < 64; i++) 
        p_ptr->spell_order[i] = 99;

    for (i = 0; ; i++)
    {
        if (_spell_ranks[i].lvl <= 0) break;
        if (p_ptr->lev >= _spell_ranks[i].lvl)
            _study(_spell_ranks[i].lvl);
    }
}
