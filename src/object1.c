/* File: object1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Object code, part 1 */

#include "angband.h"

#include <assert.h>

#if defined(MACINTOSH) || defined(MACH_O_CARBON)
#ifdef verify
#undef verify
#endif
#endif
/*
 * Reset the "visual" lists
 *
 * This involves resetting various things to their "default" state.
 *
 * If the "prefs" flag is TRUE, then we will also load the appropriate
 * "user pref file" based on the current setting of the "use_graphics"
 * flag. This is useful for switching "graphics" on/off.
 *
 * The features, objects, and monsters, should all be encoded in the
 * relevant "font.pref" and/or "graf.prf" files. XXX XXX XXX
 *
 * The "prefs" parameter is no longer meaningful. XXX XXX XXX
 */
void reset_visuals(void)
{
    int i, j;

    /* Extract some info about terrain features */
    for (i = 0; i < max_f_idx; i++)
    {
        feature_type *f_ptr = &f_info[i];

        /* Assume we will use the underlying values */
        for (j = 0; j < F_LIT_MAX; j++)
        {
            f_ptr->x_attr[j] = f_ptr->d_attr[j];
            f_ptr->x_char[j] = f_ptr->d_char[j];
        }
    }

    /* Extract default attr/char code for objects */
    for (i = 0; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Default attr/char */
        k_ptr->x_attr = k_ptr->d_attr;
        k_ptr->x_char = k_ptr->d_char;
    }

    /* Extract default attr/char code for monsters */
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Default attr/char */
        r_ptr->x_attr = r_ptr->d_attr;
        r_ptr->x_char = r_ptr->d_char;
    }

    if (use_graphics)
    {
        char buf[1024];

        /* Process "graf.prf" */
        process_pref_file("graf.prf");

        /* Access the "character" pref file */
        sprintf(buf, "graf-%s.prf", player_base);

        /* Process "graf-<playername>.prf" */
        process_pref_file(buf);
    }

    /* Normal symbols */
    else
    {
        char buf[1024];

        /* Process "font.prf" */
        process_pref_file("font.prf");

        /* Access the "character" pref file */
        sprintf(buf, "font-%s.prf", player_base);

        /* Process "font-<playername>.prf" */
        process_pref_file(buf);
    }

    /* Respect easy_mimics */
    if (easy_mimics) toggle_easy_mimics(TRUE);
}

/*
 * Obtain the "flags" for an item
 */
void weapon_flags(int hand, u32b flgs[OF_ARRAY_SIZE])
{
    object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    if (o_ptr)
    {
        int i;
        obj_flags(o_ptr, flgs);
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= p_ptr->weapon_info[hand].flags[i];
        if ((disciple_is_(DISCIPLE_TROIKA)) && (object_is_melee_weapon(o_ptr))) troika_bonus_flags(o_ptr, flgs);
    }
}

void weapon_flags_known(int hand, u32b flgs[OF_ARRAY_SIZE])
{
    object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    if (o_ptr)
    {
        int i;
        obj_flags_known(o_ptr, flgs);
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= p_ptr->weapon_info[hand].known_flags[i];
        if (disciple_is_(DISCIPLE_TROIKA)) troika_bonus_flags(o_ptr, flgs);
    }
}

void missile_flags(object_type *arrow, u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    int slot = equip_find_first(object_is_bow);

    obj_flags(arrow, flgs);
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] |= p_ptr->shooter_info.flags[i];

    if (slot)
    {
        object_type *bow = equip_obj(slot);
        u32b         bow_flgs[OF_ARRAY_SIZE];

        obj_flags(bow, bow_flgs);
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= bow_flgs[i]; /* Mask? */
    }
}

void missile_flags_known(object_type *arrow, u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    int slot = equip_find_first(object_is_bow);

    obj_flags_known(arrow, flgs);
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] |= p_ptr->shooter_info.flags[i];

    if (slot)
    {
        object_type *bow = equip_obj(slot);
        u32b         bow_flgs[OF_ARRAY_SIZE];

        obj_flags_known(bow, bow_flgs);
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= bow_flgs[i]; /* Mask? */
    }
}

void obj_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    int i;

    /* Base object */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = k_ptr->flags[i];

    /* Artifact */
    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];

        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= a_ptr->flags[i];
    }

    /* Ego-item */
    if (object_is_ego(o_ptr))
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        bool      skip = FALSE;

        /* Ego lamps lose powers when they run out of fuel */
        switch (o_ptr->name2)
        {
        case EGO_LITE_IMMOLATION:
        case EGO_LITE_INFRAVISION:
        case EGO_LITE_IMMORTAL_EYE:
            if (o_ptr->sval <= SV_LITE_LANTERN && !o_ptr->xtra4)
                skip = TRUE;
            break;
        }

        if (!skip)
        {
            for (i = 0; i < OF_ARRAY_SIZE; i++)
                flgs[i] |= e_ptr->flags[i];
        }
    }

    /* Random artifact ! */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] |= o_ptr->flags[i];

    if (object_is_smith(o_ptr))
        weaponsmith_object_flags(o_ptr, flgs);
    if (object_is_deaggravated(o_ptr))
        remove_flag(flgs, OF_AGGRAVATE);
}

/*************************************************************
 *   Object Lore
 *************************************************************/

void obj_flags_known(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    int i;

    if (o_ptr->ident & IDENT_STORE)
    {
        obj_flags(o_ptr, flgs);
        return;
    }

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = 0;

    /* Base object: Note you still know an unidentified blade of chaos
       grants resist chaos, provided your aware of the object kind.*/
    if (k_ptr->aware)
    {
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] = k_ptr->flags[i];
    }

    /* Unidentified objects require special work. Anything you learn about
       an object before idenitification is marked in o_ptr->know_flags. We
       must not reveal ego_type or artifact_type known_flags, since that
       would reveal the underlying ego! cf obj_identify and obj_learn_flag.*/
    if (!obj_is_identified(o_ptr))
    {
        u32b actual[OF_ARRAY_SIZE];
        obj_flags(o_ptr, actual);
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] = actual[i] & o_ptr->known_flags[i];
        return;
    }

    /* Identified objects generally mark lore at the level of the flag itself */
    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];

        for (i = 0; i < OF_ARRAY_SIZE; i++)
            flgs[i] |= (a_ptr->flags[i] & a_ptr->known_flags[i]);
    }
    else if (object_is_ego(o_ptr))
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        bool      skip = FALSE;

        /* Ego lamps lose powers when they run out of fuel */
        switch (o_ptr->name2)
        {
        case EGO_LITE_IMMOLATION:
        case EGO_LITE_INFRAVISION:
        case EGO_LITE_IMMORTAL_EYE:
            if (o_ptr->sval <= SV_LITE_LANTERN && !o_ptr->xtra4)
                skip = TRUE;
            break;
        }

        if (!skip)
        {
            for (i = 0; i < OF_ARRAY_SIZE; i++)
            {
                flgs[i] |= (e_ptr->flags[i] & e_ptr->known_flags[i]);
                flgs[i] |= (o_ptr->flags[i] & e_ptr->known_flags[i]);
            }
        }
    }

    /* Random artifacts, extra resists, biffs on cursed egos, etc. */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] |= (o_ptr->flags[i] & o_ptr->known_flags[i]);

    /* Patch Up Activation overrides. For example, ego dragon scale
       mail might have an unlearned activation override. So, the player
       would know about the default Breathe activation, but might not
       yet have discovered that the activation is actually for Mass Genocide! */
    if ( have_flag(o_ptr->flags, OF_ACTIVATE)
      && !have_flag(o_ptr->known_flags, OF_ACTIVATE)
      && have_flag(flgs, OF_ACTIVATE) )
    {
        remove_flag(flgs, OF_ACTIVATE);
    }

    if (object_is_smith(o_ptr))
        weaponsmith_object_flags(o_ptr, flgs);
    if (object_is_deaggravated(o_ptr))
        remove_flag(flgs, OF_AGGRAVATE);
}

static bool _object_gives_esdm(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE],  bool easy_spell)
{
    if (o_ptr->name1)
    {
        if (o_ptr->name1 == ART_NAMAKE_BOW) return (!easy_spell);
        if (prace_is_(RACE_MON_ANGEL)) return (o_ptr->name1 == ART_STONE_OF_CRUSADE);
        if (prace_is_(RACE_MON_DEMON)) return ((p_ptr->psubrace == DEMON_BALROG) && (o_ptr->name1 == ART_STONE_OF_DAEMON));
        if (prace_is_(RACE_MON_VAMPIRE)) return (o_ptr->name1 == ART_NIGHT);
        if (p_ptr->pclass == CLASS_BARD) return (o_ptr->name1 == ART_DAERON || o_ptr->name1 == ART_MAGLOR);
        if (p_ptr->pclass == CLASS_TIME_LORD) return (o_ptr->name1 == ART_ETERNITY || o_ptr->name1 == ART_ETERNAL_BLADE);
        if ((p_ptr->pclass == CLASS_MINDCRAFTER) && (o_ptr->name1 == ART_STONE_OF_MIND)) return TRUE;
        if (p_ptr->pclass == CLASS_MIRROR_MASTER)
        {
            if (o_ptr->name1 == ART_YATA) return TRUE;
            if (o_ptr->name1 == ART_GIL_GALAD) return (!easy_spell);
        }
        if (p_ptr->pclass == CLASS_NECROMANCER)
        {
            if (o_ptr->name1 == ART_EYE_OF_VECNA) return (!easy_spell);
            if (o_ptr->name1 == ART_HAND_OF_VECNA) return easy_spell;
        }
        if ((p_ptr->pclass == CLASS_PSION) && (o_ptr->name1 == ART_STONE_OF_MIND)) return TRUE;
        if ((p_ptr->pclass == CLASS_RAGE_MAGE) && (o_ptr->name1 == ART_STRESS_BALL)) return TRUE;
        if (p_ptr->pclass == CLASS_WARLOCK)
        {
            if (warlock_is_(WARLOCK_UNDEAD)) return (o_ptr->name1 == ART_STONE_OF_DEATH);
            if (warlock_is_(WARLOCK_ANGELS)) return (o_ptr->name1 == ART_STONE_OF_CRUSADE || o_ptr->name1 == ART_STONE_OF_LIFE);
            if (warlock_is_(WARLOCK_DEMONS)) return (o_ptr->name1 == ART_STONE_OF_DAEMON);
            return FALSE;
        }
    }
    else if (o_ptr->name2)
    {
        if ((p_ptr->pclass == CLASS_ROGUE) && (!easy_spell) && (p_ptr->realm1 == REALM_BURGLARY) && (o_ptr->name2 == EGO_GLOVES_THIEF)) return TRUE;
        if ((p_ptr->pclass == CLASS_WILD_TALENT) && (!easy_spell) && (o_ptr->name2 == EGO_WEAPON_WILD)) return TRUE;
    }
    if (!have_flag(flgs, easy_spell ? OF_EASY_SPELL : OF_DEC_MANA)) return FALSE;
    else
    {
         caster_info *caster_ptr = get_caster_info();
         if (caster_ptr && (caster_ptr->options & CASTER_ALLOW_DEC_MANA)) return TRUE;
         return FALSE;
    }
}

static void _obj_esdm_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], bool display)
{
    if (have_flag(flgs, OF_DEC_MANA))
    {
        if (!_object_gives_esdm(o_ptr, flgs, FALSE)) remove_flag(flgs, OF_DEC_MANA);
    }
    else if (_object_gives_esdm(o_ptr, flgs, FALSE))
    {
        if ((!display) || (obj_is_identified(o_ptr)) || (have_flag(o_ptr->known_flags, OF_DEC_MANA))) add_flag(flgs, OF_DEC_MANA);
        else if (o_ptr->loc.where == INV_EQUIP) /* learn flag on equipped item */
        {
            add_flag(flgs, OF_DEC_MANA);
            add_flag(o_ptr->known_flags, OF_DEC_MANA);
        }
    }

    if (have_flag(flgs, OF_EASY_SPELL))
    {
        if (!_object_gives_esdm(o_ptr, flgs, TRUE)) remove_flag(flgs, OF_EASY_SPELL);
    }
    else if (_object_gives_esdm(o_ptr, flgs, TRUE))
    {
        if ((!display) || (obj_is_identified(o_ptr)) || (have_flag(o_ptr->known_flags, OF_EASY_SPELL))) add_flag(flgs, OF_EASY_SPELL);
        else if (o_ptr->loc.where == INV_EQUIP) /* learn flag on equipped item */
        {
            add_flag(flgs, OF_EASY_SPELL);
            add_flag(o_ptr->known_flags, OF_EASY_SPELL);
        }
    }
}

/* We don't do this in regular obj_flags() and obj_flags_known() partly
 * because it's slower, but mostly so that object value can be calculated
 * without any player specialities factoring into it */

void obj_flags_effective(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    obj_flags(o_ptr, flgs);
    _obj_esdm_flags(o_ptr, flgs, FALSE);
    if ((disciple_is_(DISCIPLE_TROIKA)) && (object_is_melee_weapon(o_ptr))) troika_bonus_flags(o_ptr, flgs);
}

void obj_flags_display(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    obj_flags_known(o_ptr, flgs);
    _obj_esdm_flags(o_ptr, flgs, TRUE);
}

static void _obj_flags_purify(u32b flgs[OF_ARRAY_SIZE])
{
    remove_flag(flgs, OF_HIDE_TYPE);
    remove_flag(flgs, OF_SHOW_MODS);
    remove_flag(flgs, OF_FULL_NAME);
    remove_flag(flgs, OF_FIXED_FLAVOR);
}

static bool _obj_flags_any(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = 0; i < OF_ARRAY_SIZE; i++)
    {
        if (flgs[i])
            return TRUE;
    }
    return FALSE;
}

void obj_flags_unknown(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    u32b actual[OF_ARRAY_SIZE];
    u32b known[OF_ARRAY_SIZE];
    int  i;

    assert(o_ptr);

    obj_flags(o_ptr, actual);
    _obj_flags_purify(actual);

    obj_flags_known(o_ptr, known);

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = actual[i] & (~known[i]);
}

static void _obj_identify_aux(object_type *o_ptr)
{
    int i;

    k_info[o_ptr->k_idx].aware = TRUE;

    o_ptr->feeling = FEEL_NONE;
    o_ptr->ident &= ~(IDENT_SENSE | IDENT_EMPTY | IDENT_TRIED);
    o_ptr->ident |= IDENT_KNOWN;

    object_mitze(o_ptr, MITZE_ID);

    if ((!o_ptr) || (!o_ptr->number)) return;

    if ((p_ptr->sense_artifact) && (o_ptr->loc.where = INV_FLOOR) && (object_is_artifact(o_ptr)))
    {
        p_ptr->sense_artifact = sense_great_discovery();
        if (!p_ptr->sense_artifact) p_ptr->redraw |= PR_STATUS;
    }

    /* Lore on unidentified objects is tricky, but flavorful.
       Patch up the lore flags, putting them in their correct
       hierarchical locations. */
    if (o_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        for (i = 0; i < OF_ARRAY_SIZE; i++)
        {
            a_ptr->known_flags[i] |= (o_ptr->known_flags[i] & a_ptr->flags[i]);
            o_ptr->known_flags[i] &= ~a_ptr->flags[i];
        }
        add_flag(a_ptr->known_flags, OF_IGNORE_ACID);
        add_flag(a_ptr->known_flags, OF_IGNORE_ELEC);
        add_flag(a_ptr->known_flags, OF_IGNORE_FIRE);
        add_flag(a_ptr->known_flags, OF_IGNORE_COLD);
    }
    else if (o_ptr->art_name)
    {
        add_flag(o_ptr->known_flags, OF_IGNORE_ACID);
        add_flag(o_ptr->known_flags, OF_IGNORE_ELEC);
        add_flag(o_ptr->known_flags, OF_IGNORE_FIRE);
        add_flag(o_ptr->known_flags, OF_IGNORE_COLD);
    }
    else if (o_ptr->name2)
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        bool      activate = have_flag(o_ptr->known_flags, OF_ACTIVATE);

        for (i = 0; i < OF_ARRAY_SIZE; i++)
        {
            e_ptr->known_flags[i] |= (o_ptr->known_flags[i] & e_ptr->flags[i]);
            e_ptr->known_flags[i] |= (o_ptr->known_flags[i] & e_ptr->xtra_flags[i]);
            o_ptr->known_flags[i] &= ~(e_ptr->flags[i] | e_ptr->xtra_flags[i]);
        }

        /* Patch up activation overrides. OF_ACTIVATE will exist in e_ptr->xtra_flags,
           and the user may have learned the activation prior to Identify */
        if (o_ptr->activation.type && activate)
            add_flag(o_ptr->known_flags, OF_ACTIVATE);

        /* Automatically know previously learned random activations */
        if (o_ptr->activation.type && !object_is_device(o_ptr) && effect_is_known(o_ptr->activation.type))
            add_flag(o_ptr->known_flags, OF_ACTIVATE);
    }
}

static void _obj_learn_curses(object_type *o_ptr)
{
    /* Relearn about curses ... This knowledge will be wiped on Remove Curse
       so that recursing repeats the fun (As opposed to the old system, where
       you automatically knew new curses on *identified* items). */
    o_ptr->known_curse_flags = o_ptr->curse_flags;
}

static void _obj_identify_fully_aux(object_type *o_ptr)
{
    int i;

    if (o_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        for (i = 0; i < OF_ARRAY_SIZE; i++)
        {
            a_ptr->known_flags[i] |= a_ptr->flags[i];
            o_ptr->known_flags[i] |= o_ptr->flags[i] & (~a_ptr->flags[i]);
        }
    }
    else if (o_ptr->name2)
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        for (i = 0; i < OF_ARRAY_SIZE; i++)
        {
            e_ptr->known_flags[i] |= e_ptr->flags[i];
            o_ptr->known_flags[i] |= o_ptr->flags[i] & (~e_ptr->flags[i]);

            /* Mark variable ego attributes as possibilities for future.
               Note: The next time an ego of this type spawns with a known
               possible flag, it will be learned on Identify. See above. */
            if (object_is_cursed(o_ptr))
                e_ptr->known_flags[i] |= (o_ptr->flags[i] & e_ptr->xtra_flags[i]);
            else
                e_ptr->known_flags[i] |= o_ptr->flags[i];
        }
        if (object_is_device(o_ptr))
            remove_flag(e_ptr->known_flags, OF_ACTIVATE);
    }
    else /* perhaps a rand-art? */
    {
        for (i = 0; i < OF_ARRAY_SIZE; i++)
            o_ptr->known_flags[i] |= o_ptr->flags[i];
    }

    /* Learn random activations */
    if (o_ptr->activation.type && !object_is_device(o_ptr))
    {
        add_flag(o_ptr->known_flags, OF_ACTIVATE);
        effect_learn(o_ptr->activation.type);
    }

    _obj_learn_curses(o_ptr);
    object_mitze(o_ptr, MITZE_ID);
}

bool obj_is_identified(object_type *o_ptr)
{
    assert(o_ptr);
    return (o_ptr->ident & (IDENT_KNOWN | IDENT_STORE)) ? TRUE : FALSE;
}

bool obj_is_identified_fully(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (o_ptr->ident & IDENT_STORE) return TRUE;
    obj_flags_unknown(o_ptr, flgs);
    return !_obj_flags_any(flgs);
}

void obj_identify(object_type *o_ptr)
{
    assert(o_ptr);
	if (easy_id)
		obj_identify_fully(o_ptr);
	else if (!obj_is_identified(o_ptr))
        _obj_identify_aux(o_ptr);
}

void obj_identify_fully(object_type *o_ptr)
{
    if ((!o_ptr) || (!o_ptr->k_idx)) return;
    if (!obj_is_identified(o_ptr))
        _obj_identify_aux(o_ptr);
    if ((!o_ptr) || (!o_ptr->k_idx)) return;
    if (!obj_is_identified_fully(o_ptr))
        _obj_identify_fully_aux(o_ptr);
    else
        _obj_learn_curses(o_ptr);
}

void obj_learn_store(object_type *o_ptr)
{
    bool ohs = no_karrot_hack;
    no_karrot_hack = TRUE;
    _obj_identify_fully_aux(o_ptr);
    no_karrot_hack = ohs;
}

bool obj_learn_flag(object_type *o_ptr, int which)
{
    assert(o_ptr);
    /* Lore on unidentified objects is tricky, but flavorful */
    if (!obj_is_identified(o_ptr))
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, which) && !have_flag(o_ptr->known_flags, which))
        {
            add_flag(o_ptr->known_flags, which);
            return TRUE;
        }
        return FALSE;
    }
    if (o_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        if (have_flag(a_ptr->flags, which))
        {
            if (have_flag(a_ptr->known_flags, which)) return FALSE;
            add_flag(a_ptr->known_flags, which);
            return TRUE;
        }
        else if (have_flag(o_ptr->flags, which))
        {
            if (have_flag(o_ptr->known_flags, which)) return FALSE;
            add_flag(o_ptr->known_flags, which);
            return TRUE;
        }
    }
    else if (o_ptr->name2)
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        if (have_flag(e_ptr->flags, which))
        {
            if (have_flag(e_ptr->known_flags, which)) return FALSE;
            add_flag(e_ptr->known_flags, which);
            return TRUE;
        }
        else if (have_flag(o_ptr->flags, which))
        {
            if (have_flag(e_ptr->xtra_flags, which))
            {
                if (have_flag(e_ptr->known_flags, which)) return FALSE;
                add_flag(e_ptr->known_flags, which);
            }
            else
            {
                if (have_flag(o_ptr->known_flags, which)) return FALSE;
                add_flag(o_ptr->known_flags, which);
            }
            return TRUE;
        }
    }
    /* Random Artifact */
    else if (have_flag(o_ptr->flags, which))
    {
        if (!have_flag(o_ptr->known_flags, which))
        {
            add_flag(o_ptr->known_flags, which);
            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}
void obj_learn_activation(object_type *o_ptr)
{
    assert(o_ptr);
    /* Lore on unidentified objects is tricky, but flavorful */
    if (!obj_is_identified(o_ptr))
    {
        if (obj_has_effect(o_ptr) && !have_flag(o_ptr->known_flags, OF_ACTIVATE))
            add_flag(o_ptr->known_flags, OF_ACTIVATE);
        return;
    }

    if (o_ptr->activation.type)
    {
        add_flag(o_ptr->known_flags, OF_ACTIVATE);
        effect_learn(o_ptr->activation.type);
    }
    else if (o_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        if (a_ptr->activation.type)
            add_flag(a_ptr->known_flags, OF_ACTIVATE);
        else
            add_flag(o_ptr->known_flags, OF_ACTIVATE); /* Paranoia: Activation on k_ptr, but that should be known by default! */
    }
    else if (o_ptr->name2)
    {
        ego_type *e_ptr = &e_info[o_ptr->name2];
        if (e_ptr->activation.type)
            add_flag(e_ptr->known_flags, OF_ACTIVATE);
        else
            add_flag(o_ptr->known_flags, OF_ACTIVATE); /* Paranoia: Activation on k_ptr, but that should be known by default! */
    }
}

bool obj_learn_curse(object_type *o_ptr, int flag)
{
    assert(o_ptr);
    if (o_ptr->curse_flags & flag)
    {
        if (!(o_ptr->known_curse_flags & flag))
        {
            o_ptr->known_curse_flags |= flag;
            return TRUE;
        }
    }
    return FALSE;
}

void obj_learn_slay(object_type *o_ptr, int which, cptr msg)
{
    assert(o_ptr);
    if (obj_learn_flag(o_ptr, which))
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, OD_LORE);
        msg_format("<color:B>You learn that your %s %s.</color>", buf, msg);
    }

    if (which == OF_VORPAL2) /* hack - always display vorpal2 as |SS */
    {
        (void)obj_learn_flag(o_ptr, OF_VORPAL);
    }

    /* Gloves and rings may give additional slays ... */
    equip_learn_slay(which, msg);
}

const int _xtra_lore_flags[] = {
    OF_LEVITATION, OF_REGEN, OF_EASY_SPELL, OF_DEC_MANA,
    OF_AURA_FIRE, OF_AURA_ELEC, OF_AURA_COLD, OF_AURA_SHARDS,
    OF_LITE, OF_DARKNESS, OF_SLOW_DIGEST, OF_REGEN_MANA,
    OF_INVALID
};

void obj_learn_equipped(object_type *o_ptr)
{
    bool learned = FALSE;
    int  i;

    for (i = 0; ; i++)
    {
        int flg = pval_flags[i];
        if (flg == OF_INVALID) break;
        if (obj_learn_flag(o_ptr, flg)) learned = TRUE;
    }

    for (i = 0; ; i++)
    {
        int flg = _xtra_lore_flags[i];
        if (flg == OF_INVALID) break;
        if (obj_learn_flag(o_ptr, flg)) learned = TRUE;
    }

    if (p_ptr->pclass == CLASS_PRIEST)
    {
        if (obj_learn_flag(o_ptr, OF_BLESSED)) learned = TRUE;
    }

    if (learned) /* TODO: Give messages for each learned flag? */
    {
        char buf[MAX_NLEN];
        object_desc(buf, o_ptr, OD_LORE);
        msg_format("<color:B>You learn more about your %s.</color>", buf);
    }
}

bool ego_has_lore(ego_type *e_ptr)
{
    return _obj_flags_any(e_ptr->known_flags);
}

bool art_has_lore(artifact_type *a_ptr)
{
    return _obj_flags_any(a_ptr->known_flags);
}

bool obj_has_lore(object_type *o_ptr)
{
    if (_obj_flags_any(o_ptr->known_flags))
        return TRUE;
    if (o_ptr->name1)
        return art_has_lore(&a_info[o_ptr->name1]);
    if (o_ptr->name2)
        return ego_has_lore(&e_info[o_ptr->name2]);
    return FALSE;
}

/* Hack: Check if a spellbook is one of the realms we can use. -- TY */

bool check_book_realm(const byte book_tval, const byte book_sval)
{
    if (book_tval < TV_LIFE_BOOK) return FALSE;
    if (p_ptr->pclass == CLASS_SORCERER)
    {
        return is_magic(tval2realm(book_tval));
    }
    else if (p_ptr->pclass == CLASS_RED_MAGE)
    {
        if (is_magic(tval2realm(book_tval)))
            return ((book_tval == TV_ARCANE_BOOK) || (book_sval < 2));
    }
    else if (p_ptr->pclass == CLASS_GRAY_MAGE)
    {
        return gray_mage_is_allowed_book(book_tval, book_sval);
    }
    else if (p_ptr->pclass == CLASS_SKILLMASTER)
    {
        return skillmaster_is_allowed_book(book_tval, book_sval);
    }
    return (REALM1_BOOK == book_tval || REALM2_BOOK == book_tval);
}

/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
    int j;

    /* Scan windows */
    for (j = 0; j < 8; j++)
    {
        /* Unused */
        if (!angband_term[j]) continue;

        /* Flip inven to equip */
        if (window_flag[j] & (PW_INVEN))
        {
            /* Flip flags */
            window_flag[j] &= ~(PW_INVEN);
            window_flag[j] |= (PW_EQUIP);

            /* Window stuff */
            p_ptr->window |= (PW_EQUIP);
        }

        /* Flip inven to equip */
        else if (window_flag[j] & (PW_EQUIP))
        {
            /* Flip flags */
            window_flag[j] &= ~(PW_EQUIP);
            window_flag[j] |= (PW_INVEN);

            /* Window stuff */
            p_ptr->window |= (PW_INVEN);
        }
    }
}

void toggle_mon_obj_lists(void)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        if (!angband_term[i]) continue;
        if (window_flag[i] & PW_MONSTER_LIST)
        {
            window_flag[i] &= ~PW_MONSTER_LIST;
            window_flag[i] |= PW_OBJECT_LIST;
            p_ptr->window |= PW_OBJECT_LIST;
        }
        else if (window_flag[i] & PW_OBJECT_LIST)
        {
            window_flag[i] &= ~PW_OBJECT_LIST;
            window_flag[i] |= PW_MONSTER_LIST;
            p_ptr->window |= PW_MONSTER_LIST;
        }
    }
}

