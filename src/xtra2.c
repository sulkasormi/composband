/* File: xtra2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: effects of various "objects" */

#include "angband.h"

#include <assert.h>

#define REWARD_CHANCE 10
#define OLYMPIAN_CHANCE 20 /* Olympians are now a bit easier */

static bool old_target_never_okay = FALSE;

 /* Experience required to advance from level to level + 1
    Note the table is off by 1, so we encapsulate that fact.
    (e.g. table[0] gives xp needed to go from L1 to L2!)
    Also, the old code (cut and pasted multiple places) would
    overflow once expfact got too high which is easily avoided.
    Finally, Androids do it all differently!

    To be sure everybody is doing it correctly, I moved the tables
    here and renamed them!
 */
static s32b _player_exp[PY_MAX_LEVEL] =
{
    10,
    25,
    45,
    70,
    100,
    140,
    200,
    280,
    380,/*10*/
    500,
    650,
    850,
    1100,
    1400,
    1800,
    2300,
    2900,
    3600,
    4400,/*20*/
    5400,
    6800,
    8400,
    10200,
    12500,
    17500,
    25000,
    35000L,
    50000L,
    75000L,/*30*/
    100000L,
    150000L,
    200000L,
    275000L,
    350000L,
    450000L,
    550000L,
    700000L,
    850000L,
    1000000L,/*40*/
    1250000L,
    1500000L,
    1800000L,
    2100000L,
    2400000L,
    2700000L,
    3000000L,
    3500000L,
    4000000L,
    4500000L,/*50*/
    5000000L
};


static s32b _player_exp_a[PY_MAX_LEVEL] =
{
    20,
    50,
    100,
    170,
    280,
    430,
    650,
    950,
    1400,/*10*/
    1850,
    2300,
    2900,
    3600,
    4400,
    5400,
    6800,
    8400,
    10400,
    12500,/*20*/
    17500,
    25000,
    35000,
    50000L,
    75000L,
    100000L,
    150000L,
    200000L,
    275000L,
    350000L,/*30*/
    450000L,
    550000L,
    650000L,
    800000L,
    950000L,
    1100000L,
    1250000L,
    1400000L,
    1550000L,
    1700000L,/*40*/
    1900000L,
    2100000L,
    2300000L,
    2550000L,
    2800000L,
    3050000L,
    3300000L,
    3700000L,
    4100000L,
    4500000L,/*50*/
    5000000L
};

int exp_requirement(int level)
{
    bool android = (p_ptr->prace == RACE_ANDROID ? TRUE : FALSE);
    int base = (android ? _player_exp_a : _player_exp)[level-1];
    if (base % 100 == 0)
        return base / 100 * p_ptr->expfact;
    else
        return base * p_ptr->expfact / 100;
}

void gain_chosen_stat(void)
{
    int choice;
    if (statistics_hack)
        return;
    screen_save();
    while(1)
    {
        int n;
        char tmp[32];

        cnv_stat(p_ptr->stat_max[0], tmp);
        put_str(format("        a) Str (cur %6.6s)              ", tmp), 2, 14);
        cnv_stat(p_ptr->stat_max[1], tmp);
        put_str(format("        b) Int (cur %6.6s)              ", tmp), 3, 14);
        cnv_stat(p_ptr->stat_max[2], tmp);
        put_str(format("        c) Wis (cur %6.6s)              ", tmp), 4, 14);
        cnv_stat(p_ptr->stat_max[3], tmp);
        put_str(format("        d) Dex (cur %6.6s)              ", tmp), 5, 14);
        cnv_stat(p_ptr->stat_max[4], tmp);
        put_str(format("        e) Con (cur %6.6s)              ", tmp), 6, 14);
        cnv_stat(p_ptr->stat_max[5], tmp);
        put_str(format("        f) Chr (cur %6.6s)              ", tmp), 7, 14);
        put_str("                                         ", 8, 14);
        c_put_str(TERM_YELLOW, "        Which stat do you want to raise? ", 1, 14);

        while(1)
        {
            choice = inkey();
            if ((choice >= 'a') && (choice <= 'f')) break;
        }
        for(n = 0; n < 6; n++)
        {
            if (n != choice - 'a')
                put_str("                                         ", n+2, 14);
        }
        if (get_check("Are you sure? ")) break;
    }
    do_inc_stat(choice - 'a');
    screen_load();
}

/*
 * Advance experience levels and print experience
 */


void check_experience(void)
{
    int old_lev = p_ptr->lev;
    static bool level_inc_stat = FALSE;

    /* Hack -- lower limit */
    if (p_ptr->exp < 0) p_ptr->exp = 0;
    if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;
    if (p_ptr->max_max_exp < 0) p_ptr->max_max_exp = 0;

    /* Hack -- upper limit */
    if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;
    if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;
    if (p_ptr->max_max_exp > PY_MAX_EXP) p_ptr->max_max_exp = PY_MAX_EXP;

    if (p_ptr->is_dead) /* Disallow posthumous XP gains */
    {
        if (p_ptr->exp > p_ptr->max_max_exp) p_ptr->exp = p_ptr->max_max_exp;
        return;
    }

    /* Hack -- maintain "max" experience */
    if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

    /* Hack -- maintain "max max" experience */
    if (p_ptr->max_exp > p_ptr->max_max_exp) p_ptr->max_max_exp = p_ptr->max_exp;

    if (p_ptr->pclass == CLASS_POLITICIAN) politician_check_experience(FALSE);

    /* Redraw experience */
    p_ptr->redraw |= (PR_EXP);

    /* Lose levels while possible */
    while ((p_ptr->lev > 1) &&
           (p_ptr->exp < exp_requirement(p_ptr->lev - 1)))
    {
        /* Lose a level */
        p_ptr->lev--;

        /* Update some stuff */
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

        /* Redraw some stuff */
        p_ptr->redraw |= (PR_LEV);

        /* Handle stuff */
        handle_stuff();
    }

    /* Gain levels while possible */
    while ((p_ptr->lev < PY_MAX_LEVEL) &&
           (p_ptr->exp >= exp_requirement(p_ptr->lev)))
    {
        if (level_inc_stat) /* check for delays from weird stuff */
        {
            gain_chosen_stat();
            level_inc_stat = FALSE;
        }

        p_ptr->lev++;

        if (p_ptr->pclass == CLASS_WILD_TALENT) wild_talent_fix_up();

        /* Save the highest level */
        if (p_ptr->lev > p_ptr->max_plv)
        {
            class_t *class_ptr = get_class();

            p_ptr->max_plv = p_ptr->lev;

            sound(SOUND_LEVEL);
            cmsg_format(TERM_L_GREEN, "Welcome to level %d.", p_ptr->lev);

            if ((p_ptr->max_plv % 5) == 0) level_inc_stat = TRUE;

            if (class_ptr->gain_level != NULL)
                (class_ptr->gain_level)(p_ptr->lev);

            if (mut_present(MUT_CHAOS_GIFT))
                chaos_warrior_reward();

            if (p_ptr->personality == PERS_SPLIT)
                split_shuffle(FALSE);

            /* N.B. The class hook or the Chaos Gift mutation may result in a race
               change (stupid Chaos-Warriors), so we better always requery the player's
               race to make sure the correct racial hook is called. */
            {
                race_t *race_ptr = get_true_race(); /* So players don't miss if they Polymorph Demon, etc */

                if (p_ptr->prace == RACE_DOPPELGANGER) /* But a doppelganger should use the mimicked race! */
                    race_ptr = get_race();

                if (race_ptr->gain_level != NULL)
                    (race_ptr->gain_level)(p_ptr->lev);
            }
        }


        /* Update some stuff */
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

        /* Redraw some stuff */
        p_ptr->redraw |= (PR_LEV | PR_EXP);

        /* Window stuff */
        p_ptr->window |= (PW_SPELL | PW_INVEN);

        level_up = 1;

        /* Handle stuff */
        handle_stuff();

        level_up = 0;

        if (level_inc_stat)
        {
            gain_chosen_stat();
            level_inc_stat = FALSE;
        }
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
        p_ptr->redraw |= (PR_LEV);
        p_ptr->window |= (PW_SPELL);
        handle_stuff();
    }

    if (old_lev != p_ptr->lev)
    {
        race_t *race_ptr = get_true_race(); /* So players don't miss if they Polymorph Demon, etc */

        if (p_ptr->prace == RACE_DOPPELGANGER) /* But a doppelganger should use the mimicked race! */
            race_ptr = get_race();

        if (race_ptr->change_level)
            race_ptr->change_level(old_lev, p_ptr->lev);

        autopick_load_pref(0);
    }
}


/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
static int get_coin_type(int r_idx)
{
    /* Analyze monsters */
    switch (r_idx)
    {
    case MON_COPPER_COINS: return 2;
    case MON_SILVER_COINS: return 5;
    case MON_GOLD_COINS: return 10;
    case MON_MITHRIL_COINS:
    case MON_MITHRIL_GOLEM: return 16;
    case MON_ADAMANT_COINS: return 17;
    }

    /* Assume nothing */
    return 0;
}


/*
 * Hack -- determine if a template is Cloak
 */
static bool _kind_is_cloak(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if (k_ptr->tval == TV_CLOAK)
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is Polearm
 */
static bool _kind_is_polearm(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if (k_ptr->tval == TV_POLEARM)
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is Sword
 */
static bool _kind_is_sword(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if ((k_ptr->tval == TV_SWORD) && (k_ptr->sval > 2))
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is Book
 */
static bool _kind_is_book(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if ((k_ptr->tval >= TV_LIFE_BOOK) && (k_ptr->tval <= TV_NECROMANCY_BOOK))
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is Good book
 */
static bool _kind_is_good_book(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if ((k_ptr->tval >= TV_LIFE_BOOK) && (k_ptr->tval <= TV_NECROMANCY_BOOK) && (k_ptr->tval != TV_ARCANE_BOOK) && (k_ptr->sval > 1))
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is Armor
 */
static bool _kind_is_armor(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if (k_ptr->tval == TV_HARD_ARMOR)
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}


/*
 * Hack -- determine if a template is hafted weapon
 */
static bool _kind_is_hafted(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Analyze the item type */
    if (k_ptr->tval == TV_HAFTED)
    {
        return (TRUE);
    }

    /* Assume not good */
    return (FALSE);
}

/*
 * Return monster death string
 */
cptr extract_note_dies(monster_race *r_ptr)
{
    /* Some monsters get "destroyed" */
    if (!monster_living(r_ptr))
    {
        int i;

        for (i = 0; i < 4; i++)
        {
            if (r_ptr->blows[i].method == RBM_EXPLODE)
            {
                return " explodes into tiny shreds.";
            }
        }

        return " is destroyed.";
    }

    /* Assume a default death */
    return " dies.";
}

byte get_monster_drop_ct(monster_type *m_ptr)
{
    int number = 0;
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    if ((coffee_break == SPEED_INSTA_COFFEE) && (m_ptr->mflag2 & MFLAG2_PLAYER_SUMMONED)) return 0;
    if (is_pet(m_ptr) || p_ptr->inside_battle || p_ptr->inside_arena) return 0; /* Pets drop nothing */

    if ((r_ptr->flags1 & RF1_DROP_60) && (randint0(100) < 60)) number++;
    if ((r_ptr->flags1 & RF1_DROP_90) && ((r_ptr->flags1 & RF1_UNIQUE) || randint0(100) < 90)) number++;
    if  (r_ptr->flags1 & RF1_DROP_1D2) number += damroll(1, 2);
    if  (r_ptr->flags1 & RF1_DROP_2D2) number += damroll(2, 2);
    if  (r_ptr->flags1 & RF1_DROP_3D2) number += damroll(3, 2);
    if  (r_ptr->flags1 & RF1_DROP_4D2) number += damroll(4, 2);

    /* Hack: There are currently too many objects, IMO.
       Please rescale in r_info rather than the following! */
    if ( number > 2
      && !(r_ptr->flags1 & RF1_DROP_GREAT)
      && !(r_ptr->flags1 & RF1_UNIQUE) )
    {
        number = 2 + (number - 2) / 2;
    }

    if ((number) && (coffee_break))
    {
        if (py_in_dungeon() && (!(r_ptr->flags2 & RF2_MULTIPLY)))
        {
            number *= 2;
            if (number > 7) number -= (number / 7);
            if (((dun_level >= 46) || (p_ptr->max_plv >= 38)) && (number > 5) && (!(r_ptr->flags1 & RF1_DROP_GREAT))
                && (!(r_ptr->flags1 & RF1_UNIQUE)) && (!(r_ptr->flags1 & RF1_ONLY_GOLD)) && (magik(MAX(10, (p_ptr->max_plv - 35) * 3)))) number -= 1;
        }
        if (coffee_break == SPEED_INSTA_COFFEE)
        {
            int cap = (p_ptr->coffee_lv_revisits) ? 40 : MAX(40, 15 + (dun_level * 2 / 3));
            int tapot = (p_ptr->lv_kills + p_ptr->pet_lv_kills);
            bool _is_summon = (m_ptr->smart & (1U << SM_SUMMONED)) ? TRUE : FALSE;
            if (((tapot > cap) || (m_ptr->mflag2 & MFLAG2_SPAWN)) && ((_is_summon) || (py_in_dungeon())))
            {
                int lisays = _is_summon ? MAX(50, 75 + cap - tapot) : MAX(125, 150 + cap - tapot);
                int vahennys = ((number * tapot) / (tapot + lisays));
                if (randint0(tapot + lisays) < ((number * tapot) % (tapot + lisays))) vahennys++;
//                msg_format("Tiputuksia: %d->%d Monster %s", number, number - vahennys, _is_summon ? "summoned" : "not summoned");
                number = MAX(0, number - vahennys);
                if (m_ptr->mflag2 & MFLAG2_SPAWN) /* Can random monsters even spawn in insta-coffee mode? Maybe in the town... */
                {
                    number /= 2;
                }
            }
            else if ((py_in_dungeon()) && (!_is_summon) && (randint0(number) > 1)) number++;
        }
    }

    if (m_ptr->mflag2 & MFLAG2_WASPET)
    {
        number /= 8;
    }

    /* No more farming quartz veins for millions in gold */
    if (r_ptr->flags2 & RF2_MULTIPLY)
    {
        int cap = 600;
        if (r_ptr->flags1 & RF1_ONLY_GOLD)
            cap = 200; /* About 110k gp at DL21 */
        if (no_selling) cap /= 2; /* Gold drops are bigger with no_selling */
        if (coffee_break) cap /= (coffee_break * 2); /* More drops in coffee_break mode */
        if (r_ptr->flags1 & RF1_NEVER_MOVE) cap /= 2;
        if (r_ptr->r_akills > cap)
            number = 0;
    }

    /* No more farming summoners for drops (The Hoard, That Bat, Draconic Qs, etc) */
    if (m_ptr->parent_m_idx && !(r_ptr->flags1 & RF1_UNIQUE))
    {
        monster_type *pm_ptr = &m_list[m_ptr->parent_m_idx];

        if (pm_ptr->r_idx)
        {
            int max_kills = 250;
            monster_race *pr_ptr = &r_info[pm_ptr->r_idx];

            if (pr_ptr->flags1 & RF1_UNIQUE)
                max_kills = 100;
            else if (pr_ptr->d_char == 'Q')
                max_kills = 100;

            if ((no_selling) && (r_ptr->flags1 & RF1_ONLY_GOLD)) max_kills /= 2;
            if (coffee_break) max_kills /= (coffee_break * 2);

            if (pr_ptr->r_skills > max_kills)
                number = 0;
        }
    }

    return number;
}

static int _mon_drop_lvl(int dl, int rl)
{
    if (rl >= dl) return rl;
    return (rl + dl) / 2;
}

bool get_monster_drop(int m_idx, object_type *o_ptr)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    bool do_gold = (!(r_ptr->flags1 & RF1_ONLY_ITEM));
    bool do_item = (!(r_ptr->flags1 & RF1_ONLY_GOLD));
    int force_coin = get_coin_type(m_ptr->r_idx);
    u32b mo_mode = 0L;

    if (is_pet(m_ptr))
        return FALSE;

    if (m_ptr->stolen_ct >= m_ptr->drop_ct)
        return FALSE;

    if (r_ptr->flags1 & RF1_DROP_GOOD)
        mo_mode |= AM_GOOD;
    if (r_ptr->flags1 & RF1_DROP_GREAT)
        mo_mode |= AM_GREAT;

    obj_drop_theme = 0;
    if (r_ptr->drop_theme && one_in_(2))
        obj_drop_theme = r_ptr->drop_theme;
    else /* Don't try to tailor themed drops since they could easily fail ... */
    {
        if ( (m_ptr->mflag2 & MFLAG2_QUESTOR)
          || (r_ptr->flags7 & RF7_GUARDIAN) )
        {
            if (one_in_(5))
                mo_mode |= AM_TAILORED;
        }
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            if (one_in_(10) || m_ptr->r_idx == MON_NAMI)
                mo_mode |= AM_TAILORED;
        }
        else if (r_ptr->flags1 & (RF1_DROP_GOOD | RF1_DROP_GREAT))
        {
            if (one_in_(30))
                mo_mode |= AM_TAILORED;
        }
    }

    coin_type = force_coin;
    object_level = _mon_drop_lvl(MAX(base_level, dun_level), r_ptr->level);
    object_wipe(o_ptr);

    if (do_gold && (!do_item || (randint0(100) < 20)))
    {
        if (!make_gold(o_ptr, TRUE))
        {
            obj_drop_theme = 0;
            return FALSE;
        }
    }
    else
    {
        if (!make_object(o_ptr, mo_mode, ORIGIN_DROP))
        {
            obj_drop_theme = 0;
            return FALSE;
        }
        o_ptr->origin_xtra = m_ptr->r_idx;
    }

    object_level = base_level;
    coin_type = 0;
    obj_drop_theme = 0;

    return TRUE;
}

static bool _mon_is_wanted(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    if ((m_ptr->r_idx == MON_IMPLORINGTON) && (!no_wilderness)) return TRUE;
    if ((r_ptr->flags1 & RF1_UNIQUE) && !(m_ptr->smart & (1U << SM_CLONED)))
    {
        int i;
        for (i = 0; i < MAX_KUBI; i++)
        {
            if (kubi_r_idx[i] == m_ptr->r_idx && !(m_ptr->mflag2 & MFLAG2_CHAMELEON))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static bool _kind_is_basic(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];
    switch (k_ptr->tval)
    {
    case TV_FOOD:
        switch (k_ptr->sval)
        {
        case SV_FOOD_RATION:
        case SV_FOOD_WAYBREAD:
            return TRUE;
        }
        break;

    case TV_SCROLL:
        switch (k_ptr->sval)
        {
        case SV_SCROLL_TELEPORT:
        case SV_SCROLL_WORD_OF_RECALL:
            return TRUE;
        }
        break;

    case TV_FLASK:
        switch (k_ptr->sval)
        {
        case SV_FLASK_OIL:
            return (dun_level < 15) ? TRUE : FALSE;
        }
        break;

    case TV_LITE:
        switch (k_ptr->sval)
        {
        case SV_LITE_TORCH:
        case SV_LITE_LANTERN:
            return (dun_level < 15) ? TRUE : FALSE;
        }
        break;

    case TV_DIGGING:
        switch (k_ptr->sval)
        {
        case SV_SHOVEL:
        case SV_PICK:
            return (dun_level < 15) ? TRUE : FALSE;
        }
        break;
    }
    return FALSE;
}

static bool _kind_is_stat_potion(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];
    switch (k_ptr->tval)
    {
    case TV_POTION:
        switch (k_ptr->sval)
        {
        case SV_POTION_INC_STR:
        case SV_POTION_INC_INT:
        case SV_POTION_INC_WIS:
        case SV_POTION_INC_DEX:
        case SV_POTION_INC_CON:
        case SV_POTION_INC_CHR:
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static bool _kind_is_utility(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];
    switch (k_ptr->tval)
    {
    case TV_SCROLL:
        switch (k_ptr->sval)
        {
        case SV_SCROLL_TELEPORT:
        case SV_SCROLL_PHASE_DOOR:
        case SV_SCROLL_WORD_OF_RECALL:
        case SV_SCROLL_REMOVE_CURSE:
        case SV_SCROLL_STAR_REMOVE_CURSE:
        case SV_SCROLL_MAPPING:
        /* case SV_SCROLL_PROTECTION_FROM_EVIL: XXX This was a bad idea! */
        case SV_SCROLL_DETECT_MONSTERS:
            return TRUE;
        case SV_SCROLL_IDENTIFY:
        case SV_SCROLL_STAR_IDENTIFY:
			return easy_id ? FALSE : TRUE;
        }
        break;

    case TV_POTION:
        switch (k_ptr->sval)
        {
        case SV_POTION_SPEED:
        case SV_POTION_THERMAL:
        case SV_POTION_VIGOR:
        case SV_POTION_RESISTANCE:
        case SV_POTION_HEROISM:
        case SV_POTION_CURE_SERIOUS:
        case SV_POTION_CURE_CRITICAL:
        case SV_POTION_CURING:
        case SV_POTION_RESTORE_EXP:
        case SV_POTION_RES_STR:
        case SV_POTION_RES_INT:
        case SV_POTION_RES_WIS:
        case SV_POTION_RES_DEX:
        case SV_POTION_RES_CON:
        case SV_POTION_RES_CHR:
        case SV_POTION_CLARITY:
            return TRUE;
        }
        break;
    }
    return FALSE;
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 */
void monster_death(int m_idx, bool drop_item)
{
    int i, j, y, x;

    int number = 0;
    int attempt = 0;
    int dump_item = 0;
    int dump_gold = 0;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    bool visible = ((m_ptr->ml && !p_ptr->image) || (r_ptr->flags1 & RF1_UNIQUE));

    u32b mo_mode = 0L;

    bool cloned = (m_ptr->smart & (1U << SM_CLONED)) ? TRUE : FALSE;
    bool was_pet = (m_ptr->mflag2 & MFLAG2_WASPET) ? TRUE : FALSE;
    bool do_vampire_servant = FALSE;
    char m_name[MAX_NLEN];
    int corpse_chance = 3;

    object_type forge;
    object_type *q_ptr;

    bool drop_chosen_item = drop_item && !cloned && !p_ptr->inside_arena
        && !p_ptr->inside_battle && !is_pet(m_ptr) && !was_pet;


    monster_desc(m_name, m_ptr, MD_TRUE_NAME);

    /* The caster is dead? */
    if (world_monster && world_monster == m_idx) world_monster = 0;

    /* Notice changes in view */
    if (r_ptr->flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
    {
        p_ptr->update |= (PU_MON_LITE);
    }

    if (mut_present(MUT_INFERNAL_DEAL) && los(py, px, m_ptr->fy, m_ptr->fx) && !is_pet(m_ptr))
    {
        if ( p_ptr->msp > 0
          && p_ptr->pclass != CLASS_RUNE_KNIGHT
          && p_ptr->pclass != CLASS_SAMURAI
          && p_ptr->pclass != CLASS_MYSTIC )
        {
            hp_player_aux(r_ptr->level * 4 / 9);
            sp_player(r_ptr->level * 2 / 9);
        }
        else
            hp_player_aux(r_ptr->level * 2 / 3);
    }

    if (r_ptr->flags2 & RF2_MULTIPLY)
        num_repro_kill++;

    y = m_ptr->fy;
    x = m_ptr->fx;

    /* Let monsters explode! */
    for (i = 0; i < 4; i++)
    {
        if (r_ptr->blows[i].method == RBM_EXPLODE)
        {
            for (j = 0; ((j < MAX_MON_BLOW_EFFECTS) && (r_ptr->blows[i].effects[j].effect)); j++)
            {
                int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
                int typ = r_ptr->blows[i].effects[j].effect;
                int d_dice = r_ptr->blows[i].effects[j].dd;
                int d_side = r_ptr->blows[i].effects[j].ds;
                int damage = damroll(d_dice, d_side);

                project(m_idx, 3, y, x, damage, typ, flg);
            }
            break;
        }
    }

    if (m_ptr->mflag2 & MFLAG2_CHAMELEON)
    {
        choose_new_monster(m_idx, TRUE, MON_CHAMELEON);
        r_ptr = &r_info[m_ptr->r_idx];
    }

    quests_on_kill_mon(m_ptr);

    /* Handle the possibility of player vanquishing arena combatant -KMW- */
    if (p_ptr->inside_arena && !is_pet(m_ptr))
    {
        p_ptr->exit_bldg = TRUE;

        if (p_ptr->arena_number > MAX_ARENA_MONS)
        {
            msg_print("You are a Genuine Champion!");
        }
        else
        {
            msg_print("Victorious! You're on your way to becoming Champion.");
            gain_fame(1);
        }

        if (arena_info[p_ptr->arena_number].tval)
        {
            int tval = arena_info[p_ptr->arena_number].tval;
            int sval = arena_info[p_ptr->arena_number].sval;

            q_ptr = &forge;
            switch (tval)
            {
            case TV_WAND: case TV_ROD: case TV_STAFF:
                object_prep(q_ptr, lookup_kind(tval, SV_ANY));
                device_init_fixed(q_ptr, sval);
                break;
            default:
                object_prep(q_ptr, lookup_kind(tval, sval));
                apply_magic(q_ptr, object_level, AM_NO_FIXED_ART);
                obj_make_pile(q_ptr);
            }

            q_ptr->origin_type = ORIGIN_ARENA_REWARD;
            q_ptr->origin_xtra = m_ptr->r_idx;
            (void)drop_near(q_ptr, -1, y, x);
        }

        /* Moved to bldg.c do_cmd_bldg to handle the obscure case:
         * [1] Player defeats monster, but is bleeding profusely
         * [2] Player dies before reaching the exit.
         * In this scenario, the game reports the player as having
         * been defeated by the *next* arena foe, which they never
         * even faced.
        if (p_ptr->arena_number > MAX_ARENA_MONS) p_ptr->arena_number++;
        p_ptr->arena_number++;
        */

        if (p_ptr->prace == RACE_MON_RING && !p_ptr->riding)
        {
            /* Uh Oh. Rings can't move without mounts and nobody will come for them
               in the Arena. Let's boot them out! */
            if (p_ptr->arena_number > MAX_ARENA_MONS) p_ptr->arena_number++;
            p_ptr->arena_number++;

            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_NO_RETURN);
            p_ptr->inside_arena = FALSE;
            p_ptr->leaving = TRUE;

            /* Re-enter the arena */
            command_new = SPECIAL_KEY_BUILDING;

            /* No energy needed to re-enter the arena */
            energy_use = 0;
        }
    }

    if (m_idx == p_ptr->riding)
    {
        if (rakuba(-1, FALSE))
        {
            msg_print("You have fallen from your riding pet.");
        }
    }

    if ((p_ptr->prace == RACE_MON_MIMIC) && (!cloned))
        mimic_on_kill_monster(m_ptr->r_idx);

    if ( vampiric_drain_hack
      && (r_ptr->flags2 & RF2_HUMAN)
      && !is_pet(m_ptr)
      && randint1(p_ptr->lev) >= 15 )
    {
        do_vampire_servant = TRUE;
    }

    /* Drop a dead corpse? */
    if (p_ptr->prace == RACE_MON_POSSESSOR && p_ptr->current_r_idx == MON_POSSESSOR_SOUL)
        corpse_chance = 2;

    if (prace_is_(RACE_IGOR))
    {
        if (coffee_break == SPEED_INSTA_COFFEE)
        {
            if ((!(r_ptr->flags1 & RF1_UNIQUE)) || (m_ptr->smart & (1U << SM_SUMMONED)))
            {
                int ylim = (p_ptr->lv_kills + p_ptr->pet_lv_kills) - MAX(40, 15 + dun_level / 2);
                if (ylim > 0)
                {
                    corpse_chance += ylim;
                    if ((m_ptr->smart & (1U << SM_SUMMONED)) && (!(r_ptr->flags1 & RF1_UNIQUE))) corpse_chance *= 3;
                }
            }
            else if ((p_ptr->lv_kills + p_ptr->pet_lv_kills < 40) && (!(r_ptr->flags2 & RF2_MULTIPLY))) corpse_chance = 2;
        }
        if (r_ptr->flags2 & RF2_MULTIPLY)
        {
            corpse_chance = MAX(corpse_chance + 1, r_ptr->r_akills + 1);
        }
    }

    if ( (_mon_is_wanted(m_idx) || (one_in_(corpse_chance) && !do_vampire_servant))
      && (r_ptr->flags9 & (RF9_DROP_CORPSE | RF9_DROP_SKELETON))
      && !(p_ptr->inside_arena || p_ptr->inside_battle || cloned || (((m_ptr->r_idx == today_mon) || (prace_is_(RACE_IGOR))) && (is_pet(m_ptr)))))
    {
        /* Assume skeleton */
        bool corpse = FALSE;

        do_vampire_servant = FALSE;

        /*
         * We cannot drop a skeleton? Note, if we are in this check,
         * we *know* we can drop at least a corpse or a skeleton
         */
        if (!(r_ptr->flags9 & RF9_DROP_SKELETON))
            corpse = TRUE;
        else if ((r_ptr->flags9 & RF9_DROP_CORPSE) && _mon_is_wanted(m_idx))
            corpse = TRUE;
        else if ( (r_ptr->flags9 & RF9_DROP_CORPSE)
               && p_ptr->prace == RACE_MON_POSSESSOR
               && p_ptr->current_r_idx == MON_POSSESSOR_SOUL )
        {
            corpse = TRUE;
        }
        /* Else, a corpse is more likely unless we did a "lot" of damage */
        else if (r_ptr->flags9 & RF9_DROP_CORPSE)
        {
            /* Lots of damage in one blow */
            if ((0 - ((m_ptr->maxhp) / 4)) > m_ptr->hp)
            {
                if (one_in_(4)) corpse = TRUE;
            }
            else
            {
                if (!one_in_(5)) corpse = TRUE;
            }
        }

        /* Get local object */
        q_ptr = &forge;

        /* Prepare to make an object */
        object_prep(q_ptr, lookup_kind(TV_CORPSE, (corpse ? SV_CORPSE : SV_SKELETON)));

        apply_magic(q_ptr, object_level, AM_NO_FIXED_ART);

        q_ptr->pval = m_ptr->r_idx;
        if (r_ptr->weight && p_ptr->prace == RACE_MON_POSSESSOR)
        {
            /* Note: object_type.weight is an s16b and stores decipounds.
                     monster_race.weight is an s16b and stores pounds.
                     Thus, we might overflow on coversion! */
            if (corpse)
                q_ptr->weight = MIN(500*10, r_ptr->weight * 10);
            else
                q_ptr->weight = MIN(500*10, r_ptr->weight * 10 / 3);
        }

        /* Drop it in the dungeon */
        (void)drop_near(q_ptr, -1, y, x);
    }

    /* Drop objects being carried */
    monster_drop_carried_objects(m_ptr);

    if (r_ptr->flags1 & RF1_DROP_GOOD) mo_mode |= AM_GOOD;
    if (r_ptr->flags1 & RF1_DROP_GREAT) mo_mode |= AM_GREAT;

    switch (m_ptr->r_idx)
    {
    case MON_PINK_HORROR:
        /* Pink horrors are replaced with 2 Blue horrors */
        if (!(p_ptr->inside_arena || p_ptr->inside_battle))
        {
            bool notice = FALSE;

            for (i = 0; i < 2; i++)
            {
                int wy = y, wx = x;
                bool pet = is_pet(m_ptr);
                u32b mode = 0L;

                if (pet) mode |= PM_FORCE_PET;

                if (summon_specific((pet ? -1 : m_idx), wy, wx, 20, SUMMON_BLUE_HORROR, mode))
                {
                    if (player_can_see_bold(wy, wx))
                        notice = TRUE;
                }
            }

            if (notice)
                msg_print("The Pink horror divides!");
        }
        break;

    case MON_VARIANT_MAINTAINER:
    {
        bool notice = FALSE;

        for (i = 0; i < 4; i++)
        {
            int wy = y, wx = x;

            if (summon_specific(m_idx, wy, wx, 14, SUMMON_SOFTWARE_BUG, 0))
            {
                if (player_can_see_bold(wy, wx))
                    notice = TRUE;
            }
        }

        if (notice)
            msg_print("The Variant Maintainer is dead, but his crappy code remains!");
        break;
    }
    case MON_BLOODLETTER:
        /* Bloodletters of Khorne may drop a blade of chaos */
        if (drop_chosen_item && one_in_(20))
        {
            /* Get local object */
            q_ptr = &forge;

            /* Prepare to make a Blade of Chaos */
            object_prep(q_ptr, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));

            apply_magic(q_ptr, object_level, AM_NO_FIXED_ART | mo_mode);

            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = MON_BLOODLETTER;

            /* Drop it in the dungeon */
            (void)drop_near(q_ptr, -1, y, x);
        }
        break;

    case MON_OSIRIS:
        if (drop_chosen_item)
        {
            /* Get local object */
            q_ptr = &forge;

            object_prep(q_ptr, lookup_kind(TV_POTION, SV_POTION_NEW_LIFE));

            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = MON_OSIRIS;

            /* Drop it in the dungeon */
            (void)drop_near(q_ptr, -1, y, x);
        }
        break;

    case MON_AEGIR:
        if (drop_chosen_item)
        {
            /* Get local object */
            q_ptr = &forge;

            object_prep(q_ptr, lookup_kind(TV_POTION, SV_POTION_CONFUSION));

            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = MON_AEGIR;
            q_ptr->number = randint1(25);

            /* Drop it in the dungeon */
            (void)drop_near(q_ptr, -1, y, x);
        }
        break;

    case MON_RAAL:
        if (drop_chosen_item && (dun_level > 9))
        {
            /* Get local object */
            q_ptr = &forge;

            /* Wipe the object */
            object_wipe(q_ptr);

            /* Activate restriction */
            if ((dun_level > 49) && one_in_(5))
                get_obj_num_hook = _kind_is_good_book;
            else
                get_obj_num_hook = _kind_is_book;

            /* Make a book */
            if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
            {
                q_ptr->origin_xtra = MON_RAAL;
                (void)drop_near(q_ptr, -1, y, x);
            }
        }
        break;

    case MON_DAWN:
        /*
         * Mega^3-hack: killing a 'Warrior of the Dawn' is likely to
         * spawn another in the fallen one's place!
         */
        if (!p_ptr->inside_arena && !p_ptr->inside_battle)
        {
            if (!one_in_(5) && !equip_find_art(ART_SILVER_HAMMER))
            {
                int wy = y, wx = x;
                int attempts = 100;
                bool pet = is_pet(m_ptr);

                do
                {
                    scatter(&wy, &wx, y, x, 20, 0);
                }
                while (!(in_bounds(wy, wx) && cave_empty_bold2(wy, wx)) && --attempts);

                if (attempts > 0)
                {
                    u32b mode = 0L;
                    if (pet) mode |= PM_FORCE_PET;

                    if (summon_specific((pet ? -1 : m_idx), wy, wx, 40, SUMMON_DAWN, mode))
                    {
                        if (player_can_see_bold(wy, wx))
                            msg_print("A new warrior steps forth!");

                    }
                }
            }
        }
        break;

    case MON_UNMAKER:
        /* One more ultra-hack: An Unmaker goes out with a big bang! */
        {
            int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
            (void)project(m_idx, 6, y, x, 100, GF_CHAOS, flg);
        }
        break;

    case MON_UNICORN_ORD:
    case MON_MORGOTH:
    case MON_ONE_RING:
        /* Reward for "lazy" player */
        if (personality_is_(PERS_LAZY))
        {
            int a_idx = 0, yritys = 0;
            artifact_type *a_ptr = NULL;

            if (!drop_chosen_item) break;

            do
            {
                switch (randint0(3))
                {
                case 0:
                    a_idx = ART_NAMAKE_HAMMER;
                    break;
                case 1:
                    a_idx = ART_NAMAKE_BOW;
                    break;
                case 2:
                    a_idx = ART_NAMAKE_ARMOR;
                    break;
                }

                a_ptr = &a_info[a_idx];
                yritys++;
            }
            while ((yritys < 100) && (a_ptr->generated));

            if (yritys > 99) break; 

            /* Create the artifact */
            if (create_named_art(a_idx, y, x, ORIGIN_DROP, m_ptr->r_idx))
            {
                a_ptr->generated = TRUE;

                /* Hack -- Memorize location of artifact in saved floors */
                if (character_dungeon) a_ptr->floor_id = p_ptr->floor_id;
            }
        }
        break;

    case MON_SERPENT:
        if (!drop_chosen_item) break;
        if (create_named_art(ART_GROND, y, x, ORIGIN_DROP, MON_SERPENT))
            a_info[ART_GROND].generated = TRUE;
        if (create_named_art(ART_CHAOS, y, x, ORIGIN_DROP, MON_SERPENT))
            a_info[ART_CHAOS].generated = TRUE;
        break;

    case MON_B_DEATH_SWORD:
        if (drop_chosen_item)
        {
            /* Get local object */
            q_ptr = &forge;

            /* Prepare to make a broken sword */
            object_prep(q_ptr, lookup_kind(TV_SWORD, randint1(2)));

            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = MON_B_DEATH_SWORD;

            /* Drop it in the dungeon */
            (void)drop_near(q_ptr, -1, y, x);
        }
        break;

    case MON_A_GOLD:
    case MON_A_SILVER:
        if (drop_chosen_item && ((m_ptr->r_idx == MON_A_GOLD) ||
             ((m_ptr->r_idx == MON_A_SILVER) && (r_ptr->r_akills % 5 == 0))))
        {
            /* Get local object */
            q_ptr = &forge;

            /* Prepare to make a Can of Toys */
            object_prep(q_ptr, lookup_kind(TV_CHEST, SV_CHEST_KANDUME));

            apply_magic(q_ptr, object_level, AM_NO_FIXED_ART);

            object_origins(q_ptr, ORIGIN_NAGA);

            /* Drop it in the dungeon */
            (void)drop_near(q_ptr, -1, y, x);
        }
        break;

    case MON_ROLENTO:
        {
            int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
            (void)project(m_idx, 3, y, x, damroll(20, 10), GF_FIRE, flg);
        }
        break;

    case MON_ALBERICH:
    case MON_NAR:
    case MON_FUNDIN:
    {
        int odds = 150;
        if (r_ptr->level)
            odds /= r_ptr->level;
        if (one_in_(odds))
        {
            int         k_idx = lookup_kind(TV_RING, 0);
            q_ptr = &forge;

            object_prep(q_ptr, k_idx);

            apply_magic_ego = EGO_RING_DWARVES;
            apply_magic(q_ptr, object_level, AM_GOOD | AM_GREAT | AM_FORCE_EGO);

            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = m_ptr->r_idx;

            (void)drop_near(q_ptr, -1, y, x);
        }
        break;
    }
    case MON_NAZGUL:
    case MON_ANGMAR:
    case MON_KHAMUL:
    case MON_DWAR:
    case MON_HOARMURATH:
    {
        if (one_in_(3))
        {
            int         k_idx = lookup_kind(TV_RING, 0);
            q_ptr = &forge;

            object_prep(q_ptr, k_idx);

            apply_magic_ego = EGO_RING_NAZGUL;
            apply_magic(q_ptr, object_level, AM_GOOD | AM_GREAT | AM_FORCE_EGO);
            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = m_ptr->r_idx;

            (void)drop_near(q_ptr, -1, y, x);
        }
        else if (one_in_(6))
        {
            int         k_idx = lookup_kind(TV_CLOAK, SV_CLOAK);
            q_ptr = &forge;

            object_prep(q_ptr, k_idx);

            apply_magic_ego = EGO_CLOAK_NAZGUL;
            apply_magic(q_ptr, object_level, AM_GOOD | AM_GREAT | AM_FORCE_EGO);
            object_origins(q_ptr, ORIGIN_DROP);
            q_ptr->origin_xtra = m_ptr->r_idx;

            (void)drop_near(q_ptr, -1, y, x);
        }
        break;
    }
    case MON_SANTACLAUS:
    {
        int k_idx = lookup_kind(TV_HELM, SV_KNIT_CAP);
        q_ptr = &forge;

        object_prep(q_ptr, k_idx);

        apply_magic_ego = EGO_HELMET_TOMTE;
        apply_magic(q_ptr, object_level, AM_GOOD | AM_GREAT | AM_FORCE_EGO);

        object_origins(q_ptr, ORIGIN_DROP);
        q_ptr->origin_xtra = m_ptr->r_idx;

        (void)drop_near(q_ptr, -1, y, x);
        break;
    }
    default:
        if (!drop_chosen_item) break;

        switch (r_ptr->d_char)
        {
        case '(':
            if (dun_level > 0)
            {
                /* Get local object */
                q_ptr = &forge;

                /* Wipe the object */
                object_wipe(q_ptr);

                /* Activate restriction */
                get_obj_num_hook = _kind_is_cloak;

                /* Make a cloak */
                if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
                { 
                    q_ptr->origin_xtra = m_ptr->r_idx;
                    (void)drop_near(q_ptr, -1, y, x);
                }
            }
            break;

        case '/':
            if (dun_level > 4)
            {
                /* Get local object */
                q_ptr = &forge;

                /* Wipe the object */
                object_wipe(q_ptr);

                /* Activate restriction */
                get_obj_num_hook = _kind_is_polearm;

                /* Make a poleweapon */
                if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
                {
                    q_ptr->origin_xtra = m_ptr->r_idx;
                    (void)drop_near(q_ptr, -1, y, x);
                }
            }
            break;

        case '[':
            if (dun_level > 19)
            {
                /* Get local object */
                q_ptr = &forge;

                /* Wipe the object */
                object_wipe(q_ptr);

                /* Activate restriction */
                get_obj_num_hook = _kind_is_armor;

                /* Make a hard armor */
                if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
                {
                    q_ptr->origin_xtra = m_ptr->r_idx;
                    (void)drop_near(q_ptr, -1, y, x);
                }
            }
            break;

        case '\\':
            if (dun_level > 4)
            {
                /* Get local object */
                q_ptr = &forge;

                /* Wipe the object */
                object_wipe(q_ptr);

                /* Activate restriction */
                get_obj_num_hook = _kind_is_hafted;

                /* Make a hafted weapon */
                if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
                {
                    q_ptr->origin_xtra = m_ptr->r_idx;
                    (void)drop_near(q_ptr, -1, y, x);
                }
            }
            break;

        case '|':
            if (m_ptr->r_idx != MON_STORMBRINGER)
            {
                /* Get local object */
                q_ptr = &forge;

                /* Wipe the object */
                object_wipe(q_ptr);

                /* Activate restriction */
                get_obj_num_hook = _kind_is_sword;

                /* Make a sword */
                if (make_object(q_ptr, mo_mode, ORIGIN_DROP))
                {
                    q_ptr->origin_xtra = m_ptr->r_idx;
                    (void)drop_near(q_ptr, -1, y, x);
                }
            }
            break;
        }
        break;
    }

    if ((coffee_break == SPEED_INSTA_COFFEE) && (r_ptr->flags1 & RF1_UNIQUE) && (monster_pantheon(r_ptr)))
    {
        /* Get local object */
        q_ptr = &forge;

        object_prep(q_ptr, lookup_kind(TV_POTION, one_in_(7) ? SV_POTION_STAR_HEALING : SV_POTION_HEALING));

        object_origins(q_ptr, ORIGIN_DROP);
        q_ptr->origin_xtra = m_ptr->r_idx;

        if (q_ptr->number > 3) q_ptr->number = 3;

        /* Drop it in the dungeon */
        (void)drop_near(q_ptr, -1, y, x);
    }

    if (drop_chosen_item && (m_ptr->mflag2 & MFLAG2_DROP_MASK))
    {
        int k_idx;
        int mode = 0;
        q_ptr = &forge;

        if (m_ptr->mflag2 & MFLAG2_DROP_PRIZE)
        {
            if (dun_level >= 30 && dun_level <= 60  && one_in_(3))
                get_obj_num_hook = _kind_is_stat_potion;
            else
            {
                if (dun_level >= 20 && one_in_(3))
                    mode |= AM_GREAT;
                else
                    mode |= AM_GOOD;

                if (one_in_(3))
                    mode |= AM_TAILORED;
            }
        }
        else if (m_ptr->mflag2 & MFLAG2_DROP_UTILITY)
        {
            get_obj_num_hook = _kind_is_utility;
        }
        else if (m_ptr->mflag2 & MFLAG2_DROP_BASIC)
            get_obj_num_hook = _kind_is_basic;

        if (get_obj_num_hook) get_obj_num_prep();
        k_idx = get_obj_num(object_level);
        if (get_obj_num_hook)
        {
            get_obj_num_hook = NULL;
            get_obj_num_prep();
        }

        object_prep(q_ptr, k_idx);
        if (!apply_magic(q_ptr, object_level, mode) && object_is_device(q_ptr))
            apply_magic(q_ptr, object_level, 0);
        obj_make_pile(q_ptr);
        object_origins(q_ptr, ORIGIN_DROP);
        q_ptr->origin_xtra = m_ptr->r_idx;
        drop_near(q_ptr, -1, y, x);
    }

    /* Mega-Hack -- drop fixed items */
    if (drop_chosen_item)
    {
        int a_idx = 0;
        int chance = 0;
        race_t *race_ptr = get_race();

        switch (m_ptr->r_idx)
        {
        case MON_OBERON:
            if (one_in_(3))
            {
                a_idx = ART_JUDGE;
                chance = 33;
            }
            else
            {
                a_idx = ART_AMBER;
                chance = 50;
            }
            break;

        case MON_STORMBRINGER:
            a_idx = ART_STORMBRINGER;
            chance = 100;
            break;

        case MON_ECHIZEN:
            a_idx = ART_CRIMSON;
            chance = 50;
            break;

        case MON_GANDALF:
            a_idx = ART_INCANUS;
            chance = 20;
            break;

        case MON_OROCHI:
            a_idx = ART_KUSANAGI;
            chance = 25;
            break;

        case MON_DWORKIN:
            a_idx = ART_JUDGE;
            chance = 20;
            break;

        case MON_SMEAGOL:
            if (one_in_(666))
            {
                a_idx = ART_POWER;
                chance = 100;
            }
            break;

        case MON_SAURON:
            if (one_in_(10))
            {
                a_idx = ART_POWER;
                chance = 100;
            }
            else
            {
                a_idx = ART_AHO;
                chance = 100;
            }
            break;

        case MON_BRAND:
            /*if (!one_in_(3))
            {
                a_idx = ART_BRAND;
                chance = 25;
            }
            else*/
            {
                a_idx = ART_WEREWINDLE;
                chance = 33;
            }
            break;

        case MON_CORWIN:
            if (!one_in_(3))
            {
                a_idx = ART_GRAYSWANDIR;
                chance = 33;
            }
            else
            {
                a_idx = ART_CORWIN;
                chance = 33;
            }
            break;

        case MON_SARUMAN:
            a_idx = ART_ELENDIL;
            chance = 33;
            break;

        case MON_FIONA:
            a_idx = ART_FIONA;
            chance = 50;
            break;

        case MON_JULIAN:
            a_idx = ART_JULIAN;
            chance = 45;
            break;

        case MON_KLING:
            a_idx = ART_DESTINY;
            chance = 40;
            break;

        case MON_GOEMON:
            a_idx = ART_ZANTETSU;
            chance = 10;
            break;

        /*case MON_MASTER_TONBERRY:
            a_idx = ART_MASTER_TONBERRY;
            chance = 10;
            break;*/

        case MON_ZEUS:
            a_idx = ART_ZEUS;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_ZEUS))
                chance = 100;
            break;
        case MON_POSEIDON:
            a_idx = ART_POSEIDON;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_POSEIDON))
                chance = 100;
            break;
        case MON_HADES:
            a_idx = ART_HADES;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_HADES))
                chance = 100;
            break;
        case MON_ATHENA:
            a_idx = ART_ATHENA;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_ATHENA))
                chance = 100;
            break;
        case MON_ARES:
            a_idx = ART_ARES;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_ARES))
                chance = 100;
            break;
        case MON_HERMES:
            a_idx = ART_HERMES;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_HERMES))
                chance = 100;
            break;
        case MON_APOLLO:
            a_idx = ART_APOLLO;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_APOLLO))
                chance = 100;
            break;
        case MON_ARTEMIS:
            a_idx = ART_ARTEMIS;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_ARTEMIS))
                chance = 100;
            break;
        case MON_HEPHAESTUS:
            a_idx = ART_HEPHAESTUS;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_HEPHAESTUS))
                chance = 100;
            break;
        case MON_HERA:
            a_idx = ART_HERA;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_HERA))
                chance = 100;
            break;
        case MON_DEMETER:
            a_idx = ART_DEMETER;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_DEMETER))
                chance = 100;
            break;
        case MON_APHRODITE:
            a_idx = ART_APHRODITE;
            chance = OLYMPIAN_CHANCE;
            if (demigod_is_(DEMIGOD_APHRODITE))
                chance = 100;
            break;

        case MON_HAGEN:
            a_idx = ART_HAGEN;
            chance = 66;
            break;

        case MON_CAINE:
            a_idx = ART_CAINE;
            chance = 50;
            break;

        case MON_GHB:
             a_idx = ART_LEGENDARY_LOST_TREASURE;
             chance = 100;
             break;

        case MON_METATRON:
             a_idx = ART_DESTINY;
             chance = 100;
             break;

        case MON_BULLGATES:
            if (one_in_(3)) {
             a_idx = ART_MICRODOLLAR;
             chance = 100;
            }
            else {
             a_idx = ART_WINBLOWS;
             chance = 100;
            }
            break;

        case MON_MUG:
            a_idx = ART_SURVEILLANCE;
            chance = 100;
            break;

        case MON_JACK_LANTERN:
            a_idx = ART_JACK_LANTERN;
            chance = 10;
            break;

        case MON_LUNGORTHIN:
            a_idx = ART_CALRIS;
            chance = 50;
            break;

        case MON_JACK_SHADOWS:
            a_idx = ART_JACK;
            chance = 33;
            break;

        case MON_DIO:
            a_idx = ART_STONEMASK;
            chance = 20;
            break;

        case MON_BELD:
            a_idx = ART_SOULCRUSH;
            chance = 10;
            break;

        case MON_PIP:
            a_idx = ART_EXCALIBUR_J;
            chance = 50;
            break;

        case MON_SHUTEN:
            a_idx = ART_SHUTEN_DOJI;
            chance = 33;
            break;

        case MON_FUNDIN:
            a_idx = ART_FUNDIN;
            chance = 5;
            break;

        case MON_ROBIN_HOOD:
            a_idx = ART_ROBIN_HOOD;
            chance = 5;
            break;

        case MON_ARTHUR:
            if (one_in_(2))
            {
                a_idx = ART_EXCALIBUR;
                chance = 5;
            }
            else
            {
                a_idx = ART_EXCALIBUR_J;
                chance = 25;
            }
            break;

        case MON_GALAHAD:
            a_idx = ART_HOLY_GRAIL;
            chance = 10;
            break;

        case MON_FANG:
            a_idx = ART_FANG;
            if (prace_is_(RACE_MON_HOUND) || warlock_is_(WARLOCK_HOUNDS))
                chance = 50;
            else
                chance = 10;
            break;

        case MON_WOLF:
            a_idx = ART_WOLF;
            if (prace_is_(RACE_MON_HOUND) || warlock_is_(WARLOCK_HOUNDS))
                chance = 50;
            else
                chance = 10;
            break;

        case MON_GRIP:
            a_idx = ART_GRIP;
            if (prace_is_(RACE_MON_HOUND) || warlock_is_(WARLOCK_HOUNDS))
                chance = 50;
            else
                chance = 10;
            break;

        /* Monster boss rewards */
        case MON_VECNA:
            if (p_ptr->pclass == CLASS_NECROMANCER)
            {
                a_idx = ART_HAND_OF_VECNA;
                chance = 100;
            }
            else
            {
                a_idx = ART_VECNA;
                if (warlock_is_(WARLOCK_UNDEAD))
                    chance = 50;
                else
                    chance = 5;
            }
            break;
        case MON_UBBO_SATHLA:
            a_idx = ART_UBBO_SATHLA;
            chance = 5;
            break;
        case MON_UNGOLIANT:
            a_idx = ART_UNGOLIANT;
            if (warlock_is_(WARLOCK_SPIDERS))
                chance = 50;
            else
                chance = 5;
            break;
        case MON_GLAURUNG:
            a_idx = ART_GLAURUNG;
            if (warlock_is_(WARLOCK_DRAGONS))
                chance = 50;
            else
                chance = 5;
            break;
        case MON_RAPHAEL:
            a_idx = ART_LOHENGRIN;
            if (warlock_is_(WARLOCK_ANGELS))
                chance = 50;
            else
                chance = 5;
            break;
        case MON_KUNDRY:
            a_idx = ART_KUNDRY;
            chance = 12;
            break;
        case MON_AMUN:
            a_idx = ART_AMUN;
            chance = 100;
            break;
        case MON_AEGIR:
            a_idx = ART_AEGIR;
            chance = 25;
            break;
        case MON_AIJEM:
            a_idx = ART_BLACK_BELET;
            chance = 7;
            break;
        case MON_CARCHAROTH:
            a_idx = ART_CARCHAROTH;
            chance = 5;
            if (warlock_is_(WARLOCK_HOUNDS))
                chance = 50;
            break;
        case MON_SURTUR:
            a_idx = ART_TWILIGHT;
            if (warlock_is_(WARLOCK_GIANTS))
                chance = 50;
            else
                chance = 5;
            break;
        case MON_YMIR:
            a_idx = ART_YMIR;
            chance = 5;
            if (p_ptr->pclass == CLASS_MAULER || warlock_is_(WARLOCK_GIANTS))
                chance = 50;
            break;
        case MON_TYPHOEUS:
            a_idx = ART_TYPHOEUS;
            chance = 5;
            if (p_ptr->pclass == CLASS_MAULER || warlock_is_(WARLOCK_GIANTS))
                chance = 50;
            break;
        case MON_ATLAS:
            a_idx = ART_ATLAS;
            chance = 5;
            if (p_ptr->pclass == CLASS_MAULER || warlock_is_(WARLOCK_GIANTS))
                chance = 25;
            break;
        case MON_KRONOS:
            a_idx = ART_KRONOS;
            if (warlock_is_(WARLOCK_GIANTS))
                chance = 25;
            else
                chance = 5;
            break;
        case MON_OMARAX:
            a_idx = ART_OMARAX;
            chance = 5;
            break;
        case MON_GOTHMOG:
            a_idx = ART_GOTHMOG;
            chance = 5;
            break;
        case MON_LERNEAN_HYDRA:
            a_idx = ART_LERNEAN;
            chance = 5;
            break;
        case MON_OREMORJ:
            a_idx = ART_OREMORJ;
            chance = 5;
            break;
        case MON_MEPHISTOPHELES:
            if (demon_is_(DEMON_KHORNE) || p_ptr->pclass == CLASS_MAULER)
                a_idx = ART_KHORNE;
            else
                a_idx = ART_MEPHISTOPHELES;
            chance = 5;
            break;
        case MON_ULIK:
            a_idx = ART_ULIK;
            chance = 5;
            if (p_ptr->pclass == CLASS_MAULER)
                chance = 75;
            break;
        case MON_QUAKER:
            a_idx = ART_QUAKER;
            chance = 5;
            break;
        case MON_ARIEL:
            a_idx = ART_ARIEL;
            chance = 5;
            break;
        case MON_MOIRE:
            a_idx = ART_MOIRE;
            chance = 5;
            break;
        case MON_LOGE:
            a_idx = ART_LOGE;
            chance = 5;
            break;
        case MON_EMPEROR_QUYLTHULG:
            a_idx = ART_EMPEROR_QUYLTHULG;
            chance = 5;
            break;
        case MON_DESTROYER:
            a_idx = ART_DESTROYER;
            chance = 0; /* RACE_MON_GOLEM only! */
            break;
        case MON_VLAD:
            a_idx = ART_STONEMASK;
            chance = 0; /* This traditionally belongs to Dio Brando but Vlad is the Vampire boss! */
            break;
        case MON_ONE_RING:
            a_idx = ART_POWER;
            chance = 5;
            break;
        case MON_MULTIHUED_CENTIPEDE:
            a_idx = ART_MULTIHUED_CENTIPEDE;
            chance = 5;
            break;
        }

        /* I think the bug is Kill Amberite, get Blood Curse, entomb said Amberite,
           zeroing out the m_ptr while processing monster death, and continuing to call
           this routine after m_list[m_idx] has been corrupted. */

        if (race_ptr->boss_r_idx && race_ptr->boss_r_idx == m_ptr->r_idx)
        {
            msg_print("Congratulations! You have killed the boss of your race!");
            gain_fame(10);
            chance = 100;
            p_ptr->update |= PU_BONUS; /* Player is now a "Hero" (cf IS_HERO()) */
            p_ptr->redraw |= PR_STATUS;

            /* Centipedes can only take the final evolutionary step if the boss is dead */
            if (p_ptr->prace == RACE_MON_CENTIPEDE && p_ptr->lev >= 35)
                race_ptr->gain_level(p_ptr->lev);

            msg_add_tiny_screenshot(50, 24);
        }

        if ((chance > 0) && (chance < 100) && (mut_present(MUT_BAD_LUCK))) chance -= (chance / 4);

        if (a_idx > 0 && randint0(100) < chance)
        {
            artifact_type *a_ptr = &a_info[a_idx];

            if (!a_ptr->generated)
            {
                /* Create the artifact */
                if (create_named_art(a_idx, y, x, ORIGIN_DROP, m_ptr->r_idx))
                {
                    a_ptr->generated = TRUE;

                    /* Hack -- Memorize location of artifact in saved floors */
                    if (character_dungeon) a_ptr->floor_id = p_ptr->floor_id;
                }
                else if (!preserve_mode)
                    a_ptr->generated = TRUE;
            }
        }
    }

    /* Re-roll drop count (leprechaun nerf) */
    if ((r_ptr->r_akills > 40) || (r_ptr->flags1 & RF1_ONLY_GOLD) || (coffee_break == SPEED_INSTA_COFFEE)) m_ptr->drop_ct = MAX(m_ptr->stolen_ct, get_monster_drop_ct(m_ptr));

    /* Determine how much we can drop */
    number = m_ptr->drop_ct - m_ptr->stolen_ct;

    if (!drop_item && (r_ptr->d_char != '$')) number = 0;
    if (is_pet(m_ptr)) number = 0;

    /* Drop some objects */
    for (attempt = 0, j = 0; j < number && attempt < 1000; attempt++)
    {
        if (get_monster_drop(m_idx, &forge))
        {
            assert(forge.k_idx);
            if (forge.tval == TV_GOLD)
                dump_gold++;
            else
                dump_item++;

            drop_near(&forge, -1, y, x);
            j++;
        }
    }

    if ( r_ptr->level && !p_ptr->inside_arena && !p_ptr->inside_battle
      && ( (r_ptr->flags1 & (RF1_DROP_GOOD | RF1_DROP_GREAT))
        || (r_ptr->flags2 & RF2_THIEF) ) &&
        ((!(r_ptr->flags2 & RF2_MULTIPLY)) ||
        (r_ptr->r_akills < ((coffee_break || no_selling) ? 42 : 84))))
    {
        int r = (r_ptr->flags1 & RF1_DROP_GREAT) ? 7 : 3;
        int n = randint0(r);
        int i;

        object_level = (MAX(base_level, dun_level) + r_ptr->level) / 2;
        for (i = 0; i < n; i++)
        {
            object_type gold = {0};
            if (make_gold(&gold, TRUE))
                drop_near(&gold, -1, y, x);
        }
        object_level = base_level;
    }

    if (visible && (dump_item || dump_gold))
        lore_treasure(m_idx, dump_item, dump_gold);

    if (do_vampire_servant)
    {
        int r_idx = MON_VAMPIRE;
        int r_lvl = r_ptr->level;
        int mode = PM_FORCE_PET;

        if (!one_in_(3))
        {
            mode = PM_FORCE_FRIENDLY;
            cmsg_format(TERM_RED, "%^s is transformed in undeath!", m_name);
        }
        else
            cmsg_format(TERM_RED, "%^s rises to serve you!", m_name);

        if (r_ptr->flags1 & RF1_UNIQUE)
            r_lvl += 10;

        if (r_lvl >= 30)
            r_idx = MON_MASTER_VAMPIRE;
        if (r_lvl >= 40 && one_in_(2))
            r_idx = MON_VAMPIRE_LORD;
        if (r_lvl >= 60 && one_in_(3))
            r_idx = MON_ELDER_VAMPIRE;
        if (r_lvl >= 80)
            r_idx = MON_ELDER_VAMPIRE;

        summon_named_creature(0, y, x, r_idx, mode);
    }
}

/*
 * Modify the physical damage done to the monster.
 * (for example when it's invulnerable or shielded)
 *
 * ToDo: Accept a damage-type to calculate the modified damage from
 * things like fire, frost, lightning, poison, ... attacks.
 *
 * "type" is not yet used and should be 0.
 */
 /* I had to split this into 2 routines:
    [1] Player is damaging monster */
int mon_damage_mod(monster_type *m_ptr, int dam, bool is_psy_spear)
{
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];

    if ((r_ptr->flagsr & RFR_RES_ALL) && dam > 0)
    {
        /* Only the Metal Babble gets this
           Other magic immune monsters can be slain by melee and arrows */
        if (m_ptr->r_idx == MON_HAGURE || m_ptr->r_idx == MON_HAGURE2)
        {
            dam /= 100;
            if ((dam == 0) && one_in_(3)) dam = 1;
        }
    }

    if ((MON_INVULNER(m_ptr)) && (!p_ptr->ignore_invuln))
    {
        if (is_psy_spear)
        {
            if (mon_show_msg(m_ptr))
                msg_print("The barrier is penetrated!");
        }
        else if (!one_in_(PENETRATE_INVULNERABILITY))
        {
            return (0);
        }
    }

    /* Hack: Pact monsters have special resistance to all damage from the player
       I'm not sure if this is the correct spot for this code ...*/
    if ( p_ptr->pclass == CLASS_WARLOCK
      && warlock_is_pact_monster(r_ptr)
      && dam )
    {
        /* Let the player notice this for this monster race only the first time */
        if (!(r_ptr->r_flagsr & RFR_PACT_MONSTER))
        {
            msg_print("<color:v>You are less effective against monsters you have made a pact with.</color>");
            r_ptr->r_flagsr |= (RFR_PACT_MONSTER);
        }
        dam = dam/2;
    }

    return (dam);
}

/* [2] Another monster is damaging monster */
int mon_damage_mod_mon(monster_type *m_ptr, int dam, bool is_psy_spear)
{
    if ( (m_ptr->r_idx == MON_HAGURE || m_ptr->r_idx == MON_HAGURE2) && dam > 0)
    {
        dam /= 100;
        if ((dam == 0) && one_in_(3)) dam = 1;
    }

    if (MON_INVULNER(m_ptr))
    {
        if (is_psy_spear)
        {
            if (mon_show_msg(m_ptr))
                msg_print("The barrier is penetrated!");
        }
        else if (!one_in_(PENETRATE_INVULNERABILITY))
        {
            return (0);
        }
    }

    return (dam);
}

int divide_exp_by(int kills)
{
    if (coffee_break) kills *= 2;
    return MAX(2, MIN(1000, (kills - 37) / 21));
}

static void _adjust_kill_exp(s32b *new_exp, u32b *new_exp_frac, int kills)
{
    s64b_mul(new_exp, new_exp_frac, 0, 2);
    s64b_div(new_exp, new_exp_frac, 0, divide_exp_by(kills));
}

/*
 * Calculate experience point to be get
 *
 * Even the 64 bit operation is not big enough to avoid overflow
 * unless we carefully choose orders of multiplication and division.
 *
 * Get the coefficient first, and multiply (potentially huge) base
 * experience point of a monster later.
 */
static void get_exp_from_mon(int dam, monster_type *m_ptr, bool mon_dead)
{
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    s32b new_exp;
    u32b new_exp_frac;
    s32b div_h;
    u32b div_l;

    if (!m_ptr->r_idx) return;
    if (is_pet(m_ptr) || p_ptr->inside_battle || (m_ptr->mflag2 & MFLAG2_WASPET)) return;
    if ((coffee_break == SPEED_INSTA_COFFEE) && (m_ptr->mflag2 & MFLAG2_PLAYER_SUMMONED)) return;

    /*
     * - Ratio of monster's level to player's level effects
     * - Varying speed effects (Skipped for Breeders)
     * - Get a fraction in proportion of damage point
     */
    if (r_ptr->flags2 & RF2_MULTIPLY)
    {
        dam = dam * (r_ptr->hdice * (r_ptr->hside + 1) / 2) / m_ptr->maxhp;
        new_exp = r_ptr->level * SPEED_TO_ENERGY(r_ptr->speed) * dam;
    }
    else
        new_exp = r_ptr->level * SPEED_TO_ENERGY(m_ptr->mspeed) * dam;

    new_exp_frac = 0;
    div_h = 0L;
    div_l = (p_ptr->max_plv+2) * SPEED_TO_ENERGY(r_ptr->speed);

    /* Use (average maxhp * 2) as a denominator */
    if (!(r_ptr->flags1 & RF1_FORCE_MAXHP))
        s64b_mul(&div_h, &div_l, 0, r_ptr->hdice * (ironman_nightmare ? 2 : 1) * (r_ptr->hside + 1));
    else
        s64b_mul(&div_h, &div_l, 0, r_ptr->hdice * (ironman_nightmare ? 2 : 1) * r_ptr->hside * 2);

    /* Do division first to prevent overflow */
    s64b_div(&new_exp, &new_exp_frac, div_h, div_l);

    /* Limit exp gain after the 99th kill */
    if (r_ptr->r_akills > (coffee_break ? 49 : 99))
    {
        _adjust_kill_exp(&new_exp, &new_exp_frac, r_ptr->r_akills);
    }
    if ((r_ptr->flags2 & RF2_MULTIPLY) || (m_ptr->r_idx == MON_DAWN))
    {
        int biff = r_ptr->r_akills / 400;
        if (biff > 8) biff = 8;
        if (biff) s64b_RSHIFT(new_exp, new_exp_frac, biff);
    }

    /* Farming Summoners for xp is now biffed! For example, farming
       The Queen Ant for infinite Giant Fire Ants and a quick CL50. */
    if (m_ptr->parent_m_idx && !(r_ptr->flags1 & RF1_UNIQUE))
    {
        monster_type *pm_ptr = &m_list[m_ptr->parent_m_idx];

        if (pm_ptr->r_idx)
        {
            monster_race *pr_ptr = &r_info[pm_ptr->r_idx];
            int           biff;

            if (pr_ptr->flags1 & RF1_UNIQUE)
                biff = pr_ptr->r_skills / 100;
            else /* Draconic Q's? */
                biff = pr_ptr->r_skills / 250;

            if (biff > 8) biff = 8;
            if (biff) s64b_RSHIFT(new_exp, new_exp_frac, biff);
        }
    }

    /* Finally multiply base experience point of the monster */
    s64b_mul(&new_exp, &new_exp_frac, 0, r_ptr->mexp);

    /* Limit the amount of Exp gained from a single monster to remove
       player exploits that deliberately allow monsters to rest (= infinite exp) */
    {
        s32b n_h, d_h;
        u32b n_l, d_l;
        s32b pexp, mexp;

        /* Figure out the max experience to gain from this monster */
        if (r_ptr->flags2 & RF2_MULTIPLY)
            n_h = r_ptr->level * SPEED_TO_ENERGY(r_ptr->speed);
        else
            n_h = r_ptr->level * SPEED_TO_ENERGY(m_ptr->mspeed);
        n_l = 0;
        d_h = 0;
        d_l = (p_ptr->max_plv+2) * SPEED_TO_ENERGY(r_ptr->speed);

        s64b_div(&n_h, &n_l, d_h, d_l);
        s64b_mul(&n_h, &n_l, 0, r_ptr->mexp);

        mexp = n_h * 100;
        n_h = 0;
        s64b_mul(&n_h, &n_l, 0, 100);
        mexp += n_h;

        /* Figure out how much we are gaining */
        pexp = new_exp * 100;
        n_h = 0;
        n_l = new_exp_frac;
        s64b_mul(&n_h, &n_l, 0, 100);
        pexp += n_h;

        if (m_ptr->pexp + pexp > mexp)
        {
            pexp = MAX(0, mexp - m_ptr->pexp);
            new_exp = pexp / 100;
            n_h = pexp % 100;
            new_exp_frac = 0;
            s64b_div(&n_h, &new_exp_frac, 0, 100);
        }
        m_ptr->pexp += pexp;
#if 0
        msg_format(
            "Gain %d.%2.2d XP (Max %d.%2.2d, Mon %d.%2.2d, Actual %d %u)",
            pexp/100, pexp%100,
            mexp/100, mexp%100,
            m_ptr->pexp/100, m_ptr->pexp%100,
            new_exp, new_exp_frac
        );
#endif
    }

    if (mut_present(MUT_FAST_LEARNER))
    {
        s64b_mul(&new_exp, &new_exp_frac, 0, 6);
        s64b_div(&new_exp, &new_exp_frac, 0, 5);
    }

    if (quest_id_current() > 64) /* Players get too much XP from some early quests */
    {
        quest_ptr q_ptr = quests_get_current();
        if (q_ptr)
        {
             if ((strpos("Shadow Fair", q_ptr->name)) || (strpos("Tidy ", q_ptr->name)))
             {
                 s64b_mul(&new_exp, &new_exp_frac, 0, 2);
                 s64b_div(&new_exp, &new_exp_frac, 0, 3);
             }
        }
    }

    /* Intelligence affects learning! */
    s64b_mul(&new_exp, &new_exp_frac, 0, adj_exp_gain[p_ptr->stat_ind[A_INT]]);
    s64b_div(&new_exp, &new_exp_frac, 0, 100);

    /* Farming summons for XP is not good either... */
    if ((m_ptr->mflag2 & MFLAG2_PLAYER_SUMMONED) && (p_ptr->lev > 14) &&
        (((r_ptr->level < 55) && (!(r_ptr->flags1 & RF1_UNIQUE))) ||
         (m_ptr->r_idx == MON_CYBER)))
    {
        int exp_div = ironman_downward ? 220 : 110;
        if ((mon_dead) && (p_ptr->py_summon_kills < 200) && (!(r_ptr->flags2 & RF2_MULTIPLY)) &&
           (((long)(r_ptr->mexp * r_ptr->level / p_ptr->lev) > (p_ptr->max_exp / ((m_ptr->mflag2 & MFLAG2_DIRECT_PY_SUMMON) ? (1694L * (10 + r_ptr->level) / exp_div) : 1000))) || (one_in_(10))) &&
           ((m_ptr->mflag2 & MFLAG2_DIRECT_PY_SUMMON) || (ironman_downward) || (one_in_(2)))) p_ptr->py_summon_kills++;
        s64b_mul(&new_exp, &new_exp_frac, 0, 20);
        s64b_div(&new_exp, &new_exp_frac, 0, 20 + p_ptr->py_summon_kills);
        if (m_ptr->mflag2 & MFLAG2_DIRECT_PY_SUMMON)
        {
            s64b_mul(&new_exp, &new_exp_frac, 0, 10 + MIN(99, r_ptr->level));
            s64b_div(&new_exp, &new_exp_frac, 0, exp_div);
        }
    }

    if ((coffee_break) && (py_in_dungeon())) /* Accelerated EXP gain */
    {
        int coffee_mult = 2;
        if (p_ptr->lev < 50)
        {
//           coffee_mult = 3;
           if (!(r_ptr->flags2 & RF2_MULTIPLY)) coffee_mult = (((p_ptr->lev / 10) == 1) || ((p_ptr->lev >= 38))) ? 7 : 8;
           if (p_ptr->lev >= 42) coffee_mult = MIN(coffee_mult, 6);
           if (p_ptr->lev >= 47) coffee_mult = MIN(coffee_mult, 5);
        }
        s64b_mul(&new_exp, &new_exp_frac, 0, coffee_mult);
    }

    if ((coffee_break == SPEED_INSTA_COFFEE))
    {
        int exp_div = 40;
        int cap = 40;
        int mult = ((p_ptr->lev / 14) == 2) ? 5 : 4;
        int tapot = (p_ptr->lv_kills + p_ptr->pet_lv_kills);
        bool _is_summon = (m_ptr->smart & (1U << SM_SUMMONED)) ? TRUE : FALSE;
//        int mult = 4;
        if (!py_in_dungeon()) mult = 7;
        if (m_ptr->mflag2 & MFLAG2_NATIVE)
        {
            tapot = MIN(tapot, 60);
        }
        if ((dun_level >= 99) && (p_ptr->lev < 50)) mult = 6;
        else if ((mult < 7) && (dun_level > 5) && (tapot <= cap) && (quest_get_rnd_num(NULL) * 4 > p_ptr->lev - 10))
        { /* Player is really low-level for the current quest, help him along a bit */
            mult++;
        }
        s64b_mul(&new_exp, &new_exp_frac, 0, mult);
        s64b_div(&new_exp, &new_exp_frac, 0, (p_ptr->lev < 10) ? 4 : 3);
        if ((tapot > cap) && (!(r_ptr->flags1 & RF1_UNIQUE)))
        {
            exp_div += (tapot / 4);
            if (m_ptr->mflag2 & MFLAG2_SPAWN) exp_div += MAX(exp_div, 100);
            s64b_mul(&new_exp, &new_exp_frac, 0, 40);
            s64b_div(&new_exp, &new_exp_frac, 0, exp_div);
        }
        else if ((tapot < 40) && (p_ptr->lev < 44)) /* Speed up mage-type XP gain to limit desire to grind */
        {
            int _fake_hp = calc_xtra_hp_fake(33);
            exp_div = 38;
            mult = exp_div + 6 + ((200 - _fake_hp) / 5);
            if ((_is_summon) || ((!py_in_dungeon()) && (((p_ptr->lev + 10) / 20) != 1))) exp_div = 44;
            if (mult != exp_div)
            {
                s64b_mul(&new_exp, &new_exp_frac, 0, mult);
                s64b_div(&new_exp, &new_exp_frac, 0, exp_div);
            }
        }
        if ((tapot > MAX(cap, 19 + dun_level * 4 / 9)) && (_is_summon))
        {
            mult = (r_ptr->flags1 & RF1_UNIQUE) ? 30 : 10;
            exp_div = mult + tapot - (MAX(cap, dun_level * 2 / 3));
            s64b_mul(&new_exp, &new_exp_frac, 0, mult);
            s64b_div(&new_exp, &new_exp_frac, 0, exp_div);
        }
    }

    if ((new_exp > 0) && (mon_dead)) p_ptr->lv_kills++;

    /* Gain experience */
    gain_exp_64(new_exp, new_exp_frac);
}

/* Hack for Quylthulgs. Their pets are allowed to kill uniques! */
void mon_check_kill_unique(int m_idx)
{
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];

    if ((!(m_ptr->smart & (1U << SM_CLONED))) && (!p_ptr->inside_battle))
    {
        /* When the player kills a Unique, it stays dead */
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            r_ptr->max_num = 0;

            if (one_in_(3) || r_ptr->level >= 80)
            {
                gain_fame(1);
                if (r_ptr->level >= 90)
                    gain_fame(1);
            }

            /* Mega-Hack -- Banor & Lupart */
            if ((m_ptr->r_idx == MON_BANOR) || (m_ptr->r_idx == MON_LUPART))
            {
                r_info[MON_BANORLUPART].max_num = 0;
                r_info[MON_BANORLUPART].r_pkills++;
                r_info[MON_BANORLUPART].r_akills++;
                if (r_info[MON_BANORLUPART].r_tkills < MAX_SHORT) r_info[MON_BANORLUPART].r_tkills++;
            }
            else if (m_ptr->r_idx == MON_BANORLUPART)
            {
                r_info[MON_BANOR].max_num = 0;
                r_info[MON_BANOR].r_pkills++;
                r_info[MON_BANOR].r_akills++;
                if (r_info[MON_BANOR].r_tkills < MAX_SHORT) r_info[MON_BANOR].r_tkills++;
                r_info[MON_LUPART].max_num = 0;
                r_info[MON_LUPART].r_pkills++;
                r_info[MON_LUPART].r_akills++;
                if (r_info[MON_LUPART].r_tkills < MAX_SHORT) r_info[MON_LUPART].r_tkills++;
            }
        }

        /* When the player kills a Nazgul, it stays dead */
        else if (r_ptr->flags7 & RF7_NAZGUL) r_ptr->max_num--;
        else if (m_ptr->r_idx == MON_CAMELOT_KNIGHT)
        {
            if (r_ptr->max_num)
                r_ptr->max_num--;
        }
    }
}

/*
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages. Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Made name, sex, and capitalization generic -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * XXX XXX XXX Consider decreasing monster experience over time, say,
 * by using "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))"
 * instead of simply "(m_exp * m_lev) / (p_lev)", to make the first
 * monster worth more than subsequent monsters. This would also need
 * to induce changes in the monster recall code.
 */
bool mon_take_hit(int m_idx, int dam, int type, bool *fear, cptr note)
{
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];

    /* Innocent until proven otherwise */
    bool        innocent = TRUE, thief = FALSE;
    int         i;
    int         expdam;

    set_sanctuary(FALSE);

    /* Hack: Player mimic has revealed itself! */
    if (p_ptr->prace == RACE_MON_RING && !p_ptr->riding)
    {
        m_ptr->mflag2 |= MFLAG2_AWARE;
        for (i = 1; i < m_max; i++)
        {
            monster_type *m_ptr2 = &m_list[i];

            if (!m_ptr2->r_idx) continue;
            if (is_aware(m_ptr2)) continue;
            if (MON_CSLEEP(m_ptr2)) continue;

            if (!player_has_los_bold(m_ptr2->fy, m_ptr2->fx)) continue;
            /*if (!projectable(m_ptr2->fy, m_ptr2->fx, py, px)) continue;*/

            m_ptr2->mflag2 |= MFLAG2_AWARE;
        }
    }

    if ((melee_challenge) && (dam > 0) && (type != DAM_TYPE_MELEE) && (type != DAM_TYPE_WIZARD) && (type != DAM_TYPE_AURA))
    {
        dam = 0;
    }
    if ((no_melee_challenge) && (dam > 0) && (type == DAM_TYPE_MELEE)) dam = 0;

    if (!(r_ptr->flags7 & RF7_KILL_EXP))
    {
        expdam = (m_ptr->hp > dam) ? dam : m_ptr->hp;
        if (mon_race_has_healing(r_ptr)) expdam = (expdam+1) * 2 / 3;

        get_exp_from_mon(expdam, m_ptr, FALSE);

        /* Genocided by chaos patron */
        if (!m_ptr->r_idx) m_idx = 0;
    }

    /* Redraw (later) if needed */
    check_mon_health_redraw(m_idx);

    /* Wake it up */
    if (shoot_hack != SHOOT_TRANQUILIZE)
        (void)set_monster_csleep(m_idx, 0);

    /* Hack - Cancel any special player stealth magics. -LM- */
    if (p_ptr->special_defense & NINJA_S_STEALTH)
    {
        set_superstealth(FALSE);
    }

    /* Genocided by chaos patron */
    if (!m_idx) return TRUE;

    if (dam > 0 && (p_ptr->wizard || cheat_xtra || easy_damage))
        msg_format("You do %d damage.", dam);

    if ( p_ptr->melt_armor
      && note == NULL /* Hack: Trying to just get melee and shooting */
      && (-m_ptr->ac_adj) < r_ptr->ac/2
      && !mon_save_p(m_ptr->r_idx, A_NONE) )
    {
        char m_name[MAX_NLEN];
        monster_desc(m_name, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);
        msg_format("%^s armor melts.", m_name);
        m_ptr->ac_adj -= randint1(2);
        if (p_ptr->wizard || cheat_xtra || easy_damage)
            msg_format("Melt Armor: AC is now %d", mon_ac(m_ptr));
    }

    /* Rage Mage: "Blood Lust" */
    if (p_ptr->pclass == CLASS_RAGE_MAGE && dam > 0)
    {
        rage_mage_blood_lust(dam);
    }

    /* Hurt it */
    m_ptr->hp -= dam;
    if (m_ptr->hp >= 0)
        pack_on_damage_monster(m_idx);

    /* Mark monster as hurt */
    if (dam > 0) 
    {
        m_ptr->mflag2 |= MFLAG2_HURT;
        monsters_damaged_hack = TRUE;
    }

    /* It is dead now */
    if (m_ptr->hp < 0)
    {
        char         m_name[MAX_NLEN];
        monster_type exp_mon = *m_ptr; /* Copy since we will delete_monster_idx before granting experience */

        monster_desc(m_name, m_ptr, MD_TRUE_NAME);

        if (p_ptr->tim_killing_spree)
            set_fast(p_ptr->fast + 10, FALSE);

        if (r_info[m_ptr->r_idx].flags7 & RF7_TANUKI)
        {
            /* You might have unmasked Tanuki first time */
            r_ptr = &r_info[m_ptr->r_idx];
            m_ptr->ap_r_idx = m_ptr->r_idx;
            if (r_ptr->r_sights < MAX_SHORT) r_ptr->r_sights++;
        }

        if (m_ptr->mflag2 & MFLAG2_CHAMELEON)
        {
            /* You might have unmasked Chameleon first time */
            r_ptr = real_r_ptr(m_ptr);
            if (r_ptr->r_sights < MAX_SHORT) r_ptr->r_sights++;
        }

        if (!(m_ptr->smart & (1U << SM_CLONED)))
        {
            /* When the player kills a Unique, it stays dead */
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                if (m_ptr->r_idx == MON_PHOENIX && one_in_(3) && !equip_find_art(ART_SILVER_HAMMER) /* blame bostock */)
                {
                    m_ptr->hp = m_ptr->maxhp;
                    msg_print("The Phoenix rises again!");
                    return FALSE;
                }

                r_ptr->max_num = 0;

                if (one_in_(3) || r_ptr->level >= 80)
                {
                    gain_fame(1);
                    if (r_ptr->level >= 90)
                        gain_fame(1);
                }

                /* Mega-Hack -- Banor & Lupart */
                if ((m_ptr->r_idx == MON_BANOR) || (m_ptr->r_idx == MON_LUPART))
                {
                    r_info[MON_BANORLUPART].max_num = 0;
                    r_info[MON_BANORLUPART].r_pkills++;
                    r_info[MON_BANORLUPART].r_akills++;
                    if (r_info[MON_BANORLUPART].r_tkills < MAX_SHORT) r_info[MON_BANORLUPART].r_tkills++;
                }
                else if (m_ptr->r_idx == MON_BANORLUPART)
                {
                    r_info[MON_BANOR].max_num = 0;
                    r_info[MON_BANOR].r_pkills++;
                    r_info[MON_BANOR].r_akills++;
                    if (r_info[MON_BANOR].r_tkills < MAX_SHORT) r_info[MON_BANOR].r_tkills++;
                    r_info[MON_LUPART].max_num = 0;
                    r_info[MON_LUPART].r_pkills++;
                    r_info[MON_LUPART].r_akills++;
                    if (r_info[MON_LUPART].r_tkills < MAX_SHORT) r_info[MON_LUPART].r_tkills++;
                }
            }

            /* When the player kills a Nazgul, it stays dead */
            else if (r_ptr->flags7 & RF7_NAZGUL) r_ptr->max_num--;
            else if (m_ptr->r_idx == MON_CAMELOT_KNIGHT)
            {
                if (r_ptr->max_num)
                    r_ptr->max_num--;
            }
        }

        /* Handle Packs ... Check Morale */
        pack_on_slay_monster(m_idx);

        /* Count all monsters killed */
        if (r_ptr->r_akills < MAX_SHORT) r_ptr->r_akills++;

        /* Count all summons killed */
        if (m_ptr->parent_m_idx)
        {
            monster_type *pm_ptr = &m_list[m_ptr->parent_m_idx];
            if (pm_ptr->r_idx)
            {
                monster_race *pr_ptr = &r_info[pm_ptr->r_idx];
                if (pr_ptr->r_skills < MAX_SHORT)
                    pr_ptr->r_skills++;
            }
        }

        /* Recall even invisible uniques or winners (or statistics gathering runs :) */
        if ((m_ptr->ml && !p_ptr->image) || (r_ptr->flags1 & RF1_UNIQUE) || statistics_hack)
        {
            /* Count kills this life */
            if ((m_ptr->mflag2 & MFLAG2_KAGE) && (r_info[MON_KAGE].r_pkills < MAX_SHORT)) r_info[MON_KAGE].r_pkills++;
            else if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

            /* Count kills in all lives */
            if ((m_ptr->mflag2 & MFLAG2_KAGE) && (r_info[MON_KAGE].r_tkills < MAX_SHORT)) r_info[MON_KAGE].r_tkills++;
            else if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

            /* Hack -- Auto-recall */
            mon_track(m_ptr);
        }

        /* Don't kill Amberites */
        if ((r_ptr->flags3 & RF3_AMBERITE) && one_in_(2))
        {
            int curses = 1 + randint1(3);
            bool stop_ty = FALSE;
            int count = 0;

            cmsg_format(TERM_VIOLET, "%^s puts a terrible blood curse on you!", m_name);

            curse_equipment(100, 50);

            very_nice_summon_hack = TRUE;

            do
            {
                stop_ty = activate_ty_curse(stop_ty, &count);
            }
            while (--curses);
            very_nice_summon_hack = FALSE;
        }

        if (r_ptr->flags2 & RF2_CAN_SPEAK)
        {
            char line_got[1024];

            /* Dump a message */
            if (!get_rnd_line("mondeath.txt", m_ptr->r_idx, line_got))

                msg_format("%^s %s", m_name, line_got);

#ifdef WORLD_SCORE
            if (m_ptr->r_idx == MON_SERPENT)
            {
                /* Make screen dump */
                screen_dump = make_screen_dump();
            }
#endif
        }
        else if (m_ptr->r_idx == MON_R_MACHINE)
        {
             msg_format("%^s types, 'All you have done is seal the permanence of your fate!'", m_name);
        }

        if (!(d_info[dungeon_type].flags1 & DF1_BEGINNER))
        {
            if (r_ptr->level > dun_level)
            {
                if (randint1(10) <= (r_ptr->level - dun_level))
                    virtue_add(VIRTUE_VALOUR, 1);
            }
            if (r_ptr->level > 60)
            {
                virtue_add(VIRTUE_VALOUR, 1);
            }
            if (r_ptr->level >= 2 * (p_ptr->lev+1))
                virtue_add(VIRTUE_VALOUR, 2);
        }

        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            if (r_ptr->flags3 & (RF3_EVIL | RF3_GOOD)) virtue_add(VIRTUE_HARMONY, 2);

            if (r_ptr->flags3 & RF3_GOOD)
            {
                virtue_add(VIRTUE_UNLIFE, 2);
                virtue_add(VIRTUE_VITALITY, -2);
            }

            if (one_in_(3)) virtue_add(VIRTUE_INDIVIDUALISM, -1);
        }

        if (m_ptr->r_idx == MON_BEGGAR || m_ptr->r_idx == MON_LEPER)
        {
            virtue_add(VIRTUE_COMPASSION, -1);
        }

        if ((r_ptr->flags3 & RF3_GOOD) &&
            ((r_ptr->level) / 10 + (3 * dun_level) >= randint1(100)))
            virtue_add(VIRTUE_UNLIFE, 1);

        if (r_ptr->d_char == 'A')
        {
            if (r_ptr->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_FAITH, -2);
            else if ((r_ptr->level) / 10 + (3 * dun_level) >= randint1(100))
            {
                if (r_ptr->flags3 & RF3_GOOD) virtue_add(VIRTUE_FAITH, -1);
                else virtue_add(VIRTUE_FAITH, 1);
            }
        }
        else if (r_ptr->flags3 & RF3_DEMON)
        {
            if (r_ptr->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_FAITH, 2);
            else if ((r_ptr->level) / 10 + (3 * dun_level) >= randint1(100))
                virtue_add(VIRTUE_FAITH, 1);
        }

        if ((r_ptr->flags3 & RF3_UNDEAD) && (r_ptr->flags1 & RF1_UNIQUE))
            virtue_add(VIRTUE_VITALITY, 2);

        if (r_ptr->r_deaths)
        {
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                virtue_add(VIRTUE_HONOUR, 10);
            }
            else if ((r_ptr->level) / 10 + (2 * dun_level) >= randint1(100))
            {
                virtue_add(VIRTUE_HONOUR, 1);
            }
        }
        if ((r_ptr->flags2 & RF2_MULTIPLY) && (r_ptr->r_akills > 1000) && one_in_(10))
        {
            virtue_add(VIRTUE_VALOUR, -1);
        }

        for (i = 0; i < 4; i++)
        {
            if (r_ptr->blows[i].effects[0].dd != 0) innocent = FALSE; /* Murderer! */

            if ((r_ptr->blows[i].effects[0].effect == RBE_EAT_ITEM)
                || (r_ptr->blows[i].effects[0].effect == RBE_EAT_GOLD))

                thief = TRUE; /* Thief! */
        }

        /* The new law says it is illegal to live in the dungeon */
        if (r_ptr->level != 0) innocent = FALSE;

        if (thief)
        {
            if (r_ptr->flags1 & RF1_UNIQUE)
                virtue_add(VIRTUE_JUSTICE, 3);
            else if (((r_ptr->level) / 10 + (2 * dun_level))
                >= randint0(100))
                virtue_add(VIRTUE_JUSTICE, 1);
        }
        else if (innocent)
        {
            virtue_add (VIRTUE_JUSTICE, -1);
        }

        if ((r_ptr->flags3 & RF3_ANIMAL) && !(r_ptr->flags3 & RF3_EVIL) && !mon_race_is_magical(r_ptr))
        {
            if (one_in_(4)) virtue_add(VIRTUE_NATURE, -1);
        }

        /* Make a sound */
        sound(SOUND_KILL);

        /* Death by Missile/Spell attack */
        if (note)
        {
            msg_format("%^s%s", m_name, note);
        }

        /* Death by physical attack -- invisible monster */
        else if (!m_ptr->ml)
        {
            cmsg_format(TERM_L_RED, "You have killed %s.", m_name);
        }

        /* Death by Physical attack -- non-living monster */
        else if (!monster_living(r_ptr))
        {
            int i;
            bool explode = FALSE;

            for (i = 0; i < 4; i++)
            {
                if (r_ptr->blows[i].method == RBM_EXPLODE) explode = TRUE;
            }

            /* Special note at death */
            if (explode)
                cmsg_format(TERM_L_RED, "%^s explodes into tiny shreds.", m_name);
            else
                cmsg_format(TERM_L_RED, "You have destroyed %s.", m_name);
        }

        /* Death by Physical attack -- living monster */
        else
            cmsg_format(TERM_L_RED, "You have slain %s.", m_name);

        if (_mon_is_wanted(m_idx))
        {
            char m_posname[MAX_NLEN];
            monster_desc(m_posname, m_ptr, MD_TRUE_NAME | MD_POSSESSIVE);
            msg_format("There is a price on %s head.", m_posname);
            if (alert_wanted_kill) msg_print(NULL);
        }

        /* Generate treasure */
        monster_death(m_idx, TRUE);

        /* Guntujant takes interest... */
        if (disciple_is_(DISCIPLE_TROIKA))
        {
            if (r_ptr->flags1 & RF1_UNIQUE && (r_ptr->level + randint1(r_ptr->level) > p_ptr->lev * 2))
            {
                troika_effect(TROIKA_KILL_FAMOUS);
            }
            else if (r_ptr->flags1 & RF1_UNIQUE)
            {
                troika_effect(TROIKA_KILL_UNIQUE);
            }
            else if (r_ptr->flags3 & RF3_DEMON)
            {
                troika_effect(TROIKA_KILL_DEMON);
            }
            else if (r_ptr->flags3 & RF3_GOOD)
            {
                troika_effect(TROIKA_KILL_GOOD);
            }
            else if (r_ptr->level < (p_ptr->lev - 15))
            {
                troika_effect(TROIKA_KILL_WEAK);
            }
            else
            {
                troika_effect(TROIKA_KILL);
            }
            troika_learn_spell(r_ptr);
        }

        /* Mega hack : replace IKETA to BIKETAL */
        if ((m_ptr->r_idx == MON_IKETA) &&
            !(p_ptr->inside_arena || p_ptr->inside_battle))
        {
            int dummy_y = m_ptr->fy;
            int dummy_x = m_ptr->fx;
            u32b mode = 0L;

            if (is_pet(m_ptr)) mode |= PM_FORCE_PET;

            /* Delete the monster */
            delete_monster_idx(m_idx);

            if (summon_named_creature(0, dummy_y, dummy_x, MON_BIKETAL, mode))
            {
                msg_print("Uwa-hahaha!  *I* am Biketal!");
            }
        }
        else
        {
            /* Delete the monster */
            delete_monster_idx(m_idx);
        }

        /* Prevent bug of chaos patron's reward */
        if (r_ptr->flags7 & RF7_KILL_EXP)
            get_exp_from_mon((long)exp_mon.max_maxhp*2, &exp_mon, TRUE);
        else
            get_exp_from_mon(((long)exp_mon.max_maxhp+1L) * 9L / 10L, &exp_mon, TRUE);

        /* Not afraid */
        (*fear) = FALSE;

        /* Monster is dead */
        return (TRUE);
    }

    /* Mega-Hack -- Pain cancels fear */
    if (MON_MONFEAR(m_ptr) && (dam > 0))
    {
        if (set_monster_monfear(m_idx, MON_MONFEAR(m_ptr) - randint1(dam)))
            (*fear) = FALSE;
    }

    if (fear_p_hurt_m(m_idx, dam))
        (*fear) = TRUE;

    /* Not dead yet */
    return (FALSE);
}

/*
 * Map resizing whenever the main term changes size
 */
void resize_map(void)
{
    /* Only if the dungeon exists */
    if (!character_dungeon) return;

    viewport_verify();
    msg_line_clear();

    /* Update stuff */
    p_ptr->update |= (PU_TORCH | PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Forget lite/view */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update lite/view */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE);

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw everything */
    p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);

    /* Hack -- update */
    handle_stuff();

    /* Redraw */
    Term_redraw();

    /*
     * Waiting command;
     * Place the cursor on the player
     */
    if (can_save) move_cursor_relative(py, px);

    msg_line_init(ui_msg_rect());

    /* Refresh */
    Term_fresh();
}

/*
 * Redraw a term when it is resized
 */
void redraw_window(void)
{
    /* Only if the dungeon exists */
    if (!character_dungeon) return;

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL);

    /* Window stuff */
    p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_DUNGEON | PW_MONSTER | PW_OBJECT);

    /* Hack -- update */
    handle_stuff();

    /* Redraw */
    Term_redraw();
}


point_t ui_pt_to_cave_pt(point_t pt)
{
    rect_t  r = ui_map_rect();
    point_t v = point_subtract(pt, rect_topleft(r));
    return point_add(viewport_origin, v);
}

point_t ui_xy_to_cave_pt(int x, int y)
{
    return ui_pt_to_cave_pt(point(x, y));
}

point_t cave_pt_to_ui_pt(point_t pt)
{
    rect_t  r = ui_map_rect();
    point_t v = point_subtract(pt, viewport_origin);
    return point_add(rect_topleft(r), v);
}

point_t cave_xy_to_ui_pt(int x, int y)
{
    return cave_pt_to_ui_pt(point(x, y));
}

bool cave_pt_is_visible(point_t pt)
{
    point_t ui = cave_pt_to_ui_pt(pt);
    rect_t  r = ui_map_rect();
    return rect_contains_pt(r, ui.x, ui.y);
}

bool cave_xy_is_visible(int x, int y)
{
    return in_bounds2(y, x) /* This is for legacy code ... */
        && cave_pt_is_visible(point(x, y));
}

bool ui_pt_is_visible(point_t pt)
{
    rect_t r = ui_map_rect();
    return rect_contains_pt(r, pt.x, pt.y);
}

bool ui_xy_is_visible(int x, int y)
{
    rect_t r = ui_map_rect();
    return rect_contains_pt(r, x, y);
}

rect_t ui_map_rect(void)
{
    return rect(
        0,
        1,
        Term->wid - 12 - 1,
        Term->hgt - 1 - 1
    );
}

rect_t ui_menu_rect(void) { return ui_map_rect(); }
rect_t ui_doc_menu_rect(void)
{
    return rect(
        0,
        0,
        Term->wid - 12 - 1,
        Term->hgt
    );
}

rect_t ui_shop_msg_rect(void)
{
    return rect(0, 0, 80, 3);
}

rect_t ui_msg_rect(void)
{
    return rect(0, 0, MIN(72, Term->wid - 13), 10);
}

rect_t ui_shop_rect(void)
{
    return rect(
        0,
        3,
        Term->wid,
        Term->hgt - 3
    );
}

rect_t ui_screen_rect(void)
{
    return rect(0, 0, Term->wid, Term->hgt);
}

rect_t ui_char_info_rect(void)
{
    return rect(
        Term->wid - 12,
        1,
        12,
        Term->hgt - 1
    );
}

/*
 * Handle a request to change the current viewport
 *
 * Return TRUE if the panel was changed.
 *
 * Also used in do_cmd_locate
 */
bool viewport_scroll(int dy, int dx)
{
    int y, x;
    rect_t r = ui_map_rect();

    /* Apply the motion */
    y = viewport_origin.y + dy * r.cy / 2;
    x = viewport_origin.x + dx * r.cx / 2;

    /* Verify the row */
    if (y > cur_hgt - r.cy) y = cur_hgt - r.cy;
    if (y < 0) y = 0;

    /* Verify the col */
    if (x > cur_wid - r.cx) x = cur_wid - r.cx;
    if (x < 0) x = 0;

    /* Handle "changes" */
    if ((y != viewport_origin.y) || (x != viewport_origin.x))
    {
        viewport_origin.y = y;
        viewport_origin.x = x;

        p_ptr->update |= (PU_MONSTERS); /* XXX Why? */
        p_ptr->redraw |= (PR_MAP);
        redraw_hack = TRUE;
        handle_stuff();
        redraw_hack = FALSE;
        return TRUE;
    }

    /* No change */
    return FALSE;
}

/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary.
 */
void viewport_verify_aux(u32b options)
{
    point_t p = cave_xy_to_ui_pt(px, py);
    rect_t  r = ui_map_rect();
    point_t o = viewport_origin;

    if ((options & VIEWPORT_FORCE_CENTER) || !rect_contains_pt(r, p.x, p.y))
    {
        point_t c = rect_center(r);
        point_t d = point_subtract(p, c);
        o = point_add(o, d);
    }
    else
    {
        if (p.y > r.y + r.cy - 2)
            o.y += r.cy/2;
        else if (p.y < r.y + 2)
            o.y -= r.cy/2;
        if (p.x > r.x + r.cx - 4)
            o.x += r.cx/2;
        else if (p.x < r.x + 4)
            o.x -= r.cx/2;
    }
    if (o.x > cur_wid - 3*r.cx/4) o.x = cur_wid - 3*r.cx/4;
    if (o.y > cur_hgt - 3*r.cy/4) o.y = cur_hgt - 3*r.cy/4;
    if (o.x < -r.cx/4) o.x = -r.cx/4;
    if (o.y < -r.cy/4) o.y = -r.cy/4;
    if (point_compare(viewport_origin, o) != 0)
    {
        viewport_origin = o;
        if (disturb_panel && !center_player) disturb(0, 0);
        p_ptr->update |= PU_MONSTERS;
        p_ptr->redraw |= PR_MAP;
        p_ptr->window |= PW_OVERHEAD | PW_DUNGEON;
    }
}

void viewport_verify(void)
{
    int options = 0;
    if (center_player && (center_running || (!running && !travel.run)))
        options |= VIEWPORT_FORCE_CENTER;
    viewport_verify_aux(options);
}


cptr mon_health_desc(monster_type *m_ptr)
{
    monster_race *ap_r_ptr = mon_apparent_race(m_ptr);
    bool          living = monster_living(ap_r_ptr);
    int           perc = 100 * m_ptr->hp / m_ptr->maxhp;

    if (m_ptr->hp >= m_ptr->maxhp)
        return living ? "unhurt" : "undamaged";
    else if (perc >= 60)
        return living ? "somewhat wounded" : "somewhat damaged";
    else if (perc >= 25)
        return living ? "wounded" : "damaged";
    else if (perc >= 10)
        return living ? "badly wounded" : "badly damaged";

    return living ? "almost dead" : "almost destroyed";
}

cptr mon_allegiance_desc(monster_type *m_ptr)
{
    if (is_pet(m_ptr))
        return "pet";
    else if (is_friendly(m_ptr))
        return "friendly";
    return ""; /* Hostile is to be assumed! */
}

/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(vptr u, vptr v, int p, int q)
{
    int z, a, b;

    /* Done sort */
    if (p >= q) return;

    /* Pivot */
    z = p;

    /* Begin */
    a = p;
    b = q;

    /* Partition */
    while (TRUE)
    {
        /* Slide i2 */
        while (!(*ang_sort_comp)(u, v, b, z)) b--;

        /* Slide i1 */
        while (!(*ang_sort_comp)(u, v, z, a)) a++;

        /* Done partition */
        if (a >= b) break;

        /* Swap */
        (*ang_sort_swap)(u, v, a, b);

        /* Advance */
        a++, b--;
    }

    /* Recurse left side */
    ang_sort_aux(u, v, p, b);

    /* Recurse right side */
    ang_sort_aux(u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(vptr u, vptr v, int n)
{
    /* Sort the array */
    ang_sort_aux(u, v, 0, n-1);
}



/*** Targeting Code ***/


/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targeting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating. This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
bool target_able(int m_idx) { return target_able_aux(m_idx, TARGET_KILL); }
bool target_able_aux(int m_idx, int mode)
{
    monster_type *m_ptr = &m_list[m_idx];

    /* Monster must be alive */
    if (!m_ptr->r_idx) return (FALSE);

    /* Hack -- no targeting hallucinations */
    if (p_ptr->image) return (FALSE);

    /* Monster must be visible */
    if (!m_ptr->ml) return (FALSE);

    if (p_ptr->riding && (p_ptr->riding == m_idx)) return (TRUE);

    /* Monster must be projectable */
    if (mode != TARGET_DISI && !projectable(py, px, m_ptr->fy, m_ptr->fx)) return (FALSE);

    /* XXX XXX XXX Hack -- Never target trappers */
    /* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

    /* Assume okay */
    return (TRUE);
}

/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool old_target_okay_mode(int mode)
{
    if (!use_old_target) return FALSE;
    if (p_ptr->confused) return FALSE;
    if (old_target_never_okay) return FALSE;
    if (!target_okay_aux(mode)) return FALSE;
    return TRUE;
}
void target_grab(int y, int x)
{
    target_row = y;
    target_col = x;
    old_target_never_okay = FALSE;
    if ((y == py) && (x == px) && (p_ptr->riding)) old_target_never_okay = TRUE;
}
bool old_target_okay(void) { return old_target_okay_mode(TARGET_KILL); }
bool target_okay(void) { return target_okay_aux(TARGET_KILL); }
bool target_okay_aux(int mode)
{
    /* Accept stationary targets ... but cf move_player_effect
     * in cmd1.c. We will dismiss a non-projectable positional
     * target the next time the player moves. */
    if (target_who < 0) return TRUE;

    /* Check moving targets */
    if (target_who > 0)
    {        
        /* Accept reasonable targets */
        if (target_able_aux(target_who, mode))
        {
            monster_type *m_ptr = &m_list[target_who];

            target_grab(m_ptr->fy, m_ptr->fx);

            /* Good target */
            return (TRUE);
        }
    }

    /* Assume no target */
    return (FALSE);
}


/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
static bool ang_sort_comp_distance(vptr u, vptr v, int a, int b)
{
    s16b *x = (s16b*)(u);
    s16b *y = (s16b*)(v);

    int da, db, kx, ky;

    /* Absolute distance components */
    kx = x[a]; kx -= px; kx = ABS(kx);
    ky = y[a]; ky -= py; ky = ABS(ky);

    /* Approximate Double Distance to the first point */
    da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

    /* Absolute distance components */
    kx = x[b]; kx -= px; kx = ABS(kx);
    ky = y[b]; ky -= py; ky = ABS(ky);

    /* Approximate Double Distance to the first point */
    db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

    /* Compare the distances */
    return (da <= db);
}

/*
 * Sorting hook -- comp function -- by importance level of grids
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by level of monster
 */
static bool ang_sort_comp_importance(vptr u, vptr v, int a, int b)
{
    s16b *x = (s16b*)(u);
    s16b *y = (s16b*)(v);
    cave_type *ca_ptr = &cave[y[a]][x[a]];
    cave_type *cb_ptr = &cave[y[b]][x[b]];
    monster_type *ma_ptr = &m_list[ca_ptr->m_idx];
    monster_type *mb_ptr = &m_list[cb_ptr->m_idx];
    monster_race *ap_ra_ptr, *ap_rb_ptr;

    /* The player grid */
    if (y[a] == py && x[a] == px) return TRUE;
    if (y[b] == py && x[b] == px) return FALSE;

    /* Extract monster race */
    if (ca_ptr->m_idx && ma_ptr->ml) ap_ra_ptr = &r_info[ma_ptr->ap_r_idx];
    else ap_ra_ptr = NULL;
    if (cb_ptr->m_idx && mb_ptr->ml) ap_rb_ptr = &r_info[mb_ptr->ap_r_idx];
    else ap_rb_ptr = NULL;

    if (ap_ra_ptr && !ap_rb_ptr) return TRUE;
    if (!ap_ra_ptr && ap_rb_ptr) return FALSE;

    /* Compare two monsters */
    if (ap_ra_ptr && ap_rb_ptr)
    {
        /* Unique monsters first */
        if ((ap_ra_ptr->flags1 & RF1_UNIQUE) && !(ap_rb_ptr->flags1 & RF1_UNIQUE)) return TRUE;
        if (!(ap_ra_ptr->flags1 & RF1_UNIQUE) && (ap_rb_ptr->flags1 & RF1_UNIQUE)) return FALSE;

        /* Shadowers first */
        if ((ma_ptr->mflag2 & MFLAG2_KAGE) && !(mb_ptr->mflag2 & MFLAG2_KAGE)) return TRUE;
        if (!(ma_ptr->mflag2 & MFLAG2_KAGE) && (mb_ptr->mflag2 & MFLAG2_KAGE)) return FALSE;

         /* Unknown monsters first */
        if (!ap_ra_ptr->r_tkills && ap_rb_ptr->r_tkills) return TRUE;
        if (ap_ra_ptr->r_tkills && !ap_rb_ptr->r_tkills) return FALSE;

        /* Higher level monsters first (if known) */
        if (ap_ra_ptr->r_tkills && ap_rb_ptr->r_tkills)
        {
            if (ap_ra_ptr->level > ap_rb_ptr->level) return TRUE;
            if (ap_ra_ptr->level < ap_rb_ptr->level) return FALSE;
        }

        /* Sort by index if all conditions are same */
        if (ma_ptr->ap_r_idx > mb_ptr->ap_r_idx) return TRUE;
        if (ma_ptr->ap_r_idx < mb_ptr->ap_r_idx) return FALSE;
    }

    /* An object get higher priority */
    if (cave[y[a]][x[a]].o_idx && !cave[y[b]][x[b]].o_idx) return TRUE;
    if (!cave[y[a]][x[a]].o_idx && cave[y[b]][x[b]].o_idx) return FALSE;

    /* Priority from the terrain */
    if (f_info[ca_ptr->feat].priority > f_info[cb_ptr->feat].priority) return TRUE;
    if (f_info[ca_ptr->feat].priority < f_info[cb_ptr->feat].priority) return FALSE;

    /* If all conditions are same, compare distance */
    return ang_sort_comp_distance(u, v, a, b);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static void ang_sort_swap_distance(vptr u, vptr v, int a, int b)
{
    s16b *x = (s16b*)(u);
    s16b *y = (s16b*)(v);

    s16b temp;

    /* Swap "x" */
    temp = x[a];
    x[a] = x[b];
    x[b] = temp;

    /* Swap "y" */
    temp = y[a];
    y[a] = y[b];
    y[b] = temp;
}



/*
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(int y1, int x1, int dy, int dx)
{
    int i, v;

    int x2, y2, x3, y3, x4, y4;

    int b_i = -1, b_v = 9999;


    /* Scan the locations */
    for (i = 0; i < temp_n; i++)
    {
        /* Point 2 */
        x2 = temp_x[i];
        y2 = temp_y[i];

        /* Directed distance */
        x3 = (x2 - x1);
        y3 = (y2 - y1);

        /* Verify quadrant */
        if (dx && (x3 * dx <= 0)) continue;
        if (dy && (y3 * dy <= 0)) continue;

        /* Absolute distance */
        x4 = ABS(x3);
        y4 = ABS(y3);

        /* Verify quadrant */
        if (dy && !dx && (x4 > y4)) continue;
        if (dx && !dy && (y4 > x4)) continue;

        /* Approximate Double Distance */
        v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

        /* XXX XXX XXX Penalize location */

        /* Track best */
        if ((b_i >= 0) && (v >= b_v)) continue;

        /* Track best */
        b_i = i; b_v = v;
    }

    /* Result */
    return (b_i);
}


/*
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_accept(int y, int x)
{
    cave_type *c_ptr;

    s16b this_o_idx, next_o_idx = 0;

    /* Bounds */
    if (!(in_bounds(y, x))) return (FALSE);

    /* Player grid is always interesting */
    if (player_bold(y, x)) return (TRUE);


    /* Handle hallucination */
    if (p_ptr->image) return (FALSE);


    /* Examine the grid */
    c_ptr = &cave[y][x];

    /* Visible monsters */
    if (c_ptr->m_idx)
    {
        monster_type *m_ptr = &m_list[c_ptr->m_idx];

        /* Visible monsters */
        if (m_ptr->ml) return (TRUE);
    }

    /* Scan all objects in the grid */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[this_o_idx];

        /* Acquire next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Memorized object */
        if (o_ptr->marked & OM_FOUND) return (TRUE);
    }

    /* Interesting memorized features */
    if (c_ptr->info & (CAVE_MARK))
    {
        /* Notice object features */
        if (c_ptr->info & CAVE_OBJECT) return (TRUE);

        /* Feature code (applying "mimic" field) */
        if (have_flag(f_info[get_feat_mimic(c_ptr)].flags, FF_NOTICE)) return TRUE;
    }

    /* Nope */
    return (FALSE);
}


/*
 * Prepare the "temp" array for "target_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_prepare(int mode)
{
    rect_t map_rect = ui_map_rect();
    point_t uip;

    /* Reset "temp" array */
    temp_n = 0;

    /* Scan the current panel */
    for (uip.y = map_rect.y; uip.y < map_rect.y + map_rect.cy; uip.y++)
    {
        for (uip.x = map_rect.x; uip.x < map_rect.x + map_rect.cx; uip.x++)
        {
            point_t cp = ui_pt_to_cave_pt(uip);
            cave_type *c_ptr;

            if (!target_set_accept(cp.y, cp.x)) continue;

            c_ptr = &cave[cp.y][cp.x];

            /* Require target_able monsters for "TARGET_KILL" */
            if ((mode & (TARGET_KILL)) && !target_able(c_ptr->m_idx)) continue;

            if ((mode & (TARGET_KILL | TARGET_MARK)) && !target_pet && is_pet(&m_list[c_ptr->m_idx])) continue;

            /* Duelist is attempting to mark a target ... only visible monsters, please! */
            if ( ((mode & TARGET_MARK) || (mode & TARGET_DISI))
              && (!c_ptr->m_idx || !m_list[c_ptr->m_idx].ml) )
            {
                continue;
            }

            /* Save the location */
            temp_x[temp_n] = cp.x;
            temp_y[temp_n] = cp.y;
            temp_n++;
        }
    }

    /* Set the sort hooks */
    if ((mode & TARGET_KILL) || (mode & TARGET_MARK) || (mode & TARGET_DISI) || (mode & TARGET_XTRA))
    {
        /* Target the nearest monster for shooting */
        ang_sort_comp = ang_sort_comp_distance;
        ang_sort_swap = ang_sort_swap_distance;
    }
    else
    {
        /* Look important grids first in Look command */
        ang_sort_comp = ang_sort_comp_importance;
        ang_sort_swap = ang_sort_swap_distance;
    }

    /* Sort the positions */
    ang_sort(temp_x, temp_y, temp_n);

    if (p_ptr->riding && target_pet && (temp_n > 1) && (mode & (TARGET_KILL)))
    {
        s16b tmp;

        tmp = temp_y[0];
        temp_y[0] = temp_y[1];
        temp_y[1] = tmp;
        tmp = temp_x[0];
        temp_x[0] = temp_x[1];
        temp_x[1] = tmp;
    }
}


bool show_gold_on_floor = FALSE;

/*
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately. This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text. This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * Eventually, we may allow multiple objects per grid, or objects
 * and terrain features in the same grid. XXX XXX XXX
 *
 * This function must handle blindness/hallucination.
 */
static int target_set_aux(int y, int x, int mode, cptr info)
{
    cave_type *c_ptr = &cave[y][x];
    s16b this_o_idx, next_o_idx = 0;
    cptr s1 = "", s2 = "", s3 = "", x_info = "";
    bool boring = TRUE;
    s16b feat;
    feature_type *f_ptr;
    int query = '\001';
    char out_val[MAX_NLEN+80];
    inv_ptr inv = NULL;
    int obj_ct = 0;

    /* Hack -- under the player */
    if (player_bold(y, x))
    {
        /* Description */
        s1 = "You are ";

        /* Preposition */
        s2 = "on ";
    }
    else
    {
        s1 = "Target: ";
        /* Hack: I get confused about whether I am selecting a monster,
           or the position that the monster happens to currently occupy.
           This happens after a fat finger incident or a failed <dir>
           navigation that really should have worked.*/
        if (strstr(info, "o,"))
            s1 = (mode == TARGET_LOOK) ? "Feature: " : "Monster: ";
        else if (strstr(info, "m,"))
            s1 = "Position: ";
    }

    /* Hack -- hallucination */
    if (p_ptr->image)
    {
        cptr name = "something strange";


        /* Display a message */
        sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);

        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Stop on everything but "return" */
        if ((query != '\r') && (query != '\n')) return query;

        /* Repeat forever */
        return 0;
    }

    inv = inv_filter_floor(point(x, y), obj_is_found);
    obj_ct = inv_count_slots(inv, obj_exists);
    if (obj_ct)
        x_info = "x,";

    /* Actual monsters */
    if (c_ptr->m_idx && m_list[c_ptr->m_idx].ml)
    {
        monster_type *m_ptr = &m_list[c_ptr->m_idx];
        bool          fuzzy = BOOL(m_ptr->mflag2 & MFLAG2_FUZZY);
        monster_race *ap_r_ptr = mon_apparent_race(m_ptr);
        char m_name[80];
        bool recall = FALSE;

        boring = FALSE;

        if (fuzzy)
            strcpy(m_name, "Monster");
        else
        {
            monster_desc(m_name, m_ptr, MD_INDEF_VISIBLE);
            mon_track(m_ptr);
            health_track(c_ptr->m_idx);
            handle_stuff();
        }

        /* Interact */
        while (1)
        {
            /* Recall */
            if (recall && !fuzzy)
            {
                doc_ptr doc = doc_alloc(72);

                /* Save */
                screen_save();

                /* Recall on screen */
                mon_display_doc(ap_r_ptr, doc);
                doc_sync_term(doc, doc_range_all(doc), doc_pos_create(0, 1));
                doc_free(doc);

                /* Hack -- Complete the prompt (again)
                Term_addstr(-1, TERM_WHITE, format("  [r,%s%s]", x_info, info));*/

                /* Command */
                query = inkey();

                /* Restore */
                screen_load();

                /* Normal commands */
                if (query != 'r') break;

                /* Toggle recall */
                recall = FALSE;

                /* Cleare recall text and repeat */
                continue;
            }

            /*** Normal ***/

            /* Describe, and prompt for recall */
            sprintf(out_val, "%s%s%s%s ", s1, s2, s3, m_name);
            if (is_pet(m_ptr))
                strcat(out_val, "(Pet) ");
            else if (!fuzzy)
            {
                if (is_friendly(m_ptr))
                    strcat(out_val, "(Friendly) ");
                else if (m_ptr->smart & (1U << SM_CLONED))
                    strcat(out_val, "(Clone) ");
            }
            if (display_distance)
                sprintf(out_val + strlen(out_val), "(Rng %d) ", m_ptr->cdis);
            sprintf(out_val + strlen(out_val), "[r,%s%s]", x_info, info);

            prt(out_val, 0, 0);

            /* Place cursor */
            move_cursor_relative(y, x);

            /* Command */
            query = inkey();

            /* Normal commands */
            if (query != 'r') break;

            /* Toggle recall */
            recall = TRUE;
        }

        /* Always stop at "normal" keys */
        if ((query != '\r') && (query != '\n') && (query != ' ') && (query != 'x'))
        {
            inv_free(inv);
            return query;
        }

        /* Sometimes stop at "space" key */
        if ((query == ' ') && !(mode & (TARGET_LOOK)))
        {
            inv_free(inv);
            return query;
        }

        /* Change the intro */
        s1 = "It is ";


        /* Hack -- take account of gender */
        if (ap_r_ptr->flags1 & (RF1_FEMALE)) s1 = "She is ";

        else if (ap_r_ptr->flags1 & (RF1_MALE)) s1 = "He is ";


        /* Use a preposition */
        s2 = "carrying ";


        /* Scan all objects being carried */
        for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            char o_name[MAX_NLEN];

            object_type *o_ptr;

            /* Acquire object */
            o_ptr = &o_list[this_o_idx];

            /* Acquire next object */
            next_o_idx = o_ptr->next_o_idx;

            /* Obtain an object description */
            object_desc(o_name, o_ptr, 0);

            /* Describe the object */
            sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);

            prt(out_val, 0, 0);
            move_cursor_relative(y, x);
            query = inkey();

            /* Always stop at "normal" keys */
            if ((query != '\r') && (query != '\n') && (query != ' ') && (query != 'x'))
            {
                inv_free(inv);
                return query;
            }

            /* Sometimes stop at "space" key */
            if ((query == ' ') && !(mode & (TARGET_LOOK)))
            {
                inv_free(inv);
                return query;
            }

            /* Change the intro */
            s2 = "also carrying ";
        }

        /* Use a preposition */
        s2 = "on ";
    }

    /* Show objects on this grid. If multiple, show a list. If the
     * list won't fit on the screen, <CR> scrolls the list */
    if (obj_ct == 1)
    {
        obj_ptr obj = inv_obj(inv, 1);
        char name[MAX_NLEN];

        object_desc(name, obj, 0);

        sprintf(out_val, "%s%s%s%s [%s]",
            s1, s2, s3, name, info);

        prt(out_val, 0, 0);
        move_cursor_relative(y, x);

        query = inkey();
        inv_free(inv);
        return query;
    }
    else if (obj_ct > 1)
    {
        doc_ptr doc = NULL;
        rect_t  r = ui_map_rect();
        int     top = 0, lines = r.cy - 1;
        if (boring)
        {
            /* Display rough information about items */
            sprintf(out_val, "%s%s%sa pile of %d items [x,%s]",
                s1, s2, s3, obj_ct, info);

            prt(out_val, 0, 0);
            move_cursor_relative(y, x);

            query = inkey();

            if (query != 'x' && query != ' ')
            {
                inv_free(inv);
                return query;
            }
        }
        doc = doc_alloc(72);
        inv_display(inv, 1, 0, obj_exists, doc, INV_SHOW_SLOT);
        for (;;)
        {
            screen_save();
            doc_sync_term(doc,
                doc_range_middle_lines(doc, top, top + lines),
                doc_pos_create(r.x, r.y));

            sprintf(out_val, "%s%s%sa pile of %d items [Enter,%s]",
                s1, s2, s3, obj_ct, info);
            prt(out_val, 0, 0);
            query = inkey();
            screen_load();

            if (query != '\\' && query != '\r')
            {
                doc_free(doc);
                inv_free(inv);
                return query;
            }
            if (query == '\r' && top + lines < obj_ct) top++;
            if (query == '\\' && top > 0) top--;
        }
        /* unreachable */
    }
    assert(obj_ct == 0);
    inv_free(inv);

    /* Feature code (applying "mimic" field) */
    feat = get_feat_mimic(c_ptr);

    /* Require knowledge about grid, or ability to see grid */
    if (!(c_ptr->info & CAVE_MARK) && !player_can_see_bold(y, x))
    {
        /* Forget feature */
        feat = feat_none;
    }

    f_ptr = &f_info[feat];

    /* Terrain feature if needed */
    if (boring || have_flag(f_ptr->flags, FF_REMEMBER))
    {
        cptr name;

        /* Hack -- special handling for quest entrances */
        if (have_flag(f_ptr->flags, FF_QUEST_ENTER))
        {
            quest_ptr q = quests_get(c_ptr->special);
            name = format("the entrance to the quest '%s' (level %d)", kayttonimi(q), q->danger_level);
        }

        /* Hack -- special handling for building doors */
        else if (have_flag(f_ptr->flags, FF_BLDG) && !p_ptr->inside_arena)
        {
            name = building[f_ptr->subtype].name;
        }
        else if (have_flag(f_ptr->flags, FF_ENTRANCE))
        {
            if (d_info[c_ptr->special].flags1 & DF1_RANDOM)
                name = format("%s (level ?)", d_text + d_info[c_ptr->special].text);
            else
                name = format("%s (level %d)", d_text + d_info[c_ptr->special].text, d_info[c_ptr->special].mindepth);
        }
        else if (have_flag(f_ptr->flags, FF_TOWN))
        {
            name = town_name(c_ptr->special);
        }
        else if (p_ptr->wild_mode && (feat == feat_floor))
        {
            name = "road";
        }
        else
        {
            name = f_name + f_ptr->name;
        }


        /* Pick a prefix */
        if (*s2 &&
            ((!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY)) ||
             (!have_flag(f_ptr->flags, FF_LOS) && !have_flag(f_ptr->flags, FF_TREE)) ||
             have_flag(f_ptr->flags, FF_TOWN)))
        {
            s2 = "in ";
        }

        /* Hack -- special introduction for store & building doors -KMW- */
        if (have_flag(f_ptr->flags, FF_STORE) ||
            have_flag(f_ptr->flags, FF_QUEST_ENTER) ||
            (have_flag(f_ptr->flags, FF_BLDG) && !p_ptr->inside_arena) ||
            have_flag(f_ptr->flags, FF_ENTRANCE))
        {
            s3 = "";
        }
        else if (have_flag(f_ptr->flags, FF_FLOOR) ||
             have_flag(f_ptr->flags, FF_TOWN) ||
             have_flag(f_ptr->flags, FF_SHALLOW) ||
             have_flag(f_ptr->flags, FF_DEEP))
        {
            s3 ="";
        }
        else
        {
            /* Pick proper indefinite article */
            s3 = (is_a_vowel(name[0])) ? "an " : "a ";
        }

        /* Display a message */
        if (p_ptr->wild_mode && TRUE) /* TODO: I may want to hide this info later ... */
        {
            sprintf(out_val, "%s%s%s%s [%s] L%d", s1, s2, s3, name, info, wilderness_level(x, y));
        }
        else if (p_ptr->wizard)
        {
            char f_idx_str[32];
            if (c_ptr->mimic) sprintf(f_idx_str, "%d/%d", c_ptr->feat, c_ptr->mimic);
            else sprintf(f_idx_str, "%d", c_ptr->feat);
            sprintf(out_val, "%s%s%s%s [%s] %x %s %d %d %d %d (%d,%d)", s1, s2, s3, name, info, c_ptr->info, f_idx_str, c_ptr->dist, c_ptr->cost, c_ptr->when, c_ptr->special, y, x);
        }
        else if (display_distance)
        {
            /* Note: c_ptr->dist != m_ptr->cdis. The cave distance is not the range as diagonals count as 1, not 1.5
               Use distance calculation from update_mon, which sets m_ptr->cdis.*/
            int dy = (py > y) ? (py - y) : (y - py);
            int dx = (px > x) ? (px - x) : (x - px);
            int d  = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));
            sprintf(out_val, "%s%s%s%s [%s] (Rng %d)", s1, s2, s3, name, info, d);
        }
        else
            sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);

        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Always stop at "normal" keys */
        if ((query != '\r') && (query != '\n') && (query != ' ')) return query;
    }

    /* Stop on everything but "return" */
    if ((query != '\r') && (query != '\n')) return query;

    /* Repeat forever */
    return 0;
}


/*
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_fire_dir()".
 *
 * All locations must be on the current panel. Consider the use of
 * "panel_bounds()" to allow "off-panel" targets, perhaps by using
 * some form of "scrolling" the map around the cursor. XXX XXX XXX
 * That is, consider the possibility of "auto-scrolling" the screen
 * while the cursor moves around. This may require changes in the
 * "update_mon()" code to allow "visibility" even if off panel, and
 * may require dynamic recalculation of the "temp" grid set.
 *
 * Hack -- targeting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction. The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid. This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 */
bool target_set(int mode)
{
    int        i, d, m, t, bd;
    int        y = py;
    int        x = px;
    bool       done = FALSE;
    bool       flag = TRUE;
    bool       travel_tgt = FALSE;
    char       query;
    char       info[80];

    cave_type *c_ptr;
    rect_t     map_rect = ui_map_rect();


    /* Cancel target */
    target_who = 0;


    /* Cancel tracking */
    /* health_track(0); */


    /* Prepare the "temp" array */
    target_set_prepare(mode);

    /* Start near the player */
    m = 0;

    /* Interact */
    while (!done)
    {
        /* Interesting grids */
        if (flag && temp_n)
        {
            y = temp_y[m];
            x = temp_x[m];

            if ( !(mode & TARGET_LOOK)
              && !(mode & TARGET_MARK) )
            {
                prt_path(y, x, (mode & TARGET_DISI) ? PROJECT_DISI : 0);
            }

            /* Access */
            c_ptr = &cave[y][x];

            /* Allow target */
            if ( target_able(c_ptr->m_idx)
             || ((mode & (TARGET_MARK|TARGET_DISI|TARGET_XTRA)) && m_list[c_ptr->m_idx].ml))
            {
                strcpy(info, rogue_like_commands ? "q,t,p,o,x,(,+,-,?,<dir>" : "q,t,p,o,x,j,+,-,?,<dir>");
            }

            /* Dis-allow target */
            else
            {
                strcpy(info, rogue_like_commands ? "q,p,o,x,(,+,-,?,<dir>" : "q,p,o,x,j,+,-,?,<dir>");
            }

            /* Describe and Prompt */
            while (!(query = target_set_aux(y, x, mode, info)));

            /* Cancel tracking */
            /* health_track(0); */

            /* Assume no "direction" */
            d = 0;

            if (use_menu)
            {
                if (query == '\r') query = 't';
            }

            if (((query == 'j') || (query == 'J')) && (!rogue_like_commands)) query = '(';

            /* Analyze */
            switch (query)
            {
                case '?':
                    screen_save();
                    doc_display_help("context_targetting.txt", "Targetting");
                    screen_load();
                    break;
                case ESCAPE:
                case 'q':
                {
                    done = TRUE;
                    break;
                }

                case 't':
                case '.':
                case '5':
                case '0':
                {
                    if ( target_able(c_ptr->m_idx)
                     || ((mode & (TARGET_MARK | TARGET_DISI | TARGET_XTRA)) && m_list[c_ptr->m_idx].ml))
                    {
                        health_track(c_ptr->m_idx);
                        target_who = c_ptr->m_idx;
                        target_grab(y, x);
                        done = TRUE;
                    }
                    else
                    {
                        bell();
                    }
                    break;
                }

                case ' ':
                case '*':
                case '+':
                {
                    if (++m == temp_n)
                    {
                        m = 0;
                        if (!expand_list) done = TRUE;
                    }
                    break;
                }

                case '-':
                {
                    if (m-- == 0)
                    {
                        m = temp_n - 1;
                        if (!expand_list) done = TRUE;
                    }
                    break;
                }

                case 'p':
                {
                    /* Recenter the map around the player */
                    viewport_verify();

                    /* Update stuff */
                    p_ptr->update |= (PU_MONSTERS);

                    /* Redraw map */
                    p_ptr->redraw |= (PR_MAP);

                    /* Window stuff */
                    p_ptr->window |= (PW_OVERHEAD);

                    /* Handle stuff */
                    handle_stuff();

                    /* Recalculate interesting grids */
                    target_set_prepare(mode);

                    y = py;
                    x = px;
                }

                case 'o':
                {
                    flag = FALSE;
                    break;
                }

                case 'm':
                {
                    break;
                }

                case '(':
                {
                    if ((!p_ptr->wild_mode) && (in_bounds(y,x)) && (cave[y][x].info & CAVE_MARK) && ((y != py) || (x != px)) && (player_can_enter(cave[y][x].feat, 0)))
                    {
                        travel_cancel_fully();
                        travel.y = y;
                        travel.x = x;
                        if (mode & TARGET_TRVL)
                        {
                            travel_tgt = TRUE;
                            done = TRUE;
                        }
                    }
                    break;
                }

                default:
                {
                    /* Extract the action (if any) */
                    d = get_keymap_dir(query, FALSE);

                    if (!d) bell();
                    break;
                }
            }

            /* Hack -- move around */
            if (d)
            {
                /* Modified to scroll to monster */
                int y2 = viewport_origin.y;
                int x2 = viewport_origin.x;

                /* Find a new monster */
                i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

                /* Request to target past last interesting grid */
                while (flag && (i < 0))
                {
                    /* Note the change */
                    if (viewport_scroll(ddy[d], ddx[d]))
                    {
                        int v = temp_y[m];
                        int u = temp_x[m];

                        /* Recalculate interesting grids */
                        target_set_prepare(mode);

                        /* Look at interesting grids */
                        flag = TRUE;

                        /* Find a new monster */
                        i = target_pick(v, u, ddy[d], ddx[d]);

                        /* Use that grid */
                        if (i >= 0) m = i;
                    }

                    /* Nothing interesting */
                    else
                    {
                        int dx = ddx[d];
                        int dy = ddy[d];

                        /* Restore previous position */
                        viewport_origin.y = y2;
                        viewport_origin.x = x2;

                        /* Update stuff */
                        p_ptr->update |= (PU_MONSTERS); /* XXX Why? */

                        /* Redraw map */
                        p_ptr->redraw |= (PR_MAP);

                        /* Window stuff */
                        p_ptr->window |= (PW_OVERHEAD);

                        /* Handle stuff */
                        redraw_hack = TRUE;
                        handle_stuff();
                        redraw_hack = FALSE;

                        /* Recalculate interesting grids */
                        target_set_prepare(mode);

                        /* Look at boring grids */
                        flag = FALSE;

                        /* Move */
                        x += dx;
                        y += dy;

                        /* Do not move horizontally if unnecessary */
                        if (((x < viewport_origin.x + map_rect.cx / 2) && (dx > 0)) ||
                             ((x > viewport_origin.x + map_rect.cx / 2) && (dx < 0)))
                        {
                            dx = 0;
                        }

                        /* Do not move vertically if unnecessary */
                        if (((y < viewport_origin.y + map_rect.cy / 2) && (dy > 0)) ||
                             ((y > viewport_origin.y + map_rect.cy / 2) && (dy < 0)))
                        {
                            dy = 0;
                        }

                        /* Apply the motion */
                        if (!cave_xy_is_visible(x, y))
                        {
                            if (viewport_scroll(dy, dx)) target_set_prepare(mode);
                        }

                        /* Slide into legality */
                        if (x >= cur_wid-1) x = cur_wid - 2;
                        else if (x <= 0) x = 1;

                        /* Slide into legality */
                        if (y >= cur_hgt-1) y = cur_hgt- 2;
                        else if (y <= 0) y = 1;
                    }
                }

                /* Use that grid */
                m = i;
            }
        }

        /* Arbitrary grids */
        else
        {
            bool move_fast = FALSE;

            if ( !(mode & TARGET_LOOK)
              && !(mode & TARGET_MARK) )
            {
                prt_path(y, x, (mode & TARGET_DISI) ? PROJECT_DISI : 0);
            }

            /* Access */
            c_ptr = &cave[y][x];

            if ((mode & TARGET_MARK) && !m_list[c_ptr->m_idx].ml)
                strcpy(info, rogue_like_commands ? "q,p,o,x,(,+,-,?,<dir>" : "q,p,o,x,j,+,-,?,<dir>");
            else
                strcpy(info, rogue_like_commands ? "q,t,p,m,x,(,+,-,?,<dir>" : "q,t,p,m,x,j,+,-,?,<dir>");


            /* Describe and Prompt (enable "TARGET_LOOK") */
            while (!(query = target_set_aux(y, x, mode | TARGET_LOOK, info)));

            /* Cancel tracking */
            /* health_track(0); */

            /* Assume no direction */
            d = 0;

            if (use_menu)
            {
                if (query == '\r') query = 't';
            }

            if (((query == 'j') || (query == 'J')) && (!rogue_like_commands)) query = '(';

            /* Analyze the keypress */
            switch (query)
            {
                case '?':
                    screen_save();
                    doc_display_help("context_targetting.txt", "Targetting");
                    screen_load();
                    break;
                case ESCAPE:
                case 'q':
                {
                    done = TRUE;
                    break;
                }

                case 't':
                case '.':
                case '5':
                case '0':
                if ( !(mode & TARGET_MARK)
                  || (c_ptr->m_idx && m_list[c_ptr->m_idx].ml) )
                {
                    if (mode & TARGET_MARK)
                        target_who = c_ptr->m_idx;
                    else
                        target_who = -1;
                    target_grab(y, x);
                    done = TRUE;
                }
                else
                {
                    bell();
                }
                break;

                case 'p':
                {
                    /* Recenter the map around the player */
                    viewport_verify();

                    /* Update stuff */
                    p_ptr->update |= (PU_MONSTERS);

                    /* Redraw map */
                    p_ptr->redraw |= (PR_MAP);

                    /* Window stuff */
                    p_ptr->window |= (PW_OVERHEAD);

                    /* Handle stuff */
                    handle_stuff();

                    /* Recalculate interesting grids */
                    target_set_prepare(mode);

                    y = py;
                    x = px;
                }

                case 'o':
                {
                    break;
                }

                case ' ':
                case '*':
                case '+':
                case '-':
                case 'm':
                {
                    flag = TRUE;

                    m = 0;
                    bd = 999;

                    /* Pick a nearby monster */
                    for (i = 0; i < temp_n; i++)
                    {
                        t = distance(y, x, temp_y[i], temp_x[i]);

                        /* Pick closest */
                        if (t < bd)
                        {
                            m = i;
                            bd = t;
                        }
                    }

                    /* Nothing interesting */
                    if (bd == 999) flag = FALSE;

                    break;
                }

                case '(':
                {
                    if ((!p_ptr->wild_mode) && (in_bounds(y,x)) && (cave[y][x].info & CAVE_MARK) && ((y != py) || (x != px)) && (player_can_enter(cave[y][x].feat, 0)))
                    {
                        travel_cancel_fully();
                        travel.y = y;
                        travel.x = x;
                        if (mode & TARGET_TRVL)
                        {
                            travel_tgt = TRUE;
                            done = TRUE;
                        }
                    }
                    break;
                }

                default:
                {
                    /* Extract the action (if any) */
                    d = get_keymap_dir(query, FALSE);

                    /* XTRA HACK MOVEFAST */
                    if (isupper(query)) move_fast = TRUE;

                    if (!d) bell();
                    break;
                }
            }

            /* Handle "direction" */
            if (d)
            {
                int dx = ddx[d];
                int dy = ddy[d];

                /* XTRA HACK MOVEFAST */
                if (move_fast)
                {
                    int mag = MIN(map_rect.cx / 2, map_rect.cy / 2);
                    x += dx * mag;
                    y += dy * mag;
                }
                else
                {
                    x += dx;
                    y += dy;
                }

                /* Do not move horizontally if unnecessary */
                if (((x < viewport_origin.x + map_rect.cx / 2) && (dx > 0)) ||
                     ((x > viewport_origin.x + map_rect.cx / 2) && (dx < 0)))
                {
                    dx = 0;
                }

                /* Do not move vertically if unnecessary */
                if (((y < viewport_origin.y + map_rect.cy / 2) && (dy > 0)) ||
                     ((y > viewport_origin.y + map_rect.cy / 2) && (dy < 0)))
                {
                    dy = 0;
                }

                /* Apply the motion */
                if (!cave_xy_is_visible(x, y))
                {
                    if (viewport_scroll(dy, dx)) target_set_prepare(mode);
                }

                /* Slide into legality */
                if (x >= cur_wid-1) x = cur_wid - 2;
                else if (x <= 0) x = 1;

                /* Slide into legality */
                if (y >= cur_hgt-1) y = cur_hgt- 2;
                else if (y <= 0) y = 1;
            }
        }
    }

    /* Forget */
    temp_n = 0;

    msg_line_clear();
    prt("", 0, 0);

    /* Recenter the map around the player */
    viewport_verify();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_HEALTH_BARS);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_MONSTER_LIST);

    /* Prevent losing visibility on newly acquired target (PU_MONSTERS above) */
    redraw_hack = TRUE;
    handle_stuff();
    redraw_hack = FALSE;

    if (travel_tgt)
    {
        travel_begin(TRAVEL_MODE_NORMAL, travel.x, travel.y);
    }

    /* Failure to set target */
    if (!target_who) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 *
 */
bool get_fire_dir(int *dp) { return get_fire_dir_aux(dp, TARGET_KILL); }
bool get_fire_dir_aux(int *dp, int target_mode)
{
	if (old_target_okay_mode(target_mode)) {
		*dp = 5;
		p_ptr->redraw |= PR_HEALTH_BARS;
		return TRUE;
	}
    /* auto_target the closest monster if no valid target is selected up front */
    if (auto_target && !p_ptr->confused && !p_ptr->image)
    {
        int i, best_m_idx = 0, best_dis = 9999;

        for (i = 0; i < max_m_idx; i++)
        {
            monster_type *m_ptr = &m_list[i];
            if (!m_ptr->r_idx) continue;
            if (!m_ptr->ml) continue;
            if (!target_pet && is_pet(m_ptr)) continue;
            if (target_mode != TARGET_DISI && !projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;
            if (m_ptr->cdis < best_dis)
            {
                best_dis = m_ptr->cdis;
                best_m_idx = i;
            }
        }
        if (best_m_idx)
        {
            target_who = best_m_idx;
            target_grab(m_list[best_m_idx].fy, m_list[best_m_idx].fx);
            *dp = 5;
            p_ptr->redraw |= PR_HEALTH_BARS;
            return TRUE;
        }
    }
    /* fall back on normal target selection */
    return get_aim_dir_aux(dp, target_mode);
}

bool get_aim_dir(int *dp) { return get_aim_dir_aux(dp, TARGET_KILL); }
bool get_aim_dir_aux(int *dp, int target_mode)
{
    int        dir;

    char    command;

    cptr    p;

    /* Initialize */
    (*dp) = 0;

    /* Global direction */
    dir = command_dir;



#ifdef ALLOW_REPEAT /* TNB */

    if (repeat_pull(dp))
    {
        /* Confusion? */

        /* Verify */
        if (!((*dp == 5) && ((!target_okay_aux(target_mode)) || (old_target_never_okay))))
        {
/*            return (TRUE); */
            dir = *dp;
        }
    }

#endif /* ALLOW_REPEAT -- TNB */

    /* Ask until satisfied */
    while (!dir)
    {
        /* Choose a prompt */
        if (!target_okay_aux(target_mode))
        {
            p = "Direction ('*' to choose a target, Escape to cancel)? ";

        }
        else
        {
            p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";

        }

        /* Get a command (or Cancel) */
        if (!get_com(p, &command, TRUE)) break;

        if (use_menu)
        {
            if (command == '\r') command = 't';
        }

        /* Convert various keys to "standard" keys */
        switch (command)
        {
            /* Use current target */
            case 'T':
            case 't':
            case '.':
            case '5':
            case '0':
            {
                dir = 5;
                break;
            }

            /* Set new target */
            case '*':
            case ' ':
            case '\r':
            {
                if (target_set(target_mode)) dir = 5;
                break;
            }

            default:
            {
                /* Extract the action (if any) */
                dir = get_keymap_dir(command, FALSE);

                break;
            }
        }

        /* Verify requested targets */
        if ((dir == 5) && (!target_okay_aux(target_mode))) dir = 0;

        /* Error */
        if (!dir) bell();
    }

    /* No direction */
    if (!dir)
    {
        project_length = 0; /* reset to default */
        return (FALSE);
    }

    /* Save the direction */
    command_dir = dir;

    /* Check for confusion */
    if (p_ptr->confused)
    {
        /* XXX XXX XXX */
        /* Random direction */
        dir = ddd[randint0(8)];
    }

    /* Notice confusion */
    if (command_dir != dir)
    {
        /* Warn the user */
        msg_print("You are confused.");

    }

    /* Save direction */
    (*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

/*    repeat_push(dir); */
    repeat_push(command_dir);

#endif /* ALLOW_REPEAT -- TNB */

    /* A "valid" direction was entered */
    return (TRUE);
}



/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "command_dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player. Note that,
 * for example, it is no longer possible to "disarm" or "open" chests
 * in the same grid as the player.
 *
 * Direction "5" is illegal and will (cleanly) abort the command.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
int get_rep_dir(int *dp, bool under)
{
    int result = GET_DIR_OK;
    int dir;

    /* Initialize */
    (*dp) = 0;

    /* Global direction */
    dir = command_dir;

#ifdef ALLOW_REPEAT /* TNB */

    if (repeat_pull(dp))
    {
        dir = *dp;
/*        return (TRUE); */
    }

#endif /* ALLOW_REPEAT -- TNB */

    /* Get a direction */
    while (!dir)
    {
        char ch;

        /* Get a command (or Cancel) */
        if (!get_com("Direction (Escape to cancel)? ", &ch, TRUE)) break;


        /* Look up the direction */
        dir = get_keymap_dir(ch, under);

        /* Oops */
        if (!dir) bell();
    }

    /* Prevent weirdness */
    if ((dir == 5) && (!under)) dir = 0;

    /* Aborted */
    if (!dir) return (FALSE);

    /* Save desired direction */
    command_dir = dir;

    /* Apply "confusion" */
    if (p_ptr->confused)
    {
        /* Standard confusion */
        if (randint0(100) < 75)
        {
            /* Random direction */
            dir = ddd[randint0(8)];
        }
    }
    else if (p_ptr->riding)
    {
        monster_type *m_ptr = &m_list[p_ptr->riding];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        if (MON_CONFUSED(m_ptr))
        {
            /* Standard confusion */
            if (randint0(100) < 75)
            {
                /* Random direction */
                dir = ddd[randint0(8)];
            }
        }
        else if ((r_ptr->flags1 & RF1_RAND_50) && (r_ptr->flags1 & RF1_RAND_25) && (randint0(100) < 50))
        {
            /* Random direction */
            dir = ddd[randint0(8)];
        }
        else if ((r_ptr->flags1 & RF1_RAND_50) && (randint0(100) < 25))
        {
            /* Random direction */
            dir = ddd[randint0(8)];
        }
    }
    else if (p_ptr->move_random && !p_ptr->wild_mode)
    {
        if (one_in_(66))
        {
            dir = ddd[randint0(8)];
        }
    }

    /* Notice confusion */
    if (command_dir != dir)
    {
        if (p_ptr->confused)
            msg_print("You are confused.");
        else if (p_ptr->move_random)
            cmsg_print(TERM_YELLOW, "You are moving erratically.");
        else
        {
            char m_name[80];
            monster_type *m_ptr = &m_list[p_ptr->riding];

            monster_desc(m_name, m_ptr, 0);
            if (MON_CONFUSED(m_ptr))
                msg_format("%^s is confused.", m_name);
            else
                msg_format("You cannot control %s.", m_name);
        }
        /* Block running in random directions */
        result = GET_DIR_RANDOM;
    }

    /* Save direction */
    (*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

/*    repeat_push(dir); */
    repeat_push(command_dir);

#endif /* ALLOW_REPEAT -- TNB */

    /* Success */
    return result;
}


bool get_rep_dir2(int *dp)
{
    int dir;

    /* Initialize */
    (*dp) = 0;

    /* Global direction */
    dir = command_dir;

#ifdef ALLOW_REPEAT /* TNB */

    if (repeat_pull(dp))
    {
        dir = *dp;
/*        return (TRUE); */
    }

#endif /* ALLOW_REPEAT -- TNB */

    /* Get a direction */
    while (!dir)
    {
        char ch;

        /* Get a command (or Cancel) */
        if (!get_com("Direction (Escape to cancel)? ", &ch, TRUE)) break;


        /* Look up the direction */
        dir = get_keymap_dir(ch, FALSE);

        /* Oops */
        if (!dir) bell();
    }

    /* Prevent weirdness */
    if (dir == 5) dir = 0;

    /* Aborted */
    if (!dir) return (FALSE);

    /* Save desired direction */
    command_dir = dir;

    /* Apply "confusion" */
    if (p_ptr->confused)
    {
        /* Standard confusion */
        if (randint0(100) < 75)
        {
            /* Random direction */
            dir = ddd[randint0(8)];
        }
    }

    /* Notice confusion */
    if (command_dir != dir)
    {
        /* Warn the user */
        msg_print("You are confused.");

    }

    /* Save direction */
    (*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

/*    repeat_push(dir); */
    repeat_push(command_dir);

#endif /* ALLOW_REPEAT -- TNB */

    /* Success */
    return (TRUE);
}



/*
 * XAngband: determine if a given location is "interesting"
 * based on target_set_accept function.
 */
static bool tgt_pt_accept(int y, int x)
{
    cave_type *c_ptr;

    /* Bounds */
    if (!(in_bounds(y, x))) return (FALSE);

    /* Player grid is always interesting */
    if ((y == py) && (x == px)) return (TRUE);

    /* Handle hallucination */
    if (p_ptr->image) return (FALSE);

    /* Examine the grid */
    c_ptr = &cave[y][x];

    /* Interesting memorized features */
    if (c_ptr->info & (CAVE_MARK))
    {
        /* Notice stairs */
        if (cave_have_flag_grid(c_ptr, FF_LESS)) return (TRUE);
        if (cave_have_flag_grid(c_ptr, FF_MORE)) return (TRUE);

        /* Notice quest features */
        if (cave_have_flag_grid(c_ptr, FF_QUEST_ENTER)) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * XAngband: Prepare the "temp" array for "tget_pt"
 * based on target_set_prepare funciton.
 */
static void tgt_pt_prepare(void)
{
    int y, x;

    /* Reset "temp" array */
    temp_n = 0;

    if (!expand_list) return;

    /* Scan the current panel */
    for (y = 1; y < cur_hgt; y++)
    {
        for (x = 1; x < cur_wid; x++)
        {
            /* Require "interesting" contents */
            if (!tgt_pt_accept(y, x)) continue;

            /* Save the location */
            temp_x[temp_n] = x;
            temp_y[temp_n] = y;
            temp_n++;
        }
    }

    /* Target the nearest monster for shooting */
    ang_sort_comp = ang_sort_comp_distance;
    ang_sort_swap = ang_sort_swap_distance;

    /* Sort the positions */
    ang_sort(temp_x, temp_y, temp_n);
}

/*
 * old -- from PsiAngband.
 */
bool tgt_pt(int *x_ptr, int *y_ptr, int rng)
{
    char ch = 0;
    int d, x, y, n = 0;
    bool success = FALSE;
    rect_t map_rect = ui_map_rect();

    x = px;
    y = py;

    if (expand_list)
    {
        tgt_pt_prepare();
        n = 0;
    }

    msg_print("Select a point and press <color:y>space</color>. < and > cycle through stairs, * cycles through monsters");

    while ((ch != ESCAPE) && !success)
    {
        bool move_fast = FALSE;

        move_cursor_relative(y, x);
        ch = inkey();
        switch (ch)
        {
        case ESCAPE:
            break;
        case ' ':
        case 't':
        case '.':
        case '5':
        case '0':
            /* illegal place */
            if (player_bold(y, x)) ch = 0;

            /* okay place */
            else success = TRUE;

            break;

        /* XAngband: Move cursor to stairs */
		/* Composband: Move cursor to monsters */
        case '>':
        case '<':
            if (expand_list && temp_n)
            {
                int dx, dy;
                int cx = map_rect.cy / 2;
                int cy = map_rect.cx / 2;

                n++;

                while(n < temp_n)    /* Skip stairs which have different distance */
                {
                    cave_type *c_ptr = &cave[temp_y[n]][temp_x[n]];

                    if (ch == '>')
                    {
                        if (cave_have_flag_grid(c_ptr, FF_LESS) ||
                            cave_have_flag_grid(c_ptr, FF_QUEST_ENTER))
                            n++;
                        else
                            break;
                    }
                    else if (ch == '<')
                    {
                        if (cave_have_flag_grid(c_ptr, FF_MORE))
                            n++;
                        else
                            break;
                    }
                }

                if (n == temp_n)    /* Loop out taget list */
                {
                    n = 0;
                    y = py;
                    x = px;
                    viewport_verify();    /* Move cursor to player */

                    /* Update stuff */
                    p_ptr->update |= (PU_MONSTERS);

                    /* Redraw map */
                    p_ptr->redraw |= (PR_MAP);

                    /* Window stuff */
                    p_ptr->window |= (PW_OVERHEAD);

                    /* Handle stuff */
                    handle_stuff();
                }
                else    /* move cursor to next stair and change panel */
                {
                    y = temp_y[n];
                    x = temp_x[n];

                    dy = 2 * (y - cy) / map_rect.cy;
                    dx = 2 * (x - cx) / map_rect.cx;
                    if (dy || dx) viewport_scroll(dy, dx);
                }
            }
            break;

        default:
            /* Look up the direction */
            d = get_keymap_dir(ch, FALSE);

            /* XTRA HACK MOVEFAST */
            if (isupper(ch)) move_fast = TRUE;

            /* Handle "direction" */
            if (d)
            {
                int dx = ddx[d];
                int dy = ddy[d];
                int old_x = x;
                int old_y = y;

                /* XTRA HACK MOVEFAST */
                if (move_fast)
                {
                    int mag = MIN(map_rect.cx / 2, map_rect.cy / 2);
                    x += dx * mag;
                    y += dy * mag;
                }
                else
                {
                    x += dx;
                    y += dy;
                }

                if (rng > 0 && distance(py, px, y, x) > rng)
                {
                    bell();
                    x = old_x;
                    y = old_y;
                    continue;
                }

                /* Do not move horizontally if unnecessary */
                if (((x < viewport_origin.x + map_rect.cx / 2) && (dx > 0)) ||
                     ((x > viewport_origin.x + map_rect.cx / 2) && (dx < 0)))
                {
                    dx = 0;
                }

                /* Do not move vertically if unnecessary */
                if (((y < viewport_origin.y + map_rect.cy / 2) && (dy > 0)) ||
                     ((y > viewport_origin.y + map_rect.cy / 2) && (dy < 0)))
                {
                    dy = 0;
                }

                /* Apply the motion */
                if (!cave_xy_is_visible(x, y))
                {
                    /* if (change_panel(dy, dx)) target_set_prepare(mode); */
                    viewport_scroll(dy, dx);
                }

                /* Slide into legality */
                if (x >= cur_wid-1) x = cur_wid - 2;
                else if (x <= 0) x = 1;

                /* Slide into legality */
                if (y >= cur_hgt-1) y = cur_hgt- 2;
                else if (y <= 0) y = 1;

            }
            break;
        }
    }

    msg_line_clear();

    /* Recenter the map around the player */
    viewport_verify();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD);

    /* Handle stuff */
    handle_stuff();

    *x_ptr = x;
    *y_ptr = y;
    return success;
}


bool get_hack_dir(int *dp)
{
    int        dir;
    cptr    p;
    char    command;


    /* Initialize */
    (*dp) = 0;

    /* Global direction */
    dir = 0;

    /* (No auto-targeting) */

    /* Ask until satisfied */
    while (!dir)
    {
        /* Choose a prompt */
        if (!target_okay())
        {
            p = "Direction ('*' to choose a target, Escape to cancel)? ";

        }
        else
        {
            p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";

        }

        /* Get a command (or Cancel) */
        if (!get_com(p, &command, TRUE)) break;

        if (use_menu)
        {
            if (command == '\r') command = 't';
        }

        /* Convert various keys to "standard" keys */
        switch (command)
        {
            /* Use current target */
            case 'T':
            case 't':
            case '.':
            case '5':
            case '0':
            {
                dir = 5;
                break;
            }

            /* Set new target */
            case '*':
            case ' ':
            case '\r':
            {
                if (target_set(TARGET_KILL)) dir = 5;
                break;
            }

            default:
            {
                /* Look up the direction */
                dir = get_keymap_dir(command, FALSE);

                break;
            }
        }

        /* Verify requested targets */
        if ((dir == 5) && !target_okay()) dir = 0;

        /* Error */
        if (!dir) bell();
    }

    /* No direction */
    if (!dir) return (FALSE);

    /* Save the direction */
    command_dir = dir;

    /* Check for confusion */
    if (p_ptr->confused)
    {
        /* XXX XXX XXX */
        /* Random direction */
        dir = ddd[randint0(8)];
    }

    /* Notice confusion */
    if (command_dir != dir)
    {
        /* Warn the user */
        msg_print("You are confused.");

    }

    /* Save direction */
    (*dp) = dir;

    /* A "valid" direction was entered */
    return (TRUE);
}



#define Go_no_JuuJou 5*5*5*5*5*5*5*5*5*5

s16b gain_energy(void)
{
    int i;
    s32b energy_result = 10;
    s32b tmp;

    tmp = randint0(Go_no_JuuJou);

    for (i = 0; i < 9; i ++){
        energy_result += tmp % 5;
        tmp /= 5;
    }

    return (s16b)(energy_result + tmp);
}


/*
 * Return bow energy
 */
int bow_energy(int sval)
{
    int energy = 10000;

    /* Analyze the launcher */
    switch (sval)
    {
        /* Sling and ammo */
        case SV_SLING:
        {
            /*energy = 8000;*/
            energy = 7150;
            break;
        }

        /* Short Bow and Arrow */
        case SV_SHORT_BOW:
        {
            energy = 8888;
            break;
        }

        /* Long Bow and Arrow */
        case SV_LONG_BOW:
        case SV_CRIMSON:
        case SV_RAILGUN:
        case SV_HARP:
        {
            energy = 10000;
            break;
        }

        /* Bow of irresponsiblity and Arrow */
        case SV_NAMAKE_BOW:
        {
            energy = 10000; /* Now has +7 XTRA_SHOTS (currently x1.75) */
            break;
        }

        /* Light Crossbow and Bolt */
        case SV_LIGHT_XBOW:
        {
            energy = 12000;
            break;
        }

        /* Heavy Crossbow and Bolt */
        case SV_HEAVY_XBOW:
        {
            energy = 13333;
            break;
        }
    }

    return (energy);
}

/*
 * Return alignment title
 */
cptr your_alignment(void)
{
    if (p_ptr->align > 150) return "Lawful";
    else if (p_ptr->align > 50) return "Good";
    else if (p_ptr->align > 10) return "Neutral Good";
    else if (p_ptr->align > -11) return "Neutral";
    else if (p_ptr->align > -51) return "Neutral Evil";
    else if (p_ptr->align > -151) return "Evil";
    else return "Chaotic";
}


/*
 * Return proficiency level of weapons and misc. skills (except riding)
 */
int weapon_exp_level(int weapon_exp)
{
    if (weapon_exp < WEAPON_EXP_BEGINNER) return EXP_LEVEL_UNSKILLED;
    else if (weapon_exp < WEAPON_EXP_SKILLED) return EXP_LEVEL_BEGINNER;
    else if (weapon_exp < WEAPON_EXP_EXPERT) return EXP_LEVEL_SKILLED;
    else if (weapon_exp < WEAPON_EXP_MASTER) return EXP_LEVEL_EXPERT;
    else return EXP_LEVEL_MASTER;
}


/*
 * Return proficiency level of riding
 */
int riding_exp_level(int riding_exp)
{
    if (riding_exp < RIDING_EXP_BEGINNER) return EXP_LEVEL_UNSKILLED;
    else if (riding_exp < RIDING_EXP_SKILLED) return EXP_LEVEL_BEGINNER;
    else if (riding_exp < RIDING_EXP_EXPERT) return EXP_LEVEL_SKILLED;
    else if (riding_exp < RIDING_EXP_MASTER) return EXP_LEVEL_EXPERT;
    else return EXP_LEVEL_MASTER;
}


/*
 * Return proficiency level of spells
 */
int spell_exp_level(int spell_exp)
{
    if (spell_exp < SPELL_EXP_BEGINNER) return EXP_LEVEL_UNSKILLED;
    else if (spell_exp < SPELL_EXP_SKILLED) return EXP_LEVEL_BEGINNER;
    else if (spell_exp < SPELL_EXP_EXPERT) return EXP_LEVEL_SKILLED;
    else if (spell_exp < SPELL_EXP_MASTER) return EXP_LEVEL_EXPERT;
    else return EXP_LEVEL_MASTER;
}
