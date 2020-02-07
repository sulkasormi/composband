/* File: dungeon.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Angband game engine */

#include "angband.h"
#include <assert.h>

#define TY_CURSE_CHANCE 200
#define CHAINSWORD_NOISE 100

static bool load = TRUE;
static int wild_regen = 20;

/*
 * Return a "feeling" (or NULL) about an item. Method 1 (Heavy).
 *
 * For strong sensing, we have now have (3.0.3 and later):
 *
 *                    egos         artifacts
 *                    =========    =========
 * average -> good -> excellent -> special
 *         -> bad  -> awful     -> terrible
 */
byte value_check_aux1(object_type *o_ptr)
{
    if (object_is_artifact(o_ptr))
    {
        if (object_is_cursed(o_ptr) || object_is_broken(o_ptr)) return FEEL_TERRIBLE;
        return FEEL_SPECIAL;
    }

    if (object_is_ego(o_ptr))
    {
        if (object_is_cursed(o_ptr) || object_is_broken(o_ptr)) return FEEL_AWFUL;
        return FEEL_EXCELLENT;
    }

    if (object_is_cursed(o_ptr)) return FEEL_BAD;
    if (object_is_broken(o_ptr)) return FEEL_BROKEN;
    if (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET) return FEEL_AVERAGE;

    if (o_ptr->to_a > 0) return FEEL_GOOD;
    if (o_ptr->tval == TV_GLOVES || o_ptr->tval == TV_BOOTS) return FEEL_AVERAGE;
    if (o_ptr->to_h + o_ptr->to_d > 0) return FEEL_GOOD;

    return FEEL_AVERAGE;
}


/*
 * Return a "feeling" (or NULL) about an item. Method 2 (Light).
 *
 * For weak sensing, we have:
 *
 * average -> enchanted
 *         -> cursed
 */
static byte value_check_aux2(object_type *o_ptr)
{
    /* Cursed items (all of them) */
    if (object_is_cursed(o_ptr) && !object_is_device(o_ptr)) return FEEL_CURSED;

    /* Broken items (all of them) */
    if (object_is_broken(o_ptr)) return FEEL_BROKEN;

    /* Artifacts -- except cursed/broken ones */
    if (object_is_artifact(o_ptr)) return FEEL_ENCHANTED;

    /* Ego-Items -- except cursed/broken ones */
    if (object_is_ego(o_ptr)) return FEEL_ENCHANTED;

    /* Good armor bonus */
    if (o_ptr->to_a > 0) return FEEL_ENCHANTED;

    /* Don't be fooled by native to-hit/dam bonuses */
    if (o_ptr->tval == TV_GLOVES || o_ptr->tval == TV_BOOTS) return FEEL_AVERAGE;

    /* Good weapon bonuses */
    if (o_ptr->to_h + o_ptr->to_d > 0) return FEEL_ENCHANTED;

    return FEEL_AVERAGE;
}

static bool _sense_strong = FALSE;

static void _sense_obj(obj_ptr obj)
{
    byte feel;
    char name[MAX_NLEN];
    bool strong = _sense_strong;

    if (obj->ident & IDENT_SENSE) return;
    if (object_is_known(obj)) return;
    if (obj->loc.where == INV_PACK && !one_in_(3)) return;

    if (!strong && p_ptr->good_luck && !randint0(13))
        strong = TRUE;
    feel = strong ? value_check_aux1(obj) : value_check_aux2(obj);
    if (!feel) return;

    /*if (disturb_minor) disturb(0, 0);*/

    object_desc(name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED);
    msg_boundary();
    if (obj->loc.where == INV_EQUIP)
    {
        msg_format("You feel the %s (%c) you are wearing %s %s...",
               name, slot_label(obj->loc.slot),
               !object_plural(obj) ? "is" : "are",
                   game_inscriptions[feel]);
    }
    else
    {
        msg_format("You feel the %s (%c) in your %s %s %s...",
               name, slot_label(obj->loc.slot),
               obj->loc.where == INV_QUIVER ? "quiver" : "pack", 
               !object_plural(obj) ? "is" : "are",
                   game_inscriptions[feel]);
    }

    obj->ident |= IDENT_SENSE;
    obj->feeling = feel;

    autopick_alter_obj(obj, destroy_feeling && obj->loc.where != INV_EQUIP);
    obj_release(obj, OBJ_RELEASE_ID | OBJ_RELEASE_QUIET);
}

/*
 * Sense the inventory
 */
static int _adj_pseudo_id(int num)
{
	int result = num * adj_pseudo_id[p_ptr->stat_ind[A_WIS]] / 100;
    int lev = p_ptr->lev;

    result = result * (625 - virtue_current(VIRTUE_KNOWLEDGE)) / 625;

    /* Hack: Pseudo-id becomes instantaneous at CL35 */
    if (lev >= 35) return 0;
    for (;;)
    {
        lev -= 5;
        if (lev < 0) break;
        result /= 2;
    }
    return result;
}

static int _get_pseudo_id_flags(void)
{
    if (p_ptr->pclass == CLASS_MONSTER)
    {
        race_t *race_ptr = get_race();
        return get_class_aux(race_ptr->pseudo_class_idx, 0)->flags;
    }
    return get_class()->flags;
}

static void sense_inventory1(void)
{
    int  plev = p_ptr->lev + 10;
    bool strong = FALSE;

    if (p_ptr->confused) return;

	if (easy_id)
        strong = TRUE;
	else
    {
		int flags = _get_pseudo_id_flags();
		if (flags & CLASS_SENSE1_STRONG)
			strong = TRUE;
		else if (!(flags & CLASS_SENSE1_WEAK))
            return;
		if (flags & CLASS_SENSE1_FAST)
			{
			if (0 != randint0(_adj_pseudo_id(9000) / (plev * plev + 40)))
				return;
			}
		else if (flags & CLASS_SENSE1_MED)
			{
			if (0 != randint0(_adj_pseudo_id(20000) / (plev * plev + 40)))
				return;
			}
		else if (flags & CLASS_SENSE1_SLOW)
			{
			if (0 != randint0(_adj_pseudo_id(80000) / (plev * plev + 40)))
				return;
			}
		if (virtue_current(VIRTUE_KNOWLEDGE) >= 100)
			strong = TRUE;
    }

    /*** Sense everything ***/
    _sense_strong = strong;
    pack_for_each_that(_sense_obj, obj_can_sense1);
    equip_for_each_that(_sense_obj, obj_can_sense1);
    quiver_for_each_that(_sense_obj, obj_can_sense1);
}


static void sense_inventory2(void)
{
    int  plev = p_ptr->lev + 10;
    bool strong = FALSE;
//    int  flags = _get_pseudo_id_flags();

    if (p_ptr->confused) return;
	if (easy_id)
	{
        strong = TRUE;
    }
    else
    {
		int flags = _get_pseudo_id_flags();
		if (flags & CLASS_SENSE2_STRONG)
			strong = TRUE;
		else if (!(flags & CLASS_SENSE2_WEAK))
        return;
		if (flags & CLASS_SENSE2_FAST)
			{
			if (0 != randint0(_adj_pseudo_id(9000) / (plev * plev + 40)))
				return;
			}
		else if (flags & CLASS_SENSE2_MED)
			{
			if (0 != randint0(_adj_pseudo_id(20000) / (plev * plev + 40)))
				return;
			}
		else if (flags & CLASS_SENSE2_SLOW)
			{
			if (0 != randint0(_adj_pseudo_id(80000) / (plev * plev + 40)))
				return;
			}
		else /* Super duper slow */
			{
			if (0 != randint0(_adj_pseudo_id(240000) / (plev + 5)))
				return;
			}
    }

    /*** Sense everything ***/
    _sense_strong = strong;
    pack_for_each_that(_sense_obj, obj_can_sense2);
    equip_for_each_that(_sense_obj, obj_can_sense2);
}

/* Random energy */
s16b energy_need_clipper(void)
{
    s16b tulos = randnor(100, 18);
    return MIN(133, MAX(67, tulos));
}

/*
 * Go to any level (ripped off from wiz_jump)
 */
static void pattern_teleport(void)
{
    int min_level = 1;
    int max_level = 99;

    if (!dungeon_type) return;

    /* Ask for level */
    if (get_check("Teleport level? "))

    {
        char    ppp[80];
        char    tmp_val[160];

        /* Only downward in ironman mode */
        if (ironman_downward)
            min_level = dun_level;

        /* Maximum level */
        if (dungeon_type == DUNGEON_ANGBAND)
        {
            if (dun_level > 100)
                max_level = MAX_DEPTH - 1;
            else if (dun_level == 100)
                max_level = 100;
        }
        else
        {
            max_level = d_info[dungeon_type].maxdepth;
            min_level = d_info[dungeon_type].mindepth;
        }

        /* Prompt */
        sprintf(ppp, "Teleport to level (%d-%d): ", min_level, max_level);


        /* Default */
        sprintf(tmp_val, "%d", dun_level);

        /* Ask for a level */
        if (!get_string(ppp, tmp_val, 10)) return;

        /* Extract request */
        command_arg = atoi(tmp_val);
    }
    else if (get_check("Normal teleport? "))
    {
        teleport_player(200, 0L);
        return;
    }
    else if (((coffee_break) || (!ironman_downward)) && (get_check("Recall? ")))
    {
        recall_player(1, FALSE);
        return;
    }
    else
    {
        return;
    }

    /* Paranoia */
    if (command_arg < min_level) command_arg = min_level;

    /* Paranoia */
    if (command_arg > max_level) command_arg = max_level;

    /* Accept request */
    msg_format("You teleport to dungeon level %d.", command_arg);

    if (autosave_l) do_cmd_save_game(TRUE);

    /* Change level */
    dun_level = command_arg;

    quests_on_leave();
    energy_use = 0;

    /*
     * Clear all saved floors
     * and create a first saved floor
     */
    prepare_change_floor_mode(CFM_FIRST_FLOOR);

    /* Leaving */
    p_ptr->leaving = TRUE;
}

static void wreck_the_pattern(void)
{
    int to_ruin = 0, r_y, r_x;
    int pattern_type = f_info[cave[py][px].feat].subtype;

    if (pattern_type == PATTERN_TILE_WRECKED)
    {
        /* Ruined already */
        return;
    }

    msg_print("You bleed on the Pattern!");
    msg_print("Something terrible happens!");

    if (!IS_INVULN())
        take_hit(DAMAGE_NOESCAPE, damroll(10, 8), "corrupting the Pattern");

    to_ruin = randint1(45) + 35;

    while (to_ruin--)
    {
        scatter(&r_y, &r_x, py, px, 4, 0);

        if (pattern_tile(r_y, r_x) &&
            (f_info[cave[r_y][r_x].feat].subtype != PATTERN_TILE_WRECKED))
        {
            cave_set_feat(r_y, r_x, feat_pattern_corrupted);
        }
    }

    cave_set_feat(py, px, feat_pattern_corrupted);
}


/* Returns TRUE if we are on the Pattern... */
static bool pattern_effect(void)
{
    int pattern_type;

    if (!pattern_tile(py, px)) return FALSE;

    if ((prace_is_(RACE_AMBERITE)) &&
        (p_ptr->cut > 0) && one_in_(10))
    {
        wreck_the_pattern();
    }

    pattern_type = f_info[cave[py][px].feat].subtype;

    switch (pattern_type)
    {
    case PATTERN_TILE_END:
        (void)set_poisoned(0, TRUE);
        (void)set_image(0, TRUE);
        (void)set_stun(0, TRUE);
        (void)set_cut(0, TRUE);
        (void)set_blind(0, TRUE);
        fear_clear_p();
        (void)do_res_stat(A_STR);
        (void)do_res_stat(A_INT);
        (void)do_res_stat(A_WIS);
        (void)do_res_stat(A_DEX);
        (void)do_res_stat(A_CON);
        (void)do_res_stat(A_CHR);
        (void)restore_level();
        (void)hp_player(1000);
        lp_player(1000);

        cave_set_feat(py, px, feat_pattern_old);

        msg_print("This section of the Pattern looks less powerful.");

        /*
         * We could make the healing effect of the
         * Pattern center one-time only to avoid various kinds
         * of abuse, like luring the win monster into fighting you
         * in the middle of the pattern...
         */
        break;

    case PATTERN_TILE_OLD:
        /* No effect */
        break;

    case PATTERN_TILE_TELEPORT:
        pattern_teleport();
        break;

    case PATTERN_TILE_WRECKED:
        if (!IS_INVULN())
            take_hit(DAMAGE_NOESCAPE, 200, "walking the corrupted Pattern");
        break;

    default:
        if (prace_is_(RACE_AMBERITE) && !one_in_(2))
            return TRUE;
        else if (!IS_INVULN())
            take_hit(DAMAGE_NOESCAPE, damroll(1, 3), "walking the Pattern");
        break;
    }

    return TRUE;
}

static void _suppress_extra_pantheons(void)
{
    u32b pantheon_flag_mask = 0;
    int i;

    if ((!game_pantheon) || (!single_pantheon)) return; /* paranoia */

    /* Initialize pantheon flag mask (with flags of all pantheons) */
    for (i = 0; i < PANTHEON_MAX; i++)
    {
        pantheon_flag_mask |= pant_list[i].flag;
    }

    /* Pick a random pantheon if one hasn't been selected yet */
    if (game_pantheon >= PANTHEON_MAX)
    {
        if (prace_is_(RACE_DEMIGOD))
        {
            switch (p_ptr->psubrace)
            {
                case DEMIGOD_APHRODITE:
                case DEMIGOD_APOLLO:
                case DEMIGOD_ARES:
                case DEMIGOD_ARTEMIS:
                case DEMIGOD_ATHENA:
                case DEMIGOD_DEMETER:
                case DEMIGOD_HADES:
                case DEMIGOD_HEPHAESTUS:
                case DEMIGOD_HERA:
                case DEMIGOD_HERMES:
                case DEMIGOD_POSEIDON:
                case DEMIGOD_ZEUS:
                    game_pantheon = PANTHEON_OLYMPIAN;
                    break;
                default:
                    game_pantheon = randint1(PANTHEON_MAX - 1);
                    break;
            }            
        }
        else
            game_pantheon = randint1(PANTHEON_MAX - 1);
    }

    /* Wipe dungeons associated with other pantheons */
    for (i = 1; i < max_d_idx; i++)
    {
        dungeon_info_type *d_ptr = &d_info[i];
        if ((d_ptr->pantheon) && (d_ptr->pantheon != game_pantheon))
        { /* Wipe the dungeon */
            WIPE(&d_info[i], dungeon_info_type);
        }
    }

    /* Suppress monsters who belong to other pantheons, and unsuppress the active pantheon */
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        u32b race_flag_match;

        if ((!r_ptr) || (!r_ptr->name)) continue;
        race_flag_match = (r_ptr->flags3 & pantheon_flag_mask);
        if ((race_flag_match > 0) && (!(r_ptr->flags3 & pant_list[game_pantheon].flag)))
        { /* Suppress member of rival pantheon */
            r_ptr->flagsx |= RFX_SUPPRESS;
        }
        else if (race_flag_match) /* Unsuppress */
        {
            r_ptr->flagsx &= ~RFX_SUPPRESS;
        }
    }
}

/*
 * Regenerate hit points                -RAK-
 */
static void regenhp(int percent)
{
    s32b new_chp;
    u32b new_chp_frac;
    s32b old_chp;

    if (p_ptr->special_defense & KATA_KOUKIJIN) return;
    if (p_ptr->action == ACTION_QUICK_WALK) return;
    if (p_ptr->action == ACTION_STALK) return;
    if (mimic_no_regen()) return;
    if (weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE) return;
    if (p_ptr->filibuster) return;

    /* Save the old hitpoints */
    old_chp = p_ptr->chp;

    /*
     * Extract the new hitpoints
     *
     * 'percent' is the Regen factor in unit (1/2^16)
     */
    new_chp = 0;
    new_chp_frac = (p_ptr->mhp * percent + PY_REGEN_HPBASE);

    /* Convert the unit (1/2^16) to (1/2^32) */
    s64b_LSHIFT(new_chp, new_chp_frac, 16);

    /* Regenerating */
    s64b_add(&(p_ptr->chp), &(p_ptr->chp_frac), new_chp, new_chp_frac);


    /* Fully healed */
    if (0 < s64b_cmp(p_ptr->chp, p_ptr->chp_frac, p_ptr->mhp, 0))
    {
        p_ptr->chp = p_ptr->mhp;
        p_ptr->chp_frac = 0;
    }

    /* Notice changes */
    if (old_chp != p_ptr->chp)
    {
        /* Redraw */
        p_ptr->redraw |= (PR_HP);

        /* Blood Knights get extra attacks depending on how wounded they are */
        if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
            p_ptr->update |= PU_BONUS;

        if (weaponmaster_is_(WEAPONMASTER_STAVES))
            p_ptr->update |= (PU_BONUS);

        wild_regen = 20;
    }
}


/*
 * Regenerate mana points
 */
static void _decay_mana(void)
{
    /* PY_REGEN_NORMAL is the Regen factor in unit (1/2^16) */
    s32b decay = 0;
    u32b decay_frac = (p_ptr->msp * 32 * PY_REGEN_NORMAL + PY_REGEN_MNBASE);

    /* Convert the unit (1/2^16) to (1/2^32) */
    s64b_LSHIFT(decay, decay_frac, 16);

    /* Decay */
    s64b_sub(&(p_ptr->csp), &(p_ptr->csp_frac), decay, decay_frac);

    /* Stop decaying */
    if (p_ptr->csp < p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
}
static void regenmana(int percent)
{
    s32b old_csp = p_ptr->csp;

    if (p_ptr->pclass == CLASS_RUNE_KNIGHT || p_ptr->pclass == CLASS_RAGE_MAGE) return;
    if (elemental_is_(ELEMENTAL_WATER)) return;
    if (mimic_no_regen())
    {
        if (p_ptr->csp > p_ptr->msp) /* Doppelganger Samurai/Mystics should still decay supercharged mana! */
            _decay_mana();
        return;
    }

    /*
     * Excess mana will decay 32 times faster than normal
     * regeneration rate.
     */
    if (p_ptr->csp > p_ptr->msp)
    {
        _decay_mana();
    }
    /* Regenerating mana (unless the player has excess mana) */
    else if (percent > 0)
    {
        /* (percent/100) is the Regen factor in unit (1/2^16) */
        s32b new_mana = 0;
        u32b new_mana_frac = (p_ptr->msp * percent / 100 + PY_REGEN_MNBASE);

        /* Convert the unit (1/2^16) to (1/2^32) */
        s64b_LSHIFT(new_mana, new_mana_frac, 16);

        /* Regenerate */
        s64b_add(&(p_ptr->csp), &(p_ptr->csp_frac), new_mana, new_mana_frac);

        /* Must set frac to zero even if equal */
        if (p_ptr->csp >= p_ptr->msp)
        {
            p_ptr->csp = p_ptr->msp;
            p_ptr->csp_frac = 0;
        }
    }


    /* Reduce mana (even when the player has excess mana) */
    if (percent < 0)
    {
        /* PY_REGEN_NORMAL is the Regen factor in unit (1/2^16) */
        s32b reduce_mana = 0;
        u32b reduce_mana_frac = (p_ptr->msp * PY_REGEN_NORMAL + PY_REGEN_MNBASE);

        if (percent < -47674L) /* Lose mana faster */
        {
            reduce_mana_frac *= ((0L - percent) / 4334);
            reduce_mana_frac /= 11;
        }

        /* Convert the unit (1/2^16) to (1/2^32) */
        s64b_LSHIFT(reduce_mana, reduce_mana_frac, 16);

        /* Reduce mana */
        s64b_sub(&(p_ptr->csp), &(p_ptr->csp_frac), reduce_mana, reduce_mana_frac);

        /* Check overflow */
        if (p_ptr->csp < 0)
        {
            p_ptr->csp = 0;
            p_ptr->csp_frac = 0;
        }
    }


    /* Redraw mana */
    if (old_csp != p_ptr->csp)
    {
        /* Redraw */
        p_ptr->redraw |= (PR_MANA);

        /* Window stuff */
        p_ptr->window |= (PW_SPELL);

        wild_regen = 20;
    }
}




/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static void regen_monsters(void)
{
    int i, frac;


    /* Regenerate everyone */
    for (i = 1; i < m_max; i++)
    {
        /* Check the i'th monster */
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];


        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Allow regeneration (if needed) */
        if (m_ptr->hp < m_ptr->maxhp)
        {
            /* Hack -- Base regeneration */
            frac = m_ptr->maxhp / 100;

            /* Hack -- Minimal regeneration rate */
            if (!frac) if (one_in_(2)) frac = 1;

            /* Hack -- Some monsters regenerate quickly */
            if (r_ptr->flags2 & RF2_REGENERATE) frac *= 2;

            /* Hack -- Ease up on the Serpent's regen (was 600hp a pop) */
            if (frac >= 400) frac = 400;

            /* Regenerate */
            m_ptr->hp += frac;

            /* Do not over-regenerate */
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

            /* Redraw (later) if needed */
            check_mon_health_redraw(i);
        }
    }
}


/*
 * Regenerate the captured monsters (once per 30 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static bool _is_captured_mon(obj_ptr obj) { return obj->tval == TV_CAPTURE && obj->pval; }
static void _regen_captured_mon(obj_ptr obj)
{
    monster_race *r_ptr = &r_info[obj->pval];

    /* Uniques and Nazguls regenerate very slowly in capture balls */ 
    if ((r_ptr->ball_num) && ((game_turn % (TURNS_PER_TICK*60)))) return;

    if (obj->xtra4 < obj->xtra5)
    {
        int amt = obj->xtra5 / 100;
        if (!amt && one_in_(2)) amt = 1;
        if (r_ptr->flags2 & RF2_REGENERATE) amt *= 2;
        obj->xtra4 = MIN(obj->xtra5, obj->xtra4 + amt);
    }
}

static void regen_captured_monsters(void)
{
    pack_for_each_that(_regen_captured_mon, _is_captured_mon);
    equip_for_each_that(_regen_captured_mon, _is_captured_mon);
}


void notice_lite_change(object_type *o_ptr)
{
    /* Hack -- notice interesting fuel steps */
    if ((o_ptr->xtra4 < 100) || (!(o_ptr->xtra4 % 100)))
    {
        /* Window stuff */
        p_ptr->window |= (PW_EQUIP);
    }

    /* Hack -- Special treatment when blind */
    if (p_ptr->blind)
    {
        /* Hack -- save some light for later */
        if (o_ptr->xtra4 == 0) o_ptr->xtra4++;
    }

    /* The light is now out */
    else if (o_ptr->xtra4 == 0)
    {
        disturb(0, 0);
        msg_print("Your light has gone out!");

        /* We now know how many turns of light it has - zero */
        if ((!obj_is_identified(o_ptr)) && (o_ptr->ident & IDENT_SENSE)
           && (o_ptr->feeling == FEEL_AVERAGE))
        identify_item(o_ptr);

        /* Recalculate torch radius */
        p_ptr->update |= (PU_TORCH);

        /* Some ego light lose its effects without fuel */
        p_ptr->update |= (PU_BONUS);
    }

    /* The light is getting dim */
    else if (o_ptr->name2 == EGO_LITE_DURATION)
    {
        if ((o_ptr->xtra4 < 50) && (!(o_ptr->xtra4 % 5))
            && (game_turn % (TURNS_PER_TICK*2)))
        {
            if (disturb_minor) disturb(0, 0);
            msg_print("Your light is growing faint.");

        }
    }

    /* The light is getting dim */
    else if ((o_ptr->xtra4 < 100) && (!(o_ptr->xtra4 % 10)))
    {
        if (disturb_minor) disturb(0, 0);
        msg_print("Your light is growing faint.");

    }
}

void fame_on_failure(void)
{
    int dec = p_ptr->fame/2;
    if (dec > 30)
        dec = 30;
    assert (dec <= p_ptr->fame);
    p_ptr->fame -= dec;
}

void gain_fame(int amt)
{
    if (coffee_break == SPEED_INSTA_COFFEE)
    {
        amt *= 6;
        amt += randint0(4);
        amt /= 4;
    }
    p_ptr->fame += amt;
}

byte coffeebreak_recall_level(bool laskuri)
{
    byte taso = (byte)max_dlv[DUNGEON_ANGBAND];
    if (taso < 99)
    {
        taso++;
        if (!level_is_questlike(DUNGEON_ANGBAND, taso)) taso++;
        if (coffee_break == SPEED_INSTA_COFFEE)
        {
            while (!level_is_questlike(DUNGEON_ANGBAND, taso))
            {
                taso++;
                if (taso >= 100) break;
            }
        }
    }
    else if ((taso == 99) && (!level_is_questlike(DUNGEON_ANGBAND, taso))) taso++;
    if ((p_ptr->total_winner) && (taso < d_info[DUNGEON_ANGBAND].maxdepth)) taso++;
    /* Mega-hack
     * coffeebreak_recall_level() is actually called every time a dungeon
     * level is entered in coffee-break mode. This means that if it's called,
     * and taso == max_dlv, we are revisiting the depth */
    if ((laskuri) && (taso == (byte)max_dlv[DUNGEON_ANGBAND]) && (p_ptr->coffee_lv_revisits < 251) && (taso < 101))
    {
        if (p_ptr->coffee_lv_revisits == 2) /* warn the player about J's increasing mpower */
        {
            msg_print("You have a brief vision of a red-faced, overweight humanoid with an unkempt beard, yelling <color:R>'Get to it already! The longer you bum around, the stronger the Serpent grows!'</color> You aren't sure who this strange apparition is, but given how tense and irritable he appears, you hope he's not the ultimate supreme deity.");
        }
        p_ptr->coffee_lv_revisits++;
    }
    return taso;
}


/*
 * Forcibly pseudo-identify an object in the inventory
 * (or on the floor)
 *
 * note: currently this function allows pseudo-id of any object,
 * including silly ones like potions & scrolls, which always
 * get '{average}'. This should be changed, either to stop such
 * items from being pseudo-id'd, or to allow psychometry to
 * detect whether the unidentified potion/scroll/etc is
 * good (Cure Light Wounds, Restore Strength, etc) or
 * bad (Poison, Weakness etc) or 'useless' (Slime Mold Juice, etc).
 */
bool psychometry(void)
{
    obj_prompt_t prompt = {0};
    char         o_name[MAX_NLEN];
    byte         feel;
    bool         okay = FALSE;

    prompt.prompt = "Meditate on which item?";
    prompt.error = "You have nothing appropriate.";
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    /* It is fully known, no information needed */
    if (object_is_known(prompt.obj))
    {
        msg_print("You cannot find out anything more about that.");

        return TRUE;
    }

    /* Check for a feeling */
    feel = value_check_aux1(prompt.obj);

    /* Get an object description */
    object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Skip non-feelings */
    if (!feel)
    {
        msg_format("You do not perceive anything unusual about the %s.", o_name);

        return TRUE;
    }

    msg_format("You feel that the %s %s %s...",
               o_name, ((!object_plural(prompt.obj)) ? "is" : "are"),
               game_inscriptions[feel]);


    /* We have "felt" it */
    prompt.obj->ident |= (IDENT_SENSE);

    /* "Inscribe" it */
    prompt.obj->feeling = feel;

    /* Player touches it */
    prompt.obj->marked |= OM_TOUCHED;

    /* Valid "tval" codes */
    switch (prompt.obj->tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_CARD:
    case TV_RING:
    case TV_AMULET:
    case TV_LITE:
    case TV_FIGURINE:
        okay = TRUE;
        break;
    }

    autopick_alter_obj(prompt.obj, okay && destroy_feeling);
    obj_release(prompt.obj, OBJ_RELEASE_ID | OBJ_RELEASE_QUIET);

    /* Something happened */
    return (TRUE);
}


/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 */
void recharged_notice(object_type *o_ptr, unsigned char neula)
{
    char o_name[MAX_NLEN];

    cptr s;

    /* No inscription */
    if (!o_ptr->inscription) return;

    /* Find a '!' */
    s = my_strchr(quark_str(o_ptr->inscription), '!');

    /* Process notification request. */
    while (s)
    {
        /* Find another '!' */
        if (s[1] == neula)
        {
            /* Describe (briefly) */
            object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_OMIT_INSCRIPTION | OD_COLOR_CODED);

            /* Notify the player */
            if (o_ptr->number > 1)
                msg_format("Your %s are recharged.", o_name);
            else if (neula != '!')
                msg_format("Your %s now has %c charge%s.", o_name, neula, (neula == '1') ? "" : "s");
            else
                msg_format("Your %s is recharged.", o_name);

            disturb(0, 0);

            /* Done. */
            return;
        }

        /* Keep looking for '!'s */
        s = my_strchr(s + 1, '!');
    }
}


/* Choose one of items that have cursed flag */
static u32b _curse_flag = 0;
static bool _object_is_cursed(object_type *o_ptr) {
    if (o_ptr->curse_flags & _curse_flag)
        return TRUE;
    return FALSE;
}
static object_type *choose_cursed_obj_name(u32b flag)
{
    int slot;
    _curse_flag = flag;
    slot = equip_random_slot(_object_is_cursed);
    if (slot)
        return equip_obj(slot);
    return NULL;
}

void do_alter_reality(void)
{
    /* Disturbing! */
    disturb(0, 0);


    /* Determine the level */
    if ((ironman_downward) || ((!dungeon_type) && (quests_get_current())))
    {
        msg_print("The world seems to change for a moment!");
        p_ptr->alter_reality = 0;
    }
    else
    {
        if (p_ptr->alter_reality) /* Mega-hack - law */
        {
            msg_print("You reject this reality and substitute your own!");
            p_ptr->alter_reality = 0;
        } 
        else msg_print("The world changes!");

        /*
         * Clear all saved floors
         * and create a first saved floor
         */
        prepare_change_floor_mode(CFM_FIRST_FLOOR);

        /* Record position */
        p_ptr->oldpx = px;
        p_ptr->oldpy = py;

        /* Leaving */
        p_ptr->leaving = TRUE;
        if (quests_get_current()) quests_on_leave();
    }

    /* Sound */
    sound(SOUND_TPLEVEL);
}

static bool _fast_mana_regen(void)
{
    switch (get_class_idx())
    {
    case CLASS_MAGE:
    case CLASS_BLOOD_MAGE:
    case CLASS_NECROMANCER:
    case CLASS_HIGH_MAGE:
    case CLASS_SORCERER:
    case CLASS_YELLOW_MAGE:
    case CLASS_GRAY_MAGE:
        return TRUE;
    }
    return FALSE;
}

/*
 * Handle timed damage and regeneration every 10 game turns
 */
static void process_world_aux_hp_and_sp(void)
{
    feature_type *f_ptr = &f_info[cave[py][px].feat];
    bool cave_no_regen = p_ptr->nice;
    int upkeep_factor = 0;
    int upkeep_regen;

    /* Default regeneration */
    int regen_amount = PY_REGEN_NORMAL;


    /*** Damage over Time ***/

    /* Take damage from cuts */
    if (p_ptr->cut && !IS_INVULN() && p_ptr->wild_mode)
    {
        cut_info_t cut = cut_info(p_ptr->cut);
        if (cut.dam)
        {
            /*msg_format("<color:r> %d Cut Damage</color>", cut.dam);*/
            take_hit(DAMAGE_NOESCAPE, cut.dam, "a fatal wound");
        }
    }

    /* (Vampires) Take damage from sunlight. Note, Vampires are vulnerable
       to light so start with -50% resistance. Rather than res_save(RES_LIGHT)
       we will simply take damage so long as there is light vulnerability. */
    if (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || p_ptr->mimic_form == MIMIC_VAMPIRE)
    {
        int slot;
        if (!dun_level && res_pct(RES_LITE) < 0 && !IS_INVULN() && is_daytime())
        {
            if ((cave[py][px].info & (CAVE_GLOW | CAVE_MNDK)) == CAVE_GLOW)
            {
                msg_print("The sun's rays scorch your undead flesh!");
                take_hit(DAMAGE_NOESCAPE, 1, "sunlight");
                cave_no_regen = TRUE;
            }
        }

        slot = equip_find_obj(TV_LITE, SV_ANY);
        if (slot)
        {
            object_type *lite = equip_obj(slot);
            u32b         flgs[OF_ARRAY_SIZE];
            obj_flags(lite, flgs);
            if ( !have_flag(flgs, OF_DARKNESS)
              && res_pct(RES_LITE) < 0)
            {
                char o_name [MAX_NLEN];
                char ouch [MAX_NLEN+40];

                object_desc(o_name, lite, OD_OMIT_PREFIX | OD_NAME_ONLY);
                msg_format("The %s scorches your undead flesh!", o_name);
                cave_no_regen = TRUE;
                object_desc(o_name, lite, OD_NAME_ONLY);
                sprintf(ouch, "wielding %s", o_name);
                if (!IS_INVULN()) take_hit(DAMAGE_NOESCAPE, 1, ouch);
            }
        }
    }

    if (have_flag(f_ptr->flags, FF_LAVA) && !IS_INVULN() && !elemental_is_(ELEMENTAL_FIRE))
    {
        int damage = 0;

        if (have_flag(f_ptr->flags, FF_DEEP))
        {
            damage = 6000 + randint0(4000);
        }
        else if (!p_ptr->levitation)
        {
            damage = 3000 + randint0(2000);
        }

        damage = res_calc_dam(RES_FIRE, damage);
        if (p_ptr->levitation) damage = damage / 5;

        if (damage)
        {
            damage = damage / 100 + (randint0(100) < (damage % 100));

            if (p_ptr->levitation)
            {
                msg_print("The heat burns you!");
                take_hit(DAMAGE_NOESCAPE, damage, format("flying over %s", f_name + f_info[get_feat_mimic(&cave[py][px])].name));
            }
            else
            {
                cptr name = f_name + f_info[get_feat_mimic(&cave[py][px])].name;
                msg_format("The %s burns you!", name);
                take_hit(DAMAGE_NOESCAPE, damage, name);
            }

            cave_no_regen = TRUE;
        }
    }

	if (have_flag(f_ptr->flags, FF_ACID) && !IS_INVULN() && !one_in_(3))
	{
		int a_damage = 0, p_damage = 0;
		bool is_deep = have_flag(f_ptr->flags, FF_DEEP);

		if (is_deep)
		{
			a_damage = 1400 + randint0(800);
		}
		else if (!p_ptr->levitation)
		{
			a_damage = 700 + randint0(400);
		}

		if (p_ptr->levitation) a_damage = a_damage / (is_deep ? 15 : 10);
		p_damage = a_damage * 6 / 5;
		a_damage = res_calc_dam(RES_ACID, a_damage);
		p_damage = res_calc_dam(RES_POIS, p_damage);

		if (a_damage > 0 || p_damage > 0)
		{
			a_damage = a_damage / 100 + (randint0(100) < (a_damage % 100));
			p_damage = p_damage / 100 + (randint0(100) < (p_damage % 100));
			if ((a_damage > 0) && (one_in_(16)) && (minus_ac())) a_damage = (a_damage + 1) / 2;

			if ((p_ptr->levitation) && ((a_damage > 0) || (p_damage > 0)))
			{
				if (a_damage) msg_print("You are burned by toxic fumes!");
				else msg_print("You are poisoned by toxic fumes!");
				take_hit(DAMAGE_NOESCAPE, a_damage, format("flying over %s", f_name + f_info[get_feat_mimic(&cave[py][px])].name));
				if ((p_damage > 0) && (a_damage == 0) && (!p_ptr->poisoned)) /* big fat hack - avoid message duplication */
				{
					p_ptr->poisoned += 1;
					p_damage -= 1;
				}				
	 			set_poisoned(p_ptr->poisoned + p_damage, FALSE);
			}
			else if ((a_damage > 0) || (p_damage > 0))
			{
				cptr name = f_name + f_info[get_feat_mimic(&cave[py][px])].name;
				msg_format("The %s burns you!", name);
				take_hit(DAMAGE_NOESCAPE, a_damage, name);
				set_poisoned(p_ptr->poisoned + p_damage, FALSE);
			}

			cave_no_regen = TRUE;

                if ((one_in_(32)) && (!res_save_default(RES_POIS)))
                do_dec_stat(A_CON);
		}
	}

    if (have_flag(f_ptr->flags, FF_WATER) && have_flag(f_ptr->flags, FF_DEEP) &&
        !p_ptr->levitation && !p_ptr->can_swim && !elemental_is_(ELEMENTAL_WATER))
    {
        if (py_total_weight() > weight_limit())
        {
            /* Take damage */
            msg_print("You are drowning!");
            take_hit(DAMAGE_NOESCAPE, randint1(p_ptr->lev), "drowning");

            cave_no_regen = TRUE;
        }
    }

    if (p_ptr->riding)
    {
        if (r_info[m_list[p_ptr->riding].r_idx].flags2 & RF2_AURA_FIRE)
        {
            int dam = r_info[m_list[p_ptr->riding].r_idx].level / 2;
            dam = res_calc_dam(RES_FIRE, dam);

            if (dam > 0)
            {
                msg_print("It's hot!");
                take_hit(DAMAGE_NOESCAPE, dam, "Fire aura");
            }
        }
        if (r_info[m_list[p_ptr->riding].r_idx].flags2 & RF2_AURA_ELEC)
        {
            int dam = r_info[m_list[p_ptr->riding].r_idx].level / 2;
            dam = res_calc_dam(RES_ELEC, dam);

            if (dam > 0)
            {
                msg_print("It hurts!");
                take_hit(DAMAGE_NOESCAPE, dam, "Elec aura");
            }
        }
        if (r_info[m_list[p_ptr->riding].r_idx].flags3 & RF3_AURA_COLD)
        {
            int dam = r_info[m_list[p_ptr->riding].r_idx].level / 2;
            dam = res_calc_dam(RES_COLD, dam);
            if (dam > 0)
            {
                msg_print("It's cold!");
                take_hit(DAMAGE_NOESCAPE, dam, "Cold aura");
            }
        }
    }

    /* Spectres -- take damage when moving through walls */
    /*
     * Added: ANYBODY takes damage if inside through walls
     * without wraith form -- NOTE: Spectres will never be
     * reduced below 0 hp by being inside a stone wall; others
     * WILL BE!
     */
    if (!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY))
    {
        if (!IS_INVULN() && !IS_WRAITH())
        {
            int dam;
            cptr dam_desc;

            dam = 1 + p_ptr->lev/5;
            /* Passwall now takes more energy ...
            dam = MAX(1 + p_ptr->lev/5, 1 + p_ptr->mhp/24);*/
            if (p_ptr->pass_wall)
            {
                if (p_ptr->no_passwall_dam)
                    dam = 0;
                else
                {
                    msg_print("Your molecules feel disrupted!");
                    dam_desc = "density";
                    if (p_ptr->prace == RACE_SPECTRE && dam > p_ptr->chp)
                        dam = p_ptr->chp;
                }
            }
            else
            {
                msg_print("You are being crushed!");
                dam_desc = "solid rock";
            }

            if (dam)
            {
                cave_no_regen = TRUE;
                take_hit(DAMAGE_NOESCAPE, dam, dam_desc);
            }
        }
    }


    /*** handle regeneration ***/

    /* Getting Weak */
    if (p_ptr->food < PY_FOOD_WEAK)
    {
        /* Lower regeneration */
        if (p_ptr->food < PY_FOOD_STARVE)
        {
            regen_amount = 0;
        }
        else if (p_ptr->food < PY_FOOD_FAINT)
        {
            regen_amount = PY_REGEN_FAINT;
        }
        else
        {
            regen_amount = PY_REGEN_WEAK;
        }
    }

    /* Are we walking the pattern? */
    if (pattern_effect())
    {
        cave_no_regen = TRUE;
    }
    else
    {
        regen_amount = regen_amount * p_ptr->regen/100;
    }

    if ( p_ptr->action == ACTION_SEARCH
      || p_ptr->action == ACTION_REST
      || p_ptr->action == ACTION_GLITTER )
    {
        regen_amount = regen_amount * 2;
    }

    upkeep_factor = calculate_upkeep();

    /* No regeneration while special action */
    if (p_ptr->action == ACTION_LEARN ||
        p_ptr->action == ACTION_QUICK_WALK ||
        p_ptr->action == ACTION_STALK ||
        (p_ptr->special_defense & KATA_KOUKIJIN) ||
        p_ptr->filibuster ||
        weaponmaster_get_toggle() == TOGGLE_SHADOW_STANCE)
    {
        upkeep_factor += 100;
    }

    /* Regenerate the mana
     * Use PY_REGEN_NORMAL as multiplier for negative regen to avoid a
     * situation where "regen-improving" things make mana loss even worse
     * and regen-worsening things make it better */
    upkeep_regen = (100 - upkeep_factor) * ((upkeep_factor > 100) ? PY_REGEN_NORMAL : regen_amount);

    if ((_fast_mana_regen()) && (upkeep_regen > 0))
        upkeep_regen = upkeep_regen * 2;

    if (!p_ptr->nice) regenmana(upkeep_regen);

    /* Mega-hack - interrupt resting if we're only resting for mana and our mana isn't regenerating */
    if ((resting < 0) && ((p_ptr->chp == p_ptr->mhp) || (mimic_no_regen()))
        && (upkeep_regen <= 0) && (!magic_eater_can_regen()) && (!samurai_can_concentrate())
        && (!p_ptr->blind) && (!p_ptr->confused) && (!p_ptr->poisoned) && (!p_ptr->afraid)
        && (!p_ptr->stun) && (!p_ptr->cut) && (!player_slow()) && (!p_ptr->paralyzed)
        && (!p_ptr->image) && (!p_ptr->word_recall) && (!p_ptr->alter_reality))
    {
        set_action(ACTION_NONE);
    }

    if (magic_eater_regen(regen_amount))
        wild_regen = 20;

    if (((p_ptr->csp == 0) && (p_ptr->csp_frac == 0)) || (elemental_is_(ELEMENTAL_WATER)))
    {
        if (p_ptr->msp == 0 && !one_in_(5))
        {
            /* Currently, non-mana characters can't have many pets. Maybe this is OK for
               warriors but what about a Trump Blood Mage? */
        }
        else
        {
            while (upkeep_factor > 100)
            {
                msg_print("Too many pets to control at once!");
                msg_print(NULL);
                do_cmd_pet_dismiss();
                upkeep_factor = calculate_upkeep();
                msg_format("Upkeep: %d%% mana.", upkeep_factor);
                msg_print(NULL);
            }
        }
    }

    /* Poisoned or cut yields no healing */
    if (p_ptr->poisoned) regen_amount = 0;
    if (p_ptr->cut && p_ptr->pclass != CLASS_BLOOD_KNIGHT) regen_amount = 0;

    /* Special floor -- Pattern, in a wall -- yields no healing */
    if (cave_no_regen) regen_amount = 0;

    regen_amount = (regen_amount * mutant_regenerate_mod) / 100;

    /* Regenerate Hit Points if needed */
    if ((p_ptr->chp < p_ptr->mhp) && !cave_no_regen)
    {
        regenhp(regen_amount);
    }
}

/*
 * Handle timeout every 10 game turns
 */
static void process_world_aux_timeout(void)
{
    /*** Timeout Various Things ***/

    process_maul_of_vice();

    if (p_ptr->prace == RACE_DOPPELGANGER)
        mimic_upkeep();

    /* Mimic */
    if (p_ptr->tim_mimic)
    {
        (void)set_mimic(p_ptr->tim_mimic - 1, p_ptr->mimic_form, TRUE);
    }

    /* Hack -- Hallucinating */
    if (p_ptr->image)
    {
        do { set_image(p_ptr->image - 1, TRUE); }
            while (p_ptr->image && res_save_default(RES_CHAOS));
    }

    /* Blindness */
    if (p_ptr->blind)
    {
        do { set_blind(p_ptr->blind - 1, TRUE); }
            while (p_ptr->blind && res_save_default(RES_BLIND));
    }

    /* Times see-invisible */
    if (p_ptr->tim_invis)
    {
        (void)set_tim_invis(p_ptr->tim_invis - 1, TRUE);
    }

    /* Timed esp */
    if (p_ptr->tim_esp)
    {
        (void)set_tim_esp(p_ptr->tim_esp - 1, TRUE);
    }

    if (p_ptr->tim_esp_magical)
    {
        (void)set_tim_esp_magical(p_ptr->tim_esp_magical - 1, TRUE);
    }

    /* Timed temporary elemental brands. -LM- */
    if (p_ptr->ele_attack)
    {
        p_ptr->ele_attack--;

        /* Clear all temporary elemental brands. */
        if (!p_ptr->ele_attack) set_ele_attack(0, 0);
    }

    /* Timed temporary elemental immune. -LM- */
    if (p_ptr->ele_immune)
    {
        p_ptr->ele_immune--;

        /* Clear all temporary elemental brands. */
        if (!p_ptr->ele_immune) set_ele_immune(0, 0);
    }

    /* Timed infra-vision */
    if (p_ptr->tim_infra)
    {
        (void)set_tim_infra(p_ptr->tim_infra - 1, TRUE);
    }

    /* Timed stealth */
    if (p_ptr->tim_stealth)
    {
        (void)set_tim_stealth(p_ptr->tim_stealth - 1, TRUE);
    }

    /* Timed levitation */
    if (p_ptr->tim_levitation)
    {
        (void)set_tim_levitation(p_ptr->tim_levitation - 1, TRUE);
    }

    /* Timed sh_touki */
    if (p_ptr->tim_sh_touki)
    {
        (void)set_tim_sh_touki(p_ptr->tim_sh_touki - 1, TRUE);
    }

    /* Timed sh_fire */
    if (p_ptr->tim_sh_fire)
    {
        (void)set_tim_sh_fire(p_ptr->tim_sh_fire - 1, TRUE);
    }

    if (p_ptr->tim_sh_elements)
        set_tim_sh_elements(p_ptr->tim_sh_elements - 1, TRUE);

    if (p_ptr->tim_sh_shards)
        set_tim_sh_shards(p_ptr->tim_sh_shards - 1, TRUE);

    if (p_ptr->tim_sh_domination)
        set_tim_sh_domination(p_ptr->tim_sh_domination - 1, TRUE);

    if (p_ptr->tim_weaponmastery)
        set_tim_weaponmastery(p_ptr->tim_weaponmastery - 1, TRUE);

    /* Timed sh_holy */
    if (p_ptr->tim_sh_holy)
    {
        (void)set_tim_sh_holy(p_ptr->tim_sh_holy - 1, TRUE);
    }

    /* Timed eyeeye */
    if (p_ptr->tim_eyeeye)
    {
        (void)set_tim_eyeeye(p_ptr->tim_eyeeye - 1, TRUE);
    }

    /* Timed resist-magic */
    if (p_ptr->resist_magic)
    {
        (void)set_resist_magic(p_ptr->resist_magic - 1, TRUE);
    }

    /* Timed regeneration */
    if (p_ptr->tim_regen)
    {
        (void)set_tim_regen(p_ptr->tim_regen - 1, TRUE);
    }

    /* Timed resist nether */
    if (p_ptr->tim_res_nether)
    {
        (void)set_tim_res_nether(p_ptr->tim_res_nether - 1, TRUE);
    }

    /* Timed resist time */
    if (p_ptr->tim_res_time)
    {
        (void)set_tim_res_time(p_ptr->tim_res_time - 1, TRUE);
    }

    if (p_ptr->tim_res_disenchantment)
        (void)set_tim_res_disenchantment(p_ptr->tim_res_disenchantment - 1, TRUE);

    /* Timed reflect */
    if (p_ptr->tim_reflect)
    {
        (void)set_tim_reflect(p_ptr->tim_reflect - 1, TRUE);
    }

    /* Multi-shadow */
    if (p_ptr->multishadow)
    {
        (void)set_multishadow(p_ptr->multishadow - 1, TRUE);
    }

    /* Timed Robe of dust */
    if (p_ptr->dustrobe)
    {
        (void)set_dustrobe(p_ptr->dustrobe - 1, TRUE);
    }

    if (p_ptr->kabenuke)
    {
        (void)set_kabenuke(p_ptr->kabenuke - 1, TRUE);
    }

    /* Confusion */
    if (p_ptr->confused)
    {
        do { set_confused(p_ptr->confused - 1, TRUE); }
            while (p_ptr->confused && res_save_default(RES_CONF));
    }

    /* Fast */
    if (p_ptr->fast)
    {
        (void)set_fast(p_ptr->fast - 1, TRUE);
    }

    /* Slow: Note FA helps recovery, but this aid should be
     * much slower than recovery from paralysis (2L vs L/2) */
    if (p_ptr->slow)
    {
        do { set_slow(p_ptr->slow - 1, TRUE); }
            while (p_ptr->slow && free_act_save_p(dun_level*2));
    }

    /* Unwellness recovery */
    if (p_ptr->unwell)
    {
        (void)set_unwell(p_ptr->unwell - 1, TRUE);
    }

    /* Mini-slow recovery - regen helps */
    if (p_ptr->minislow)
    {
        int myregen = MAX(0, p_ptr->regen / 100);        
        if ((!myregen) && (one_in_(3))) myregen++;
        p_ptr->mini_energy += myregen * ((p_ptr->minislow * 2) + 2) / 3;
        if (p_ptr->mini_energy >= 100)
        {
            p_ptr->mini_energy -= 100;
            p_inc_minislow(-1);
        }
    }
    else p_ptr->mini_energy = 0;

    /* Protection from evil */
    if (p_ptr->protevil)
    {
        (void)set_protevil(p_ptr->protevil - 1, TRUE);
    }

    /* Invulnerability */
    if (p_ptr->invuln)
    {
        (void)set_invuln(p_ptr->invuln - 1, TRUE);
    }

    /* Wraith form */
    if (p_ptr->wraith_form)
    {
        (void)set_wraith_form(p_ptr->wraith_form - 1, TRUE);
    }

    /* Heroism */
    if (p_ptr->hero)
    {
        (void)set_hero(p_ptr->hero - 1, TRUE);
    }

    /* Super Heroism */
    if (p_ptr->shero)
    {
        (void)set_shero(p_ptr->shero - 1, TRUE);
    }

    /* Blessed */
    if (p_ptr->blessed)
    {
        (void)set_blessed(p_ptr->blessed - 1, TRUE);
    }

    /* Shield */
    if (p_ptr->shield)
    {
        (void)set_shield(p_ptr->shield - 1, TRUE);
    }

    /* Tsubureru */
    if (p_ptr->tsubureru)
    {
        (void)set_tsubureru(p_ptr->tsubureru - 1, TRUE);
    }

    /* Magicdef */
    if (p_ptr->magicdef)
    {
        (void)set_magicdef(p_ptr->magicdef - 1, TRUE);
    }

    /* Tsuyoshi */
    if (p_ptr->tsuyoshi)
    {
        (void)set_tsuyoshi(p_ptr->tsuyoshi - 1, TRUE);
    }

    /* Oppose Acid */
    if (p_ptr->oppose_acid)
    {
        (void)set_oppose_acid(p_ptr->oppose_acid - 1, TRUE);
    }

    /* Oppose Lightning */
    if (p_ptr->oppose_elec)
    {
        (void)set_oppose_elec(p_ptr->oppose_elec - 1, TRUE);
    }

    /* Oppose Fire */
    if (p_ptr->oppose_fire)
    {
        (void)set_oppose_fire(p_ptr->oppose_fire - 1, TRUE);
    }

    /* Oppose Cold */
    if (p_ptr->oppose_cold)
    {
        (void)set_oppose_cold(p_ptr->oppose_cold - 1, TRUE);
    }

    /* Oppose Poison */
    if (p_ptr->oppose_pois)
    {
        (void)set_oppose_pois(p_ptr->oppose_pois - 1, TRUE);
    }

    /* Spin */
    if (p_ptr->spin)
    {
        (void)set_spin(p_ptr->spin - 1, TRUE);
    }

    if (p_ptr->ult_res)
    {
        (void)set_ultimate_res(p_ptr->ult_res - 1, TRUE);
    }

    if (p_ptr->tim_spurt)
    {
        (void)set_tim_spurt(p_ptr->tim_spurt - 1, TRUE);
    }

    if (p_ptr->tim_blood_shield)
    {
        (void)set_tim_blood_shield(p_ptr->tim_blood_shield - 1, TRUE);
    }

    if (p_ptr->tim_blood_rite)
    {
        (void)set_tim_blood_rite(p_ptr->tim_blood_rite - 1, TRUE);
    }

    if (p_ptr->tim_blood_seek)
    {
        (void)set_tim_blood_seek(p_ptr->tim_blood_seek - 1, TRUE);
    }

    if (p_ptr->tim_blood_sight)
    {
        (void)set_tim_blood_sight(p_ptr->tim_blood_sight - 1, TRUE);
    }

    if (p_ptr->tim_blood_feast)
    {
        (void)set_tim_blood_feast(p_ptr->tim_blood_feast - 1, TRUE);
    }

    if (p_ptr->tim_blood_revenge)
    {
        (void)set_tim_blood_revenge(p_ptr->tim_blood_revenge - 1, TRUE);
    }

    if (p_ptr->tim_superstealth)
        set_tim_superstealth(p_ptr->tim_superstealth - 1, TRUE);

    if (p_ptr->tim_force)
    {
        set_tim_force(p_ptr->tim_force - 1, TRUE);
    }

    if (p_ptr->tim_building_up)
    {
        set_tim_building_up(p_ptr->tim_building_up - 1, TRUE);
    }

    if (p_ptr->tim_vicious_strike)
    {
        set_tim_vicious_strike(p_ptr->tim_vicious_strike - 1, TRUE);
    }

    if (p_ptr->tim_enlarge_weapon)
    {
        set_tim_enlarge_weapon(p_ptr->tim_enlarge_weapon - 1, TRUE);
    }

    if (p_ptr->tim_field)
    {
        set_tim_field(p_ptr->tim_field - 1, TRUE);
    }

    if (p_ptr->tim_spell_reaction)
        set_tim_spell_reaction(p_ptr->tim_spell_reaction - 1, TRUE);

    if (p_ptr->tim_resist_curses)
        set_tim_resist_curses(p_ptr->tim_resist_curses - 1, TRUE);

    if (p_ptr->tim_armor_of_fury)
        set_tim_armor_of_fury(p_ptr->tim_armor_of_fury - 1, TRUE);

    if (p_ptr->tim_spell_turning)
        set_tim_spell_turning(p_ptr->tim_spell_turning - 1, TRUE);

    if (p_ptr->tim_sustain_str)
        set_tim_sustain_str(p_ptr->tim_sustain_str - 1, TRUE);

    if (p_ptr->tim_sustain_int)
        set_tim_sustain_int(p_ptr->tim_sustain_int - 1, TRUE);

    if (p_ptr->tim_sustain_wis)
        set_tim_sustain_wis(p_ptr->tim_sustain_wis - 1, TRUE);

    if (p_ptr->tim_sustain_dex)
        set_tim_sustain_dex(p_ptr->tim_sustain_dex - 1, TRUE);

    if (p_ptr->tim_sustain_con)
        set_tim_sustain_con(p_ptr->tim_sustain_con - 1, TRUE);

    if (p_ptr->tim_sustain_chr)
        set_tim_sustain_chr(p_ptr->tim_sustain_chr - 1, TRUE);

    if (p_ptr->tim_hold_life)
        set_tim_hold_life(p_ptr->tim_hold_life - 1, TRUE);

    if (p_ptr->tim_transcendence)
        set_tim_transcendence(p_ptr->tim_transcendence - 1, TRUE);

    if (p_ptr->tim_quick_walk)
        set_tim_quick_walk(p_ptr->tim_quick_walk - 1, TRUE);

    if (p_ptr->tim_inven_prot)
        set_tim_inven_prot(p_ptr->tim_inven_prot - 1, TRUE);

    if (p_ptr->tim_inven_prot2)
        set_tim_inven_prot2(p_ptr->tim_inven_prot2 - 1, TRUE);

    if (p_ptr->tim_device_power)
        set_tim_device_power(p_ptr->tim_device_power - 1, TRUE);

    if (p_ptr->tim_sh_time)
        set_tim_sh_time(p_ptr->tim_sh_time - 1, TRUE);

    if (p_ptr->tim_foresight)
        set_tim_foresight(p_ptr->tim_foresight - 1, TRUE);

    if (p_ptr->tim_dark_stalker)
        set_tim_dark_stalker(p_ptr->tim_dark_stalker - 1, TRUE);

    if (p_ptr->tim_nimble_dodge)
        set_tim_nimble_dodge(p_ptr->tim_nimble_dodge - 1, TRUE);

    if (p_ptr->tim_stealthy_snipe)
        set_tim_stealthy_snipe(p_ptr->tim_stealthy_snipe - 1, TRUE);

    if (p_ptr->tim_killing_spree)
        set_tim_killing_spree(p_ptr->tim_killing_spree - 1, TRUE);

    if (p_ptr->tim_slay_sentient)
        set_tim_slay_sentient(p_ptr->tim_slay_sentient - 1, TRUE);

    wild_decrement_counters();

    if (p_ptr->pclass == CLASS_PSION)
        psion_decrement_counters();

    if (disciple_is_(DISCIPLE_TROIKA))
        troika_reduce_timeouts();

    if (p_ptr->fasting && one_in_(7))
    {
        if (randint1(PY_FOOD_ALERT) > p_ptr->food)
        {
            switch (randint1(8))
            {
            case 1: do_res_stat(A_STR); break;
            case 2: do_res_stat(A_INT); break;
            case 3: do_res_stat(A_WIS); break;
            case 4: do_res_stat(A_DEX); break;
            case 5: do_res_stat(A_CON); break;
            case 6: do_res_stat(A_CHR); break;
            case 7: restore_level(); break;
            case 8: lp_player(150); break;
            }
        }
    }

    /*** Poison and Stun and Cut ***/

    /* Stun */
    if (p_ptr->stun > STUN_NONE && p_ptr->stun < STUN_KNOCKED_OUT)
    {
        int adjust = adj_con_fix[p_ptr->stat_ind[A_CON]] + 1;

        /* Apply some healing */
        (void)set_stun(p_ptr->stun - adjust, TRUE);
    }

    /* Cut */
    if (p_ptr->cut)
    {
        int adjust = adj_con_fix[p_ptr->stat_ind[A_CON]] + 1;

        /* Hack -- Truly "mortal" wound */
        if (p_ptr->cut > 1000) adjust = 0;

        /* Blood Knights thrive on cuts, and even regen while cut.
           So, cuts heal much more slowly */
        if (p_ptr->pclass == CLASS_BLOOD_KNIGHT)
        {
            adjust /= 3;
            if (adjust < 1) adjust = 1;
        }

        /* Apply some healing */
        (void)set_cut(p_ptr->cut - adjust, TRUE);
    }
}


/*
 * Handle burning fuel every 10 game turns
 */
static void process_world_aux_light(void)
{
    int slot = equip_find_obj(TV_LITE, SV_ANY);
    if (slot)
    {
        object_type *lite = equip_obj(slot);
        if ( !(lite->name1 || lite->name3 || lite->art_name || lite->sval == SV_LITE_FEANOR)
          && lite->xtra4 > 0 )
        {
            if (lite->name2 == EGO_LITE_DURATION)
            {
                if (game_turn % (TURNS_PER_TICK*2)) lite->xtra4--;
            }
            else lite->xtra4--;
            notice_lite_change(lite);
        }
    }
}


/*
 * Handle curse effects once every 10 game turns
 */
static void process_world_aux_curse(void)
{
    if ((p_ptr->cursed & TRC_P_FLAG_MASK) && !p_ptr->inside_battle && !p_ptr->wild_mode)
    {
        /*
         * Hack: Uncursed teleporting items (e.g. Trump Weapons)
         * can actually be useful!
         */
        if ((p_ptr->cursed & OFC_TELEPORT_SELF) && one_in_(200))
        {
            char o_name[MAX_NLEN];
            object_type *o_ptr;
            int i, i_keep = 0, count = 0;

            /* Scan the equipment with random teleport ability */
            for (i = 1; i <= equip_max(); i++)
            {
                u32b flgs[OF_ARRAY_SIZE];
                o_ptr = equip_obj(i);

                if (!o_ptr) continue;
                if (o_ptr->marked & OM_SLIPPING) continue;
                obj_flags(o_ptr, flgs);
                if (have_flag(flgs, OF_TELEPORT))
                {
                    /* {.} will stop random teleportation. */
                    if (!o_ptr->inscription || !my_strchr(quark_str(o_ptr->inscription), '.'))
                    {
                        count++;
                        if (one_in_(count)) i_keep = i;
                    }
                }
            }

            if (i_keep)
            {
                o_ptr = equip_obj(i_keep);
                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
                msg_format("Your %s is activating teleportation.", o_name);
                if (get_check_strict("Teleport? ", CHECK_OKAY_CANCEL))
                {
                    disturb(0, 0);
                    teleport_player(50, 0L);
                }
                else
                {
                    msg_format("You can inscribe {.} on your %s to disable random teleportation. ", o_name);
                    disturb(1, 0);
                }
                obj_learn_flag(o_ptr, OF_TELEPORT);
            }
        }
        /* Make a chainsword noise */
        if ((p_ptr->cursed & OFC_CHAINSWORD) && one_in_(CHAINSWORD_NOISE))
        {
            char noise[1024];
            if (!get_rnd_line("chainswd.txt", 0, noise))
                msg_print(noise);
            disturb(FALSE, FALSE);
        }
        /* TY Curse */
        if ((p_ptr->cursed & OFC_TY_CURSE) && one_in_(TY_CURSE_CHANCE))
        {
            int count = 0;
            (void)activate_ty_curse(FALSE, &count);
            equip_learn_curse(OFC_TY_CURSE);
        }
        /* Handle experience draining */
        if (p_ptr->prace != RACE_ANDROID &&
            ((p_ptr->cursed & OFC_DRAIN_EXP) && one_in_(4)))
        {
            p_ptr->exp -= (p_ptr->lev+1)/2;
            if (p_ptr->exp < 0) p_ptr->exp = 0;
            p_ptr->max_exp -= (p_ptr->lev+1)/2;
            if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;
            check_experience();
            equip_learn_curse(OFC_DRAIN_EXP);
            equip_learn_flag(OF_DRAIN_EXP);
        }
        /* Add light curse (Later) */
        if ((p_ptr->cursed & OFC_ADD_L_CURSE) && one_in_(2000))
        {
            u32b new_curse;
            object_type *o_ptr;

            o_ptr = choose_cursed_obj_name(OFC_ADD_L_CURSE);

            new_curse = get_curse(0, o_ptr);
            if (!(o_ptr->curse_flags & new_curse))
            {
                char o_name[MAX_NLEN];

                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

                o_ptr->curse_flags |= new_curse;
                msg_format("There is a malignant black aura surrounding your %s...", o_name);

                o_ptr->feeling = FEEL_NONE;

                p_ptr->update |= (PU_BONUS);
                obj_learn_curse(o_ptr, OFC_ADD_L_CURSE);
            }
        }
        /* Add heavy curse (Later) */
        if ((p_ptr->cursed & OFC_ADD_H_CURSE) && one_in_(2000))
        {
            u32b new_curse;
            object_type *o_ptr;

            o_ptr = choose_cursed_obj_name(OFC_ADD_H_CURSE);

            new_curse = get_curse(1, o_ptr);
            if (!(o_ptr->curse_flags & new_curse))
            {
                char o_name[MAX_NLEN];

                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

                o_ptr->curse_flags |= new_curse;
                msg_format("There is a malignant black aura surrounding your %s...", o_name);

                o_ptr->feeling = FEEL_NONE;

                p_ptr->update |= (PU_BONUS);
                obj_learn_curse(o_ptr, OFC_ADD_H_CURSE);
            }
        }
        /* Call animal */
        if ((p_ptr->cursed & OFC_CALL_ANIMAL) && one_in_(2500))
        {
            if (summon_specific(0, py, px, dun_level, SUMMON_ANIMAL,
                (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
            {
                char o_name[MAX_NLEN];
                object_type *o_ptr = choose_cursed_obj_name(OFC_CALL_ANIMAL);

                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
                msg_format("Your %s %s attracted an animal!", o_name, object_plural(o_ptr) ? "have" : "has");

                disturb(0, 0);
                obj_learn_curse(o_ptr, OFC_CALL_ANIMAL);
            }
        }
        /* Call demon */
        if ((p_ptr->cursed & OFC_CALL_DEMON) && one_in_(1111))
        {
            if (summon_specific(0, py, px, dun_level, SUMMON_DEMON, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
            {
                char o_name[MAX_NLEN];
                object_type *o_ptr = choose_cursed_obj_name(OFC_CALL_DEMON);

                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
                msg_format("Your %s %s attracted a demon!", o_name, object_plural(o_ptr) ? "have" : "has");

                disturb(0, 0);
                obj_learn_curse(o_ptr, OFC_CALL_DEMON);
            }
        }
        /* Call dragon */
        if ((p_ptr->cursed & OFC_CALL_DRAGON) && one_in_(800))
        {
            if (summon_specific(0, py, px, dun_level, SUMMON_DRAGON,
                (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
            {
                char o_name[MAX_NLEN];
                object_type *o_ptr = choose_cursed_obj_name(OFC_CALL_DRAGON);

                object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
                msg_format("Your %s %s attracted a dragon!", o_name, object_plural(o_ptr) ? "have" : "has");

                disturb(0, 0);
                obj_learn_curse(o_ptr, OFC_CALL_DRAGON);
            }
        }
        if ((p_ptr->cursed & OFC_COWARDICE) && one_in_(1500))
        {
            if (!fear_save_p(fear_threat_level()))
            {
                disturb(0, 0);
                msg_print("It's so dark... so scary!");

                fear_add_p(FEAR_SCARED);
                equip_learn_curse(OFC_COWARDICE);
            }
        }
        /* Teleport player */
        if ((p_ptr->cursed & OFC_TELEPORT) && one_in_(200) && !p_ptr->anti_tele)
        {
            disturb(0, 0);

            /* Teleport player */
            teleport_player(40, TELEPORT_PASSIVE);
            equip_learn_curse(OFC_TELEPORT);
            equip_learn_flag(OF_TELEPORT);
        }
        /* Handle HP draining */
        if ((p_ptr->cursed & OFC_DRAIN_HP) && one_in_(666))
        {
            char o_name[MAX_NLEN];
            object_type *o_ptr = choose_cursed_obj_name(OFC_DRAIN_HP);

            object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
            msg_format("Your %s drains HP from you!", o_name);
            take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->lev*2, 100), o_name);
            obj_learn_curse(o_ptr, OFC_DRAIN_HP);
        }
        /* Handle mana draining */
        if ((p_ptr->cursed & OFC_DRAIN_MANA) && p_ptr->csp && one_in_(666))
        {
            char o_name[MAX_NLEN];
            object_type *o_ptr = choose_cursed_obj_name(OFC_DRAIN_MANA);

            object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
            msg_format("Your %s drains mana from you!", o_name);
            p_ptr->csp -= MIN(p_ptr->lev, 50);
            if (p_ptr->csp < 0)
            {
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }
            p_ptr->redraw |= PR_MANA;
            obj_learn_curse(o_ptr, OFC_DRAIN_MANA);
        }
    }

    /* Rarely, take damage from the Jewel of Judgement */
    if (one_in_(999) && !p_ptr->anti_magic)
    {
        int slot = equip_find_art(ART_JUDGE);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            if (object_is_known(o_ptr))
                msg_print("The Jewel of Judgement drains life from you!");
            else
                msg_print("Something drains life from you!");
            take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->lev, 50), "the Jewel of Judgement");
        }
    }

    if (r_info[MON_SAURON].max_num && one_in_(666))
    {
        int slot = equip_find_ego(EGO_RING_NAZGUL);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);

            o_ptr->curse_flags |= OFC_HEAVY_CURSE;
            o_ptr->curse_flags |= OFC_CURSED;
            o_ptr->curse_flags |= get_curse(2, o_ptr);
            p_ptr->update |= PU_BONUS;

            msg_boundary();
            cmsg_print(TERM_VIOLET, "You behold the Eye of Sauron!");
            if (one_in_(2))
            {
                msg_print("You feel your life draining away...");
                lose_exp(p_ptr->exp / 16);
            }
            while (one_in_(2))
            {
                do_dec_stat(randint0(6));
            }
            if (one_in_(2))
            {
                msg_print("You forget yourself in utter terror!");
                lose_all_info();
            }
            if (one_in_(2))
                set_stun(p_ptr->stun + randint1(40), FALSE);
            if (one_in_(2))
                set_confused(p_ptr->confused + randint1(5) + 5, FALSE);
        }
    }

    if (one_in_(666))
    {
        int slot = equip_find_art(ART_HAND_OF_VECNA);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            if (object_is_known(o_ptr))
                msg_print("The Hand of Vecna strangles you!");
            else
                msg_print("The Hand strangles you!");
            take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->lev, 50), "the Hand of Vecna");
        }
    }

    if (one_in_(666))
    {
        int slot = equip_find_art(ART_EYE_OF_VECNA);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            if (object_is_known(o_ptr))
                msg_print("The Eye of Vecna causes mental anquish!");
            else
                msg_print("The Eye causes mental anquish!");

            p_ptr->csp -= MIN(p_ptr->lev, 50);
            if (p_ptr->csp < 0)
            {
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }
            p_ptr->redraw |= PR_MANA;
        }
    }

    if (!p_ptr->wild_mode && one_in_(66))
    {
        int slot = equip_find_art(ART_BLOODRIP);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            if (set_cut(p_ptr->cut + 66, FALSE))
            {
                if (object_is_known(o_ptr))
                    msg_print("Bloodrip feeds on you!");
                else
                    msg_print("Something feeds on you!");
            }
        }
    }
}


/*
 * Handle recharging objects once every 10 game turns
 */
static bool _recharge_changed = FALSE;
static void _recharge_aux(object_type *o_ptr)
{
    if (o_ptr->timeout > 0)
    {
        o_ptr->timeout--;
        if (!o_ptr->timeout)
        {
            recharged_notice(o_ptr, '!');
            _recharge_changed = TRUE;
        }
    }
}
static void process_world_aux_recharge(void)
{
    int i;
    _recharge_changed = FALSE;
    equip_for_each(_recharge_aux);
    if (_recharge_changed)
    {
        p_ptr->window |= PW_EQUIP;
        wild_regen = 20;
    }

    /*
     * Recharge Devices
     */
    _recharge_changed = FALSE;
    for (i = 1; i <= pack_max(); i++)
    {
        object_type *o_ptr = pack_obj(i);

        if (!o_ptr) continue;

        switch (o_ptr->tval)
        {
        case TV_ROD:
            device_regen_sp(o_ptr, 10);
            break;
        case TV_WAND:
        case TV_STAFF:
            if ((game_turn % (TURNS_PER_TICK*10)) == 0)
                device_regen_sp(o_ptr, 10);
            break;
        }

        /* artifact mushrooms for the snotling ... they never stack */
        if (object_is_mushroom(o_ptr) && o_ptr->timeout)
        {
            o_ptr->timeout--;
            if (o_ptr->timeout < 0) o_ptr->timeout = 0;
            if (!o_ptr->timeout)
            {
                recharged_notice(o_ptr, '!');
                _recharge_changed = TRUE;
            }
        }
    }
    if (_recharge_changed)
    {
        p_ptr->window |= PW_INVEN;
        wild_regen = 20;
    }
}


/*
 * Handle involuntary movement once every 10 game turns
 */
void process_world_aux_movement(void)
{
    /* Delayed Word-of-Recall */
    if (p_ptr->word_recall)
    {
        /*
         * HACK: Autosave BEFORE resetting the recall counter (rr9)
         * The player is yanked up/down as soon as
         * he loads the autosaved game.
         */
        if (autosave_l && (p_ptr->word_recall == 1) && !p_ptr->inside_battle)
            do_cmd_save_game(TRUE);

        /* Count down towards recall */
        p_ptr->word_recall--;

        p_ptr->redraw |= (PR_STATUS);

        /* Activate the recall */
        if (!p_ptr->word_recall)
        {
            /* Disturbing! */
            disturb(0, 0);

            /* Determine the level */
            if (py_on_surface())
            {
                cmsg_print(TERM_YELLOW, "You feel yourself yanked downwards!");
                dungeon_type = p_ptr->recall_dungeon;
                dun_level = max_dlv[dungeon_type];
                if (dun_level < 1) dun_level = 1;

                if (coffee_break) dun_level = coffeebreak_recall_level(TRUE);

                /* Nightmare mode makes recall more dangerous */
                if (ironman_nightmare && !randint0(666) && (dungeon_type == DUNGEON_ANGBAND))
                {
                    if (dun_level < 50)
                    {
                        dun_level *= 2;
                    }
                    else if (dun_level < 99)
                    {
                        dun_level = (dun_level + 99) / 2;
                    }
                    else if (dun_level > 100)
                    {
                        dun_level = d_info[dungeon_type].maxdepth - 1;
                    }
                }

                if (p_ptr->wild_mode)
                {
                    if (py != p_ptr->wilderness_y || px != p_ptr->wilderness_x)
                    {
                        p_ptr->wilderness_y = py;
                        p_ptr->wilderness_x = px;
                        p_ptr->wilderness_dx = 0;
                        p_ptr->wilderness_dy = 0;
                    }
                }
                else
                {
                    /* Save player position */
                    p_ptr->oldpx = px;
                    p_ptr->oldpy = py;
                }
                p_ptr->wild_mode = FALSE;

                /*
                 * Clear all saved floors
                 * and create a first saved floor
                 */
                prepare_change_floor_mode(CFM_FIRST_FLOOR);

                /* Leaving */
                p_ptr->leaving = TRUE;
            }
            else
            {
                cmsg_print(TERM_YELLOW, "You feel yourself yanked upwards!");
                if (dungeon_type) p_ptr->recall_dungeon = dungeon_type;

                dun_level = 0;
                dungeon_type = 0;
                quests_on_leave();
                p_ptr->leaving = TRUE;

                /* Mega-hack - place player on stairs to R'lyeh */
                if ((thrall_mode) && (p_ptr->recall_dungeon == DUNGEON_CTH) &&
                    (p_ptr->wilderness_y == d_info[DUNGEON_CTH].dy) &&
                    (p_ptr->wilderness_x == d_info[DUNGEON_CTH].dx))
                {
                    p_ptr->leaving_dungeon = DUNGEON_CTH;
                }
            }

            /* Sound */
            sound(SOUND_TPLEVEL);
        }
    }


    /* Delayed Alter reality */
    if (p_ptr->alter_reality)
    {
        if (autosave_l && (p_ptr->alter_reality == 1) && !p_ptr->inside_battle)
            do_cmd_save_game(TRUE);

        /* Count down towards alter */
        p_ptr->alter_reality--;

        p_ptr->redraw |= (PR_STATUS);

        /* Activate the alter reality */
        if (!p_ptr->alter_reality)
        {
            do_alter_reality();
        }
    }
}


/*
 * Count number of adjacent monsters
 */
static int get_monster_crowd_number(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    int my = m_ptr->fy;
    int mx = m_ptr->fx;
    int i;
    int count = 0;

    for (i = 0; i < 7; i++)
    {
        int ay = my + ddy_ddd[i];
        int ax = mx + ddx_ddd[i];

        if (!in_bounds(ay, ax)) continue;

        /* Count number of monsters */
        if (cave[ay][ax].m_idx > 0) count++;
     }

    return count;
}



/*
 * Dungeon rating is no longer linear
 */
#define RATING_BOOST(delta) (delta * delta + 50 * delta)

/*
 * Examine all monsters and unidentified objects,
 * and get the feeling of current dungeon floor
 */
static byte get_dungeon_feeling(void)
{
    const int base = 10;
    int rating = 0;
    int i;
    const race_t *race_ptr = get_race();

    /* Hack -- no feeling in the town */
    if (!dun_level) return 0;

    /* Examine each monster */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;
        int delta = 0;

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Ignore pet */
        if (is_pet(m_ptr)) continue;

        /* Experimental Hack: Monster Boss is "Special" */
        if (race_ptr->boss_r_idx == m_ptr->r_idx)
            return 1;

        r_ptr = &r_info[m_ptr->r_idx];

        /* Unique monsters */
        if (r_ptr->flags1 & (RF1_UNIQUE))
        {
            /* Nearly out-of-depth unique monsters */
            if (r_ptr->level + 10 > dun_level)
            {
                /* Boost rating by twice delta-depth */
                delta += (r_ptr->level + 10 - dun_level) * 2 * base;
            }
        }
        else
        {
            /* Out-of-depth monsters */
            if (r_ptr->level > dun_level)
            {
                /* Boost rating by delta-depth */
                delta += (r_ptr->level - dun_level) * base;
            }
        }

        /* Unusually crowded monsters get a little bit of rating boost */
        if (r_ptr->flags1 & RF1_FRIENDS)
        {
            if (5 <= get_monster_crowd_number(i)) delta += 1;
        }
        else
        {
            if (2 <= get_monster_crowd_number(i)) delta += 1;
        }


        rating += RATING_BOOST(delta);
    }

    /* Examine each unidentified object */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];
        object_kind *k_ptr = &k_info[o_ptr->k_idx];
        int delta = 0;

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Skip known objects */
        if (object_is_known(o_ptr))
        {
            /* Touched? */
            if (o_ptr->marked & OM_TOUCHED) continue;
        }

        /* Skip pseudo-known objects */
        if (o_ptr->ident & IDENT_SENSE)
        {
            if ((!p_ptr->munchkin_pseudo_id) || (o_ptr->marked & OM_TOUCHED)) continue;
        }

        /* Experimental Hack: Force Special Feelings for artifacts no matter what. */
        if (object_is_artifact(o_ptr))
            return 1;

        if ( object_is_artifact(o_ptr)
          || object_is_ego(o_ptr)
          || o_ptr->tval == TV_DRAG_ARMOR
          || object_is_dragon_armor(o_ptr) )
        {
            s32b cost = obj_value_real(o_ptr);

            delta += 10 * base;
            if (cost > 10000L) delta += 10 * base;
            if (cost > 50000L) delta += 10 * base;
            if (cost > 100000L) delta += 10 * base;

            if (!preserve_mode && object_is_artifact(o_ptr))
                return 1;
        }

        /* Out-of-depth objects */
        if (k_ptr->level > dun_level)
        {
            delta += (k_ptr->level - dun_level) * base;
        }

        rating += RATING_BOOST(delta);
    }


    if (rating > RATING_BOOST(1000)) return 2;
    if (rating > RATING_BOOST(800)) return 3;
    if (rating > RATING_BOOST(600)) return 4;
    if (rating > RATING_BOOST(400)) return 5;
    if (rating > RATING_BOOST(300)) return 6;
    if (rating > RATING_BOOST(200)) return 7;
    if (rating > RATING_BOOST(100)) return 8;
    if (rating > RATING_BOOST(0)) return 9;

    return 10;
}


/*
 * Update dungeon feeling, and announce it if changed
 */
static void update_dungeon_feeling(void)
{
    bool feeling_was_special = (p_ptr->feeling == 1);
    byte new_feeling;
    int delay;

    /* No feeling on the surface */
    if (!dun_level) return;

    /* No feeling in the arena */
    if (p_ptr->inside_battle) return;

    /* Extract delay time */
    delay = MAX(10, 150 - p_ptr->skills.fos) * (150 - dun_level) * TURNS_PER_TICK / 100;

    delay = delay * adj_pseudo_id[p_ptr->stat_ind[A_WIS]] / 100;
    delay = delay * (625 - virtue_current(VIRTUE_ENLIGHTENMENT)) / 625;

     /* Not yet felt anything */
    if ((game_turn < p_ptr->feeling_turn + delay) && (!cheat_xtra)) return;

    if (!quests_allow_feeling()) return;

    /* Get new dungeon feeling */
    new_feeling = get_dungeon_feeling();

    /* Remember last time updated */
    p_ptr->feeling_turn = game_turn;

    /* No change */
    if (p_ptr->feeling == new_feeling) return;

    /* Dungeon feeling is changed */
    p_ptr->feeling = new_feeling;

    /* Announce feeling */
    do_cmd_feeling();

    /* Update the level indicator */
    p_ptr->redraw |= (PR_DEPTH);

    /* Disturb */
    if ((p_ptr->feeling == 1) || ((disturb_minor) && (feeling_was_special))) disturb(0, 0);
}


/*
 * Handle certain things once every 10 game turns
 */
static void process_world(void)
{
    int day, hour, min;

    const s32b A_DAY = TURNS_PER_TICK * TOWN_DAWN;
    s32b prev_turn_in_today = ((game_turn - TURNS_PER_TICK) % A_DAY + A_DAY / 4) % A_DAY;
    int prev_min = (1440 * prev_turn_in_today / A_DAY) % 60;

    extract_day_hour_min(&day, &hour, &min);

    /* Update dungeon feeling, and announce it if changed */
    update_dungeon_feeling();

    /*** Check monster arena ***/
    if (p_ptr->inside_battle && !p_ptr->leaving)
    {
        int i2, j2;
        int win_m_idx = 0;
        int number_mon = 0;

        /* Count all hostile monsters */
        for (i2 = 0; i2 < cur_wid; ++i2)
            for (j2 = 0; j2 < cur_hgt; j2++)
            {
                cave_type *c_ptr = &cave[j2][i2];

                if ((c_ptr->m_idx > 0) && (c_ptr->m_idx != p_ptr->riding))
                {
                    number_mon++;
                    win_m_idx = c_ptr->m_idx;
                }
            }

        if (number_mon == 0)
        {
            msg_print("They have killed each other at the same time.");
            msg_print(NULL);
            p_ptr->energy_need = 0;
            battle_monsters();
        }
        else if ((number_mon-1) == 0)
        {
            char m_name[80];
            monster_type *wm_ptr;

            wm_ptr = &m_list[win_m_idx];

            monster_desc(m_name, wm_ptr, 0);
            msg_format("%^s is the winner!", m_name);
            /* Hack: Make sure the player sees this one! */
            auto_more_state = AUTO_MORE_PROMPT;
            msg_print(NULL);

            if (win_m_idx == (sel_monster+1))
            {
                msg_print("Congratulations.");
                msg_format("You received %d gold.", battle_odds);
                p_ptr->au += battle_odds;
                stats_on_gold_winnings(battle_odds);
            }
            else
            {
                msg_print("You lost gold.");
            }
            msg_print(NULL);
            p_ptr->energy_need = 0;
            battle_monsters();
        }
        else if (game_turn - old_turn == 150*TURNS_PER_TICK)
        {
            msg_format("This battle has ended in a draw.");
            p_ptr->au += kakekin;
            stats_on_gold_winnings(kakekin);
            msg_print(NULL);
            p_ptr->energy_need = 0;
            battle_monsters();
        }
    }

    /* Every 10 game turns */
    if (game_turn % TURNS_PER_TICK) return;

    /*** Check the Time and Load ***/

    if (!(game_turn % (50*TURNS_PER_TICK)))
    {
        /* Check time and load */
        if ((0 != check_time()) || (0 != check_load()))
        {
            /* Warning */
            if (closing_flag <= 2)
            {
                /* Disturb */
                disturb(0, 0);

                /* Count warnings */
                closing_flag++;

                /* Message */
                msg_print("The gates to ANGBAND are closing...");
                msg_print("Please finish up and/or save your game.");

            }

            /* Slam the gate */
            else
            {
                /* Message */
                msg_print("The gates to ANGBAND are now closed.");


                /* Stop playing */
                p_ptr->playing = FALSE;

                /* Leaving */
                p_ptr->leaving = TRUE;
            }
        }
    }

    /*** Attempt timed autosave ***/
    if (autosave_t && autosave_freq && !p_ptr->inside_battle)
    {
        if (!(game_turn % ((s32b)autosave_freq * TURNS_PER_TICK)))
            do_cmd_save_game(TRUE);
    }

    if (mon_fight && !ignore_unview)
    {
        msg_print("You hear noise.");
    }

    /*** Handle the wilderness/town (sunshine) ***/

    /* While in town/wilderness */
    if (py_on_surface())
    {
        /* Hack -- Daybreak/Nighfall in town */
        if (!(game_turn % ((TURNS_PER_TICK * TOWN_DAWN) / 2)))
        {
            bool dawn;

            /* Check for dawn */
            dawn = (!(game_turn % (TURNS_PER_TICK * TOWN_DAWN)));

            /* Day breaks */
            if (dawn)
            {
                int y, x;

                /* Message */
                msg_print("The sun has risen.");

                if (!p_ptr->wild_mode)
                {
                    /* Hack -- Scan the town */
                    for (y = 0; y < cur_hgt; y++)
                    {
                        for (x = 0; x < cur_wid; x++)
                        {
                            /* Get the cave grid */
                            cave_type *c_ptr = &cave[y][x];

                            /* Assume lit */
                            c_ptr->info |= (CAVE_GLOW | CAVE_AWARE);

                            /* Hack -- Memorize lit grids if allowed */
                            if (view_perma_grids) c_ptr->info |= (CAVE_MARK);

                            /* Hack -- Notice spot */
                            note_spot(y, x);
                        }
                    }
                }
            }

            /* Night falls */
            else
            {
                int y, x;

                /* Message */
                msg_print("The sun has fallen.");

                if (!p_ptr->wild_mode)
                {
                    /* Hack -- Scan the town */
                    for (y = 0; y < cur_hgt; y++)
                    {
                        for (x = 0; x < cur_wid; x++)
                        {
                            /* Get the cave grid */
                            cave_type *c_ptr = &cave[y][x];

                            /* Feature code (applying "mimic" field) */
                            feature_type *f_ptr = &f_info[get_feat_mimic(c_ptr)];

                            if (!is_mirror_grid(c_ptr) && !have_flag(f_ptr->flags, FF_QUEST_ENTER) &&
                                !have_flag(f_ptr->flags, FF_ENTRANCE))
                            {
                                /* Assume dark */
                                c_ptr->info &= ~(CAVE_GLOW);

                                if (!have_flag(f_ptr->flags, FF_REMEMBER))
                                {
                                    /* Forget the normal floor grid */
                                    c_ptr->info &= ~(CAVE_MARK);

                                    /* Hack -- Notice spot */
                                    note_spot(y, x);
                                }
                            }
                        }

                        /* Glow deep lava and building entrances */
                        glow_deep_lava_and_bldg();
                    }
                }
            }

            /* Update the monsters */
            p_ptr->update |= (PU_MONSTERS | PU_MON_LITE);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Window stuff */
            p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

            if (p_ptr->special_defense & NINJA_S_STEALTH)
            {
                if (cave[py][px].info & CAVE_GLOW) set_superstealth(FALSE);
            }
        }
    }

    /*** Process the monsters ***/

    /* Check for creature generation. */
    if (!p_ptr->inside_arena && !quests_get_current() && !p_ptr->inside_battle)
    {
        int  chance = d_info[dungeon_type].max_m_alloc_chance;

        chance = chance * (100 + dun_level) / 100;
        chance = chance * (375 - virtue_current(VIRTUE_PATIENCE)) / 375;

        if (one_in_(chance))
        {
            spawn_hack = TRUE;
            if (p_ptr->action == ACTION_GLITTER && one_in_(3))
                ring_summon_ring_bearer();
            else
                alloc_monster(MAX_SIGHT + 5, 0);
            spawn_hack = FALSE;
        }
    }
    /* It's too easy to get stuck playing a race that can't move! Sigh ... */
    else if ( quests_get_current()
           && p_ptr->action == ACTION_GLITTER
           && one_in_(50) )
    {
        ring_summon_ring_bearer();
    }

    /* Hack -- Check for creature regeneration */
    if (!(game_turn % (TURNS_PER_TICK*10)) && !p_ptr->inside_battle) regen_monsters();
    if (!(game_turn % (TURNS_PER_TICK*3))) regen_captured_monsters();

    /* Date changes */
    if (!hour && !min)
    {
        if (min != prev_min)
        {
            determine_today_mon(FALSE);
            if (p_ptr->prace == RACE_WEREWOLF) werewolf_check_midnight();
        }
    }

    /*
     * Nightmare mode activates the TY_CURSE at midnight
     *
     * Require exact minute -- Don't activate multiple times in a minute
     */
    if (ironman_nightmare && (min != prev_min))
    {
        /* Every 15 minutes after 11:00 pm */
        if ((hour == 23) && !(min % 15))
        {
            /* Disturbing */
            disturb(0, 0);

            switch (min / 15)
            {
            case 0:
                msg_print("You hear a distant bell toll ominously.");
                break;

            case 1:
                msg_print("A distant bell sounds twice.");
                break;

            case 2:
                msg_print("A distant bell sounds three times.");
                break;

            case 3:
                msg_print("A distant bell tolls four times.");
                break;
            }
        }

        /* TY_CURSE activates at midnight! */
        if (!hour && !min)
        {
            int count = 0;

            disturb(1, 0);
            msg_print("A distant bell tolls many times, fading into a deathly silence.");

            activate_ty_curse(FALSE, &count);
        }
    }


    /*** Check the Food, and Regenerate ***/

    if (!p_ptr->inside_battle)
    {
        /* Digest quickly when gorged */
        if (p_ptr->food >= PY_FOOD_MAX)
        {
            /* Digest a lot of food */
            (void)set_food(p_ptr->food - 100);
        }

        /* Digest normally -- Every 50 game turns */
        else if (!(game_turn % (TURNS_PER_TICK*5)))
        {
            /* Basic digestion rate based on speed */
            int digestion = SPEED_TO_ENERGY(p_ptr->pspeed);

            /* Regeneration takes more food */
            if (p_ptr->regen > 100)
                digestion += 10*(p_ptr->regen-100)/100;
            if (p_ptr->special_defense & (KAMAE_MASK | KATA_MASK))
                digestion += 20;
            if (p_ptr->cursed & OFC_FAST_DIGEST)
                digestion += 30;

            /* Slow digestion takes less food */
            if (p_ptr->slow_digest)
                digestion /= 2;

            /* Temperance slows digestion */
            digestion = digestion * (375 - virtue_current(VIRTUE_TEMPERANCE)) / 375;

            /* Minimal digestion */
            if (digestion < 1) digestion = 1;
            /* Maximal digestion */
            if (digestion > 100) digestion = 100;

            /* Digest some food */
            (void)set_food(p_ptr->food - digestion);
        }


        /* Getting Faint */
        if ((p_ptr->food < PY_FOOD_FAINT))
        {
            /* Faint occasionally */
            if (!p_ptr->paralyzed && (randint0(100) < 10))
            {
                /* Message */
                msg_print("You faint from the lack of food.");

                disturb(1, 0);

                /* Hack -- faint (bypass free action) */
                (void)set_paralyzed(randint1(4), FALSE);
            }

            /* Starve to death (slowly) */
            if (p_ptr->food < PY_FOOD_STARVE)
            {
                /* Calculate damage */
                int dam = (PY_FOOD_STARVE - p_ptr->food) / 10;

                /* Take damage */
                if (!IS_INVULN()) take_hit(DAMAGE_LOSELIFE, dam, "starvation");
            }
        }
    }



    /* Process timed damage and regeneration */
    process_world_aux_hp_and_sp();

    /* Process timeout */
    process_world_aux_timeout();

    /* Process light */
    process_world_aux_light();

    /* Process mutation effects */
    mut_process();

    /* Process curse effects */
    process_world_aux_curse();

    /* Process recharging */
    process_world_aux_recharge();

    /* Feel the inventory */
    sense_inventory1();
    sense_inventory2();

    /* Involuntary Movement */
    process_world_aux_movement();

    {
        race_t *race_ptr = get_race();
        if (race_ptr->process_world)
            race_ptr->process_world();
    }
}



/*
 * Verify use of "wizard" mode
 */
static bool enter_wizard_mode(void)
{
    /* Ask first time */
    if (!p_ptr->noscore)
    {
        /* Wizard mode is not permitted */
        if (!allow_debug_opts || arg_wizard)
        {
            msg_print("Wizard mode is not permitted.");
            return FALSE;
        }
        else
        {
#ifndef ALLOW_WIZARD
            msg_print("Wizard mode is only permitted in special builds (#define ALLOW_WIZARD in z-config.h).");
            return FALSE;
#endif
        }

        /* Mention effects */
        msg_print("Wizard mode is for debugging and experimenting.");
        msg_print("The game will not be scored if you enter wizard mode.");
        if (!get_check("Are you sure you want to enter wizard mode? "))
        {
            return (FALSE);
        }

        /* Mark savefile */
        p_ptr->noscore |= 0x0002;
    }

    /* Success */
    return (TRUE);
}


#ifdef ALLOW_WIZARD

/*
 * Verify use of "debug" commands
 */
static bool enter_debug_mode(void)
{
    /* Ask first time */
    if (!p_ptr->noscore)
    {
        /* Debug mode is not permitted */
        if (!allow_debug_opts)
        {
            msg_print("Use of debug command is not permitted.");
            return FALSE;
        }

        /* Mention effects */
        msg_print("The debug commands are for debugging and experimenting.");
        msg_print("The game will not be scored if you use debug commands.");
        if (!get_check("Are you sure you want to use debug commands? "))
        {
            return (FALSE);
        }

        /* Mark savefile */
        p_ptr->noscore |= 0x0008;
    }

    /* Success */
    return (TRUE);
}

/*
 * Hack -- Declare the Debug Routines
 */
extern void do_cmd_debug(void);

#endif /* ALLOW_WIZARD */



/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 */
static void _dispatch_command(int old_now_turn)
{
    switch (command_cmd)
    {
        /* Ignore */
        case ' ':
        case '\r':
        case '\n':
        case ESCAPE:
            break;

        /*** Wizard Commands ***/

        /* Toggle Wizard Mode */
        case KTRL('Y'):
        case KTRL('W'):
        {
            if (p_ptr->wizard)
            {
                p_ptr->wizard = FALSE;
                msg_print("Wizard mode off.");

            }
            else if (enter_wizard_mode())
            {
                p_ptr->wizard = TRUE;
                msg_print("Wizard mode on.");

            }

            /* Update monsters */
            p_ptr->update |= PU_MONSTERS;
            p_ptr->redraw |= PR_EFFECTS;
            break;
        }


#ifdef ALLOW_WIZARD

        /* Special "debug" commands */
        case KTRL('A'):
        {
            /* Enter debug mode */
            if (enter_debug_mode())
            {
                do_cmd_debug();
            }
            break;
        }

#endif /* ALLOW_WIZARD */


#ifdef ALLOW_SPOILERS
		case KTRL('Z'):
			/*  v~~~ ^Z(d|D) is useful info for game design ... */
			if (0 || allow_spoilers)
				do_cmd_spoilers();
			break;
#endif /* ALLOW_SPOILERS */
		
        /*** Inventory Commands ***/

        /* Wear/wield equipment */
        case 'w':
        {
            if (!p_ptr->wild_mode) equip_wield_ui();
            break;
        }        

        /* Take off equipment */
        case 't':
        {
            if (!p_ptr->wild_mode) equip_takeoff_ui();
            break;
        }

        /* Drop an item */
        case 'd':
        {
            if (!p_ptr->wild_mode) do_cmd_drop();
            break;
        }

        /* Destroy an item */
        case 'k':
        {
            obj_destroy_ui();
            break;
        }

        /* Equipment list */
        case 'e':
        {
            equip_ui();
            break;
        }

        /* Inventory list */
        case 'i':
        {
            pack_ui();
            break;
        }

        case 'W':
        {
            ring_finger_swap_ui(0, 0);
            break;
        }


        /*** Various commands ***/

        /* Identify an object */
        case 'I':
        {
            obj_inspect_ui();
            break;
        }

        /* Hack -- toggle windows */
        case KTRL('I'):
        {
            toggle_inven_equip();
            toggle_mon_obj_lists();
            break;
        }


        /*** Standard "Movement" Commands ***/

        /* Alter a grid */
        case '+':
        {
            if (!p_ptr->wild_mode) do_cmd_alter();
            break;
        }

        /* Dig a tunnel */
        case 'T':
        {
            if (!p_ptr->wild_mode) do_cmd_tunnel();
            break;
        }

        /* Move (usually pick up things) */
        case ';':
        {
#ifdef ALLOW_EASY_DISARM /* TNB */

            do_cmd_walk(FALSE);

#else /* ALLOW_EASY_DISARM -- TNB */

            do_cmd_walk(always_pickup);

#endif /* ALLOW_EASY_DISARM -- TNB */
            break;
        }

        /* Move (usually do not pick up) */
        case '-':
        {
#ifdef ALLOW_EASY_DISARM /* TNB */

            do_cmd_walk(TRUE);

#else /* ALLOW_EASY_DISARM -- TNB */

            do_cmd_walk(!always_pickup);

#endif /* ALLOW_EASY_DISARM -- TNB */

            break;
        }


        /*** Running, Resting, Searching, Staying */

        /* Begin Running -- Arg is Max Distance */
        case '.':
        {
            if (!p_ptr->wild_mode)
                do_cmd_run();
            break;
        }

        /* Stay still (usually pick things up) */
        case ',':
        {
            delay_autopick_hack = 1;
            do_cmd_stay(always_pickup);
            delay_autopick_hack = 0;
            break;
        }

        case 'g':
        {
            delay_autopick_hack = 2;
            do_cmd_get();
            delay_autopick_hack = 0;
            break;
        }

        case KTRL('G'):
        {
            do_cmd_autoget();
            break;
        }
        case 'H':
        {
            if (!p_ptr->wild_mode) do_cmd_get_nearest();
            break;
        }
        /* Rest -- Arg is time */
        case 'R':
        {
            do_cmd_rest();
            break;
        }

        /* Search for traps/doors */
        case 's':
        {
            do_cmd_search();
            break;
        }

        /* Toggle search mode */
        case 'S':
        {
            if (p_ptr->action == ACTION_SEARCH) set_action(ACTION_NONE);
            else set_action(ACTION_SEARCH);
            break;
        }


        /*** Stairs and Doors and Chests and Traps ***/


        /* Enter quest level -KMW- */
        case SPECIAL_KEY_QUEST:
        {
            if (!p_ptr->wild_mode) do_cmd_quest();
            break;
        }

        /* Go up staircase */
        case '<':
        {
            if (py_on_surface())
            {
                if (no_wilderness) break;

                if (p_ptr->food < PY_FOOD_WEAK)
                {
                    msg_print("You must eat something here.");
                    break;
                }

                change_wild_mode();
            }
            else
                do_cmd_go_up();
            break;
        }

        /* Go down staircase */
        case '>':
        {
            if (p_ptr->wild_mode)
                change_wild_mode();
            else
                do_cmd_go_down();

            break;
        }

        /* Open a door or chest */
        case 'o':
        {
            if (!p_ptr->wild_mode) do_cmd_open();
            break;
        }

        /* Close a door */
        case 'c':
        {
            if (!p_ptr->wild_mode) do_cmd_close();
            break;
        }

        /* Jam a door with spikes */
        case 'j':
        {
            if (!p_ptr->wild_mode) do_cmd_spike();
            break;
        }

        /* Bash a door */
        case 'B':
        {
            if (!p_ptr->wild_mode) do_cmd_bash();
            break;
        }

        /* Disarm a trap or chest */
        case 'D':
        {
            if (!p_ptr->wild_mode) do_cmd_disarm();
            break;
        }


        /*** Magic and Prayers ***/

        /* Gain new spells/prayers */
        case 'G':
        {
            if (p_ptr->pclass == CLASS_SORCERER || p_ptr->pclass == CLASS_RED_MAGE)
                msg_print("You don't have to learn spells!");
            else if (p_ptr->pclass == CLASS_SKILLMASTER)
                skillmaster_gain_skill();
            else if (p_ptr->pclass == CLASS_SAMURAI)
                do_cmd_gain_hissatsu();
            else if (p_ptr->pclass == CLASS_RAGE_MAGE)
                rage_mage_gain_spell();
            else if (p_ptr->pclass == CLASS_MAGIC_EATER)
                magic_eater_gain();
            else if (p_ptr->pclass == CLASS_GRAY_MAGE)
                gray_mage_gain_spell();
            else if (p_ptr->pclass == CLASS_PSION)
            {
                msg_print("You can only gain spells at certain levels.");
            }
            else
                do_cmd_study();
            break;
        }

        /* Browse a book */
        case 'b':
        {
            if (p_ptr->prace == RACE_MON_RING)
                ring_browse();
            else if (p_ptr->pclass == CLASS_MAGIC_EATER)
                magic_eater_browse();
            else if (p_ptr->pclass == CLASS_RAGE_MAGE)
                rage_mage_browse_spell();
            else if (p_ptr->pclass == CLASS_SKILLMASTER)
                skillmaster_browse();
            else if (p_ptr->pclass == CLASS_ALCHEMIST)
                alchemist_browse();
            else if (p_ptr->pclass == CLASS_GRAY_MAGE)
                gray_mage_browse_spell();
            else if (p_ptr->pclass == CLASS_ARCHAEOLOGIST ||
                     p_ptr->pclass == CLASS_BERSERKER ||
                     p_ptr->pclass == CLASS_DUELIST ||
                     p_ptr->pclass == CLASS_WARLOCK ||
                     p_ptr->pclass == CLASS_PSION ||
                     p_ptr->pclass == CLASS_BLOOD_KNIGHT ||
                     p_ptr->pclass == CLASS_MINDCRAFTER ||
                     p_ptr->pclass == CLASS_MIRROR_MASTER ||
                     p_ptr->pclass == CLASS_MONSTER ||
                     p_ptr->pclass == CLASS_NINJA ||
                     p_ptr->pclass == CLASS_RUNE_KNIGHT ||
                     p_ptr->pclass == CLASS_WILD_TALENT ||
                     p_ptr->pclass == CLASS_WEAPONMASTER ||
                     p_ptr->pclass == CLASS_DEVICEMASTER ||
                     p_ptr->pclass == CLASS_SCOUT ||
                     p_ptr->pclass == CLASS_MAULER ||
                     p_ptr->pclass == CLASS_MYSTIC ||
                     p_ptr->pclass == CLASS_SNIPER ||
                     p_ptr->pclass == CLASS_TIME_LORD )
            {
                /* This is the preferred entry point ... I'm still working on
                   coverting everything else */
                do_cmd_spell_browse();
            }
            else do_cmd_browse();
            break;
        }

        /* Cast a spell */
        case 'm':
            /* -KMW- */
            if (p_ptr->wild_mode) break;
            if (p_ptr->pclass == CLASS_WARRIOR || p_ptr->pclass == CLASS_ARCHER || p_ptr->pclass == CLASS_CAVALRY)
            {
                msg_print("You cannot cast spells!");
            }
            else if (p_ptr->tim_no_spells)
            {
                msg_print("Your spells are blocked!");
                flush();
                /*energy_use = 100;*/
            }
            else if ((beorning_is_(BEORNING_FORM_BEAR)) && (p_ptr->pclass != CLASS_DUELIST))
            {
                msg_print("You cannot use magic in bear shape!");
                flush();
            }
            else if ( dun_level && (d_info[dungeon_type].flags1 & DF1_NO_MAGIC)
                   && p_ptr->pclass != CLASS_BERSERKER
                   && p_ptr->pclass != CLASS_BLOOD_KNIGHT
                   && p_ptr->pclass != CLASS_WEAPONMASTER
                   && p_ptr->pclass != CLASS_MAULER
                   && p_ptr->pclass != CLASS_ALCHEMIST
                   && p_ptr->pclass != CLASS_RAGE_MAGE
                   && p_ptr->prace  != RACE_MON_POSSESSOR
                   && p_ptr->prace  != RACE_MON_MIMIC)
            {
                if (flush_failure) flush();
                msg_print("The dungeon absorbs all attempted magic!");
                msg_print(NULL);
            }
            else if ( p_ptr->anti_magic
                   && p_ptr->pclass != CLASS_BERSERKER
                   && p_ptr->pclass != CLASS_BLOOD_KNIGHT
                   && p_ptr->pclass != CLASS_WEAPONMASTER
                   && p_ptr->pclass != CLASS_MAULER
                   && p_ptr->pclass != CLASS_ALCHEMIST
                   && p_ptr->pclass != CLASS_RAGE_MAGE
                   && p_ptr->prace  != RACE_MON_POSSESSOR
                   && p_ptr->prace  != RACE_MON_MIMIC)
            {
                cptr which_power = "magic";
                if (p_ptr->pclass == CLASS_MINDCRAFTER || p_ptr->pclass == CLASS_PSION)
                    which_power = "psionic powers";
                else if (p_ptr->pclass == CLASS_SAMURAI)
                    which_power = "hissatsu";
                else if (p_ptr->pclass == CLASS_LAWYER || p_ptr->pclass == CLASS_NINJA_LAWYER)
                    which_power = "legal trickery";
                else if (p_ptr->pclass == CLASS_MIRROR_MASTER)
                    which_power = "mirror magic";
                else if (p_ptr->pclass == CLASS_NINJA)
                    which_power = "ninjutsu";
                else if (mp_ptr->spell_book == TV_LIFE_BOOK)
                    which_power = "prayer";

                if (flush_failure) flush();
                msg_format("An anti-magic shell disrupts your %s!", which_power);
                equip_learn_flag(OF_NO_MAGIC);
                energy_use = 0;
            }
            else if (IS_SHERO() && p_ptr->pclass != CLASS_BERSERKER && p_ptr->pclass != CLASS_BLOOD_KNIGHT && p_ptr->pclass != CLASS_RAGE_MAGE
             && p_ptr->pclass != CLASS_ALCHEMIST && ((!beorning_is_(BEORNING_FORM_BEAR) || (p_ptr->pclass != CLASS_DUELIST))))
            {
                if (flush_failure) flush();
                msg_format("You cannot think clearly!");
                energy_use = 0;
            }
            else
            {
                spell_problem = 0;
                if (p_ptr->prace == RACE_MON_RING)
                    ring_cast();
                else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC || p_ptr->pclass == CLASS_BLUE_MAGE)
                    possessor_cast();
                else if (p_ptr->pclass == CLASS_MAGIC_EATER)
                    magic_eater_cast(0);
                else if (p_ptr->pclass == CLASS_SKILLMASTER)
                    skillmaster_cast();
                else if (p_ptr->pclass == CLASS_SAMURAI)
                    do_cmd_hissatsu();
                else if (p_ptr->pclass == CLASS_GRAY_MAGE)
                    gray_mage_cast_spell();
                else if (p_ptr->pclass == CLASS_ALCHEMIST)
                    alchemist_cast(0);
                else if (p_ptr->pclass == CLASS_ARCHAEOLOGIST ||
                            p_ptr->pclass == CLASS_BERSERKER ||
                            p_ptr->pclass == CLASS_DUELIST ||
                            p_ptr->pclass == CLASS_WARLOCK ||
                            p_ptr->pclass == CLASS_BLOOD_KNIGHT ||
                            p_ptr->pclass == CLASS_MINDCRAFTER ||
                            p_ptr->pclass == CLASS_MIRROR_MASTER ||
                            p_ptr->pclass == CLASS_MONSTER ||
                            p_ptr->pclass == CLASS_NINJA ||
                            p_ptr->pclass == CLASS_PSION ||
                            p_ptr->pclass == CLASS_RUNE_KNIGHT ||
                            p_ptr->pclass == CLASS_WILD_TALENT ||
                            p_ptr->pclass == CLASS_WEAPONMASTER ||
                            p_ptr->pclass == CLASS_DEVICEMASTER ||
                            p_ptr->pclass == CLASS_RAGE_MAGE ||
                            p_ptr->pclass == CLASS_SCOUT ||
                            p_ptr->pclass == CLASS_MAULER ||
                            p_ptr->pclass == CLASS_MYSTIC ||
                            p_ptr->pclass == CLASS_PSION ||
                            p_ptr->pclass == CLASS_SNIPER ||
                            p_ptr->pclass == CLASS_DISCIPLE ||
                            p_ptr->pclass == CLASS_TIME_LORD )
                {
                    /* This is the preferred entrypoint for spells ...
                        I'm still working on coverting everything else */
                    do_cmd_spell();
                }
                else
                {
                    do_cmd_cast();
                }
                if (spell_problem & PWR_AFRAID)
                {
                    msg_print("You tremble in fear!");
                    if (energy_use < 100) energy_use = 100;
                    if (p_ptr->pclass == CLASS_ALCHEMIST) energy_use = alchemist_infusion_energy_use();
                }
                spell_problem = 0;
            }
            break;

        /* Issue a pet command */
        case 'p':
            if (!p_ptr->wild_mode) do_cmd_pet();
            break;

        /*** Use various objects ***/

        /* Inscribe an object */
        case 'Z':
        case '{':
        {
            obj_inscribe_ui();
            break;
        }

        /* Uninscribe an object */
        case '}':
        {
            obj_uninscribe_ui();
            break;
        }

        /* Activate an artifact */
        case 'A':
        {
            if (!p_ptr->wild_mode)
            {
                if (!p_ptr->inside_arena)
                    do_cmd_activate();
                else
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
            }
            break;
        }

        /* Eat some food */
        case 'E':
        {
            do_cmd_eat_food();
            break;
        }

        /* Fuel your lantern/torch */
        case 'F':
        {
            do_cmd_refill();
            break;
        }

        /* Fire an item */
        case 'f':
        {
            if (!p_ptr->wild_mode) do_cmd_fire();
            break;
        }

        /* Throw an item */
        case 'v':
        {
            if (!p_ptr->wild_mode)
            {
                py_throw_t context = {0};
                py_throw(&context);
            }
            break;
        }

        /* Aim a wand */
        case 'a':
        {
            if (!p_ptr->wild_mode)
            {
                if (p_ptr->inside_arena && !devicemaster_is_(DEVICEMASTER_WANDS))
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
                else
                {
                    do_cmd_aim_wand();
                }
            }
            break;
        }

        /* Zap a rod */
        case 'z':
        {
            if (!p_ptr->wild_mode)
            {
                if (p_ptr->inside_arena && !devicemaster_is_(DEVICEMASTER_RODS))
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
                else
                {
                    do_cmd_zap_rod();
                }
            }
            break;
        }

        /* Quaff a potion */
        case 'q':
        {
            if (!p_ptr->wild_mode)
            {
                if (p_ptr->inside_arena && !devicemaster_is_(DEVICEMASTER_POTIONS) && p_ptr->pclass != CLASS_ALCHEMIST)
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
                else
                {
                    do_cmd_quaff_potion();
                }
            }
            break;
        }

        /* Read a scroll */
        case 'r':
        {
            if (!p_ptr->wild_mode)
            {
                if (p_ptr->inside_arena && !devicemaster_is_(DEVICEMASTER_SCROLLS))
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
                else
                {
                    do_cmd_read_scroll();
                }
            }
            break;
        }

        /* Use a staff */
        case 'u':
        {
            if (!p_ptr->wild_mode)
            {
                if (p_ptr->inside_arena && !devicemaster_is_(DEVICEMASTER_STAVES))
                {
                    msg_print("The arena absorbs all attempted magic!");
                    msg_print(NULL);
                }
                else
                    do_cmd_use_staff();
            }
            break;
        }

        /* Use racial power */
        case 'U':
        {
            if (!p_ptr->wild_mode)
            {
                if ((do_cmd_power() & PWR_AFRAID) && (energy_use < 100))
                {
                    msg_print("You tremble in fear!");
                    energy_use = 100;
                }
            }
            break;
        }


        /*** Looking at Things (nearby or on map) ***/

        /* Full dungeon map */
        case 'M':
        {
            do_cmd_view_map();
            break;
        }

        case KTRL('V'):
        {
            viewport_verify_aux(VIEWPORT_FORCE_CENTER);
            break;
        }

        /* Locate player on map */
        case 'L':
        {
            do_cmd_locate();
            break;
        }

        /* Look around */
        case 'l':
        {
            do_cmd_look();
            break;
        }

        case 'Y':
        case '[':
            if (!p_ptr->image)
                do_cmd_list_monsters(MON_LIST_NORMAL);
            break;

        case KTRL('O'):
        case 'O':
        case ']':
            if (!p_ptr->image)
                do_cmd_list_objects();
            break;

        /* Target monster or location */
        case '*':
        {
            if (!p_ptr->wild_mode) do_cmd_target();
            else do_cmd_look();
            break;
        }



        /*** Help and Such ***/

        /* Help */
        case '?':
        {
            do_cmd_help();
            break;
        }

        /* Identify symbol */
        case '/':
        {
            do_cmd_query_symbol();
            break;
        }

        /* Character description */
        case 'C':
        {
            py_display();
            /*do_cmd_change_name();*/
            break;
        }


        /*** System Commands ***/

        /* Single line from a pref file */
        case '!':
        {
            do_cmd_pref();
            break;
        }

        case '$':
        {
            do_cmd_reload_autopick();
            break;
        }

        case '_':
        {
            do_cmd_edit_autopick();
            break;
        }

        /* Interact with macros */
        case KTRL('E'):
        case '@':
        {
            do_cmd_macros();
            break;
        }

        /* Interact with visuals */
        case '%':
        {
            do_cmd_visuals();
            do_cmd_redraw();
            break;
        }

        /* Interact with colors */
        case '&':
        {
            do_cmd_colors();
            do_cmd_redraw();
            break;
        }

        /* Interact with options */
        case '=':
        {
            do_cmd_options();
            do_cmd_redraw();
            break;
        }

        /*** Misc Commands ***/

        /* Take notes */
        case ':':
        {
            do_cmd_note();
            break;
        }

        /* Version info */
        case 'V':
        {
            do_cmd_version();
            break;
        }

        /* Repeat level feeling */
        case KTRL('F'):
        {
            if (!p_ptr->wild_mode)
            {
                do_cmd_feeling();
                if (p_ptr->pclass == CLASS_DISCIPLE) disciple_feeling();
            }
            break;
        }

        /* Show previous messages */
        case KTRL('P'):
        {
            do_cmd_messages(old_now_turn);
            break;
        }

        /* Show quest status -KMW- */
        case KTRL('Q'):
        {
            quests_display();
            break;
        }

        /* Redraw the screen */
        case KTRL('R'):
        {
            now_turn = old_now_turn;
            do_cmd_redraw();
            break;
        }

#ifndef VERIFY_SAVEFILE

        /* Hack -- Save and don't quit */
        case KTRL('S'):
        {
            do_cmd_save_game(FALSE);
            break;
        }

#endif /* VERIFY_SAVEFILE */

        case KTRL('T'):
        {
            do_cmd_time();
            break;
        }

        /* Save and quit */
        case KTRL('X'):
        case SPECIAL_KEY_QUIT:
        {
            do_cmd_save_and_exit();
            break;
        }

        /* Quit (commit suicide) */
        case 'Q':
        {
            do_cmd_suicide();
            break;
        }

        /* Check artifacts, uniques, objects */
        case '~':
        {
            do_cmd_knowledge();
            break;
        }

        /* Save "screen dump" */
        case ')':
        {
            do_cmd_save_screen();
            break;
        }

        case '`':
        {
            if (!p_ptr->wild_mode) do_cmd_travel();
            break;
        }

        case 'J':
        {
            if ((!p_ptr->wild_mode) && (travel.x) && (travel.y) && ((px != travel.x) || (py != travel.y)) && (in_bounds(travel.y, travel.x)) && (get_check("Resume travelling? ")))
            travel_begin(TRAVEL_MODE_NORMAL, travel.x, travel.y);
            break;
        }

        /* Hack -- Unknown command */
        default:
        {
            if (flush_failure) flush();
            if (one_in_(2))
            {
                char error_m[1024];
                sound(SOUND_ILLEGAL);
                if (get_rnd_line("error.txt", 0, error_m) == ERROR_SUCCESS)
                    msg_print(error_m);
                else
                    msg_print("Unknown command. Type <color:y>?</color> for help.");
            }
            else
                msg_print("Unknown command. Type <color:y>?</color> for help.");

            break;
        }
    }
}

static void process_command(void)
{
    int old_now_turn = now_turn;

#ifdef ALLOW_REPEAT /* TNB */

    /* Handle repeating the last command */
    repeat_check(FALSE);

#endif /* ALLOW_REPEAT -- TNB */

    now_turn = game_turn;
    msg_boundary();

    if (p_ptr->pclass == CLASS_SNIPER && p_ptr->concent)
        reset_concent = TRUE;

    switch (command_cmd)
    {
    case SPECIAL_KEY_STORE:
        if (!p_ptr->wild_mode)
        {
            cave_type *c_ptr = &cave[py][px];

            if (cave_have_flag_grid(c_ptr, FF_STORE))
            {
                int which = f_info[c_ptr->feat].subtype;

                if (which == SHOP_HOME) home_ui();
                else if (which == SHOP_MUSEUM) museum_ui();
                else
                {
                    town_ptr town = towns_current_town();
                    shop_ptr shop = town_get_shop(town, which);

                    shop_ui(shop);
                }
            }
        }
        break;
    case SPECIAL_KEY_BUILDING:
        if (!p_ptr->wild_mode) do_cmd_bldg();
        break;
    default:
        pack_lock();
        _dispatch_command(old_now_turn);
        pack_unlock();
    }

    if (!energy_use)
        now_turn = old_now_turn;
}




static bool monster_tsuri(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if ((r_ptr->flags7 & RF7_AQUATIC) && !(r_ptr->flags1 & RF1_UNIQUE) && my_strchr("Jjlw", r_ptr->d_char))
        return TRUE;
    else
        return FALSE;
}



/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 */
static void process_player(void)
{
    int i;

    /*** Apply energy ***/

    if (p_ptr->inside_battle)
    {
        for(i = 1; i < m_max; i++)
        {
            monster_type *m_ptr = &m_list[i];

            if (!m_ptr->r_idx) continue;

            /* Hack -- Detect monster */
            m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

            /* Update the monster */
            update_mon(i, FALSE);
        }
        prt_time();
    }

    /* Give the player some energy */
    else if (!(load && p_ptr->energy_need <= 0))
    {
        p_ptr->energy_need -= SPEED_TO_ENERGY(p_ptr->pspeed);
    }

    /* No turn yet */
    if (p_ptr->energy_need > 0) return;
    if (!command_rep) prt_time();

    /*** Check for interupts ***/

    /* Complete resting */
    if (resting < 0)
    {
        /* Basic resting */
        if (resting == -1)
        {
            /* Stop resting */
            if ( (p_ptr->chp == p_ptr->mhp || mimic_no_regen())
              && ( p_ptr->csp >= p_ptr->msp
                || p_ptr->pclass == CLASS_RUNE_KNIGHT
                || p_ptr->pclass == CLASS_RAGE_MAGE
                || elemental_is_(ELEMENTAL_WATER)
                || mimic_no_regen() )
              && !magic_eater_can_regen() 
			  && !samurai_can_concentrate())
            {
                set_action(ACTION_NONE);
            }
        }

        /* Complete resting */
        else if (resting == -2)
        {
            /* Stop resting */
            if ( (p_ptr->chp == p_ptr->mhp || mimic_no_regen())
              && ( p_ptr->csp >= p_ptr->msp
                || p_ptr->pclass == CLASS_RUNE_KNIGHT
                || p_ptr->pclass == CLASS_RAGE_MAGE
                || elemental_is_(ELEMENTAL_WATER)
                || mimic_no_regen() )
              && !magic_eater_can_regen()
              && !samurai_can_concentrate()
              && !p_ptr->blind
              && !p_ptr->confused
              && !p_ptr->poisoned
              && !p_ptr->afraid
              && !p_ptr->stun
              && !p_ptr->cut
              && !player_slow()
              && !p_ptr->paralyzed
              && !p_ptr->image
              && !p_ptr->word_recall
              && !p_ptr->alter_reality )
            {
                set_action(ACTION_NONE);
            }
        }
    }

    if (p_ptr->action == ACTION_FISH)
    {
        /* Delay */
        Term_xtra(TERM_XTRA_DELAY, 10);
        if (one_in_(1000))
        {
            int r_idx;
            bool success = FALSE;
            get_mon_num_prep(monster_tsuri,NULL);
            r_idx = get_mon_num(dun_level ? dun_level : wilderness[p_ptr->wilderness_y][p_ptr->wilderness_x].level);
            msg_print(NULL);
            if (r_idx && one_in_(2))
            {
                int y, x;
                y = py+ddy[tsuri_dir];
                x = px+ddx[tsuri_dir];
                if (place_monster_aux(0, y, x, r_idx, PM_NO_KAGE))
                {
                    char m_name[80];
                    monster_desc(m_name, &m_list[cave[y][x].m_idx], 0);
                    msg_format("You have a good catch!", m_name);
                    success = TRUE;
                }
            }
            if (!success)
            {
                msg_print("Damn!  The fish stole your bait!");
            }
            disturb(0, 0);
        }
    }

    /* Handle "abort" */
    if (check_abort)
    {
        /* Check for "player abort" (semi-efficiently for resting) */
        if ( running
          || travel.run
          || command_rep
          || p_ptr->action == ACTION_REST
          || p_ptr->action == ACTION_GLITTER
          || p_ptr->action == ACTION_FISH )
        {
            /* Do not wait */
            inkey_scan = TRUE;

            /* Check for a key */
            if (inkey())
            {
                /* Flush input */
                flush();

                /* Disturb */
                disturb(0, 0);

                /* Hack -- Show a Message */
                msg_print("Canceled.");

            }
        }
    }

    if (p_ptr->riding && !p_ptr->confused && !p_ptr->blind)
    {
        monster_type *m_ptr = &m_list[p_ptr->riding];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        if (MON_CSLEEP(m_ptr))
        {
            char m_name[80];

            /* Recover fully */
            (void)set_monster_csleep(p_ptr->riding, 0);

            /* Acquire the monster name */
            monster_desc(m_name, m_ptr, 0);
            msg_format("You have waked %s up.", m_name);
        }

        if (MON_STUNNED(m_ptr))
        {
            /* Hack -- Recover from stun */
            if (set_monster_stunned(p_ptr->riding,
                (randint0(r_ptr->level) < skills_riding_current()) ? 0 : (MON_STUNNED(m_ptr) - 1)))
            {
                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message */
                msg_format("%^s is no longer stunned.", m_name);
            }
        }

        if (MON_CONFUSED(m_ptr))
        {
            /* Hack -- Recover from confusion */
            if (set_monster_confused(p_ptr->riding,
                (randint0(r_ptr->level) < skills_riding_current()) ? 0 : (MON_CONFUSED(m_ptr) - 1)))
            {
                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message */
                msg_format("%^s is no longer confused.", m_name);
            }
        }

        if (MON_MONFEAR(m_ptr))
        {
            /* Hack -- Recover from fear */
            if (set_monster_monfear(p_ptr->riding,
                (randint0(r_ptr->level) < skills_riding_current()) ? 0 : (MON_MONFEAR(m_ptr) - 1)))
            {
                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message */
                msg_format("%^s is no longer afraid.", m_name);
            }
        }

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();
    }

    /* Handle the player song/hex spells */
    if ((!load) && (!p_ptr->nice))
    {
        bard_check_music();
        check_hex();
        revenge_spell();
    }

    if ((!load) && (!p_ptr->nice))
    {
    class_t *class_ptr = get_class();

        if (class_ptr != NULL && class_ptr->process_player != NULL)
            class_ptr->process_player();
    }

    if (load)
        equip_on_load();


    /* XXX While many timed effects are processed every 10 game turns, some
     * game mechanics work better if they are indexed to player actions.
     * cf process_world_aux_hp_and_sp. */

    if ((!load) && (!p_ptr->nice))
    {
        if (p_ptr->lightspeed)
        {
            (void)set_lightspeed(p_ptr->lightspeed - 1, TRUE);
        }
        if (p_ptr->tim_no_spells)
        {
            (void)set_tim_no_spells(p_ptr->tim_no_spells - 1, TRUE);
        }
        if (p_ptr->tim_no_device)
        {
            (void)set_tim_no_device(p_ptr->tim_no_device - 1, TRUE);
        }
        if ((p_ptr->pclass == CLASS_FORCETRAINER) && (p_ptr->magic_num1[0]))
        {
            if (p_ptr->magic_num1[0] < 40)
            {
                p_ptr->magic_num1[0] = 0;
            }
            else p_ptr->magic_num1[0] -= 40;
            p_ptr->update |= (PU_BONUS);
        }
        if (p_ptr->action == ACTION_LEARN)
        {
            s32b cost = 0L;
            u32b cost_frac = (p_ptr->msp + 30L) * 256L;

            /* Convert the unit (1/2^16) to (1/2^32) */
            s64b_LSHIFT(cost, cost_frac, 16);

            if (s64b_cmp(p_ptr->csp, p_ptr->csp_frac, cost, cost_frac) < 0)
            {
                /* Mana run out */
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
                set_action(ACTION_NONE);
            }
            else
            {
                /* Reduce mana */
                s64b_sub(&(p_ptr->csp), &(p_ptr->csp_frac), cost, cost_frac);
            }
            p_ptr->redraw |= PR_MANA;
        }

        if (p_ptr->special_defense & KATA_MASK)
        {
            if (p_ptr->special_defense & KATA_MUSOU)
            {
                if (p_ptr->csp < 3)
                {
                    set_action(ACTION_NONE);
                }
                else
                {
                    p_ptr->csp -= 2;
                    p_ptr->redraw |= (PR_MANA);
                }
            }
        }

        fear_recover_p();
        if ((elemental_is_(ELEMENTAL_WATER)) && (p_ptr->csp))
        {
            int tiputus = MAX(5, p_ptr->csp / 20);
            if (cave_have_flag_bold(py, px, FF_WATER)) tiputus -= (tiputus / 2);
            p_ptr->csp -= tiputus;
            p_ptr->csp = MAX(0, p_ptr->csp);
            p_ptr->update |= (PU_BONUS);
            p_ptr->redraw |= (PR_MANA | PR_STATS);
        }
    }

    if (load) /* Mega-hack */
    {
        race_t *race_ptr = ((p_ptr->prace == RACE_DOPPELGANGER) ? get_race() : get_true_race());
        if ((race_ptr != NULL) && (race_ptr->flags & RACE_DEMI_TALENT) && (race_ptr->gain_level != NULL))
        {
            race_ptr->gain_level(p_ptr->lev);
        }
    }

    load = FALSE;

    /*** Handle actual user input ***/

    /* Repeat until out of energy */
    while (p_ptr->energy_need <= 0)
    {
        p_ptr->sutemi = FALSE;
        p_ptr->counter = FALSE;
        monsters_damaged_hack = FALSE;
        p_ptr->nice = FALSE;
        shuffling_hack_hp = p_ptr->chp;

        player_turn++;

        /* Handle "p_ptr->notice" */
        notice_stuff();

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();

        /* Place the cursor on the player */
        move_cursor_relative(py, px);

        /* Refresh (optional) */
        if (fresh_before) Term_fresh();

        /* Hack -- Pack Overflow */
        pack_overflow();

        /* Hack -- cancel "lurking browse mode" */
        if (!command_new) command_see = FALSE;

        /* Check if pets may turn hostile this turn */
        p_ptr->upset_okay = p_ptr->upkeep_warning;

        /* Assume free turn */
        energy_use = 0;

        if (p_ptr->inside_battle)
        {
            /* Place the cursor on the player */
            move_cursor_relative(py, px);

            command_cmd = SPECIAL_KEY_BUILDING;

            /* Process the command */
            process_command();
        }
        /* Paralyzed */
        else if (p_ptr->paralyzed)
        {
            energy_use = 100;
            do { set_paralyzed(p_ptr->paralyzed - 1, TRUE); }
            while (p_ptr->paralyzed && free_act_save_p(dun_level/2));
        }
        /* Knocked Out */
        else if (p_ptr->stun >= STUN_KNOCKED_OUT)
        {
            energy_use = 100;
            set_stun(p_ptr->stun - 25, TRUE);
        }

        /* Resting */
        else if (p_ptr->action == ACTION_REST)
        {
			caster_info *caster_ptr = get_caster_info();
			/* Timed rest */
            if (resting > 0)
            {
                /* Reduce rest count */
                resting--;

                if (!resting) set_action(ACTION_NONE);

                /* Redraw the state */
                p_ptr->redraw |= (PR_STATE);
            }

            /* Take a turn */
            energy_use = 100;

            if (caster_ptr && (caster_ptr->options & CASTER_SUPERCHARGE_MANA))
            {
                msg_boundary();
                cast_concentration();
            }
            else if (p_ptr->clear_mind)
            {
                msg_boundary();
                cast_clear_mind();
            }
        }

        else if (p_ptr->action == ACTION_FISH)
        {
            energy_use = 100;
        }
        else if (p_ptr->action == ACTION_GLITTER)
        {
            energy_use = 100;
        }

        /* Running */
        else if (running)
        {
            /* Take a step */
            run_step(0);
        }

        /* Traveling */
        else if (travel.run)
        {
            /* Take a step */
            travel_step();
        }

        /* Repeated command */
        else if (command_rep)
        {
            /* Count this execution */
            command_rep--;

            /* Redraw the state */
            p_ptr->redraw |= (PR_STATE);

            /* Redraw stuff */
            redraw_stuff();

            /* Hack -- Assume messages were seen 
            msg_line_clear(); */

            /* Process the command */
            process_command();
        }

        /* Normal command */
        else
        {
            /* Place the cursor on the player */
            move_cursor_relative(py, px);

            /* This is a very ugly place for this alert, but it's the only
             * way to make sure the alert serves its intended purpose and
             * won't be completely broken by any minor tweaks in the future */
            if ((alert_poison) && (p_ptr->poisoned > pienempi(p_ptr->mhp * 4 / 5, MIN(499, p_ptr->chp))))
            {
                if ((!poison_warning_hack) || ((int)poison_warning_hack < (p_ptr->poisoned + 9) / 10) || (p_ptr->poisoned / 4 > p_ptr->chp))
                {
                    msg_boundary();
                    msg_format("<color:G>*** POISON WARNING! ***</color>");
                    if ((!poison_warning_hack) || (p_ptr->poisoned / 4 > p_ptr->chp)) msg_print(NULL);
                }
                poison_warning_hack = MIN(255, (p_ptr->poisoned + 9) / 10);
            }
            else poison_warning_hack = 0;

            can_save = TRUE;
            /* Get a command (normal) */
            request_command(FALSE);
            can_save = FALSE;

            /* Process the command */
            process_command();
        }

        /* Hack -- Pack Overflow */
        pack_overflow();


        /*** Clean up ***/

        /* Significant */
        if (energy_use)
        {
            class_t *class_ptr = get_class();
            race_t  *race_ptr = get_race();

            if (class_ptr->player_action)
                class_ptr->player_action(energy_use);

            if (race_ptr->player_action)
                race_ptr->player_action(energy_use);

            /* Take damage from poison.
             * Note: Poison is now a delayed damage pool. No longer is there
             * any immediate damage. It's also much harder to 'cure'. This mechanic
             * works better if the player is hurt on every move they make. */
            if (p_ptr->poisoned)
            {
                int amt = MAX(MAX(1, p_ptr->mhp/60), p_ptr->poisoned/4);

                /* quickwalking ninjas should not be overly poisoned! */
                amt = amt * energy_use / 100;

                if ((amt < 1) && (randint0(100) < energy_use)) amt = 1;

                if (amt > p_ptr->poisoned)
                    amt = p_ptr->poisoned;
                if (0 || p_ptr->wizard)
                    msg_format("<color:G> %d Poison Damage</color>", amt);
                if (!IS_INVULN())
                    take_hit(DAMAGE_NOESCAPE, amt, "poison");
                set_poisoned(p_ptr->poisoned - amt, TRUE);
            }

            /* Cuts now work the same way poison does
             * (except that cut healing is handled separately) */
            if ((p_ptr->cut) && (!p_ptr->wild_mode))
            {
                /* Take damage from cuts */
                if (!IS_INVULN())
                {
                    cut_info_t cut = cut_info(p_ptr->cut);
                    if (cut.dam)
                    {
                        /*msg_format("<color:r> %d Cut Damage</color>", cut.dam);*/
                        take_hit(DAMAGE_NOESCAPE, cut.dam, "a fatal wound");
                    }
                }
            }

            if (p_ptr->free_turns)
            {
                p_ptr->redraw |= PR_STATUS;
                p_ptr->free_turns--;
                if (p_ptr->free_turns)
                    energy_use = 0;
            }

            if (world_player || energy_use > 400)
            {
                /* The Randomness is irrelevant */
                p_ptr->energy_need += energy_use * TURNS_PER_TICK / 10;
            }
            else
            {
                int amt = (s16b)((s32b)energy_use * ENERGY_NEED() / 100L);
                if (p_ptr->wizard)
                {
                    rect_t r = ui_char_info_rect();
                    c_put_str(TERM_WHITE, format("E:%3d/%3d", amt, energy_use), r.y + r.cy - 2, r.x);
                }
                p_ptr->energy_need += amt;
            }

            /* Hack -- constant hallucination */
            if (p_ptr->image) p_ptr->redraw |= (PR_MAP);

            /* Shimmer monsters if needed */
            if (shimmer_monsters)
            {
                /* Clear the flag */
                shimmer_monsters = FALSE;

                /* Shimmer multi-hued monsters */
                for (i = 1; i < m_max; i++)
                {
                    monster_type *m_ptr;
                    monster_race *r_ptr;

                    m_ptr = &m_list[i];
                    if (!m_ptr->r_idx) continue;
                    if (!m_ptr->ml) continue;
                    r_ptr = &r_info[m_ptr->ap_r_idx];
                    if (!(r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_SHAPECHANGER)))
                        continue;

                    shimmer_monsters = TRUE;
                    lite_spot(m_ptr->fy, m_ptr->fx);
                }
            }

            if (randint1(200) < energy_use)
                fear_process_p();

            if (repair_monsters)
            {
                repair_monsters = FALSE;

                for (i = 1; i < m_max; i++)
                {
                    monster_type *m_ptr;

                    m_ptr = &m_list[i];
                    if (!m_ptr->r_idx) continue;
                    if (m_ptr->mflag & MFLAG_NICE)
                    {
                        m_ptr->mflag &= ~(MFLAG_NICE);
                    }

                    if (m_ptr->mflag2 & MFLAG2_MARK)
                    {
                        if (m_ptr->mflag2 & MFLAG2_SHOW)
                        {
                            m_ptr->mflag2 &= ~(MFLAG2_SHOW);
                            repair_monsters = TRUE;
                        }
                        else
                        {
                            m_ptr->mflag2 &= ~(MFLAG2_MARK);
                            m_ptr->ml = FALSE;
                            update_mon(i, FALSE);
                            check_mon_health_redraw(i);
                            lite_spot(m_ptr->fy, m_ptr->fx);
                        }
                    }
                }
            }
            if (p_ptr->action == ACTION_LEARN)
            {
                new_mane = FALSE;
                p_ptr->redraw |= (PR_STATE);
            }

            if (world_player && (p_ptr->energy_need > - 1000))
            {
                p_ptr->redraw |= (PR_MAP | PR_STATUS);
                p_ptr->update |= (PU_MONSTERS);
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
                msg_print("You feel time flowing around you once more.");
                msg_print(NULL);
                world_player = FALSE;
                p_ptr->energy_need = ENERGY_NEED();
                handle_stuff();
            }
            predictable_energy_hack = FALSE;
        }
        else
            player_turn--;

        if (!p_ptr->playing || p_ptr->is_dead)
        {
            world_player = FALSE;
            break;
        }

        /* Sniper */
        if (energy_use && reset_concent) reset_concentration(TRUE);

        if (p_ptr->leaving) break;
    }

    /* Update scent trail */
    update_smell();
}


/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
static void dungeon(bool load_game)
{
    /* Set the base level */
    if (dun_level)
        base_level = dun_level;

    /* Reset various flags */
    hack_mind = FALSE;

    /* Not leaving */
    p_ptr->leaving = FALSE;

    /* Reset the "command" vars */
    command_cmd = 0;

#if 0 /* Don't reset here --- It's used for Arena */
    command_new = 0;
#endif

    command_rep = 0;
    command_arg = 0;
    command_dir = 0;


    /* Cancel the target */
    target_who = 0;
    pet_t_m_idx = 0;
    riding_t_m_idx = 0;

    /* Cancel the health bar */
    health_track(0);

    /* Check visual effects */
    shimmer_monsters = TRUE;
    shimmer_objects = TRUE;
    repair_monsters = TRUE;
    repair_objects = TRUE;


    /* Disturb */
    disturb(1, 0);

    /* Track maximum player level */
    if (p_ptr->max_plv < p_ptr->lev)
    {
        p_ptr->max_plv = p_ptr->lev;
    }


    /* Track maximum dungeon level (if not in quest -KMW-)
     * XXX Why is this here? Why not in generate()?
     * XXX Removed quest check since it gave recall weirdness for random quests. */
    if ( dungeon_type
      && max_dlv[dungeon_type] < dun_level
      && !(d_info[dungeon_type].flags1 & DF1_RANDOM) )
    {
        max_dlv[dungeon_type] = dun_level;
    }

    (void)calculate_upkeep();

    /* Verify the panel */
    viewport_verify();

    /* Flush messages
    msg_print(NULL);*/


    /* Enter "xtra" mode */
    character_xtra = TRUE;

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_MONSTER_LIST | PW_OBJECT_LIST | PW_MONSTER | PW_OVERHEAD | PW_DUNGEON);

    /* Redraw dungeon */
    p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_EQUIPPY | PR_MSG_LINE);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Update lite/view */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE | PU_TORCH);

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS | PU_DISTANCE | PU_FLOW);

    /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
    handle_stuff();

    /* Leave "xtra" mode */
    character_xtra = FALSE;

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Combine / Reorder the pack */
    p_ptr->notice |= (PN_OPTIMIZE_PACK | PN_OPTIMIZE_QUIVER);

    /* Handle "p_ptr->notice" */
    notice_stuff();

    /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
    handle_stuff();

    /* Refresh */
    Term_fresh();

    if (p_ptr->inside_battle)
    {
        if (load_game)
        {
            p_ptr->energy_need = 0;
            battle_monsters();
        }
        else
        {
            msg_format("Ready..Fight!");
            msg_print(NULL);
        }
    }

    if ((p_ptr->pclass == CLASS_BARD) && (p_ptr->magic_num1[0] > MUSIC_DETECT))
        p_ptr->magic_num1[0] = MUSIC_DETECT;

    /* Hack -- notice death or departure */
    if (!p_ptr->playing || p_ptr->is_dead) return;

    /* Print quest message if appropriate */
    if ((dun_level == d_info[dungeon_type].maxdepth) && d_info[dungeon_type].final_guardian)
    {
        if (mon_available_num(&r_info[d_info[dungeon_type].final_guardian]))
        {
            cmsg_format(
                TERM_YELLOW, "%^s lives on this level as the keeper of %s.",
                r_name + r_info[d_info[dungeon_type].final_guardian].name,
                d_name + d_info[dungeon_type].name);
        }
    }

    if (!load_game && (p_ptr->special_defense & NINJA_S_STEALTH)) set_superstealth(FALSE);

    /*** Process this dungeon level ***/

    /* Reset the monster generation level */
    monster_level = base_level;

    /* Reset the object generation level */
    object_level = base_level;

    hack_mind = TRUE;

    if (p_ptr->energy_need > 0 && !p_ptr->inside_battle &&
        (dun_level || p_ptr->leaving_dungeon || p_ptr->inside_arena))
        p_ptr->energy_need = 0;

    /* Not leaving dungeon */
    p_ptr->leaving_dungeon = 0;

    /* Run Exo's patch */
    if (autosave_l) updatecharinfoS();

    /* Main loop */
    while (TRUE)
    {
        /* Hack -- Compact the monster list occasionally */
        if ((m_cnt + 32 > max_m_idx) && !p_ptr->inside_battle) compact_monsters(64);

        /* Hack -- Compress the monster list occasionally */
        if ((m_cnt + 32 < m_max) && !p_ptr->inside_battle) compact_monsters(0);


        /* Hack -- Compact the object list occasionally */
        if (o_cnt + 32 > max_o_idx) compact_objects(64);

        /* Hack -- Compress the object list occasionally */
        if (o_cnt + 32 < o_max) compact_objects(0);


        /* Process the player */
        process_player();

        /* Handle "p_ptr->notice" */
        notice_stuff();

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();

        /* Hack -- Hilite the player */
        move_cursor_relative(py, px);

        /* Optional fresh */
        if (fresh_after) Term_fresh();

        /* Hack -- Notice death or departure */
        if (!p_ptr->playing || p_ptr->is_dead) break;

        /* Process all of the monsters */
        process_monsters();

#ifdef _DEBUG
        if (p_ptr->action == ACTION_GLITTER)
        {
            int msec = delay_time();
            Term_xtra(TERM_XTRA_DELAY, msec);
            Term_fresh();
        }
#endif

        /* Handle "p_ptr->notice" */
        notice_stuff();

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();

        /* Hack -- Hilite the player */
        move_cursor_relative(py, px);

        /* Optional fresh */
        if (fresh_after) Term_fresh();

        /* Hack -- Notice death or departure */
        if (!p_ptr->playing || p_ptr->is_dead) break;


        /* Process the world */
        process_world();

        /* Handle "p_ptr->notice" */
        notice_stuff();

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();

        /* Hack -- Hilite the player */
        move_cursor_relative(py, px);

        /* Optional fresh */
        if (fresh_after) Term_fresh();

        /* Hack -- Notice death or departure */
        if (!p_ptr->playing || p_ptr->is_dead) break;

        /* Handle "leaving" */
        if (p_ptr->leaving) break;

        /* Count game turns */
        game_turn++;

        if (dungeon_turn < dungeon_turn_limit)
        {
            if (!p_ptr->wild_mode || wild_regen) dungeon_turn++;
            else if (p_ptr->wild_mode && !(game_turn % ((MAX_HGT + MAX_WID) / 2))) dungeon_turn++;
        }

        prevent_turn_overflow();

        if (wild_regen) wild_regen--;
    }

    /* Not save-and-quit and not dead? */
    if (p_ptr->playing && !p_ptr->is_dead)
    {
        /*
         * Maintain Unique monsters and artifact, save current
         * floor, then prepare next floor
         */
        leave_floor();

        /* Forget the flag */
        reinit_wilderness = FALSE;
    }

    /* Write about current level on the play record once per level */
    write_level = TRUE;
}


/*
 * Load some "user pref files"
 *
 * Modified by Arcum Dagsson to support
 * separate macro files for different realms.
 */
static void load_all_pref_files(bool new_game)
{
    char buf[1024];
    int alp_mode = ALP_CHECK_NUMERALS;

    /* Access the "user" pref file */
    sprintf(buf, "user.prf");

    /* Process that file */
    process_pref_file(buf);

    /* Access the "user" system pref file */
    sprintf(buf, "user-%s.prf", ANGBAND_SYS);

    /* Process that file */
    process_pref_file(buf);

    /* Access the "race" pref file */
    sprintf(buf, "%s.prf", get_true_race()->name);

    /* Process that file */
    process_pref_file(buf);

    /* Access the "class" pref file */
    sprintf(buf, "%s.prf", get_class()->name);

    /* Process that file */
    process_pref_file(buf);

    /* Access the "character" pref file */
    sprintf(buf, "%s.prf", player_base);

    strcpy(pref_save_base, player_base);

    /* Process that file, look for old files */
    if ((process_pref_file(buf) < 0) && (name_is_numbered(player_name)))
    {
        char old_py_name[32];
        strcpy(old_py_name, player_name);
        temporary_name_hack = TRUE;

        while (1)
        {
            bump_numeral(player_name, -1);
            process_player_name(FALSE);

            sprintf(buf, "%s.prf", player_base);

            if (process_pref_file(buf) >= 0)
            {
                strcpy(pref_save_base, player_base);
                break;
            }
            if (!name_is_numbered(player_name)) break;
        }
        strcpy(player_name, old_py_name);
        process_player_name(FALSE);
        temporary_name_hack = FALSE;
    }

    /* Access the "realm 1" pref file */
    if (p_ptr->realm1 != REALM_NONE)
    {
        sprintf(buf, "%s.prf", realm_names[p_ptr->realm1]);

        /* Process that file */
        process_pref_file(buf);
    }

    /* Access the "realm 2" pref file */
    if (p_ptr->realm2 != REALM_NONE)
    {
        sprintf(buf, "%s.prf", realm_names[p_ptr->realm2]);

        /* Process that file */
        process_pref_file(buf);
    }

    if (new_game) alp_mode |= ALP_NEW_GAME;

    /* Load an autopick preference file */
    autopick_load_pref(alp_mode);
}


/*
 * Extract option variables from bit sets
 */
void extract_option_vars(void)
{
    int i;

    for (i = 0; option_info[i].o_desc; i++)
    {
        int os = option_info[i].o_set;
        int ob = option_info[i].o_bit;

        /* Set the "default" options */
        if (option_info[i].o_var)
        {
            /* Set */
            if (option_flag[os] & (1L << ob))
            {
                /* Set */
                (*option_info[i].o_var) = TRUE;
            }

            /* Clear */
            else
            {
                /* Clear */
                (*option_info[i].o_var) = FALSE;
            }
        }
    }
}

/*
 * Determine today's bounty monster
 * Note: conv_old is used if loaded 0.0.3 or older save file
 */
void determine_today_mon(bool conv_old)
{
    int max_dl = 3, i;
    bool old_inside_battle = p_ptr->inside_battle;
    monster_race *r_ptr;
	int loopcounter = 0, old_today_mon = today_mon;

    if (!conv_old)
    {
        for (i = 0; i < max_d_idx; i++)
        {
            if (max_dlv[i] < d_info[i].mindepth) continue;
            if (max_dl < max_dlv[i]) max_dl = max_dlv[i];
        }
    }
    else max_dl = MAX(max_dlv[DUNGEON_ANGBAND], 3);

    p_ptr->inside_battle = TRUE;
    get_mon_num_prep(NULL, NULL);

    while (loopcounter < 2000)
    {
        today_mon = get_mon_num(max_dl);
        r_ptr = &r_info[today_mon];
		loopcounter++;

        if (r_ptr->flags1 & RF1_UNIQUE) continue;
        if (r_ptr->flags7 & (RF7_NAZGUL | RF7_UNIQUE2)) continue;
        if (r_ptr->flags2 & RF2_MULTIPLY) continue;
        if ((r_ptr->flags9 & (RF9_DROP_CORPSE | RF9_DROP_SKELETON)) != (RF9_DROP_CORPSE | RF9_DROP_SKELETON)) continue;
        if (r_ptr->level < MIN(max_dl / 2, 40)) continue;
        if (r_ptr->rarity > 10) continue;
        break;
    }

	if (loopcounter >= 2000) today_mon = old_today_mon;
    p_ptr->today_mon = 0;
    p_ptr->inside_battle = old_inside_battle;
}


/*
 * Actually play a game
 *
 * If the "new_game" parameter is true, then, after loading the
 * savefile, we will commit suicide, if necessary, to allow the
 * player to start a new game.
 */
void play_game(bool new_game)
{
    int i;
    bool load_game = TRUE;

    autosave_l = TRUE;

    /* Hack -- Character is "icky" */
    character_icky = TRUE;

    /* Make sure main term is active */
    Term_activate(angband_term[0]);

    /* Initialise the resize hooks */
    angband_term[0]->resize_hook = resize_map;

    for (i = 1; i < 8; i++)
    {
        /* Does the term exist? */
        if (angband_term[i])
        {
            /* Add the redraw on resize hook */
            angband_term[i]->resize_hook = redraw_window;
        }
    }

    /* The Windows port blocks until the user chooses a menu for a New game, or
       to load an existing game. Thus, it will display its own start screen ... */
    if (strcmp(ANGBAND_SYS, "win") != 0)
    {
        /* On X11, you need to flush() before Term->hgt is accurate! */
        Term_flush();
        display_news();
    }

    /* Hack -- turn off the cursor */
    (void)Term_set_cursor(0);

    /* Attempt to load */
    if (!load_player())
    {
        quit("broken savefile");
    }

    /* Extract the options */
    if (!character_loaded) extract_option_vars();

    creating_savefile = new_game;

    /* Nothing loaded */
    if (!character_loaded)
    {
        /* Make new player */
        new_game = TRUE;

        /* The dungeon is not ready */
        character_dungeon = FALSE;

        /* Prepare to init the RNG */
        Rand_quick = TRUE;

        /* Initialize the saved floors data */
        init_saved_floors(FALSE);
    }

    /* Old game is loaded. But new game is requested. */
    else if (new_game)
    {
        /* Initialize the saved floors data */
        init_saved_floors(TRUE);
    }

    /* Process old character */
    if (!new_game)
    {
        /* Process the player name */
        process_player_name(FALSE);
    }

    /* Init the RNG */
    if (Rand_quick)
    {
        u32b seed;

        /* Basic seed */
        seed = (u32b)time(NULL);

#ifdef SET_UID

        /* Mutate the seed on Unix machines */
        seed = ((seed >> 3) * (getpid() << 1));

#endif

        /* Use the complex RNG */
        Rand_quick = FALSE;

        /* Seed the "complex" RNG */
        Rand_state_init(seed);
    }

    /* Roll new character */
    if (new_game)
    {
        /* The dungeon is not ready */
        character_dungeon = FALSE;

        /* Start in town */
        dun_level = 0;
        p_ptr->inside_arena = FALSE;
        p_ptr->inside_battle = FALSE;

        write_level = TRUE;

        /* Hack -- seed for flavors */
        seed_flavor = randint0(0x10000000);

        /* Hack -- seed for town layout */
        seed_town = randint0(0x10000000);

        /* Roll up a new character */
        player_birth();

        counts_write(2,0);
        p_ptr->count = 0;

        load = FALSE;

        determine_today_mon(FALSE);

        /* Initialize object array */
        wipe_o_list();

        /* Confirm unusual settings
         * (players who play both coffee-break and normal games and start
         * new games by loading old savefiles sometimes turn coffee-break off,
         * but forget to turn no_wilderness and ironman_downward off) */
        if ((!coffee_break) && (ironman_downward))
        {
            bool okei = get_check("Really play with ironman stairs? ");
            if (!okei)
            {
                ironman_downward = FALSE;
                if ((no_wilderness) && (!get_check("Really play with no wilderness? "))) no_wilderness = FALSE;
            }
        }

        /* After the last opportunity to modify birth options... */
        birth_location();
    }
    else
    {
        write_level = FALSE;
        if (p_ptr->riding == -1)
        {
            p_ptr->riding = 0;
            for (i = m_max; i > 0; i--)
            {
                if (player_bold(m_list[i].fy, m_list[i].fx))
                {
                    p_ptr->riding = i;
                    break;
                }
            }
        }
    }

    /* Suppress extra pantheons */
    if (single_pantheon) _suppress_extra_pantheons();

    /* Empty lore */
    if ((new_game) && (empty_lore)) empty_lore_wipe();

    creating_savefile = FALSE;

    p_ptr->teleport_town = FALSE;
    p_ptr->sutemi = FALSE;
    world_monster = FALSE;
    now_turn = game_turn;
    start_time = time(NULL);

    /* TODO: py_skills_init() or some such ... w_max needs to be reset each time you play, 
     * not just on player birth */
    if (p_ptr->pclass == CLASS_WEAPONMASTER && !new_game)
        weaponmaster_adjust_skills();

    /* Fill the arrays of floors and walls in the good proportions */
    set_floor_and_wall(dungeon_type);

    /* Flavor the objects */
    flavor_init();

    /* Flush the message */
    Term_fresh();

    /* Hack -- Enter wizard mode */
    if (arg_wizard)
    {
        if (enter_wizard_mode())
        {
            p_ptr->wizard = TRUE;

            if (p_ptr->is_dead || !py || !px)
            {
                /* Initialize the saved floors data */
                init_saved_floors(TRUE);

                /* Avoid crash in update_view() */
                py = px = 10;
            }
        }
        else if (p_ptr->is_dead)
        {
            quit("Already dead.");
        }
    }

    /* Initialize the town-buildings if necessary ... user restarted the
     * game while on the surface, possibly even inside a town */
    if (py_on_surface())
    {
        process_dungeon_file("w_info.txt", 0);
        towns_init_buildings();
    }

    /* Generate a dungeon level if needed */
    if (!character_dungeon)
    {
        change_floor();
    }

    else
    {
        /* HACK -- Restore from panic-save */
        if (p_ptr->panic_save)
        {
            /* No player?  -- Try to regenerate floor */
            if (!py || !px)
            {
                quest_ptr qp = quests_get_current();
                msg_print("What a strange player location. Regenerate the dungeon floor.");
                if (qp && qp->id) qp->status = QS_TAKEN;
                enter_quest = FALSE;
                if (qp) /* Broken quest - force exit */
                {
                   dungeon_type = 0;
                   dun_level = 0;
                   quests_on_leave();
                }
                change_floor();
            }

            /* Still no player?  -- Try to locate random place */
            if (!py || !px) py = px = 10;

            /* No longer in panic */
            p_ptr->panic_save = 0;
        }
    }

    /* Character is now "complete" */
    character_generated = TRUE;

    /* Hack -- Character is no longer "icky" */
    character_icky = FALSE;

    /* Start game */
    p_ptr->playing = TRUE;

    /* Reset the visual mappings */
    reset_visuals();

    /* Load the "pref" files */
    load_all_pref_files(new_game);

    /* Turn on easy mimics */
    toggle_easy_mimics(easy_mimics);

    Term_xtra(TERM_XTRA_REACT, 0);
    p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL);
    p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_DUNGEON | PW_MONSTER_LIST | PW_OBJECT_LIST | PW_MONSTER | PW_OBJECT);
    window_stuff();
    viewport_verify_aux(VIEWPORT_FORCE_CENTER);

    /* Give startup outfit (after loading pref files) */
    if (new_game)
    {
        class_t *class_ptr = get_class();
        race_t *race_ptr = get_race();
        personality_ptr pers_ptr = get_personality();

        do_cmd_redraw();  /* Not sure why this is required?! */

        if (thrall_mode)
        msg_print("<color:B>Welcome!</color> You begin your adventure deep in the dungeon, "
                  "a runaway slave of this dark pit's demonic masters. Your attempts to "
                  "gather supplies for your escape have been detected, and now everybody "
                  "is on high alert... \n"
                  "This is the message line where important information is "
                  "communicated to you while you play the game. "
                  "Press <color:y>SPACE</color> every time you see a <color:B>-more-</color> prompt and "
                  "you are finished reading the current messages. "
                  "Press <color:y>CTRL+P</color> to review recent messages. "
                  "You may press <color:y>?</color> at any time for help.\n\n");
        else msg_print("<color:B>Welcome!</color> You begin life in the town where you may purchase "
                  "supplies for the dangers that await you.\n"
                  "This is the message line where important information is "
                  "communicated to you while you play the game. "
                  "Press <color:y>SPACE</color> every time you see a <color:B>-more-</color> prompt and "
                  "you are finished reading the current messages. "
                  "Press <color:y>CTRL+P</color> to review recent messages. "
                  "You may press <color:y>?</color> at any time for help.\n\n");
        msg_boundary();

        skills_on_birth();   /* Hack: Skills must init before racial birth for monster race innate proficiency! */
        if (pers_ptr->birth) /* Hack: Personality goes first for the Sexy Whip! */
            pers_ptr->birth();

        /* birth functions should handle this
        player_outfit();*/

        /* Note: The class birth function should give starting
         * equipment and spellbooks while the race birth function
         * should give starting food and light (in general) */
        if (class_ptr->birth)
            class_ptr->birth();

        if (race_ptr->birth)
            race_ptr->birth();
        else
        {
            /* most races won't need a special birth function, so
             * give standard food and light by default */
            py_birth_food();
            py_birth_light();
        }
        if ((coffee_break) && (!thrall_mode) && (p_ptr->pclass != CLASS_BERSERKER)) py_birth_obj_aux(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL, (game_mode == GAME_MODE_BEGINNER) ? 10 : 1);
        if (thrall_mode)
        {
            if (p_ptr->pclass == CLASS_BERSERKER)
            {
                py_birth_obj_aux(TV_POTION, SV_POTION_ENLIGHTENMENT, 10);
                py_birth_obj_aux(TV_POTION, SV_POTION_CURING, 2);
            }
            else
            {
                object_type forge;
                py_birth_obj_aux(TV_SCROLL, SV_SCROLL_IDENTIFY, 12);
                py_birth_obj_aux(TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION, 10);
                py_birth_obj_aux(TV_SCROLL, SV_SCROLL_REMOVE_CURSE, 1);
                py_birth_obj_aux(TV_POTION, SV_POTION_CURING, 2);
                object_prep(&forge, lookup_kind(TV_ROD, SV_ANY));
                device_init_fixed(&forge, EFFECT_DETECT_ALL);
                py_birth_obj(&forge);
            }

            /* Start with no gold */
            p_ptr->au = 0;
        }

        spell_stats_on_birth();

        stats_on_gold_find(p_ptr->au); /* Found? Inherited? What's the difference? */

        if (class_ptr->gain_level) /* Gain CL1 (e.g. Chaos Warriors) */
            (class_ptr->gain_level)(p_ptr->lev);
    }


    /* Set or clear "rogue_like_commands" if requested */
    if (arg_force_original) rogue_like_commands = FALSE;
    if (arg_force_roguelike) rogue_like_commands = TRUE;

    /* Hack -- Enforce "delayed death" */
    if (p_ptr->chp < 0) p_ptr->is_dead = TRUE;

    if (p_ptr->prace == RACE_ANDROID) android_calc_exp();

    if (new_game && ((p_ptr->pclass == CLASS_CAVALRY) || (p_ptr->pclass == CLASS_BEASTMASTER)))
    {
        monster_type *m_ptr;
        int pet_r_idx = ((p_ptr->pclass == CLASS_CAVALRY) ? MON_HORSE : MON_YASE_HORSE);
        int my = (no_wilderness) ? py - 1 : py + 1;
        int mx = px;
        monster_race *r_ptr = &r_info[pet_r_idx];
        if (!monster_can_enter(my, mx, r_ptr, 0)) /* Find empty place for initial pet */
        {
            int dmy, dmx, ties = 0, paras = 1000, etaisyys = 1;
            for (etaisyys = 1; ((etaisyys < 8) && (paras >= etaisyys)); etaisyys++)
            {
                for (dmy = 0 - etaisyys; dmy <= etaisyys; dmy++)
                {
                    for (dmx = 0 - etaisyys; dmx <= etaisyys; dmx++)
                    {
                        int tulos = etaisyys;
                        if ((ABS(dmy) != etaisyys) && (ABS(dmx) != etaisyys)) dmx = etaisyys;
                        if (!in_bounds(py + dmy, px + dmx)) continue;
                        if (!monster_can_enter(py + dmy, px + dmx, r_ptr, 0)) continue;
                        if (!projectable(py, px, py + dmy, px + dmx)) tulos += 3;
                        if (tulos < paras)
                        {
                            paras = tulos;
                            ties = 0;
                        }
                        if ((tulos <= paras) && (randint0(++ties) == 0))
                        {
                            my = py + dmy;
                            mx = px + dmx;
                        }
                    }
                }
            }
        }
        place_monster_aux(0, my, mx, pet_r_idx, (PM_FORCE_PET | PM_NO_KAGE));
        m_ptr = &m_list[hack_m_idx_ii];
        m_ptr->mspeed = r_ptr->speed;
        m_ptr->maxhp = r_ptr->hdice*(r_ptr->hside+1)/2;
        m_ptr->max_maxhp = m_ptr->maxhp;
        m_ptr->hp = r_ptr->hdice*(r_ptr->hside+1)/2;
        m_ptr->energy_need = ENERGY_NEED() + ENERGY_NEED();
    }

    /* Process */
    while (TRUE)
    {
        /* Process the level */
        dungeon(load_game);

        /* Handle "p_ptr->notice" */
        notice_stuff();

        /* Hack -- prevent "icky" message */
        character_xtra = TRUE;

        /* Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window" */
        handle_stuff();

        character_xtra = FALSE;

        /* Cancel the target */
        target_who = 0;

        /* Cancel the health bar */
        health_track(0);


        /* Forget the lite */
        forget_lite();

        /* Forget the view */
        forget_view();

        /* Forget the view */
        clear_mon_lite();

        /* Handle "quit and save" */
        if (!p_ptr->playing && !p_ptr->is_dead) break;

        /* Erase the old cave */
        wipe_o_list();
        if (!p_ptr->is_dead) wipe_m_list();

        load_game = FALSE;

        /* Accidental Death */
        if (p_ptr->playing && p_ptr->is_dead)
        {
            if (p_ptr->inside_arena)
            {
                p_ptr->inside_arena = FALSE;
                if (p_ptr->arena_number > MAX_ARENA_MONS)
                    p_ptr->arena_number++;
                else
                    p_ptr->arena_number = -1 - p_ptr->arena_number;
                p_ptr->is_dead = FALSE;
                p_ptr->chp = 0;
                p_ptr->chp_frac = 0;
                p_ptr->exit_bldg = TRUE;
                reset_tim_flags();

                fame_on_failure();

                /* Leave through the exit */
                prepare_change_floor_mode(CFM_FIRST_FLOOR);

                /* prepare next floor */
                leave_floor();
            }
            else
            {
                /* Mega-Hack -- Allow player to cheat death */
                if (((p_ptr->total_winner) && (unique_is_friend(MON_R_MACHINE))) || ((p_ptr->wizard || cheat_live) && !get_check("Die? ")))
                {
                    quest_ptr qp = quests_get_current();
                    bool was_in_dung = (dun_level > 0);
                    bool no_cheat = ((!p_ptr->wizard) && (!cheat_live));

                    /* Mark savefile */
                    if (!no_cheat)
                    {
                        p_ptr->noscore |= 0x0001;
                        msg_print("You invoke wizard mode and cheat death.");
                        msg_print(NULL);
                    }

                    /* Restore hit points */
                    p_ptr->chp = p_ptr->mhp;
                    p_ptr->chp_frac = 0;

                    magic_eater_restore_all();

                    /* Restore spell points */
                    p_ptr->csp = p_ptr->msp;
                    p_ptr->csp_frac = 0;

                    /* Hack -- cancel recall */
                    if (p_ptr->word_recall)
                    {
                        /* Message */
                        msg_print("A tension leaves the air around you...");

                        msg_print(NULL);

                        /* Hack -- Prevent recall */
                        p_ptr->word_recall = 0;
                        p_ptr->redraw |= (PR_STATUS);
                    }

                    /* Hack -- cancel alter */
                    if (p_ptr->alter_reality)
                    {
                        /* Hack -- Prevent alter */
                        p_ptr->alter_reality = 0;
                        p_ptr->redraw |= (PR_STATUS);
                    }

                    /* Hack -- cancel quest */
                    if (qp && qp->id) qp->status = QS_TAKEN;
                    enter_quest = FALSE;
                    if (qp) /* Exit the quest */
                    {
                        dungeon_type = 0;
                        dun_level = 0;
                        quests_on_leave();
                    }

                    /* Note cause of death XXX XXX XXX */
                    if (!no_cheat) (void)strcpy(p_ptr->died_from, "Cheating death");

                    /* Do not die */
                    p_ptr->is_dead = FALSE;

                    /* Hack -- Healing */
                    (void)set_blind(0, TRUE);
                    (void)set_confused(0, TRUE);
                    (void)set_poisoned(0, TRUE);
                    fear_clear_p();
                    (void)set_paralyzed(0, TRUE);
                    (void)set_image(0, TRUE);
                    (void)set_stun(0, TRUE);
                    (void)set_cut(0, TRUE);

                    /* Hack -- Prevent starvation */
                    (void)set_food(PY_FOOD_MAX - 1);

                    dun_level = 0;
                    p_ptr->inside_arena = FALSE;
                    p_ptr->inside_battle = FALSE;
                    if (dungeon_type) p_ptr->recall_dungeon = dungeon_type;
                    dungeon_type = 0;
                    if (was_in_dung)
                    {
                        if (no_wilderness)
                        {
                            p_ptr->wilderness_y = 1;
                            p_ptr->wilderness_x = 1;
                            p_ptr->wilderness_dx = 0;
                            p_ptr->wilderness_dy = 0;
                            p_ptr->oldpy = 33;
                            p_ptr->oldpx = 131;
                        }
                        else /* Move to convenient safe location (Morivant) */
                        {
                            p_ptr->wilderness_y = 50;
                            p_ptr->wilderness_x = 47;
                            p_ptr->wilderness_dx = 0;
                            p_ptr->wilderness_dy = 0;
                            p_ptr->oldpy = 42;
                            p_ptr->oldpx = 96;
                        }
                    }
                    /* Leaving */
                    p_ptr->wild_mode = FALSE;
                    p_ptr->leaving = TRUE;
                    quest_reward_drop_hack = FALSE;

                    if (no_cheat)
                    {
                        msg_print("You are resurrected!");
                        dungeon_type = DUNGEON_HEAVEN;
                        dun_level = d_info[dungeon_type].maxdepth;
                    }

                    /* Prepare next floor */
                    leave_floor();
                    wipe_m_list();
                }
            }
        }

        /* Handle "death" */
        if (p_ptr->is_dead) break;

        /* Make a new level */
        change_floor();
    }

    /* Close stuff */
    close_game();

    /* Quit */
    quit(NULL);
}

s32b turn_real(s32b hoge)
{
    switch (p_ptr->start_race)
    {
    case RACE_VAMPIRE:
    case RACE_MON_VAMPIRE:
    case RACE_SKELETON:
    case RACE_ZOMBIE:
    case RACE_SPECTRE:
        return hoge - (TURNS_PER_TICK * TOWN_DAWN * 3 / 4);
    default:
        return hoge;
    }
}

void prevent_turn_overflow(void)
{
    int rollback_days;
    s32b rollback_turns;

    if (game_turn < game_turn_limit) return;

    rollback_days = 1 + (game_turn - game_turn_limit) / (TURNS_PER_TICK * TOWN_DAWN);
    rollback_turns = TURNS_PER_TICK * TOWN_DAWN * rollback_days;

    if (game_turn > rollback_turns) game_turn -= rollback_turns;
    else game_turn = 1; /* Paranoia */
    if (old_turn > rollback_turns) old_turn -= rollback_turns;
    else old_turn = 1;
    if (old_battle > rollback_turns) old_battle -= rollback_turns;
    else old_battle = 1;
    if (p_ptr->feeling_turn > rollback_turns) p_ptr->feeling_turn -= rollback_turns;
    else p_ptr->feeling_turn = 1;

    towns_on_turn_overflow(rollback_turns);
}


