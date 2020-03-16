#include "angband.h"

void acid_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Acid Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate an Acid Ball on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 35 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ACID, dir, spell_power(3*p_ptr->lev/2 + 35 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void acid_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 8;

    if (elemental_is_(ELEMENTAL_WATER)) dd = 2 + p_ptr->lev / 5 + water_flow_rate() / 10;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Acid Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt or beam of acid.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(damroll(dd, ds) + p_ptr->to_d_spell);
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir, dam);
        var_set_bool(res, TRUE);
        water_mana_action(2, dam * 3 / 5);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        if (elemental_is_(ELEMENTAL_WATER)) var_set_int(res, dd * 3 / 2 - 6);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void alchemy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Alchemy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Turns valuable items into gold.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the Midas touch.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the Midas touch.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can turn ordinary items to gold.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (alchemy())
            var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_alchemy(void) { return cast_spell(alchemy_spell); }

void alter_reality_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Alter Reality");
        break;
    case SPELL_DESC:
        var_set_string(res, "Recreates current dungeon level.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_delay(15, 20));
        break;
    case SPELL_CAST:
        alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void amnesia_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Amnesia");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to make target monster forget something.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int lvl = p_ptr->lev;
        if (p_ptr->lev > 40)
            lvl += (p_ptr->lev - 40) * 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        project_hook(GF_AMNESIA, dir, lvl, PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_ray_gun_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ray Gun");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires unresistable damage at chosen foe.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(5 + (p_ptr->lev+1) / 2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        
        msg_print("You fire your ray gun.");
        fire_bolt(GF_MISSILE, dir, spell_power(5 + (p_ptr->lev+1) / 2));
        
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_blaster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blaster");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(5 + p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You fire your blaster.");
        fire_bolt(GF_MISSILE, dir, spell_power(5 + p_ptr->lev));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_bazooka_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bazooka");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires your bazooka at a nearby monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(25 + p_ptr->lev * 2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You fire your bazooka.");
        fire_ball(GF_MISSILE, dir, spell_power(25 + p_ptr->lev * 2), 2);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_beam_cannon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Beam Cannon");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(25 + p_ptr->lev * 3)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You fire a beam cannon.");
        fire_beam(GF_MISSILE, dir, spell_power(25 + p_ptr->lev * 3));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_rocket_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rocket Launcher");
        break;
    case SPELL_DESC:
        var_set_string(res, "Launches a powerful rocket at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 7)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You launch a rocket.");
        fire_rocket(GF_ROCKET, dir, spell_power(p_ptr->lev * 7), 2);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void animate_dead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Animate Dead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Resurrects nearby corpse and skeletons. And makes these your pets.");
        break;
    case SPELL_CAST:
        animate_dead(0, py, px);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void awesome_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Awesome Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack a monster with a single melee blow. If blow hits, does normal melee damage and propels the monster backwards.");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        var_set_bool(res, FALSE);

        if (!equip_find_first(object_is_melee_weapon))
        {
            msg_print("You need to wield a weapon!");
            return;
        }
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (cave[y][x].m_idx)
        {
            py_attack(y, x, MELEE_AWESOME_BLOW);
        }
        else
        {
            msg_print("There is no monster.");
            return;
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void banish_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Banish Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to remove a single evil opponent.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a holy wrath fill you.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel a holy wrath.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can send evil creatures directly to Hell.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;
        cave_type *c_ptr;
        monster_type *m_ptr;
        monster_race *r_ptr;

        if (!get_rep_dir2(&dir)) 
        {
            var_set_bool(res, FALSE);
            break;
        }

        var_set_bool(res, TRUE);

        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];

        if (!c_ptr->m_idx)
        {
            msg_print("You sense no evil there!");
            break;
        }

        m_ptr = &m_list[c_ptr->m_idx];
        r_ptr = &r_info[m_ptr->r_idx];

        if ((r_ptr->flags3 & RF3_EVIL) &&
            !(m_ptr->mflag2 & MFLAG2_QUESTOR) &&
            !(r_ptr->flags1 & RF1_UNIQUE) &&
            !p_ptr->inside_arena && !quests_get_current() &&
            (r_ptr->level < randint1(p_ptr->lev+50)) &&
            !(m_ptr->mflag2 & MFLAG2_NOGENO))
        {
            /* Delete the monster, rather than killing it. */
            delete_monster_idx(c_ptr->m_idx);
            msg_print("The evil creature vanishes in a puff of sulfurous smoke!");
        }
        else
        {
            msg_print("Your invocation is ineffectual!");
            if (one_in_(13)) m_ptr->mflag2 |= MFLAG2_NOGENO;
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_banish_evil(void) { return cast_spell(banish_evil_spell); }

void battle_frenzy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Battle Frenzy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives another bonus to hit and HP, resistance to fear for a while. Hastes you. But decreases AC.");
        break;
    case SPELL_CAST:
    {
        int b_base = spell_power(25);
        int sp_base = spell_power(p_ptr->lev / 2);
        int sp_sides = 20 + p_ptr->lev / 2;

        set_shero(randint1(b_base) + b_base, FALSE);
        set_fast(randint1(sp_sides) + sp_base, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void berserk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Berserk");
        break;
    case SPELL_DESC:
        var_set_string(res, "Enter a berserk frenzy, gaining great combat bonuses, but losing the ability to think clearly.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel a controlled rage.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer feel a controlled rage.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can drive yourself into a berserk frenzy.");
        break;
    case SPELL_CAST:
    {
        if ((elemental_is_(ELEMENTAL_WATER)) && (p_ptr->shero))
        {
            msg_print("You are already in a berserk rage!");
            break;
        }
        msg_print("Raaagh! You feel like hitting something.");
        set_shero(10 + randint1(p_ptr->lev), FALSE);
        var_set_bool(res, TRUE);
        water_mana_action(FALSE, 50);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_berserk(void) { return cast_spell(berserk_spell); }

void bless_spell(int cmd, variant *res)
{
    int base = spell_power(12);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bless");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives bonus to hit and AC for a few turns.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_blessed(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void bless_weapon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bless Weapon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Blesses your current weapon.");
        break;
    case SPELL_CAST:
        var_set_bool(res, bless_weapon());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void brain_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Brain Smash");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gaze intently at a single foe, causing damage, confusion and stunning");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(12, spell_power(12), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(
            GF_BRAIN_SMASH,
            dir,
            spell_power(damroll(12, 12) + p_ptr->to_d_spell),
            0
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void breathe_disintegration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Disintegration");
        break;
    case SPELL_DESC:
        var_set_string(res, "A disintegration breath. Not even the dungeon walls can withstand its power!");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(MIN(p_ptr->chp / 6, 150))));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir_aux(&dir, TARGET_DISI)) return;

        stop_mouth();
        msg_print("You breathe disintegration.");
        fire_ball(GF_DISINTEGRATE, dir, 
            spell_power(MIN(p_ptr->chp / 6, 150)), 
            (p_ptr->lev > 40 ? -3 : -2));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void breathe_fire_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes Fire at your opponent.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to breathe fire.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the ability to breathe fire.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can breathe fire (dam lvl * 2).");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(2 * p_ptr->lev)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+1)/2);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("You breathe fire...");
            fire_ball(GF_FIRE, dir, spell_power(2 * p_ptr->lev), -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_breathe_fire_I(void) { return cast_spell(breathe_fire_I_spell); }

void breathe_fire_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes Fire at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->chp*2/5)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("You breathe fire...");
            fire_ball(GF_FIRE, dir, spell_power(p_ptr->chp*2/5), -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void building_up_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Building Up");
        break;
    case SPELL_DESC:
        var_set_string(res, "Increases your physical prowess.");
        break;
    case SPELL_CAST:
        set_tim_building_up(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
