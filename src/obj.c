#include "angband.h"

#include <assert.h>

obj_ptr obj_alloc(void)
{
    obj_ptr obj = malloc(sizeof(object_type));
    object_wipe(obj);
    return obj;
}

obj_ptr obj_copy(obj_ptr obj)
{
    obj_ptr copy = malloc(sizeof(object_type));
    assert(obj);
    *copy = *obj;
    return copy;
}

obj_ptr obj_split(obj_ptr obj, int amt)
{
    obj_ptr copy;
    assert(obj);
    assert(0 < amt && amt < obj->number);
    copy = obj_copy(obj);
    copy->loc.where = INV_TMP_ALLOC;
    copy->loc.slot = 0;

    copy->number = amt;
    obj->number -= amt;

    if (obj->insured)
    {
        if (((obj->loc.where == INV_EQUIP) || (obj->loc.where == INV_PACK)) && (obj->insured % 100 > obj->number))
        {
            int vahennys = (obj->insured % 100) - obj->number;
            copy->insured = (obj->insured / 100) * 100 + vahennys;
            obj_dec_insured(obj, vahennys);
        }
        else if ((obj->loc.where != INV_EQUIP) && (obj->loc.where != INV_PACK))
        {
            int vahennys = MIN((obj->insured % 100), amt);
            copy->insured = (obj->insured / 100) * 100 + vahennys;
            obj_dec_insured(obj, vahennys);
        }
        else if (obj->insured % 100 > obj->number)
        {
            int vahennys = (obj->insured % 100) - obj->number;
            copy->insured = (obj->insured / 100) * 100 + vahennys;
            obj_dec_insured(obj, vahennys);
        }
        else copy->insured = 0;
    }

    return copy;
}

void obj_clear_dun_info(obj_ptr obj)
{
    obj->next_o_idx = 0;
    obj->held_m_idx = 0;
    obj->loc.x = 0;
    obj->loc.y = 0;
    obj->marked &= (OM_WORN | OM_COUNTED | OM_EFFECT_COUNTED | OM_EGO_COUNTED | OM_ART_COUNTED);
}

void obj_free(obj_ptr obj)
{
    if (obj)
    {
        object_wipe(obj);
        free(obj);
    }
}

void obj_make_pile(obj_ptr obj)
{
    int          size = 1;
    object_kind *k_ptr = &k_info[obj->k_idx];

    if (object_is_artifact(obj)) return;
    if (object_is_ego(obj) && !object_is_ammo(obj)) return;
    if (!k_ptr->stack_chance) return;
    if (randint1(100) > k_ptr->stack_chance) return;
    if ((quest_id_current() == 60) && (k_ptr->gen_flags & OFG_NO_SHUFFLE)) return; /* mega-hack - Clear the Tunnels */

    assert(k_ptr->stack_dice);
    assert(k_ptr->stack_sides);
    size = damroll(k_ptr->stack_dice, k_ptr->stack_sides);

    if (size <= 1) return;

    obj->number = size;
    if (!store_hack)
    {
        k_ptr->counts.generated += size - 1;
        if (obj->name2)
            e_info[obj->name2].counts.generated += size - 1;
    }
    else if (obj->discount)
    {
        obj->number -= (size * obj->discount / 100);
    }
}

static void _destroy(obj_ptr obj);

void obj_release(obj_ptr obj, int options)
{
    char name[MAX_NLEN];
    bool quiet = BOOL(options & OBJ_RELEASE_QUIET);
    bool delayed = BOOL(options & OBJ_RELEASE_DELAYED_MSG);

    if (!obj) return;
    if (!quiet)
        object_desc(name, obj, OD_COLOR_CODED);

    if ((obj->marked & OM_AUTODESTROY) && obj->number)
    {
        _destroy(obj);
        obj->number = 0;
    }

    if (options & OBJ_RELEASE_ENCHANT)
        gear_notice_enchant(obj);
    if (options & OBJ_RELEASE_ID)
        gear_notice_id(obj);

    switch (obj->loc.where)
    {
    case INV_FLOOR:
        if (!quiet)
            msg_format("You see %s.", name);
        if (obj->number <= 0)
            delete_object_idx(obj->loc.slot);
        break;
    case INV_EQUIP:
        if (obj->number <= 0)
        {
            if (!quiet)
                msg_format("You are no longer wearing %s.", name);
            equip_remove(obj->loc.slot);
        }
        else if (!quiet)
            msg_format("You are wearing %s.", name);
        p_ptr->window |= PW_EQUIP;
        break;
    case INV_PACK:
        if (!quiet && !delayed)
            msg_format("You have %s in your pack.", name);
        if (obj->number <= 0)
            pack_remove(obj->loc.slot);
        else if (delayed)
        {
            obj->marked |= OM_DELAYED_MSG;
            p_ptr->notice |= PN_CARRY;
        }
        p_ptr->window |= PW_INVEN;
        break;
    case INV_QUIVER:
        if (!quiet && !delayed)
            msg_format("You have %s in your quiver.", name);
        if (obj->number <= 0)
            quiver_remove(obj->loc.slot);
        else if (delayed)
        {
            obj->marked |= OM_DELAYED_MSG;
            p_ptr->notice |= PN_CARRY;
        }
        p_ptr->window |= PW_EQUIP; /* a Quiver [32 of 110] */
        break;
    case INV_TMP_ALLOC:
        obj_free(obj);
        break;
    case INV_SPECIAL1:
        if ((obj->number <= 0) && (!(obj->marked & OM_BEING_SHUFFLED)))
            special1_remove(obj->loc.slot);
        break;
    case INV_SPECIAL2:
        if ((obj->number <= 0) && (!(obj->marked & OM_BEING_SHUFFLED)))
            special2_remove(obj->loc.slot);
        break;
    default:
        break;
    }
}

/* obj_zero() is for objects being destroyed (not moved, dropped, etc.) */
void obj_zero(obj_ptr obj)
{
    if (obj->insured) cornucopia_mark_destroyed(cornucopia_item_policy(obj), obj->number);
    obj->number = 0;
}

void obj_dec_number(obj_ptr obj, int amount, bool mark)
{
    obj->number -= amount;
    if (!obj->insured) return;
    else {
        int uninsured = MAX(0, (obj->number + amount) - (obj->insured % 100));
        if (mark) cornucopia_mark_destroyed(cornucopia_item_policy(obj), amount - uninsured);
        if ((obj->number) && ((obj->insured % 100) > obj->number)) obj->insured = (obj->insured / 100) * 100 + obj->number;
    }
}

void obj_dec_insured(obj_ptr obj, int amount)
{
    int vakuutettu = (obj->insured % 100);
    if (amount >= vakuutettu) obj->insured = 0;
    else obj->insured -= amount;
}


void gear_notice_id(obj_ptr obj)
{
    switch (obj->loc.where)
    {
    case INV_EQUIP:
        p_ptr->update |= PU_BONUS; /* dis_to_h, dis_to_d, dis_to_ac, etc. */
        p_ptr->window |= PW_EQUIP;
        break;
    case INV_PACK:
        p_ptr->notice |= PN_OPTIMIZE_PACK;
        p_ptr->window |= PW_INVEN;
        break;
    case INV_QUIVER:
        p_ptr->notice |= PN_OPTIMIZE_QUIVER;
        p_ptr->window |= PW_EQUIP; /* a Quiver [32 of 110] */
        break;
    }
}

void gear_notice_enchant(obj_ptr obj)
{
    switch (obj->loc.where)
    {
    case INV_EQUIP:
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        android_calc_exp();
        break;
    case INV_PACK:
        p_ptr->notice |= PN_OPTIMIZE_PACK;
        p_ptr->window |= PW_INVEN;
        break;
    case INV_QUIVER:
        p_ptr->notice |= PN_OPTIMIZE_QUIVER;
        p_ptr->window |= PW_EQUIP; /* a Quiver [32 of 110] */
        break;
    }
}

/************************************************************************
 * Predicates
 ***********************************************************************/
bool obj_can_sense1(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_BOW:
    case TV_QUIVER:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_CARD:
        return TRUE;
    }
    return FALSE;
}

bool obj_can_sense2(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_RING:
    case TV_AMULET:
    case TV_LITE:
    case TV_FIGURINE:
    case TV_WAND:
    case TV_STAFF:
    case TV_ROD:
        return TRUE;
    }
    return FALSE;
}

bool obj_can_shoot(obj_ptr obj)
{
    if (!obj_is_ammo(obj)) return FALSE;
    if (!equip_find_obj(TV_BOW, SV_ANY)) return FALSE;
    return obj->tval == p_ptr->shooter_info.tval_ammo;
}

bool obj_is_blessed(obj_ptr obj)
{
    u32b flgs[OF_ARRAY_SIZE];
    obj_flags(obj, flgs);
    return have_flag(flgs, OF_BLESSED);
}

bool obj_is_known(obj_ptr obj)
{
    obj_kind_ptr k;
    if (obj->ident & (IDENT_KNOWN | IDENT_STORE)) return TRUE;
    k = &k_info[obj->k_idx];
    if (k->easy_know && k->aware) return TRUE;
    return FALSE;
}

bool obj_is_readable_book(obj_ptr obj)
{
    if (!obj_is_book(obj)) return FALSE;
    if (p_ptr->pclass == CLASS_SORCERER)
    {
        return is_magic(tval2realm(obj->tval));
    }
    else if (p_ptr->pclass == CLASS_RED_MAGE)
    {
        if (is_magic(tval2realm(obj->tval)))
            return ((obj->tval == TV_ARCANE_BOOK) || (obj->sval < 2));
    }
    else if (p_ptr->pclass == CLASS_GRAY_MAGE)
    {
        return gray_mage_is_allowed_book(obj->tval, obj->sval);
    }
    else if (p_ptr->pclass == CLASS_SKILLMASTER)
    {
        return skillmaster_is_allowed_book(obj->tval, obj->sval);
    }
    return (REALM1_BOOK == obj->tval || REALM2_BOOK == obj->tval);
}

bool obj_exists(obj_ptr obj)     { return BOOL(obj); }
bool obj_is_ammo(obj_ptr obj)    { return TV_MISSILE_BEGIN <= obj->tval && obj->tval <= TV_MISSILE_END; }
bool obj_is_armor(obj_ptr obj)   { return TV_ARMOR_BEGIN <= obj->tval && obj->tval <= TV_ARMOR_END; }
bool obj_is_art(obj_ptr obj)     { return obj->name1 || obj->art_name; }
bool obj_is_book(obj_ptr obj)    { return TV_BOOK_BEGIN <= obj->tval && obj->tval <= TV_BOOK_END; }
bool obj_is_device(obj_ptr obj)  { return obj_is_wand(obj) || obj_is_rod(obj) || obj_is_staff(obj); }
bool obj_is_ego(obj_ptr obj)     { return BOOL(obj->name2); }
bool obj_is_found(obj_ptr obj)   { return BOOL(obj->marked & OM_FOUND); }
bool obj_is_inscribed(obj_ptr obj) { return BOOL(obj->inscription); }
bool obj_is_quiver(obj_ptr obj)  { return obj->tval == TV_QUIVER; }
bool obj_is_rod(obj_ptr obj)     { return obj->tval == TV_ROD; }
bool obj_is_staff(obj_ptr obj)   { return obj->tval == TV_STAFF; }
bool obj_is_unknown(obj_ptr obj) { return !obj_is_known(obj); }
bool obj_is_wand(obj_ptr obj)    { return obj->tval == TV_WAND; }

bool obj_is_shooter(obj_ptr obj) { return obj->tval == TV_BOW; }
bool obj_is_bow(obj_ptr obj)
{
    if (!obj_is_shooter(obj)) return FALSE;
    if (obj->sval != SV_SHORT_BOW && obj->sval != SV_LONG_BOW) return FALSE;
    return TRUE;
}
bool obj_is_sling(obj_ptr obj)
{
    if (!obj_is_shooter(obj)) return FALSE;
    if (obj->sval != SV_SLING) return FALSE;
    return TRUE;
}
bool obj_is_crossbow(obj_ptr obj)
{
    if (!obj_is_shooter(obj)) return FALSE;
    if (obj->sval != SV_LIGHT_XBOW && obj->sval != SV_HEAVY_XBOW) return FALSE;
    return TRUE;
}
bool obj_is_harp(obj_ptr obj)
{
    if (!obj_is_shooter(obj)) return FALSE;
    if (obj->sval != SV_HARP) return FALSE;
    return TRUE;
}
bool obj_is_gun(obj_ptr obj)
{
    if (!obj_is_shooter(obj)) return FALSE;
    if (obj->sval != SV_CRIMSON && obj->sval != SV_RAILGUN) return FALSE;
    return TRUE;
}
/************************************************************************
 * Sorting
 ***********************************************************************/
void obj_clear_scratch(obj_ptr obj)
{
    if (obj) obj->scratch = 0;
}

static int _obj_cmp_type(obj_ptr obj)
{
    if (!object_is_device(obj))
    {
        if (object_is_fixed_artifact(obj)) return 3;
        else if (obj->art_name) return 2;
        else if (object_is_ego(obj)) return 1;
    }
    return 0;
}

int obj_cmp(obj_ptr left, obj_ptr right)
{
    int left_type, right_type;
    /* Modified from object_sort_comp but the comparison is tri-valued
     * as is standard practice for compare functions. We also memoize
     * computation of obj_value for efficiency. */

    /* Empty slots sort to the end */
    if (!left && !right) return 0;
    if (!left && right) return 1;
    if (left && !right) return -1;
    if (left == right) return 0;

    assert(left && right);

    /* Hack -- readable books always come first (This fails for the Skillmaster) */
    if (left->tval == REALM1_BOOK && right->tval != REALM1_BOOK) return -1;
    if (left->tval != REALM1_BOOK && right->tval == REALM1_BOOK) return 1;

    if (left->tval == REALM2_BOOK && right->tval != REALM2_BOOK) return -1;
    if (left->tval != REALM2_BOOK && right->tval == REALM2_BOOK) return 1;

    /* Objects sort by decreasing type */
    if (left->tval < right->tval) return 1;
    if (left->tval > right->tval) return -1;

    /* Non-aware (flavored) items always come last (buggy test in shops) */
    if (left->loc.where != INV_SHOP && right->loc.where != INV_SHOP)
    {
        if (!object_is_aware(left) && object_is_aware(right)) return 1;
        if (object_is_aware(left) && !object_is_aware(right)) return -1;
    }

    /* Objects sort by increasing sval */
    if (left->sval < right->sval) return -1;
    if (left->sval > right->sval) return 1;

    /* Unidentified objects always come last */
    if (!object_is_known(left) && object_is_known(right)) return 1;
    if (object_is_known(left) && !object_is_known(right)) return -1;

    /* Fixed artifacts, random artifacts and ego items */
    left_type = _obj_cmp_type(left);
    right_type = _obj_cmp_type(right);
    if (left_type < right_type) return -1;
    if (left_type > right_type) return 1;

    switch (left->tval)
    {
    case TV_FIGURINE:
    case TV_STATUE:
    case TV_CORPSE:
    case TV_CAPTURE:
        if ((left->tval == TV_CORPSE) && (left->sval >= SV_BODY_HEAD)) break;
        if (r_info[left->pval].level < r_info[right->pval].level) return -1;
        if (r_info[left->pval].level > r_info[right->pval].level) return 1;
        if (left->pval < right->pval) return -1;
        if (left->pval > right->pval) return 1;
        break;

    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        if (left->to_h + left->to_d < right->to_h + right->to_d) return -1;
        if (left->to_h + left->to_d > right->to_h + right->to_d) return 1;
        break;

    case TV_ROD:
    case TV_WAND:
    case TV_STAFF:
        if (left->activation.type < right->activation.type) return -1;
        if (left->activation.type > right->activation.type) return 1;
        if (device_level(left) < device_level(right)) return -1;
        if (device_level(left) > device_level(right)) return 1;
        break;

    case TV_LITE:
        if (left->xtra4 < right->xtra4) return 1;
        if (left->xtra4 > right->xtra4) return -1;
        break;
    }

    if (!left->scratch) left->scratch = obj_value(left);
    if (!right->scratch) right->scratch = obj_value(right);

    if (left->scratch < right->scratch) return 1;
    if (left->scratch > right->scratch) return -1;

    if (left->number < right->number) return 1;
    if (left->number > right->number) return -1;
    return 0;
}

/************************************************************************
 * Menus
 ***********************************************************************/
char obj_label(obj_ptr obj)
{
    cptr insc;
    if (!obj->inscription) return '\0';
    insc = quark_str(obj->inscription);

    for (insc = strchr(insc, '@'); insc; insc = strchr(insc, '@'))
    {
        insc++;
        /* @mc uses 'c' as a label only for the 'm' command */
        if (command_cmd && *insc == command_cmd)
        {
            insc++;
            if ( ('a' <= *insc && *insc <= 'z')
              || ('A' <= *insc && *insc <= 'Z')
              || ('0' <= *insc && *insc <= '9') )
            {
                return *insc;
            }
        }
        /* @3 uses '3' as a label for *any* command */
        else if ('0' <= *insc && *insc <= '9')
            return *insc;
    }
    return '\0';
}

bool obj_confirm_choice(obj_ptr obj)
{
    char name[MAX_NLEN], prompt[MAX_NLEN + 20];
    cptr insc, pos;
    int  ct = 0;

    if (!obj->inscription) return TRUE;
    if (!command_cmd) return TRUE;

    insc = quark_str(obj->inscription);
    /* !sdk = !s!d!k */
    for (pos = strchr(insc, '!');
            pos && *pos;
            pos = strchr(pos + 1, '!'))
    {
        for (;;)
        {
            pos++;
            if (!*pos) return TRUE;
            else if (*pos == command_cmd || *pos == '*')
            {
                if (!ct++)
                {
                    object_desc(name, obj, OD_COLOR_CODED);
                    sprintf(prompt, "Really choose %s? ", name);
                }
                if (!get_check(prompt)) return FALSE;
            }
            else if (!isalpha(*pos))
            {
                if (*pos == '!') { pos--; break; } /* !k!q */
                else if (*pos == '?')
                {
                    while ((*(pos++)) && (isdigit(*pos)))
                    {
                    }
                    if (!*pos) return TRUE;
                    else if (*pos == '!') { pos--; break; }
                    else if (isalpha(*pos)) pos--;
                }
                else break;
            }
        }
    }
    return TRUE;
}

/************************************************************************
 * Stacking
 ***********************************************************************/
bool obj_can_combine(obj_ptr dest, obj_ptr obj, int loc)
{
    int  i;

    if (dest == obj) return FALSE;
    if (dest->k_idx != obj->k_idx) return FALSE; /* i.e. same tval/sval */
    if (obj_is_art(dest) || obj_is_art(obj)) return FALSE;

    switch (dest->tval)
    {
    /* Objects that never combine */
    case TV_CHEST:
    case TV_CARD:
    case TV_CAPTURE:
    case TV_RUNE:
    case TV_STAFF:
    case TV_WAND:
    case TV_ROD:
        return FALSE;

    case TV_STATUE:
        if (dest->sval != SV_PHOTO) break;
        /* Fall Thru for monster check (Q: Why don't statues with same monster combine?) */
    case TV_FIGURINE:
    case TV_CORPSE:
        if (dest->pval != obj->pval) return FALSE;
        if ((dest->tval == TV_CORPSE) && (dest->sval >= SV_BODY_HEAD)) return FALSE;
        break;

    case TV_FOOD:
    case TV_POTION:
    case TV_SCROLL:
        break;

    /* Equipment */
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_QUIVER:
    case TV_RING:
    case TV_AMULET:
    case TV_LITE:
    case TV_WHISTLE:
        /* Require full knowledge of both items. Ammo skips this check
         * so that you can shoot unidentifed stacks of arrows and have
         * them recombine later. */
        if (loc != INV_SHOP)
        {
            if (!object_is_known(dest) || !object_is_known(obj)) return FALSE;
        }
        /* Fall through */
    case TV_BOLT:
    case TV_ARROW:
    case TV_SHOT:
        if (loc != INV_SHOP)
        {
            if (object_is_known(dest) != object_is_known(obj)) return FALSE;
            if (dest->feeling != obj->feeling) return FALSE;
        }

        /* Require identical bonuses */
        if (dest->to_h != obj->to_h) return FALSE;
        if (dest->to_d != obj->to_d) return FALSE;
        if (dest->to_a != obj->to_a) return FALSE;
        if (dest->pval != obj->pval) return FALSE;

        /* Require identical ego types (Artifacts were precluded above) */
        if (dest->name2 != obj->name2) return FALSE;

        /* Require identical added essence  */
        if (dest->xtra3 != obj->xtra3) return FALSE;
        if (dest->xtra4 != obj->xtra4) return FALSE;

        /* Hack -- Never stack "powerful" items (Q: What does this mean?) */
        if (dest->xtra1 || obj->xtra1) return FALSE;

        /* Hack -- Never stack recharging items */
        if (dest->timeout || obj->timeout) return FALSE;

        /* Require identical "values" */
        if (dest->ac != obj->ac) return FALSE;
        if (dest->dd != obj->dd) return FALSE;
        if (dest->ds != obj->ds) return FALSE;

        break;

    default:
        /* Require knowledge */
        if (!object_is_known(dest) || !object_is_known(obj)) return FALSE;
    }

    /* Hack -- Identical art_flags! */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        if (dest->flags[i] != obj->flags[i]) return FALSE;

    /* Hack -- Require identical "cursed" status */
    if (dest->curse_flags != obj->curse_flags) return FALSE;

    /* Require identical activations */
    if ( dest->activation.type != obj->activation.type
      || dest->activation.cost != obj->activation.cost
      || dest->activation.power != obj->activation.power
      || dest->activation.difficulty != obj->activation.difficulty
      || dest->activation.extra != obj->activation.extra )
    {
        return FALSE;
    }

    /* Hack -- Require identical "broken" status */
    if ((dest->ident & IDENT_BROKEN) != (obj->ident & IDENT_BROKEN)) return FALSE;

    /* Shops always merge inscriptions, but never discounts. For the
     * player, merging of inscriptions and discounts is controlled
     * by options (stack_force_*) */
    if (loc == INV_SHOP)
    {
        if (dest->discount != obj->discount) return FALSE;
    }
    else
    {
        /* If both objects are inscribed, then inscriptions must match. */
        if (dest->inscription && obj->inscription && dest->inscription != obj->inscription)
            return FALSE;

        if (!stack_force_notes && dest->inscription != obj->inscription) return FALSE;
        if (!stack_force_costs && dest->discount != obj->discount) return FALSE;
    }

    /* Objects insured under separate policies do not combine */
    if ((obj->insured) && (dest->insured) && ((cornucopia_item_policy(obj)) != (cornucopia_item_policy(dest)))) return FALSE;

    return TRUE;
}

bool _obj_origins_match(obj_ptr dest, obj_ptr obj, bool check_mitze)
{
    if (dest->origin_type == ORIGIN_MIXED) return TRUE; /* no need to change anything */
    if (dest->origin_type != obj->origin_type) return FALSE;
    if (dest->origin_place != obj->origin_place) return FALSE;
    if (dest->origin_xtra != obj->origin_xtra) return FALSE;
    if (!check_mitze) return TRUE;
    if (dest->mitze_type != obj->mitze_type) return FALSE;
    if (dest->mitze_turn != obj->mitze_turn) return FALSE;
    return TRUE;
}

void _obj_combine_origins(obj_ptr dest, obj_ptr obj)
{
    if (!show_discovery)
    {
        if (!_obj_origins_match(dest, obj, FALSE))
        {
            dest->origin_type = ORIGIN_MIXED;
            return;
        }
    }
    if ((dest->mitze_turn > obj->mitze_turn) && (obj->mitze_turn > 0))
    { /* Dest should inherit obj's origin */
        dest->origin_type = obj->origin_type;
        dest->origin_place = obj->origin_place;
        dest->origin_xtra = obj->origin_xtra;
        dest->mitze_type = obj->mitze_type;
        dest->mitze_turn = obj->mitze_turn;
        dest->mitze_level = obj->mitze_level;
    }
    dest->mitze_type |= MITZE_MIXED;
}

/* combine obj into dest up to the max stack size.
 * decrease obj->number by the amount combined and 
 * return the amount combined. */
int obj_combine(obj_ptr dest, obj_ptr obj, int loc)
{
    int amt;

    assert(dest && obj);
    assert(dest->number <= OBJ_STACK_MAX);

    if (!obj_can_combine(dest, obj, loc)) return 0;

    if (dest->number + obj->number > OBJ_STACK_MAX)
        amt = OBJ_STACK_MAX - dest->number;
    else
        amt = obj->number;

    dest->number += amt;
    obj->number -= amt;

    /* Combine insurance */
    if ((obj->insured) && (amt > 0))
    {
        if (dest->insured == 0)
        {
            dest->insured = obj->insured;
            obj->insured = 0;
        }
        else
        {
            int _policy = (dest->insured / 100); /* Both items insured, must be under same policy or combine would have failed */
            int dest_vk = (dest->insured % 100), obj_vk = (obj->insured % 100);
            if (dest_vk + obj_vk < 100)
            {
                dest_vk += obj_vk;
                obj_vk = 0;
            }
            else
            {
                obj_vk = dest_vk + obj_vk - 99;
                dest_vk = 99;
            }
            dest->insured = _policy * 100 + dest_vk;
            obj->insured = (obj_vk) ? (_policy * 100 + obj_vk) : 0;
        }
    }

    if (loc != INV_SHOP)
    {
        if (object_is_known(obj)) obj_identify(dest);

        /* Hack -- clear "storebought" if only one has it */
        if ( ((dest->ident & IDENT_STORE) || (obj->ident & IDENT_STORE))
          && !((dest->ident & IDENT_STORE) && (obj->ident & IDENT_STORE)) )
        {
            if (obj->ident & IDENT_STORE) obj->ident &= ~IDENT_STORE;
            if (dest->ident & IDENT_STORE) dest->ident &= ~IDENT_STORE;
        }

        /* Mixed origins */
        if (!_obj_origins_match(dest, obj, TRUE)) _obj_combine_origins(dest, obj);

        /* Hack -- blend "inscriptions" */
        if (obj->inscription && !dest->inscription) dest->inscription = obj->inscription;

        /* Hack -- blend "feelings" */
        if (obj->feeling) dest->feeling = obj->feeling;
        if (obj->marked & OM_DELAYED_MSG) dest->marked |= OM_DELAYED_MSG;

        /* Hack -- could average discounts XXX XXX XXX */
        /* Hack -- save largest discount XXX XXX XXX */
        if (dest->discount < obj->discount) dest->discount = obj->discount;
    }
    return amt;
}

void obj_delayed_describe(obj_ptr obj)
{
    if (obj->marked & OM_DELAYED_MSG)
    {
        string_ptr msg = string_alloc();
        char       name[MAX_NLEN];
        bool       show_slot = FALSE;

        object_desc(name, obj, OD_COLOR_CODED);
        if (obj->loc.where == INV_EQUIP) /* paranoia */
            string_append_s(msg, "You are wearing");
        else
            string_append_s(msg, "You have");
        string_printf(msg, " %s", name);
        if (obj->loc.where == INV_QUIVER)
            string_append_s(msg, " in your quiver");

        switch (obj->loc.where)
        {
        case INV_QUIVER:
        case INV_PACK:
            show_slot = use_pack_slots;
            break;
        case INV_EQUIP:
            show_slot = TRUE;
            break;
        }
        if (show_slot)
            string_printf(msg, " (%c)", slot_label(obj->loc.slot));
        string_append_c(msg, '.');
        msg_print(string_buffer(msg));
        string_free(msg);

        obj->marked &= ~OM_DELAYED_MSG;
    }
}

/************************************************************************
 * Commands:
 * For Inspect and Inscribe, it seems useful to keep the obj_prompt up
 * to allow multiple operations. There is no energy cost for these commands.
 ***********************************************************************/
static int _inspector(obj_prompt_context_ptr context, int cmd)
{
    obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
    slot_t             slot = inv_label_slot(tab->inv, cmd);
    if (slot)
    {
        obj_ptr obj = inv_obj(tab->inv, slot);
        if (!obj) return OP_CMD_SKIPPED; /* gear_ui(INV_EQUIP) */
        doc_clear(context->doc);
        if (object_is_flavor(obj) && !object_is_known(obj))
        {
            char name[MAX_NLEN];
            object_desc(name, obj, OD_COLOR_CODED);
            doc_insert(context->doc, name);
            doc_insert(context->doc, "\n\nYou have no special knowledge about this item.\n");
        }
        else
            obj_display_doc(obj, context->doc);
        doc_insert(context->doc, "<color:B>[Press <color:y>Any Key</color> to Continue]</color>\n\n");
        Term_load();
        doc_sync_menu(context->doc);
        cmd = inkey();
        return OP_CMD_HANDLED;
    }
    return OP_CMD_SKIPPED;
}

void obj_inspect_ui(void)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Examine which item <color:w>(<color:keypress>Esc</color> to exit)</color>?";
    prompt.error = "You have nothing to examine.";
    prompt.filter = obj_exists;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.cmd_handler = _inspector;
    allow_special3_hack = TRUE;
    obj_prompt_add_special_packs(&prompt);

    obj_prompt(&prompt);
    allow_special3_hack = FALSE;

    /* The '-' key autoselects a single floor object */
    if (prompt.obj)
        obj_display(prompt.obj);
}

void gear_ui(int which)
{
    obj_prompt_t prompt = {0};
    int          wgt = py_total_weight();
    int          pct = wgt * 100 / weight_limit();
    string_ptr   s;

    s = string_alloc_format(
        "<color:w>Carrying %d.%d pounds (<color:%c>%d%%</color> capacity).</color>\n\n"
        "Examine which item <color:w>(<color:keypress>Esc</color> to exit)</color>?",
         wgt / 10, wgt % 10, pct > 100 ? 'r' : 'G', pct);
    prompt.prompt = string_buffer(s);
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.top_loc = which;

    prompt.cmd_handler = _inspector;

    obj_prompt(&prompt);
    string_free(s);
}

static int _inscriber(obj_prompt_context_ptr context, int cmd)
{
    obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
    slot_t             slot = inv_label_slot(tab->inv, cmd);
    if (slot)
    {
        obj_ptr obj = inv_obj(tab->inv, slot);
        char    name[MAX_NLEN];
        char    insc[80];

        object_desc(name, obj, OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
        if (obj->inscription)
            strcpy(insc, quark_str(obj->inscription));
        else
            strcpy(insc, "");

        doc_clear(context->doc);
        doc_printf(context->doc, "Inscribing %s.\n", name);
        doc_printf(context->doc, "Inscription: ");
        Term_load();
        doc_sync_menu(context->doc);
        if (askfor(insc, 80))
            obj->inscription = quark_add(insc);
        return OP_CMD_HANDLED;
    }
    return OP_CMD_SKIPPED;
}

void obj_inscribe_ui(void)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Inscribe which item <color:w>(<color:keypress>Esc</color> to exit)</color>?";
    prompt.error = "You have nothing to inscribe.";
    prompt.filter = obj_exists;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.cmd_handler = _inscriber;

    obj_prompt(&prompt);

    p_ptr->notice |= PN_OPTIMIZE_PACK | PN_OPTIMIZE_QUIVER;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
}

static int _uninscriber(obj_prompt_context_ptr context, int cmd)
{
    obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
    slot_t             slot = inv_label_slot(tab->inv, cmd);
    if (slot)
    {
        obj_ptr obj = inv_obj(tab->inv, slot);
        obj->inscription = 0;
        return OP_CMD_HANDLED;
    }
    return OP_CMD_SKIPPED;
}

void obj_uninscribe_ui(void)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Remove inscription from which item <color:w>(<color:keypress>Esc</color> to exit)</color>?";
    prompt.error = "You have nothing to uninscribe.";
    prompt.filter = obj_is_inscribed;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.cmd_handler = _uninscriber;

    obj_prompt(&prompt);

    p_ptr->notice |= PN_OPTIMIZE_PACK | PN_OPTIMIZE_QUIVER;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
}

static void _drop(obj_ptr obj)
{
    char name[MAX_NLEN];
    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You drop %s.", name);
    drop_near(obj, 0, py, px);
    p_ptr->update |= PU_BONUS; /* Weight changed */
    if (obj->loc.where == INV_PACK)
        p_ptr->window |= PW_INVEN;
}

void obj_drop(obj_ptr obj, int amt)
{
    assert(obj);
    assert(amt <= obj->number);

    if (!amt) return;

    if (amt < obj->number)
    {
        obj_t copy = *obj;
        copy.number = amt;
        if (obj->insured)
        {
            copy.insured = 0;
            if ((obj->insured % 100) > (obj->number - amt)) copy.insured = (obj->insured / 100) * 100 + (obj->insured % 100) - (obj->number - amt);
        }
        obj_dec_number(obj, amt, FALSE);

        obj->marked |= OM_DELAYED_MSG;
        p_ptr->notice |= PN_CARRY;
        if (obj->loc.where == INV_PACK)
            p_ptr->notice |= PN_OPTIMIZE_PACK;
        else if (obj->loc.where == INV_QUIVER)
            p_ptr->notice |= PN_OPTIMIZE_QUIVER;

        copy.marked &= ~OM_WORN;
        _drop(&copy);
    }
    else
    {
        obj->marked &= ~OM_WORN;
        _drop(obj);
        obj->number = 0;
        obj_release(obj, OBJ_RELEASE_QUIET);
    }
}

static void _drop_at(obj_ptr obj, int x, int y, int break_chance)
{
    drop_near(obj, break_chance, y, x);
}

void obj_drop_at(obj_ptr obj, int amt, int x, int y, int break_chance)
{
    assert(obj);
    assert(amt <= obj->number);

    if (!amt) return;

    if (amt < obj->number)
    {
        obj_t copy = *obj;
        copy.number = amt;
        if (obj->insured)
        {
            copy.insured = 0;
            if ((obj->insured % 100) > (obj->number - amt)) copy.insured = (obj->insured / 100) * 100 + (obj->insured % 100) - (obj->number - amt);
        }
        obj_dec_number(obj, amt, FALSE);
        _drop_at(&copy, x, y, break_chance);
    }
    else
    {
        obj->marked &= ~OM_WORN;
        _drop_at(obj, x, y, break_chance);
        obj->number = 0;
        obj_release(obj, OBJ_RELEASE_QUIET);
    }
}

static bool _can_destroy(obj_ptr obj)
{
    if (obj->loc.where == INV_EQUIP && obj->rune != RUNE_SACRIFICE)
        return FALSE;
    return TRUE;
}

void obj_destroy_ui(void)
{
    obj_prompt_t prompt = {0};
    char         name[MAX_NLEN];
    int          pos = 0;
    bool         force = command_arg > 0; /* 033kx to destroy 33 in slot (x) */
    int          amt = 1;

    if (p_ptr->special_defense & KATA_MUSOU)
        set_action(ACTION_NONE);

    /* Prompt for an object */
    prompt.prompt = "Destroy which item?";
    prompt.error = "You have nothing to destroy.";
    prompt.filter = _can_destroy;
    prompt.where[pos++] = INV_PACK;
    if (p_ptr->pclass == CLASS_RUNE_KNIGHT)
        prompt.where[pos++] = INV_EQUIP;
    prompt.where[pos++] = INV_QUIVER;
    prompt.where[pos++] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    /* Verify unless quantity given beforehand */
    if (!force && (confirm_destroy || (obj_value(prompt.obj) > 0) || ((prompt.obj->tval == TV_CORPSE) && (prace_is_(RACE_IGOR)) && (prompt.obj->sval != SV_SKELETON))))
    {
        char ch;
        int  options = OD_COLOR_CODED;
        char msg[MAX_NLEN + 100];

        if (prompt.obj->number > 1)
            options |= OD_OMIT_PREFIX;
        object_desc(name, prompt.obj, options);
        sprintf(msg, "Really destroy %s? <color:y>[y/n/Auto]</color>", name);

        ch = msg_prompt(msg, "nyA", PROMPT_DEFAULT);
        if (ch == 'n') return;
        if (ch == 'A')
        {
            if (autopick_autoregister(prompt.obj))
            {
                autopick_alter_obj(prompt.obj, TRUE); /* destroyed! */
                obj_release(prompt.obj, OBJ_RELEASE_QUIET);
            }
            return;
        }
    }

    /* Get a quantity. Note: get_quantity will return command_arg if set. */
    if (prompt.obj->number > 1)
    {
        amt = get_quantity(NULL, prompt.obj->number);
        if (amt <= 0) return;
    }

    /* Artifacts cannot be destroyed */
    if (!can_player_destroy_object(prompt.obj)) /* side effect: obj->sense = FEEL_SPECIAL */
    {
        object_desc(name, prompt.obj, OD_COLOR_CODED);
        msg_format("You cannot destroy %s.", name);
        return;
    }
    obj_destroy(prompt.obj, amt);
}

static void _destroy(obj_ptr obj)
{
    bool silent = FALSE;
    stats_on_p_destroy(obj, obj->number);

    /* Destroying a capture ball releases the monster inside */
    if ((obj->tval == TV_CAPTURE) && (obj->pval > 0)) 
    {
        int y = py;
        int x = px;
        if (obj->loc.where == INV_FLOOR)
        {
            y = obj->loc.y;
            x = obj->loc.x;
        }
        capture_ball_opening(obj, y, x, FALSE);
        silent = TRUE; /* capture ball destruction has its own messages */
    }
    {
        race_t  *race_ptr = get_race();
        class_t *class_ptr = get_class();
        bool     handled = FALSE;

        if (!handled && race_ptr->destroy_object)
            handled = race_ptr->destroy_object(obj);

        if (!handled && class_ptr->destroy_object)
            handled = class_ptr->destroy_object(obj);

        if (!handled && !silent)
        {
            if (obj->loc.where)
                msg_print("Destroyed.");
            else  /* Destroying part of a pile */
            {
                char name[MAX_NLEN];
                object_desc(name, obj, OD_COLOR_CODED);
                msg_format("You destroy %s.", name);
            }
        }
    }

    sound(SOUND_DESTITEM);

    if (high_level_book(obj))
        spellbook_destroy(obj);
    if (obj->to_a || obj->to_h || obj->to_d)
        virtue_add(VIRTUE_ENCHANTMENT, -1);

    if (obj_value_real(obj) > 30000)
        virtue_add(VIRTUE_SACRIFICE, 2);

    else if (obj_value_real(obj) > 10000)
        virtue_add(VIRTUE_SACRIFICE, 1);

    if (obj->to_a != 0 || obj->to_d != 0 || obj->to_h != 0)
        virtue_add(VIRTUE_HARMONY, 1);
}

void obj_destroy(obj_ptr obj, int amt)
{
    assert(obj);
    assert(amt <= obj->number);

    if (!amt) return;

    energy_use = 100;
    if (obj->loc.where != INV_FLOOR) p_ptr->update |= PU_BONUS; /* recalculate weight */
    if (amt < obj->number)
    {
        obj_t copy = *obj;
        copy.number = amt;
        obj_dec_number(obj, amt, TRUE);
        _destroy(&copy);
        if (obj->loc.where == INV_PACK)
            p_ptr->window |= PW_INVEN;
    }
    else
    {
        _destroy(obj);
        obj_zero(obj);
        obj_release(obj, OBJ_RELEASE_QUIET);
    }
}

void obj_describe_charges(obj_ptr obj)
{
    int charges;

    if (!object_is_device(obj)) return;
    if (!object_is_known(obj)) return;
    if (!obj->activation.cost) return; /* Just checking ... */

    charges = device_sp(obj) / obj->activation.cost;

    if (charges == 1)
    {
        msg_format("%s 1 charge remaining.",
            obj->loc.where == INV_FLOOR ? "There is" : "You have");
    }
    else
    {
        msg_format("%s %d charges remaining.",
            obj->loc.where == INV_FLOOR ? "There are" : "You have",
            charges);
    }
}

/************************************************************************
 * Savefiles
 ***********************************************************************/

enum object_save_fields_e {
    OBJ_SAVE_DONE = 0,
    OBJ_SAVE_PVAL,
    OBJ_SAVE_DISCOUNT,
    OBJ_SAVE_NUMBER,
    OBJ_SAVE_NAME1,
    OBJ_SAVE_NAME2,
    OBJ_SAVE_NAME3,
    OBJ_SAVE_ART_NAME,
    OBJ_SAVE_TIMEOUT,
    OBJ_SAVE_COMBAT,
    OBJ_SAVE_ARMOR,
    OBJ_SAVE_DAMAGE_DICE,
    OBJ_SAVE_IDENT,
    OBJ_SAVE_MARKED_BYTE,
    OBJ_SAVE_FEELING,
    OBJ_SAVE_INSCRIPTION,
    OBJ_SAVE_ART_FLAGS_0,
    OBJ_SAVE_ART_FLAGS_1,
    OBJ_SAVE_ART_FLAGS_2,
    OBJ_SAVE_ART_FLAGS_3,
    OBJ_SAVE_ART_FLAGS_4,
    OBJ_SAVE_ART_FLAGS_5,
    OBJ_SAVE_ART_FLAGS_6,
    OBJ_SAVE_ART_FLAGS_7,
    OBJ_SAVE_ART_FLAGS_8,
    OBJ_SAVE_ART_FLAGS_9,
    OBJ_SAVE_CURSE_FLAGS,
    OBJ_SAVE_RUNE_FLAGS,
    OBJ_SAVE_HELD_M_IDX,
    OBJ_SAVE_XTRA1,
    OBJ_SAVE_XTRA2,
    OBJ_SAVE_XTRA3,
    OBJ_SAVE_XTRA4,
    OBJ_SAVE_XTRA5_OLD,
    OBJ_SAVE_ACTIVATION,
    OBJ_SAVE_MULT,
    OBJ_SAVE_MARKED,
    OBJ_SAVE_XTRA5,
    OBJ_SAVE_KNOWN_FLAGS_0,
    OBJ_SAVE_KNOWN_FLAGS_1,
    OBJ_SAVE_KNOWN_FLAGS_2,
    OBJ_SAVE_KNOWN_FLAGS_3,
    OBJ_SAVE_KNOWN_FLAGS_4,
    OBJ_SAVE_KNOWN_FLAGS_5,
    OBJ_SAVE_KNOWN_FLAGS_6,
    OBJ_SAVE_KNOWN_FLAGS_7,
    OBJ_SAVE_KNOWN_FLAGS_8,
    OBJ_SAVE_KNOWN_FLAGS_9,
    OBJ_SAVE_KNOWN_CURSE_FLAGS,
    OBJ_SAVE_LEVEL,
    OBJ_SAVE_KNOWN_XTRA,
    OBJ_SAVE_ORIGIN,
    OBJ_SAVE_MITZE,
    OBJ_SAVE_INSURED,
};

void obj_load(obj_ptr obj, savefile_ptr file)
{
    object_kind *k_ptr;
    char         buf[128];

    object_wipe(obj);

    obj->k_idx = savefile_read_s16b(file);
    k_ptr = &k_info[obj->k_idx];
    obj->tval = k_ptr->tval;
    obj->sval = k_ptr->sval;

    obj->loc.where = savefile_read_byte(file);
    obj->loc.x = savefile_read_byte(file);
    obj->loc.y = savefile_read_byte(file);
    obj->loc.slot = savefile_read_s32b(file);

    obj->weight = savefile_read_s16b(file);

    obj->number = 1;

    for (;;)
    {
        byte code = savefile_read_byte(file);
        if (code == OBJ_SAVE_DONE)
            break;

        switch (code)
        {
        case OBJ_SAVE_PVAL:
            obj->pval = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_DISCOUNT:
            obj->discount = savefile_read_byte(file);
            break;
        case OBJ_SAVE_NUMBER:
            obj->number = savefile_read_byte(file);
            break;
        case OBJ_SAVE_NAME1:
            obj->name1 = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_NAME2:
            obj->name2 = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_NAME3:
            obj->name3 = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_TIMEOUT:
            obj->timeout = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_COMBAT:
            obj->to_h = savefile_read_s16b(file);
            obj->to_d = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_ARMOR:
            obj->to_a = savefile_read_s16b(file);
            obj->ac = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_DAMAGE_DICE:
            obj->dd = savefile_read_byte(file);
            obj->ds = savefile_read_byte(file);
            break;
        case OBJ_SAVE_MULT:
            obj->mult = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_IDENT:
            obj->ident = savefile_read_byte(file);
            break;
        case OBJ_SAVE_MARKED_BYTE:
            obj->marked = savefile_read_byte(file);
            break;
        case OBJ_SAVE_MARKED:
            obj->marked = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_0:
            obj->flags[0] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_1:
            obj->flags[1] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_2:
            obj->flags[2] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_3:
            obj->flags[3] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_4:
            obj->flags[4] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_ART_FLAGS_5:
            obj->flags[5] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_CURSE_FLAGS:
            obj->curse_flags = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_0:
            obj->known_flags[0] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_1:
            obj->known_flags[1] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_2:
            obj->known_flags[2] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_3:
            obj->known_flags[3] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_4:
            obj->known_flags[4] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_FLAGS_5:
            obj->known_flags[5] = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_CURSE_FLAGS:
            obj->known_curse_flags = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_KNOWN_XTRA:
            obj->known_xtra = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_RUNE_FLAGS:
            obj->rune = savefile_read_u32b(file);
            break;
        case OBJ_SAVE_HELD_M_IDX:
            obj->held_m_idx = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_XTRA1:
            obj->xtra1 = savefile_read_byte(file);
            break;
        case OBJ_SAVE_XTRA2:
            obj->xtra2 = savefile_read_byte(file);
            break;
        case OBJ_SAVE_XTRA3:
            obj->xtra3 = savefile_read_byte(file);
            break;
        case OBJ_SAVE_XTRA4:
            obj->xtra4 = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_XTRA5_OLD:
            obj->xtra5 = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_XTRA5:
            obj->xtra5 = savefile_read_s32b(file);
            break;
        case OBJ_SAVE_FEELING:
            obj->feeling = savefile_read_byte(file);
            break;
        case OBJ_SAVE_INSCRIPTION:
            savefile_read_cptr(file, buf, sizeof(buf));
            obj->inscription = quark_add(buf);
            break;
        case OBJ_SAVE_ART_NAME:
            savefile_read_cptr(file, buf, sizeof(buf));
            obj->art_name = quark_add(buf);
            break;
        case OBJ_SAVE_ACTIVATION:
            obj->activation.type = savefile_read_s16b(file);
            obj->activation.power = savefile_read_byte(file);
            obj->activation.difficulty = savefile_read_byte(file);
            obj->activation.cost = savefile_read_s16b(file);
            obj->activation.extra = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_LEVEL:
            obj->level = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_ORIGIN:
            obj->origin_type = savefile_read_byte(file);
            obj->origin_place = savefile_read_u16b(file);
            obj->origin_xtra = savefile_read_s16b(file);
            break;
        case OBJ_SAVE_MITZE:
            obj->mitze_type = savefile_read_byte(file);
            obj->mitze_level = savefile_read_byte(file);
            obj->mitze_turn = savefile_read_s32b(file);
            break;
        case OBJ_SAVE_INSURED:
            obj->insured = savefile_read_u16b(file);
            break;
        /* default:
            TODO: Report an error back to the load routine!!*/
        }
    }
    if (object_is_device(obj))
        add_flag(obj->flags, OF_ACTIVATE);
}

void obj_save(obj_ptr obj, savefile_ptr file)
{
    savefile_write_s16b(file, obj->k_idx);
    savefile_write_byte(file, obj->loc.where);
    savefile_write_byte(file, obj->loc.x);
    savefile_write_byte(file, obj->loc.y);
    savefile_write_s32b(file, obj->loc.slot);
    savefile_write_s16b(file, obj->weight);
    if (obj->pval)
    {
        savefile_write_byte(file, OBJ_SAVE_PVAL);
        savefile_write_s16b(file, obj->pval);
    }
    if (obj->discount)
    {
        savefile_write_byte(file, OBJ_SAVE_DISCOUNT);
        savefile_write_byte(file, obj->discount);
    }
    if (obj->number != 1)
    {
        savefile_write_byte(file, OBJ_SAVE_NUMBER);
        savefile_write_byte(file, obj->number);
    }
    if (obj->name1)
    {
        savefile_write_byte(file, OBJ_SAVE_NAME1);
        savefile_write_s16b(file, obj->name1);
    }
    if (obj->name2)
    {
        savefile_write_byte(file, OBJ_SAVE_NAME2);
        savefile_write_s16b(file, obj->name2);
    }
    if (obj->name3)
    {
        savefile_write_byte(file, OBJ_SAVE_NAME3);
        savefile_write_s16b(file, obj->name3);
    }
    if (obj->timeout)
    {
        savefile_write_byte(file, OBJ_SAVE_TIMEOUT);
        savefile_write_s16b(file, obj->timeout);
    }
    if (obj->to_h || obj->to_d)
    {
        savefile_write_byte(file, OBJ_SAVE_COMBAT);
        savefile_write_s16b(file, obj->to_h);
        savefile_write_s16b(file, obj->to_d);
    }
    if (obj->to_a || obj->ac)
    {
        savefile_write_byte(file, OBJ_SAVE_ARMOR);
        savefile_write_s16b(file, obj->to_a);
        savefile_write_s16b(file, obj->ac);
    }
    if (obj->dd || obj->ds)
    {
        savefile_write_byte(file, OBJ_SAVE_DAMAGE_DICE);
        savefile_write_byte(file, obj->dd);
        savefile_write_byte(file, obj->ds);
    }
    if (obj->mult)
    {
        savefile_write_byte(file, OBJ_SAVE_MULT);
        savefile_write_s16b(file, obj->mult);
    }
    if (obj->ident)
    {
        savefile_write_byte(file, OBJ_SAVE_IDENT);
        savefile_write_byte(file, obj->ident);
    }
    if (obj->marked)
    {
        savefile_write_byte(file, OBJ_SAVE_MARKED);
        savefile_write_u32b(file, obj->marked);
    }
    if (obj->flags[0])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_0);
        savefile_write_u32b(file, obj->flags[0]);
    }
    if (obj->flags[1])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_1);
        savefile_write_u32b(file, obj->flags[1]);
    }
    if (obj->flags[2])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_2);
        savefile_write_u32b(file, obj->flags[2]);
    }
    if (obj->flags[3])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_3);
        savefile_write_u32b(file, obj->flags[3]);
    }
    if (obj->flags[4])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_4);
        savefile_write_u32b(file, obj->flags[4]);
    }
    if (obj->flags[5])
    {
        savefile_write_byte(file, OBJ_SAVE_ART_FLAGS_5);
        savefile_write_u32b(file, obj->flags[5]);
    }
    if (obj->curse_flags)
    {
        savefile_write_byte(file, OBJ_SAVE_CURSE_FLAGS);
        savefile_write_u32b(file, obj->curse_flags);
    }
    if (obj->known_flags[0])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_0);
        savefile_write_u32b(file, obj->known_flags[0]);
    }
    if (obj->known_flags[1])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_1);
        savefile_write_u32b(file, obj->known_flags[1]);
    }
    if (obj->known_flags[2])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_2);
        savefile_write_u32b(file, obj->known_flags[2]);
    }
    if (obj->known_flags[3])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_3);
        savefile_write_u32b(file, obj->known_flags[3]);
    }
    if (obj->known_flags[4])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_4);
        savefile_write_u32b(file, obj->known_flags[4]);
    }
    if (obj->known_flags[5])
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_FLAGS_5);
        savefile_write_u32b(file, obj->known_flags[5]);
    }
    if (obj->known_curse_flags)
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_CURSE_FLAGS);
        savefile_write_u32b(file, obj->known_curse_flags);
    }
    if (obj->known_xtra)
    {
        savefile_write_byte(file, OBJ_SAVE_KNOWN_XTRA);
        savefile_write_u32b(file, obj->known_xtra);
    }
    if (obj->rune)
    {
        savefile_write_byte(file, OBJ_SAVE_RUNE_FLAGS);
        savefile_write_u32b(file, obj->rune);
    }
    if (obj->held_m_idx)
    {
        savefile_write_byte(file, OBJ_SAVE_HELD_M_IDX);
        savefile_write_s16b(file, obj->held_m_idx);
    }
    if (obj->xtra1)
    {
        savefile_write_byte(file, OBJ_SAVE_XTRA1);
        savefile_write_byte(file, obj->xtra1);
    }
    if (obj->xtra2)
    {
        savefile_write_byte(file, OBJ_SAVE_XTRA2);
        savefile_write_byte(file, obj->xtra2);
    }
    if (obj->xtra3)
    {
        savefile_write_byte(file, OBJ_SAVE_XTRA3);
        savefile_write_byte(file, obj->xtra3);
    }
    if (obj->xtra4)
    {
        savefile_write_byte(file, OBJ_SAVE_XTRA4);
        savefile_write_s16b(file, obj->xtra4);
    }
    if (obj->xtra5)
    {
        savefile_write_byte(file, OBJ_SAVE_XTRA5);
        savefile_write_s32b(file, obj->xtra5);
    }
    if (obj->feeling)
    {
        savefile_write_byte(file, OBJ_SAVE_FEELING);
        savefile_write_byte(file, obj->feeling);
    }
    if (obj->inscription)
    {
        savefile_write_byte(file, OBJ_SAVE_INSCRIPTION);
        savefile_write_cptr(file, quark_str(obj->inscription));
    }
    if (obj->art_name)
    {
        savefile_write_byte(file, OBJ_SAVE_ART_NAME);
        savefile_write_cptr(file, quark_str(obj->art_name));
    }
    if (obj->activation.type)
    {
        savefile_write_byte(file, OBJ_SAVE_ACTIVATION);
        savefile_write_s16b(file, obj->activation.type);
        savefile_write_byte(file, obj->activation.power);
        savefile_write_byte(file, obj->activation.difficulty);
        savefile_write_s16b(file, obj->activation.cost);
        savefile_write_s16b(file, obj->activation.extra);
    }
    if (obj->level)
    {
        savefile_write_byte(file, OBJ_SAVE_LEVEL);
        savefile_write_s16b(file, obj->level);
    }
    if (obj->origin_type)
    {
        savefile_write_byte(file, OBJ_SAVE_ORIGIN);
        savefile_write_byte(file, obj->origin_type);
        savefile_write_u16b(file, obj->origin_place);
        savefile_write_s16b(file, obj->origin_xtra);
        if (obj->mitze_type)
        {
            savefile_write_byte(file, OBJ_SAVE_MITZE);
            savefile_write_byte(file, obj->mitze_type);
            savefile_write_byte(file, obj->mitze_level);
            savefile_write_s32b(file, obj->mitze_turn);
        }
    }
    if (obj->insured)
    {
        savefile_write_byte(file, OBJ_SAVE_INSURED);
        savefile_write_u16b(file, obj->insured);
    }

    savefile_write_byte(file, OBJ_SAVE_DONE);
}

void special1_remove(int slot)
{
    inv_remove(get_race()->bonus_pack, slot);
}

void special2_remove(int slot)
{
    inv_remove(get_class()->bonus_pack, slot);
}

void special1_drop(obj_ptr obj)
{
    int amt = obj->number;

    assert(obj);
    assert(obj->loc.where == INV_SPECIAL1);
    assert(obj->number > 0);

    if (obj->number > 1)
    {
        amt = get_quantity(NULL, obj->number);
        if (amt <= 0)
        {
            energy_use = 0;
            return;
        }
    }

    if (amt >= obj->number)
    {
        char o_name[MAX_NLEN];
        char i_name[80];
        inv_ptr special_pack = get_race()->bonus_pack;
        assert(special_pack);
        object_desc(o_name, obj, OD_COLOR_CODED);
        strcpy(i_name, inv_name(special_pack));
        i_name[0] = tolower(i_name[0]);
        if (streq("ice Bag", i_name)) i_name[4] = tolower(i_name[4]);
        msg_format("You no longer have %s in your %s.", o_name, i_name);
    }

    obj_drop(obj, amt);
}

void special2_drop(obj_ptr obj)
{
    int amt = obj->number;

    assert(obj);
    assert(obj->loc.where == INV_SPECIAL2);
    assert(obj->number > 0);

    if (obj->number > 1)
    {
        amt = get_quantity(NULL, obj->number);
        if (amt <= 0)
        {
            energy_use = 0;
            return;
        }
    }

    if (amt >= obj->number)
    {
        char o_name[MAX_NLEN];
        char i_name[80];
        inv_ptr special_pack = get_class()->bonus_pack;
        assert(special_pack);
        object_desc(o_name, obj, OD_COLOR_CODED);
        strcpy(i_name, inv_name(special_pack));
        i_name[0] = tolower(i_name[0]);
        msg_format("You no longer have %s in your %s.", o_name, i_name);
    }

    obj_drop(obj, amt);
}
