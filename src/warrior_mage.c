#include "angband.h"

static power_info _get_powers[] =
{
    { A_INT, { 25, 0, 50, hp_to_sp_spell}},
    { A_INT, { 25, 0, 50, sp_to_hp_spell}},
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
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1200;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *warrior_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  37,  36,   2,  18,  16,  56,  50};
    skills_t xs = {  7,  11,  10,   0,   0,   0,  18,  15};

        me.name = "Warrior-Mage";
        me.desc = "A Warrior-Mage is precisely what the name suggests: a cross "
                    "between a warrior and a mage. To support their good-for-mages combat "
                    "abilities, Warrior-Mages begin the game with Arcane magic and can "
                    "freely select another spell realm. Although they do not gain "
                    "new spells as fast as regular mages, they will eventually learn "
                    "every spell in both realms, thus making them a very competitive "
                    "choice for those who appreciate Arcane spells. Their class powers "
                    "allow them to convert either HP to mana, or mana to HP, as needed.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 8;
        me.exp = 140;
        me.pets = 35;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;
        
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
