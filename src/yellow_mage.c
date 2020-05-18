#include "angband.h"

static power_info _get_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _calc_bonuses(void)
{
    p_ptr->spells_per_round += py_prorata_level(150);
}


static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_spellbooks();
}

class_t *yellow_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  38,   3,  16,  20,  34,  20};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "Yellow-Mage";
        me.desc = "A Yellow-Mage is a type of Mage who focuses on rapid spell casting. "
                    "The casting speed of Yellow-Mages improves with experience; low-level spells "
                    "are particularly quick to use, and can eventually be cast as many as five "
                    "times in one round! But even a level 50 spell requires less than half a "
                    "turn for an accomplished Yellow-Mage.\n\n"
                    "In most other respects, a Yellow-Mage is similar to a Mage; they use Intelligence "
                    "as their primary spell stat, and may learn spells from two spell realms of their choice. " 
                    "However, due to their focus on hasty casting, some of the deepest "
                    "and most powerful magic will remain forever beyond their grasp.";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 95;
        me.base_hp = 0;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG |
                   CLASS_REGEN_MANA;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        me.character_dump = spellbook_character_dump;
        me.get_powers = _get_powers;
        init = TRUE;
    }

    return &me;
}
