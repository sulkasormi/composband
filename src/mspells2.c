/* File: mspells2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Monster spells (attack monster) */

#include "angband.h"

/*
 * Determine if a beam spell will hit the target.
 */
bool direct_beam(int y1, int x1, int y2, int x2, monster_type *m_ptr)
{
    bool hit2 = FALSE;
    int i, y, x;

    int grid_n = 0;
    u16b grid_g[512];

    bool friend = (is_pet(m_ptr) || is_friendly(m_ptr));

    /* Check the projection path */
    grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, PROJECT_THRU);

    /* No grid is ever projectable from itself */
    if (!grid_n) return (FALSE);

    for (i = 0; i < grid_n; i++)
    {
        y = GRID_Y(grid_g[i]);
        x = GRID_X(grid_g[i]);

        if (y == y2 && x == x2)
            hit2 = TRUE;
        else if (friend && cave[y][x].m_idx > 0 &&
             !are_enemies(m_ptr, &m_list[cave[y][x].m_idx]))
        {
            /* Friends don't shoot friends */
            return FALSE;
        }

        if (friend && player_bold(y, x))
            return FALSE;
    }
    if (!hit2)
        return FALSE;
    return TRUE;
}

bool breath_direct(int y1, int x1, int y2, int x2, int rad, int typ, bool friend)
{
    /* Must be the same as projectable() */

    int i;

    /* Initial grid */
    int y = y1;
    int x = x1;

    int grid_n = 0;
    u16b grid_g[512];

    int grids = 0;
    byte gx[1024], gy[1024];
    byte gm[32];
    int gm_rad = rad;

    bool hit2 = FALSE;
    bool hityou = FALSE;

    int flg;

    switch (typ)
    {
    case GF_LITE:
    case GF_LITE_WEAK:
        flg = PROJECT_LOS;
        break;
    case GF_DISINTEGRATE:
        flg = PROJECT_DISI;
        break;
    default:
        flg = 0;
        break;
    }

    /* Check the projection path */
    grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, flg);

    /* Project along the path */
    for (i = 0; i < grid_n; ++i)
    {
        int ny = GRID_Y(grid_g[i]);
        int nx = GRID_X(grid_g[i]);

        if (flg & PROJECT_DISI)
        {
            /* Hack -- Balls explode before reaching walls */
            if (cave_stop_disintegration(ny, nx)) break;
        }
        else if (flg & PROJECT_LOS)
        {
            /* Hack -- Balls explode before reaching walls */
            if (!cave_los_bold(ny, nx)) break;
        }
        else
        {
            /* Hack -- Balls explode before reaching walls */
            if (!cave_have_flag_bold(ny, nx, FF_PROJECT)) break;
        }

        /* Save the "blast epicenter" */
        y = ny;
        x = nx;
    }

    grid_n = i;

    if (!grid_n)
    {
        if (flg & PROJECT_DISI)
        {
            if (in_disintegration_range(y1, x1, y2, x2) && (distance(y1, x1, y2, x2) <= rad)) hit2 = TRUE;
            if (in_disintegration_range(y1, x1, py, px) && (distance(y1, x1, py, px) <= rad)) hityou = TRUE;
        }
        else if (flg & PROJECT_LOS)
        {
            if (los(y1, x1, y2, x2) && (distance(y1, x1, y2, x2) <= rad)) hit2 = TRUE;
            if (los(y1, x1, py, px) && (distance(y1, x1, py, px) <= rad)) hityou = TRUE;
        }
        else
        {
            if (projectable(y1, x1, y2, x2) && (distance(y1, x1, y2, x2) <= rad)) hit2 = TRUE;
            if (projectable(y1, x1, py, px) && (distance(y1, x1, py, px) <= rad)) hityou = TRUE;
        }
    }
    else
    {
        breath_shape(grid_g, grid_n, &grids, gx, gy, gm, &gm_rad, rad, y1, x1, y, x, typ);

        for (i = 0; i < grids; i++)
        {
            /* Extract the location */
            y = gy[i];
            x = gx[i];

            if ((y == y2) && (x == x2)) hit2 = TRUE;
            if (player_bold(y, x)) hityou = TRUE;
        }
    }

    if (!hit2) return FALSE;
    if (friend && hityou) return FALSE;

    return TRUE;
}

/*
 * Get the actual center point of ball spells (rad > 1) (originally from TOband)
 */
void get_project_point(int sy, int sx, int *ty, int *tx, int flg)
{
    u16b path_g[128];
    int  path_n, i;

    path_n = project_path(path_g, MAX_RANGE, sy, sx, *ty, *tx, flg);

    *ty = sy;
    *tx = sx;

    /* Project along the path */
    for (i = 0; i < path_n; i++)
    {
        sy = GRID_Y(path_g[i]);
        sx = GRID_X(path_g[i]);

        /* Hack -- Balls explode before reaching walls */
        if (!cave_have_flag_bold(sy, sx, FF_PROJECT)) break;

        *ty = sy;
        *tx = sx;
    }
}


