/*
 * File: streams.c
 * Purpose: Used by dungeon generation. This file holds all the
 * functions that are applied to a level after the rest has been
 * generated, ie streams and level destruction.
 */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

#include "angband.h"
#include "generate.h"
#include "streams.h"
#include "grid.h"


/*
 * Recursive fractal algorithm to place water through the dungeon.
 */
static void recursive_river(int x1, int y1, int x2, int y2, int feat1, int feat2, int width)
{
    int dx, dy, length, l, x, y;
    int changex, changey;
    int ty, tx;
    bool done;
    cave_type *c_ptr;

    length = distance(x1, y1, x2, y2);

    if (length > 4)
    {
        /*
         * Divide path in half and call routine twice.
         * There is a small chance of splitting the river
         */
        dx = (x2 - x1) / 2;
        dy = (y2 - y1) / 2;

        if (dy != 0)
        {
            /* perturbation perpendicular to path */
            changex = randint1(abs(dy)) * 2 - abs(dy);
        }
        else
        {
            changex = 0;
        }

        if (dx != 0)
        {
            /* perturbation perpendicular to path */
            changey = randint1(abs(dx)) * 2 - abs(dx);
        }
        else
        {
            changey = 0;
        }

        if (!in_bounds(y1 + dy + changey, x1 + dx + changex))
        {
            changex = 0;
            changey = 0;
        }

        /* construct river out of two smaller ones */
        recursive_river(x1, y1, x1 + dx + changex, y1 + dy + changey, feat1, feat2, width);
        recursive_river(x1 + dx + changex, y1 + dy + changey, x2, y2, feat1, feat2, width);

        /* Split the river some of the time - junctions look cool */
        if (one_in_(DUN_WAT_CHG) && (width > 0))
        {
            recursive_river(x1 + dx + changex, y1 + dy + changey,
                    x1 + 8 * (dx + changex), y1 + 8 * (dy + changey),
                    feat1, feat2, width - 1);
        }
    }
    else
    {
        /* Actually build the river */
        for (l = 0; l < length; l++)
        {
            x = x1 + l * (x2 - x1) / length;
            y = y1 + l * (y2 - y1) / length;

            done = FALSE;

            while (!done)
            {
                for (ty = y - width - 1; ty <= y + width + 1; ty++)
                {
                    for (tx = x - width - 1; tx <= x + width + 1; tx++)
                    {
                        if (!in_bounds2(ty, tx)) continue;

                        c_ptr = &cave[ty][tx];

                        if (c_ptr->feat == feat1) continue;
                        if (c_ptr->feat == feat2) continue;

                        if (distance(ty, tx, y, x) > rand_spread(width, 1)) continue;

                        /* Do not convert permanent features */
                        if (cave_perma_grid(c_ptr)) continue;

                        /*
                         * Clear previous contents, add feature
                         * The border mainly gets feat2, while the center gets feat1
                         */
                        if (distance(ty, tx, y, x) > width)
                            c_ptr->feat = feat2;
                        else
                            c_ptr->feat = feat1;

                        /* Clear garbage of hidden trap or door */
                        c_ptr->mimic = 0;

                        /* Lava terrain glows */
                        if (have_flag(f_info[feat1].flags, FF_LAVA))
                        {
                            if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS)) c_ptr->info |= CAVE_GLOW;
                        }

                        /* Hack -- don't teleport here */
                        c_ptr->info |= CAVE_ICKY;
                    }
                }

                done = TRUE;
            }
        }
    }
}


/*
 * Places water /lava through dungeon.
 */
void add_river(int feat1, int feat2)
{
    int y2, x2;
    int y1 = 0, x1 = 0;
    int wid;


    /* Hack -- Choose starting point */
    y2 = randint1(cur_hgt / 2 - 2) + cur_hgt / 2;
    x2 = randint1(cur_wid / 2 - 2) + cur_wid / 2;

    /* Hack -- Choose ending point somewhere on boundary */
    switch(randint1(4))
    {
        case 1:
        {
            /* top boundary */
            x1 = randint1(cur_wid-2)+1;
            y1 = 1;
            break;
        }
        case 2:
        {
            /* left boundary */
            x1 = 1;
            y1 = randint1(cur_hgt-2)+1;
            break;
        }
        case 3:
        {
            /* right boundary */
            x1 = cur_wid-1;
            y1 = randint1(cur_hgt-2)+1;
            break;
        }
        case 4:
        {
            /* bottom boundary */
            x1 = randint1(cur_wid-2)+1;
            y1 = cur_hgt-1;
            break;
        }
    }

    wid = randint1(DUN_WAT_RNG);
    recursive_river(x1, y1, x2, y2, feat1, feat2, wid);

    /* Hack - Save the location as a "room" */
    if (dun->cent_n < CENT_MAX)
    {
        dun->cent[dun->cent_n].y = y2;
        dun->cent[dun->cent_n].x = x2;
        dun->cent_n++;
    }
}


/*
 * Places "streamers" of rock through dungeon
 *
 * Note that there are actually six different terrain features used
 * to represent streamers. Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold. The
 * hidden gold types are currently unused.
 */
void build_streamer(int feat, int chance)
{
    int        i, tx, ty;
    int        y, x, dir;
    int dummy = 0;

    cave_type *c_ptr;
    feature_type *f_ptr;

    feature_type *streamer_ptr = &f_info[feat];
    bool streamer_is_wall = have_flag(streamer_ptr->flags, FF_WALL) && !have_flag(streamer_ptr->flags, FF_PERMANENT);
    bool streamer_may_have_gold = have_flag(streamer_ptr->flags, FF_MAY_HAVE_GOLD);

    /* Hack -- Choose starting point */
    y = rand_spread(cur_hgt / 2, cur_hgt / 6);
    x = rand_spread(cur_wid / 2, cur_wid / 6);

    /* Choose a random compass direction */
    dir = ddd[randint0(8)];

    /* Place streamer into dungeon */
    while (dummy < SAFE_MAX_ATTEMPTS)
    {
        dummy++;

        /* One grid per density */
        for (i = 0; i < DUN_STR_DEN; i++)
        {
            int d = DUN_STR_RNG;

            /* Pick a nearby grid */
            while (1)
            {
                ty = rand_spread(y, d);
                tx = rand_spread(x, d);
                if (!in_bounds2(ty, tx)) continue;
                break;
            }

            /* Access the grid */
            c_ptr = &cave[ty][tx];
            f_ptr = &f_info[c_ptr->feat];

            if (have_flag(f_ptr->flags, FF_MOVE) && (have_flag(f_ptr->flags, FF_WATER) || have_flag(f_ptr->flags, FF_LAVA)))
                continue;

            /* Do not convert permanent features */
            if (have_flag(f_ptr->flags, FF_PERMANENT)) continue;

            /* Only convert "granite" walls */
            if (streamer_is_wall)
            {
                if (!is_extra_grid(c_ptr) && !is_inner_grid(c_ptr) && !is_outer_grid(c_ptr) && !is_solid_grid(c_ptr)) continue;
                if (is_closed_door(c_ptr->feat)) continue;
            }

            if (c_ptr->m_idx && !(have_flag(streamer_ptr->flags, FF_PLACE) && monster_can_cross_terrain(feat, &r_info[m_list[c_ptr->m_idx].r_idx], 0)))
            {
                /* Delete the monster (if any) */
                delete_monster(ty, tx);
            }

            if (c_ptr->o_idx && !have_flag(streamer_ptr->flags, FF_DROP))
            {
                s16b this_o_idx, next_o_idx = 0;

                /* Scan all objects in the grid */
                for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
                {
                    /* Acquire object */
                    object_type *o_ptr = &o_list[this_o_idx];

                    /* Acquire next object */
                    next_o_idx = o_ptr->next_o_idx;

                    /* Hack -- Preserve unknown artifacts */
                    if (object_is_fixed_artifact(o_ptr))
                    {
                        /* Mega-Hack -- Preserve the artifact */
                        a_info[o_ptr->name1].generated = FALSE;

                        if (cheat_peek)
                        {
                            char o_name[MAX_NLEN];
                            object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));
                            msg_format("Artifact (%s) was deleted by streamer.", o_name);
                        }
                    }
                    else if (random_artifacts && o_ptr->name3)
                    {
                        /* Mega-Hack -- Preserve the artifact */
                        a_info[o_ptr->name3].generated = FALSE;

                        if (cheat_peek)
                        {
                            char o_name[MAX_NLEN];
                            object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));
                            msg_format("Artifact (%s) was deleted by streamer.", o_name);
                        }
                    }
                    else if (cheat_peek && o_ptr->art_name)
                    {
                        msg_print("One of the random artifacts was deleted by streamer.");
                    }
                }

                /* Delete objects */
                delete_object(ty, tx);
            }

            /* Clear previous contents, add proper vein type */
            c_ptr->feat = feat;

            /* Paranoia: Clear mimic field */
            c_ptr->mimic = 0;

            if (streamer_may_have_gold)
            {
                /* Hack -- Add some known treasure */
                if (one_in_(chance))
                {
                    cave_alter_feat(ty, tx, FF_MAY_HAVE_GOLD);
                }

                /* Hack -- Add some hidden treasure */
                else if (one_in_(chance / 3))
                {
                    cave_alter_feat(ty, tx, FF_MAY_HAVE_GOLD);
                    cave_alter_feat(ty, tx, FF_ENSECRET);
                }
            }
        }

        if (dummy >= SAFE_MAX_ATTEMPTS)
        {
            if (cheat_room)
            {
                msg_print("Warning! Could not place streamer!");

            }
            return;
        }


        /* Advance the streamer */
        y += ddy[dir];
        x += ddx[dir];

        /* Quit before leaving the dungeon */
        if (!in_bounds(y, x)) break;
    }
}


/*
 * Put trees near a hole in the dungeon roof  (rubble on ground + up stairway)
 * This happens in real world lava tubes.
 */
void place_trees(int x, int y)
{
    int i, j;
    cave_type *c_ptr;

    /* place trees/ rubble in ovalish distribution */
    for (i = x - 3; i < x + 4; i++)
    {
        for (j = y - 3; j < y + 4; j++)
        {
            if (!in_bounds(j, i)) continue;
            c_ptr = &cave[j][i];

            if (c_ptr->info & CAVE_ICKY) continue;
            if (c_ptr->o_idx) continue;

            /* Want square to be in the circle and accessable. */
            if ((distance(j, i, y, x) < 4) && !cave_perma_grid(c_ptr))
            {
                /*
                 * Clear previous contents, add feature
                 * The border mainly gets trees, while the center gets rubble
                 */
                if ((distance(j, i, y, x) > 1) || (randint1(100) < 25))
                {
                    if (randint1(100) < 75)
                        cave[j][i].feat = feat_tree;
                }
                else
                {
                    cave[j][i].feat = feat_rubble;
                }

                /* Clear garbage of hidden trap or door */
                c_ptr->mimic = 0;

                /* Light area since is open above */
                if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS)) cave[j][i].info |= (CAVE_GLOW | CAVE_ROOM);
            }
        }
    }

    /* No up stairs in ironman mode */
    if (!only_downward() && one_in_(3))
    {
        /* up stair */
        cave[y][x].feat = feat_up_stair;
    }
}


/*
 * Build a destroyed level
 */
void destroy_level(void)
{
    int y1, x1, n;

    /* Note destroyed levels */
    if (cheat_room) msg_print("Destroyed Level");

    /* Drop a few epi-centers (usually about two) */
    for (n = 0; n < randint1(5); n++)
    {
        /* Pick an epi-center */
        x1 = rand_range(5, cur_wid - 1 - 5);
        y1 = rand_range(5, cur_hgt - 1 - 5);

        (void)destroy_area(y1, x1, 15, -1);
    }
}
