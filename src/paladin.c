#include "angband.h"

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 40)
        res_add(RES_FEAR);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_FEAR);
}

static power_info _get_good_powers[] =
{
    { A_WIS, { 30, 30, 70, holy_lance_spell}},
    { -1, {-1, -1, -1, NULL}}
};
static power_info _get_evil_powers[] =
{
    { A_WIS, { 30, 30, 70, hell_lance_spell}},
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
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 5;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_RING_MAIL, 1);
    py_birth_spellbooks();
}

class_t *paladin_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  24,  34,   1,  12,   2,  68,  40};
    skills_t xs = {  7,  10,  11,   0,   0,   0,  21,  18};

        me.name = "Paladin";
        me.desc = "A Paladin is a combination of a warrior and a priest. Paladins excel "
                    "as melee fighters, but are only middling in ranged combat; their "
                    "stealth, perception, and device skill are likewise mediocre, "
                    "though their divine alliance gives them a decent saving throw. Wisdom "
                    "determines a Paladin's success at praying to his deity.\n \n"
                    "Paladins can select a realm from Life, Crusade, Daemon and Death. "
                    "Like priests, they cannot select which prayers to learn, but are "
                    "rewarded with new prayers by their deities. They can learn all spells, "
                    "but not as fast as priests. They detest paganism so strongly that they "
                    "even gain experience for destroying high-level pagan spellbooks: "
                    "'pagan' means Life or Crusade spellbooks for a Death/Daemon "
                    "Paladin, and all spellbooks other than Life or Crusade for a "
                    "Life/Crusade Paladin. Paladins receive one class power, 'Holy Lance' "
                    "or 'Hell Lance', depending on the alignment of their realm.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 110;
        me.base_hp = 12;
        me.exp = 135;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers_fn = _get_powers;
        me.character_dump = spellbook_character_dump;
        me.get_flags = _get_flags;
        init = TRUE;
    }

    return &me;
}
