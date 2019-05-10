/* File: monster1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: describe monsters (using monster memory)
   Note: The (beloved?) roff_aux() has moved to mon_display.c
*/

#include "angband.h"


/* Monster saving throws versus player attacks.

I changed this to competing dice rolls. Evaluating this
change is best done in a spread sheet, but here is a sample
assuming end game max player power (CL50 + max save stat):

ML    New    Old
10    6.1%     0%
20   11.7%     0%
30   17.2%     0%
40   22.8%     0%
50   28.3%    10%
60   33.9%    20%
70   39.4%    30%
80   45.0%    40%
90   50.6%    50%
100  55.5%    60%
110  59.5%    70%
120  62.9%    80%
130  65.8%    90%
140  68.2%   100%
150  70.3%   100%

The player is now slightly less overwhelming vs. weaker monsters
while end game uniques keep their nearly 60% fail (Serpent goes from
power 100 to 127 with this change so his save goes from 60% to 65%).
*/

int mon_save_r_level(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    int           ml = r_ptr->level;

    if (r_ptr->flags1 & RF1_UNIQUE)
        ml += ml/5;

    if (ml < 1)
        ml = 1;

    return ml;
}

bool mon_save_aux(int r_idx, int power)
{
    int  ml = mon_save_r_level(r_idx);
    bool result = FALSE;

    if (power < 1)
        power = 1;

    /*if (p_ptr->wizard)
        msg_format("mon_save_aux: 1d%d <= 1d%d", power, ml);*/
    if (randint1(power) <= randint1(ml))
        result = TRUE;

    return result;
}

bool mon_save_p(int r_idx, int stat)
{
    int pl = p_ptr->lev;

    if (stat >= 0 && stat < 6)
        pl += adj_stat_save[p_ptr->stat_ind[stat]];

    return mon_save_aux(r_idx, pl);
}

bool mon_save_m(int r_idx, int src_r_idx)
{
    return mon_save_aux(r_idx, mon_save_r_level(src_r_idx));
}

void mon_lore_1(monster_type *m_ptr, u32b mask)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_1(&r_info[m_ptr->r_idx], mask);
}

void mon_lore_2(monster_type *m_ptr, u32b mask)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_2(&r_info[m_ptr->r_idx], mask);
}

void mon_lore_3(monster_type *m_ptr, u32b mask)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_3(&r_info[m_ptr->r_idx], mask);
}

void mon_lore_r(monster_type *m_ptr, u32b mask)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_r(&r_info[m_ptr->r_idx], mask);
}

void mon_lore_blow(monster_type *m_ptr, mon_blow_ptr blow, int options)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_blow(&r_info[m_ptr->r_idx], blow, options);
}

void mon_lore_aux_blow(monster_race *r_ptr, mon_blow_ptr blow, int options)
{
    if (!(options & MON_BLOW_SILLY))
    {
        if ( (options & MON_BLOW_OBVIOUS)
          || (options & MON_BLOW_DAMAGE)
          || blow->lore > 10 )
        {
            if (blow->lore < MAX_SHORT)
            {
                blow->lore++;
                if (r_ptr->id == p_ptr->monster_race_idx)
                    p_ptr->window |= PW_MONSTER;
            }
        }
    }
}

void mon_lore_effect(monster_type *m_ptr, mon_effect_ptr effect)
{
    if (is_original_ap_and_seen(m_ptr))
        mon_lore_aux_effect(&r_info[m_ptr->r_idx], effect);
}

void mon_lore_aux_effect(monster_race *r_ptr, mon_effect_ptr effect)
{
    if (effect->lore < MAX_SHORT)
    {
        effect->lore++;
        if (r_ptr->id == p_ptr->monster_race_idx)
            p_ptr->window |= PW_MONSTER;
    }
}

void mon_lore_spell(mon_ptr mon, mon_spell_ptr spell)
{
    if (is_original_ap_and_seen(mon))
        mon_lore_aux_spell(&r_info[mon->r_idx], spell);
}

void mon_lore_aux_spell(mon_race_ptr race, mon_spell_ptr spell)
{
    if (spell->lore < MAX_SHORT)
    {
        spell->lore++;
        if (race->id == p_ptr->monster_race_idx)
            p_ptr->window |= PW_MONSTER;
    }
    mon_lore_aux_spell_turns(race);
}

void mon_lore_aux_spell_turns(mon_race_ptr race)
{
    u32b old = race->r_spell_turns;
    race->r_spell_turns++;
    if (race->r_spell_turns < old) /* wrap? */
        race->r_spell_turns = old;

    if (race->r_spell_turns != old && race->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

void mon_lore_aux_1(monster_race *r_ptr, u32b mask)
{
    u32b old = r_ptr->r_flags1;

    r_ptr->r_flags1 |= (r_ptr->flags1 & mask);
    if (r_ptr->r_flags1 != old && r_ptr->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

void mon_lore_aux_2(monster_race *r_ptr, u32b mask)
{
    u32b old = r_ptr->r_flags2;

    r_ptr->r_flags2 |= (r_ptr->flags2 & mask);
    if (r_ptr->r_flags2 != old && r_ptr->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

void mon_lore_aux_3(monster_race *r_ptr, u32b mask)
{
    u32b old = r_ptr->r_flags3;

    r_ptr->r_flags3 |= (r_ptr->flags3 & mask);
    if (r_ptr->r_flags3 != old && r_ptr->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

static void _mon_lore_aux_move(monster_race *r_ptr)
{
    u32b old = r_ptr->r_move_turns;
    r_ptr->r_move_turns++;
    if (r_ptr->r_move_turns < old) /* wrap? */
        r_ptr->r_move_turns = old;
    if (r_ptr->r_move_turns != old && r_ptr->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

void mon_lore_move(monster_type *m_ptr)
{
    if (is_original_ap_and_seen(m_ptr))
        _mon_lore_aux_move(&r_info[m_ptr->r_idx]);
}

void mon_lore_aux_r(monster_race *r_ptr, u32b mask)
{
    u32b old = r_ptr->r_flagsr;

    r_ptr->r_flagsr |= (r_ptr->flagsr & mask);
    if (r_ptr->r_flagsr != old && r_ptr->id == p_ptr->monster_race_idx)
        p_ptr->window |= PW_MONSTER;
}

/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
void roff_top(int r_idx)
{
    monster_race    *r_ptr = &r_info[r_idx];

    byte        a1, a2;
    char        c1, c2;


    /* Access the chars */
    c1 = r_ptr->d_char;
    c2 = r_ptr->x_char;

    /* Access the attrs */
    a1 = r_ptr->d_attr;
    a2 = r_ptr->x_attr;


    /* Clear the top line */
    Term_erase(0, 0, 255);

    /* Reset the cursor */
    Term_gotoxy(0, 0);

    /* A title (use "The" for non-uniques) */
    if (!(r_ptr->flags1 & RF1_UNIQUE))
    {
        Term_addstr(-1, TERM_WHITE, "The ");
    }

    /* Dump the name */
    Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

    /* Append the "standard" attr/char info */
    Term_addstr(-1, TERM_WHITE, " ('");
    Term_add_bigch(a1, c1);
    Term_addstr(-1, TERM_WHITE, "')");

    /* Append the "optional" attr/char info */
    Term_addstr(-1, TERM_WHITE, "/('");
    Term_add_bigch(a2, c2);
    Term_addstr(-1, TERM_WHITE, "'):");

    /* Wizards get extra info */
    if (p_ptr->wizard)
    {
        char buf[6];

        sprintf(buf, "%d", r_idx);

        Term_addstr(-1, TERM_WHITE, " (");
        Term_addstr(-1, TERM_L_BLUE, buf);
        Term_addch(TERM_WHITE, ')');
    }
}


bool mon_hook_dungeon(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & RF8_WILD_ONLY)
    {
        dungeon_info_type *d_ptr = &d_info[dungeon_type];
        if (no_wilderness && (r_ptr->flags1 & RF1_UNIQUE)) return TRUE;
        if ((d_ptr->mflags8 & RF8_WILD_MOUNTAIN) && (r_ptr->flags8 & RF8_WILD_MOUNTAIN)) return TRUE;
        return FALSE;
    }
    else
        return TRUE;
}

static bool _mon_hook_wild_daytime_check(int r_idx)
{
    bool result = TRUE;
    if (is_daytime())
    {
        monster_race *r_ptr = &r_info[r_idx];
        if (r_ptr->flags3 & RF3_HURT_LITE)
            return FALSE;
    }
    return result;
}

static bool mon_hook_ocean(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & RF8_WILD_OCEAN)
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_shore(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & RF8_WILD_SHORE)
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_waste(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & (RF8_WILD_WASTE | RF8_WILD_ALL))
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_town(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & (RF8_WILD_TOWN | RF8_WILD_ALL))
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_wood(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & (RF8_WILD_WOOD | RF8_WILD_ALL))
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_volcano(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & RF8_WILD_VOLCANO)
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_mountain(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & RF8_WILD_MOUNTAIN)
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_grass(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags8 & (RF8_WILD_GRASS | RF8_WILD_ALL))
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_deep_water(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (!mon_hook_dungeon(r_idx)) return FALSE;

    if (r_ptr->flags7 & RF7_AQUATIC)
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_shallow_water(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (!mon_hook_dungeon(r_idx)) return FALSE;

    if (r_ptr->flags2 & RF2_AURA_FIRE)
        return FALSE;
    else
        return _mon_hook_wild_daytime_check(r_idx);
}


static bool mon_hook_lava(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (!mon_hook_dungeon(r_idx)) return FALSE;

    if (((r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK) ||
         (r_ptr->flags7 & RF7_CAN_FLY)) &&
        !(r_ptr->flags3 & RF3_AURA_COLD))
        return _mon_hook_wild_daytime_check(r_idx);
    else
        return FALSE;
}


static bool mon_hook_floor(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (!(r_ptr->flags7 & RF7_AQUATIC) ||
        (r_ptr->flags7 & RF7_CAN_FLY))
        return TRUE;
    else
        return FALSE;
}

monster_hook_type get_wilderness_monster_hook(int x, int y)
{
    if (wilderness[y][x].town)
        return mon_hook_town;

    switch (wilderness[y][x].terrain)
    {
    case TERRAIN_TOWN: /* Probably no longer used ... ? */
        return mon_hook_town;
    case TERRAIN_DEEP_WATER:
        return mon_hook_ocean;
    case TERRAIN_SHALLOW_WATER:
    case TERRAIN_SWAMP:
        return mon_hook_shore;
    case TERRAIN_DIRT:
    case TERRAIN_DESERT:
        return mon_hook_waste;
    case TERRAIN_GRASS:
        return mon_hook_grass;
    case TERRAIN_TREES:
        return mon_hook_wood;
    case TERRAIN_SHALLOW_LAVA:
    case TERRAIN_DEEP_LAVA:
        return mon_hook_volcano;
    case TERRAIN_MOUNTAIN:
        return mon_hook_mountain;
    default:
        return mon_hook_dungeon;
    }
}

monster_hook_type get_monster_hook(void)
{
    if (py_on_surface())
        return get_wilderness_monster_hook(p_ptr->wilderness_x, p_ptr->wilderness_y);
    else
        return (monster_hook_type)mon_hook_dungeon;
}


monster_hook_type get_monster_hook2(int y, int x)
{
    feature_type *f_ptr = &f_info[cave[y][x].feat];

    /* Set the monster list */

    /* Water */
    if (have_flag(f_ptr->flags, FF_WATER))
    {
        /* Deep water */
        if (have_flag(f_ptr->flags, FF_DEEP))
        {
            return (monster_hook_type)mon_hook_deep_water;
        }

        /* Shallow water */
        else
        {
            return (monster_hook_type)mon_hook_shallow_water;
        }
    }

    /* Lava */
    else if (have_flag(f_ptr->flags, FF_LAVA))
    {
        return (monster_hook_type)mon_hook_lava;
    }

    else return (monster_hook_type)mon_hook_floor;
}


void set_friendly(monster_type *m_ptr)
{
    m_ptr->smart |= (1U << SM_FRIENDLY);
}

void set_friendly_ingame(monster_type *m_ptr)
{
    m_ptr->smart |= (1U << SM_FRIENDLY);
    quests_on_kill_mon(m_ptr);
    politician_set_friend(m_ptr->r_idx, TRUE);
}

void set_pet(monster_type *m_ptr)
{
    if (!allow_pets) return;
    if (!is_pet(m_ptr)) check_pets_num_and_align(m_ptr, TRUE);

    quests_on_kill_mon(m_ptr);

    m_ptr->smart |= (1U << SM_PET);
    m_ptr->mflag2 |= MFLAG2_WASPET;
    if (!(r_info[m_ptr->r_idx].flags3 & (RF3_EVIL | RF3_GOOD)))
        m_ptr->sub_align = SUB_ALIGN_NEUTRAL;
    politician_set_friend(m_ptr->r_idx, TRUE);
}

/*
 * Makes the monster hostile towards the player
 */
void set_hostile(monster_type *m_ptr)
{   
    int i;
    bool was_pet = FALSE;
    if (p_ptr->inside_battle) return;
    if (is_hostile(m_ptr)) return; /* Nothing to do */

    if (is_pet(m_ptr)) 
    {
        check_pets_num_and_align(m_ptr, FALSE);
        was_pet = TRUE;
    }

    m_ptr->smart &= ~(1U << SM_PET);
    m_ptr->smart &= ~(1U << SM_FRIENDLY);
    politician_set_friend(m_ptr->r_idx, FALSE);
    for (i = 1; i < m_max; i++) /* The monster's dependents also turn hostile */
    {
        monster_type *mon = &m_list[i];
        bool seuraaja = FALSE;
        if (!mon || !mon->r_idx) continue; /* No monster */
        if (i == p_ptr->riding) continue; /* Not the mount */
        if (!mon->pack_idx && !mon->parent_m_idx) continue;
        if (is_hostile(mon)) continue;
        if (mon->parent_m_idx == m_ptr->id) seuraaja = TRUE;
        else if ((mon->pack_idx) && (pack_info_list[mon->pack_idx].leader_idx == m_ptr->id)) seuraaja = TRUE;
        if (seuraaja) {
            monster_race *r_ptr = &r_info[mon->r_idx];
            bool same_type = (r_ptr->d_char == r_info[m_ptr->r_idx].d_char);
            bool hostile_align = monster_has_hostile_align(NULL, 0, -1, r_ptr);
            if ((is_pet(mon)) && (!mon->pack_idx))
            {
                if ((!was_pet) || (one_in_(same_type ? (hostile_align ? (p_ptr->spin ? 12 : 24) : (p_ptr->spin ? 3 : 6)) : (hostile_align ? (p_ptr->spin ? 3 : 6) : (p_ptr->spin ? 1 : 3)))))
                /* The monster becomes our direct pet */
                {
                    mon_set_parent(mon, 0);
                    continue;
                }                
            }
            else if (!mon->pack_idx)
            {
                if ((!hostile_align) && (one_in_(same_type ? (p_ptr->spin ? 5 : 10) : (p_ptr->spin ? 3 : 5))))
                {
                    mon_set_parent(mon, 0);
                    continue;
                }
            }
            /* Mon gets angry */
            if (mon->ml)
            {
                char m_name[80];                     
                monster_desc(m_name, mon, 0);
                msg_format("%^s gets angry!", m_name);
            }
            set_hostile(mon); /* We call ourselves from inside a loop because that's how awesome we are */
        }
    }
}


/*
 * Anger the monster
 */
void anger_monster(monster_type *m_ptr)
{
    if (p_ptr->inside_battle) return;
    if ((is_friendly(m_ptr)) || (is_pet(m_ptr) && (one_in_(3)) &&
        (r_info[m_ptr->r_idx].max_num < 10) && (m_ptr->id != p_ptr->riding)))
    {
        char m_name[80];

        monster_desc(m_name, m_ptr, 0);
        msg_format("%^s gets angry!", m_name);

        set_hostile(m_ptr);

        virtue_add(VIRTUE_INDIVIDUALISM, 1);
        virtue_add(VIRTUE_HONOUR, -1);
        virtue_add(VIRTUE_JUSTICE, -1);
        virtue_add(VIRTUE_COMPASSION, -1);
    }
}


/*
 * Check if monster can cross terrain
 */
bool monster_can_cross_terrain(s16b feat, monster_race *r_ptr, u16b mode)
{
    feature_type *f_ptr = &f_info[feat];
    bool          lev = FALSE;

    if ((mode & CEM_RIDING) && p_ptr->prace == RACE_MON_RING && p_ptr->levitation)
        lev = TRUE;
    if ((mode & CEM_MIMIC) && p_ptr->levitation)
        lev = TRUE;

    /* Pattern */
    if (have_flag(f_ptr->flags, FF_PATTERN))
    {
        if (!(mode & CEM_RIDING))
        {
            if (!(r_ptr->flags7 & RF7_CAN_FLY)) return FALSE;
        }
        else
        {
            if (!(mode & CEM_P_CAN_ENTER_PATTERN)) return FALSE;
        }
    }

    /* "CAN" flags */
    if (have_flag(f_ptr->flags, FF_CAN_FLY) && ((r_ptr->flags7 & RF7_CAN_FLY) || lev)) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_CLIMB) && (r_ptr->flags7 & RF7_CAN_CLIMB)) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_SWIM) && ((r_ptr->flags7 & RF7_CAN_SWIM) || lev)) return TRUE;
    if (have_flag(f_ptr->flags, FF_CAN_PASS))
    {
        if ((r_ptr->flags2 & RF2_PASS_WALL) && (!(mode & CEM_RIDING) || p_ptr->pass_wall)) return TRUE;
    }

    if (!have_flag(f_ptr->flags, FF_MOVE)) return FALSE;

    /* Some monsters can walk on mountains */
    if (have_flag(f_ptr->flags, FF_MOUNTAIN) && (r_ptr->flags8 & RF8_WILD_MOUNTAIN)) return TRUE;

    /* Water */
    if (have_flag(f_ptr->flags, FF_WATER))
    {
        if (!(r_ptr->flags7 & RF7_AQUATIC))
        {
            /* Deep water */
            if (have_flag(f_ptr->flags, FF_DEEP)) return FALSE;

            /* Shallow water */
            else if (r_ptr->flags2 & RF2_AURA_FIRE) return FALSE;
        }
    }

    /* Aquatic monster into non-water? */
    else if (r_ptr->flags7 & RF7_AQUATIC) return FALSE;

    /* Lava */
    if (have_flag(f_ptr->flags, FF_LAVA))
    {
        if (!(r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)) return FALSE;
    }

    /* Toxic waste/acid */
    if (have_flag(f_ptr->flags, FF_ACID))
    {
        /* Deep nukage */
        if (have_flag(f_ptr->flags, FF_DEEP)) return FALSE;
	
	   if ((!(r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)) || (!(r_ptr->flagsr & RFR_EFF_IM_POIS_MASK))) return FALSE;
    }

    return TRUE;
}


/*
 * Strictly check if monster can enter the grid
 */
bool monster_can_enter(int y, int x, monster_race *r_ptr, u16b mode)
{
    cave_type *c_ptr = &cave[y][x];

    /* Player or other monster */
    if (player_bold(y, x)) return FALSE;
    if (c_ptr->m_idx) return FALSE;

    return monster_can_cross_terrain(c_ptr->feat, r_ptr, mode);
}


/*
 * Check if this monster has "hostile" alignment (aux)
 */
static bool check_hostile_align(byte sub_align1, byte sub_align2)
{
    if (sub_align1 != sub_align2)
    {
        if (((sub_align1 & SUB_ALIGN_EVIL) && (sub_align2 & SUB_ALIGN_GOOD)) ||
            ((sub_align1 & SUB_ALIGN_GOOD) && (sub_align2 & SUB_ALIGN_EVIL)))
            return TRUE;
    }

    /* Non-hostile alignment */
    return FALSE;
}


/*
 * Check if two monsters are enemies
 */
bool are_enemies(monster_type *m_ptr, monster_type *n_ptr)
{
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    monster_race *s_ptr = &r_info[n_ptr->r_idx];

    if (p_ptr->inside_battle)
    {
        if (is_pet(m_ptr) || is_pet(n_ptr)) return FALSE;
        return TRUE;
    }

    if ((r_ptr->flags8 & (RF8_WILD_TOWN | RF8_WILD_ALL))
        && (s_ptr->flags8 & (RF8_WILD_TOWN | RF8_WILD_ALL)))
    {
        if (!is_pet(m_ptr) && !is_pet(n_ptr)) return FALSE;
    }

    /* Friendly vs. opposite aligned normal or pet */
    if (check_hostile_align(m_ptr->sub_align, n_ptr->sub_align))
    {
        /* No monster fighting (option) except involving pets */
        if (!allow_hostile_monster && !is_pet(m_ptr) && !is_pet(n_ptr)) return FALSE;

        if (((m_ptr->r_idx == MON_DJINNI) || (n_ptr->r_idx == MON_DJINNI)) && (is_hostile(m_ptr) == is_hostile(n_ptr))) return FALSE;

        if (!(m_ptr->mflag2 & MFLAG2_CHAMELEON) || !(n_ptr->mflag2 & MFLAG2_CHAMELEON)) return TRUE;
    }

    /* Demeter's ents keep killing her... */
    if ((((m_ptr->r_idx == MON_DEMETER) && (n_ptr->r_idx == MON_ENT)) || ((m_ptr->r_idx == MON_ENT) && (n_ptr->r_idx == MON_DEMETER))) && (!prace_is_(RACE_ENT))) return FALSE;

    /* Hostile vs. non-hostile */
    if (is_hostile(m_ptr) != is_hostile(n_ptr))
    {
        return TRUE;
    }

    /* Default */
    return FALSE;
}


/*
 * Check if this monster race has "hostile" alignment
 * If user is player, m_ptr == NULL.
 */
bool monster_has_hostile_align(monster_type *m_ptr, int pa_good, int pa_evil, monster_race *r_ptr)
{
    byte sub_align1 = SUB_ALIGN_NEUTRAL;
    byte sub_align2 = SUB_ALIGN_NEUTRAL;

    if (m_ptr) /* For a monster */
    {
        sub_align1 = m_ptr->sub_align;
    }
    else /* For player */
    {
        if (p_ptr->align >= pa_good) sub_align1 |= SUB_ALIGN_GOOD;
        if (p_ptr->align <= pa_evil) sub_align1 |= SUB_ALIGN_EVIL;
    }

    /* Racial alignment flags */
    if (r_ptr->flags3 & RF3_EVIL) sub_align2 |= SUB_ALIGN_EVIL;
    if (r_ptr->flags3 & RF3_GOOD) sub_align2 |= SUB_ALIGN_GOOD;

    if (check_hostile_align(sub_align1, sub_align2)) return TRUE;

    /* Non-hostile alignment */
    return FALSE;
}


/*
 * Is the monster "alive"?
 *
 * Used to determine the message to print for a killed monster.
 * ("dies", "destroyed")
 */
bool monster_living(monster_race *r_ptr)
{
    /* Non-living, undead, or demon */
    if (r_ptr->flags3 & (RF3_DEMON | RF3_UNDEAD | RF3_NONLIVING))
        return FALSE;
    else
        return TRUE;
}

bool monster_magical(monster_race *r_ptr)
{
    if ((r_ptr->spells) && (r_ptr->spells->freq >= 16))
        return TRUE;
    else
        return FALSE;
}

