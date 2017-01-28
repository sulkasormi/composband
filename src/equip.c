#include "angband.h"
#include "equip.h"

#include <assert.h>

/* Slots on the equipment template now *match* slots in our inventory.
 * This never used to be the case, and coding was awkward! */
static equip_template_ptr _template = NULL;
static inv_ptr _inv = NULL;

bool _object_is_amulet(obj_ptr obj)
{
    if (obj->tval == TV_AMULET || obj->tval == TV_WHISTLE) return TRUE;
    return FALSE;
}

bool _object_is_anything(obj_ptr obj)
{
    if (TV_WEARABLE_BEGIN <= obj->tval && obj->tval <= TV_WEARABLE_END)
        return TRUE;
    return FALSE;
}

bool _object_is_body_armor(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_SOFT_ARMOR: case TV_HARD_ARMOR: case TV_DRAG_ARMOR:
        return TRUE;
    }
    return FALSE;
}

bool _object_is_boots(obj_ptr obj)
{
    if (obj->tval == TV_BOOTS) return TRUE;
    return FALSE;
}

bool _object_is_bow(obj_ptr obj)
{
    if (obj->tval == TV_BOW) return TRUE;
    return FALSE;
}

bool _object_is_cloak(obj_ptr obj)
{
    if (obj->tval == TV_CLOAK) return TRUE;
    return FALSE;
}

bool _object_is_gloves(obj_ptr obj)
{
    if (obj->tval == TV_GLOVES) return TRUE;
    return FALSE;
}

bool _object_is_helmet(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_HELM: case TV_CROWN:
        return TRUE;
    }
    return FALSE;
}

bool _object_is_lite(obj_ptr obj)
{
    if (obj->tval == TV_LITE) return TRUE;
    return FALSE;
}

bool _object_is_ring(obj_ptr obj)
{
    if (obj->tval == TV_RING) return TRUE;
    return FALSE;
}

bool _object_is_weapon(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_DIGGING: case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
        return TRUE;
    }
    return FALSE;
}

bool _object_is_weapon_or_shield(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_DIGGING: case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
    case TV_SHIELD:  case TV_CARD: case TV_CAPTURE:
        return TRUE;
    }
    return FALSE;
}

bool _object_is_capture_ball(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_CAPTURE:
        return TRUE;
    }
    return FALSE;
}

static obj_p _accept[EQUIP_SLOT_MAX] = {
    NULL,
    _object_is_gloves,
    _object_is_weapon_or_shield,
    _object_is_ring,
    _object_is_bow,
    _object_is_amulet,
    _object_is_lite,
    _object_is_body_armor,
    _object_is_cloak,
    _object_is_boots,
    _object_is_helmet,
    _object_is_anything,
    _object_is_weapon,
    _object_is_capture_ball
};

static int _slot_count(obj_ptr obj)
{
    int     ct = 0;
    slot_t  slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_p p = _accept[_template->slots[slot].type];
        if (p(obj))
            ct++;
    }
    return ct;
}

static bool _can_wield(obj_ptr obj)
{
    if (_slot_count(obj) > 0) return TRUE;
    return FALSE;
}

static int _get_slots(obj_ptr obj, slot_t slots[EQUIP_MAX_SLOTS])
{
    int    ct = 0;
    slot_t slot = equip_first_slot(obj);

    while (equip_is_valid_slot(slot))
    {
        slots[ct++] = slot;
        slot = equip_next_slot(obj, slot);
    }
    return ct;
}

static void _slot_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    slot_t  slot = ((slot_t*)cookie)[which];
    obj_ptr obj = equip_obj(slot);

    switch (cmd)
    {
    case MENU_KEY:
        var_set_int(res, slot - 1 + 'a');
        break;
    case MENU_TEXT:
        if (obj)
        {
            char buf[MAX_NLEN+50];
            char o_name[MAX_NLEN];
            object_desc(o_name, obj, 0);
            sprintf(buf, "%-14s: %s", b_tag + _template->slots[slot].tag, o_name);
            var_set_string(res, buf);
        }
        else
        {
            char buf[MAX_NLEN+50];
            sprintf(buf, "%-14s:", b_tag + _template->slots[slot].tag);
            var_set_string(res, buf);
        }
        break;
    case MENU_COLOR:
        if (obj)
        {
            if (obj->timeout)
                var_set_int(res, TERM_L_DARK);
            else
                var_set_int(res, tval_to_attr[obj->tval % 128]);
        }
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static slot_t _prompt_wield_slot(obj_ptr obj)
{
    slot_t slots[EQUIP_MAX_SLOTS];
    int    ct = _get_slots(obj, slots);

    if (ct == 1)
        return slots[0];
    else if (ct > 1)
    {
        int    idx;
        menu_t menu = { "Choose an equipment slot", NULL, NULL,
                        _slot_menu_fn, slots, ct };

        idx = menu_choose(&menu);
        if (idx >= 0)
            return slots[idx];
    }

    return 0;
}

/*************************************************************************
 Public Interface
 *************************************************************************/
cptr equip_describe_slot(slot_t slot)
{
    if (_template->slots[slot].type == EQUIP_SLOT_WEAPON_SHIELD || _template->slots[slot].type == EQUIP_SLOT_WEAPON)
    {
        int hand = _template->slots[slot].hand;
        if (p_ptr->weapon_info[hand].heavy_wield)
            return "Just Lifting";
        if (p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS && !prace_is_(RACE_MON_SWORD))
        {
            if (p_ptr->current_r_idx == MON_BLOODTHIRSTER)
                return "Both Paws";
            else
                return "Both Arms";
        }
        if (p_ptr->weapon_info[hand].riding)
            return "Riding Reins";
    }
    if (_template->slots[slot].type == EQUIP_SLOT_BOW)
    {
        if (p_ptr->shooter_info.heavy_shoot)
            return "Just Holding";
    }
    return b_tag + _template->slots[slot].tag;
}

slot_t equip_find_art(int which)
{
    return inv_find_art(_inv, which);
}

slot_t equip_find_ego(int which)
{
    return inv_find_ego(_inv, which);
}

slot_t equip_find_obj(int tval, int sval)
{
    return inv_find_obj(_inv, tval, sval);
}

slot_t equip_find_first(obj_p p)
{
    return inv_first(_inv, p);
}

slot_t equip_find_next(obj_p p, slot_t prev_match)
{
    return inv_next(_inv, p, prev_match);
}

slot_t equip_first_empty_slot(obj_ptr obj)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_p p = _accept[_template->slots[slot].type];
        if (p(obj) && !equip_obj(slot))
            return slot;
    }
    return 0;
}

slot_t equip_find_empty_hand(void)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        if ( (_template->slots[slot].type == EQUIP_SLOT_WEAPON_SHIELD || _template->slots[slot].type == EQUIP_SLOT_WEAPON)
          && !equip_obj(slot) )
        {
            return slot;
        }
    }
    return 0;
}

bool equip_can_wield_kind(int tval, int sval)
{
    object_type forge;
    int         k_idx = lookup_kind(tval, sval);

    object_prep(&forge, k_idx);
    if (equip_first_slot(&forge))
        return TRUE;

    return FALSE;
}

int equip_first_slot(obj_ptr obj)
{
    return equip_next_slot(obj, EQUIP_BEGIN - 1);
}

int equip_next_slot(obj_ptr obj, int last)
{
    slot_t slot;
    for (slot = last + 1; slot <= _template->max; slot++)
    {
        obj_p p = _accept[_template->slots[slot].type];
        if (p(obj))
            return slot;
    }
    return 0;
}

bool equip_is_valid_slot(slot_t slot)
{
    if (slot >= 1 && slot <= _template->max)
        return TRUE;
    return FALSE;
}

bool equip_verify_slot(slot_t slot, obj_ptr obj)
{
    if (equip_is_valid_slot(slot))
    {
        obj_p p = _accept[_template->slots[slot].type];
        if (p(obj))
            return TRUE;
    }
    return FALSE;
}

void equip_for_each_obj(obj_f f)
{
    inv_for_each(_inv, f);
}

int equip_weight(obj_p p)
{
    return inv_weight(_inv, p);
}

int equip_count_used(void)
{
    return inv_count_slots(_inv, obj_exists);
}

slot_t equip_is_worn(obj_ptr obj)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        object_type *o = equip_obj(slot);
        if (o == obj)
            return slot;
    }
    return 0;
}

int equip_which_hand(obj_ptr obj)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        object_type *o = equip_obj(slot);
        if (o == obj)
            return _template->slots[slot].hand;
    }
    return HAND_NONE;
}

void equip_for_each_slot(slot_f f)
{
    inv_for_each_slot(_inv, f);
}

int equip_random_slot(obj_p p)
{
    return inv_random_slot(_inv, p);
}

obj_ptr equip_obj(slot_t slot)
{
    return inv_obj(_inv, slot);
}

int equip_max(void)
{
    return _template->max;
}

int equip_slot_type(slot_t slot)
{
    if (equip_is_valid_slot(slot))
        return _template->slots[slot].type;
    return EQUIP_SLOT_NONE;
}

bool equip_is_empty_two_handed_slot(int slot)
{
    int idx = slot - EQUIP_BEGIN;
    if (equip_obj(slot)) return FALSE;

    if (_template->slots[idx].type == EQUIP_SLOT_WEAPON_SHIELD)
    {
        int hand = _template->slots[idx].hand;
        int arm = hand / 2;
        int rhand = arm*2;
        int lhand = arm*2 + 1;
        int other_hand = (hand == rhand) ? lhand : rhand;

        if (p_ptr->weapon_info[other_hand].wield_how == WIELD_TWO_HANDS)
            return TRUE;
    }
    return FALSE;
}

/************************************************************************
 * Wielding
 ***********************************************************************/
typedef struct { int item; obj_ptr obj; } _obj_get_t, *_obj_get_ptr; /* TODO: Rewrite get_item */

/* Wielding has the following phases where various things might/must happen */
static _obj_get_t _wield_get_obj(void);
static bool       _wield_verify(obj_ptr obj);
static slot_t     _wield_get_slot(obj_ptr obj);
static bool       _wield_confirm(obj_ptr obj, slot_t slot);
static void       _wield_before(obj_ptr obj, slot_t slot);
static void       _wield(obj_ptr obj, slot_t slot);
static void       _wield_after(slot_t slot);

void equip_wield(void)
{
    slot_t     slot;
    _obj_get_t obj_get = _wield_get_obj();

    if (!obj_get.obj) return;
    if (!_wield_verify(obj_get.obj)) return;

    slot = _wield_get_slot(obj_get.obj);
    if (!slot) return;
    if (!_wield_confirm(obj_get.obj, slot)) return;

    _wield_before(obj_get.obj, slot);

    energy_use = weaponmaster_wield_hack(obj_get.obj);
    _wield(obj_get.obj, slot);

    _wield_after(slot);

    /* TODO: obj_get_update(&obj_get); */
    if (obj_get.item >= 0)
    {
        inven_item_increase(obj_get.item, -1);
        inven_item_optimize(obj_get.item);
    }
    else
    {
        floor_item_increase(-obj_get.item, -1);
        floor_item_optimize(-obj_get.item);
    }

}

void equip_wield_aux(obj_ptr obj, slot_t slot)
{
    _wield(obj, slot);
    _wield_after(slot);
}

static _obj_get_t _wield_get_obj(void)
{
    _obj_get_t obj_get = {0};

    item_tester_hook = _can_wield;
    if (get_item(&obj_get.item, "Wear/Wield which item? ",
            "You have nothing you can wear or wield.", USE_INVEN | USE_FLOOR))
    {
        if (obj_get.item >= 0)
            obj_get.obj = &inventory[obj_get.item];
        else
            obj_get.obj = &o_list[-obj_get.item];
    }

    return obj_get;
}

static bool _wield_verify(obj_ptr obj)
{
    if (!psion_can_wield(obj)) return FALSE;
    /* We'll confirm cursed gear later (_wield_confirm)
     * since the user might cancle the slot prompt */
    return TRUE;
}

static slot_t _wield_get_slot(obj_ptr obj)
{
    slot_t slot = _prompt_wield_slot(obj);
    if (!equip_is_valid_slot(slot)) return 0;
    return slot;
}

static bool _wield_confirm(obj_ptr obj, slot_t slot)
{
    obj_ptr old_obj = inv_obj(_inv, slot);
    char    o_name[MAX_NLEN];

    if (old_obj && have_flag(old_obj->flags, OF_NO_REMOVE)) /* Hack!!!! */
    {
        msg_print("You can't replace yourself with that!");
        return FALSE;
    }

    if (old_obj && object_is_cursed(old_obj))
    {
        object_desc(o_name, old_obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED);
        msg_format("The %s you are wearing appears to be cursed.", o_name);
        return FALSE;
    }

    if (confirm_wear)
    {
        bool do_prompt = FALSE;

        if (object_is_known(obj) && object_is_cursed(obj))
        {
            do_prompt = TRUE;
        }
        else if (obj->ident & IDENT_SENSE)
        {
            switch (obj->feeling)
            {
            case FEEL_BROKEN:
            case FEEL_BAD:
            case FEEL_TERRIBLE:
            case FEEL_AWFUL:
            case FEEL_CURSED:
                do_prompt = TRUE;
                break;
            }
        }
        if (do_prompt)
        {
            char dummy[MAX_NLEN+80];
            object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED);
            sprintf(dummy, "Really use the %s {cursed}? ", o_name);
            if (!get_check(dummy)) return FALSE;
        }
    }
    if ( obj->name1 == ART_STONEMASK
      && object_is_known(obj)
      && p_ptr->prace != RACE_VAMPIRE
      && p_ptr->prace != RACE_ANDROID
      && !(get_race()->flags & RACE_IS_MONSTER)
      && p_ptr->pclass != CLASS_BLOOD_KNIGHT)
    {
        char dummy[MAX_NLEN+80];
        object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY);
        msg_format("%s will transforms you into a vampire permanently when equiped.", o_name);
        sprintf(dummy, "Do you become a vampire?");
        if (!get_check(dummy)) return FALSE;
    }
    return TRUE;
}

static void _wield_before(obj_ptr obj, slot_t slot)
{
    int i;

    if (p_ptr->special_defense & KATA_MUSOU)
        set_action(ACTION_NONE);

    for (i = 0; i < max_quests; i++)
    {
        if ( quest[i].type == QUEST_TYPE_FIND_ARTIFACT
          && quest[i].status == QUEST_STATUS_TAKEN
          && (quest[i].k_idx == obj->name1 || quest[i].k_idx == obj->name3) )
        {
            quest[i].status = QUEST_STATUS_COMPLETED;
            p_ptr->redraw |= PR_DEPTH;
            quest[i].complev = p_ptr->lev;
            cmsg_print(TERM_L_BLUE, "You completed the quest!");
        }
    }
}

static void _wield(obj_ptr obj, slot_t slot)
{
    obj_ptr old_obj = inv_obj(_inv, slot);

    if (old_obj)
        equip_takeoff_aux(slot);

    inv_add_at(_inv, obj, slot);
    /* TODO: obj->number--; */
}

static void _wield_after(slot_t slot)
{
    char    o_name[MAX_NLEN];
    obj_ptr obj = inv_obj(_inv, slot);

    if (p_ptr->personality == PERS_MUNCHKIN)
    {
        identify_item(obj);
        autopick_alter_item(slot, FALSE);
    }
    obj_learn_equipped(obj);
    stats_on_equip(obj);
    obj->marked |= OM_TOUCHED;
    obj->marked &= ~OM_WORN;

    /* Hack: Extra Might and Weaponmastery require a calc_bonus() to display correctly */
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    object_desc(o_name, obj, OD_COLOR_CODED);
    if (p_ptr->prace == RACE_MON_SWORD || p_ptr->prace == RACE_MON_RING)
        msg_format("You are %s.", o_name);
    else
        msg_format("You are wearing %s (%c).", o_name, slot - 1 + 'a');

    /* After Effects? */
    if (object_is_cursed(obj))
    {
        msg_print("Oops! It feels deathly cold!");
        virtue_add(VIRTUE_HARMONY, -1);
        obj->ident |= IDENT_SENSE;
    }
    if (obj->name1 == ART_HAND_OF_VECNA)
    {
        cmsg_print(TERM_VIOLET, "You chop off your own hand to wield the Hand of Vecna!");
        set_cut(CUT_MORTAL_WOUND, FALSE);
    }
    if (obj->name1 == ART_EYE_OF_VECNA)
    {
        cmsg_print(TERM_VIOLET, "You pluck out your own eye to wield the Eye of Vecna!");
        set_cut(CUT_MORTAL_WOUND, FALSE);
    }
    if ( obj->name1 == ART_STONEMASK
      && p_ptr->prace != RACE_VAMPIRE
      && p_ptr->prace != RACE_ANDROID
      && p_ptr->pclass != CLASS_BLOOD_KNIGHT )
    {
        change_race(RACE_VAMPIRE, "");
    }

    p_ptr->update |= PU_BONUS;
    p_ptr->update |= PU_TORCH;
    p_ptr->update |= PU_MANA;
    p_ptr->redraw |= PR_EQUIPPY;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
    android_calc_exp();
}

/************************************************************************
 * Unwielding (Take Off)
 ***********************************************************************/
void equip_takeoff(void)
{
    int slot;
    obj_ptr obj;

    if (p_ptr->special_defense & KATA_MUSOU)
        set_action(ACTION_NONE);

    if (!get_item(&slot, "Take off which item? ", "You are not wearing anything to take off.", USE_EQUIP)) return;
    obj = &inventory[slot];

    if (!psion_can_wield(obj)) return;
    if (have_flag(obj->flags, OF_NO_REMOVE)) /* Hack!!!! */
    {
        msg_print("You try to take yourself off, but fail!");
        return;
    }

    if (object_is_cursed(obj))
    {
        if (obj->curse_flags & OFC_PERMA_CURSE || (p_ptr->pclass != CLASS_BERSERKER))
        {
            msg_print("Hmmm, it seems to be cursed.");
            return;
        }
        if (((obj->curse_flags & OFC_HEAVY_CURSE) && one_in_(7)) || one_in_(4))
        {
            msg_print("You teared a cursed equipment off by sheer strength!");
            obj->ident |= IDENT_SENSE;
            obj->curse_flags = 0L;
            obj->feeling = FEEL_NONE;
            p_ptr->update |= PU_BONUS;
            p_ptr->window |= PW_EQUIP;
            p_ptr->redraw |= PR_EFFECTS;

            msg_print("You break the curse.");
        }
        else
        {
            msg_print("You couldn't remove the equipment.");
            energy_use = 50;
            return;
        }
    }
    energy_use = 50;
    equip_takeoff_aux(slot);
    if (weaponmaster_is_(WEAPONMASTER_SHIELDS))
        handle_stuff();

    android_calc_exp();
}

void equip_takeoff_aux(int slot)
{
    obj_ptr obj = equip_obj(slot);

    if (obj)
    {
        object_type copy;
        int  new_slot;
        char o_name[MAX_NLEN];

        object_copy(&copy, obj);
        object_desc(o_name, &copy, OD_COLOR_CODED);
        inven_item_increase(slot, -1);
        inven_item_optimize(slot);

        new_slot = inven_carry(&copy);
        msg_format("You were using %s (%c).", o_name, index_to_label(new_slot));

        p_ptr->update |= PU_BONUS;
        p_ptr->update |= PU_TORCH;
        p_ptr->update |= PU_MANA;
        p_ptr->redraw |= PR_EQUIPPY;
        p_ptr->window |= PW_INVEN | PW_EQUIP;
    }
}

/************************************************************************
 * Calc Bonuses ... This monster has the dreaded RF2_ELDRITCH_HORROR flag!
 ***********************************************************************/

/* Rings and Gloves are somewhat complicated.
   We support an arbitrary number of hands (cf MAX_HANDS) paired
   into an arbitrary number of sets of arms (cf MAX_ARMS). Gloves
   affect both weapons for that arm, possibly with proration. Rings
   affect the hand in question, unless the other hand on that set
   of arms is wielding a weapon two handed.
 */
static void _weapon_info_flag(slot_t slot, u32b flgs[OF_ARRAY_SIZE], int flg)
{
    if (have_flag(flgs, flg))
    {
        int hand = _template->slots[slot].hand;
        int arm = hand / 2;
        int rhand = arm*2;
        int lhand = arm*2 + 1;
        int other_hand = (hand == rhand) ? lhand : rhand;

        switch (_template->slots[slot].type)
        {
        case EQUIP_SLOT_GLOVES:
            if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
                add_flag(p_ptr->weapon_info[rhand].flags, flg);
            if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
                add_flag(p_ptr->weapon_info[lhand].flags, flg);
            break;
        case EQUIP_SLOT_RING:
            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                add_flag(p_ptr->weapon_info[hand].flags, flg);
            else if (p_ptr->weapon_info[other_hand].wield_how == WIELD_TWO_HANDS)
                add_flag(p_ptr->weapon_info[other_hand].flags, flg);
            break;
        default:
            for (hand = 0; hand < MAX_HANDS; hand++)
            {
                if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                    add_flag(p_ptr->weapon_info[hand].flags, flg);
            }
            break;
        }
    }
}

static void _weaponmastery(slot_t slot, int amt)
{
    int hand = _template->slots[slot].hand;
    int arm = hand / 2;
    int rhand = arm*2;
    int lhand = arm*2 + 1;
    int other_hand = (hand == rhand) ? lhand : rhand;

    switch (_template->slots[slot].type)
    {
    case EQUIP_SLOT_RING:
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            p_ptr->weapon_info[hand].to_dd += amt;
        else if (p_ptr->weapon_info[other_hand].wield_how == WIELD_TWO_HANDS)
            p_ptr->weapon_info[other_hand].to_dd += amt;
        else
            p_ptr->innate_attack_info.to_dd += amt;
        break;
    case EQUIP_SLOT_ANY:
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            p_ptr->weapon_info[hand].to_dd += amt;
        else
            p_ptr->innate_attack_info.to_dd += amt;
        break;
    default: /* At the moment, this is just the Robe of the Kamikaze Warrior (+2) */
        if (p_ptr->weapon_ct)
        {
            for (hand = 0; hand < MAX_HANDS; hand++)
            {
                if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                    p_ptr->weapon_info[hand].to_dd += amt / p_ptr->weapon_ct;
            }
        }
        else if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE) /* TODO: I'm not sure martial arts should boost the weapon_ct ... */
            p_ptr->weapon_info[hand].to_dd += amt;
        else
            p_ptr->innate_attack_info.to_dd += amt;
    }
}

static void _weapon_bonus_hand(int hand, int to_h, int to_d, bool known)
{
    p_ptr->weapon_info[hand].to_h += to_h;
    p_ptr->weapon_info[hand].to_d += to_d;
    if (known)
    {
        p_ptr->weapon_info[hand].dis_to_h += to_h;
        p_ptr->weapon_info[hand].dis_to_d += to_d;
    }
}

static int _sign(int n)
{
    if (n > 0) return 1;
    if (n < 0) return -1;
    return 0;
}

static void _weapon_bonus(slot_t slot, int to_h, int to_d)
{
    int hand = _template->slots[slot].hand;
    int arm = hand / 2;
    int rhand = arm*2;
    int lhand = arm*2 + 1;
    int other_hand = (hand == rhand) ? lhand : rhand;
    obj_ptr obj = inv_obj(_inv, slot);

    if (!p_ptr->weapon_ct) return;

    switch (_template->slots[slot].type)
    {
    case EQUIP_SLOT_GLOVES:
        if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE && p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
        {
            _weapon_bonus_hand(rhand, (to_h + 1) / 2, (to_d + 1) / 2, object_is_known(obj));
            _weapon_bonus_hand(lhand, to_h/2, to_d/2, object_is_known(obj));
        }
        else if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
            _weapon_bonus_hand(rhand, to_h, to_d, object_is_known(obj));
        else if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
            _weapon_bonus_hand(lhand, to_h, to_d, object_is_known(obj));
        break;
    case EQUIP_SLOT_RING:
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            _weapon_bonus_hand(hand, to_h, to_d, object_is_known(obj));
        else if (p_ptr->weapon_info[other_hand].wield_how == WIELD_TWO_HANDS)
            _weapon_bonus_hand(other_hand, to_h, to_d, object_is_known(obj));
        break;
    default:
    {
        int x_to_h = to_h - (to_h/p_ptr->weapon_ct)*p_ptr->weapon_ct;
        int x_to_d = to_d - (to_d/p_ptr->weapon_ct)*p_ptr->weapon_ct;
        for (hand = 0; hand < MAX_HANDS; hand++)
        {
            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                _weapon_bonus_hand(hand, to_h/p_ptr->weapon_ct, to_d/p_ptr->weapon_ct, object_is_known(obj));
        }
        for (hand = 0; hand < MAX_HANDS && (x_to_h || x_to_d); hand++)
        {
            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            {
                _weapon_bonus_hand(hand, _sign(x_to_h), _sign(x_to_d), object_is_known(obj));
                if (x_to_h > 0) x_to_h--;
                else if (x_to_h < 0) x_to_h++;
                if (x_to_d > 0) x_to_d--;
                else if (x_to_d < 0) x_to_d++;
            }
        }
    }
    }
}

bool equip_is_valid_hand(int hand)
{
    return p_ptr->weapon_info[hand].slot;
}

bool equip_is_empty_hand(int hand)
{
    return equip_is_valid_hand(hand)
        && !equip_obj(p_ptr->weapon_info[hand].slot);
}

void equip_calc_bonuses(void)
{
    slot_t slot;
    int    i;

    /* Find the weapons */
    for (slot = 1; slot <= _template->max; slot++)
    {
        if ( _template->slots[slot].type == EQUIP_SLOT_WEAPON_SHIELD
          || _template->slots[slot].type == EQUIP_SLOT_WEAPON )
        {
            obj_ptr obj;
            int     hand = _template->slots[slot].hand;

            p_ptr->weapon_info[hand].slot = slot;
            p_ptr->weapon_info[hand].wield_how = WIELD_NONE;
            obj = equip_obj(slot);

            if (obj)
            {
                if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH)
                {
                    if (object_is_shield(obj))
                        p_ptr->weapon_info[hand].wield_how = WIELD_ONE_HAND;
                }
                else if (object_is_melee_weapon(obj))
                {
                    p_ptr->weapon_info[hand].wield_how = WIELD_ONE_HAND;
                    if (obj->rune == RUNE_AIR)
                        p_ptr->weapon_info[hand].xtra_blow += 100;
                }
            }
            /* N.B. if (p_ptr->monk_lvl) would be a simpler check, but class_t.calc_bonuses gets called after equip_calc_bonuses ... */
            else if ( p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_MYSTIC || p_ptr->pclass == CLASS_FORCETRAINER
                   || (p_ptr->pclass == CLASS_SKILLMASTER && skillmaster_martial_arts_prof() > 0) )
            {
                p_ptr->weapon_info[hand].wield_how = WIELD_ONE_HAND;
                p_ptr->weapon_info[hand].bare_hands = TRUE;
            }
        }
    }

    /* Patch up for monks using weapons (with empty hands too) */
    for (i = 0; i < MAX_ARMS; i++)
    {
        int rhand = i*2;
        int lhand = i*2+1;

        if (!equip_is_valid_hand(rhand) || !equip_is_valid_hand(lhand)) continue;

        if ( p_ptr->weapon_info[rhand].wield_how != WIELD_NONE
          && p_ptr->weapon_info[rhand].bare_hands
          && p_ptr->weapon_info[lhand].wield_how != WIELD_NONE
          && equip_obj(p_ptr->weapon_info[lhand].slot) )
        {
            p_ptr->weapon_info[rhand].wield_how = WIELD_NONE;
            p_ptr->weapon_info[rhand].bare_hands = FALSE;
        }
        if ( p_ptr->weapon_info[lhand].wield_how != WIELD_NONE
          && p_ptr->weapon_info[lhand].bare_hands
          && p_ptr->weapon_info[rhand].wield_how != WIELD_NONE
          && equip_obj(p_ptr->weapon_info[rhand].slot) )
        {
            p_ptr->weapon_info[lhand].wield_how = WIELD_NONE;
            p_ptr->weapon_info[lhand].bare_hands = FALSE;
        }
    }

    /* Control mount with a free hand if possible using the last available hand */
    if (p_ptr->riding)
    {
        p_ptr->riding_ryoute = TRUE;
        if (p_ptr->prace == RACE_MON_RING)
        {
            p_ptr->riding_ryoute = FALSE;
        }
        else if (!(p_ptr->pet_extra_flags & PF_RYOUTE))
        {
            for (i = MAX_HANDS - 1; i >= 0; i--)
            {
                if (equip_is_empty_hand(i))
                {
                    p_ptr->weapon_info[i].riding = TRUE;
                    p_ptr->weapon_info[i].wield_how = WIELD_NONE;
                    p_ptr->weapon_info[i].bare_hands = FALSE;
                    p_ptr->riding_ryoute = FALSE;
                    break;
                }
            }
        }
    }

    /* Figure out which weapons are being used with 2 hands */
    if (CAN_TWO_HANDS_WIELDING())
    {
        for (i = 0; i < MAX_ARMS; i++)
        {
            int rhand = 2*i;
            int lhand = 2*i+1;

            if ( p_ptr->weapon_info[rhand].wield_how == WIELD_ONE_HAND
              && (p_ptr->weapon_info[rhand].bare_hands || object_allow_two_hands_wielding(equip_obj(p_ptr->weapon_info[rhand].slot))))
            {
                if (equip_is_empty_hand(lhand))
                {
                    p_ptr->weapon_info[rhand].wield_how = WIELD_TWO_HANDS;
                    p_ptr->weapon_info[lhand].wield_how = WIELD_NONE;
                }
                /* Hack for Shieldmaster */
                else if ( weaponmaster_is_(WEAPONMASTER_SHIELDS)
                       && weaponmaster_get_toggle() != TOGGLE_SHIELD_BASH
                       && equip_is_valid_hand(lhand)
                       && object_is_shield(equip_obj(p_ptr->weapon_info[lhand].slot)) )
                {
                    p_ptr->weapon_info[rhand].wield_how = WIELD_TWO_HANDS;
                    p_ptr->weapon_info[lhand].wield_how = WIELD_NONE;
                }
            }

            if ( p_ptr->weapon_info[lhand].wield_how == WIELD_ONE_HAND
              && (p_ptr->weapon_info[lhand].bare_hands || object_allow_two_hands_wielding(equip_obj(p_ptr->weapon_info[lhand].slot))))
            {
                if (equip_is_empty_hand(rhand))
                {
                    p_ptr->weapon_info[lhand].wield_how = WIELD_TWO_HANDS;
                    p_ptr->weapon_info[rhand].wield_how = WIELD_NONE;
                }
                /* Hack for Shieldmaster */
                else if ( weaponmaster_is_(WEAPONMASTER_SHIELDS)
                       && weaponmaster_get_toggle() != TOGGLE_SHIELD_BASH
                       && equip_is_valid_hand(rhand)
                       && object_is_shield(equip_obj(p_ptr->weapon_info[rhand].slot)) )
                {
                    p_ptr->weapon_info[lhand].wield_how = WIELD_TWO_HANDS;
                    p_ptr->weapon_info[rhand].wield_how = WIELD_NONE;
                }
            }
        }
    }

    /* Hack for Death Swords ... but not Broken Death Swords ;) */
    if (prace_is_(RACE_MON_SWORD) && p_ptr->lev >= 10)
        p_ptr->weapon_info[0].wield_how = WIELD_TWO_HANDS;

    /* Its convenient to have an accurate weapon count later */
    p_ptr->weapon_ct = 0;
    for (i = 0; i < MAX_HANDS; i++)
    {
        if (p_ptr->weapon_info[i].wield_how != WIELD_NONE)
            p_ptr->weapon_ct++;
    }

    /* Scan equipment for bonuses. */
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        u32b    flgs[OF_ARRAY_SIZE];
        int     bonus_to_h, bonus_to_d;

        if (!obj->k_idx) continue;

        obj_flags(obj, flgs);

        p_ptr->cursed |= obj->curse_flags;
        if (p_ptr->cursed)
            p_ptr->redraw |= PR_EFFECTS;
        if (obj->name1 == ART_CHAINSWORD) p_ptr->cursed |= OFC_CHAINSWORD;

        if (obj->name1 == ART_MAUL_OF_VICE)
            p_ptr->maul_of_vice = TRUE;

        if (have_flag(flgs, OF_LORE2))
            p_ptr->auto_id = TRUE;
        else if (have_flag(flgs, OF_LORE1))
            p_ptr->auto_pseudo_id = TRUE;

        if (obj->name2 == EGO_GLOVES_GIANT)
        {
            int hand = _template->slots[slot].hand;
            int arm = hand / 2;
            int rhand = arm*2;
            int lhand = arm*2 + 1;
            if (p_ptr->weapon_info[rhand].wield_how == WIELD_TWO_HANDS)
                p_ptr->weapon_info[rhand].giant_wield = obj->pval;
            else if (p_ptr->weapon_info[lhand].wield_how == WIELD_TWO_HANDS)
                p_ptr->weapon_info[lhand].giant_wield = obj->pval;
        }

        if (obj->rune)
        {
            rune_calc_bonuses(obj);
            rune_calc_stats(obj, p_ptr->stat_add);
        }

        if (have_flag(flgs, OF_STR)) p_ptr->stat_add[A_STR] += obj->pval;
        if (have_flag(flgs, OF_INT)) p_ptr->stat_add[A_INT] += obj->pval;
        if (have_flag(flgs, OF_WIS)) p_ptr->stat_add[A_WIS] += obj->pval;
        if (have_flag(flgs, OF_DEX)) p_ptr->stat_add[A_DEX] += obj->pval;
        if (have_flag(flgs, OF_CON)) p_ptr->stat_add[A_CON] += obj->pval;
        if (have_flag(flgs, OF_CHR)) p_ptr->stat_add[A_CHR] += obj->pval;

        if (have_flag(flgs, OF_DEC_STR)) p_ptr->stat_add[A_STR] -= obj->pval;
        if (have_flag(flgs, OF_DEC_INT)) p_ptr->stat_add[A_INT] -= obj->pval;
        if (have_flag(flgs, OF_DEC_WIS)) p_ptr->stat_add[A_WIS] -= obj->pval;
        if (have_flag(flgs, OF_DEC_DEX)) p_ptr->stat_add[A_DEX] -= obj->pval;
        if (have_flag(flgs, OF_DEC_CON)) p_ptr->stat_add[A_CON] -= obj->pval;
        if (have_flag(flgs, OF_DEC_CHR)) p_ptr->stat_add[A_CHR] -= obj->pval;

        if (have_flag(flgs, OF_MAGIC_MASTERY))
            p_ptr->skills.dev += 8*obj->pval;

        if (have_flag(flgs, OF_DEVICE_POWER))
            p_ptr->device_power += obj->pval;

        if (have_flag(flgs, OF_DEC_MAGIC_MASTERY))
        {
            p_ptr->skills.dev -= 8*obj->pval;
            p_ptr->device_power -= obj->pval;
        }

        if (have_flag(flgs, OF_STEALTH)) p_ptr->skills.stl += obj->pval;
        if (have_flag(flgs, OF_DEC_STEALTH)) p_ptr->skills.stl -= obj->pval;
        if (have_flag(flgs, OF_SEARCH)) p_ptr->skills.srh += (obj->pval * 5);
        if (have_flag(flgs, OF_SEARCH)) p_ptr->skills.fos += (obj->pval * 5);
        if (have_flag(flgs, OF_INFRA)) p_ptr->see_infra += obj->pval;
        if (have_flag(flgs, OF_TUNNEL)) p_ptr->skill_dig += (obj->pval * 20);
        if (have_flag(flgs, OF_SPEED)) p_ptr->pspeed += obj->pval;
        if (have_flag(flgs, OF_DEC_SPEED)) p_ptr->pspeed -= obj->pval;

        if (have_flag(flgs, OF_BLOWS) || have_flag(flgs, OF_DEC_BLOWS))
        {
            int hand = _template->slots[slot].hand;
            int amt = 0;

            if (have_flag(flgs, OF_BLOWS))
                amt += obj->pval * 50;
            if (have_flag(flgs, OF_DEC_BLOWS))
                amt -= obj->pval * 100;
            if (p_ptr->pclass == CLASS_MAULER && amt > 0)
                amt = 0;

            switch (_template->slots[slot].type)
            {
            case EQUIP_SLOT_GLOVES:
            {
                int arm = hand / 2;
                int rhand = arm*2;
                int lhand = arm*2 + 1;
                if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE && p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
                {
                    if (amt > 0)
                    {
                        p_ptr->weapon_info[rhand].xtra_blow += amt/2;
                        p_ptr->weapon_info[lhand].xtra_blow += amt/2;
                    }
                    else
                    {
                        p_ptr->weapon_info[rhand].xtra_blow += amt;
                        p_ptr->weapon_info[lhand].xtra_blow += amt;
                    }
                }
                else if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
                    p_ptr->weapon_info[rhand].xtra_blow += amt;
                else if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
                    p_ptr->weapon_info[lhand].xtra_blow += amt;
                else
                    p_ptr->innate_attack_info.xtra_blow += amt;
                break;
            }
            case EQUIP_SLOT_RING:
                if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                    p_ptr->weapon_info[hand].xtra_blow += amt;
                else
                {
                    int other_hand;
                    if (hand % 2 == 0)
                        other_hand = hand + 1;
                    else
                        other_hand = hand - 1;
                    if (p_ptr->weapon_info[other_hand].wield_how == WIELD_TWO_HANDS)
                        p_ptr->weapon_info[other_hand].xtra_blow += amt;
                    else if (p_ptr->weapon_info[other_hand].wield_how == WIELD_NONE)
                        p_ptr->innate_attack_info.xtra_blow += amt;
                }
                break;
            case EQUIP_SLOT_WEAPON_SHIELD:
            case EQUIP_SLOT_WEAPON:
                if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
                    p_ptr->weapon_info[hand].xtra_blow += amt;
                break;
            default:
            {
                if (object_is_melee_weapon(obj)) break; /* Hack for Jellies ... */
                if (p_ptr->weapon_ct)
                {
                    int  j;
                    for (j = 0; j < MAX_HANDS; j++)
                    {
                        if (p_ptr->weapon_info[j].wield_how != WIELD_NONE)
                            p_ptr->weapon_info[j].xtra_blow += amt/p_ptr->weapon_ct;
                    }
                }
                else
                    p_ptr->innate_attack_info.xtra_blow += amt;
            }
            }
        }

        /* New: Rings and Gloves can grant weapon slays */
        if ( !object_is_melee_weapon(obj) /* Hack for Jellies ... */
          && !object_is_bow(obj) )
        {
            _weapon_info_flag(slot, flgs, OF_BRAND_FIRE);
            _weapon_info_flag(slot, flgs, OF_BRAND_COLD);
            _weapon_info_flag(slot, flgs, OF_BRAND_ELEC);
            _weapon_info_flag(slot, flgs, OF_BRAND_ACID);
            _weapon_info_flag(slot, flgs, OF_BRAND_POIS);
            _weapon_info_flag(slot, flgs, OF_IMPACT);     /* Quaker */
            _weapon_info_flag(slot, flgs, OF_SLAY_GOOD);  /* Thanos, Nazgul */
            _weapon_info_flag(slot, flgs, OF_SLAY_HUMAN); /* Nazgul */
            _weapon_info_flag(slot, flgs, OF_BRAND_VAMP); /* Dragon Armor (Death), Helm of the Vampire */
        }

        if (have_flag(flgs, OF_XTRA_SHOTS))
            p_ptr->shooter_info.num_fire += 25 * obj->pval;

        if (have_flag(flgs, OF_LIFE))
            p_ptr->life += 3*obj->pval;
        if (have_flag(flgs, OF_DEC_LIFE))
            p_ptr->life -= 3*obj->pval;

        if (have_flag(flgs, OF_AGGRAVATE))   p_ptr->cursed |= OFC_AGGRAVATE;
        if (have_flag(flgs, OF_DRAIN_EXP))   p_ptr->cursed |= OFC_DRAIN_EXP;
        if (have_flag(flgs, OF_TY_CURSE))    p_ptr->cursed |= OFC_TY_CURSE;

        if (have_flag(flgs, OF_DEC_MANA))
        {
            /* In general, you need to be a Mage/Priest to gain from wizardstaves, et. al.
             * There are exceptions (e.g. Bards and the two artifact harps; Vampires
             * and The Amulet of the Pitch Dark Night; etc). You will find code
             * for these exceptions in class/race specific calc_bonuses functions.
             * However, The Yumi of Irresponsibility works for everybody! */
            if (obj->name1 == ART_NAMAKE_BOW)
            {
                p_ptr->dec_mana = TRUE;
            }
            else
            {
                caster_info *caster_ptr = get_caster_info();
                if (caster_ptr && (caster_ptr->options & CASTER_ALLOW_DEC_MANA))
                    p_ptr->dec_mana = TRUE;
            }

        }
        if (have_flag(flgs, OF_EASY_SPELL))
        {
            caster_info *caster_ptr = get_caster_info();
            if (caster_ptr && (caster_ptr->options & CASTER_ALLOW_DEC_MANA))
                p_ptr->easy_spell = TRUE;
        }

        if (have_flag(flgs, OF_SPELL_POWER)) p_ptr->spell_power += obj->pval;
        if (have_flag(flgs, OF_DEC_SPELL_POWER)) p_ptr->spell_power -= obj->pval;
        if (have_flag(flgs, OF_SPELL_CAP))   p_ptr->spell_cap += obj->pval;
        if (have_flag(flgs, OF_DEC_SPELL_CAP))   p_ptr->spell_cap -= obj->pval;
        if (have_flag(flgs, OF_MAGIC_RESISTANCE))   p_ptr->magic_resistance += 5*obj->pval;

        if (have_flag(flgs, OF_XTRA_MIGHT) && obj->tval != TV_BOW)
            p_ptr->shooter_info.to_mult += 25 * obj->pval;

        if (have_flag(flgs, OF_SLOW_DIGEST)) p_ptr->slow_digest = TRUE;
        if (have_flag(flgs, OF_REGEN))       p_ptr->regen += 100;
        if (have_flag(flgs, OF_TELEPATHY))   p_ptr->telepathy = TRUE;
        if (have_flag(flgs, OF_ESP_ANIMAL))  p_ptr->esp_animal = TRUE;
        if (have_flag(flgs, OF_ESP_UNDEAD))  p_ptr->esp_undead = TRUE;
        if (have_flag(flgs, OF_ESP_DEMON))   p_ptr->esp_demon = TRUE;
        if (have_flag(flgs, OF_ESP_ORC))     p_ptr->esp_orc = TRUE;
        if (have_flag(flgs, OF_ESP_TROLL))   p_ptr->esp_troll = TRUE;
        if (have_flag(flgs, OF_ESP_GIANT))   p_ptr->esp_giant = TRUE;
        if (have_flag(flgs, OF_ESP_DRAGON))  p_ptr->esp_dragon = TRUE;
        if (have_flag(flgs, OF_ESP_HUMAN))   p_ptr->esp_human = TRUE;
        if (have_flag(flgs, OF_ESP_EVIL))    p_ptr->esp_evil = TRUE;
        if (have_flag(flgs, OF_ESP_GOOD))    p_ptr->esp_good = TRUE;
        if (have_flag(flgs, OF_ESP_NONLIVING)) p_ptr->esp_nonliving = TRUE;
        if (have_flag(flgs, OF_ESP_UNIQUE))  p_ptr->esp_unique = TRUE;

        if (have_flag(flgs, OF_SEE_INVIS))   p_ptr->see_inv = TRUE;
        if (have_flag(flgs, OF_LEVITATION))  p_ptr->levitation = TRUE;
        if (have_flag(flgs, OF_FREE_ACT))    p_ptr->free_act = TRUE;
        if (have_flag(flgs, OF_HOLD_LIFE))   p_ptr->hold_life = TRUE;
        if (have_flag(flgs, OF_WARNING))
        {
            if (!obj->inscription || !(my_strchr(quark_str(obj->inscription),'$')))
              p_ptr->warning = TRUE;
        }

        if (have_flag(flgs, OF_TELEPORT))
        {
            if (object_is_cursed(obj)) p_ptr->cursed |= OFC_TELEPORT;
            else
            {
                cptr insc = quark_str(obj->inscription);
                if (obj->inscription && my_strchr(insc, '.')) {}
                else p_ptr->cursed |= OFC_TELEPORT_SELF;
            }
        }

        res_calc_bonuses(flgs);

        if (have_flag(flgs, OF_REFLECT))  p_ptr->reflect = TRUE;
        if (have_flag(flgs, OF_AURA_FIRE))  p_ptr->sh_fire = TRUE;
        if (have_flag(flgs, OF_AURA_ELEC))  p_ptr->sh_elec = TRUE;
        if (have_flag(flgs, OF_AURA_COLD))  p_ptr->sh_cold = TRUE;
        if (have_flag(flgs, OF_AURA_SHARDS))  p_ptr->sh_shards = TRUE;
        if (have_flag(flgs, OF_AURA_REVENGE))  p_ptr->sh_retaliation = TRUE;
        if (have_flag(flgs, OF_NO_MAGIC)) p_ptr->anti_magic = TRUE;
        if (have_flag(flgs, OF_NO_TELE))  p_ptr->anti_tele = TRUE;
        if (have_flag(flgs, OF_NO_SUMMON)) p_ptr->anti_summon = TRUE;

        if (have_flag(flgs, OF_SUST_STR)) p_ptr->sustain_str = TRUE;
        if (have_flag(flgs, OF_SUST_INT)) p_ptr->sustain_int = TRUE;
        if (have_flag(flgs, OF_SUST_WIS)) p_ptr->sustain_wis = TRUE;
        if (have_flag(flgs, OF_SUST_DEX)) p_ptr->sustain_dex = TRUE;
        if (have_flag(flgs, OF_SUST_CON)) p_ptr->sustain_con = TRUE;
        if (have_flag(flgs, OF_SUST_CHR)) p_ptr->sustain_chr = TRUE;

        if (have_flag(flgs, OF_DUAL_WIELDING))
        {
            switch (_template->slots[slot].type)
            {
            case EQUIP_SLOT_GLOVES:
            {
                int hand = _template->slots[slot].hand;
                int arm = hand / 2;
                int rhand = arm*2;
                int lhand = arm*2 + 1;

                p_ptr->weapon_info[rhand].genji = TRUE;
                p_ptr->weapon_info[lhand].genji = TRUE;
                break;
            }
            default: /* Weaponsmith with their beloved Boots of Genji :) */
                p_ptr->weapon_info[0].genji = TRUE;
                p_ptr->weapon_info[1].genji = TRUE;
            }
        }

        if (obj->name1 == ART_SPECTRAL_DSM)
        {
            p_ptr->pass_wall = TRUE;
            p_ptr->no_passwall_dam = TRUE;
        }

        if (obj->curse_flags & OFC_LOW_MAGIC)
        {
            if (obj->curse_flags & OFC_HEAVY_CURSE)
                p_ptr->to_m_chance += 10;
            else
                p_ptr->to_m_chance += 3;
        }

        if (obj->tval == TV_CAPTURE) continue;

        /* Modify the base armor class */
        p_ptr->ac += obj->ac;
        p_ptr->dis_ac += obj->ac;

        /* Apply the bonuses to armor class */
        p_ptr->to_a += obj->to_a;
        if (object_is_known(obj)) p_ptr->dis_to_a += obj->to_a;

        if (obj->curse_flags & OFC_LOW_MELEE)
        {
            int penalty = (obj->curse_flags & OFC_HEAVY_CURSE) ? -15 : -5;
            switch (_template->slots[slot].type)
            {
            case EQUIP_SLOT_BOW:
                p_ptr->shooter_info.to_h += penalty;
                if (obj->known_curse_flags & OFC_LOW_MELEE)
                    p_ptr->shooter_info.dis_to_h += penalty;
                break;
            case EQUIP_SLOT_WEAPON_SHIELD:
            case EQUIP_SLOT_WEAPON:
            {
                int hand = _template->slots[slot].hand;
                p_ptr->weapon_info[hand].to_h += penalty;
                if (obj->known_curse_flags & OFC_LOW_MELEE)
                    p_ptr->weapon_info[hand].dis_to_h += penalty;
                break;
            }
            }
        }
        if (obj->curse_flags & OFC_LOW_AC)
        {
            if (obj->curse_flags & OFC_HEAVY_CURSE)
            {
                p_ptr->to_a -= 30;
                if (obj->known_curse_flags & OFC_LOW_AC)
                    p_ptr->dis_to_a -= 30;
            }
            else
            {
                p_ptr->to_a -= 10;
                if (obj->known_curse_flags & OFC_LOW_AC)
                    p_ptr->dis_to_a -= 10;
            }
        }

        /* Hack -- do not apply "weapon" bonuses */
        if (object_is_melee_weapon(obj)) continue;

        /* Hack -- do not apply "bow" bonuses */
        if (_object_is_bow(obj)) continue;

        /* Hack -- Sniper gloves apply to missiles only */
        if (obj->name2 == EGO_GLOVES_SNIPER || obj->name2 == EGO_RING_ARCHERY)
        {
            p_ptr->shooter_info.to_h += obj->to_h;
            p_ptr->shooter_info.to_d += obj->to_d;
            if (object_is_known(obj))
            {
                p_ptr->shooter_info.dis_to_h += obj->to_h;
                p_ptr->shooter_info.dis_to_d += obj->to_d;
            }
            continue;
        }
        /* Hack -- Spell Damage on Wizard Egos */
        if ( obj->name2 == EGO_RING_WIZARDRY
          || obj->name2 == EGO_AMULET_MAGI
          || obj->name2 == EGO_CROWN_MAGI )
        {
            p_ptr->to_d_spell += obj->to_d;
            continue;
        }

        /* Hack -- Archery now benefits from equipment slays. Without this, only
           Sniper gloves and Archery rings affect shooting, and that seems a bit
           unfair (especially since melee is so favored).
           BTW, as best I can tell, Hengband always applied bonuses to hit to archery,
           but never applied bonuses to damage.
           Perhaps we need a TR_ARCHERY flag?  */
        else if ( obj->name2 != EGO_GLOVES_BERSERKER
               && obj->name2 != EGO_GLOVES_GIANT
               && obj->name2 != EGO_GLOVES_SLAYING
               && obj->name2 != EGO_GLOVES_THIEF
               && obj->name2 != EGO_RING_COMBAT
               && obj->name2 != EGO_HELMET_TROLL
               && obj->name2 != EGO_HELMET_RAGE
               && obj->name2 != EGO_SHIELD_DWARVEN
               && obj->name2 != EGO_SHIELD_ORCISH
               && obj->name2 != EGO_CROWN_MIGHT
               && obj->name2 != EGO_CLOAK_BAT
               && obj->name2 != EGO_CLOAK_COWARDICE
               && obj->name1 != ART_KAMIKAZE_ROBE
               && obj->name1 != ART_TERROR
               && obj->name1 != ART_HAMMERHAND )
        {
            p_ptr->shooter_info.to_h += obj->to_h;
            p_ptr->shooter_info.to_d += obj->to_d;
            if (object_is_known(obj))
            {
                p_ptr->shooter_info.dis_to_h += obj->to_h;
                p_ptr->shooter_info.dis_to_d += obj->to_d;
            }
        }

        bonus_to_h = obj->to_h;
        bonus_to_d = obj->to_d;

        if (p_ptr->pclass == CLASS_NINJA)
        {
            if (obj->to_h > 0) bonus_to_h = (obj->to_h+1)/2;
            if (obj->to_d > 0) bonus_to_d = (obj->to_d+1)/2;
        }

        p_ptr->to_h_m += bonus_to_h;
        p_ptr->to_d_m += bonus_to_d;

        _weapon_bonus(slot, bonus_to_h, bonus_to_d);
        if (have_flag(flgs, OF_WEAPONMASTERY))
        {
            _weaponmastery(slot, obj->pval);
        }
    }
}

void equip_on_init(void)
{
    race_t *race_ptr = get_race();
    if (race_ptr->equip_template)
        _template = race_ptr->equip_template;
    else
        _template = &b_info[0];
    assert(!_inv);
    _inv = inv_alloc(EQUIP_MAX_SLOTS, 0);
}

/* Attempt to gracefully handle changes to body type between
   releases. New slots may be added, old slots may be removed.
   Slots may be shuffled or have their types changed.
   This is called by process_player() during startup if a
   savefile has been loaded. At this point, drop_near is allowed. */
void equip_on_load(void)
{
    slot_t  slot, max = inv_last(_inv, obj_exists);
    inv_ptr temp = inv_alloc(EQUIP_MAX_SLOTS, 0);

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (!obj) continue;

        if (slot > _template->max)
        {
            inv_add(temp, obj);
            inv_remove(_inv, slot, obj->number);
        }
        else
        {
            obj_p p = _accept[_template->slots[slot].type];
            if (!p(obj))
            {
                inv_add(temp, obj);
                inv_remove(_inv, slot, obj->number);
            }
        }
    }

    max = inv_last(temp, obj_exists);
    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(temp, slot);
        slot_t  new_slot;

        if (!obj) continue;
        new_slot = equip_first_empty_slot(obj);

        if (new_slot)
            inv_add_at(_inv, obj, new_slot);
        else
        {
            char name[MAX_NLEN];
            object_desc(name, obj, OD_COLOR_CODED);
            msg_format("You can no longer wield %s.", name);
            /* TODO
            if (!pack_add(obj))
                pack_push_overflow(obj);*/
        }
    }
    inv_free(temp);
}

void equip_on_change_race(void)
{
    equip_template_ptr old_template = _template;
    equip_template_ptr new_template = get_race()->equip_template;

    if (!new_template)
        new_template = &b_info[0];

    if (old_template != new_template)
    {
        slot_t  slot;
        inv_ptr temp = inv_copy(_inv);

        inv_clear(_inv);
        _template = new_template;

        for (slot = 1; slot <= old_template->max; slot++)
        {
            obj_ptr src = inv_obj(temp, slot);
            slot_t  new_slot;

            if (!src) continue;
            new_slot = equip_first_empty_slot(src);
            if (new_slot)
                inv_add_at(_inv, src, new_slot);
            else
            {
                char name[MAX_NLEN];

                object_desc(name, src, 0);
                msg_format("You can no longer wield %s.", name);

                /* Mark the object as previously worn. Next time we shift bodies,
                   we will attempt to wield this item again automatically */
                if (!src->inscription)
                    src->marked |= OM_WORN;
                else
                {
                    cptr inscription = quark_str(src->inscription);
                    if ( !strstr(inscription, "@mimic")
                      && !strstr(inscription, "@vampire")
                      && !strstr(inscription, "@bat")
                      && !strstr(inscription, "@mist")
                      && !strstr(inscription, "@wolf") )
                    {
                        src->marked |= OM_WORN;
                    }
                }

                /* TODO
                if (!pack_add(obj))
                    pack_push_overflow(obj);*/
            }
        }
        inv_free(temp);
        temp = NULL;

        /* TODO
        pack_overflow();
        for (slot = 1; slot <= pack_max(); slot++)
        {
            obj_ptr obj = pack_obj(slot);
            slot_t  new_slot;

            if (!obj) continue;
            if (!(obj->marked & OM_WORN)) continue;

            new_slot = equip_first_empty_slot(obj);
            if (new_slot && obj->number == 1)
            {
                obj->marked &= ~OM_WORN;
                equip_wield_aux(obj, new_slot);
                pack_remove(slot, 1);
            }
        }
        pack_sort();
        */

        /*p_ptr->notice |= PN_REORDER;*/
        p_ptr->update |= PU_BONUS;
        p_ptr->update |= PU_TORCH;
        p_ptr->update |= PU_MANA;
        p_ptr->redraw |= PR_EQUIPPY;
        p_ptr->window |= PW_INVEN | PW_EQUIP;
        android_calc_exp();

    }
}

void equip_learn_curse(int flag)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (obj && obj_learn_curse(obj, flag))
        {
            char buf[MAX_NLEN];
            object_desc(buf, obj, OD_LORE);
            msg_format("<color:B>You feel that your %s is <color:r>cursed</color>.</color>", buf);
        }
    }
}

void _learn_resist_aux(int obj_flag, cptr desc)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (obj && obj_learn_flag(obj, obj_flag))
        {
            char buf[MAX_NLEN];
            object_desc(buf, obj, OD_LORE);
            msg_format("<color:B>You feel that your %s is %s you.</color>", buf, desc);
        }
    }
}

void equip_learn_resist(int obj_flag)
{
    _learn_resist_aux(obj_flag, "protecting");
}

void equip_learn_vuln(int obj_flag)
{
    _learn_resist_aux(obj_flag, "exposing");
}

void equip_learn_flag(int obj_flag)
{
    slot_t slot;
    for (slot = 1; slot <= _template->max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (obj && obj_learn_flag(obj, obj_flag))
        {
            char buf[MAX_NLEN];
            object_desc(buf, obj, OD_LORE);
            msg_format("<color:B>You learn more about your %s.</color>", buf);
        }
    }
}

