#include "angband.h"

static power_info _hex_mage_powers[] =
{
    { A_NONE, { 1, 0,  0, hex_stop_spelling_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info _high_mage_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info *_get_powers(void)
{
    if (p_ptr->realm1 == REALM_HEX)
        return _hex_mage_powers;
    else
        return _high_mage_powers;
}

static void _calc_bonuses(void)
{
    p_ptr->spell_cap += 3;
    p_ptr->to_d_spell += 5 + p_ptr->lev/5;
/*  p_ptr->spell_power += 2; 
    p_ptr->device_power += 2; */
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPELL_CAP);
/*  add_flag(flgs, TR_SPELL_POWER);
    add_flag(flgs, TR_MAGIC_MASTERY); */
}

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

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CLARITY, rand_range(10, 20));
    py_birth_obj_aux(TV_WAND, EFFECT_BOLT_MISSILE, 1);
    py_birth_spellbooks();
}

class_t *high_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  38,   3,  16,  20,  34,  20};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "High-Mage";
        me.desc = "High-Mages are mages who specialize in one particular field of "
                    "magic and learn it very well - much better than an ordinary mage.\n\n"
                    "For the price of giving up a second realm of magic, High-Mages "
                    "gain substantial benefits in the mana costs, power, minimum levels, and "
                    "failure rates of the spells in their speciality realm. They are also the "
                    "only class able to cast Hex spells. High-Mages have a class power - "
                    "'Eat Magic' - which absorbs mana from wands, staves, or rods; although "
                    "this power is not available to those who choose the Hex realm. Their "
                    "primary spellcasting stat is Intelligence.";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  4;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 94;
        me.base_hp = 0;
        me.exp = 130;
        me.pets = 25;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG |
                   CLASS_REGEN_MANA;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers_fn = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
