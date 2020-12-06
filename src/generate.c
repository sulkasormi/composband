/* File: generate.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Dungeon generation */

/*
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines... Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "v_info.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors. Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type. The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away). This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart. This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks. As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds. Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the old 33x11 blocks)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries. This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen. It may be annoying enough to go back to 33x11
 * blocks to prevent this visual situation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening. But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits". The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon). Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite. XXX XXX XXX
 */

#include "angband.h"
#include "generate.h"
#include "grid.h"
#include "str-map.h"
#include "rooms.h"
#include "streams.h"

int enter_quest = 0;

int dun_tun_rnd;
int dun_tun_chg;
int dun_tun_con;
int dun_tun_pen;
int dun_tun_jct;


/*
 * Dungeon generation data -- see "cave_gen()"
 */
dun_data *dun;

/* Check if grid is wall-like */
static bool _is_wally(int y, int x, bool check_bold)
{
    if ((check_bold) && (!is_extra_bold(y, x))) return FALSE;
    if ((!check_bold) || (atlantis_hack)) return (
        ((cave_have_flag_bold(y, x, FF_WALL)) ||
        ((dungeon_type) && (dungeon_type < max_d_idx) &&
        (d_info[dungeon_type].outer_wall == cave[y][x].feat))) ? TRUE : FALSE);
    return TRUE;
}


/*
 * Count the number of walls adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y, x)"
 *
 * We count only granite walls and permanent walls.
 */
static int next_to_walls(int y, int x, bool bold)
{
    int k = 0;

    if (in_bounds(y + 1, x) && _is_wally(y + 1, x, bold)) k++;
    if (in_bounds(y - 1, x) && _is_wally(y - 1, x, bold)) k++;
    if (in_bounds(y, x + 1) && _is_wally(y, x + 1, bold)) k++;
    if (in_bounds(y, x - 1) && _is_wally(y, x - 1, bold)) k++;

    return (k);
}


/*
 *  Helper function for alloc_stairs().
 *
 *  Is this a good location for stairs?
 */
static bool alloc_stairs_aux(int y, int x, int walls)
{
    /* Access the grid */
    cave_type *c_ptr = &cave[y][x];

    /* Require "naked" floor grid */
    if (!is_floor_grid(c_ptr)) return FALSE;
    if (pattern_tile(y, x)) return FALSE;
    if (c_ptr->o_idx || c_ptr->m_idx) return FALSE;

    /* Require a certain number of adjacent walls */
    if (next_to_walls(y, x, TRUE) < walls) return FALSE;

    return TRUE;
}


/*
 * Places some staircases near walls
 */
static bool alloc_stairs(int feat, int num, int walls)
{
    int i;
    int shaft_num = 0;

    feature_type *f_ptr = &f_info[feat];

    /* Aussie hack - try to avoid billabongs becoming a stair magnet */
    if (dungeon_type == DUNGEON_AUSSIE) walls = randint0(walls);

    if (have_flag(f_ptr->flags, FF_LESS))
    {
        /* No up stairs in town or in ironman mode */
        if (only_downward() || !dun_level) return TRUE;

        /* No way out!!
        if ( dun_level == d_info[dungeon_type].mindepth
          && (dungeon_flags[dungeon_type] & DUNGEON_NO_ENTRANCE) )
        {
            return TRUE;
        }

        if ( dun_level == d_info[dungeon_type].mindepth + 1
          && (dungeon_flags[dungeon_type] & DUNGEON_NO_ENTRANCE) )
        {
            shaft_num = 0;
        }
        else */if (dun_level > d_info[dungeon_type].mindepth)
        {
            shaft_num = (randint1(num+1))/2;
            /* Battlefield is all shafts */
            if ((d_info[dungeon_type].flags1 & DF1_ALL_SHAFTS) || (coffee_break)) shaft_num = num;
        }
    }
    else if (have_flag(f_ptr->flags, FF_MORE))
    {
        /* player must complete the quest to gain the down staircase */
        if (!quests_allow_downstairs()) return TRUE;

        /* No downstairs on random wilderness entrances */
        if (d_info[dungeon_type].flags1 & DF1_RANDOM) return TRUE;

        /* No downstairs at the bottom */
        if (dun_level >= d_info[dungeon_type].maxdepth) return TRUE;

        if ( dun_level < d_info[dungeon_type].maxdepth - 1
        /* Note: If we exclude downshafts, then the astute player can be certain
         * that normal stairs do *not* lead to a quest level. Instead, we'll change
         * the behavior of FF_SHAFT in do_cmd_go_down. */
        /*&& quests_allow_downshaft()*/)
        {
            shaft_num = (randint1(num)+1)/2;
            if ((d_info[dungeon_type].flags1 & DF1_ALL_SHAFTS) || (coffee_break)) shaft_num = num;
        }
    }

    /* Paranoia */
    else return FALSE;

    /* Place "num" stairs */
    for (i = 0; i < num; i++)
    {
        while (TRUE)
        {
            int y = 0, x = 0;
            cave_type *c_ptr;

            int candidates = 0;
            int pick;

            for (y = 1; y < cur_hgt - 1; y++)
            {
                for (x = 1; x < cur_wid - 1; x++)
                {
                    if (alloc_stairs_aux(y, x, walls))
                    {
                        /* A valid space found */
                        candidates++;
                    }
                }
            }

            /* No valid place! */
            if (!candidates)
            {
                /* There are exactly no place! */
                if (walls <= 0) return FALSE;

                /* Decrease walls limit, and try again */
                walls--;
                continue;
            }

            /* Choose a random one */
            pick = randint1(candidates);

            for (y = 1; y < cur_hgt - 1; y++)
            {
                for (x = 1; x < cur_wid - 1; x++)
                {
                    if (alloc_stairs_aux(y, x, walls))
                    {
                        pick--;

                        /* Is this a picked one? */
                        if (!pick) break;
                    }
                }

                if (!pick) break;
            }

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Clear possible garbage of hidden trap */
            c_ptr->mimic = 0;

            /* Clear previous contents, add stairs */
            c_ptr->feat = (i < shaft_num) ? feat_state(feat, FF_SHAFT) : feat;

            /* No longer "FLOOR" */
            c_ptr->info &= ~(CAVE_FLOOR);

            /* Success */
            break;
        }
    }
    return TRUE;
}

static bool _get_loc(int set, int* x, int *y)
{
    int dummy = 0;
    while (dummy < SAFE_MAX_ATTEMPTS)
    {
        bool       room;
        cave_type *c_ptr;

        dummy++;

        *x = randint0(cur_wid);
        *y = randint0(cur_hgt);

        c_ptr = &cave[*y][*x];

        if (!is_floor_grid(c_ptr) || c_ptr->o_idx || c_ptr->m_idx) continue;
        if (player_bold(*y, *x)) continue;

        room = (cave[*y][*x].info & CAVE_ROOM) ? TRUE : FALSE;
        if (set == ALLOC_SET_CORR && room) continue;
        if (set == ALLOC_SET_ROOM && !room) continue;

        return TRUE;
    }
    return FALSE;
}

/*
 * Allocates some objects (using "place" and "type")
 */
static int _adjust_small_level(int n);
static void alloc_object(int set, int typ, int num)
{
    int y = 0, x = 0, k;

    /* Diligent players should be encouraged to explore more! */
    if (typ == ALLOC_TYP_OBJECT || typ == ALLOC_TYP_GOLD)
        num = num * (625 + virtue_current(VIRTUE_DILIGENCE)) / 625;

    /* A small level has few objects. */
    num = MAX(1, _adjust_small_level(num));

    for (k = 0; k < num; k++)
    {
        object_type forge;
        int         k_idx;

        if (!_get_loc(set, &x, &y))
        {
            if (cheat_room)
                msg_print("Warning! Could not place object!");

            return;
        }

        switch (typ)
        {
        case ALLOC_TYP_RUBBLE:
            place_rubble(y, x);
            cave[y][x].info &= ~(CAVE_FLOOR);
            break;

        case ALLOC_TYP_TRAP:
            place_trap(y, x);
            cave[y][x].info &= ~(CAVE_FLOOR);
            break;

        case ALLOC_TYP_GOLD:
            place_gold(y, x);
            break;

        case ALLOC_TYP_OBJECT:
            /* Comment: Monsters drop objects at (ML + DL)/2. In practice,
               this means that your best drops are just laying on the ground,
               and this encourages recall scumming for end game resources such
               as wands of rockets. Note: Vaults are not affected and we want
               to encourage these! Room templates need some thought ... */
            if (base_level > 31)
            {
               int n = base_level - 30;
               object_level = 30 + n/2 + randint1(n/2);
            }
            else
                object_level = base_level; /* paranoia */
            place_object(y, x, 0L, ORIGIN_FLOOR);
            object_level = base_level;
            break;

        case ALLOC_TYP_FOOD:
            if (prace_is_(RACE_ENT))
                k_idx = lookup_kind(TV_POTION, SV_POTION_WATER);
            else
                k_idx = lookup_kind(TV_FOOD, SV_FOOD_RATION);
            object_prep(&forge, k_idx);
            obj_make_pile(&forge);
            object_origins(&forge, ORIGIN_FLOOR);
            drop_near(&forge, -1, y, x);
            break;

        case ALLOC_TYP_LIGHT:
            if (one_in_(3))
                k_idx = lookup_kind(TV_FLASK, SV_FLASK_OIL);
            else
                k_idx = lookup_kind(TV_LITE, SV_LITE_LANTERN);
            object_prep(&forge, k_idx);
            apply_magic(&forge, dun_level, 0);
            obj_make_pile(&forge);
            object_origins(&forge, ORIGIN_FLOOR);
            drop_near(&forge, -1, y, x);
            break;

        case ALLOC_TYP_RECALL:
            k_idx = lookup_kind(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
            object_prep(&forge, k_idx);
            object_origins(&forge, ORIGIN_FLOOR);
            /*obj_make_pile(&forge);*/
            drop_near(&forge, -1, y, x);
            break;

        case ALLOC_TYP_SKELETON:
            k_idx = lookup_kind(TV_CORPSE, SV_SKELETON);
            object_prep(&forge, k_idx);
            apply_magic(&forge, dun_level, 0);
            object_origins(&forge, ORIGIN_FLOOR);
            drop_near(&forge, -1, y, x);
            break;
        }
    }
}

static void _mon_give_extra_drop(u32b flag, int ct)
{
    int tot = 0;
    int i;

    ct = _adjust_small_level(ct);
    if (ct <= 0) return;

    for (i = 0; i < max_m_idx; i++)
    {
        monster_type *m_ptr = &m_list[i];

        if (!m_ptr->r_idx) continue;
        if (!m_ptr->drop_ct) continue;
        if (r_info[m_ptr->r_idx].flags1 & RF1_ONLY_GOLD) continue;

        tot++;
    }

    ct = MIN(ct, tot);

    for (i = 0; i < ct; i++)
    {
        int j = randint1(tot);
        int k;
        for (k = 0; k < max_m_idx; k++)
        {
            monster_type *m_ptr = &m_list[k];
            if (!m_ptr->r_idx) continue;
            if (!m_ptr->drop_ct) continue;
            if (r_info[m_ptr->r_idx].flags1 & RF1_ONLY_GOLD) continue;

            --j;
            if (j <= 0)
            {
                m_ptr->mflag2 |= flag;
                break;
            }
        }
    }
}

/*
 * Count the number of "corridor" grids adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y1, x1)"
 *
 * XXX XXX This routine currently only counts actual "empty floor"
 * grids which are not in rooms. We might want to also count stairs,
 * open doors, closed doors, etc.
 */
static int next_to_corr(int y1, int x1)
{
    int i, y, x, k = 0;

    cave_type *c_ptr;

    /* Scan adjacent grids */
    for (i = 0; i < 4; i++)
    {
        /* Extract the location */
        y = y1 + ddy_ddd[i];
        x = x1 + ddx_ddd[i];

        /* Access the grid */
        c_ptr = &cave[y][x];

        /* Skip non floors */
        if (cave_have_flag_grid(c_ptr, FF_WALL)) continue;

        /* Skip non "empty floor" grids */
        if (!is_floor_grid(c_ptr))
            continue;

        /* Skip grids inside rooms */
        if (c_ptr->info & (CAVE_ROOM)) continue;

        /* Count these grids */
        k++;
    }

    /* Return the number of corridors */
    return (k);
}

static bool door_is_sane(int y, int x)
{
    bool op_w = FALSE, op_n = FALSE;
    if (!in_bounds(y, x)) return FALSE;
    if (atlantis_hack) return TRUE;
    if (in_bounds(y - 1, x)) /* Check opening to north */
    {
        if ((is_floor_bold(y - 1, x)) || (is_floor_bold(y - 1, x - 1)) || (is_floor_bold(y - 1, x + 1))) op_n = TRUE;
    }
    if ((op_n) && (in_bounds(y + 1, x))) /* Check opening to south */
    {
        if ((is_floor_bold(y + 1, x)) || (is_floor_bold(y + 1, x - 1)) || (is_floor_bold(y + 1, x + 1)))
        {
            /* Require openness in a cardinal direction */
            if (is_floor_bold(y + 1, x) || is_floor_bold(y - 1, x))
            {
                /* Verify closedness in other direction */
                if ((!is_floor_bold(y, x - 1)) && (!is_floor_bold(y, x + 1))) return TRUE;

                /* Special case - allow diagonal doors */
                if ((is_floor_bold(y + 1, x + 1)) && (is_floor_bold(y - 1, x - 1)) &&
                   (!is_floor_bold(y - 1, x + 1)) && (!is_floor_bold(y + 1, x - 1)) &&
                   (((!is_floor_bold(y, x - 1)) && (!is_floor_bold(y - 1, x))) ||
                   ((!is_floor_bold(y, x + 1)) && (!is_floor_bold(y + 1, x))))) return TRUE;
                if ((is_floor_bold(y + 1, x - 1)) && (is_floor_bold(y - 1, x + 1)) &&
                   (!is_floor_bold(y - 1, x - 1)) && (!is_floor_bold(y + 1, x + 1)) &&
                   (((!is_floor_bold(y, x - 1)) && (!is_floor_bold(y + 1, x))) ||
                   ((!is_floor_bold(y, x + 1)) && (!is_floor_bold(y - 1, x))))) return TRUE;
            }
        }
    }
    if (in_bounds(y, x - 1)) /* Check opening to west */
    {
        if ((is_floor_bold(y, x - 1)) || (is_floor_bold(y - 1, x - 1)) || (is_floor_bold(y + 1, x - 1))) op_w = TRUE;
    }
    if ((op_w) && (in_bounds(y, x + 1))) /* Check opening to east */
    {
        if ((is_floor_bold(y, x + 1)) || (is_floor_bold(y + 1, x + 1)) || (is_floor_bold(y - 1, x + 1)))
        {
            /* Require openness in a cardinal direction */
            if (is_floor_bold(y, x - 1) || is_floor_bold(y, x + 1))
            {
                /* Verify closedness in other direction */
                if ((!is_floor_bold(y - 1, x)) && (!is_floor_bold(y + 1, x))) return TRUE;

                /* Special case - allow diagonal doors */
                if ((is_floor_bold(y + 1, x + 1)) && (is_floor_bold(y - 1, x - 1)) &&
                   (!is_floor_bold(y - 1, x + 1)) && (!is_floor_bold(y + 1, x - 1)) &&
                   (((!is_floor_bold(y, x - 1)) && (!is_floor_bold(y - 1, x))) ||
                   ((!is_floor_bold(y, x + 1)) && (!is_floor_bold(y + 1, x))))) return TRUE;
                if ((is_floor_bold(y + 1, x - 1)) && (is_floor_bold(y - 1, x + 1)) &&
                   (!is_floor_bold(y - 1, x - 1)) && (!is_floor_bold(y + 1, x + 1)) &&
                   (((!is_floor_bold(y, x - 1)) && (!is_floor_bold(y + 1, x))) ||
                   ((!is_floor_bold(y, x + 1)) && (!is_floor_bold(y - 1, x))))) return TRUE;
            }
        }
    }

    /* Require openings on opposite sides */
    return FALSE;
}


/*
 * Determine if the given location is "between" two walls,
 * and "next to" two corridor spaces. XXX XXX XXX
 *
 * Assumes "in_bounds(y, x)"
 */
static bool possible_doorway(int y, int x)
{
    /* Count the adjacent corridors */
    if (next_to_corr(y, x) >= 2)
    {
        /* Check Vertical */
        if (cave_have_flag_bold(y - 1, x, FF_WALL) &&
            cave_have_flag_bold(y + 1, x, FF_WALL))
        {
            return (TRUE);
        }

        /* Check Horizontal */
        if (cave_have_flag_bold(y, x - 1, FF_WALL) &&
            cave_have_flag_bold(y, x + 1, FF_WALL))
        {
            return (TRUE);
        }
    }

    /* No doorway */
    return (FALSE);
}


/*
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(int y, int x)
{
    /* Paranoia */
    if (!in_bounds(y, x)) return;

    /* Ignore walls */
    if (cave_have_flag_bold(y, x, FF_WALL)) return;

    /* Ignore room grids */
    if (cave[y][x].info & (CAVE_ROOM)) return;

    /* Occasional door (if allowed) */
    if ((randint0(100) < dun_tun_jct) && possible_doorway(y, x) && door_is_sane(y, x) && !(d_info[dungeon_type].flags1 & DF1_NO_DOORS))
    {
//        msg_format("Placing door at %d/%d", y, x);
        /* Place a door */
        place_random_door(y, x, FALSE);
    }
}


/*
 * Set boundary mimic and add "solid" perma-wall
 */
static void set_bound_perm_wall(cave_type *c_ptr)
{
    if (bound_walls_perm)
    {
        /* Clear boundary mimic */
        c_ptr->mimic = 0;
    }
    else
    {
        feature_type *f_ptr = &f_info[c_ptr->feat];

        /* Hack -- Decline boundary walls with known treasure  */
        if ((have_flag(f_ptr->flags, FF_HAS_GOLD) || have_flag(f_ptr->flags, FF_HAS_ITEM)) &&
            !have_flag(f_ptr->flags, FF_SECRET))
            c_ptr->feat = feat_state(c_ptr->feat, FF_ENSECRET);

        /* Set boundary mimic */
        c_ptr->mimic = c_ptr->feat;
    }

    /* Add "solid" perma-wall */
    place_solid_perm_grid(c_ptr);
}


/*
 * Generate various caverns and lakes
 *
 * There were moved from cave_gen().
 */
static void gen_caverns_and_lakes(void)
{
    /* Possible "destroyed" level */
    if ((dun_level > 30) && one_in_(DUN_DEST*2) && (d_info[dungeon_type].flags1 & DF1_DESTROY))
    {
        dun->destroyed = TRUE;

        /* extra rubble around the place looks cool */
        build_lake(one_in_(2) ? LAKE_T_CAVE : LAKE_T_EARTH_VAULT);
    }

    /* Make a lake some of the time */
    if (one_in_(LAKE_LEVEL) && !dun->empty_level && !dun->destroyed &&
        (d_info[dungeon_type].flags1 & DF1_LAKE_MASK))
    {
        int count = 0;
        if (d_info[dungeon_type].flags1 & DF1_LAKE_WATER) count += 3;
        if (d_info[dungeon_type].flags1 & DF1_LAKE_LAVA) count += 3;
        if (d_info[dungeon_type].flags1 & DF1_LAKE_RUBBLE) count += 3;
        if (d_info[dungeon_type].flags1 & DF1_LAKE_TREE) count += 3;
        if (d_info[dungeon_type].flags1 & DF1_LAKE_NUKE) count += 3;

        if (d_info[dungeon_type].flags1 & DF1_LAKE_LAVA)
        {
            /* Lake of Lava */
            if ((dun_level > 80) && (randint0(count) < 2)) dun->laketype = LAKE_T_LAVA;
            count -= 2;

            /* Lake of Lava2 */
            if (!dun->laketype && (dun_level > 80) && one_in_(count)) dun->laketype = LAKE_T_FIRE_VAULT;
            count--;
        }

        if ((d_info[dungeon_type].flags1 & DF1_LAKE_WATER) && !dun->laketype)
        {
            /* Lake of Water */
            if ((dun_level > 50) && randint0(count) < 2) dun->laketype = LAKE_T_WATER;
            count -= 2;

            /* Lake of Water2 */
            if (!dun->laketype && (dun_level > 50) && one_in_(count)) dun->laketype = LAKE_T_WATER_VAULT;
            count--;
        }

        if ((d_info[dungeon_type].flags1 & DF1_LAKE_RUBBLE) && !dun->laketype)
        {
            /* Lake of rubble */
            if ((dun_level > 35) && (randint0(count) < 2)) dun->laketype = LAKE_T_CAVE;
            count -= 2;

            /* Lake of rubble2 */
            if (!dun->laketype && (dun_level > 35) && one_in_(count)) dun->laketype = LAKE_T_EARTH_VAULT;
            count--;
        }

        /* Lake of nukage */
        if ((dun_level > 5) && (d_info[dungeon_type].flags1 & DF1_LAKE_NUKE) && !dun->laketype) dun->laketype = LAKE_T_NUKE_VAULT;

        /* Lake of tree */
        if ((dun_level > 5) && (d_info[dungeon_type].flags1 & DF1_LAKE_TREE) && !dun->laketype) dun->laketype = LAKE_T_AIR_VAULT;

        if (dun->laketype)
        {
            if (cheat_room)
                msg_print("Lake on the level.");

            build_lake(dun->laketype);
        }
    }

    if ((dun_level > DUN_CAVERN) && !dun->empty_level &&
        (d_info[dungeon_type].flags1 & DF1_CAVERN) &&
        !dun->laketype && !dun->destroyed && (randint1(1000) < dun_level))
    {
        dun->cavern = TRUE;

        /* make a large fractal cave in the middle of the dungeon */

        if (cheat_room)
            msg_print("Cavern on level.");

        build_cavern();
    }

    /* Hack -- No destroyed "quest" levels */
    if (quests_get_current()) dun->destroyed = FALSE;
}

/*
 * Generate a new dungeon level
 *
 * Note that "dun_body" adds about 4000 bytes of memory to the stack.
 */
static int _adjust_small_level(int n)
{
    if (cur_hgt < MAX_HGT || cur_wid < MAX_WID)
    {
        int div = MAX_HGT * MAX_WID;
        int amt = n * cur_hgt * cur_wid * 10 / div;

        n = amt/10;
        if (randint0(10) < amt%10)
            n++;
    }
    return n;
}
static int _depth_amt(void)
{
    return MAX(2, MIN(10, dun_level/3));
}
/* static bool _oz_swimmer(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    if ((!r_ptr) || (!r_ptr->name)) return FALSE;
    if (r_ptr->dungeon != DUNGEON_AUSSIE) return FALSE;
    if ((r_ptr->flags7 & (RF7_AQUATIC | RF7_CAN_SWIM))) return TRUE;
    return FALSE;
} */
static void _cave_gen_monsters(void)
{
    if (dungeon_type != DUNGEON_ARENA)
    {
        int ct, i;

        /* Note: These are extra monsters, filling out NORMAL and FRACAVE rooms.
         * Most TEMPLATE rooms and all VAULT rooms will generate their own monsters
         * during room generation. */
        ct = d_info[dungeon_type].min_m_alloc_level; /* usually 14 */
        if ((increase_density) && ((cur_hgt < MAX_HGT) || (cur_wid < MAX_WID)))
        {
            int small_tester = ct;
            ct = (ct * cur_hgt) / MAX_HGT;
            ct = (ct * cur_wid) / MAX_WID;
            ct += 1;
            if (ct > small_tester) ct = small_tester;
            ct += _depth_amt();
            ct += randint1(8);
            if (dun_level >= 50) ct = (ct * 3 / 5);
        }
        else
        {
            ct += randint1(_depth_amt());
            ct = _adjust_small_level(ct);
            if (ct < 4)
                ct = 4;
        }

        for (i = 0; i < ct; i++)
            alloc_monster(0, PM_ALLOW_SLEEP);
    }
}
static void _cave_gen_traps(void)
{
    int ct = _depth_amt();
    /* XXX alloc_object will _adjust_small_level() for us ... */
    alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint1(ct));
    if (!(d_info[dungeon_type].flags1 & DF1_NO_CAVE))
        alloc_object(ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint1(ct));
}
static void _cave_gen_objects(void)
{
    if (dungeon_type != DUNGEON_ARENA)
    {
        /* Put some objects in rooms */
        alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ROOM, 3));

        /* Put some objects/gold in the dungeon */
        alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ITEM, 3));
        alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_GOLD, randnor(DUN_AMT_GOLD, 3));

        /* Experimental: Guarantee certain objects. Give surprise goodies. */
        if (one_in_(2))
            alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_FOOD, 1);
        if (dun_level <= 15 && one_in_(2))
            alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_LIGHT, 1);
        if (dun_level >= 10  && dun_level < 50 && one_in_(2))
            alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_RECALL, 1);
        if (dun_level >= 10 && one_in_(20))
            alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_SKELETON, damroll(3, 5));

        _mon_give_extra_drop(MFLAG2_DROP_BASIC, 1);
        _mon_give_extra_drop(MFLAG2_DROP_UTILITY, randint0(4));
        if (dun_level > max_dlv[dungeon_type])
            _mon_give_extra_drop(MFLAG2_DROP_PRIZE, 1);
    }

    object_level = base_level;
}
static bool cave_gen(void)
{
    int i, y, x;

    dun_data dun_body;

    /* Global data */
    dun = &dun_body;

    dun->destroyed = FALSE;
    dun->empty_level = FALSE;
    dun->cavern = FALSE;
    dun->laketype = 0;

    /* Fill the arrays of floors and walls in the good proportions */
    set_floor_and_wall(dungeon_type);

    /* Prepare allocation table */
    get_mon_num_prep(get_monster_hook(), NULL);

    /* Randomize the dungeon creation values */
    dun_tun_rnd = rand_range(DUN_TUN_RND_MIN, DUN_TUN_RND_MAX);
    dun_tun_chg = rand_range(DUN_TUN_CHG_MIN, DUN_TUN_CHG_MAX);
    dun_tun_con = rand_range(DUN_TUN_CON_MIN, DUN_TUN_CON_MAX);
    dun_tun_pen = rand_range(DUN_TUN_PEN_MIN, DUN_TUN_PEN_MAX);
    dun_tun_jct = rand_range(DUN_TUN_JCT_MIN, DUN_TUN_JCT_MAX);

    /* Actual maximum number of rooms on this level */
    dun->row_rooms = cur_hgt / BLOCK_HGT;
    dun->col_rooms = cur_wid / BLOCK_WID;

    /* Initialize the room table */
    for (y = 0; y < dun->row_rooms; y++)
    {
        for (x = 0; x < dun->col_rooms; x++)
        {
            dun->room_map[y][x] = FALSE;
        }
    }

    /* No rooms yet */
    dun->cent_n = 0;

    /* Empty arena levels */
    if ((generate_empty == EMPTY_ALWAYS) ||
        ((d_info[dungeon_type].flags1 & DF1_ARENA) &&
         (generate_empty == EMPTY_SOMETIMES) && (one_in_(EMPTY_LEVEL))))
    {
        dun->empty_level = TRUE;
        if (cheat_room)
            msg_print("Arena level.");
    }

    if (dun->empty_level)
    {
        /* Start with floors */
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                place_floor_bold(y, x);
            }
        }

        /* Special boundary walls -- Top and bottom */
        for (x = 0; x < cur_wid; x++)
        {
            place_extra_bold(0, x);
            place_extra_bold(cur_hgt - 1, x);
        }

        /* Special boundary walls -- Left and right */
        for (y = 1; y < (cur_hgt - 1); y++)
        {
            place_extra_bold(y, 0);
            place_extra_bold(y, cur_wid - 1);
        }
    }
    else if (atlantis_hack) /* Semi-open dungeon */
    {
        int yy, xx;
        int ysize = 22, xsize = 22, ylim, xlim;
        bool tayta[9][6] = {0};
        bool perma = one_in_(50) ? TRUE : FALSE;
        byte rounded_corner_prob = (!one_in_(3)) ? ((one_in_(2)) ? 50 : 100) : 0;

        if ((cur_hgt < 50) && (one_in_(2))) ysize = 11;
        if (cur_wid > 130)
        {
            xsize = 33;
            if ((cur_wid > 190) && (one_in_(2))) xsize = 22;
        }
        ylim = cur_hgt / ysize;
        xlim = cur_wid / xsize;
        for (yy = 0; yy < ylim; yy++)
        {
            for (xx = 0; xx < xlim; xx++)
            {
                tayta[xx][yy] = one_in_(3);
            }
        }

        /* Start with floors */
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                place_floor_bold(y, x);
            }
        }

        /* Fill in walls region by region */
        for (yy = 0; yy < ylim; yy++)
        {
            for (xx = 0; xx < xlim; xx++)
            {
                if (!tayta[xx][yy]) continue;
                else
                {
                    bool do_gold = ((!perma) && (one_in_(36)));
                    bool round_c[2][2] = {0};
                    int kx, ky, cx, cy;
                    if (rounded_corner_prob)
                    {
                        ky = (yy * ysize) + (ysize / 2);
                        kx = (xx * xsize) + (xsize / 2);
                        round_c[0][0] = ((yy) && (xx) && (!tayta[xx][yy-1]) && (!tayta[xx-1][yy]) && (randint0(100) < rounded_corner_prob));
                        round_c[0][1] = ((yy < ylim - 1) && (xx) && (!tayta[xx][yy+1]) && (!tayta[xx-1][yy]) && (randint0(100) < rounded_corner_prob));
                        round_c[1][0] = ((yy) && (xx < xlim - 1) && (!tayta[xx][yy-1]) && (!tayta[xx+1][yy]) && (randint0(100) < rounded_corner_prob));
                        round_c[1][1] = ((yy < ylim - 1) && (xx < xlim - 1) && (!tayta[xx][yy+1]) && (!tayta[xx+1][yy]) && (randint0(100) < rounded_corner_prob));
                    }
/*                    msg_format("Walled area at %d/%d - Rounding corners: %s, %s, %s, %s - Extent of area: %d/%d to %d/%d", xx, yy,
                       round_c[0][0] ? "Yes" : "No",
                       round_c[1][0] ? "Yes" : "No",
                       round_c[0][1] ? "Yes" : "No",
                       round_c[1][1] ? "Yes" : "No",
                       xx * xsize, yy * ysize, (xx + 1) * xsize - 1, (yy + 1) * ysize - 1);*/
                    for (y = yy * ysize; y < (yy + 1) * ysize; y++)
                    {
                        for (x = xx * xsize; x < (xx + 1) * xsize; x++)
                        {
                            if (rounded_corner_prob)
                            {
                                cy = (y > ky) ? 1 : 0;
                                cx = (x > kx) ? 1 : 0;
                                if (round_c[cx][cy])
                                {
                                    int ry = cy ? (yy + 1) * ysize - 1 : yy * ysize;
                                    int rx = cx ? (xx + 1) * xsize - 1 : xx * xsize;
                                    int yxsize = MIN(ysize, xsize) / 2;
                                    int ky2 = cy ? ry - yxsize : ry + yxsize;
                                    int kx2 = cx ? rx - yxsize : rx + yxsize;
                                    int mdist = distance(ry, kx2, ky2, kx2);
                                    int kx3 = cx ? MIN(kx2, x) : MAX(kx2, x);
                                    int ky3 = cy ? MIN(ky2, y) : MAX(ky2, y);
                                    mdist += (mdist / 7);
                                    if (distance(y, x, ky3, kx3) > mdist)
                                    {
//                                        msg_format("Avoiding wall at %d/%d - exceeds Rng %d from %d/%d", x, y, mdist, kx2, ky2);
                                        continue;
                                    }
                                }
                            }
                            if (perma)
                            {
                                place_extra_perm_bold(y, x);
                                cave[y][x].mimic = 0;
                            }
                            else if (do_gold)
                            {
                                int _feat = (one_in_(4)) ? feat_magma_vein : feat_quartz_vein;
                                place_extra_bold(y, x);
                                set_cave_feat(y, x, _feat);
                                if (!one_in_(3)) cave_alter_feat(y, x, FF_MAY_HAVE_GOLD);
                            }
                            else
                            {
                                int prob = randint0(100);
                                int _feat = feat_granite;
                                if (prob < 5) _feat = feat_magma_vein;
                                else if (prob < 20) _feat = feat_quartz_vein;
                                place_extra_bold(y, x);
                                set_cave_feat(y, x, _feat);
                            }
                        }
                    }
                }
            }
        }        
    }
    else
    {
        /* Start with walls */
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                place_extra_bold(y, x);
            }
        }
    }


    /* Generate various caverns and lakes */
    gen_caverns_and_lakes();


    /* Build maze */
    if (d_info[dungeon_type].flags1 & DF1_MAZE)
    {
        build_maze_vault(cur_wid/2-1, cur_hgt/2-1, cur_wid-4, cur_hgt-4, FALSE);

        /* Place 3 or 4 down stairs near some walls */
        if (!alloc_stairs(feat_down_stair, rand_range(4, 5), 3)) return FALSE;

        /* Place 1 or 2 up stairs near some walls */
        if (!alloc_stairs(feat_up_stair, 1, 2)) return FALSE;
    }

    /* Build some rooms */
    else
    {
        int tunnel_fail_count = 0;

        /*
         * Build each type of room in turn until we cannot build any more.
         */
        if (!generate_rooms()) return FALSE;


        /* Make a hole in the dungeon roof sometimes at level 1
           But not in Angband. See Issue #3 */
        if (dun_level == 1 && dungeon_type != DUNGEON_ANGBAND && !wacky_rooms)
        {
            while (one_in_(DUN_MOS_DEN))
            {
                place_trees(randint1(cur_wid - 2), randint1(cur_hgt - 2));
            }
        }

        /* Destroy the level if necessary */
        if (dun->destroyed) destroy_level();

        /* Hack -- Add some rivers */
        if (one_in_((dungeon_type == DUNGEON_AUSSIE) ? 3 : 7) && (randint1(dun_level) > 5))
        {
            int feat1 = 0, feat2 = 0;

            /* Choose water or lava */
            if ( randint1(MAX_DEPTH * 2) - 1 > dun_level
              && ( (d_info[dungeon_type].flags1 & DF1_WATER_RIVER)
                || (no_wilderness && one_in_(3)) ) )
            {
                feat1 = feat_deep_water;
                feat2 = feat_shallow_water;
            }
            else if (d_info[dungeon_type].flags1 & DF1_LAVA_RIVER)
            {
                feat1 = feat_deep_lava;
                feat2 = feat_shallow_lava;
            }
            else if (d_info[dungeon_type].flags1 & DF1_NUKE_RIVER)
            {
                feat1 = feat_deep_waste;
                feat2 = feat_shallow_waste;
            }
            else feat1 = 0;

            if (feat1)
            {
                feature_type *f_ptr = &f_info[feat1];

                /* Only add river if matches lake type or if have no lake at all */
                if (((dun->laketype == LAKE_T_LAVA) && have_flag(f_ptr->flags, FF_LAVA)) ||
                    ((dun->laketype == LAKE_T_WATER) && have_flag(f_ptr->flags, FF_WATER)) ||
                     !dun->laketype)
                {
                    add_river(feat1, feat2);
                    if (dungeon_type == DUNGEON_AUSSIE) /* Ensure some aquatic life */
                    {
                        int mon_ct0 = damroll(2, 3) - randint0(2);
                        for (i = 0; i < mon_ct0; i++)
                        {
                            alloc_monster(2, (PM_ALLOW_SLEEP | PM_FORCE_AQUATIC));
                        }
                    }
                }
            }
        }

        /* Hack -- Scramble the room order */
        for (i = 0; i < dun->cent_n; i++)
        {
            int ty, tx;
            int pick = rand_range(0, i);

            ty = dun->cent[i].y;
            tx = dun->cent[i].x;
            dun->cent[i].y = dun->cent[pick].y;
            dun->cent[i].x = dun->cent[pick].x;
            dun->cent[pick].y = ty;
            dun->cent[pick].x = tx;
        }

        /* Start with no tunnel doors */
        dun->door_n = 0;

        /* Hack -- connect the first room to the last room */
        y = dun->cent[dun->cent_n-1].y;
        x = dun->cent[dun->cent_n-1].x;

        /* Connect all the rooms together */
        for (i = 0; i < dun->cent_n; i++)
        {
            int j;

            /* Reset the arrays */
            dun->tunn_n = 0;
            dun->wall_n = 0;

            /* Connect the room to the previous room */
            if (randint1(dun_level) > d_info[dungeon_type].tunnel_percent)
            {
                /* make cave-like tunnel */
                (void)build_tunnel2(dun->cent[i].x, dun->cent[i].y, x, y, 2, 2);
            }
            else
            {
                /* make normal tunnel */
                if (!build_tunnel(dun->cent[i].y, dun->cent[i].x, y, x)) tunnel_fail_count++;
            }

            if (tunnel_fail_count >= 2) return FALSE;

            /* Turn the tunnel into corridor */
            for (j = 0; j < dun->tunn_n; j++)
            {
                cave_type *c_ptr;
                feature_type *f_ptr;

                /* Access the grid */
                y = dun->tunn[j].y;
                x = dun->tunn[j].x;

                /* Access the grid */
                c_ptr = &cave[y][x];
                f_ptr = &f_info[c_ptr->feat];

                /* Clear previous contents (if not a lake), add a floor */
                if (!have_flag(f_ptr->flags, FF_MOVE) || (!have_flag(f_ptr->flags, FF_WATER) && !have_flag(f_ptr->flags, FF_LAVA)))
                {
                    /* Clear mimic type */
                    c_ptr->mimic = 0;

                    place_floor_grid(c_ptr);
                }
            }

            /* Apply the piercings that we found */
            for (j = 0; j < dun->wall_n; j++)
            {
                cave_type *c_ptr;

                /* Access the grid */
                y = dun->wall[j].y;
                x = dun->wall[j].x;

                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Clear mimic type */
                c_ptr->mimic = 0;

                /* Clear previous contents, add up floor */
                place_floor_grid(c_ptr);

                /* Occasional doorway */
                if ((randint0(100) < dun_tun_pen) && !(d_info[dungeon_type].flags1 & DF1_NO_DOORS))
                {
                    if ((next_to_walls(y, x, FALSE) < 2) || (!door_is_sane(y, x)))
                    { /* Reject weird door location */
//                        msg_format("Rejecting door at %d/%d", y, x);
                    }
                    /* Place a random door */
                    else
                    {
//                        msg_format("Accepting door at %d/%d", y, x);
                        place_random_door(y, x, TRUE);
                    }
                }
            }

            /* Remember the "previous" room */
            y = dun->cent[i].y;
            x = dun->cent[i].x;
        }

        /* Place intersection doors */
        for (i = 0; i < dun->door_n; i++)
        {
            /* Extract junction location */
            y = dun->door[i].y;
            x = dun->door[i].x;

            /* Try placing doors */
            try_door(y, x - 1);
            try_door(y, x + 1);
            try_door(y - 1, x);
            try_door(y + 1, x);
        }

        if (!alloc_stairs(feat_down_stair, rand_range(4, 5), 3)) return FALSE;

        /* Place 1 or 2 up stairs near some walls */
        if (!alloc_stairs(feat_up_stair, rand_range(1, 2), 3)) return FALSE;
    }

    if (!dun->laketype)
    {
        if (d_info[dungeon_type].stream2)
        {
            /* Hack -- Add some quartz streamers */
            for (i = 0; i < DUN_STR_QUA; i++)
            {
                build_streamer(d_info[dungeon_type].stream2, DUN_STR_QC);
            }
        }

        if (d_info[dungeon_type].stream1)
        {
            /* Hack -- Add some magma streamers */
            for (i = 0; i < DUN_STR_MAG; i++)
            {
                build_streamer(d_info[dungeon_type].stream1, DUN_STR_MC);
            }
        }
    }

    /* Special boundary walls -- Top and bottom */
    for (x = 0; x < cur_wid; x++)
    {
        set_bound_perm_wall(&cave[0][x]);
        set_bound_perm_wall(&cave[cur_hgt - 1][x]);
    }

    /* Special boundary walls -- Left and right */
    for (y = 1; y < (cur_hgt - 1); y++)
    {
        set_bound_perm_wall(&cave[y][0]);
        set_bound_perm_wall(&cave[y][cur_wid - 1]);
    }

    /* Determine the character location */
    if (!new_player_spot()) return FALSE;

    _cave_gen_monsters();
    _cave_gen_traps();

    _cave_gen_objects();

    /* Put the Guardian */
    if (!alloc_guardian(TRUE)) return FALSE;

    if (dun->empty_level && (!one_in_(DARK_EMPTY) || (randint1(100) > dun_level)) && !(d_info[dungeon_type].flags1 & DF1_DARKNESS))
    {
        /* Lite the cave */
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                cave[y][x].info |= (CAVE_GLOW);
            }
        }
    }

    return TRUE;
}


/*
 * Builds the arena after it is entered -KMW-
 */
static void build_arena(void)
{
    int yval, y_height, y_depth, xval, x_left, x_right;
    register int i, j;

    yval = SCREEN_HGT / 2;
    xval = SCREEN_WID / 2;
    y_height = yval - 10;
    y_depth = yval + 10;
    x_left = xval - 32;
    x_right = xval + 32;

    for (i = y_height; i <= y_height + 5; i++)
        for (j = x_left; j <= x_right; j++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
        }
    for (i = y_depth; i >= y_depth - 5; i--)
        for (j = x_left; j <= x_right; j++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
        }
    for (j = x_left; j <= x_left + 17; j++)
        for (i = y_height; i <= y_depth; i++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
        }
    for (j = x_right; j >= x_right - 17; j--)
        for (i = y_height; i <= y_depth; i++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
        }

    place_extra_perm_bold(y_height+6, x_left+18);
    cave[y_height+6][x_left+18].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
    place_extra_perm_bold(y_depth-6, x_left+18);
    cave[y_depth-6][x_left+18].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
    place_extra_perm_bold(y_height+6, x_right-18);
    cave[y_height+6][x_right-18].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
    place_extra_perm_bold(y_depth-6, x_right-18);
    cave[y_depth-6][x_right-18].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);

    i = y_height + 5;
    j = xval;
    cave[i][j].feat = f_tag_to_index("ARENA_GATE");
    cave[i][j].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
    player_place(i, j);
}


/*
 * Town logic flow for generation of arena -KMW-
 */
static void arena_gen(void)
{
    int y, x;
    int qy = 0;
    int qx = 0;

    /* Smallest area */
    cur_hgt = SCREEN_HGT;
    cur_wid = SCREEN_WID;

    /* Start with solid walls */
    for (y = 0; y < MAX_HGT; y++)
    {
        for (x = 0; x < MAX_WID; x++)
        {
            /* Create "solid" perma-wall */
            place_solid_perm_bold(y, x);

            /* Illuminate and memorize the walls */
            cave[y][x].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
        }
    }

    /* Then place some floors */
    for (y = qy + 1; y < qy + SCREEN_HGT - 1; y++)
    {
        for (x = qx + 1; x < qx + SCREEN_WID - 1; x++)
        {
            /* Create empty floor */
            cave[y][x].feat = feat_floor;
        }
    }

    build_arena();

    place_monster_aux(0, py + 5, px, arena_info[p_ptr->arena_number].r_idx,
        (PM_NO_KAGE | PM_NO_PET));
}



/*
 * Builds the arena after it is entered -KMW-
 */
static void build_battle(void)
{
    int yval, y_height, y_depth, xval, x_left, x_right;
    register int i, j;

    yval = SCREEN_HGT / 2;
    xval = SCREEN_WID / 2;
    y_height = yval - 10;
    y_depth = yval + 10;
    x_left = xval - 32;
    x_right = xval + 32;

    /* Hack: Move the arena down 5 lines so that messages do not
       obstruct the battle. Couldn't this be read from a file just
       like vaults/rooms in v_info?*/
    y_height += 5;
    y_depth += 5;
    for (i = 0; i < y_height; i++)
        for (j = x_left; j <= x_right; j++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
        }


    for (i = y_height; i <= y_height + 5; i++)
        for (j = x_left; j <= x_right; j++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
        }
    for (i = y_depth; i >= y_depth - 3; i--)
        for (j = x_left; j <= x_right; j++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
        }
    for (j = x_left; j <= x_left + 17; j++)
        for (i = y_height; i <= y_depth; i++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
        }
    for (j = x_right; j >= x_right - 17; j--)
        for (i = y_height; i <= y_depth; i++)
        {
            place_extra_perm_bold(i, j);
            cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
        }

    place_extra_perm_bold(y_height+6, x_left+18);
    cave[y_height+6][x_left+18].info |= (CAVE_GLOW | CAVE_MARK);
    place_extra_perm_bold(y_depth-4, x_left+18);
    cave[y_depth-4][x_left+18].info |= (CAVE_GLOW | CAVE_MARK);
    place_extra_perm_bold(y_height+6, x_right-18);
    cave[y_height+6][x_right-18].info |= (CAVE_GLOW | CAVE_MARK);
    place_extra_perm_bold(y_depth-4, x_right-18);
    cave[y_depth-4][x_right-18].info |= (CAVE_GLOW | CAVE_MARK);

    for (i = y_height + 1; i <= y_height + 5; i++)
        for (j = x_left + 20 + 2 * (y_height + 5 - i); j <= x_right - 20 - 2 * (y_height + 5 - i); j++)
        {
            cave[i][j].feat = feat_permanent_glass_wall;
        }

    i = y_height + 1;
    j = xval;
    cave[i][j].feat = f_tag_to_index("BUILDING_3");
    cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
    player_place(i, j);
}


/*
 * Town logic flow for generation of arena -KMW-
 */
static void battle_gen(void)
{
    int y, x, i;
    int qy = 0;
    int qx = 0;

    /* Start with solid walls */
    for (y = 0; y < MAX_HGT; y++)
    {
        for (x = 0; x < MAX_WID; x++)
        {
            /* Create "solid" perma-wall */
            place_solid_perm_bold(y, x);

            /* Illuminate and memorize the walls */
            cave[y][x].info |= (CAVE_GLOW | CAVE_MARK);
        }
    }

    /* Then place some floors */
    for (y = qy + 1; y < qy + SCREEN_HGT - 1; y++)
    {
        for (x = qx + 1; x < qx + SCREEN_WID - 1; x++)
        {
            /* Create empty floor */
            cave[y][x].feat = feat_floor;
        }
    }

    build_battle();

    for(i=0;i<4;i++)
    {
        place_monster_aux(0, py + 8 + (i/2)*4, px - 2 + (i%2)*4, battle_mon[i],
                  (PM_NO_KAGE | PM_NO_PET));
        set_friendly(&m_list[cave[py+8+(i/2)*4][px-2+(i%2)*4].m_idx]);
    }
    for(i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        if (!m_ptr->r_idx) continue;

        /* Hack -- Detect monster */
        m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

        /* Update the monster */
        update_mon(i, FALSE);
    }
}

/* Make a real level */
static bool level_gen(cptr *why)
{
    bool small = FALSE;
    bool coffee = FALSE;

    /* XXX Roll for a small level. Users should not be able to control
     * the level size at runtime. For example, I personally used to force
     * levels small to fight Oberon since this made him much easier to deal
     * with! Small levels also give an inordinate advantage against thieving
     * monsters (like most Amberites and Bull Gates). They are no longer harder
     * than normal sized levels since all aspects of dungeon generation scale
     * with dungeon size (including monster density). */
    if ((d_info[dungeon_type].flags1 & DF1_BIG) && ((dungeon_type == DUNGEON_ARENA) || (!no_big_dungeons)))
        small = FALSE;
    else if (d_info[dungeon_type].flags1 & DF1_SMALLEST)
        small = TRUE;
    else if (one_in_(SMALL_LEVEL))
        small = TRUE;
    if ((dungeon_type == DUNGEON_ICKY) && (dun_level == 10)) small = TRUE;
    if ((dungeon_type == DUNGEON_SNOW) && (level_is_questlike(DUNGEON_SNOW, dun_level))) small = FALSE;
    if (((coffee_break) || ((
        ((small_level_type == SMALL_LVL_DUNG_COFFEE) && (dungeon_type != DUNGEON_ARENA))
        || (d_info[dungeon_type].flags1 & DF1_COFFEE)) && (!ironman_downward))))
    {
        coffee = TRUE;
        if ((dun_level < 99) && (randint0(484) < 484 - (MIN(60, dun_level) * 2))) small = TRUE;
    }
    if ((dungeon_type == DUNGEON_MOUND) && (randint0(125) < (484 - (dun_level * 5)))) small = TRUE;
    if ((small_level_type > SMALL_LVL_DUNG_COFFEE) && (dungeon_type != DUNGEON_ARENA))
    {
        small = TRUE;
        if (small_level_type == SMALL_LVL_COFFEE) coffee = TRUE;
    }
    if (small_level_type == SMALL_LVL_HUGE) small = FALSE;

    if (small)
    {
        int hgt, wid;
        bool big = (d_info[dungeon_type].flags1 & DF1_BIG) ? TRUE : FALSE;

        if (d_info[dungeon_type].flags1 & DF1_SMALLEST) /* Labyrinth and Mine */
        {
            hgt = 1;
            wid = 1;
        }
        else if ((dungeon_type == DUNGEON_ICKY) && (dun_level == 10)) 
        { 
            hgt = randint1(2); 
            wid = 1; 
        }
        else if ((big) && (!no_big_dungeons))
        {
            hgt = 3;
            wid = 3;
        }
        else if (small_level_type > SMALL_LVL_COFFEE)
        {
            switch (small_level_type)
            {
                case SMALL_LVL_HUGE:
                case SMALL_LVL_RESPECTFUL_HUGE:
                    hgt = 3;
                    wid = 3;
                    break;
                case SMALL_LVL_TINY:
                    hgt = 1;
                    wid = 1;
                    break;
                case SMALL_LVL_EXTREMELY_SMALL:
                {
                    for (;;)
                    {
                        hgt = randint1(2);
                        wid = randint1(2);
                        if (big)
                        {
                            hgt = 2;
                            wid = 1;
                        }
                        if ((hgt * wid) > 2) continue;
                        break;
                    }
                    break;
                }
                case SMALL_LVL_VERY_SMALL:
                {
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > 4) continue;
                        if (((hgt * wid) == 1) && ((one_in_(2)) || (big))) continue;
                        if (((hgt * wid) == 4) && (!big) && (!one_in_(6))) continue;
                        if (((hgt * wid) == 3) && (!big) && (!one_in_(4))) continue;
                        break;
                    }
                    break;
                }
                case SMALL_LVL_BABY_COFFEE:
                {
                    int sattuma = randint1(128);
                    int minimi = MIN(3, (sattuma + dun_level) / 75);
                    int maksimi = MAX(minimi, MIN(3, (sattuma + (dun_level * 3)) / 50)) + 1;
                    if (big) minimi = MIN(3, minimi + 1);
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > maksimi) continue;
                        if ((hgt * wid) < minimi) continue;
                        break;
                    }
                    break;
                }
                case SMALL_LVL_SMALL:
                {
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > (big ? 6 : 4)) continue;
                        if ((big) && ((hgt * wid) < 4)) continue;
                        break;
                    }
                    break;
                }
                case SMALL_LVL_INSTANT_COFFEE:
                {
                    int koko = isompi(0, pienempi(dun_level + randint0(6) + (big ? 14 : 0), 90) / 10);
                    static int minimit[10] =  {1, 1, 2, 2, 2, 3, 4, 4, 6, 6};
                    static int maksimit[10] = {1, 2, 2, 3, 4, 4, 4, 6, 6, 6};
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > maksimit[koko]) continue;
                        if ((hgt * wid) < minimit[koko]) continue;
                        break;
                    }
                    break;
                }
                case SMALL_LVL_INSTANT_COFFEE_BIG:
                {
                    int koko = isompi(0, pienempi(dun_level + randint0(6) + (big ? 14 : 0), 100) / 10);
                    static int minimit_big[11] =  {1, 1, 2, 2, 3, 4, 4, 4, 6, 6, 9};
                    static int maksimit_big[11] = {1, 2, 2, 3, 4, 4, 4, 6, 6, 9, 9};
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > maksimit_big[koko]) continue;
                        if ((hgt * wid) < minimit_big[koko]) continue;
                        break;
                    }
                    break;
                }
                default: /* medium */
                {
                    for (;;)
                    {
                        hgt = randint1(3);
                        wid = randint1(3);
                        if ((hgt * wid) > 6) continue;
                        if ((hgt * wid) < (big ? 4 : 2)) continue;
                        break;
                    }
                    break;
                }
            }
        }
        else
        {
            int max_hgt = MAX_HGT/SCREEN_HGT;
            int max_wid = MAX_WID/SCREEN_WID;

            for (;;)
            {
                int sattuma = randint1(35);
                hgt = randint1(max_hgt);
                wid = randint1(max_wid);
                if (hgt == max_hgt && wid == max_wid) continue;
                /* exclude 1x1, 1x2 and 2x1 */
                if (hgt * wid <= (coffee ? (MIN(4, (dun_level + 10 + sattuma) / 25)) : 2)) continue;
                break;
            }
        }

        if (((even_proportions) || (dungeon_type == DUNGEON_AUSSIE)) && (hgt == 1) && (wid != 1) && (!one_in_(wid)))
        {
            hgt = wid;
            wid = 1;
        }

        if ((wacky_rooms) && (hgt == 1) && (wid == 1) && (!(d_info[dungeon_type].flags1 & DF1_CHAMELEON)))
        {
            hgt = 2;
            if ((one_in_(3)) && (small_level_type != SMALL_LVL_TINY) && (small_level_type != SMALL_LVL_EXTREMELY_SMALL)) wid = 2;
        }

        cur_hgt = hgt * SCREEN_HGT;
        cur_wid = wid * SCREEN_WID;

        if (cheat_room)
          msg_format("<color:B>A 'small' dungeon level: (<color:R>%d,%d</color>).</color>", cur_wid, cur_hgt);
    }
    else
    {
        /* Big dungeon */
        cur_hgt = MAX_HGT;
        cur_wid = MAX_WID;
    }

    /* Make a dungeon */
    if (!cave_gen())
    {
        *why = "could not place player";
        return FALSE;
    }
    return TRUE;
}


/*
 * Wipe all unnecessary flags after cave generation
 */
void wipe_generate_cave_flags(void)
{
    int x, y;

    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            /* Wipe unused flags */
            cave[y][x].info &= ~(CAVE_MASK);
        }
    }

    if (dun_level)
    {
        for (y = 1; y < cur_hgt - 1; y++)
        {
            for (x = 1; x < cur_wid - 1; x++)
            {
                /* There might be trap */
                cave[y][x].info |= CAVE_UNSAFE;
            }
        }
    }
}


/*
 *  Clear and empty the cave
 */
void clear_cave(void)
{
    int x, y;

    /* Very simplified version of wipe_o_list() */
    C_WIPE(o_list, o_max, object_type);
    o_max = 1;
    o_cnt = 0;
    unique_count = 0;

    /* Note, when I replaced the above with wipe_o_list(), artifacts started spawning
       multiple times!
      wipe_o_list();*/

    wipe_m_list();

    /* Pre-calc cur_num of pets in party_mon[] */
    precalc_cur_num_of_pet();

    /* Start with a blank cave */
    for (y = 0; y < MAX_HGT; y++)
    {
        for (x = 0; x < MAX_WID; x++)
        {
            cave_type *c_ptr = &cave[y][x];

            /* No flags */
            c_ptr->info = 0;

            /* No features */
            c_ptr->feat = 0;

            /* No objects */
            c_ptr->o_idx = 0;

            /* No monsters */
            c_ptr->m_idx = 0;

            /* No special */
            c_ptr->special = 0;

            /* No mimic */
            c_ptr->mimic = 0;

            /* No flow */
            c_ptr->cost = 0;
            c_ptr->dist = 0;
            c_ptr->when = 0;
        }
    }

    /* Mega-Hack -- no player yet */
    px = py = 0;

    /* Set the base level */
    base_level = dun_level;

    /* Reset the monster generation level */
    monster_level = base_level;

    /* Reset the object generation level */
    object_level = base_level;
}


/*
 * Generates a random dungeon level            -RAK-
 *
 * Hack -- regenerate any "overflow" levels
 */
void generate_cave(void)
{
    int num;

    /* Fill the arrays of floors and walls in the good proportions */
    set_floor_and_wall(dungeon_type);

    /* Generate */
    for (num = 0; TRUE; num++)
    {
        bool okay = TRUE;
        cptr why = NULL;

        clear_cave();

        /* Build the arena -KMW- */
        if (p_ptr->inside_arena)
        {
            arena_gen();
        }
        /* Build the battle -KMW- */
        else if (p_ptr->inside_battle)
        {
            battle_gen();
        }
        /* Enter a special quest level from the wilderness (QUEST_ENTER(id)) */
        else if (enter_quest)
        {
            quests_generate(enter_quest);
            enter_quest = 0;
        }
        /* Build the town */
        else if (!dun_level)
        {
            /* Make the wilderness */
            if (p_ptr->wild_mode) wilderness_gen_small();
            else wilderness_gen();
        }
        /* Build a real level, possibly a quest level.
         * The quest level might want to generate itself 
         * or it might simply need to 'place quest monsters' */
        else
        {
            quest_ptr q;
            quests_on_generate(dungeon_type, dun_level);
            q = quests_get_current();
            if (q && (q->flags & QF_GENERATE))
                quest_generate(q);
            else
            {
                okay = level_gen(&why);
                if (okay && q)
                    okay = quest_post_generate(q);
            }
        }


        if (o_max >= max_o_idx)
        {
            why = "too many objects";
            okay = FALSE;
        }
        else if (m_max >= max_m_idx)
        {
            why = "too many monsters";
            okay = FALSE;
        }

        if (okay)
            break;
        if ((why) && (p_ptr->wizard))
            msg_format("Generation restarted (%s)", why);

        wipe_o_list();
        wipe_m_list();
    }
    glow_deep_lava_and_bldg();
    p_ptr->enter_dungeon = FALSE;
    wipe_generate_cave_flags();

#if 0
    no_karrot_hack = TRUE;
    wiz_lite(FALSE);
    detect_all(255);
    if (1)
    {
        int i, ct = 0;
        char buf[MAX_NLEN];
        for (i = 0; i < max_o_idx; i++)
        {
            if (!o_list[i].k_idx) continue;
            ct++;
            identify_item(&o_list[i]);
            o_list[i].ident |= IDENT_MENTAL;
            if (o_list[i].name1 || o_list[i].name2)
            {
                object_desc(buf, &o_list[i], 0);
                msg_print(buf);
            }
        }
        msg_format("Objects=%d", ct);
    }
    {
        int i;
        int lvl = 0, ct = 0, uniques = 0, ct_drops = 0;
        for (i = 1; i < max_m_idx; i++)
        {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;

            if (!m_ptr->r_idx) continue;
            r_ptr = real_r_ptr(m_ptr);
            ct++;
            ct_drops += m_ptr->drop_ct;
            lvl += r_ptr->level;
            if (r_ptr->flags1 & RF1_UNIQUE)
                uniques++;
        }
        msg_format("DL=%d, Monsters=%d, Drops=%d, <ML>= %d, Uniques=%d", dun_level, ct, ct_drops, lvl/MAX(ct, 1), uniques);
        for (i = 0; i < ct_drops; i++)
        {
            object_type forge;
            char        buf[MAX_NLEN];

            make_object(&forge, 0, ORIGIN_CHEAT); /* TODO: DROP_GOOD? */
            /*if (forge.name1 || forge.name2)*/
            if (forge.curse_flags)
            {
                identify_item(&forge);
                forge.ident |= IDENT_MENTAL;
                object_desc(buf, &forge, 0);
                msg_print(buf);
            }
        }
    }
    no_karrot_hack = FALSE;
#endif
}
