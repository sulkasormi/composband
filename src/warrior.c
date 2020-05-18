#include "angband.h"

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 30)
        res_add(RES_FEAR);
    p_ptr->regen += 2 * p_ptr->lev;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->lev >= 45)
        add_flag(flgs, OF_REGEN);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    info_ptr->to_d += p_ptr->lev/5;
    info_ptr->dis_to_d += p_ptr->lev/5;
    info_ptr->xtra_blow += py_prorata_level_aux(100, 0, 1, 1);
}

static power_info _get_powers[] =
{
    { A_DEX, { 30, 25, 80, sword_dance_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(15, 30));
}

class_t *warrior_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  31,   1,  14,   2,  70,  55};
    skills_t xs = { 12,   7,  10,   0,   0,   0,  30,  30};

        me.name = "Warrior";
        me.desc = "A Warrior is a hack-and-slash character, who solves most of his "
                    "problems by cutting them to pieces, but will occasionally fall "
                    "back on the help of a magical device. Unfortunately, many "
                    "high-level devices may be forever beyond their use.\n \n"
                    "Warriors cast no spells. They hate magic. In fact, they even "
                    "gain experience for destroying high level spellbooks. They have a "
                    "class power - 'Sword Dancing' - which allows them to conduct a "
                    "melee attack in six random directions.";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 115;
        me.base_hp = 18;
        me.exp = 100;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        init = TRUE;
    }

    return &me;
}
