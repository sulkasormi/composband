#include "angband.h"

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    spell_info* spell = &spells[ct++];
    spell->level = 1;
    spell->cost = 0;
    spell->fail = calculate_fail_rate(spell->level, 70, p_ptr->stat_ind[A_CHR]);
    spell->fn = dominate_living_I_spell;

    spell = &spells[ct++];
    spell->level = 30;
    spell->cost = 0;
    spell->fail = calculate_fail_rate(spell->level, 70, p_ptr->stat_ind[A_CHR]);
    spell->fn = dominate_living_II_spell;

    return ct;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.min_fail = 5;
        me.min_level = 3;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_POLEARM, SV_SPEAR, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *beastmaster_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  25,  32,   2,  18,  16,  52,  63};
    skills_t xs = {  7,  10,  10,   0,   0,   0,  14,  25};

        me.name = "Beastmaster";
        me.desc = "Beastmasters see the fierce creatures of the world as opportunities rather than threats, and their "
                    "riding skills are almost unparalleled. Between their class powers and access to Trump magic, "
                    "they have many ways to summon or charm living creatures to serve as "
                    "their hands and feet. Beastmasters are some of the best shooters outside archery specialists, and their "
                    "melee skills are good enough to get by on. Charisma determines the spellcasting prowess of a Beastmaster.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 103;
        me.base_hp = 6;
        me.exp = 120;
        me.pets = 10;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_WEAK;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
