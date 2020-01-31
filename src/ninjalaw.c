#include "angband.h"
#include "equip.h"

static void _detect_near_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Near");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 25)
            var_set_string(res, "Detects nearby monsters, traps, doors, stairs and objects.");
        else if (p_ptr->lev >= 5)
            var_set_string(res, "Detects nearby monsters, traps, doors and stairs.");
        else 
            var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (p_ptr->lev >= 5)
        {
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
        }
        if (p_ptr->lev >= 25)
        {
            detect_objects_normal(DETECT_RAD_DEFAULT);
        }
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
#define MAX_LAWYER_NINJUTSU 14

static spell_info _spells[MAX_LAWYER_NINJUTSU] =
{
    /*lvl cst fail spell */
    { 1,   1,  20, create_darkness_spell},
    { 2,   2,  25, _detect_near_spell},
    { 5,   6,  30, hide_in_leaves_spell},
    { 7,   8,  35, kawarimi_spell},
    {10,  15,  40, absconding_spell},
    {12,  15,  40, hit_and_away_spell},
    {15,  15,  50, floating_spell},
    {18,  18,  50, hide_in_flame_spell},
    {21,  24,  50, nyusin_spell},
    {24,  10,  50, syuriken_spreading_spell},
    {30,  35,  60, swap_pos_spell},
    {36,  40,  50, hide_in_mud_spell},
    {40,  45,  60, hide_in_mist_spell},
    {45,  60,  65, bunshin_spell},
};

static power_info _powers[] =
{
    { A_NONE, { 25, 0,  0, quick_walk_spell}},
    { -1, {-1, -1, -1, NULL}}
};

/* NOTE: We avoid using get_spells_aux() due to weird casting stat */
static int _get_spells(spell_info* spells, int max)
{
    int i, ct = 0;

    for (i = 0; i < MAX_LAWYER_NINJUTSU; i++)
    {
        spell_info *base = &_spells[i];
        if (ct >= max) break;
        if ((base->level <= p_ptr->lev) || (show_future_spells))
        {
            spell_info* current = &spells[ct];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;
            current->fail = calculate_fail_rate(base->level, base->fail, p_ptr->stat_ind[A_DEX]);

            ct++;
        }
    }
    return ct;
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    spellbook_character_dump(doc);
    doc_insert(doc, "<color:r>Realm:</color> <color:B>Ninjutsu</color>\n");
    py_display_spells_aux(doc, spells, ct);
}

static int _get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _powers);
}

static void _calc_bonuses(void)
{
    if (heavy_armor())
    {
        p_ptr->pspeed -= p_ptr->lev/10;
        p_ptr->skills.stl -= p_ptr->lev/10;
    }
    else if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->pspeed += 1;
        p_ptr->pspeed += p_ptr->lev/15;
        p_ptr->skills.stl += p_ptr->lev/15;
        if (p_ptr->lev >= 35)
            p_ptr->free_act++;
        /* Ninjas are not archers, and have relatively poor thb skills.
         * However, they excel at throwing (tht)! */
        p_ptr->skill_tht += 30 + p_ptr->lev;
    }
    if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->to_a += p_ptr->lev/2;
        p_ptr->dis_to_a += p_ptr->lev/2;
    }
    p_ptr->slow_digest = TRUE;
    if (p_ptr->lev >= 20) res_add(RES_FEAR);
    if (p_ptr->lev >= 30) res_add(RES_POIS);
    if (p_ptr->lev >= 40) p_ptr->see_inv++;
    p_ptr->see_nocto = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (heavy_armor())
        add_flag(flgs, OF_SPEED);
    else
    {
        if (!equip_find_obj(TV_SHIELD, SV_ANY))
        {
            add_flag(flgs, OF_SPEED);
        }
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_FREE_ACT);
    }
    if (p_ptr->lev >= 20) add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 40) add_flag(flgs, OF_SEE_INVIS);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if ( skills_weapon_is_icky(o_ptr->tval, o_ptr->sval) 
      || equip_find_obj(TV_SHIELD, SV_ANY) )
    {
        info_ptr->to_h -= 40;
        info_ptr->dis_to_h -= 40;
        info_ptr->icky_wield = TRUE;
        info_ptr->base_blow /= 2;
        info_ptr->xtra_blow /= 2;
        if (info_ptr->base_blow < 100) info_ptr->base_blow = 100;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "ninjutsu";
        me.which_stat = A_WIS;
        me.min_fail = 1;
        me.encumbrance.max_wgt = 160;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 500;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
    py_birth_obj_aux(TV_SPIKE, 0, rand_range(15, 20));
    py_birth_spellbooks();

    p_ptr->au += 100;
}

class_t *ninja_lawyer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  29,  38,   3,  42,  28,  66,  35 };
    skills_t xs = { 15,  11,  10,   1,   0,   0,  22,  11 };

        me.name = "Ninja-Lawyer";
        me.desc = "The Ninja-Lawyer moves silently in the night, combining the "
                  "stealth, agility and offensive power of the ninja with the "
                  "subtle practical tools of the lawyer. Ninja-Lawyers, like "
                  "specialist ninjas, prefer light armour and stabbing weapons and "
                  "acquire the powerful 'Quick Walk' class power. A ninja-lawyer "
                  "relies on both wisdom and dexterity, requiring the former for "
                  "legal tricks and the latter for ninjutsu.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 99;
        me.base_hp = 8;
        me.exp = 115;
        me.pets = 40;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        me.known_icky_object = skills_obj_is_icky_weapon;
        init = TRUE;
    }

    return &me;
}
