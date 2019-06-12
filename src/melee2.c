/* File: melee2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Monster spells and movement */

/*
* This file has several additions to it by Keldon Jones (keldon@umr.edu)
* to improve the general quality of the AI (version 0.1.1).
*/

#include "angband.h"

#define SPEAK_CHANCE 8
#define GRINDNOISE 20
#define CYBERNOISE 20

/*
 * Calculate the direction to the next enemy
 */
static bool get_enemy_dir(int m_idx, int *mm)
{
    int i;
    int x = 0, y = 0;
    int t_idx;
    int start;
    int plus = 1;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    monster_type *t_ptr;

    if (riding_t_m_idx && player_bold(m_ptr->fy, m_ptr->fx))
    {
        y = m_list[riding_t_m_idx].fy;
        x = m_list[riding_t_m_idx].fx;
    }
    else if (is_pet(m_ptr) && pet_t_m_idx)
    {
        y = m_list[pet_t_m_idx].fy;
        x = m_list[pet_t_m_idx].fx;
    }
    else
    {
        if (p_ptr->inside_battle)
        {
            start = randint1(m_max-1)+m_max;
            if(randint0(2)) plus = -1;
        }
        else start = m_max + 1;

        /* Scan thru all monsters */
        for (i = start; ((i < start + m_max) && (i > start - m_max)); i+=plus)
        {
            int dummy = (i % m_max);

            if (!dummy) continue;

            t_idx = dummy;
            t_ptr = &m_list[t_idx];

            /* The monster itself isn't a target */
            if (t_ptr == m_ptr) continue;

            /* Paranoia -- Skip dead monsters */
            if (!t_ptr->r_idx) continue;

            if (is_pet(m_ptr))
            {
                /* Hack -- only fight away from player */
                if (p_ptr->pet_follow_distance < 0)
                {
                    /* No fighting near player */
                    if (t_ptr->cdis <= (0 - p_ptr->pet_follow_distance))
                    {
                        continue;
                    }
                }
                /* Hack -- no fighting away from player */
                else if ((m_ptr->cdis < t_ptr->cdis) &&
                            (t_ptr->cdis > p_ptr->pet_follow_distance))
                {
                    continue;
                }

                if (r_ptr->aaf < t_ptr->cdis) continue;
            }

            /* Monster must be 'an enemy' */
            if (!are_enemies(m_ptr, t_ptr)) continue;

            /* Monster must be projectable if we can't pass through walls */
            if (((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != p_ptr->riding) || p_ptr->pass_wall)) ||
                ((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != p_ptr->riding)))
            {
                if (!in_disintegration_range(m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx)) continue;
            }
            else
            {
                if (!projectable(m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx)) continue;
            }

            /* OK -- we've got a target */
            y = t_ptr->fy;
            x = t_ptr->fx;

            break;
        }
        if (!x && !y) return FALSE;
    }

    /* Extract the direction */
    x -= m_ptr->fx;
    y -= m_ptr->fy;

    /* Check for long melee */
    if ((r_ptr->flags7 & RF7_RANGED_MELEE) && (MAX(ABS(x), ABS(y)) == 2) && (MIN(ABS(x), ABS(y)) < 2) && (very_clean_shot(m_ptr->fy, m_ptr->fx, m_ptr->fy + y, m_ptr->fx + x)))
    {
        mm[0] = mm[1] = mm[2] = (30 - (5 * x) - y);
        return (TRUE);
    }

    /* North */
    if ((y < 0) && (x == 0))
    {
        mm[0] = 8;
        mm[1] = 7;
        mm[2] = 9;
    }
    /* South */
    else if ((y > 0) && (x == 0))
    {
        mm[0] = 2;
        mm[1] = 1;
        mm[2] = 3;
    }
    /* East */
    else if ((x > 0) && (y == 0))
    {
        mm[0] = 6;
        mm[1] = 9;
        mm[2] = 3;
    }
    /* West */
    else if ((x < 0) && (y == 0))
    {
        mm[0] = 4;
        mm[1] = 7;
        mm[2] = 1;
    }
    /* North-West */
    else if ((y < 0) && (x < 0))
    {
        mm[0] = 7;
        mm[1] = 4;
        mm[2] = 8;
    }
    /* North-East */
    else if ((y < 0) && (x > 0))
    {
        mm[0] = 9;
        mm[1] = 6;
        mm[2] = 8;
    }
    /* South-West */
    else if ((y > 0) && (x < 0))
    {
        mm[0] = 1;
        mm[1] = 4;
        mm[2] = 2;
    }
    /* South-East */
    else if ((y > 0) && (x > 0))
    {
        mm[0] = 3;
        mm[1] = 6;
        mm[2] = 2;
    }

    /* Found a monster */
    return TRUE;
}


/*
 * Hack, based on mon_take_hit... perhaps all monster attacks on
 * other monsters should use this?
 */
void mon_take_hit_mon(int m_idx, int dam, bool *fear, cptr note, int who)
{
    monster_type    *m_ptr = &m_list[m_idx];

    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    bool             who_is_pet = FALSE;

    char m_name[160];

    bool seen = mon_show_msg(m_ptr);

    /* Can the player be aware of this attack? */
    bool known = (m_ptr->cdis <= MAX_SIGHT);

    if (who && is_pet(&m_list[who]))
        who_is_pet = TRUE;

    /* Extract monster name */
    monster_desc(m_name, m_ptr, 0);

    /* Redraw (later) if needed */
    if (m_ptr->ml)
    {
        check_mon_health_redraw(m_idx);
    }

    /* Wake it up */
    (void)set_monster_csleep(m_idx, 0);

    if (p_ptr->riding && (m_idx == p_ptr->riding)) disturb(1, 0);

    if (MON_INVULNER(m_ptr) && randint0(PENETRATE_INVULNERABILITY))
    {
        if (seen)
            msg_format("%^s is unharmed.", m_name);

        return;
    }

    if (r_ptr->flagsr & RFR_RES_ALL)
    {
        if(dam > 0)
        {
            dam /= 100;
            if((dam == 0) && one_in_(3)) dam = 1;
        }
        if (dam==0)
        {
            if (seen)
                msg_format("%^s is unharmed.", m_name);

            return;
        }
    }

    /* Mark monster as hurt */
    if (dam > 0) m_ptr->mflag2 |= MFLAG2_HURT;

    /* Hurt it */
    m_ptr->hp -= dam;

    /* It is dead now... or is it? */
    if (m_ptr->hp < 0)
    {
        if ( ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL) || (m_ptr->mflag2 & MFLAG2_QUESTOR))
          && !p_ptr->inside_battle
          && !prace_is_(RACE_MON_QUYLTHULG))
        {
            m_ptr->hp = 1;
        }
        else
        {
            /* Make a sound */
            if (!monster_living(r_ptr))
            {
                sound(SOUND_N_KILL);
            }
            else
            {
                sound(SOUND_KILL);
            }

            if (known)
            {
                monster_desc(m_name, m_ptr, MD_TRUE_NAME);
                /* Unseen death by normal attack */
                if (!seen)
                {
                    mon_fight = TRUE;
                }
                /* Death by special attack */
                else if (note)
                {
                    msg_format("%^s%s", m_name, note);
                }
                /* Death by normal attack -- nonliving monster */
                else if (!monster_living(r_ptr))
                {
                    msg_format("%^s is destroyed.", m_name);
                }
                /* Death by normal attack -- living monster */
                else
                {
                    msg_format("%^s is killed.", m_name);
                }
            }

            pack_on_slay_monster(m_idx);

            monster_gain_exp(who, m_idx);

            mon_check_kill_unique(m_idx);

            /* Generate treasure */
            monster_death(m_idx, who_is_pet);

            /* Delete the monster */
            delete_monster_idx(m_idx);

            /* Not afraid */
            (*fear) = FALSE;

            /* Monster is dead */
            return;
        }
    }

#ifdef ALLOW_FEAR

    /* Mega-Hack -- Pain cancels fear */
    if (MON_MONFEAR(m_ptr) && (dam > 0))
    {
        /* Cure fear */
        if (set_monster_monfear(m_idx, MON_MONFEAR(m_ptr) - randint1(dam / 4)))
        {
            /* No more fear */
            (*fear) = FALSE;
        }
    }

    /* Sometimes a monster gets scared by damage */
    if (!MON_MONFEAR(m_ptr) && !(r_ptr->flags3 & RF3_NO_FEAR))
    {
        /* Percentage of fully healthy */
        int percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

        /*
        * Run (sometimes) if at 10% or less of max hit points,
        * or (usually) when hit for half its current hit points
         */
        if (((percentage <= 10) && (randint0(10) < percentage)) ||
            ((dam >= m_ptr->hp) && (randint0(100) < 80)))
        {
            /* Hack -- note fear */
            (*fear) = TRUE;

            /* XXX XXX XXX Hack -- Add some timed fear */
            (void)set_monster_monfear(m_idx, (randint1(10) +
                (((dam >= m_ptr->hp) && (percentage > 7)) ?
                20 : ((11 - percentage) * 5))));
        }
    }

#endif /* ALLOW_FEAR */

    if ((dam > 0) && !is_pet(m_ptr) && !is_friendly(m_ptr) && (who != m_idx))
    {
        if (is_pet(&m_list[who]) && !player_bold(m_ptr->target_y, m_ptr->target_x))
        {
            set_target(m_ptr, m_list[who].fy, m_list[who].fx);
        }
    }

    if (p_ptr->riding && (p_ptr->riding == m_idx) && (dam > 0))
    {
        char m_name[80];

        /* Extract monster name */
        monster_desc(m_name, m_ptr, 0);

        if (m_ptr->hp > m_ptr->maxhp/3) dam = (dam + 1) / 2;
        if (rakuba((dam > 200) ? 200 : dam, FALSE))
        {
            msg_format("You are thrown off from %s!", m_name);
        }
    }

    /* Not dead yet */
    return;
}


/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players. See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency. Like not using "mod" or "div" when possible. And
 * attempting to check the conditions in an optimal order. Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

#ifdef ALLOW_TERROR

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    u16b p_lev, m_lev;
    u16b p_chp, p_mhp;
    u16b m_chp, m_mhp;
    u32b p_val, m_val;

#endif

    /* Friends can be commanded to avoid the player */
    if (is_pet(m_ptr))
    {
        /* Are we trying to avoid the player? */
        return ((p_ptr->pet_follow_distance < 0) &&
                  (m_ptr->cdis <= (0 - p_ptr->pet_follow_distance)));
    }

    /* Keep monsters from running too far away */
    if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

    /* All "afraid" monsters will run away */
    if (MON_MONFEAR(m_ptr)) return (TRUE);

#ifdef ALLOW_TERROR

    /* Nearby monsters will not become terrified */
    if (m_ptr->cdis <= 5) return (FALSE);

    if ( p_ptr->prace == RACE_MON_RING
      && !p_ptr->riding
      && !is_aware(m_ptr)
      && mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER) )
    {
        return FALSE;
    }

    /* Examine player power (level) */
    p_lev = p_ptr->lev;

    /* Examine monster power (level plus morale) */
    m_lev = r_ptr->level + (m_idx & 0x08) + 25;

    /* Optimize extreme cases below */
    if (m_lev > p_lev + 4) return (FALSE);
    if (m_lev + 4 <= p_lev) return (TRUE);

    /* Examine player health */
    p_chp = p_ptr->chp;
    p_mhp = p_ptr->mhp;

    /* Examine monster health */
    m_chp = m_ptr->hp;
    m_mhp = m_ptr->maxhp;

    /* Prepare to optimize the calculation */
    p_val = (p_lev * p_mhp) + (p_chp << 2); /* div p_mhp */
    m_val = (m_lev * m_mhp) + (m_chp << 2); /* div m_mhp */

    /* Strong players scare strong monsters */
    if (p_val * m_mhp > m_val * p_mhp) return (TRUE);

#endif

    /* Assume no terror */
    return (FALSE);
}




/*
 * Search spell castable grid
 */
static bool get_moves_aux2(int m_idx, int *yp, int *xp)
{
    int i, y, x, y1, x1, best = 999;

    cave_type *c_ptr;
    bool can_open_door = FALSE;
    int now_cost;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Monster location */
    y1 = m_ptr->fy;
    x1 = m_ptr->fx;

    /* Monster can already cast spell to player */
    if (projectable(y1, x1, py, px)) return (FALSE);

    /* Set current grid cost */
    now_cost = cave[y1][x1].cost;
    if (now_cost == 0) now_cost = 999;

    /* Can monster bash or open doors? */
    if (r_ptr->flags2 & (RF2_BASH_DOOR | RF2_OPEN_DOOR))
    {
        can_open_door = TRUE;
    }

    /* Check nearby grids, diagonals first */
    for (i = 7; i >= 0; i--)
    {
        int cost;

        /* Get the location */
        y = y1 + ddy_ddd[i];
        x = x1 + ddx_ddd[i];

        /* Ignore locations off of edge */
        if (!in_bounds2(y, x)) continue;

        /* Simply move to player */
        if (player_bold(y, x)) return (FALSE);

        c_ptr = &cave[y][x];

        cost = c_ptr->cost;

        /* Monster cannot kill or pass walls */
        if (!(((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != p_ptr->riding) || p_ptr->pass_wall)) || ((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != p_ptr->riding))))
        {
            if (cost == 0) continue;
            if (!can_open_door && is_closed_door(c_ptr->feat)) continue;
        }

        /* Hack -- for kill or pass wall monster.. */
        if (cost == 0) cost = 998;

        if (now_cost < cost) continue;

        if (!projectable(y, x, py, px)) continue;

        /* Accept louder sounds */
        if (best < cost) continue;
        best = cost;

        (*yp) = y1 + ddy_ddd[i];
        (*xp) = x1 + ddx_ddd[i];
    }

    /* No legal move (?) */
    if (best == 999) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Choose the "best" direction for "flowing"
 *
 * Note that ghosts and rock-eaters are never allowed to "flow",
 * since they should move directly towards the player.
 *
 * Prefer "non-diagonal" directions, but twiddle them a little
 * to angle slightly towards the player's actual location.
 *
 * Allow very perceptive monsters to track old "spoor" left by
 * previous locations occupied by the player. This will tend
 * to have monsters end up either near the player or on a grid
 * recently occupied by the player (and left via "teleport").
 *
 * Note that if "smell" is turned on, all monsters get vicious.
 *
 * Also note that teleporting away from a location will cause
 * the monsters who were chasing you to converge on that location
 * as long as you are still near enough to "annoy" them without
 * being close enough to chase directly. I have no idea what will
 * happen if you combine "smell" with low "aaf" values.
 */
static bool get_moves_aux(int m_idx, int *yp, int *xp, bool no_flow)
{
    int i, y, x, y1, x1, best;
    int rng = 16; /* <== I don't understand this value! */

    cave_type *c_ptr;
    bool use_scent = FALSE;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    if ( p_ptr->action == ACTION_GLITTER
      && mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER) )
    {
        rng = AAF_LIMIT_RING;
    }
    else if (py_on_surface())
        rng = AAF_LIMIT;

    if (mon_has_attack_spell(m_ptr))
    {
        /* Can move spell castable grid? */
        if (get_moves_aux2(m_idx, yp, xp)) return (TRUE);
    }

    /* Monster can't flow */
    if (no_flow) return (FALSE);

    /* Monster can go through rocks */
    if ( dungeon_type != DUNGEON_ARENA 
      && dungeon_type != DUNGEON_MOUNTAIN 
      && dungeon_type != DUNGEON_GIANTS_HALL )
    {
        if ((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != p_ptr->riding) || p_ptr->pass_wall))
            return FALSE;
        if ((r_ptr->flags2 & RF2_KILL_WALL) && (m_idx != p_ptr->riding))
            return FALSE;
    }

    /* Monster location */
    y1 = m_ptr->fy;
    x1 = m_ptr->fx;

    /* Hack -- Player can see us, run towards him */
    if (player_has_los_bold(y1, x1) && projectable(py, px, y1, x1)) return (FALSE);

    /* Monster grid */
    c_ptr = &cave[y1][x1];

    /* If we can hear noises, advance towards them */
    if (c_ptr->cost)
    {
        best = 999;
    }

    /* Otherwise, try to follow a scent trail */
    else if (c_ptr->when)
    {
        /* Too old smell */
        if (cave[py][px].when - c_ptr->when > 127) return (FALSE);

        use_scent = TRUE;
        best = 0;
    }

    /* Otherwise, advance blindly */
    else
    {
        return (FALSE);
    }

    /* Check nearby grids, diagonals first */
    for (i = 7; i >= 0; i--)
    {
        /* Get the location */
        y = y1 + ddy_ddd[i];
        x = x1 + ddx_ddd[i];

        /* Ignore locations off of edge */
        if (!in_bounds2(y, x)) continue;

        c_ptr = &cave[y][x];

        /* We're following a scent trail */
        if (use_scent)
        {
            int when = c_ptr->when;

            /* Accept younger scent */
            if (best > when) continue;
            best = when;
        }

        /* We're using sound */
        else
        {
            int cost;

            if (r_ptr->flags2 & (RF2_BASH_DOOR | RF2_OPEN_DOOR))
                cost = c_ptr->dist;
            else cost = c_ptr->cost;

            /* Accept louder sounds */
            if ((cost == 0) || (best < cost)) continue;
            /* Hack: Don't flow into occupied squares
               TODO: Don't flow into squares that the monster can't move into.
            */
            if (c_ptr->m_idx && !(r_ptr->flags2 & (RF2_MOVE_BODY|RF2_KILL_BODY))) continue;

            best = cost;
        }

        /* Hack -- Save the "twiddled" location */
        (*yp) = py + MAX(rng, r_ptr->aaf) * ddy_ddd[i];
        (*xp) = px + MAX(rng, r_ptr->aaf) * ddx_ddd[i];
    }

    /* No legal move (?) */
    if (best == 999 || best == 0) return (FALSE);

    /* Success */
    return (TRUE);
}


/*
* Provide a location to flee to, but give the player a wide berth.
*
* A monster may wish to flee to a location that is behind the player,
* but instead of heading directly for it, the monster should "swerve"
* around the player so that he has a smaller chance of getting hit.
*/
static bool get_fear_moves_aux(int m_idx, int *yp, int *xp)
{
    int y, x, y1, x1, fy, fx, gy = 0, gx = 0;
    int score = -1;
    int i;

    monster_type *m_ptr = &m_list[m_idx];

    /* Monster location */
    fy = m_ptr->fy;
    fx = m_ptr->fx;

    /* Desired destination */
    y1 = fy - (*yp);
    x1 = fx - (*xp);

    /* Check nearby grids, diagonals first */
    for (i = 7; i >= 0; i--)
    {
        int dis, s;

        /* Get the location */
        y = fy + ddy_ddd[i];
        x = fx + ddx_ddd[i];

        /* Ignore locations off of edge */
        if (!in_bounds2(y, x)) continue;

        /* Don't move toward player */
        /* if (cave[y][x].dist < 3) continue; */ /* Hmm.. Need it? */

        /* Calculate distance of this grid from our destination */
        dis = distance(y, x, y1, x1);

        /* Score this grid */
        s = 5000 / (dis + 3) - 500 / (cave[y][x].dist + 1);

        /* No negative scores */
        if (s < 0) s = 0;

        /* Ignore lower scores */
        if (s < score) continue;

        /* Save the score and time */
        score = s;

        /* Save the location */
        gy = y;
        gx = x;
    }

    /* No legal move (?) */
    if (score == -1) return (FALSE);

    /* Find deltas */
    (*yp) = fy - gy;
    (*xp) = fx - gx;

    /* Success */
    return (TRUE);
}

/*
 * Hack -- Precompute a bunch of calls to distance() in find_safety() and
 * find_hiding().
 *
 * The pair of arrays dist_offsets_y[n] and dist_offsets_x[n] contain the
 * offsets of all the locations with a distance of n from a central point,
 * with an offset of (0,0) indicating no more offsets at this distance.
 *
 * This is, of course, fairly unreadable, but it eliminates multiple loops
 * from the previous version.
 *
 * It is probably better to replace these arrays with code to compute
 * the relevant arrays, even if the storage is pre-allocated in hard
 * coded sizes. At the very least, code should be included which is
 * able to generate and dump these arrays (ala "los()"). XXX XXX XXX
 *
 * Also, the storage needs could be halved by using bytes. XXX XXX XXX
 *
 * These arrays could be combined into two big arrays, using sub-arrays
 * to hold the offsets and lengths of each portion of the sub-arrays, and
 * this could perhaps also be used somehow in the "look" code. XXX XXX XXX
 */


static sint d_off_y_0[] =
{ 0 };

static sint d_off_x_0[] =
{ 0 };


static sint d_off_y_1[] =
{ -1, -1, -1, 0, 0, 1, 1, 1, 0 };

static sint d_off_x_1[] =
{ -1, 0, 1, -1, 1, -1, 0, 1, 0 };


static sint d_off_y_2[] =
{ -1, -1, -2, -2, -2, 0, 0, 1, 1, 2, 2, 2, 0 };

static sint d_off_x_2[] =
{ -2, 2, -1, 0, 1, -2, 2, -2, 2, -1, 0, 1, 0 };


static sint d_off_y_3[] =
{ -1, -1, -2, -2, -3, -3, -3, 0, 0, 1, 1, 2, 2,
  3, 3, 3, 0 };

static sint d_off_x_3[] =
{ -3, 3, -2, 2, -1, 0, 1, -3, 3, -3, 3, -2, 2,
  -1, 0, 1, 0 };


static sint d_off_y_4[] =
{ -1, -1, -2, -2, -3, -3, -3, -3, -4, -4, -4, 0,
  0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 0 };

static sint d_off_x_4[] =
{ -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, -4, 4,
  -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, 0 };


static sint d_off_y_5[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -4, -4, -5, -5,
  -5, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5,
  5, 0 };

static sint d_off_x_5[] =
{ -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1, 0, 1,
  -5, 5, -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1,
  0, 1, 0 };


static sint d_off_y_6[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
  5, 5, 6, 6, 6, 0 };

static sint d_off_x_6[] =
{ -6, 6, -5, 5, -5, 5, -4, 4, -2, -3, 2, 3, -1,
  0, 1, -6, 6, -6, 6, -5, 5, -5, 5, -4, 4, -2,
  -3, 2, 3, -1, 0, 1, 0 };


static sint d_off_y_7[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, -6, -7, -7, -7, 0, 0, 1, 1, 2, 2, 3,
  3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 0 };

static sint d_off_x_7[] =
{ -7, 7, -6, 6, -6, 6, -5, 5, -4, -5, 4, 5, -2,
  -3, 2, 3, -1, 0, 1, -7, 7, -7, 7, -6, 6, -6,
  6, -5, 5, -4, -5, 4, 5, -2, -3, 2, 3, -1, 0,
  1, 0 };


static sint d_off_y_8[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -6, -6, -7, -7, -7, -7, -8, -8, -8, 0, 0, 1, 1,
  2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
  8, 8, 8, 0 };

static sint d_off_x_8[] =
{ -8, 8, -7, 7, -7, 7, -6, 6, -6, 6, -4, -5, 4,
  5, -2, -3, 2, 3, -1, 0, 1, -8, 8, -8, 8, -7,
  7, -7, 7, -6, 6, -6, 6, -4, -5, 4, 5, -2, -3,
  2, 3, -1, 0, 1, 0 };


static sint d_off_y_9[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -7, -7, -7, -7, -8, -8, -8, -8, -9, -9, -9, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7,
  7, 8, 8, 8, 8, 9, 9, 9, 0 };

static sint d_off_x_9[] =
{ -9, 9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4,
  -5, 4, 5, -2, -3, 2, 3, -1, 0, 1, -9, 9, -9,
  9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4, -5,
  4, 5, -2, -3, 2, 3, -1, 0, 1, 0 };


static sint *dist_offsets_y[10] =
{
    d_off_y_0, d_off_y_1, d_off_y_2, d_off_y_3, d_off_y_4,
    d_off_y_5, d_off_y_6, d_off_y_7, d_off_y_8, d_off_y_9
};

static sint *dist_offsets_x[10] =
{
    d_off_x_0, d_off_x_1, d_off_x_2, d_off_x_3, d_off_x_4,
    d_off_x_5, d_off_x_6, d_off_x_7, d_off_x_8, d_off_x_9
};

/*
* Choose a "safe" location near a monster for it to run toward.
*
* A location is "safe" if it can be reached quickly and the player
* is not able to fire into it (it isn't a "clean shot"). So, this will
* cause monsters to "duck" behind walls. Hopefully, monsters will also
* try to run towards corridor openings if they are in a room.
*
* This function may take lots of CPU time if lots of monsters are
* fleeing.
*
* Return TRUE if a safe location is available.
*/
static bool find_safety(int m_idx, int *yp, int *xp)
{
    monster_type *m_ptr = &m_list[m_idx];

    int fy = m_ptr->fy;
    int fx = m_ptr->fx;

    int y, x, dy, dx, d, dis, i;
    int gy = 0, gx = 0, gdis = 0;

    sint *y_offsets;
    sint *x_offsets;

    cave_type *c_ptr;

    /* Start with adjacent locations, spread further */
    for (d = 1; d < 10; d++)
    {
        /* Get the lists of points with a distance d from (fx, fy) */
        y_offsets = dist_offsets_y[d];
        x_offsets = dist_offsets_x[d];

        /* Check the locations */
        for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
             dx != 0 || dy != 0;
             i++, dx = x_offsets[i], dy = y_offsets[i])
        {
            y = fy + dy;
            x = fx + dx;

            /* Skip illegal locations */
            if (!in_bounds(y, x)) continue;

            c_ptr = &cave[y][x];

            /* Skip locations in a wall */
            if (!monster_can_cross_terrain(c_ptr->feat, &r_info[m_ptr->r_idx], (m_idx == p_ptr->riding) ? CEM_RIDING : 0)) continue;

            /* Check for "availability" (if monsters can flow) */
            if (!(m_ptr->mflag2 & MFLAG2_NOFLOW))
            {
                /* Ignore grids very far from the player */
                if (c_ptr->dist == 0) continue;

                /* Ignore too-distant grids */
                if (c_ptr->dist > cave[fy][fx].dist + 2 * d) continue;
            }

            /* Check for absence of shot (more or less) */
            if (!projectable(py, px, y, x))
            {
                /* Calculate distance from player */
                dis = distance(y, x, py, px);

                /* Remember if further than previous */
                if (dis > gdis)
                {
                    gy = y;
                    gx = x;
                    gdis = dis;
                }
            }
        }

        /* Check for success */
        if (gdis > 0)
        {
            /* Good location */
            (*yp) = fy - gy;
            (*xp) = fx - gx;

            /* Found safe place */
            return (TRUE);
        }
    }

    /* No safe place */
    return (FALSE);
}


/*
 * Choose a good hiding place near a monster for it to run toward.
 *
 * Pack monsters will use this to "ambush" the player and lure him out
 * of corridors into open space so they can swarm him.
 *
 * Return TRUE if a good location is available.
 */
static bool find_hiding(int m_idx, int *yp, int *xp)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    int fy = m_ptr->fy;
    int fx = m_ptr->fx;

    int y, x, dy, dx, d, dis, i;
    int gy = 0, gx = 0, gdis = 999;

    sint *y_offsets, *x_offsets;

    /* Start with adjacent locations, spread further */
    for (d = 1; d < 10; d++)
    {
        /* Get the lists of points with a distance d from (fx, fy) */
        y_offsets = dist_offsets_y[d];
        x_offsets = dist_offsets_x[d];

        /* Check the locations */
        for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
             dx != 0 || dy != 0;
             i++, dx = x_offsets[i], dy = y_offsets[i])
        {
            y = fy + dy;
            x = fx + dx;

            /* Skip illegal locations */
            if (!in_bounds(y, x)) continue;

            /* Skip occupied locations */
            if (!monster_can_enter(y, x, r_ptr, 0)) continue;

            /* Check for hidden, available grid */
            if (!projectable(py, px, y, x) && clean_shot(fy, fx, y, x, FALSE))
            {
                /* Calculate distance from player */
                dis = distance(y, x, py, px);

                /* Remember if closer than previous */
                if (dis < gdis && dis >= 2)
                {
                    gy = y;
                    gx = x;
                    gdis = dis;
                }
            }
        }

        /* Check for success */
        if (gdis < 999)
        {
            /* Good location */
            (*yp) = fy - gy;
            (*xp) = fx - gx;

            /* Found good place */
            return (TRUE);
        }
    }

    /* No good place */
    return (FALSE);
}


/*
 * Choose "logical" directions for monster movement
 */
static bool get_moves(int m_idx, int *mm)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    pack_info_t  *pack_ptr = pack_info_ptr(m_idx);
    int          y = 0, ay, x = 0, ax;
    int          move_val = 0;
    int          y2 = py;
    int          x2 = px;
    bool         done = FALSE;
    bool         will_run = mon_will_run(m_idx);
    cave_type    *c_ptr;
    bool         no_flow = ((m_ptr->mflag2 & MFLAG2_NOFLOW) && (cave[m_ptr->fy][m_ptr->fx].cost > 2));
    bool         can_pass_wall = ((r_ptr->flags2 & RF2_PASS_WALL) && ((m_idx != p_ptr->riding) || p_ptr->pass_wall));
    bool         allow_long_melee = ((r_ptr->flags7 & RF7_RANGED_MELEE) && (!MON_CONFUSED(m_ptr)) && (!will_run));
    bool         use_long_melee = FALSE;
    bool         is_clean_shot = ((allow_long_melee) && ((m_ptr->cdis == 2) || ((m_ptr->cdis == 3) && (ABS(m_ptr->fy - py) == 2) && (ABS(m_ptr->fx - px) == 2))) && (is_hostile(m_ptr)) && (very_clean_shot(m_ptr->fy, m_ptr->fx, py, px)));

    if (pack_ptr)
    {
        if (pack_ptr->ai == AI_FEAR)
        {
            int odds = 500 - m_ptr->cdis * m_ptr->cdis;

            /* Recover from fear? */
            if (odds <= 1 || one_in_(odds))
                pack_ptr->ai = AI_SEEK;
            else if (m_ptr->cdis > 5)
                will_run = TRUE;
        }
        else if (pack_ptr->ai == AI_MAINTAIN_DISTANCE)
        {
            if ( 1 < m_ptr->cdis
              && m_ptr->cdis <= pack_ptr->distance
              && p_ptr->chp >= p_ptr->mhp * 4 / 5 ) /* If @ wounded, pursue! */
            {
                will_run = TRUE;
            }
        }
        /* Change Tactics?  This is still pretty lame ...
        else if (m_ptr->cdis < 3 && pack_ptr->ai != AI_SEEK)
        {
            if (los(m_ptr->fy, m_ptr->fx, py, px))
                pack_ptr->ai = AI_SEEK;
        }
        else if (m_ptr->cdis > 15 && pack_ptr->ai == AI_SEEK)
        {
            pack_ptr->ai = AI_LURE;
        }
        */
    }

    /* Counter attack to an enemy monster */
    if (!will_run && m_ptr->target_y)
    {
        int t_m_idx = cave[m_ptr->target_y][m_ptr->target_x].m_idx;

        /* The monster must be an enemy, and in LOS */
        if (t_m_idx &&
            are_enemies(m_ptr, &m_list[t_m_idx]) &&
            los(m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x) &&
            projectable(m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x))
        {
            /* Extract the "pseudo-direction" */
            y = m_ptr->fy - m_ptr->target_y;
            x = m_ptr->fx - m_ptr->target_x;
            if ((allow_long_melee) && (ABS(y) < 3) && (ABS(x) < 3) && ((ABS(y) == 2) || (ABS(x) == 2)) && (very_clean_shot(m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x))) use_long_melee = TRUE;
            done = TRUE;
        }
    }

    /* Handle Packs ... */
    if (!done && !will_run && is_hostile(m_ptr) && pack_ptr)
    {
        /* Check for ranged melee */
        if (is_clean_shot)
        {
            y = m_ptr->fy - py;
            x = m_ptr->fx - px;
            use_long_melee = TRUE;
            done = TRUE;
        }

        if (!done && pack_ptr->ai == AI_LURE &&  !can_pass_wall && !(r_ptr->flags2 & RF2_KILL_WALL))
        {
            int i, room = 0;

            /* Count room grids next to player */
            for (i = 0; i < 8; i++)
            {
                int xx = px + ddx_ddd[i];
                int yy = py + ddy_ddd[i];

                if (!in_bounds2(yy, xx)) continue;

                c_ptr = &cave[yy][xx];

                /* Check grid */
                if (monster_can_cross_terrain(c_ptr->feat, r_ptr, 0))
                {
                    /* One more room grid */
                    room++;
                }
            }
            if (cave[py][px].info & CAVE_ROOM) room -= 2;
            if (!r_ptr->spells) room -= 2;

            /* Not in a room and strong player */
            if (room < (8 * (p_ptr->chp + p_ptr->csp)) /
                (p_ptr->mhp + p_ptr->msp))
            {
                /* Find hiding place */
                if (find_hiding(m_idx, &y, &x)) done = TRUE;
            }
        }

        if (!done && pack_ptr->ai == AI_SHOOT && m_ptr->cdis > 1)
        {
            x = 0;
            y = 0;
            done = TRUE;
        }
        if (!done && pack_ptr->ai == AI_GUARD_POS
          && m_ptr->cdis > 3    /* abandon guarding if the player gets close ... */
          && m_ptr->anger < 10 ) /* ... or if we get ticked off. */
        {
            x = m_ptr->fx - pack_ptr->guard_x;
            y = m_ptr->fy - pack_ptr->guard_y;
            done = TRUE;
        }

        if (!done && pack_ptr->ai == AI_GUARD_MON)
        {
            monster_type *m_ptr2 = &m_list[pack_ptr->guard_idx];
            if (!m_ptr2->r_idx)
                pack_ptr->ai = AI_SEEK;    /* detect a dead guardian */
            else if ( m_idx != pack_ptr->guard_idx
                   && m_ptr->cdis > 3 )
            {
                x = m_ptr->fx - m_ptr2->fx;
                y = m_ptr->fy - m_ptr2->fy;
                done = TRUE;
            }
        }

        /* Monster groups try to surround the player */
        if (!done && (cave[m_ptr->fy][m_ptr->fx].dist < 3))
        {
            int i;

            if (pack_ptr->leader_idx == m_idx || pack_ptr->count < 5)
            {
                /* Skip pack processing and flow normally towards player ... */
            }
            else
            {
                /* Find an empty square near the player to fill */
                for (i = 0; i < 8; i++)
                {
                    /* Pick squares near player (semi-randomly) */
                    y2 = py + ddy_ddd[(m_idx + i) & 7];
                    x2 = px + ddx_ddd[(m_idx + i) & 7];

                    /* Already there? */
                    if ((m_ptr->fy == y2) && (m_ptr->fx == x2))
                    {
                        /* Attack the player */
                        y2 = py;
                        x2 = px;

                        break;
                    }

                    if (!in_bounds2(y2, x2)) continue;

                    /* Ignore filled grids */
                    if (!monster_can_enter(y2, x2, r_ptr, 0)) continue;

                    /* Try to fill this hole */
                    break;
                }

                /* Extract the new "pseudo-direction" */
                y = m_ptr->fy - y2;
                x = m_ptr->fx - x2;

                done = TRUE;
            }
        }
    }
    if ((!done) && (is_clean_shot))
    {
        y = m_ptr->fy - py;
        x = m_ptr->fx - px;
        use_long_melee = TRUE;
        done = TRUE;
    }

    if (!done)
    {
        /* Flow towards the player ... Note (x2,y2) == (px,py). If
         * get_moves_aux fails (returns FALSE) then we proceed directly
         * towards the player. */
        (void)get_moves_aux(m_idx, &y2, &x2, no_flow);

        /* Extract the "pseudo-direction" */
        y = m_ptr->fy - y2;
        x = m_ptr->fx - x2;

        /* Not done */
    }

    /* Apply fear if possible and necessary */
    if (is_pet(m_ptr) && will_run)
    {
        /* XXX XXX Not very "smart" */
        y = (-y), x = (-x);
    }
    else
    {
        if (!done && will_run)
        {
            int tmp_x = (-x);
            int tmp_y = (-y);

            /* Try to find safe place */
            if (find_safety(m_idx, &y, &x))
            {
                /* Attempt to avoid the player */
                if (!no_flow)
                {
                    /* Adjust movement */
                    if (get_fear_moves_aux(m_idx, &y, &x)) done = TRUE;
                }
            }

            if (!done)
            {
                /* This is not a very "smart" method XXX XXX */
                y = tmp_y;
                x = tmp_x;
            }
        }
    }


    /* Check for no move */
    if (!x && !y) return (FALSE);

    /* Check for long melee */
    if ((use_long_melee) && (ABS(x) < 3) && (ABS(y) < 3))
    {
        mm[0] = mm[1] = mm[2] = mm[3] = mm[4] = (30 + (5 * x) + y);
        return (TRUE);
    }

    /* Extract the "absolute distances" */
    ax = ABS(x);
    ay = ABS(y);

    /* Do something weird */
    if (y < 0) move_val += 8;
    if (x > 0) move_val += 4;

    /* Prevent the diamond maneuvre */
    if (ay > (ax << 1)) move_val += 2;
    else if (ax > (ay << 1)) move_val++;

    /* Extract some directions */
    switch (move_val)
    {
    case 0:
        mm[0] = 9;
        if (ay > ax)
        {
            mm[1] = 8;
            mm[2] = 6;
            mm[3] = 7;
            mm[4] = 3;
        }
        else
        {
            mm[1] = 6;
            mm[2] = 8;
            mm[3] = 3;
            mm[4] = 7;
        }
        break;
    case 1:
    case 9:
        mm[0] = 6;
        if (y < 0)
        {
            mm[1] = 3;
            mm[2] = 9;
            mm[3] = 2;
            mm[4] = 8;
        }
        else
        {
            mm[1] = 9;
            mm[2] = 3;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
    case 2:
    case 6:
        mm[0] = 8;
        if (x < 0)
        {
            mm[1] = 9;
            mm[2] = 7;
            mm[3] = 6;
            mm[4] = 4;
        }
        else
        {
            mm[1] = 7;
            mm[2] = 9;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
    case 4:
        mm[0] = 7;
        if (ay > ax)
        {
            mm[1] = 8;
            mm[2] = 4;
            mm[3] = 9;
            mm[4] = 1;
        }
        else
        {
            mm[1] = 4;
            mm[2] = 8;
            mm[3] = 1;
            mm[4] = 9;
        }
        break;
    case 5:
    case 13:
        mm[0] = 4;
        if (y < 0)
        {
            mm[1] = 1;
            mm[2] = 7;
            mm[3] = 2;
            mm[4] = 8;
        }
        else
        {
            mm[1] = 7;
            mm[2] = 1;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
    case 8:
        mm[0] = 3;
        if (ay > ax)
        {
            mm[1] = 2;
            mm[2] = 6;
            mm[3] = 1;
            mm[4] = 9;
        }
        else
        {
            mm[1] = 6;
            mm[2] = 2;
            mm[3] = 9;
            mm[4] = 1;
        }
        break;
    case 10:
    case 14:
        mm[0] = 2;
        if (x < 0)
        {
            mm[1] = 3;
            mm[2] = 1;
            mm[3] = 6;
            mm[4] = 4;
        }
        else
        {
            mm[1] = 1;
            mm[2] = 3;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
    case 12:
        mm[0] = 1;
        if (ay > ax)
        {
            mm[1] = 2;
            mm[2] = 4;
            mm[3] = 3;
            mm[4] = 7;
        }
        else
        {
            mm[1] = 4;
            mm[2] = 2;
            mm[3] = 7;
            mm[4] = 3;
        }
        break;
    }

    /* invisible monsters, currently undetected, attempt to fool the
     * player by moving somewhat unpredictably (but not erratically) */
    if ((r_ptr->flags2 & RF2_INVISIBLE) && !m_ptr->ml && one_in_(2))
    {
        int i;
        /* permute mm[0..2] (first three primary directions) */
        for (i = 0; i < 2; i++)
        {
            int j = rand_range(i, 2);
            int t = mm[i];
            mm[i] = mm[j];
            mm[j] = t;
        }
    }

    /* Wants to move... */
    return (TRUE);
}


static int check_hit2(int power, int level, int ac, int stun)
{
    int i, k;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- Always miss or hit */
    if (k < 10) return (k < 5);

    /* Calculate the "attack quality" */
    i = (power + (level * 3));
    if (stun)
        i -= i * MIN(100, stun) / 150;

    /* Power and Level compete against Armor */
    if ((i > 0) && (randint1(i) > ((ac * 3) / 4))) return (TRUE);

    /* Assume miss */
    return (FALSE);
}


#define BLOW_EFFECT_TYPE_NONE  0
#define BLOW_EFFECT_TYPE_FEAR  1
#define BLOW_EFFECT_TYPE_SLEEP 2
#define BLOW_EFFECT_TYPE_HEAL  3


/* Monster attacks monster */
static bool _mon_gf_mon(int who, mon_ptr tgt, int type, int dam)
{
    return gf_affect_m(who, tgt, type, dam, GF_AFFECT_ATTACK);
}
bool mon_attack_mon(int m_idx, int t_idx)
{
    monster_type    *m_ptr = &m_list[m_idx];
    monster_type    *t_ptr = &m_list[t_idx];
    int              stun = MON_STUNNED(m_ptr);

    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    monster_race    *tr_ptr = &r_info[t_ptr->r_idx];

    int             ap_cnt, j;
    int             ac, rlev, pt, to_dd = 0;
    char            m_name[80], t_name[80];
    bool            blinked;
    bool            explode = FALSE, touched = FALSE, fear = FALSE;
    int             y_saver = t_ptr->fy;
    int             x_saver = t_ptr->fx;
    int             effect_type;

    bool see_m = mon_show_msg(m_ptr);
    bool see_t = mon_show_msg(t_ptr);
    bool see_either = see_m || see_t;

    /* Can the player be aware of this attack? */
    bool known = (m_ptr->cdis <= MAX_SIGHT) || (t_ptr->cdis <= MAX_SIGHT);
    bool do_silly_attack = (one_in_(2) && p_ptr->image);

    /* Cannot attack self */
    if (m_idx == t_idx) return FALSE;

    /* Not allowed to attack */
    if (r_ptr->flags1 & RF1_NEVER_BLOW) return FALSE;

    if (d_info[dungeon_type].flags1 & DF1_NO_MELEE) return (FALSE);

    /* Total armor */
    ac = mon_ac(t_ptr);

    /* Extract the effective monster level */
    rlev = r_ptr->level;

    /* Apply Dragon Songs to the player's mount */
    if (p_ptr->riding == m_idx && warlock_is_(WARLOCK_DRAGONS))
    {
        switch (warlock_get_toggle())
        {
        case WARLOCK_DRAGON_TOGGLE_BLESS:
            rlev += 5;
            break;
        case WARLOCK_DRAGON_TOGGLE_HEROIC_CHARGE:
            rlev += 20;
            to_dd += 2;
            break;
        }
    }

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the monster name (or "it") */
    monster_desc(t_name, t_ptr, 0);

    /* Assume no blink */
    blinked = FALSE;

    if (!see_either && known)
    {
        mon_fight = TRUE;
    }

    if (p_ptr->riding && (m_idx == p_ptr->riding)) disturb(1, 0);

    if (!retaliation_hack)
        retaliation_count = 0;

    if (!retaliation_hack && see_either)
        cmsg_format(TERM_GREEN, "<color:B>%^s</color> attacks <color:o>%s</color>:", m_name, t_name);
    monster_desc(m_name, m_ptr, MD_PRON_VISIBLE);
    monster_desc(t_name, t_ptr, MD_PRON_VISIBLE);

    /* Scan through all four blows */
    for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
    {
        int method;
        int power = 0;
        int damage = 0;

        cptr act = NULL;

        if (retaliation_hack)
        {
            ap_cnt = retaliation_count;
            if (ap_cnt >= 4) return FALSE;
        }

        method = r_ptr->blows[ap_cnt].method;
        power = r_ptr->blows[ap_cnt].power;

        if (!m_ptr->r_idx) break;

        /* Stop attacking if the target dies! */
        if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
            break;

        /* Hack -- no more attacks */
        if (!method) break;

        if (retaliation_hack && see_either)
        {
            cmsg_format(TERM_GREEN, "(<color:o>%^s</color> retaliates:", m_name);
            mon_lore_2(m_ptr, RF2_AURA_REVENGE);
        }

        /* Monster hits */
        if ( !r_ptr->blows[ap_cnt].effects[0].effect  /* XXX B:BEG or B:INSULT */
          || check_hit2(power, rlev, ac, stun) )
        {
            (void)set_monster_csleep(t_idx, 0);

            if (t_ptr->ml)
                check_mon_health_redraw(t_idx);

            switch (method)
            {
            case RBM_HIT:
                act = "hits";
                touched = TRUE;
                break;
            case RBM_TOUCH:
                act = "touches";
                touched = TRUE;
                break;
            case RBM_PUNCH:
                act = "punches";
                touched = TRUE;
                break;
            case RBM_KICK:
                act = "kicks";
                touched = TRUE;
                break;
            case RBM_CLAW:
                act = "claws";
                touched = TRUE;
                break;
            case RBM_BITE:
                act = "bites";
                touched = TRUE;
                break;
            case RBM_STING:
                act = "stings";
                touched = TRUE;
                break;
            case RBM_SLASH:
                act = "slashes";
                break;
            case RBM_BUTT:
                act = "butts";
                touched = TRUE;
                break;
            case RBM_CRUSH:
                act = "crushes";
                touched = TRUE;
                break;
            case RBM_ENGULF:
                act = "engulfs";
                touched = TRUE;
                break;
            case RBM_CHARGE:
                act = "charges";
                touched = TRUE;
                break;
            case RBM_CRAWL:
                act = "crawls";
                touched = TRUE;
                break;
            case RBM_DROOL:
                act = "drools";
                touched = FALSE;
                break;
            case RBM_SPIT:
                act = "spits";
                touched = FALSE;
                break;
            case RBM_EXPLODE:
                if (see_either) disturb(1, 0);
                act = "explodes";
                explode = TRUE;
                touched = FALSE;
                break;
            case RBM_GAZE:
                act = "gazes";
                touched = FALSE;
                break;
            case RBM_WAIL:
                act = "wails";
                touched = FALSE;
                break;
            case RBM_SPORE:
                act = "releases spores";
                touched = FALSE;
                break;
            case RBM_XXX4:
                act = "projects XXX4's at %s.";
                touched = FALSE;
                break;
            case RBM_BEG:
                act = "begs";
                touched = FALSE;
                break;
            case RBM_INSULT:
                act = "insults";
                touched = FALSE;
                break;
            case RBM_MOAN:
                act = "moans";
                touched = FALSE;
                break;
            case RBM_SHOW:
                act = "sings";
                touched = FALSE;
                break;
            }

            /* Message */
            if (act && see_m)
            {
                if (do_silly_attack)
                {
                    act = silly_attacks[randint0(MAX_SILLY_ATTACK)];
                    msg_format("<color:B>%^s</color> %s <color:o>%s</color>%s", m_name, act, t_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                else
                    msg_format("%^s %s%s", m_name, act, retaliation_hack ? ".<color:g>)</color>" : ".");

            }

            for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
            {
                mon_effect_t e = r_ptr->blows[ap_cnt].effects[j];

                if (!e.effect) break;
                if (e.pct && randint1(100) > e.pct) continue;

                damage = damroll(e.dd + to_dd, e.ds);
                if (stun)
                    damage -= damage * MIN(100, stun) / 150;

                effect_type = BLOW_EFFECT_TYPE_NONE;
                pt = GF_MISSILE;
                switch (e.effect)
                {
                case 0:
                case RBE_HURT:
                    damage = damage * ac_melee_pct(ac) / 100;
                    break;

                case RBE_DISEASE:
                    pt = GF_POIS;
                    break;

                case RBE_DRAIN_CHARGES:
                    pt = GF_DISENCHANT;
                    break;

                /* non-damaging attacks */
                case RBE_EAT_ITEM:
                case RBE_EAT_GOLD:
                    if ((p_ptr->riding != m_idx) && one_in_(2)) blinked = TRUE;
                case RBE_EAT_FOOD:
                case RBE_EAT_LITE:
                case RBE_LOSE_STR:
                case RBE_LOSE_INT:
                case RBE_LOSE_WIS:
                case RBE_LOSE_DEX:
                case RBE_LOSE_CON:
                case RBE_LOSE_CHR:
                case RBE_LOSE_ALL:
                case RBE_DRAIN_EXP:
                    pt = 0;
                    break;

                case RBE_SHATTER:
                    damage = damage * ac_melee_pct(ac) / 100;
                    if (damage > 23) earthquake_aux(m_ptr->fy, m_ptr->fx, 8, m_idx);
                    break;

                case RBE_VAMP:
                    pt = GF_OLD_DRAIN;
                    effect_type = BLOW_EFFECT_TYPE_HEAL;
                    break;

                default:
                    pt = e.effect;
                    break;
                }
                if (pt)
                {
                    /* Do damage if not exploding */
                    if (!explode)
                        _mon_gf_mon(m_idx, t_ptr, pt, damage);

                    switch (effect_type)
                    {
                    case BLOW_EFFECT_TYPE_FEAR:
                        _mon_gf_mon(m_idx, t_ptr, GF_TURN_ALL, damage);
                        break;

                    case BLOW_EFFECT_TYPE_SLEEP:
                        _mon_gf_mon(m_idx, t_ptr, GF_OLD_SLEEP, r_ptr->level);
                        break;

                    case BLOW_EFFECT_TYPE_HEAL:
                        if ((monster_living(tr_ptr)) && (damage > 2))
                        {
                            bool did_heal = FALSE;

                            if (m_ptr->hp < m_ptr->maxhp) did_heal = TRUE;

                            /* Heal */
                            m_ptr->hp += damroll(4, damage / 6);
                            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

                            /* Redraw (later) if needed */
                            check_mon_health_redraw(m_idx);

                            /* Special message */
                            if (see_m && did_heal)
                            {
                                msg_format("%^s appears healthier.", m_name);
                            }
                        }
                        break;
                    }

                    if (touched)
                    {
                        int k;
                        if (tr_ptr->flags2 & RF2_AURA_REVENGE && !retaliation_hack)
                        {
                            retaliation_hack = TRUE;
                            mon_attack_mon(t_idx, m_idx);
                            retaliation_count++;
                            retaliation_hack = FALSE;
                        }

                        /* Aura fire */
                        if ((tr_ptr->flags2 & RF2_AURA_FIRE) && m_ptr->r_idx)
                        {
                            if (!(r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
                            {
                                int dd = 1 + tr_ptr->level/26;
                                int ds = 1 + tr_ptr->level/17;
                                int dam = damroll(dd, ds);
                                if (see_either)
                                    msg_format("%^s is <color:r>burned</color>!", m_name);
                                if (m_ptr->ml)
                                    mon_lore_2(t_ptr, RF2_AURA_FIRE);
                                _mon_gf_mon(t_idx, m_ptr, GF_FIRE, dam);
                            }
                            else
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
                            }
                        }

                        /* Aura cold */
                        if ((tr_ptr->flags3 & RF3_AURA_COLD) && m_ptr->r_idx)
                        {
                            if (!(r_ptr->flagsr & RFR_EFF_IM_COLD_MASK))
                            {
                                int dd = 1 + tr_ptr->level/26;
                                int ds = 1 + tr_ptr->level/17;
                                int dam = damroll(dd, ds);
                                if (see_either)
                                    msg_format("%^s is <color:w>frozen</color>!", m_name);
                                if (m_ptr->ml)
                                    mon_lore_3(t_ptr, RF3_AURA_COLD);
                                _mon_gf_mon(t_idx, m_ptr, GF_COLD, dam);
                            }
                            else
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
                            }
                        }

                        /* Aura elec */
                        if ((tr_ptr->flags2 & RF2_AURA_ELEC) && m_ptr->r_idx)
                        {
                            if (!(r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK))
                            {
                                int dd = 1 + tr_ptr->level/26;
                                int ds = 1 + tr_ptr->level/17;
                                int dam = damroll(dd, ds);
                                if (see_either)
                                    msg_format("%^s is <color:b>zapped</color>!", m_name);
                                if (m_ptr->ml)
                                    mon_lore_2(t_ptr, RF2_AURA_ELEC);
                                _mon_gf_mon(t_idx, m_ptr, GF_ELEC, dam);
                            }
                            else
                            {
                                mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
                            }
                        }
                        for (k = 0; k < MAX_MON_AURAS; k++)
                        {
                            mon_effect_t aura = tr_ptr->auras[k];
                            int          dam;
                            if (!aura.effect) continue;
                            if (aura.pct && randint1(100) > aura.pct) continue;
                            dam = damroll(aura.dd, aura.ds);
                            _mon_gf_mon(t_idx, m_ptr, aura.effect, dam);
                        }
                    }
                }
            } /* for each effect */
        }

        /* Monster missed monster */
        else
        {
            /* Analyze failed attacks */
            switch (method)
            {
            case RBM_HIT:
            case RBM_TOUCH:
            case RBM_PUNCH:
            case RBM_KICK:
            case RBM_CLAW:
            case RBM_BITE:
            case RBM_STING:
            case RBM_SLASH:
            case RBM_BUTT:
            case RBM_CRUSH:
            case RBM_ENGULF:
            case RBM_CHARGE:
            case RBM_CRAWL:
            case RBM_SPIT:
            case RBM_EXPLODE:
                set_monster_csleep(t_idx, 0);
                if (see_m)
                    msg_format("%^s misses%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                break;
            case RBM_DROOL:
                if (see_m)
                    msg_format("%^s slobbers ineffectually%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                break;
            case RBM_WAIL:
                if (see_m)
                    msg_format("%^s wails ineffectually%s", m_name, retaliation_hack ? ".<color:g>)</color>" : ".");
                break;
            case RBM_GAZE:
                if (see_m)
                {
                    char tmp[MAX_NLEN];
                    monster_desc(tmp, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);
                    msg_format("<color:o>%^s</color> avoids <color:B>%s</color> gaze%s", t_name, tmp, retaliation_hack ? ".<color:g>)</color>" : ".");
                }
                break;
            }
        }

        /* XXX Lore? I'd rather the player only lore attacks they actually experience.
         * For one thing, the effects are different when it is monster vs monster. For 
         * another, they are probably a bit distracted! */

        if (retaliation_hack)
            break;
    }

    if (explode)
    {
        sound(SOUND_EXPLODE);

        /* Cancel Invulnerability */
        (void)set_monster_invulner(m_idx, 0, FALSE);

        mon_take_hit_mon(m_idx, m_ptr->hp + 1, &fear, " explodes into tiny shreds.", m_idx);

        blinked = FALSE;
    }

    /* Blink away */
    if (blinked && m_ptr->r_idx)
    {
        if (teleport_barrier(m_idx))
        {
            if (see_m)
            {
                msg_print("The thief flees laughing...? But magic barrier obstructs it.");
            }
            else if (known)
            {
                mon_fight = TRUE;
            }
        }
        else
        {
            if (see_m)
            {
                msg_print("The thief flees laughing!");
            }
            else if (known)
            {
                mon_fight = TRUE;
            }

            teleport_away(m_idx, MAX_SIGHT * 2 + 5, 0L);
        }
    }
    return TRUE;
}


static bool check_hp_for_feat_destruction(feature_type *f_ptr, monster_type *m_ptr)
{
    return !have_flag(f_ptr->flags, FF_GLASS) ||
           (r_info[m_ptr->r_idx].flags2 & RF2_STUPID) ||
           (m_ptr->hp >= MAX(m_ptr->maxhp / 3, 200));
}

/*
 * Process a monster
 *
 * The monster is known to be within 100 grids of the player
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level. This should prevent the level from being "swamped" by
 * reproducing monsters. It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it. Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * XXX XXX XXX In addition, monsters which *cannot* open or bash
 * down a door will still stand there trying to open it...
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * A "direction" of "5" means "pick a random direction".
 */
static void process_monster(int m_idx)
{
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    monster_race    *ap_r_ptr = &r_info[m_ptr->ap_r_idx];
    class_t         *class_ptr = get_class();

    int             i, d, oy, ox, ny, nx;

    int             mm[8] = {0};

    cave_type       *c_ptr;
    feature_type    *f_ptr;

    monster_type    *y_ptr;

    bool            do_turn;
    bool            do_move;
    bool            do_view;
    bool            asc_nerf = FALSE;
    bool            could_splash = FALSE;
    bool            must_alter_to_move;

    bool            did_open_door;
    bool            did_bash_door;
    bool            did_take_item;
    bool            did_kill_item;
    bool            did_move_body;
    bool            did_pass_wall;
    bool            did_kill_wall;
    bool            gets_angry = FALSE;
    bool            can_cross;
    bool            aware = TRUE;

    bool            fear;

    bool            is_riding_mon = (m_idx == p_ptr->riding);

    bool            see_m = mon_show_msg(m_ptr);

    /* Hack: Trump monsters blink continually for free.
       Note, if you move this code below, the monster actually spawns???  Probably,
       he is just in N places at once.
    */
    if ((r_ptr->flags1 & RF1_TRUMP) && one_in_(2))
    {
        if (!MON_CSLEEP(m_ptr) && !is_riding_mon) /* TODO: Currently broken for riding! */
            teleport_away(m_idx, 5, 0L);
    }

    if (is_riding_mon)
    {
        if (p_ptr->prace == RACE_MON_RING)
        {
            ring_process_m(m_idx);
        }
        else if (!(r_ptr->flags7 & RF7_RIDING))
        {
            if (rakuba(0, TRUE))
            {
                char m_name[80];
                monster_desc(m_name, &m_list[p_ptr->riding], 0);
                msg_format("You have fallen from %s.", m_name);
            }
        }
    }
    else if ((is_pet(m_ptr)) && (p_ptr->csp * 15 <= p_ptr->msp * 14))
    {
        int upkeep_factor = calculate_upkeep();
        while ((upkeep_factor > SAFE_UPKEEP_PCT) && (p_ptr->upset_okay)) /* Neglected pets */
        {
            if (unique_is_friend(m_ptr->r_idx)) break;
            if (p_ptr->csp * 3 > p_ptr->msp) 
            {
                 if (randint0(p_ptr->msp) < p_ptr->csp) break;
            }
            /* Evil monsters and good monsters turn hostile more easily
             * (evil monsters because they're untrustworthy, and good
             * monsters because they expect more from the player) */
            if (!one_in_((r_ptr->flags3 & (RF3_EVIL | RF3_GOOD)) ? 2 : 4)) break;
            if (randint1(1500) > upkeep_factor) break;
            /* Preferentially keep uniques and high-level pets */
            if ((r_ptr->flags1 & RF1_UNIQUE) && ((!one_in_(3)) || (randint1(1500) > upkeep_factor))) break;
            if ((randint1(125) < r_ptr->level) && (randint1(2000) > upkeep_factor)) break;
            if ((!m_ptr->parent_m_idx) && (one_in_(2))) break;
            /* Uniques turn hostile less often. Hydras always disappear */
            if (((r_ptr->flags1 & RF1_UNIQUE) && (one_in_(2))) || (r_ptr->d_char == 'M') ||
                ((p_ptr->csp) && (randint0(p_ptr->msp) < p_ptr->csp) && (randint1(125) > r_ptr->level))) /* Screw this, I'm outta here */
            {
                if (see_m)
                {
                    char m_name[80];
                    monster_desc(m_name, m_ptr, 0);
                    msg_format("<color:G>%^s</color> feels neglected.", m_name);
                    cmsg_format(TERM_L_RED, "%^s disappears!", m_name);
                }
                delete_monster_idx(m_idx);
                return;
            }
            else /* Turn hostile */
            {
                char m_name[80];
                monster_desc(m_name, m_ptr, 0);
                msg_format("<color:G>%^s</color> feels neglected.", m_name);
                cmsg_format(TERM_L_RED, "%^s gets angry!", m_name);
                set_hostile(m_ptr);
            }
        }
    }

    if ((m_ptr->mflag2 & MFLAG2_CHAMELEON) && one_in_(13) && !MON_CSLEEP(m_ptr))
    {
        choose_new_monster(m_idx, FALSE, 0);
        r_ptr = &r_info[m_ptr->r_idx];
    }

    /* Players hidden in shadow are almost imperceptible. -LM- */
    if (p_ptr->special_defense & NINJA_S_STEALTH && !is_pet(m_ptr))
    {
        int tmp = p_ptr->lev*6+(p_ptr->skills.stl+10)*4;
        if (p_ptr->monlite) tmp /= 3;
        if (p_ptr->cursed & OFC_AGGRAVATE) tmp /= 2;
        if (r_ptr->level > (p_ptr->lev*p_ptr->lev/20+10)) tmp /= 3;
        /* Low-level monsters will find it difficult to locate the player. */
        if (randint0(tmp) > (r_ptr->level+20)) aware = FALSE;

        /* Note: The aware flag will induce random monster movement since
           the player is hiding in the shadows. The is_aware() attribute
           means the monster is unaware that a mimic is not an object. In
           this case, movement should still be towards the object (player). */
    }

    /* Are there its parent? */
    if (m_ptr->parent_m_idx && !m_list[m_ptr->parent_m_idx].r_idx)
    {
        /* Its parent have gone, it also goes away.
           Hack: Only for pets.
        */
        if (!is_pet(m_ptr))
        {
            mon_set_parent(m_ptr, 0);
        }
        else
        {
            if (see_m)
            {
                char m_name[80];
                monster_desc(m_name, m_ptr, 0);
                msg_format("%^s disappears!", m_name);
            }

            /* Delete the monster */
            delete_monster_idx(m_idx);

            return;
        }
    }

    /* Quantum monsters are odd */
    if (r_ptr->flags2 & (RF2_QUANTUM))
    {
        /* Sometimes skip move */
        if (!randint0(2)) return;

        /* Sometimes die */
        if (!randint0((m_idx % 100) + 10) && !(m_ptr->mflag2 & MFLAG2_QUESTOR))
        {
            bool sad = FALSE;

            if (is_pet(m_ptr) && !(m_ptr->ml))
                sad = TRUE;

            if (see_m)
            {
                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Oops */
                msg_format("%^s disappears!", m_name);
            }

            /* Generate treasure, etc */
            monster_death(m_idx, FALSE);

            /* Delete the monster */
            delete_monster_idx(m_idx);

            if (sad)
                msg_print("You feel sad for a moment.");

            return;
        }
    }

    if ((m_ptr->r_idx == MON_LEPRECHAUN_FANATIC) && (m_ptr->mflag & MFLAG_NICE))
        return;

    if (m_ptr->r_idx == MON_SHURYUUDAN)
        mon_take_hit_mon(m_idx, 1, &fear, " explodes into tiny shreds.", m_idx);

    if (((is_pet(m_ptr)) || (is_friendly(m_ptr))) && (!p_ptr->inside_battle))
    {
        static int riding_pinch = 0;

        if (m_ptr->hp < m_ptr->maxhp/3)
        {
            char m_name[80];
            bool skip = FALSE;
            monster_desc(m_name, m_ptr, 0);

            for (i = 0; i < MAX_MON_BLOWS; i++)
            {
                if (r_ptr->blows[i].method == RBM_EXPLODE)
                {
                    skip = TRUE;
                    break;
                }
            }

            if (skip) {} /* Kamikaze pets fight to death */
            else if (is_riding_mon && riding_pinch < 2)
            {
                msg_format("%^s seems to be in so much pain, and trying to escape from your restriction.", m_name);
                riding_pinch++;
                disturb(1, 0);
            }
            else if ((!is_riding_mon) && (!((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))) && ((randint1((s32b)m_ptr->maxhp * 7 / 20) < m_ptr->hp) || (one_in_(5)) || ((p_ptr->pclass == CLASS_POLITICIAN) && (!is_pet(m_ptr)) && (one_in_(2)))))
            {
                if (is_pet(m_ptr)) msg_format("<color:B>%^s</color> seems to be in great pain!", m_name);
            }
            else
            {
                if (is_riding_mon)
                {
                    msg_format("%^s succeeded to escape from your restriction!", m_name);
                    if (rakuba(-1, FALSE))
                        msg_print("You have fallen from riding pet.");
                }

                if (see_m)
                {
                    if ((r_ptr->flags2 & RF2_CAN_SPEAK) && (m_ptr->r_idx != MON_GRIP) && (m_ptr->r_idx != MON_WOLF) && (m_ptr->r_idx != MON_FANG) &&
                        player_has_los_bold(m_ptr->fy, m_ptr->fx) && projectable(m_ptr->fy, m_ptr->fx, py, px))
                    {
                        msg_format("<color:B>%^s</color> says 'It is the pinch! I will retreat'.", m_name);
                        msg_format("<color:B>%^s</color> reads a scroll of Teleport Level.", m_name);
                        msg_format("<color:B>%^s</color> disappears.", m_name);
                    }
                    else msg_format("<color:B>%^s</color> escapes.", m_name);
                }

                if (is_riding_mon && rakuba(-1, FALSE))
                    msg_print("You have fallen from riding pet.");

                quests_on_kill_mon(m_ptr);
                delete_monster_idx(m_idx);
                return;
            }
        }
        else
        {
            /* Reset the counter */
            if (is_riding_mon) riding_pinch = 0;
        }
    }

    /* Hack: Rings wake up potential ring bearers */
    if ( MON_CSLEEP(m_ptr)
      && p_ptr->action == ACTION_GLITTER
      && mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER) )
    {
        set_monster_csleep(m_idx, 0);
    }

    /* Handle "sleep" */
    if (MON_CSLEEP(m_ptr))
    {
        /* Handle non-aggravation - Still sleeping */
        if (!(p_ptr->cursed & OFC_AGGRAVATE))
        {
            /* Troika disciples always wake orcs up */
            if ((!disciple_is_(DISCIPLE_TROIKA)) || (!(r_ptr->flags3 & RF3_ORC))) return;
        }

        /* Handle aggravation */

        /* Reset sleep counter */
        (void)set_monster_csleep(m_idx, 0);

        /* Notice the "waking up" */
        if (m_ptr->ml && disturb_minor)
        {
            char m_name[80];

            monster_desc(m_name, m_ptr, 0);
            msg_format("%^s wakes up.", m_name);
        }

        /* Hack -- Count the wakings */
        if (is_original_ap_and_seen(m_ptr) && (r_ptr->r_wake < MAX_UCHAR))
        {
            r_ptr->r_wake++;
        }
    }

    if (is_riding_mon)
    {
        p_ptr->update |= (PU_BONUS);
    }

    /* No one wants to be your friend if you're aggravating */
    if (is_friendly(m_ptr) && (p_ptr->cursed & OFC_AGGRAVATE) && (!p_ptr->uimapuku))
        gets_angry = TRUE;

    /* Uniques are hard to keep tame */
    if (is_pet(m_ptr) &&
        ((((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL)) &&
          monster_has_hostile_align(NULL, 10, -10, r_ptr))))
    {
        if (p_ptr->prace != RACE_MON_RING)
            gets_angry = TRUE;
    }

    if (p_ptr->inside_battle) gets_angry = FALSE;

    if (gets_angry)
    {
        if (is_pet(m_ptr) || see_m)
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, is_pet(m_ptr) ? MD_ASSUME_VISIBLE : 0);
            msg_format("%^s suddenly becomes hostile!", m_name);
        }

        set_hostile(m_ptr);
    }

    /* Get the origin */
    oy = m_ptr->fy;
    ox = m_ptr->fx;


    /* Attempt to "multiply" if able and allowed */
    if ( (r_ptr->flags2 & RF2_MULTIPLY)
      && num_repro < MAX_REPRO
      && randint1(375) > virtue_current(VIRTUE_HARMONY) )
    {
        int k, y, x;

        /* Count the adjacent monsters */
        for (k = 0, y = oy - 1; y <= oy + 1; y++)
        {
            for (x = ox - 1; x <= ox + 1; x++)
            {
                if (!in_bounds2(y, x)) continue;
                if (cave[y][x].m_idx) k++;
            }
        }

        /* Hex */
        if (multiply_barrier(m_idx)) k = 8;

        /* Hack -- multiply slower in crowded areas */
        if ((k < 4) && (!k || !randint0(k * MON_MULT_ADJ)))
        {
            bool allow = TRUE;
            /* Try to multiply */
            if (is_pet(m_ptr))
            {
                int upkeep = calculate_upkeep();
                if (upkeep > 80)
                    allow = FALSE;
                if (p_ptr->pet_extra_flags & PF_NO_BREEDING)
                    allow = FALSE;
            }
            if (allow && multiply_monster(m_idx, FALSE, (is_pet(m_ptr) ? PM_FORCE_PET : 0)))
            {
                /* Take note if visible */
                if (m_list[hack_m_idx_ii].ml)
                {
                    mon_lore_2(m_ptr, RF2_MULTIPLY);
                }

                /* Multiplying takes energy */
                return;
            }
        }
    }


    /* Hack -- Ohmu scatters molds! This is no longer a spell ... just do it! */
    if (m_ptr->r_idx == MON_OHMU)
    {
        if (!p_ptr->inside_arena && !p_ptr->inside_battle)
        {
            if (one_in_(3))
            {
                int  k;
                int  flags = PM_ALLOW_GROUP;
                if (is_pet(m_ptr)) flags |= PM_FORCE_PET;
                for (k = 0; k < 6; k++)
                {
                    summon_specific(m_idx, m_ptr->fy, m_ptr->fx,
                        r_ptr->level, SUMMON_BIZARRE1, flags);
                }
            }
        }
    }


    if (!p_ptr->inside_battle)
    {
        /* Hack! "Cyber" monster makes noise... */
        if (m_ptr->ap_r_idx == MON_CYBER &&
            one_in_(CYBERNOISE) &&
            !m_ptr->ml && (m_ptr->cdis <= MAX_SIGHT))
        {
            if (disturb_minor)
            {
                msg_print("You hear heavy steps.");
                disturb(0, 0);
            }
        }

        /* Some monsters can speak */
        if ((ap_r_ptr->flags2 & RF2_CAN_SPEAK) && aware && is_aware(m_ptr) &&
            m_idx != p_ptr->riding &&
            one_in_(SPEAK_CHANCE) &&
            player_has_los_bold(oy, ox) &&
            projectable(oy, ox, py, px))
        {
            char m_name[80];
            char monmessage[1024];
            cptr filename;

            /* Acquire the monster name/poss */
            if (m_ptr->ml)
                monster_desc(m_name, m_ptr, 0);
            else
                strcpy(m_name, "It");

            /* Select the file for monster quotes */
            if (MON_MONFEAR(m_ptr))
                filename = "monfear.txt";
            else if ((is_friendly(m_ptr)) || (is_pet(m_ptr)))
                filename = "monfrien.txt";
            else
                filename = "monspeak.txt";

            if (get_rnd_line(filename, m_ptr->ap_r_idx, monmessage) == 0)
            {
                msg_format("<color:g>%^s</color> %s", m_name, monmessage);
                msg_boundary();
            }
        }
    }

    /* Pack AI ... Attempt to wake up your buddies!
    {
    pack_info_t *pack_ptr = pack_info_ptr(m_idx);
    int dir, x, y;
    cave_type *c_ptr;
    monster_type *m_ptr2;
    int max_wake = 1;
    int count = max_wake;

        while (one_in_(5))
            ++max_wake;

        if (pack_ptr)
        {
            for (dir = 0; count && dir < 8; dir++)
            {
                y = m_ptr->fy + ddy_ddd[dir];
                x = m_ptr->fx + ddx_ddd[dir];
                c_ptr = &cave[y][x];

                m_ptr2 = &m_list[c_ptr->m_idx];

                if (c_ptr->m_idx &&
                    m_ptr2->pack_idx == m_ptr->pack_idx &&
                    MON_CSLEEP(m_ptr2))
                {
                    set_monster_csleep(c_ptr->m_idx, 0);
                    --count;
                }
            }

            if (count < max_wake) return;
        }
    } */

    /* Clear head occasionally */
    if ((MON_CONFUSED(m_ptr)) && (r_ptr->flags3 & RF3_CLEAR_HEAD) && (one_in_(4)))
    {
        if (set_monster_confused(m_ptr->id, 0))
        {
            if (mon_show_msg(m_ptr))
            {
                char m_name[80];
                monster_desc(m_name, m_ptr, 0);
                msg_format("%^s is no longer confused.", m_name);
            }
        }
    }

    /* Try to cast spell occasionally */
    if (r_ptr->spells)
    {
        int freq = r_ptr->spells->freq;
        pack_info_t *pack_ptr = pack_info_ptr(m_idx);
        bool blocked = FALSE;

        /* XXX Block spells occasionally if the monster just cast (EXPERIMENTAL)
         * Here, were are attempting to prevent long runs of consecutive casts for
         * melee characters (anger=0). Of course, Nodens should still spell every
         * turn! See ^A"F for analysis ... This is approach II. */
        if (!m_ptr->anger && m_ptr->mana > 0 && freq <= 50 && !one_in_(1 + m_ptr->mana))
            blocked = TRUE;

        /* Increase spell frequency for pack AI or player glyphs of warding */
        if (is_glyph_grid(&cave[py][px]))
        {
            freq = MAX(30, freq + 10);
        }
        else if (pack_ptr)
        {
            switch (pack_ptr->ai)
            {
            case AI_SHOOT:
                freq = MAX(30, freq + 15);
                break;
            case AI_MAINTAIN_DISTANCE:
                freq += MIN(freq/2, 15);
                break;
            case AI_LURE:
            case AI_FEAR:
            case AI_GUARD_POS:
                freq += MIN(freq/2, 10);
                break;
            }
        }

        /* Angry monsters will eventually spell if they get too pissed off.
         * Monsters are angered by distance attacks (spell casters/archers) */
        freq += m_ptr->anger;

        if (freq > 100) freq = 100;

        /* XXX Adapt spell frequency down if monster is stunned (EXPERIMENTAL)
         * Sure, stunning effects fail rates, but not on innate spells (breaths).
         * In fact, distance stunning gives no benefit against big breathers ...
         * Try a sprite mindcrafter and you'll see what I mean. */
        if (MON_STUNNED(m_ptr))
        {
            int s = MON_STUNNED(m_ptr);
            int p = MAX(0, 100 - s);
            freq = freq * p / 100;
            if (freq < 1) freq = 1;
        }

        /* Hack for Rage Mage Anti-magic Ray ... */
        if (!blocked && m_ptr->anti_magic_ct)
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);

            /* Monsters that spell 1 in 1 get a save on each action. Make the
               save and the spell is broken. Fail, and a turn is lost. */
            if (r_ptr->spells->freq == 100)
            {
                if (!mon_save_p(m_ptr->r_idx, A_STR))
                {
                    if (mon_show_msg(m_ptr))
                        msg_format("%^s tries to break your anti-magic ray but fails.", m_name);
                    m_ptr->anti_magic_ct--;

                    return;
                }
                else
                {
                    if (mon_show_msg(m_ptr))
                        msg_format("%^s breaks your anti-magic ray.", m_name);
                    m_ptr->anti_magic_ct = 0;
                }
            }
            /* Other monsters continue to take a move, but can't cast spells */
            else
            {
                blocked = TRUE;
                m_ptr->anti_magic_ct--;
            }
        }

        #if 0
        if (/*p_ptr->wizard &&*/ m_ptr->id == target_who)
            msg_format("<color:B>Freq=%d%% (%d%%,%d,%d,%d)</color>", freq, r_ptr->spells->freq, m_ptr->anger, m_ptr->mana, MON_STUNNED(m_ptr));
        #endif

        if (!blocked && randint1(100) <= freq)
        {
            bool counterattack = FALSE;

            /* Give priority to counter attack? */
            if (m_ptr->target_y)
            {
                int t_m_idx = cave[m_ptr->target_y][m_ptr->target_x].m_idx;

                /* The monster must be an enemy, and projectable */
                if (t_m_idx &&
                    are_enemies(m_ptr, &m_list[t_m_idx]) &&
                    projectable(m_ptr->fy, m_ptr->fx, m_ptr->target_y, m_ptr->target_x))
                {
                    counterattack = TRUE;
                }
            }

            if (!counterattack)
            {
                /* Attempt to cast a spell */
                if (aware && mon_spell_cast(m_ptr, NULL))
                {
                    m_ptr->anger = 0;
                    m_ptr->mana++;
                    return;
                }
                /*
                 * Attempt to cast a spell at an enemy other than the player
                 * (may slow the game a smidgeon, but I haven't noticed.)
                 */
                if (mon_spell_cast_mon(m_ptr, NULL))
                    return;
            }
            else
            {
                /* Attempt to do counter attack at first */
                if (mon_spell_cast_mon(m_ptr, NULL))
                    return;

                if (aware && mon_spell_cast(m_ptr, NULL))
                    return;
            }
        }
    }

    /* XXX Regain mana (EXPERIMENTAL) */
    if (m_ptr->mana)
        m_ptr->mana--;

    /* Really, a non-spell turn. The monster may never move, or may be blocked, and we still
       want to track those turns for accurate reporting of spell frequency. Also, don't count
       unless the monster could spell.*/
    if (projectable(py, px, m_ptr->fy, m_ptr->fx))
        mon_lore_move(m_ptr);

    /* Confused -- 100% random */
    if (MON_CONFUSED(m_ptr) || !aware)
    {
        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 75% random movement */
    else if (((r_ptr->flags1 & (RF1_RAND_50 | RF1_RAND_25)) == (RF1_RAND_50 | RF1_RAND_25)) &&
         (randint0(100) < 75))
    {
        /* Memorize flags */
        mon_lore_1(m_ptr, RF1_RAND_50 | RF1_RAND_25);

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 50% random movement */
    else if ((r_ptr->flags1 & RF1_RAND_50) &&
                (randint0(100) < 50))
    {
        /* Memorize flags */
        mon_lore_1(m_ptr, RF1_RAND_50);

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 25% random movement */
    else if ((r_ptr->flags1 & RF1_RAND_25) &&
                (randint0(100) < 25))
    {
        /* Memorize flags */
        mon_lore_1(m_ptr, RF1_RAND_25);

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* Can't reach player - find something else to hit */
    else if ((r_ptr->flags1 & RF1_NEVER_MOVE) && (m_ptr->cdis > 1))
    {
        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;

        /* Look for an enemy */
#if 0  /* Hack - Too slow. Mimic pits are horrible with this on. */
        get_enemy_dir(m_idx, mm);
#endif /* 0 */
    }

    /* Pets will follow the player */
    else if (is_pet(m_ptr))
    {
        /* Are we trying to avoid the player? */
        bool avoid = ((p_ptr->pet_follow_distance < 0) &&
                          (m_ptr->cdis <= (0 - p_ptr->pet_follow_distance)));

        /* Do we want to find the player? */
        bool lonely = (!avoid && (m_ptr->cdis > p_ptr->pet_follow_distance));

        /* Should we find the player if we can't find a monster? */
        bool distant = (m_ptr->cdis > PET_SEEK_DIST);

        /* by default, move randomly */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;

        /* Look for an enemy */
        if (!get_enemy_dir(m_idx, mm))
        {
            /* Find the player if necessary */
            if (avoid || lonely || distant)
            {
                /* Remember the leash length */
                int dis = p_ptr->pet_follow_distance;

                /* Hack -- adjust follow distance temporarily */
                if (p_ptr->pet_follow_distance > PET_SEEK_DIST)
                {
                    p_ptr->pet_follow_distance = PET_SEEK_DIST;
                }

                /* Find the player */
                (void)get_moves(m_idx, mm);

                /* Restore the leash */
                p_ptr->pet_follow_distance = dis;
            }
        }
    }

    /* Friendly monster movement */
    else if (!is_hostile(m_ptr))
    {
        /* XXX Perhaps friendly monsters should join up with the
         * player? This would make them more interesting/useful
         * (e.g. Gandalf). The random stuff just looks silly
         * when there are no enemies around. */
        if (!get_enemy_dir(m_idx, mm)/* && !get_moves(m_idx, mm) */)
            mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }
    /* Normal movement */
    else
    {
        /* Logical moves, may do nothing */
        if (!get_moves(m_idx, mm)) return;
    }

    /* Assume nothing */
    do_turn = FALSE;
    do_move = FALSE;
    do_view = FALSE;
    must_alter_to_move = FALSE;

    /* Assume nothing */
    did_open_door = FALSE;
    did_bash_door = FALSE;
    did_take_item = FALSE;
    did_kill_item = FALSE;
    did_move_body = FALSE;
    did_pass_wall = FALSE;
    did_kill_wall = FALSE;


    /* Take a zero-terminated array of "directions" */
    for (i = 0; mm[i]; i++)
    {
        /* Get the direction */
        d = mm[i];

        if (d > 15) /* Long melee */
        {
             int ldx = ((d + 2) / 5) - 6;
             int ldy = (d - (5 * ldx)) - 30;
             ny = oy - ldy;
             nx = ox - ldx;
        }

        else /* Short melee */
        {
            /* Hack -- allow "randomized" motion */
            if (d == 5) d = ddd[randint0(8)];

            /* Get the destination */
            ny = oy + ddy[d];
            nx = ox + ddx[d];
        }

        /* Ignore locations off of edge */
        if (!in_bounds2(ny, nx)) continue;

        /* Access that cave grid */
        c_ptr = &cave[ny][nx];
        f_ptr = &f_info[c_ptr->feat];
        can_cross = monster_can_cross_terrain(c_ptr->feat, r_ptr, is_riding_mon ? CEM_RIDING : 0);

        /* Access that cave grid's contents */
        y_ptr = &m_list[c_ptr->m_idx];

        /* Hack -- player 'in' wall */
        if (player_bold(ny, nx))
        {
            do_move = TRUE;
        }

        /* Possibly a monster to attack */
        else if (c_ptr->m_idx)
        {
            do_move = TRUE;
        }

        /* Monster destroys walls (and doors) */
        else if ((r_ptr->flags2 & RF2_KILL_WALL) &&
                 (can_cross ? !have_flag(f_ptr->flags, FF_LOS) : !is_riding_mon) &&
                 have_flag(f_ptr->flags, FF_HURT_DISI) && !have_flag(f_ptr->flags, FF_PERMANENT) &&
                 check_hp_for_feat_destruction(f_ptr, m_ptr))
        {
            /* Eat through walls/doors/rubble */
            do_move = TRUE;
            if (!can_cross) must_alter_to_move = TRUE;

            /* Monster destroyed a wall (later) */
            did_kill_wall = TRUE;
        }

        /* Floor is open? (XXX Passwall monsters as well, right?) */
        else if (can_cross)
        {
            /* Nerf ASC a bit */
            if (mon_has_summon_spell(m_ptr))
            {
                if ( p_ptr->chp > p_ptr->mhp * 4 / 5 /* If @ wounded, pursue! */
                  && !player_bold(ny, nx)            /* Moving from out of LOS into LOS */
                  && player_has_los_bold(ny, nx)
                  && projectable(py, px, ny, nx)
                  && !projectable(py, px, m_ptr->fy, m_ptr->fx) )
                {
                    int ct_open = 0;
                    int ct_enemy = 0;
                    int y, x;

                    /* Inspect @'s surroundings */
                    for (y = py - 2; y <= py + 2; y++)
                    {
                        for (x = px - 2; x <= px + 2; x++)
                        {
                            if (!in_bounds(y, x)) continue;
                            if (distance(py, px, y, x) > 2) continue;
                            if (pattern_tile(y, x)) continue;
                            if (y == ny && x == nx) continue;
                            if (y == m_ptr->fy && x == m_ptr->fx) continue;
                            if (cave[y][x].m_idx && is_hostile(&m_list[cave[y][x].m_idx]))
                                ct_enemy++;
                            if (cave_empty_bold(y, x) && projectable(py, px, y, x) && projectable(y, x, py, px))
                                ct_open++;
                        }
                    }

                    if (ct_enemy)
                    {
                        /* If @ is in battle, join the fray! */
                    }
                    else if (ct_open < 1 + randint1(4))
                    {
                        /* not enough summoning opportunities, so hold off unless angered */
                        if (!m_ptr->anger || !one_in_(3))
                        {
                            asc_nerf = TRUE;
                            could_splash = mon_could_splash(m_ptr, point(px, py));
                            continue;
                        }
                    }
                }
            }

            /* Go ahead and move */
            do_move = TRUE;

            /* Monster moves through walls (and doors) */
            if ((r_ptr->flags2 & RF2_PASS_WALL) && (!is_riding_mon || p_ptr->pass_wall) &&
                have_flag(f_ptr->flags, FF_CAN_PASS))
            {
                /* Monster went through a wall */
                did_pass_wall = TRUE;
            }
        }

        /* Handle doors and secret doors */
        else if (is_closed_door(c_ptr->feat))
        {
            bool may_bash = TRUE;

            /* Assume no move allowed */
            do_move = FALSE;

            /* Creature can open doors. */
            if ((r_ptr->flags2 & RF2_OPEN_DOOR) && have_flag(f_ptr->flags, FF_OPEN) &&
                 (!is_pet(m_ptr) || (p_ptr->pet_extra_flags & PF_OPEN_DOORS)))
            {
                /* Closed doors */
                if (!f_ptr->power)
                {
                    /* The door is open */
                    did_open_door = TRUE;

                    /* Do not bash the door */
                    may_bash = FALSE;

                    /* Take a turn */
                    do_turn = TRUE;
                }

                /* Locked doors (not jammed) */
                else
                {
                    /* Try to unlock it XXX XXX XXX */
                    if (randint0(m_ptr->hp / 10) > f_ptr->power || p_ptr->action == ACTION_GLITTER)
                    {
                        /* Unlock the door */
                        cave_alter_feat(ny, nx, FF_DISARM);

                        /* Do not bash the door */
                        may_bash = FALSE;

                        /* Take a turn */
                        do_turn = TRUE;
                    }
                }
            }

            /* Stuck doors -- attempt to bash them down if allowed */
            if (may_bash && (r_ptr->flags2 & RF2_BASH_DOOR) && have_flag(f_ptr->flags, FF_BASH) &&
                (!is_pet(m_ptr) || (p_ptr->pet_extra_flags & PF_OPEN_DOORS)))
            {
                /* Attempt to Bash XXX XXX XXX */
                if (check_hp_for_feat_destruction(f_ptr, m_ptr) && (randint0(m_ptr->hp / 10) > f_ptr->power))
                {
                    /* Disturb (sometimes) */
                    if (disturb_minor)
                    {
                        if (have_flag(f_ptr->flags, FF_GLASS))
                            msg_print("You hear a glass was crashed!");
                        else
                            msg_print("You hear a door burst open!");

                        disturb(0, 0);
                    }

                    /* The door was bashed open */
                    did_bash_door = TRUE;

                    /* Hack -- fall into doorway */
                    do_move = TRUE;
                    must_alter_to_move = TRUE;
                }
            }


            /* Deal with doors in the way */
            if (did_open_door || did_bash_door)
            {
                /* Break down the door */
                if (did_bash_door && ((randint0(100) < 50) || (feat_state(c_ptr->feat, FF_OPEN) == c_ptr->feat) || have_flag(f_ptr->flags, FF_GLASS)))
                {
                    cave_alter_feat(ny, nx, FF_BASH);

                    if (!m_ptr->r_idx) /* Killed by shards of glass, etc. */
                    {
                        /* Update some things */
                        p_ptr->update |= (PU_FLOW);
                        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
                        mon_lore_2(m_ptr, RF2_BASH_DOOR);

                        return;
                    }
                }

                /* Open the door */
                else
                {
                    cave_alter_feat(ny, nx, FF_OPEN);
                }

                f_ptr = &f_info[c_ptr->feat];

                /* Handle viewable doors */
                do_view = TRUE;
            }
        }

        /* Hack -- check for Glyph of Warding */
        if (do_move && is_glyph_grid(c_ptr) &&
            !((r_ptr->flags1 & RF1_NEVER_BLOW) && player_bold(ny, nx)))
        {
            /* Assume no move allowed */
            do_move = FALSE;

            /* Break the ward */
            if (!is_pet(m_ptr) && (randint1(player_bold(ny, nx) ? BREAK_GLYPH * 2 / 3: BREAK_GLYPH) < r_ptr->level))
            {
                /* Describe observable breakage */
                if (c_ptr->info & CAVE_MARK)
                    msg_print("The rune of protection is broken!");

                /* Forget the rune */
                c_ptr->info &= ~(CAVE_MARK);

                /* Break the rune */
                c_ptr->info &= ~(CAVE_OBJECT);
                c_ptr->mimic = 0;

                /* Allow movement */
                do_move = TRUE;

                /* Notice */
                note_spot(ny, nx);
            }
        }
        else if (do_move && is_mon_trap_grid(c_ptr) &&
             !((r_ptr->flags1 & RF1_NEVER_BLOW) && player_bold(ny, nx)))
        {
            do_move = FALSE;

            if (!is_pet(m_ptr))
            {
                int y, x;
                y = m_ptr->fy;
                x = m_ptr->fx;
                hit_mon_trap(ny, nx, m_idx);

                /* Killed Monster? */
                if (!m_ptr->r_idx) return;

                /* Teleported Monster? */
                if (m_ptr->fy != y || m_ptr->fx != x) return;

                /* Slept/Confused/Stunned monster? */
                if (MON_CONFUSED(m_ptr) || MON_STUNNED(m_ptr) || MON_CSLEEP(m_ptr)) return;

                /* Bug with traps that summon?? Better to cancel the move until I can figure
                   out the issue.
                 */
                do_move = FALSE;
            }
        }

        /* The player is in the way */
        if (do_move && player_bold(ny, nx))
        {
            /* Some monsters never attack */
            if (r_ptr->flags1 & RF1_NEVER_BLOW)
            {
                /* Hack -- memorize lack of attacks */
                mon_lore_1(m_ptr, RF1_NEVER_BLOW);

                /* Do not move */
                do_move = FALSE;
            }

            /* In anti-melee dungeon, stupid or confused monster takes useless turn */
            if (do_move && (d_info[dungeon_type].flags1 & DF1_NO_MELEE))
            {
                if (!MON_CONFUSED(m_ptr))
                {
                    if (!(r_ptr->flags2 & RF2_STUPID)) do_move = FALSE;
                    else
                    {
                        mon_lore_2(m_ptr, RF2_STUPID);
                    }
                }
            }

            /* The player is in the way. Attack him. */
            if (do_move)
            {
                if (!p_ptr->riding || one_in_(2))
                {
                    /* Do the attack */
                    (void)make_attack_normal(m_idx);
                    if ((r_ptr->flags2 & RF2_INVISIBLE) && p_ptr->see_inv && !m_ptr->ml)
                        update_mon(m_idx, FALSE);

                    /* Do not move */
                    do_move = FALSE;

                    /* Took a turn */
                    do_turn = TRUE;
                }
            }
        }

        /* A monster is in the way */
        if (do_move && c_ptr->m_idx)
        {
            monster_race *z_ptr = &r_info[y_ptr->r_idx];

            /* Assume no movement */
            do_move = FALSE;

            /* Attack 'enemies' */
            if (((r_ptr->flags2 & RF2_KILL_BODY) && !(r_ptr->flags1 & RF1_NEVER_BLOW) &&
                 (r_ptr->mexp * r_ptr->level > z_ptr->mexp * z_ptr->level) &&
                 can_cross && (c_ptr->m_idx != p_ptr->riding)) ||
                are_enemies(m_ptr, y_ptr) || MON_CONFUSED(m_ptr))
            {
                if (!(r_ptr->flags1 & RF1_NEVER_BLOW))
                {
                    if (r_ptr->flags2 & RF2_KILL_BODY)
                    {
                        mon_lore_2(m_ptr, RF2_KILL_BODY);
                    }

                    /* attack */
                    if (y_ptr->r_idx && (y_ptr->hp >= 0))
                    {
                        if (mon_attack_mon(m_idx, c_ptr->m_idx)) return;

                        /* In anti-melee dungeon, stupid or confused monster takes useless turn */
                        else if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
                        {
                            if (MON_CONFUSED(m_ptr)) return;
                            else if (r_ptr->flags2 & RF2_STUPID)
                            {
                                mon_lore_2(m_ptr, RF2_STUPID);
                                return;
                            }
                        }
                    }
                }
            }

            /* Push past weaker monsters (unless leaving a wall) */
            else if ((r_ptr->flags2 & RF2_MOVE_BODY) && !(r_ptr->flags1 & RF1_NEVER_MOVE) &&
                (r_ptr->mexp > z_ptr->mexp) &&
                can_cross && (c_ptr->m_idx != p_ptr->riding) &&
                monster_can_cross_terrain(cave[m_ptr->fy][m_ptr->fx].feat, z_ptr, 0))
            {
                /* Allow movement */
                do_move = TRUE;

                /* Monster pushed past another monster */
                did_move_body = TRUE;

                /* Wake up the moved monster */
                (void)set_monster_csleep(c_ptr->m_idx, 0);

                /* XXX XXX XXX Message */
            }
        }

        if (is_riding_mon)
        {
            if (!p_ptr->riding_ryoute && !MON_MONFEAR(&m_list[p_ptr->riding])) do_move = FALSE;
        }

        if (r_ptr->level > 70 && do_move && r_ptr->flags2 & RF2_KILL_WALL)
        {
            int dir, x, y;
            cave_type       *c_ptr;
            feature_type    *f_ptr;
            for (dir = 0; dir < 8; dir++)
            {
                y = ny + ddy_ddd[dir];
                x = nx + ddx_ddd[dir];

                if (!in_bounds(y, x)) continue;

                c_ptr = &cave[y][x];
                f_ptr = &f_info[c_ptr->feat];

                if (have_flag(f_ptr->flags, FF_PERMANENT)) continue;

                cave_alter_feat(y, x, FF_HURT_DISI);

                if (m_ptr->r_idx == MON_GOTHMOG)
                {
                    if (c_ptr->feat != feat_deep_lava)
                        cave_set_feat(y, x, feat_shallow_lava);
                }
            }

            if (m_ptr->r_idx == MON_GOTHMOG)
                cave_set_feat(ny, nx, feat_deep_lava);
        }


        if (did_kill_wall && do_move)
        {
            if (one_in_(GRINDNOISE))
            {
                if (have_flag(f_ptr->flags, FF_GLASS))
                    msg_print("There is a crashing sound.");
                else
                    msg_print("There is a grinding sound.");
            }


            cave_alter_feat(ny, nx, FF_HURT_DISI);

            if (!m_ptr->r_idx) /* Killed by shards of glass, etc. */
            {
                /* Update some things */
                p_ptr->update |= (PU_FLOW);
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
                mon_lore_2(m_ptr, RF2_KILL_WALL);

                return;
            }

            f_ptr = &f_info[c_ptr->feat];

            /* Note changes to viewable region */
            do_view = TRUE;

            /* Take a turn */
            do_turn = TRUE;
        }

        if (must_alter_to_move && (r_ptr->flags7 & RF7_AQUATIC))
        {
            if (!monster_can_cross_terrain(c_ptr->feat, r_ptr, is_riding_mon ? CEM_RIDING : 0))
            {
                /* Assume no move allowed */
                do_move = FALSE;
            }
        }

        /*
         * Check if monster can cross terrain
         * This is checked after the normal attacks
         * to allow monsters to attack an enemy,
         * even if it can't enter the terrain.
         */
        if (do_move && !can_cross && !did_kill_wall && !did_bash_door)
        {
            /* Assume no move allowed */
            do_move = FALSE;
        }

        /* Some monsters never move */
        if (do_move && (r_ptr->flags1 & RF1_NEVER_MOVE))
        {
            /* Hack -- memorize lack of moves */
            mon_lore_1(m_ptr, RF1_NEVER_MOVE);

            /* Do not move */
            do_move = FALSE;
            if ((r_ptr->flags2 & RF2_INVISIBLE) && p_ptr->see_inv)
                update_mon(m_idx, FALSE);
        }

        /* Creature has been allowed move */
        if (do_move)
        {
            /* Take a turn */
            do_turn = TRUE;

            if (have_flag(f_ptr->flags, FF_TREE))
            {
                if (!(r_ptr->flags7 & RF7_CAN_FLY) && !(r_ptr->flags8 & RF8_WILD_WOOD))
                {
                    m_ptr->energy_need += ENERGY_NEED();
                }
            }
            if (have_flag(f_ptr->flags, FF_WEB) && r_ptr->d_char != 'S')
            {
                m_ptr->energy_need += ENERGY_NEED();
                /* Monster hacks web to bits? We assume at the moment that only
                 * the player (in particular, a Spider player monster race) can
                 * produce webs. */
                if (!(r_ptr->flags2 & RF2_PASS_WALL) && mon_save_p(m_ptr->r_idx, A_NONE))
                {
                    cave_set_feat(ny, nx, floor_type[randint0(100)]);
                }
            }

            if (!is_riding_mon)
            {
                /* Hack -- Update the old location */
                cave[oy][ox].m_idx = c_ptr->m_idx;

                /* Mega-Hack -- move the old monster, if any */
                if (c_ptr->m_idx)
                {
                    /* Move the old monster */
                    y_ptr->fy = oy;
                    y_ptr->fx = ox;

                    /* Update the old monster */
                    update_mon(c_ptr->m_idx, TRUE);
                }

                /* Hack -- Update the new location */
                c_ptr->m_idx = m_idx;

                /* Move the monster */
                m_ptr->fy = ny;
                m_ptr->fx = nx;
                p_ptr->window |= PW_MONSTER_LIST; /* May have moved in our out of LOS */

                /* Update the monster */
                update_mon(m_idx, TRUE);

                /* Redraw the old grid */
                lite_spot(oy, ox);

                /* Redraw the new grid */
                lite_spot(ny, nx);

                /* Hack: Some classes have techniques to apply whenever a monster
                   moves too close */
                if (class_ptr && class_ptr->move_monster)
                {
                    class_ptr->move_monster(m_idx);
                    if (!m_list[m_idx].r_idx) /* dead? */
                        return;
                }
            }
            else
            {
                /* Sound */
                /* sound(SOUND_WALK); */

                /* Move the player */
                if (!move_player_effect(ny, nx, MPE_DONT_PICKUP)) break;
            }

            /* Possible disturb */
            if (m_ptr->ml &&
                (disturb_move ||
                 (m_ptr->cdis <= 2 && projectable(py, px, m_ptr->fy, m_ptr->fx)) ||
                 (disturb_near && projectable(py, px, m_ptr->fy, m_ptr->fx)) ||
                 (disturb_high && ap_r_ptr->r_tkills && ap_r_ptr->level >= p_ptr->lev)))
            {
                if (town_no_disturb && py_in_town() && r_ptr->level == 0)
                {
                }
                else if (is_hostile(m_ptr))
                    disturb(0, 0);
            }

            /* Take or Kill objects on the floor */
            if (c_ptr->o_idx && (r_ptr->flags2 & (RF2_TAKE_ITEM | RF2_KILL_ITEM)) &&
                (!is_pet(m_ptr) || ((p_ptr->pet_extra_flags & PF_PICKUP_ITEMS) && (r_ptr->flags2 & RF2_TAKE_ITEM))))
            {
                s16b this_o_idx, next_o_idx;
                bool do_take = (r_ptr->flags2 & RF2_TAKE_ITEM) ? TRUE : FALSE;

                /* Scan all objects in the grid */
                for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                {
                    u32b flgs[OF_ARRAY_SIZE], flg2 = 0L, flg3 = 0L, flgr = 0L;
                    char m_name[80], o_name[MAX_NLEN];

                    /* Acquire object */
                    object_type *o_ptr = &o_list[this_o_idx];

                    /* Acquire next object */
                    next_o_idx = o_ptr->next_o_idx;

                    if (do_take)
                    {
                        /* Skip gold */
                        if (o_ptr->tval == TV_GOLD) continue;

                        /*
                         * Skip "real" corpses and statues, to avoid extreme
                         * silliness like a novice rogue pockets full of statues
                         * and corpses.
                         */
                        if ((o_ptr->tval == TV_CORPSE) ||
                            (o_ptr->tval == TV_STATUE)) continue;
                    }

                    /* Extract some flags */
                    obj_flags(o_ptr, flgs);

                    /* Acquire the object name */
                    object_desc(o_name, o_ptr, 0);

                    /* Acquire the monster name */
                    monster_desc(m_name, m_ptr, MD_INDEF_HIDDEN);

                    /* React to objects that hurt the monster */
                    if (have_flag(flgs, OF_SLAY_DRAGON)) flg3 |= (RF3_DRAGON);
                    if (have_flag(flgs, OF_KILL_DRAGON)) flg3 |= (RF3_DRAGON);
                    if (have_flag(flgs, OF_SLAY_TROLL))  flg3 |= (RF3_TROLL);
                    if (have_flag(flgs, OF_KILL_TROLL))  flg3 |= (RF3_TROLL);
                    if (have_flag(flgs, OF_SLAY_GIANT))  flg3 |= (RF3_GIANT);
                    if (have_flag(flgs, OF_KILL_GIANT))  flg3 |= (RF3_GIANT);
                    if (have_flag(flgs, OF_SLAY_ORC))    flg3 |= (RF3_ORC);
                    if (have_flag(flgs, OF_KILL_ORC))    flg3 |= (RF3_ORC);
                    if (have_flag(flgs, OF_SLAY_DEMON))  flg3 |= (RF3_DEMON);
                    if (have_flag(flgs, OF_KILL_DEMON))  flg3 |= (RF3_DEMON);
                    if (have_flag(flgs, OF_SLAY_UNDEAD)) flg3 |= (RF3_UNDEAD);
                    if (have_flag(flgs, OF_KILL_UNDEAD)) flg3 |= (RF3_UNDEAD);
                    if (have_flag(flgs, OF_SLAY_ANIMAL)) flg3 |= (RF3_ANIMAL);
                    if (have_flag(flgs, OF_KILL_ANIMAL)) flg3 |= (RF3_ANIMAL);
                    if (have_flag(flgs, OF_SLAY_EVIL))   flg3 |= (RF3_EVIL);
                    if (have_flag(flgs, OF_KILL_EVIL))   flg3 |= (RF3_EVIL);
                    if (have_flag(flgs, OF_SLAY_GOOD))   flg3 |= (RF3_GOOD);
					if (have_flag(flgs, OF_KILL_GOOD))   flg3 |= (RF3_GOOD);
                    if (have_flag(flgs, OF_SLAY_HUMAN))  flg2 |= (RF2_HUMAN);
                    if (have_flag(flgs, OF_KILL_HUMAN))  flg2 |= (RF2_HUMAN);
                    if (have_flag(flgs, OF_BRAND_ACID))  flgr |= (RFR_IM_ACID);
                    if (have_flag(flgs, OF_BRAND_ELEC))  flgr |= (RFR_IM_ELEC);
                    if (have_flag(flgs, OF_BRAND_FIRE))  flgr |= (RFR_IM_FIRE);
                    if (have_flag(flgs, OF_BRAND_COLD))  flgr |= (RFR_IM_COLD);
                    if (have_flag(flgs, OF_BRAND_POIS))  flgr |= (RFR_IM_POIS);

                    /* The object cannot be picked up by the monster */
                    if (object_is_artifact(o_ptr) || (r_ptr->flags3 & flg3) || (r_ptr->flags2 & flg2) ||
                        ((~(r_ptr->flagsr) & flgr) && !(r_ptr->flagsr & RFR_RES_ALL)))
                    {
                        /* Only give a message for "take_item" */
                        if (do_take && (r_ptr->flags2 & RF2_STUPID))
                        {
                            /* Take note */
                            did_take_item = TRUE;

                            /* Describe observable situations */
                            if (m_ptr->ml && player_can_see_bold(ny, nx))
                            {
                                /* Dump a message */
                                msg_format("%^s tries to pick up %s, but fails.", m_name, o_name);
                            }
                        }
                    }

                    /* Pick up the item */
                    else if (do_take)
                    {
                        /* Take note */
                        did_take_item = TRUE;

                        /* Describe observable situations */
                        if (player_can_see_bold(ny, nx))
                        {
                            /* Dump a message */
                            msg_format("%^s picks up %s.", m_name, o_name);
                        }

                        /* Excise the object */
                        excise_object_idx(this_o_idx);

                        /* Forget mark */
                        o_ptr->marked &= (OM_TOUCHED | OM_COUNTED | OM_EFFECT_COUNTED | OM_EGO_COUNTED | OM_ART_COUNTED);

                        /* Forget location */
                        o_ptr->loc.y = o_ptr->loc.x = 0;

                        /* Memorize monster */
                        o_ptr->held_m_idx = m_idx;

                        /* Build a stack */
                        o_ptr->next_o_idx = m_ptr->hold_o_idx;

                        /* Carry object */
                        m_ptr->hold_o_idx = this_o_idx;
                    }

                    /* Destroy the item if not a pet */
                    else if (!is_pet(m_ptr))
                    {
                        /* Take note */
                        if (o_ptr->name2 != EGO_AMMO_ENDURANCE)
                            did_kill_item = TRUE;

                        /* Describe observable situations */
                        if (player_has_los_bold(ny, nx))
                        {
                            /* Dump a message */
                            if (o_ptr->name2 != EGO_AMMO_ENDURANCE)
                            {
                                msg_format("%^s destroys %s.", m_name, o_name);
                                stats_on_m_destroy(o_ptr, o_ptr->number);
                            }
                            else
                                msg_format("%^s tries to destroy %s but fails.", m_name, o_name);
                        }

                        /* Delete the object */
                        if (o_ptr->name2 != EGO_AMMO_ENDURANCE)
                            delete_object_idx(this_o_idx);
                    }
                }
            }
        }

        /* Stop when done */
        if (do_turn) break;
    }

    /*
     *  Forward movements failed, but now received LOS attack!
     *  Try to flow by smell.
     */
    if (p_ptr->no_flowed && i > 2 &&  m_ptr->target_y)
        m_ptr->mflag2 &= ~MFLAG2_NOFLOW;

    /* Lore Issues wrt ASC: Monsters often hang back out of los until they choose
     * a splash spell. We aren't counting the moves, but we are counting the spells,
     * so the reported frequency is too high. */
    if (asc_nerf && could_splash && do_turn && do_move && !projectable(py, px, m_ptr->fy, m_ptr->fx))
    {
        /*msg_print("<color:v>asc mon_lore_move</color>");*/
        mon_lore_move(m_ptr);
    }

    #if 0
    /* If we haven't done anything, try casting a spell again
     * XXX Is this really necessary? There is a lot of code to implement dynamic
     * spell frequencies that I'd rather not duplicate. Also, I'd rather not try
     * to reason about what the actual effect this logic has in the real world. When
     * spell frequencies seem to high, it is not obvious why. (Is it this rule, or
     * is it pack AI? Or is it monster anger?) Packs of hounds may get artificial
     * boosts due to weird correlations between pack layout (place_monster_group)
     * and actual m_idx processing (process_monsters). That is, are we processing
     * monster packs from innermost (placed first) to outermost? If not, it is not
     * obvious how exactly this is being avoided, but teleport into a room with a 
     * pack of hounds and you seem to get more breaths than you should. */
    if (!do_turn && !do_move && !MON_MONFEAR(m_ptr) && !is_riding_mon && aware)
    {
        /* Try to cast spell again */
        if (r_ptr->spells && randint1(100) <= r_ptr->spells->freq)
        {
            if (mon_spell_cast(m_ptr, NULL)) return;
        }
    }
    #endif


    /* Notice changes in view */
    if (do_view)
    {
        /* Update some things */
        p_ptr->update |= (PU_FLOW);

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
    }

    /* Notice changes in view */
    if (do_move && ((r_ptr->flags7 & (RF7_SELF_LD_MASK | RF7_HAS_DARK_1 | RF7_HAS_DARK_2))
        || ((r_ptr->flags7 & (RF7_HAS_LITE_1 | RF7_HAS_LITE_2)) && !p_ptr->inside_battle)))
    {
        /* Update some things */
        p_ptr->update |= (PU_MON_LITE);
    }

    /* Learn things from observable monster */
    if (is_original_ap_and_seen(m_ptr))
    {
        /* Monster opened a door */
        if (did_open_door) mon_lore_aux_2(r_ptr, RF2_OPEN_DOOR);

        /* Monster bashed a door */
        if (did_bash_door) mon_lore_aux_2(r_ptr, RF2_BASH_DOOR);

        /* Monster tried to pick something up */
        if (did_take_item) mon_lore_aux_2(r_ptr, RF2_TAKE_ITEM);

        /* Monster tried to crush something */
        if (did_kill_item) mon_lore_aux_2(r_ptr, RF2_KILL_ITEM);

        /* Monster pushed past another monster */
        if (did_move_body) mon_lore_aux_2(r_ptr, RF2_MOVE_BODY);

        /* Monster passed through a wall */
        if (did_pass_wall) mon_lore_aux_2(r_ptr, RF2_PASS_WALL);

        /* Monster destroyed a wall */
        if (did_kill_wall) mon_lore_aux_2(r_ptr, RF2_KILL_WALL);
    }


    /* Hack -- get "bold" if out of options */
    if (!do_turn && !do_move && MON_MONFEAR(m_ptr) && aware)
    {
        /* No longer afraid */
        (void)set_monster_monfear(m_idx, 0);

        /* Message if seen */
        if (see_m)
        {
            char m_name[80];

            /* Acquire the monster name */
            monster_desc(m_name, m_ptr, 0);

            /* Dump a message */
            msg_format("%^s turns to fight!", m_name);
        }

        if (m_ptr->ml) virtue_add(VIRTUE_COMPASSION, -1);

        /* XXX XXX XXX Actually do something now (?) */
    }
}

static u32b csleep_noise;
static void process_mon_mtimed(mon_ptr mon);

/*
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * Note that monsters can never move in the monster array (except when the
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",
 * especially when the player is running.
 *
 * Note the special "MFLAG_BORN" flag, which allows us to ignore "fresh"
 * monsters while they are still being "born". A monster is "fresh" only
 * during the turn in which it is created, and we use the "hack_m_idx" to
 * determine if the monster is yet to be processed during the current turn.
 *
 * Note the special "MFLAG_NICE" flag, which allows the player to get one
 * move before any "nasty" monsters get to use their spell attacks.
 *
 * Note that when the "knowledge" about the currently tracked monster
 * changes (flags, attacks, spells), we induce a redraw of the monster
 * recall window.
 */
void process_monsters(void)
{
    int             i;
    int             fx, fy;

    bool            test;

    monster_type    *m_ptr;
    monster_race    *r_ptr;

    int speed;

    /* Clear monster fighting indicator */
    mon_fight = FALSE;

    if (game_turn%TURNS_PER_TICK == 0)
        csleep_noise = (1L << (30 - p_ptr->skills.stl));

    /* Process the monsters (backwards) */
    for (i = m_max - 1; i >= 1; i--)
    {
        int radius = 0;

        /* Access the monster */
        m_ptr = &m_list[i];
        r_ptr = &r_info[m_ptr->r_idx];

        /* Handle "leaving" */
        if (p_ptr->leaving) break;

        /* Ignore "dead" monsters */
        if (!m_ptr->r_idx) continue;

        if (p_ptr->wild_mode) continue;

        /* Handle "fresh" monsters */
        if (m_ptr->mflag & MFLAG_BORN)
        {
            /* No longer "fresh" */
            m_ptr->mflag &= ~(MFLAG_BORN);

            /* Skip */
            continue;
        }

        if (game_turn%TURNS_PER_TICK == 0)
            process_mon_mtimed(m_ptr);

        /* Hack -- Require proximity */
        if (p_ptr->action == ACTION_GLITTER)
        {
            if (m_ptr->cdis >= AAF_LIMIT_RING) continue;
        }
        else
        {
            if (m_ptr->cdis >= AAF_LIMIT) continue;
        }

        /* Access the location */
        fx = m_ptr->fx;
        fy = m_ptr->fy;

        /* Hack -- Monsters are automatically aware of the player (except for mimics) */
        if (!is_aware(m_ptr))
        {
            if (p_ptr->prace != RACE_MON_RING)
                m_ptr->mflag2 |= MFLAG2_AWARE;
            else if (p_ptr->riding && player_has_los_bold(fy, fx) && !MON_CSLEEP(m_ptr))
            {
                /* Player has a ring bearer, which this monster can see */
                m_ptr->mflag2 |= MFLAG2_AWARE;
            }
        }

        /* Flow by smell is allowed */
        if (!p_ptr->no_flowed)
        {
            m_ptr->mflag2 &= ~MFLAG2_NOFLOW;
        }

        /* Assume no move */
        test = FALSE;

        /* Handle "sensing radius" */
        radius = r_ptr->aaf;
        if (is_pet(m_ptr) && radius > MAX_SIGHT)
            radius = MAX_SIGHT;
        else if ( p_ptr->prace == RACE_MON_RING
               && !p_ptr->riding
               && !is_aware(m_ptr)
               && mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER) )
        {
            /* Lure a potential ring bearer, no matter how distant */
            radius = AAF_LIMIT_RING;
        }
        else if (py_on_surface())
            radius *= 3;

        if (m_ptr->cdis <= radius)
        {
            /* We can "sense" the player */
            test = TRUE;
        }

        /* Handle "sight" and "aggravation" */
        else if ((m_ptr->cdis <= MAX_SIGHT) &&
            (player_has_los_bold(fy, fx) || (p_ptr->cursed & OFC_AGGRAVATE) || ((disciple_is_(DISCIPLE_TROIKA)) && (r_ptr->flags3 & RF3_ORC))))
        {
            /* We can "see" or "feel" the player */
            test = TRUE;
        }

        else if (m_ptr->target_y) test = TRUE;

        /* Do nothing */
        if (!test)
            continue;


        if (p_ptr->riding == i)
            speed = p_ptr->pspeed;
        else
        {
            speed = m_ptr->mspeed;

            /* Monsters move quickly in Nightmare mode */
            if (ironman_nightmare) speed += 5;

            if (MON_FAST(m_ptr)) speed += 10;
            speed -= monster_slow(m_ptr);
            if (p_ptr->filibuster) speed -= SPEED_ADJ_FILIBUSTER;
        }

        /* Give this monster some energy */
        m_ptr->energy_need -= SPEED_TO_ENERGY(speed);
//        {
//            char mon_name[MAX_NLEN];
//            monster_desc(mon_name, m_ptr, 0);
//            msg_format("%^s (%+d) now at %d (vs. %d)", mon_name, speed, m_ptr->energy_need, p_ptr->energy_need);
//        }

        /* Check for SI now in case the monster is sleeping or lacks enough
         * energy to move. Should the monster move or attack, we'll make another
         * (overriding) check again later. */
        if ((r_ptr->flags2 & RF2_INVISIBLE) && p_ptr->see_inv)
            update_mon(i, FALSE);

        /* Not enough energy to move */
        if (m_ptr->energy_need > 0) continue;

        /* Use up "some" energy */
        m_ptr->energy_need += ENERGY_NEED();

        /* Process the Psion's Ego Whip first since
           (a) it may kill the monster, obviating the need for further processing;
           (b) it should always fire even if (especially if!) the monster is frightened */
        if (p_ptr->pclass == CLASS_PSION)
        {                                     /* v------ Ego Whip killed it! */
            if (psion_process_monster(i)) continue;
        }

        if (m_ptr->mflag2 & MFLAG2_TRIPPED)
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            msg_format("%^s gets back up and looks mad as hell.", m_name);
            m_ptr->mflag2 &= ~MFLAG2_TRIPPED;
            mon_anger(m_ptr);
            continue;
        }

        if (!fear_process_m(i))
            continue;

        if (m_ptr->mtimed[MTIMED_PARALYZED])
        {
            set_monster_paralyzed(i, m_ptr->mtimed[MTIMED_PARALYZED] - 1);
            continue;
        }

        /* Save global index */
        hack_m_idx = i;

        /* Process the monster */
        msg_boundary();
        process_monster(i);

        reset_target(m_ptr);

        /* Mark as not hurt */
        m_ptr->mflag2 &= ~(MFLAG2_HURT);

        /* Give up flow_by_smell when it might useless */
        if (p_ptr->no_flowed && one_in_(3))
            m_ptr->mflag2 |= MFLAG2_NOFLOW;

        /* Hack -- notice death or departure */
        if (!p_ptr->playing || p_ptr->is_dead) break;

        /* Notice leaving */
        if (p_ptr->leaving) break;
    }

    /* Reset global index */
    hack_m_idx = 0;
}

/* Bound a number to a valid range. This could be a public utility.
 * Note: bound(int, int, int) is ambiguous in argument order while
 *       bound(int, range_t) leaves no doubt. */
typedef struct { int min; int max; } _range_t;
static _range_t _range(int min, int max)
{
    _range_t r;
    r.min = min;
    r.max = max;
    return r;
}
static int _bound(int n, _range_t r)
{
    if (n < r.min) return r.min;
    if (n > r.max) return r.max;
    return n;
}
/* All mtimed set functions return whether or not the
 * change could be observed by the player */
bool set_monster_paralyzed(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 100));
    if (v)
    {
        if (!MON_PARALYZED(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_PARALYZED(m_ptr))
            notice = TRUE;
    }
    m_ptr->mtimed[MTIMED_PARALYZED] = v;
    if (notice)
    {
        if (m_ptr->ml)
            check_mon_health_redraw(m_idx);
        if (r_info[m_ptr->r_idx].flags7 & RF7_HAS_LD_MASK)
            p_ptr->update |= PU_MON_LITE;
    }
    return notice;
}

bool set_monster_csleep(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 10000));
    if (v)
    {
        if (!MON_CSLEEP(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_CSLEEP(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_CSLEEP] = v;

    if (!notice) return FALSE;

    if (m_ptr->ml)
        check_mon_health_redraw(m_idx);

    if (r_info[m_ptr->r_idx].flags7 & RF7_HAS_LD_MASK)
        p_ptr->update |= (PU_MON_LITE);

    p_ptr->window |= PW_MONSTER_LIST;

    return TRUE;
}

bool set_monster_fast(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_FAST(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_FAST(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_FAST] = v;

    if (!notice) return FALSE;

    if ((p_ptr->riding == m_idx) && !p_ptr->leaving)
        p_ptr->update |= PU_BONUS;

    return TRUE;
}

bool set_monster_slow(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_SLOW(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_SLOW(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_SLOW] = v;

    if (!notice) return FALSE;

    if ((p_ptr->riding == m_idx) && !p_ptr->leaving)
        p_ptr->update |= PU_BONUS;

    return TRUE;
}

bool set_monster_stunned(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_STUNNED(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_STUNNED(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_STUNNED] = v;

    if (m_ptr->ml)
        check_mon_health_redraw(m_idx);

    return notice;
}

bool set_monster_confused(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_CONFUSED(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_CONFUSED(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_CONFUSED] = v;

    if (m_ptr->ml)
        check_mon_health_redraw(m_idx);

    return notice;
}

bool set_monster_monfear(int m_idx, int v)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_MONFEAR(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_MONFEAR(m_ptr))
            notice = TRUE;
    }

    m_ptr->mtimed[MTIMED_MONFEAR] = v;

    if (!notice) return FALSE;

    if (m_ptr->ml)
        check_mon_health_redraw(m_idx);

    return TRUE;
}

bool set_monster_invulner(int m_idx, int v, bool energy_need)
{
    monster_type *m_ptr = &m_list[m_idx];
    bool notice = FALSE;

    v = _bound(v, _range(0, 200));
    if (v)
    {
        if (!MON_INVULNER(m_ptr))
            notice = TRUE;
    }
    else
    {
        if (MON_INVULNER(m_ptr))
        {
            if (energy_need && !p_ptr->wild_mode) m_ptr->energy_need += ENERGY_NEED();
            notice = TRUE;
        }
    }

    m_ptr->mtimed[MTIMED_INVULNER] = v;

    if (!notice) return FALSE;

    if (m_ptr->ml)
        check_mon_health_redraw(m_idx);

    return TRUE;
}

static void process_mon_mtimed(mon_ptr mon)
{
    mon_race_ptr race = &r_info[mon->r_idx];
    if (mon->mtimed[MTIMED_CSLEEP])
    {
        bool test = FALSE;

        if (mon->cdis < AAF_LIMIT)
        {
            int aaf = race->aaf;
            if (is_pet(mon)) aaf = MIN(MAX_SIGHT, aaf);
            if (mon->cdis <= aaf) test = TRUE;
            else if (mon->cdis <= MAX_SIGHT
                && (player_has_los_bold(mon->fy, mon->fx)))
            {
                test = TRUE;
            }
        }

        if (test)
        {
            u32b notice = randint0(1024);
            u32b noise = csleep_noise;

            if (ironman_nightmare) notice /= 2;

            if (notice * notice * notice <= noise)
            {
                /* Wake up faster near the player */
                int d = (mon->cdis < AAF_LIMIT / 2) ? (AAF_LIMIT / mon->cdis) : 1;

                /* Hack -- amount of "waking" is affected by speed of player */
                d = (d * SPEED_TO_ENERGY(p_ptr->pspeed)) / 10;
                if (d < 0) d = 1;


                /* Still asleep */
                if (!set_monster_csleep(mon->id, MON_CSLEEP(mon) - d))
                {
                    if (is_original_ap_and_seen(mon))
                    {
                        if (race->r_ignore < MAX_UCHAR) race->r_ignore++;
                    }
                }
                /* Just woke up */
                else
                {
                    if (mon->ml && disturb_minor && mon_show_msg(mon))
                    {
                        char m_name[80];
                        monster_desc(m_name, mon, 0);
                        msg_format("%^s wakes up.", m_name);
                    }
                    if (is_original_ap_and_seen(mon))
                    {
                        if (race->r_wake < MAX_UCHAR) race->r_wake++;
                    }
                }
            }
        }
    }
    if (mon->mtimed[MTIMED_FAST])
    {
        if (set_monster_fast(mon->id, mon->mtimed[MTIMED_FAST] - 1))
        {
            if (mon_show_msg(mon))
            {
                char m_name[80];
                monster_desc(m_name, mon, 0);
                msg_format("%^s is no longer fast.", m_name);
            }
        }
    }
    if (mon->mtimed[MTIMED_SLOW])
    {
        if (set_monster_slow(mon->id, mon->mtimed[MTIMED_SLOW] - 1))
        {
            if (mon_show_msg(mon))
            {
                char m_name[80];
                monster_desc(m_name, mon, 0);
                msg_format("%^s is no longer slow.", m_name);
            }
        }
    }
    if (mon->mtimed[MTIMED_STUNNED])
    {
        int stun = mon->mtimed[MTIMED_STUNNED];
        int rlev = race->level;
        int dec = 1 + rlev/10;
        if (randint0(10000) < rlev * rlev) /* shake it off ... */
            dec = MAX(dec, stun/2);
        if (set_monster_stunned(mon->id, MON_STUNNED(mon) - dec))
        {
            if (mon_show_msg(mon))
            {
                char m_name[80];
                monster_desc(m_name, mon, 0);
                msg_format("%^s is no longer stunned.", m_name);
            }
        }
    }
    if (mon->mtimed[MTIMED_CONFUSED])
    {
        int dec = randint1(race->level/20 + 1);
        if (set_monster_confused(mon->id, MON_CONFUSED(mon) - dec)) 
        {
            if (mon_show_msg(mon))
            {
                char m_name[80];
                monster_desc(m_name, mon, 0);
                msg_format("%^s is no longer confused.", m_name);
            }
        }
    }
    if (mon->mtimed[MTIMED_INVULNER])
    {
        if (set_monster_invulner(mon->id, mon->mtimed[MTIMED_INVULNER] - 1, TRUE))
        {
            if (mon_show_msg(mon))
            {
                char m_name[80];
                monster_desc(m_name, mon, 0);
                msg_format("%^s is no longer invulnerable.", m_name);
            }
        }
    }
    if ((mon->minislow) && (randint0(race->flags2 & RF2_REGENERATE ? 50 : 100) < mon->minislow))
    {
        (void)m_inc_minislow(mon, -1);
    }
}

void dispel_monster_status(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    char         m_name[80];

    monster_desc(m_name, m_ptr, 0);
    if (set_monster_invulner(m_idx, 0, TRUE))
    {
        if (m_ptr->ml) msg_format("%^s is no longer invulnerable.", m_name);
    }
    if (set_monster_fast(m_idx, 0))
    {
        if (m_ptr->ml) msg_format("%^s is no longer fast.", m_name);
    }
    if (set_monster_slow(m_idx, 0))
    {
        if (m_ptr->ml) msg_format("%^s is no longer slow.", m_name);
    }
}


bool process_the_world(int num, int who, bool vs_player)
{
    monster_type *m_ptr = &m_list[hack_m_idx];  /* the world monster */

    if (world_monster) return (FALSE);

    if(vs_player)
    {
        char m_name[80];
        monster_desc(m_name, m_ptr, 0);

        if (who == 1)
            msg_format("%s yells 'The World! Time has stopped!'", m_name);
        else if (who == 3)
            msg_format("%s yells 'Time!'", m_name);
        else msg_print("hek!");

        if (p_ptr->pclass == CLASS_TIME_LORD && !mon_save_p(m_ptr->r_idx, A_WIS))
        {
            msg_format("You grin as time begins to flow once again!");
            return TRUE;
        }

        msg_print(NULL);
    }

    /* This monster cast spells */
    world_monster = hack_m_idx;

    if (vs_player) do_cmd_redraw();

    while(num--)
    {
        if(!m_ptr->r_idx) break;
        process_monster(world_monster);

        reset_target(m_ptr);

        /* Notice stuff */
        if (p_ptr->notice) notice_stuff();

        /* Update stuff */
        if (p_ptr->update) update_stuff();

        /* Redraw stuff */
        if (p_ptr->redraw) redraw_stuff();

        /* Redraw stuff */
        if (p_ptr->window) window_stuff();

        /* Delay */
        if (vs_player) Term_xtra(TERM_XTRA_DELAY, 500);
    }

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    world_monster = 0;
    if (vs_player || (player_has_los_bold(m_ptr->fy, m_ptr->fx) && projectable(py, px, m_ptr->fy, m_ptr->fx)))
    {
        msg_print("You feel time flowing around you once more.");
        msg_print(NULL);
    }

    handle_stuff();

    return (TRUE);
}

void mon_gain_exp(mon_ptr mon, int amt)
{
    mon_race_ptr race = &r_info[mon->r_idx];
    if (!race->next_exp) return;
    mon->exp += amt;

    if (mon->mflag2 & MFLAG2_CHAMELEON) return;

    if (mon->exp >= race->next_exp)
    {
        mon_change_race(mon, race->next_r_idx, "evolved");
    }
    if (mon->id == p_ptr->riding) p_ptr->update |= PU_BONUS;
}

void monster_gain_exp(int m_idx, int s_idx)
{
    monster_type *m_ptr;
    monster_type *sm_ptr;
    monster_race *r_ptr;
    monster_race *s_ptr;
    int new_exp;

    /* Paranoia */
    if (m_idx <= 0 || s_idx <= 0) return;

    m_ptr = &m_list[m_idx];

    /* Paranoia -- Skip dead monsters */
    if (!m_ptr->r_idx) return;

    /* The other monster shouldn't have been deleted yet */
    sm_ptr = &m_list[s_idx];
    if (!sm_ptr->r_idx) return;

    r_ptr = &r_info[m_ptr->r_idx];
    s_ptr = &r_info[sm_ptr->r_idx];

    if (p_ptr->inside_battle) return;

    /* No XP for player-aligned monsters killing player summons
     * (or summons of player summons) */
    if ((sm_ptr->mflag2 & MFLAG2_PLAYER_SUMMONED) && ((is_pet(m_ptr)) ||
        (is_friendly(m_ptr)) || (m_ptr->mflag2 & MFLAG2_PLAYER_SUMMONED)))
        return;

    new_exp = s_ptr->mexp * s_ptr->level / (r_ptr->level + 2);
    if (m_idx == p_ptr->riding) new_exp = (new_exp + 1) / 2;
    if (!dun_level) new_exp /= 5;

    /* Experimental: Share the xp with the player */
    if (is_pet(m_ptr))
    {
        int  div = 5;
        bool penalty = TRUE;
        int  exp;

        if ( prace_is_(RACE_MON_QUYLTHULG)
          || (prace_is_(RACE_MON_RING) && p_ptr->riding == m_idx) )
        {
            div = 1;
            penalty = FALSE;
        }

        exp = new_exp / div;
        gain_exp(exp);
        if (penalty)
            new_exp -= exp;
        if (new_exp < 0) new_exp = 0;
    }

    mon_gain_exp(m_ptr, new_exp);
}
