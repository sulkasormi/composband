#include "angband.h"

void heroism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Heroism");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily grants increased combat prowess and great bravery.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        set_hero(randint1(25) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_heroism(void) { return cast_spell(heroism_spell); }

void hide_in_mud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hide in Mud");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gain the ability to pass into walls temporarily, as well as extra resistance to acid.");
        break;
    case SPELL_CAST:
        set_kabenuke(randint1(p_ptr->lev/2) + p_ptr->lev/2, FALSE);
        set_oppose_acid(p_ptr->lev, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void ice_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 15;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ice Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of ice.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(
            GF_ICE,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void identify_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Identify");
        break;
    case SPELL_DESC:
        var_set_string(res, "Identify a single object.");
        break;
    case SPELL_CAST:
        var_set_bool(res, ident_spell(NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_identify(void) { return cast_spell(identify_spell); }

void identify_fully_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Identify True");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        var_set_bool(res, identify_fully(NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_identify_fully(void) { return cast_spell(identify_fully_spell); }

void hand_of_doom_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hand of Doom");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to mortally wound a target monster, draining a large proportion of their remaining health.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You invoke the Hand of Doom!");
        fire_ball_hide(GF_HAND_DOOM, dir, spell_power(p_ptr->lev * 3), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void haste_self_spell(int cmd, variant *res)
{
    int base = spell_power(p_ptr->lev);
    int sides = spell_power(20 + p_ptr->lev);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Haste Self");
        break;
    case SPELL_DESC:
        var_set_string(res, "Hastes you for a while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, sides));
        break;
    case SPELL_CAST:
        set_fast(base + randint1(sides), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void healing_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Healing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Powerful healing magic:  heals hitpoints, cuts and stun.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(spell_power(300)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(300));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void healing_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Healing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Powerful healing magic:  heals hitpoints, cuts and stun.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(500)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(500));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hellfire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hellfire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a powerful ball of evil power directly from the bowels of hell. Good monsters are especially susceptible.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(666 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_ball(GF_HELL_FIRE, dir, spell_power(666 + p_ptr->to_d_spell), 3);
            if (!demon_is_(DEMON_BALROG))
                take_hit(DAMAGE_USELIFE, 20 + randint1(30), "the strain of casting Hellfire");
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void hell_lance_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hell Lance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of pure hellfire.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_HELL_FIRE, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_hell_lance(void) { return cast_spell(hell_lance_spell); }

void holy_lance_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Holy Lance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of pure holiness.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_HOLY_FIRE, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_holy_lance(void) { return cast_spell(holy_lance_spell); }

void hp_to_sp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Convert HP to SP");
        break;
    case SPELL_DESC:
        var_set_string(res, "Converts HP into SP");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are subject to fits of painful clarity.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer subject to fits of painful clarity.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your blood sometimes rushes to your head.");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(4000))
        {
            int wounds = p_ptr->msp - p_ptr->csp;

            if (wounds > 0 && p_ptr->pclass != CLASS_RUNE_KNIGHT)
            {
                int healing = p_ptr->chp;

                if (healing > wounds)
                    healing = wounds;

                p_ptr->csp += healing;

                p_ptr->redraw |= (PR_MANA);
                take_hit(DAMAGE_LOSELIFE, healing, "blood rushing to the head");
            }
        }
        break;

    case SPELL_CAST:
    {
        int gain_sp = take_hit(DAMAGE_USELIFE, p_ptr->lev, "thoughtless convertion from HP to SP") / 5;
        if (gain_sp && p_ptr->pclass != CLASS_RUNE_KNIGHT)
        {
            p_ptr->csp += gain_sp;
            if (p_ptr->csp > p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }

            p_ptr->redraw |= PR_MANA;
        }
        else
            msg_print("You failed to convert.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void hypnotic_gaze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hypnotic Gaze");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to charm a monster.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your eyes look mesmerizing...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your eyes look uninteresting.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your gaze is hypnotic.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int power = p_ptr->lev;
            
            if (prace_is_(RACE_MON_VAMPIRE))
                power *= 2;

            msg_print("Your eyes look mesmerizing...");
            charm_monster(dir, power);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_hypnotic_gaze(void) { return cast_spell(hypnotic_gaze_spell); }

void imp_fire_spell(int cmd, variant *res)
{
    const int ball_lev = 30;
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, "Fire Ball");
        else
            var_set_string(res, "Fire Bolt");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "Fire Bolt/Ball");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, "Generate a Fire Ball on chosen target.");
        else
            var_set_string(res, "Hurls a fiery missile at chosen target.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Fire Bolt for L damage. At L30, does a radius 2 Fire Ball for 2L damage instead.");
        break;
    case SPELL_INFO:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        else
            var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        if (p_ptr->lev >= ball_lev)
            fire_ball(GF_FIRE, dir, spell_power(p_ptr->lev * 2), 2);
        else
            fire_bolt(GF_FIRE, dir, spell_power(p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        if (p_ptr->lev >= ball_lev)
            var_set_int(res, 7);
        else
            var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void invoke_logrus_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Invoke Logrus");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of chaos.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(10), 10, spell_power(p_ptr->lev*4 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_CHAOS, dir, spell_power(damroll(10, 10) + p_ptr->lev*4 + p_ptr->to_d_spell), 4);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void invulnerability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Invulnerability");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a globe of invulnerability which completely protects you from almost all attacks, consuming one turn when the barrier breaks or duration time is exceeded.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(4, 4));
        break;
    case SPELL_CAST:
        msg_print("You cast a Globe of Invulnerability.");
        set_invuln(spell_power(randint1(4) + 4), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void kiss_of_succubus_spell(int cmd, variant *res)
{
    int dam = spell_power(100 + p_ptr->lev * 2 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Kiss of Succubus");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of nexus.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) break;
        fire_ball(GF_NEXUS, dir, dam, 4);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void kutar_expand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Expand Horizontally");
        break;
    case SPELL_DESC:
        var_set_string(res, "Expand like a cat, temporarily giving +35 AC but a very poor saving throw.");
        break;
    case SPELL_CAST:
        set_tsubureru(randint1(20) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void laser_eye_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Laser Eye");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a laser beam.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev*2)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your eyes burn for a moment.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your eyes burn for a moment, then feel soothed.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your eyes can fire laser beams.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_LITE, dir, spell_power(2 * p_ptr->lev));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_laser_eye(void) { return cast_spell(laser_eye_spell); }

void light_area_spell(int cmd, variant *res)
{
    int dice = 2;
    int sides = p_ptr->lev / 2;
    int rad = spell_power(p_ptr->lev / 10 + 1);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Light Area");
        break;
    case SPELL_DESC:
        var_set_string(res, "Lights up nearby area and the inside of a room permanently.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You can light up rooms with your presence.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You can no longer light up rooms with your presence.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can emit bright light.");
        break;
    case SPELL_CAST:
        lite_area(spell_power(damroll(dice, sides)), rad);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_light_area(void) { return cast_spell(light_area_spell); }

void lightning_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of electricity.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 20 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ELEC, dir, spell_power(3*p_ptr->lev/2 + 20 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void lightning_bolt_spell(int cmd, variant *res)
{
    int dd = 3 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of electricity.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_ELEC,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void living_trump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Living Trump");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives mutation which makes you teleport randomly or makes you able to teleport at will.");
        break;
    case SPELL_CAST:
    {
        int mutation = one_in_(7) ? MUT_TELEPORT : MUT_TELEPORT_RND;

        if (mut_gain(mutation))
            msg_print("You have turned into a Living Trump.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

