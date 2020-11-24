/* File: cmd1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Movement commands (part 1) */

#include "angband.h"
#include "equip.h"
#include <assert.h>
static int _max_vampiric_drain(void)
{
    if (prace_is_(RACE_MON_VAMPIRE) || prace_is_(MIMIC_BAT))
        return 100;
    return 50;
}

/* Changes from MPA ... but I also tweaked to incrementally biff AC.
     Author: elliptic
     Date:   Wed Mar 30 20:42:54 2016 -0400

     Rune sword changes.

     It starts at (-10,-10) now and grows faster (though still with the same
     monster level caps). The brand selection is a bit larger (and with tweaked
     chances). Aggravation and TY_CURSE are now permanent once you gain them
     (but you don't get TY_CURSE until 9d7/8d8/7d9 now instead of one step
     sooner). Untested, but compiles.

   Here are some test runs (Acuire Runesword on DL40 and 'play' to end game (about 7k kills)):
   a Rune Sword (9d6) (+32,+39) [-44] {cursed, Ag|EFCoP/D}
   a Rune Sword (7d8) (+33,+37) [-50] {cursed, Ag|EFCoPCaS}
   a Rune Sword (9d7) (+34,+37) [-39] {cursed, AgTy|A}
   a Rune Sword (8d9) (+43,+28) [-40] {cursed, AgTy|CoPCa/PLZ}
   a Rune Sword (9d6) (+36,+36) [-44] {cursed, Ag|FCoPS}

   See Wizard commands '-' and '=' in wizard2.c. Simply 'find' and wield a rune sword
   at the desired time (wizard command c0w) and then complete the statistics run.
*/
/*static*/ void rune_sword_kill(object_type *o_ptr, monster_race *r_ptr)
{
    if (o_ptr->curse_flags & OFC_PERMA_CURSE)
    {
        bool feed = FALSE;
        bool unique = (r_ptr->flags1 & RF1_UNIQUE);

        switch (randint1(4))
        {
        case 1:
            if ((r_ptr->level > o_ptr->to_h + o_ptr->to_d + 15))
            {
                if (unique || (o_ptr->to_h < 10 && one_in_(2)) || one_in_(3))
                {
                    if (o_ptr->to_h < 50 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->to_h++;
                    }
                }
            }
            break;

        case 2:
            if ((r_ptr->level > o_ptr->to_h + o_ptr->to_d + 15))
            {
                if (unique || (o_ptr->to_d < 10 && one_in_(2)) || one_in_(3))
                {
                    if (o_ptr->to_d < 50 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->to_d++;
                    }
                }
            }
            break;

        case 3:
            if ((r_ptr->level > o_ptr->dd * o_ptr->ds))
            {
                if (one_in_((o_ptr->dd + 1) * o_ptr->ds / 2) && (unique || one_in_(6)))
                {
                    if (o_ptr->dd < 9 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->dd++;
                        o_ptr->to_a -= randint1(3);
                    }
                }
            }
            break;

        case 4:
            if ((r_ptr->level > o_ptr->dd * o_ptr->ds))
            {
                if (one_in_(o_ptr->dd * (o_ptr->ds + 1) / 2) && (unique || one_in_(6)))
                {
                    if (o_ptr->ds < 9 || one_in_(666))
                    {
                        feed = TRUE;
                        o_ptr->ds++;
                        o_ptr->to_a -= randint1(3);
                    }
                }
            }
            break;
        }

        if (unique && (randint0(150) < r_ptr->level))
        {
            switch (randint1(13))
            {
            case 1:
                if (one_in_(6))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_POIS);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 2:
                if (one_in_(6))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_FIRE);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 3:
                if (one_in_(6))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_COLD);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 4:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_ELEC);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 5:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_UNDEAD);
                }
                break;
            case 6:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_DEMON);
                }
                break;
            case 7:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_DRAGON);
                }
                break;
            case 8:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_ACID);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 9:
                if (one_in_(12))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_GIANT);
                }
                break;
            case 10:
                if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_HUMAN);
                }
                break;
            case 11:
                if (one_in_(666))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_EVIL);
                    o_ptr->to_a -= randint1(5);
                }
                else if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_BRAND_CHAOS);
                }
                break;
            case 12:
                if (one_in_(666))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_VORPAL2);
                    o_ptr->to_a -= randint1(5);
                }
                else if (one_in_(24))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_VORPAL);
                    o_ptr->to_a -= randint1(5);
                }
                break;
            case 13:
                if (one_in_(12))
                {
                    feed = TRUE;
                    add_flag(o_ptr->flags, OF_SLAY_ANIMAL);
                }
                break;
            }
        }

        if (feed)
        {
            if (!have_flag(o_ptr->flags, OF_TY_CURSE)
              && o_ptr->dd * o_ptr->ds > 60 )
            {
                add_flag(o_ptr->flags, OF_TY_CURSE);
                msg_print("Your Rune Sword seeks to dominate you!");
            }
            else if (!have_flag(o_ptr->flags, OF_AGGRAVATE)
                   && o_ptr->dd * o_ptr->ds > 30 )
            {
                add_flag(o_ptr->flags, OF_AGGRAVATE);
                msg_print("The thirst of your sword redoubles!");
            }
            else
                msg_print("Your rune sword grows more powerful!");
        }
    }
    else
        msg_print("Only cursed Rune Swords may feed.");
}

void death_scythe_miss(object_type *o_ptr, int hand, int mode)
{
    u32b flgs[OF_ARRAY_SIZE];
    int k;
    critical_t crit;
    int dd = o_ptr->dd;
    int ds = o_ptr->ds;
    int to_h = 0;
    int to_d = 0;

    /* Sound */
    sound(SOUND_HIT);

    /* Message */
    if (hand == HAND_NONE) /* this is a thrown  weapon */
        cmsg_print(TERM_VIOLET, "Your scythe viciously slashes you!");
    else
    {
        cmsg_print(TERM_VIOLET, "Your scythe returns to you!");
        dd += p_ptr->weapon_info[hand].to_dd;
        ds += p_ptr->weapon_info[hand].to_ds;
        to_h += p_ptr->weapon_info[hand].to_h;
        to_d += p_ptr->weapon_info[hand].to_d;
    }

    /* Extract the flags */
    obj_flags(o_ptr, flgs);

    k = damroll(dd, ds);
    {
        int mult;
        switch (p_ptr->mimic_form)
        {
        case MIMIC_NONE:
            switch (p_ptr->prace)
            {
                case RACE_YEEK:
                case RACE_BOIT:
                case RACE_KLACKON:
                case RACE_HUMAN:
                case RACE_AMBERITE:
                case RACE_DUNADAN:
                case RACE_BARBARIAN:
                case RACE_BEASTMAN:
                case RACE_DEMIGOD:
                    mult = 25;break;
                case RACE_SNOTLING:
                case RACE_HALF_TROLL:
                case RACE_OGRE:
            	case RACE_HALF_ORC:
                case RACE_HALF_GIANT:
                case RACE_HALF_TITAN:
                case RACE_CYCLOPS:
                case RACE_IMP:
                case RACE_SKELETON:
                case RACE_ZOMBIE:
                case RACE_VAMPIRE:
                case RACE_MON_VAMPIRE:
                case RACE_SPECTRE:
                case RACE_BALROG:
                case RACE_DRACONIAN:
                case RACE_TONBERRY:
                case RACE_MON_LICH:
                case RACE_MON_DRAGON:
                    mult = 30;break;
                default:
                    mult = 10;break;
            }
            break;
        case MIMIC_DEMON:
        case MIMIC_DEMON_LORD:
        case MIMIC_VAMPIRE:
            mult = 30;break;
        default:
            mult = 10;break;
        }

        if (p_ptr->align < 0 && mult < 20)
            mult = 20;
        if (!res_save_default(RES_ACID) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_ELEC) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_FIRE) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_COLD) && mult < 25)
            mult = 25;
        if (!res_save_default(RES_POIS) && mult < 25)
            mult = 25;

        if ((have_flag(flgs, OF_BRAND_MANA) || p_ptr->tim_force) && (p_ptr->csp > (p_ptr->msp / 30)) && (!elemental_is_(ELEMENTAL_WATER)))
        {
            p_ptr->csp -= (1+(p_ptr->msp / 30));
            p_ptr->redraw |= (PR_MANA);
            mult = mult * 3 / 2 + 15;
        }

        k *= mult;
        k /= 10;
    }

    crit = critical_norm(o_ptr->weight, o_ptr->to_h, to_h, mode, hand);
    if (crit.desc)
    {
        k = k * crit.mul/100 + crit.to_d;
        msg_print(crit.desc);
    }

    if (one_in_(6))
    {
        int mult = 2;
        msg_format("Your weapon cuts deep into yourself!");
        /* Try to increase the damage */
        while (one_in_(4))
        {
            mult++;
        }

        k *= mult;
    }
    k += to_d + o_ptr->to_d;

    if (k < 0) k = 0;

    take_hit(DAMAGE_FORCE, k, "Death scythe");
}

/*
 * Determine if the player "hits" a monster (ranged combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
    int k;

    /* Never hit */
    if (chance <= 0) return (FALSE);
    if (melee_challenge) return (FALSE);

    /* Invisible monsters are harder to hit */
    if (!vis) chance = (chance + 1) / 2;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Punish lazy characters */
    if ((personality_is_(PERS_LAZY)) && (one_in_(20))) return (FALSE);
    if ((mut_present(MUT_HUMAN_CHR)) && (one_in_(20))) return (FALSE);

    /* Power competes against armor */
    if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
    int k;

    /* Wimpy attack never hits */
    if (chance <= 0) return (FALSE);

    /* Penalize invisible targets */
    if (!vis) chance = (chance + 1) / 2;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Punish lazy characters */
    if ((personality_is_(PERS_LAZY)) && (one_in_(20))) return (FALSE);
    if ((mut_present(MUT_HUMAN_CHR)) && (one_in_(20))) return (FALSE);

    /* Power must defeat armor */
    if (randint0(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}

s16b tot_dam_aux_monk(int tdam, monster_type *m_ptr, int mode)
{
    int mult = 10;
    int bonus = 0;
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    const int monk_elem_slay = 17;

    if (have_flag(p_ptr->weapon_info[0].flags, OF_BRAND_ACID) || mode == MYSTIC_ACID || mode == DRACONIAN_STRIKE_ACID)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_ACID_MASK);
        }
        else if (mode == MYSTIC_ACID)
        {
            mult = MAX(mult, 20);
            bonus = MAX(bonus, 5);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, OF_BRAND_ELEC) || mode == MYSTIC_ELEC || mode == DRACONIAN_STRIKE_ELEC)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
        }
        else if (mode == MYSTIC_ELEC)
        {
            mult = MAX(mult, 25);
            bonus = MAX(bonus, 7);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, OF_BRAND_FIRE) || mode == MYSTIC_FIRE || mode == DRACONIAN_STRIKE_FIRE)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
        }
        else
        {
            if (r_ptr->flags3 & RF3_HURT_FIRE)
            {
                mult = MAX(mult, 25);
                mon_lore_3(m_ptr, RF3_HURT_FIRE);
            }
            else if (mode == MYSTIC_FIRE)
            {
                mult = MAX(mult, 17);
                bonus = MAX(bonus, 3);
            }
            else
                mult = MAX(mult, monk_elem_slay);
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, OF_BRAND_COLD) || mode == MYSTIC_COLD || mode == DRACONIAN_STRIKE_COLD)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
        }
        else
        {
            if (r_ptr->flags3 & RF3_HURT_COLD)
            {
                mult = MAX(mult, 25);
                mon_lore_3(m_ptr, RF3_HURT_COLD);
            }
            else if (mode == MYSTIC_COLD)
            {
                mult = MAX(mult, 17);
                bonus = MAX(bonus, 3);
            }
            else
                mult = MAX(mult, monk_elem_slay);
        }
    }

    if (have_flag(p_ptr->weapon_info[0].flags, OF_BRAND_POIS) || mode == MYSTIC_POIS || mode == DRACONIAN_STRIKE_POIS)
    {
        if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_POIS_MASK);
        }
        else if (mode == MYSTIC_POIS)
        {
            mult = MAX(mult, 17);
            bonus = MAX(bonus, 3);
        }
        else
        {
            if (mult < monk_elem_slay) mult = monk_elem_slay;
        }
    }

    if (p_ptr->tim_force) /* Craft skillmaster martial artist. Craft Monks cannot learn Mana Branding */
    {
        int cost = 1 + tdam / 7; /* 100 -> 170 for +70 costs 15 (4.7 dmg/sp) */
        if (p_ptr->csp >= cost)
        {
            p_ptr->csp -= cost;
            p_ptr->redraw |= (PR_MANA);
            mult = mult * 12 / 10 + 5; /* 1.0x -> 1.7x; 1.7x -> 2.5x; 2.5x -> 3.5x */
        }
    }
    return tdam * mult / 10 + bonus;
}

/*
 * Search for hidden things
 */
void search(void)
{
    int y, x, chance;

    s16b this_o_idx, next_o_idx = 0;

    cave_type *c_ptr;


    /* Start with base search ability */
    chance = p_ptr->skills.srh;

    /* Penalize various conditions */
    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused || p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (y = (py - 1); y <= (py + 1); y++)
    {
        for (x = (px - 1); x <= (px + 1); x++)
        {
            /* Sometimes, notice things */
            if (randint0(100) < chance)
            {
                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Invisible trap */
                if (c_ptr->mimic && is_trap(c_ptr->feat))
                {
                    /* Pick a trap */
                    disclose_grid(y, x);

                    /* Message */
                    msg_print("You have found a trap.");

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Secret door */
                if (is_hidden_door(c_ptr))
                {
                    /* Message */
                    msg_print("You have found a secret door.");

                    /* Disclose */
                    disclose_grid(y, x);

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Scan all objects in the grid */
                for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                {
                    object_type *o_ptr;

                    /* Acquire object */
                    o_ptr = &o_list[this_o_idx];

                    /* Acquire next object */
                    next_o_idx = o_ptr->next_o_idx;

                    /* Skip non-chests */
                    if (o_ptr->tval != TV_CHEST) continue;

                    /* Skip non-trapped chests */
                    if (o_ptr->pval < 0 || !chest_traps[o_ptr->pval]) continue;

                    /* Identify once */
                    if (!object_is_known(o_ptr))
                    {
                        /* Message */
                        msg_print("You have discovered a trap on the chest!");

                        /* Know the trap */
                        obj_identify(o_ptr);

                        /* Notice it */
                        disturb(0, 0);
                    }
                }
            }
        }
    }
}

/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int _check_hit(int power)
{
    int k, ac;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- 5% hit, 5% miss */
    if (k < 10) return (k < 5);

    /* Punish lazy characters */
    if ((personality_is_(PERS_LAZY)) && (one_in_(20))) return (TRUE);

    /* Paranoia -- No power */
    if (power <= 0) return (FALSE);

    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;

    /* Power competes against Armor */
    if (randint1(power) > ((ac * 3) / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}



/*
 * Handle player hitting a real trap
 */
static void hit_trap(bool break_trap, bool do_jump)
{
    int i, num, dam;
    int x = px, y = py;

    /* Get the cave grid */
    cave_type *c_ptr = &cave[y][x];
    feature_type *f_ptr = &f_info[c_ptr->feat];
    int trap_feat_type = have_flag(f_ptr->flags, FF_TRAP) ? f_ptr->subtype : NOT_TRAP;

    cptr name = "a trap";

    /* Disturb the player */
    disturb(0, 0);

    cave_alter_feat(y, x, FF_HIT_TRAP);

    /* Analyze XXX XXX XXX */
    switch (trap_feat_type)
    {
        case TRAP_TRAPDOOR:
        {
            if (p_ptr->levitation && !do_jump)
            {
                msg_print("You fly over a trap door.");

            }
            else
            {
                msg_print("You have fallen through a trap door!");
                sound(SOUND_FALL);
                dam = damroll(2, 8);
                name = "a trap door";

                take_hit(DAMAGE_NOESCAPE, dam, name);

                /* Still alive and autosave enabled */
                if (autosave_l && (p_ptr->chp >= 0))
                    do_cmd_save_game(TRUE);

                prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);

                /* Leaving */
                p_ptr->leaving = TRUE;
            }
            break;
        }

        case TRAP_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a pit trap.");

            }
            else
            {
                msg_print("You have fallen into a pit!");

                dam = damroll(2, 6);
                name = "a pit trap";

                take_hit(DAMAGE_NOESCAPE, dam, name);
            }
            break;
        }

        case TRAP_SPIKED_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a spiked pit.");

            }
            else
            {
                msg_print("You fall into a spiked pit!");


                /* Base damage */
                name = "a pit trap";

                dam = damroll(2, 6);

                /* Extra spike damage */
                if (randint0(100) < 50)
                {
                    msg_print("You are impaled!");


                    name = "a spiked pit";

                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint1(dam), FALSE);
                }

                /* Take the damage */
                take_hit(DAMAGE_NOESCAPE, dam, name);
            }
            break;
        }

        case TRAP_POISON_PIT:
        {
            if (p_ptr->levitation)
            {
                msg_print("You fly over a spiked pit.");

            }
            else
            {
                msg_print("You fall into a spiked pit!");


                /* Base damage */
                dam = damroll(2, 6);

                name = "a pit trap";


                /* Extra spike damage */
                if (randint0(100) < 50)
                {
                    msg_print("You are impaled on poisonous spikes!");


                    name = "a spiked pit";


                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint1(dam), FALSE);

                    if (res_save_default(RES_POIS))
                        msg_print("The poison does not affect you!");
                    else
                    {
                        (void)set_poisoned(p_ptr->poisoned + dam, FALSE);
                    }
                }

                /* Take the damage */
                take_hit(DAMAGE_NOESCAPE, dam, name);
            }

            break;
        }

        case TRAP_TY_CURSE:
        {
            msg_print("There is a flash of shimmering light!");

            num = 2 + randint1(3);
            for (i = 0; i < num; i++)
            {
                (void)summon_specific(0, y, x, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            }

            if (dun_level > randint1(100)) /* No nasty effect for low levels */
            {
                bool stop_ty = FALSE;
                int count = 0;

                do
                {
                    stop_ty = activate_ty_curse(stop_ty, &count);
                }
                while (one_in_(6));
            }
            break;
        }

        case TRAP_TELEPORT:
        {
            msg_print("You hit a teleport trap!");

            teleport_player(100, TELEPORT_PASSIVE);
            break;
        }

        case TRAP_FIRE:
        {
            msg_print("You are enveloped in flames!");

            dam = damroll(4, 6);
            gf_affect_p(GF_WHO_TRAP, GF_FIRE, dam, GF_AFFECT_TRAP);
            break;
        }

        case TRAP_ACID:
        {
            msg_print("You are splashed with acid!");

            dam = damroll(4, 6);
            gf_affect_p(GF_WHO_TRAP, GF_ACID, dam, GF_AFFECT_TRAP);
            break;
        }

        case TRAP_SLOW:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap");

                if (!CHECK_MULTISHADOW() && !free_act_save_p(dun_level))
                    (void)set_slow(p_ptr->slow + randint0(20) + 20, FALSE);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_STR:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap");

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_STR);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_DEX:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap");

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_DEX);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_LOSE_CON:
        {
            if (_check_hit(125))
            {
                msg_print("A small dart hits you!");

                dam = damroll(1, 4);
                take_hit(DAMAGE_ATTACK, dam, "a dart trap");

                if (!CHECK_MULTISHADOW()) (void)do_dec_stat(A_CON);
            }
            else
            {
                msg_print("A small dart barely misses you.");

            }
            break;
        }

        case TRAP_BLIND:
        {
            msg_print("A black gas surrounds you!");
            if (!res_save_default(RES_BLIND))
                (void)set_blind(p_ptr->blind + randint0(50) + 25, FALSE);
            break;
        }

        case TRAP_CONFUSE:
        {
            msg_print("A gas of scintillating colors surrounds you!");
            if (!res_save_default(RES_CONF))
                (void)set_confused(p_ptr->confused + randint0(20) + 10, FALSE);
            break;
        }

        case TRAP_POISON:
        {
            msg_print("A pungent green gas surrounds you!");
            if (!res_save_default(RES_POIS))
                (void)set_poisoned(p_ptr->poisoned + randint0(20) + 10, FALSE);
            break;
        }

        case TRAP_SLEEP:
        {
            msg_print("A strange white mist surrounds you!");

            if (!free_act_save_p(0))
            {
                msg_print("You fall asleep.");


                if (ironman_nightmare)
                {
                    msg_print("A horrible vision enters your mind.");


                    /* Pick a nightmare */
                    get_mon_num_prep(get_nightmare, NULL);

                    /* Have some nightmares */
                    have_nightmare(get_mon_num(MAX_DEPTH));

                    /* Remove the monster restriction */
                    get_mon_num_prep(NULL, NULL);
                }
                (void)set_paralyzed(randint1(4), FALSE);
            }
            else equip_learn_flag(OF_FREE_ACT);
            break;
        }

        case TRAP_TRAPS:
        {
            msg_print("There is a bright flash of light!");

            /* Make some new traps */
            project(0, 1, y, x, 0, GF_MAKE_TRAP, PROJECT_HIDE | PROJECT_JUMP | PROJECT_GRID);

            break;
        }

        case TRAP_ALARM:
        {
            msg_print("An alarm sounds!");

            aggravate_monsters(0);

            break;
        }

        case TRAP_OPEN:
        {
            msg_print("Suddenly, surrounding walls are opened!");
            /*TODO: Fire beams in 4 principle directions that kill adjacent walls ... */
            project(0, 10, y, x, 0, GF_DISINTEGRATE, PROJECT_GRID | PROJECT_HIDE);
            aggravate_monsters(0);

            break;
        }

        case TRAP_ARMAGEDDON:
        {
            static int levs[10] = {0, 0, 20, 10, 5, 3, 2, 1, 1, 1};
            int evil_idx = 0, good_idx = 0;

            int lev;
            msg_print("Suddenly, you are surrounded by immotal beings!");

            /* Summon Demons and Angels */
            for (lev = dun_level; lev >= 20; lev -= 1 + lev/16)
            {
                num = levs[MIN(lev/10, 9)];
                for (i = 0; i < num; i++)
                {
                    int x1 = rand_spread(x, 7);
                    int y1 = rand_spread(y, 5);

                    /* Skip illegal grids */
                    if (!in_bounds(y1, x1)) continue;

                    /* Require line of projection */
                    if (!projectable(py, px, y1, x1)) continue;

                    if (summon_specific(0, y1, x1, lev, SUMMON_ARMAGE_EVIL, (PM_NO_PET)))
                        evil_idx = hack_m_idx_ii;

                    if (summon_specific(0, y1, x1, lev, SUMMON_ARMAGE_GOOD, (PM_NO_PET)))
                    {
                        good_idx = hack_m_idx_ii;
                    }

                    /* Let them fight each other */
                    if (evil_idx && good_idx)
                    {
                        monster_type *evil_ptr = &m_list[evil_idx];
                        monster_type *good_ptr = &m_list[good_idx];
                        evil_ptr->target_y = good_ptr->fy;
                        evil_ptr->target_x = good_ptr->fx;
                        good_ptr->target_y = evil_ptr->fy;
                        good_ptr->target_x = evil_ptr->fx;
                    }
                }
            }
            break;
        }

        case TRAP_PIRANHA:
        {
            msg_print("Suddenly, the room is filled with water with piranhas!");

            /* Water fills room */
            fire_ball_hide(GF_WATER_FLOW, 0, 1, 10);

            /* Summon Piranhas */
            num = 1 + dun_level/20;
            for (i = 0; i < num; i++)
            {
                (void)summon_specific(0, y, x, dun_level, SUMMON_PIRANHA, (PM_ALLOW_GROUP | PM_NO_PET));
            }
            break;
        }
        case TRAP_BEAR:
        {
            int prot = 0, slot = equip_find_first(object_is_helmet), dam = 40 + randint1(60);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                prot = o_ptr->ac + o_ptr->to_a;
                if (o_ptr->sval == SV_POINTY_HAT) prot *= 2;
            }
            if (!summon_named_creature(0, y, x, MON_DROP_BEAR, PM_NO_PET))
            {
                msg_print("A bear falls on top of you from the branches above, and disappears into the bushes!");
            }
            else msg_print("A bear falls on top of you from the branch above!");

            dam -= (prot * 3 / 2);
            if ((dam < 1) || ((IS_INVULN()) && (!p_ptr->ignore_invuln) &&
                (!one_in_(PENETRATE_INVULNERABILITY)))) msg_print("You shrug off the attack.");
            else
            {
                set_stun(p_ptr->stun + dam, TRUE);
                (void)take_hit(DAMAGE_USELIFE/* already checked invu */, dam, "a drop bear");
            }
            break;
        }
        case TRAP_ICICLE:
        {
            msg_print("You are hit by a falling icicle!");
            dam = damroll(5, 10) + 10;
            gf_affect_p(GF_WHO_TRAP, GF_ICE, dam, GF_AFFECT_TRAP);
            break;
        }
        case TRAP_BANANA:
        {
            if (!p_ptr->levitation)
            {
                msg_print("You slip on a banana peel!");
                dam = randint1(10);
                take_hit(DAMAGE_NOESCAPE, dam, "slipping on a banana peel");
                p_ptr->energy_need += ENERGY_NEED() * 3 / 4;
                if (magik(40))
                {
                    slot_t slot = pack_random_slot(obj_exists);
                    obj_ptr obj;
                    char o_name[MAX_NLEN];
                    if (!slot) break;
                    obj = pack_obj(slot);
                    if ((!obj) || (!obj->number)) break;
                    object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NO_PLURAL | OD_COLOR_CODED);
                    if (obj->number > 1)
                    {
                        msg_format("A %^s flies from your pack!", o_name);
                        command_arg = 1;
                    }
                    else msg_format("The %^s is dislodged from your pack and flies on the floor!", o_name);
                    silent_drop_hack = TRUE;
                    pack_drop(obj);
                    silent_drop_hack = FALSE;
                    msg_print(NULL);
                }
                break;
            }
            else
            {
                msg_print("You fly over the banana peel.");
                break;
            }
        }
    }

    if (break_trap && is_trap(c_ptr->feat))
    {
        cave_alter_feat(y, x, FF_DISARM);
        msg_print("You destroyed the trap.");
    }
}


void touch_zap_player(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int           i;

    /* beholders gaze on their enemies without touching (even as a ranged py_attack())
     * staffmasters gain a quick strike that avoids monster auras
     * other classes, like samurai, have a range 2 attack, but this is still a touch */
    if (p_ptr->prace == RACE_MON_BEHOLDER || p_ptr->lightning_reflexes)
        return;

    if ( (r_ptr->flags2 & RF2_AURA_REVENGE)
      && !retaliation_hack
      && !MON_CONFUSED(m_ptr)
      && !MON_PARALYZED(m_ptr)
      && randint0(150) < r_ptr->level )
    {
        retaliation_hack = TRUE;
        make_attack_normal(m_idx);
        retaliation_count++; /* Indexes which blow to use per retaliation, but start at 0 ... See py_attack() for initialization.*/
        retaliation_hack = FALSE;
    }

    if ((r_ptr->flags2 & (RF2_AURA_FIRE | RF2_AURA_ELEC)) || (r_ptr->flags3 & RF3_AURA_COLD))
    {
        int dd = 1 + r_ptr->level/26;
        int ds = 1 + r_ptr->level/17;
        int fire_dam = 0, cold_dam = 0, elec_dam = 0;

        if (r_ptr->flags2 & RF2_AURA_FIRE)
        {
            fire_dam = res_calc_dam(RES_FIRE, damroll(dd, ds));
            if (fire_dam > 0)
                mon_lore_2(m_ptr, RF2_AURA_FIRE);
        }
        if (r_ptr->flags3 & RF3_AURA_COLD)
        {
            cold_dam = res_calc_dam(RES_COLD, damroll(dd, ds));
            if (cold_dam > 0)
                mon_lore_3(m_ptr, RF3_AURA_COLD);
        }
        if (r_ptr->flags2 & RF2_AURA_ELEC)
        {
            elec_dam = res_calc_dam(RES_ELEC, damroll(dd, ds));
            if (elec_dam > 0)
                mon_lore_2(m_ptr, RF2_AURA_ELEC);
        }

        if (fire_dam + cold_dam + elec_dam)
        {
            char m_name[MAX_NLEN];
            char buf[100];

            buf[0] = '\0';
            if (fire_dam)
                strcat(buf, "<color:r>burned</color>");
            if (cold_dam)
            {
                if (strlen(buf))
                    strcat(buf, elec_dam ? ", " : " and ");
                strcat(buf, "<color:w>frozen</color>");
            }
            if (elec_dam)
            {
                if (strlen(buf))
                    strcat(buf, " and ");
                strcat(buf, "<color:b>shocked</color>");
            }

            msg_format("You are %s.", buf);
            monster_desc(m_name, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
            take_hit(DAMAGE_NOESCAPE, fire_dam + cold_dam + elec_dam, m_name);
            handle_stuff();
        }
    }

    for (i = 0; i < MAX_MON_AURAS; i++)
    {
        mon_effect_ptr aura = &r_ptr->auras[i];
        int            dam;
        if (!aura->effect) continue;
        if (aura->pct && randint1(100) > aura->pct) continue;
        dam = damroll(aura->dd, aura->ds);
        if (!dam) continue;
        gf_affect_p(m_idx, aura->effect, dam, GF_AFFECT_AURA);
        mon_lore_effect(m_ptr, aura);
    }
}

/* Fractional Blows: 2.75 is stored as 275 and should give 3 blows 75% of the time. */
static int _get_num_blow(int hand, int mode)
{
    int result;
    int num_blow = NUM_BLOWS(hand);

    if (mode == MAULER_CRITICAL_BLOW)
    {
        if (num_blow > 100)
            num_blow = 100 + (num_blow - 100) / 2;
    }

    result = num_blow / 100;

    if (randint0(100) < (num_blow % 100))
        result++;

    return result;
}

static int _get_num_blow_innate(int which)
{
    int num_blow = p_ptr->innate_attacks[which].blows;
    int result;

    if (which == 0)
        num_blow += p_ptr->innate_attack_info.xtra_blow;

    result = num_blow / 100;
    if (randint0(100) < (num_blow % 100))
        result++;

    return result;
}

static void do_monster_knockback(int x, int y, int dist);

void wizard_report_damage(int amt)
{
#if 0
    static int count = 0;
    static int total = 0;

    count++;
    total += amt;
    cmsg_format(TERM_L_RED, "You did %d damage (%d Avg).", amt, total / count);
#endif
}

static bool _gf_innate(mon_ptr m, int type, int dam)
{
    return gf_affect_m(GF_WHO_PLAYER, m, type, dam, GF_AFFECT_ATTACK);
}

static bool _pumpkin_drain_life(mon_ptr m_ptr, int *power, bool *weak)
{
    int vahennys = 0;
    *power = damroll(2, (*power) / 6);
    vahennys = ((*power)+7) / 8;
    m_ptr->maxhp -= vahennys;
    if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
    if (m_ptr->maxhp < 1) m_ptr->maxhp = 1;
    if (vahennys > 0) *weak = TRUE;
    return TRUE;
}

static bool _allow_crits = TRUE;

static void innate_attacks(s16b m_idx, bool *fear, bool *mdeath, int mode)
{
    int             dam, base_dam, effect_pow, to_h, chance;
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    byte            old_fy = m_ptr->fy, old_fx = m_ptr->fx;
    byte            old_py = py, old_px = px;
    int             old_hp = m_ptr->hp;
    int             old_r_idx = m_ptr->r_idx;
    char            m_name_subject[MAX_NLEN], m_name_object[MAX_NLEN];
    int             i, j, k;
    int             delay_sleep = 0;
    int             delay_stasis = 0;
    int             delay_paralysis = 0;
    bool            delay_quake = FALSE;
    int             drain_amt = 0;
    int             steal_ct = 0;
    int             hit_ct = 0;
    bool            weak = FALSE;
    const int       max_drain_amt = _max_vampiric_drain();
    bool            backstab = FALSE, fuiuchi = FALSE, stab_fleeing = FALSE, sleep_hit = FALSE;
    bool            do_werewolf_effect = (((p_ptr->prace == RACE_WEREWOLF) || (p_ptr->current_r_idx == MON_WEREWOLF)) && (r_ptr->flags7 & RF7_SILVER)) ? TRUE : FALSE;
    bool            is_gaze = FALSE;

    if ((MON_CSLEEP(m_ptr)) && (m_ptr->ml)) sleep_hit = TRUE;
    set_monster_csleep(m_idx, 0);

    monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE);
    monster_desc(m_name_object, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);

    if (p_ptr->afraid)
    {
        if (!fear_allow_melee(m_idx))
        {
            if (m_ptr->ml)
                cmsg_format(TERM_VIOLET, "You are too afraid to attack %s!", m_name_object);
            else
                cmsg_format(TERM_VIOLET, "There is something scary in your way!");
            return;
        }
    }

    /* Flavor ... Allow Ninjas and Rogues to backstab after metamorphosis */
    if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        switch (p_ptr->pclass)
        {
        case CLASS_ROGUE: /* <=== Burglary Book of Shadows currently has NINJA_S_STEALTH */
        case CLASS_NINJA_LAWYER: 
        case CLASS_NINJA:
            if (m_ptr->ml)
            {
                int tmp = p_ptr->lev * 6 + (p_ptr->skills.stl + 10) * 4;
                if (p_ptr->monlite && (mode != HISSATSU_NYUSIN)) tmp /= 3;
                if (p_ptr->cursed & OFC_AGGRAVATE) tmp /= 2;
                if (r_ptr->level > (p_ptr->lev * p_ptr->lev / 20 + 10)) tmp /= 3;
                if (sleep_hit)
                {
                    backstab = TRUE;
                }
                else if ( (p_ptr->special_defense & NINJA_S_STEALTH)
                       && randint0(tmp) > r_ptr->level + 20
                       && !(r_ptr->flagsr & RFR_RES_ALL) )
                {
                    fuiuchi = TRUE;
                }
                else if (MON_MONFEAR(m_ptr))
                {
                    stab_fleeing = TRUE;
                }
            }
            break;
        case CLASS_SCOUT:
            if (p_ptr->ambush)
            {
                if (sleep_hit && m_ptr->ml)
                    backstab = TRUE;
            }
            break;
        }
    }

    if ((p_ptr->pclass == CLASS_DUELIST) && ((!retaliation_hack) || (!p_ptr->duelist_target_idx)) && (m_ptr->maxhp > 100) && ((r_ptr->level >= (p_ptr->lev * 4 / 5)) || (m_ptr->maxhp > 1000)) && (m_idx != p_ptr->duelist_target_idx) && (!duelist_equip_error()))
    {
        p_ptr->duelist_target_idx = m_idx;
        msg_format("You challenge %s to a duel!", duelist_current_challenge());
        set_monster_csleep(m_idx, 0);
        set_hostile(&m_list[m_idx]);
        p_ptr->redraw |= PR_STATUS;
        return;
    }

    for (i = 0; i < p_ptr->innate_attack_ct; i++)
    {
        innate_attack_ptr a = &p_ptr->innate_attacks[i];
        int               blows = _get_num_blow_innate(i);
        if ((mode == BEORNING_SWIPE) || (mode == BEORNING_BIG_SWIPE)) blows = 1;

        if (a->flags & INNATE_SKIP) continue;
        if (m_ptr->fy != old_fy || m_ptr->fx != old_fx) break; /* Teleport Effect? */
        if (py != old_py || px != old_px) break;
        if (prace_is_(RACE_MON_BEHOLDER))
        {
            is_gaze = TRUE;
            if ((!a->name) || (!strpos("Gaze", a->name)))
            {
                is_gaze = FALSE;
                if (m_ptr->cdis > 1) continue;
            }
        }

        skills_innate_gain(skills_innate_calc_name(a), r_ptr->level);

        for (j = 0; j < blows; j++)
        {
            int ac = mon_ac(m_ptr);

            if (m_ptr->fy != old_fy || m_ptr->fx != old_fx) break; /* Teleport Effect? */

            to_h = a->to_h + p_ptr->to_h_m;
            chance = p_ptr->skills.thn + (to_h * BTH_PLUS_ADJ);
            if (p_ptr->stun)
                chance -= chance * MIN(100, p_ptr->stun) / 150;

            if (prace_is_(RACE_MON_GOLEM))
                ac = ac * (100 - p_ptr->lev) / 100;

            p_inc_fatigue(MUT_EASY_TIRING, 50);

            if ((fuiuchi) || ((sleep_hit) && (j == 0) && (personality_is_(PERS_SNEAKY))) || (test_hit_norm(chance, ac, m_ptr->ml)))
            {
                int dd = a->dd + p_ptr->innate_attack_info.to_dd;

                if (do_werewolf_effect) werewolf_silver_effect(ac / 4, FALSE);

                if (backstab) cmsg_format(TERM_L_GREEN, "You cruelly attack %s!", m_name_object);
                else if (fuiuchi) cmsg_format(TERM_L_GREEN, "You make a surprise attack, and hit %s with a powerful blow!", m_name_object);
                else if (stab_fleeing) cmsg_format(TERM_L_GREEN, "You attack %s in the back!",  m_name_object);

                hit_ct++;
                sound(SOUND_HIT);
                if (mode == BEORNING_BIG_SWIPE) msg_print("You hit.");
                else msg_format(a->msg, m_name_object);

                base_dam = damroll(dd, a->ds);
                if ((a->flags & INNATE_VORPAL) && one_in_(6))
                {
                    int m = 2;
                    while (one_in_(4))
                        m++;

                    base_dam *= m;
                    switch (m)
                    {
                    case 2: msg_format("You <color:U>gouge</color> %s!", m_name_object); break;
                    case 3: msg_format("You <color:y>maim</color> %s!", m_name_object); break;
                    case 4: msg_format("You <color:R>carve</color> %s!", m_name_object); break;
                    case 5: msg_format("You <color:r>cleave</color> %s!", m_name_object); break;
                    case 6: msg_format("You <color:v>smite</color> %s!", m_name_object); break;
                    case 7: msg_format("You <color:v>eviscerate</color> %s!", m_name_object); break;
                    default: msg_format("You <color:v>shred</color> %s!", m_name_object); break;
                    }
                }

                /* Slop for Draconian Metamorphosis ... */
                if (backstab)
                {
                    if (p_ptr->pclass == CLASS_SCOUT)
                        base_dam *= 3;
                    else
                        base_dam *= (3 + (p_ptr->lev / 20));
                }
                else if (fuiuchi)
                {
                    base_dam = base_dam*(5+(p_ptr->lev*2/25))/2;
                }
                else if (stab_fleeing)
                {
                    base_dam = (3 * base_dam) / 2;
                }

                base_dam += a->to_d;
                if ((!(a->flags & (INNATE_NO_DAM | INNATE_NO_CRIT))) && (_allow_crits))
                {
                    critical_t crit = critical_norm(a->weight, to_h, 0, mode, HAND_NONE);
                    if (crit.desc)
                    {
                        base_dam = base_dam * crit.mul/100 + crit.to_d;
                        msg_print(crit.desc);
                        if (mut_present(MUT_HUMAN_STR))
                        {
                            _allow_crits = FALSE;
                            energy_use += (energy_use / 5);
                        }
                    }
                }

                dam = base_dam + p_ptr->to_d_m;
                if (p_ptr->stun)
                {
                    dam -= dam * MIN(100, p_ptr->stun) / 150;
                    base_dam -= base_dam * MIN(100, p_ptr->stun) / 150;
                }
                dam = ((dam * (class_melee_mult() * race_melee_mult(TRUE) / 100)) + 50) / 100;
                base_dam = ((base_dam * (class_melee_mult() * race_melee_mult(TRUE) / 100)) + 50) / 100;

                if (mode == BEORNING_BIG_SWIPE)
                {
                    dam = (dam * (race_melee_mult(TRUE) + p_ptr->lev)) / 100;
                }

                /* More slop for Draconian Metamorphosis ... */
                if ( (player_is_ninja)
                  && (p_ptr->cur_lite <= 0 || one_in_(7)) )
                {
                    int maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
                    if (one_in_(backstab ? 13 : (stab_fleeing || fuiuchi) ? 15 : 27))
                    {
                        dam *= 5;
                        msg_format("You critically injured %s!", m_name_object);
                    }
                    else if ( (m_ptr->hp < maxhp/2 && one_in_(50))
                           || ( (one_in_(666) || ((backstab || fuiuchi) && one_in_(p_ptr->pclass == CLASS_NINJA ? 11 : 20)))
                             && !(r_ptr->flags1 & RF1_UNIQUE)
                             && !(r_ptr->flags7 & RF7_UNIQUE2)) )
                    {
                        if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_UNIQUE2) || (m_ptr->hp >= maxhp/2))
                        {
                            dam = MAX(dam*5, m_ptr->hp/2);
                            msg_format("You fatally injured %s!", m_name_object);
                        }
                        else
                        {
                            dam = m_ptr->hp + 1;
                            msg_format("You hit %s on a fatal spot!", m_name_object);
                        }
                    }
                }

                if (a->flags & INNATE_NO_DAM)
                {
                    base_dam = 0;
                    effect_pow = p_ptr->lev * 2;
                    dam = 0;
                }
                else
                    effect_pow = base_dam;

                if (dam < 0)
                    dam = 0;
                dam = mon_damage_mod(m_ptr, dam, FALSE);
                if (dam > 0)
                    anger_monster(m_ptr);

                for (k = 0; k < MAX_INNATE_EFFECTS && !*mdeath; k++)
                {
                    int e = a->effect[k];
                    int p = a->effect_chance[k];

                    if (m_ptr->fy != old_fy || m_ptr->fx != old_fx) break; /* Teleport Effect? */

                    if (p == 0) p = 100;
                    if (!e && k == 0)
                    {
                        if (a->flags & INNATE_NO_DAM)
                            continue;
                        e = GF_MISSILE;
                    }

                    /* Hack: When I decreased monster base resistance (89% -> 50%)
                       I inadvertantly made dragon bite attacks too strong. Let's
                       emulate the way branded weapons work, but only when the elemental
                       effect is added on top of some base effect (generally GF_MISSILE). */
                    if (k && mode == DRAGON_DEADLY_BITE)
                    {
                        switch (e)
                        {
                        case GF_ACID:
                            if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_ACID_MASK);
                                e = 0;
                            }
                            break;
                        case GF_FIRE:
                            if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
                                e = 0;
                            }
                            break;
                        case GF_COLD:
                            if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
                                e = 0;
                            }
                            break;
                        case GF_ELEC:
                            if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
                                e = 0;
                            }
                            break;
                        case GF_POIS:
                            if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_POIS_MASK);
                                e = 0;
                            }
                            break;
                        case GF_CONFUSION:
                            if (r_ptr->flags3 & RF3_NO_CONF)
                            {
                                mon_lore_3(m_ptr, RF3_NO_CONF);
                                e = 0;
                            }
                            break;
                        case GF_SOUND:
                            if (r_ptr->flagsr & RFR_RES_SOUN)
                            {
                                mon_lore_r(m_ptr, RFR_RES_SOUN);
                                e = 0;
                            }
                            break;
                        case GF_SHARDS:
                            if (r_ptr->flagsr & RFR_RES_SHAR)
                            {
                                mon_lore_r(m_ptr, RFR_RES_SHAR);
                                e = 0;
                            }
                            break;
                        case GF_NETHER:
                            if (r_ptr->flagsr & RFR_RES_NETH)
                            {
                                mon_lore_r(m_ptr, RFR_RES_NETH);
                                e = 0;
                            }
                            break;
                        case GF_NEXUS:
                            if (r_ptr->flagsr & RFR_RES_NEXU)
                            {
                                mon_lore_r(m_ptr, RFR_RES_NEXU);
                                e = 0;
                            }
                            break;
                        case GF_CHAOS:
                            if (r_ptr->flagsr & RFR_RES_CHAO)
                            {
                                mon_lore_r(m_ptr, RFR_RES_CHAO);
                                e = 0;
                            }
                            break;
                        case GF_DISENCHANT:
                            if (r_ptr->flagsr & RFR_RES_DISE)
                            {
                                mon_lore_r(m_ptr, RFR_RES_DISE);
                                e = 0;
                            }
                            break;
                        case GF_LITE:
                            if (r_ptr->flagsr & RFR_RES_LITE)
                            {
                                mon_lore_r(m_ptr, RFR_RES_LITE);
                                e = 0;
                            }
                            break;
                        case GF_DARK:
                            if (r_ptr->flagsr & RFR_RES_DARK)
                            {
                                mon_lore_r(m_ptr, RFR_RES_DARK);
                                e = 0;
                            }
                            break;
                        case GF_INERT:
                        	if (r_ptr->flags3 & RFR_RES_INER)
                        	{
                        		mon_lore_3(m_ptr, RFR_RES_INER);
                        		e = 0;
                        	}
                        	break;
                        }
                    }

                    if (!e) continue;
                    if (randint1(100) > p) continue;

                    if (p_ptr->current_r_idx == MON_AETHER_VORTEX)
                        e = vortex_get_effect();

                    switch (e)
                    {
                    case GF_MISSILE:
                        *mdeath = mon_take_hit(m_idx, dam, DAM_TYPE_MELEE, fear, NULL);
                        break;
                    case GF_DISENCHANT:
                        _gf_innate(m_ptr, e, k ? effect_pow : dam);
                        *mdeath = (m_ptr->r_idx == 0);
                        if (!(*mdeath) && one_in_(7))
                            dispel_monster_status(m_idx);
                        break;
                    case GF_OLD_SLEEP:
                        delay_sleep += effect_pow/2;
                        break;
                    case GF_STASIS:
                        delay_stasis += effect_pow;
                        break;
                    case GF_PARALYSIS:
                        delay_paralysis += effect_pow;
                        break;
                    case GF_OLD_CONF:
                    case GF_OLD_SLOW:
                    case GF_STUN:
                    {
                        int pow = (prace_is_(RACE_MON_GOLEM)) ? (effect_pow / (5 + (p_ptr->lev / 9) + (p_ptr->lev / 48) + randint1(4))) : effect_pow;
                        if (prace_is_(RACE_MON_PUMPKIN)) pow /= 2;
                        _gf_innate(m_ptr, e, pow);
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    }
                    case GF_DRAIN_MANA:
                    {
                        int amt = MIN(effect_pow, max_drain_amt - drain_amt);
                        if (amt && _gf_innate(m_ptr, e, amt))
                            drain_amt += amt;
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    }
                    case GF_OLD_DRAIN:
                        if (monster_living(r_ptr) &&
                            (((k > 0) && (_pumpkin_drain_life(m_ptr, &effect_pow, &weak))) ||
                            (_gf_innate(m_ptr, e, effect_pow))))
                        {
                            int amt = MIN(effect_pow, max_drain_amt - drain_amt);
                            if (prace_is_(MIMIC_BAT))
                            {
                                vampire_feed(amt);
                            }
                            else
                            {
                                msg_format("You <color:D>drain life</color> from %s!", m_name_object);
                                vampirism_hack = m_ptr->hp;
                                vamp_player(amt);
                            }
                            drain_amt += amt;
                        }
                        *mdeath = (m_ptr->r_idx == 0);
                        break;
                    case GF_STEAL:
                        if (leprechaun_steal(m_idx))
                            steal_ct++;
                        break;
                    case GF_QUAKE:
                        if (base_dam > 50 || one_in_(7))
                        {
                            delay_quake = TRUE;
                            _gf_innate(m_ptr, GF_STUN, base_dam);
                        }
                        break;
                    default:              /* v--- Check for pure elemental attack (e.g. Fire vortex) */
                        _gf_innate(m_ptr, e, k ? effect_pow : dam);
                        *mdeath = (m_ptr->r_idx == 0);
                        /* Polymorph effect? */
                        if (m_ptr->r_idx && m_ptr->r_idx != old_r_idx)
                        {
                            old_r_idx = m_ptr->r_idx;
                            monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE);
                            monster_desc(m_name_object, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
                        }
                    }
                }
                touch_zap_player(m_idx);
                check_muscle_sprains(300, "You pull a muscle!");

                if (a->flags & INNATE_EXPLODE)
                {
                    possessor_explode(dam);
                    return;
                }

                if (*mdeath)
                {
                    wizard_report_damage(old_hp);
                    return;
                }

                on_p_hit_m(m_idx);

                if (mode == DRAGON_SNATCH)
                {
                    msg_format("You grab %s in your jaws.", m_name_object);
                    monster_toss(m_idx);
                    return;
                }
            }
            else
            {
                sound(SOUND_MISS);
                if (is_gaze)
                    msg_format("%^s avoids your gaze.", m_name_subject);
                else
                {
                    msg_print("You miss.");
                    check_muscle_sprains(300, "You pull a muscle!");
                }
            }
            fuiuchi = FALSE; /* Clumsy! */

            if (mode == WEAPONMASTER_RETALIATION) break;
        }
        if (mode == WEAPONMASTER_RETALIATION) break;
    }
    if (((mode == DRAGON_TAIL_SWEEP) || (mode == BEORNING_BIG_SWIPE)) && (!*mdeath) && (hit_ct))
    {
        int dist = randint1((mode == BEORNING_BIG_SWIPE) ? 2 : 1 + p_ptr->lev/20);
        do_monster_knockback(m_ptr->fx, m_ptr->fy, dist);
    }
    if (delay_quake)
        earthquake(py, px, 10);
    if (delay_sleep && !*mdeath)
        _gf_innate(m_ptr, GF_OLD_SLEEP, delay_sleep);
    if (delay_stasis && !*mdeath)
        _gf_innate(m_ptr, GF_STASIS, delay_stasis);
    if (delay_paralysis && !*mdeath)
        _gf_innate(m_ptr, GF_PARALYSIS, delay_paralysis);
    if (weak && !(*mdeath))
        msg_format("%^s seems weakened.", m_name_subject);
    if (steal_ct && !*mdeath)
    {
        if (mon_save_p(m_ptr->r_idx, A_DEX))
            msg_print("You fail to run away!");
        else
            teleport_player(25 + p_ptr->lev/2, 0L);
    }

    wizard_report_damage(old_hp - m_ptr->hp);
}

/*
 * Player attacks a (poor, defenseless) creature        -RAK-
 */

static int drain_left = 0;
bool melee_hack = FALSE;
static bool fear_stop = FALSE;

static int calculate_dir(int sx, int sy, int tx, int ty)
{
    int dir;
    for (dir = 0; dir <= 9; dir++)
    {
        int x = sx + ddx[dir];
        int y = sy + ddy[dir];

        if (x == tx && y == ty) return dir;
    }
    return 5;
}

static int get_next_dir(int dir)
{
    switch (dir)
    {
    case 1: return 4;
    case 4: return 7;
    case 7: return 8;
    case 8: return 9;
    case 9: return 6;
    case 6: return 3;
    case 3: return 2;
    case 2: return 1;
    }
    return 5;
}

static void do_monster_knockback(int x, int y, int dist)
{
    monster_type   *m_ptr = &m_list[cave[y][x].m_idx];
    int             dir = calculate_dir(px, py, x, y);

    if (dir != 5)
    {
        int i;
        int msec = delay_time();

        for (i = 0; i < dist; i++)
        {
            int ty = y, tx = x;
            int oy = y, ox = x;

            y += ddy[dir];
            x += ddx[dir];
            if (cave_empty_bold(y, x))
            {
                ty = y;
                tx = x;
            }
            if (ty != oy || tx != ox)
            {
                int m_idx = cave[oy][ox].m_idx;

                cave[oy][ox].m_idx = 0;
                cave[ty][tx].m_idx = m_idx;
                m_ptr->fy = ty;
                m_ptr->fx = tx;

                Term_fresh();
                update_mon(m_idx, TRUE);
                lite_spot(oy, ox);
                lite_spot(ty, tx);

                if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                    p_ptr->update |= PU_MON_LITE;

                Term_xtra(TERM_XTRA_DELAY, msec);
                Term_fresh();
            }
            else
                break;
        }
    }
}
static cptr py_attack_desc(int mode)
{
    switch (mode)
    {
    case PY_POWER_ATTACK:
        return " <color:R>powerfully</color>";
    case DRAGON_DEADLY_BITE:
        return " with a <color:R>deadly</color> bite";
    }
    return "";
}

static int _many_strike_mon = 0;

static bool py_attack_aux(int y, int x, bool *fear, bool *mdeath, s16b hand, int mode)
{
    int             num = 0, k, k2 = 0, dam_tot = 0, bonus, chance;
    int             to_h = 0, to_d = 0;
    int             touch_ct = 0;
    critical_t      crit;
    cave_type      *c_ptr = &cave[y][x];
    monster_type   *m_ptr = NULL;
    monster_race   *r_ptr = NULL;
    object_type    *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    char            o_name[MAX_NLEN];
    u32b            flgs[OF_ARRAY_SIZE] = {0};
    char            m_name_subject[MAX_NLEN];
    char            m_name_object[MAX_NLEN];
    bool            success_hit = FALSE;
    bool            backstab = FALSE;
    bool            vorpal_cut = FALSE;
    int             chaos_effect = 0;
    bool            stab_fleeing = FALSE;
    bool            fuiuchi = FALSE;
    bool            monk_attack = FALSE;
    bool            duelist_attack = FALSE;
    bool            duelist_challenge = FALSE;
    bool            perfect_strike = FALSE;
    bool            sleep_hit = FALSE;
    bool            do_quake = FALSE;
    bool            weak = FALSE;
    bool            drain_msg = TRUE;
    int             drain_result = 0, drain_heal = 0;
    bool            can_drain = FALSE;
    int             num_blow;
    bool            is_human;
    bool            is_lowlevel;
    bool            zantetsu_mukou = FALSE, e_j_mukou = FALSE, bird_recoil = FALSE;
    int             knock_out = 0;
    int             dd, ds, old_hp;
    int             opy = py, opx = px;
    bool            hit_ct = 0;
    bool            poison_needle = FALSE;
    bool            insta_kill = FALSE;
    static bool     _reaper_lock = FALSE;
    static int      _reaper_bonus = 0;

    if (!c_ptr->m_idx)
    {
        msg_print("You swing wildly at nothing.");
        return FALSE;
    }

    m_ptr = &m_list[c_ptr->m_idx];
    old_hp = m_ptr->hp;
    r_ptr = &r_info[m_ptr->r_idx];
    is_human = (r_ptr->d_char == 'p');
    is_lowlevel = (r_ptr->level < (p_ptr->lev - 15));

    weapon_flags(hand, flgs);
    if (o_ptr)
    {
        object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_OMIT_PREFIX);
        if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH)
        {
            assert(o_ptr->tval == TV_SHIELD);
            dd = 3;
            ds = o_ptr->ac;
            to_h = o_ptr->to_a;
            to_d = o_ptr->to_a;

            to_h += 2*o_ptr->to_h;
            to_d += 2*o_ptr->to_d;
        }
        else if (p_ptr->prace == RACE_MON_ARMOR)
        {
            dd = 0;
            ds = 0;
            to_d = o_ptr->to_d;
            to_h = o_ptr->to_h;
        }
        else
        {
            dd = o_ptr->dd;
            ds = o_ptr->ds;
            to_h = o_ptr->to_h;
            to_d = o_ptr->to_d;
        }
        if (o_ptr->name1 == ART_ZANTETSU && r_ptr->d_char == 'j')
            zantetsu_mukou = TRUE;
        if (o_ptr->name1 == ART_EXCALIBUR_J && r_ptr->d_char == 'S')
            e_j_mukou = TRUE;
        if (o_ptr->name1 == ART_SKYNAIL && r_ptr->d_char == 'B')
            bird_recoil = TRUE;
    }
    else
    {
        sprintf(o_name, "Your fists");
        dd = 1;
        ds = 1;
        to_h = 0;
        to_d = 0;
        if (p_ptr->monk_lvl && !p_ptr->riding)
            monk_attack = TRUE;
    }

    if (p_ptr->painted_target)
    {
        p_ptr->painted_target_idx = 0;
        p_ptr->painted_target_ct = 0;
    }

    if ((personality_is_(PERS_SNEAKY)) && (MON_CSLEEP(m_ptr)) && (m_ptr->ml))
        sleep_hit = TRUE;

	switch (p_ptr->pclass)
	{
	case CLASS_DUELIST:
		if (c_ptr->m_idx == p_ptr->duelist_target_idx)
		{
			duelist_attack = TRUE;
		}
		else if (((!retaliation_hack) || (!p_ptr->duelist_target_idx)) && ((r_ptr->level >= p_ptr->lev * 4 / 5) || (m_ptr->maxhp >= p_ptr->lev * 9) || ((p_ptr->lev < 35) && (m_ptr->r_idx == MON_OCHRE_JELLY))) && (!duelist_equip_error()))
		{
			duelist_challenge = TRUE;
		}
        break;

    case CLASS_WEAPONMASTER:
        if (!p_ptr->sneak_attack)
            break;
        /* vvvvv FALL THRU vvvvvv */
    case CLASS_ROGUE:
    case CLASS_NINJA_LAWYER:
    case CLASS_NINJA:
        if (o_ptr && !p_ptr->weapon_info[hand].icky_wield)
        {
            int tmp = p_ptr->lev * 6 + (p_ptr->skills.stl + 10) * 4;
            if (p_ptr->monlite && (mode != HISSATSU_NYUSIN)) tmp /= 3;
            if (p_ptr->cursed & OFC_AGGRAVATE) tmp /= 2;
            if (r_ptr->level > (p_ptr->lev * p_ptr->lev / 20 + 10)) tmp /= 3;
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
            {
                /* Can't backstab creatures that we can't see, right? */
                backstab = TRUE;
            }
            else if ((p_ptr->special_defense & NINJA_S_STEALTH) && (randint0(tmp) > (r_ptr->level+20)) && m_ptr->ml && !(r_ptr->flagsr & RFR_RES_ALL))
            {
                fuiuchi = TRUE;
            }
            else if (MON_MONFEAR(m_ptr) && m_ptr->ml)
            {
                stab_fleeing = TRUE;
            }
        }
        break;

    case CLASS_SCOUT:
        if (p_ptr->ambush && o_ptr && !p_ptr->weapon_info[hand].icky_wield)
        {
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
                backstab = TRUE;
        }
        break;

    case CLASS_MONSTER:
        if (p_ptr->ambush && o_ptr)
        {
            if (MON_CSLEEP(m_ptr) && m_ptr->ml)
                backstab = TRUE;
        }
        break;
    case CLASS_SKILLMASTER:
        if (MON_CSLEEP(m_ptr) && m_ptr->ml) /* Works for Martial Arts as well */
        {
            if (p_ptr->ambush || (p_ptr->special_defense & NINJA_S_STEALTH))
                backstab = TRUE;
        }
        else if (p_ptr->special_defense & NINJA_S_STEALTH) /* Burglary Hide in Shadows */
        {
            int tmp = p_ptr->lev * 6 + (p_ptr->skills.stl + 10) * 4;
            if (randint0(tmp) > r_ptr->level + 20 && m_ptr->ml && !(r_ptr->flagsr & RFR_RES_ALL))
                fuiuchi = TRUE;
        }
        break;
    }

    if (o_ptr)
    {
        if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH && o_ptr->tval == TV_SHIELD)
            skills_shield_gain(o_ptr->sval, r_ptr->level);
        else if (!bird_recoil)
            skills_weapon_gain(o_ptr->tval, o_ptr->sval, r_ptr->level);
    }
    else
    {
        if (r_ptr->level + 10 > p_ptr->lev)
            skills_martial_arts_gain();
    }

    if (o_ptr && o_ptr->name1 == ART_ASSASSINATOR && MON_CSLEEP(m_ptr) && m_ptr->ml)
    {
        mode = ROGUE_ASSASSINATE;
    }

    /* Wake up monster */
    set_monster_csleep(c_ptr->m_idx, 0);
    monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE);
    monster_desc(m_name_object, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);

    if (mode == PY_POWER_ATTACK)
    {
        to_h += 10;
        to_d += p_ptr->lev / 2;
    }

    /* Calculate the "attack quality" */
    bonus = p_ptr->weapon_info[hand].to_h + to_h;
    if (mode == WEAPONMASTER_KNOCK_BACK) bonus -= 20;
    if (mode == WEAPONMASTER_REAPING) bonus -= 20;
    if (mode == WEAPONMASTER_CUNNING_STRIKE) bonus += 20;
    if (mode == WEAPONMASTER_SMITE_EVIL && hand == 0 && (r_ptr->flags3 & RF3_EVIL)) bonus += 200;
    if (duelist_attack) bonus += p_ptr->lev;

    chance = (p_ptr->skills.thn + (bonus * BTH_PLUS_ADJ));
    if (chance > 0)
        chance = chance * p_ptr->weapon_info[hand].dual_wield_pct / 1000;

    if (mode == HISSATSU_IAI) chance += 60;
    if (p_ptr->special_defense & KATA_KOUKIJIN) chance += 150;
    if (p_ptr->sutemi) chance = MAX(chance * 3 / 2, chance + 60);

    chance += virtue_current(VIRTUE_VALOUR) / 10;
    if (p_ptr->stun)
        chance -= chance * MIN(100, p_ptr->stun) / 150;

    num_blow = _get_num_blow(hand, mode);

    if (mode == HISSATSU_COLD) num_blow += 2;
    if (mode == WEAPONMASTER_FLURRY) num_blow *= 2;
    if (mode == WEAPONMASTER_CUNNING_STRIKE) num_blow = (num_blow + 1)/2;

    if (o_ptr && o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE)
    {
        poison_needle = TRUE;
        num_blow = 1;
    }

    /* Hack - message for new monster */
    if ((mode == WEAPONMASTER_MANY_STRIKE) && (c_ptr->m_idx != _many_strike_mon))
    {
        char m_name_norm[MAX_NLEN];
        monster_desc(m_name_norm, m_ptr, 0);
        cmsg_format(TERM_L_UMBER, "You attack %s:", m_name_norm);
        _many_strike_mon = c_ptr->m_idx;
    }

    /* Attack once for each legal blow */
    while ((num++ < num_blow) && !p_ptr->is_dead)
    {
        bool do_whirlwind = FALSE;

        water_mana_action(FALSE, (mode == PY_ATTACK_ACID) ? 24 : 18);

        if ((py != opy) || (px != opx))
        {
            fear_stop = TRUE; /* hack */
            break; /* Player has teleported */
        }

        if (!c_ptr->m_idx) /* A real thing that can happen */
        {
            msg_print("You swing wildly at nothing.");
            break;
        }

        /* We now check fear on every blow, and only lose energy equal to the number of blows attempted.
           Monsters with AURA_FEAR can induce fear any time the player damages them! */
        if (p_ptr->afraid)
        {
            if (!fear_allow_melee(c_ptr->m_idx))
            {
                if (m_ptr->ml)
                    msg_format("You are too afraid to attack %s!", m_name_object);
                else
                    msg_format("There is something scary in your way!");

                fear_stop = TRUE;
                break;
            }
        }

		/* If we just challenged we don't attack */
		if (duelist_challenge)
		{
			int m_idx = c_ptr->m_idx;
			p_ptr->duelist_target_idx = c_ptr->m_idx;
			msg_format("You challenge %s to a duel!", duelist_current_challenge());
			set_monster_csleep(m_idx, 0);
			set_hostile(&m_list[m_idx]);
			p_ptr->redraw |= PR_STATUS;
			break;
		}

        if (p_ptr->stun >= STUN_KNOCKED_OUT) /* Grand Master Mystic retaliation knocked the player out! */
            break;

        if (p_ptr->paralyzed)
            break;

        if (bird_recoil)
        {
            msg_print("You recoil at the thought of harming a bird!");
            break;
        }

        /* Check for easy tiring */
        p_inc_fatigue(MUT_EASY_TIRING, 50);

        /* Weaponmaster Whirlwind turns a normal strike into a sweeping whirlwind strike */
        if (p_ptr->whirlwind && mode == 0)
        {
            if (one_in_(5))
                do_whirlwind = TRUE;
        }

        if (poison_needle || mode == HISSATSU_KYUSHO || mode == MYSTIC_KILL)
        {
            int n = p_ptr->weapon_ct;

            if (mode == HISSATSU_3DAN)
                n *= 2;

            success_hit = one_in_(n);
        }
        else if (sleep_hit)
        {
            success_hit = TRUE;
            sleep_hit = FALSE;
        }
        else if (fuiuchi && !(r_ptr->flagsr & RFR_RES_ALL)) success_hit = TRUE;
        else if ((player_is_ninja) && ((backstab || fuiuchi) && !(r_ptr->flagsr & RFR_RES_ALL))) success_hit = TRUE;
        else if (duelist_attack && one_in_(2))
        {
            perfect_strike = TRUE;
            success_hit = TRUE;
        }
        else if (weaponmaster_get_toggle() == TOGGLE_BURNING_BLADE) success_hit = TRUE;
        else success_hit = test_hit_norm(chance, mon_ac(m_ptr), m_ptr->ml);

        if (mode == HISSATSU_MAJIN)
        {
            if (one_in_(2))
                success_hit = FALSE;
        }

        if (success_hit)
        {
            int vorpal_chance = 4;

            if (have_flag(flgs, OF_VORPAL2))
                vorpal_chance = 2;

            hit_ct++;
            sound(SOUND_HIT);

            /* Uber mega-hack ... aren't they all?! */
            if (weaponmaster_is_(WEAPONMASTER_STAVES))
            {
                bool update = FALSE;
                if (p_ptr->elaborate_defense == 0) update = TRUE;
                p_ptr->elaborate_defense = 1;
                if (update)
                {
                    p_ptr->update |= PU_BONUS;
                    p_ptr->redraw |= PR_ARMOR;
                    handle_stuff();
                }
            }

            if (backstab) cmsg_format(TERM_L_GREEN, "You cruelly attack %s!", m_name_object);
            else if (fuiuchi) cmsg_format(TERM_L_GREEN, "You make a surprise attack, and hit %s with a powerful blow!", m_name_object);
            else if (stab_fleeing) cmsg_format(TERM_L_GREEN, "You backstab %s!",  m_name_object);
            else if (perfect_strike) msg_format("You land a <color:G>perfect strike</color> against %s.", m_name_object);
            else if (!monk_attack) msg_format("You hit.", m_name_object);

            /* Hack -- bare hands do one damage */
            k = 1;

            if ((have_flag(flgs, OF_BRAND_CHAOS)) && one_in_(7))
            {
                if (one_in_(10)) virtue_add(VIRTUE_CHANCE, 1);
                if (randint1(5) < 4) chaos_effect = 1;
                else if (one_in_(5)) chaos_effect = 3;
            }

            /* Vampiric drain */
            if ( have_flag(flgs, OF_BRAND_VAMP)
              || chaos_effect == 1
              || mode == HISSATSU_DRAIN
              || mode == PY_ATTACK_VAMP
              || mode == WEAPONMASTER_REAPING
              || hex_spelling(HEX_VAMP_BLADE)
              || weaponmaster_get_toggle() == TOGGLE_BLOOD_BLADE
              || mauler_get_toggle() == MAULER_TOGGLE_DRAIN )
            {
                if (monster_living(r_ptr))
                    can_drain = TRUE;
            }

            vorpal_cut = FALSE;
            if (!zantetsu_mukou) /* No jelly cuts with Zantetsuken */
            {
                if (have_flag(flgs, OF_VORPAL) && p_ptr->vorpal && vorpal_chance > 3) vorpal_chance = 3;

                if ( have_flag(flgs, OF_VORPAL)
                  || have_flag(flgs, OF_VORPAL2)
                  || hex_spelling(HEX_RUNESWORD)
                  || p_ptr->vorpal
                  || mode == PY_ATTACK_VORPAL )
                {
                    if (randint1(vorpal_chance*3/2) == 1)
                        vorpal_cut = TRUE;
                }
            }

            if (monk_attack && mode != MYSTIC_KILL)
            {
                int special_effect = 0, stun_effect = 0;
                martial_arts *ma_ptr = &ma_blows[monk_get_attack_idx()];
                int resist_stun = 0;

                if (r_ptr->flags1 & RF1_UNIQUE) resist_stun += r_ptr->level;
                if (r_ptr->flags3 & RF3_NO_CONF) resist_stun += 33;
                if (r_ptr->flags3 & RF3_NO_SLEEP) resist_stun += 33;
                if ((r_ptr->flags3 & RF3_UNDEAD) || (r_ptr->flags3 & RF3_NONLIVING))
                    resist_stun += 66;

                k = damroll(ma_ptr->dd + p_ptr->weapon_info[hand].to_dd, ma_ptr->ds + p_ptr->weapon_info[hand].to_ds);
                k = tot_dam_aux_monk(k, m_ptr, mode);

                if (backstab || fuiuchi) /* skillmaster (stealthy or hiding in shadows) */
                {
                    int mult = 250 + p_ptr->lev * 4;
                    if (backstab) mult += 50;
                    k = k * mult / 100;
                }
                if (p_ptr->special_attack & ATTACK_SUIKEN) k *= 2; /* Drunken Boxing! */

                if (ma_ptr->effect == MA_KNEE)
                {
                    if (r_ptr->flags1 & RF1_MALE)
                    {
                        msg_format("You hit %s in the groin with your knee!", m_name_object);
                        sound(SOUND_PAIN);
                        special_effect = MA_KNEE;
                    }
                    else
                        msg_format(ma_ptr->desc, m_name_object);
                }

                else if (ma_ptr->effect == MA_SLOW)
                {
                    if (!((r_ptr->flags1 & RF1_NEVER_MOVE) ||
                        my_strchr("~#{}.UjmeEv$,DdsbBFIJQSXclnw!=?", r_ptr->d_char)))
                    {
                        msg_format("You kick %s in the ankle.", m_name_object);
                        special_effect = MA_SLOW;
                    }
                    else msg_format(ma_ptr->desc, m_name_object);
                }
                else
                {
                    if (ma_ptr->effect)
                        stun_effect = (ma_ptr->effect / 2) + randint1(ma_ptr->effect / 2);
                    if (mode == MYSTIC_STUN)
                        stun_effect *= 2;
                    msg_format(ma_ptr->desc, m_name_object);
                }

                if (_allow_crits)
                {
                    crit = monk_get_critical(ma_ptr, hand, mode);
                    if (crit.desc)
                    {
                        k = k * crit.mul/100 + crit.to_d;
                        msg_print(crit.desc);
                        if (mut_present(MUT_HUMAN_STR))
                        {
                            _allow_crits = FALSE;
                            energy_use += (energy_use / 5);
                        }
                    }
                }

                if ((special_effect == MA_KNEE) && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    msg_format("%^s moans in agony!", m_name_subject);
                    stun_effect = 7 + randint1(13);
                    resist_stun /= 3;
                }

                else if ((special_effect == MA_SLOW) && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    if (!(r_ptr->flags1 & RF1_UNIQUE) &&
                        (randint1(p_ptr->lev) > r_ptr->level) &&
                        m_ptr->mspeed > 60)
                    {
                        msg_format("%^s starts limping slower.", m_name_subject);
                        m_ptr->mspeed -= 10;
                    }
                }

                if (mode == DRACONIAN_STRIKE_STUN)
                    stun_effect *= 2;

                /* Massive Hack: Monk stunning is now greatly biffed! */
                if (r_ptr->flags1 & RF1_UNIQUE) stun_effect /= 2;
                if (r_ptr->flags3 & RF3_NO_STUN) stun_effect = 0;
                if (r_ptr->flagsr & RFR_RES_SOUN) stun_effect = 0;

                if (stun_effect && ((k + p_ptr->weapon_info[hand].to_d) < m_ptr->hp))
                {
                    if (p_ptr->lev > randint1(r_ptr->level + resist_stun + 10))
                    {
                        int cur_stun = MON_STUNNED(m_ptr);
                        if (cur_stun)
                        {
                            int div = 1 + cur_stun / 20;
                            stun_effect = MAX(1, stun_effect/div);
                        }

                        if (stun_effect == 0)
                        {
                            /* No message */
                        }
                        else if (set_monster_stunned(c_ptr->m_idx, cur_stun + stun_effect))
                            msg_format("%^s is stunned.", m_name_subject);
                        else
                            msg_format("%^s is more stunned.", m_name_subject);
                    }
                }
                if (mode == MYSTIC_KNOCKOUT)
                {
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                        msg_format("%^s is immune.", m_name_subject);
                    }
                    else if (mon_save_p(m_ptr->r_idx, A_DEX))
                    {
                        msg_format("%^s resists.", m_name_subject);
                    }
                    else
                    {
                        cmsg_format(TERM_VIOLET, "%^s is knocked out.", m_name_subject);
                        knock_out++;
                        /* No more retaliation this round! */
                        retaliation_count = 100; /* Any number >= 4 will do ... */
                    }
                }
            }
            /* Handle normal weapon */
            else if (o_ptr)
            {
                if (weaponmaster_get_toggle() == TOGGLE_ORDER_BLADE)
                    k = (dd + p_ptr->weapon_info[hand].to_dd) * (ds + p_ptr->weapon_info[hand].to_ds);
                else
                    k = damroll(dd + p_ptr->weapon_info[hand].to_dd, ds + p_ptr->weapon_info[hand].to_ds);
                k = tot_dam_aux(o_ptr, k, m_ptr, hand, mode, FALSE);

                if (backstab)
                {
                    if (p_ptr->pclass == CLASS_SCOUT)
                        k *= 3;
                    else
                        k *= (3 + (p_ptr->lev / 20));
                }
                else if (fuiuchi)
                {
                    k = k*(5+(p_ptr->lev*2/25))/2;
                }
                else if (stab_fleeing)
                {
                    k = (3 * k) / 2;
                }

                if (mode == MAULER_CRUSHING_BLOW)
                    k = k * NUM_BLOWS(hand) / 50;

                if (!poison_needle
                 && mode != HISSATSU_KYUSHO
                 && mode != MYSTIC_KILL
                 && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE
                 && _allow_crits
                 && !have_flag(flgs, OF_BRAND_ORDER) )
                {
                    int bonus = 0;
                    if (mode == WEAPONMASTER_SMITE_EVIL && hand == 0 && (r_ptr->flags3 & RF3_EVIL)) bonus = 200;
                    crit = critical_norm(o_ptr->weight, to_h, p_ptr->weapon_info[hand].to_h + bonus, mode, hand);
                    if (crit.desc)
                    {
                        k = k * crit.mul/100 + crit.to_d;
                        msg_print(crit.desc);
                        if (mut_present(MUT_HUMAN_STR))
                        {
                            _allow_crits = FALSE;
                            energy_use += (energy_use / 5);
                        }
                    }
                }

                if ( (have_flag(flgs, OF_IMPACT) && (k > 50 || one_in_(7)))
                  || chaos_effect == 2
                  || mode == HISSATSU_QUAKE
                  || (mauler_get_toggle() == MAULER_TOGGLE_SHATTER && (k > 50 || one_in_(7))) )
                {
                    do_quake = TRUE;
                    if ( k <= 50
                      || (r_ptr->flagsr & RFR_RES_ALL)
                      || (r_ptr->flags3 & RF3_NO_STUN) )
                    {
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name_subject);
                        mon_stun(m_ptr, mon_stun_amount(k));
                    }
                }

                if ((((have_flag(flgs, OF_STUN)) || (p_ptr->tim_field)) && randint1(100) < k) || ((o_ptr->name1 == ART_SILVER_HAMMER) && ((r_ptr->id == MON_WEREWOLF) || (r_ptr->id == MON_DRAUGLUIN) || (r_ptr->id == MON_CARCHAROTH))))
                {
                    if ( (r_ptr->flagsr & RFR_RES_ALL)
                      || (r_ptr->flags3 & RF3_NO_STUN) )
                    {
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name_subject);
                        mon_stun(m_ptr, mon_stun_amount(k));
                        obj_learn_slay(o_ptr, OF_STUN, "<color:o>Stuns</color> your enemies");
                    }
                }

                drain_result = k;
                k2 = k;

                if (vorpal_cut)
                {
                    int mult = 2;
                    if (o_ptr->name1 == ART_CHAINSWORD && !one_in_(2))
                    {
                        char chainsword_noise[1024];
                        if (!get_rnd_line("chainswd.txt", 0, chainsword_noise))
                            msg_print(chainsword_noise);
                    }

                    if (o_ptr->name1 == ART_VORPAL_BLADE)
                        msg_print("Your Vorpal Blade goes <color:y>snicker-snack</color>!");
                    else
                        msg_format("Your weapon <color:y>cuts deep</color> into %s!", m_name_object);

                    while (one_in_(vorpal_chance))
                        mult++;

                    k *= mult;

                    /* Ouch! */
                    if (((r_ptr->flagsr & RFR_RES_ALL) ? k/100 : k) > m_ptr->hp)
                        cmsg_format(TERM_VIOLET, "You cut %s in half!", m_name_object);
                    else
                    {
                        switch (mult)
                        {
                        case 2: msg_format("You <color:U>gouge</color> %s!", m_name_object); break;
                        case 3: msg_format("You <color:y>maim</color> %s!", m_name_object); break;
                        case 4: msg_format("You <color:R>carve</color> %s!", m_name_object); break;
                        case 5: msg_format("You <color:r>cleave</color> %s!", m_name_object); break;
                        case 6: msg_format("You <color:v>smite</color> %s!", m_name_object); break;
                        case 7: msg_format("You <color:v>eviscerate</color> %s!", m_name_object); break;
                        default: msg_format("You <color:v>shred</color> %s!", m_name_object); break;
                        }
                    }
                    drain_result = drain_result * 3 / 2;
                    if (have_flag(flgs, OF_VORPAL2))
                        obj_learn_slay(o_ptr, OF_VORPAL2, "is <color:v>*Sharp*</color>");
                    else
                        obj_learn_slay(o_ptr, OF_VORPAL, "is <color:R>Sharp</color>");
                }

                k += to_d;
                drain_result += to_d;
            }

            /* Apply the player damage bonuses */
            k += p_ptr->weapon_info[hand].to_d;
            drain_result += p_ptr->weapon_info[hand].to_d;

            if (mode == HISSATSU_SUTEMI || mode == HISSATSU_3DAN) k *= 2;
            if (mode == HISSATSU_SEKIRYUKA && !monster_living(r_ptr)) k = 0;
            if (mode == HISSATSU_SEKIRYUKA && !p_ptr->cut) k /= 2;

            if (k < 0) k = 0;

            if (mode == HISSATSU_ZANMA && !(!monster_living(r_ptr) && (r_ptr->flags3 & RF3_EVIL)))
                k = 0;

            if (zantetsu_mukou)
            {
                msg_print("You cannot cut such an elastic thing!");
                k = 0;
            }

            if (e_j_mukou)
            {
                msg_print("Spiders are difficult for you to deal with!");
                k /= 2;
            }

            if (mode == HISSATSU_MINEUCHI)
            {
                int tmp = (10 + randint1(15) + p_ptr->lev / 5);

                k = 0;
                anger_monster(m_ptr);

                if (!(r_ptr->flags3 & (RF3_NO_STUN)))
                {
                    if (MON_STUNNED(m_ptr))
                    {
                        msg_format("%^s is more dazed.", m_name_subject);
                        tmp /= 2;
                    }
                    else
                    {
                        msg_format("%^s is dazed.", m_name_subject);
                    }
                    (void)set_monster_stunned(c_ptr->m_idx, MON_STUNNED(m_ptr) + tmp);
                }
                else
                {
                    msg_format("%^s is not affected.", m_name_subject);
                }
            }

            /* Modify the damage */
            k = mon_damage_mod(
                m_ptr,
                k,
                (o_ptr && o_ptr->tval == TV_POLEARM && o_ptr->sval == SV_DEATH_SCYTHE)
             || (p_ptr->pclass == CLASS_BERSERKER && one_in_(2))
             || mode == WEAPONMASTER_CRUSADERS_STRIKE
            );

            if (duelist_attack)
            {
                int d = k;
                
                if (p_ptr->lev >= 10) /* Careful Aim */
                {
                    k += d;
                }
                if ( p_ptr->lev >= 15    /* Hamstring */
                    && !(r_ptr->flags1 & (RF1_UNIQUE))
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("You <color:y>hamstring</color> %s.", m_name_object);
                    set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50);
                }
                if ( p_ptr->lev >= 25    /* Stunning Blow */
                    && !(r_ptr->flags3 & (RF3_NO_STUN))
                    && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a <color:B>stunning</color> blow.", m_name_subject);
                    mon_stun(m_ptr, mon_stun_amount(d));
                }
                if ( p_ptr->lev >= 20    /* Wounding Strike */
                  && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a <color:r>wounding</color> strike.", m_name_subject);
                    k += pienempi(m_ptr->hp / 5, randint1(3) * d);
                    drain_result = k;
                }
                if ( p_ptr->lev >= 40    /* Greater Wounding Strike */
                  && !mon_save_p(m_ptr->r_idx, A_DEX) )
                {
                    msg_format("%^s is dealt a <color:v>*WOUNDING*</color> strike.", m_name_subject);
                    k += pienempi(m_ptr->hp * 2 / 5, rand_range(2, 6) * d);
                    drain_result = k;
                }
            }

            /* Adjust for race/class penalties */
            k = ((k * (class_melee_mult() * race_melee_mult(FALSE) / 100)) + 50) / 100;

            if (poison_needle || mode == HISSATSU_KYUSHO || mode == MYSTIC_KILL)
            {
                if ( randint1(randint1(r_ptr->level/7)+5) == 1
                  && (!(r_ptr->flags1 & RF1_UNIQUE) || m_ptr->r_idx == MON_HAGURE2)
                  && !(r_ptr->flags7 & RF7_UNIQUE2))
                {
                    k = m_ptr->hp + 1;
                    msg_format("You hit %s on a fatal spot!", m_name_object);
                    insta_kill = TRUE;
                }
                else k = 1;
            }
            else if ( (player_is_ninja)
                   && (o_ptr)
                   && (!p_ptr->weapon_info[hand].icky_wield)
                   && ((p_ptr->cur_lite <= 0 || one_in_(7))) )
            {
                int maxhp = maxroll(r_ptr->hdice, r_ptr->hside);

                if (one_in_(backstab ? 13 : (stab_fleeing || fuiuchi) ? 15 : 27))
                {
                    k *= 5;
                    drain_result *= 2;
                    msg_format("You critically injured %s!", m_name_object);
                }
                else if ( (m_ptr->hp < maxhp/2 && one_in_(num_blow*10))
                       || ( (one_in_(666) || ((backstab || fuiuchi) && one_in_(p_ptr->pclass == CLASS_NINJA ? 11 : 20)))
                         && !(r_ptr->flags1 & RF1_UNIQUE)
                         && !(r_ptr->flags7 & RF7_UNIQUE2)) )
                {
                    if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_UNIQUE2) || (m_ptr->hp >= maxhp/2))
                    {
                        k *= 5; 
                        if (p_ptr->pclass == CLASS_NINJA) k = MAX(k, m_ptr->hp/2);
                        drain_result *= 2;
                        msg_format("You fatally injured %s!", m_name_object);
                    }
                    else
                    {
                        k = m_ptr->hp + 1;
                        msg_format("You hit %s on a fatal spot!", m_name_object);
                        insta_kill = TRUE;
                    }
                }
            }

            if (k <= 0) can_drain = FALSE;

            if (drain_result > m_ptr->hp)
                drain_result = m_ptr->hp;

            if (have_flag(flgs, OF_BRAND_WILD))
            {
                wild_weapon_strike();
                obj_learn_slay(o_ptr, OF_BRAND_WILD, "is a <color:o>Wild Weapon</color>");
            }

            if (mode == ROGUE_ASSASSINATE)
            {
                if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_UNIQUE2) || mon_save_p(m_ptr->r_idx, A_DEX))
                {
                    k = MAX(k*5, m_ptr->hp/2);
                    drain_result *= 2;
                    msg_format("You fatally injured %s!", m_name_object);
                }
                else
                {
                    k = m_ptr->hp + 1;
                    msg_format("You hit %s on a fatal spot!", m_name_object);
                    insta_kill = TRUE;
                }
            }

            if (p_ptr->pclass == CLASS_WEAPONMASTER)
            {
                if (mode == WEAPONMASTER_VICIOUS_STRIKE)
                    k *= 2;

                if (mode == WEAPONMASTER_REAPING)
                {
                    k += (int)(((s32b)k) * (NUM_BLOWS(hand) + 200L) / 300L);
                    if (!_reaper_lock) k += (k / 4);
                    else k += (k * _reaper_bonus / 100);
                }
                    
                switch(weaponmaster_get_toggle())
                {
                case TOGGLE_SHIELD_BASH:
                    if (one_in_(12))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            mon_lore_r(m_ptr, RFR_RES_ALL);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (r_ptr->flags3 & RF3_NO_STUN)
                        {
                            mon_lore_3(m_ptr, RF3_NO_STUN);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (mon_stun_save(r_ptr->level, k))
                        {
                            msg_format("%^s resists.", m_name_subject);
                        }
                        else
                        {
                            msg_format("%^s is stunned.", m_name_subject);
                            mon_stun(m_ptr, mon_stun_amount(k));
                        }
                    }
                    break;

                case TOGGLE_BURNING_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_FIRE)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_IM_FIRE);
                    }
                    else if (r_ptr->flagsr & RFR_RES_FIRE)
                    {
                        msg_format("%^s resists.", m_name_subject);
                        k /= 3;
                        mon_lore_r(m_ptr, RFR_RES_FIRE);
                    }
                    else if (r_ptr->flags3 & (RF3_HURT_FIRE))
                    {
                        msg_format("%^s is hit hard.", m_name_subject);
                        k *= 2;
                        mon_lore_3(m_ptr, RF3_HURT_FIRE);
                    }
                    break;
                case TOGGLE_ICE_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_COLD)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_IM_COLD);
                    }
                    else if (r_ptr->flagsr & RFR_RES_COLD)
                    {
                        msg_format("%^s resists.", m_name_subject);
                        k /= 3;
                        mon_lore_r(m_ptr, RFR_RES_COLD);
                    }
                    else
                    {
                        if (r_ptr->flags3 & (RF3_HURT_COLD))
                        {
                            msg_format("%^s is hit hard.", m_name_subject);
                            k *= 2;
                            mon_lore_3(m_ptr, RF3_HURT_COLD);
                        }
                        if (one_in_(5)
                            && !(r_ptr->flags1 & (RF1_UNIQUE))
                            && !mon_save_p(m_ptr->r_idx, A_STR) )
                        {
                            msg_format("%^s is slowed by the cold.", m_name_subject);
                            set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50);
                        }
                    }
                    break;
                case TOGGLE_THUNDER_BLADE:
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                    }
                    else if (r_ptr->flagsr & RFR_IM_ELEC)
                    {
                        msg_format("%^s is immune.", m_name_subject);
                        k = 0;
                        mon_lore_r(m_ptr, RFR_IM_ELEC);
                    }
                    else if (r_ptr->flagsr & RFR_RES_ELEC)
                    {
                        msg_format("%^s resists.", m_name_subject);
                        k /= 3;
                        mon_lore_r(m_ptr, RFR_RES_ELEC);
                    }
                    else
                    {
                        if ( one_in_(5)
                          && !(r_ptr->flags3 & RF3_NO_STUN)
                          && !mon_stun_save(r_ptr->level, k) )
                        {
                            msg_format("%^s is shocked convulsively.", m_name_subject);
                            mon_stun(m_ptr, mon_stun_amount(k));
                        }
                    }
                    break;
                case TOGGLE_WILD_BLADE:
                    wild_weapon_strike(); /* TODO: Add EGO_WILD_BLADE and TR_WILD for weapons */
                    break;
                }
            }

            if (mode == WEAPONMASTER_CRUSADERS_STRIKE)
                k = k * 3 / 2;

            if ((p_ptr->stun) && (!insta_kill))
                k -= k * MIN(100, p_ptr->stun) / 150;

            dam_tot += k;

            check_muscle_sprains(500, "You sprain a muscle!");

            /* Damage, check for fear and death */
            if (mon_take_hit(c_ptr->m_idx, k, DAM_TYPE_MELEE, fear, NULL))
            {
                *mdeath = TRUE;

                if (mauler_get_toggle() == MAULER_TOGGLE_SPLATTER)
                {
                    project(0, 2, y, x, k,
                            GF_BLOOD, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_FULL_DAM);
                }

                if ( o_ptr
                  && o_ptr->tval == TV_SWORD
                  && o_ptr->sval == SV_RUNESWORD )
                {
                    rune_sword_kill(o_ptr, r_ptr);
                }

                if (duelist_attack)
                {
                    p_ptr->duelist_target_idx = 0;
                    p_ptr->redraw |= PR_STATUS;

                    if (p_ptr->lev >= 35 && duelist_can_challenge())    /* Endless Duel */
                    {
                        /* Hacks so that get_fire_dir() actually allows user to select a new target */
                        target_who = 0;
                        command_dir = 0;
                        msg_print("Your chosen target is vanquished!  Select another.");
                        msg_print(NULL);
                        duelist_issue_challenge();
                    }
                    else
                        msg_print("Your chosen target is vanquished!");
                }

                if ((p_ptr->pclass == CLASS_BERSERKER || mut_present(MUT_FANTASTIC_FRENZY) || beorning_is_(BEORNING_FORM_BEAR)) && energy_use)
                {
                    int ct = MAX(1, p_ptr->weapon_ct); /* paranoia ... if we are called with 0, that is a bug (I cannot reproduce) */
                    int frac = (_allow_crits) ? 100/ct : 120/ct; /* Perhaps the 'zerker leveled up to 35 in the middle of a round of attacks? */

                    energy_use = 0;
                    if (hand) /* hand is 0, 1, ... so hand is the number of successful rounds of attacks so far */
                        energy_use += hand * frac;
                    energy_use += num * frac / num_blow;
                }
                if (mode == WEAPONMASTER_REAPING)
                {
                    _reaper_bonus += 15;
                    goto weaponmaster_reap;
                }
                if (o_ptr && o_ptr->name1 == ART_ZANTETSU && is_lowlevel)
                    msg_print("Sigh... Another trifling thing I've cut....");
                break;
            }
            else
            {
                if (o_ptr && o_ptr->name1 == ART_ETERNAL_BLADE)
                {
                    /* Hack: Time Brand. Effectively a 2x slay ... k2 is damage from dice alone.*/
                    gf_affect_m(GF_WHO_PLAYER, m_ptr, GF_TIME, k2, GF_AFFECT_ATTACK);
                    *mdeath = (m_ptr->r_idx == 0);
                    if (*mdeath) break;
                }

                if (mauler_get_toggle() == MAULER_TOGGLE_DRAIN)
                {
                    int amt = (k+2)/3;
                    m_ptr->maxhp -= amt;
                    msg_format("%^s seems weakened.", m_name_subject);
                }
                if (mode == MAULER_STUNNING_BLOW || mode == DRACONIAN_STRIKE_STUN)
                {
                    if (r_ptr->flagsr & RFR_RES_ALL)
                    {
                        mon_lore_r(m_ptr, RFR_RES_ALL);
                        msg_format("%^s is immune.", m_name_subject);
                    }
                    else if (r_ptr->flags3 & RF3_NO_STUN)
                    {
                        mon_lore_3(m_ptr, RF3_NO_STUN);
                        msg_format("%^s is immune.", m_name_subject);
                    }
                    else if ((r_ptr->flags1 & RF1_UNIQUE) && mon_stun_save(r_ptr->level, k))
                    {
                        msg_format("%^s resists.", m_name_subject);
                    }
                    else
                    {
                        msg_format("%^s is stunned.", m_name_subject);
                        mon_stun(m_ptr, mon_stun_amount(k));
                    }
                }
                if (mode == MELEE_AWESOME_BLOW)
                {
                    int dir = calculate_dir(px, py, x, y);
                    if (dir != 5)
                    {
                        int ct = 0;
                        int max = 3;
                        int m_idx = c_ptr->m_idx;
                        int ty = y, tx = x;
                        int oy = y, ox = x;

                        if (p_ptr->pclass == CLASS_RAGE_MAGE)
                        {
                            if (p_ptr->shero)
                                max = 6;
                        }
                        else if (p_ptr->pclass == CLASS_MAULER && o_ptr)
                        {
                            int w = o_ptr->weight;
                            max = MIN(p_ptr->lev/5, w/40);
                        }

                        for (ct = 0; ct < max; ct++)
                        {
                            y += ddy[dir];
                            x += ddx[dir];
                            if (!cave_empty_bold(y, x))
                            {
                                int dam = 50;

                                if ( cave[y][x].m_idx
                                  || cave_have_flag_bold(y, x, FF_TREE)
                                  || cave[y][x].feat == feat_rubble
                                  || cave[y][x].feat == feat_dark_pit )
                                {
                                    dam = 25;
                                }
                                msg_format("%^s is wounded.", m_name_subject);
                                mon_take_hit(m_idx, dam * (max - ct), DAM_TYPE_MELEE, fear, NULL);
                                break;
                            }
                            else
                            {
                                ty = y;
                                tx = x;
                            }
                            if (ty != oy || tx != ox)
                            {
                                int m_idx = cave[oy][ox].m_idx;

                                cave[oy][ox].m_idx = 0;
                                cave[ty][tx].m_idx = m_idx;
                                m_ptr->fy = ty;
                                m_ptr->fx = tx;

                                update_mon(m_idx, TRUE);
                                lite_spot(oy, ox);
                                lite_spot(ty, tx);

                                oy = ty;
                                ox = tx;

                                if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                                    p_ptr->update |= PU_MON_LITE;
                            }
                        }
                    }
                }

                if (mode == WEAPONMASTER_CRUSADERS_STRIKE)
                {
                    msg_format("Your Crusader's Strike drains life from %s!", m_name_object);
                    hp_player(MIN(k, 150));
                }

                /* Clubmaster Hacks. We do these effects *after* the monster takes damage. */
                if (weaponmaster_is_(WEAPONMASTER_CLUBS) && p_ptr->speciality_equip)
                {
                    int odds = 5;

                    if (mode == WEAPONMASTER_CUNNING_STRIKE)
                        odds = 2;

                    if (one_in_(odds))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            mon_lore_r(m_ptr, RFR_RES_ALL);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (r_ptr->flags3 & RF3_NO_CONF)
                        {
                            mon_lore_3(m_ptr, RF3_NO_CONF);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name_subject);
                        }
                        else
                        {
                            msg_format("%^s appears <color:U>confused</color>.", m_name_subject);
                            set_monster_confused(c_ptr->m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
                        }
                    }

                    if (p_ptr->lev >= 20 && one_in_(odds))
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            mon_lore_r(m_ptr, RFR_RES_ALL);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (r_ptr->flags3 & RF3_NO_SLEEP)
                        {
                            mon_lore_3(m_ptr, RF3_NO_SLEEP);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (mon_save_p(m_ptr->r_idx, A_STR))
                        {
                            msg_format("%^s resists.", m_name_subject);
                        }
                        else
                        {
                            msg_format("%^s is <color:b>knocked out</color>.", m_name_subject);
                            knock_out++;
                            /* No more retaliation this round! */
                            retaliation_count = 100; /* Any number >= 4 will do ... */
                        }
                    }

                    if ( mode == WEAPONMASTER_CUNNING_STRIKE
                      || (p_ptr->lev >= 45 && one_in_(odds)) )
                    {
                        if (r_ptr->flagsr & RFR_RES_ALL)
                        {
                            mon_lore_r(m_ptr, RFR_RES_ALL);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (r_ptr->flags3 & RF3_NO_STUN)
                        {
                            mon_lore_3(m_ptr, RF3_NO_STUN);
                            msg_format("%^s is immune.", m_name_subject);
                        }
                        else if (mon_stun_save(r_ptr->level, k))
                        {
                            msg_format("%^s resists.", m_name_subject);
                        }
                        else
                        {
                            msg_format("%^s is <color:B>stunned</color>.", m_name_subject);
                            mon_stun(m_ptr, mon_stun_amount(k));
                        }
                    }
                }
            }

            if (k > 0) anger_monster(m_ptr);

            touch_zap_player(c_ptr->m_idx);
            touch_ct++;
            /* XXX if (distance(py, px, m_ptr->fy, m_ptr->fx) > 1)  monster aura moved us!
                return success_hit; */

            if (can_drain && (drain_result > 0))
            {
                if (o_ptr && o_ptr->name1 == ART_MURAMASA)
                {
                    if (is_human)
                    {
                        int to_h = o_ptr->to_h;
                        int to_d = o_ptr->to_d;
                        int i, flag;

                        flag = 1;
                        for (i = 0; i < to_h + 3; i++) if (one_in_(4)) flag = 0;
                        if (flag) to_h++;

                        flag = 1;
                        for (i = 0; i < to_d + 3; i++) if (one_in_(4)) flag = 0;
                        if (flag) to_d++;

                        if (o_ptr->to_h != to_h || o_ptr->to_d != to_d)
                        {
                            msg_print("Muramasa sucked blood, and became more powerful!");
                            o_ptr->to_h = to_h;
                            o_ptr->to_d = to_d;
                        }
                    }
                }
                else
                {
                    if (drain_result > 5) /* Did we really hurt it? */
                    {
                        drain_heal = damroll(2, drain_result / 6);

                        /* Hex */
                        if (hex_spelling(HEX_VAMP_BLADE)) drain_heal *= 2;
                        if (prace_is_(RACE_MON_VAMPIRE)) drain_heal *= 2;

                        if (mode == WEAPONMASTER_REAPING) drain_heal = (_reaper_lock ? 8 : 16);

                        if (drain_left)
                        {
                            if (drain_heal < drain_left)
                                drain_left -= drain_heal;
                            else
                            {
                                drain_heal = drain_left;
                                drain_left = 0;
                            }

                            if (drain_msg)
                            {
                                if (o_ptr)
                                    msg_format("Your weapon drains life from %s!", m_name_object);
                                else
                                    msg_format("You drain life from %s!", m_name_object);
                                drain_msg = FALSE;
                            }
                            drain_heal = (drain_heal * mutant_regenerate_mod) / 100;
                            vampirism_hack = m_ptr->hp;
                            vamp_player(drain_heal);
                            obj_learn_slay(o_ptr, OF_BRAND_VAMP, "is <color:D>Vampiric</color>");
                        }
                    }
                }
                m_ptr->maxhp -= (k+7)/8;
                if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
                if (m_ptr->maxhp < 1) m_ptr->maxhp = 1;
                weak = TRUE;
            }
            can_drain = FALSE;
            drain_result = 0;

            /* Confusion attack */
            if ( (p_ptr->special_attack & ATTACK_CONFUSE)
              || chaos_effect == 3
              || mode == HISSATSU_CONF
              || mode == MYSTIC_CONFUSE
              || mode == DRACONIAN_STRIKE_CONF
              || hex_spelling(HEX_CONFUSION)
              || (giant_is_(GIANT_TITAN) && p_ptr->lev >= 30 && one_in_(5)) )
            {
                /* Cancel glowing hands */
                if (p_ptr->special_attack & ATTACK_CONFUSE)
                {
                    p_ptr->special_attack &= ~(ATTACK_CONFUSE);
                    msg_print("Your hands stop glowing.");
                    p_ptr->redraw |= (PR_STATUS);

                }

                /* Confuse the monster */
                if (r_ptr->flags3 & RF3_NO_CONF)
                {
                    mon_lore_3(m_ptr, RF3_NO_CONF);
                    msg_format("%^s is unaffected.", m_name_subject);
                }
                else if (randint0(100) < r_ptr->level)
                    msg_format("%^s is unaffected.", m_name_subject);
                else
                {
                    msg_format("%^s appears confused.", m_name_subject);
                    (void)set_monster_confused(c_ptr->m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
                }

                /* Only try to confuse once */
                if (mode == HISSATSU_CONF)
                    mode = 0;
            }

            else if (chaos_effect == 4)
            {
                bool resists_tele = FALSE;

                if (mummy_get_toggle() == MUMMY_TOGGLE_ANTITELE)
                {
                     msg_format("%^s is unaffected!", m_name_subject);
                     resists_tele = TRUE;
                }
                else if (r_ptr->flagsr & RFR_RES_TELE)
                {
                    if (r_ptr->flags1 & RF1_UNIQUE)
                    {
                        mon_lore_r(m_ptr, RFR_RES_TELE);
                        msg_format("%^s is unaffected!", m_name_subject);
                        resists_tele = TRUE;
                    }
                    else if (r_ptr->level > randint1(100))
                    {
                        mon_lore_r(m_ptr, RFR_RES_TELE);
                        msg_format("%^s resists!", m_name_subject);
                        resists_tele = TRUE;
                    }
                }

                if (!resists_tele)
                {
                    msg_format("%^s disappears!", m_name_subject);
                    teleport_away(c_ptr->m_idx, 50, TELEPORT_PASSIVE);
                    num = num_blow + 1; /* Can't hit it anymore! */
                    *mdeath = TRUE;
                }
            }

            else if ((chaos_effect == 5) && (randint1(90) > r_ptr->level))
            {
                if (!(r_ptr->flags1 & RF1_UNIQUE) &&
                    !(m_ptr->mflag2 & MFLAG2_QUESTOR) && 
                    !(r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK))
                {
                    if (polymorph_monster(m_ptr))
                    {
                        msg_format("%^s changes!", m_name_subject);
                        *fear = FALSE;
                        weak = FALSE;
                    }
                    else
                        msg_format("%^s is unaffected.", m_name_subject);

                    m_ptr = &m_list[c_ptr->m_idx];
                    monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
                    monster_desc(m_name_object, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
                    r_ptr = &r_info[m_ptr->r_idx];
                }
            }
            else if (o_ptr && o_ptr->name1 == ART_G_HAMMER)
            {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];

                if (m_ptr->hold_o_idx)
                {
                    object_type *q_ptr = &o_list[m_ptr->hold_o_idx];
                    char stolen_name[MAX_NLEN];

                    object_desc(stolen_name, q_ptr, OD_NAME_ONLY);
                    q_ptr->held_m_idx = 0;
                    q_ptr->marked = OM_TOUCHED;
                    m_ptr->hold_o_idx = q_ptr->next_o_idx;
                    q_ptr->next_o_idx = 0;
                    msg_format("You snatched %s.", stolen_name);
                    pack_carry(q_ptr);
                    obj_release(q_ptr, OBJ_RELEASE_QUIET);
                }
            }

            if ( p_ptr->pclass == CLASS_DUELIST
              && o_ptr
              && o_ptr->tval == TV_POLEARM
              && o_ptr->sval == SV_DEATH_SCYTHE
              && !one_in_(3) )
            {
                death_scythe_miss(o_ptr, hand, mode);
            }

            if (disciple_is_(DISCIPLE_TROIKA))
            {
                if (chaos_effect) troika_effect(TROIKA_CHANCE);
                else troika_effect(TROIKA_HIT);
            }
        }
        /* Player misses */
        else
        {
            backstab = FALSE; /* Clumsy! */
            fuiuchi = FALSE; /* Clumsy! */

            if ( o_ptr
              && o_ptr->tval == TV_POLEARM
              && o_ptr->sval == SV_DEATH_SCYTHE
              && one_in_(3) )
            {
                msg_format("You miss.", m_name_object);
                death_scythe_miss(o_ptr, hand, mode);
            }
            else
            {
                sound(SOUND_MISS);
                msg_format("You miss.", m_name_object);
            }
            check_muscle_sprains(250, "You sprain a muscle!");
        }
        backstab = FALSE;
        fuiuchi = FALSE;

weaponmaster_reap:
        if (mode == WEAPONMASTER_REAPING)
        {
            do_whirlwind = TRUE;
        }

        /* Hack: Whirlwind first attacks chosen monster, than attempts to strike
           all other monsters adjacent.*/
        if ((do_whirlwind) && (!_reaper_lock))
        {
            int              start_dir, x2, y2;
            int                 dir;
            cave_type       *c_ptr2;
            monster_type    *m_ptr2;
            bool            fear2 = FALSE;
            bool            mdeath2 = FALSE;
            int             _laskuri = 0;

            /* Message order */
            if (weak && !(*mdeath))
            {
                msg_format("%^s seems weakened.", m_name_subject);
                weak = FALSE;
            }            

            start_dir = calculate_dir(px, py, x, y);
            dir = start_dir;
            _reaper_lock = TRUE; /* Prevent this whirlwind from triggering further whirlwinds... */

            for (;;)
            {
                dir = get_next_dir(dir);
                if (dir == start_dir || dir == 5) break;

                x2 = px + ddx[dir];
                y2 = py + ddy[dir];
                c_ptr2 = &cave[y2][x2];
                m_ptr2 = &m_list[c_ptr2->m_idx];

                if (c_ptr2->m_idx && (m_ptr2->ml || cave_have_flag_bold(y2, x2, FF_PROJECT)))
                {
                    char m_name2[MAX_NLEN];
                    monster_desc(m_name2, m_ptr2, 0);
                    if (!_laskuri)
                    {
                        if (mode == WEAPONMASTER_REAPING)
                        {
                            cmsg_format(TERM_RED, "You swing your %s about, reaping a harvest of death!", o_name);
                        }
                        else cmsg_format(TERM_BLUE, "You swing your %s about, striking all nearby foes.", o_name);
                    }
                    _laskuri++;
                    cmsg_format(TERM_L_UMBER, "You attack %s:", m_name2);
                    py_attack_aux(y2, x2, &fear2, &mdeath2, hand, (mode == WEAPONMASTER_REAPING) ? mode : WEAPONMASTER_WHIRLWIND);
                }
            }

            if (p_ptr->wizard)
                msg_print("****END WHIRLWIND****");
            _reaper_lock = FALSE;
            _reaper_bonus = 0;
        }

        if (mode == WEAPONMASTER_RETALIATION) break;
        if (mode == WEAPONMASTER_CRUSADERS_STRIKE) break;
        if (mode == WEAPONMASTER_MANY_STRIKE) break;
        if (mode == WEAPONMASTER_PIERCING_STRIKE && num > p_ptr->lev/25) break;
        if (mode == WEAPONMASTER_PROXIMITY_ALERT && num > p_ptr->lev/25) break;
        if (mode == WEAPONMASTER_WHIRLWIND) break;
        if (mode == WEAPONMASTER_REAPING) break;
        if (mode == MELEE_AWESOME_BLOW) break;
        if (mode == ROGUE_ASSASSINATE) break;
        if (mauler_get_toggle() == MAULER_TOGGLE_MAUL) break;
        if (mode == MAULER_STUNNING_BLOW) break;
        if (mode == MAULER_KNOCKBACK) break;
        if (mode == MAULER_CRUSHING_BLOW) break;
        if (mode == MAULER_SCATTER) break;
        if (mode == HISSATSU_KYUSHO) break;
        if (mode == HISSATSU_MINEUCHI) break;
        if (mode == HISSATSU_3DAN) break;
        if (mode == HISSATSU_IAI) break;
        if (mode == MYSTIC_KILL) break;
        if (mode == MYSTIC_KNOCKOUT) break;
        if (mode == MYSTIC_CONFUSE) break;
        if (!p_ptr->weapon_ct) break; /* Draconian Metamorphosis in the middle of attacking ... mon_take_hit -> gain_exp ->
                                         gain level 35 -> change body type -> drop all weapons -> handle_stuff -> continue attacks!
                                         BTW: Gaining experience and calling handle_stuff in the middle of a round of attacks
                                         causes numerous other problems and should be removed at some point. The problem is that
                                         handle_stuff is like a virus ... someone will keep calling it to resurface these sorts
                                         of issues! */
    }

    if (mode == WEAPONMASTER_ELUSIVE_STRIKE && hit_ct)
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);

    if ( (mode == WEAPONMASTER_KNOCK_BACK || mode == MAULER_KNOCKBACK || mode == MAULER_SCATTER)
      && hit_ct
      && !*mdeath )
    {
        int dist = hit_ct * 2;

        if (mode == MAULER_KNOCKBACK)
            dist = 8;
        if (mode == MAULER_SCATTER)
            dist = 3;

        do_monster_knockback(x, y, dist);
    }

    /* Sleep counter ticks down in energy units ... Also, *lots* of code
       will just wake the monster back up, so we need to be very careful
       about when we call this. */
    if (knock_out && !(*mdeath))
        set_monster_csleep(c_ptr->m_idx, MON_CSLEEP(m_ptr) + 500);

    if (success_hit && weaponmaster_get_toggle() == TOGGLE_TRIP && mode == 0 && !(*mdeath) && !fear_stop)
    {
        if (test_hit_norm(chance, mon_ac(m_ptr), m_ptr->ml))
        {
            if (m_ptr->mflag2 & MFLAG2_TRIPPED)
                msg_format("%^s is already tripped up.", m_name_subject);
            else if ( !(r_ptr->flags1 & RF1_UNIQUE)
                   || !mon_save_p(m_ptr->r_idx, A_STR) )
            {
                msg_format("%^s cries 'Help! I've fallen and I can't get up!'", m_name_subject);
                m_ptr->mflag2 |= MFLAG2_TRIPPED;
            }
            else
                msg_format("%^s nimbly dodges your attempt to trip.", m_name_subject);
        }
        else
            msg_format("You attempt to trip %s, but miss.", m_name_object);
    }

    if (weak && !(*mdeath))
        msg_format("%^s seems weakened.", m_name_subject);

    if (drain_left != _max_vampiric_drain())
    {
        if (one_in_(4))
            virtue_add(VIRTUE_UNLIFE, 1);
    }

    if (touch_ct && !(*mdeath))
        fear_p_touch_m(m_ptr);

    if (do_quake)
    {
        earthquake(py, px, 10);
        obj_learn_slay(o_ptr, OF_IMPACT, "causes <color:U>Earthquakes</color>");
    }

#if 0
    if (p_ptr->pclass == CLASS_MAULER)
        c_put_str(TERM_WHITE, format("Maul:%5d", dam_tot), 24, 0);
    if (p_ptr->pclass == CLASS_DUELIST)
        c_put_str(TERM_WHITE, format("Duel:%5d", dam_tot), 24, 0);
#endif
    if (*mdeath)
        wizard_report_damage(old_hp);
    else
        wizard_report_damage(old_hp - m_ptr->hp);

    if ((o_ptr) && (o_ptr->name1 == ART_BLOODRIP) && ((p_ptr->pclass == CLASS_BLOOD_KNIGHT) || (one_in_(2))))
    {
        if (object_is_known(o_ptr))
            msg_print("Bloodrip swings back to feed on you!");
        else
            msg_print("Your sword swings back to feed on you!");
        (void)set_cut(p_ptr->cut + 2 + damroll(2, 3), FALSE);
    }

    return success_hit;
}

bool random_opponent(int *y, int *x)
{
    int dirs[9];
    int ct = 0;
    int i, tx, ty;
    cave_type *c_ptr;

    for (i = 0; i < 8; i++)
    {
        ty = py + ddy_ddd[i];
        tx = px + ddx_ddd[i];
        c_ptr = &cave[ty][tx];
        if (c_ptr->m_idx)
        {
            dirs[ct] = i;
            ct++;
        }
    }

    if (ct)
    {
        i = randint0(ct);
        *y = py + ddy_ddd[dirs[i]];
        *x = px + ddx_ddd[dirs[i]];
        return TRUE;
    }
    return FALSE;
}

/* Check for the !? inscription (confirm melee attacks) */
bool _melee_attack_not_confirmed(monster_type *m_ptr)
{
    int i, taso = 0;
    static int last_okay_race = 0;
    object_type *eka = NULL;
    cptr insc, pos;
    if (!p_ptr->weapon_ct) return FALSE;
    if (m_ptr->mflag2 & MFLAG2_MON_HIT_OKAY) return FALSE;
    if (m_ptr->r_idx == last_okay_race)
    {
        m_ptr->mflag2 |= MFLAG2_MON_HIT_OKAY;
        return FALSE;
    }
    last_okay_race = 0;
    for (i = 0; i < MAX_ARMS; i++)
    {
        int rhand = 2*i;
        int lhand = 2*i+1;

        if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
            eka = equip_obj(p_ptr->weapon_info[rhand].slot);

        else if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
            eka = equip_obj(p_ptr->weapon_info[lhand].slot);

        if (eka) break;
    }
    if (!eka) return FALSE; /* paranoia */
    if (!eka->inscription) return FALSE;
    insc = quark_str(eka->inscription);
    for (pos = strchr(insc, '!'); pos && *pos; pos = strchr(pos + 1, '!'))
    {
        for (;;)
        {
            pos++;
            if (!*pos) return FALSE;
            else if (*pos == '?')
            {
                char m_name[80];
                while ((*(pos++)) && (isdigit(*pos)))
                {
                    int luku = D2I(*pos);
                    taso *= 10;
                    taso += luku;
                    taso = MIN(taso, 128); /* paranoia */
                }
                if (r_info[m_ptr->r_idx].level < taso) return FALSE;
                monster_desc(m_name, m_ptr, 0);
                if (!get_check(format("Really attack %s? ", m_name)))
                {
                    return TRUE;
                }
                
                m_ptr->mflag2 |= MFLAG2_MON_HIT_OKAY;
                last_okay_race = m_ptr->r_idx;
                return FALSE;
            }
            else if (!isalpha(*pos))
            {
                if (*pos == '!') pos--;
                break;
            }
        }
    }
    return FALSE;
}

bool _py_attack_exit(void)
{
    if (travel.mode != TRAVEL_MODE_NORMAL) travel_cancel_fully();
    return FALSE;
}

bool no_melee_check(void)
{
    if (!no_melee_challenge) return FALSE;
    msg_print("You would never hit anybody!");
    return TRUE;
}

bool py_attack(int y, int x, int mode)
{
    bool            fear = FALSE;
    bool            mdeath = FALSE;
    int             i;
    cave_type       *c_ptr = &cave[y][x];
    monster_type    *m_ptr = &m_list[c_ptr->m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    char            m_name[80];
    static s32b     last_attack_turn = 0;

    /* Disturb the player */
    disturb(0, 0);

    /* Allow crits */
    if (game_turn != last_attack_turn)
    {
        _allow_crits = TRUE;
        last_attack_turn = game_turn;
    }

    energy_use = 100;

    if (no_melee_check())
    {
        energy_use = 0;
        if (!m_ptr->ml) energy_use = 50; /* bumping into an invisible monster shouldn't give a free turn */
        return _py_attack_exit();
    }

    if (!p_ptr->weapon_ct && !p_ptr->innate_attack_ct && !possessor_can_attack())
    {
        msg_print("You have no melee attacks.");
        energy_use = 0;
        if (!m_ptr->ml) energy_use = 50; /* bumping into an invisible monster shouldn't give a free turn */
        return _py_attack_exit();
    }

    if (prace_is_(MIMIC_MIST))
    {
        msg_print("You cannot attack while incorporeal.");
        energy_use = 0;
        if (!m_ptr->ml) energy_use = 50; /* bumping into an invisible monster shouldn't give a free turn */
        return _py_attack_exit();
    }

    monster_desc(m_name, m_ptr, 0);
    if (m_ptr->ml)
    {
        if (!p_ptr->image) mon_track(m_ptr);
        health_track(c_ptr->m_idx);
    }

    if ( (r_ptr->flags1 & RF1_FEMALE)
      && !(p_ptr->stun || p_ptr->confused || p_ptr->image || !m_ptr->ml)
      && equip_find_art(ART_ZANTETSU))
    {
        msg_print("I can not attack women!");
        return _py_attack_exit();
    }

    if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
    {
        msg_print("Something prevents you from attacking!");
        return _py_attack_exit();
    }

    if (_melee_attack_not_confirmed(m_ptr))
    {
        energy_use = 0;
        return _py_attack_exit();
    }

    else if ( !is_hostile(m_ptr)
      && !(p_ptr->stun || p_ptr->confused || p_ptr->image || IS_SHERO() || !m_ptr->ml) )
    {
        if (equip_find_art(ART_STORMBRINGER))
        {
            msg_format("Your black blade greedily attacks %s!", m_name);
            virtue_add(VIRTUE_INDIVIDUALISM, 1);
            virtue_add(VIRTUE_HONOUR, -1);
            virtue_add(VIRTUE_JUSTICE, -1);
            virtue_add(VIRTUE_COMPASSION, -1);
        }
        else if (p_ptr->pclass != CLASS_BERSERKER)
        {
            if (get_check("Really hit it? "))
            {
                virtue_add(VIRTUE_INDIVIDUALISM, 1);
                virtue_add(VIRTUE_HONOUR, -1);
                virtue_add(VIRTUE_JUSTICE, -1);
                virtue_add(VIRTUE_COMPASSION, -1);
                if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_VILLAINY);
            }
            else
            {
                msg_format("You stop to avoid hitting %s.", m_name);
                return _py_attack_exit();
            }
        }
    }

    if (MON_CSLEEP(m_ptr)) /* It is not honorable etc to attack helpless victims */
    {
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_COMPASSION, -1);
        if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) virtue_add(VIRTUE_HONOUR, -1);
    }

    for (i = 0; i < MAX_ARMS; i++)
    {
        int rhand = 2*i;
        int lhand = 2*i+1;
        object_type *robj = NULL, *lobj = NULL;

        if (p_ptr->weapon_info[rhand].wield_how != WIELD_NONE)
            robj = equip_obj(p_ptr->weapon_info[rhand].slot);

        if (p_ptr->weapon_info[lhand].wield_how != WIELD_NONE)
            lobj = equip_obj(p_ptr->weapon_info[lhand].slot);

        if (robj && lobj)
        {
            skills_dual_wielding_gain(r_ptr);
            break;
        }
    }

    if (p_ptr->riding)
        skills_riding_gain_melee(r_ptr);

    riding_t_m_idx = c_ptr->m_idx;

    drain_left = _max_vampiric_drain();
    retaliation_count = 0;
    melee_hack = TRUE;
    fear_stop = FALSE;

    if (mode != WEAPONMASTER_RETALIATION)
        cmsg_format(TERM_L_UMBER, "You attack %s%s:", m_name, py_attack_desc(mode));

    if (weaponmaster_get_toggle() == TOGGLE_MANY_STRIKE && mode == 0)
    {
        int i, j;
        bool stop = FALSE;
        int msec = delay_time();

        _many_strike_mon = c_ptr->m_idx;

        for (i = 0; i < MAX_HANDS && !stop; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                object_type *o_ptr = equip_obj(p_ptr->weapon_info[i].slot);
                int num_blow = _get_num_blow(i, 0);

                for (j = 0; j < num_blow; j++)
                {
                    if (o_ptr) /* paranoia */
                    {
                        if (panel_contains(y, x) && player_can_see_bold(y, x))
                        {
                            char c = object_char(o_ptr);
                            byte a = object_attr(o_ptr);

                            print_rel(c, a, y, x);
                            move_cursor_relative(y, x);
                            Term_fresh();
                            Term_xtra(TERM_XTRA_DELAY, msec);
                            lite_spot(y, x);
                            Term_fresh();
                        }
                        else
                        {
                            /* Pause anyway, for consistancy */
                            Term_xtra(TERM_XTRA_DELAY, msec);
                        }
                    }
                    py_attack_aux(y, x, &fear, &mdeath, i, WEAPONMASTER_MANY_STRIKE);
                    if (fear_stop || !random_opponent(&y, &x))
                    {
                        stop = TRUE;
                        break;
                    }
                }
            }
        }
        _many_strike_mon = 0;
    }
    else if (weaponmaster_get_toggle() == TOGGLE_PIERCING_STRIKE && mode == 0)
    {
    u16b    path[512];
    int        ct = project_path(path, 3, py, px, y, x, PROJECT_PATH | PROJECT_THRU);
    int        msec = delay_time();

        int i, j, k;

        for (i = 0; i < MAX_HANDS; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                object_type *o_ptr = equip_obj(p_ptr->weapon_info[i].slot);
                int num_blow = _get_num_blow(i, 0);
                for (j = 0; j < num_blow; j++)
                {
                    for (k = 0; k < ct; k++)
                    {
                        int nx, ny;
                        ny = GRID_Y(path[k]);
                        nx = GRID_X(path[k]);

                        if (!cave_have_flag_bold(ny, nx, FF_PROJECT)
                         && !cave[ny][nx].m_idx)
                        {
                            break;
                        }

                        if (!cave[ny][nx].m_idx) break;

                        if (o_ptr && panel_contains(ny, nx) && player_can_see_bold(ny, nx))
                        {
                            char c = object_char(o_ptr);
                            byte a = object_attr(o_ptr);
                            int msec = delay_time();

                            print_rel(c, a, ny, nx);
                            move_cursor_relative(ny, nx);
                            Term_fresh();
                            Term_xtra(TERM_XTRA_DELAY, msec);
                            lite_spot(ny, nx);
                            Term_fresh();
                        }
                        else
                            Term_xtra(TERM_XTRA_DELAY, msec);

                        if (!py_attack_aux(ny, nx, &fear, &mdeath, i, WEAPONMASTER_PIERCING_STRIKE) || fear_stop) break;
                    }
                }
            }
        }
    }
    else if (mauler_get_toggle() == MAULER_TOGGLE_MAUL && mode == 0)
    {
        int dir = calculate_dir(px, py, x, y);
        int i;
        if (dir != 5)
        {
            for (i = 0; i < MAX_HANDS; i++)
            {
                int j;
                drain_left = _max_vampiric_drain();
                if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
                {
                    int num_blow = _get_num_blow(i, 0);
                    for (j = 0; j < num_blow; j++)
                    {
                        int y, x;
                        int ny, nx;
                        int m_idx;
                        cave_type *c_ptr;
                        monster_type *m_ptr;

                        y = py + ddy[dir];
                        x = px + ddx[dir];
                        c_ptr = &cave[y][x];

                        if (c_ptr->m_idx)
                            py_attack_aux(y, x, &fear, &mdeath, i, 0);
                        else
                        {
                            msg_print("There is no monster.");
                            break;
                        }

                        /* Monster is dead? */
                        if (!c_ptr->m_idx) break;

                        ny = y + ddy[dir];
                        nx = x + ddx[dir];
                        m_idx = c_ptr->m_idx;
                        m_ptr = &m_list[m_idx];

                        /* Monster cannot move back? */
                        if (!monster_can_enter(ny, nx, &r_info[m_ptr->r_idx], 0))
                        {
                            continue;
                        }

                        c_ptr->m_idx = 0;
                        cave[ny][nx].m_idx = m_idx;
                        m_ptr->fy = ny;
                        m_ptr->fx = nx;

                        update_mon(m_idx, TRUE);

                        lite_spot(y, x);
                        lite_spot(ny, nx);

                        /* Player can move forward? */
                        if (player_can_enter(c_ptr->feat, 0))
                        {
                            if (!move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP)) break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i < MAX_HANDS; i++)
        {
            drain_left = _max_vampiric_drain();
            if (p_ptr->weapon_info[i].wield_how != WIELD_NONE && !mdeath && !fear_stop)
            {
                py_attack_aux(y, x, &fear, &mdeath, i, mode);
                if (mode == WEAPONMASTER_RETALIATION)
                    break;
            }
        }
    }

    if (p_ptr->cleave && !fear_stop && mdeath && mode == 0)
    {
        int ny = 0, nx = 0, i, dir = 0;

        melee_hack = FALSE;
        for (i = 1; i <= 4 + p_ptr->lev/10; i++)
        {
            dir = randint0(8);
            ny = py + ddy_ddd[dir];
            nx = px + ddx_ddd[dir];

            if (cave[ny][nx].m_idx)
            {
                msg_print("You attempt to cleave another foe!");
                py_attack(ny, nx, WEAPONMASTER_CLEAVE);
                break;
            }
        }
        return TRUE;
    }

    if (possessor_can_attack() && !mdeath && !fear_stop)
    {
        if (mode == WEAPONMASTER_RETALIATION && p_ptr->weapon_ct > 0)
        {
        }
        else possessor_attack(point(x,y), &fear, &mdeath, mode);
    }
    if (p_ptr->innate_attack_ct && !mdeath && !fear_stop)
    {
        bool do_innate_attacks = TRUE;

        switch (mode)
        {
        case WEAPONMASTER_RETALIATION:
            if (p_ptr->weapon_ct == 0) /* Kenshirou Possessor can retaliate with innate attacks! */
                break;
        case WEAPONMASTER_PROXIMITY_ALERT:
        case WEAPONMASTER_CRUSADERS_STRIKE:
        case WEAPONMASTER_MANY_STRIKE:
        case WEAPONMASTER_PIERCING_STRIKE:
        case WEAPONMASTER_WHIRLWIND:
        case WEAPONMASTER_REAPING:
        case MELEE_AWESOME_BLOW:
        case ROGUE_ASSASSINATE:
        case MAULER_STUNNING_BLOW:
        case MAULER_KNOCKBACK:
        case MAULER_CRUSHING_BLOW:
        case MAULER_CRITICAL_BLOW:
        case MAULER_SCATTER:
        case HISSATSU_2:
        case HISSATSU_KYUSHO:
        case HISSATSU_MINEUCHI:
        case HISSATSU_3DAN:
        case HISSATSU_IAI:
        case MYSTIC_KILL:
        case MYSTIC_KNOCKOUT:
        case MYSTIC_CONFUSE:
            do_innate_attacks = FALSE;
            break;
        }

        if (mauler_get_toggle() == MAULER_TOGGLE_MAUL)
            do_innate_attacks = FALSE;

        if (do_innate_attacks)
        {
            if (mode == DRAGON_TAIL_SWEEP)
            {
                int           ct = 3 + p_ptr->lev / 25;
                int           x2, y2, dir, start_dir;
                int           msec = delay_time();

                start_dir = calculate_dir(px, py, x, y);
                dir = start_dir;

                while(ct--)
                {
                    x2 = px + ddx[dir];
                    y2 = py + ddy[dir];

                    if (in_bounds(y2, x2))
                    {
                        cave_type    *c_ptr2 = &cave[y2][x2];
                        monster_type *m_ptr2 = &m_list[c_ptr2->m_idx];

                        if (panel_contains(y2, x2) && player_can_see_bold(y2, x2))
                        {
                            char c = '*';
                            byte a = TERM_WHITE;

                            print_rel(c, a, y2, x2);
                            move_cursor_relative(y2, x2);
                            Term_fresh();
                            Term_xtra(TERM_XTRA_DELAY, msec);
                            lite_spot(y2, x2);
                            Term_fresh();
                        }
                        else
                            Term_xtra(TERM_XTRA_DELAY, msec);

                        if (c_ptr2->m_idx && (m_ptr2->ml || cave_have_flag_bold(y2, x2, FF_PROJECT)))
                        {
                            bool fear2 = FALSE, mdeath2 = FALSE;
                            innate_attacks(c_ptr2->m_idx, &fear2, &mdeath2, DRAGON_TAIL_SWEEP);
                        }
                    }
                    dir = get_next_dir(dir);
                    if (dir == start_dir || dir == 5) break;
                }
            }
            else
                innate_attacks(c_ptr->m_idx, &fear, &mdeath, mode);

        }
    }

    melee_hack = FALSE;

    if (fear && m_ptr->ml && !mdeath)
    {
        sound(SOUND_FLEE);
        msg_format("%^s flees in terror!", m_name);
    }

    if ((p_ptr->special_defense & KATA_IAI) && (mode != HISSATSU_IAI || mdeath))
        set_action(ACTION_NONE);

    return mdeath;
}


bool pattern_seq(int c_y, int c_x, int n_y, int n_x)
{
    feature_type *cur_f_ptr = &f_info[cave[c_y][c_x].feat];
    feature_type *new_f_ptr = &f_info[cave[n_y][n_x].feat];
    bool is_pattern_tile_cur = have_flag(cur_f_ptr->flags, FF_PATTERN);
    bool is_pattern_tile_new = have_flag(new_f_ptr->flags, FF_PATTERN);
    int pattern_type_cur, pattern_type_new;

    if (!is_pattern_tile_cur && !is_pattern_tile_new) return TRUE;

    pattern_type_cur = is_pattern_tile_cur ? cur_f_ptr->subtype : NOT_PATTERN_TILE;
    pattern_type_new = is_pattern_tile_new ? new_f_ptr->subtype : NOT_PATTERN_TILE;

    if (pattern_type_new == PATTERN_TILE_START)
    {
        if (!is_pattern_tile_cur && !p_ptr->confused && !p_ptr->stun && !p_ptr->image)
        {
            if (get_check("If you start walking the Pattern, you must walk the whole way. Ok? "))
                return TRUE;
            else
                return FALSE;
        }
        else
            return TRUE;
    }
    else if ((pattern_type_new == PATTERN_TILE_OLD) ||
         (pattern_type_new == PATTERN_TILE_END) ||
         (pattern_type_new == PATTERN_TILE_WRECKED) ||
         ((pattern_type_new == PATTERN_TILE_TELEPORT) && (!py_in_dungeon())))
    {
        if (is_pattern_tile_cur)
        {
            return TRUE;
        }
        else
        {
            msg_print("You must start walking the Pattern from the startpoint.");

            return FALSE;
        }
    }
    else if ((pattern_type_new == PATTERN_TILE_TELEPORT) ||
         (pattern_type_cur == PATTERN_TILE_TELEPORT))
    {
        return TRUE;
    }
    else if (pattern_type_cur == PATTERN_TILE_START)
    {
        if (is_pattern_tile_new)
            return TRUE;
        else
        {
            msg_print("You must walk the Pattern in correct order.");

            return FALSE;
        }
    }
    else if ((pattern_type_cur == PATTERN_TILE_OLD) ||
         (pattern_type_cur == PATTERN_TILE_END) ||
         (pattern_type_cur == PATTERN_TILE_WRECKED))
    {
        if (!is_pattern_tile_new)
        {
            msg_print("You may not step off from the Pattern.");

            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        if (!is_pattern_tile_cur)
        {
            msg_print("You must start walking the Pattern from the startpoint.");

            return FALSE;
        }
        else
        {
            byte ok_move = PATTERN_TILE_START;
            switch (pattern_type_cur)
            {
                case PATTERN_TILE_1:
                    ok_move = PATTERN_TILE_2;
                    break;
                case PATTERN_TILE_2:
                    ok_move = PATTERN_TILE_3;
                    break;
                case PATTERN_TILE_3:
                    ok_move = PATTERN_TILE_4;
                    break;
                case PATTERN_TILE_4:
                    ok_move = PATTERN_TILE_1;
                    break;
                default:
                    if (p_ptr->wizard)
                        msg_format("Funny Pattern walking, %d.", pattern_type_cur);

                    return TRUE; /* Goof-up */
            }

            if ((pattern_type_new == ok_move) ||
                (pattern_type_new == pattern_type_cur))
                return TRUE;
            else
            {
                if (!is_pattern_tile_new)
                    msg_print("You may not step off from the Pattern.");
                else
                    msg_print("You must walk the Pattern in correct order.");

                return FALSE;
            }
        }
    }
}


bool player_can_enter(s16b feature, u16b mode)
{
    feature_type *f_ptr = &f_info[feature];

    if (p_ptr->riding)
        return monster_can_cross_terrain(feature, &r_info[m_list[p_ptr->riding].r_idx], mode | CEM_RIDING);

    if (p_ptr->prace == RACE_MON_POSSESSOR && p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
        return monster_can_cross_terrain(feature, &r_info[p_ptr->current_r_idx], mode | CEM_MIMIC);

    if (p_ptr->prace == RACE_MON_MIMIC && p_ptr->current_r_idx != MON_MIMIC)
        return monster_can_cross_terrain(feature, &r_info[p_ptr->current_r_idx], mode | CEM_MIMIC);

    /* Pattern */
    if (have_flag(f_ptr->flags, FF_PATTERN))
    {
        if (!(mode & CEM_P_CAN_ENTER_PATTERN)) return FALSE;
    }

    /* "CAN" flags */
    if (have_flag(f_ptr->flags, FF_CAN_FLY) && p_ptr->levitation) return TRUE;
    /*if (have_flag(f_ptr->flags, FF_CAN_CLIMB) && p_ptr->climbing) return TRUE;*/
    if (have_flag(f_ptr->flags, FF_CAN_SWIM) && p_ptr->can_swim) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_PASS) && p_ptr->pass_wall) return TRUE;

    if (!have_flag(f_ptr->flags, FF_MOVE)) return FALSE;

    return TRUE;
}

static bool _auto_detect_traps(void)
{
    slot_t slot;
    if (!auto_detect_traps) return FALSE;
    if (p_ptr->pclass == CLASS_BERSERKER) return FALSE;
    if (beorning_is_(BEORNING_FORM_BEAR)) return FALSE;
    if (p_ptr->pclass == CLASS_MAGIC_EATER && magic_eater_auto_detect_traps()) return TRUE;

    slot = pack_find_device(EFFECT_DETECT_TRAPS);
    if (slot)
    {
        obj_ptr device = pack_obj(slot);
        detect_traps(DETECT_RAD_DEFAULT, TRUE);
        stats_on_use(device, 1);
        device_decrease_sp(device, device->activation.cost);
        return TRUE;
    }
    slot = pack_find_obj(TV_SCROLL, SV_SCROLL_DETECT_TRAP);
    if (slot && !p_ptr->blind && !(get_race()->flags & RACE_IS_ILLITERATE))
    {
        obj_ptr scroll = pack_obj(slot);
        if (obj_is_known(scroll))
        {
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            stats_on_use(scroll, 1);
            scroll->number--;
            obj_release(scroll, 0);
            return TRUE;
        }
    }
    slot = pack_find_device(EFFECT_DETECT_ALL);
    if (slot)
    {
        obj_ptr device = pack_obj(slot);
        detect_all(DETECT_RAD_DEFAULT);
        stats_on_use(device, 1);
        device_decrease_sp(device, device->activation.cost);
        return TRUE;
    }
    return FALSE;
}

static bool _auto_mapping(void)
{
    slot_t slot;
    if (!auto_map_area) return FALSE;
    if (p_ptr->pclass == CLASS_BERSERKER) return FALSE;
    if (beorning_is_(BEORNING_FORM_BEAR)) return FALSE;
    if (p_ptr->pclass == CLASS_MAGIC_EATER && magic_eater_auto_mapping()) return TRUE;

    slot = pack_find_device(EFFECT_ENLIGHTENMENT);
    if (slot)
    {
        obj_ptr device = pack_obj(slot);
        map_area(DETECT_RAD_MAP);
        stats_on_use(device, 1);
        device_decrease_sp(device, device->activation.cost);
        return TRUE;
    }
    slot = pack_find_obj(TV_SCROLL, SV_SCROLL_MAPPING);
    if (slot && !p_ptr->blind && !(get_race()->flags & RACE_IS_ILLITERATE))
    {
        obj_ptr scroll = pack_obj(slot);
        if (obj_is_known(scroll))
        {
            map_area(DETECT_RAD_MAP);
            stats_on_use(scroll, 1);
            scroll->number--;
            obj_release(scroll, 0);
            return TRUE;
        }
    }
    return FALSE;
}

void _snow_adjust_energy(bool snow, monster_race *r_ptr)
{
    if (!p_ptr->levitation &&
        !prace_is_(RACE_HIGH_ELF) &&
        !prace_is_(RACE_TOMTE) &&
        (!p_ptr->riding || (
          (!(r_ptr->flags2 & RF2_PASS_WALL))
          && (!(r_ptr->flags8 & RF8_WILD_SNOW))
          && (!(r_ptr->flags7 & RF7_CAN_FLY)))))
    {
        s32b lisays = 33;
        if (!p_ptr->riding)
        {
            int _tw = py_total_weight(), _lw = weight_limit();
            if (_tw > _lw)
            {
                lisays += MIN(200, _tw * 100L / _lw) - 100;
            }
        }
        else lisays = 40;
        if (!snow) lisays /= 3;
        energy_use += MIN(120, energy_use) * lisays / 100;
    }
}

/*
 * Move the player
 */
bool move_player_effect(int ny, int nx, u32b mpe_mode)
{
    cave_type *c_ptr = &cave[ny][nx];
    feature_type *f_ptr = &f_info[c_ptr->feat];
    bool old_dtrap = FALSE, new_dtrap = FALSE;
    bool old_map = FALSE, new_map = FALSE;

    if (cave[py][px].info & CAVE_IN_DETECT)
        old_dtrap = TRUE;
    if (cave[ny][nx].info & CAVE_IN_DETECT)
        new_dtrap = TRUE;

    /* Stop running if leaving a trap detected zone */
    if (!(mpe_mode & MPE_STAYING) && (running || travel.run) && disturb_trap_detect)
    {
        if (old_dtrap && !new_dtrap && !_auto_detect_traps())
        {
            disturb(0, 0);
            energy_use = 0;
            cmsg_print(TERM_VIOLET, "You are about to leave a trap detected zone.");
            return FALSE;
        }
    }

    /* Automatically detecting traps is just so lovable. Let's try the same
     * with Magic Mapping! */
    if (cave[py][px].info & CAVE_IN_MAP)
        old_map = TRUE;
    if (cave[ny][nx].info & CAVE_IN_MAP)
        new_map = TRUE;

    if (!(mpe_mode & MPE_STAYING) && (running || travel.run))
    {
        if (old_map && !new_map)
            _auto_mapping();
    }

    if (cave[py][px].info & CAVE_IN_DETECT)
        old_dtrap = TRUE;
    if (cave[ny][nx].info & CAVE_IN_DETECT)
        new_dtrap = TRUE;

    if (!(mpe_mode & MPE_STAYING))
    {
        int oy = py;
        int ox = px;
        cave_type *oc_ptr = &cave[oy][ox];
        int om_idx = oc_ptr->m_idx;
        int nm_idx = c_ptr->m_idx;

        /* Move the player */
        py = ny;
        px = nx;

        /* Hack -- For moving monster or riding player's moving */
        if (!(mpe_mode & MPE_DONT_SWAP_MON))
        {
            /* Swap two monsters */
            c_ptr->m_idx = om_idx;
            oc_ptr->m_idx = nm_idx;

            if (om_idx > 0) /* Monster on old spot (or p_ptr->riding) */
            {
                monster_type *om_ptr = &m_list[om_idx];
                om_ptr->fy = ny;
                om_ptr->fx = nx;
                update_mon(om_idx, TRUE);
            }

            if (nm_idx > 0) /* Monster on new spot */
            {
                monster_type *nm_ptr = &m_list[nm_idx];
                nm_ptr->fy = oy;
                nm_ptr->fx = ox;
                update_mon(nm_idx, TRUE);
            }
        }

        /* Redraw old spot */
        lite_spot(oy, ox);

        /* Redraw new spot */
        lite_spot(ny, nx);

        /* Check for new panel (redraw map) */
        viewport_verify();

        /* Check detection status */
        if (old_dtrap && !new_dtrap)
        {
            if (alert_trap_detect)
            {
                cmsg_print(TERM_VIOLET, "You leave a trap detected zone.");
                msg_print(NULL); /* Force a -more- prompt (unless auto_more is enabled!) */
            }
            p_ptr->redraw |= (PR_STATUS);
        }
        else if (!old_dtrap && new_dtrap)
        {
            if (alert_trap_detect)
                cmsg_print(TERM_L_BLUE, "You enter a trap detected zone.");
            p_ptr->redraw |= (PR_STATUS);
        }

        if (mpe_mode & MPE_FORGET_FLOW)
        {
            forget_flow();

            /* Mega-Hack -- Forget the view */
            p_ptr->update |= (PU_UN_VIEW);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);
        }

        /* Update stuff */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_DISTANCE);
        p_ptr->window |= PW_MONSTER_LIST | PW_OBJECT_LIST;

        /* Position Targets are confusing. They should be dismissed when no longer valid.
         * Note: Originally, I had this check in target_okay(), which is, of course, called
         * fairly often and repeatedly. While this had the fortunate side effect of preventing
         * many 'trick shot' projection abuses, it also messed up 'disintegration' effects
         * (such as Breathe Disintegration or Beam of Disintegration). For these, the user
         * needs to target a non-projectable monster. As a compromise, we will continue to
         * dismiss such targets, but only once the player moves. */
        if (target_who < 0)
        {
            if ( (!in_bounds(target_row, target_col))
              || (!projectable(py, px, target_row, target_col)) || ((target_row == oy) && (target_col == ox)) )
            {
                target_who = 0;
                target_row = 0;
                target_col = 0;
                p_ptr->redraw |= PR_HEALTH_BARS;
            }
        }

        if (!view_unsafe_grids)
            p_ptr->redraw |= PR_STATUS;

        {
            class_t *class_ptr = get_class();
            race_t  *race_ptr = get_race();
            if (class_ptr->move_player)
                class_ptr->move_player();
            if (race_ptr->move_player)
                race_ptr->move_player();

            if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
            {
                wilderness_move_player(ox, oy);
                c_ptr = &cave[py][px]; /* re-aquire in case of wilderness scroll */
            }
        }

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

        /* Remove "unsafe" flag */
        if ((!p_ptr->blind && !no_lite()) || !is_trap(c_ptr->feat)) c_ptr->info &= ~(CAVE_UNSAFE);

        /* For get everything when requested hehe I'm *NASTY* */
        if (dun_level && (d_info[dungeon_type].flags1 & DF1_FORGET) && !never_forget) wiz_dark();

        /* Handle stuff */
        if (mpe_mode & MPE_HANDLE_STUFF) handle_stuff();

        if ((player_is_ninja) || (p_ptr->tim_superstealth))
        {
            if (c_ptr->info & (CAVE_GLOW)) set_superstealth(FALSE);
            else if (p_ptr->cur_lite <= 0) set_superstealth(TRUE);
        }

        if ((p_ptr->action == ACTION_QUICK_WALK) &&
            (!have_flag(f_ptr->flags, FF_PROJECT) ||
             (!p_ptr->levitation && have_flag(f_ptr->flags, FF_DEEP))))
        {
            msg_print("You cannot run in here.");
            set_action(ACTION_NONE);

            if (weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE)
            {
                msg_print("Your shadow stance is disrupted!");
                weaponmaster_set_toggle(TOGGLE_NONE);
            }
        }
    }

    if (mpe_mode & MPE_ENERGY_USE)
    {
        if (music_singing(MUSIC_WALL))
        {
            (void)project(0, 0, py, px, (60 + p_ptr->lev), GF_DISINTEGRATE,
                PROJECT_KILL | PROJECT_ITEM);

            if (!player_bold(ny, nx) || p_ptr->is_dead || p_ptr->leaving) return FALSE;
        }

        /* Spontaneous Searching */
        if ((p_ptr->skills.fos >= 50) || (0 == randint0(50 - p_ptr->skills.fos)))
        {
            search();
        }

        /* Continuous Searching */
        if (p_ptr->action == ACTION_SEARCH)
        {
            search();
        }
    }

    /* Handle "objects" */
    if (!(mpe_mode & MPE_DONT_PICKUP))
    {
        if (mpe_mode & MPE_DO_PICKUP)
            pack_get_floor();
        else
        {
            char name[MAX_NLEN];
            int  this_o_idx, next_o_idx = 0;
            if ((!travel_ignore_items) || (!travel.run) || (travel.mode != TRAVEL_MODE_NORMAL) ||
                ((travel.x == nx) && (travel.y == ny)))
            autopick_get_floor(TRUE);
            else autopick_get_floor(FALSE);
            for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                obj_ptr obj = &o_list[this_o_idx];
                next_o_idx = obj->next_o_idx;
                object_desc(name, obj, OD_COLOR_CODED);
                msg_format("You see %s.", name);
                disturb(0, 0);
            }
        }
    }

    /* Handle "store doors" */
    if (have_flag(f_ptr->flags, FF_STORE))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter store */
        command_new = SPECIAL_KEY_STORE;
    }

    /* Handle "building doors" -KMW- */
    else if (have_flag(f_ptr->flags, FF_BLDG))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter building */
        command_new = SPECIAL_KEY_BUILDING;
    }

    /* Handle quest areas -KMW- */
    else if (have_flag(f_ptr->flags, FF_QUEST_ENTER))
    {
        /* Disturb */
        disturb(0, 0);

        energy_use = 0;
        /* Hack -- Enter quest level */
        command_new = SPECIAL_KEY_QUEST;
    }

    else if (c_ptr->mimic == feat_shadow_zap)
    {
        /* Disturb */
        disturb(0, 0);

        /* Merge with shadow */
        c_ptr->info &= ~(CAVE_MARK);
        c_ptr->info &= ~(CAVE_OBJECT);
        c_ptr->mimic = 0;
        note_spot(py, px);
        msg_print("You merge with your shadow!");
    }

    /* Set off a trap */
    else if (have_flag(f_ptr->flags, FF_HIT_TRAP) && !(mpe_mode & MPE_STAYING))
    {
        /* Disturb */
        disturb(0, 0);

        /* Hidden trap */
        if (c_ptr->mimic || have_flag(f_ptr->flags, FF_SECRET))
        {
            /* Message */
            msg_print("You found a trap!");

            /* Pick a trap */
            disclose_grid(py, px);
        }

        /* Hit the trap */
        hit_trap((mpe_mode & MPE_BREAK_TRAP) ? TRUE : FALSE, (mpe_mode & MPE_DO_JUMP) ? TRUE : FALSE);

        if (!player_bold(ny, nx) || p_ptr->is_dead || p_ptr->leaving) return FALSE;
    }

    return player_bold(ny, nx) && !p_ptr->is_dead && !p_ptr->leaving;
}


bool trap_can_be_ignored(int feat)
{
    feature_type *f_ptr = &f_info[feat];

    if (!have_flag(f_ptr->flags, FF_TRAP)) return TRUE;

    switch (f_ptr->subtype)
    {
    case TRAP_TRAPDOOR:
    case TRAP_PIT:
    case TRAP_SPIKED_PIT:
    case TRAP_POISON_PIT:
    case TRAP_BANANA:
        if (p_ptr->levitation) return TRUE;
        break;
    case TRAP_TELEPORT:
        if (p_ptr->anti_tele) return TRUE;
        break;
    case TRAP_FIRE:
        if (res_can_ignore(RES_FIRE)) return TRUE;
        break;
    case TRAP_ACID: /* Note: Your armor still gets messed up even if your pack is safe! */
        if (res_pct(RES_ACID) == 100) return TRUE;
        break;
    case TRAP_BLIND:
        if (res_can_ignore(RES_BLIND)) return TRUE;
        break;
    case TRAP_CONFUSE:
        if (res_can_ignore(RES_CONF)) return TRUE;
        break;
    case TRAP_POISON:
        if (res_can_ignore(RES_POIS)) return TRUE;
        break;
    }

    return FALSE;
}


/*
 * Determine if a "boundary" grid is "floor mimic"
 */
#define boundary_floor(C, F, MF) \
    ((C)->mimic && permanent_wall(F) && \
     (have_flag((MF)->flags, FF_MOVE) || have_flag((MF)->flags, FF_CAN_FLY)) && \
     have_flag((MF)->flags, FF_PROJECT) && \
     !have_flag((MF)->flags, FF_OPEN))

bool _glacier_hack = FALSE;

void _glacier_slip(feature_type *f_ptr)
{
     int ow_level = 25 + MAX(0, 200 - p_ptr->ac) / 4;
     energy_use += 75;
     ow_level -= randint1(ow_level / 2);
     if (p_ptr->levitation) /* paranoia */
     {
         cmsg_print(TERM_VIOLET, "Please report a Mysterious Bug to the maintainer.");
         return;
     }
     if ((have_flag(f_ptr->flags, FF_MOUNTAIN)) && (!have_flag(f_ptr->flags, FF_MOVE)))
     {
         msg_print("You slip and crash into the hard mountainside at speed!");
         if (!res_save_default(RES_SOUND)) set_stun(p_ptr->stun + ow_level, FALSE);
     }
     else if (have_flag(f_ptr->flags, FF_TREE))
     {
         msg_print("You slip and crash into the trees at speed!");
         ow_level -= (ow_level / 8); /* Better than hitting a mountain... */
         if (!res_save_default(RES_SOUND)) set_stun(p_ptr->stun + ow_level, FALSE);
     }
     else if (have_flag(f_ptr->flags, FF_SLIPPERY))
     {
         msg_print("You slip and crash onto the ice!");
         if (!res_save_default(RES_SOUND)) set_stun(p_ptr->stun + ow_level, FALSE);
     }
     else if (have_flag(f_ptr->flags, FF_SNOW)) /* Hey, something soft */
     {
         msg_print("You slip off the icy slope and fall into the snow!");
         ow_level /= 5;
     }
     else if (have_flag(f_ptr->flags, FF_CREVASSE)) /* OW */
     {
         cmsg_print(TERM_RED, "You slip off the icy hillside and fall into the crevasse!");
     }
     else msg_print("You slip off the icy slope!");
     if (have_flag(f_ptr->flags, FF_CREVASSE))
     {
         take_hit(DAMAGE_NOESCAPE, ow_level * 2, "falling into a crevasse");
         if (!p_ptr->is_dead)
         {
             inven_damage(0, set_cold_destroy, 3, RES_SOUND);
             inven_damage(0, set_cold_destroy, 3, RES_SHARDS);
             if (!res_save_default(RES_SOUND)) set_stun(p_ptr->stun + (ow_level * 3 / 2), FALSE);
             if (!res_save_default(RES_SHARDS)) set_cut(p_ptr->cut + ow_level, FALSE);
         }
     }
     else take_hit(DAMAGE_NOESCAPE, ow_level, "slipping off a glacier");
     _glacier_hack = FALSE;
}

/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid. Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player(int dir, bool do_pickup, bool break_trap)
{
    /* Find the result of moving */
    int y = py + ddy[dir];
    int x = px + ddx[dir];

    /* Examine the destination */
    cave_type *c_ptr = &cave[y][x];

    feature_type *f_ptr = &f_info[c_ptr->feat];

    monster_type *m_ptr;

    monster_type *riding_m_ptr = &m_list[p_ptr->riding];
    monster_race *riding_r_ptr = &r_info[p_ptr->riding ? riding_m_ptr->r_idx : 0]; /* Paranoia */

    char m_name[80];

    bool p_can_enter = player_can_enter(c_ptr->feat, CEM_P_CAN_ENTER_PATTERN);
    bool p_can_kill_walls = FALSE;
    bool stormbringer = FALSE;
    bool shadow_strike = FALSE;
    bool oktomove = TRUE;
    bool do_past = FALSE;

    bool ring_lev = FALSE;

    if (p_ptr->prace == RACE_MON_RING && p_ptr->levitation)
        ring_lev = TRUE;

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];

    if ((_glacier_hack) && ((c_ptr->m_idx) ||
        ((!p_can_enter) && (!have_flag(f_ptr->flags, FF_CREVASSE)))))
    {
        _glacier_slip(f_ptr);
        return;
    }

    if (equip_find_art(ART_STORMBRINGER)) stormbringer = TRUE;

    /* Player can not walk through "walls"... */
    /* unless in Shadow Form */
    p_can_kill_walls = p_ptr->kill_wall && have_flag(f_ptr->flags, FF_HURT_DISI) &&
        (!p_can_enter || !have_flag(f_ptr->flags, FF_LOS)) &&
        !have_flag(f_ptr->flags, FF_PERMANENT);

    /* Hack -- attack monsters */
    if (c_ptr->m_idx && (m_ptr->ml || p_can_enter || p_can_kill_walls))
    {
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Attack -- only if we can see it OR it is not in a wall */
        if (!is_hostile(m_ptr) &&
            !(p_ptr->confused || p_ptr->image || !m_ptr->ml || p_ptr->stun ||
            (mut_present(MUT_BERS_RAGE) && IS_SHERO())) &&
            pattern_seq(py, px, y, x) && (p_can_enter || p_can_kill_walls))
        {
            /* Disturb the monster */
            (void)set_monster_csleep(c_ptr->m_idx, 0);

            /* Extract monster name (or "it") */
            monster_desc(m_name, m_ptr, 0);

            if (m_ptr->ml)
            {
                /* Auto-Recall if possible and visible */
                if (!p_ptr->image) mon_track(m_ptr);

                /* Track a new monster */
                health_track(c_ptr->m_idx);
            }

            /* displace? */
            if ((stormbringer && (randint1(1000) > 666)) || (p_ptr->pclass == CLASS_BERSERKER))
            {
                py_attack(y, x, 0);
                oktomove = FALSE;
            }
            else if ((monster_can_cross_terrain(cave[py][px].feat, r_ptr, 0)))
            {
                do_past = (m_ptr->id != p_ptr->riding);
            }
            else
            {
                msg_format("%^s is in your way!", m_name);

                energy_use = 0;
                oktomove = FALSE;
            }

            /* now continue on to 'movement' */
        }
        else
        {
            py_attack(y, x, 0);
            if (weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE)
                shadow_strike = TRUE;
            else
                oktomove = FALSE;
        }
    }

    if (!oktomove)
    {
    }
    else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
    {
        monster_race *r_ptr = &r_info[p_ptr->current_r_idx];
        if (r_ptr->flags1 & RF1_NEVER_MOVE)
        {
            energy_use *= 3;
            /*msg_print("You can't move!");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);*/
        }
        else if (have_flag(f_ptr->flags, FF_CAN_FLY) && ((r_ptr->flags7 & RF7_CAN_FLY) || p_ptr->levitation))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_CLIMB) && (r_ptr->flags7 & RF7_CAN_CLIMB))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_SWIM) && (r_ptr->flags7 & RF7_CAN_SWIM))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_WATER) &&
            !(r_ptr->flags7 & RF7_AQUATIC) &&
            (have_flag(f_ptr->flags, FF_DEEP) || (r_ptr->flags2 & RF2_AURA_FIRE)))
        {
            msg_print("You can't swim.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (!have_flag(f_ptr->flags, FF_WATER) && (r_ptr->flags7 & RF7_AQUATIC))
        {
            msg_print("You can't move onto dry land.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
    }
    else if (p_ptr->prace == RACE_MON_RING && !p_ptr->riding)
    {
        msg_print("You can't move! Try using your Glitter power to lure a ringbearer instead.");
        energy_use = 0;
        oktomove = FALSE;
        disturb(0, 0);
    }
    else if (p_ptr->riding)
    {
        if (riding_r_ptr->flags1 & RF1_NEVER_MOVE)
        {
            msg_print("You can't move!");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (MON_CSLEEP(riding_m_ptr))
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("%^s is sleeping.", m_name);
            oktomove = FALSE;
            disturb(0,0);
        }
        else if (MON_PARALYZED(riding_m_ptr))
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("%^s is paralyzed.", m_name);
            oktomove = FALSE;
            disturb(0,0);
        }
        else if (MON_MONFEAR(riding_m_ptr))
        {
            char m_name[80];

            /* Acquire the monster name */
            monster_desc(m_name, riding_m_ptr, 0);

            /* Dump a message */
            msg_format("%^s is too scared to control.", m_name);
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (p_ptr->riding_ryoute)
        {
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (have_flag(f_ptr->flags, FF_CAN_FLY) && ((riding_r_ptr->flags7 & RF7_CAN_FLY) || ring_lev))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_CLIMB) && (riding_r_ptr->flags7 & RF7_CAN_CLIMB))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_CAN_SWIM) && ((riding_r_ptr->flags7 & RF7_CAN_SWIM) || ring_lev))
        {
            /* Allow moving */
        }
        else if (have_flag(f_ptr->flags, FF_WATER) &&
            !(riding_r_ptr->flags7 & RF7_AQUATIC) &&
            (have_flag(f_ptr->flags, FF_DEEP) || (riding_r_ptr->flags2 & RF2_AURA_FIRE)))
        {
            msg_print("Can't swim.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (!have_flag(f_ptr->flags, FF_WATER) && (riding_r_ptr->flags7 & RF7_AQUATIC))
        {
            msg_print("Can't land.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }
        else if (have_flag(f_ptr->flags, FF_LAVA) && !(riding_r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
        {
            msg_print("Too hot to go through.");
            energy_use = 0;
            oktomove = FALSE;
            disturb(0, 0);
        }

        if (oktomove && MON_STUNNED(riding_m_ptr) && one_in_(2))
        {
            char m_name[80];
            monster_desc(m_name, riding_m_ptr, 0);
            msg_format("You cannot control stunned %s!",m_name);
            oktomove = FALSE;
            disturb(0, 0);
        }
    }
    else if ( !have_flag(f_ptr->flags, FF_MOVE)
           && (have_flag(f_ptr->flags, FF_CAN_FLY) || have_flag(f_ptr->flags, FF_CAN_CLIMB))
           && !p_ptr->levitation )
    {
        {
            if (!_glacier_hack)
            {
                msg_format("You need to fly to go through the %s.", f_name + f_info[get_feat_mimic(c_ptr)].name);
                oktomove = FALSE;
            }
            else if (have_flag(f_ptr->flags, FF_CREVASSE)) p_can_enter = TRUE;
            else oktomove = FALSE;
            running = 0;
            if (_glacier_hack)
            {
                if (!p_can_enter)
                {
                    _glacier_slip(f_ptr);
                    return;
                }
            }
            else if (!(p_ptr->confused || p_ptr->image))
            {
                if (!shadow_strike)
                    energy_use = 0;
            }
        }
    }


    if (!oktomove)
    {
        /* FYI: Either the player was blocked from movement -OR- the player attacked
           because a monster was in the way.*/
        if ((!energy_use) && (travel.mode != TRAVEL_MODE_NORMAL)) travel_cancel_fully(); /* disturb doesn't cancel autoget */
    }
    /*
     * Player can move through trees and
     * has effective -10 speed
     */
    else if (have_flag(f_ptr->flags, FF_TREE) && !p_can_kill_walls && !p_ptr->pass_wall && !_glacier_hack)
    {
        if ( p_ptr->pclass != CLASS_RANGER
          && p_ptr->pclass != CLASS_SCOUT
          && !prace_is_(RACE_ENT)
          && !prace_is_(RACE_CENTAUR)
          && !prace_is_(RACE_WOOD_ELF)
          && !p_ptr->levitation
          && (!p_ptr->riding || !(riding_r_ptr->flags8 & RF8_WILD_WOOD)))
        {
            energy_use *= 2;
        }
        if (have_flag(f_ptr->flags, FF_SNOW))
        {
            _snow_adjust_energy(TRUE, riding_r_ptr);
        }
    }
    else if ((!p_ptr->levitation) && (have_flag(f_ptr->flags, FF_MOUNTAIN)) &&
             (have_flag(f_ptr->flags, FF_SLIPPERY)) && (!_glacier_hack) &&
             (!p_ptr->riding || !(riding_r_ptr->flags8 & RF8_WILD_SNOW)))
    {
        msg_print("You struggle for safe footholds on the icy slope.");
        energy_use *= 2;
    }
    else if (((have_flag(f_ptr->flags, FF_SNOW)) || (have_flag(f_ptr->flags, FF_SLUSH))) && !p_ptr->pass_wall)
    {
        bool snow = have_flag(f_ptr->flags, FF_SNOW);
        if (!_glacier_hack) _snow_adjust_energy(snow, riding_r_ptr);
    }
    else if (have_flag(f_ptr->flags, FF_WEB))
    {
        if (prace_is_(RACE_MON_SPIDER))
            energy_use = energy_use * (90 - p_ptr->lev) / 100;
        else if (warlock_is_(WARLOCK_SPIDERS))
            energy_use = energy_use * (150 - p_ptr->lev) / 150;
        else
            energy_use *= 2;
    }

#ifdef ALLOW_EASY_DISARM /* TNB */

    /* Disarm a visible trap */
    else if ((do_pickup != easy_disarm) && have_flag(f_ptr->flags, FF_DISARM) && !c_ptr->mimic && !_glacier_hack)
    {
        if (!trap_can_be_ignored(c_ptr->feat))
        {
            (void)do_cmd_disarm_aux(y, x, dir);
            return;
        }
    }

#endif /* ALLOW_EASY_DISARM -- TNB */

    /* Player can not walk through "walls" unless in wraith form...*/
    else if (!p_can_enter && !p_can_kill_walls)
    {
        /* Feature code (applying "mimic" field) */
        s16b feat = get_feat_mimic(c_ptr);
        feature_type *mimic_f_ptr = &f_info[feat];
        cptr name = f_name + mimic_f_ptr->name;

        oktomove = FALSE;

        /* Disturb the player */
        disturb(0, 0);

        /* Notice things in the dark */
        if (!(c_ptr->info & CAVE_MARK) && !player_can_see_bold(y, x))
        {
            /* Boundary floor mimic */
            if (boundary_floor(c_ptr, f_ptr, mimic_f_ptr))
            {
                msg_print("You feel you cannot go any more.");
            }

            /* Wall (or secret door) */
            else
            {
                msg_format("You feel %s %s blocking your way.",
                    is_a_vowel(name[0]) ? "an" : "a", name);

                c_ptr->info |= (CAVE_MARK | CAVE_AWARE);
                lite_spot(y, x);
            }
        }

        /* Notice things */
        else
        {
            /* Boundary floor mimic */
            if (boundary_floor(c_ptr, f_ptr, mimic_f_ptr))
            {
                msg_print("You cannot go any more.");

                if (!(p_ptr->confused || p_ptr->image))
                {
                    if (!shadow_strike)
                        energy_use = 0;
                }
            }

            /* Wall (or secret door) */
            else
            {
#ifdef ALLOW_EASY_OPEN
                /* Closed doors */
                if (easy_open && is_closed_door(feat) && easy_open_door(y, x, dir))
                {
                    /* Hack. Try to deduce what happened since easy_open_door hides this.
                       Try to repeat attempting to unlock the door, but do a quick check
                       for jammed doors so we don't waste 99 turns. Also, only make
                       99 attempts to pick the lock ... But using command_rep would be
                       unwise since we will then run thru the door once we pick the lock! */
                    if (always_repeat)
                    {
                        static int _repeat_count = 0;

                        cave_type *c_ptr = &cave[y][x];
                        feature_type *f_ptr = &f_info[c_ptr->feat];

                        if (is_closed_door(c_ptr->feat) && have_flag(f_ptr->flags, FF_OPEN))
                        {
                            if (_repeat_count == 0)
                                _repeat_count = 99;
                            else
                                --_repeat_count;

                            if (_repeat_count)
                                command_rep = 1;
                        }
                        else
                            _repeat_count = 0;
                    }
                    return;
                }
#endif /* ALLOW_EASY_OPEN */

                msg_format("There is %s %s blocking your way.",
                    is_a_vowel(name[0]) ? "an" : "a", name);

                /*
                 * Well, it makes sense that you lose time bumping into
                 * a wall _if_ you are confused or blind; but
                 * typing mistakes should not cost you a turn...
                 */
                if (!(p_ptr->confused || p_ptr->image))
                {
                    if (!shadow_strike)
                        energy_use = 0;
                }
            }
        }

        /* Sound */
        if (!boundary_floor(c_ptr, f_ptr, mimic_f_ptr)) sound(SOUND_HITWALL);
    }

    if (oktomove)
    {
        if (have_flag(f_ptr->flags, FF_CAN_PASS) && !p_can_kill_walls && !elemental_is_(ELEMENTAL_EARTH))
            energy_use = energy_use * 3 / 2;

        if (have_flag(f_ptr->flags, FF_LAVA) && elemental_is_(ELEMENTAL_FIRE))
            energy_use /= 2;

        if (have_flag(f_ptr->flags, FF_WATER) && elemental_is_(ELEMENTAL_WATER))
            energy_use /= 2;
    }

    /* Normal movement */
    if (oktomove && !pattern_seq(py, px, y, x))
    {
        if (!(p_ptr->confused || p_ptr->image))
        {
            if (!shadow_strike)
                energy_use = 0;
        }

        /* To avoid a loop with running */
        disturb(0, 0);

        oktomove = FALSE;
    }

    /* Normal movement */
    if (oktomove)
    {
        static bool _tulppa = FALSE;
        u32b mpe_mode = MPE_ENERGY_USE;

        if ((p_ptr->warning) && (!_glacier_hack))
        {
            if (!process_warning(x, y))
            {
                if (!shadow_strike)
                    energy_use = 25;
                return;
            }
        }

        if (do_past)
        {
            msg_format("You push past %s.", m_name);
        }

        /* Change oldpx and oldpy to place the player well when going back to big mode */
        if (p_ptr->wild_mode)
        {
            if (ddy[dir] > 0)  p_ptr->oldpy = 1;
            if (ddy[dir] < 0)  p_ptr->oldpy = MAX_HGT - 2;
            if (ddy[dir] == 0) p_ptr->oldpy = MAX_HGT / 2;
            if (ddx[dir] > 0)  p_ptr->oldpx = 1;
            if (ddx[dir] < 0)  p_ptr->oldpx = MAX_WID - 2;
            if (ddx[dir] == 0) p_ptr->oldpx = MAX_WID / 2;
            p_ptr->wilderness_dx = 0;
            p_ptr->wilderness_dy = 0;
        }

        if (p_can_kill_walls)
        {
            cave_alter_feat(y, x, FF_HURT_DISI);

            /* Update some things -- similar to GF_KILL_WALL */
            p_ptr->update |= (PU_FLOW);
        }

        /* Sound */
        /* sound(SOUND_WALK); */

#ifdef ALLOW_EASY_DISARM /* TNB */

        if (do_pickup != always_pickup) mpe_mode |= MPE_DO_PICKUP;
        if (do_pickup) mpe_mode |= MPE_DO_JUMP;

#else /* ALLOW_EASY_DISARM -- TNB */

        if (do_pickup) mpe_mode |= MPE_DO_PICKUP;
        if (do_pickup != always_pickup) mpe_mode |= MPE_DO_JUMP;

#endif /* ALLOW_EASY_DISARM -- TNB */

        if (break_trap) mpe_mode |= MPE_BREAK_TRAP;

        if (_glacier_hack)
        {
            mpe_mode = 0;
            _glacier_slip(f_ptr);
        }

        if ((dungeon_type == DUNGEON_AUSSIE) && (!_tulppa) && (dun_level > 30) && (c_ptr->info & CAVE_ROOM) &&
            (!(cave[py][px].info & CAVE_ROOM)) && (!have_flag(f_ptr->flags, FF_WATER)))
        {
            bool loytyi = FALSE;
            int y2, x2, i;

            /* Check for water nearby */
            if (!_tulppa)
            {
                cave_type *c2_ptr;
                for (i = 0; i < 4; i++)
                {
                    y2 = y + ddy_ddd[i];
                    x2 = x + ddx_ddd[i];
                    if (!in_bounds(y2, x2)) continue;
                    c2_ptr = &cave[y2][x2];
                    if ((c2_ptr->info & CAVE_ROOM) && (have_flag(f_info[c2_ptr->feat].flags, FF_WATER)))
                    {
                        loytyi = TRUE;
                        break;
                    }
                }
                if (loytyi)
                {
                    msg_print("You hear the ghostly song of a dead swagman as you approach the billabong.");
                    _tulppa = TRUE;
                }
            }
        }

        else if ((have_flag(f_ptr->flags, FF_SLIPPERY)) && (!p_ptr->levitation) && (!p_ptr->pass_wall) &&
                 (!p_ptr->wild_mode) && (!p_ptr->riding) && ((p_ptr->confused) || (p_ptr->image) || (one_in_(5))))
        {
            bool sattui = FALSE;
            int i, yrkat = randint1(10);
            int _save = p_ptr->stat_ind[A_DEX] * 3;
            if (p_ptr->confused) _save /= 2;
            if (!mpe_mode)
            {
                _save = 100; /* hack - avoid loop */
                _glacier_hack = FALSE;
                sattui = TRUE; /* hack - suppress message */
            }
            if (_save < 100)
            {
                int boots;
                for (boots = equip_find_first(object_is_boots); boots; boots = equip_find_next(object_is_boots, boots))
                {
                    object_type *o_ptr = equip_obj(boots);
                    u32b flags[OF_ARRAY_SIZE];
                    obj_flags(o_ptr, flags);
                    if (have_flag(flags, OF_RES_COLD))
                    {
                        _save += (p_ptr->confused ? 30 : 50);
                        obj_learn_flag(o_ptr, OF_RES_COLD);
                        break;
                    }
                }
            }
            if (_save < 100)
            {
                int ow_level = 0, stun_plus = 0, cut_plus = 0, conf_plus = 0;
                for (i = 0; i < yrkat; i++)
                {
                    if (saving_throw(_save)) continue;
                    if (!sattui)
                    {
                        sattui = TRUE;
                        if (saving_throw(_save))
                        {
                            msg_print("You briefly lose your balance on the slippery ice!");
                            energy_use += 25;
                            break;
                        }
                        else
                        {
                            msg_print("You lose your footing and fall on the slippery ice!");
                            ow_level = randint0(MAX(5, 25 - ((p_ptr->ac + p_ptr->to_a) / 8))) + 5;
                            take_hit(DAMAGE_NOESCAPE, ow_level, "slipping on ice");
                            if (hex_spelling_any()) stop_hex_spell_all();
                            p_ptr->counter = FALSE;
                            energy_use += 100;
                        }
                    }
                    switch (randint1(12))
                    {
                        case 1:
                            stun_plus += ow_level;
                            break;
                        case 2:
                            if (!res_save_default(RES_SOUND)) stun_plus += ow_level;
                            break;
                        case 3:
                            inven_damage(0, set_cold_destroy, 3, RES_SOUND);
                            break;
                        case 4:
                            inven_damage(0, set_cold_destroy, 3, RES_SHARDS);
                            break;
                        case 5:
                        case 6:
                            if (!res_save_default(RES_CONF)) conf_plus += randint1(3);
                            break;
                        case 7:
                            cut_plus += randint1(ow_level);
                            break;
                        case 8:
                            if (!res_save_default(RES_SHARDS)) cut_plus += (ow_level + 1) / 2;
                            break;
                        case 9: /* give the lucky guy a free pass */
                            if (i < yrkat) i = yrkat;
                            break;
                        case 10:
                            if (have_flag(f_ptr->flags, FF_MOUNTAIN)) _glacier_hack = TRUE;
                            break;
                        default: break;
                    }
                }
                if (cut_plus) set_cut(p_ptr->cut + cut_plus, FALSE);
                if (stun_plus) set_stun(p_ptr->stun + stun_plus, FALSE);
                if (conf_plus) set_confused(p_ptr->confused + conf_plus, FALSE);
            }
            if (!sattui)
            {
                msg_print("You nimbly maintain your balance on the slippery ice!");
            }
            else
            {
                disturb(0,0);
                if (p_ptr->special_defense & (KATA_MASK)) lose_kata();
                else if (p_ptr->special_defense & (KAMAE_MASK)) lose_kamae();
                if (p_ptr->concent) reset_concentration(TRUE);
                mpe_mode |= (MPE_DONT_PICKUP);
            }
        }

        if (!mpe_mode) /* clean up after glacier hack */
        {
            mpe_mode = MPE_ENERGY_USE;
        }

        /* Move the player */
        (void)move_player_effect(y, x, mpe_mode);

        if (_glacier_hack)
        /* Keep moving */
        {
            if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
               _glacier_hack = FALSE; /* Code too achy */
            else if (p_ptr->riding) _glacier_hack = FALSE; /* Paranoia */
            else
            {
                travel_cancel_fully();
                move_player(dir, FALSE, FALSE);
            }
        }
    }
}


static bool ignore_avoid_run;

/*
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
    cave_type   *c_ptr;

    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are not known walls */
    if (!in_bounds2(y, x)) return (FALSE);

    /* Access grid */
    c_ptr = &cave[y][x];

    /* Must be known to the player */
    if (c_ptr->info & (CAVE_MARK))
    {
        /* Feature code (applying "mimic" field) */
        s16b         feat = get_feat_mimic(c_ptr);
        feature_type *f_ptr = &f_info[feat];

        /* Wall grids are known walls */
        if (!player_can_enter(feat, 0)) return !have_flag(f_ptr->flags, FF_DOOR);

        /* Don't run on a tree unless explicitly requested */
        if (have_flag(f_ptr->flags, FF_AVOID_RUN) && !ignore_avoid_run)
            return TRUE;

        /* Don't run in a wall */
        if (!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY))
            return !have_flag(f_ptr->flags, FF_DOOR);
    }

    return FALSE;
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are unknown */
    if (!in_bounds2(y, x)) return (TRUE);

    /* Memorized grids are always known */
    if (cave[y][x].info & (CAVE_MARK)) return (FALSE);

    /* Viewable door/wall grids are known */
    if (player_can_see_bold(y, x)) return (FALSE);

    /* Default */
    return (TRUE);
}





/*
 * The running algorithm:                       -CJS-
 *
 * In the diagrams below, the player has just arrived in the
 * grid marked as '@', and he has just come from a grid marked
 * as 'o', and he is about to enter the grid marked as 'x'.
 *
 * Of course, if the "requested" move was impossible, then you
 * will of course be blocked, and will stop.
 *
 * Overview: You keep moving until something interesting happens.
 * If you are in an enclosed space, you follow corners. This is
 * the usual corridor scheme. If you are in an open space, you go
 * straight, but stop before entering enclosed space. This is
 * analogous to reaching doorways. If you have enclosed space on
 * one side only (that is, running along side a wall) stop if
 * your wall opens out, or your open space closes in. Either case
 * corresponds to a doorway.
 *
 * What happens depends on what you can really SEE. (i.e. if you
 * have no light, then running along a dark corridor is JUST like
 * running in a dark room.) The algorithm works equally well in
 * corridors, rooms, mine tailings, earthquake rubble, etc, etc.
 *
 * These conditions are kept in static memory:
 * find_openarea         You are in the open on at least one
 * side.
 * find_breakleft        You have a wall on the left, and will
 * stop if it opens
 * find_breakright       You have a wall on the right, and will
 * stop if it opens
 *
 * To initialize these conditions, we examine the grids adjacent
 * to the grid marked 'x', two on each side (marked 'L' and 'R').
 * If either one of the two grids on a given side is seen to be
 * closed, then that side is considered to be closed. If both
 * sides are closed, then it is an enclosed (corridor) run.
 *
 * LL           L
 * @x          LxR
 * RR          @R
 *
 * Looking at more than just the immediate squares is
 * significant. Consider the following case. A run along the
 * corridor will stop just before entering the center point,
 * because a choice is clearly established. Running in any of
 * three available directions will be defined as a corridor run.
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * #.#
 * ##.##
 * .@x..
 * ##.##
 * #.#
 *
 * Likewise, a run along a wall, and then into a doorway (two
 * runs) will work correctly. A single run rightwards from @ will
 * stop at 1. Another run right and down will enter the corridor
 * and make the corner, stopping at the 2.
 *
 * ##################
 * o@x       1
 * ########### ######
 * #2          #
 * #############
 *
 * After any move, the function area_affect is called to
 * determine the new surroundings, and the direction of
 * subsequent moves. It examines the current player location
 * (at which the runner has just arrived) and the previous
 * direction (from which the runner is considered to have come).
 *
 * Moving one square in some direction places you adjacent to
 * three or five new squares (for straight and diagonal moves
 * respectively) to which you were not previously adjacent,
 * marked as '!' in the diagrams below.
 *
 *   ...!              ...
 *   .o@!  (normal)    .o.!  (diagonal)
 *   ...!  (east)      ..@!  (south east)
 *                      !!!
 *
 * You STOP if any of the new squares are interesting in any way:
 * for example, if they contain visible monsters or treasure.
 *
 * You STOP if any of the newly adjacent squares seem to be open,
 * and you are also looking for a break on that side. (that is,
 * find_openarea AND find_break).
 *
 * You STOP if any of the newly adjacent squares do NOT seem to be
 * open and you are in an open area, and that side was previously
 * entirely open.
 *
 * Corners: If you are not in the open (i.e. you are in a corridor)
 * and there is only one way to go in the new squares, then turn in
 * that direction. If there are more than two new ways to go, STOP.
 * If there are two ways to go, and those ways are separated by a
 * square which does not seem to be open, then STOP.
 *
 * Otherwise, we have a potential corner. There are two new open
 * squares, which are also adjacent. One of the new squares is
 * diagonally located, the other is straight on (as in the diagram).
 * We consider two more squares further out (marked below as ?).
 *
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid, and "check_dir" to the grid marked 's'.
 *
 * ##s
 * @x?
 * #.?
 *
 * If they are both seen to be closed, then it is seen that no benefit
 * is gained from moving straight. It is a known corner. To cut the
 * corner, go diagonally, otherwise go straight, but pretend you
 * stepped diagonally into that next location for a full view next
 * time. Conversely, if one of the ? squares is not seen to be closed,
 * then there is a potential choice. We check to see whether it is a
 * potential corner or an intersection/room entrance. If the square
 * two spaces straight ahead, and the space marked with 's' are both
 * unknown space, then it is a potential corner and enter if
 * find_examine is set, otherwise must stop because it is not a
 * corner. (find_examine option is removed and always is TRUE.)
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
static byte find_current;

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;

static bool adjacent_treasure;

/* Check for buried treasure in an adjacent square */
static bool _treasure_is_adjacent(int iy, int ix)
{
    bool loytyi = FALSE;
    if (find_ignore_veins) return TRUE; /* Don't waste time */
    else {
        int i, nx, ny;
        cave_type *c_ptr;
        s16b feat;
        for (i = 0; i < 9; i++) /* Note that we do check the square the player's standing on */
        {
            ny = iy + ddy_ddd[i];
            nx = ix + ddx_ddd[i];
            if (!in_bounds(ny, nx)) continue;
            c_ptr = &cave[ny][nx];
            feat = get_feat_mimic(c_ptr);
            if (have_flag(f_info[feat].flags, FF_HAS_GOLD))
            {
                loytyi = TRUE;
                break;
            }
        }
    }
    return loytyi;
}

/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                 p
 */
static void run_init(int dir)
{
    int             row, col, deepleft, deepright;
    int             i, shortleft, shortright;


    /* Save the direction */
    find_current = dir;

    /* Assume running straight */
    find_prevdir = dir;

    /* Assume looking for open area */
    find_openarea = TRUE;

    /* Assume not looking for breaks */
    find_breakright = find_breakleft = FALSE;

    /* Assume no nearby walls */
    deepleft = deepright = FALSE;
    shortright = shortleft = FALSE;

    /* Check adjacent treasure */
    adjacent_treasure = _treasure_is_adjacent(py, px);

    p_ptr->run_py = py;
    p_ptr->run_px = px;

    /* Find the destination grid */
    row = py + ddy[dir];
    col = px + ddx[dir];

    ignore_avoid_run = cave_have_flag_bold(row, col, FF_AVOID_RUN);

    /* Extract cycle index */
    i = chome[dir];

    /* Check for walls */
    if (see_wall(cycle[i+1], py, px))
    {
        find_breakleft = TRUE;
        shortleft = TRUE;
    }
    else if (see_wall(cycle[i+1], row, col))
    {
        find_breakleft = TRUE;
        deepleft = TRUE;
    }

    /* Check for walls */
    if (see_wall(cycle[i-1], py, px))
    {
        find_breakright = TRUE;
        shortright = TRUE;
    }
    else if (see_wall(cycle[i-1], row, col))
    {
        find_breakright = TRUE;
        deepright = TRUE;
    }

    /* Looking for a break */
    if (find_breakleft && find_breakright)
    {
        /* Not looking for open area */
        find_openarea = FALSE;

        /* Hack -- allow angled corridor entry */
        if (dir & 0x01)
        {
            if (deepleft && !deepright)
            {
                find_prevdir = cycle[i - 1];
            }
            else if (deepright && !deepleft)
            {
                find_prevdir = cycle[i + 1];
            }
        }

        /* Hack -- allow blunt corridor entry */
        else if (see_wall(cycle[i], row, col))
        {
            if (shortleft && !shortright)
            {
                find_prevdir = cycle[i - 2];
            }
            else if (shortright && !shortleft)
            {
                find_prevdir = cycle[i + 2];
            }
        }
    }
}

/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
    int         prev_dir, new_dir, check_dir = 0;
    int         row, col;
    int         i, max, inv;
    int         option = 0, option2 = 0;
    cave_type   *c_ptr;
    s16b        feat;
    feature_type *f_ptr;

    /* Where we came from */
    prev_dir = find_prevdir;

    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;

    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++)
    {
        s16b this_o_idx, next_o_idx = 0;

        /* New direction */
        new_dir = cycle[chome[prev_dir] + i];

        /* New location */
        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        /* Access grid */
        c_ptr = &cave[row][col];

        /* Feature code (applying "mimic" field) */
        feat = get_feat_mimic(c_ptr);
        f_ptr = &f_info[feat];

        /* Visible monsters abort running */
        if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];

            /* Visible monster */
            if (m_ptr->ml) return (TRUE);
        }

        /* Visible objects abort running */
        for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            object_type *o_ptr;

            /* Acquire object */
            o_ptr = &o_list[this_o_idx];

            /* Acquire next object */
            next_o_idx = o_ptr->next_o_idx;

            /* Visible object */
            if (o_ptr->marked & OM_FOUND) return (TRUE);
        }

        /* Assume unknown */
        inv = TRUE;

        /* Check memorized grids */
        if (c_ptr->info & (CAVE_MARK))
        {
            bool notice = have_flag(f_ptr->flags, FF_NOTICE);

            if (notice && have_flag(f_ptr->flags, FF_MOVE))
            {
                /* Open doors */
                if (find_ignore_doors && have_flag(f_ptr->flags, FF_DOOR) && have_flag(f_ptr->flags, FF_CLOSE))
                {
                    /* Option -- ignore */
                    notice = FALSE;
                }

                /* Stairs */
                else if (find_ignore_stairs && have_flag(f_ptr->flags, FF_STAIRS))
                {
                    /* Option -- ignore */
                    notice = FALSE;
                }

                /* Lava */
                else if (have_flag(f_ptr->flags, FF_LAVA) && (res_pct(RES_FIRE) >= 100 || IS_INVULN()))
                {
                    /* Ignore */
                    notice = FALSE;
                }

                /* Deep water */
                else if (have_flag(f_ptr->flags, FF_WATER) && have_flag(f_ptr->flags, FF_DEEP) &&
                         (p_ptr->levitation || p_ptr->can_swim || (py_total_weight() <= weight_limit())))
                {
                    /* Ignore */
                    notice = FALSE;
                }
            }

            /* Interesting feature */
            if (notice) return (TRUE);

            /* The grid is "visible" */
            inv = FALSE;
        }

        /* Analyze unknown grids and floors considering mimic */
        if (inv || !see_wall(0, row, col))
        {
            /* Looking for open area */
            if (find_openarea)
            {
                /* Nothing */
            }

            /* The first new direction. */
            else if (!option)
            {
                option = new_dir;
            }

            /* Three new directions. Stop running. */
            else if (option2)
            {
                return (TRUE);
            }

            /* Two non-adjacent new directions. Stop running. */
            else if (option != cycle[chome[prev_dir] + i - 1])
            {
                return (TRUE);
            }

            /* Two new (adjacent) directions (case 1) */
            else if (new_dir & 0x01)
            {
                check_dir = cycle[chome[prev_dir] + i - 2];
                option2 = new_dir;
            }

            /* Two new (adjacent) directions (case 2) */
            else
            {
                check_dir = cycle[chome[prev_dir] + i + 1];
                option2 = option;
                option = new_dir;
            }
        }

        /* Obstacle, while looking for open area */
        else
        {
            if (find_openarea)
            {
                if (i < 0)
                {
                    /* Break to the right */
                    find_breakright = TRUE;
                }

                else if (i > 0)
                {
                    /* Break to the left */
                    find_breakleft = TRUE;
                }
            }
        }
    }

    /* Looking for open area */
    if (find_openarea)
    {
        /* Hack -- look again */
        for (i = -max; i < 0; i++)
        {
            /* Unknown grid or non-wall */
            if (!see_wall(cycle[chome[prev_dir] + i], py, px))
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }
        }

        /* Hack -- look again */
        for (i = max; i > 0; i--)
        {
            /* Unknown grid or non-wall */
            if (!see_wall(cycle[chome[prev_dir] + i], py, px))
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }
        }
    }

    /* Not looking for open area */
    else
    {
        /* No options */
        if (!option)
        {
            return (TRUE);
        }

        /* One option */
        else if (!option2)
        {
            /* Primary option */
            find_current = option;

            /* No other options */
            find_prevdir = option;
        }

        /* Two options, examining corners */
        else if (!find_cut)
        {
            /* Primary option */
            find_current = option;

            /* Hack -- allow curving */
            find_prevdir = option2;
        }

        /* Two options, pick one */
        else
        {
            /* Get next location */
            row = py + ddy[option];
            col = px + ddx[option];

            /* Don't see that it is closed off. */
            /* This could be a potential corner or an intersection. */
            if (!see_wall(option, row, col) ||
                !see_wall(check_dir, row, col))
            {
                /* Can not see anything ahead and in the direction we */
                /* are turning, assume that it is a potential corner. */
                if (see_nothing(option, row, col) &&
                    see_nothing(option2, row, col))
                {
                    find_current = option;
                    find_prevdir = option2;
                }

                /* STOP: we are next to an intersection or a room */
                else
                {
                    return (TRUE);
                }
            }

            /* This corner is seen to be enclosed; we cut the corner. */
            else if (find_cut)
            {
                find_current = option2;
                find_prevdir = option2;
            }

            /* This corner is seen to be enclosed, and we */
            /* deliberately go the long way. */
            else
            {
                find_current = option;
                find_prevdir = option2;
            }
        }
    }

    /* About to hit a known wall, stop */
    if (see_wall(find_current, py, px))
    {
        return (TRUE);
    }

    /* Stop for treasure if find_ignore_treasure is off */
    if (!find_ignore_veins)
    {
        bool old_adj_treasure = adjacent_treasure;
        adjacent_treasure = _treasure_is_adjacent(py, px);
        if ((!old_adj_treasure) && (adjacent_treasure)) return (TRUE);
    }

    /* Failure */
    return (FALSE);
}

/*
 * Take one step along the current "run" path
 */
void run_step(int dir)
{
    bool ongelma = FALSE;
    /* Start running */
    if (dir)
    {
        /* Ignore AVOID_RUN on a first step */
        ignore_avoid_run = TRUE;

        /* Hack -- do not start silly run */
        if (see_wall(dir, py, px))
        {
            /* Message */
            msg_print("You cannot run in that direction.");

            /* Disturb */
            disturb(0, 0);

            /* Done */
            return;
        }

        /* Initialize */
        run_init(dir);
    }

    /* Keep running */
    else
    {
        /* Update run */
        if (run_test())
        {
            /* Disturb */
            disturb(0, 0);

            /* Done */
            return;
        }
        else if ((p_ptr->confused) && (randint0(4)))
        {
            int uus_dir = ddd[randint0(8)];
            if (uus_dir != find_current)
            {
                find_current = uus_dir;
                ongelma = TRUE;
                if (see_wall(uus_dir, py, px))
                {
                    msg_print("You are confused.");
                    disturb(0, 0);
                    return;
                }
            }
        }
    }

    /* Decrease the run counter */
    if (--running <= 0) return;

    /* Take time */
    energy_use = 100;

    /* Move the player, using the "pickup" flag */
#ifdef ALLOW_EASY_DISARM /* TNB */

    move_player(find_current, FALSE, FALSE);

#else /* ALLOW_EASY_DISARM -- TNB */

    move_player(find_current, always_pickup, FALSE);

#endif /* ALLOW_EASY_DISARM -- TNB */

    if (ongelma)
    {
        msg_print("You are confused.");
        disturb(0, 0);
        return;
    }
    if (player_bold(p_ptr->run_py, p_ptr->run_px))
    {
        p_ptr->run_py = 0;
        p_ptr->run_px = 0;
        disturb(0, 0);
    }
}


static bool travel_abort(void)
{
    int prev_dir, new_dir;
    int row, col;
    int i, max;
    bool stop = TRUE;
    cave_type *c_ptr;

    /* Where we came from */
    prev_dir = find_prevdir;

    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;

    for (i = 0; i < 8; i++)
    {
        if (travel.cost[py+ddy_ddd[i]][px+ddx_ddd[i]] < travel.cost[py][px]) stop = FALSE;
    }

    if (stop) return (TRUE);

    /* Cannot travel when blind */
    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");
        return (TRUE);
    }

    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++)
    {
        /* New direction */
        new_dir = cycle[chome[prev_dir] + i];

        /* New location */
        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        if (!in_bounds(row, col)) continue;

        /* Access grid */
        c_ptr = &cave[row][col];

        if (disturb_trap_detect)
        {
            bool old_dtrap = FALSE;
            bool new_dtrap = FALSE;

            if (cave[py][px].info & CAVE_IN_DETECT)
                old_dtrap = TRUE;

            if (c_ptr->info & CAVE_IN_DETECT)
                new_dtrap = TRUE;

            if (old_dtrap && !new_dtrap && !_auto_detect_traps())
            {
                cmsg_print(TERM_VIOLET, "You are about to leave a trap detected zone.");
                return TRUE;
            }
        }

        /* Visible monsters abort running after the first step */
		if (c_ptr->m_idx && travel.run != 255)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];

			/* Visible monster */
			if (m_ptr->ml)
				return TRUE;
		}
    }

    return FALSE;
}

static int travel_cost(point_t pt)
{
    return travel.cost[pt.y][pt.x];
}

void travel_step(void)
{
    int i;
    int dir = 0;
    int old_run = travel.run;
    int dirs[8] = { 2, 4, 6, 8, 1, 7, 9, 3 };
    point_t pt_best = {0};
    int py_old=py;
    int px_old=px;
    bool ongelma = FALSE;
    bool door_hack = FALSE;

    find_prevdir = travel.dir;

    if (travel_abort())
    {
        if (travel.run == 255)
            msg_print("No route is found!");
        disturb(0, 0);
        return;
    }

    energy_use = 100;

    for (i = 0; i < 8; i++)
    {
        int     d = dirs[i];
        point_t pt = point(px + ddx[d], py + ddy[d]);

        if (!dir || travel_cost(pt) < travel_cost(pt_best))
        {
            dir = d;
            pt_best = pt;
        }
    }

    /* Travelling is bumping into jammed doors and getting stuck */
    if (is_jammed_door(cave[pt_best.y][pt_best.x].feat))
    {
        disturb(0, 0);
        return;
    }

    /* Closed door */
    else if (is_closed_door(cave[pt_best.y][pt_best.x].feat))
    {
        door_hack = TRUE;
        if (!easy_open)
        {
            disturb(0, 0);
            return;
        }
    }
    /* Travelling is bumping into mountains and permanent walls and getting stuck */
    else if (!player_can_enter(cave[pt_best.y][pt_best.x].feat, 0))
    {
        disturb(0, 0);
        return;
    }

    command_dir = dir;
    if ((p_ptr->confused) && (randint0(4))) /* paranoia - get_rep_dir() doesn't handle this situation well */
    {
        command_dir = ddd[randint0(8)];
        dir = command_dir;
        ongelma = TRUE;
    }
    else if (get_rep_dir(&dir, FALSE) == GET_DIR_RANDOM)
    {
        ongelma = TRUE;
    }
    travel.dir = dir;
    move_player(dir, always_pickup, easy_disarm);
    Term_xtra(TERM_XTRA_DELAY, delay_time());
    Term_fresh();
    travel.run = old_run;

    if (((py == travel.y) && (px == travel.x)) || ((py == py_old)&&(px == px_old) && (!door_hack)) || (ongelma))
    {
        travel_end();
    }
    else
        travel.run--;
}


