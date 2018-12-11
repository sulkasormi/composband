/* File: corny.c */

/* Purpose: Cornucopia offices */

#include "angband.h"

#include <assert.h>

#define MAX_POLICY 4
#define INVALID_POLICY() ((policy < 0) || (policy >= MAX_POLICY))

static u32b _deposit = 0;
static u32b _received = 0;
static u32b _paid = 0;
static u32b _loan = 0;
static u32b _loan_turn = 0;
static u32b _deposit_turn = 0;
static u32b _fake_loan = 0;
static u32b _fake_deposit = 0;
static u32b _loaned = 0;
static u32b _repaid = 0;
static u32b _deposited = 0;
static u32b _withdrawn = 0;
static byte _next_policy = 0;

/* This one's even more fake than the other fakes */
static int _fake_value = 0;

struct _insurance_s
{
    byte i_idx; /* The index of the policy. Policy indices go from 0 to 255, but only four can ever co-exist */
    byte num_destroyed;
    u32b disen_value;
    u32b destroy_value;
    bool restore_fully;
    bool restore_fully_korvaus;
    u32b compensation;
    byte num_insured;
    object_type *o_ptr; /* Independent copy of the object insured */
};
typedef struct _insurance_s _insurance_type;

static _insurance_type _my_policies[MAX_POLICY];

static void _show_cornucopia_help(void)
{
    screen_save();
    doc_display_help("corny.txt", NULL);
    screen_load();
}

static int _interpret_building_command(building_type *bldg, char komento)
{
    int i, tulos = 0;
    for (i = 0; i < 8; i++)
    {
        if (bldg->letters[i])
        {
            if (bldg->letters[i] == komento)
            {
                tulos = bldg->actions[i];
                break;
            }
        }
    }
    return tulos;
}

static int _loan_cap(void)
{
    return pienempi(300000L, isompi(1500L, pienempi(reforge_limit() * 2, stats_gold_counts.found + stats_gold_counts.selling + stats_gold_counts.winnings)));
}

static u32b _calculate_deposit(void)
{
    static int viim1 = 0;
    static u32b viim2 = 0, viim3 = 0, viim4 = 0;
    u32b _my_deposit_frac;
    s32b _my_deposit;
    if (!_deposit) return 0;
    if ((viim1 == game_turn) && (viim2 == _deposited) && (viim3 == _withdrawn)) return viim4;
    {
        int i, days_passed, cur_day, cur_hr, cur_min, old_day, old_hr, old_min;
        extract_day_hour_min(&cur_day, &cur_hr, &cur_min);
        extract_day_hour_min_imp(_deposit_turn, &old_day, &old_hr, &old_min);
        days_passed = cur_day - old_day;
        if (days_passed < 1)
        {
            viim1 = game_turn;
            viim2 = _deposited;
            viim3 = _withdrawn;
            viim4 = _deposit;
            return viim4;
        }
        _my_deposit = 0;
        _my_deposit_frac = _deposit;
        s64b_LSHIFT(_my_deposit, _my_deposit_frac, 8);
        for (i = 0; i < days_passed; i++)
        {
            s64b_mul(&_my_deposit, &_my_deposit_frac, 0, 10001);
            s64b_div(&_my_deposit, &_my_deposit_frac, 0, 10000);
        }
        viim1 = game_turn;
        viim2 = _deposited;
        viim3 = _withdrawn;
        s64b_RSHIFT(_my_deposit, _my_deposit_frac, 8);
        viim4 = _my_deposit_frac;
    }    
    return viim4;
}

static u32b _calculate_loan(void)
{
    static int viim1 = 0;
    static u32b viim2 = 0, viim3 = 0, viim4 = 0;
    u32b _my_loan_frac;
    s32b _my_loan;
    if (!_loan) return 0;
    if ((viim1 == game_turn) && (viim2 == _loaned) && (viim3 == _repaid)) return viim4;
    {
        int i, days_passed, cur_day, cur_hr, cur_min, old_day, old_hr, old_min;
        extract_day_hour_min(&cur_day, &cur_hr, &cur_min);
        extract_day_hour_min_imp(_loan_turn, &old_day, &old_hr, &old_min);
        days_passed = cur_day - old_day;
        if (days_passed < 1)
        {
            viim1 = game_turn;
            viim2 = _loaned;
            viim3 = _repaid;
            viim4 = _loan;
            return viim4;
        }
        _my_loan = 0;
        _my_loan_frac = _loan;
        s64b_LSHIFT(_my_loan, _my_loan_frac, 8);
        for (i = 0; i < days_passed; i++)
        {
            s64b_mul(&_my_loan, &_my_loan_frac, 0, 11);
            s64b_div(&_my_loan, &_my_loan_frac, 0, 10);
        }
        viim1 = game_turn;
        viim2 = _loaned;
        viim3 = _repaid;
        s64b_RSHIFT(_my_loan, _my_loan_frac, 8);
        viim4 = _my_loan_frac;
    }    
    return viim4;
}

static void _fake_update(void)
{
    _fake_loan = _calculate_loan();
    _fake_deposit = _calculate_deposit();
}

static bool _excessive_loan(void)
{
    _fake_update();
    if (!_loan) return FALSE;
    if (_fake_deposit >= _fake_loan) return FALSE;
    return (_fake_loan - _fake_deposit) > 400000L;
}

static u32b _allowable(void)
{
    int limiitti = (stats_gold_counts.found + stats_gold_counts.selling + stats_gold_counts.winnings) / 100;
    int tappiot = _paid - _received;
    if (limiitti > 50000L) limiitti = 50000L;
    if (limiitti < 5000) limiitti = 5000;
    if (tappiot > limiitti) return 0;
    return (limiitti - tappiot);
}

static int _index_to_policy(int ip_idx)
{
    int i, result = -1;
    for (i = 0; i < MAX_POLICY; i++)
    {
        if (ip_idx == _my_policies[i].i_idx)
        {
            if ((!ip_idx) && (!_my_policies[i].num_insured)) continue; /* Watch out for 0 being both a valid policy index and the "no policy" index */
            result = i;
            break;
        }
    }
    return result;
}

int cornucopia_item_policy(object_type *o_ptr)
{
    if (!o_ptr->insured) return -1;
    {
        int result = _index_to_policy(o_ptr->insured / 100);
        if (result == -1) o_ptr->insured = 0; /* Cleanup */
        return result;
    }
}

static bool _policy_is_active(int policy)
{
    if (INVALID_POLICY()) return FALSE;
    if ((_my_policies[policy].num_insured) || (_my_policies[policy].compensation)) return TRUE;
    return FALSE;
}

static bool _policy_is_active2(int policy)
{
    if (INVALID_POLICY()) return FALSE;
    if ((_my_policies[policy].num_insured > _my_policies[policy].num_destroyed) || (_my_policies[policy].compensation) || (_my_policies[policy].restore_fully_korvaus)) return TRUE;
    return FALSE;
}


static void _give_compensation(int policy, int number)
{
    if (INVALID_POLICY()) return;
    _my_policies[policy].compensation += _my_policies[policy].destroy_value * number;
    if ((_my_policies[policy].restore_fully) && (_my_policies[policy].num_insured <= _my_policies[policy].num_destroyed)) _my_policies[policy].restore_fully_korvaus = TRUE;
}

void cornucopia_mark_destroyed(int policy, int amt)
{
    if (INVALID_POLICY()) return;
    _my_policies[policy].num_destroyed += amt;
}

void cornucopia_object_destroyed(object_type *o_ptr, int amt, bool mon_attack)
{
    int i, tuhottu = 0, montako = o_ptr->number, vakuutettu = (o_ptr->insured % 100), policy = cornucopia_item_policy(o_ptr);
    if (INVALID_POLICY()) return;
    for (i = 0; i < amt; i++) /* Pile of objects. Roll to see if the destroyed object is an insured one */
    {
        if ((montako <= vakuutettu) || (randint0(montako) < vakuutettu))
        {
            vakuutettu--;
            tuhottu++;
        }
        montako--;
    }
    if (!tuhottu) return;
    obj_dec_insured(o_ptr, tuhottu);
    cornucopia_mark_destroyed(policy, tuhottu);
    if (mon_attack) _give_compensation(policy, tuhottu);
}

/* _clear_bldg() and _prt_gold() are from bldg.c */

static void _clear_bldg(int min_row, int max_row)
{
    int i;

    for (i = min_row; i <= max_row; i++)
        prt("", i, 0);
}

static void _prt_gold(void)
{
    char tmp[10];
    char tmp_str[80];

    prt("Gold Remaining: ", 23, 53);

    big_num_display(p_ptr->au, tmp);
    sprintf(tmp_str, "%6.6s", tmp);
    prt(tmp_str, 23, 68);
}

static bool _allow_destroy_insurance(object_type *o_ptr)
{
    if (p_ptr->pclass == CLASS_BERSERKER) return (o_ptr->tval == TV_POTION);
    if (o_ptr->tval == TV_STAFF) return ((o_ptr->name2 != EGO_DEVICE_RESISTANCE) && (p_ptr->pclass != CLASS_MAGIC_EATER));
    if (o_ptr->tval == TV_WAND) return ((o_ptr->name2 != EGO_DEVICE_RESISTANCE) && (p_ptr->pclass != CLASS_MAGIC_EATER));
    if (o_ptr->tval == TV_POTION) return TRUE;
    if (o_ptr->tval == TV_SCROLL) return TRUE;
    return FALSE;
}

static byte _allow_disen_limit(void)
{
    int _limit = p_ptr->lev / 5 + 2;
    if ((p_ptr->pclass == CLASS_WARRIOR) || (p_ptr->pclass == CLASS_SAMURAI) ||
        (p_ptr->pclass == CLASS_CAVALRY) || (p_ptr->pclass == CLASS_BERSERKER) ||
        (p_ptr->pclass == CLASS_BLOOD_KNIGHT) || (p_ptr->pclass == CLASS_MAULER)) _limit += 3;
    if ((p_ptr->realm1 == REALM_CRAFT) || (p_ptr->realm2 == REALM_CRAFT)) _limit = 15;
    if (p_ptr->pclass == CLASS_WEAPONSMITH) _limit = 20;
    return _limit;
}

static bool _allow_disen_insurance(object_type *o_ptr)
{
    int _limit = _allow_disen_limit();
    if (p_ptr->prace == RACE_MON_ARMOR || p_ptr->prace == RACE_MON_RING || p_ptr->prace == RACE_MON_SWORD) return FALSE;
    if (have_flag(o_ptr->flags, OF_RES_DISEN)) return FALSE;
    if (!object_is_weapon_armour_ammo(o_ptr)) return FALSE;
    if (object_is_ammo(o_ptr)) return FALSE;
    if (o_ptr->number != 1) return FALSE;
    if (obj_value(o_ptr) < 20) return FALSE;
    if (o_ptr->to_a > _limit) return TRUE;
    if (o_ptr->to_d > _limit) return TRUE;
    if (o_ptr->to_h > _limit) return TRUE;
    if (object_is_weapon(o_ptr)) return ((o_ptr->to_a > 0) || (o_ptr->pval > 1));
    if (object_is_armour(o_ptr)) return ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->pval > 1));
    return FALSE;
}

#define _DISEN_TO_A 0
#define _DISEN_TO_H 1
#define _DISEN_TO_D 2
#define _DISEN_PVAL 3
#define _DISEN_START 0
#define _DISEN_TYPES 4

static int _item_disenchantment_value(object_type *o_ptr, int tyyppi, int muutos, int base_val, bool is_real, bool was_applied)
{
    int mult = 1;
    int div = 1;
    bool outo_tyyppi = FALSE;
    if ((muutos < 1) || (base_val < 1)) return 0;
    if (object_is_ego(o_ptr)) mult = 2;
    if (object_is_artifact(o_ptr)) mult = 4;
    if ((tyyppi == _DISEN_PVAL) && (mult > 1)) mult *= 5;
    if ((object_is_weapon(o_ptr)) && (tyyppi == _DISEN_TO_A)) outo_tyyppi = TRUE;
    else if ((object_is_armour(o_ptr)) && ((tyyppi == _DISEN_TO_H) || (tyyppi == _DISEN_TO_D))) outo_tyyppi = TRUE;
    if (outo_tyyppi) mult *= 3;
    if ((tyyppi == _DISEN_TO_A || tyyppi == _DISEN_TO_H)) div = 2;
    if ((is_real) && (!outo_tyyppi) && (tyyppi != _DISEN_PVAL))
    {
        int lahto_arvo = o_ptr->to_a;
        int uusi_arvo;
        if (tyyppi == _DISEN_TO_H) lahto_arvo = o_ptr->to_h;
        if (tyyppi == _DISEN_TO_D) lahto_arvo = o_ptr->to_d;
        if (was_applied) lahto_arvo += muutos; /* The disenchantment was already applied to the item - we want the situation before */
        uusi_arvo = MAX(lahto_arvo - muutos, _allow_disen_limit());
        muutos = MAX(0, lahto_arvo - uusi_arvo);
    }
    return base_val * muutos * mult / div;
}

static int _item_disenchantment_value_aux(object_type *o_ptr, int tyyppi, int arvo)
{
    if (arvo < 1) return 0;
    switch (tyyppi)
    {
        case _DISEN_TO_H: return ((o_ptr->to_d > 0) ? _item_disenchantment_value(o_ptr, tyyppi, 1, arvo, TRUE, FALSE) : 0);
        case _DISEN_TO_D: return ((o_ptr->to_d > 0) ? _item_disenchantment_value(o_ptr, tyyppi, 1, arvo, TRUE, FALSE) : 0);
        case _DISEN_PVAL: return ((o_ptr->pval > 1) ? _item_disenchantment_value(o_ptr, tyyppi, 1, arvo, TRUE, FALSE) : 0);
        default: return ((o_ptr->to_a > 0) ? _item_disenchantment_value(o_ptr, _DISEN_TO_A, 1, arvo, TRUE, FALSE) : 0);
    }
    return 0; /* keep dumb compilers happy */
}

void cornucopia_item_disenchanted(object_type *o_ptr, int old_to_a, int old_to_h, int old_to_d, int old_pval)
{
    int policy;
    if (!o_ptr->insured) return;
    policy = cornucopia_item_policy(o_ptr);
    if (INVALID_POLICY()) return;
    if (!_my_policies[policy].disen_value) return;
    if ((_my_policies[policy].o_ptr) && (_my_policies[policy].o_ptr->k_idx) && (_my_policies[policy].o_ptr->k_idx != o_ptr->k_idx)) return; /* paranoia */
    _my_policies[policy].compensation += _item_disenchantment_value(o_ptr, _DISEN_TO_A, old_to_a - o_ptr->to_a, _my_policies[policy].disen_value, TRUE, TRUE);
    _my_policies[policy].compensation += _item_disenchantment_value(o_ptr, _DISEN_TO_H, old_to_h - o_ptr->to_h, _my_policies[policy].disen_value, TRUE, TRUE);
    _my_policies[policy].compensation += _item_disenchantment_value(o_ptr, _DISEN_TO_D, old_to_d - o_ptr->to_d, _my_policies[policy].disen_value, TRUE, TRUE);
    _my_policies[policy].compensation += _item_disenchantment_value(o_ptr, _DISEN_PVAL, old_pval - o_ptr->pval, _my_policies[policy].disen_value, TRUE, TRUE);

    /* Keep track of changes to item */
    if ((_my_policies[policy].o_ptr) && (_my_policies[policy].o_ptr->k_idx))
    {
        _my_policies[policy].o_ptr->to_a = o_ptr->to_a;
        _my_policies[policy].o_ptr->to_h = o_ptr->to_h;
        _my_policies[policy].o_ptr->to_d = o_ptr->to_d;
        _my_policies[policy].o_ptr->pval = o_ptr->pval;
    }
}

static bool _allow_restore_fully(object_type *o_ptr)
{
    if (p_ptr->pclass == CLASS_BERSERKER) return FALSE;
    if (o_ptr->tval == TV_STAFF) return (o_ptr->name2 != EGO_DEVICE_RESISTANCE);
    if (o_ptr->tval == TV_WAND) return (o_ptr->name2 != EGO_DEVICE_RESISTANCE);
    return FALSE;
}

static bool _allow_insurance(object_type *o_ptr)
{
    if (o_ptr->insured)
    {
        (void)cornucopia_item_policy(o_ptr);
        if ((o_ptr->insured) && ((o_ptr->insured % 100) >= o_ptr->number) && (o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return FALSE;
    }
    if (_allow_disen_insurance(o_ptr)) return TRUE;
    if (_allow_destroy_insurance(o_ptr)) return TRUE;
    return FALSE;
}

static bool _is_captured_implorington(obj_ptr obj)
{
    if (obj->tval == TV_CAPTURE && obj->pval == MON_IMPLORINGTON)
        return TRUE;
    return FALSE;
}
static bool _is_corpse_implorington(obj_ptr obj)
{
    if (obj->tval == TV_CORPSE && obj->pval == MON_IMPLORINGTON)
        return TRUE;
    return FALSE;
}
static void _process_implorington(obj_ptr obj)
{
    char name[MAX_NLEN];
    char buf[MAX_NLEN+30];
    int amt = 25000L;
    object_desc(name, obj, OD_COLOR_CODED);
    sprintf(buf, "Convert %s into money? ", name);
    if (get_check(buf))
    {
        if (obj->tval == TV_CAPTURE)
        {
            r_info[MON_IMPLORINGTON].max_num = 0;
            r_info[MON_IMPLORINGTON].ball_num = 0;
            empty_capture_ball(obj);
            amt *= 10;
            msg_print("Thank you! We're looking forward to a quality discussion with the guy.");
        }
        else
            obj->number = 0;
        msg_format("You get %dgp.", amt);
        p_ptr->au += amt;
        stats_on_gold_winnings(amt);
        obj_release(obj, 0);
        p_ptr->redraw |= PR_GOLD;
        _prt_gold();
        p_ptr->notice |= PN_OPTIMIZE_PACK;
    }
}

static void _check_implorington(void)
{
    equip_for_each_that(_process_implorington, _is_captured_implorington);
    pack_for_each_that(_process_implorington, _is_captured_implorington);
    pack_for_each_that(_process_implorington, _is_corpse_implorington);
}

static void _wanted_poster(void)
{
    _clear_bldg(4,18);
    c_put_str(TERM_L_RED, "   WANTED FOR INSURANCE FRAUD:", 5, 15);
    c_put_str(TERM_YELLOW,"        Implorington III      ", 6, 15);
    c_put_str(TERM_GREEN, "         Also known as:       ", 8, 15);
    c_put_str(TERM_GREEN, "         Muffin Murkley       ", 9, 15);
    c_put_str(TERM_GREEN, "        Larry the Looter      ", 10, 15);
    c_put_str(TERM_GREEN, "          Impy the Git        ", 11, 15);
    c_put_str(TERM_GREEN, "The Fastest Teleport in the Deeps", 12, 15);
    c_put_str(TERM_GREEN, "      That @$^%& White 'p'    ", 13, 15);
    c_put_str(TERM_WHITE, "        Rewards offered:      ", 15, 15);
    c_put_str(TERM_WHITE, "   catch alive ----   $250,000", 16, 15);
    c_put_str(TERM_WHITE, "   corpse      ----    $25,000", 17, 15);
}

static void _cornucopia_ini_policies(int which)
{
    int i, alku, loppu;
    if ((which < 0) || (which >= MAX_POLICY))
    {
        alku = 0;
        loppu = MAX_POLICY - 1;
    }
    else
    {
        alku = which;
        loppu = which;
    }
    for (i = alku; i <= loppu; i++)
    {
        _my_policies[i].i_idx = 0;
        _my_policies[i].num_destroyed = 0;
        _my_policies[i].disen_value = 0;
        _my_policies[i].destroy_value = 0;
        _my_policies[i].restore_fully = FALSE;
        _my_policies[i].restore_fully_korvaus = FALSE;
        _my_policies[i].compensation = 0;
        _my_policies[i].num_insured = 0;
        _my_policies[i].o_ptr = NULL;
    }
}

static void _open_insurance_policy_aux(int i, object_type *o_ptr)
{
    if ((_my_policies[i].o_ptr) && (_my_policies[i].o_ptr->k_idx)) obj_free(_my_policies[i].o_ptr);
    _cornucopia_ini_policies(i);
    _my_policies[i].i_idx = _next_policy;
    _my_policies[i].o_ptr = obj_copy(o_ptr);
    obj_clear_dun_info(_my_policies[i].o_ptr);
    _next_policy++;
    if (_next_policy > 250) _next_policy = 0;
    while (_index_to_policy(_next_policy) > -1) /* Avoid nightmare scenario of looping back to policy 0 with the original policy 0 still active */
    {
        _next_policy++;
        if (_next_policy > 250) _next_policy = 0;
    }
}

/* See if the policy can be safely wiped now */
static void _lopeta_policy(int policy)
{
    if (INVALID_POLICY()) return;
    if (_my_policies[policy].compensation) return;
    if (_my_policies[policy].restore_fully_korvaus) return;
    if (_my_policies[policy].num_insured > _my_policies[policy].num_destroyed) return;
    if ((_my_policies[policy].o_ptr) && (_my_policies[policy].o_ptr->k_idx)) obj_free(_my_policies[policy].o_ptr);
    _cornucopia_ini_policies(policy);
}

static void _update_loan(int amount)
{
    if (!amount) return;
    _fake_loan = _calculate_loan();
    if (amount > 0) /* Add to loan */
    {
        _loaned += amount;

        /* Charge immediate interest on the newly loaned money */
        _loan = _fake_loan + (amount * 11 / 10);
        _loan_turn = game_turn;
        p_ptr->au += amount;
    }
    else /* Reduce loan */
    {
        amount = 0 - amount;
        if (amount > MIN((s32b)_fake_loan, p_ptr->au)) amount = MIN((s32b)_fake_loan, p_ptr->au); /* paranoia */
        _repaid += amount;
        _loan = _fake_loan - amount;
        _loan_turn = game_turn;
        p_ptr->au -= amount;
    }
    _fake_loan = _calculate_loan();
    msg_print("Thank you for using Cornucopia services!");
    if (_fake_loan) msg_format("You now owe us %d gold.", _fake_loan);
    else msg_print("You have fully repaid your loan.");
    p_ptr->redraw |= PR_GOLD;
    _prt_gold();
}

static void _update_deposit(int amount)
{
    _fake_deposit = _calculate_deposit();
    _deposit = _fake_deposit + amount;
    p_ptr->au -= amount;
    if (amount > 0)
    {
        stats_on_gold_services(MAX(500, amount / 5000));
        p_ptr->au -= MAX(500, amount / 5000);
        _deposited += amount;
    }
    else _withdrawn += (0 - amount);
    _deposit_turn = game_turn;
    _fake_deposit = _calculate_deposit();
    msg_print("Thank you for choosing Cornucopia!");
    if (_fake_deposit) msg_format("You now have %d gold in your deposit account.", _fake_deposit);
    else msg_print("Your deposit account is now empty.");
    p_ptr->redraw |= PR_GOLD;
    _prt_gold();
}

static void _update_fake_value(int amount)
{
    _fake_value = amount;
}

static void _suorita_maksu(int amt, int amt2)
{
    p_ptr->au -= amt;
    _received += (amt - amt2);
    if (amt2) stats_on_gold_services(amt2);
    p_ptr->redraw |= PR_GOLD;
    _prt_gold();
}

static void _suorita_korvaus(int amt)
{
    if (!amt) return; /* this can happen due to exposure limits */
    p_ptr->au += amt;
    _paid += amt;
    msg_format("You collect %d gold pieces' worth of gold pieces.", amt);
    p_ptr->redraw |= PR_GOLD;
    _prt_gold();
}

static void _claim_compensation(int policy)
{
    if (INVALID_POLICY()) return;
    if (_my_policies[policy].compensation > _allowable())
    {
        if (!get_check(format("Due to exposure limits, you can only claim %d gold now. You will forfeit any claim to the remaining %d gold if you do so. Proceed anyway? ", _allowable(), _my_policies[policy].compensation - _allowable()))) return;
    }
    _suorita_korvaus(pienempi(_my_policies[policy].compensation, _allowable()));
    _my_policies[policy].compensation = 0;
    _lopeta_policy(policy);
}

static void _claim_replacement(int policy)
{
    if (INVALID_POLICY()) return;
    if ((_my_policies[policy].restore_fully_korvaus) && (_my_policies[policy].o_ptr)) /* paranoia */
    {
        _my_policies[policy].o_ptr->number = 1; /* further paranoia */
        object_origins(_my_policies[policy].o_ptr, ORIGIN_CORNUCOPIA);
        _my_policies[policy].o_ptr->mitze_type = 0;
        object_mitze(_my_policies[policy].o_ptr, MITZE_ID);
        pack_carry(_my_policies[policy].o_ptr);
        _my_policies[policy].restore_fully_korvaus = FALSE;
        _lopeta_policy(policy);
    }
}

static int _cornucopia_komento_handler(building_type *bldg, int *amount, int amount_min, int amount_cap, bool allow_mode_switch, bool *negative_mode, void updater(int amount))
{
    char komento = inkey_special(FALSE);
    switch (komento)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            if (*amount < amount_cap) *amount = (*amount * 10) + komento - '0';
            *amount = MIN(*amount, amount_cap);
            break;
        }
        case ESCAPE:
        {
            if (*amount)
            {
                *amount = 0;
                break;
            }
            else return -1;
        }
        case 'M': /* Switch modes */
        {
            if (allow_mode_switch)
            {
                *negative_mode = !(*negative_mode);
                *amount = 0;
            }
            break;
        }
        case '\n':
        case '\r': /* Enter */
        {
            if (*amount < 1) break;
            if (*amount < amount_min)
            {
                msg_format("We feel a bit uncomfortable about the size of your loan. Please repay at least %d gold.", amount_min);
                break;
            }
            *amount = MIN(*amount, amount_cap); /* paranoia */
            updater((*negative_mode) ? 0 - *amount : *amount);
            return -1;
        }
        case '\010': /* Backspace */
        {
            *amount /= 10;
            break;
        }
        default:
        {
            int bact = _interpret_building_command(bldg, komento);
            if (bact == BACT_VIEW_POLICY) _show_cornucopia_help();
            else if (bact > 0) return bact;
            break;
        }
    }
    return 0;
}

static int _cornucopia_loan_interact(building_type *bldg)
{
    bool mode = FALSE, old_mode;
    bool allow_mode_switch = FALSE;
    int amount = 0, paikka = 0;
    int cap = _loan_cap();
    int amount_cap = 0;
    int tulos;
    int amount_min = _excessive_loan() ? _fake_loan - (_fake_deposit + 300000) : 0;
    char viesti[12] = "0";
    if (_loan) mode = TRUE;
    old_mode = !mode;
    if (p_ptr->au < amount_min)
    {
        msg_format("Please return when you can show us at least %d gold.", amount_min);
        return 0;
    }
    while (1)
    {
        if (old_mode != mode)
        {
            _clear_bldg(4, 18);
            old_mode = mode;
        }
        allow_mode_switch = ((mode || _loan) && ((s32b)_fake_loan < cap));
        if (allow_mode_switch)
        {
            if (mode) c_prt(TERM_L_BLUE, "Press 'M' to extend your loan instead.", 17, 5);
            else c_prt(TERM_L_BLUE, "Press 'M' to repay your loan instead.", 17, 5);
        }
        if (mode)
        {
            put_str(format("You owe us %d gold pieces.", _fake_loan), 7, 5);
            prt(format("Pay back: %s", viesti), 8, 5);
            Term_gotoxy(15 + paikka, 8);
            amount_cap = MIN((s32b)_fake_loan, p_ptr->au);
        }
        else
        {
            int rivi = 7;
            if (_fake_loan)
            {
                put_str(format("You currently owe us %d gold pieces.", _fake_loan), rivi++, 5);
                put_str(format("We are willing to extend your loan by another %d gold.", cap - _fake_loan), rivi++, 5);
                amount_cap = cap - _fake_loan;
            }
            else
            {
                if (p_ptr->au < 3000) put_str(format("Strapped for cash? We're willing to loan you up to %d gold.", cap), rivi++, 5);
                else if (p_ptr->au < 15000) put_str(format("Looking for a line of credit? We're willing to loan you %d gold.", cap), rivi++, 5);
                else if (p_ptr->au < 100000) put_str(format("We know you can pay back, so we're willing to loan you %d gold.", cap), rivi++, 5);
                else put_str(format("You being such a great hero, we're willing to loan you %d gold.", cap), rivi++, 5);
                amount_cap = cap;
            }
            put_str(format("Request: %s", viesti), rivi++, 5);
            Term_gotoxy(14 + paikka, rivi - 1);
        }
        tulos = _cornucopia_komento_handler(bldg, &amount, amount_min, amount_cap, allow_mode_switch, &mode, _update_loan);
        if (tulos < 0) return 0;
        else if (tulos > 0) return tulos;
        sprintf(viesti, "%d", amount);
        paikka = (!amount) ? 0 : strlen(viesti);
    }
}

static int _cornucopia_deposit_interact(building_type *bldg)
{
    bool withdraw_mode = FALSE;
    bool allow_mode_switch = FALSE;
    int amount = 0, paikka = 0;
    int cap = p_ptr->au - MAX(500, p_ptr->au / 5001);
    int amount_cap = 0;
    char viesti[12] = "0";
    if (_excessive_loan()) /* Note that this also runs _fake_update() for us */
    {
        msg_print("Please reduce your loan to 300,000 gp or less.");
        return 0;
    }
    if ((cap <= 0) && (!_deposit))
    {
        msg_print("You can't afford to pay the 500gp deposit fee!");
        return 0;
    }
    if (cap <= 0) withdraw_mode = TRUE;
    _clear_bldg(4, 18);
    while (1)
    {
        int tulos;
        allow_mode_switch = (cap > 0) && (_deposit);
        if (allow_mode_switch)
        {
            if (withdraw_mode) c_prt(TERM_L_BLUE, "Press 'M' to deposit money instead.", 17, 5);
            else c_prt(TERM_L_BLUE, "Press 'M' to withdraw money instead.", 17, 5);
        }
        if (withdraw_mode)
        {
            prt(format("You have %d gold in your savings account.", _fake_deposit), 7, 5);
            prt(format("Withdraw: %s", viesti), 8, 5);
            Term_gotoxy(15 + paikka, 8);
            amount_cap = _fake_deposit;
        }
        else
        {
            int rivi = 7;
            amount_cap = p_ptr->au - MAX(500, p_ptr->au / 5001);
            if (_fake_deposit)
            {
                prt(format("You currently have %d gold in your savings account.", _fake_deposit), rivi++, 5);
                prt(format("Additional deposit: %s", viesti), rivi++, 5);
                Term_gotoxy(25 + paikka, rivi - 1);
            }
            else
            {
                prt("Currently, you don't have any gold in your account.", rivi++, 5);
                prt(format("New deposit: %s", viesti), rivi++, 5);
                Term_gotoxy(18 + paikka, rivi - 1);
            }
        }
        tulos = _cornucopia_komento_handler(bldg, &amount, 0, amount_cap, allow_mode_switch, &withdraw_mode, _update_deposit);
        if (tulos < 0) return 0;
        else if (tulos > 0) return tulos;
        sprintf(viesti, "%d", amount);
        paikka = (!amount) ? 0 : strlen(viesti);
    }
}

static u32b _count_policies(bool use_bitflag)
{
    u32b i, luku = 0, laskuri = 0;
    for (i = 0; i < MAX_POLICY; i++)
    {
        if (_policy_is_active(i))
        {
            laskuri += (1 << i);
            luku += 1;
        }
    }
    return (use_bitflag ? laskuri : luku);
}

static int _open_insurance_policy(object_type *o_ptr)
{
    int i, policy = -1;

    /* Find first available policy */
    for (i = 0; i < MAX_POLICY; i++)
    {
        if (!_policy_is_active(i))
        {
            policy = i;
            break;
        }
    }
    if (policy < 0)
    {
        for (i = 0; i < MAX_POLICY; i++)
        {
            if (!_policy_is_active2(i))
            {
                policy = i;
                break;
            }
        }
    }
    if (INVALID_POLICY()) return -1;
    _open_insurance_policy_aux(policy, o_ptr);
    return policy;
}

static int _disen_insure_item(object_type *o_ptr, building_type *bldg)
{
    int i, _value_cap = MIN(MIN(p_ptr->au, pienempi(obj_value(o_ptr), obj_value_real(o_ptr)) / 20), 2000);
    bool dummy = FALSE;
    char o_name[MAX_NLEN];
    int paikka = 0, paikka_modifier = 0;
    int amount = 0;
    int tulos;
    char viesti[12] = "0";
    if (!_value_cap)
    {
        if (!p_ptr->au) msg_print("You don't have any money!");
        else msg_print("This item is too worthless to be insured.");
        return 0;
    }
    object_desc(o_name, o_ptr, OD_SINGULAR);
    _clear_bldg(4,18);
    c_put_str(tval_to_attr[o_ptr->tval], format("%-72.72s", o_name), 9, 5);
    put_str("Compensation for damage to:   To-AC   To-Hit  To-Dam  Pval", 13, 5);
    if (_value_cap < 1000)
    {
        paikka_modifier--;
        if (_value_cap < 100) paikka_modifier--;
        if (_value_cap < 10) paikka_modifier--;
    }
    while (1)
    {
        for (i = _DISEN_START; i < _DISEN_TYPES; i++)
        {
            int arvo = _item_disenchantment_value_aux(o_ptr, i, amount);
            if (!arvo) c_put_str(TERM_RED, "---- ", 14, 35 + 8 * i);
            else c_put_str(TERM_L_GREEN, format("%-5d", arvo), 14, 35 + 8 * i);
        }
        prt(format("Insure item for (1-%d): %s", _value_cap, viesti), 10, 5);
        Term_gotoxy(31 + paikka + paikka_modifier, 10);

        _fake_value = 0;
        tulos = _cornucopia_komento_handler(bldg, &amount, 0, _value_cap, FALSE, &dummy, _update_fake_value);
        if (_fake_value > 0)
        {
            int policy = _open_insurance_policy(o_ptr);
            if (INVALID_POLICY())
            {
                msg_print("Failed to open new insurance policy!");
                return 0;
            }
            else
                msg_print("Thank you for purchasing a Cornucopia insurance policy!");
            _my_policies[policy].num_insured = 1;
            _my_policies[policy].disen_value = _fake_value;
            o_ptr->insured = (_my_policies[policy].i_idx * 100) + 1;
            _suorita_maksu(amount, 0);
            return 0;
        }
        if (tulos < 0) return 0;
        if (tulos > 0) return tulos;
        sprintf(viesti, "%d", amount);
        paikka = (!amount) ? 0 : strlen(viesti);
    }
}

static int _destroy_insure_item(object_type *o_ptr, building_type *bldg)
{
    char o_name[MAX_NLEN];
    int vakuutettu = 0;
    int ulkopuoliset = 0;
    int valinta = 0;
    int hinta = 0, total_cost = 0;
    int arvo = MAX(obj_value(o_ptr), obj_value_real(o_ptr));
    int div = (object_is_device(o_ptr)) ? 3 : 1;
    int mult = (object_is_device(o_ptr)) ? 4 : 2;
    int qty = o_ptr->number;
    int maksimi = qty;
    bool allow_full = _allow_restore_fully(o_ptr);
    byte update_bitflag = 0x02;
    byte existing_policies = 0;
    int rivi = 0;
    int replacement_mult = 15;
    int vari;
    int komento;
    if (!o_ptr) return 0; /* paranoia */
    object_desc(o_name, o_ptr, 0);
    if (!allow_full) valinta = 1; /* What other options do we have? */
    if (object_is_ego(o_ptr)) replacement_mult *= 2;
    if (o_ptr->insured)
    {
        int policy = cornucopia_item_policy(o_ptr);
        vakuutettu = (o_ptr->insured % 100);
        if (!INVALID_POLICY())
        {
            if (_my_policies[policy].destroy_value) existing_policies += 0x01;
            if (_my_policies[policy].restore_fully) existing_policies += 0x02;
            ulkopuoliset = MAX(0, _my_policies[policy].num_insured - _my_policies[policy].num_destroyed - vakuutettu);
        }
        /* Check if the item's already fully insured */
        if ((vakuutettu > qty) || ((vakuutettu == qty) && (existing_policies == (allow_full ? 0x03 : 0x01))))
        {
            msg_print("You cannot insure this item any further!");
            return 0;
        }
        maksimi = MIN(qty, MIN(99 - ulkopuoliset, 99 + vakuutettu - _my_policies[policy].num_insured));
        qty = MIN(qty, maksimi);
        valinta = MAX(valinta, existing_policies);
    }
    if (qty < 1)
    {
        msg_print("You cannot insure this item any further!");
        return 0;
    }
    _clear_bldg(4,18);
    c_put_str(tval_to_attr[o_ptr->tval], format("%-69.69s", o_name), 8, 8);
    put_str("[Space] to unselect a plan, [Enter] to buy this policy", 15, 5);
    if (o_ptr->number > vakuutettu + 1) put_str("+/- or N/n to modify item quantity", 16, 5);
    vari = TERM_SLATE;
    if (valinta & 0x02) vari = TERM_L_BLUE;
    else if ((allow_full) && (p_ptr->au >= hinta + (arvo * replacement_mult))) vari = TERM_UMBER;
    if (allow_full) c_prt(vari, format("b) Replacement Plan - Cost: %d", arvo * replacement_mult), 13, 5);
    else c_prt(vari, "Replacement Plan - not available for this item", 13, 5);
    while (1)
    {
        hinta = 0;
        total_cost = 0;
        if (valinta & 0x01)
        {
            hinta = arvo / div;
            total_cost = arvo / div * (qty - vakuutettu);
        }
        if ((valinta & 0x02) && (!(existing_policies & 0x02)))
        {
            hinta += arvo * replacement_mult;
            total_cost += arvo * replacement_mult;
        }
        if (update_bitflag & 0x10)
        {
            object_desc(o_name, o_ptr, (qty == 1) ? OD_SINGULAR : 0);
            c_prt(tval_to_attr[o_ptr->tval], format("%-69.69s", o_name), 8, ((o_ptr->number >= 10) && (qty == 1)) ? 9 : 8);
            c_put_str(tval_to_attr[o_ptr->tval], format(o_ptr->number >= 10 ? "%2d" : "%d", qty), 8, 8);
            update_bitflag &= ~0x11;
        }
        if (update_bitflag & 0x02)
        {
            vari = TERM_SLATE;
            if (valinta & 0x01) vari = TERM_L_BLUE;
            else if (p_ptr->au >= (hinta + (arvo / div)) * (qty - vakuutettu)) vari = TERM_UMBER;
            c_prt(vari, format("%sRegular Plan - Cost: %d%s - Pays: %d%s", allow_full ? "a) " : "", (arvo / div), ((qty > 1) ? "/item" : ""), (arvo * mult / div), ((qty > 1) ? "/item" : "")), 12, 5);
            update_bitflag &= ~0x02;
        }
        if (valinta & 0x01) c_put_str(vari, "SELECTED", 12, 67);
        if (update_bitflag & 0x01)
        {
            c_put_str(tval_to_attr[o_ptr->tval], format(o_ptr->number >= 10 ? "%2d" : "%d", qty), 8, 8);
            update_bitflag &= ~0x01;
        }
        if (allow_full || o_ptr->number > 1)
        {
            vari = TERM_YELLOW;
            if (p_ptr->au < total_cost) vari = TERM_SLATE;
            c_prt(vari, format("Cost of %s this policy: %d gp", (qty == vakuutettu) ? "keeping" : vakuutettu ? "upgrading to" : "buying", total_cost), 10, 20);
        }
        if (allow_full)
        {
            if (update_bitflag & 0x04)
            {
                vari = TERM_SLATE;
                if (valinta & 0x02) vari = TERM_L_BLUE;
                else if (p_ptr->au >= hinta + (arvo * replacement_mult)) vari = TERM_UMBER;
                c_prt(vari, format("b) Replacement Plan - Cost: %d", arvo * replacement_mult), 13, 5);
                update_bitflag &= ~0x04;
            }
            if (valinta & 0x02) c_put_str(vari, "SELECTED", 13, 67);
            Term_gotoxy(5, 12 + rivi);
        }
        komento = inkey_special(FALSE);
        switch (komento)
        {
            case 'A':
            case 'a':
            {
                rivi = 0;
                valinta = (valinta ^ 0x01);
                if (existing_policies & 0x01) valinta |= 0x01;
                update_bitflag |= 0x02;
                break;
            }
            case 'B':
            case 'b':
            {
                if (!allow_full) break;
                rivi = 1;
                valinta = (valinta ^ 0x02);
                if (existing_policies & 0x02) valinta |= 0x02;
                update_bitflag |= 0x06;
                break;
            }
            case ' ':
            {
                valinta = (valinta ^ (0x01 << rivi));
                if (existing_policies & (0x01 << rivi)) valinta |= (0x01 << rivi);
                update_bitflag |= 0x06;
                break;
            }
            case SKEY_UP:
            case SKEY_DOWN:
            case '8':
            case '2':
            {
                if (allow_full) rivi = 1 - rivi;
                break;
            }
            case 'N':
            case '+':
            {
                if (maksimi == 1) break;
                if (qty == 1) update_bitflag |= 0x10;
                qty++;
                if (qty > maksimi) qty = MAX(1, vakuutettu);
                if (qty == 1) update_bitflag |= 0x10;
                update_bitflag |= 0x07;
                break;
            }
            case 'n':
            case '-':
            {
                if (maksimi == 1) break;
                if (qty < 3) update_bitflag |= 0x10;
                qty--;
                if (qty < MAX(1, vakuutettu)) qty = maksimi;
                update_bitflag |= 0x07;
                break;
            }
            case ESCAPE:
                return 0;
            case '\n':
            case '\r':
            {
                int policy;
                if (!total_cost) break;
                if (total_cost > p_ptr->au)
                {
                    msg_print("You don't have enough gold to buy this policy.");
                    break;
                }
                if (!get_check(format("Buy this policy for %d gold? ", total_cost))) return 0;
                policy = vakuutettu ? cornucopia_item_policy(o_ptr) : _open_insurance_policy(o_ptr);
                if (INVALID_POLICY())
                {
                    msg_print("Failed to open new insurance policy!");
                    return 0;
                }
                else
                    msg_print("Thank you for purchasing a Cornucopia insurance policy!");
                _my_policies[policy].num_insured += (qty - vakuutettu); /* There may be other objects that are insured under this policy but aren't in this particular pile */
                _my_policies[policy].disen_value = 0;
                if (valinta & 0x01) _my_policies[policy].destroy_value = arvo * mult / div;
                _my_policies[policy].restore_fully = (valinta & 0x02) ? TRUE : FALSE;
                o_ptr->insured = (_my_policies[policy].i_idx * 100) + qty;
                _suorita_maksu(total_cost, ((valinta & 0x02) && (!(existing_policies & 0x02))) ? arvo * replacement_mult : 0);
                return 0;
            }
            default:
            {
                int bact = _interpret_building_command(bldg, komento);
                if (bact == BACT_VIEW_POLICY) _show_cornucopia_help();
                else if (bact > 0) return bact;
                break;
            }
        }
    }
}

static int _insure_new_item(building_type *bldg)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Insure which item?";
    prompt.error = "You have no items that can be insured.";
    prompt.filter = _allow_insurance;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    obj_prompt(&prompt);
    if (!prompt.obj) return 0;

    /* Figure which type of insurance it is that fits this item */
    if (_allow_disen_insurance(prompt.obj)) return _disen_insure_item(prompt.obj, bldg);
    else return _destroy_insure_item(prompt.obj, bldg);
}

static bool _policy_allows_cash_in(int policy)
{
    if (INVALID_POLICY()) return FALSE;
    if (_my_policies[policy].restore_fully_korvaus) return TRUE;
    if (_my_policies[policy].compensation > 0) return TRUE;
    return FALSE;
}

static void _cornucopia_cash_in(void)
{
    int i, could_claim = FALSE;
    _check_implorington();
    if (_excessive_loan())
    {
        msg_print("Please reduce your loan to 300,000 gp or less.");
        return;
    }
    for (i = 0; i < MAX_POLICY; i++)
    {
        if (!_policy_is_active(i)) continue;
        if (!_my_policies[i].compensation && !_my_policies[i].restore_fully_korvaus) continue;
        could_claim = TRUE;
        if (_my_policies[i].compensation) _claim_compensation(i);
        if (_my_policies[i].restore_fully_korvaus)
        {
            _claim_replacement(i);
            if (pack_overflow_count())
            {
                msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
                return;
            }
        }
    }
    if (!could_claim) msg_print("You cannot currently claim any rewards, replacement items or monetary compensation.");
}

static void _policy_blurb(int i, int *rivi)
{
    if (_policy_is_active(i))
    {
        if (!_my_policies[i].o_ptr || !_my_policies[i].o_ptr->k_idx)
        {
            /* Something's wrong */
            c_prt(TERM_SLATE, format("%d)  software bug", i + 1), 5, (*rivi)++);
        }
        else
        {
            char o_name[MAX_NLEN];
            _my_policies[i].o_ptr->number = _my_policies[i].num_insured - _my_policies[i].num_destroyed;
            object_desc(o_name, _my_policies[i].o_ptr, 0);
            c_prt(tval_to_attr[_my_policies[i].o_ptr->tval], format("%d) %-66.66s", i + 1, o_name), (*rivi)++, 5);
            if (_my_policies[i].num_destroyed)
            {
                if (_my_policies[i].num_insured > 1)
                {
                     c_prt(TERM_L_UMBER, format("%d insured", _my_policies[i].num_insured), *rivi, 8);
                     c_prt(TERM_RED, format("%d lost or destroyed", _my_policies[i].num_destroyed), (*rivi)++, 21);
                }
                else c_prt(TERM_RED, "Destroyed", (*rivi)++, 8);
            }
            if (_policy_allows_cash_in(i))
            {
                char tmp_val[100] = "";
                int paikka = 5;
                if (_my_policies[i].compensation)
                {
                    sprintf(tmp_val, "Unclaimed compensation: %d gp ", _my_policies[i].compensation);
                    prt(tmp_val, *rivi, paikka);
                    paikka += (strlen(tmp_val) + 2);
                    if (_my_policies[i].compensation > _allowable())
                    {
                        sprintf(tmp_val, format("(Claimable: %d) ", _allowable()));
                        c_prt(TERM_YELLOW, tmp_val, *rivi, paikka);
                        paikka += (strlen(tmp_val) + 2);
                    }
                }
                if (_my_policies[i].restore_fully_korvaus)
                {
                    sprintf(tmp_val, "Unclaimed Copy");
                    c_prt(TERM_L_BLUE, tmp_val, *rivi, paikka);
                }
                if (strlen(tmp_val) > 2) *rivi += 1;
            }
        }
    }
    else
        c_prt(TERM_SLATE, format("%d)  empty", i + 1), (*rivi)++, 5);
}

static bool _object_policy_matches(obj_ptr obj)
{
    if ((obj) && (obj->insured) && (obj->insured / 100 == _fake_value)) return TRUE;
    return FALSE;
}

static int _interact_with_policy(int policy, building_type *bldg)
{
    int i, rivi = 5;
    char komento;
    char option_keys[5] = "dcrm";
    char viestit[4][30] = {"d) Cancel this policy", "c) Claim compensation", "r) Get replacement item", "m) Modify policy"};
    bool avoid_redraw = FALSE;
    object_type *obj = NULL;
    slot_t slot = 0;
    if (INVALID_POLICY()) return 0;

    /* Try to find a compatible object */
    _fake_value = _my_policies[policy].i_idx;
    slot = pack_find_first(_object_policy_matches);
    if (slot) obj = pack_obj(slot);
    if (!obj)
    {
        slot = equip_find_first(_object_policy_matches);
        if (slot) obj = equip_obj(slot);
    }
    _fake_value = 0;
    
    while (1)
    {
        byte _option_flags = 0x01; /* Policy can be deleted */

         /* See if compensation can be claimed (0x02) */
        if ((_my_policies[policy].compensation) && (_allowable() > 0)) _option_flags |= 0x02;

        /* See if a replacement item can be claimed (0x04) */
        if (_my_policies[policy].restore_fully_korvaus) _option_flags |= 0x04;
        
        if ((obj) && ((_my_policies[policy].destroy_value) || (_my_policies[policy].restore_fully)))
        {
            /* See if policy can be modified (0x08) */
            if (((obj->insured % 100) < obj->number) || ((!_my_policies[policy].restore_fully) && (_allow_restore_fully(obj))) || (!_my_policies[policy].destroy_value))
                _option_flags |= 0x08;
        }
        if (!avoid_redraw)
        {
            rivi = 5;
            _clear_bldg(4,18);
            _policy_blurb(policy, &rivi);
            if (_my_policies[policy].disen_value)
            {
                c_put_str(TERM_YELLOW, "Compensation for damage to:   To-AC   To-Hit  To-Dam  Pval", rivi++, 5);
                for (i = _DISEN_START; i < _DISEN_TYPES; i++)
                {
                    int arvo = _item_disenchantment_value_aux(_my_policies[policy].o_ptr, i, _my_policies[policy].disen_value);
                    if (!arvo) c_put_str(TERM_RED, "---- ", rivi, 35 + 8 * i);
                    else c_put_str(TERM_L_GREEN, format("%-5d", arvo), rivi, 35 + 8 * i);
                }
                rivi++;
            }
            else if (((_my_policies[policy].destroy_value) || (_my_policies[policy].restore_fully)) && (_my_policies[policy].num_insured > _my_policies[policy].num_destroyed))
            {
                bool singular = (_my_policies[policy].num_insured == 1);
                if (_my_policies[policy].destroy_value) c_put_str(TERM_YELLOW, format("Compensation for destroyed item%s: %d gp%s%s", singular ? "" : "s", _my_policies[policy].destroy_value, singular ? "" : "/item", _my_policies[policy].restore_fully ? " and a copy" : ""), rivi++, 5);
                else c_put_str(TERM_YELLOW, "Compensation for destroyed item: a copy", rivi++, 5);
            }
            for (i = 0; i < 4; i++)
            {
                int y = 15 + i / 2;
                int x = 5 + 35 * (i % 2);
                int vari = (_option_flags & (0x01 << i)) ? TERM_WHITE : TERM_SLATE;
                c_put_str(vari, viestit[i], y, x);
            }
        }
        komento = inkey_special(FALSE);
        avoid_redraw = FALSE;
        switch (komento)
        {
            case ESCAPE: return -1;
            default:
            {
                bool loytyi = FALSE;
                int bact;
                for (i = 0; i < 4; i++)
                {
                    if ((komento == option_keys[i]) && (_option_flags & (0x01 << i)))
                    {
                        loytyi = TRUE;
                        switch (i)
                        {
                            case 0: /* Wipe policy */
                            {
                                if (!get_check("Are you sure you want to cancel this policy? ")) break;
                                if ((_my_policies[policy].o_ptr) && (_my_policies[policy].o_ptr->k_idx)) obj_free(_my_policies[policy].o_ptr);
                                _cornucopia_ini_policies(policy);
                                msg_print("Policy cancelled.");
                                return 0;
                            }
                            case 1: /* Claim compensation */
                            {
                                _claim_compensation(policy);
                                if (!_policy_is_active2(policy)) return 0;
                                break;
                            }
                            case 2: /* Claim replacement item */
                            {
                                _claim_replacement(policy);
                                if (pack_overflow_count())
                                {
                                    msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
                                    return 0;
                                }
                                if (!_policy_is_active2(policy)) return 0;
                                break;
                            }
                            default: /* Modify - destroy insurance only */
                            {
                                if (!obj) break; /* paranoia */
                                bact = _destroy_insure_item(obj, bldg);
                                if (bact > 0) return bact;
                                break;
                            }
                            
                        }
                        break;
                    }
                }
                if (loytyi) break;
                avoid_redraw = TRUE;
                bact = _interpret_building_command(bldg, komento);
                if (bact == BACT_VIEW_POLICY) _show_cornucopia_help();
                else if (bact > 0) return bact;
                break;
            }
        }
    }
}

static int _view_active_policies(building_type *bldg)
{
    int i;
    int rivi = 5;
    char komento;
    u32b active_policies = _count_policies(TRUE);
    _clear_bldg(4,18);
    for (i = 0; i < MAX_POLICY; i++)
    {
        _policy_blurb(i, &rivi);
    }
    while (1)
    {
        komento = inkey_special(FALSE);
        switch (komento)
        {
            case ESCAPE: return 0;
            default:
            {
                int yrkka = komento - '1', tulos = 0;
                if ((yrkka >= 0) && (yrkka < MAX_POLICY) && (active_policies & (1 << yrkka)))
                {
                    tulos = _interact_with_policy(yrkka, bldg);
                    if (tulos == -1) return -2;
                    return tulos;
                }
                
                yrkka = komento - 'a';
                if ((yrkka >= 0) && (yrkka < MAX_POLICY) && (active_policies & (1 << yrkka)))
                {
                    tulos = _interact_with_policy(yrkka, bldg);
                    if (tulos == -1) return -2;
                    return tulos;
                }

                yrkka = _interpret_building_command(bldg, komento);
                if (yrkka == BACT_VIEW_POLICY) _show_cornucopia_help();
                else if (yrkka > 0) return yrkka;
                break;
            }
        }
    }
}

static int _cornucopia_insurance_interact(building_type *bldg)
{
    int luku = _count_policies(FALSE);
    bool vaihtoehto = ((luku > 0) && (luku < MAX_POLICY));
    _clear_bldg(4,18);
    if (_excessive_loan())
    {
        msg_print("Please reduce your loan to 300,000 gp or less.");
        return 0;
    }
    while (1)
    {
        int valinta = 1;
        if (luku == MAX_POLICY) valinta = 2;
        if (vaihtoehto)
        {
            int komento;
            bool lopeta = FALSE;
            put_str("a) Insure new item", 7, 5);
            put_str("b) View or modify existing insurance policies", 8, 5);
            while (!lopeta)
            {
                Term_gotoxy(5, 6 + valinta);
                komento = inkey_special(FALSE);
                switch (komento)
                {
                    case SKEY_UP:
                    case SKEY_DOWN:
                    case '8':
                    case '2':
                        valinta = 3 - valinta;
                        break;
                    case 'a':
                    case 'A':
                        valinta = 1;
                        lopeta = TRUE;
                        break;
                    case 'b':
                    case 'B':
                        valinta = 2;
                        lopeta = TRUE;
                        break;
                    case ESCAPE:
                        return 0;
                    case '\n':
                    case '\r':
                        lopeta = TRUE;
                        break;
                    default:
                    {
                        int bact = _interpret_building_command(bldg, komento);
                        if (bact == BACT_VIEW_POLICY) _show_cornucopia_help();
                        else if (bact > 0) return bact;
                        break;
                    }
                }
            }
        }
        if (valinta == 1) return _insure_new_item(bldg);
        else
        {
            int tulos = -2;
            while (tulos == -2)
            {
                tulos = _view_active_policies(bldg);
            }
            return tulos;
        }
    }
}

void cornucopia_do_command(building_type *bldg, int bact)
{
    while (bact > 0)
    {
        switch (bact)
        {
            case BACT_LOAN:
                bact = _cornucopia_loan_interact(bldg);
                _clear_bldg(4, 18);
                break;
            case BACT_DEPOSIT:
                bact = _cornucopia_deposit_interact(bldg);
                _clear_bldg(4, 18);
                break;
            case BACT_INSURANCE:
                bact = _cornucopia_insurance_interact(bldg);
                _clear_bldg(4, 18);
                break;
            case BACT_CORNY_CASH_IN:
                _cornucopia_cash_in();
                return;
            case BACT_VIEW_POLICY:
                _show_cornucopia_help();
                return;
            case BACT_VIEW_POSTER:
                _wanted_poster();
                return;
            default: return;
        }
    }
}

void cornucopia_print_stats(doc_ptr doc)
{
    _fake_update();
    if (_loaned)
    {
        s32b tulos;
        char vari = 'G';
        doc_printf(doc, "  Loans    : <color:w>%8d</color>\n", _loaned);
        doc_printf(doc, "  Interest : <color:w>%8d</color>\n", _fake_loan + _repaid - _loaned);
        doc_printf(doc, "  Repaid   : <color:w>%8d</color>\n", _repaid);
        tulos = _loaned - _repaid;
        if (tulos < 0)
        {
            tulos = 0 - tulos;
            vari = 'R';
        }
        doc_printf(doc, "  Unpaid   : <color:w>%8d</color> <color:%c>%8d</color>\n", _fake_loan, vari, tulos);
    }
    if (_deposited)
    {
        s32b tulos;
        char vari = 'G';
        doc_printf(doc, "  Deposits : <color:w>%8d</color>\n", _deposited);
        doc_printf(doc, "  Interest : <color:w>%8d</color>\n", _fake_deposit + _withdrawn - _deposited);
        doc_printf(doc, "  Withdrawn: <color:w>%8d</color>\n", _withdrawn);
        tulos = _withdrawn - _deposited;
        if (tulos < 0)
        {
            tulos = 0 - tulos;
            vari = 'R';
        }
        doc_printf(doc, "  Remaining: <color:w>%8d</color> <color:%c>%8d</color>\n", _fake_deposit, vari, tulos);
    }
    if (_received)
    {
        s32b tulos;
        char vari = 'G';
        doc_printf(doc, "  Insurance: <color:w>%8d</color>\n", _received);
        tulos = _paid - _received;
        if (tulos < 0)
        {
            tulos = 0 - tulos;
            vari = 'R';
        }
        doc_printf(doc, "  Payout   : <color:w>%8d</color> <color:%c>%8d</color>\n", _paid, vari, tulos);
    }
}


void cornucopia_init(void)
{
    _deposit = 0;
    _received = 0;
    _paid = 0;
    _loan = 0;
    _loan_turn = 0;
    _deposit_turn = 0;
    _next_policy = 0;
    _cornucopia_ini_policies(-1);
}

void cornucopia_save(savefile_ptr file)
{
    int i;
    savefile_write_u32b(file, _deposit);
    savefile_write_u32b(file, _received);
    savefile_write_u32b(file, _paid);
    savefile_write_u32b(file, _loan);
    savefile_write_u32b(file, _loan_turn);
    savefile_write_u32b(file, _deposit_turn);
    savefile_write_u32b(file, _loaned);
    savefile_write_u32b(file, _repaid);
    savefile_write_u32b(file, _deposited);
    savefile_write_u32b(file, _withdrawn);
    savefile_write_byte(file, _next_policy);
    savefile_write_byte(file, MAX_POLICY);
    for (i = 0; i < MAX_POLICY; i++)
    {
        savefile_write_byte(file, _my_policies[i].i_idx);
        savefile_write_byte(file, _my_policies[i].num_destroyed);
        savefile_write_u32b(file, _my_policies[i].disen_value);
        savefile_write_u32b(file, _my_policies[i].destroy_value);
        savefile_write_byte(file, _my_policies[i].restore_fully);
        savefile_write_byte(file, _my_policies[i].restore_fully_korvaus);
        savefile_write_u32b(file, _my_policies[i].compensation);
        savefile_write_byte(file, _my_policies[i].num_insured);
        if ((_my_policies[i].num_insured) && (_my_policies[i].o_ptr) && (_my_policies[i].o_ptr->k_idx))
        {
            savefile_write_s16b(file, 0x5FF9);
            obj_save(_my_policies[i].o_ptr, file);
        }
        else savefile_write_s16b(file, 0x5FF7);
    }
}

void cornucopia_load(savefile_ptr file)
{
    int i, old_policies;
    if (savefile_is_older_than(file, 7,0,9,4))
    {
        cornucopia_init();
        return;
    }
    _deposit = savefile_read_u32b(file);
    _received = savefile_read_u32b(file);
    _paid = savefile_read_u32b(file);
    _loan = savefile_read_u32b(file);
    _loan_turn = savefile_read_u32b(file);
    _deposit_turn = savefile_read_u32b(file);
    _loaned = savefile_read_u32b(file);
    _repaid = savefile_read_u32b(file);
    _deposited = savefile_read_u32b(file);
    _withdrawn = savefile_read_u32b(file);
    _next_policy = savefile_read_byte(file);
    old_policies = savefile_read_byte(file);
    assert(old_policies <= MAX_POLICY);
    for (i = 0; i < old_policies; i++)
    {
        s16b _marker;
        _my_policies[i].i_idx = savefile_read_byte(file);
        _my_policies[i].num_destroyed = savefile_read_byte(file);
        _my_policies[i].disen_value = savefile_read_u32b(file);
        _my_policies[i].destroy_value = savefile_read_u32b(file);
        _my_policies[i].restore_fully = savefile_read_byte(file) ? TRUE : FALSE;
        _my_policies[i].restore_fully_korvaus = savefile_read_byte(file) ? TRUE : FALSE;
        _my_policies[i].compensation = savefile_read_u32b(file);
        _my_policies[i].num_insured = savefile_read_byte(file);
        _marker = savefile_read_s16b(file);
        if (_marker == 0x5FF9)
        {
            obj_ptr obj = malloc(sizeof(object_type));
            object_wipe(obj);
            obj_load(obj, file);
            _my_policies[i].o_ptr = obj;
        }
        else _my_policies[i].o_ptr = NULL;
    }
    for (i = old_policies; i < MAX_POLICY; i++)
    {
        _cornucopia_ini_policies(i);
    }
}
