/* File: spells2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Spell code (part 2) */

#include "angband.h"
#include "grid.h"


/*
 * self-knowledge... idea from nethack. Useful for determining powers and
 * resistences of items. It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time. (There are a LOT of attributes to
 * list. It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge(void)
{
    int i = 0, j, k;

    int v_nr = 0;
    char v_string [8] [128];
    char s_string [6] [128];
    char r_string [RES_MAX] [128];

    u32b flgs[OF_ARRAY_SIZE];

    object_type *o_ptr;

    char Dummy[80];
    char buf[2][80];

    cptr info[220];

    for (j = 0; j < OF_ARRAY_SIZE; j++)
        flgs[j] = 0L;

    p_ptr->knowledge |= (KNOW_STAT | KNOW_HPRATE);

    strcpy(Dummy, "");
    sprintf(Dummy, "Your current Life Rating is %d/100.", life_rating());

    strcpy(buf[0], Dummy);
    info[i++] = buf[0];
    info[i++] = "";

    virtue_add(VIRTUE_KNOWLEDGE, 1);
    virtue_add(VIRTUE_ENLIGHTENMENT, 1);

    /* Acquire item flags from equipment */
    for (k = 1; k <= equip_max(); k++)
    {
        u32b tflgs[OF_ARRAY_SIZE];

        o_ptr = equip_obj(k);
        if (!o_ptr) continue;
        if (o_ptr->marked & OM_SLIPPING) continue;

        /* Extract the flags */
        obj_flags(o_ptr, tflgs);

        /* Extract flags */
        for (j = 0; j < OF_ARRAY_SIZE; j++)
            flgs[j] |= tflgs[j];
    }

    info[i++] = "Limits of maximum stats";

    for (v_nr = 0; v_nr < 6; v_nr++)
    {
        char stat_desc[80];

        if (decimal_stats) sprintf(stat_desc, "%s %d", stat_names[v_nr], (p_ptr->stat_max_max[v_nr]-18)/10+18);
        else sprintf(stat_desc, "%s 18/%d", stat_names[v_nr], p_ptr->stat_max_max[v_nr]-18);

        strcpy(s_string[v_nr], stat_desc);

        info[i++] = s_string[v_nr];
    }
    info[i++] = "";

    if (enable_virtues)
    {
        sprintf(Dummy, "Your alignment : %s (%d)", your_alignment(), p_ptr->align);
        strcpy(buf[1], Dummy);
        info[i++] = buf[1];
        for (v_nr = 0; v_nr < 8; v_nr++)
        {
            char v_name [20];
            char vir_desc[80];
            int tester = p_ptr->virtues[v_nr];

            strcpy(v_name, virtue_name(p_ptr->vir_types[v_nr]));

            sprintf(vir_desc, "Oops. No info about %s.", v_name);
            if (tester < -100)
                sprintf(vir_desc, "You are the polar opposite of %s (%d).", v_name, tester);
            else if (tester < -80)
                sprintf(vir_desc, "You are an arch-enemy of %s (%d).", v_name, tester);
            else if (tester < -60)
                sprintf(vir_desc, "You are a bitter enemy of %s (%d).", v_name, tester);
            else if (tester < -40)
                sprintf(vir_desc, "You are an enemy of %s (%d).", v_name, tester);
            else if (tester < -20)
                sprintf(vir_desc, "You have sinned against %s (%d).", v_name, tester);
            else if (tester < 0)
                sprintf(vir_desc, "You have strayed from the path of %s (%d).", v_name, tester);
            else if (tester == 0)
                sprintf(vir_desc,"You are neutral to %s (%d).", v_name, tester);
            else if (tester < 20)
                sprintf(vir_desc,"You are somewhat virtuous in %s (%d).", v_name, tester);
            else if (tester < 40)
                sprintf(vir_desc,"You are virtuous in %s (%d).", v_name, tester);
            else if (tester < 60)
                sprintf(vir_desc,"You are very virtuous in %s (%d).", v_name, tester);
            else if (tester < 80)
                sprintf(vir_desc,"You are a champion of %s (%d).",  v_name, tester);
            else if (tester < 100)
                sprintf(vir_desc,"You are a great champion of %s (%d).", v_name, tester);
            else
                sprintf(vir_desc,"You are the living embodiment of %s (%d).", v_name, tester);

            strcpy(v_string[v_nr], vir_desc);

            info[i++] = v_string[v_nr];
        }
        info[i++] = "";
    }

    if (p_ptr->blind)
    {
        info[i++] = "You cannot see.";
    }
    if (p_ptr->confused)
    {
        info[i++] = "You are confused.";
    }
    if (p_ptr->afraid)
    {
        info[i++] = "You are terrified.";
    }
    if (p_ptr->cut)
    {
        info[i++] = "You are bleeding.";
    }
    if (p_ptr->stun)
    {
        info[i++] = "You are stunned.";
    }
    if (p_ptr->poisoned)
    {
        info[i++] = "You are poisoned.";
    }
    if (p_ptr->image)
    {
        info[i++] = "You are hallucinating.";
    }
    if (p_ptr->cursed & OFC_TY_CURSE)
    {
        info[i++] = "You carry an ancient foul curse.";
    }
    if (p_ptr->cursed & OFC_BY_CURSE)
    {
        info[i++] = "You carry a baby foul curse.";
    }
    if (p_ptr->cursed & OFC_AGGRAVATE)
    {
        info[i++] = "You aggravate monsters.";
    }
    if (p_ptr->cursed & OFC_CATLIKE)
    {
        info[i++] = "You make a great deal of noise as you move.";
    }
    if (p_ptr->cursed & OFC_DANGER)
    {
        info[i++] = "You face great dangers at shallow levels.";
    }
    if (p_ptr->cursed & OFC_DRAIN_EXP)
    {
        info[i++] = "You are drained.";
    }
    if (p_ptr->cursed & OFC_SLOW_REGEN)
    {
        info[i++] = "You regenerate slowly.";
    }
    if (p_ptr->cursed & OFC_ADD_L_CURSE)
    {
        info[i++] = "Your weak curses multiply.";
    }
    if (p_ptr->cursed & OFC_ADD_H_CURSE)
    {
        info[i++] = "Your heavy curses multiply.";
    }
    if (p_ptr->cursed & OFC_CALL_ANIMAL)
    {
        info[i++] = "You attract animals.";
    }
    if (p_ptr->cursed & OFC_CALL_DEMON)
    {
        info[i++] = "You attract demons.";
    }
    if (p_ptr->cursed & OFC_CALL_DRAGON)
    {
        info[i++] = "You attract dragons.";
    }
    if (p_ptr->cursed & OFC_COWARDICE)
    {
        info[i++] = "You are subject to cowardice.";
    }
    if (p_ptr->cursed & OFC_TELEPORT)
    {
        info[i++] = "Your position is very uncertain.";
    }
    if (p_ptr->cursed & OFC_LOW_MELEE)
    {
        info[i++] = "Your weapon causes you to miss blows.";
    }
    if (p_ptr->cursed & OFC_LOW_AC)
    {
        info[i++] = "You are subject to being hit.";
    }
    if (p_ptr->cursed & OFC_LOW_MAGIC)
    {
        info[i++] = "You are a careless spellcaster.";
    }
    if (p_ptr->cursed & OFC_LOW_DEVICE)
    {
        info[i++] = "You are careless at using magic devices.";
    }
    if (p_ptr->cursed & OFC_FAST_DIGEST)
    {
        info[i++] = "You have a good appetite.";
    }
    if (p_ptr->cursed & OFC_ALLERGY)
    {
        info[i++] = "You are allergic to your equipment.";
    }
    if (p_ptr->cursed & OFC_CRAPPY_MUT)
    {
        info[i++] = "You occasionally suffer harmful mutations.";
    }
    if (p_ptr->cursed & OFC_NORMALITY)
    {
        info[i++] = "You are subject to bouts of normality.";
    }
    if (p_ptr->cursed & OFC_OPEN_WOUNDS)
    {
        info[i++] = "Your wounds heal very slowly.";
    }
    if (p_ptr->cursed & OFC_DRAIN_HP)
    {
        info[i++] = "Your body is drained.";
    }
    if (p_ptr->cursed & OFC_DRAIN_MANA)
    {
        info[i++] = "Your brain is drained.";
    }
    if (p_ptr->cursed & OFC_DRAIN_PACK)
    {
        info[i++] = "Your inventory is drained.";
    }
    if (IS_BLESSED())
    {
        info[i++] = "You feel righteous.";

    }
    if (IS_HERO())
    {
        info[i++] = "You feel heroic.";

    }
    if (IS_SHERO())
    {
        info[i++] = "You are in a battle rage.";

    }
    if (IS_PROT_EVIL())
    {
        info[i++] = "You are protected from evil.";

    }
    if (p_ptr->shield)
    {
        info[i++] = "You are protected by a mystic shield.";

    }
    if (IS_INVULN())
    {
        info[i++] = "You are temporarily invulnerable.";

    }
    if (IS_WRAITH())
    {
        info[i++] = "You are temporarily incorporeal.";

    }
    if (p_ptr->special_attack & ATTACK_CONFUSE)
    {
        info[i++] = "Your hands are glowing dull red.";

    }
    if (p_ptr->special_attack & ATTACK_FIRE)
    {
        info[i++] = "You can strike the enemy with flame.";

    }
    if (p_ptr->special_attack & ATTACK_COLD)
    {
        info[i++] = "You can strike the enemy with cold.";

    }
    if (p_ptr->special_attack & ATTACK_ACID)
    {
        info[i++] = "You can strike the enemy with acid.";

    }
    if (p_ptr->special_attack & ATTACK_ELEC)
    {
        info[i++] = "You can strike the enemy with electoric shock.";

    }
    if (p_ptr->special_attack & ATTACK_POIS)
    {
        info[i++] = "You can strike the enemy with poison.";

    }
    switch (p_ptr->action)
    {
        case ACTION_SEARCH:
            info[i++] = "You are looking around very carefully.";
            break;
    }
    if (p_ptr->new_spells)
    {
        info[i++] = "You can learn some spells/prayers.";

    }
    if (p_ptr->word_recall)
    {
        info[i++] = "You will soon be recalled.";

    }
    if (p_ptr->alter_reality)
    {
        info[i++] = "You will soon be altered.";

    }
    if (p_ptr->see_infra)
    {
        info[i++] = "Your eyes are sensitive to infrared light.";

    }
    if (p_ptr->see_inv)
    {
        info[i++] = "You can see invisible creatures.";

    }
    if (p_ptr->levitation)
    {
        info[i++] = "You can fly.";

    }
    if (p_ptr->free_act)
    {
        info[i++] = "You have free action.";

    }
    if (p_ptr->regen > 100)
    {
        info[i++] = "You regenerate quickly.";

    }
    if (p_ptr->slow_digest)
    {
        info[i++] = "Your appetite is small.";

    }
    if (p_ptr->telepathy)
    {
        info[i++] = "You have ESP.";

    }
    if (p_ptr->esp_animal)
    {
        info[i++] = "You sense natural creatures.";

    }
    if (p_ptr->esp_undead)
    {
        info[i++] = "You sense undead.";

    }
    if (p_ptr->esp_demon)
    {
        info[i++] = "You sense demons.";

    }
    if (p_ptr->esp_orc)
    {
        info[i++] = "You sense orcs.";

    }
    if (p_ptr->esp_troll)
    {
        info[i++] = "You sense trolls.";

    }
    if (p_ptr->esp_giant)
    {
        info[i++] = "You sense giants.";

    }
    if (p_ptr->esp_dragon)
    {
        info[i++] = "You sense dragons.";

    }
    if (p_ptr->esp_human)
    {
        info[i++] = "You sense humans.";

    }
    if (p_ptr->esp_evil)
    {
        info[i++] = "You sense evil creatures.";

    }
    if (p_ptr->esp_good)
    {
        info[i++] = "You sense good creatures.";

    }
    if (p_ptr->esp_nonliving)
    {
        info[i++] = "You sense non-living creatures.";

    }
    if (p_ptr->esp_unique)
    {
        info[i++] = "You sense unique monsters.";

    }
    if (p_ptr->hold_life)
    {
        info[i++] = "You have a firm hold on your life force.";

    }
    if (p_ptr->reflect)
    {
        info[i++] = "You reflect arrows and bolts.";

    }
    if (p_ptr->sh_fire)
        info[i++] = "You are surrounded with a fiery aura.";
    if (p_ptr->sh_shards)
        info[i++] = "You are surrounded with a shard aura.";
    if (p_ptr->tim_blood_revenge)
        info[i++] = "You are surrounded with a bloody aura.";
    if (p_ptr->sh_elec)
    {
        info[i++] = "You are surrounded with electricity.";

    }
    if (p_ptr->sh_cold)
    {
        info[i++] = "You are surrounded with an aura of coldness.";

    }
    if (p_ptr->tim_sh_holy)
    {
        info[i++] = "You are surrounded with a holy aura.";

    }
    if (p_ptr->tim_sh_touki)
    {
        info[i++] = "You are surrounded with a energy aura.";

    }
    if (p_ptr->anti_magic)
    {
        info[i++] = "You are surrounded by an anti-magic shell.";

    }
    if (p_ptr->anti_tele)
    {
        info[i++] = "You cannot teleport.";

    }
    if (p_ptr->lite)
    {
        info[i++] = "You are carrying a permanent light.";

    }
    if (p_ptr->warning)
    {
        info[i++] = "You will be warned before dangerous actions.";

    }
    if (p_ptr->dec_mana)
    {
        info[i++] = "You can cast spells with fewer mana points.";

    }
    if (p_ptr->easy_spell)
    {
        info[i++] = "Fail rate of your magic is decreased.";

    }
    if (p_ptr->heavy_spell)
    {
        info[i++] = "Fail rate of your magic is increased.";

    }
    if (p_ptr->mighty_throw)
    {
        info[i++] = "You can throw objects powerfully.";

    }

    for (j = 0; j < RES_MAX; j++)
    {
        int pct = res_pct(j);
        if (pct != 0)
        {
            if (j == RES_FEAR)
                sprintf(r_string[j], "You are %s to %s.", pct > 0 ? "resistant" : "vulnerable", res_name(j));
            else
                sprintf(r_string[j], "You are %d%% %s to %s.", ABS(pct), pct > 0 ? "resistant" : "vulnerable", res_name(j));
            info[i++] = r_string[j];
        }
    }
    if (p_ptr->sustain_str)
    {
        info[i++] = "Your strength is sustained.";

    }
    if (p_ptr->sustain_int)
    {
        info[i++] = "Your intelligence is sustained.";

    }
    if (p_ptr->sustain_wis)
    {
        info[i++] = "Your wisdom is sustained.";

    }
    if (p_ptr->sustain_con)
    {
        info[i++] = "Your constitution is sustained.";

    }
    if (p_ptr->sustain_dex)
    {
        info[i++] = "Your dexterity is sustained.";

    }
    if (p_ptr->sustain_chr)
    {
        info[i++] = "Your charisma is sustained.";

    }

    if (have_flag(flgs, OF_STR))
    {
        info[i++] = "Your strength is affected by your equipment.";

    }
    if (have_flag(flgs, OF_INT))
    {
        info[i++] = "Your intelligence is affected by your equipment.";

    }
    if (have_flag(flgs, OF_WIS))
    {
        info[i++] = "Your wisdom is affected by your equipment.";

    }
    if (have_flag(flgs, OF_DEX))
    {
        info[i++] = "Your dexterity is affected by your equipment.";

    }
    if (have_flag(flgs, OF_CON))
    {
        info[i++] = "Your constitution is affected by your equipment.";

    }
    if (have_flag(flgs, OF_CHR))
    {
        info[i++] = "Your charisma is affected by your equipment.";

    }

    if (have_flag(flgs, OF_STEALTH))
    {
        info[i++] = "Your stealth is affected by your equipment.";

    }
    if (have_flag(flgs, OF_SEARCH))
    {
        info[i++] = "Your searching ability is affected by your equipment.";

    }
    if (have_flag(flgs, OF_INFRA))
    {
        info[i++] = "Your infravision is affected by your equipment.";

    }
    if (have_flag(flgs, OF_TUNNEL))
    {
        info[i++] = "Your digging ability is affected by your equipment.";

    }
    if (have_flag(flgs, OF_SPEED))
    {
        info[i++] = "Your speed is affected by your equipment.";

    }
    if (have_flag(flgs, OF_BLOWS))
    {
        info[i++] = "Your attack speed is affected by your equipment.";

    }

    if (p_ptr->no_eldritch)
        info[i++] = "You are unaffected by the Eldritch Horror.";
    if (p_ptr->no_cut)
        info[i++] = "You cannot be cut.";
    if (p_ptr->no_stun)
        info[i++] = "You cannot be stunned.";
    if (p_ptr->no_charge_drain)
        info[i++] = "You are immune to charge draining attacks.";
    if (p_ptr->auto_id)
        info[i++] = "Objects are automatically identified as you pass over them.";
    else if (p_ptr->auto_pseudo_id)
        info[i++] = "Objects are automatically pseudo-identified as you pass over them.";
    if (p_ptr->cult_of_personality)
        info[i++] = "Summoned monsters sometimes switch their allegiance.";

    /* TODO: We used to spoil your first weapon, and ignore any alternate weapons. Rethink ... */

    /* Save the screen */
    screen_save();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

    /* Label the information */
    prt("     Your Attributes:", 1, 15);


    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++)
    {
        /* Show the info */
        prt(info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j+1 < i))
        {
            prt("-- more --", k, 15);

            inkey();
            for (; k > 2; k--) prt("", k, 15);
        }
    }

    /* Pause */
    prt("[Press any key to continue]", k, 13);

    inkey();

    /* Restore the screen */
    screen_load();
}


static int report_magics_aux(int dur)
{
    if (dur <= 5)
    {
        return 0;
    }
    else if (dur <= 10)
    {
        return 1;
    }
    else if (dur <= 20)
    {
        return 2;
    }
    else if (dur <= 50)
    {
        return 3;
    }
    else if (dur <= 100)
    {
        return 4;
    }
    else if (dur <= 200)
    {
        return 5;
    }
    else
    {
        return 6;
    }
}

static cptr report_magic_durations[] =
{
    "for a short time",
    "for a little while",
    "for a while",
    "for a long while",
    "for a long time",
    "for a very long time",
    "for an incredibly long time",
    "until you hit a monster"

};


/*
 * Report all currently active magical effects.
 */
void report_magics(void)
{
    int     i = 0, j, k;
    char    Dummy[80];
    cptr    info[128];
    int     info2[128];


    if (p_ptr->blind)
    {
        info2[i]  = report_magics_aux(p_ptr->blind);
        info[i++] = "You cannot see";

    }
    if (p_ptr->confused)
    {
        info2[i]  = report_magics_aux(p_ptr->confused);
        info[i++] = "You are confused";

    }
    if (p_ptr->afraid)
    {
        info2[i]  = report_magics_aux(p_ptr->afraid);
        info[i++] = "You are terrified";

    }
    if (p_ptr->poisoned)
    {
        info2[i]  = report_magics_aux(p_ptr->poisoned);
        info[i++] = "You are poisoned";

    }
    if (p_ptr->image)
    {
        info2[i]  = report_magics_aux(p_ptr->image);
        info[i++] = "You are hallucinating";

    }
    if (p_ptr->blessed)
    {
        info2[i]  = report_magics_aux(p_ptr->blessed);
        info[i++] = "You feel rightous";

    }
    if (p_ptr->hero)
    {
        info2[i]  = report_magics_aux(p_ptr->hero);
        info[i++] = "You feel heroic";

    }
    if (p_ptr->shero)
    {
        info2[i]  = report_magics_aux(p_ptr->shero);
        info[i++] = "You are in a battle rage";

    }
    if (p_ptr->protevil)
    {
        info2[i]  = report_magics_aux(p_ptr->protevil);
        info[i++] = "You are protected from evil";

    }
    if (p_ptr->shield)
    {
        info2[i]  = report_magics_aux(p_ptr->shield);
        info[i++] = "You are protected by a mystic shield";

    }
    if (p_ptr->invuln)
    {
        info2[i]  = report_magics_aux(p_ptr->invuln);
        info[i++] = "You are invulnerable";

    }
    if (p_ptr->wraith_form)
    {
        info2[i]  = report_magics_aux(p_ptr->wraith_form);
        info[i++] = "You are incorporeal";

    }
    if (p_ptr->special_attack & ATTACK_CONFUSE)
    {
        info2[i]  = 7;
        info[i++] = "Your hands are glowing dull red.";

    }
    if (p_ptr->word_recall)
    {
        info2[i]  = report_magics_aux(p_ptr->word_recall);
        info[i++] = "You are waiting to be recalled";

    }
    if (p_ptr->alter_reality)
    {
        info2[i]  = report_magics_aux(p_ptr->alter_reality);
        info[i++] = "You waiting to be altered";

    }
    if (p_ptr->oppose_acid)
    {
        info2[i]  = report_magics_aux(p_ptr->oppose_acid);
        info[i++] = "You are resistant to acid";

    }
    if (p_ptr->oppose_elec)
    {
        info2[i]  = report_magics_aux(p_ptr->oppose_elec);
        info[i++] = "You are resistant to lightning";

    }
    if (p_ptr->oppose_fire)
    {
        info2[i]  = report_magics_aux(p_ptr->oppose_fire);
        info[i++] = "You are resistant to fire";

    }
    if (p_ptr->oppose_cold)
    {
        info2[i]  = report_magics_aux(p_ptr->oppose_cold);
        info[i++] = "You are resistant to cold";

    }
    if (p_ptr->oppose_pois)
    {
        info2[i]  = report_magics_aux(p_ptr->oppose_pois);
        info[i++] = "You are resistant to poison";

    }

    /* Save the screen */
    screen_save();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

    /* Label the information */
    prt("     Your Current Magic:", 1, 15);


    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++)
    {
        /* Show the info */
        sprintf(Dummy, "%s %s.", info[j],

            report_magic_durations[info2[j]]);
        prt(Dummy, k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j + 1 < i))
        {
            prt("-- more --", k, 15);

            inkey();
            for (; k > 2; k--) prt("", k, 15);
        }
    }

    /* Pause */
    prt("[Press any key to continue]", k, 13);

    inkey();

    /* Restore the screen */
    screen_load();
}


static bool detect_feat_flag(int range, int flag, bool known)
{
    int       x, y;
    bool      detect = FALSE;
    cave_type *c_ptr;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan the current panel */
    for (y = 1; y < cur_hgt - 1; y++)
    {
        for (x = 1; x <= cur_wid - 1; x++)
        {
            int dist = distance(py, px, y, x);
            if (dist > range) continue;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Hack -- Safe */
            if (flag == FF_TRAP)
            {
                /* Mark as detected */
                if (dist <= range && known)
                {
                    if (dist <= range - 1)
                        c_ptr->info |= CAVE_IN_DETECT;

                    c_ptr->info &= ~CAVE_UNSAFE;

                    /* Redraw */
                    lite_spot(y, x);
                }
            }

            /* Detect flags */
            if (cave_have_flag_grid(c_ptr, flag))
            {
                /* Detect secrets */
                disclose_grid(y, x);

                /* Hack -- Memorize */
                c_ptr->info |= (CAVE_MARK | CAVE_AWARE);

                /* Redraw */
                lite_spot(y, x);

                /* Obvious */
                detect = TRUE;
            }
        }
    }

    if (flag == FF_TRAP && !view_unsafe_grids)
        p_ptr->redraw |= PR_STATUS;

    /* Result */
    return detect;
}


/*
 * Detect all traps on current panel
 */
bool detect_traps(int range, bool known)
{
    bool detect = detect_feat_flag(range, FF_TRAP, known);

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 0) detect = FALSE;

    /* Avoid spoiling, but mark grids as safe if the effect is obvious */ 
    if (detect && !known) detect_feat_flag(range, FF_TRAP, TRUE);

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of traps!");
    }

    /* Result */
    return detect;
}


/*
 * Detect all doors on current panel
 */
bool detect_doors(int range)
{
    bool detect = detect_feat_flag(range, FF_DOOR, TRUE);

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 0) detect = FALSE;

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of doors!");
    }

    /* Result */
    return detect;
}


/*
 * Detect all stairs on current panel
 */
bool detect_stairs(int range)
{
    bool detect = detect_feat_flag(range, FF_STAIRS, TRUE);

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 0) detect = FALSE;

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of stairs!");
    }

    /* Result */
    return detect;
}


/*
 * Detect any treasure on the current panel
 */
bool detect_treasure(int range)
{
    bool detect = detect_feat_flag(range, FF_HAS_GOLD, TRUE);

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 6) detect = FALSE;

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of buried treasure!");
    }

    /* Result */
    return detect;
}


/*
 * Detect all "gold" objects on the current panel
 */
bool detect_objects_gold(int range)
{
    int i, y, x;
    int range2 = range;

    bool detect = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range2 /= 3;

    /* Scan objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Skip held objects */
        if (o_ptr->held_m_idx) continue;

        /* Location */
        y = o_ptr->loc.y;
        x = o_ptr->loc.x;

        /* Only detect nearby objects */
        if (distance(py, px, y, x) > range2) continue;

        /* Detect "gold" objects */
        if (o_ptr->tval == TV_GOLD)
        {
            /* Hack -- memorize it */
            o_ptr->marked |= OM_FOUND;

            /* Redraw */
            lite_spot(y, x);

            /* Detect */
            detect = TRUE;
        }
    }

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 6) detect = FALSE;

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of treasure!");

    }

    if (detect_monsters_string(range, "$"))
    {
        detect = TRUE;
    }

    /* Result */
    return (detect);
}


/*
 * Detect all "normal" objects on the current panel
 */
bool detect_objects_normal(int range)
{
    int i, y, x;
    int range2 = range;

    bool detect = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range2 /= 3;

    /* Scan objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Skip held objects */
        if (o_ptr->held_m_idx) continue;

        /* Location */
        y = o_ptr->loc.y;
        x = o_ptr->loc.x;

        /* Only detect nearby objects */
        if (distance(py, px, y, x) > range2) continue;

        /* Detect "real" objects */
        if (o_ptr->tval != TV_GOLD)
        {
            /* Hack -- memorize it */
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;

            /* Redraw */
            lite_spot(y, x);

            /* Detect */
            detect = TRUE;
        }
    }

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 6) detect = FALSE;

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of objects!");

    }

    if (detect_monsters_string(range, "!=?|/`"))
    {
        detect = TRUE;
    }

    /* Result */
    return (detect);
}


/*
 * Detect all "magic" objects on the current panel.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 */
bool detect_objects_magic(int range)
{
    int i, y, x, tv;

    bool detect = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan all objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Skip held objects */
        if (o_ptr->held_m_idx) continue;

        /* Location */
        y = o_ptr->loc.y;
        x = o_ptr->loc.x;

        /* Only detect nearby objects */
        if (distance(py, px, y, x) > range) continue;

        /* Examine the tval */
        tv = o_ptr->tval;

        /* Artifacts, misc magic items, or enchanted wearables */
        if (object_is_artifact(o_ptr) ||
            object_is_ego(o_ptr) ||
            (tv == TV_WHISTLE) ||
            (tv == TV_AMULET) ||
            (tv == TV_RING) ||
            (tv == TV_STAFF) ||
            (tv == TV_WAND) ||
            (tv == TV_ROD) ||
            (tv == TV_SCROLL) ||
            (tv == TV_POTION) ||
            (tv == TV_LIFE_BOOK) ||
            (tv == TV_SORCERY_BOOK) ||
            (tv == TV_NATURE_BOOK) ||
            (tv == TV_CHAOS_BOOK) ||
            (tv == TV_DEATH_BOOK) ||
            (tv == TV_TRUMP_BOOK) ||
            (tv == TV_ARCANE_BOOK) ||
            (tv == TV_CRAFT_BOOK) ||
            (tv == TV_DAEMON_BOOK) ||
            (tv == TV_CRUSADE_BOOK) ||
            (tv == TV_NECROMANCY_BOOK) ||
            (tv == TV_ARMAGEDDON_BOOK) ||
            (tv == TV_MUSIC_BOOK) ||
            (tv == TV_HISSATSU_BOOK) ||
            (tv == TV_HEX_BOOK) ||
            ((o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0)))
        {
            /* Memorize the item */
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;

            /* Redraw */
            lite_spot(y, x);

            /* Detect */
            detect = TRUE;
        }
    }

    /* Describe */
    if (detect)
    {
        msg_print("You sense the presence of magic objects!");

    }

    /* Return result */
    return (detect);
}


/*
 * Detect all "normal" monsters on the current panel
 */
bool detect_monsters_normal(int range)
{
    int i, y, x;

    bool flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect all non-invisible monsters
         * N.B. update_mon() will roll the perception check for Invisibility */
        if (!(r_ptr->flags2 & RF2_INVISIBLE) || p_ptr->see_inv)
        {
            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 3) flag = FALSE;

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of monsters!");

    }

    /* Result */
    return (flag);
}


/*
 * Detect all "invisible" monsters around the player
 */
bool detect_monsters_invis(int range)
{
    int i, y, x;
    bool flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect invisible monsters */
        if (r_ptr->flags2 & RF2_INVISIBLE)
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
            {
                /* Window stuff */
                p_ptr->window |= (PW_MONSTER);
            }

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 3) flag = FALSE;

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of invisible creatures!");

    }

    /* Result */
    return (flag);
}



/*
 * Detect all "evil" monsters on current panel
 */
bool detect_monsters_evil(int range)
{
    int i, y, x;
    bool flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect evil monsters */
        if (r_ptr->flags3 & RF3_EVIL)
        {
            if (is_original_ap(m_ptr))
                mon_lore_aux_3(r_ptr, RF3_EVIL);

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of evil creatures!");

    }

    /* Result */
    return (flag);
}




/*
 * Detect all "nonliving", "undead" or "demonic" monsters on current panel
 */
bool detect_monsters_nonliving(int range)
{
    int     i, y, x;
    bool    flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect non-living monsters */
        if (!monster_living(r_ptr))
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
            {
                /* Window stuff */
                p_ptr->window |= (PW_MONSTER);
            }

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of unnatural beings!");

    }

    /* Result */
    return (flag);
}

/*
 * Detect all "living" monsters on current panel
 */
bool detect_monsters_living(int range, cptr msg)
{
    int     i, y, x;
    bool    flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        if (!m_ptr->r_idx) continue;

        y = m_ptr->fy;
        x = m_ptr->fx;

        if (distance(py, px, y, x) > range) continue;

        if (monster_living(r_ptr))
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
                p_ptr->window |= (PW_MONSTER);

            repair_monsters = TRUE;
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);
            update_mon(i, FALSE);
            flag = TRUE;
        }
    }

    if (flag && msg)
        msg_print(msg);

    return flag;
}

bool detect_monsters_magical(int range)
{
    int     i, y, x;
    bool    flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect non-living monsters */
        if (monster_magical(r_ptr))
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
            {
                /* Window stuff */
                p_ptr->window |= (PW_MONSTER);
            }

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    /* Describe */
    if (flag)
    {
        msg_print("You sense the presence of magical foes!");
    }

    /* Result */
    return (flag);
}

/*
 * Detect all monsters it has mind on current panel
 */
bool detect_monsters_mind(int range)
{
    int     i, y, x;
    bool    flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect non-living monsters */
        if (!(r_ptr->flags2 & RF2_EMPTY_MIND))
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
            {
                /* Window stuff */
                p_ptr->window |= (PW_MONSTER);
            }

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of someone's mind!");

    }

    /* Result */
    return (flag);
}


/*
 * Detect all (string) monsters on current panel
 */
bool detect_monsters_string(int range, cptr Match)
{
    int i, y, x;
    bool flag = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect monsters with the same symbol */
        if (my_strchr(Match, r_ptr->d_char))
        {
            /* Update monster recall window */
            if (p_ptr->monster_race_idx == m_ptr->r_idx)
            {
                /* Window stuff */
                p_ptr->window |= (PW_MONSTER);
            }

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    if (music_singing(MUSIC_DETECT) && p_ptr->magic_num1[2] > 3) flag = FALSE;

    /* Describe */
    if (flag)
    {
        /* Describe result */
        msg_print("You sense the presence of monsters!");

    }

    /* Result */
    return (flag);
}


/*
 * A "generic" detect monsters routine, tagged to flags3
 */
bool detect_monsters_xxx(int range, u32b match_flag)
{
    int  i, y, x;
    bool flag = FALSE;
    cptr desc_monsters = "weird monsters";

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Only detect nearby monsters */
        if (distance(py, px, y, x) > range) continue;

        /* Detect evil monsters */
        if (r_ptr->flags3 & (match_flag))
        {
            if (is_original_ap(m_ptr))
                mon_lore_aux_3(r_ptr, match_flag);

            /* Repair visibility later */
            repair_monsters = TRUE;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);

            /* Detect */
            flag = TRUE;
        }
    }

    /* Describe */
    if (flag)
    {
        switch (match_flag)
        {
            case RF3_DEMON:
                desc_monsters = "demons";

                break;
            case RF3_UNDEAD:
                desc_monsters = "the undead";

                break;
        }

        /* Describe result */
        msg_format("You sense the presence of %s!", desc_monsters);

        msg_print(NULL);
    }

    /* Result */
    return (flag);
}


/*
 * Detect everything
 */
bool detect_all(int range)
{
    bool detect = FALSE;

    /* Detect everything */
    if (detect_traps(range, TRUE)) detect = TRUE;
    if (detect_doors(range)) detect = TRUE;
    if (detect_stairs(range)) detect = TRUE;

    /* There are too many hidden treasure. So... */
    /* if (detect_treasure(range)) detect = TRUE; */

    if (detect_objects_gold(range)) detect = TRUE;
    if (detect_objects_normal(range)) detect = TRUE;
    if (detect_monsters_invis(range)) detect = TRUE;
    if (detect_monsters_normal(range)) detect = TRUE;

    /* Result */
    return (detect);
}


/*
 * Apply a "project()" directly to all viewable monsters
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 *
 * To avoid misbehavior when monster deaths have side-effects,
 * this is done in two passes. -- JDL
 */
bool project_hack(int typ, int dam)
{
    int     i, x, y;
    int     flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;
    bool    obvious = FALSE;


    /* Mark all (nearby) monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Require line of sight */
        if (!player_has_los_bold(y, x) || !projectable(py, px, y, x)) continue;

        /* Mark the monster */
        m_ptr->mflag |= (MFLAG_TEMP);
    }

    /* Affect all marked monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Skip unmarked monsters */
        if (!(m_ptr->mflag & (MFLAG_TEMP))) continue;

        /* Remove mark */
        m_ptr->mflag &= ~(MFLAG_TEMP);

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Jump directly to the target monster */
        if (project(0, 0, y, x, dam, typ, flg)) obvious = TRUE;
    }

    /* Result */
    return (obvious);
}


/*
 * Speed monsters
 */
bool speed_monsters(void)
{
    return (project_hack(GF_OLD_SPEED, p_ptr->lev));
}

/*
 * Slow monsters
 */
bool slow_monsters(int power)
{
    return (project_hack(GF_OLD_SLOW, power));
}

/*
 * Sleep monsters
 */
bool sleep_monsters(int power)
{
    return (project_hack(GF_OLD_SLEEP, power));
}


/*
 * Banish evil monsters
 */
bool banish_evil(int dist)
{
    return (project_hack(GF_AWAY_EVIL, dist));
}


/*
 * Turn undead
 */
bool turn_undead(void)
{
    bool tester = (project_hack(GF_TURN_UNDEAD, p_ptr->lev));
    if (tester)
        virtue_add(VIRTUE_UNLIFE, -1);
    return tester;
}


/*
 * Dispel undead monsters
 */
bool dispel_undead(int dam)
{
    bool tester = (project_hack(GF_DISP_UNDEAD, dam));
    if (tester)
        virtue_add(VIRTUE_UNLIFE, -2);
    return tester;
}

/*
 * Dispel evil monsters
 */
bool dispel_evil(int dam)
{
    return (project_hack(GF_DISP_EVIL, dam));
}

/*
 * Dispel good monsters
 */
bool dispel_good(int dam)
{
    return (project_hack(GF_DISP_GOOD, dam));
}

/*
 * Dispel all monsters
 */
bool dispel_monsters(int dam)
{
    return (project_hack(GF_DISP_ALL, dam));
}

/*
 * Dispel 'living' monsters
 */
bool dispel_living(int dam)
{
    return (project_hack(GF_DISP_LIVING, dam));
}

/*
 * Dispel demons
 */
bool dispel_demons(int dam)
{
    return (project_hack(GF_DISP_DEMON, dam));
}


/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
    int     i;
    bool    sleep = FALSE;
    bool    speed = FALSE;


    /* Aggravate everyone nearby */
    for (i = 1; i < m_max; i++)
    {
        monster_type    *m_ptr = &m_list[i];
/*        monster_race    *r_ptr = &r_info[m_ptr->r_idx]; */

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip aggravating monster (or player) */
        if (i == who) continue;

        /* Wake up nearby sleeping monsters */
        if (m_ptr->cdis < MAX_SIGHT * 2)
        {
            /* Wake up */
            if (MON_CSLEEP(m_ptr))
            {
                (void)set_monster_csleep(i, 0);
                sleep = TRUE;
            }
            if ((is_hostile(m_ptr)) && ((who < 0) || (!p_ptr->uimapuku) ||
                ((one_in_((r_info[m_ptr->r_idx].max_num == 1) ? 33 : 9))))) m_ptr->mflag2 |= MFLAG2_NOPET;
        }

        /* Speed up monsters in line of sight */
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx))
        {
            if ((is_hostile(m_ptr)) && ((!very_nice_summon_hack) || (!(m_ptr->mflag & MFLAG_NICE))))
            {
                (void)set_monster_fast(i, MON_FAST(m_ptr) + 100);
                speed = TRUE;
            }
        }
    }

    /* Messages */
    if (speed) msg_print("You feel a sudden stirring nearby!");
    else if (sleep) msg_print("You hear a sudden stirring in the distance!");
    if (p_ptr->riding) p_ptr->update |= PU_BONUS;
}


/*
 * Delete a non-unique/non-quest monster
 */
bool genocide_aux(int m_idx, int power, bool player_cast, int dam_side, cptr spell_name)
{
    int          msec = delay_time();
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    bool         resist = FALSE;

    if (is_pet(m_ptr) && !player_cast) return FALSE;

    /* Hack -- Skip Unique Monsters or Quest Monsters */
    if (r_ptr->flags1 & RF1_UNIQUE) resist = TRUE;
    else if (m_ptr->mflag2 & MFLAG2_QUESTOR) resist = TRUE;
    else if (m_ptr->smart & (1U << SM_GUARDIAN)) resist = TRUE;

    else if (r_ptr->flags7 & RF7_UNIQUE2) resist = TRUE;

    else if (m_idx == p_ptr->riding) resist = TRUE;

    else if (!quests_allow_all_spells()) resist = TRUE;
    else if (p_ptr->inside_arena || p_ptr->inside_battle) resist = TRUE;

    else if (player_cast && (r_ptr->level > randint0(power))) resist = TRUE;

    else if (player_cast && (m_ptr->mflag2 & MFLAG2_NOGENO)) resist = TRUE;

    /* Delete the monster */
    else
    {
        delete_monster_idx(m_idx);
    }

    if (resist && player_cast)
    {
        bool see_m = mon_show_msg(m_ptr);
        char m_name[80];

        monster_desc(m_name, m_ptr, 0);
        if (see_m)
        {
            msg_format("%^s is unaffected.", m_name);
        }
        if (MON_CSLEEP(m_ptr))
        {
            (void)set_monster_csleep(m_idx, 0);
            if (m_ptr->ml)
            {
                msg_format("%^s wakes up.", m_name);
            }
        }
        if (is_friendly(m_ptr) && !is_pet(m_ptr))
        {
            if (see_m)
            {
                msg_format("%^s gets angry!", m_name);
            }
            set_hostile(m_ptr);
        }
        if (one_in_(13)) m_ptr->mflag2 |= MFLAG2_NOGENO;
    }

    if (player_cast)
    {
        /* Take damage */
        take_hit(DAMAGE_GENO, randint1(dam_side), format("the strain of casting %^s", spell_name));
    }

    /* Visual feedback */
    move_cursor_relative(py, px);

    /* Redraw */
    p_ptr->redraw |= (PR_HP);

    /* Handle */
    handle_stuff();

    /* Fresh */
    Term_fresh();

    /* Delay */
    Term_xtra(TERM_XTRA_DELAY, msec);

    return !resist;
}


/*
 * Delete all non-unique/non-quest monsters of a given "type" from the level
 */
bool symbol_genocide(int power, bool player_cast)
{
    int  i;
    char typ;
    bool do_virtue = FALSE;
    bool okay = FALSE;

    /* Prevent genocide in quest levels */
    if (!quests_allow_all_spells()) return TRUE;
    if (p_ptr->inside_arena || p_ptr->inside_battle)
    {
        return TRUE; /* But charge the player for the (stupid) action! */
    }

    /* Mega-Hack -- Get a monster symbol */
    while (!okay)
    {
        if (!get_com("Choose a monster species (by symbol; '?' for help) to genocide: ", &typ, FALSE))
            return FALSE;
        if (typ == 'n') /* naga hack */
        {
            if (msg_prompt("Really genocide Nagas? <color:y>[Y/N]</color>", "NY", PROMPT_DEFAULT) != 'Y') continue;
        }
        else if ((rogue_like_commands) && (typ == 'X'))
        {
            if (msg_prompt("Really genocide Xorns? <color:y>[Y/N]</color>", "NY", PROMPT_DEFAULT) != 'Y') continue;
        }
        else if (typ == '?')
        {
            screen_save();
            doc_display_help("monster.txt#Genocide", NULL);
            screen_load();
            continue;
        } 
        else if (!isalpha(typ))
        {
            if (msg_prompt("Confirm genocide? <color:y>[Y/N]</color>", "NY", PROMPT_DEFAULT) != 'Y') continue;
        }
        okay = TRUE;
    }

    /* Delete the monsters of that "type" */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        if (!m_ptr->r_idx) continue;
        if (r_ptr->d_char != typ) continue;

        if (genocide_aux(i, power, player_cast, 4, "Genocide"))
            do_virtue = TRUE;
    }

    if (do_virtue)
    {
        virtue_add(VIRTUE_VITALITY, -2);
        virtue_add(VIRTUE_CHANCE, -1);
    }

    return TRUE;
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_genocide(int power, bool player_cast)
{
    int  i;
    bool result = FALSE;

    /* Prevent mass genocide in quest levels */
    if (!quests_allow_all_spells()) return FALSE;
    if (p_ptr->inside_arena || p_ptr->inside_battle) return FALSE;

    /* Delete the (nearby) monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip distant monsters */
        if (m_ptr->cdis > MAX_SIGHT) continue;

        /* Note effect */
        result |= genocide_aux(i, power, player_cast, 3, "Mass Genocide");
    }

    if (result)
    {
        virtue_add(VIRTUE_VITALITY, -2);
        virtue_add(VIRTUE_CHANCE, -1);
    }

    return result;
}



/*
 * Delete all nearby (non-unique) undead
 */
bool mass_genocide_undead(int power, bool player_cast)
{
    int  i;
    bool result = FALSE;

    /* Prevent mass genocide in quest levels */
    if (!quests_allow_all_spells()) return FALSE;
    if (p_ptr->inside_arena || p_ptr->inside_battle) return FALSE;

    /* Delete the (nearby) monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        if (!(r_ptr->flags3 & RF3_UNDEAD)) continue;

        /* Skip distant monsters */
        if (m_ptr->cdis > MAX_SIGHT) continue;

        /* Note effect */
        result |= genocide_aux(i, power, player_cast, 3, "Annihilate Undead");
    }

    if (result)
    {
        virtue_add(VIRTUE_UNLIFE, -2);
        virtue_add(VIRTUE_CHANCE, -1);
    }

    return result;
}



/*
 * Probe nearby monsters
 */
bool probing(void)
{
    do_cmd_list_monsters(MON_LIST_PROBING);
    return TRUE; /*?? */
}

void _get_earthquake_feats(int *_feats, bool tree)
{
    if (tree)
    {
        _feats[0] = feat_tree;
        _feats[1] = feat_brake;
        _feats[2] = feat_tree;
        return;
    }
    else
    {
        _feats[0] = feat_granite;
        _feats[1] = feat_quartz_vein;
        _feats[2] = feat_magma_vein;
        if (dungeon_type)
        {
            dungeon_info_type *d_ptr = &d_info[dungeon_type];
            int i, _tf[3] = {feat_none, feat_none, feat_none};
            for (i = 0; i < 3; i++)
            {
                if (d_ptr->fill[i].percent > 0) _tf[i] = d_ptr->fill[i].feat;
            }
            if (d_ptr->stream1 != feat_none) _tf[1] = d_ptr->stream1;
            if (d_ptr->stream2 != feat_none) _tf[2] = d_ptr->stream2;
            for (i = 0; i < 3; i++)
            {
                if (_tf[i] == feat_none) continue;
                else
                {
                    feature_type *f_ptr = &f_info[_tf[i]];
                    if (!f_ptr->name) continue;
                    if (have_flag(f_ptr->flags, FF_PERMANENT)) continue;
                    if (have_flag(f_ptr->flags, FF_CREVASSE)) continue;
                    _feats[i] = _tf[i];
                }
            }
        }
        return;
    }
}

/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 */
bool destroy_area(int y1, int x1, int r, int power)
{
    int       y, x, k, t;
    cave_type *c_ptr;
    bool      flag = FALSE;
    bool      in_generate = FALSE;
    bool      tree = FALSE;

    if (power < 0)
        in_generate = TRUE;

    /* Prevent destruction of quest levels and town */
    if (!py_in_dungeon() || !quests_allow_all_spells())
    {
        return (FALSE);
    }

    if (d_info[dungeon_type].wild_type == TERRAIN_TREES)
    {
        tree = TRUE;
        power = MIN(75, power / 3);
    }

    /* Lose monster light */
    if (!in_generate) clear_mon_lite();

    /* Big area of affect */
    for (y = (y1 - r); y <= (y1 + r); y++)
    {
        for (x = (x1 - r); x <= (x1 + r); x++)
        {
            /* Skip illegal grids */
            if (!in_bounds(y, x)) continue;

            /* Extract the distance */
            k = distance(y1, x1, y, x);

            /* Stay in the circle of death */
            if (k > r) continue;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Lose room and vault */
            c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

            /* Lose light and knowledge */
            if ((!cave_perma_grid(c_ptr)) && ((!tree) || (!cave_have_flag_bold(y, x, FF_WATER))))
                c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);

            if (!in_generate) /* Normal */
            {
                /* Lose unsafety */
                c_ptr->info &= ~(CAVE_UNSAFE);

                /* Hack -- Notice player affect */
                if (player_bold(y, x))
                {
                    /* Hurt the player later */
                    flag = TRUE;

                    /* Do not hurt this grid */
                    continue;
                }
            }

            /* Hack -- Skip the epicenter */
            if ((y == y1) && (x == x1)) continue;

            if (c_ptr->m_idx)
            {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                if (in_generate) /* In generation */
                {
                    /* Delete the monster (if any) */
                    delete_monster(y, x);
                }
                else
                {
                    bool resist = FALSE;

                    if ((m_ptr->smart & (1U << SM_SUMMONED)) && ((m_ptr->mflag & MFLAG_NICE) || (coffee_break == SPEED_INSTA_COFFEE)))
                    {
                        /* Used to be summoned monsters never resisted! */
                        resist = FALSE;
                    }
                    else if (m_ptr->mflag2 & MFLAG2_NODESTRUCT) resist = TRUE;
                    else if (r_ptr->level > randint0(power)) resist = TRUE;

                    if (resist)
                    {
                        bool see_m = mon_show_msg(m_ptr);
                        char m_name[80];

                        monster_desc(m_name, m_ptr, 0);

                        if (see_m)
                            msg_format("%^s is unaffected.", m_name);

                        if (MON_CSLEEP(m_ptr))
                        {
                            set_monster_csleep(c_ptr->m_idx, 0);
                            if (m_ptr->ml)
                                msg_format("%^s wakes up.", m_name);
                        }

                        if (is_friendly(m_ptr) && !is_pet(m_ptr))
                        {
                            if (see_m)
                                msg_format("%^s gets angry!", m_name);
                            set_hostile(m_ptr);
                        }

                        if (!(m_ptr->mflag2 & MFLAG2_QUESTOR) /* Questors becoming immune to *destruct* can be advantageous! */
                             && !(r_ptr->flags2 & RF2_MULTIPLY)  /* Unmakers ... *shudder* */
                             && !(m_ptr->smart & (1U << SM_SUMMONED))
                             && one_in_(13))
                        {
                            m_ptr->mflag2 |= MFLAG2_NODESTRUCT;
                        }
                        continue;
                    }
                    else
                    {
                        if (m_ptr->mflag2 & MFLAG2_QUESTOR)
                        {
                            /* Heal the monster */
                            m_ptr->hp = m_ptr->maxhp;

                            /* Try to teleport away quest monsters */
                            if (!teleport_away(c_ptr->m_idx, (r * 2) + 1, TELEPORT_DEC_VALOUR)) continue;
                        }
                        else
                        {
                            delete_monster(y, x);
                        }
                    }
                }
            }

            /* During generation, destroyed artifacts are "preserved" */
            if (preserve_mode || in_generate)
            {
                s16b this_o_idx, next_o_idx = 0;

                /* Scan all objects in the grid */
                for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                {
                    object_type *o_ptr;

                    /* Acquire object */
                    o_ptr = &o_list[this_o_idx];

                    /* Acquire next object */
                    next_o_idx = o_ptr->next_o_idx;

                    /* Hack -- Preserve unknown artifacts */
                    if (object_is_fixed_artifact(o_ptr) && (!object_is_known(o_ptr) || in_generate))
                    {
                        /* Mega-Hack -- Preserve the artifact */
                        a_info[o_ptr->name1].generated = FALSE;

                        if (in_generate && cheat_peek)
                        {
                            char o_name[MAX_NLEN];
                            object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));
                            msg_format("Artifact (%s) was *destroyed* during generation.", o_name);
                        }
                    }
                    else if (random_artifacts && o_ptr->name3 && (!object_is_known(o_ptr) || in_generate))
                    {
                        /* Mega-Hack -- Preserve the artifact */
                        a_info[o_ptr->name3].generated = FALSE;

                        if (in_generate && cheat_peek)
                        {
                            char o_name[MAX_NLEN];
                            object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));
                            msg_format("Artifact (%s) was *destroyed* during generation.", o_name);
                        }
                    }
                    else if (in_generate && cheat_peek && o_ptr->art_name)
                    {
                        msg_print("One of the random artifacts was *destroyed* during generation.");
                    }
                }
            }

            /* Delete objects */
            delete_object(y, x);

            /* Destroy "non-permanent" grids */
            if ((!cave_perma_grid(c_ptr)) && ((!tree) || (!cave_have_flag_bold(y, x, FF_WATER))))
            {
                int _feats[3];

                _get_earthquake_feats(_feats, tree);

                /* Wall (or floor) type */
                t = randint0(200);

                if (!in_generate) /* Normal */
                {
                    if (t < 20)
                    {
                        /* Create granite wall */
                        cave_set_feat(y, x, _feats[0]);
                    }
                    else if (t < 70)
                    {
                        /* Create quartz vein */
                        cave_set_feat(y, x, _feats[1]);
                    }
                    else if (t < 100)
                    {
                        /* Create magma vein */
                        cave_set_feat(y, x, _feats[2]);
                    }
                    else
                    {
                        /* Create floor */
                        cave_set_feat(y, x, floor_type[randint0(100)]);
                    }
                }
                else /* In generation */
                {
                    if (t < 20)
                    {
                        /* Create granite wall */
                        place_extra_grid(c_ptr);
                    }
                    else if (t < 70)
                    {
                        /* Create quartz vein */
                        c_ptr->feat = feat_quartz_vein;
                    }
                    else if (t < 100)
                    {
                        /* Create magma vein */
                        c_ptr->feat = feat_magma_vein;
                    }
                    else
                    {
                        /* Create floor */
                        place_floor_grid(c_ptr);
                    }

                    /* Clear garbage of hidden trap or door */
                    c_ptr->mimic = 0;
                }
            }
        }
    }

    if (!in_generate)
    {
        /* Process "re-glowing" */
        for (y = (y1 - r); y <= (y1 + r); y++)
        {
            for (x = (x1 - r); x <= (x1 + r); x++)
            {
                /* Skip illegal grids */
                if (!in_bounds(y, x)) continue;

                /* Extract the distance */
                k = distance(y1, x1, y, x);

                /* Stay in the circle of death */
                if (k > r) continue;

                /* Access the grid */
                c_ptr = &cave[y][x];

                if (is_mirror_grid(c_ptr)) c_ptr->info |= CAVE_GLOW;
                else if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS))
                {
                    int i, yy, xx;
                    cave_type *cc_ptr;

                    for (i = 0; i < 9; i++)
                    {
                        yy = y + ddy_ddd[i];
                        xx = x + ddx_ddd[i];
                        if (!in_bounds2(yy, xx)) continue;
                        cc_ptr = &cave[yy][xx];
                        if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
                        {
                            c_ptr->info |= CAVE_GLOW;
                            break;
                        }
                    }
                }
            }
        }

        /* Hack -- Affect player */
        if (flag)
        {
            /* Message */
            msg_print("There is a searing blast of light!");

            /* Blind the player */
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_LITE))
            {
                /* Become blind */
                (void)set_blind(p_ptr->blind + 10 + randint1(10), FALSE);
            }
        }

        forget_flow();

        /* Mega-Hack -- Forget the view and lite */
        p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

        /* Update stuff */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_MONSTERS);

        if (weaponmaster_is_(WEAPONMASTER_DIGGERS))
            p_ptr->update |= PU_BONUS;

        /* Redraw map */
        p_ptr->redraw |= (PR_MAP);

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

        if (p_ptr->special_defense & NINJA_S_STEALTH)
        {
            if (cave[py][px].info & CAVE_GLOW) set_superstealth(FALSE);
        }
    }

    /* Success */
    return (TRUE);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 */
bool earthquake_aux(int cy, int cx, int r, int m_idx)
{
    int             i, t, y, x, yy, xx, dy, dx;
    int             damage = 0;
    int             sn = 0, sy = 0, sx = 0;
    bool            hurt = FALSE;
    cave_type       *c_ptr;
    bool            map[32][32];
    bool            tree = FALSE;


    /* Prevent destruction of quest levels and town */
    if (!py_in_dungeon())
    {
        return (FALSE);
    }

    if (d_info[dungeon_type].wild_type == TERRAIN_TREES)
    {
        tree = TRUE;
    }

    /* Paranoia -- Enforce maximum range */
    if (r > 12) r = 12;

    /* Clear the "maximal blast" area */
    for (y = 0; y < 32; y++)
    {
        for (x = 0; x < 32; x++)
        {
            map[y][x] = FALSE;
        }
    }

    /* Check around the epicenter */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip illegal grids */
            if (!in_bounds(yy, xx)) continue;

            /* Skip distant grids */
            if (distance(cy, cx, yy, xx) > r) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            /* Lose room and vault */
            c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY | CAVE_UNSAFE);

            if ((tree) && (cave_have_flag_bold(yy, xx, FF_WATER))) continue;

            /* Lose light and knowledge */
            c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

            /* Skip the epicenter */
            if (!dx && !dy) continue;

            /* Skip most grids */
            if (randint0(100) < 85) continue;

            /* Damage this grid */
            map[16+yy-cy][16+xx-cx] = TRUE;

            /* Hack -- Take note of player damage */
            if (player_bold(yy, xx)) hurt = TRUE;
        }
    }

    /* First, affect the player (if necessary) */
    if (hurt && !p_ptr->pass_wall && !p_ptr->kill_wall)
    {
        /* Check around the player */
        for (i = 0; i < 8; i++)
        {
            /* Access the location */
            y = py + ddy_ddd[i];
            x = px + ddx_ddd[i];

            /* Skip non-empty grids */
            if (!cave_empty_bold(y, x)) continue;

            /* Important -- Skip "quake" grids */
            if (map[16+y-cy][16+x-cx]) continue;

            if (cave[y][x].m_idx) continue;

            /* Count "safe" grids */
            sn++;

            /* Randomize choice */
            if (randint0(sn) > 0) continue;

            /* Save the safe location */
            sy = y; sx = x;
        }

        /* Random message */
        switch (randint1(3))
        {
            case 1:
            {
                if (tree) msg_print("The cave quakes! A tree falls on top of you!");
                else msg_print("The cave ceiling collapses!");
                break;
            }
            case 2:
            {
                msg_print("The cave floor twists in an unnatural way!");
                break;
            }
            default:
            {
                msg_print("The cave quakes! You are pummeled with debris!");
                break;
            }
        }

        /* Hurt the player a lot */
        damage = 0;
        if (!sn)
        {
            if (!mut_present(MUT_EVASION) || one_in_(2))
            {
                msg_print("You are <color:v>severely crushed</color>!");
                damage = 50 + damroll(3, 50);
                (void)set_stun(p_ptr->stun + damroll(7, 7), FALSE);
            }
        }

        /* Destroy the grid, and push the player to safety */
        else
        {
            /* Calculate results */
            switch (randint1(3))
            {
                case 1:
                {
                    msg_print("You nimbly dodge the blast!");
                    break;
                }
                case 2:
                {
                    if (!mut_present(MUT_EVASION) || one_in_(2))
                    {
                        msg_print("You are <color:R>bashed by rubble</color>!");
                        damage = damroll(10, 4);
                        (void)set_stun(p_ptr->stun + damroll(5, 5), FALSE);
                    }
                    break;
                }
                case 3:
                {
                    if (!mut_present(MUT_EVASION) || one_in_(2))
                    {
                        msg_print("You are <color:R>crushed between the floor and ceiling</color>!");
                        damage = damroll(10, 4);
                        (void)set_stun(p_ptr->stun + damroll(5, 5), FALSE);
                    }
                    break;
                }
            }

            /* Move the player to the safe location */
            (void)move_player_effect(sy, sx, MPE_DONT_PICKUP);
        }

        /* Important -- no wall on player */
        map[16+py-cy][16+px-cx] = FALSE;

        /* Take some damage */
        if (damage)
        {
            char *killer;

            if (m_idx)
            {
                char m_name[80];
                monster_type *m_ptr = &m_list[m_idx];

                /* Get the monster's real name */
                monster_desc(m_name, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);

                killer = format("an earthquake caused by %s", m_name);
            }
            else
            {
                killer = "an earthquake";
            }

            take_hit(DAMAGE_ATTACK, damage, killer);
        }
    }

    /* Examine the quaked region */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip unaffected grids */
            if (!map[16+yy-cy][16+xx-cx]) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            if (c_ptr->m_idx == p_ptr->riding) continue;

            /* Process monsters */
            if (c_ptr->m_idx)
            {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                /* Quest monsters */
                if ((m_ptr->mflag2 & MFLAG2_QUESTOR) || ((r_ptr->flags3 & RF3_AMBERITE) && (m_ptr->hp < 0)))
                {
                    /* No wall on quest monsters */
                    map[16+yy-cy][16+xx-cx] = FALSE;

                    continue;
                }

                /* Most monsters cannot co-exist with rock */
                if (!(r_ptr->flags2 & (RF2_KILL_WALL)) &&
                    !(r_ptr->flags2 & (RF2_PASS_WALL)))
                {
                    char m_name[80];

                    /* Assume not safe */
                    sn = 0;

                    /* Monster can move to escape the wall */
                    if (!(r_ptr->flags1 & (RF1_NEVER_MOVE)))
                    {
                        /* Look for safety */
                        for (i = 0; i < 8; i++)
                        {
                            /* Access the grid */
                            y = yy + ddy_ddd[i];
                            x = xx + ddx_ddd[i];

                            /* Skip non-empty grids */
                            if (!cave_empty_bold(y, x)) continue;

                            /* Hack -- no safety on glyph of warding */
                            if (is_glyph_grid(&cave[y][x])) continue;
                            if (is_mon_trap_grid(&cave[y][x])) continue;

                            /* ... nor on the Pattern */
                            if (pattern_tile(y, x)) continue;

                            /* Important -- Skip "quake" grids */
                            if (map[16+y-cy][16+x-cx]) continue;

                            if (cave[y][x].m_idx) continue;
                            if (player_bold(y, x)) continue;

                            /* Count "safe" grids */
                            sn++;

                            /* Randomize choice */
                            if (randint0(sn) > 0) continue;

                            /* Save the safe grid */
                            sy = y; sx = x;
                        }
                    }

                    /* Describe the monster */
                    monster_desc(m_name, m_ptr, 0);

                    /* Scream in pain */
                    if (mon_show_msg(m_ptr)) msg_format("%^s wails out in pain!", m_name);

                    /* Take damage from the quake */
                    damage = (sn ? damroll(4, 8) : (m_ptr->hp + 1));

                    /* Monster is certainly awake */
                    (void)set_monster_csleep(c_ptr->m_idx, 0);

                    /* Apply damage directly */
                    m_ptr->hp -= damage;

                    /* Delete (not kill) "dead" monsters */
                    if (m_ptr->hp < 0)
                    {
                        if (mon_show_msg(m_ptr)) msg_format("%^s is embedded in the rock!", m_name);

                        /* Delete the monster */
                        delete_monster(yy, xx);

                        /* No longer safe */
                        sn = 0;
                    }

                    /* Hack -- Escape from the rock */
                    if (sn)
                    {
                        int m_idx = cave[yy][xx].m_idx;

                        /* Update the old location */
                        cave[yy][xx].m_idx = 0;

                        /* Update the new location */
                        cave[sy][sx].m_idx = m_idx;

                        /* Move the monster */
                        m_ptr->fy = sy;
                        m_ptr->fx = sx;

                        /* Update the monster (new location) */
                        update_mon(m_idx, TRUE);

                        /* Redraw the old grid */
                        lite_spot(yy, xx);

                        /* Redraw the new grid */
                        lite_spot(sy, sx);
                    }
                }
            }
        }
    }

    /* Lose monster light */
    clear_mon_lite();

    /* Examine the quaked region */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip unaffected grids */
            if (!map[16+yy-cy][16+xx-cx]) continue;

            /* Access the cave grid */
            c_ptr = &cave[yy][xx];

            /* Paranoia -- never affect player */
/*            if (player_bold(yy, xx)) continue; */

            /* Destroy location (if valid) */
            if (cave_valid_bold(yy, xx))
            {
                int _feats[3];

                _get_earthquake_feats(_feats, tree);

                /* Delete objects */
                delete_object(yy, xx);

                /* Wall (or floor) type */
                t = cave_have_flag_bold(yy, xx, FF_PROJECT) ? randint0(100) : 200;

                /* Granite */
                if (t < 20)
                {
                    /* Create granite wall */
                    cave_set_feat(yy, xx, _feats[0]);
                }

                /* Quartz */
                else if (t < 70)
                {
                    /* Create quartz vein */
                    cave_set_feat(yy, xx, _feats[1]);
                }

                /* Magma */
                else if (t < 100)
                {
                    /* Create magma vein */
                    cave_set_feat(yy, xx, _feats[2]);
                }

                /* Floor */
                else
                {
                    /* Create floor */
                    cave_set_feat(yy, xx, floor_type[randint0(100)]);
                }
            }
        }
    }


    /* Process "re-glowing" */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip illegal grids */
            if (!in_bounds(yy, xx)) continue;

            /* Skip distant grids */
            if (distance(cy, cx, yy, xx) > r) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            if (is_mirror_grid(c_ptr)) c_ptr->info |= CAVE_GLOW;
            else if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS))
            {
                int ii, yyy, xxx;
                cave_type *cc_ptr;

                for (ii = 0; ii < 9; ii++)
                {
                    yyy = yy + ddy_ddd[ii];
                    xxx = xx + ddx_ddd[ii];
                    if (!in_bounds2(yyy, xxx)) continue;
                    cc_ptr = &cave[yyy][xxx];
                    if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
                    {
                        c_ptr->info |= CAVE_GLOW;
                        break;
                    }
                }
            }
        }
    }


    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_MONSTERS);

    if (weaponmaster_is_(WEAPONMASTER_DIGGERS))
        p_ptr->update |= PU_BONUS;

    /* Update the health bar */
    p_ptr->redraw |= PR_HEALTH_BARS;

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    if (p_ptr->special_defense & NINJA_S_STEALTH)
    {
        if (cave[py][px].info & CAVE_GLOW) set_superstealth(FALSE);
    }

    /* Success */
    return (TRUE);
}

bool earthquake(int cy, int cx, int r)
{
    return earthquake_aux(cy, cx, r, 0);
}


void discharge_minion(void)
{
    int i;
    bool okay = TRUE;

    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        if (!m_ptr->r_idx || !is_pet(m_ptr)) continue;
        if (m_ptr->nickname) okay = FALSE;
    }
    if (!okay || p_ptr->riding)
    {
        if (!get_check("You will blast all pets. Are you sure? "))
            return;
    }
    for (i = 1; i < m_max; i++)
    {
        int dam;
        int typ = GF_PLASMA;
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;

        if (p_ptr->pclass == CLASS_NECROMANCER)
        {
            switch (randint1(4))
            {
            case 1: typ = GF_POIS; break;
            case 2: typ = GF_NETHER; break;
            case 3: typ = GF_DISENCHANT; break;
            case 4: typ = GF_MANA; break;
            }
        }

        if (!m_ptr->r_idx || !is_pet(m_ptr)) continue;
        r_ptr = &r_info[m_ptr->r_idx];

        /* Uniques resist discharging */
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0x00);
            msg_format("%^s resists being blasted, and runs away.", m_name);
            delete_monster_idx(i);
            continue;
        }
        dam = m_ptr->maxhp / 2;
        if (dam > 100) dam = (dam-100)/2 + 100;
        if (dam > 400) dam = (dam-400)/2 + 400;
        if (dam > 800) dam = 800;
        project(i, 2+(r_ptr->level/20), m_ptr->fy,
            m_ptr->fx, dam, typ,
            PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);

        delete_monster_idx(i);
    }
}


/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        cave_type *c_ptr = &cave[y][x];

        /* No longer in the array */
        c_ptr->info &= ~(CAVE_TEMP);

        /* Update only non-CAVE_GLOW grids */
        /* if (c_ptr->info & (CAVE_GLOW)) continue; */

        /* Perma-Lite */
        c_ptr->info |= (CAVE_GLOW);

        /* Process affected monsters */
        if (c_ptr->m_idx)
        {
            int chance = 25;

            monster_type    *m_ptr = &m_list[c_ptr->m_idx];

            monster_race    *r_ptr = &r_info[m_ptr->r_idx];

            /* Update the monster */
            update_mon(c_ptr->m_idx, FALSE);

            /* Stupid monsters rarely wake up */
            if (r_ptr->flags2 & (RF2_STUPID)) chance = 10;

            /* Smart monsters always wake up */
            if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

            /* Sometimes monsters wake up */
            if (MON_CSLEEP(m_ptr) && (randint0(100) < chance))
            {
                /* Wake up! */
                (void)set_monster_csleep(c_ptr->m_idx, 0);

                /* Notice the "waking up" */
                if (m_ptr->ml)
                {
                    char m_name[80];

                    /* Acquire the monster name */
                    monster_desc(m_name, m_ptr, 0);

                    /* Dump a message */
                    msg_format("%^s wakes up.", m_name);
                }
            }
        }

        /* Note */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);

        update_local_illumination(y, x);
    }

    /* None left */
    temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];
        int j;

        cave_type *c_ptr = &cave[y][x];
        bool do_dark = !is_mirror_grid(c_ptr);

        /* No longer in the array */
        c_ptr->info &= ~(CAVE_TEMP);

        /* Darken the grid */
        if (do_dark)
        {
            if (dun_level || !is_daytime())
            {
                for (j = 0; j < 9; j++)
                {
                    int by = y + ddy_ddd[j];
                    int bx = x + ddx_ddd[j];

                    if (in_bounds2(by, bx))
                    {
                        cave_type *cc_ptr = &cave[by][bx];

                        if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
                        {
                            do_dark = FALSE;
                            break;
                        }
                    }
                }

                if (!do_dark) continue;
            }

            c_ptr->info &= ~(CAVE_GLOW);

            /* Hack -- Forget "boring" grids */
            if (!have_flag(f_info[get_feat_mimic(c_ptr)].flags, FF_REMEMBER))
            {
                /* Forget the grid */
                if (!view_torch_grids) c_ptr->info &= ~(CAVE_MARK);

                /* Notice */
                note_spot(y, x);
            }

            /* Process affected monsters */
            if (c_ptr->m_idx)
            {
                /* Update the monster */
                update_mon(c_ptr->m_idx, FALSE);
            }

            /* Redraw */
            lite_spot(y, x);

            update_local_illumination(y, x);
        }
    }

    /* None left */
    temp_n = 0;
}


/*
 * Determine how much contiguous open space this grid is next to
 */
static int next_to_open(int cy, int cx, bool (*pass_bold)(int, int))
{
    int i;

    int y, x;

    int len = 0;
    int blen = 0;

    for (i = 0; i < 16; i++)
    {
        y = cy + ddy_cdd[i % 8];
        x = cx + ddx_cdd[i % 8];

        /* Found a wall, break the length */
        if (!pass_bold(y, x))
        {
            /* Track best length */
            if (len > blen)
            {
                blen = len;
            }

            len = 0;
        }
        else
        {
            len++;
        }
    }

    return (MAX(len, blen));
}


static int next_to_walls_adj(int cy, int cx, bool (*pass_bold)(int, int))
{
    int i;

    int y, x;

    int c = 0;

    for (i = 0; i < 8; i++)
    {
        y = cy + ddy_ddd[i];
        x = cx + ddx_ddd[i];

        if (!pass_bold(y, x)) c++;
    }

    return c;
}


/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x, bool only_room, bool (*pass_bold)(int, int))
{
    cave_type *c_ptr;

    /* Get the grid */
    c_ptr = &cave[y][x];

    /* Avoid infinite recursion */
    if (c_ptr->info & (CAVE_TEMP)) return;

    /* Do not "leave" the current room */
    if (!(c_ptr->info & (CAVE_ROOM)))
    {
        if (only_room) return;

        /* Verify */
        if (!in_bounds2(y, x)) return;

        /* Do not exceed the maximum spell range ...
         * ... x2 for quests. When designing quests, it is a bit
         * tedious to set the ROOM flag correctly (unless the entire
         * quest is a single room). Much easier to just have light
         * area work a bit farther than normal.*/
        if (distance(py, px, y, x) > MAX_RANGE*2) return;

        /* Verify this grid */
        /*
         * The reason why it is ==6 instead of >5 is that 8 is impossible
         * due to the check for cave_bold above.
         * 7 lights dead-end corridors (you need to do this for the
         * checkboard interesting rooms, so that the boundary is lit
         * properly.
         * This leaves only a check for 6 bounding walls!
         */
        if (in_bounds(y, x) && pass_bold(y, x) &&
            (next_to_walls_adj(y, x, pass_bold) == 6) && (next_to_open(y, x, pass_bold) <= 1)) return;
    }

    /* Paranoia -- verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid as "seen" */
    c_ptr->info |= (CAVE_TEMP);

    /* Add it to the "seen" set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}

/*
 * Aux function -- see below
 */
static bool cave_pass_lite_bold(int y, int x)
{
    return cave_los_bold(y, x);
}

/*
 * Aux function -- see below
 */
static void cave_temp_lite_room_aux(int y, int x)
{
    cave_temp_room_aux(y, x, FALSE, cave_pass_lite_bold);
}

/*
 * Aux function -- see below
 */
static bool cave_pass_dark_bold(int y, int x)
{
    return cave_have_flag_bold(y, x, FF_PROJECT);
}

/*
 * Aux function -- see below
 */
static void cave_temp_unlite_room_aux(int y, int x)
{
    cave_temp_room_aux(y, x, TRUE, cave_pass_dark_bold);
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_lite_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get lit, but stop light */
        if (!cave_pass_lite_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_lite_room_aux(y + 1, x);
        cave_temp_lite_room_aux(y - 1, x);
        cave_temp_lite_room_aux(y, x + 1);
        cave_temp_lite_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_lite_room_aux(y + 1, x + 1);
        cave_temp_lite_room_aux(y - 1, x - 1);
        cave_temp_lite_room_aux(y - 1, x + 1);
        cave_temp_lite_room_aux(y + 1, x - 1);
    }

    /* Now, lite them all up at once */
    cave_temp_room_lite();

    if (p_ptr->special_defense & NINJA_S_STEALTH)
    {
        if (cave[py][px].info & CAVE_GLOW) set_superstealth(FALSE);
    }
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_unlite_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get dark, but stop darkness */
        if (!cave_pass_dark_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_unlite_room_aux(y + 1, x);
        cave_temp_unlite_room_aux(y - 1, x);
        cave_temp_unlite_room_aux(y, x + 1);
        cave_temp_unlite_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_unlite_room_aux(y + 1, x + 1);
        cave_temp_unlite_room_aux(y - 1, x - 1);
        cave_temp_unlite_room_aux(y - 1, x + 1);
        cave_temp_unlite_room_aux(y + 1, x - 1);
    }

    /* Now, darken them all at once */
    cave_temp_room_unlite();
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool lite_area(int dam, int rad)
{
    int flg = PROJECT_GRID | PROJECT_KILL;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS)
    {
        msg_print("The darkness of this dungeon absorbs your light.");
        return FALSE;
    }

    /* Hack -- Message */
    if (!p_ptr->blind)
    {
        msg_print("You are surrounded by a white light.");

    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_LITE_WEAK, flg);

    /* Lite up the room */
    lite_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlite_area(int dam, int rad)
{
    int flg = PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->blind)
    {
        msg_print("Darkness surrounds you.");

    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_DARK_WEAK, flg);

    /* Lite up the room */
    unlite_room(py, px);

    /* Assume seen */
    return (TRUE);
}



/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
    return fire_ball_aux(typ, dir, dam, rad, 0);
}

bool fire_ball_aux(int typ, int dir, int dam, int rad, int xtra_flgs)
{
    int tx, ty;

    int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | xtra_flgs;

    if (typ == GF_CONTROL_LIVING) flg|= PROJECT_HIDE;
    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target"
     * Note: target_okay() will check target_able() which requires projectable() 
     * This requirement is *wrong* in certain situations (e.g. Breathe Disintegration) 
     * but it is unclear how to best communicate that fact. However, what would we do
     * at this point anyway? Project() at the players feet? That is just silly ... */
    if (dir == 5 /* && target_okay() */)
    {
        flg &= ~(PROJECT_STOP);
        tx = target_col;
        ty = target_row;
    }
    else if (rad < 0) flg &= ~(PROJECT_STOP); /* Breath attack */

    /* Analyze the "dir" and the "target". Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_rocket(int typ, int dir, int dam, int rad)
{
    int tx, ty;

    int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target". Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball_hide(int typ, int dir, int dam, int rad)
{
    int tx, ty;

    int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_HIDE;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        flg &= ~(PROJECT_STOP);
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target". Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
}


/*
 * Cast a meteor spell, defined as a ball spell cast by an arbitary monster,
 * player, or outside source, that starts out at an arbitrary location, and
 * leaving no trail from the "caster" to the target. This function is
 * especially useful for bombardments and similar. -LM-
 *
 * Option to hurt the player.
 */
bool fire_meteor(int who, int typ, int y, int x, int dam, int rad)
{
    int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

    /* Analyze the "target" and the caster. */
    return (project(who, rad, y, x, dam, typ, flg));
}


bool fire_blast(int typ, int dir, int dd, int ds, int num, int dev)
{
    int ly, lx, ld;
    int ty, tx, y, x;
    int i;

    int flg = PROJECT_FAST | PROJECT_THRU | PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE | PROJECT_GRID;

    /* Assume okay */
    bool result = TRUE;

    /* Use the given direction */
    if (dir != 5)
    {
        ly = ty = py + 20 * ddy[dir];
        lx = tx = px + 20 * ddx[dir];
    }

    /* Use an actual "target" */
    else /* if (dir == 5) */
    {
        tx = target_col;
        ty = target_row;

        lx = 20 * (tx - px) + px;
        ly = 20 * (ty - py) + py;
    }

    ld = distance(py, px, ly, lx);

    /* Blast */
    for (i = 0; i < num; i++)
    {
        while (1)
        {
            /* Get targets for some bolts */
            y = rand_spread(ly, ld * dev / 20);
            x = rand_spread(lx, ld * dev / 20);

            if (distance(ly, lx, y, x) <= ld * dev / 20) break;
        }

        /* Analyze the "dir" and the "target". */
        if (!project(0, 0, y, x, damroll(dd, ds), typ, flg))
        {
            result = FALSE;
        }
    }

    return (result);
}


/*
 * Switch position with a monster.
 */
bool teleport_swap(int dir)
{
    int tx, ty;
    cave_type * c_ptr;
    monster_type * m_ptr;
    monster_race * r_ptr;

    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }
    else
    {
        tx = px + ddx[dir];
        ty = py + ddy[dir];
    }
    c_ptr = &cave[ty][tx];

    if (p_ptr->anti_tele)
    {
        msg_print("A mysterious force prevents you from teleporting!");
        equip_learn_flag(OF_NO_TELE);
        return FALSE;
    }

    /* Require clean line of sight */
    if (!very_clean_shot(py, px, ty, tx))
    {
        msg_print("You cannot see your target well enough.");
        return FALSE;
    }

    if (!c_ptr->m_idx || (c_ptr->m_idx == p_ptr->riding))
    {
        msg_print("You can't trade places with that!");


        /* Failure */
        return FALSE;
    }

    if ((c_ptr->info & CAVE_ICKY) || (distance(ty, tx, py, px) > p_ptr->lev * 3 / 2 + 10))
    {
        msg_print("Failed to swap.");


        /* Failure */
        return FALSE;
    }

    m_ptr = &m_list[c_ptr->m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    (void)set_monster_csleep(c_ptr->m_idx, 0);

    if (r_ptr->flagsr & RFR_RES_TELE)
    {
        msg_print("Your teleportation is blocked!");
        mon_lore_r(m_ptr, RFR_RES_TELE);
        return FALSE;
    }

    sound(SOUND_TELEPORT);

    /* Swap the player and monster */
    (void)move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);

    /* Success */
    return TRUE;
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
bool project_hook(int typ, int dir, int dam, int flg)
{
    int tx, ty;

    /* Pass through the target if needed */
    flg |= (PROJECT_THRU);

    /* Use the given direction */
    tx = px + ddx[dir];
    ty = py + ddy[dir];

    /* Hack -- Use an actual "target"
     * Note: target_okay() will check target_able() which requires projectable() 
     * This requirement is *wrong* in certain situations (e.g. Beam of Disintegration) 
     * but it is unclear how to best communicate that fact. However, what would we do
     * at this point anyway? Project() at the players feet? That is just silly ... */
    if (dir == 5 /*&& target_okay()*/)
    {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(0, 0, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell.
 * Stop if we hit a monster, as a "bolt".
 * Affect monsters and grids (not objects).
 */
bool fire_bolt(int typ, int dir, int dam)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE | PROJECT_GRID;
    return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a beam spell.
 * Pass through monsters, as a "beam".
 * Affect monsters, grids and objects.
 */
bool fire_beam(int typ, int dir, int dam)
{
    int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (randint0(100) < prob)
    {
        return (fire_beam(typ, dir, dam));
    }
    else
    {
        return (fire_bolt(typ, dir, dam));
    }
}


/*
 * Some of the old functions
 */
bool lite_line(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
    return (project_hook(GF_LITE_WEAK, dir, damroll(6, 8), flg));
}


bool drain_life(int dir, int dam)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    if (melee_challenge) return FALSE;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}


bool wall_to_mud(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint1(30), flg));
}


bool wizard_lock(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
    return (project_hook(GF_JAM_DOOR, dir, 20 + randint1(30), flg));
}


bool destroy_door(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}


bool disarm_trap(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}


bool heal_monster(int dir, int dam)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_HEAL, dir, dam, flg));
}


bool speed_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_SPEED, dir, p_ptr->lev, flg));
}


bool slow_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}


bool sleep_monster(int dir, int power)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_OLD_SLEEP, dir, power, flg));
}


bool stasis_monster(int dir)
{
    return (fire_ball_hide(GF_STASIS, dir, p_ptr->lev*2, 0));
}


bool stasis_evil(int dir)
{
    return (fire_ball_hide(GF_STASIS_EVIL, dir, p_ptr->lev*2, 0));
}


bool confuse_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}


bool stun_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_STUN, dir, 5 + plev/5, flg));
}


bool poly_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    bool tester = (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
    if (tester)
        virtue_add(VIRTUE_CHANCE, 1);
    return(tester);
}


bool clone_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}


bool fear_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE;
    return (project_hook(GF_TURN_ALL, dir, plev, flg));
}


bool teleport_monster(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_KILL;
    return (project_hook(GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}

/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */
bool door_creation(void)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_DOOR, flg));
}


bool trap_creation(int y, int x)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, y, x, 0, GF_MAKE_TRAP, flg));
}


bool tree_creation(void)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_TREE, flg));
}


bool glyph_creation(void)
{
    int flg = PROJECT_GRID | PROJECT_ITEM;
    return (project(0, 1, py, px, 0, GF_MAKE_GLYPH, flg));
}


bool wall_stone(void)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;

    bool dummy = (project(0, 1, py, px, 0, GF_MAKE_WALL, flg));

    /* Update stuff */
    p_ptr->update |= (PU_FLOW);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    return dummy;
}


bool destroy_doors_touch(void)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_KILL_DOOR, flg));
}


bool sleep_monsters_touch(void)
{
    int flg = PROJECT_KILL | PROJECT_HIDE;
    return (project(0, 1, py, px, p_ptr->lev, GF_OLD_SLEEP, flg));
}


bool animate_dead(int who, int y, int x)
{
    int flg = PROJECT_ITEM | PROJECT_HIDE;
    return (project(who, 5, y, x, 0, GF_ANIM_DEAD, flg));
}


void call_chaos(int pct)
{
    int chaos_type, dummy, dir;
    int plev = p_ptr->lev;
    bool line_chaos = FALSE;

    int hurt_types[31] =
    {
        GF_ELEC,      GF_POIS,    GF_ACID,    GF_COLD,
        GF_FIRE,      GF_MISSILE, GF_ARROW,   GF_PLASMA,
        GF_HOLY_FIRE, GF_WATER,   GF_LITE,    GF_DARK,
        GF_FORCE,     GF_INERT, GF_MANA,    GF_METEOR,
        GF_ICE,       GF_CHAOS,   GF_NETHER,  GF_DISENCHANT,
        GF_SHARDS,    GF_SOUND,   GF_NEXUS,   GF_CONFUSION,
        GF_TIME,      GF_GRAVITY, GF_ROCKET,  GF_NUKE,
        GF_HELL_FIRE, GF_DISINTEGRATE, GF_PSY_SPEAR
    };

    chaos_type = hurt_types[randint0(31)];
    if (one_in_(4)) line_chaos = TRUE;

    if (one_in_(6))
    {
        for (dummy = 1; dummy < 10; dummy++)
        {
            if (dummy - 5)
            {
                if (line_chaos)
                    fire_beam(chaos_type, dummy, 150*pct/100);
                else
                    fire_ball(chaos_type, dummy, 150*pct/100, 2);
            }
        }
    }
    else if (one_in_(3))
    {
        fire_ball(chaos_type, 0, 500*pct/100, 8);
    }
    else
    {
        if (!get_fire_dir(&dir)) return;
        if (line_chaos)
            fire_beam(chaos_type, dir, 250*pct/100);
        else
            fire_ball(chaos_type, dir, 250*pct/100, 3 + (plev / 35));
    }
}


/*
 * Activate the evil Topi Ylinen curse
 * rr9: Stop the nasty things when a Cyberdemon is summoned
 * or the player gets paralyzed.
 */
bool activate_ty_curse(bool stop_ty, int *count)
{
    int     i = 0;

    int flg = (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);

    if (statistics_hack) return TRUE;

    do
    {
        switch (randint1(34))
        {
        case 28: case 29:
            if (!(*count))
            {
                msg_print("The ground trembles...");

                earthquake(py, px, 5 + randint0(10));
                if (!one_in_(6)) break;
            }
        case 30: case 31:
            if (!(*count))
            {
                int dam = damroll(10, 10);
                msg_print("A portal opens to a plane of raw mana!");

                project(0, 8, py, px, dam, GF_MANA, flg);
                take_hit(DAMAGE_NOESCAPE, dam, "released pure mana");
                if (!one_in_(6)) break;
            }
        case 32: case 33:
            if (!(*count))
            {
                msg_print("Space warps about you!");

                teleport_player(damroll(10, 10), TELEPORT_PASSIVE);
                if (randint0(13)) (*count) += activate_hi_summon(py, px, FALSE);
                if (!one_in_(6)) break;
            }
        case 34:
            msg_print("You feel a surge of energy!");

            wall_breaker();
            if (!randint0(7))
            {
                project(0, 7, py, px, 50, GF_KILL_WALL, flg);
                take_hit(DAMAGE_NOESCAPE, 50, "surge of energy");
            }
            if (!one_in_(6)) break;
        case 1: case 2: case 3: case 16: case 17:
            aggravate_monsters(0);
            if (!one_in_(6)) break;
        case 4: case 5: case 6:
            (*count) += activate_hi_summon(py, px, FALSE);
            if (!one_in_(6)) break;
        case 7: case 8: case 9: case 18:
            (*count) += summon_specific(0, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            if (!one_in_(6)) break;
        case 10: case 11: case 12:
            msg_print("You feel your life draining away...");

            lose_exp(p_ptr->exp / 16);
            if (!one_in_(6)) break;
        case 13: case 14: case 15: case 19: case 20:
            if (stop_ty || (p_ptr->free_act && (randint1(125) < p_ptr->skills.sav)) || (p_ptr->pclass == CLASS_BERSERKER))
            {
                /* Do nothing */ ;
            }
            else
            {
                msg_print("You feel like a statue!");

                if (p_ptr->free_act)
                {
                    set_paralyzed(randint1(2), FALSE);
                    equip_learn_flag(OF_FREE_ACT);
                }
                else
                    set_paralyzed(randint1(13), FALSE);
                stop_ty = TRUE;
            }
            if (!one_in_(6)) break;
        case 21: case 22: case 23:
            (void)do_dec_stat(randint0(6));
            if (!one_in_(6)) break;
        case 24:
            if (!never_forget)
            {
                msg_print("Huh? Who am I? What am I doing here?");

                lose_all_info();
                if (!one_in_(6)) break;
            }
        case 25:
            /*
             * Only summon Cyberdemons deep in the dungeon.
             */
            if ((dun_level > 65) && !stop_ty)
            {
                (*count) += summon_cyber(-1, py, px);
                stop_ty = TRUE;
                break;
            }
            if (!one_in_(6)) break;
        default:
            while (i < 6)
            {
                do
                {
                    (void)do_dec_stat(i);
                }
                while (one_in_(2));

                i++;
            }
        }
    }
    while (one_in_(3) && !stop_ty);

    return stop_ty;
}


int activate_hi_summon(int y, int x, bool can_pet)
{
    int i;
    int count = 0;
    int summon_lev;
    u32b mode = PM_ALLOW_GROUP;
    bool pet = FALSE;
    int kuka = (can_pet) ? SUMMON_WHO_PLAYER : SUMMON_WHO_NOBODY;

    if (can_pet)
    {
        if (one_in_(4))
        {
            mode |= PM_FORCE_FRIENDLY;
        }
        else
        {
            mode |= PM_FORCE_PET;
            pet = TRUE;
        }
    }

    if (!pet) mode |= PM_NO_PET;

    summon_lev = (pet ? p_ptr->lev * 2 / 3 + randint1(p_ptr->lev / 2) : dun_level);

    for (i = 0; i < (randint1(7) + (dun_level / 40)); i++)
    {
        int j, ct = 0;
        /* Make sure we get something on each roll ... */
        for (j = 0; j < 1000 && !ct; j++)
        {
            switch (randint1(25) + (dun_level / 20))
            {
                case 1: case 2:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_ANT, mode);
                    break;
                case 3: case 4:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_SPIDER, mode);
                    break;
                case 5: case 6:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_HOUND, mode);
                    break;
                case 7: case 8:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_HYDRA, mode);
                    break;
                case 9: case 10:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_ANGEL, mode);
                    break;
                case 11: case 12:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_UNDEAD, mode);
                    break;
                case 13: case 14:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_DRAGON, mode);
                    break;
                case 15: case 16:
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_DEMON, mode);
                    break;
                case 17:
                    if (can_pet) break;
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_AMBERITE, (mode | PM_ALLOW_UNIQUE));
                    break;
                case 18: case 19:
                    if (can_pet) break;
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_UNIQUE, (mode | PM_ALLOW_UNIQUE));
                    break;
                case 20: case 21:
                    if (!can_pet) mode |= PM_ALLOW_UNIQUE;
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_HI_UNDEAD, mode);
                    break;
                case 22: case 23:
                    if (!can_pet) mode |= PM_ALLOW_UNIQUE;
                    ct = summon_specific(kuka, y, x, summon_lev, SUMMON_HI_DRAGON, mode);
                    break;
                case 24:
                    ct = summon_specific(kuka, y, x, 100, SUMMON_CYBER, mode);
                    break;
                default:
                    if (!can_pet) mode |= PM_ALLOW_UNIQUE;
                    ct = summon_specific(kuka, y, x,pet ? summon_lev : (((summon_lev * 3) / 2) + 5), 0, mode);
            }
        }
        count += ct;
    }

    return count;
}


/* ToDo: check */
int summon_cyber(int who, int y, int x)
{
    int i;
    int max_cyber = dun_level / 50 + randint1(2);
    int count = 0;
    u32b mode = PM_ALLOW_GROUP;

    /* Summoned by a monster */
    if (who > 0)
    {
        monster_type *m_ptr = &m_list[who];
        if (is_pet(m_ptr)) mode |= PM_FORCE_PET;
    }

    if (max_cyber > 4) max_cyber = 4;

    for (i = 0; i < max_cyber; i++)
    {
        count += summon_specific(who, y, x, 100, SUMMON_CYBER, mode);
    }

    return count;
}


void wall_breaker(void)
{
    int i;
    int y = 0, x = 0;
    int attempts = 1000;

    if (randint1(80 + p_ptr->lev) < 70)
    {
        while (attempts--)
        {
            scatter(&y, &x, py, px, 4, 0);

            if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;

            if (!player_bold(y, x)) break;
        }

        project(0, 0, y, x, 20 + randint1(30), GF_KILL_WALL,
                  (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
    }
    else if (randint1(100) > 30)
    {
        earthquake(py, px, 1);
    }
    else
    {
        int num = damroll(5, 3);

        for (i = 0; i < num; i++)
        {
            while (1)
            {
                scatter(&y, &x, py, px, 10, 0);

                if (!player_bold(y, x)) break;
            }

            project(0, 0, y, x, 20 + randint1(30), GF_KILL_WALL,
                      (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
        }
    }
}


/*
 * Confuse monsters
 */
bool confuse_monsters(int dam)
{
    return (project_hack(GF_OLD_CONF, dam));
}


/*
 * Charm monsters
 */
bool charm_monsters(int dam)
{
    return (project_hack(GF_CHARM, dam));
}


/*
 * Charm animals
 */
bool charm_animals(int dam)
{
    return (project_hack(GF_CONTROL_ANIMAL, dam));
}


/*
 * Stun monsters
 */
bool stun_monsters(int dam)
{
    return (project_hack(GF_STUN, dam));
}


/*
 * Stasis monsters
 */
bool stasis_monsters(int dam)
{
    return (project_hack(GF_STASIS, dam));
}


/*
 * Mindblast monsters
 */
bool mindblast_monsters(int dam)
{
    return (project_hack(GF_PSI, dam));
}


/*
 * Banish all monsters
 */
bool banish_monsters(int dist)
{
    return (project_hack(GF_AWAY_ALL, dist));
}


/*
 * Turn evil
 */
bool turn_evil(int dam)
{
    return (project_hack(GF_TURN_EVIL, dam));
}


/*
 * Turn everyone
 */
bool turn_monsters(int dam)
{
    return (project_hack(GF_TURN_ALL, dam));
}


/*
 * Death-ray all monsters (note: OBSCENELY powerful)
 */
bool deathray_monsters(void)
{
    return (project_hack(GF_DEATH_RAY, p_ptr->lev * 200));
}


bool charm_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_CHARM, dir, plev, flg));
}


bool control_one_undead(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_CONTROL_UNDEAD, dir, plev, flg));
}


bool control_one_demon(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_CONTROL_DEMON, dir, plev, flg));
}


bool charm_animal(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_CONTROL_ANIMAL, dir, plev, flg));
}


bool charm_living(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_CONTROL_LIVING, dir, plev, flg));
}


bool kawarimi(bool success)
{
    int y, x;

    if (p_ptr->is_dead) return FALSE;
    if (p_ptr->confused || p_ptr->blind || p_ptr->paralyzed || p_ptr->image) return FALSE;
    if (randint0(200) < p_ptr->stun) return FALSE;

    if (!success && one_in_(3))
    {
        msg_print("Failed! You could not escape.");
        p_ptr->special_defense &= ~(NINJA_KAWARIMI);
        p_ptr->redraw |= (PR_STATUS);
        return FALSE;
    }

    y = py;
    x = px;

    teleport_player(5 + randint0(4) + 10 * randint0(8), 0L);

    if (p_ptr->pclass == CLASS_NINJA)
    {
        object_type forge;
        object_wipe(&forge);
        object_prep(&forge, lookup_kind(TV_STATUE, SV_WOODEN_STATUE));
        forge.pval = MON_NINJA;
        object_origins(&forge, ORIGIN_KAWARIMI);
        drop_near(&forge, -1, y, x);
    }

    if (success) msg_print("You have escaped just before the attack hit you.");
    else msg_print("Failed! You are hit by the attack.");

    p_ptr->special_defense &= ~(NINJA_KAWARIMI);
    p_ptr->redraw |= (PR_STATUS);

    return TRUE;
}


/*
 * "Rush Attack" routine for Samurai or Ninja
 * Return value is for checking "done"
 * Hacked up for Duelist as well.
 */
bool rush_attack(int rng, bool *mdeath)
{
    int dir;
    int tx, ty;
    int tm_idx = 0;
    u16b path_g[100];
    int path_n, i;
    bool tmp_mdeath = FALSE;
    bool moved = FALSE;

    if (mdeath) *mdeath = FALSE;

    project_length = rng;

    /* Mega Hack for the Duelist */
    if (p_ptr->pclass == CLASS_DUELIST)
    {
        /* assert(p_ptr->duelist_target_idx); */
        tm_idx = p_ptr->duelist_target_idx;
        tx = m_list[tm_idx].fx;
        ty = m_list[tm_idx].fy;

        if (!los(ty, tx, py, px))
        {
            msg_format("%^s is not in your line of sight.", duelist_current_challenge());
            return FALSE;
        }
    }
    else
    {
        if (!get_fire_dir(&dir)) return FALSE;

        /* Use the given direction */
        tx = px + project_length * ddx[dir];
        ty = py + project_length * ddy[dir];

        /* Hack -- Use an actual "target" */
        if ((dir == 5) && target_okay())
        {
            tx = target_col;
            ty = target_row;
        }

        if (in_bounds(ty, tx)) tm_idx = cave[ty][tx].m_idx;
    }

    path_n = project_path(path_g, project_length, py, px, ty, tx, PROJECT_STOP | PROJECT_KILL);
    project_length = 0;

    /* No need to move */
    if (!path_n) return TRUE;

    /* Use ty and tx as to-move point */
    ty = py;
    tx = px;

    /* Scrolling the cave would invalidate our path! */
    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
        wilderness_scroll_lock = TRUE;

    /* Project along the path */
    for (i = 0; i < path_n; i++)
    {
        monster_type *m_ptr;

        int ny = GRID_Y(path_g[i]);
        int nx = GRID_X(path_g[i]);

        if (cave_empty_bold(ny, nx) && player_can_enter(cave[ny][nx].feat, 0))
        {
            ty = ny;
            tx = nx;

            /* Go to next grid */
            continue;
        }

        if (!cave[ny][nx].m_idx)
        {
            if (tm_idx)
            {
                msg_print("Failed!");
            }
            else
            {
                msg_print("You can't move to that place.");
            }

            /* Exit loop */
            break;
        }

        /* Move player before updating the monster */
        if (!player_bold(ty, tx)) teleport_player_to(ty, tx, TELEPORT_NONMAGICAL | TELEPORT_RUSH_ATTACK);

        /* Update the monster */
        update_mon(cave[ny][nx].m_idx, TRUE);

        /* Found a monster */
        m_ptr = &m_list[cave[ny][nx].m_idx];

        if (tm_idx != cave[ny][nx].m_idx)
        {
            msg_format("There is %s in the way!", m_ptr->ml ? (tm_idx ? "another monster" : "a monster") : "someone");
        }
        else if (!player_bold(ty, tx))
        {
            /* Hold the monster name */
            char m_name[80];

            /* Get the monster name (BEFORE polymorphing) */
            monster_desc(m_name, m_ptr, 0);
            msg_format("You quickly jump in and attack %s!", m_name);
        }

        if (!player_bold(ty, tx)) teleport_player_to(ty, tx, TELEPORT_NONMAGICAL | TELEPORT_RUSH_ATTACK);
        moved = TRUE;
        tmp_mdeath = py_attack(ny, nx, HISSATSU_NYUSIN);

        break;
    }

    if (!moved && !player_bold(ty, tx)) teleport_player_to(ty, tx, TELEPORT_NONMAGICAL | TELEPORT_RUSH_ATTACK);
    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
    {
        wilderness_scroll_lock = FALSE;
        wilderness_move_player(px, py);
    }

    if (mdeath) *mdeath = tmp_mdeath;
    return TRUE;
}


