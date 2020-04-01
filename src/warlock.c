/****************************************************************
 * The Warlock
 ****************************************************************/

#include "angband.h"

#include <assert.h>

/****************************************************************
 * Toggles are techniques or songs that grant bonuses as long
 * as they are maintained. In addition, the warlock might pay
 * upkeep to maintain the 'toggle'.
 ****************************************************************/
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

int warlock_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (warlock_is_(WARLOCK_DRAGONS) && p_ptr->riding)
        result = _get_toggle();
    else if (p_ptr->pclass == CLASS_WARLOCK) /* In case I add toggles for other pacts ... */
        result = _get_toggle();
    return result;
}

void warlock_stop_singing(void)
{   /* Dragon Songs */
    if (warlock_is_(WARLOCK_DRAGONS))
        _set_toggle(TOGGLE_NONE);
}

/****************************************************************
 * The Eldritch Blast
 ****************************************************************/
static int _blast_range(void)
{
    int rng = 5;

    if (p_ptr->lev >= 48)
        rng = 8;
    else if (p_ptr->lev >= 32)
        rng = 7;
    else if (p_ptr->lev >= 16)
        rng = 6;

    return rng;
}

static int _blast_dd(int div)
{
    return 1 + (((p_ptr->lev * 20) + (p_ptr->lev * p_ptr->lev * 10 / 25) + (div / 2)) / div);
}

static int _blast_ds(void)
{
    static int _table[] =
    {
        4    /* 3 */,
        5    /* 4 */,
        5    /* 5 */,
        6    /* 6 */,
        6    /* 7 */,
        6    /* 8 */,
        6    /* 9 */,
        6    /* 10 */,
        6    /* 11 */,
        6    /* 12 */,
        6    /* 13 */,
        6    /* 14 */,
        7    /* 15 */,
        7    /* 16 */,
        7    /* 17 */,
        8    /* 18/00-18/09 */,
        8    /* 18/10-18/19 */,
        8    /* 18/20-18/29 */,
        8    /* 18/30-18/39 */,
        8    /* 18/40-18/49 */,
        9    /* 18/50-18/59 */,
        9    /* 18/60-18/69 */,
        10    /* 18/70-18/79 */,
        10    /* 18/80-18/89 */,
        11    /* 18/90-18/99 */,
        11    /* 18/100-18/109 */,
        12    /* 18/110-18/119 */,
        12    /* 18/120-18/129 */,
        13    /* 18/130-18/139 */,
        13    /* 18/140-18/149 */,
        14    /* 18/150-18/159 */,
        14    /* 18/160-18/169 */,
        15    /* 18/170-18/179 */,
        16    /* 18/180-18/189 */,
        17    /* 18/190-18/199 */,
        18    /* 18/200-18/209 */,
        19    /* 18/210-18/219 */,
        20    /* 18/220+ */
    };
    return _table[p_ptr->stat_ind[A_CHR]];
}

static int _power_cost(int _base, int mod)
{
    int hinta = _base + _blast_dd(mod);
    if (hinta > p_ptr->csp) hinta += (hinta - p_ptr->csp);
    return hinta;
}

static void _basic_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Eldritch Blast");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires your basic Eldritch Blast.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d (Rng %d)", _blast_dd(100), _blast_ds(), p_ptr->to_d_spell, _blast_range()));
        else
            var_set_string(res, format("%dd%d (Rng %d)", _blast_dd(100), _blast_ds(), _blast_range()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1 + (p_ptr->lev / 27), 185));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _easy_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Easy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a slightly weakened Eldritch Blast without consuming mana.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d", _blast_dd(150), _blast_ds(), p_ptr->to_d_spell, _blast_range()));
        else
            var_set_string(res, format("%dd%d", _blast_dd(150), _blast_ds(), _blast_range()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(150), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _extended_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Extended");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a slightly weakened Eldritch Blast with increased range.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d (Rng %d)", _blast_dd(100), _blast_ds() * 3 / 4, p_ptr->to_d_spell, _blast_range() + p_ptr->lev / 5));
        else
            var_set_string(res, format("%dd%d (Rng %d)", _blast_dd(100), _blast_ds() * 3 / 4, _blast_range() + p_ptr->lev / 5));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1 + (p_ptr->lev / 27), 185));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range() + p_ptr->lev/5;
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds() * 3 / 4) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spear_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Spear");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an Eldritch Beam.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_beam(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _burst_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Burst");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an Eldritch Ball with full damage within radius 2.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball_aux(
            GF_ELDRITCH,
            dir,
            spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
            2,
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

static void _stunning_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Stunning");
        break;
    case SPELL_DESC:
        var_set_string(res, "Augments your Eldritch Blast with stunning effects.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH_STUN,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _empowered_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Empowered");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a very powerful Eldritch Blast, at the cost of disabling spells next turn.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d", _blast_dd(57), _blast_ds(), p_ptr->to_d_spell));
        else
            var_set_string(res, format("%dd%d", _blast_dd(57), _blast_ds(), _blast_range));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(57), _blast_ds()) + p_ptr->to_d_spell),
                  0);
        set_tim_no_spells(p_ptr->tim_no_spells + 1 + 1, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * The Warlock Pacts: Implemented as a Subclass
 ****************************************************************/
#define _MAX_SPELLS  20

typedef struct {
    cptr name;
    cptr desc;
    cptr alliance;
    calc_bonuses_fn calc_bonuses;
    calc_weapon_bonuses_fn calc_weapon_bonuses;
    stats_fn calc_stats;
    flags_fn get_flags;
    process_player_fn process_player;
    int stats[MAX_STATS];
    skills_t base_skills;
    skills_t extra_skills;
    int life;
    int base_hp;
    int exp;
    spell_info spells[_MAX_SPELLS];        /* There is always a sentinel at the end */
    ang_spell special_blast;
} _pact_t, *_pact_ptr;

/****************************************************************
 * Warlock Pact: Undead
 ****************************************************************/
static void _undead_calc_bonuses(void)
{
    p_ptr->align -= 200;
    res_add(RES_COLD);
    if (p_ptr->lev >= 15)
    {
        p_ptr->see_inv++;
        res_add(RES_POIS);
    }
    if (p_ptr->lev >= 30)
    {
        res_add(RES_NETHER);
        p_ptr->hold_life++;
    }
    if (p_ptr->lev >= 35)
        res_add(RES_DARK);
}

static void _undead_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_WIS] -= 3 * p_ptr->lev/50;
    stats[A_CON] += 3 * p_ptr->lev/50;
}

static void _undead_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_COLD);
    if (p_ptr->lev >= 15)
    {
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_SEE_INVIS);
    }
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_HOLD_LIFE);
    }
    if (p_ptr->lev >= 35)
        add_flag(flgs, OF_RES_DARK);
}

static void _draining_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Draining");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an Eldritch Blast which also does Drain Life.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH_DRAIN,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _undead_pact = {
  "Undead",
  "The undead are tough creatures, each having already survived death at least once. Warlocks who make a pact "
      "with Undead will find themselves gaining additional resistances such as cold, poison, darkness and "
      "nether. In addition, they will be able to see invisible monsters and gain resistance to "
      "life-draining forces. They will also gain access to undeadly powers, which typically "
      "involve detection and control over the forces of the netherworld. Eventually, this pact will allow "
      "its practitioners to destroy monsters with a single word, and, perhaps, to even partially leave "
      "the world of the living altogether. Of course, allying with the undead substantially reduces the "
      "warlock's ability to fight these creatures.",
  "", /* RF3_UNDEAD suffices */
  _undead_calc_bonuses,
  NULL,
  _undead_calc_stats,
  _undead_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  {-1,  2, -3,  0,  2,  2},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  40,  40,   4,  16,  20, 48, 35},
  {   8,  15,  12,   0,   0,   0, 13, 11},
/*Life  BaseHP     Exp */
   107,     15,    135,
  {
    {  1,   1, 30, detect_life_spell},
    {  3,   2, 30, detect_unlife_spell},
    {  5,   3, 30, scare_spell},
    {  7,   5, 50, nether_bolt_spell},
    { 10,   7, 60, satisfy_hunger_spell},
    { 15,  10, 60, animate_dead_spell},
    { 20,  20, 60, restore_life_spell},
    { 22,  10, 50, enslave_undead_spell},
    { 25,  15, 40, nether_ball_spell},
    { 32,  40, 60, summon_undead_spell},
    { 35,  30, 70, darkness_storm_I_spell},
    { 37,  50, 80, polymorph_vampire_spell},
    { 40,  70, 80, summon_hi_undead_spell},
    { 42,  50, 80, genocide_spell},
    { 50, 100, 80, wraithform_spell},
    { -1,   0,  0, NULL },
  },
  _draining_blast
};

/****************************************************************
 * Warlock Pact: Dragons
 * The main bonus of this pact is that they are Dragonriders!
 ****************************************************************/
static monster_type *_get_mount(void)
{
    monster_type *result = NULL;
    if (p_ptr->riding)
        result = &m_list[p_ptr->riding];
    return result;
}

static bool _is_lance(object_type *o_ptr)
{
    return object_is_(o_ptr, TV_POLEARM, SV_LANCE)
        || object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE);
}

static void _dragon_calc_bonuses(void)
{
    res_add(RES_FEAR);
    if (p_ptr->lev >= 30)
        p_ptr->sustain_str = TRUE;
}

static void _dragon_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if ( _get_toggle() == WARLOCK_DRAGON_TOGGLE_HEROIC_CHARGE
      && p_ptr->riding
      && _is_lance(o_ptr) )
    {
        info_ptr->to_dd += 2;
    }
}

static void _dragon_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_STR] += 3 * p_ptr->lev/50;
}

static void _dragon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_SUST_STR);
}

static void _dragon_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Dragon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes your eldritch blast at a chosen foe.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1 + (p_ptr->lev / 27), 185));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dice = _blast_dd(100);
        int sides = _blast_ds();

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball_aux(GF_ELDRITCH, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell), -1 - (p_ptr->lev / 20), PROJECT_FULL_DAM);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/* Dragon Spells */
static void _dragon_lore_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon's Lore");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

static void _dragon_eye_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon's Eye");
        break;
    default:
        telepathy_spell(cmd, res);
        break;
    }
}

static void _understanding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Understanding");
        break;
    default:
        probing_spell(cmd, res);
        break;
    }
}

static void _word_of_command_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Word of Command");
        break;
    case SPELL_DESC:
        var_set_string(res, "By uttering a word of obediance, the true dragon rider can bend the will of all but the mightiest serpents.");
        break;
    case SPELL_CAST:                        /*v-- This is meaningless, but set high enough so that the project code actually works */
        project_hack(GF_CONTROL_PACT_MONSTER, 100);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/* Dragon Songs */
static void _dragon_upkeep_song(void)
{
    int           cost = 0;
    monster_type *mount = _get_mount();

    /* Check if the player got thrown, dismounted, or their steed was slain */
    if (!mount)
    {
        _set_toggle(TOGGLE_NONE);
        return;
    }

    switch (_get_toggle())
    {
    case WARLOCK_DRAGON_TOGGLE_BLESS:
        cost = 1;
        break;
    case WARLOCK_DRAGON_TOGGLE_CANTER:
        cost = 2;
        break;
    case WARLOCK_DRAGON_TOGGLE_GALLOP:
        cost = 5;
        break;
    case WARLOCK_DRAGON_TOGGLE_HEALING:
        cost = p_ptr->lev/2;
        break;
    case WARLOCK_DRAGON_TOGGLE_HEROIC_CHARGE:
        cost = p_ptr->lev/2;
        break;
    }
    if (cost > p_ptr->csp)
    {
        msg_print("You can no longer maintain the song.");
        _set_toggle(TOGGLE_NONE);
    }
    else
    {
        sp_player(-cost);

        /* Apply the Song. Most songs are handled in calc_bonuses(), or
           perhaps scattered throughout the code base as 'hacks' */
        if (_get_toggle() == WARLOCK_DRAGON_TOGGLE_HEALING)
        {
            hp_player(p_ptr->lev);
            if (mount->hp < mount->maxhp)
            {
                int heal = MIN(p_ptr->lev*3, mount->maxhp - mount->hp);
                mount->hp += heal;
            }
        }
        else if (_get_toggle() == WARLOCK_DRAGON_TOGGLE_HEROIC_CHARGE)
        {
            char m_name[MAX_NLEN];
            monster_desc(m_name, mount, 0);
            if (MON_STUNNED(mount))
            {
                msg_format("%^s is no longer stunned.", m_name);
                set_monster_stunned(p_ptr->riding, 0);
            }
            if (MON_CONFUSED(mount))
            {
                msg_format("%^s is no longer confused.", m_name);
                set_monster_confused(p_ptr->riding, 0);
            }
            if (MON_MONFEAR(mount))
            {
                msg_format("%^s is not longer afraid.", m_name);
                set_monster_monfear(p_ptr->riding, 0);
            }
        }
    }
}

static void _dragon_song(int which, cptr desc, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_get_mount())
        {
            msg_print("You may only sing when riding.");
            return;
        }
        if (_get_toggle() == which)
        {
            msg_format("You stop singing %s.", desc);
            _set_toggle(TOGGLE_NONE);
        }
        else
        {
            msg_format("You begin singing %s.", desc);
            _set_toggle(which);
        }
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);    /* no charge for dismissing a song */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


static void _bless_song(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Song of Blessing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Adagio. When sung, both you and your steed will gain enhanced melee skill.");
        break;
    default:
        _dragon_song(WARLOCK_DRAGON_TOGGLE_BLESS, "of heroic deeds", cmd, res);
        break;
    }
}

static void _canter_song(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Joyful Song");
        break;
    case SPELL_DESC:
        var_set_string(res, "Allegro. This is a pleasing melody, and your steed will prance along in time with the song.");
        break;
    default:
        _dragon_song(WARLOCK_DRAGON_TOGGLE_CANTER, "a joyful, upbeat melody", cmd, res);
        break;
    }
}

static void _gallop_song(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Song of Haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "Presto. A quick beat marks this song of urgency.");
        break;
    default:
        _dragon_song(WARLOCK_DRAGON_TOGGLE_GALLOP, "a spurring melody", cmd, res);
        break;
    }
}

static void _healing_song(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Song of Life");
        break;
    case SPELL_DESC:
        var_set_string(res, "Largo. Slowly and majestically, both you and your mount feel the life giving effects of this powerful ballad.");
        break;
    default:
        _dragon_song(WARLOCK_DRAGON_TOGGLE_HEALING, "a rejuvenating melody", cmd, res);
        break;
    }
}

static void _heroic_charge_song(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Song of War");
        break;
    case SPELL_DESC:
        var_set_string(res, "Molto allegro. A song of defiance, its melody captures the essence of "
                            "the dragon's lust for treasure and conquest. Woe be to all that hear "
                            "this ancient dragon song.");
        break;
    default:
        _dragon_song(WARLOCK_DRAGON_TOGGLE_HEROIC_CHARGE, "of battle and riches", cmd, res);
        break;
    }
}

/* Riding Techniques */
static void _mount_jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Jump");
        break;
    case SPELL_CAST:
        if (!p_ptr->riding)
        {
            msg_print("This is a riding technique. Where is your dragon?");
            var_set_bool(res, FALSE);
            return;
        }
    default:
        jump_spell(cmd, res);
        break;
    }
}

static void _mount_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Guided Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Guide your dragon to attack a chosen foe.");
        break;
    case SPELL_CAST:
    {
        monster_type *mount = _get_mount();
        int x = 0, y = 0;
        int dir;
        int m_idx = 0;

        var_set_bool(res, FALSE);
        if (!mount)
        {
            msg_print("This is a riding technique. Where is your dragon?");
            return;
        }

        if (mount->energy_need > 300)
        {
            msg_print("You sense your dragon is too tired for another attack.");
            return;
        }

        if (old_target_okay())
        {
            y = target_row;
            x = target_col;
            m_idx = cave[y][x].m_idx;
            if (m_idx)
            {
                if (m_list[m_idx].cdis > 1)
                    m_idx = 0;
                else
                    dir = 5;
            }
        }

        if (!m_idx)
        {
            if (!get_rep_dir2(&dir)) return;
            if (dir == 5) return;

            y = py + ddy[dir];
            x = px + ddx[dir];
            m_idx = cave[y][x].m_idx;

            if (!m_idx)
            {
                msg_print("There is no monster there.");
                if (p_ptr->blind > 0) var_set_bool(res, TRUE);
                return;
            }
        }

        if (m_idx)
        {
            mon_attack_mon(p_ptr->riding, m_idx);
            mount->energy_need += ENERGY_NEED();
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _hack_dir;
static bool _dragonrider_ai(mon_spell_cast_ptr cast)
{
    mon_spells_ptr spells = cast->race->spells;
    mon_spell_group_ptr group;

    /* steel dragons? */
    if (!spells) return FALSE; 
    if (!spells->groups[MST_BREATH]) return FALSE;

    if (_hack_dir == 5)
    {
        cast->dest = point(target_col, target_row);
        if (target_who > 0)
        {
            char tmp[MAX_NLEN];
            cast->mon2 = &m_list[target_who];
            monster_desc(tmp, cast->mon2, 0);
            tmp[0] = toupper(tmp[0]);
            sprintf(cast->name2, "<color:o>%s</color>", tmp);
        }
    }
    else
    {
        cast->dest.x = px + 99 * ddx[_hack_dir];
        cast->dest.y = py + 99 * ddy[_hack_dir];
    }

    if (!cast->mon2)
        strcpy(cast->name2, "<color:o>the ground</color>");

    group = spells->groups[MST_BREATH];
    cast->spell = &group->spells[randint0(group->count)];
    return TRUE;
}

static void _mount_breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Guided Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, "Guide your dragon to breathe at a chosen foe.");
        break;
    case SPELL_CAST:
    {
        monster_type *mount = _get_mount();

        var_set_bool(res, FALSE);
        if (!mount)
        {
            msg_print("This is a riding technique. Where is your dragon?");
            return;
        }

        if (mount->energy_need > 300)
        {
            msg_print("You sense your dragon is too tired for another attack.");
            return;
        }

        if (!get_fire_dir(&_hack_dir)) return;

        if (mon_spell_cast_mon(mount, _dragonrider_ai))
            mount->energy_need += ENERGY_NEED();

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _pets_breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragons' Fury");
        break;
    case SPELL_DESC:
        var_set_string(res, "Guide all of your pet dragons to breathe at a chosen target.");
        break;
    case SPELL_CAST:
    {
        monster_type *mount = _get_mount();
        int i;

        var_set_bool(res, FALSE);
        if (!mount)
        {
            msg_print("This is a riding technique. Where is your dragon?");
            return;
        }

        if (mount->energy_need > 300)
        {
            msg_print("You sense your dragon is too tired for another attack.");
            return;
        }

        if (!get_fire_dir(&_hack_dir)) return;

        msg_print("<color:v>Dragons: As One!!</color>");
        msg_boundary();

        if (mon_spell_cast_mon(mount, _dragonrider_ai))
        {
            mount->energy_need += ENERGY_NEED();
            msg_boundary();
        }

        for (i = m_max - 1; i >= 1; i--)
        {
            monster_type *m_ptr = &m_list[i];
            monster_race *r_ptr;

            if (!m_ptr->r_idx) continue;
            if (!is_pet(m_ptr)) continue;
            if (i == p_ptr->riding) continue;

            r_ptr = &r_info[m_ptr->r_idx];
            if (!(r_ptr->flags3 & RF3_DRAGON)) continue;

            if (mon_spell_cast_mon(m_ptr, _dragonrider_ai))
            {
                m_ptr->energy_need += ENERGY_NEED();
                if (one_in_(2))
                {
                    if (mon_show_msg(m_ptr))
                    {
                        char m_name[MAX_NLEN];
                        monster_desc(m_name, m_ptr, 0);
                        msg_format("%^s disappears!", m_name);
                    }
                    delete_monster_idx(i);
                }
                msg_boundary();
            }
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _dragons_pact = {
  "Dragons",
  "The bond between a Dragonrider and a Dragon is one of the strongest and most enduring ties "
  "seen the world over. Each draws strength, power, and encouragement from the other, and together "
  "they make a formidable foe. An alliance with Dragonkind enables this bond to form, and the Warlock "
  "of this pact gains impressive powers for mounted combat. Indeed, they are among the elite riders "
  "of the world, only slightly inferior to Cavalry and Beastmasters. But where they have a slight "
  "deficiency in skill (and perhaps in melee as well), they have a strong advantage in riding techniques "
  "and magical enhancements. Dragonriders favor the lance above all other weapons, and seek continuously "
  "the legendary 'Dragonlance' which they view as their long-lost birthright.",
  "", /* RF3_DRAGON suffices */
  _dragon_calc_bonuses,
  _dragon_calc_weapon_bonuses,
  _dragon_calc_stats,
  _dragon_get_flags,
  _dragon_upkeep_song,
/*  S   I   W   D   C   C */
  { 2,  0,  0, -1,  1,  2},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  25,  30,   1,  14,  12, 52, 35},
  {   7,  11,  10,   0,   0,   0, 14, 11},
/*Life  BaseHP     Exp */
   102,     10,    130,
  {
    {  5,   3, 40, detect_menace_spell},
    {  7,   5, 40, detect_objects_spell},
    { 15,  15, 60, _dragon_lore_spell},
    { 20,  40, 70, summon_dragon_spell},
    { 23,  10, 50, _mount_attack_spell},
    { 25,   0,  0, _bless_song},
    { 27,  15, 60, _understanding_spell},
    { 29,   0, 60, _mount_jump_spell},
    { 30,   0,  0, _canter_song},
    { 32,  20, 50, _dragon_eye_spell},
    { 35,  30, 60, _mount_breathe_spell},
    { 37,   0,  0, _gallop_song},
    { 40,  90, 80, _word_of_command_spell},
    { 42,   0,  0, _healing_song},
    { 45,   0,  0, _heroic_charge_song},
    { 50, 100, 70, _pets_breathe_spell},
    { -1,   0,  0, NULL },
  },
  _dragon_blast
};

/****************************************************************
 * Warlock Pact: Angels
 ****************************************************************/
static void _angel_calc_bonuses(void)
{
    p_ptr->align += 200;
    p_ptr->levitation = TRUE;
    if (p_ptr->lev >= 15)
        p_ptr->see_inv++;
    if (p_ptr->lev >= 35)
        p_ptr->reflect = TRUE;
}

static void _angel_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_WIS] += 3 * p_ptr->lev/50;
}

static void _angel_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 15)
        add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->lev >= 35)
        add_flag(flgs, OF_REFLECT);
}

static void _dispelling_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Dispelling");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an Eldritch Blast which also does Dispel Magic.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1 + (p_ptr->lev / 27), 185));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH_DISPEL,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _angels_pact = {
  "Angels",
  "Angels are heavenly beings who use a variety of techniques to smite those they view as evil. "
    "Warlocks who make pacts with Angels will also find their saving throws significantly improved, "
    "and (eventually) their body almost immune to bolt-like effects. Since Angels are strongly aligned "
    "with the forces of good, making a pact with Angels will reduce damage done to all good monsters "
    "by a substantial amount.",
  "A", /* + RF3_GOOD ... They are even allied with Fallen Angels! */
  _angel_calc_bonuses,
  NULL,
  _angel_calc_stats,
  _angel_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  { 1,  1,  2,  1,  1,  3},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  35,  40,   1,  16,   8, 48, 35},
  {   7,  11,  15,   0,   0,   0, 13, 11},
/*Life  BaseHP     Exp */
    98,      4,    150,
  {
    {  2,  2, 30, bless_spell},
    {  5,  5, 40, light_area_spell},
    {  7,  4, 30, detect_monsters_spell},
    { 10,  7, 40, heroism_spell},
    { 20, 20, 60, remove_curse_I_spell},
    { 25, 15, 60, earthquake_spell},
    { 27, 20, 50, protection_from_evil_spell},
    { 29, 20, 60, dispel_evil_spell},
    { 33, 30, 70, starburst_I_spell},
    { 37, 20, 60, healing_I_spell},
    { 42, 70, 60, destruction_spell},
    { 47,100, 90, summon_angel_spell},
    { 50,120, 90, crusade_spell},
    { -1,   0,  0, NULL },
  },
  _dispelling_blast
};

/****************************************************************
 * Warlock Pact: Demons
 ****************************************************************/
static void _demon_calc_bonuses(void)
{
    res_add(RES_FIRE);
    p_ptr->skills.dev += 50 * p_ptr->lev/50;
    p_ptr->device_power += 5 * p_ptr->lev/50;
    if (p_ptr->lev >= 15)
        p_ptr->hold_life++;
    if (p_ptr->lev >= 30)
        p_ptr->no_eldritch = TRUE;
    if (p_ptr->lev >= 40)
        p_ptr->no_charge_drain = TRUE;
    if (p_ptr->lev >= 45)
        p_ptr->kill_wall = TRUE;
    if (p_ptr->lev >= 50)
        res_add_immune(RES_FIRE);
}

static void _demon_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_INT] += 3 * p_ptr->lev/50;
}

static void _demon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    if (p_ptr->lev >= 10) add_flag(flgs, OF_DEVICE_POWER);
    if (p_ptr->lev >= 15) add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 50) add_flag(flgs, OF_IM_FIRE);
}

static void _vengeful_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Vengeful");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an extremely deadly Eldritch Blast, but you also take 100 damage.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d", _blast_dd(50), _blast_ds(), p_ptr->to_d_spell));
        else
            var_set_string(res, format("%dd%d", _blast_dd(50), _blast_ds(), _blast_range));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1, 400));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = damroll(_blast_dd(50), _blast_ds());
        dam = spell_power(dam + p_ptr->to_d_spell);

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (p_ptr->chp < 100)
        {
            if (!get_check("Really cast Vengeful Blast? ")) return;
        }
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH, dir, dam, 0);
        take_hit(DAMAGE_USELIFE, 100, "a vengeful blast");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _demons_pact = {
  "Demons",
  "Demons are crafty creatures of Hell, using whatever means at their disposal to bring "
    "down their enemies. Warlocks who make pacts with Demons will find their abilities to use "
    "all magical devices improved, and learn the ability to Recharge these devices at will. "
    "Eventually, Demon Warlocks attain the ability to crush walls beneath their footsteps, and "
    "even gain immunity to fire. Making a pact with Demons will reduce damage done to all demons by a substantial amount.",
  "", /* RF3_DEMON is sufficient */
  _demon_calc_bonuses,
  NULL,
  _demon_calc_stats,
  _demon_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  { 3,  1,-10,  1,  1,  2},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  40,  40,   0,  12,   2, 64, 35},
  {   7,  15,  12,   0,   0,   0, 18, 11},
/*Life  BaseHP     Exp */
   100,     15,    150,
  {
    {  3,   2, 25, evil_bless_spell},
    {  6,   5, 30, resist_fire_spell},
    { 11,   9, 35, summon_manes_spell},
    { 20,  10, 50, teleport_spell},
    { 30,  25, 60, recharging_spell},
    { 37,  40, 80, kiss_of_succubus_spell},
    { 40,  50, 80, polymorph_demon_spell},
    { 43,  90, 90, summon_greater_demon_spell},
    { 45,  80, 85, hellfire_spell},
    { -1,   0,  0, NULL },
  },
  _vengeful_blast
};

/****************************************************************
 * Warlock Pact: Hounds
 ****************************************************************/
static void _hound_calc_bonuses(void)
{
    p_ptr->skill_dig += 60;
    p_ptr->pspeed += 5 * p_ptr->lev / 50;

    if (p_ptr->lev >= 5)
        res_add(RES_FIRE);
    if (p_ptr->lev >= 10)
        res_add(RES_COLD);
    if (p_ptr->lev >= 15)
        res_add(RES_ELEC);
    if (p_ptr->lev >= 20)
        res_add(RES_ACID);
    if (p_ptr->lev >= 35)
        res_add(RES_POIS);
}

static void _hound_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_DEX] += 3 * p_ptr->lev/50;
}

static void _hound_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 5)
        add_flag(flgs, OF_RES_FIRE);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_COLD);
    if (p_ptr->lev >= 15)
        add_flag(flgs, OF_RES_ELEC);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_RES_ACID);
    if (p_ptr->lev >= 35)
        add_flag(flgs, OF_RES_POIS);
}

#define _AETHER_EFFECT_CT 15
static void _aether_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Aether");
        break;
    case SPELL_DESC:
        var_set_string(res, "You channel the aether to unleash 2+1d5 random effects via your Eldritch Blast.");
        break;
    case SPELL_INFO:
        if (p_ptr->to_d_spell)
            var_set_string(res, format("%dd%d+%d", _blast_dd(242), _blast_ds(), p_ptr->to_d_spell));
        else
            var_set_string(res, format("%dd%d", _blast_dd(242), _blast_ds(), _blast_range));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 100));
        break;
    case SPELL_CAST:
    {
        int dir = 0, i;
        int ct = rand_range(3, 7);
        int dice = _blast_dd(242);
        int sides = _blast_ds();
        int effects[_AETHER_EFFECT_CT] =
               {GF_ACID,  GF_ELEC,   GF_FIRE,      GF_COLD,       GF_POIS,
                GF_LITE,  GF_DARK,   GF_CONFUSION, GF_NETHER,     GF_NEXUS,
                GF_SOUND, GF_SHARDS, GF_CHAOS,     GF_DISENCHANT, GF_TIME};
        int resists[_AETHER_EFFECT_CT] =
               {RES_ACID, RES_ELEC,  RES_FIRE,     RES_COLD,      RES_POIS,
                RES_LITE, RES_DARK,  RES_CONF,     RES_NETHER,    RES_NEXUS,
                RES_SOUND,RES_SHARDS,RES_CHAOS,    RES_DISEN,     RES_TIME};

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        for (i = 0; i < ct; i++)
        {
            int idx = randint0(_AETHER_EFFECT_CT);
            int effect = effects[idx];
            int resist = resists[idx];
            int dam = spell_power(damroll(dice, sides) + p_ptr->to_d_spell);

            msg_format("You channel <color:%c>%s</color>.",
                attr_to_attr_char(res_color(resist)),
                res_name(resist)
            );
            fire_ball(effect, dir, dam, 0);
            msg_boundary();
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _dog_whistle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dog Whistle");
        break;
    case SPELL_DESC:
        var_set_string(res, "By emitting a shrill whistle, unaudible to most, you attempt to control nearby canines.");
        break;
    case SPELL_CAST:
        project(0, 18, py, px, 1000, GF_CONTROL_PACT_MONSTER, PROJECT_KILL | PROJECT_HIDE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _aether_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Aether Shield");
        break;
    case SPELL_DESC:
        var_set_string(res, "Much like the dreaded Aether hound, you will gain protective elemental auras for a bit.");
        break;
    case SPELL_CAST:
        set_tim_sh_elements(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _hounds_pact = {
  "Hounds",
  "An alliance with hounds is sneakily strong: hounds are fast, stealthy and nimble, "
  "and the warlock will gain these attributes as well, in addition to basic elemental "
  "resistances. They also gain decent melee, and the ability to channel the unpredictable "
  "powers of the aether. Of course, Hound Warlocks may also call on their kin for assistance, "
  "though this may not be enough against the more difficult foes of the world. Still, wouldn't "
  "it be fun to have some of those annoying hounds on your side for a change?",
  "ZC",
  _hound_calc_bonuses,
  NULL,
  _hound_calc_stats,
  _hound_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  { 0, -2, -2,  2,  2,  1},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  25,  31,   5,  20,  15, 52, 25},
  {   7,  10,  10,   0,   0,   0, 16, 11},
/*Life  BaseHP     Exp */
   102,     12,    110,
  {
    {  1,   1, 30, hound_sniff_spell},
    { 20,  20, 60, summon_hounds_spell},
    { 27,  20, 50, haste_self_spell},
    { 30,  20, 50, resistance_spell},
    { 32,  30, 70, _dog_whistle_spell},
    { 40,  20, 60, _aether_shield_spell},
    { -1,   0,  0, NULL },
  },
  _aether_blast
};

/****************************************************************
 * Warlock Pact: Spiders
 ****************************************************************/
static void _spider_calc_bonuses(void)
{
    p_ptr->pspeed += 7 * p_ptr->lev / 50;

    if (p_ptr->lev >= 10)
        res_add(RES_POIS);
    if (p_ptr->lev >= 20)
        res_add(RES_NEXUS);
    if (p_ptr->lev >= 30)
        res_add(RES_TELEPORT);
}

static void _spider_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_DEX] += 3 * p_ptr->lev/50;
}

static void _spider_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 8)
        add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_RES_NEXUS);
}

static void _phase_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Phasing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fire an Eldritch Blast and then jump to safety in a single move.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(1 + (p_ptr->lev / 27), 185));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        teleport_player(25, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _nexus_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nexus Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a ball of nexus on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev + 20 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(
            GF_NEXUS,
            dir,
            spell_power(3*p_ptr->lev/2 + 30 + p_ptr->to_d_spell),
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

/* Spider Jumps*/
static int _jump_rad(void) { return 2 + p_ptr->lev/10; }

static int _poison_jump_dam(void) { return spell_power(p_ptr->lev + p_ptr->to_d_spell); }
static void _poison_jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Poison Jump");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a ball of poison as you jump to safety.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _poison_jump_dam()));
        break;
    case SPELL_CAST:
        fire_ball(GF_POIS, 0, _poison_jump_dam()*2, _jump_rad());
        teleport_player(30, 0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _nexus_jump_dam(void) { return spell_power(p_ptr->lev + 5 + p_ptr->to_d_spell); }
static void _nexus_jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nexus Jump");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a ball of nexus as you jump to safety.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _nexus_jump_dam()));
        break;
    case SPELL_CAST:
        fire_ball(GF_NEXUS, 0, _nexus_jump_dam()*2, _jump_rad());
        teleport_player(30, 0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _greater_nexus_jump_dam(void) { return spell_power(py_prorata_level_aux(200, 0, 0, 1) + p_ptr->to_d_spell); }
static void _greater_nexus_jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Greater Nexus Jump");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a huge ball of nexus as you jump to safety.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _greater_nexus_jump_dam()));
        break;
    case SPELL_CAST:
        fire_ball(GF_NEXUS, 0, _greater_nexus_jump_dam()*2, 3 + _jump_rad());
        teleport_player(30, 0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _spiders_pact = {
  "Spiders",
  "Of all the icky, crawling things in the world, you have chosen to make an alliance "
    "with spiders? Sure, they are stealthy, and fast, and very adept at hiding. Able "
    "to extrude a fine sticky substance to ensnare the unwary, the spider lurks nearby, "
    "or so you assume since you very seldom see them. Moreover, many spiders are adept at "
    "the powers of teleportation, able to jump quickly from one location to another, which "
    "can be quite baffling to a would be predator. They are comfortable with both poison "
    "and nexus. They are a bit on the squishy side, though - that is, if you "
    "manage to catch one!",
  "S",
  _spider_calc_bonuses,
  NULL,
  _spider_calc_stats,
  _spider_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  {-1,  0, -2,  2,  0,  1},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  37,  31,   7,  12,   2, 56, 60},
  {   7,  11,  10,   0,   0,   0, 12, 15},
/*Life  BaseHP     Exp */
    97,      1,    120,
  {
    {  1,   1, 30, phase_door_spell},
    {  3,   3, 30, stinking_cloud_spell},
    {  5,   4, 30, detect_monsters_spell},
    {  7,   5, 30, teleport_spell},
    { 15,   7, 60, summon_spiders_spell},
    { 17,   8, 50, _nexus_ball_spell},
    { 22,   9, 50, _poison_jump_spell},
    { 29,  12, 50, _nexus_jump_spell},
    { 30,  10, 60, spider_web_spell},
    { 35,  20, 60, dimension_door_spell},
    { 42,  40, 70, _greater_nexus_jump_spell},
    { -1,   0,  0, NULL },
  },
  _phase_blast
};

/****************************************************************
 * Warlock Pact: Giants
 ****************************************************************/
static void _giant_calc_bonuses(void)
{
    if (p_ptr->lev >= 30)
        res_add(RES_SOUND);
    if (p_ptr->lev >= 40)
        res_add(RES_SHARDS);
    if (p_ptr->lev >= 50)
        res_add(RES_CHAOS);
    p_ptr->skill_tht += 2*p_ptr->lev;
}

static void _giant_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (o_ptr->weight >= 200)
    {
        int to_h = 10 * p_ptr->lev / 50;
        int to_d = 10 * p_ptr->lev / 50;

        info_ptr->to_h += to_h;
        info_ptr->dis_to_h += to_h;

        info_ptr->to_d += to_d;
        info_ptr->dis_to_d += to_d;
    }
}

static void _giant_calc_stats(s16b stats[MAX_STATS])
{
    stats[A_STR] += 5 * p_ptr->lev/50;
    stats[A_CON] += 3 * p_ptr->lev/50;
}

static void _giant_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_SOUND);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_SHARDS);
    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_RES_CHAOS);
}

static void _confusing_blast(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast: Confusing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an Eldritch Blast that also confuses your opponent.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _power_cost(2 + (p_ptr->lev / 18), 125));
        break;
    case SPELL_CAST:
    {
        int dir = 0;

        var_set_bool(res, FALSE);

        project_length = _blast_range();
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_ELDRITCH_CONFUSE,
                  dir,
                  spell_power(damroll(_blast_dd(100), _blast_ds()) + p_ptr->to_d_spell),
                  0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _giant_healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Giant's Healing");
        break;
    case SPELL_DESC:
        var_set_string(res, "All powerful giants can heal themselves, right? Why not you?");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Heals %d", spell_power(p_ptr->lev * 4)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(p_ptr->lev * 4));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static _pact_t _giants_pact = {
  "Giants",
  "An alliance with giants and titans grants impressive physical strength and fortitude, "
    "but other stats will generally suffer. Later in life, you will gain giant-like resistances "
    "including sound, shards and chaos. Your combat will be impressive; and like giants, you "
    "will favor heavy weapons. Oh, and you won't mind tossing a boulder or two should the need "
    "arise! This alliance offers much in the way of physical advantages, but very "
    "little in the way of magical abilities or skill. Warlocks with this alliance are "
    "excellent fighters, being nearly indistiguishable in skill from Warriors!",
  "", /* RF3_GIANT will suffice */
  _giant_calc_bonuses,
  _giant_calc_weapon_bonuses,
  _giant_calc_stats,
  _giant_get_flags,
  NULL,
/*  S   I   W   D   C   C */
  { 2, -4, -4, -3,  2,  0},
/* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
  {  20,  18,  31,   0,  12,   2, 70, 30},
  {   7,   8,  10,   0,   0,   0, 30, 15},
/*Life  BaseHP     Exp */
   112,     25,    140,
  {
    {  5,   0, 50, throw_boulder_spell},
    { 10,   7,  0, stunning_blow_spell},
    { 30,   0,  0, monster_toss_spell},
    { 40,  30, 50, summon_kin_spell},
    { 50,  50, 70, _giant_healing_spell},
    { -1,   0,  0, NULL },
  },
  _confusing_blast
};

/****************************************************************
 * Private Helpers
 ****************************************************************/
static _pact_ptr _get_pact(int which)
{
    switch (which)
    {
    case WARLOCK_UNDEAD:
        return &_undead_pact;
    case WARLOCK_DRAGONS:
        return &_dragons_pact;
    case WARLOCK_ANGELS:
        return &_angels_pact;
    case WARLOCK_DEMONS:
        return &_demons_pact;
    case WARLOCK_HOUNDS:
        return &_hounds_pact;
    case WARLOCK_SPIDERS:
        return &_spiders_pact;
    case WARLOCK_GIANTS:
        return &_giants_pact;
    }
    return NULL;
}

/****************************************************************
 * Powers and Spells
 ****************************************************************/
#define MAX_WARLOCK_BLASTS    8

static spell_info _powers[MAX_WARLOCK_BLASTS] =
{
    /*lvl cst fail spell */
    {  1,  0,  20, _basic_blast},
    {  9,  0,  40, _extended_blast},
    { 17,  0,  45, _spear_blast},
    { 22,  0,  25, _easy_blast},
    { 27,  0,  60, _burst_blast},
    { 32,  0,  60, _stunning_blast},
    { 37,  0,  70, NULL},
    { 42,  0,  75, _empowered_blast},
};

static int _get_powers(spell_info* spells, int max)
{
    int       i;
    int       ct = 0;
    int       stat_idx = p_ptr->stat_ind[A_CHR];
    _pact_ptr pact = _get_pact(p_ptr->psubclass);

    assert(pact);

    /* This is debatable. Empowered blast is supposed to block your powers
       for one turn. They used to be spells, but now they are actually class
       powers, so won't be blocked by tim_no_spells ... Unless we do the
       following. Note, this now blocks all pact spells as well! */
    if (p_ptr->tim_no_spells)
        return 0;

    for (i = 0; i < MAX_WARLOCK_BLASTS; i++)
    {
        spell_info *base = &_powers[i];
        if (ct >= max) break;
        if ((base->level <= p_ptr->lev) || (show_future_powers))
        {
            spell_info* current = &spells[ct];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;
            current->fail = calculate_fail_rate(base->level, base->fail, stat_idx);
            if (current->fn == NULL)
                current->fn = pact->special_blast;

            ct++;
        }
    }
    return ct;
}
static int _get_spells(spell_info* spells, int max)
{
    int       i;
    int       ct = 0;
    int       stat_idx = p_ptr->stat_ind[A_CHR];
    _pact_ptr pact = _get_pact(p_ptr->psubclass);

    assert(pact);

    for (i = 0; ; i++)
    {
        spell_info *base = &pact->spells[i];
        if (base->level <= 0) break;
        if (ct >= max) break;
        if ((base->level <= p_ptr->lev) || (show_future_spells))
        {
            spell_info* current = &spells[ct++];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;
            current->fail = calculate_fail_rate(base->level, base->fail, stat_idx);
        }
    }

    return ct;
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    py_display_spells(doc, spells, ct);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "arcane power";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 450;
        if (p_ptr->psubclass == WARLOCK_DRAGONS)
            me.encumbrance.weapon_pct = 33;
        else if (p_ptr->psubclass == WARLOCK_GIANTS)
            me.encumbrance.weapon_pct = 20;
        else
            me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    if (p_ptr->psubclass == WARLOCK_GIANTS)
    {
        skills_weapon_init(TV_SWORD, SV_CLAYMORE, WEAPON_EXP_BEGINNER);
        py_birth_obj_aux(TV_SWORD, SV_CLAYMORE, 1);
    }
    else
        py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

/****************************************************************
 * Public API
 ****************************************************************/
bool warlock_is_pact_monster(monster_race *r_ptr)
{
    if (p_ptr->pclass == CLASS_WARLOCK)
    {
        _pact_ptr pact = _get_pact(p_ptr->psubclass);
        char     *pc = my_strchr(pact->alliance, r_ptr->d_char);

        if (pc != NULL)
            return TRUE;

        switch (p_ptr->psubclass)
        {
        case WARLOCK_UNDEAD:
            if (r_ptr->flags3 & RF3_UNDEAD)
                return TRUE;
            break;

        case WARLOCK_DRAGONS:
            if (r_ptr->flags3 & RF3_DRAGON)
                return TRUE;
            break;

        case WARLOCK_ANGELS:
            /* Angel pact is now all good monsters!!! */
            if (r_ptr->flags3 & RF3_GOOD)
                return TRUE;
            break;

        case WARLOCK_DEMONS:
            if (r_ptr->flags3 & RF3_DEMON)
                return TRUE;
            break;

        case WARLOCK_GIANTS:
            if (r_ptr->flags3 & RF3_GIANT)
                return TRUE;
            break;
        }
    }

    return FALSE;
}

class_t *warlock_get_class(int psubclass)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {
        me.name = "Warlock";
        me.desc =
        "Warlocks are a magical class; they derive their spells and powers not from books, "
        "but from an eldritch pact made with a given class of monsters. This pact is irrevocable "
        "and is made at the outset of the Warlock's career. Depending on the class of monsters with whom "
        "they ally, the warlock will gain unique bonuses, abilities and magical powers; for example, "
        "alliance with the forces of the netherworld grants resistances to poison and nether, enhanced "
        "constitution and spells to control and conjure the unliving, as well as spells to access "
        "the damaging forces of the netherworld directly. Each such alliance has thematic bonuses "
        "and powers, and you can read the details in the respective help sections.\n\n"
        "Allying with a given class of monsters dramatically reduces the warlock's ability to fight "
        "these foes. Instead, the warlock seeks cooperation with their brethren, or perhaps domination "
        "over them; direct assaults are rarely successful.\n \n"
        "In addition to pact-related spells, all warlocks gain access to the unique power of the "
        "Eldritch Blast. Their primary spell stat is Charisma, since they seek dominion and alliance "
        "with their chosen kin, and these monsters tend to have a strong will of their own.";

        me.birth = _birth;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.pets = 15;
        init = TRUE;
    }

    me.stats[A_STR] = 0;
    me.stats[A_INT] = 0;
    me.stats[A_WIS] = 0;
    me.stats[A_DEX] = 0;
    me.stats[A_CON] = 0;
    me.stats[A_CHR] = 0;
    me.life = 100;
    me.base_hp = 0;
    me.exp = 100;

    me.subname = "";
    me.subdesc = "";

    if (0 <= psubclass && psubclass < WARLOCK_MAX)
    {
        int       i;
        _pact_ptr pact = _get_pact(psubclass);

        me.subname = pact->name;
        me.subdesc = pact->desc;

        for (i = 0; i < MAX_STATS; i++)
            me.stats[i] += pact->stats[i];

        me.base_skills = pact->base_skills;
        me.extra_skills = pact->extra_skills;

        me.life = pact->life;
        me.base_hp = pact->base_hp;
        me.exp = pact->exp;

        me.calc_bonuses = pact->calc_bonuses;
        me.calc_weapon_bonuses = pact->calc_weapon_bonuses;
        me.calc_stats = pact->calc_stats;
        me.get_flags = pact->get_flags;
        me.process_player = pact->process_player;
    }

    return &me;
}

