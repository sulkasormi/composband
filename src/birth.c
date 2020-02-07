/* Purpose: create a player character */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

#include "angband.h"
#include "z-doc.h"


cptr realm_jouhou[VALID_REALM] =
{
"Life magic is very good for healing; it relies mostly on healing, protection and detection spells. Also life magic have a few attack spells as well. It said that some high level spell of life magic can disintegrate Undead monsters into ash.",
"Sorcery is a `meta` realm, including enchantment and general spells. It provides superb protection spells, spells to enhance your odds in combat and, most importantly, a vast selection of spells for gathering information. However, Sorcery has one weakness: it has no spells to deal direct damage to your enemies.",
"Nature magic makes you master of elements; it provides protection, detection, curing and attack spells. Nature also has a spell of Herbal Healing, which is the only powerful healing spell outside the realm of Life magic.",
"There are few types of magic more unpredictable and difficult to control than Chaos magic. Chaos is the very element of unmaking, and the Chaos spells are the most terrible weapons of destruction imaginable. The caster can also call on the primal forces of Chaos to induce mutations in his/her opponents and even him/herself.",
"There is no fouler nor more evil category of spells than the necromantic spells of Death Magic. These spells are relatively hard to learn, but at higher levels the spells give the caster power over living and the (un)dead, but the most powerful spells need his / her own blood as the focus, often hurting the caster in the process of casting.",
"Trump magic has, indeed, an admirable selection of teleportation spells. Since the Trump gateways can also be used to summon other creatures, Trump magic has an equally impressive selection of summoning spells. However, not all monsters appreciate being drawn to another place by Trump user.",
"Arcane magic is a general purpose realm of magic. It attempts to encompass all 'useful' spells from all realms. This is the downside of Arcane magic: while Arcane does have all the necessary 'tool' spells for a dungeon delver, it has no ultra-powerful high level spells. As a consequence, all Arcane spellbooks can be bought in town. It should also be noted that the 'specialized' realms usually offer the same spell at a lower level and cost. ",
"Craft magic can strengthen the caster or the equipments. These spells can greatly improve the caster's fighting ability. Using them against opponents directly is not possible.",
"Demon is a very evil realm, same as Death. It provides various attack spells and devilish detection spells. at higher levels, Demon magic provides ability to dominate demons, and to polymorph yourself into a demon.",
"Crusade is a magic of 'Justice'. It includes damage spells, which are greatly effective against foul and evil monsters, but have poor effects against good monsters.",
"Necromancy allows communication with and ultimately control over the deceased. All direct damage afforded by this realm requires the caster to touch his or her opponent. Any weapons or gloves will obstruct this macabre contact.",
"Armageddon is the most deadly offensive realm. You won't be lacking for firepower here. "
    "However, every spell is an offensive spell, so this realm suffers from a lack of any "
    "utility spells.",
"Experts in Law know many ways to confuse and hinder their enemies, to set and detect traps, to escape difficult situations and to win others to their side. However, they are somewhat lacking in direct offensive power.",
"Music adepts affect the world around them by singing songs. There are two types of song; some have instant effects, while others have a continuous effect until SP runs out. But the latter type has a limit: only one song can be sung at a time.",
"The books of Kendo describe about various combat techniques. When learning new techniques, you are required to carry the books, but once you memorizes them, you don't have to carry them. When using a technique, wielding a weapon is required.",
"Hex is a very terrible realm. Spells gives continual effects when they are spelled continually like songs. Spells may obstract monsters' actions, may deal damages in sight, may revenge against enemies.",
"The books of Rage describe various techniques. To learn a new technique, you must perform a ritual of rage, destroying the book in the process."
    " Once learned, you may use techniques without requiring the corresponding Rage book, but you will need to find many copies of each book in order"
    " to learn all of the techniques.",
"Burglary is the preferred realm of rogues, allowing them to specialize in what they do best: Stealing! "
    "This realm offers good detection and escapes, offers talents for picking pockets and setting traps, "
    "and even allows for direct assassination of sleeping monsters. The books for this realm are only "
    "available in the Black Market (or in the dungeon).",
};



#define AUTOROLLER_STEP 50
#define AUTOROLLER_MAX 50 * 1000
#define AUTOROLLER_DELAY
#define MAX_TRIES 100

void get_max_stats(void)
{
    int        i, j, roll;
    int        dice[6];

    /* Roll and verify some stats */
    while (TRUE)
    {
        /* Roll some dice */
        for (j = i = 0; i < 6; i++)
        {
            /* Roll the dice */
            roll = randint1(7);

            dice[i] = roll;

            /* Collect the maximum */
            j += dice[i];
        }

        /* Verify totals */
        if (j == 24) break;
    }

    /* Acquire the stats */
    for (i = 0; i < 6; i++)
    {
        j = 18 + 60 + dice[i]*10;

        /* Save that value */
        p_ptr->stat_max_max[i] = j;
        if (p_ptr->stat_max[i] > j)
            p_ptr->stat_max[i] = j;
        if (p_ptr->stat_cur[i] > j)
            p_ptr->stat_cur[i] = j;
    }
    p_ptr->knowledge &= ~(KNOW_STAT);

    /* Redisplay the stats later */
    p_ptr->redraw |= (PR_STATS);
}

int _race_exp_factor(void)
{
    if (p_ptr->prace == RACE_DOPPELGANGER)
        return get_race()->exp;
    return get_true_race()->exp;
}
int calc_exp_factor(void)
{
    int exp;
    int r_exp = _race_exp_factor();
    int c_exp = get_class()->exp;
    int a_exp = get_personality()->exp;

    if (p_ptr->prace == RACE_ANDROID)
        return r_exp;

    exp = r_exp * c_exp / 100;
    exp = exp * a_exp / 100;

    if (p_ptr->prace == RACE_MON_DRAGON)
    {
        dragon_realm_ptr realm = dragon_get_realm(p_ptr->dragon_realm);
        exp = exp * realm->exp / 100;
    }

    if (coffee_break == SPEED_INSTA_COFFEE) /* Does not really lend itself to extreme XP mods - flatten the scale a bit */
    {
        exp = ((exp + 50) * 2 / 3);
    }

    return exp;
}

static void e_info_reset(bool empty)
{
    int i, j;
    if (!empty) return;
    for (i = 1; i < max_e_idx; i++)
    {
        ego_type *e_ptr = &e_info[i];
        for (j = 0; j < OF_ARRAY_SIZE; j++) e_ptr->known_flags[j] = 0;
    }
}

static void k_info_reset(bool empty)
{
    int i;

    /* Reset the "objects" */
    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        k_ptr->tried = FALSE;
        k_ptr->aware = FALSE;
    }
}

static void a_info_reset(bool empty)
{
   int i;
   /* Start with no artifacts made yet */
    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        a_ptr->generated = FALSE;
        a_ptr->found = FALSE;
        if (empty)
        {
            int j;
            for (j = 0; j < OF_ARRAY_SIZE; j++) a_ptr->known_flags[j] = 0;
        }
    }
}

void empty_lore_wipe(void)
{
    e_info_reset(TRUE);
    k_info_reset(TRUE);
    a_info_reset(TRUE);
}

/*
 * Clear all the global "character" data
 */
static void player_wipe(void)
{
    int i;

    /* Hack -- free the "last message" string */
    if (p_ptr->last_message) z_string_free(p_ptr->last_message);

    /* Hack -- zero the struct */
    (void)WIPE(p_ptr, player_type);
    p_ptr->mimic_form = MIMIC_NONE;


    /* Reset the objects */
    a_info_reset(FALSE);
    k_info_reset(FALSE);
    e_info_reset(FALSE);
    stats_reset();

    /* Reset the "monsters" */
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Hack -- Reset the counter */
        r_ptr->cur_num = 0;
        r_ptr->ball_num = 0;

        /* Hack -- Reset the max counter */
        r_ptr->max_num = 100;
        r_ptr->flagsx = 0;

        /* Hack -- Reset the max counter */
        if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 1;

        /* Hack -- Non-unique Nazguls are semi-unique */
        else if (r_ptr->flags7 & RF7_NAZGUL) r_ptr->max_num = MAX_NAZGUL_NUM;
        else if (i == MON_CAMELOT_KNIGHT) r_ptr->max_num = MAX_CAMELOT_KNIGHT_NUM;

        /* Clear visible kills in this life */
        r_ptr->r_pkills = 0;

        /* Clear all kills in this life */
        r_ptr->r_akills = 0;
        r_ptr->r_skills = 0;
        r_ptr->stolen_ct = 0;

        /* Wipe out pact alliances from previous character
           Currently, flagsr is only set to make the memory field
           work, but perhaps it would be better to set this once
           and for all when a pact is made?  This would break
           my savefiles though ...*/
        r_ptr->flagsr &= ~(RFR_PACT_MONSTER);
        r_ptr->r_flagsr &= ~(RFR_PACT_MONSTER);
    }


    /* Hack -- Well fed player */
    p_ptr->food = PY_FOOD_FULL - 1;


    /* Wipe the spells */
    if (p_ptr->pclass == CLASS_SORCERER)
    {
        p_ptr->spell_learned1 = p_ptr->spell_learned2 = 0xffffffffL;
        p_ptr->spell_worked1 = p_ptr->spell_worked2 = 0xffffffffL;
    }
    else
    {
        p_ptr->spell_learned1 = p_ptr->spell_learned2 = 0L;
        p_ptr->spell_worked1 = p_ptr->spell_worked2 = 0L;
    }
    p_ptr->spell_forgotten1 = p_ptr->spell_forgotten2 = 0L;
    for (i = 0; i < 64; i++) p_ptr->spell_order[i] = 99;
    p_ptr->learned_spells = 0;
    p_ptr->add_spells = 0;
    p_ptr->knowledge = 0;

    /* Clean the mutation count */
    mutant_regenerate_mod = 100;

    /* Clear "cheat" options */
    cheat_peek = FALSE;
    cheat_hear = FALSE;
    cheat_room = FALSE;
    cheat_xtra = FALSE;
    cheat_live = FALSE;
    cheat_save = FALSE;

    /* Assume no winning game */
    p_ptr->total_winner = FALSE;

    world_player = FALSE;

    /* Assume no panic save */
    p_ptr->panic_save = 0;

    /* Assume no cheating */
    p_ptr->noscore = 0;
    p_ptr->wizard = FALSE;

    /* Assume no upkeep warning */
    p_ptr->upkeep_warning = FALSE;

    /* No coffee revisits yet */
    p_ptr->coffee_lv_revisits = 0;

    /* No minislow */
    p_ptr->minislow = 0;
    p_ptr->mini_energy = 0;

    /* No player summoned monsters killed */
    p_ptr->py_summon_kills = 0;

    /* Not waiting to report score */
    p_ptr->wait_report_score = FALSE;

    /* Default pet command settings */
    p_ptr->pet_follow_distance = PET_FOLLOW_DIST;
    p_ptr->pet_extra_flags = (PF_TELEPORT | PF_ATTACK_SPELL | PF_SUMMON_SPELL | PF_HILITE_LISTS);

    /* Wipe the recall depths */
    for (i = 0; i < max_d_idx; i++)
    {
        max_dlv[i] = 0;
        dungeon_flags[i] = 0;
    }

    p_ptr->wild_mode = FALSE;

    for (i = 0; i < MAX_MAGIC_NUM; i++)
    {
        p_ptr->magic_num1[i] = 0;
        p_ptr->magic_num2[i] = 0;
    }

    /* Level one */
    p_ptr->max_plv = p_ptr->lev = 1;
    p_ptr->clp = 1000;

    /* Initialize arena and rewards information -KMW- */
    p_ptr->arena_number = 0;
    p_ptr->inside_arena = FALSE;
    for (i = 0; i < MAX_MANE; i++)
    {
        p_ptr->mane_spell[i] = -1;
        p_ptr->mane_dam[i] = 0;
    }
    p_ptr->mane_num = 0;
    p_ptr->exit_bldg = TRUE; /* only used for arena now -KMW- */

    /* Bounty */
    p_ptr->today_mon = 0;

    /* Reset monster arena */
    battle_monsters();

    /* Reset mutations */
    for (i = 0; i < MUT_FLAG_SIZE; ++i)
    {
        p_ptr->muta[i] = 0;
        p_ptr->muta_lock[i] = 0;
    }

    for (i = 0; i < MAX_DEMIGOD_POWERS; ++i)
        p_ptr->demigod_power[i] = -1;

    p_ptr->draconian_power = -1;

    p_ptr->duelist_target_idx = 0;

    /* Reset virtues*/
    for (i = 0; i < 8; i++) p_ptr->virtues[i]=0;
}

/*
 * Reset turn
 */
static void init_turn(void)
{
    if ( p_ptr->prace == RACE_VAMPIRE
      || p_ptr->prace == RACE_MON_VAMPIRE
      || p_ptr->prace == RACE_SKELETON
      || p_ptr->prace == RACE_ZOMBIE
      || p_ptr->prace == RACE_SPECTRE )
    {
        /* Undead start just after midnight */
        game_turn = (TURNS_PER_TICK*3 * TOWN_DAWN) / 4 + 1;
        game_turn_limit = TURNS_PER_TICK * TOWN_DAWN * MAX_DAYS + TURNS_PER_TICK * TOWN_DAWN * 3 / 4;
    }
    else
    {
        game_turn = 1;
        game_turn_limit = TURNS_PER_TICK * TOWN_DAWN * (MAX_DAYS - 1) + TURNS_PER_TICK * TOWN_DAWN * 3 / 4;
    }

    dungeon_turn = 1;
    dungeon_turn_limit = TURNS_PER_TICK * TOWN_DAWN * (MAX_DAYS - 1) + TURNS_PER_TICK * TOWN_DAWN * 3 / 4;
}

/*
 * Hook function for human corpses
 */
bool monster_hook_human(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    if (r_ptr->flags1 & (RF1_UNIQUE)) return FALSE;

    if (my_strchr("pht", r_ptr->d_char)) return TRUE;

    return FALSE;
}

cptr birth_get_realm_desc(int i)
{
    return realm_jouhou[i-1];
}

void birth_location(void)
{
    if (no_wilderness)
    {
        dungeon_type = 0;
        p_ptr->recall_dungeon = DUNGEON_ANGBAND;
    }
    else
    {
        dungeon_type = 0;
        p_ptr->recall_dungeon = DUNGEON_WARREN;
    }

    if (thrall_mode)
    {
        dun_level = 90;
        if (!no_wilderness)
        {
            dungeon_type = DUNGEON_CTH;
            p_ptr->recall_dungeon = DUNGEON_CTH;
            p_ptr->wilderness_y = d_info[dungeon_type].dy;
            p_ptr->wilderness_x = d_info[dungeon_type].dx;
        }
        else
        {
            dungeon_type = DUNGEON_ANGBAND;
        }
        max_dlv[dungeon_type] = 90;
    }
}

/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
bool birth_hack = FALSE;
void player_birth(void)
{
    birth_hack = TRUE;
    playtime = 0;

    wipe_m_list();
    player_wipe();

    /* Create a new character */
    if (py_birth() != UI_OK)
        quit(NULL);

    /* Here's a bunch of crap that py_birth() shouldn't need to know */
    init_turn();

    /* Generate the random seeds for the wilderness */
    seed_wilderness();

    birth_hack = FALSE;
}

