#include "angband.h"

void satisfy_hunger_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Satisfy Hunger");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fills your belly with pure yumminess.");
        break;
    case SPELL_CAST:
        set_food(PY_FOOD_MAX - 1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_satisfy_hunger(void) { return cast_spell(satisfy_hunger_spell); }

void scare_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Scare Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to scare a nearby monster.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stop_mouth();
        /*
        msg_print("You make a horrible scream!";
        msg_print("You emit an eldritch howl!");
        */
        fear_monster(dir, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void scare_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Terrify");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to scare one or more monsters.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            fear_monster(dir, p_ptr->lev);
        }
        else
        {
            turn_monsters(p_ptr->lev);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void self_knowledge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Self Knowledge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.");
        break;
    case SPELL_CAST:
        self_knowledge();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sense_surroundings_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sense Surroundings");
        break;
    default:
        magic_mapping_spell(cmd, res);
        break;
    }
}

void shadow_shifting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shadow Shifting");
        break;
    case SPELL_DESC:
        var_set_string(res, "Recreates the current dungeon level after a short delay.");
        break;
    case SPELL_CAST:
        msg_print("You start walking around.");
        alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void shoot_arrow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shoot Arrow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires an arrow.");
        break;
    case SPELL_INFO:
    {
        int slot = equip_find_first(object_is_melee_weapon);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            var_set_string(res, info_damage(o_ptr->dd, o_ptr->ds, o_ptr->to_d));
        }
        else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        {
            /* XXX Remove this spell for Possessors.
            monster_race *r_ptr = &r_info[p_ptr->current_r_idx];
            int i;
            for (i = 0; i < 4; i++)
            {
                if (r_ptr->blows[i].method == RBM_SHOOT)
                {
                    var_set_string(res, info_damage(r_ptr->blows[i].effects[0].dd, r_ptr->blows[i].effects[0].ds, 0));
                    return;
                }
            }
            */
        }
        else
            var_set_string(res, info_damage(0, 0, 1));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = 1;
        int slot;
        
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You fire an arrow.");

        slot = equip_find_first(object_is_melee_weapon);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            dam = damroll(o_ptr->dd, o_ptr->ds)+ o_ptr->to_d;
            if (dam < 1) dam = 1;
        }
        else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        {
            /* XXX Remove me! */
        }

        fire_bolt(GF_ARROW, dir, spell_power(dam));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void shriek_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shriek");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a large sound ball centered on you.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("Your vocal cords get much tougher.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("Your vocal cords get much weaker.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can emit a horrible shriek.");
        break;
    case SPELL_CAST:
        stop_mouth();
        fire_ball(GF_SOUND, 0, spell_power(2 * p_ptr->lev), 8);
        aggravate_monsters(0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_shriek(void) { return cast_spell(shriek_spell); }

void sleeping_dust_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sleeping Dust");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        msg_print("You throw some magic dust...");
        if (p_ptr->lev < 25) sleep_monsters_touch();
        else sleep_monsters(p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sleep_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sleep");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to sleep one or more monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            sleep_monster(dir, p_ptr->lev*2);
        }
        else
            sleep_monsters(p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void slow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Slow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to slow one or more monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            slow_monster(dir, p_ptr->lev * 2);
        }
        else
            slow_monsters(p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void smell_metal_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smell Metal");
        break;
    case SPELL_DESC:
        var_set_string(res, "Smells nearby metallic odors.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You smell a metallic odor.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer smell a metallic odor.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can smell nearby precious metal.");
        break;
    case SPELL_CAST:
        stop_mouth();
        detect_treasure(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void smell_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smell Monsters");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You smell filthy monsters.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer smell filthy monsters.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can smell nearby monsters.");
        break;
    case SPELL_CAST:
        stop_mouth();
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sp_to_hp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Convert SP to HP");
        break;
    case SPELL_DESC:
        var_set_string(res, "Converts SP into HP");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You are subject to fits of magical healing.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer subject to fits of magical healing.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "Your blood sometimes rushes to your muscles.");
        break;
    case SPELL_PROCESS:
        if (one_in_(2000))
        {
            int wounds = p_ptr->mhp - p_ptr->chp;

            if (wounds > 0)
            {
                int healing = p_ptr->csp;

                if (healing > wounds)
                    healing = wounds;

                hp_player(healing);
                p_ptr->csp -= healing;

                p_ptr->redraw |= (PR_MANA);
            }
        }
        break;
    case SPELL_CAST:
        if (p_ptr->csp >= p_ptr->lev / 5)
        {
            p_ptr->csp -= p_ptr->lev / 5;
            p_ptr->redraw |= PR_MANA;
            hp_player(p_ptr->lev);
        }
        else
            msg_print("You failed to convert.");

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void spit_acid_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spit Acid");
        break;
    case SPELL_DESC:
        if (p_ptr->lev < 25)
            var_set_string(res, "Spits a bolt of acid.");
        else
            var_set_string(res, "Spits a ball of acid.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to spit acid.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the ability to spit acid.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can spit acid (dam lvl*2).");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev/5);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("You spit acid...");
            if (p_ptr->lev < 25) fire_bolt(GF_ACID, dir, spell_power(p_ptr->lev * 2));
            else fire_ball(GF_ACID, dir, spell_power(p_ptr->lev * 2), 2);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_spit_acid(void) { return cast_spell(spit_acid_spell); }

static int _starburst_I_dam(void)
{
    if (p_ptr->pclass == CLASS_WILD_TALENT) /* Wild-Talents gain both I and II versions ... */
        return 100 + py_prorata_level_aux(100, 1, 1, 0);
    return 100 + py_prorata_level_aux(200, 1, 1, 2);
}

void starburst_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Star Burst");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of powerful light.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_starburst_I_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You invoke a starburst.");
        fire_ball(GF_LITE, dir, spell_power(_starburst_I_dam() + p_ptr->to_d_spell), spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _starburst_II_dam(void)
{
    return py_prorata_level_aux(450, 1, 0, 2);
}

void starburst_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Star Burst");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of powerful light.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_starburst_II_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You invoke a starburst.");
        fire_ball(GF_LITE, dir, 
            spell_power(_starburst_II_dam() + p_ptr->to_d_spell),
            spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void sterility_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sterility");
        break;
    case SPELL_DESC:
        var_set_string(res, "Stops breeding monsters from ... umm ... doing the nasty.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You can give everything around you a headache.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You hear a massed sigh of relief.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can cause mass impotence.");
        break;
    case SPELL_CAST:
        msg_print("You suddenly have a headache!");
        take_hit(DAMAGE_LOSELIFE, randint1(17) + 17, "the strain of forcing abstinence");

        /* Fake a population explosion. */
        num_repro += MAX_REPRO;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_sterility(void) { return cast_spell(sterility_spell); }

void stinking_cloud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stinking Cloud");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of poison.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(10 + p_ptr->lev / 2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_POIS, dir, spell_power(10 + p_ptr->lev / 2 + p_ptr->to_d_spell), spell_power(2));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void stone_skin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stone Skin");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        set_shield(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_stone_skin(void) { return cast_spell(stone_skin_spell); }

void stone_to_mud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stone to Mud");
        break;
    case SPELL_DESC:
        var_set_string(res, "Turns one rock square to mud.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        wall_to_mud(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_stone_to_mud(void) { return cast_spell(stone_to_mud_spell); }

void stop_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "The World");
        break;
    case SPELL_DESC:
        var_set_string(res, "Spend all of your spell points to stop time. You gain a number of free moves depending on the amount of spell points spent.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("%d acts.", MIN((p_ptr->csp + 100-p_ptr->energy_need - 50)/100, 5)));
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (world_player)
        {
            msg_print("Time is already stopped.");
            return;
        }

        world_player = TRUE;
        msg_print("You yell 'Time!'");
        msg_print(NULL);

        /* Note: We pay the casting cost up front these days. So, add back the 150
           to figure the starting sp, and then bash sp down to 0. We can't use the 
           SPELL_COST_EXTRA mechanism here ... */
        p_ptr->energy_need -= 1000 + (100 + (p_ptr->csp + 150) - 50)*TURNS_PER_TICK/10;
        p_ptr->energy_need = MAX(-1550, p_ptr->energy_need);

        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        p_ptr->redraw |= (PR_MAP | PR_STATUS);
        p_ptr->update |= (PU_MONSTERS);
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        handle_stuff();

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_amberites_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Amberites");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon Amberites for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        msg_print("You summon a Lord of Amber!");
        if (!summon_specific(-1, py, px, l, SUMMON_AMBERITE, PM_FORCE_PET | PM_ALLOW_UNIQUE))
            msg_print("No Amberites arrive.");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_angel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Angel");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a single angel for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_ANGEL, PM_FORCE_PET);
        if (!ct)
            msg_print("No angel arrives.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_ants_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ants");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon ants for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_ANT, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No ants arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_cyberdemon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Cyberdemon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a single cyberdemon for assistance.");
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_CYBER, PM_FORCE_PET);
        if (!ct)
            msg_print("No cyberdemon arrives.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a single demon for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(spell_power(p_ptr->lev*2/3), spell_power(p_ptr->lev/2)));
        break;
    case SPELL_CAST:
    {
        bool pet = !one_in_(3);
        u32b mode = 0L;

        if (pet) mode |= PM_FORCE_PET;
        else mode |= PM_NO_PET;
        if (!(pet && (p_ptr->lev < 50))) mode |= PM_ALLOW_GROUP;

        if (summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(p_ptr->lev*2/3+randint1(p_ptr->lev/2)), SUMMON_DEMON, mode))
        {
            msg_print("The area fills with a stench of sulphur and brimstone.");
            if (pet)
                msg_print("'What is thy bidding... Master?'");
            else
                msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
        }
        else
            msg_print("No demons arrive.");
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_demon_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a single demon for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_DEMON, PM_FORCE_PET);
        if (ct)
            msg_print("The area fills with a stench of sulphur and brimstone.");
        else
            msg_print("No demon arrives.");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Dragon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon a single dragon for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev * 2 / 3);

        ct += summon_specific(-1, py, px, l, SUMMON_DRAGON, PM_FORCE_PET);
        if (!ct)
            msg_print("No dragon arrives.");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_greater_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Greater Demon");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summons greater demon. You need to sacrifice a corpse of a human ('p','h' or 't') and the more powerful the corpse, the more powerful the demon you will conjure.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("lvl %d+sac", p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
        var_set_bool(res, cast_summon_greater_demon());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hi_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ancient Dragons");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon one or more ancient dragons for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;

        if (p_ptr->dragon_realm == DRAGON_REALM_DOMINATION)
            num = 2 + randint1(3);

        for (i = 0; i < num; i++)
        {
            int l = p_ptr->lev + randint1(p_ptr->lev * 2 / 3);
            ct += summon_specific(-1, py, px, l, SUMMON_HI_DRAGON, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("No dragons arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hi_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Greater Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon greater undead for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HI_UNDEAD, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("No undead arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Hounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon hounds for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HOUND, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No hounds arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hydras_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Hydras");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon hydras for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HYDRA, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No hydras arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_kin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Kin");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon related monsters for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, 0));
        break;
    case SPELL_CAST:
        if (!summon_kin_player(p_ptr->lev, py, px, PM_FORCE_PET | PM_ALLOW_GROUP))
            msg_print("No help arrives.");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_manes_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Manes");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon some demonic friends.");
        break;
    case SPELL_CAST:
        if (!summon_specific(-1, py, px, (p_ptr->lev * 3) / 2, SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            msg_print("No Manes arrive.");

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon a monster for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        if (!summon_specific(-1, py, px, l, 0, PM_FORCE_PET | PM_ALLOW_GROUP))
            msg_print("No monsters arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Monsters");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon monsters for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, 0, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No monsters arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_spiders_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Spiders");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon spiders for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_SPIDER, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No spiders arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_tree_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= 45)
            var_set_string(res, "Summon Trees");
        else
            var_set_string(res, "Summon Tree");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "Summon Tree");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "Attempts to summon many trees");
        else
            var_set_string(res, "Attempts to summon a tree.");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "Attempts to summon a tree. At L45, attempts to surround the player with trees.");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 45)
        {
            tree_creation();
            var_set_bool(res, TRUE);
        }    
        else
        {
            int attempts = 0;
            int x, y, dir;

            var_set_bool(res, TRUE);
            for (;;)
            {
                if (attempts > 4)
                {
                    msg_print("No trees arrive.");
                    break;
                }

                dir = randint0(9);
                if (dir == 5) continue;

                attempts++;
                y = py + ddy[dir];
                x = px + ddx[dir];

                if (!in_bounds(y, x)) continue;
                if (!cave_naked_bold(y, x)) continue;
                if (player_bold(y, x)) continue;

                cave_set_feat(y, x, feat_tree);
                break;
            }
        }
        break;
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 45)
            n += 30;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_summon_tree(void) { return cast_spell(summon_tree_spell); }

void summon_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon undead for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_UNDEAD, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("No undead arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_uniques_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Uniques");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to summon unique monsters for assistance.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        msg_print("You summon a special opponent!");
        if (!summon_specific(-1, py, px, l, SUMMON_UNIQUE, PM_FORCE_PET | PM_ALLOW_UNIQUE))
            msg_print("Nobody arrives.");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void super_stealth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hide in Darkness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Grants the stealth of the Ninja!  You may hide in shadows and see in the dark. Your light radius is decreased by 3.");
        break;
    case SPELL_CAST:
        if (p_ptr->tim_superstealth)
        {
            msg_print("You are already moving in the shadows.");
            var_set_bool(res, FALSE);
        }
        else
        {
            set_tim_superstealth(spell_power(randint1(p_ptr->lev/2) + p_ptr->lev/2), FALSE);
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void swap_pos_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Swap Position");
        break;
    case SPELL_DESC:
        var_set_string(res, "Swap locations with a given monster.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel like walking a mile in someone else's shoes.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You feel like staying in your own shoes.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can switch locations with another being.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);

        project_length = -1;
        if (get_fire_dir(&dir))
        {
            teleport_swap(dir);
            var_set_bool(res, TRUE);
        }
        project_length = 0;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_swap_pos(void) { return cast_spell(swap_pos_spell); }

void sword_dance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sword Dancing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attacks adjacent monsters randomly.");
        break;
    case SPELL_CAST:
    {
        int y = 0, x = 0, i, dir = 0;
        cave_type *c_ptr;

        for (i = 0; i < 6; i++)
        {
            dir = randint0(8);
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            if (c_ptr->m_idx)
                py_attack(y, x, 0);
            else
                msg_print("You attack the empty air.");
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void telekinesis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Telekinesis");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to fetch a distant object.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the ability to move objects telekinetically.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the ability to move objects telekinetically.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You are telekinetic.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            fetch(dir, p_ptr->lev * 10, TRUE);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_telekinesis(void) { return cast_spell(telekinesis_spell); }

void telepathy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Telepathy");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives telepathy for a while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 30));
        break;
    case SPELL_CAST:
        set_tim_esp(randint1(25) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void teleport_other_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Teleport Other");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleports all monsters on the line away unless resisted.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_dist(spell_power(p_ptr->lev*2)));
        break;
    case SPELL_CAST:
    {
        int dir;
        int power = spell_power(p_ptr->lev*2);

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_AWAY_ALL, dir, power);
            
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void teleport_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Teleport");
        break;
    case SPELL_DESC:
        var_set_string(res, "Escape to a distant location.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You gain the power of teleportation at will.");
        mut_lose(MUT_TELEPORT_RND);
        break;
    case SPELL_LOSE_MUT:
        msg_print("You lose the power of teleportation at will.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can teleport at will.");
        break;
    case SPELL_CAST:
        teleport_player(10 + 4 * p_ptr->lev, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, 30);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_teleport(void) { return cast_spell(teleport_spell); }

void teleport_level_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Teleport Level");
        break;
    case SPELL_DESC:
        var_set_string(res, "Escape to another level.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!py_teleport_level("Are you sure? (Teleport Level) ")) return;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_teleport_level(void) { return cast_spell(teleport_level_spell); }

void teleport_to_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Teleport To");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleport a visible monster next to you.");
        break;
    case SPELL_CAST:
    {
        monster_type *m_ptr;
        char m_name[80];

        if (!target_set(TARGET_KILL)) break;
        if (!cave[target_row][target_col].m_idx) break;
        if (!player_has_los_bold(target_row, target_col)) break;
        if (!projectable(py, px, target_row, target_col)) break;
        if (cave[target_row][target_col].m_idx == p_ptr->riding) break;

        var_set_bool(res, TRUE);

        m_ptr = &m_list[cave[target_row][target_col].m_idx];
        monster_desc(m_name, m_ptr, 0);
        if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
        msg_format("You command %s to return.", m_name);
        teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _boulder_dam(void)
{
    return py_prorata_level_aux(250, 2, 1, 2);
}
void throw_boulder_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Throw Boulder");
        break;
    case SPELL_DESC:
        var_set_string(res, "Hurls a huge boulder at chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _boulder_dam()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You throw a huge boulder.");
        fire_bolt(GF_ROCK, dir, _boulder_dam());
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, (_boulder_dam() + 6)/7);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void touch_of_confusion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Touch of Confusion");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to confuse the next monster that you hit.");
        break;
    case SPELL_CAST:
        if (!(p_ptr->special_attack & ATTACK_CONFUSE))
        {
            msg_print("Your hands start glowing.");
            p_ptr->special_attack |= ATTACK_CONFUSE;
            p_ptr->redraw |= (PR_STATUS);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void turn_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Turn Undead");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to scare undead monsters in sight.");
        break;
    case SPELL_CAST:
        if (project_hack(GF_TURN_UNDEAD, spell_power(p_ptr->lev)))
            virtue_add(VIRTUE_UNLIFE, -1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vampiric Drain");
        break;
    case SPELL_DESC:
        var_set_string(res, "Suck blood from an adjacent monster, gaining hp in the process.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("You become vampiric.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You are no longer vampiric.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can drain life from a foe like a vampire.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if ((d_info[dungeon_type].flags1 & DF1_NO_MELEE) || (no_melee_challenge))
        {
            msg_print("Something prevents you from attacking.");
            return;
        }
        else
        {
            int x, y, dummy;
            cave_type *c_ptr;
            int dir = 0;

            /* Only works on adjacent monsters */
            if (!get_rep_dir2(&dir)) break;

            var_set_bool(res, TRUE);

            y = py + ddy[dir];
            x = px + ddx[dir];
            c_ptr = &cave[y][x];

            stop_mouth();

            if (!(c_ptr->m_idx))
            {
                msg_print("You bite into thin air!");
                break;
            }

            msg_print("You grin and bare your fangs...");
            dummy = spell_power(p_ptr->lev * 2);

            if (drain_life(dir, dummy))
            {
                /* No heal if we are "full" */
                if (p_ptr->food < PY_FOOD_FULL)
                    vamp_player(dummy);
                else
                    msg_print("You were not hungry.");

                /* Gain nutritional sustenance: 150/hp drained
                 * A Food ration gives 5000 food points (by contrast)
                 * Don't ever get more than "Full" this way
                 * But if we ARE Gorged,  it won't cure us 
                 */
                dummy = p_ptr->food + MIN(5000, 100 * dummy);
                if (p_ptr->food < PY_FOOD_MAX)   /* Not gorged already */
                    set_food(dummy >= PY_FOOD_MAX ? PY_FOOD_MAX-1 : dummy);
            }
            else
                msg_print("Yechh. That tastes foul.");
        }
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev / 3);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_vampirism(void) { return cast_spell(vampirism_spell); }

void water_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of water.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev*4 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("You gesture fluidly.");
        fire_ball(GF_WATER, dir, spell_power(50 + p_ptr->lev*4 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void water_bolt_spell(int cmd, variant *res)
{
    int dd = 7 + p_ptr->lev / 4;
    int ds = 15;

    if (elemental_is_(ELEMENTAL_WATER)) dd = (dd - 3 + water_flow_rate() / 10);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of water.");
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
        fire_bolt(GF_WATER, dir, dam);
        water_mana_action(2, dam / 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void weigh_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Weigh Magic");
        break;
    case SPELL_DESC:
        var_set_string(res, "Determine the strength of magics affecting you.");
        break;
    case SPELL_GAIN_MUT:
        msg_print("You feel you can better understand the magic around you.");
        break;
    case SPELL_LOSE_MUT:
        msg_print("You no longer sense magic.");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "You can feel the strength of the magics affecting you.");
        break;
    case SPELL_CAST:
        report_magics();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_weigh_magic(void) { return cast_spell(weigh_magic_spell); }

void wonder_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wonder");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires something with random effects.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        cast_wonder(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void wraithform_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wraithform");
        break;
    case SPELL_DESC:
        var_set_string(res, "Leave the world of the living and travel the shadows of the underwold. You gain passwall and great resistance to damage.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(p_ptr->lev/2), spell_power(p_ptr->lev/2)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(p_ptr->lev / 2);
        set_wraith_form(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
