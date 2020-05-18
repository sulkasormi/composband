/* File: obj_kind.c */

#include "angband.h"

#include <assert.h>

const int pval_flags[] = {
    OF_STR, OF_INT, OF_WIS, OF_DEX, OF_CON, OF_CHR,
    OF_MAGIC_MASTERY, OF_DEVICE_POWER, OF_MAGIC_RESISTANCE,
    OF_STEALTH, OF_SEARCH, OF_INFRA, OF_TUNNEL, OF_SPEED,
    OF_BLOWS, OF_XTRA_SHOTS, OF_XTRA_MIGHT,
    OF_SPELL_POWER, OF_SPELL_CAP, OF_WEAPONMASTERY, OF_LIFE,
    OF_DEC_STR, OF_DEC_INT, OF_DEC_WIS, OF_DEC_DEX, OF_DEC_CON, OF_DEC_CHR,
    OF_DEC_STEALTH, OF_DEC_SPEED, OF_DEC_LIFE,
    OF_DEC_MAGIC_MASTERY, OF_DEC_SPELL_CAP, OF_DEC_SPELL_POWER, OF_DEC_BLOWS,
    OF_INVALID
};

bool is_pval_flag(int which)
{
    int i;
    for (i = 0; ; i++)
    {
        int flg = pval_flags[i];
        if (flg == OF_INVALID) break;
        if (flg == which) return TRUE;
    }
    return FALSE;
}
bool have_pval_flag(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = 0; ; i++)
    {
        int flg = pval_flags[i];
        if (flg == OF_INVALID) break;
        if (have_flag(flgs, flg)) return TRUE;
    }
    return FALSE;
}

bool object_is_mushroom(object_type *o_ptr)
{
    bool result = FALSE;
    assert(SV_FOOD_MIN_MUSHROOM == 0);
    if ( o_ptr->tval == TV_FOOD
      /*&& SV_FOOD_MIN_MUSHROOM <= o_ptr->sval gcc warning on 0 <= unsigned char always true */
      && o_ptr->sval <= SV_FOOD_MAX_MUSHROOM )
    {
        result = TRUE;
    }
    return result;
}

bool object_is_flavor(object_type *o_ptr)
{
    return object_is_mushroom(o_ptr) || o_ptr->tval == TV_POTION || o_ptr->tval == TV_SCROLL;
}

bool object_is_potion(object_type *o_ptr)
{
    return (k_info[o_ptr->k_idx].tval == TV_POTION);
}

bool mon_is_wanted(int r_idx)
{
    return BOOL(r_info[r_idx].flagsx & RFX_WANTED);
}

bool object_is_shoukinkubi(object_type *o_ptr)
{
    /* Require corpse or skeleton */
    if (o_ptr->tval != TV_CORPSE) return FALSE;
    if ((o_ptr->sval > SV_BODY_HEAD) && (o_ptr->sval != SV_BODY_EARS)) return FALSE;

    /* Today's wanted */
    if ((p_ptr->today_mon > 0 && (streq(r_name + r_info[o_ptr->pval].name, r_name + r_info[today_mon].name)))
     && (o_ptr->sval < SV_BODY_HEAD)) return TRUE;

    /* Tsuchinoko */
    if ((o_ptr->pval == MON_TSUCHINOKO) && (o_ptr->sval < SV_BODY_HEAD)) return TRUE;

    /* Unique monster */
    if (mon_is_wanted((o_ptr->sval == SV_BODY_HEAD) ? o_ptr->xtra4 : o_ptr->pval)) return TRUE;

    /* Implorington */
    if ((!no_wilderness) && (o_ptr->pval == MON_IMPLORINGTON)) return TRUE;

    /* Not wanted */
    return FALSE;
}


/*
 * Favorite weapons
 */
bool object_is_favorite(object_type *o_ptr)
{
    if (p_ptr->pclass == CLASS_WEAPONMASTER)
        return weaponmaster_is_favorite(o_ptr);

    if (!object_is_melee_weapon(o_ptr) && !object_is_bow(o_ptr))
        return FALSE;

    /* Favorite weapons are varied depend on the class */
    switch (p_ptr->pclass)
    {
    case CLASS_PRIEST:
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags_known(o_ptr, flgs);

        if (!have_flag(flgs, OF_BLESSED) && 
            !(o_ptr->tval == TV_HAFTED))
            return FALSE;
        break;
    }

    case CLASS_MYSTIC:
        return FALSE;

    case CLASS_MONK:
    case CLASS_FORCETRAINER:
    case CLASS_NINJA:
    case CLASS_NINJA_LAWYER:
    case CLASS_DUELIST:
    case CLASS_SKILLMASTER:
        if (skills_weapon_is_icky(o_ptr->tval, o_ptr->sval))
            return FALSE;
        break;

    case CLASS_BEASTMASTER:
    case CLASS_CAVALRY:
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags_known(o_ptr, flgs);

        /* Is it known to be suitable to using while riding? */
        if (!(have_flag(flgs, OF_RIDING)))
            return FALSE;

        break;
    }

    case CLASS_MAULER:
        return o_ptr->weight >= 280;

    case CLASS_ARCHAEOLOGIST:
        return archaeologist_is_favored_weapon(o_ptr);

    case CLASS_ARCHER:
    case CLASS_RANGER:
        return object_is_bow(o_ptr);

    default:
        /* All weapons are okay for non-special classes */
        return TRUE;
    }

    return TRUE;
}


/*
 * Rare weapons/aromors
 * including Blade of Chaos, Dragon armors, etc.
 */
bool object_is_rare(object_type *o_ptr)
{
    switch(o_ptr->tval)
    {
    case TV_BOW:
        if (o_ptr->sval == SV_HARP) return TRUE;
        break;

    case TV_HAFTED:
        if (o_ptr->sval == SV_MACE_OF_DISRUPTION ||
            o_ptr->sval == SV_WIZSTAFF) return TRUE;
        break;

    case TV_POLEARM:
        if (o_ptr->sval == SV_SCYTHE_OF_SLICING ||
            o_ptr->sval == SV_DEATH_SCYTHE) return TRUE;
        break;

    case TV_SWORD:
        if (o_ptr->sval == SV_BLADE_OF_CHAOS ||
            o_ptr->sval == SV_DIAMOND_EDGE ||
            o_ptr->sval == SV_DOKUBARI ||
            o_ptr->sval == SV_DRAGON_FANG ||
            o_ptr->sval == SV_HAYABUSA) return TRUE;
        break;

    case TV_SHIELD:
        if (o_ptr->sval == SV_DRAGON_SHIELD ||
            o_ptr->sval == SV_MIRROR_SHIELD) return TRUE;
        break;

    case TV_HELM:
        if (o_ptr->sval == SV_DRAGON_HELM) return TRUE;
        break;

    case TV_BOOTS:
        if (o_ptr->sval == SV_PAIR_OF_DRAGON_GREAVE) return TRUE;
        break;

    case TV_CLOAK:
        if (o_ptr->sval == SV_ELVEN_CLOAK ||
            o_ptr->sval == SV_ETHEREAL_CLOAK ||
            o_ptr->sval == SV_DRAGON_CLOAK ||
            o_ptr->sval == SV_SHADOW_CLOAK) return TRUE;
        break;

    case TV_GLOVES:
        if (o_ptr->sval == SV_SET_OF_DRAGON_GLOVES) return TRUE;
        break;

    case TV_SOFT_ARMOR:
        if (o_ptr->sval == SV_KUROSHOUZOKU ||
            o_ptr->sval == SV_ABUNAI_MIZUGI) return TRUE;
        break;

    case TV_DRAG_ARMOR:
        return TRUE;

    case TV_ARROW:
        if (o_ptr->sval == SV_SEEKER_ARROW || o_ptr->sval == SV_MITHRIL_ARROW)
            return TRUE;
        break;

    case TV_BOLT:
        if (o_ptr->sval == SV_SEEKER_BOLT || o_ptr->sval == SV_MITHRIL_BOLT || o_ptr->sval == SV_ADAMANTINE_BOLT)
            return TRUE;
        break;

    case TV_SHOT:
        if (o_ptr->sval == SV_MITHRIL_SHOT)
            return TRUE;
        break;
    }

    /* Any others are not "rare" objects. */
    return FALSE;
}

bool object_is_cursed(object_type *o_ptr)
{
    return BOOL(o_ptr->curse_flags);
}

/*
 * Check if an object is weapon (including bows and ammo)
 */
bool object_is_weapon(object_type *o_ptr)
{
    if (TV_WEAPON_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_WEAPON_END) return TRUE;

    return FALSE;
}

bool object_is_cloak(object_type *o_ptr)
{
    if (o_ptr->tval == TV_CLOAK) return TRUE;
    return FALSE;
}
bool object_is_gloves(object_type *o_ptr)
{
    if (o_ptr->tval == TV_GLOVES) return TRUE;
    return FALSE;
}
bool object_is_helmet(object_type *o_ptr)
{
    if (o_ptr->tval == TV_HELM || o_ptr->tval == TV_CROWN) return TRUE;
    return FALSE;
}
bool object_is_ring(object_type *o_ptr)
{
    if (o_ptr->tval == TV_RING) return TRUE;
    return FALSE;
}
bool object_is_amulet(object_type *o_ptr)
{
    if (o_ptr->tval == TV_AMULET) return TRUE;
    return FALSE;
}
bool object_is_lite(object_type *o_ptr)
{
    if (o_ptr->tval == TV_LITE) return TRUE;
    return FALSE;
}
bool object_is_boots(object_type *o_ptr)
{
    if (o_ptr->tval == TV_BOOTS) return TRUE;
    return FALSE;
}
bool object_is_device(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_ROD: case TV_WAND: case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}

/*
 * Check if an object is weapon (including bows and ammo)
 */
bool object_is_weapon_ammo(object_type *o_ptr)
{
    if (TV_MISSILE_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_WEAPON_END) return TRUE;

    return FALSE;
}
bool object_is_bow(object_type *o_ptr)
{
    return o_ptr->tval == TV_BOW;
}

/*
 * Check if an object is ammo
 */
bool object_is_ammo(object_type *o_ptr)
{
    if (TV_MISSILE_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_MISSILE_END) return TRUE;

    return FALSE;
}


/*
 * Check if an object is armour
 */
bool object_is_armour(object_type *o_ptr)
{
    if (TV_ARMOR_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_ARMOR_END) return TRUE;

    return FALSE;
}

bool object_is_shield(object_type *o_ptr)
{
    if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CARD) return TRUE;
    return FALSE;
}

bool object_is_body_armour(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
        return TRUE;
        break;
    }
    return FALSE;
}


/*
 * Check if an object is weapon, armour or ammo
 */
bool enchantment_hack = FALSE;
bool object_is_weapon_armour_ammo(object_type *o_ptr)
{
    if (enchantment_hack && o_ptr->name1 == ART_HEPHAESTUS) return FALSE;

    if (object_is_weapon_ammo(o_ptr) || object_is_armour(o_ptr)) return TRUE;

    return FALSE;
}


/*
 * Melee weapons
 */
bool object_is_melee_weapon(object_type *o_ptr)
{
    if (TV_DIGGING <= o_ptr->tval && o_ptr->tval <= TV_SWORD) return TRUE;

    return FALSE;
}

bool object_is_jewelry(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_RING:
    case TV_AMULET:
        return TRUE;
    }

    return FALSE;
}

/*
 * Wearable including all weapon, all armour, bow, light source, amulet, and ring
 */
bool object_is_wearable(object_type *o_ptr)
{
    if (TV_WEARABLE_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_WEARABLE_END) return TRUE;

    return FALSE;
}


/*
 * Equipment including all wearable objects and ammo
 */
bool object_is_equipment(object_type *o_ptr)
{
    if (TV_EQUIP_BEGIN <= o_ptr->tval && o_ptr->tval <= TV_EQUIP_END) return TRUE;

    return FALSE;
}

/* Check for unenchantability */
bool object_is_unenchantable(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (o_ptr->name2 == EGO_ROBE_TWILIGHT) return TRUE;
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_DOKUBARI) return TRUE;
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_RUNESWORD) return TRUE;

    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_NO_ENCHANT)) return TRUE;

    return FALSE;
}


/*
 * Check if an object is weapon (including bows and ammo) and allows enchantment
 */
bool object_allow_enchant_weapon(object_type *o_ptr)
{
    if (object_is_weapon_ammo(o_ptr) && !object_is_unenchantable(o_ptr)) return TRUE;

    return FALSE;
}


/*
 * Check if an object is melee weapon and allows enchantment
 */
bool object_allow_enchant_melee_weapon(object_type *o_ptr)
{
    if (object_is_melee_weapon(o_ptr) && !object_is_unenchantable(o_ptr)) return TRUE;

    return FALSE;
}

/*
 * Check if an object is melee weapon and allows enchantment
 */
bool object_allow_enchant_armour(object_type *o_ptr)
{
    if (object_is_armour(o_ptr) && !object_is_unenchantable(o_ptr)) return TRUE;

    return FALSE;
}


/*
 * Check if an object is made by a smith's special ability
 */
bool object_is_smith(object_type *o_ptr)
{
    if (object_is_weapon_armour_ammo(o_ptr) && o_ptr->xtra3) return TRUE;

    return FALSE;
}


/*
 * Check if an object is artifact
 */
bool object_is_artifact(object_type *o_ptr)
{
    if (object_is_fixed_artifact(o_ptr) || o_ptr->art_name) return TRUE;

    return FALSE;
}

bool object_is_dragon_armor(object_type *o_ptr)
{
    /* TODO: Better name? This is all the dragon armor that gets bonus resistances
       and does not include Dragon Scale Mail */

    if (o_ptr->tval == TV_HELM && o_ptr->sval == SV_DRAGON_HELM)
        return TRUE;

    if (o_ptr->tval == TV_CLOAK && o_ptr->sval == SV_DRAGON_CLOAK)
        return TRUE;

    if (o_ptr->tval == TV_SHIELD && o_ptr->sval == SV_DRAGON_SHIELD)
        return TRUE;

    if (o_ptr->tval == TV_GLOVES && o_ptr->sval == SV_SET_OF_DRAGON_GLOVES)
        return TRUE;

    if (o_ptr->tval == TV_BOOTS && o_ptr->sval == SV_PAIR_OF_DRAGON_GREAVE)
        return TRUE;

    return FALSE;
}

/*
 * Check if an object is neither artifact, ego, nor 'smith' object
 */
bool object_is_nameless(object_type *o_ptr)
{
    if (!object_is_artifact(o_ptr) && !object_is_ego(o_ptr) && !object_is_smith(o_ptr))
        return TRUE;

    return FALSE;
}

/*
 * Check if an object is either an artifact or an ego
 */
bool object_is_art_or_ego(object_type *o_ptr)
{
    if (!o_ptr) return FALSE;
    if (object_is_artifact(o_ptr) || object_is_ego(o_ptr)) return TRUE;
    return FALSE;
}


/*
 * Check if an object is melee weapon and allows wielding with two-hands
 */
bool object_allow_two_hands_wielding(object_type *o_ptr)
{
    if (object_is_melee_weapon(o_ptr) && (o_ptr->name1 != ART_MJOLLNIR) && 
        ((o_ptr->weight > 99) || (o_ptr->tval == TV_POLEARM) || 
        ((o_ptr->tval == TV_HAFTED) && (o_ptr->sval == SV_BASEBALL_BAT)))) return TRUE;

    /* Hack: Shield Mastery technique */
    if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH && object_is_shield(o_ptr)) return TRUE;
    return FALSE;
}

/*
 * Check if an object is suitable ammo
 */
bool object_is_suitable_ammo(object_type *o_ptr)
{
    if (o_ptr->tval == p_ptr->shooter_info.tval_ammo) return TRUE;
    if (p_ptr->shooter_info.tval_ammo == TV_ANY_AMMO) return (object_is_ammo(o_ptr));
    return FALSE;
}
 
