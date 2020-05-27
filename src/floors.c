/* File: floors.c */

/* Purpose: management of the saved floor */

/*
 * Copyright (c) 2002  Mogami
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "grid.h"


static s16b new_floor_id;       /* floor_id of the destination */
static u32b change_floor_mode;  /* Mode flags for changing floor */
static u32b latest_visit_mark;  /* Max number of visit_mark */


/*
 * Initialize saved_floors array. Make sure that old temporal files
 * are not remaining as gurbages.
 */
void init_saved_floors(bool force)
{
    char floor_savefile[1024];
    int i;
    int fd = -1;
    int mode = 0644;

#ifdef SET_UID
# ifdef SECURE
    /* Get "games" permissions */
    beGames();
# endif
#endif

    for (i = 0; i < MAX_SAVED_FLOORS; i++)
    {
        saved_floor_type *sf_ptr = &saved_floors[i];

        /* File name */
        sprintf(floor_savefile, "%s.F%02d", savefile, i);

        /* Grab permissions */
        safe_setuid_grab();

        /* Try to create the file */
        fd = fd_make(floor_savefile, mode);

        /* Drop permissions */
        safe_setuid_drop();

        /* Failed! */
        if (fd < 0)
        {
            if (!force)
            {
                msg_print("Error: There are old temporal files.");
                msg_print("Make sure you are not running two game processes simultaneously.");
                msg_print("If the temporal files are garbages of old crashed process, ");
                msg_print("you can delete it safely.");
                if (!get_check("Do you delete old temporal files? ")) quit("Aborted.");
                force = TRUE;
            }
        }
        else
        {
            /* Close the "fd" */
            (void)fd_close(fd);
        }

        /* Grab permissions */
        safe_setuid_grab();

        /* Simply kill the temporal file */ 
        (void)fd_kill(floor_savefile);

        /* Drop permissions */
        safe_setuid_drop();

        sf_ptr->floor_id = 0;
    }

    /* No floor_id used yet (No.0 is reserved to indicate non existance) */
    max_floor_id = 1;

    /* vist_mark is from 1 */
    latest_visit_mark = 1;

    /* A sign to mark temporal files */
    saved_floor_file_sign = (u32b)time(NULL);

    /* No next floor yet */
    new_floor_id = 0;

    /* No change floor mode yet */
    change_floor_mode = 0;

#ifdef SET_UID
# ifdef SECURE
    /* Drop "games" permissions */
    bePlayer();
# endif
#endif
}


/*
 * Kill temporal files
 * Should be called just before the game quit.
 */
void clear_saved_floor_files(void)
{
    char floor_savefile[1024];
    int i;

#ifdef SET_UID
# ifdef SECURE
    /* Get "games" permissions */
    beGames();
# endif
#endif

    for (i = 0; i < MAX_SAVED_FLOORS; i++)
    {
        saved_floor_type *sf_ptr = &saved_floors[i];

        /* No temporal file */
        if (!sf_ptr->floor_id) continue;
        if (sf_ptr->floor_id == p_ptr->floor_id) continue;

        /* File name */
        sprintf(floor_savefile, "%s.F%02d", savefile, i);

        /* Grab permissions */
        safe_setuid_grab();

        /* Simply kill the temporal file */ 
        (void)fd_kill(floor_savefile);

        /* Drop permissions */
        safe_setuid_drop();
    }

#ifdef SET_UID
# ifdef SECURE
    /* Drop "games" permissions */
    bePlayer();
# endif
#endif
}


/*
 * Get a pointer for an item of the saved_floors array.
 */
saved_floor_type *get_sf_ptr(s16b floor_id)
{
    int i;

    /* floor_id No.0 indicates no floor */
    if (!floor_id) return NULL;

    for (i = 0; i < MAX_SAVED_FLOORS; i++)
    {
        saved_floor_type *sf_ptr = &saved_floors[i];

        if (sf_ptr->floor_id == floor_id) return sf_ptr;
    }

    /* None found */
    return NULL;
}


/*
 * kill a saved floor and get an empty space
 */
static void kill_saved_floor(saved_floor_type *sf_ptr)
{
    char floor_savefile[1024];

    /* Paranoia */
    if (!sf_ptr) return;

    /* Already empty */
    if (!sf_ptr->floor_id) return;

    if (sf_ptr->floor_id == p_ptr->floor_id)
    {
        /* Kill current floor */
        p_ptr->floor_id = 0;

        /* Current floor doesn't have temporal file */
    }
    else 
    {
        /* File name */
        sprintf(floor_savefile, "%s.F%02d", savefile, (int)sf_ptr->savefile_id);

        /* Grab permissions */
        safe_setuid_grab();

        /* Simply kill the temporal file */ 
        (void)fd_kill(floor_savefile);

        /* Drop permissions */
        safe_setuid_drop();
    }

    /* No longer exists */
    sf_ptr->floor_id = 0;
}


/*
 * Initialize new saved floor and get its floor id. If number of
 * saved floors are already MAX_SAVED_FLOORS, kill the oldest one.
 */
s16b get_new_floor_id(void)
{
    saved_floor_type *sf_ptr = 0;
    int i;

    /* Look for empty space */
    for (i = 0; i < MAX_SAVED_FLOORS; i++)
    {
        sf_ptr = &saved_floors[i];

        if (!sf_ptr->floor_id) break;
    }

    /* None found */
    if (i == MAX_SAVED_FLOORS)
    {
        int oldest = 0;
        u32b oldest_visit = 0xffffffffL;

        /* Search for oldest */
        for (i = 0; i < MAX_SAVED_FLOORS; i++)
        {
            sf_ptr = &saved_floors[i];

            /* Don't kill current floor */
            if (sf_ptr->floor_id == p_ptr->floor_id) continue;

            /* Don't kill newer */
            if (sf_ptr->visit_mark > oldest_visit) continue;

            oldest = i;
            oldest_visit = sf_ptr->visit_mark;
        }

        /* Kill oldest saved floor */
        sf_ptr = &saved_floors[oldest];
        kill_saved_floor(sf_ptr);

        /* Use it */
        i = oldest;
    }

    /* Prepare new floor data */
    sf_ptr->savefile_id = i;
    sf_ptr->floor_id = max_floor_id;
    sf_ptr->last_visit = 0;
    sf_ptr->upper_floor_id = 0;
    sf_ptr->lower_floor_id = 0;
    sf_ptr->visit_mark = latest_visit_mark++;

    /* sf_ptr->dun_level may be changed later */
    sf_ptr->dun_level = dun_level;


    /* Increment number of floor_id */
    if (max_floor_id < MAX_SHORT) max_floor_id++;

    /* 32767 floor_ids are all used up!  Re-use ancient IDs */
    else max_floor_id = 1;

    return sf_ptr->floor_id;
}


/*
 * Prepare mode flags of changing floor
 */
void prepare_change_floor_mode(u32b mode)
{
    change_floor_mode |= mode;
}


/*
 * Builds the dead end
 */
static void build_dead_end(void)
{
    int x,y;

    /* Clear and empty the cave */
    clear_cave();

    /* Fill the arrays of floors and walls in the good proportions */
    set_floor_and_wall(0);

    /* Smallest area */
    cur_hgt = SCREEN_HGT;
    cur_wid = SCREEN_WID;

    /* Filled with permanent walls */
    for (y = 0; y < MAX_HGT; y++)
    {
        for (x = 0; x < MAX_WID; x++)
        {
            /* Create "solid" perma-wall */
            place_solid_perm_bold(y, x);
        }
    }

    /* Place at center of the floor */
    py = cur_hgt / 2;
    px = cur_wid / 2;

    /* Give one square */
    place_floor_bold(py, px);

    wipe_generate_cave_flags();
}


/* Maximum number of preservable pets */
#define MAX_PARTY_MON 21

static monster_type party_mon[MAX_PARTY_MON];


/*
 * Preserve_pets
 */
static void preserve_pet(void)
{
    int num, i;
    static bool inside_arena = FALSE;

    for (num = 0; num < MAX_PARTY_MON; num++)
    {
        party_mon[num].r_idx = 0;
    }

    /* Nothing follows if you Rewind Time, not even your mount! */
    if (p_ptr->leaving_method == LEAVING_REWIND_TIME)
    {
        p_ptr->leaving_method = LEAVING_UNKNOWN;
        return;
    }

    if (p_ptr->riding)
    {
        monster_type *m_ptr = &m_list[p_ptr->riding];

        m_ptr->parent_m_idx = 0;
        m_ptr->pack_idx = 0;
        COPY(&party_mon[0], m_ptr, monster_type);

        /* Delete from this floor */
        delete_monster_idx(p_ptr->riding);
    }

    /* Teleport Level and Alter Reality loses all pets except your mount, and no monsters may follow. */
    if ( p_ptr->leaving_method == LEAVING_TELEPORT_LEVEL 
      || p_ptr->leaving_method == LEAVING_ALTER_REALITY )
    {
        p_ptr->leaving_method = LEAVING_UNKNOWN;
        return;
    }

    /*
     * If player is in wild mode, no pets are preserved
     * except a monster whom player riding
     * Hack: In the wilderness, hostile monsters now follow!
     */
    if (!p_ptr->wild_mode && !inside_arena && !p_ptr->inside_battle && !p_ptr->inside_arena)
    {
        for (i = m_max - 1, num = 1; (i >= 1 && num < MAX_PARTY_MON); i--)
        {
            monster_type *m_ptr = &m_list[i];

            if (!m_ptr->r_idx) continue;
            if (i == p_ptr->riding) continue;

            if (reinit_wilderness)
            {
                /* Don't lose sight of pets when getting a Quest */
                if (!is_pet(m_ptr)) continue;
            }
            else
            {
                int dis = distance(py, px, m_ptr->fy, m_ptr->fx);

                /* Confused (etc.) monsters don't follow. */
                if (MON_CONFUSED(m_ptr) || MON_STUNNED(m_ptr) || MON_CSLEEP(m_ptr)) continue;

                /* Pet of other pet don't follow. */
                if (m_ptr->parent_m_idx) continue;

                /* Hack: Hostile monsters follow, even if you recall! */
                if (!is_pet(m_ptr))
                {
                    int rng = 1;
                    if (dun_level < 1) continue;
                    if (dis > rng) continue;
                    if (r_info[m_ptr->r_idx].flags1 & RF1_UNIQUE) continue;
                    if (m_ptr->mflag2 & MFLAG2_QUESTOR) continue;
                    if (!los(m_ptr->fy, m_ptr->fx, py, px)) continue;
                    m_ptr->mflag |= MFLAG_NICE;
                }
                else if (m_ptr->parent_m_idx)
                {
                    continue;
                }
                else if (m_ptr->nickname && 
                         ((player_has_los_bold(m_ptr->fy, m_ptr->fx) && projectable(py, px, m_ptr->fy, m_ptr->fx)) ||
                         (los(m_ptr->fy, m_ptr->fx, py, px) && projectable(m_ptr->fy, m_ptr->fx, py, px))))
                {
                    if (dis > 4) continue;
                }
                else
                {
                    if (dis > 2) continue;
                }
            }

            m_ptr->pack_idx = 0;
            COPY(&party_mon[num], &m_list[i], monster_type);

            num++;

            /* Delete from this floor */
            delete_monster_idx(i);
        }
    }

    /* Pet of other pet may disappear. */
    for (i = m_max - 1; i >=1; i--)
    {
        monster_type *m_ptr = &m_list[i];

        /* Are there its parent? */
        if (m_ptr->parent_m_idx && !m_list[m_ptr->parent_m_idx].r_idx)
        {
            /* Its parent have gone, it also goes away. */

            if (mon_show_msg(m_ptr))
            {
                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                msg_format("%^s disappears!", m_name);
            }

            /* Delete the monster */
            delete_monster_idx(i);
        }
    }

    /* Hack: p_ptr->inside_arena is out of synch with the level we are leaving
       so stay one step behind ...
       p_ptr->inside_arena = TRUE for leaving the town to enter the arena
       p_ptr->inside_arena = FALSE for leaving the arena to enter the town
       Both of these are obviously wrong.

       p_ptr->inside_arena = FALSE for most other circumstances!
    */
    inside_arena = p_ptr->inside_arena;
    p_ptr->leaving_method = LEAVING_UNKNOWN;
}


/*
 * Pre-calculate the racial counters of preserved pets
 * To prevent multiple generation of unique monster who is the minion of player
 */
void precalc_cur_num_of_pet(void)
{
    monster_type *m_ptr;
    int i;
    int max_num = p_ptr->wild_mode ? 1 : MAX_PARTY_MON;

    for (i = 0; i < max_num; i++)
    {
        m_ptr = &party_mon[i];

        /* Skip empty monsters */
        if (!m_ptr->r_idx) continue;

        /* Hack -- Increase the racial counter */
        inc_cur_num(m_ptr, 1);
    }
}


/*
 * Place preserved pet monsters on new floor
 */
static void place_pet(void)
{
    int i;
    int max_num = MAX_PARTY_MON;

    for (i = 0; i < max_num; i++)
    {
        int cy = 0, cx = 0, m_idx = 0;

        if (!(party_mon[i].r_idx)) continue;

        if (i == 0)
        {
            m_idx = m_pop();
            p_ptr->riding = m_idx;
            if (m_idx)
            {
                cy = py;
                cx = px;
            }
        }
        else if (!p_ptr->inside_arena)
        {
            int j, d;

            for (d = 1; d < 6; d++)
            {
                for (j = 1000; j > 0; j--)
                {
                    scatter(&cy, &cx, py, px, d, 0);
                    if (monster_can_enter(cy, cx, &r_info[party_mon[i].r_idx], 0)) break;
                }
                if (j) break;
            }
            m_idx = (d == 6) ? 0 : m_pop();
        }

        if (m_idx)
        {
            monster_type *m_ptr = &m_list[m_idx];
            monster_race *r_ptr;

            cave[cy][cx].m_idx = m_idx;

            m_ptr->r_idx = party_mon[i].r_idx;

            /* Copy all member of the structure */
            *m_ptr = party_mon[i];
            r_ptr = real_r_ptr(m_ptr);

            m_ptr->id = m_idx;
            m_ptr->fy = cy;
            m_ptr->fx = cx;
            m_ptr->ml = TRUE;
            m_ptr->mtimed[MTIMED_CSLEEP] = 0;

            /* Paranoia */
            m_ptr->hold_o_idx = 0;
            m_ptr->target_y = 0;

            if ((r_ptr->flags1 & RF1_FORCE_SLEEP) && !ironman_nightmare)
            {
                /* Monster is still being nice */
                m_ptr->mflag |= (MFLAG_NICE);

                /* Must repair monsters */
                repair_monsters = TRUE;
            }

            /* Update the monster */
            update_mon(m_idx, TRUE);
            lite_spot(cy, cx);

            /* Pre-calculated in precalc_cur_num_of_pet() */
            /* r_ptr->cur_num++; */

            /* Hack -- Count the number of "reproducers" */
            if (r_ptr->flags2 & RF2_MULTIPLY) num_repro++;

            /* Hack -- Notice new multi-hued monsters */
            {
                monster_race *ap_r_ptr = &r_info[m_ptr->ap_r_idx];
                if (ap_r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_SHAPECHANGER))
                    shimmer_monsters = TRUE;
            }
        }
        else
        {
            monster_type *m_ptr = &party_mon[i];
            monster_race *r_ptr = real_r_ptr(m_ptr);
            char m_name[80];

            monster_desc(m_name, m_ptr, 0);
            msg_format("You have lost sight of %s.", m_name);

            /* Pre-calculated in precalc_cur_num_of_pet(), but need to decrease */
            if (r_ptr->cur_num) inc_cur_num(m_ptr, -1);
        }
    }

    /* For accuracy of precalc_cur_num_of_pet() */
    C_WIPE(party_mon, MAX_PARTY_MON, monster_type);
}

static vec_ptr saily_objects = NULL;

/*
 * Hack -- preserve quest rewards on the town floor when a new quest is
 * requested
 */
static void preserve_object_aux(object_type *o_ptr)
{
    object_type *saily_obj = obj_alloc();
    if (!saily_objects)
    {
        saily_objects = vec_alloc(free);
    }
    object_copy(saily_obj, o_ptr);
    vec_add(saily_objects, saily_obj);
    quest_reward_drop_hack = TRUE;
}

static void preserve_objects(void)
{
    int i;
    if (!py_on_surface()) quest_reward_drop_hack = FALSE; /* paranoia */
    if (!quest_reward_drop_hack) return;
    quest_reward_drop_hack = FALSE;
    for (i = 0; i < max_o_idx; i++)
    {
        object_type *o_ptr = &o_list[i];
        if (!o_ptr->k_idx) continue;
        if (o_ptr->loc.where != INV_FLOOR) continue; /* paranoia */
        if (o_ptr->origin_type != ORIGIN_QUEST_REWARD) continue;
        preserve_object_aux(o_ptr);
    }
}

static void place_saily_objects(void)
{
    if (!quest_reward_drop_hack) return;
    if ((!saily_objects) || (!vec_length(saily_objects))) return; /* paranoia */
    while (vec_length(saily_objects))
    {
        object_type *saily_obj = vec_pop(saily_objects);

        /* Copy the object */
//        object_copy(o_ptr, saily_obj);

        /* Drop it */
        (void)drop_near(saily_obj, -1, saily_obj->loc.y, saily_obj->loc.x);

        obj_free(saily_obj);
//        obj_free(o_ptr);
    }
    vec_free(saily_objects);
    saily_objects = NULL;
    quest_reward_drop_hack = FALSE;
}

/*
 * Hack -- Update location of unique monsters and artifacts
 *
 * The r_ptr->floor_id and a_ptr->floor_id are not updated correctly
 * while new floor creation since dungeons may be re-created by
 * auto-scum option.
 */
static void update_unique_artifact(s16b cur_floor_id)
{
    int i;

    /* Maintain unique monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_race *r_ptr;
        monster_type *m_ptr = &m_list[i];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Extract real monster race */
        r_ptr = real_r_ptr(m_ptr);

        /* Memorize location of the unique monster */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->flags7 & RF7_NAZGUL))
        {
            r_ptr->floor_id = cur_floor_id;
        }
    }

    /* Maintain artifacts */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Memorize location of the artifact */
        if (object_is_fixed_artifact(o_ptr))
        {
            a_info[o_ptr->name1].floor_id = cur_floor_id;
        }
        else if (o_ptr->name3)
        {
            a_info[o_ptr->name3].floor_id = cur_floor_id;
        }
    }
}


/*
 * When a monster is at a place where player will return,
 * Get out of the my way!
 */
static void get_out_monster(void)
{
    int tries = 0;
    int dis = 1;
    int oy = py;
    int ox = px;
    int m_idx = cave[oy][ox].m_idx;

    /* Nothing to do if no monster */
    if (!m_idx) return;

    /* Look until done */
    while (TRUE)
    {
        monster_type *m_ptr;

        /* Pick a (possibly illegal) location */
        int ny = rand_spread(oy, dis);
        int nx = rand_spread(ox, dis);

        tries++;

        /* Stop after 1000 tries */
        if (tries > 10000) return;

        /*
         * Increase distance after doing enough tries
         * compared to area of possible space
         */
        if (tries > 20 * dis * dis) dis++;

        /* Ignore illegal locations */
        if (!in_bounds(ny, nx)) continue;

        /* Require "empty" floor space */
        if (!cave_empty_bold(ny, nx)) continue;

        /* Hack -- no teleport onto glyph of warding */
        if (is_glyph_grid(&cave[ny][nx])) continue;
        if (is_mon_trap_grid(&cave[ny][nx])) continue;

        /* ...nor onto the Pattern */
        if (pattern_tile(ny, nx)) continue;

        /*** It's a good place ***/

        m_ptr = &m_list[m_idx];

        /* Update the old location */
        cave[oy][ox].m_idx = 0;

        /* Update the new location */
        cave[ny][nx].m_idx = m_idx;

        /* Move the monster */
        m_ptr->fy = ny;
        m_ptr->fx = nx; 

        /* No need to do update_mon() */

        /* Success */
        return;
    }
}


/*
 * Is this feature has special meaning (except floor_id) with c_ptr->special?
 */
#define feat_uses_special(F) (have_flag(f_info[(F)].flags, FF_SPECIAL))


/*
 * Virtually teleport onto the stairs that is connecting between two
 * floors.
 *
 * Teleport level spell and trap doors will always lead the player to
 * the one of the floors connected by the one of the stairs in the
 * current floor.
 */
static void locate_connected_stairs(saved_floor_type *sf_ptr)
{
    int x, y, sx = 0, sy = 0;
    int x_table[20];
    int y_table[20];
    int num = 0;
    int i;

    /* Search usable stairs */
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            cave_type *c_ptr = &cave[y][x];
            feature_type *f_ptr = &f_info[c_ptr->feat];
            bool ok = FALSE;

            if (change_floor_mode & CFM_UP)
            {
                if (have_flag(f_ptr->flags, FF_LESS) && have_flag(f_ptr->flags, FF_STAIRS) &&
                    !have_flag(f_ptr->flags, FF_SPECIAL))
                {
                    ok = TRUE;

                    /* Found fixed stairs? */
                    if (c_ptr->special &&
                        c_ptr->special == sf_ptr->upper_floor_id)
                    {
                        sx = x;
                        sy = y;
                    }
                }
            }

            else if (change_floor_mode & CFM_DOWN)
            {
                if (have_flag(f_ptr->flags, FF_MORE) && have_flag(f_ptr->flags, FF_STAIRS) &&
                    !have_flag(f_ptr->flags, FF_SPECIAL))
                {
                    ok = TRUE;

                    /* Found fixed stairs */
                    if (c_ptr->special &&
                        c_ptr->special == sf_ptr->lower_floor_id)
                    {
                        sx = x;
                        sy = y;
                    }
                }
            }

            else
            {
                if (have_flag(f_ptr->flags, FF_BLDG))
                {
                    ok = TRUE;
                }
            }

            if (ok && (num < 20))
            {
                x_table[num] = x;
                y_table[num] = y;
                num++;
            }
        }
    }

    if (sx)
    {
        /* Already fixed */
        py = sy;
        px = sx;
    }
    else if (!num)
    {
        /* No stairs found! -- No return */
        prepare_change_floor_mode(CFM_RAND_PLACE | CFM_NO_RETURN);

        /* Mega Hack -- It's not the stairs you enter. Disable it. */
        if (!feat_uses_special(cave[py][px].feat)) cave[py][px].special = 0;
    }
    else
    {
        /* Choose random one */
        i = randint0(num);

        /* Point stair location */
        py = y_table[i];
        px = x_table[i];
    }
}

/*
 * Maintain quest monsters, mark next floor_id at stairs, save current
 * floor, and prepare to enter next floor.
 */
static void _fix_art_hack(obj_ptr obj)
{
    if (obj->name1) a_info[obj->name1].floor_id = 0;
    else if (obj->name3) a_info[obj->name3].floor_id = 0;
}
void leave_floor(void)
{
    cave_type *c_ptr = NULL;
    feature_type *f_ptr;
    saved_floor_type *sf_ptr;

    /* Preserve pets and prepare to take these to next floor */
    preserve_pet();

    /* Preserve objects (in special cases) */
    preserve_objects();

    /* Remove all mirrors without explosion */
    remove_all_mirrors(FALSE);

    if (p_ptr->special_defense & NINJA_S_STEALTH) set_superstealth(FALSE);

    /* New floor is not yet prepared */
    new_floor_id = 0;

    /* Hack - fix bug with recalling and taking stairs at the same time */
    if (recall_stairs_hack)
    {
        change_floor_mode = (dun_level ? CFM_FIRST_FLOOR : 0);
    }

    /* Temporary get a floor_id (for Arena) */
    if (!p_ptr->floor_id &&
        (change_floor_mode & CFM_SAVE_FLOORS) &&
        !(change_floor_mode & CFM_NO_RETURN))
    {
        /* Get temporal floor_id */
        p_ptr->floor_id = get_new_floor_id();
    }

    /* Check if there is a same item */
    equip_for_each(_fix_art_hack);
    pack_for_each(_fix_art_hack);
    quiver_for_each(_fix_art_hack);

    /* Extract current floor info or NULL */
    sf_ptr = get_sf_ptr(p_ptr->floor_id);

    /* Choose random stairs */
    if ((change_floor_mode & CFM_RAND_CONNECT) && p_ptr->floor_id)
    {
        locate_connected_stairs(sf_ptr);
    }

    /* Extract new dungeon level */
    if (change_floor_mode & CFM_SAVE_FLOORS)
    {
        /* Extract stair position */
        c_ptr = &cave[py][px];
        f_ptr = &f_info[c_ptr->feat];

        /* Get back to old saved floor? */
        if (c_ptr->special && !have_flag(f_ptr->flags, FF_SPECIAL) && get_sf_ptr(c_ptr->special))
        {
            /* Saved floor is exist. Use it. */
            new_floor_id = c_ptr->special;
        }

        /* Mark shaft up/down */
        if (have_flag(f_ptr->flags, FF_STAIRS) && have_flag(f_ptr->flags, FF_SHAFT) && quests_allow_downshaft())
        {
            prepare_change_floor_mode(CFM_SHAFT);
        }
    }

    /* Climb up/down some sort of stairs */
    if (change_floor_mode & (CFM_DOWN | CFM_UP))
    {
        int move_num = 0;

        /* Extract level movement number */
        if (change_floor_mode & CFM_DOWN) move_num = 1;
        else if (change_floor_mode & CFM_UP) move_num = -1;

        /* Shafts are deeper than normal stairs */
        if (change_floor_mode & CFM_SHAFT)
            move_num += SGN(move_num);

        /* Pyramidal Mound has 4-level shafts near the top */
        if (((dungeon_type == DUNGEON_MOUND) || (dungeon_type == DUNGEON_ASGARD)) && (dun_level < ((change_floor_mode & CFM_DOWN) ? 78 : 82)))
            move_num += SGN(move_num) * 2;

        /* Get out from or Enter the dungeon */
        if (change_floor_mode & CFM_DOWN)
        {
            if (!dun_level)
            {
                if (d_info[dungeon_type].flags1 & DF1_RANDOM)
                    move_num = rand_range(d_info[dungeon_type].mindepth, d_info[dungeon_type].maxdepth);
                else if ((coffee_break) && (dungeon_type == DUNGEON_ANGBAND))
                    move_num = coffeebreak_recall_level(TRUE);
                else
                    move_num = d_info[dungeon_type].mindepth;
            }
            else if (coffee_break == SPEED_INSTA_COFFEE)
            {
                move_num = coffeebreak_recall_level(TRUE) - dun_level;
            }
        }
        else if (change_floor_mode & CFM_UP)
        {
            if ( dun_level + move_num < d_info[dungeon_type].mindepth
              || (d_info[dungeon_type].flags1 & DF1_RANDOM) )
            {
                move_num = -dun_level;
            }
        }

        dun_level += move_num;
    }

    /* Leaving the dungeon to town */
    if (!dun_level && dungeon_type)
    {
        p_ptr->leaving_dungeon = dungeon_type;

        if (!(d_info[dungeon_type].flags1 & DF1_RANDOM))
        {
            if (!no_wilderness)
            {
                p_ptr->wilderness_y = d_info[dungeon_type].dy;
                p_ptr->wilderness_x = d_info[dungeon_type].dx;
                p_ptr->wilderness_dx = 0;
                p_ptr->wilderness_dy = 0;
            }
            p_ptr->recall_dungeon = dungeon_type;
        }
        dungeon_type = 0;

        /* Reach to the surface -- Clear all saved floors */
        change_floor_mode &= ~CFM_SAVE_FLOORS;
    }

    /* Hack */
    if ((p_ptr->no_air) && (!dun_level) && (!(get_race()->flags & RACE_IS_NONLIVING)))
    {
        msg_print("You feel a welcome breeze as you return to the world of air!");
        p_ptr->no_air = 0;
        no_air_monster = 0;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
        p_ptr->redraw |= (PR_STATUS);
    }

    /* End temporary dimensional lock */
    if ((prace_is_(RACE_MON_MUMMY)) && (mummy_get_toggle() == MUMMY_TOGGLE_ANTITELE))
    {
        mummy_cast_antitele();
    }

    /* Kill some old saved floors */
    if (!(change_floor_mode & CFM_SAVE_FLOORS))
    {
        int i;

        /* Kill all saved floors */
        for (i = 0; i < MAX_SAVED_FLOORS; i++)
            kill_saved_floor(&saved_floors[i]);

        /* Reset visit_mark count */
        latest_visit_mark = 1;
    }
    else if (change_floor_mode & CFM_NO_RETURN)
    {
        /* Kill current floor */
        kill_saved_floor(sf_ptr);
    }

    /* No current floor -- Left/Enter dungeon etc... */
    if (!p_ptr->floor_id)
    {
        /* No longer need to save current floor */
        return;
    }


    /* Mark next floor_id on the previous floor */
    if (!new_floor_id)
    {
        /* Get new id */
        new_floor_id = get_new_floor_id();

        /* Connect from here */
        if (c_ptr && !feat_uses_special(c_ptr->feat))
        {
            c_ptr->special = new_floor_id;
        }
    }

    /* Fix connection -- level teleportation or trap door */
    if (change_floor_mode & CFM_RAND_CONNECT)
    {
        if (change_floor_mode & CFM_UP)
            sf_ptr->upper_floor_id = new_floor_id;
        else if (change_floor_mode & CFM_DOWN)
            sf_ptr->lower_floor_id = new_floor_id;
    }

    /* If you can return, you need to save previous floor */
    if ((change_floor_mode & CFM_SAVE_FLOORS) &&
        !(change_floor_mode & CFM_NO_RETURN))
    {
        /* Get out of the my way! */
        get_out_monster();

        /* Record the last visit turn of current floor */
        sf_ptr->last_visit = game_turn;

        /* Forget the lite */
        forget_lite();

        /* Forget the view */
        forget_view();

        /* Forget the view */
        clear_mon_lite();

        /* Save current floor */
        if (!save_floor(sf_ptr, 0))
        {
            /* Save failed -- No return */
            prepare_change_floor_mode(CFM_NO_RETURN);

            /* Kill current floor */
            kill_saved_floor(get_sf_ptr(p_ptr->floor_id));
        }
    }
}


/*
 * Enter new floor. If the floor is an old saved floor, it will be
 * restored from the temporal file. If the floor is new one, new cave
 * will be generated.
 */
void change_floor(void)
{
    saved_floor_type *sf_ptr;
    bool loaded = FALSE;

    /* The dungeon is not ready */
    character_dungeon = FALSE;

    /* No saved floors (On the surface etc.) */
    if (!(change_floor_mode & CFM_SAVE_FLOORS) &&
        !(change_floor_mode & CFM_FIRST_FLOOR))
    {
        /* Create cave */
        hack_mind = FALSE;
        generate_cave();
        hack_mind = TRUE;

        /* Paranoia -- No new saved floor */
        new_floor_id = 0;
    }

    /* In the dungeon */
    else
    {
        /* No floor_id yet */
        if (!new_floor_id)
        {
            /* Get new id */
            new_floor_id = get_new_floor_id();
        }

        /* Pointer for infomations of new floor */
        sf_ptr = get_sf_ptr(new_floor_id);

        /* Try to restore old floor */
        if (sf_ptr->last_visit)
        {
            /* Old saved floor is exist */
            if (load_floor(sf_ptr, 0))
            {
                loaded = TRUE;

                /* Forbid return stairs */
                if (change_floor_mode & CFM_NO_RETURN)
                {
                    cave_type *c_ptr = &cave[py][px];

                    if (!feat_uses_special(c_ptr->feat))
                    {
                        if (change_floor_mode & (CFM_DOWN | CFM_UP))
                        {
                            /* Reset to floor */
                            c_ptr->feat = floor_type[randint0(100)];
                        }

                        c_ptr->special = 0;
                    }
                }
            }
        }

        /*
         * Set lower/upper_floor_id of new floor when the new
         * floor is right-above/right-under the current floor.
         *
         * Stair creation/Teleport level/Trap door will take
         * you the same floor when you used it later again.
         */
        if (p_ptr->floor_id)
        {
            saved_floor_type *cur_sf_ptr = get_sf_ptr(p_ptr->floor_id);

            if (change_floor_mode & CFM_UP)
            {
                /* New floor is right-above */
                if (cur_sf_ptr->upper_floor_id == new_floor_id)
                    sf_ptr->lower_floor_id = p_ptr->floor_id;
            }
            else if (change_floor_mode & CFM_DOWN)
            {
                /* New floor is right-under */
                if (cur_sf_ptr->lower_floor_id == new_floor_id)
                    sf_ptr->upper_floor_id = p_ptr->floor_id;
            }
        }

        /* Break connection to killed floor */
        else
        {
            if (change_floor_mode & CFM_UP)
                sf_ptr->lower_floor_id = 0;
            else if (change_floor_mode & CFM_DOWN)
                sf_ptr->upper_floor_id = 0;
        }

        /* Maintain monsters and artifacts */
        if (loaded)
        {
            int i;
            s32b tmp_last_visit = sf_ptr->last_visit;
            s32b absence_ticks;
            int alloc_chance = d_info[dungeon_type].max_m_alloc_chance;
            int alloc_times;

            while (tmp_last_visit > game_turn) tmp_last_visit -= TURNS_PER_TICK * TOWN_DAWN;
            absence_ticks = (game_turn - tmp_last_visit) / TURNS_PER_TICK;

            /* Maintain monsters */
            for (i = 1; i < m_max; i++)
            {
                monster_race *r_ptr;
                monster_type *m_ptr = &m_list[i];

                /* Skip dead monsters */
                if (!m_ptr->r_idx) continue;

                if (!is_pet(m_ptr))
                {
                    /* Restore HP */
                    m_ptr->hp = m_ptr->maxhp = m_ptr->max_maxhp;

                    /* Remove timed status (except MTIMED_CSLEEP) */
                    (void)set_monster_fast(i, 0);
                    (void)set_monster_slow(i, 0);
                    (void)set_monster_stunned(i, 0);
                    (void)set_monster_confused(i, 0);
                    (void)set_monster_monfear(i, 0);
                    (void)set_monster_invulner(i, 0, FALSE);
                }

                /* Extract real monster race */
                r_ptr = real_r_ptr(m_ptr);

                /* Ignore non-unique */
                if (!(r_ptr->flags1 & RF1_UNIQUE) &&
                    !(r_ptr->flags7 & RF7_NAZGUL)) continue;

                /* Appear at a different floor? */
                if (r_ptr->floor_id != new_floor_id)
                {
                    /* Disapper from here */
                    delete_monster_idx(i);
                }
            }

            /* Maintain artifacts */
            for (i = 1; i < o_max; i++)
            {
                object_type *o_ptr = &o_list[i];

                /* Skip dead objects */
                if (!o_ptr->k_idx) continue;

                if (object_is_fixed_artifact(o_ptr))
                {
                    /* Appear at a different floor? */
                    if (a_info[o_ptr->name1].floor_id != new_floor_id)
                    {
                        /* Disappear from here */
                        delete_object_idx(i);
                    }
                    else
                    {
                        /* Cancel preserve */
                        a_info[o_ptr->name1].generated = TRUE;
                    }
                }
                if (o_ptr->name3)
                {
                    if (a_info[o_ptr->name3].floor_id != new_floor_id)
                        delete_object_idx(i);
                    else
                        a_info[o_ptr->name3].generated = TRUE;
                }
                if ((o_ptr->insured) && (o_ptr->number > 0))
                {
                    cornucopia_mark_destroyed(cornucopia_item_policy(o_ptr), 0 - (o_ptr->insured % 100));
                }
            }

            quests_on_restore_floor(dungeon_type, dun_level);

            /* Place some random monsters */
            alloc_times = absence_ticks / alloc_chance;

            if (randint0(alloc_chance) < (absence_ticks % alloc_chance))
                alloc_times++;

            for (i = 0; i < alloc_times; i++)
            {
                /* Make a (group of) new monster */
                (void)alloc_monster(0, 0);
            }
        }

        /* New floor_id or failed to restore */
        else /* if (!loaded) */
        {
            if (sf_ptr->last_visit)
            {
                /* Temporal file is broken? */
                msg_print("The staircases come to a dead end...");

                /* Create simple dead end */
                build_dead_end();

                /* Break connection */
                if (change_floor_mode & CFM_UP)
                {
                    sf_ptr->upper_floor_id = 0;
                }
                else if (change_floor_mode & CFM_DOWN)
                {
                    sf_ptr->lower_floor_id = 0;
                }
            }
            else
            {
                /* Newly create cave */
                hack_mind = FALSE;
                generate_cave();
                hack_mind = TRUE;
            }

            /* Record last visit turn */
            sf_ptr->last_visit = game_turn;

            /* Set correct dun_level value */
            sf_ptr->dun_level = dun_level;

            /* Create connected stairs */
            if (!(change_floor_mode & CFM_NO_RETURN))
            {
                /* Extract stair position */
                cave_type *c_ptr = &cave[py][px];

                /*** Create connected stairs ***/

                /* No stairs down from Quest */
                if ((change_floor_mode & CFM_UP) && !quests_get_current())
                {
                    c_ptr->feat = (change_floor_mode & CFM_SHAFT) ? feat_state(feat_down_stair, FF_SHAFT) : feat_down_stair;
                }

                /* No stairs up when ironman_downward */
                else if ((change_floor_mode & CFM_DOWN) && !ironman_downward)
                {
                    c_ptr->feat = (change_floor_mode & CFM_SHAFT) ? feat_state(feat_up_stair, FF_SHAFT) : feat_up_stair;
                }

                /* Paranoia -- Clear mimic */
                c_ptr->mimic = 0;

                /* Connect to previous floor */
                c_ptr->special = p_ptr->floor_id;
            }
        }

        /* Arrive at random grid */
        if (change_floor_mode & (CFM_RAND_PLACE))
        {
            (void)new_player_spot();
        }

        /* You see stairs blocked */
        else if ((change_floor_mode & CFM_NO_RETURN) &&
             (change_floor_mode & (CFM_DOWN | CFM_UP)))
        {
            if (!p_ptr->blind)
            {
                msg_print("Suddenly the stairs are blocked!");
            }
            else
            {
                msg_print("You hear some noises.");
            }
        }

        /*
         * Update visit mark
         *
         * The "turn" is not always different number because
         * the level teleport doesn't take any turn. Use
         * visit mark instead of last visit turn to find the
         * oldest saved floor.
         */
        sf_ptr->visit_mark = latest_visit_mark++;
    }

    /* Place preserved pet monsters */
    place_pet();

    /* Place preserved quest rewards */
    place_saily_objects();

    /* Hack -- maintain unique and artifacts */
    update_unique_artifact(new_floor_id);

    /* Now the player is in new floor */
    p_ptr->floor_id = new_floor_id;

    /* The dungeon is ready */
    character_dungeon = TRUE;

    /* Remember when this level was "created" */
    old_turn = game_turn;

    /* We have no travelling target on this level */
    travel.x = 0;
    travel.y = 0;

    /* Politicians have level-based resource pools */
    if (p_ptr->pclass == CLASS_POLITICIAN)
    {
        politician_check_experience(TRUE);
        politician_check_au(TRUE);
    }

    /* No dungeon feeling yet */
    p_ptr->feeling_turn = old_turn;
    p_ptr->feeling = 0;

    /* No kills yet */
    p_ptr->lv_kills = 0;
    p_ptr->pet_lv_kills = 0;

    /* Clear all flags */
    change_floor_mode = 0L;

    /* Player is still nice */
    p_ptr->nice = TRUE;
}



/*
 * Create stairs at or move previously created stairs into the player
 * location.
 */
void stair_creation(bool down_only)
{
    saved_floor_type *sf_ptr;
    saved_floor_type *dest_sf_ptr;

    bool up = TRUE;
    bool down = TRUE;
    bool force_shaft = FALSE;
    s16b dest_floor_id = 0;


    /* Forbid up staircases on Ironman mode */
    if (ironman_downward || down_only) up = FALSE;

    /* Forbid down staircases on quest level */
    if (quests_get_current() || (dun_level >= d_info[dungeon_type].maxdepth)) down = FALSE;

    /* No effect out of standard dungeon floor */
    if (py_on_surface() || (!up && !down) || !quests_allow_all_spells())
    {
        /* arena or quest */
        msg_print("There is no effect!");
        return;
    }

    /* Artifacts resists */
    if (!cave_valid_bold(py, px))
    {
        msg_print("The object resists the spell.");

        return;
    }

    /* Destroy all objects in the grid */
    delete_object(py, px);

    /* Extract current floor data */
    sf_ptr = get_sf_ptr(p_ptr->floor_id);

    /* Paranoia */
    if (!sf_ptr)
    {
        /* No floor id? -- Create now! */
        p_ptr->floor_id = get_new_floor_id();
        sf_ptr = get_sf_ptr(p_ptr->floor_id);
    } 

    /* Choose randomly */
    if (up && down)
    {
        if (randint0(100) < 50) up = FALSE;
        else down = FALSE;
    }

    /* Destination is already fixed */
    if (up)
    {
        if (sf_ptr->upper_floor_id) dest_floor_id = sf_ptr->upper_floor_id;
    }
    else
    {
        if (sf_ptr->lower_floor_id) dest_floor_id = sf_ptr->lower_floor_id;
    }


    /* Search old stairs leading to the destination */
    if (dest_floor_id)
    {
        int x, y;

        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                cave_type *c_ptr = &cave[y][x];

                if (!c_ptr->special) continue;
                if (feat_uses_special(c_ptr->feat)) continue;
                if (c_ptr->special != dest_floor_id) continue;

                /* Remove old stairs */
                c_ptr->special = 0;
                cave_set_feat(y, x, floor_type[randint0(100)]);
            }
        }
    }

    /* No old destination -- Get new one now */
    else
    {
        dest_floor_id = get_new_floor_id();

        /* Fix it */
        if (up)
            sf_ptr->upper_floor_id = dest_floor_id;
        else
            sf_ptr->lower_floor_id = dest_floor_id;
    }

    /* Extract destination floor data */
    dest_sf_ptr = get_sf_ptr(dest_floor_id);

    if ((coffee_break) || (d_info[dungeon_type].flags1 & DF1_ALL_SHAFTS)) force_shaft = TRUE;

    /* Create a staircase */
    if (up)
    {
        cave_set_feat(py, px, ((force_shaft) ||
            (dest_sf_ptr->last_visit && (dest_sf_ptr->dun_level <= dun_level - 2))) ?
            feat_state(feat_up_stair, FF_SHAFT) : feat_up_stair);
    }
    else
    {
        cave_set_feat(py, px, ((force_shaft) ||
            (dest_sf_ptr->last_visit && (dest_sf_ptr->dun_level >= dun_level + 2))) ?
            feat_state(feat_down_stair, FF_SHAFT) : feat_down_stair);
    }


    /* Connect this stairs to the destination */
    cave[py][px].special = dest_floor_id;
}
