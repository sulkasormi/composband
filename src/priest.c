#include "angband.h"

static power_info _get_good_powers[] =
{
    { A_WIS, { 35, 70, 90, bless_weapon_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info _get_evil_powers[] =
{
    { A_WIS, { 42, 40, 80, evocation_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info *_get_powers(void)
{
    if (is_good_realm(p_ptr->realm1))
    {
        return _get_good_powers;
    }
    else
    {
        return _get_evil_powers;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "prayer";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

static bool _priest_weapon_is_icky(object_type *o_ptr)
{
    if (!object_is_weapon(o_ptr)) return FALSE;
    if (!obj_is_identified(o_ptr)) return FALSE; /* Might be icky... but we don't know yet */
    if ((o_ptr->tval != TV_SWORD) && (o_ptr->tval != TV_POLEARM)) return FALSE;
    if (is_evil_realm(p_ptr->realm1)) return FALSE;
    else
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        return !have_flag(flgs, OF_BLESSED);
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM)
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_BLESSED))
        {
        }
        else if (is_evil_realm(p_ptr->realm1))
        {
        }
        else
        {
            info_ptr->to_h -= 2;
            info_ptr->dis_to_h -= 2;

            info_ptr->to_d -= 2;
            info_ptr->dis_to_d -= 2;

            info_ptr->icky_wield = TRUE;
        }
    }
}

bool priest_is_good(void)
{
    if (p_ptr->pclass == CLASS_PRIEST && is_good_realm(p_ptr->realm1))
        return TRUE;
    return FALSE;
}

bool priest_is_evil(void)
{
    if (p_ptr->pclass == CLASS_PRIEST && is_evil_realm(p_ptr->realm1))
        return TRUE;
    return FALSE;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_HAFTED, SV_MACE, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_HEALING, 1);
    py_birth_spellbooks();
}

class_t *priest_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  28,  40,   2,  16,   8,  48,  35};
    skills_t xs = {  7,  11,  12,   0,   0,   0,  13,  11};

        me.name = "Priest";
        me.desc = "A Priest is a character devoted to serving a higher power. They "
                    "explore the dungeon in the service of their God. They are fairly "
                    "familiar with magical devices, which they believe act as foci for divine "
                    "intervention in the natural order of things.\n \n"
                    "There are two types of priests: Good and Evil. If the priest chooses Life "
                    "or Crusade as their first realm, they will follow the path of good; "
                    "as such, they may not choose an evil realm for their second realm. Good "
                    "priests abhor bloodshed, and therefore are not comfortable with edged weapons, "
                    "although they eventually learn to bless such weapons and then use them without disrupting their prayers. "
                    "Should a priest choose Death or Daemon as their first realm, however, "
                    "they will serve an evil god; evil priests actually enjoy shedding blood, and "
                    "suffer no such weapon restrictions. Of course, evil priests abhor good things, "
                    "and are unable to choose Life or Crusade for their second realm.\n \n"
                    "Good priests have a strong affinity for Life prayers, and learn them very well, even better "
                    "than a High-Mage. Conversely, evil priests favor prayers of Death, and receive "
                    "strong bonuses when choosing this foul realm. Otherwise, priests learn magic less "
                    "efficiently than a Mage would; but their greater stamina and combat skills compensate for this. "
                    "Priests, unlike Mages, cannot choose to study specific spells; rather, they "
                    "are granted new prayers by the whim of their deity, presumably in accordance with "
                    "some greater divine plan. The primary stat for a priest is Wisdom.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 120;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers_fn = _get_powers;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.character_dump = spellbook_character_dump;
        me.known_icky_object = _priest_weapon_is_icky;
        init = TRUE;
    }

    return &me;
}
