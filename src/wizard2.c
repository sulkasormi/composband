/* File: wizard2.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Wizard commands */

#include "angband.h"

#include <assert.h>

/* Statistics: Use the wizard commands '-' and '=' to gather statistics.
   The Wizard command '"' and 'A' will then show all found artifacts,
   including rand-arts. The character sheet will show statistics on object
   distributions and key resources. The object info commands '~' for egos 'e'
   and objects 'o' are also useful. Be sure to begin each statistics run
   with a fresh, newly created character.*/
bool statistics_hack = FALSE;
static vec_ptr _rand_arts = NULL;
static vec_ptr _egos = NULL;

vec_ptr stats_rand_arts(void)
{
    if (!_rand_arts)
        _rand_arts = vec_alloc(free);
    return _rand_arts;
}

vec_ptr stats_egos(void)
{
    if (!_egos)
        _egos = vec_alloc(free);
    return _egos;
}

void stats_add_rand_art(object_type *o_ptr)
{
    if (o_ptr->art_name)
    {
        object_type *copy = malloc(sizeof(object_type));
        *copy = *o_ptr;
        no_karrot_hack = TRUE;
        obj_identify_fully(copy);
        no_karrot_hack = FALSE;
        vec_add(stats_rand_arts(), copy);
    }
}

void stats_add_ego(object_type *o_ptr)
{
    if (o_ptr->name2)
    {
        object_type *copy = malloc(sizeof(object_type));
        *copy = *o_ptr;
        obj_identify_fully(copy);
        vec_add(stats_egos(), copy);
    }
}

typedef struct {
    int total;
    int count;
} _tally_t;
static _tally_t _monster_levels[MAX_DEPTH];
static _tally_t _object_levels[MAX_DEPTH];
static int      _object_histogram[MAX_DEPTH];

static void _stats_reset_monster_levels(void)
{
    int i;
    for (i = 0; i < MAX_DEPTH; i++)
    {
        _monster_levels[i].total = 0;
        _monster_levels[i].count = 0;
    }
}

static void _stats_note_monster_level(int dlvl, int mlvl)
{
    if (0 <= dlvl && dlvl < MAX_DEPTH)
    {
        _monster_levels[dlvl].total += mlvl;
        _monster_levels[dlvl].count++;
    }
}

static void _stats_reset_object_levels(void)
{
    int i;
    for (i = 0; i < MAX_DEPTH; i++)
    {
        _object_levels[i].total = 0;
        _object_levels[i].count = 0;

        _object_histogram[i] = 0;
    }
}

static void _stats_note_object_level(int dlvl, int olvl)
{
    if (0 <= dlvl && dlvl < MAX_DEPTH)
    {
        _object_levels[dlvl].total += olvl;
        _object_levels[dlvl].count++;
    }

    if (0 <= olvl && olvl < MAX_DEPTH)
        _object_histogram[olvl]++;
}


/*
 * Strip an "object name" into a buffer
 */
void strip_name_aux(char *dest, const char *src)
{
    char *t;

    /* Skip past leading characters */
    while (*src == ' ' || *src == '&' || *src == '[')
        src++;

    /* Copy useful chars */
    for (t = dest; *src; src++)
    {
        if (*src != '~' && *src != ']')
            *t++ = *src;
    }

    *t = '\0';
}

void strip_name(char *buf, int k_idx)
{
    strip_name_aux(buf, k_name + k_info[k_idx].name);
}

int _life_rating_aux(int lvl)
{
    return (p_ptr->player_hp[lvl-1]-100) * 100 / (50*(lvl-1));
}

int life_rating(void)
{
    return _life_rating_aux(PY_MAX_LEVEL);
}

void do_cmd_rerate_aux(void)
{
    for(;;)
    {
        int i, pct;
        p_ptr->player_hp[0] = 100;

        for (i = 1; i < PY_MAX_LEVEL; i++)
            p_ptr->player_hp[i] = p_ptr->player_hp[i - 1] + randint1(100);

        /* These extra early checks give a slight boost to average life ratings (~102%) */
        pct = _life_rating_aux(5);
        if (pct < 87) continue;

        pct = _life_rating_aux(10);
        if (pct < 87) continue;

        pct = _life_rating_aux(25);
        if (pct < 87) continue;

        pct = life_rating();
        if (87 <= pct && pct <= 117) break;
    }
}

void do_cmd_rerate(bool display)
{
    do_cmd_rerate_aux();

    p_ptr->update |= (PU_HP);
    p_ptr->redraw |= (PR_HP);
    handle_stuff();

    if (display)
    {
        msg_format("Your life rate is %d/100 now.", life_rating());
        p_ptr->knowledge |= KNOW_HPRATE;
    }
    else
    {
        msg_print("Life rate is changed.");
        p_ptr->knowledge &= ~(KNOW_HPRATE);
    }
}


#ifdef ALLOW_WIZARD

/*
 * Dimension Door
 */
static bool wiz_dimension_door(void)
{
    int    x = 0, y = 0;

    if (!tgt_pt(&x, &y, -1)) return FALSE;

    teleport_player_to(y, x, TELEPORT_NONMAGICAL);

    return (TRUE);
}


/*
 * Create the artifact of the specified number -- DAN
 *
 */
static void wiz_create_named_art(int a_idx)
{
    if (create_named_art(a_idx, py, px, ORIGIN_CHEAT, 0))
        a_info[a_idx].generated = TRUE;
}

#ifdef MONSTER_HORDES

/* Summon a horde of monsters */
static void do_cmd_summon_horde(void)
{
    int wy = py, wx = px;
    int attempts = 1000;

    while (--attempts)
    {
        scatter(&wy, &wx, py, px, 3, 0);
        if (cave_empty_bold(wy, wx)) break;
    }

    (void)alloc_horde(wy, wx);
}

#endif /* MONSTER_HORDES */

/*
 * Hack -- Teleport to the target
 */
static void do_cmd_wiz_bamf(void)
{
    /* Must have a target */
    if (!target_who) return;

    /* Teleport to the target */
    teleport_player_to(target_row, target_col, TELEPORT_NONMAGICAL);
}


/*
 * Aux function for "do_cmd_wiz_change()".   -RAK-
 */
static void do_cmd_wiz_change_aux(void)
{
    int i, j;
    int tmp_int;
    long tmp_long;
    s16b tmp_s16b;
    char tmp_val[160];
    char ppp[80];


    /* Query the stats */
    for (i = 0; i < 6; i++)
    {
        /* Prompt */
        sprintf(ppp, "%s (3-%d): ", stat_names[i], p_ptr->stat_max_max[i]);

        /* Default */
        sprintf(tmp_val, "%d", p_ptr->stat_max[i]);

        /* Query */
        if (!get_string(ppp, tmp_val, 3)) return;

        /* Extract */
        tmp_int = atoi(tmp_val);

        /* Verify */
        if (tmp_int > p_ptr->stat_max_max[i]) tmp_int = p_ptr->stat_max_max[i];
        else if (tmp_int < 3) tmp_int = 3;

        /* Save it */
        p_ptr->stat_cur[i] = p_ptr->stat_max[i] = tmp_int;
    }


    /* Default */
    sprintf(tmp_val, "%d", WEAPON_EXP_MASTER);

    /* Query */
    if (!get_string("Proficiency: ", tmp_val, 9)) return;

    /* Extract */
    tmp_s16b = atoi(tmp_val);

    /* Verify */
    if (tmp_s16b < WEAPON_EXP_UNSKILLED) tmp_s16b = WEAPON_EXP_UNSKILLED;
    if (tmp_s16b > WEAPON_EXP_MASTER) tmp_s16b = WEAPON_EXP_MASTER;

    for (j = 0; j <= TV_WEAPON_END - TV_WEAPON_BEGIN; j++)
    {
        for (i = 0;i < 64;i++)
        {
            int max = skills_weapon_max(TV_WEAPON_BEGIN + j, i);
            p_ptr->weapon_exp[j][i] = tmp_s16b;
            if (p_ptr->weapon_exp[j][i] > max) p_ptr->weapon_exp[j][i] = max;
        }
    }

    for (j = 0; j < 10; j++)
    {
        p_ptr->skill_exp[j] = tmp_s16b;
        if (p_ptr->skill_exp[j] > s_info[p_ptr->pclass].s_max[j]) p_ptr->skill_exp[j] = s_info[p_ptr->pclass].s_max[j];
    }

    /* Hack for WARLOCK_DRAGONS. Of course, reading skill tables directly is forbidden, so this code is inherently wrong! */
    p_ptr->skill_exp[SKILL_RIDING] = MIN(skills_riding_max(), tmp_s16b);

    for (j = 0; j < 32; j++)
        p_ptr->spell_exp[j] = (tmp_s16b > SPELL_EXP_MASTER ? SPELL_EXP_MASTER : tmp_s16b);
    for (; j < 64; j++)
        p_ptr->spell_exp[j] = (tmp_s16b > SPELL_EXP_EXPERT ? SPELL_EXP_EXPERT : tmp_s16b);

    /* Default */
    sprintf(tmp_val, "%d", p_ptr->au);

    /* Query */
    if (!get_string("Gold: ", tmp_val, 9)) return;

    /* Extract */
    tmp_long = atol(tmp_val);

    /* Verify */
    if (tmp_long < 0) tmp_long = 0L;

    /* Save */
    p_ptr->au = tmp_long;


    /* Default */
    sprintf(tmp_val, "%d", p_ptr->max_exp);

    /* Query */
    if (!get_string("Experience: ", tmp_val, 9)) return;

    /* Extract */
    tmp_long = atol(tmp_val);

    /* Verify */
    if (tmp_long < 0) tmp_long = 0L;

    if (p_ptr->prace != RACE_ANDROID)
    {
        /* Save */
        p_ptr->max_exp = tmp_long;
        p_ptr->exp = tmp_long;

        /* Update */
        check_experience();
    }

    sprintf(tmp_val, "%d", p_ptr->fame);
    if (!get_string("Fame: ", tmp_val, 3)) return;
    tmp_long = atol(tmp_val);
    if (tmp_long < 0) tmp_long = 0L;
    p_ptr->fame = (s16b)tmp_long;
}


/*
 * Change various "permanent" player variables.
 */
static void do_cmd_wiz_change(void)
{
    /* Interact */
    do_cmd_wiz_change_aux();

    /* Redraw everything */
    do_cmd_redraw();
}

/* Blue-Mage - learn spells for free */
static void do_cmd_wiz_blue_mage(void)
{
    int n = get_quantity("Which type? ", MST_COUNT - 1);
    int e = get_quantity("Which effect? ", 200);
    blue_mage_learn_spell_aux(n, e, 0, 0, TRUE);
}

/*
 * A structure to hold a tval and its description
 */
typedef struct tval_desc
{
    int        tval;
    cptr       desc;
} tval_desc;

/*
 * A list of tvals and their textual names
 */
static tval_desc tvals[] =
{
    { TV_SWORD,             "Sword"                },
    { TV_POLEARM,           "Polearm"              },
    { TV_HAFTED,            "Hafted Weapon"        },
    { TV_BOW,               "Bow"                  },
    { TV_ARROW,             "Arrows"               },
    { TV_BOLT,              "Bolts"                },
    { TV_SHOT,              "Shots"                },
    { TV_SHIELD,            "Shield"               },
    { TV_CROWN,             "Crown"                },
    { TV_HELM,              "Helm"                 },
    { TV_GLOVES,            "Gloves"               },
    { TV_BOOTS,             "Boots"                },
    { TV_CLOAK,             "Cloak"                },
    { TV_DRAG_ARMOR,        "Dragon Scale Mail"    },
    { TV_HARD_ARMOR,        "Hard Armor"           },
    { TV_SOFT_ARMOR,        "Soft Armor"           },
    { TV_RING,              "Ring"                 },
    { TV_AMULET,            "Amulet"               },
    { TV_LITE,              "Lite"                 },
    { TV_POTION,            "Potion"               },
    { TV_SCROLL,            "Scroll"               },
    { TV_WAND,              "Wand"                 },
    { TV_STAFF,             "Staff"                },
    { TV_ROD,               "Rod"                  },
    { TV_LIFE_BOOK,         "Life Spellbook"       },
    { TV_SORCERY_BOOK,      "Sorcery Spellbook"    },
    { TV_NATURE_BOOK,       "Nature Spellbook"     },
    { TV_CHAOS_BOOK,        "Chaos Spellbook"      },
    { TV_DEATH_BOOK,        "Death Spellbook"      },
    { TV_TRUMP_BOOK,        "Trump Spellbook"      },
    { TV_ARCANE_BOOK,       "Arcane Spellbook"     },
    { TV_CRAFT_BOOK,        "Craft Spellbook"},
    { TV_DAEMON_BOOK,       "Daemon Spellbook"},
    { TV_CRUSADE_BOOK,      "Crusade Spellbook"},
    { TV_NECROMANCY_BOOK,   "Necromancy Spellbook"},
    { TV_ARMAGEDDON_BOOK,   "Armageddon Spellbook"},
    { TV_LAW_BOOK,          "Law Spellbook"        },
    { TV_MUSIC_BOOK,        "Music Spellbook"      },
    { TV_HISSATSU_BOOK,     "Book of Kendo"        },
    { TV_HEX_BOOK,          "Hex Spellbook"        },
    { TV_RAGE_BOOK,         "Rage Spellbook"       },
    { TV_BURGLARY_BOOK,     "Thieves' Guide"       },
    { TV_PARCHMENT,         "Parchment" },
    { TV_WHISTLE,           "Whistle"    },
    { TV_QUIVER,            "Quiver"               },
    { TV_SPIKE,             "Spikes"               },
    { TV_DIGGING,           "Digger"               },
    { TV_CHEST,             "Chest"                },
    { TV_CAPTURE,           "Capture Ball"         },
    { TV_CARD,              "Express Card"         },
    { TV_FIGURINE,          "Magical Figurine"     },
    { TV_STATUE,            "Statue"               },
    { TV_CORPSE,            "Corpse"               },
    { TV_FOOD,              "Food"                 },
    { TV_FLASK,             "Flask"                },
    { TV_BOTTLE,            "Bottle"               },
    { TV_JUNK,              "Junk"                 },
    { TV_SKELETON,          "Skeleton"             },
    { 0,                    NULL                   }
};



/*
 * Specify tval and sval (type and subtype of object) originally
 * by RAK, heavily modified by -Bernd-
 *
 * This function returns the k_idx of an object type, or zero if failed
 *
 * List up to 50 choices in three columns
 */
static int wiz_create_itemtype(void)
{
    int i, num, max_num, lvl;
    int col, row;
    int tval;

    cptr tval_desc;
    char ch;

    int choice[120];

    char buf[160];


    /* Clear screen */
    Term_clear();

    /* Print all tval's and their descriptions */
    for (num = 0; (num < 80) && tvals[num].tval; num++)
    {
        row = 2 + (num % 30);
        col = 30 * (num / 30);
        ch = listsym[num];
        prt(format("[%c] %s", ch, tvals[num].desc), row, col);
    }

    /* Me need to know the maximal possible tval_index */
    max_num = num;

    /* Choose! */
    if (!get_com("Get what type of object? ", &ch, FALSE)) return (0);

    /* Analyze choice */
    for (num = 0; num < max_num; num++)
    {
        if (listsym[num] == ch) break;
    }

    /* Bail out if choice is illegal */
    if ((num < 0) || (num >= max_num)) return (0);

    /* Base object type chosen, fill in tval */
    tval = tvals[num].tval;
    tval_desc = tvals[num].desc;


    /*** And now we go for k_idx ***/

    /* Clear screen */
    Term_clear();

    /* We have to search the whole itemlist. */
    num = 0;
    for (lvl = 0; lvl <= 120 && num < 120; lvl++) /* Who cares if this is slow. But order the choices please!! */
    {
        for (i = 1; i < max_k_idx && num < 120; i++)
        {
            object_kind *k_ptr = &k_info[i];

            /* Analyze matching items */
            if (k_ptr->tval == tval && k_ptr->level == lvl)
            {
                /* Prepare it */
                row = 2 + (num % 30);
                col = 30 * (num / 30);
                ch = listsym[num];
                strcpy(buf,"                    ");

                /* Acquire the "name" of object "i" */
                strip_name(buf, i);

                /* Print it */
                if (k_ptr->max_level)
                    prt(format("[%c] %s (L%d-%d)", ch, buf, lvl, k_ptr->max_level), row, col);
                else
                    prt(format("[%c] %s (L%d-*)", ch, buf, lvl), row, col);

                /* Remember the object index */
                choice[num++] = i;
            }
        }
    }

    /* Me need to know the maximal possible remembered object_index */
    max_num = num;

    /* Choose! */
    if (!get_com(format("What Kind of %s? ", tval_desc), &ch, FALSE)) return (0);

    /* Analyze choice */
    for (num = 0; num < max_num; num++)
    {
        if (listsym[num] == ch) break;
    }

    /* Bail out if choice is "illegal" */
    if ((num < 0) || (num >= max_num)) return (0);

    /* And return successful */
    return (choice[num]);
}

/*
 * Wizard routine for creating objects        -RAK-
 * Heavily modified to allow magification and artifactification  -Bernd-
 *
 * Note that wizards cannot create objects on top of other objects.
 *
 * Hack -- this routine always makes a "dungeon object", and applies
 * magic to it, and attempts to decline cursed items.
 */
static void wiz_create_item(void)
{
    object_type    forge;
    object_type *q_ptr;
    int n = 1;

    int k_idx;


    /* Save the screen */
    screen_save();

    /* Get object base type */
    k_idx = wiz_create_itemtype();

    /* Restore the screen */
    screen_load();


    /* Return if failed */
    if (!k_idx) return;

    if (k_info[k_idx].gen_flags & OFG_INSTA_ART)
    {
        int i;

        /* Artifactify */
        for (i = 1; i < max_a_idx; i++)
        {
            /* Ignore incorrect tval */
            if (a_info[i].tval != k_info[k_idx].tval) continue;

            /* Ignore incorrect sval */
            if (a_info[i].sval != k_info[k_idx].sval) continue;

            /* Create this artifact */
            if (create_named_art(i, py, px, ORIGIN_CHEAT, 0))
                a_info[i].generated = TRUE;

            /* All done */
            msg_print("Allocated(INSTA_ART).");

            return;
        }
    }
    else if (k_info[k_idx].tval == TV_CORPSE) /* Possessor Testing! */
    {
        char buf[81];
        buf[0] = 0;
        if (msg_input("Which monster? ", buf, 80))
        {
            n = parse_lookup_monster(buf, 0);
            if (!n) n = atoi(buf);
        }
    }
    else
    {
        switch (k_info[k_idx].tval)
        {
        case TV_WAND: case TV_ROD: case TV_STAFF:
            n = 1;
            break;
        default:
            n = get_quantity("How many? ", 99);
        }
    }

    /* Get local object */
    q_ptr = &forge;

    /* Create the item */
    object_prep(q_ptr, k_idx);

    /* Apply magic */
    apply_magic(q_ptr, dun_level, AM_NO_FIXED_ART);
    if (k_info[k_idx].tval == TV_CORPSE)
    {
        if ((k_info[k_idx].sval >= SV_BODY_HEAD) && (k_info[k_idx].sval < SV_BODY_EARS)) q_ptr->xtra4 = n;
        else q_ptr->pval = n;
    }
    else
        q_ptr->number = n;

    object_origins(q_ptr, ORIGIN_CHEAT);

    /* Drop the object from heaven */
    (void)drop_near(q_ptr, -1, py, px);

    /* All done */
    msg_print("Allocated.");
}


/*
 * Cure everything instantly
 */
static void do_cmd_wiz_cure_all(void)
{
    /* Restore stats */
    (void)res_stat(A_STR);
    (void)res_stat(A_INT);
    (void)res_stat(A_WIS);
    (void)res_stat(A_CON);
    (void)res_stat(A_DEX);
    (void)res_stat(A_CHR);

    /* Restore the level */
    (void)restore_level();

    /* Heal the player */
    if (p_ptr->chp < p_ptr->mhp)
    {
        p_ptr->chp = p_ptr->mhp;
        p_ptr->chp_frac = 0;

        /* Redraw */
        p_ptr->redraw |= (PR_HP);
    }

    /* Restore mana */
    if (p_ptr->csp < p_ptr->msp)
    {
        if (!elemental_is_(ELEMENTAL_WATER)) p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;

        p_ptr->redraw |= (PR_MANA);
        p_ptr->window |= (PW_SPELL);
    }

    /* Cure stuff */
    (void)set_blind(0, TRUE);
    (void)set_confused(0, TRUE);
    (void)set_poisoned(0, TRUE);
    fear_clear_p();
    (void)set_paralyzed(0, TRUE);
    (void)set_image(0, TRUE);
    (void)set_stun(0, TRUE);
    (void)set_cut(0, TRUE);
    (void)set_slow(0, TRUE);
    (void)p_inc_minislow(-10);
    (void)set_unwell(0, TRUE);

    /* No longer hungry
    (void)set_food(PY_FOOD_MAX - 1);*/
}


/*
 * Go to any level
 */
static void do_cmd_wiz_jump(void)
{
    /* Ask for level */
    if (command_arg <= 0)
    {
        char    ppp[80];

        char    tmp_val[160];
        int        tmp_dungeon_type;

        /* Prompt */
        sprintf(ppp, "Jump which dungeon : ");

        /* Default */
        sprintf(tmp_val, "%d", dungeon_type);

        /* Ask for a level */
        if (!get_string(ppp, tmp_val, 2)) return;

        tmp_dungeon_type = atoi(tmp_val);
        if (!d_info[tmp_dungeon_type].maxdepth || (tmp_dungeon_type > max_d_idx)) tmp_dungeon_type = DUNGEON_ANGBAND;

        /* Prompt */
        sprintf(ppp, "Jump to level (0, %d-%d): ", d_info[tmp_dungeon_type].mindepth, d_info[tmp_dungeon_type].maxdepth);

        /* Default */
        sprintf(tmp_val, "%d", dun_level);

        /* Ask for a level */
        if (!get_string(ppp, tmp_val, 10)) return;

        /* Extract request */
        command_arg = atoi(tmp_val);

        dungeon_type = tmp_dungeon_type;
    }

    /* Paranoia */
    if (command_arg < d_info[dungeon_type].mindepth) command_arg = 0;

    /* Paranoia */
    if (command_arg > d_info[dungeon_type].maxdepth) command_arg = d_info[dungeon_type].maxdepth;

    /* Accept request */
    msg_format("<color:U>You jump to dungeon level %d:</color>", command_arg);

    if (autosave_l) do_cmd_save_game(TRUE);

    /* Change level */
    dun_level = command_arg;

    prepare_change_floor_mode(CFM_RAND_PLACE);

    if (!dun_level) dungeon_type = 0;
    p_ptr->inside_arena = FALSE;
    p_ptr->wild_mode = FALSE;

    quests_on_leave();
    energy_use = 0;

    /* Prevent energy_need from being too lower than 0 */
    p_ptr->energy_need = 0;

    /*
     * Clear all saved floors
     * and create a first saved floor
     */
    prepare_change_floor_mode(CFM_FIRST_FLOOR);

    /* Leaving */
    p_ptr->leaving = TRUE;
}


/*
 * Become aware of a lot of objects
 */
static void do_cmd_wiz_learn(void)
{
    int i;

    object_type forge;
    object_type *q_ptr;

    /* Scan every object */
    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Induce awareness */
        if (k_ptr->level <= command_arg)
        {
            /* Get local object */
            q_ptr = &forge;

            /* Prepare object */
            object_prep(q_ptr, i);

            /* Awareness */
            object_aware(q_ptr);
        }
    }
}


/*
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        (void)summon_specific(0, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE));
    }
}


/*
 * Summon a creature of the specified type
 *
 * XXX XXX XXX This function is rather dangerous
 */
static void do_cmd_wiz_named(int r_idx)
{
    int x = px;
    int y = py;

    if (target_who < 0)
    {
        x = target_col;
        y = target_row;
    }

    {
        monster_race *r_ptr = &r_info[r_idx];
        if (((r_ptr->flags1 & (RF1_UNIQUE)) ||
                (r_ptr->flags7 & (RF7_NAZGUL))) &&
            (r_ptr->cur_num >= mon_available_num(r_ptr)))
        {
            r_ptr->max_num = MAX(r_ptr->max_num, 1);
            /* In the event that the summon still fails, let it fail
             * We resurrect dead uniques but not captured uniques and do
             * not create multiple copies of the unique */
        }
//        msg_format("Max: %d Cur: %d Ball: %d", r_ptr->max_num, r_ptr->cur_num, r_ptr->ball_num);
    }


    (void)summon_named_creature(0, y, x, r_idx, (PM_ALLOW_SLEEP | PM_ALLOW_GROUP));
}


/*
 * Summon a creature of the specified type
 *
 * XXX XXX XXX This function is rather dangerous
 */
static void do_cmd_wiz_named_friendly(int r_idx)
{
    (void)summon_named_creature(0, py, px, r_idx, (PM_ALLOW_SLEEP | PM_ALLOW_GROUP | PM_FORCE_PET));
}



/*
 * Hack -- Delete all nearby monsters
 */
static void do_cmd_wiz_zap(void)
{
    int i;


    /* Genocide everyone nearby */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip the mount */
        if (i == p_ptr->riding) continue;

        /* Delete nearby monsters */
        if (m_ptr->cdis <= MAX_SIGHT)
        {
            bool fear = FALSE;
            mon_take_hit(i, m_ptr->hp + 1, DAM_TYPE_WIZARD, &fear, NULL);
            /*delete_monster_idx(i);*/
        }
    }
}


/*
 * Hack -- Delete all monsters
 */
static void do_cmd_wiz_zap_all(void)
{
    int i;

    /* Genocide everyone */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip the mount */
        if (i == p_ptr->riding) continue;

        delete_monster_idx(i);
    }
}


/*
 * Create desired feature
 */
static void do_cmd_wiz_create_feature(void)
{
    static int   prev_feat = 0;
    static int   prev_mimic = 0;
    cave_type    *c_ptr;
    feature_type *f_ptr;
    char         tmp_val[160];
    int          tmp_feat, tmp_mimic;
    int          y, x;

    if (!tgt_pt(&x, &y, -1)) return;

    c_ptr = &cave[y][x];

    /* Default */
    sprintf(tmp_val, "%d", prev_feat);

    /* Query */
    if (!get_string("Feature: ", tmp_val, 3)) return;

    /* Extract */
    tmp_feat = atoi(tmp_val);
    if (tmp_feat < 0) tmp_feat = 0;
    else if (tmp_feat >= max_f_idx) tmp_feat = max_f_idx - 1;

    /* Default */
    sprintf(tmp_val, "%d", prev_mimic);

    /* Query */
    if (!get_string("Feature (mimic): ", tmp_val, 3)) return;

    /* Extract */
    tmp_mimic = atoi(tmp_val);
    if (tmp_mimic < 0) tmp_mimic = 0;
    else if (tmp_mimic >= max_f_idx) tmp_mimic = max_f_idx - 1;

    cave_set_feat(y, x, tmp_feat);
    c_ptr->mimic = tmp_mimic;

    f_ptr = &f_info[get_feat_mimic(c_ptr)];

    if (have_flag(f_ptr->flags, FF_GLYPH) ||
        have_flag(f_ptr->flags, FF_MON_TRAP))
        c_ptr->info |= (CAVE_OBJECT);
    else if (have_flag(f_ptr->flags, FF_MIRROR))
        c_ptr->info |= (CAVE_GLOW | CAVE_OBJECT);

    /* Notice */
    note_spot(y, x);

    /* Redraw */
    lite_spot(y, x);

    /* Update some things */
    p_ptr->update |= (PU_FLOW);

    prev_feat = tmp_feat;
    prev_mimic = tmp_mimic;
}

/*************************************************************************
 * Wizard Stats
 ************************************************************************/
static doc_ptr _wiz_doc = NULL;
static bool    _wiz_show_scores = TRUE;
static int     _wiz_obj_count = 0;
static int     _wiz_obj_score = 0;

static void _wiz_stats_begin(void)
{
    _wiz_doc = doc_alloc(120);
    doc_insert(_wiz_doc, "<style:table>");
    _wiz_obj_count = 0;
    _wiz_obj_score = 0;
    statistics_hack = TRUE;
}

static void _wiz_stats_end(void)
{
    doc_insert(_wiz_doc, "</style>");
    if (_wiz_obj_count)
    {
        doc_printf(_wiz_doc, "\n\n<color:R>%d</color> objects. <color:R>%d</color> average score.\n",
            _wiz_obj_count, _wiz_obj_score / _wiz_obj_count);
    }
    if (original_score)
        doc_printf(_wiz_doc, "<color:R>%d%%</color> replacement power.\n", replacement_score * 100 / original_score);
    statistics_hack = FALSE;
}

static void _wiz_stats_free(void)
{
    doc_free(_wiz_doc);
    _wiz_doc = NULL;
}

static void _wiz_stats_display(void)
{
    if (doc_line_count(_wiz_doc))
        doc_display(_wiz_doc, "Statistics", 0);

    viewport_verify();
    do_cmd_redraw();
}

static char _score_color(int score)
{
    if (score < 1000)
        return 'D';
    if (score < 10000)
        return 'w';
    if (score < 20000)
        return 'W';
    if (score < 40000)
        return 'u';
    if (score < 60000)
        return 'y';
    if (score < 80000)
        return 'o';
    if (score < 100000)
        return 'R';
    if (score < 150000)
        return 'r';
    return 'v';
}

#if 0
static void _wiz_stats_log_android(int level, object_type *o_ptr)
{
    int  score = obj_value_real(o_ptr);
    int  exp   = android_obj_exp(o_ptr);
    char name[MAX_NLEN];
    char buf[10];

    if (!_wiz_doc) return;
    if (!exp) return;

    object_desc(name, o_ptr, OD_COLOR_CODED);

    big_num_display(score, buf);
    doc_printf(_wiz_doc, "<color:%c>%s</color> ", _score_color(score), buf);

    big_num_display(exp, buf);
    doc_printf(_wiz_doc, "<color:%c>%s</color>:", _score_color(exp/10), buf);

    doc_printf(_wiz_doc, " <indent><style:indent>%s</style></indent>\n", name);
}
#endif

static void _wiz_stats_log_device(int level, object_type *o_ptr)
{
    char buf[MAX_NLEN];
    if (!_wiz_doc) return;
    object_desc(buf, o_ptr, OD_COLOR_CODED);
    _wiz_obj_count++;
    doc_printf(_wiz_doc, "C%2d D%2d O%2d P%2d D%2d: <indent><style:indent>%s</style></indent>\n",
        p_ptr->lev, level, o_ptr->level, o_ptr->activation.power, o_ptr->activation.difficulty, buf);
}
static void _wiz_stats_log_obj(int level, object_type *o_ptr)
{
    char buf[MAX_NLEN];
    if (!_wiz_doc) return;
    object_desc(buf, o_ptr, OD_COLOR_CODED);
    _wiz_obj_count++;
    if (_wiz_show_scores)
    {
        int  score;
        score = obj_value_real(o_ptr);
        _wiz_obj_score += score;
        doc_printf(_wiz_doc, "C%2d D%2d O%2d <color:%c>%6d</color>: <indent><style:indent>%s</style></indent>\n",
            p_ptr->lev, level, o_ptr->level, _score_color(score), score, buf);
    }
    else
        doc_printf(_wiz_doc, "C%2d D%2d O%2d: <indent><style:indent>%s</style></indent>\n", p_ptr->lev, level, o_ptr->level, buf);
}
static void _wiz_stats_log_speed(int level, object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_SPEED) && !object_is_artifact(o_ptr))
        _wiz_stats_log_obj(level, o_ptr);
}
static void _wiz_stats_log_books(int level, object_type *o_ptr, int max3, int max4)
{
    if (obj_is_book(o_ptr) && o_ptr->tval != TV_ARCANE_BOOK)
    {
            if ( (o_ptr->sval == 2 && k_info[o_ptr->k_idx].counts.found < max3)
              || (o_ptr->sval == 3 && k_info[o_ptr->k_idx].counts.found < max4) )
        {
            if (check_book_realm(o_ptr->tval, o_ptr->sval))
                _wiz_stats_log_obj(level, o_ptr);
        }
    }
}
static void _wiz_stats_log_devices(int level, object_type *o_ptr)
{
    #if 0
    if (o_ptr->tval == TV_WAND)
    {
        switch (o_ptr->activation.type)
        {
        case EFFECT_BALL_DISINTEGRATE:
        case EFFECT_BALL_WATER:
        case EFFECT_ROCKET:
            _wiz_stats_log_device(level, o_ptr);
            break;
        }
    }
    #endif
    #if 0
    if (o_ptr->tval == TV_STAFF)
    {
        switch (o_ptr->activation.type)
        {
        case EFFECT_HEAL_CURING:
            _wiz_stats_log_device(level, o_ptr);
            break;
        }
    }
    #endif

    #if 1
    if (obj_is_device(o_ptr))
        _wiz_stats_log_device(level, o_ptr);
    #endif
}
static void _wiz_stats_log_arts(int level, object_type *o_ptr)
{
    if (o_ptr->name1)
        _wiz_stats_log_obj(level, o_ptr);
}
static void _wiz_stats_log_rand_arts(int level, object_type *o_ptr)
{
    if (o_ptr->art_name)
        _wiz_stats_log_obj(level, o_ptr);
}
static bool _wiz_stats_skip(point_t pt)
{
    if (0 && (cave[pt.y][pt.x].info & CAVE_ICKY)) return TRUE;
    return FALSE;
}
static void _wiz_stats_kill(int level)
{
    int i;

    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;
        bool          fear = FALSE;
        int           slot = equip_find_obj(TV_SWORD, SV_RUNESWORD);

        if (!m_ptr->r_idx) continue;
        if (i == p_ptr->riding) continue;

        r_ptr = &r_info[m_ptr->r_idx];
        if (0 && r_ptr->level > level) continue;
        if (r_ptr->id == MON_DAWN) continue; /* inflates pct of humans */
        if (0 && (r_ptr->flags1 & RF1_UNIQUE) && r_ptr->level > level - 4) continue;
        if (_wiz_stats_skip(point(m_ptr->fx, m_ptr->fy))) continue;

        r_ptr->r_sights++;
        _stats_note_monster_level(level, r_ptr->level);
        mon_take_hit(i, m_ptr->hp + 1, DAM_TYPE_WIZARD, &fear, NULL);
        if (slot) rune_sword_kill(equip_obj(slot), r_ptr);
    }
}
static bool _is_stat_potion(obj_ptr obj)
{
    if (obj->tval != TV_POTION) return FALSE;
    switch (obj->sval)
    {
    case SV_POTION_INC_STR:
    case SV_POTION_INC_INT:
    case SV_POTION_INC_WIS:
    case SV_POTION_INC_DEX:
    case SV_POTION_INC_CON:
    case SV_POTION_INC_CHR: return TRUE;
    }
    return FALSE;
}

static bool _wiz_improve_gear_aux(obj_ptr obj, slot_t slot)
{
    obj_ptr old = equip_obj(slot);
    if (old)
    {
        int score, old_score;
        if (object_is_melee_weapon(old) && !object_is_melee_weapon(obj)) return FALSE;
        if (object_is_shield(old) && !object_is_shield(obj)) return FALSE;
        score = obj_value_real(obj);
        old_score = obj_value_real(old);
        if (score > old_score)
        {
            home_carry(old);
            equip_remove(slot);
            equip_wield(obj, slot);
            return TRUE;
        }
    }
    else
    {
        equip_wield(obj, slot);
        return TRUE;
    }
    return FALSE;
}

static void _wiz_improve_gear(obj_ptr obj)
{
    slot_t slot;

    /* XXX this is tedious ... */
    if (object_is_gloves(obj))
    {
        class_t *class_ptr = get_class();
        caster_info *caster = NULL;

        if (class_ptr->caster_info)
            caster = class_ptr->caster_info();

        if (caster && caster->options & CASTER_GLOVE_ENCUMBRANCE)
        {
            u32b flags[OF_ARRAY_SIZE];
            obj_flags(obj, flags);
            if ( !have_flag(flags, OF_FREE_ACT)
              && !have_flag(flags, OF_DEX)
              && !have_flag(flags, OF_MAGIC_MASTERY) )
            {
                return;
            }
        }
    }
    if (object_is_melee_weapon(obj) && skills_weapon_is_icky(obj->tval, obj->sval)) return;

    if (obj->name1 == ART_POWER || obj->name1 == ART_STONEMASK || have_flag(obj->flags, OF_NO_SUMMON)) return;
    if (obj->name2 == EGO_RING_NAZGUL) return;
    /* hydras have many heads ... */
    for (slot = equip_first_slot(obj); slot; slot = equip_next_slot(obj, slot))
    {
        if (_wiz_improve_gear_aux(obj, slot))
            break;
    }
}

static obj_ptr _pack_obj = NULL;
static bool _improve_pack_p(obj_ptr obj)
{
    assert(_pack_obj);
    return obj->tval == _pack_obj->tval && obj->sval == _pack_obj->sval;
}

static void _wiz_improve_pack(obj_ptr obj)
{
    if (obj->tval == TV_POTION || obj->tval == TV_SCROLL)
    {
        int ct;
        _pack_obj = obj;
        ct = pack_count(_improve_pack_p);
        if (ct + obj->number < 30)
            pack_carry_aux(obj);
        else
            home_carry(obj);
    }
    else if (obj_is_device(obj))
    {
        int slot = pack_find_device(obj->activation.type);

        if (!slot)
            pack_carry_aux(obj);
        else
        {
            obj_ptr old = pack_obj(slot);
            int score, old_score;
            score = obj_value_real(obj);
            old_score = obj_value_real(old);
            if (score > old_score)
            {
                home_carry(old);
                pack_remove(slot);
                pack_carry_aux(obj);
            }
        }
    }
    else if (obj_is_readable_book(obj))
    {
        int ct;
        _pack_obj = obj;
        ct = pack_count(_improve_pack_p);
        if (ct < 2)
            pack_carry_aux(obj);
    }
    else
    {
        pack_carry_aux(obj);
    }
}

static bool _device_is_(obj_ptr obj, int tval, int effect)
{
    return obj->tval == tval
        && obj->activation.type == effect;
}

static void _wiz_stats_inspect(int level)
{
    race_t  *race_ptr = get_race();
    class_t *class_ptr = get_class();
    int      i;

    no_karrot_hack = TRUE;

    for (i = 0; i < max_o_idx; i++)
    {
        object_type *o_ptr = &o_list[i];

        if (!o_ptr->k_idx) continue;
        if (o_ptr->tval == TV_GOLD)
        {
            pack_get(o_ptr);
            continue;
        }
        if (o_ptr->held_m_idx) continue;
        if (o_ptr->marked & OM_COUNTED) continue; /* skip player drops */
        if (_wiz_stats_skip(point(o_ptr->loc.x, o_ptr->loc.y))) continue;

        obj_identify_fully(o_ptr);
        stats_on_identify(o_ptr);
        if (o_ptr->level)
            _stats_note_object_level(level, o_ptr->level);

        if (o_ptr->art_name)
            stats_add_rand_art(o_ptr);

        if (o_ptr->name2)
            stats_add_ego(o_ptr);

        /* Logging: I simply hand-edit this and recompile as desired */
        if (0 && (o_ptr->name1 || o_ptr->name3))
            _wiz_stats_log_obj(level, o_ptr);

        if (0) _wiz_stats_log_speed(level, o_ptr);
        if (0) _wiz_stats_log_books(level, o_ptr, 20, 20);
        if (1) _wiz_stats_log_devices(level, o_ptr);
        if (0) _wiz_stats_log_arts(level, o_ptr);
        if (0) _wiz_stats_log_rand_arts(level, o_ptr);

        if (0 && o_ptr->name3)
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && o_ptr->name2 && !object_is_device(o_ptr) && !object_is_ammo(o_ptr))
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && !object_is_nameless(o_ptr) && o_ptr->tval == TV_BOW)
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && !object_is_nameless(o_ptr) && o_ptr->tval == TV_QUIVER)
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && !object_is_nameless(o_ptr) && object_is_ammo(o_ptr))
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && object_is_dragon_armor(o_ptr))
            _wiz_stats_log_obj(level, o_ptr);

        if (0 && _device_is_(o_ptr, TV_WAND, EFFECT_BALL_WATER))
            _wiz_stats_log_obj(level, o_ptr);

        /* Use Resources: Quaff stat potions and improve equipment (mindlessly).
         * This makes it easier for me to poke around a bit after a stat run.
         * Destroying objects is for Death-swords or other race/classes that
         * gain powers that way. */
        if (_is_stat_potion(o_ptr))
            do_device(o_ptr, SPELL_CAST, 0);

        /* Use the autopicker to 'improve' this character. For example, you can
         * conditionally have rogues only use weapons less than a certain weight
         * and only shoot slings. */
        if (o_ptr->number)
        {
            int auto_pick_idx = is_autopick(o_ptr);
            if (auto_pick_idx >= 0 && autopick_list[auto_pick_idx].action & DO_AUTOPICK)
            {
                if (object_is_wearable(o_ptr))
                    _wiz_improve_gear(o_ptr);
                else
                    _wiz_improve_pack(o_ptr);
            }
            else if ( (auto_pick_idx < 0 || !(autopick_list[auto_pick_idx].action & DO_AUTODESTROY))
                   && object_is_wearable(o_ptr) )
            {
                _wiz_improve_gear(o_ptr);
            }
        }

        if (o_ptr->number)
        {
            if (race_ptr->destroy_object)
                race_ptr->destroy_object(o_ptr);

            if (class_ptr->destroy_object)
                class_ptr->destroy_object(o_ptr);
        }
    }
    pack_overflow();
    home_optimize();
    if (p_ptr->cursed) remove_all_curse();
    no_karrot_hack = FALSE;
}
static void _wiz_stats_gather(int which_dungeon, int level, int reps)
{
    int i;
    dungeon_type = which_dungeon;
    for (i = 0; i < reps; i++)
    {
        quests_on_leave();

        dun_level = level;
        prepare_change_floor_mode(CFM_RAND_PLACE);
        energy_use = 0;
        p_ptr->energy_need = 0;
        change_floor();

        _wiz_stats_kill(level);
        _wiz_stats_inspect(level);
    }
}

/*************************************************************************
 * Handle the ^A wizard commands. Perhaps there should be a UI for this?
 ************************************************************************/
extern void do_cmd_debug(void);
void do_cmd_debug(void)
{
    int     x, y, n, repeat;
    char    cmd;

    if (REPEAT_PULL(&repeat))
        cmd = repeat;
    else
    {
        get_com("Debug Command: ", &cmd, FALSE);
        REPEAT_PUSH(cmd);
    }

    /* Analyze the command */
    switch (cmd)
    {
    /* Nothing */
    case ESCAPE:
    case ' ':
    case '\n':
    case '\r':
        break;

#ifdef ALLOW_SPOILERS

    /* Hack -- Generate Spoilers */
    case '"':
        do_cmd_spoilers();
        break;

#endif /* ALLOW_SPOILERS */

    /* Hack -- Help */
    case '?':
        do_cmd_help();
        break;

    /* Cure all maladies */
    case 'a':
        do_cmd_wiz_cure_all();
        break;

    /* Know alignment */
    case 'A':
        msg_format("Your alignment is %d.", p_ptr->align);
        break;

    /* Teleport to target */
    case 'b':
        do_cmd_wiz_bamf();
        break;

    case 'B':
        battle_monsters();
        break;

    /* Create any object */
    case 'c':
        wiz_create_item();
        break;

    /* Create a named artifact */
    case 'C':
    {
        char buf[81];
        buf[0] = 0;
        if (msg_input("Which artifact? ", buf, 80))
        {
            int idx = parse_lookup_artifact(buf, 0);
            if (!idx) idx = atoi(buf);
            wiz_create_named_art(idx);
        }
        break;
    }
    /* Detect everything */
    case 'd':
        detect_all(DETECT_RAD_ALL * 3);
        break;

    /* Dimension_door */
    case 'D':
        wiz_dimension_door();
        break;

    /* Edit character */
    case 'e':
        do_cmd_wiz_change();
        break;

    /* Blue-Mage spells */
    case 'E':
        if (p_ptr->pclass == CLASS_BLUE_MAGE) do_cmd_wiz_blue_mage();
        break;

    /* View item info */
    case 'f':
        identify_fully(NULL);
        break;

    /* Create desired feature */
    case 'F':
        do_cmd_wiz_create_feature();
        break;

    /* Good Objects */
    case 'g':
#if 1
    {
        object_type forge;
        int num = 10;

        while (num--)
        {
            object_wipe(&forge);
            if (!make_object(&forge, AM_GOOD, ORIGIN_CHEAT)) continue;
            drop_near(&forge, -1, py, px);
        }
    }
#else
        if (command_arg <= 0) command_arg = 10;
        acquirement(py, px, command_arg, FALSE, TRUE, ORIGIN_CHEAT);
#endif
        break;

    /* Hitpoint rerating */
    case 'h':
    {
        int i, r;
        int tot = 0, min = 0, max = 0;

        for (i = 0; i < 100; i++)
        {
            do_cmd_rerate_aux();
            r = life_rating();
            tot += r;
            if (!min) min = r;
            else min = MIN(min, r);
            max = MAX(max, r);
        }
        msg_format("Life Ratings: %d%% (%d%%-%d%%)", tot/100, min, max);

    /*    do_cmd_rerate(TRUE); */

        break;
    }
#ifdef MONSTER_HORDES
    case 'H':
        do_cmd_summon_horde();
        break;
#endif /* MONSTER_HORDES */

    /* Identify */
    case 'i':
        (void)identify_fully(NULL);
        break;

    case 'I':
    {
        int i, ct = 0;
        char buf[MAX_NLEN];
        no_karrot_hack = TRUE;
        for (i = 0; i < max_o_idx; i++)
        {
            if (!o_list[i].k_idx) continue;
            ct++;
            obj_identify_fully(&o_list[i]);
            if (o_list[i].name1 || o_list[i].name2)
            {
                object_desc(buf, &o_list[i], 0);
                msg_print(buf);
            }
        }
        no_karrot_hack = FALSE;
        msg_format("Objects=%d", ct);
        break;
    }

    /* Go up or down in the dungeon */
    case 'j':
        do_cmd_wiz_jump();
        break;

    /* Self-Knowledge */
    case 'k':
        self_knowledge();
        break;

    /* Learn about objects */
    case 'l':
        do_cmd_wiz_learn();
        break;

    /* Magic Mapping */
    case 'm':
        map_area(DETECT_RAD_ALL * 3);
        (void)detect_monsters_invis(255);
        (void)detect_monsters_normal(255);
        break;

    /* Mutation */
    case 'M':
    {
    /*
        for (n = 0; n < 120; n++)
            mut_gain_random(NULL);*/
    /*  mut_gain_choice(mut_demigod_pred);*/
    /*  mut_gain_choice(mut_draconian_pred); */

      n = get_quantity("Which One? ", 500);
        if (n == 500)
        {
            int i;
            for (i = 0; i < 32; ++i)
                mut_gain(i);
        }
        else
            mut_gain(n);
        break;
    }

    /* Summon _friendly_ named monster */
    case 'N':
        do_cmd_wiz_named_friendly(command_arg);
        break;

    /* Summon Named Monster */
    case 'n':
    {
        char buf[81];
        buf[0] = 0;
        if (msg_input("Which monster? ", buf, 80))
        {
            int idx = parse_lookup_monster(buf, 0);
            if (!idx) idx = atoi(buf);
            do_cmd_wiz_named(idx);
        }
        break;
    }

    /* Object playing routines */
    case 'o':
        wiz_obj_smith();
        break;

    /* Phase Door */
    case 'p':
        teleport_player(10, 0L);
        break;

    /* Wizard Probe */
    case 'P':
        if (target_who > 0)
        {
            mon_ptr mon = &m_list[target_who];
            doc_ptr doc = doc_alloc(80);
            mon_spell_wizard(mon, NULL, doc);
            doc_display(doc, "Spells", 0);
            doc_free(doc);
            do_cmd_redraw();
        }
        break;
    case 'q':
    {
        quests_wizard();
        break;
    }
    /* Summon Random Monster(s) */
    case 's':
        if (command_arg <= 0) command_arg = 1;
        do_cmd_wiz_summon(command_arg);
        break;

    case 'S':
#ifdef ALLOW_SPOILERS
        generate_spoilers();
#endif
        break;

    /* Teleport */
    case 't':
        dimension_door(255);
        /*teleport_player(100, 0L);*/
        break;

    /* Make every dungeon square "known" to test streamers -KMW- */
    case 'u':
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
                cave[y][x].info |= (CAVE_GLOW | CAVE_MARK | CAVE_AWARE);
            }
        }
        no_karrot_hack = TRUE;
        wiz_lite(FALSE);
        if (0) detect_treasure(255);
        if (1)
        {
            int i, ct = 0;
            /*char buf[MAX_NLEN];*/
            for (i = 0; i < max_o_idx; i++)
            {
                if (!o_list[i].k_idx) continue;
                if (o_list[i].tval == TV_GOLD) continue;
                ct += o_list[i].number;
                identify_item(&o_list[i]);
                obj_identify_fully(&o_list[i]);
                #if 0
                if (o_list[i].name1 || o_list[i].name2)
                {
                    object_desc(buf, &o_list[i], 0);
                    msg_print(buf);
                }
                #endif
            }
            if (0) msg_format("Objects=%d", ct);
        }
        no_karrot_hack = FALSE;
        break;


    /* Very Good Objects */
    case 'v':
        if (command_arg <= 0) command_arg = 1;
        acquirement(py, px, command_arg, TRUE, TRUE, ORIGIN_CHEAT);
        break;

    /* Wizard Light the Level */
    case 'w':
        wiz_lite(player_is_ninja);
        break;

    /* Increase Experience */
    case 'x':
        gain_exp(command_arg ? command_arg : (p_ptr->exp + 1));
        break;

    case 'X':
    {
        char tmp_val[80];
        long tmp_long;

        sprintf(tmp_val, "%d", p_ptr->exp);
        /* Query */
        if (!get_string("Experience: ", tmp_val, 9)) break;

        /* Extract */
        tmp_long = atol(tmp_val);

        /* Verify */
        if (tmp_long < 0) tmp_long = 0L;
        if (tmp_long > 99999999L) tmp_long = 99999999L;
        if (p_ptr->prace != RACE_ANDROID)
        {
            /* Save */
            p_ptr->max_exp = tmp_long;
            p_ptr->exp = tmp_long;

            /* Update */
            check_experience();
            p_ptr->max_plv = p_ptr->lev;
            p_ptr->max_max_exp = tmp_long;
            do_cmd_redraw();
        }
        break;
    }

    /* Zap Monsters (Genocide) */
    case 'z':
        do_cmd_wiz_zap();
        break;

    /* Zap Monsters (Omnicide) */
    case 'Z':
        do_cmd_wiz_zap_all();
        break;

    case '-':
    {
        /* Generate Statistics on object/monster distributions. Create a new
           character, run this command, then create a character dump
           or browse the object knowledge command (~2). Wizard commands "A and "O
           are also useful.*/
        int lev;
        int max_depth = get_quantity("Max Depth? ", 100);

        _wiz_stats_begin();
        _stats_reset_monster_levels();
        _stats_reset_object_levels();
        for (lev = MAX(1, dun_level); lev <= max_depth; lev += 1)
        {
            int reps = 1;

            if (lev % 10 == 0) reps += 1;
            if (lev % 20 == 0) reps += 1;
            if (lev % 30 == 0) reps += 2;

            _wiz_stats_gather(DUNGEON_ANGBAND, lev, reps);
        }
#if 1
        {
            _tally_t mon_total_tally = {0};
            _tally_t obj_total_tally = {0};
            int      obj_total = 0;
            int      obj_running = 0;
            int      last_lev = 0;

            for (lev = 0; lev < MAX_DEPTH; lev++)
                obj_total += _object_histogram[lev];

            doc_newline(_wiz_doc);
            doc_insert(_wiz_doc, "<color:G>Depth   Monster Level    Object Level   Object Counts</color>\n");
            for (lev = 0; lev < MAX_DEPTH; lev++)
            {
                _tally_t mon_tally = _monster_levels[lev];
                _tally_t obj_tally = _object_levels[lev];
                int      j, obj_ct = 0;

                if (!mon_tally.count || !obj_tally.count) continue;

                mon_total_tally.total += mon_tally.total;
                mon_total_tally.count += mon_tally.count;

                obj_total_tally.total += obj_tally.total;
                obj_total_tally.count += obj_tally.count;

                for (j = last_lev + 1; j <= lev; j++)
                {
                    obj_ct += _object_histogram[j];
                }
                last_lev = lev;
                obj_running += obj_ct;

                doc_printf(_wiz_doc, "%5d   %3d.%02d (%4d)    %3d.%02d (%4d)        %3d.%02d%%  %3d.%02d%%\n",
                    lev,
                    mon_tally.total / mon_tally.count,
                    (mon_tally.total*100 / mon_tally.count) % 100,
                    mon_tally.count,
                    obj_tally.total / obj_tally.count,
                    (obj_tally.total*100 / obj_tally.count) % 100,
                    obj_tally.count,
                    obj_ct*100 / obj_total,
                    (obj_ct*10000 / obj_total) % 100,
                    obj_running*100 / obj_total,
                    (obj_running*10000 / obj_total) % 100
                );
            }

            if (mon_total_tally.count && obj_total_tally.count)
            {
                doc_printf(_wiz_doc, "<color:R>        %3d.%02d (%5d)   %3d.%02d (%5d)</color>\n",
                    mon_total_tally.total / mon_total_tally.count,
                    (mon_total_tally.total*100 / mon_total_tally.count) % 100,
                    mon_total_tally.count,
                    obj_total_tally.total / obj_total_tally.count,
                    (obj_total_tally.total*100 / obj_total_tally.count) % 100,
                    obj_total_tally.count
                );
            }
            doc_newline(_wiz_doc);
        }
#endif
        _wiz_stats_end();
        _wiz_stats_display();
        _wiz_stats_free();

        break;
    }
    case '=':
    {
        /* In this version, we gather statistics on the current level of the
           current dungeon. You still want to start with a fresh character. */
        int reps = get_quantity("How many reps? ", 100);

        _wiz_stats_begin();
        _wiz_stats_gather(dungeon_type, dun_level, reps);
        _wiz_stats_end();
        _wiz_stats_display();
        _wiz_stats_free();

        break;
    }
    case '_': {
        int i;
        for (i = 0; i < 1000 * 1000; i++)
        {
            int roll = damroll(11,6);
            if (roll < 66*19/20) continue;
            msg_format("%d ", roll);
        }
        break; }
    default:
        msg_print("That is not a valid debug command.");
        break;
    }
}

#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif


