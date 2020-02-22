#include "angband.h"

static void _calc_bonuses(void)
{
    /* rangers are decent shooters all around, but especially good with bows */
    slot_t slot = equip_find_obj(TV_BOW, SV_ANY); /* fyi, shooter_info not set yet ... */
    if (slot) p_ptr->skills.thb += 20 + p_ptr->lev;
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if (p_ptr->shooter_info.tval_ammo != TV_ARROW )
        p_ptr->shooter_info.base_shot = 100;
}

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    spell_info* spell = &spells[ct++];
    spell->level = 15;
    spell->cost = 20;
    spell->fail = calculate_fail_rate(spell->level, 90, p_ptr->stat_ind[A_WIS]);
    spell->fn = probing_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        me.min_level = 3;
        me.min_fail = 5;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(20, 40));
    py_birth_spellbooks();
}

class_t *ranger_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  37,  36,   3,  24,  16,  56,  50};
    skills_t xs = {  8,  11,  10,   0,   0,   0,  18,  16};

        me.name = "Ranger";
        me.desc = "A Ranger is a seasoned wanderer from the plains or woods, attuned "
                    "to the natural world. Rangers have few weak points; like mages they "
                    "are excellent with magic devices, yet they are also fairly good "
                    "at using bows and melee weapons. Their stealth, searching and "
                    "perception have been sharpened by their time in the wilderness, "
                    "and their alliance with the spirits of nature even gives them a "
                    "good saving throw.\n \n"
                    "All rangers are trained in Nature magic, and all Nature spells are "
                    "available to them; they even learn these spells almost as fast as "
                    "mages. They can also select a secondary realm (Sorcery, "
                    "Chaos, Death, Trump, Arcane, and Daemon); but they are slow "
                    "learners here, and may find themselves unable to learn some of the "
                    "highest level spells. Another downside is that rangers, like the "
                    "priestly classes, lack the ability to choose their own spells; "
                    "they will learn whatever the capricious nature gods choose to teach them.\n\n"
                    "Rangers have a class power, 'Probe Monster', "
                    "which allows them to assess the strengths and weaknesses of the "
                    "monsters they meet. The magical powers of a ranger depend on Wisdom.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 8;
        me.exp = 140;
        me.pets = 35;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_SLOW | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
