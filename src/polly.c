#include "angband.h"
#include "equip.h"

#define _ANDROID_DIV ((prace_is_(RACE_ANDROID)) ? 5 : 1)
#define _OBJ_DEAGGRAVATED 10
#define _mon_array_size (int)((max_r_idx + 30) / 32)
#define _dung_array_size (int)((DUNGEON_MAX + 63) / 32)

/* TODO: Free friend_list on game exit */

static s32b _peak_au = 0;       /* Peak gold on this level */
static s32b _peak_exp = 0;      /* Peak experience on this level */
static s32b _spent_exp = 0;     /* Total exp spent (for androids) */
static u32b *friend_list;       /* List of monster races by friendliness */
static byte _list_init = 0;
static u32b dungeon_statup[_dung_array_size];

#define _INIT_FRIENDS 0x01
#define _INIT_STATUPS 0x02

static void _ini_friend_list(void)
{
    if (_list_init & _INIT_FRIENDS) return;
    C_MAKE(friend_list, _mon_array_size, u32b);
    _list_init |= _INIT_FRIENDS;
}

bool unique_is_friend(int which)
{
    monster_race *r_ptr = &r_info[which];
    if (p_ptr->pclass != CLASS_POLITICIAN) return FALSE;
    if ((which < 1) || (which >= max_r_idx)) return FALSE;
    if (!r_ptr->name) return FALSE; /* Nonexistent or removed */
    if (r_ptr->flagsx & RFX_SUPPRESS) return FALSE; /* Suppressed */
    if (r_ptr->max_num != 1) return FALSE; /* Either dead or not unique */
    if (!(_list_init & _INIT_FRIENDS)) _ini_friend_list();
    return have_flag(friend_list, which);
}

bool dungeon_conquered(int which)
{
    if ((which < 1) || (which >= max_d_idx)) return FALSE;
    if (!d_info[which].maxdepth) return FALSE;
    if (d_info[which].flags1 & DF1_RANDOM) return FALSE;
    if (!d_info[which].final_guardian) return ((max_dlv[which] == d_info[which].maxdepth) ? TRUE : FALSE);
    if (!r_info[d_info[which].final_guardian].max_num) return TRUE;
    if (prace_is_(RACE_MON_RING))
    {
        if (!(_list_init & _INIT_STATUPS)) ini_statup_list();
        if (have_flag(dungeon_statup, which)) return TRUE;
    }
    return (unique_is_friend(d_info[which].final_guardian));
}

void politician_set_friend(int which, bool kamu)
{
    if (p_ptr->pclass != CLASS_POLITICIAN) return;
    if ((which < 1) || (which >= max_r_idx)) return;
    /* I think this technically allows us to permanently befriend a Nazgul
     * if there's only one left... which is probably okay. */
    if (r_info[which].max_num > 1) return;
    if (kamu) add_flag(friend_list, which);
    else remove_flag(friend_list, which);
}

bool politician_dungeon_on_statup(int which)
{
    if ((p_ptr->pclass != CLASS_POLITICIAN) && (!prace_is_(RACE_MON_RING))) return TRUE;
    if (have_flag(dungeon_statup, which)) return FALSE;
    else add_flag(dungeon_statup, which);
    return TRUE;
}

void ini_statup_list(void)
{
    int i;
    for (i = 0; i < _dung_array_size; i++) dungeon_statup[i] = 0;
    _list_init |= _INIT_STATUPS;
}

static void _detect_threats_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Threats");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (p_ptr->lev >= 24) detect_monsters_invis(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _sleepy_speech_spell(int cmd, variant *res)
{
    int pow = (p_ptr->lev * 2) + ((p_ptr->lev >= 32) ? 21 : 14);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sleep-Inducing Speech");
        break;
    case SPELL_DESC:
        var_set_string(res, "Delivers a speech with soporific effects on nearby monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_CAST:
        msg_print("You deliver a sleep-inducing speech.");
        sleep_monsters(pow);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mystification_spell(int cmd, variant *res)
{
    int dir, pow = (p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mystification");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to confuse a nearby monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_CAST:
        if (!get_fire_dir(&dir))
        {
            var_set_bool(res, FALSE);
            return;
        }
        confuse_monster(dir, pow);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _charm_monster_spell(int cmd, variant *res)
{
    int dir, dice = (p_ptr->lev / 17) + 4, pow = (p_ptr->lev / 2) + damroll(dice, 7);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Charm Monster");
        break;
    case SPELL_DESC:
        if (politician_get_toggle() == POLLY_TOGGLE_XPCAST) var_set_string(res, "Attempts to charm a monster. In Glory mode you receive a 10% bonus for charming neutral monsters.");
        else if (politician_get_toggle() == POLLY_TOGGLE_AUCAST) var_set_string(res, "Attempts to charm a monster. In Grease mode you receive a 10% bonus for charming evil monsters.");
        else var_set_string(res, "Attempts to charm a monster. In Grit mode you receive a 10% bonus for charming good monsters.");
        break;
    case SPELL_INFO:
        if (p_ptr->spin && p_ptr->uimapuku) var_set_string(res, "power +110%");
        else if (p_ptr->uimapuku) var_set_string(res, "power +50%");
        else if (p_ptr->spin) var_set_string(res, "power +40%");
        else var_set_string(res, format("power %d+%dd7", (p_ptr->lev / 2) + (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1), dice));
        break;
    case SPELL_CAST:
        if (!get_fire_dir(&dir))
        {
            var_set_bool(res, FALSE);
            return;
        }
        charm_monster(dir, pow);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _background_check_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Background Check");
        break;
    case SPELL_DESC:
        var_set_string(res, "Reveals information about nearby monsters.");
        break;
    case SPELL_CAST:
        probing();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _object_is_aggravating(object_type *o_ptr)
{
    u32b flags[OF_ARRAY_SIZE];
    obj_flags(o_ptr, flags);
    return have_flag(flags, OF_AGGRAVATE);
}

bool object_is_deaggravated(object_type *o_ptr)
{
     if ((o_ptr->name1 == ART_HEAVENLY_MAIDEN) && (p_ptr->psex == SEX_FEMALE)) return TRUE;
     return (o_ptr->xtra1 == _OBJ_DEAGGRAVATED);
}

static void _remove_aggro_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Remove Aggravation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Makes an item nonaggravating.");
        break;
    case SPELL_CAST:
        {
            obj_prompt_t prompt = {0};

            prompt.prompt = "Make which object nonaggravating?";
            prompt.error = "You do not have any aggravating objects.";
            prompt.filter = _object_is_aggravating;
            prompt.where[0] = INV_PACK;
            prompt.where[1] = INV_EQUIP;
            prompt.where[2] = INV_FLOOR;
            obj_prompt(&prompt);
            var_set_bool(res, FALSE);

            if (!prompt.obj) return;
            if (prompt.obj->name1 == ART_WINBLOWS || prompt.obj->name1 == ART_MICRODOLLAR)
            {
                msg_print("Even you can't make a Micro$oft product nonaggravating.");
                return;
            }
            prompt.obj->xtra1 = _OBJ_DEAGGRAVATED;
            var_set_bool(res, TRUE);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_aide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Aide");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summons a friendly monster.");
        break;
    case SPELL_CAST:
        (void)summon_specific(-1, py, px, (p_ptr->lev * 7 / 5) - 15, 0, PM_FORCE_PET);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spying_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spying");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives you temporary telepathy.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        set_tim_esp(randint1(25) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spinning_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spinning");
        break;
    case SPELL_DESC:
        var_set_string(res, "Provides temporary resistance to nether and makes charming effects more powerful.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        set_spin(randint1(25) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _doctoring_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Doctoring");
        break;
    case SPELL_DESC:
        if (p_ptr->lev < 40)
            var_set_string(res, "Heals HP and Stun.");
        else if (p_ptr->lev < 45)
            var_set_string(res, "Heals HP and Stun. Cures cuts.");
        else
            var_set_string(res, "Heals HP and Stun. Cures cuts and slows poison.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Heals HP and Stun. Cures cuts (L40). Slows poison (L45).");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, p_ptr->lev + 5));
        break;
    case SPELL_CAST:
        warning_hack_hp = p_ptr->chp;
        hp_player(p_ptr->lev + 5);
        set_stun(0, TRUE);

        if (p_ptr->lev >= 40)
            set_cut(0, TRUE);
        if (p_ptr->lev >= 45)
            set_poisoned(p_ptr->poisoned - MAX(50, p_ptr->poisoned / 5), TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _diplomatic_impunity_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Diplomatic Impunity");
        break;
    case SPELL_DESC:
        var_set_string(res, "Removes fear and provides temporary heroism.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        if (p_ptr->afraid) fear_clear_p();
        set_hero(randint1(25) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void set_filibuster(bool paalle)
{
    bool old_fili = p_ptr->filibuster;
    p_ptr->filibuster = paalle;
    if (old_fili != paalle)
    {
        if (old_fili) msg_print("You stop filibustering.");
        else msg_print("You start filibustering.");
        p_ptr->redraw |= PR_STATUS;
        p_ptr->update |= PU_BONUS;
        handle_stuff();
    }
}

static void _filibuster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Filibuster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Toggles filibuster mode. While filibustering, you move very slowly (-12 to speed) and don't regenerate at all, but all monsters also receive the same speed penalty.");
        break;
    case SPELL_CAST:
        p_ptr->redraw |= PR_STATUS;
        p_ptr->update |= PU_BONUS;
        set_filibuster(p_ptr->filibuster ? FALSE : TRUE);
        var_set_bool(res, p_ptr->filibuster);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _create_chaos_spell(int cmd, variant *res)
{
    int pow = (p_ptr->lev * 7) + 13;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Create Chaos");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates a ball of chaos centered on you, potentially causing slight damage to you as well.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_CAST:
        fire_ball(GF_CHAOS, 0, pow * 2, 4);
        if (!res_save_default(RES_CHAOS))
        {
            int dam = res_calc_dam(RES_CHAOS, 32 + randint1(32));
            take_hit(DAMAGE_NOESCAPE, dam, "political chaos");
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _nether_storm_spell(int cmd, variant *res)
{
    int dir, pow = (p_ptr->lev * 6) + 63;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nether Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a large ball of nether.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_CAST:
        if (!get_fire_dir(&dir))
        {
            var_set_bool(res, FALSE);
            return;
        }
        fire_ball(GF_NETHER, dir, pow, 6);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
#define MAX_POLI_SKILL 14

static spell_info _spells[MAX_POLI_SKILL] =
{
    /*lvl cst fail spell */
    { POLITICIAN_FIRST_SPELL,   6,  20, _detect_threats_spell},
    { 9,   9,  30, _sleepy_speech_spell},
    {12,  12,  30, _mystification_spell},
    {15,  15,  35, _charm_monster_spell},
    {18,  18,  40, _background_check_spell},
    {21,  21,  45, _remove_aggro_spell},
    {24,  24,  45, _summon_aide_spell},
    {27,  27,  50, _spying_spell},
    {30,  30,  50, _spinning_spell},
    {33,  33,  50, _doctoring_spell},
    {36,  36,  50, _diplomatic_impunity_spell},
    {39,  39,   0, _filibuster_spell},
    {42,  42,  60, _create_chaos_spell},
    {45,  45,  60, _nether_storm_spell},
};

static int _get_toggle(void)
{
    if (!p_ptr->magic_num1[0]) return POLLY_TOGGLE_HPCAST;
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= (PR_STATUS | PR_POOL);
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    return result;
}

int politician_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->pclass != CLASS_POLITICIAN) return result;
    return _get_toggle();
}

/* Verify that we have enough HP/XP/AU available to use this spell */
int politician_verify_spell(spell_info *loitsu)
{
    int hinta = politician_get_cost(loitsu);
    int loytyy = politician_max_cost();
    if (hinta > loytyy) return _get_toggle();
    return 0;
}

#define _exp_pool MIN(500000L / _ANDROID_DIV, _peak_exp / (20 * MIN(3, _ANDROID_DIV)))
#define _au_pool MIN(500000L, _peak_au / 5)

int politician_get_cost(const spell_info *loitsu)
{
    if (!loitsu) return 0; /* paranoia */

    /* Hack - filibuster cost
     * Stopping filibustering actually costs nothing anyway, but we need to
     * specify the cost as zero here or the player will get error messages
     * like "you do not have enough hp" at low HP etc. */
    if ((loitsu->cost == 39) && (p_ptr->filibuster)) return 0;

    switch (_get_toggle())
    {
        case POLLY_TOGGLE_XPCAST:
        {
            int base = (loitsu->cost * loitsu->cost * loitsu->cost) / 30;
            int tila = MAX(_exp_pool, _au_pool / 2);
            int bonus = MIN(base * 10, (tila * loitsu->level) / ((p_ptr->lev == 50) ? 605 : 968));
            return ((base + bonus) / _ANDROID_DIV);
        }
        case POLLY_TOGGLE_AUCAST:
        {
            int base = (loitsu->cost * loitsu->cost * loitsu->cost) / 30;
            int tila = MAX(_exp_pool / 2, _au_pool);
            int bonus = MIN(base * 10, (tila * loitsu->level) / 1000);
            return (base + bonus);
        }
        default: return loitsu->cost;
    }
}

void politician_android_experience(void)
{
    p_ptr->exp -= MIN(p_ptr->exp, _spent_exp); /* Avoid negative XP */
    p_ptr->max_exp = p_ptr->exp;
}

void politician_check_experience(bool new_level)
{
    if ((new_level) || (p_ptr->max_exp > _peak_exp)) _peak_exp = p_ptr->max_exp;
    if (_get_toggle() == POLLY_TOGGLE_XPCAST) p_ptr->redraw |= (PR_POOL);
}

void politician_check_au(bool new_level)
{
    if ((new_level) || (p_ptr->au > _peak_au)) _peak_au = p_ptr->au;
    if (_get_toggle() == POLLY_TOGGLE_AUCAST) p_ptr->redraw |= (PR_POOL);
}

int politician_max_cost(void)
{
    switch (_get_toggle())
    {
        case POLLY_TOGGLE_XPCAST:
        {
             s32b max_pool = _exp_pool;
             int ylimeneva = p_ptr->max_exp + max_pool - _peak_exp;
             return MIN(p_ptr->exp, MAX(ylimeneva, 0));
        }
        case POLLY_TOGGLE_AUCAST:
        {
             s32b max_pool = _au_pool;
             int ylimeneva = p_ptr->au + max_pool - _peak_au;
             return MIN(p_ptr->au, MAX(ylimeneva, 0));
        }
        default: return p_ptr->chp;
    }
}

static void _grit_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Grit");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enters Grit mode (draw on your hitpoints to power your political career).");
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    case SPELL_CAST:
        _set_toggle(POLLY_TOGGLE_HPCAST);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _grease_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Grease");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enters Grease mode (draw on your wealth to power your political career).");
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    case SPELL_CAST:
        _set_toggle(POLLY_TOGGLE_AUCAST);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _glory_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Glory");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enters Glory mode (draw on your experience to power your political career).");
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    case SPELL_CAST:
        _set_toggle(POLLY_TOGGLE_XPCAST);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _powers[] =
{
    { A_NONE, { 1, 0,  0, _grit_spell}},
    { A_NONE, { 10, 0,  0, _grease_spell}},
    { A_NONE, { 20, 0,  0, _glory_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static int _get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, MIN(max, MAX_POLI_SKILL), _spells);
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    spellbook_character_dump(doc);
    doc_insert(doc, "<color:r>Realm:</color> <color:B>Politics</color>\n");
    py_display_spells_aux(doc, spells, ct);
}

static int _get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _powers);
}

static void _on_cast(const spell_info *spell)
{
    int kulu = politician_get_cost(spell);
    if (spell->cost == 39) /* ultra-hack */
    {
        p_ptr->filibuster = FALSE;
        kulu = politician_get_cost(spell);
        p_ptr->filibuster = TRUE;
        /* always true if we get here, since switching filibuster off
         * doesn't trigger on_cast */
    }
    switch (_get_toggle())
    {
        case POLLY_TOGGLE_AUCAST:
        {
            p_ptr->au -= kulu;
            stats_on_gold_services(kulu);
            p_ptr->redraw |= (PR_GOLD | PR_POOL);
            break;
        }
        case POLLY_TOGGLE_XPCAST:
        {
            p_ptr->exp -= kulu;
            p_ptr->max_exp -= kulu;
            _spent_exp += kulu;
            check_experience();
            p_ptr->redraw |= (PR_EXP | PR_POOL);
            break;
        }
        default:
        {
            take_hit(DAMAGE_USELIFE, kulu, "overworking");
            break;
        }
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 25) add_flag(flgs, OF_RES_FEAR);
}

static void _calc_bonuses(void)
{
    if ((!p_ptr->realm1) && (equip_find_obj(TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI))) p_ptr->uimapuku = TRUE;
    if (p_ptr->realm1)
    {
        p_ptr->skills.stl = MIN(p_ptr->skills.stl, 1);
        p_ptr->sustain_con = FALSE;
    }
    if (IS_HERO())
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
    }
    if (p_ptr->lev >= 25) res_add(RES_FEAR);
}

static void _politician_check_magic(bool syntyma)
{
    if ((!strlen(player_name)))
    {
        p_ptr->realm1 = REALM_NONE;
        return;
    }

    /* Some politicians can access a magic realm */
    if ((strpos("Don", player_name)) || (strpos("ump", player_name)) ||
        (strpos("Small", player_name)) || (strpos("Little", player_name)) ||
        (strpos("Tiny", player_name)) || (strpos("Matt", player_name)) ||
        (strpos("God", player_name)))
    {
        class_t *poli = politician_get_class();
        poli->exp = 145;
        poli->life = 98;
        poli->stats[A_WIS] = -2;
        p_ptr->realm1 = REALM_TRUMP;
        if (syntyma) py_birth_obj_aux(TV_TRUMP_BOOK, 0, 1);
    }
    else p_ptr->realm1 = REALM_NONE;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "political skill";
        me.which_stat = A_CHR;
        me.min_fail = 0;
        me.on_cast = _on_cast;
        me.encumbrance.max_wgt = 600;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 1000;
        init = TRUE;
    }
    if (p_ptr->realm1 && !mp_ptr->spell_book)
    {
        mp_ptr->spell_first = 1;
        mp_ptr->spell_book = realm2tval(p_ptr->realm1);
    }
    return &me;
}

void dungeon_statup_load(savefile_ptr file)
{
    byte _old_dung_array_size = savefile_read_byte(file);
    int i;
    for (i = 0; i < _old_dung_array_size; i++)
    {
        if (i < _dung_array_size) dungeon_statup[i] = savefile_read_u32b(file);
        else (void)savefile_read_u32b(file);
    }
}

static void _load_player(savefile_ptr file)
{
    _ini_friend_list();
    ini_statup_list();
    _peak_au = savefile_read_s32b(file);
    if (_peak_au == 0xFFF4)
    {
        byte _old_mon_array_size = savefile_read_byte(file);
        int i;
        for (i = 0; i < _old_mon_array_size; i++)
        {
            if (i < _mon_array_size) friend_list[i] = savefile_read_u32b(file);
            else (void)savefile_read_u32b(file);
        }
        _peak_au = savefile_read_s32b(file);
    }
    if (_peak_au == 0xFFF5)
    {
        dungeon_statup_load(file);
        _peak_au = savefile_read_s32b(file);
    }
    _peak_exp = savefile_read_s32b(file);
    _spent_exp = savefile_read_s32b(file);
    _politician_check_magic(FALSE);
}

void dungeon_statup_save(savefile_ptr file)
{
    int i;
    savefile_write_byte(file, _dung_array_size);
    for (i = 0; i < _dung_array_size; i++)
    {
        u32b tmp = dungeon_statup[i];
        savefile_write_u32b(file, tmp);
    }
}

static void _save_player(savefile_ptr file)
{
    int i;
    savefile_write_s32b(file, 0xFFF4); /* marker for compatibility of dev versions */
    savefile_write_byte(file, _mon_array_size);
    for (i = 0; i < _mon_array_size; i++)
    {
        u32b tmp = friend_list[i];
        savefile_write_u32b(file, tmp);
    }
    savefile_write_s32b(file, 0xFFF5); /* marker for compatibility of dev versions */
    dungeon_statup_save(file);
    savefile_write_s32b(file, _peak_au);
    savefile_write_s32b(file, _peak_exp);
    savefile_write_s32b(file, _spent_exp);
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_RAPIER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CURE_LIGHT, 3 + randint1(3));
    py_birth_obj_aux(TV_SCROLL, SV_SCROLL_PHASE_DOOR, 3 + randint1(3));
    py_birth_spellbooks();

    p_ptr->au += 150;
    _peak_au = 0;
    _peak_exp = 0;
    _spent_exp = 0;
    _ini_friend_list();
    ini_statup_list();
    _politician_check_magic(TRUE);
}

class_t *politician_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  24,  36,   2,  45,  28,  36,  24 };
    skills_t xs = { 15,   9,  11,   1,   0,   0,   6,   8 };

        me.name = "Politician";
        me.desc = "Politicians are masters of spinning and manipulation, seeking "
                  "to convert others to their cause and to destroy those they cannot "
                  "convert. They are not very good at either direct combat or at using "
                  "magical devices, and rely on their savvy and special skills and on "
                  "the help of their friends to win fights. They have the ability to "
                  "toggle between hit points, wealth and experience as the source of "
                  "their power, although the latter two only become available at higher "
                  "levels. Charisma is a key stat for Politicians.";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 9;
        me.exp = 130;
        me.pets = 25;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        me.character_dump = _character_dump;
        me.load_player = _load_player;
        me.save_player = _save_player;
        init = TRUE;
    }

    return &me;
}
