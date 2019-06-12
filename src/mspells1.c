/* File: mspells1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Monster spells (attack player) */

#include "angband.h"


/*
 * Determine if there is a space near the player in which
 * a summoned creature can appear
 */
bool summon_possible(int y1, int x1)
{
    int y, x;

    /* Start at the player's location, and check 2 grids in each dir */
    for (y = y1 - 2; y <= y1 + 2; y++)
    {
        for (x = x1 - 2; x <= x1 + 2; x++)
        {
            /* Ignore illegal locations */
            if (!in_bounds(y, x)) continue;

            /* Only check a circular area */
            if (distance(y1, x1, y, x)>2) continue;

            /* ...nor on the Pattern */
            if (pattern_tile(y, x)) continue;

            /* Require empty floor grid in line of projection */
            if (cave_empty_bold(y, x) && projectable(y1, x1, y, x) && projectable(y, x, y1, x1)) return (TRUE);
        }
    }

    return FALSE;
}


bool raise_possible(monster_type *m_ptr)
{
    int xx, yy;
    int y = m_ptr->fy;
    int x = m_ptr->fx;
    s16b this_o_idx, next_o_idx = 0;
    cave_type *c_ptr;

    for (xx = x - 5; xx <= x + 5; xx++)
    {
        for (yy = y - 5; yy <= y + 5; yy++)
        {
            if (distance(y, x, yy, xx) > 5) continue;
            if (!los(y, x, yy, xx)) continue;
            if (!projectable(y, x, yy, xx)) continue;

            c_ptr = &cave[yy][xx];
            /* Scan the pile of objects */
            for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                /* Acquire object */
                object_type *o_ptr = &o_list[this_o_idx];

                /* Acquire next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Known to be worthless? */
                if (o_ptr->tval == TV_CORPSE)
                {
                    if (!monster_has_hostile_align(m_ptr, 0, 0, &r_info[o_ptr->pval])) return TRUE;
                }
            }
        }
    }
    return FALSE;
}


/*
 * Originally, it was possible for a friendly to shoot another friendly.
 * Change it so a "clean shot" means no equally friendly monster is
 * between the attacker and target.
 */
/*
 * Determine if a bolt spell will hit the player.
 *
 * This is exactly like "projectable", but it will
 * return FALSE if a monster is in the way.
 * no equally friendly monster is
 * between the attacker and target.
 */
bool clean_shot(int y1, int x1, int y2, int x2, bool friend)
{
    /* Must be the same as projectable() */

    int i, y, x;

    int grid_n = 0;
    u16b grid_g[512];

    /* Check the projection path */
    grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, 0);

    /* No grid is ever projectable from itself */
    if (!grid_n) return (FALSE);

    /* Final grid */
    y = GRID_Y(grid_g[grid_n-1]);
    x = GRID_X(grid_g[grid_n-1]);

    /* May not end in an unrequested grid */
    if ((y != y2) || (x != x2)) return (FALSE);

    for (i = 0; i < grid_n; i++)
    {
        y = GRID_Y(grid_g[i]);
        x = GRID_X(grid_g[i]);

        if ((cave[y][x].m_idx > 0) && !((y == y2) && (x == x2)))
        {
            monster_type *m_ptr = &m_list[cave[y][x].m_idx];
            if (friend == is_pet(m_ptr))
            {
                return (FALSE);
            }
        }
        /* Pets may not shoot through the character - TNB */
        if (player_bold(y, x))
        {
            if (friend) return (FALSE);
        }
    }

    return (TRUE);
}

/*
 * This is like clean_shot, but returns FALSE if any monster is in the way
 */
bool very_clean_shot(int y1, int x1, int y2, int x2)
{
    /* Must be the same as projectable() */

    int i, y, x;

    int grid_n = 0;
    u16b grid_g[512];

    /* Check the projection path */
    grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, 0);

    /* No grid is ever projectable from itself */
    if (!grid_n) return (FALSE);

    /* Final grid */
    y = GRID_Y(grid_g[grid_n-1]);
    x = GRID_X(grid_g[grid_n-1]);

    /* May not end in an unrequested grid */
    if ((y != y2) || (x != x2)) return (FALSE);

    for (i = 0; i < grid_n; i++)
    {
        y = GRID_Y(grid_g[i]);
        x = GRID_X(grid_g[i]);

        if ((cave[y][x].m_idx > 0) && !((y == y2) && (x == x2)) && !(player_bold(y, x))) return (FALSE);

        /* The player is in the way, the monster is targeting a non-player square */
        if ((player_bold(y, x)) && ((y != y2) || (x != x2))) return (FALSE);
    }

    return (TRUE);
}

u32b get_curse(int power, object_type *o_ptr)
{
    u32b new_curse;

    while(1)
    {
        new_curse = (1 << (randint0(MAX_CURSE)+4));
        if (power == 2)
        {
            if (!(new_curse & TRC_HEAVY_MASK)) continue;
        }
        else if (power == 1)
        {
            if (new_curse & TRC_SPECIAL_MASK) continue;
        }
        else if (power == 0)
        {
            if (new_curse & TRC_HEAVY_MASK) continue;
        }
        if (new_curse == OFC_LOW_MELEE && !object_is_weapon(o_ptr)) continue;
        if (new_curse == OFC_LOW_AC && !object_is_armour(o_ptr)) continue;
        break;
    }
    return new_curse;
}

static bool _object_is_any(object_type *o_ptr) { return TRUE; }
void curse_equipment(int chance, int heavy_chance)
{
    int slot = equip_random_slot(_object_is_any);
    if (slot)
    {
        bool         changed = FALSE;
        int          curse_power = 0;
        u32b         new_curse;
        u32b         oflgs[OF_ARRAY_SIZE];
        object_type *o_ptr = equip_obj(slot);
        char         o_name[MAX_NLEN];

        if (!o_ptr) return;
        if (randint1(100) > chance) return;
        if (p_ptr->prace == RACE_MON_ARMOR) return;

        obj_flags(o_ptr, oflgs);
        object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        if (have_flag(oflgs, OF_BLESSED) && (randint1(888) > chance))
        {
            msg_format("Your %s resists cursing!", o_name);
            return;
        }

        if ((randint1(100) <= heavy_chance) &&
            (object_is_artifact(o_ptr) || object_is_ego(o_ptr)))
        {
            if (!(o_ptr->curse_flags & OFC_HEAVY_CURSE))
                changed = TRUE;
            o_ptr->curse_flags |= OFC_HEAVY_CURSE;
            o_ptr->curse_flags |= OFC_CURSED;
            curse_power++;
        }
        else
        {
            if (!object_is_cursed(o_ptr))
                changed = TRUE;
            o_ptr->curse_flags |= OFC_CURSED;
        }
        if (heavy_chance >= 50) curse_power++;

        new_curse = get_curse(curse_power, o_ptr);
        if (!(o_ptr->curse_flags & new_curse))
        {
            changed = TRUE;
            o_ptr->curse_flags |= new_curse;
        }

        if (changed)
        {
            msg_format("There is a malignant black aura surrounding %s...", o_name);
            o_ptr->feeling = FEEL_NONE;
        }
        p_ptr->update |= PU_BONUS;
    }
}


int anti_magic_check(void)
{
    if (p_ptr->anti_magic)
        return 0;

    if (p_ptr->tim_no_spells)
        return 0;

    switch (p_ptr->pclass)
    {
    case CLASS_WARRIOR:
    case CLASS_BERSERKER:
    case CLASS_WEAPONSMITH:
    case CLASS_ARCHER:
    case CLASS_ALCHEMIST:
    case CLASS_CAVALRY:
        return 0;

    case CLASS_TOURIST:
        return 10;

    case CLASS_DUELIST:
        return 10;

    case CLASS_ROGUE:
    case CLASS_SCOUT:
    case CLASS_RANGER:
    case CLASS_PALADIN:
    case CLASS_WARRIOR_MAGE:
    case CLASS_CHAOS_WARRIOR:
    case CLASS_MONK:
    case CLASS_MYSTIC:
    case CLASS_BEASTMASTER:
    case CLASS_BLOOD_KNIGHT:
    case CLASS_MAULER:
    case CLASS_DISCIPLE:
        return 20;

    case CLASS_MINDCRAFTER:
    case CLASS_FORCETRAINER:
    case CLASS_PSION:
        return 30;

    case CLASS_MAGIC_EATER:
    case CLASS_RED_MAGE:
    case CLASS_DEVICEMASTER:
        return 50;
    case CLASS_MONSTER:
        switch (p_ptr->prace)
        {
        case RACE_MON_LICH:
        case RACE_MON_BEHOLDER:
        case RACE_MON_QUYLTHULG:
        case RACE_MON_RING:
            return 100;

        case RACE_MON_POSSESSOR:
        case RACE_MON_MIMIC:
            return 10; /* XXX Depends on current_r_idx */

        case RACE_MON_DRAGON:
        case RACE_MON_ANGEL:
        case RACE_MON_DEMON:
        case RACE_MON_LEPRECHAUN:
        case RACE_MON_VAMPIRE:
            return 50;

        case RACE_MON_JELLY:
        case RACE_MON_SPIDER:
        case RACE_MON_XORN:
        case RACE_MON_HOUND:
        case RACE_MON_GIANT:
        case RACE_MON_HYDRA:
        case RACE_MON_TROLL:
        case RACE_MON_ELEMENTAL:
        case RACE_MON_SWORD:
        case RACE_MON_ARMOR:
        case RACE_MON_GOLEM:
        case RACE_MON_CENTIPEDE:
        case RACE_MON_VORTEX:
        case RACE_MON_ORC:
        case RACE_MON_PUMPKIN:
            return 0;
        }
    }

    return 10;
}

/*
 * Check should monster cast dispel spell.
 */
bool dispel_check(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* TODO: Monsters should have to learn this! */
    if (psion_mental_fortress() && !one_in_(12)) return FALSE;

    if (p_ptr->tim_slay_sentient) return TRUE;

    /* Invulnabilty (including the song) */
    if (IS_INVULN()) return (TRUE);

    /* Wraith form */
    if (IS_WRAITH()) return (TRUE);

    /* Shield */
    if (p_ptr->shield) return (TRUE);

    /* Magic defence */
    if (p_ptr->magicdef) return (TRUE);

    /* Multi Shadow */
    if (p_ptr->multishadow) return (TRUE);

    /* Robe of dust */
    if (p_ptr->dustrobe) return (TRUE);

    /* Berserk Strength */
    if (IS_SHERO() && (p_ptr->pclass != CLASS_BERSERKER) && (!beorning_is_(BEORNING_FORM_BEAR))) return (TRUE);

    /* Powerful Mimickry: Note Colossus and Demon-Lord have insane XP requirements,
       so will always trigger a dispel. */
    if (p_ptr->mimic_form != MIMIC_NONE)
    {
        if (randint1(500) < get_race()->exp) return TRUE;
    }

    /* Craft Munckin Checks :) */
    if (p_ptr->tim_force) return TRUE;
    if (p_ptr->tim_enlarge_weapon) return TRUE;
    if (p_ptr->tim_field) return TRUE;
    if (p_ptr->kabenuke) return TRUE;

    /* Elemental resistances */
    #if 0
    if (r_ptr->flags4 & RF4_BR_ACID)
    {
        if (res_pct(RES_ACID) <= 75 && (p_ptr->oppose_acid || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_ACID) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_FIRE)
    {
        if (res_pct(RES_FIRE) <= 75 && (p_ptr->oppose_fire || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_FIRE) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_ELEC)
    {
        if (res_pct(RES_ELEC) <= 75 && (p_ptr->oppose_elec || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_ELEC) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_COLD)
    {
        if (res_pct(RES_COLD) <= 75 && (p_ptr->oppose_cold || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_COLD) return (TRUE);
    }

    if (r_ptr->flags4 & (RF4_BR_POIS | RF4_BR_NUKE))
    {
        if (p_ptr->oppose_pois || music_singing(MUSIC_RESIST)) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_POIS) return (TRUE);
    }
    #endif

    /* Ultimate resistance */
    if (p_ptr->ult_res) return (TRUE);

    /* Potion of Neo Tsuyosi special */
    if (p_ptr->tsuyoshi) return (TRUE);

    /* Elemental Brands */
    if ((p_ptr->special_attack & ATTACK_ACID) && !(r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_FIRE) && !(r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_ELEC) && !(r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_COLD) && !(r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_POIS) && !(r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)) return (TRUE);

    /* Speed */
    if (p_ptr->pspeed < 145)
    {
        if (IS_FAST()) return (TRUE);
    }

    /* Light speed */
    if (IS_LIGHT_SPEED() && (m_ptr->mspeed < 136)) return (TRUE);

    /* Filibuster */
    if ((p_ptr->filibuster) && (m_ptr->mspeed < p_ptr->pspeed)) return (TRUE);

    if (p_ptr->riding && (m_list[p_ptr->riding].mspeed < 135))
    {
        if (MON_FAST(&m_list[p_ptr->riding])) return (TRUE);
    }

    if (psion_check_dispel()) return TRUE;

    if ( p_ptr->prace == RACE_MON_MIMIC
      && p_ptr->current_r_idx != MON_MIMIC )
    {
        int lvl = r_info[p_ptr->current_r_idx].level;
        if (lvl >= 50 && randint1(100) < lvl)
            return TRUE;
    }

    /* No need to cast dispel spell */
    return (FALSE);
}


