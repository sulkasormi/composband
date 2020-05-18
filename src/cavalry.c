#include "angband.h"

void rodeo_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rodeo");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
    {
        char m_name[80];
        monster_type *m_ptr;
        monster_race *r_ptr;
        int rlev;
        bool tame_success = FALSE;

        var_set_bool(res, FALSE);
        if (p_ptr->riding)
        {
            msg_print("You are already riding.");
            return;
        }
        if (!do_riding(TRUE)) return;
        
        var_set_bool(res, TRUE);

        m_ptr = &m_list[p_ptr->riding];
        r_ptr = &r_info[m_ptr->r_idx];
        monster_desc(m_name, m_ptr, 0);
        cmsg_format(TERM_L_GREEN, "You ride on %s.", m_name);
        if (is_pet(m_ptr)) break;
        rlev = r_ptr->level;
        if (r_ptr->flags1 & RF1_UNIQUE) rlev = rlev * 3 / 2;
        if (rlev > 60) rlev = 60+(rlev-60)/2;

        if (p_ptr->inside_arena || p_ptr->inside_battle)
        {
            cmsg_format(TERM_RED, "You cannot tame anything in here!");
            tame_success = FALSE;
        }
        else if ((r_ptr->flags7 & RF7_GUARDIAN) || (r_ptr->flagsx & RFX_QUESTOR))
        {
            cmsg_format(TERM_RED, "It is impossible to tame %s!", m_name);
            tame_success = FALSE;
        }
        else if (!((skills_riding_current() / 120 + p_ptr->lev * 2 / 3) > rlev
          && rlev < p_ptr->lev * 3 / 2 + (p_ptr->lev / 5)))
        {
            cmsg_format(TERM_RED, "You are not powerful enough to tame %s.", m_name);
            tame_success = FALSE;
        }
        else if (!(randint1(skills_riding_current() / 120 + p_ptr->lev * 2 / 3) > rlev
          && one_in_(2) 
          && rlev < p_ptr->lev * 3 / 2 + randint0(p_ptr->lev / 5) ))
        {
            // No message here, but still the "you have been thrown off" later down.
            tame_success = FALSE;
        }
        else
        {
            tame_success = TRUE;
        }

        if (tame_success)
        {
            cmsg_format(TERM_L_GREEN, "You tame %s.", m_name);
            set_pet(m_ptr);
        }
        else
        {
            cmsg_format(TERM_VIOLET, "You have been thrown off %s.", m_name);
            rakuba(1,TRUE);
            p_ptr->riding = 0;
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _calc_bonuses(void)
{
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if (p_ptr->shooter_info.tval_ammo != TV_ARROW )
        p_ptr->shooter_info.base_shot = 100;
}

static power_info _cavalry_powers[] =
{
    { A_STR, { 10, 0, 50, rodeo_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _birth(void)
{
    py_birth_obj_aux(TV_POLEARM, SV_BROAD_SPEAR, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(15, 25));
}

class_t *cavalry_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  18,  32,   1,  16,  10,  60,  66};
    skills_t xs = { 10,   7,  10,   0,   0,   0,  22,  26};

        me.name = "Cavalry";
        me.desc = "Cavalry are masters of horsemanship and enjoy riding to battle. Being good at "
                    "both melee and archery, they take pride in their overwhelming offensive strength "
                    "on horseback; but spellcasting is a closed book to them, and magical devices "
                    "difficult to use. High-level Cavalry learn to forcibly saddle and "
                    "tame wild monsters with their 'Rodeo' class power.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 111;
        me.base_hp = 10;
        me.exp = 120;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        me.get_powers = _cavalry_powers;
        me.get_flags = _get_flags;
        init = TRUE;
    }

    return &me;
}
