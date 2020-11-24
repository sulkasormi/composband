
/* File: spells1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Spell projection */

#include "angband.h"
#include <assert.h>

int rakubadam_m;
int rakubadam_p;

int project_length = 0;

/*
 * Ticked off monsters must have a powerful distance attack
 */
bool allow_ticked_off(monster_race *r_ptr)
{
    if (!r_ptr->spells) return FALSE;
    if ( r_ptr->spells->groups[MST_BREATH]
      || r_ptr->spells->groups[MST_BALL]
      || r_ptr->spells->groups[MST_BOLT]
      || r_ptr->spells->groups[MST_BEAM]
      || r_ptr->spells->groups[MST_SUMMON] )
    {
        return TRUE;
    }
    return FALSE;
}

/*
 * Get another mirror. for SEEKER
 */
static void next_mirror( int* next_y , int* next_x , int cury, int curx)
{
    int mirror_x[10],mirror_y[10];
    int mirror_num=0;
    int x,y;
    int num;

    for( x=0 ; x < cur_wid ; x++ )
    {
        for( y=0 ; y < cur_hgt ; y++ )
        {
            if( is_mirror_grid(&cave[y][x])){
                mirror_y[mirror_num]=y;
                mirror_x[mirror_num]=x;
                mirror_num++;
            }
        }
    }
    if( mirror_num )
    {
        num=randint0(mirror_num);
        *next_y=mirror_y[num];
        *next_x=mirror_x[num];
        return;
    }
    *next_y=cury+randint0(5)-2;
    *next_x=curx+randint0(5)-2;
    return;
}

/*
 * Get a legal "multi-hued" color for drawing "spells"
 */
static byte mh_attr(int max)
{
    switch (randint1(max))
    {
        case  1: return (TERM_RED);
        case  2: return (TERM_GREEN);
        case  3: return (TERM_BLUE);
        case  4: return (TERM_YELLOW);
        case  5: return (TERM_ORANGE);
        case  6: return (TERM_VIOLET);
        case  7: return (TERM_L_RED);
        case  8: return (TERM_L_GREEN);
        case  9: return (TERM_L_BLUE);
        case 10: return (TERM_UMBER);
        case 11: return (TERM_L_UMBER);
        case 12: return (TERM_SLATE);
        case 13: return (TERM_WHITE);
        case 14: return (TERM_L_WHITE);
        case 15: return (TERM_L_DARK);
    }

    return (TERM_WHITE);
}


/*
 * Return a color to use for the bolt/ball spells
 */
static byte spell_color(int type)
{
    /* Check if A.B.'s new graphics should be used (rr9) */
    if (streq(ANGBAND_GRAF, "new"))
    {
        /* Analyze */
        switch (type)
        {
            case GF_PSY_SPEAR:      return (0x06);
            case GF_MISSILE:        return (0x0F);
            case GF_ACID:           return (0x04);
            case GF_ELEC:           return (0x02);
            case GF_BLOOD:          return (0x00);
            case GF_FIRE:           return (0x00);
            case GF_COLD:           return (0x01);
            case GF_POIS:           return (0x03);
            case GF_HOLY_FIRE:      return (0x00);
            case GF_HELL_FIRE:      return (0x00);
            case GF_MANA:           return (0x0E);
              /* by henkma */
            case GF_SEEKER:         return (0x0E);
            case GF_SUPER_RAY:      return (0x0E);

            case GF_ARROW:          return (0x0F);
            case GF_STORM:
            case GF_WATER:
            case GF_WATER2: return (0x04);
            case GF_WEB:          return (0x04);
            case GF_NETHER:         return (0x07);
            case GF_CHAOS:          return (mh_attr(15));
            case GF_DISENCHANT:     return (0x05);
            case GF_NEXUS:          return (0x0C);
            case GF_CONFUSION:      return (mh_attr(4));
            case GF_SOUND:          return (0x09);
            case GF_SHARDS:         return (0x08);
            case GF_ROCK:         return (0x08);
            case GF_FORCE:          return (0x09);
            case GF_INERT:        return (0x09);
            case GF_GRAVITY:        return (0x09);
            case GF_AMNESIA:          return (0x09);
            case GF_TIME:           return (0x09);
            case GF_LITE_WEAK:      return (0x06);
            case GF_LITE:           return (0x06);
            case GF_DARK_WEAK:      return (0x07);
            case GF_DARK:           return (0x07);
            case GF_PLASMA:         return (0x0B);
            case GF_METEOR:         return (0x00);
            case GF_ICE:            return (0x01);
            case GF_ROCKET:         return (0x0F);
            case GF_DEATH_RAY:      return (0x07);
            case GF_CHICKEN:        return (0x0B);
            case GF_NUKE:           return (mh_attr(2));
            case GF_DISINTEGRATE:   return (0x05);
            case GF_PSI:
            case GF_PSI_DRAIN:
            case GF_TELEKINESIS:
            case GF_DOMINATION:
            case GF_DRAIN_MANA:
            case GF_MIND_BLAST:
            case GF_BRAIN_SMASH:
                        return (0x09);
            case GF_CAUSE_1:
            case GF_CAUSE_2:
            case GF_CAUSE_3:
            case GF_CAUSE_4:        return (0x0E);
            case GF_HAND_DOOM:      return (0x07);
            case GF_CAPTURE  :      return (0x0E);
            case GF_IDENTIFY:       return (0x01);
            case GF_ATTACK:        return (0x0F);
            case GF_PHOTO   :      return (0x06);
        }
    }
    /* Normal tiles or ASCII */
    else
    {
        byte a;
        char c;

        /* Lookup the default colors for this type */
        cptr s = quark_str(gf_color[type]);

        /* Oops */
        if (!s) return (TERM_WHITE);

        /* Pick a random color */
        c = s[randint0(strlen(s))];

        /* Lookup this color */
        a = my_strchr(color_char, c) - color_char;

        /* Invalid color (note check for < 0 removed, gave a silly
         * warning because bytes are always >= 0 -- RG) */
        if (a >= MAX_COLOR) return (TERM_WHITE);

        /* Use this color */
        return (a);
    }

    /* Standard "color" */
    return (TERM_WHITE);
}


/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
u16b bolt_pict(int y, int x, int ny, int nx, int typ)
{
    int base;

    byte k;

    byte a;
    char c;

    /* No motion (*) */
    if ((ny == y) && (nx == x)) base = 0x30;

    /* Vertical (|) */
    else if (nx == x) base = 0x40;

    /* Horizontal (-) */
    else if (ny == y) base = 0x50;

    /* Diagonal (/) */
    else if ((ny - y) == (x - nx)) base = 0x60;

    /* Diagonal (\) */
    else if ((ny - y) == (nx - x)) base = 0x70;

    /* Weird (*) */
    else base = 0x30;

    /* Basic spell color */
    k = spell_color(typ);

    /* Obtain attr/char */
    a = misc_to_attr[base + k];
    c = misc_to_char[base + k];

    /* Create pict */
    return (PICT(a, c));
}


/*
 * Determine the path taken by a projection.
 *
 * The projection will always start from the grid (y1,x1), and will travel
 * towards the grid (y2,x2), touching one grid per unit of distance along
 * the major axis, and stopping when it enters the destination grid or a
 * wall grid, or has travelled the maximum legal distance of "range".
 *
 * Note that "distance" in this function (as in the "update_view()" code)
 * is defined as "MAX(dy,dx) + MIN(dy,dx)/2", which means that the player
 * actually has an "octagon of projection" not a "circle of projection".
 *
 * The path grids are saved into the grid array pointed to by "gp", and
 * there should be room for at least "range" grids in "gp". Note that
 * due to the way in which distance is calculated, this function normally
 * uses fewer than "range" grids for the projection path, so the result
 * of this function should never be compared directly to "range". Note
 * that the initial grid (y1,x1) is never saved into the grid array, not
 * even if the initial grid is also the final grid. XXX XXX XXX
 *
 * The "flg" flags can be used to modify the behavior of this function.
 *
 * In particular, the "PROJECT_STOP" and "PROJECT_THRU" flags have the same
 * semantics as they do for the "project" function, namely, that the path
 * will stop as soon as it hits a monster, or that the path will continue
 * through the destination grid, respectively.
 *
 * The "PROJECT_JUMP" flag, which for the "project()" function means to
 * start at a special grid (which makes no sense in this function), means
 * that the path should be "angled" slightly if needed to avoid any wall
 * grids, allowing the player to "target" any grid which is in "view".
 * This flag is non-trivial and has not yet been implemented, but could
 * perhaps make use of the "vinfo" array (above). XXX XXX XXX
 *
 * This function returns the number of grids (if any) in the path. This
 * function will return zero if and only if (y1,x1) and (y2,x2) are equal.
 *
 * This algorithm is similar to, but slightly different from, the one used
 * by "update_view_los()", and very different from the one used by "los()".
 */
sint project_path_old(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg)
{
    int y, x;

    int n = 0;
    int k = 0;

    /* Absolute */
    int ay, ax;

    /* Offsets */
    int sy, sx;

    /* Fractions */
    int frac;

    /* Scale factors */
    int full, half;

    /* Slope */
    int m;

    /* No path necessary (or allowed) */
    if ((x1 == x2) && (y1 == y2)) return (0);


    /* Analyze "dy" */
    if (y2 < y1)
    {
        ay = (y1 - y2);
        sy = -1;
    }
    else
    {
        ay = (y2 - y1);
        sy = 1;
    }

    /* Analyze "dx" */
    if (x2 < x1)
    {
        ax = (x1 - x2);
        sx = -1;
    }
    else
    {
        ax = (x2 - x1);
        sx = 1;
    }


    /* Number of "units" in one "half" grid */
    half = (ay * ax);

    /* Number of "units" in one "full" grid */
    full = half << 1;

    /* Vertical */
    if (ay > ax)
    {
        /* Let m = ((dx/dy) * full) = (dx * dx * 2) */
        m = ax * ax * 2;

        /* Start */
        y = y1 + sy;
        x = x1;

        frac = m;

        if (frac > half)
        {
            /* Advance (X) part 2 */
            x += sx;

            /* Advance (X) part 3 */
            frac -= full;

            /* Track distance */
            k++;
        }

        /* Create the projection path */
        while (1)
        {
            /* Save grid */
            gp[n++] = GRID(y, x);

            /* Hack -- Check maximum range */
            if ((n + (k >> 1)) >= range) break;

            /* Sometimes stop at destination grid */
            if (!(flg & (PROJECT_THRU)))
            {
                if ((x == x2) && (y == y2)) break;
            }

            if (flg & (PROJECT_DISI))
            {
                if ((n > 0) && cave_stop_disintegration(y, x)) break;
            }
            else if (flg & (PROJECT_LOS))
            {
                if ((n > 0) && !cave_los_bold(y, x)) break;
            }
            else if (!(flg & (PROJECT_PATH)))
            {
                /* Always stop at non-initial wall grids */
                if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
            }

            /* Sometimes stop at non-initial monsters/players */
            if (flg & (PROJECT_STOP))
            {
                if ((n > 0) &&
                    (player_bold(y, x) || cave[y][x].m_idx != 0))
                    break;
            }

            if (!in_bounds(y, x)) break;

            /* Slant */
            if (m)
            {
                /* Advance (X) part 1 */
                frac += m;

                /* Horizontal change */
                if (frac > half)
                {
                    /* Advance (X) part 2 */
                    x += sx;

                    /* Advance (X) part 3 */
                    frac -= full;

                    /* Track distance */
                    k++;
                }
            }

            /* Advance (Y) */
            y += sy;
        }
    }

    /* Horizontal */
    else if (ax > ay)
    {
        /* Let m = ((dy/dx) * full) = (dy * dy * 2) */
        m = ay * ay * 2;

        /* Start */
        y = y1;
        x = x1 + sx;

        frac = m;

        /* Vertical change */
        if (frac > half)
        {
            /* Advance (Y) part 2 */
            y += sy;

            /* Advance (Y) part 3 */
            frac -= full;

            /* Track distance */
            k++;
        }

        /* Create the projection path */
        while (1)
        {
            /* Save grid */
            gp[n++] = GRID(y, x);

            /* Hack -- Check maximum range */
            if ((n + (k >> 1)) >= range) break;

            /* Sometimes stop at destination grid */
            if (!(flg & (PROJECT_THRU)))
            {
                if ((x == x2) && (y == y2)) break;
            }

            if (flg & (PROJECT_DISI))
            {
                if ((n > 0) && cave_stop_disintegration(y, x)) break;
            }
            else if (flg & (PROJECT_LOS))
            {
                if ((n > 0) && !cave_los_bold(y, x)) break;
            }
            else if (!(flg & (PROJECT_PATH)))
            {
                /* Always stop at non-initial wall grids */
                if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
            }

            /* Sometimes stop at non-initial monsters/players */
            if (flg & (PROJECT_STOP))
            {
                if ((n > 0) &&
                    (player_bold(y, x) || cave[y][x].m_idx != 0))
                    break;
            }

            if (!in_bounds(y, x)) break;

            /* Slant */
            if (m)
            {
                /* Advance (Y) part 1 */
                frac += m;

                /* Vertical change */
                if (frac > half)
                {
                    /* Advance (Y) part 2 */
                    y += sy;

                    /* Advance (Y) part 3 */
                    frac -= full;

                    /* Track distance */
                    k++;
                }
            }

            /* Advance (X) */
            x += sx;
        }
    }

    /* Diagonal */
    else
    {
        /* Start */
        y = y1 + sy;
        x = x1 + sx;

        /* Create the projection path */
        while (1)
        {
            /* Save grid */
            gp[n++] = GRID(y, x);

            /* Hack -- Check maximum range */
            if ((n + (n >> 1)) >= range) break;

            /* Sometimes stop at destination grid */
            if (!(flg & (PROJECT_THRU)))
            {
                if ((x == x2) && (y == y2)) break;
            }

            if (flg & (PROJECT_DISI))
            {
                if ((n > 0) && cave_stop_disintegration(y, x)) break;
            }
            else if (flg & (PROJECT_LOS))
            {
                if ((n > 0) && !cave_los_bold(y, x)) break;
            }
            else if (!(flg & (PROJECT_PATH)))
            {
                /* Always stop at non-initial wall grids */
                if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
            }

            /* Sometimes stop at non-initial monsters/players */
            if (flg & (PROJECT_STOP))
            {
                if ((n > 0) &&
                    (player_bold(y, x) || cave[y][x].m_idx != 0))
                    break;
            }

            if (!in_bounds(y, x)) break;

            /* Advance (Y) */
            y += sy;

            /* Advance (X) */
            x += sx;
        }
    }

    /* Length */
    return (n);
}

/* Check whether or not to continue this projection */
static bool _path_continue(int n, int y, int x, int flg)
{
    if (flg & (PROJECT_DISI))
    {
        if ((n > 0) && cave_stop_disintegration(y, x)) return FALSE;
    }
    else if (flg & (PROJECT_LOS))
    {
        if ((n > 0) && !cave_los_bold(y, x)) return FALSE;
    }
    else /*if (!(flg & (PROJECT_PATH))) ?? This makes the printed path differ from the actual path!!!*/
    {
        /* Always stop at non-initial wall grids */
        if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) return FALSE;
    }

    /* Sometimes stop at non-initial monsters/players */
    if (flg & (PROJECT_STOP))
    {
        if ((n > 0) &&
            (player_bold(y, x) || cave[y][x].m_idx != 0))
            return FALSE;
    }

    return TRUE;
}

/* Try a given projection with horizontal major axis
   These projections handle arbitrary rational slope values by the use of integer
   counters. The steps are always +-1 along the major axis, but may be any of -1,0,+1
   along the minor axis. The start values are used to select different tile paths between
   the end points whenever there might be any ambiguity. We return whether or not the
   desired endpoint was reached, but note that the path may extend beyond this point.
*/
static bool _project_path_x(u16b *path, int* ct, int range, int y1, int x1, int y2, int x2,
    int step_x, int step_y, int count_x, int count_y, int start_cx, int start_cy,
    int flg)
{
    int cx = start_cx;
    int cy = start_cy;
    int x = x1;
    int y = y1;
    int n = 0;
    int k = 0;
    bool result = FALSE;

    for (;;)
    {
        --cx;
        --cy;
        if (cx <= 0) /* cx < 0 should never occur */
        {
            x += step_x;
            cx += count_x;

            if (cy <= 0) /* cy < 0 is to be expected and quite common */
            {
                y += step_y;
                cy += count_y;
                if (step_y) ++k; /* k tracks units of (SQRT(2) - 1) in distance ... we fudge this as .5 */
            }

            if (!in_bounds(y, x)) break;

            path[n++] = GRID(y, x);

            if (x == x2 && y == y2)
            {
                result = TRUE;
                if (!(flg & PROJECT_THRU)) break;
            }

            if ((n + (k >> 1)) >= range) break;

            if (!_path_continue(n, y, x, flg)) break;
        }
    }

    *ct = n;
    return result;
}

/* Try a given projection with vertical major axis */
static bool _project_path_y(u16b *path, int *ct, int range, int y1, int x1, int y2, int x2,
    int step_x, int step_y, int count_x, int count_y, int start_cx, int start_cy,
    int flg)
{
    int cx = start_cx;
    int cy = start_cy;
    int x = x1;
    int y = y1;
    int n = 0;
    int k = 0;
    bool result = FALSE;

    for (;;)
    {
        --cx;
        --cy;
        if (cy <= 0) /* cy < 0 should never occur */
        {
            y += step_y;
            cy += count_y;

            if (cx <= 0) /* cx < 0 is to be expected and quite common */
            {
                x += step_x;
                cx += count_x;
                if (step_x) ++k; /* k tracks units of (SQRT(2) - 1) in distance ... we fudge this as .5 */
            }

            if (!in_bounds(y, x)) break;

            path[n++] = GRID(y, x);

            if (x == x2 && y == y2)
            {
                result = TRUE;
                if (!(flg & PROJECT_THRU)) break;
            }

            if ((n + (k >> 1)) >= range) break;

            if (!_path_continue(n, y, x, flg)) break;
        }
    }

    *ct = n;
    return result;
}

static void _copy_path(u16b* dest, u16b* src, int ct)
{
    int i;
    for (i = 0; i < ct; ++i)
        dest[i] = src[i];
}

/* Lots of attempts have been made to fix asymmetrical targeting. Here is my
   attempt. Simply put, there are multiple ways to discretize a line into a
   sequence of grids. We will simply try alternate discretizations should a
   given one fail. */
static int project_path_new(u16b *path, int range, int y1, int x1, int y2, int x2, int flg)
{
    int dx, dy, ax, ay;
    int n = 0;

    /* No path necessary (or allowed) */
    if ((x1 == x2) && (y1 == y2)) return 0;

    dx = x2 - x1;
    ax = ABS(dx);

    dy = y2 - y1;
    ay = ABS(dy);

    /* Case 1: Horizontal */
    if (ay == 0)
    {
        int step_x = (dx > 0) ? 1 : -1;
        _project_path_x(path, &n, range, y1, x1, y2, x2, step_x, 0, 1, 1, 1, 1, flg);
    }
    /* Case 2: Vertical */
    else if (ax == 0)
    {
        int step_y = (dy > 0) ? 1 : -1;
        _project_path_y(path, &n, range, y1, x1, y2, x2, 0, step_y, 1, 1, 1, 1, flg);
    }
    /* Case 3: Primary Axis Horizontal */
    else if (ax > ay)
    {
        int step_x = (dx > 0) ? 1 : -1;
        int step_y = (dy > 0) ? 1 : -1;
        int count_x = ay;
        int count_y = ax;
        int start_cx = count_x;
        int start_cy = (count_y + 1)/2;

        if (count_y % 2 == 1)
        {
            _project_path_x(path, &n, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx, start_cy, flg);
        }
        else
        {
        int ct2 = 0;
        u16b path2[512];
            if (!_project_path_x(path, &n, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx, start_cy, flg))
            {
                /* Take the second path if it succeeds, or if it fails but is longer */
                if ( _project_path_x(path2, &ct2, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx, start_cy + 1, flg)
                  || ct2 > n )
                {
                    _copy_path(path, path2, ct2);
                    n = ct2;
                }
            }
        }
    }
    /* Case 4: Primary Axis Vertical */
    else if (ay > ax)
    {
        int step_x = (dx > 0) ? 1 : -1;
        int step_y = (dy > 0) ? 1 : -1;
        int count_x = ay;
        int count_y = ax;
        int start_cx = (count_x + 1)/2;
        int start_cy = count_y;

        if (count_x % 2 == 1)
        {
            _project_path_y(path, &n, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx, start_cy, flg);
        }
        else
        {
        int ct2 = 0;
        u16b path2[512];
            if (!_project_path_y(path, &n, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx, start_cy, flg))
            {
                /* Take the second path if it succeeds, or if it fails but is longer */
                if ( _project_path_y(path2, &ct2, range, y1, x1, y2, x2, step_x, step_y, count_x, count_y, start_cx + 1, start_cy, flg)
                  || ct2 > n )
                {
                    _copy_path(path, path2, ct2);
                    n = ct2;
                }
            }
        }
    }
    /* Case 5: Exact Diagonal */
    else
    {
        int step_x = (dx > 0) ? 1 : -1;
        int step_y = (dy > 0) ? 1 : -1;
        _project_path_x(path, &n, range, y1, x1, y2, x2, step_x, step_y, 1, 1, 1, 1, flg);
    }

    return n;
}

sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg)
{
    return project_path_new(gp, range, y1, x1, y2, x2, flg);
    /*return project_path_old(gp, range, y1, x1, y2, x2, flg);*/
}

/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;
/* Mega-Hack -- monsters target */
s16b monster_target_x;
s16b monster_target_y;


/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * XXX XXX XXX Perhaps we should affect doors?
 */
static bool project_f(int who, int r, int y, int x, int dam, int typ)
{
    cave_type       *c_ptr = &cave[y][x];
    feature_type    *f_ptr = &f_info[c_ptr->feat];

    bool obvious = FALSE;
    bool known = player_has_los_bold(y, x);


    /* XXX XXX XXX */
    who = who ? who : 0;

    /* Reduce damage by distance */
    dam = (dam + r) / (r + 1);

    if (have_flag(f_ptr->flags, FF_WEB))
    {
        cptr message = NULL;
        switch (typ)
        {
        case GF_FIRE:
        case GF_ELEC:
        case GF_PLASMA:
            message = "burns up.";
            break;
        case GF_METEOR:
        case GF_CHAOS:
        case GF_MANA:
        case GF_SHARDS:
        case GF_ROCK:
        case GF_ROCKET:
        case GF_FORCE:
            message = "is blasted.";
            break;
        case GF_ACID:
            message = "melts.";
            break;
        case GF_COLD:
        case GF_ICE:
            message = "is frozen.";
            break;
        case GF_PSY_SPEAR:
        case GF_LITE:
            message = "shrivels in the light.";
            break;
        case GF_STORM:
            message = "is washed away.";
            break;
        case GF_WATER:
        case GF_WATER2:
            message = "is washed away.";
            break;
        case GF_GRAVITY:
            message = "disappears.";
            break;
        }
        if (message)
        {
            msg_format("The web %s", message);
            cave_set_feat(y, x, floor_type[randint0(100)]);
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;
        }
    }

    if (have_flag(f_ptr->flags, FF_TREE))
    {
        cptr message;
        switch (typ)
        {
        case GF_DEATH_TOUCH:
            message = "dies.";break;

        case GF_POIS:
        case GF_NUKE:
        case GF_DEATH_RAY:
            message = "was blasted.";break;
        case GF_TIME:
            message = "shrank.";break;
        case GF_ACID:
            message = "melted.";break;
        case GF_COLD:
        case GF_ICE:
            if (have_flag(f_ptr->flags, FF_SNOW)) message = NULL;
            else message = "was frozen and smashed.";
            break;
        case GF_FIRE:
        case GF_ELEC:
        case GF_PLASMA:
        {
            if (dungeon_type == DUNGEON_AUSSIE) /* Aussie rules fireball */
            {
                switch (randint0(3))
                {
                    case 0: message = NULL; break;
                    case 1: message = "burns up!"; break;
                    default:
                        if (!cave[y][x].m_idx)
                        {
                            message = "catches fire!";
                            cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);
                            summon_named_creature(0, y, x, MON_BUSH, 0);
                        }
                        else message = "burns up!";
                        break;
                }
            }
            else message = "burns up!";
            break;
        }
        case GF_METEOR:
        case GF_CHAOS:
        case GF_MANA:
        case GF_SEEKER:
        case GF_SUPER_RAY:
        case GF_SHARDS:
        case GF_ROCK:
        case GF_ROCKET:
        case GF_SOUND:
        case GF_DISENCHANT:
        case GF_FORCE:
        case GF_GRAVITY:
            message = "was crushed.";break;
        default:
            message = NULL;break;
        }
        if (message)
        {
            msg_format("A tree %s", message);
            if (have_flag(f_ptr->flags, FF_SNOW))
            {
                if ((typ == GF_FIRE) || (typ == GF_ELEC) || (typ == GF_PLASMA))
                {
                    cave_alter_feat(y, x, FF_HURT_FIRE);
                }
                else cave_set_feat(y, x, feat_snow_floor);
            }
            else cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);

            /* Observe */
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;
        }
    }

    /* Analyze the type */
    switch (typ)
    {
        /* Ignore most effects */
        case GF_CAPTURE:
        case GF_HAND_DOOM:
        case GF_CAUSE_1:
        case GF_CAUSE_2:
        case GF_CAUSE_3:
        case GF_CAUSE_4:
        case GF_MIND_BLAST:
        case GF_BRAIN_SMASH:
        case GF_DRAIN_MANA:
        case GF_PSY_SPEAR:
        case GF_FORCE:
        case GF_HOLY_FIRE:
        case GF_HELL_FIRE:
        case GF_PSI:
        case GF_PSI_DRAIN:
        case GF_TELEKINESIS:
        case GF_DOMINATION:
        case GF_IDENTIFY:
        case GF_ATTACK:
        case GF_ACID:
        case GF_ELEC:
        case GF_COLD:
        case GF_ICE:
        case GF_PLASMA:
        case GF_METEOR:
        case GF_CHAOS:
        case GF_MANA:
        case GF_SEEKER:
        case GF_SUPER_RAY:
        {
            break;
        }

        /* Destroy Traps (and Locks) */
        case GF_KILL_TRAP:
        {
            /* Reveal secret doors */
            if (is_hidden_door(c_ptr))
            {
                /* Pick a door */
                disclose_grid(y, x);

                /* Check line of sight */
                if (known)
                {
                    obvious = TRUE;
                }
            }

            /* Destroy traps */
            if (is_trap(c_ptr->feat))
            {
                /* Check line of sight */
                if (known)
                {
                    msg_print("There is a bright flash of light!");

                    obvious = TRUE;
                }

                /* Destroy the trap */
                cave_alter_feat(y, x, FF_DISARM);
            }

            /* Locked doors are unlocked */
            if (is_closed_door(c_ptr->feat) && f_ptr->power && have_flag(f_ptr->flags, FF_OPEN))
            {
                s16b old_feat = c_ptr->feat;

                /* Unlock the door */
                cave_alter_feat(y, x, FF_DISARM);

                /* Check line of sound */
                if (known && (old_feat != c_ptr->feat))
                {
                    msg_print("Click!");

                    obvious = TRUE;
                }
            }

            /* Remove "unsafe" flag if player is not blind */
            if (!p_ptr->blind && player_has_los_bold(y, x))
            {
                c_ptr->info &= ~(CAVE_UNSAFE);

                /* Redraw */
                lite_spot(y, x);

                obvious = TRUE;
            }

            break;
        }

        /* Destroy Doors (and traps) */
        case GF_KILL_DOOR:
        case GF_REMOVE_OBSTACLE:
        {
            /* Destroy all doors and traps */
            if (is_trap(c_ptr->feat) || have_flag(f_ptr->flags, FF_DOOR) ||
                (typ == GF_REMOVE_OBSTACLE && have_flag(f_ptr->flags, FF_TREE)))
            {
                /* Check line of sight */
                if (known)
                {
                    /* Message */
                    msg_print("There is a bright flash of light!");

                    obvious = TRUE;
                }

                /* Destroy the feature */
                cave_alter_feat(y, x, FF_TUNNEL);
            }

            /* Remove "unsafe" flag if player is not blind */
            if (!p_ptr->blind && player_has_los_bold(y, x))
            {
                c_ptr->info &= ~(CAVE_UNSAFE);

                /* Redraw */
                lite_spot(y, x);

                obvious = TRUE;
            }

            break;
        }

        case GF_JAM_DOOR: /* Jams a door (as if with a spike) */
        {
            if (have_flag(f_ptr->flags, FF_SPIKE))
            {
                s16b old_mimic = c_ptr->mimic;
                feature_type *mimic_f_ptr = &f_info[get_feat_mimic(c_ptr)];

                cave_alter_feat(y, x, FF_SPIKE);

                c_ptr->mimic = old_mimic;

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                /* Check line of sight */
                if (known && have_flag(mimic_f_ptr->flags, FF_OPEN))
                {
                    /* Message */
                    msg_format("The %s seems stuck.", f_name + mimic_f_ptr->name);

                    obvious = TRUE;
                }
            }
            break;
        }

        /* Destroy walls (and doors) */
        case GF_KILL_WALL:
        {
            if (have_flag(f_ptr->flags, FF_HURT_ROCK))
            {
                /* Message */
                if (known && (c_ptr->info & (CAVE_MARK)))
                {
                    msg_format("The %s turns into mud!", f_name + f_info[get_feat_mimic(c_ptr)].name);

                    obvious = TRUE;
                }

                /* Destroy the wall */
                cave_alter_feat(y, x, FF_HURT_ROCK);

                /* Update some things */
                p_ptr->update |= (PU_FLOW);
            }

            break;
        }

        case GF_FIRE:
        {
            if ((have_flag(f_ptr->flags, FF_HURT_FIRE)) && (dam > 15) && (!have_flag(f_ptr->flags, FF_PERMANENT)))
            {
                /* Message */
                if (known && (c_ptr->info & (CAVE_MARK)))
                {
                    msg_format("The %s melts!", f_name + f_info[get_feat_mimic(c_ptr)].name);

                    obvious = TRUE;
                }

                /* Destroy the feature */
                cave_alter_feat(y, x, FF_HURT_FIRE);

                /* Update some things */
                p_ptr->update |= (PU_FLOW);
            }

            break;
        }

        /* Make doors */
        case GF_MAKE_DOOR:
        {
            /* Require a "naked" floor grid */
            if (!cave_naked_bold(y, x)) break;

            /* Not on the player */
            if (player_bold(y, x)) break;

            /* Create a closed door */
            cave_set_feat(y, x, feat_door[DOOR_DOOR].closed);

            /* Observe */
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;

            break;
        }

        /* Make traps */
        case GF_MAKE_TRAP:
        {
            /* Place a trap */
            place_trap(y, x);

            break;
        }

        case GF_WEB:
        {
            if (!cave_clean_bold(y,x) || cave[y][x].m_idx) break;
            cave_set_feat(y, x, feat_web);
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;
            break;
        }

        /* Make doors */
        case GF_MAKE_TREE:
        {
            /* Require a "naked" floor grid */
            if (!cave_naked_bold(y, x)) break;

            /* Not on the player */
            if (player_bold(y, x)) break;

            /* Create a closed door */
            cave_set_feat(y, x, feat_tree);

            /* Observe */
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;


            break;
        }

        case GF_MAKE_GLYPH:
        {
            /* Require a "naked" floor grid */
            if (!cave_naked_bold(y, x)) break;

            /* Create a glyph */
            c_ptr->info |= CAVE_OBJECT;
            c_ptr->mimic = feat_glyph;

            /* Notice */
            note_spot(y, x);

            /* Redraw */
            lite_spot(y, x);

            break;
        }

        case GF_MAKE_WALL:
        {
            /* Require a "naked" floor grid */
            if (!cave_naked_bold(y, x)) break;

            /* Not on the player */
            if (player_bold(y, x)) break;

            /* Place a wall */
            cave_set_feat(y, x, feat_granite);

            /* Observe */
            if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;

            break;
        }


        case GF_LAVA_FLOW:
        {
            /* Ignore permanent grid */
            if (have_flag(f_ptr->flags, FF_PERMANENT)) break;

            /* Shallow Lava */
            if (dam == 1)
            {
                /* Ignore grid without enough space */
                if (!have_flag(f_ptr->flags, FF_FLOOR)) break;

                /* Place a shallow lava */
                cave_set_feat(y, x, feat_shallow_lava);
            }
            /* Deep Lava */
            else if (dam)
            {
                /* Place a deep lava */
                cave_set_feat(y, x, feat_deep_lava);
            }
            break;
        }

        case GF_WATER2:
            if (have_flag(f_ptr->flags, FF_PERMANENT)) break;
            if (randint0(50) > p_ptr->lev) break;
            if (randint0(1000) < dam)
            {
                cave_set_feat(y, x, feat_deep_water);
            }
            else
            {
                if (!have_flag(f_ptr->flags, FF_FLOOR)) break;
                cave_set_feat(y, x, feat_shallow_water);
            }
            break;

        case GF_WATER_FLOW:
        {
            /* Ignore permanent grid */
            if (have_flag(f_ptr->flags, FF_PERMANENT)) break;

            /* Shallow Water */
            if (dam == 1)
            {
                /* Ignore grid without enough space */
                if (!have_flag(f_ptr->flags, FF_FLOOR)) break;

                /* Place a shallow water */
                cave_set_feat(y, x, feat_shallow_water);
            }
            /* Deep Water */
            else if (dam)
            {
                /* Place a deep water */
                cave_set_feat(y, x, feat_deep_water);
            }
            break;
        }

        /* Lite up the grid */
        case GF_LITE_WEAK:
        case GF_LITE:
        {
            /* Turn on the light */
            if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS))
            {
                c_ptr->info |= (CAVE_GLOW);

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                update_local_illumination(y, x);

                /* Observe */
                if (player_can_see_bold(y, x)) obvious = TRUE;

                /* Mega-Hack -- Update the monster in the affected grid */
                /* This allows "spear of light" (etc) to work "correctly" */
                if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

                if (p_ptr->special_defense & NINJA_S_STEALTH)
                {
                    if (player_bold(y, x)) set_superstealth(FALSE);
                }
                if (prace_is_(RACE_MON_VAMPIRE))
                    vampire_check_light_status();
            }

            break;
        }

        /* Darken the grid */
        case GF_DARK_WEAK:
        case GF_DARK:
        {
            bool do_dark = !p_ptr->inside_battle && !is_mirror_grid(c_ptr);
            int j;

            /* Turn off the light. */
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

                    if (!do_dark) break;
                }

                c_ptr->info &= ~(CAVE_GLOW);

                /* Hack -- Forget "boring" grids */
                if (!have_flag(f_ptr->flags, FF_REMEMBER))
                {
                    /* Forget */
                    c_ptr->info &= ~(CAVE_MARK);

                    /* Notice */
                    note_spot(y, x);
                }

                /* Redraw */
                lite_spot(y, x);

                update_local_illumination(y, x);

                /* Notice */
                if (player_can_see_bold(y, x)) obvious = TRUE;

                /* Mega-Hack -- Update the monster in the affected grid */
                /* This allows "spear of light" (etc) to work "correctly" */
                if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

                if (prace_is_(RACE_MON_VAMPIRE))
                    vampire_check_light_status();
            }

            /* All done */
            break;
        }

        case GF_SHARDS:
        case GF_ROCK:
        case GF_ROCKET:
        case GF_CHICKEN:
        case GF_BOMB:
        {
            if (is_mirror_grid(c_ptr))
            {
                msg_print("The mirror was crashed!");
                sound(SOUND_GLASS);
                remove_mirror(y, x);
                project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS, (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP));
            }

            if (have_flag(f_ptr->flags, FF_GLASS) && !have_flag(f_ptr->flags, FF_PERMANENT) && (dam >= 50))
            {
                /* Message */
                if (known && (c_ptr->info & CAVE_MARK))
                {
                    msg_format("The %s was crashed!", f_name + f_info[get_feat_mimic(c_ptr)].name);
                    sound(SOUND_GLASS);
                }

                /* Destroy the wall */
                cave_alter_feat(y, x, FF_HURT_ROCK);

                /* Update some things */
                p_ptr->update |= (PU_FLOW);
            }
            break;
        }

        case GF_SOUND:
        {
            if (is_mirror_grid(c_ptr) && p_ptr->lev < 40)
            {
                msg_print("The mirror was crashed!");
                sound(SOUND_GLASS);
                remove_mirror(y, x);
                project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS, (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP));
            }

            if (have_flag(f_ptr->flags, FF_GLASS) && !have_flag(f_ptr->flags, FF_PERMANENT) && (dam >= 200))
            {
                /* Message */
                if (known && (c_ptr->info & CAVE_MARK))
                {
                    msg_format("The %s was crashed!", f_name + f_info[get_feat_mimic(c_ptr)].name);
                    sound(SOUND_GLASS);
                }

                /* Destroy the wall */
                cave_alter_feat(y, x, FF_HURT_ROCK);

                /* Update some things */
                p_ptr->update |= (PU_FLOW);
            }
            break;
        }

        case GF_DISINTEGRATE:
        {
            /* Destroy mirror/glyph */
            if (is_mirror_grid(c_ptr) || is_glyph_grid(c_ptr) || is_mon_trap_grid(c_ptr))
                remove_mirror(y, x);

            /* Permanent features don't get effect */
            /* But not protect monsters and other objects */
            if (have_flag(f_ptr->flags, FF_HURT_DISI) && !have_flag(f_ptr->flags, FF_PERMANENT))
            {
                cave_alter_feat(y, x, FF_HURT_DISI);

                /* Update some things -- similar to GF_KILL_WALL */
                p_ptr->update |= (PU_FLOW);
            }
            break;
        }
    }

    lite_spot(y, x);
    /* Return "Anything seen?" */
    return (obvious);
}



/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool project_o(int who, int r, int y, int x, int dam, int typ)
{
    cave_type *c_ptr = &cave[y][x];

    s16b this_o_idx, next_o_idx = 0;

    bool obvious = FALSE;
    bool known = player_has_los_bold(y, x);

    u32b flgs[OF_ARRAY_SIZE];

    char o_name[MAX_NLEN];

    int k_idx = 0;
    bool is_potion = FALSE;


    /* XXX XXX XXX */
    who = who ? who : 0;

    /* Reduce damage by distance */
    dam = (dam + r) / (r + 1);


    /* Scan all objects in the grid */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        /* Acquire object */
        object_type *o_ptr = &o_list[this_o_idx];

        bool is_art = FALSE;
        bool ignore = FALSE;
        bool do_kill = FALSE;

        cptr note_kill = NULL;

        /* Get the "plural"-ness */
        bool plural = object_plural(o_ptr);

        if (!o_ptr->number) continue;

        /* Acquire next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Extract the flags */
        obj_flags(o_ptr, flgs);

        /* Check for artifact */
        if (object_is_artifact(o_ptr)) is_art = TRUE;
        else if (o_ptr->name2 == EGO_AMMO_ENDURANCE) is_art = TRUE; /* lazy */

        /* Analyze the type */
        switch (typ)
        {
        case GF_ACID:
            if (hates_acid(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " melt!" : " melts!");
                if (have_flag(flgs, OF_IGNORE_ACID)) ignore = TRUE;
            }
            break;
        case GF_ELEC:
            if (hates_elec(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " are destroyed!" : " is destroyed!");
                if (have_flag(flgs, OF_IGNORE_ELEC)) ignore = TRUE;
            }
            break;
        case GF_FIRE:
            if (hates_fire(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " burn up!" : " burns up!");
                if (have_flag(flgs, OF_IGNORE_FIRE)) ignore = TRUE;
            }
            break;
        case GF_COLD:
            if (hates_cold(o_ptr))
            {
                note_kill = (plural ? " shatter!" : " shatters!");
                do_kill = TRUE;
                if (have_flag(flgs, OF_IGNORE_COLD)) ignore = TRUE;
            }
            break;
        case GF_PLASMA:
            if (hates_fire(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " burn up!" : " burns up!");
                if (have_flag(flgs, OF_IGNORE_FIRE)) ignore = TRUE;
            }
            if (hates_elec(o_ptr))
            {
                ignore = FALSE;
                do_kill = TRUE;
                note_kill = (plural ? " are destroyed!" : " is destroyed!");
                if (have_flag(flgs, OF_IGNORE_ELEC)) ignore = TRUE;
            }
            break;
        case GF_METEOR:
            if (hates_fire(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " burn up!" : " burns up!");
                if (have_flag(flgs, OF_IGNORE_FIRE)) ignore = TRUE;
            }
            if (hates_cold(o_ptr))
            {
                ignore = FALSE;
                do_kill = TRUE;
                note_kill = (plural ? " shatter!" : " shatters!");
                if (have_flag(flgs, OF_IGNORE_COLD)) ignore = TRUE;
            }
            break;
        case GF_ICE:
        case GF_SHARDS:
        case GF_ROCK:
        case GF_FORCE:
        case GF_SOUND:
        case GF_BOMB:
            if (hates_cold(o_ptr))
            {
                note_kill = (plural ? " shatter!" : " shatters!");
                do_kill = TRUE;
            }
            break;
        case GF_MANA:
        case GF_SEEKER:
        case GF_SUPER_RAY:
            do_kill = TRUE;
            note_kill = (plural ? " are destroyed!" : " is destroyed!");
            break;
        case GF_DISINTEGRATE:
            do_kill = TRUE;
            note_kill = (plural ? " evaporate!" : " evaporates!");
            break;
        case GF_CHAOS:
            do_kill = TRUE;
            note_kill = (plural ? " are destroyed!" : " is destroyed!");
            if (have_flag(flgs, OF_RES_CHAOS)) ignore = TRUE;
            break;
        case GF_HOLY_FIRE:
        case GF_HELL_FIRE:
            if (object_is_cursed(o_ptr))
            {
                do_kill = TRUE;
                note_kill = (plural ? " are destroyed!" : " is destroyed!");
            }
            break;
        case GF_IDENTIFY:
            identify_item(o_ptr);

            /* Auto-inscription */
            autopick_alter_obj(o_ptr, FALSE);
            break;
    case GF_KILL_TRAP:
    case GF_KILL_DOOR:
    case GF_REMOVE_OBSTACLE:
            /* Chests are noticed only if trapped or locked */
            if (o_ptr->tval == TV_CHEST)
            {
                /* Disarm/Unlock traps */
                if (o_ptr->pval > 0)
                {
                    /* Disarm or Unlock */
                    o_ptr->pval = (0 - o_ptr->pval);

                    /* Identify */
                    obj_identify(o_ptr);

                    /* Notice */
                    if (known && (o_ptr->marked & OM_FOUND))
                    {
                        msg_print("Click!");
                        obvious = TRUE;
                    }
                }
            }
            break;
        case GF_ANIM_DEAD:
            if (o_ptr->tval == TV_CORPSE)
            {
                int i;
                u32b mode = 0L;

                if (!who || is_pet(&m_list[who]))
                    mode |= PM_FORCE_PET;

                for (i = 0; i < o_ptr->number ; i++)
                {
                    if (((o_ptr->sval == SV_CORPSE) && (randint1(100) > 80)) ||
                        ((o_ptr->sval == SV_SKELETON) && (randint1(100) > 60)) ||
                        (mon_is_quest_target(o_ptr->pval)))
                    {
                        if (!note_kill)
                            note_kill = (plural ? " become dust." : " becomes dust.");
                        continue;
                    }
                    else if (summon_named_creature(who, y, x, o_ptr->pval, mode))
                    {
                        note_kill = " revived.";
                    }
                    else if (!note_kill)
                    {
                        note_kill = (plural ? " become dust." : " becomes dust.");
                    }
                }
                do_kill = TRUE;
                obvious = TRUE;
            }
            break;
        }


        /* Attempt to destroy the object */
        if (do_kill)
        {
            /* Effect "observed" */
            if (known && (o_ptr->marked & OM_FOUND))
            {
                obvious = TRUE;
                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED));
            }

            /* Artifacts, and other objects, get to resist */
            if (is_art || ignore)
            {
                /* Observe the resist */
                if (known && (o_ptr->marked & OM_FOUND))
                {
                    msg_format("The %s %s unaffected!",
                            o_name, (plural ? "are" : "is"));
                }
            }

            /* Kill it */
            else
            {
                /* Describe if needed */
                if (known && (o_ptr->marked & OM_FOUND) && note_kill)
                {
                    msg_format("The %s%s", o_name, note_kill);
                    stats_on_m_destroy(o_ptr, o_ptr->number);
                }

                k_idx = o_ptr->k_idx;
                is_potion = object_is_potion(o_ptr);


                /* Delete the object */
                delete_object_idx(this_o_idx);

                /* Potions produce effects when 'shattered' */
                if (is_potion)
                {
                    (void)potion_smash_effect(who, y, x, k_idx);
                }

                /* Redraw */
                lite_spot(y, x);
            }
        }
    }

    /* Return "Anything seen?" */
    return (obvious);
}


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero. Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer. XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less. If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint1(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint1(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint1(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint1(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint1(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL. So,
 * to make a spell have "no effect" just set "note" to NULL. You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
/* "flg" was added. */
bool project_m(int who, int r, int y, int x, int dam, int typ, int flg, bool see_s_msg)
{
    bool result;
    int  m_idx = cave[y][x].m_idx;
    u32b liput = GF_AFFECT_SPELL;

    if (!m_idx) return FALSE;

    /* Reduce damage by distance */
    if ((!(flg & PROJECT_FULL_DAM)) && (typ != GF_BOMB))
        dam = (dam + r) / (r + 1);

    /* Do it ... shared with other non-projection damage like melee attacks and auras */
    gf_distance_hack = r;
    if (flg & PROJECT_NO_PAIN) liput |= GF_NO_PAIN;
    result = gf_affect_m(who, &m_list[m_idx], typ, dam, liput);
    gf_distance_hack = 1;

    /* Track it */
    project_m_n++;
    project_m_x = x;
    project_m_y = y;

    return result;
}

/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type". See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above). This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * NOTE (Zangband): 'Bolt' attacks can be reflected back, so we need
 * to know if this is actually a ball or a bolt spell
 *
 *
 * We return "TRUE" if any "obvious" effects were observed. XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static int _reduce_dam(int dam, int distance)
{          /* OLD: 100, 50, 33, 25, 20, 16, 14, 12 */
    int pct[8] = { 100, 80, 60, 50, 44, 37, 33, 30 };
    return (dam * pct[MIN(7, distance)] + 50) / 100;
}
static bool project_p(int who, cptr who_name, int r, int y, int x, int dam, int typ, int flg)
{

    /* Hack -- assume obvious */
    bool obvious = TRUE;

    /* Player blind-ness */
    bool blind = (p_ptr->blind ? TRUE : FALSE);


    /* Source monster */
    monster_type *m_ptr = NULL;

    /* Monster name (for attacks) */
    char m_name[80];

    int get_damage = 0;


    /* Player is not here */
    if (!player_bold(y, x)) return (FALSE);

    if ((p_ptr->special_defense & NINJA_KAWARIMI) && dam && (randint0(55) < (p_ptr->lev*3/5+20)) && who && (who != p_ptr->riding))
    {
        if (kawarimi(TRUE)) return FALSE;
    }

    /* Player cannot hurt himself */
    if (!who) return (FALSE);
    if (who == p_ptr->riding) return (FALSE);

    if (who > 0)
    {
        m_ptr = &m_list[who];
        monster_desc(m_name, m_ptr, 0);
    }

    if (p_ptr->tim_spell_reaction && !p_ptr->fast)
    {
        set_fast(4, FALSE);
    }

    /* Magic Resistance for the Rune Knight works differently than for
     * other classes (notably golems). They gain this thru their {absorption}
     * rune which absorbs not just magical energy, but elemental attacks
     * (e.g. breaths) as well. Restricting their mana gain to only magical
     * attacks is far too annoyingly rare to matter much for 99% of the game.
     * Of course, this makes it easier for the Rune-Knight to scum mana ...
     * I can only appeal to your sense of honor in this regard!
     * We check this first, before applying evasion or even reflection!
     *
     * XXX Damage has not been reduced yet for distance. Let's even leave
     * that alone for the time being. SP regain should be quicker than before.
     *
     * XXX Not sure about the who check (who > 0 is the m_idx). Currently, it
     * is ignored, but might be useful to prevent players scumming weak casters.
     */
    if (p_ptr->pclass == CLASS_RUNE_KNIGHT && who > 0)
        dam = rune_knight_absorption(who, typ, dam);

    if ((p_ptr->reflect || ((p_ptr->special_defense & KATA_FUUJIN) && !p_ptr->blind)) && (flg & PROJECT_REFLECTABLE) && !one_in_(4))
    {
        byte t_y, t_x;
        int max_attempts = 10;
        static bool _lukko = FALSE;

        if (blind) msg_print("Something bounces!");
        else msg_print("The attack bounces!");

        equip_learn_flag(OF_REFLECT);
        update_smart_learn(who, SM_REFLECTION);

        /* Choose 'new' target */
        if (who > 0)
        {
            do
            {
                t_y = m_list[who].fy - 2 + randint0(5);
                t_x = m_list[who].fx - 2 + randint0(5);
                max_attempts--;
            }
            while (max_attempts && in_bounds2u(t_y, t_x) && !projectable(py, px, t_y, t_x));

            if (max_attempts < 1)
            {
                t_y = m_list[who].fy;
                t_x = m_list[who].fx;
            }
        }
        else
        {
            t_y = py - 2 + randint0(5);
            t_x = px - 2 + randint0(5);
        }

        if ((p_ptr->action == ACTION_LEARN) && (!_lukko) && (who > 0))
        {
            blue_mage_learn_spell();
            _lukko = TRUE;
        }

        project(0, 0, t_y, t_x, dam, typ, (PROJECT_STOP|PROJECT_KILL|PROJECT_REFLECTABLE));

        _lukko = FALSE;

        disturb(1, 0);
        return TRUE;
    }

    /* XXX XXX XXX */
    /* Limit maximum damage */
    if ((dam > 1600) && (typ != GF_GAIN_EXP)) dam = 1600;

    p_ptr->spell_turned = FALSE;

    /* Reduce damage by distance (bombs have special code for this) */
    if (typ != GF_BOMB) dam = _reduce_dam(dam, r);

    if (mon_spell_current())
    {
        if (mon_spell_current()->flags & MSF_INNATE)
        {
            bool evaded = FALSE; /* Demigod Scout with Evasion talent *and* Nimble Dodge cast? */

            if (p_ptr->tim_nimble_dodge)
            {
                int odds = 7 * p_ptr->open_terrain_ct;
                if (randint0(100) < odds)
                {
                    msg_print("You nimbly dodge the attack!");
                    dam = 0;
                    evaded = TRUE;
                    check_muscle_sprains(75, "You feel a sudden stabbing pain!");
                }
            }

            if (!evaded && mut_present(MUT_EVASION))
            {
                msg_print("You evade the attack!");
                dam -= dam * (10 + randint1(10))/100;
            }
        }
        else if (p_ptr->tim_spell_turning)
        {
            bool turn = FALSE;
            if (p_ptr->shero)
                turn = (randint1(100) <= p_ptr->lev) ? TRUE : FALSE;
            else
                turn = (randint1(200) <= 20 + p_ptr->lev) ? TRUE : FALSE;

            if (turn)
            {
                msg_print("You turn the magic on the caster!");
                p_ptr->spell_turned = TRUE;
                disturb(1, 0);
                return TRUE;
            }
        }
        else if (p_ptr->pclass != CLASS_RUNE_KNIGHT && p_ptr->magic_resistance)
        {
            dam -= dam * p_ptr->magic_resistance / 100;
        }
    }

    if ( p_ptr->pclass == CLASS_DUELIST
      && p_ptr->duelist_target_idx == who )
    {
        dam -= dam/3;
    }

    if (psion_drain())
        dam = psion_do_drain(dam);

    gf_distance_hack = r;
    get_damage = gf_affect_p(who, typ, dam, GF_AFFECT_SPELL);
    gf_distance_hack = 1;

    /* Hex - revenge damage stored */
    revenge_store(get_damage);
    if (get_damage > 0 && !p_ptr->is_dead && who > 0)
    {
        assert(m_ptr);
        weaponmaster_do_readied_shot(m_ptr);
    }

    if (IS_REVENGE() && get_damage > 0 && !p_ptr->is_dead && who > 0)
    {
        char m_name_self[80];
        assert(m_ptr);
        monster_desc(m_name_self, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE | MD_OBJECTIVE);
        msg_format("%^s harms %s!", m_name, m_name_self);
        project(0, 0, m_ptr->fy, m_ptr->fx, psion_backlash_dam(get_damage), GF_MISSILE, PROJECT_KILL);
        if (p_ptr->tim_eyeeye) set_tim_eyeeye(p_ptr->tim_eyeeye-5, TRUE);
    }

    if (p_ptr->riding && dam > 0)
    {
        rakubadam_p = (dam > 200) ? 200 : dam;
    }

    if (who > 0 && get_damage && p_ptr->tim_armor_of_fury)
    {
        assert(m_ptr);
        msg_format("%^s is hit by your fury!", m_name);
        if ( mon_save_p(m_ptr->r_idx, A_STR)
          && (!p_ptr->shero || mon_save_p(m_ptr->r_idx, A_STR)) )
        {
            msg_format("%^s resists!", m_name);
        }
        else
        {
            int dur = 1;
            if (p_ptr->shero) dur = 3;
            if (!MON_SLOW(m_ptr))
            {
                msg_format("%^s is slowed.", m_name);
                set_monster_slow(who, dur);
            }
            if (mon_stun(m_ptr, mon_stun_amount(dur*get_damage)))
                msg_format("%^s is stunned.", m_name);
            else
                msg_format("%^s is more stunned.", m_name);
        }
    }

    disturb(1, 0);

    if ((p_ptr->special_defense & NINJA_KAWARIMI) && dam && who && (who != p_ptr->riding))
    {
        (void)kawarimi(FALSE);
    }

    /* Return "Anything seen?" */
    return (obvious);
}


/*
 * Find the distance from (x, y) to a line.
 */
int dist_to_line(int y, int x, int y1, int x1, int y2, int x2)
{
    /* Vector from (x, y) to (x1, y1) */
    int py = y1 - y;
    int px = x1 - x;

    /* Normal vector */
    int ny = x2 - x1;
    int nx = y1 - y2;

   /* Length of N */
    int pd = distance(y1, x1, y, x);
    int nd = distance(y1, x1, y2, x2);

    if (pd > nd) return distance(y, x, y2, x2);

    /* Component of P on N */
    nd = ((nd) ? ((py * ny + px * nx) / nd) : 0);

   /* Absolute value */
   return((nd >= 0) ? nd : 0 - nd);
}



/*
 * XXX XXX XXX
 * Modified version of los() for calculation of disintegration balls.
 * Disintegration effects are stopped by permanent walls.
 */
bool in_disintegration_range(int y1, int x1, int y2, int x2)
{
    /* Delta */
    int dx, dy;

    /* Absolute */
    int ax, ay;

    /* Signs */
    int sx, sy;

    /* Fractions */
    int qx, qy;

    /* Scanners */
    int tx, ty;

    /* Scale factors */
    int f1, f2;

    /* Slope, or 1/Slope, of LOS */
    int m;


    /* Extract the offset */
    dy = y2 - y1;
    dx = x2 - x1;

    /* Extract the absolute offset */
    ay = ABS(dy);
    ax = ABS(dx);


    /* Handle adjacent (or identical) grids */
    if ((ax < 2) && (ay < 2)) return (TRUE);


    /* Paranoia -- require "safe" origin */
    /* if (!in_bounds(y1, x1)) return (FALSE); */


    /* Directly South/North */
    if (!dx)
    {
        /* South -- check for walls */
        if (dy > 0)
        {
            for (ty = y1 + 1; ty < y2; ty++)
            {
                if (cave_stop_disintegration(ty, x1)) return (FALSE);
            }
        }

        /* North -- check for walls */
        else
        {
            for (ty = y1 - 1; ty > y2; ty--)
            {
                if (cave_stop_disintegration(ty, x1)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }

    /* Directly East/West */
    if (!dy)
    {
        /* East -- check for walls */
        if (dx > 0)
        {
            for (tx = x1 + 1; tx < x2; tx++)
            {
                if (cave_stop_disintegration(y1, tx)) return (FALSE);
            }
        }

        /* West -- check for walls */
        else
        {
            for (tx = x1 - 1; tx > x2; tx--)
            {
                if (cave_stop_disintegration(y1, tx)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }


    /* Extract some signs */
    sx = (dx < 0) ? -1 : 1;
    sy = (dy < 0) ? -1 : 1;


    /* Vertical "knights" */
    if (ax == 1)
    {
        if (ay == 2)
        {
            if (!cave_stop_disintegration(y1 + sy, x1)) return (TRUE);
        }
    }

    /* Horizontal "knights" */
    else if (ay == 1)
    {
        if (ax == 2)
        {
            if (!cave_stop_disintegration(y1, x1 + sx)) return (TRUE);
        }
    }


    /* Calculate scale factor div 2 */
    f2 = (ax * ay);

    /* Calculate scale factor */
    f1 = f2 << 1;


    /* Travel horizontally */
    if (ax >= ay)
    {
        /* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
        qy = ay * ay;
        m = qy << 1;

        tx = x1 + sx;

        /* Consider the special case where slope == 1. */
        if (qy == f2)
        {
            ty = y1 + sy;
            qy -= f1;
        }
        else
        {
            ty = y1;
        }

        /* Note (below) the case (qy == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (x2 - tx)
        {
            if (cave_stop_disintegration(ty, tx)) return (FALSE);

            qy += m;

            if (qy < f2)
            {
                tx += sx;
            }
            else if (qy > f2)
            {
                ty += sy;
                if (cave_stop_disintegration(ty, tx)) return (FALSE);
                qy -= f1;
                tx += sx;
            }
            else
            {
                ty += sy;
                qy -= f1;
                tx += sx;
            }
        }
    }

    /* Travel vertically */
    else
    {
        /* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
        qx = ax * ax;
        m = qx << 1;

        ty = y1 + sy;

        if (qx == f2)
        {
            tx = x1 + sx;
            qx -= f1;
        }
        else
        {
            tx = x1;
        }

        /* Note (below) the case (qx == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (y2 - ty)
        {
            if (cave_stop_disintegration(ty, tx)) return (FALSE);

            qx += m;

            if (qx < f2)
            {
                ty += sy;
            }
            else if (qx > f2)
            {
                tx += sx;
                if (cave_stop_disintegration(ty, tx)) return (FALSE);
                qx -= f1;
                ty += sy;
            }
            else
            {
                tx += sx;
                qx -= f1;
                ty += sy;
            }
        }
    }

    /* Assume los */
    return (TRUE);
}


/*
 * breath shape
 */
void breath_shape(u16b *path_g, int dist, int *pgrids, byte *gx, byte *gy, byte *gm, int *pgm_rad, int rad, int y1, int x1, int y2, int x2, int typ)
{
    int by = y1;
    int bx = x1;
    int brad = 0;
    int brev = rad * rad / dist;
    int bdis = 0;
    int cdis;
    int path_n = 0;
    int mdis = distance(y1, x1, y2, x2) + rad;

    while (bdis <= mdis)
    {
        int x, y;

        if ((0 < dist) && (path_n < dist))
        {
            int ny = GRID_Y(path_g[path_n]);
            int nx = GRID_X(path_g[path_n]);
            int nd = distance(ny, nx, y1, x1);

            /* Get next base point */
            if (bdis >= nd)
            {
                by = ny;
                bx = nx;
                path_n++;
            }
        }

        /* Travel from center outward */
        for (cdis = 0; cdis <= brad; cdis++)
        {
            /* Scan the maximal blast area of radius "cdis" */
            for (y = by - cdis; y <= by + cdis; y++)
            {
                for (x = bx - cdis; x <= bx + cdis; x++)
                {
                    /* Ignore "illegal" locations */
                    if (!in_bounds(y, x)) continue;

                    /* Enforce a circular "ripple" */
                    if (distance(y1, x1, y, x) != bdis) continue;

                    /* Enforce an arc */
                    if (distance(by, bx, y, x) != cdis) continue;

                    switch (typ)
                    {
                    case GF_LITE:
                    case GF_LITE_WEAK:
                        /* Lights are stopped by opaque terrains */
                        if (!los(by, bx, y, x)) continue;
                        break;
                    case GF_DISINTEGRATE:
                        /* Disintegration are stopped only by perma-walls */
                        if (!in_disintegration_range(by, bx, y, x)) continue;
                        break;
                    default:
                        /* Ball explosions are stopped by walls */
                        if (!projectable(by, bx, y, x)) continue;
                        break;
                    }

                    /* Save this grid */
                    gy[*pgrids] = y;
                    gx[*pgrids] = x;
                    (*pgrids)++;
                }
            }
        }

        /* Encode some more "radius" info */
        gm[bdis + 1] = *pgrids;

        /* Increase the size */
        brad = rad * (path_n + brev) / (dist + brev);

        /* Find the next ripple */
        bdis++;
    }

    /* Store the effect size */
    *pgm_rad = bdis;
}


/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (zero for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall. Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away. The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons. First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball. For the grid right
 * next to the epicenter, this results in 150% damage being done. The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed. Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should. Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array. First, gm[0] is always
 * zero. Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center. Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function. Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced. This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path. Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination. This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
    int i, t, dist;

    int y1, x1;
    int y2, x2;
    int by, bx;
    int old_rad = rad;

    int dist_hack = 0;

    int y_saver, x_saver; /* For reflecting monsters */

    int msec = delay_time();

    /* Assume the player sees nothing */
    bool notice = FALSE;

    /* Assume the player has seen nothing */
    bool visual = FALSE;

    /* Assume the player has seen no blast grids */
    bool drawn = FALSE;

    /* Assume to be a normal ball spell */
    bool breath = FALSE;

    /* Is the player blind? */
    bool blind = (p_ptr->blind ? TRUE : FALSE);

    bool old_hide = FALSE;

    /* Number of grids in the "path" */
    int path_n = 0;

    /* Actual grids in the "path" */
    u16b path_g[512];

    /* Number of grids in the "blast area" (including the "beam" path) */
    int grids = 0;

    /* Coordinates of the affected grids */
    byte gx[1024], gy[1024];

    /* Encoded "radius" info (see above) */
    byte gm[64];

    /* Actual radius encoded in gm[] */
    int gm_rad = rad;

    bool jump = FALSE;

    /* Attacker's name (prepared before polymorph)*/
    char who_name[80];

    /* Can the player see the source of this effect? */
    bool see_s_msg = TRUE;

    /* Initialize by null string */
    who_name[0] = '\0';

    rakubadam_p = 0;
    rakubadam_m = 0;

    /* Default target of monsterspell is player */
    monster_target_y=py;
    monster_target_x=px;

    /* Hack -- Jump to target */
    if (flg & (PROJECT_JUMP))
    {
        x1 = x;
        y1 = y;

        /* Clear the flag */
        flg &= ~(PROJECT_JUMP);

        jump = TRUE;
    }

    /* Start at player */
    else if (who <= 0)
    {
        x1 = px;
        y1 = py;
    }

    /* Start at monster */
    else if (who > 0)
    {
        p_ptr->spell_turned = FALSE;
        x1 = m_list[who].fx;
        y1 = m_list[who].fy;
        monster_desc(who_name, &m_list[who], MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
    }

    /* Oops */
    else
    {
        x1 = x;
        y1 = y;
    }

    y_saver = y1;
    x_saver = x1;

    /* Default "destination" */
    y2 = y;
    x2 = x;


    /* Hack -- verify stuff */
    if (flg & (PROJECT_THRU))
    {
        if ((x1 == x2) && (y1 == y2))
        {
            flg &= ~(PROJECT_THRU);
        }
    }

    /* Handle a breath attack */
    if (rad < 0)
    {
        rad = 0 - rad;
        breath = TRUE;
        if (flg & PROJECT_HIDE) old_hide = TRUE;
        flg |= PROJECT_HIDE;
    }


    /* Hack -- Assume there will be no blast (max radius 32) */
    for (dist = 0; dist < 32; dist++) gm[dist] = 0;


    /* Initial grid */
    y = y1;
    x = x1;
    dist = 0;

    /* Collect beam grids */
    if (flg & (PROJECT_BEAM))
    {
        gy[grids] = y;
        gx[grids] = x;
        grids++;
    }

    switch (typ)
    {
    case GF_LITE:
    case GF_LITE_WEAK:
    case GF_DARK:
        if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_LOS);
        break;
    case GF_DISINTEGRATE:
        flg |= (PROJECT_GRID);
        if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_DISI);
        break;
    case GF_SOUND:
        if (p_ptr->no_air)
        {
            if (who == PROJECT_WHO_PLAYER) msg_print("The sound fizzles out in the airless dungeon!");
            return TRUE;
        }
        break;
    case GF_AIR:
    case GF_STORM:
        if (p_ptr->no_air)
        {
            (void)set_no_air(0, TRUE);
        }
        break;
    }

    /* Calculate the projection path */

    path_n = project_path(path_g, (project_length ? project_length : MAX_RANGE), y1, x1, y2, x2, flg);

    /* Hack -- Handle stuff */
    handle_stuff();

    /* Hack -- Track spell type */
    while ((who == PROJECT_WHO_PLAYER) && (typ > GF_NONE) && (attack_spell_hack == ASH_UNKNOWN))
    {
        gf_info_ptr _tyyppi = gf_lookup(typ);
        if ((!_tyyppi) || (!_tyyppi->name)) break;
        if (!(_tyyppi->flags & (GFF_ATTACK | GFF_STATUS))) attack_spell_hack = ASH_NOT_ATTACK;
        else if (_tyyppi->flags & (GFF_TERRAIN | GFF_UTILITY)) attack_spell_hack = ASH_NOT_ATTACK;
        else if (_tyyppi->flags & GFF_STATUS) attack_spell_hack = ASH_UNASSESSED_1;
        else attack_spell_hack = ASH_UNASSESSED_2;
        break;
    }

    /* Giga-Hack SEEKER & SUPER_RAY */
    if( typ == GF_SEEKER )
    {
        int j;
        int last_i=0;

        /* Mega-Hack */
        project_m_n = 0;
        project_m_x = 0;
        project_m_y = 0;

        for (i = 0; i < path_n; ++i)
        {
            int oy = y;
            int ox = x;

            int ny = GRID_Y(path_g[i]);
            int nx = GRID_X(path_g[i]);

            /* Advance */
            y = ny;
            x = nx;

            gy[grids] = y;
            gx[grids] = x;
            grids++;


            /* Only do visuals if requested */
            if (!blind && !(flg & (PROJECT_HIDE)))
            {
                /* Only do visuals if the player can "see" the bolt */
                if (panel_contains(y, x) && player_has_los_bold(y, x))
                {
                    u16b p;

                    byte a;
                    char c;

                    /* Obtain the bolt pict */
                    p = bolt_pict(oy, ox, y, x, typ);

                    /* Extract attr/char */
                    a = PICT_A(p);
                    c = PICT_C(p);

                    /* Visual effects */
                    print_rel(c, a, y, x);
                    move_cursor_relative(y, x);
                    /*if (fresh_before)*/ Term_fresh();
                    Term_xtra(TERM_XTRA_DELAY, msec);
                    lite_spot(y, x);
                    /*if (fresh_before)*/ Term_fresh();

                    /* Display "beam" grids */
                    if (flg & (PROJECT_BEAM))
                    {
                        /* Obtain the explosion pict */
                        p = bolt_pict(y, x, y, x, typ);

                        /* Extract attr/char */
                        a = PICT_A(p);
                        c = PICT_C(p);

                        /* Visual effects */
                        print_rel(c, a, y, x);
                    }

                    /* Hack -- Activate delay */
                    visual = TRUE;
                }

                /* Hack -- delay anyway for consistency */
                else if (visual)
                {
                    /* Delay for consistency */
                    Term_xtra(TERM_XTRA_DELAY, msec);
                }
            }
            if(project_o(0,0,y,x,dam,GF_SEEKER))notice=TRUE;
            if( is_mirror_grid(&cave[y][x]))
            {
              /* The target of monsterspell becomes tha mirror(broken) */
                monster_target_y=(s16b)y;
                monster_target_x=(s16b)x;

                remove_mirror(y,x);
                next_mirror( &oy,&ox,y,x );

                path_n = i+project_path(&(path_g[i+1]), (project_length ? project_length : MAX_RANGE), y, x, oy, ox, flg);
                for( j = last_i; j <=i ; j++ )
                {
                    y = GRID_Y(path_g[j]);
                    x = GRID_X(path_g[j]);
                    if(project_m(0,0,y,x,dam,GF_SEEKER,flg,TRUE))notice=TRUE;
                    if(!who && (project_m_n==1) && !jump ){
                      if(cave[project_m_y][project_m_x].m_idx >0 ){
                        monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

                        if (m_ptr->ml)
                        {
                          /* Hack -- auto-recall */
                          if (!p_ptr->image) mon_track(m_ptr);

                          /* Hack - auto-track */
                          health_track(cave[project_m_y][project_m_x].m_idx);
                        }
                      }
                    }
                    (void)project_f(0,0,y,x,dam,GF_SEEKER);
                }
                last_i = i;
            }
        }
        for( i = last_i ; i < path_n ; i++ )
        {
            int x,y;
            y = GRID_Y(path_g[i]);
            x = GRID_X(path_g[i]);
            if(project_m(0,0,y,x,dam,GF_SEEKER,flg,TRUE))
              notice=TRUE;
            if(!who && (project_m_n==1) && !jump ){
              if(cave[project_m_y][project_m_x].m_idx >0 ){
                monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

                if (m_ptr->ml)
                {
                  /* Hack -- auto-recall */
                  if (!p_ptr->image) mon_track(m_ptr);

                  /* Hack - auto-track */
                  health_track(cave[project_m_y][project_m_x].m_idx);
                }
              }
            }
            (void)project_f(0,0,y,x,dam,GF_SEEKER);
        }
        return notice;
    }
    else if(typ == GF_SUPER_RAY){
        int j;
        int second_step = 0;

        /* Mega-Hack */
        project_m_n = 0;
        project_m_x = 0;
        project_m_y = 0;

        for (i = 0; i < path_n; ++i)
        {
            int oy = y;
            int ox = x;

            int ny = GRID_Y(path_g[i]);
            int nx = GRID_X(path_g[i]);

            /* Advance */
            y = ny;
            x = nx;

            gy[grids] = y;
            gx[grids] = x;
            grids++;


            /* Only do visuals if requested */
            if (!blind && !(flg & (PROJECT_HIDE)))
            {
                /* Only do visuals if the player can "see" the bolt */
                if (panel_contains(y, x) && player_has_los_bold(y, x))
                {
                    u16b p;

                    byte a;
                    char c;

                    /* Obtain the bolt pict */
                    p = bolt_pict(oy, ox, y, x, typ);

                    /* Extract attr/char */
                    a = PICT_A(p);
                    c = PICT_C(p);

                    /* Visual effects */
                    print_rel(c, a, y, x);
                    move_cursor_relative(y, x);
                    /*if (fresh_before)*/ Term_fresh();
                    Term_xtra(TERM_XTRA_DELAY, msec);
                    lite_spot(y, x);
                    /*if (fresh_before)*/ Term_fresh();

                    /* Display "beam" grids */
                    if (flg & (PROJECT_BEAM))
                    {
                        /* Obtain the explosion pict */
                        p = bolt_pict(y, x, y, x, typ);

                        /* Extract attr/char */
                        a = PICT_A(p);
                        c = PICT_C(p);

                        /* Visual effects */
                        print_rel(c, a, y, x);
                    }

                    /* Hack -- Activate delay */
                    visual = TRUE;
                }

                /* Hack -- delay anyway for consistency */
                else if (visual)
                {
                    /* Delay for consistency */
                    Term_xtra(TERM_XTRA_DELAY, msec);
                }
            }
            if(project_o(0,0,y,x,dam,GF_SUPER_RAY) )notice=TRUE;
            if (!cave_have_flag_bold(y, x, FF_PROJECT))
            {
                if( second_step )continue;
                break;
            }
            if( is_mirror_grid(&cave[y][x]) && !second_step )
            {
              /* The target of monsterspell becomes tha mirror(broken) */
                monster_target_y=(s16b)y;
                monster_target_x=(s16b)x;

                remove_mirror(y,x);
                for( j = 0; j <=i ; j++ )
                {
                    y = GRID_Y(path_g[j]);
                    x = GRID_X(path_g[j]);
                    (void)project_f(0,0,y,x,dam,GF_SUPER_RAY);
                }
                path_n = i;
                second_step =i+1;
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x-1, flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x  , flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x+1, flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y  , x-1, flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y  , x+1, flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x-1, flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x  , flg);
                path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x+1, flg);
            }
        }
        for( i = 0; i < path_n ; i++ )
        {
            int x,y;
            y = GRID_Y(path_g[i]);
            x = GRID_X(path_g[i]);
            (void)project_m(0,0,y,x,dam,GF_SUPER_RAY,flg,TRUE);
            if(!who && (project_m_n==1) && !jump ){
              if(cave[project_m_y][project_m_x].m_idx >0 ){
                monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

                if (m_ptr->ml)
                {
                  /* Hack -- auto-recall */
                  if (!p_ptr->image) mon_track(m_ptr);

                  /* Hack - auto-track */
                  health_track(cave[project_m_y][project_m_x].m_idx);
                }
              }
            }
            (void)project_f(0,0,y,x,dam,GF_SUPER_RAY);
        }
        return notice;
    }

    /* Project along the path */
    for (i = 0; i < path_n; ++i)
    {
        int oy = y;
        int ox = x;

        int ny = GRID_Y(path_g[i]);
        int nx = GRID_X(path_g[i]);

        if (flg & PROJECT_DISI)
        {
            /* Hack -- Balls explode before reaching walls */
            if (cave_stop_disintegration(ny, nx) && (rad > 0)) break;
        }
        else if (flg & PROJECT_LOS)
        {
            /* Hack -- Balls explode before reaching walls */
            if (!cave_los_bold(ny, nx) && (rad > 0)) break;
        }
        else
        {
            /* Hack -- Balls explode before reaching walls */
            if (!cave_have_flag_bold(ny, nx, FF_PROJECT) && (rad > 0)) break;
        }

        /* Advance */
        y = ny;
        x = nx;

        /* Collect beam grids */
        if (flg & (PROJECT_BEAM))
        {
            gy[grids] = y;
            gx[grids] = x;
            grids++;
        }

        /* Only do visuals if requested */
        if (!blind && !(flg & (PROJECT_HIDE | PROJECT_FAST)))
        {
            /* Only do visuals if the player can "see" the bolt */
            if (panel_contains(y, x) && player_has_los_bold(y, x))
            {
                u16b p;

                byte a;
                char c;

                /* Obtain the bolt pict */
                p = bolt_pict(oy, ox, y, x, typ);

                /* Extract attr/char */
                a = PICT_A(p);
                c = PICT_C(p);

                /* Visual effects */
                print_rel(c, a, y, x);
                move_cursor_relative(y, x);
                /*if (fresh_before)*/ Term_fresh();
                Term_xtra(TERM_XTRA_DELAY, msec);
                lite_spot(y, x);
                /*if (fresh_before)*/ Term_fresh();

                /* Display "beam" grids */
                if (flg & (PROJECT_BEAM))
                {
                    /* Obtain the explosion pict */
                    p = bolt_pict(y, x, y, x, typ);

                    /* Extract attr/char */
                    a = PICT_A(p);
                    c = PICT_C(p);

                    /* Visual effects */
                    print_rel(c, a, y, x);
                }

                /* Hack -- Activate delay */
                visual = TRUE;
            }

            /* Hack -- delay anyway for consistency */
            else if (visual)
            {
                /* Delay for consistency */
                Term_xtra(TERM_XTRA_DELAY, msec);
            }
        }
    }

    path_n = i;

    /* Save the "blast epicenter" */
    by = y;
    bx = x;

    if (breath && !path_n)
    {
        breath = FALSE;
        gm_rad = rad;
        if (!old_hide)
        {
            flg &= ~(PROJECT_HIDE);
        }
    }

    /* Start the "explosion" */
    gm[0] = 0;

    /* Hack -- make sure beams get to "explode" */
    gm[1] = grids;

    dist = path_n;
    dist_hack = dist;

    project_length = 0;

    /* If we found a "target", explode there */
    if (dist <= MAX_RANGE)
    {
        /* Mega-Hack -- remove the final "beam" grid */
        if ((flg & (PROJECT_BEAM)) && (grids > 0)) grids--;

        /*
         * Create a conical breath attack
         *
         *         ***
         *     ********
         * D********@**
         *     ********
         *         ***
         */

        if (breath)
        {
            flg &= ~(PROJECT_HIDE);

            breath_shape(path_g, dist, &grids, gx, gy, gm, &gm_rad, rad, y1, x1, by, bx, typ);
        }
        else
        {
            /* Determine the blast area, work from the inside out */
            for (dist = 0; dist <= rad; dist++)
            {
                /* Scan the maximal blast area of radius "dist" */
                for (y = by - dist; y <= by + dist; y++)
                {
                    for (x = bx - dist; x <= bx + dist; x++)
                    {
                        /* Ignore "illegal" locations */
                        if (!in_bounds2(y, x)) continue;

                        /* Enforce a "circular" explosion */
                        if (distance(by, bx, y, x) != dist) continue;

                        switch (typ)
                        {
                        case GF_LITE:
                        case GF_LITE_WEAK:
                            /* Lights are stopped by opaque terrains */
                            if (!los(by, bx, y, x)) continue;
                            break;
                        case GF_DISINTEGRATE:
                            /* Disintegration are stopped only by perma-walls */
                            if (!in_disintegration_range(by, bx, y, x)) continue;
                            break;
                        default:
                            /* Ball explosions are stopped by walls */
                            if (!projectable(by, bx, y, x)) continue;
                            break;
                        }

                        /* Save this grid */
                        gy[grids] = y;
                        gx[grids] = x;
                        grids++;
                    }
                }

                /* Encode some more "radius" info */
                gm[dist+1] = grids;
            }
        }
    }

    /* Speed -- ignore "non-explosions" */
    if (!grids)
    {
        if (who > 0 && p_ptr->spell_turned)
            project(-1, old_rad, y1, x1, dam, typ, flg);
        return (FALSE);
    }

    /* Display the "blast area" if requested */
    if (!blind && !(flg & (PROJECT_HIDE)))
    {
        /* Then do the "blast", from inside out */
        for (t = 0; t <= gm_rad; t++)
        {
            /* Dump everything with this radius */
            for (i = gm[t]; i < gm[t+1]; i++)
            {
                /* Extract the location */
                y = gy[i];
                x = gx[i];

                /* Only do visuals if the player can "see" the blast */
                if (panel_contains(y, x) && player_has_los_bold(y, x))
                {
                    u16b p;

                    byte a;
                    char c;

                    drawn = TRUE;

                    /* Obtain the explosion pict */
                    p = bolt_pict(y, x, y, x, typ);

                    /* Extract attr/char */
                    a = PICT_A(p);
                    c = PICT_C(p);

                    /* Visual effects -- Display */
                    print_rel(c, a, y, x);
                }
            }

            /* Hack -- center the cursor */
            move_cursor_relative(by, bx);

            /* Flush each "radius" seperately */
            /*if (fresh_before)*/ Term_fresh();

            /* Delay (efficiently) */
            if (visual || drawn)
            {
                Term_xtra(TERM_XTRA_DELAY, msec);
            }
        }

        /* Flush the erasing */
        if (drawn)
        {
            /* Erase the explosion drawn above */
            for (i = 0; i < grids; i++)
            {
                /* Extract the location */
                y = gy[i];
                x = gx[i];

                /* Hack -- Erase if needed */
                if (panel_contains(y, x) && player_has_los_bold(y, x))
                {
                    lite_spot(y, x);
                }
            }

            /* Hack -- center the cursor */
            move_cursor_relative(by, bx);

            /* Flush the explosion */
            /*if (fresh_before)*/ Term_fresh();
        }
    }


    /* Update stuff if needed */
    if (p_ptr->update) update_stuff();


    if (flg & PROJECT_KILL)
    {
        see_s_msg = (who > 0) ? mon_show_msg(&m_list[who]) :
            (!who ? TRUE : (player_can_see_bold(y1, x1) && projectable(py, px, y1, x1)));
    }


    /* Check features */
    if (flg & (PROJECT_GRID))
    {
        /* Start with "dist" of zero */
        dist = 0;

        /* Scan for features */
        for (i = 0; i < grids; i++)
        {
            /* Hack -- Notice new "dist" values */
            if (gm[dist+1] == i) dist++;

            /* Get the grid location */
            y = gy[i];
            x = gx[i];

            /* Find the closest point in the blast */
            if (breath)
            {
                int d = dist_to_line(y, x, y1, x1, by, bx);

                /* Affect the grid */
                if (project_f(who, d, y, x, dam, typ)) notice = TRUE;
            }
            else
            {
                /* Affect the grid */
                if (project_f(who, dist, y, x, dam, typ)) notice = TRUE;
            }
        }
    }

    /* Update stuff if needed */
    if (p_ptr->update) update_stuff();

    /* Check objects */
    if (flg & (PROJECT_ITEM))
    {
        /* Start with "dist" of zero */
        dist = 0;

        /* Scan for objects */
        for (i = 0; i < grids; i++)
        {
            /* Hack -- Notice new "dist" values */
            if (gm[dist+1] == i) dist++;

            /* Get the grid location */
            y = gy[i];
            x = gx[i];

            /* Find the closest point in the blast */
            if (breath)
            {
                int d = dist_to_line(y, x, y1, x1, by, bx);

                /* Affect the object in the grid */
                if (project_o(who, d, y, x, dam, typ)) notice = TRUE;
            }
            else
            {
                /* Affect the object in the grid */
                if (project_o(who, dist, y, x, dam, typ)) notice = TRUE;
            }
        }
    }

    /* Check monsters */
    if (flg & (PROJECT_KILL))
    {
        /* Mega-Hack */
        project_m_n = 0;
        project_m_x = 0;
        project_m_y = 0;

        /* Start with "dist" of zero */
        dist = 0;

        hack_max_m_dam = 0;  /* XXX see below for device lore */

        /* Scan for monsters */
        for (i = 0; i < grids; i++)
        {
            int effective_dist;

            /* Hack -- Notice new "dist" values */
            if (gm[dist + 1] == i) dist++;

            /* Get the grid location */
            y = gy[i];
            x = gx[i];

            /* A single bolt may be reflected */
            if (grids <= 1)
            {
                monster_type *m_ptr = &m_list[cave[y][x].m_idx];
                monster_race *ref_ptr = &r_info[m_ptr->r_idx];

                if ((flg & PROJECT_REFLECTABLE) && cave[y][x].m_idx && (ref_ptr->flags2 & RF2_REFLECTING) &&
                    ((cave[y][x].m_idx != p_ptr->riding) || !(flg & PROJECT_PLAYER)) &&
                    (!who || dist_hack > 1) && !one_in_(4))
                {
                    byte t_y, t_x;
                    int max_attempts = 10;

                    /* Choose 'new' target */
                    do
                    {
                        t_y = y_saver - 2 + randint0(5);
                        t_x = x_saver - 2 + randint0(5);
                        max_attempts--;
                    }
                    while (max_attempts && in_bounds2u(t_y, t_x) && !projectable(y, x, t_y, t_x));

                    if (max_attempts < 1)
                    {
                        t_y = y_saver;
                        t_x = x_saver;
                    }

                    if (mon_show_msg(m_ptr))
                    {
                        msg_print("The attack bounces!");
                    }
                    mon_lore_2(m_ptr, RF2_REFLECTING);

                    /* Reflected bolts randomly target either one */
                    if (player_bold(y, x) || one_in_(2)) flg &= ~(PROJECT_PLAYER);
                    else flg |= PROJECT_PLAYER;

                    /* The bolt is reflected */
                    project(cave[y][x].m_idx, 0, t_y, t_x, dam, typ, flg);

                    /* Don't affect the monster any longer */
                    continue;
                }
            }


            /* Find the closest point in the blast */
            if (breath)
            {
                effective_dist = dist_to_line(y, x, y1, x1, by, bx);
            }
            else
            {
                effective_dist = dist;
            }


            /* There is the riding player on this monster */
            if (p_ptr->riding && player_bold(y, x))
            {
                /* Aimed on the player */
                if (flg & PROJECT_PLAYER)
                {
                    if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
                    {
                        /*
                         * A beam or bolt is well aimed
                         * at the PLAYER!
                         * So don't affects the mount.
                         */
                        continue;
                    }
                    else
                    {
                        /*
                         * The spell is not well aimed,
                         * So partly affect the mount too.
                         */
                        effective_dist++;
                    }
                }

                /*
                 * This grid is the original target.
                 * Or aimed on your horse.
                 */
                else if (((y == y2) && (x == x2)) || (flg & PROJECT_AIMED))
                {
                    /* Hit the mount with full damage */
                }

                /*
                 * Otherwise this grid is not the
                 * original target, it means that line
                 * of fire is obstructed by this
                 * monster.
                 */
                /*
                 * A beam or bolt will hit either
                 * player or mount. Choose randomly.
                 */
                else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE))
                {
                    if (one_in_(2))
                    {
                        /* Hit the mount with full damage */
                    }
                    else
                    {
                        /* Hit the player later */
                        flg |= PROJECT_PLAYER;

                        /* Don't affect the mount */
                        continue;
                    }
                }

                /*
                 * The spell is not well aimed, so
                 * partly affect both player and
                 * mount.
                 */
                else
                {
                    effective_dist++;
                }
            }

            /* Affect the monster in the grid */
            if (project_m(who, effective_dist, y, x, dam, typ, flg, see_s_msg)) notice = TRUE;
        }

        /* Hack: Handle device lore for offensive effects. We could do this on
         * a case by case basis in do_effect(), but this is easier. Of course,
         * we don't know whether or not project() was called from a device at
         * this point, but erroneously setting device_lore is harmless. */
        if (hack_max_m_dam >= dam) /* && one_in_(?) */
            device_lore = TRUE;

        /* Player affected one monster (without "jumping") */
        if (!who && (project_m_n == 1) && !jump)
        {
            /* Location */
            x = project_m_x;
            y = project_m_y;

            /* Track if possible */
            if (cave[y][x].m_idx > 0)
            {
                monster_type *m_ptr = &m_list[cave[y][x].m_idx];

                if (m_ptr->ml)
                {
                    /* Hack -- auto-recall */
                    if (!p_ptr->image) mon_track(m_ptr);

                    /* Hack - auto-track */
                    if (m_ptr->ml) health_track(cave[y][x].m_idx);
                }
            }
        }
    }


    /* Check player */
    if (flg & (PROJECT_KILL))
    {
        /* XXX If project_p moves the player short range, then there
         * is the possibility that the effect projects on them more
         * than once. For example, Kavlax can breathe nexus for 500,
         * which is a bit harsh, don't you think? This is also a big
         * issue with BR_STORM which does 300 dam + short teleport. */
        bool did_project_p = FALSE;

        /* Start with "dist" of zero */
        dist = 0;

        /* Scan for player */
        for (i = 0; i < grids; i++)
        {
            int effective_dist;

            /* Hack -- Notice new "dist" values */
            if (gm[dist+1] == i) dist++;

            /* Get the grid location */
            y = gy[i];
            x = gx[i];

            /* Affect the player? */
            if (!player_bold(y, x)) continue;

            /* Find the closest point in the blast */
            if (breath)
            {
                effective_dist = dist_to_line(y, x, y1, x1, by, bx);
            }
            else
            {
                effective_dist = dist;
            }

            /* Target may be your horse */
            if (p_ptr->riding)
            {
                /* Aimed on the player */
                if (flg & PROJECT_PLAYER)
                {
                    /* Hit the player with full damage */
                }

                /*
                 * Hack -- When this grid was not the
                 * original target, a beam or bolt
                 * would hit either player or mount,
                 * and should be choosen randomly.
                 *
                 * But already choosen to hit the
                 * mount at this point.
                 *
                 * Or aimed on your horse.
                 */
                else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
                {
                    /*
                     * A beam or bolt is well aimed
                     * at the mount!
                     * So don't affects the player.
                     */
                    continue;
                }
                else
                {
                    /*
                     * The spell is not well aimed,
                     * So partly affect the player too.
                     */
                    effective_dist++;
                }
            }

            /* Affect the player ... at most once! */
            if (!did_project_p)
            {
                if (project_p(who, who_name, effective_dist, y, x, dam, typ, flg)) notice = TRUE;
                did_project_p = TRUE;
            }
        }
    }

    if (p_ptr->riding)
    {
        char m_name[80];

        monster_desc(m_name, &m_list[p_ptr->riding], 0);

        if (rakubadam_m > 0)
        {
            if (rakuba(rakubadam_m, FALSE))
            {
                msg_format("%^s has thrown you off!", m_name);
            }
        }
        if (p_ptr->riding && rakubadam_p > 0)
        {
            if(rakuba(rakubadam_p, FALSE))
            {
                msg_format("You have fallen from %s.", m_name);
            }
        }
    }

    if (who > 0 && p_ptr->spell_turned)
        project(-1, old_rad, y1, x1, dam, typ, flg);

    /* Return "something was noticed" */
    return (notice);
}

bool binding_field( int dam )
{
    int mirror_x[10],mirror_y[10];
    int mirror_num=0;
    int x,y;
    int centersign;
    int x1,x2,y1,y2;
    u16b p;
    int msec= delay_factor*delay_factor*delay_factor;

    int point_x[3];
    int point_y[3];

    /* Default target of monsterspell is player */
    monster_target_y=py;
    monster_target_x=px;

    for( x=0 ; x < cur_wid ; x++ )
    {
        for( y=0 ; y < cur_hgt ; y++ )
        {
            if( is_mirror_grid(&cave[y][x]) &&
                distance(py,px,y,x) <= MAX_RANGE &&
                distance(py,px,y,x) != 0 &&
                player_has_los_bold(y,x) &&
                projectable(py, px, y, x)
                ){
                mirror_y[mirror_num]=y;
                mirror_x[mirror_num]=x;
                mirror_num++;
            }
        }
    }

    if( mirror_num < 2 )return FALSE;

    point_x[0] = randint0( mirror_num );
    do {
      point_x[1] = randint0( mirror_num );
    }
    while( point_x[0] == point_x[1] );

    point_y[0]=mirror_y[point_x[0]];
    point_x[0]=mirror_x[point_x[0]];
    point_y[1]=mirror_y[point_x[1]];
    point_x[1]=mirror_x[point_x[1]];
    point_y[2]=py;
    point_x[2]=px;

    x=point_x[0]+point_x[1]+point_x[2];
    y=point_y[0]+point_y[1]+point_y[2];

    centersign = (point_x[0]*3-x)*(point_y[1]*3-y)
        - (point_y[0]*3-y)*(point_x[1]*3-x);
    if( centersign == 0 )return FALSE;

    x1 = point_x[0] < point_x[1] ? point_x[0] : point_x[1];
    x1 = x1 < point_x[2] ? x1 : point_x[2];
    y1 = point_y[0] < point_y[1] ? point_y[0] : point_y[1];
    y1 = y1 < point_y[2] ? y1 : point_y[2];

    x2 = point_x[0] > point_x[1] ? point_x[0] : point_x[1];
    x2 = x2 > point_x[2] ? x2 : point_x[2];
    y2 = point_y[0] > point_y[1] ? point_y[0] : point_y[1];
    y2 = y2 > point_y[2] ? y2 : point_y[2];

    for( y=y1 ; y <=y2 ; y++ ){
        for( x=x1 ; x <=x2 ; x++ ){
            if( centersign*( (point_x[0]-x)*(point_y[1]-y)
                     -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
                centersign*( (point_x[1]-x)*(point_y[2]-y)
                     -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
                centersign*( (point_x[2]-x)*(point_y[0]-y)
                     -(point_y[2]-y)*(point_x[0]-x)) >=0 )
            {
                if (player_has_los_bold(y, x) && projectable(py, px, y, x))
                {
                    /* Visual effects */
                    if(!(p_ptr->blind)
                       && panel_contains(y,x)){
                      p = bolt_pict(y,x,y,x, GF_MANA );
                      print_rel(PICT_C(p), PICT_A(p),y,x);
                      move_cursor_relative(y, x);
                      /*if (fresh_before)*/ Term_fresh();
                      Term_xtra(TERM_XTRA_DELAY, msec);
                    }
                }
            }
        }
    }
    for( y=y1 ; y <=y2 ; y++ ){
        for( x=x1 ; x <=x2 ; x++ ){
            if( centersign*( (point_x[0]-x)*(point_y[1]-y)
                     -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
                centersign*( (point_x[1]-x)*(point_y[2]-y)
                     -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
                centersign*( (point_x[2]-x)*(point_y[0]-y)
                     -(point_y[2]-y)*(point_x[0]-x)) >=0 )
            {
                if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
                    (void)project_f(0,0,y,x,dam,GF_MANA);
                }
            }
        }
    }
    for( y=y1 ; y <=y2 ; y++ ){
        for( x=x1 ; x <=x2 ; x++ ){
            if( centersign*( (point_x[0]-x)*(point_y[1]-y)
                     -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
                centersign*( (point_x[1]-x)*(point_y[2]-y)
                     -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
                centersign*( (point_x[2]-x)*(point_y[0]-y)
                     -(point_y[2]-y)*(point_x[0]-x)) >=0 )
            {
                if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
                    (void)project_o(0,0,y,x,dam,GF_MANA);
                }
            }
        }
    }
    for( y=y1 ; y <=y2 ; y++ ){
        for( x=x1 ; x <=x2 ; x++ ){
            if( centersign*( (point_x[0]-x)*(point_y[1]-y)
                     -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
                centersign*( (point_x[1]-x)*(point_y[2]-y)
                     -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
                centersign*( (point_x[2]-x)*(point_y[0]-y)
                     -(point_y[2]-y)*(point_x[0]-x)) >=0 )
            {
                if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
                    (void)project_m(0,0,y,x,dam,GF_MANA,
                      (PROJECT_GRID|PROJECT_ITEM|PROJECT_KILL|PROJECT_JUMP),TRUE);
                }
            }
        }
    }
    if( one_in_(7) ){
        msg_print("The field broke a mirror!");
        remove_mirror(point_y[0],point_x[0]);
    }

    return TRUE;
}


