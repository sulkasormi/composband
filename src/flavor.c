/* Purpose: Object flavor code */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

#include "angband.h"

#include <assert.h>
/*
 * Certain items, if aware, are known instantly
 * This function is used only by "flavor_init()"
 */
static bool object_easy_know(int i)
{
    object_kind *k_ptr = &k_info[i];

    /* Analyze the "tval" */
    switch (k_ptr->tval)
    {
        /* Spellbooks */
        case TV_LIFE_BOOK:
        case TV_SORCERY_BOOK:
        case TV_NATURE_BOOK:
        case TV_CHAOS_BOOK:
        case TV_DEATH_BOOK:
        case TV_TRUMP_BOOK:
        case TV_ARCANE_BOOK:
        case TV_CRAFT_BOOK:
        case TV_DAEMON_BOOK:
        case TV_CRUSADE_BOOK:
        case TV_NECROMANCY_BOOK:
        case TV_ARMAGEDDON_BOOK:
        case TV_LAW_BOOK:
        case TV_MUSIC_BOOK:
        case TV_HISSATSU_BOOK:
        case TV_HEX_BOOK:
        case TV_RAGE_BOOK:
        case TV_BURGLARY_BOOK:
        {
            return (TRUE);
        }

        /* Simple items */
        case TV_FLASK:
        case TV_JUNK:
        case TV_BOTTLE:
        case TV_SKELETON:
        case TV_SPIKE:
        case TV_WHISTLE:
        {
            return (TRUE);
        }

        /* All Food, Potions, Scrolls, Rods */
        case TV_FOOD:
        case TV_POTION:
        case TV_SCROLL:
        {
            return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}


/*
 * Create a name from random parts.
 */
void get_table_name_aux(char *out_string)
{
#define MAX_SYLLABLES 164       /* Used with scrolls (see below) */

    static cptr syllables[MAX_SYLLABLES] = {
        "a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
        "arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
        "bie", "bin", "bit", "bjor", "blu", "bot", "bu",
        "byt", "comp", "con", "cos", "cre", "dalf", "dan",
        "den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
        "esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
        "gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
        "ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
        "kho", "kli", "klis", "la", "lech", "man", "mar",
        "me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
        "nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
        "od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
        "ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
        "ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
        "see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
        "sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
        "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
        "ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
        "vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
        "yerg", "yp", "zun", "tri", "blaa", "jah", "bul", "on",
        "foo", "ju", "xuxu"
    };

    int testcounter = randint1(3) + 1;

    strcpy(out_string, "");

    if (randint1(3) == 2)
    {
        while (testcounter--)
            strcat(out_string, syllables[randint0(MAX_SYLLABLES)]);
    }
    else
    {
        char Syllable[80];
        testcounter = randint1(2) + 1;
        while (testcounter--)
        {
            (void)get_rnd_line("elvish.txt", 0, Syllable);
            strcat(out_string, Syllable);
        }
    }

    out_string[0] = toupper(out_string[1]);

    out_string[16] = '\0';
}


/*
 * Create a name from random parts with quotes.
 */
void get_table_name(char *out_string)
{
    char buff[80];
    get_table_name_aux(buff);

    sprintf(out_string, "'%s'", buff);
}


/*
 * Make random Sindarin name
 */
void get_table_sindarin_aux(char *out_string)
{
    char Syllable[80];

    get_rnd_line("sname.txt", 1, Syllable);
    strcpy(out_string, Syllable);

    get_rnd_line("sname.txt", 2, Syllable);
    strcat(out_string, Syllable);
}


/*
 * Make random Sindarin name with quotes
 */
void get_table_sindarin(char *out_string)
{
    char buff[80];
    get_table_sindarin_aux(buff);

    sprintf(out_string, "'%s'", buff);
}


/*
 * Shuffle flavor indices of a group of objects with given tval
 */
static void shuffle_flavors(byte tval)
{
    s16b *k_idx_list;
    int k_idx_list_num = 0;
    int i;

    /* Allocate an array for a list of k_idx */
    C_MAKE(k_idx_list, max_k_idx, s16b);

    /* Search objects with given tval for shuffle */
    for (i = 0; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Skip non-Rings */
        if (k_ptr->tval != tval) continue;

        /* Paranoia -- Skip objects without flavor */
        if (!k_ptr->flavor) continue;

        /* Skip objects with a fixed flavor name */
        if (have_flag(k_ptr->flags, OF_FIXED_FLAVOR)) continue;

        /* Remember k_idx */
        k_idx_list[k_idx_list_num] = i;

        /* Increase number of remembered indices */
        k_idx_list_num++;
    }

    /* Shuffle flavors */
    for (i = 0; i < k_idx_list_num; i++)
    {
        object_kind *k1_ptr = &k_info[k_idx_list[i]];
        object_kind *k2_ptr = &k_info[k_idx_list[randint0(k_idx_list_num)]];

        /* Swap flavors of this pair */
        s16b tmp = k1_ptr->flavor;
        k1_ptr->flavor = k2_ptr->flavor;
        k2_ptr->flavor = tmp;
    }

    /* Free an array for a list of k_idx */
    C_KILL(k_idx_list, max_k_idx, s16b);
}

/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long. This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters. The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first four letters (not the most efficient method, and not
 * the least efficient method, but it will always work).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()". Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 *
 * Note that the "hacked seed" may provide an RNG with alternating parity!
 */
void flavor_init(void)
{
    int i;

    /* Hack -- Use the "simple" RNG */
    Rand_quick = TRUE;

    /* Hack -- Induce consistant flavors */
    Rand_value = seed_flavor;


    /* Initialize flavor index of each object by itself */
    for (i = 0; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Skip objects without flavor name */
        if (!k_ptr->flavor_name) continue;

        /*
         * Initialize flavor index to itself
         *  -> Shuffle it later
         */
        k_ptr->flavor = i;
    }

    /* Shuffle Mushrooms */
    shuffle_flavors(TV_FOOD);

    /* Shuffle Potions */
    shuffle_flavors(TV_POTION);

    /* Shuffle Scrolls */
    shuffle_flavors(TV_SCROLL);


    /* Hack -- Use the "complex" RNG */
    Rand_quick = FALSE;

    /* Analyze every object */
    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Skip "empty" objects */
        if (!k_ptr->name) continue;

        /* No flavor yields aware */
        if (!k_ptr->flavor) k_ptr->aware = TRUE;

        /* Check for "easily known" */
        k_ptr->easy_know = object_easy_know(i);
    }
}


/*
 * Print a char "c" into a string "t", as if by sprintf(t, "%c", c),
 * and return a pointer to the terminator (t + 1).
 */
static char *object_desc_chr(char *t, char c)
{
    /* Copy the char */
    *t++ = c;

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}


/*
 * Print a string "s" into a string "t", as if by strcpy(t, s),
 * and return a pointer to the terminator.
 */
static char *object_desc_str(char *t, cptr s)
{
    /* Copy the string */
    while (*s) *t++ = *s++;

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}



/*
 * Print an unsigned number "n" into a string "t", as if by
 * sprintf(t, "%u", n), and return a pointer to the terminator.
 */
static char *object_desc_num(char *t, uint n)
{
    uint p;

    /* Find "size" of "n" */
    for (p = 1; n >= p * 10; p = p * 10) /* loop */;

    /* Dump each digit */
    while (p >= 1)
    {
        /* Dump the digit */
        *t++ = '0' + n / p;

        /* Remove the digit */
        n = n % p;

        /* Process next digit */
        p = p / 10;
    }

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}





/*
 * Print an signed number "v" into a string "t", as if by
 * sprintf(t, "%+d", n), and return a pointer to the terminator.
 * Note that we always print a sign, either "+" or "-".
 */
static char *object_desc_int(char *t, sint v)
{
    uint p, n;

    /* Negative */
    if (v < 0)
    {
        /* Take the absolute value */
        n = 0 - v;

        /* Use a "minus" sign */
        *t++ = '-';
    }

    /* Positive (or zero) */
    else
    {
        /* Use the actual number */
        n = v;

        /* Use a "plus" sign */
        *t++ = '+';
    }

    /* Find "size" of "n" */
    for (p = 1; n >= p * 10; p = p * 10) /* loop */;

    /* Dump each digit */
    while (p >= 1)
    {
        /* Dump the digit */
        *t++ = '0' + n / p;

        /* Remove the digit */
        n = n % p;

        /* Process next digit */
        p = p / 10;
    }

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}


/*
 * Structs and tables for Auto Inscription for flags
 */

typedef struct flag_insc_table
{
    cptr english;
    int flag;
    int except_flag;
} flag_insc_table;

static flag_insc_table flag_insc_plus[] =
{
    { "At", OF_BLOWS, -1 },
    { "Sp", OF_SPEED, -1 },
    { "St", OF_STR, -1 },
    { "In", OF_INT, -1 },
    { "Wi", OF_WIS, -1 },
    { "Dx", OF_DEX, -1 },
    { "Cn", OF_CON, -1 },
    { "Ch", OF_CHR, -1 },
    { "Md", OF_MAGIC_MASTERY, -1 },
    { "Sl", OF_STEALTH, -1 },
    { "Sr", OF_SEARCH, -1 },
    { "If", OF_INFRA, -1 },
    { "Dg", OF_TUNNEL, -1 },
    { "Lf", OF_LIFE, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_minus[] =
{
    { "At", OF_DEC_BLOWS, -1 },
    { "Sp", OF_DEC_SPEED, -1 },
    { "St", OF_DEC_STR, -1 },
    { "In", OF_DEC_INT, -1 },
    { "Wi", OF_DEC_WIS, -1 },
    { "Dx", OF_DEC_DEX, -1 },
    { "Cn", OF_DEC_CON, -1 },
    { "Ch", OF_DEC_CHR, -1 },
    { "Md", OF_DEC_MAGIC_MASTERY, -1 },
    { "Sl", OF_DEC_STEALTH, -1 },
    { "Lf", OF_DEC_LIFE, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_immune[] =
{
    { "Ac", OF_IM_ACID, -1 },
    { "El", OF_IM_ELEC, -1 },
    { "Fi", OF_IM_FIRE, -1 },
    { "Co", OF_IM_COLD, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_resistance[] =
{
    { "Ac", OF_RES_ACID, OF_IM_ACID },
    { "El", OF_RES_ELEC, OF_IM_ELEC },
    { "Fi", OF_RES_FIRE, OF_IM_FIRE },
    { "Co", OF_RES_COLD, OF_IM_COLD },
    { "Po", OF_RES_POIS, -1 },
    { "Li", OF_RES_LITE, -1 },
    { "Dk", OF_RES_DARK, -1 },
    { "Cf", OF_RES_CONF, -1 },
    { "Nt", OF_RES_NETHER, -1 },
    { "Nx", OF_RES_NEXUS, -1 },
    { "So", OF_RES_SOUND, -1 },
    { "Sh", OF_RES_SHARDS, -1 },
    { "Ca", OF_RES_CHAOS, -1 },
    { "Di", OF_RES_DISEN, -1 },
    { "Ti", OF_RES_TIME, -1 },
    { "Bl", OF_RES_BLIND, -1 },
    { "Fe", OF_RES_FEAR, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_vulnerability[] =
{
    { "Ac", OF_VULN_ACID, -1 },
    { "El", OF_VULN_ELEC, -1 },
    { "Fi", OF_VULN_FIRE, -1 },
    { "Co", OF_VULN_COLD, -1 },
    { "Po", OF_VULN_POIS, -1 },
    { "Li", OF_VULN_LITE, -1 },
    { "Dk", OF_VULN_DARK, -1 },
    { "Cf", OF_VULN_CONF, -1 },
    { "Nt", OF_VULN_NETHER, -1 },
    { "Nx", OF_VULN_NEXUS, -1 },
    { "So", OF_VULN_SOUND, -1 },
    { "Sh", OF_VULN_SHARDS, -1 },
    { "Ca", OF_VULN_CHAOS, -1 },
    { "Di", OF_VULN_DISEN, -1 },
    { "Bl", OF_VULN_BLIND, -1 },
    { "Fe", OF_VULN_FEAR, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_misc[] =
{
    { "Es", OF_EASY_SPELL, -1 },
    { "Dm", OF_DEC_MANA, -1 },
    { "Th", OF_THROWING, -1 },
    { "Rf", OF_REFLECT, -1 },
    { "Fa", OF_FREE_ACT, -1 },
    { "Si", OF_SEE_INVIS, -1 },
    { "Hl", OF_HOLD_LIFE, -1 },
    { "Sd", OF_SLOW_DIGEST, -1 },
    { "Rg", OF_REGEN, -1 },
    { "Rm", OF_REGEN_MANA, -1 },
    { "Lv", OF_LEVITATION, -1 },
    { "Nv", OF_NIGHT_VISION, -1 },
    { "Lu", OF_LITE, -1 },
    { "Wr", OF_WARNING, -1 },
    { "Xm", OF_XTRA_MIGHT, -1 },
    { "Xs", OF_XTRA_SHOTS, -1 },
    { "Te", OF_TELEPORT, -1 },
    { "Ag", OF_AGGRAVATE, -1 },
    { "Bs", OF_BLESSED, -1 },
    { "Ty", OF_TY_CURSE, -1 },
    { "Ds", OF_DARKNESS, -1 },
    { "Wm", OF_WEAPONMASTERY, -1 },
    { "Id", OF_LORE2, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_aura[] =
{
    { "F", OF_AURA_FIRE, -1 },
    { "E", OF_AURA_ELEC, -1 },
    { "C", OF_AURA_COLD, -1 },
    { "Sh", OF_AURA_SHARDS, -1 },
    { "At", OF_AURA_REVENGE, -1 },
    { "M", OF_NO_MAGIC, -1 },
    { "T", OF_NO_TELE, -1 },
    { "Sm", OF_NO_SUMMON, -1 },
    { "Mr", OF_MAGIC_RESISTANCE, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_brand[] =
{
    { "A", OF_BRAND_ACID, -1 },
    { "E", OF_BRAND_ELEC, -1 },
    { "F", OF_BRAND_FIRE, -1 },
    { "Co", OF_BRAND_COLD, -1 },
    { "P", OF_BRAND_POIS, -1 },
    { "Dk", OF_BRAND_DARK, -1 },
    { "Ca", OF_BRAND_CHAOS, -1 },
    { "V", OF_BRAND_VAMP, -1 },
    { "Q", OF_IMPACT, -1 },
    { "St", OF_STUN, -1 },
    { "S", OF_VORPAL, -1 },
    { "S", OF_VORPAL2, -1 },
    { "M", OF_BRAND_MANA, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_kill[] =
{
    { "*", OF_KILL_EVIL, -1 },
    { "p", OF_KILL_HUMAN, -1 },
    { "D", OF_KILL_DRAGON, -1 },
    { "o", OF_KILL_ORC, -1 },
    { "T", OF_KILL_TROLL, -1 },
    { "P", OF_KILL_GIANT, -1 },
    { "U", OF_KILL_DEMON, -1 },
    { "L", OF_KILL_UNDEAD, -1 },
    { "Z", OF_KILL_ANIMAL, -1 },
	{ "A", OF_KILL_GOOD, -1 },
	{ "Lv", OF_KILL_LIVING, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_slay[] =
{
    { "*", OF_SLAY_EVIL, OF_KILL_EVIL },
    { "p", OF_SLAY_HUMAN, OF_KILL_HUMAN },
    { "D", OF_SLAY_DRAGON, OF_KILL_DRAGON },
    { "o", OF_SLAY_ORC, OF_KILL_ORC },
    { "T", OF_SLAY_TROLL, OF_KILL_TROLL },
    { "P", OF_SLAY_GIANT, OF_KILL_GIANT },
    { "U", OF_SLAY_DEMON, OF_KILL_DEMON },
    { "L", OF_SLAY_UNDEAD, OF_KILL_UNDEAD },
    { "Z", OF_SLAY_ANIMAL, OF_KILL_ANIMAL },
    { "A", OF_SLAY_GOOD, OF_KILL_GOOD },
    { "Lv", OF_SLAY_LIVING, OF_KILL_LIVING },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_esp1[] =
{
    { "Tele", OF_TELEPATHY, -1 },
    { "Evil", OF_ESP_EVIL, -1 },
    { "Good", OF_ESP_GOOD, -1 },
    { "Nolv", OF_ESP_NONLIVING, -1 },
    { "Uniq", OF_ESP_UNIQUE, -1 },
	{ "Lvng", OF_ESP_LIVING, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_esp2[] =
{
    { "p", OF_ESP_HUMAN, -1 },
    { "D", OF_ESP_DRAGON, -1 },
    { "o", OF_ESP_ORC, -1 },
    { "T", OF_ESP_TROLL, -1 },
    { "P", OF_ESP_GIANT, -1 },
    { "U", OF_ESP_DEMON, -1 },
    { "L", OF_ESP_UNDEAD, -1 },
    { "Z", OF_ESP_ANIMAL, -1 },
    { NULL, 0, -1 }
};

static flag_insc_table flag_insc_sust[] =
{
    { "St", OF_SUST_STR, -1 },
    { "In", OF_SUST_INT, -1 },
    { "Wi", OF_SUST_WIS, -1 },
    { "Dx", OF_SUST_DEX, -1 },
    { "Cn", OF_SUST_CON, -1 },
    { "Ch", OF_SUST_CHR, -1 },
    { NULL, 0, -1 }
};

/* Simple macro for get_inscription() */
#define ADD_INSC(STR) (void)(ptr = object_desc_str(ptr, (STR)))

/*
 *  Helper function for get_inscription()
 */
static char *inscribe_flags_aux(flag_insc_table *fi_ptr, u32b flgs[OF_ARRAY_SIZE], char *ptr)
{
    while (fi_ptr->english)
    {
        if (have_flag(flgs, fi_ptr->flag) &&
            (fi_ptr->except_flag == -1 || !have_flag(flgs, fi_ptr->except_flag)))
            ADD_INSC(fi_ptr->english);
        fi_ptr++;
    }

    return ptr;
}

int resist_opposite_flag(int i)
{
    byte loytyi = 0;
    cptr my_english = NULL;
    flag_insc_table *flgs = flag_insc_resistance;
    while (flgs->english)
    {
        if (i == flgs->flag)
        {
            loytyi = 1;
            my_english = flgs->english;
            break;
        }
        flgs++;
    }
    if (!loytyi)
    {
        flgs = flag_insc_vulnerability;
        while (flgs->english)
        {
            if (i == flgs->flag)
            {
                loytyi = 2;
                my_english = flgs->english;
                break;
            }
            flgs++;
        }
    }
    if (!loytyi)
    {
        return OF_INVALID;
    }
    flgs = (loytyi == 2) ? flag_insc_resistance : flag_insc_vulnerability;
    while (flgs->english)
    {
        if (streq(flgs->english, my_english))
        {
            return flgs->flag;
        }
        flgs++;
    }
    return OF_INVALID;
}

/*
 *  Special variation of have_flag for auto-inscription
 */
static bool have_flag_of(flag_insc_table *fi_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    while (fi_ptr->english)
    {
        if (have_flag(flgs, fi_ptr->flag) &&
           (fi_ptr->except_flag == -1 || !have_flag(flgs, fi_ptr->except_flag)))
            return (TRUE);
        fi_ptr++;
    }

    return (FALSE);
}

static char *get_ability_abbreviation(char *ptr, object_type *o_ptr, bool all)
{
    char *prev_ptr = ptr;
    u32b flgs[OF_ARRAY_SIZE];

    if (object_is_device(o_ptr))
        return ptr;

    /* Extract the flags */
    obj_flags_display(o_ptr, flgs);

    /* Remove obvious flags */
    if (!all)
    {
        object_kind *k_ptr = &k_info[o_ptr->k_idx];
        int j;
                
        /* Base object */
        for (j = 0; j < OF_ARRAY_SIZE; j++)
            flgs[j] &= ~k_ptr->flags[j];

        if (object_is_fixed_artifact(o_ptr))
        {
            artifact_type *a_ptr = &a_info[o_ptr->name1];
                    
            for (j = 0; j < OF_ARRAY_SIZE; j++)
                flgs[j] &= ~a_ptr->flags[j];
        }

        if (object_is_ego(o_ptr))
        {
            ego_type *e_ptr = &e_info[o_ptr->name2];
                    
            for (j = 0; j < OF_ARRAY_SIZE; j++)
                flgs[j] &= ~e_ptr->flags[j];
        }
    }

    /* Remove opposite flags */
    remove_opposite_flags(flgs);

    /* Plusses */
    ptr = inscribe_flags_aux(flag_insc_plus, flgs, ptr);

    /* Minusses */
    if (have_flag_of(flag_insc_minus, flgs))
    {
        if (ptr != prev_ptr)
        {
            ADD_INSC(";");
            prev_ptr = ptr;
        }
        ADD_INSC("-");
    }
    ptr = inscribe_flags_aux(flag_insc_minus, flgs, ptr);

    /* Immunity */
    if (have_flag_of(flag_insc_immune, flgs))
    {
        if (ptr != prev_ptr)
        {
            ADD_INSC(";");
            prev_ptr = ptr;
        }
        ADD_INSC("*");
    }
    ptr = inscribe_flags_aux(flag_insc_immune, flgs, ptr);

    /* Vulnerability */
    if (have_flag_of(flag_insc_vulnerability, flgs))
    {
        if (ptr != prev_ptr)
        {
            ADD_INSC(";");
            prev_ptr = ptr;
        }
        ADD_INSC("-");
    }
    ptr = inscribe_flags_aux(flag_insc_vulnerability, flgs, ptr);

    /* Resistance */
    if (have_flag_of(flag_insc_resistance, flgs))
    {
        if (ptr != prev_ptr)
        {
            ADD_INSC(";");
            prev_ptr = ptr;
        }
    }
    ptr = inscribe_flags_aux(flag_insc_resistance, flgs, ptr);

    /* Misc Ability */
    if (have_flag_of(flag_insc_misc, flgs))
    {
        if (ptr != prev_ptr)
        {
            ADD_INSC(";");
            prev_ptr = ptr;
        }
    }
    ptr = inscribe_flags_aux(flag_insc_misc, flgs, ptr);

    /* Aura */
    if (have_flag_of(flag_insc_aura, flgs))
    {
        ADD_INSC("[");
    }
    ptr = inscribe_flags_aux(flag_insc_aura, flgs, ptr);

    /* Brand Weapon */
    if (have_flag_of(flag_insc_brand, flgs))
        ADD_INSC("|");
    ptr = inscribe_flags_aux(flag_insc_brand, flgs, ptr);

    /* Kill Weapon */
    if (have_flag_of(flag_insc_kill, flgs))
        ADD_INSC("/X");
    ptr = inscribe_flags_aux(flag_insc_kill, flgs, ptr);

    /* Slay Weapon */
    if (have_flag_of(flag_insc_slay, flgs))
        ADD_INSC("/");
    ptr = inscribe_flags_aux(flag_insc_slay, flgs, ptr);

    /* Esp */
    if (have_flag_of(flag_insc_esp1, flgs))
        ADD_INSC("~");
    ptr = inscribe_flags_aux(flag_insc_esp1, flgs, ptr);
    if (have_flag_of(flag_insc_esp2, flgs))
        ADD_INSC("~");
    ptr = inscribe_flags_aux(flag_insc_esp2, flgs, ptr);

    /* sustain */
    if (have_flag_of(flag_insc_sust, flgs))
    {
        ADD_INSC("(");
    }
    ptr = inscribe_flags_aux(flag_insc_sust, flgs, ptr);

    /* Is there more to learn about this object? Perhaps, but don't leak quality info! */
    if ( obj_is_identified(o_ptr)
      && (object_is_wearable(o_ptr) || object_is_ammo(o_ptr))
      && ((object_is_artifact(o_ptr)) || (object_is_ego(o_ptr)) ||
          ((mark_dragon) && (object_is_dragon_armor(o_ptr))))
      && !obj_is_identified_fully(o_ptr)
      && !(o_ptr->ident & IDENT_STORE) )
    {
        ADD_INSC("?");
    }

    *ptr = '\0';

    return ptr;
}


/*
 *  Get object inscription with auto inscription of object flags.
 */
static void get_inscription(char *buff, object_type *o_ptr)
{
    cptr insc = quark_str(o_ptr->inscription);
    char *ptr = buff;

    /* Not fully identified */
    if (!obj_is_identified_fully(o_ptr))
    {
        /* Copy until end of line or '#' */
        while (*insc)
        {
            if (*insc == '#') break;
            *buff++ = *insc++;
        }

        *buff = '\0';
        return;
    }

    *buff = '\0';
    for (; *insc; insc++)
    {
        /* Ignore fake artifact inscription */
        if (*insc == '#') break;

        /* {%} will be automatically converted */
        else if ('%' == *insc)
        {
            bool all;
            cptr start = ptr;

            /* check for too long inscription */
            if (ptr >= buff + MAX_NLEN) continue;

            if ('a' == insc[1] && 'l' == insc[2] && 'l' == insc[3])
            {
                all = TRUE;
                insc += 3;
            }
            else
            {
                all = FALSE;
            }

            ptr = get_ability_abbreviation(ptr, o_ptr, all);

            if (ptr == start)
                ADD_INSC(" ");
        }
        else
        {
            *ptr++ = *insc;
        }
    }
    *ptr = '\0';
}

char attr_to_attr_char(byte a)
{
    //char hack[30] = "dwsorgbuDWvyRGBULPiptSmMTOVcn";
    char c = (color_char[a & COLOR_MASK]);
    return c;
}

char tval_to_attr_char(int tval)
{
    return attr_to_attr_char(tval_to_attr[tval % 128]);
}

/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be MAX_NLEN chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under MAX_NLEN-1 chars (plus a terminator).
 *
 * Note the use of "object_desc_num()" and "object_desc_int()" as hyper-efficient,
 * portable, versions of some common "sprintf()" commands.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lites use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Rings and Amulets, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace). They will NEVER "append" the "k_info" name. But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * Mode:
 *   OD_NAME_ONLY        : The Cloak of Death
 *   OD_NAME_AND_ENCHANT : The Cloak of Death [1,+3]
 *   OD_OMIT_INSCRIPTION : The Cloak of Death [1,+3] (+2 to Stealth)
 *   0                   : The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
 *
 *   OD_OMIT_PREFIX      : Forbidden numeric prefix
 *   OD_NO_PLURAL        : Forbidden use of plural 
 *   OD_STORE            : Assume to be aware and known
 *   OD_NO_FLAVOR        : Allow to hidden flavor
 *   OD_FORCE_FLAVOR     : Get un-shuffled flavor name
 *   OD_SINGULAR         : Pretend o_ptr->number == 1.
 */
void object_desc(char *buf, object_type *o_ptr, u32b mode)
{
    /* Extract object kind name */
    cptr            kindname = k_name + k_info[o_ptr->k_idx].name;

    /* Extract default "base" string */
    cptr            basenm = kindname;

    /* Assume no "modifier" string */
    cptr            modstr = "";

    int             power;

    bool            aware = FALSE;
    bool            known = FALSE;
    bool            flavor = TRUE;
    bool            device = FALSE;

    bool            show_weapon = FALSE;
    bool            show_armour = FALSE;

    cptr            s, s0;
    char            *t;

    char            p1 = '(', p2 = ')';
    char            b1 = '[', b2 = ']';
    char            c1 = '{', c2 = '}';

    char            tmp_val[MAX_NLEN+160];
    char            tmp_val2[MAX_NLEN+10];
    char            fake_insc_buf[30];

    u32b            flgs[OF_ARRAY_SIZE];
    u32b            known_flgs[OF_ARRAY_SIZE];

    object_kind    *k_ptr = &k_info[o_ptr->k_idx];
    object_kind    *flavor_k_ptr = &k_info[k_ptr->flavor];

    int             number = (mode & OD_SINGULAR) ? 1 : o_ptr->number;

    /* Extract some flags */
    obj_flags(o_ptr, flgs); /* TR_FULL_NAME and TR_SHOW_MODS should never really be hidden ... */
    obj_flags_known(o_ptr, known_flgs); /* ... but please don't leak  Activations, Spell Power, etc! */

    /* See if the object is "aware" */
    if (object_is_aware(o_ptr)) aware = TRUE;
    /* Devices no longer use flavor system ... k_info[].aware is meaningless! */
    if (object_is_device(o_ptr) && object_is_known(o_ptr)) aware = TRUE;

    /* Hack object_is_aware() is wrong in this case.
       Anybody know how k_info[].aware gets set?  Perhaps flavor_init? */
    if (o_ptr->name1 == ART_HAND_OF_VECNA || o_ptr->name1 == ART_EYE_OF_VECNA)
        aware = FALSE;

    /* See if the object is "known" */
    if (object_is_known(o_ptr)) known = TRUE;

    /* Allow flavors to be hidden when aware */
    if (aware && ((mode & OD_NO_FLAVOR) || plain_descriptions)) flavor = FALSE;

    /* Object is in the inventory of a store or spoiler */
    if ((mode & OD_STORE) || (o_ptr->ident & IDENT_STORE))
    {
        /* Don't show flavors */
        flavor = FALSE;

        /* Pretend known and aware */
        aware = TRUE;
        known = TRUE;
    }

    /* Force to be flavor name only */
    if (mode & OD_FORCE_FLAVOR)
    {
        aware = FALSE;
        flavor = TRUE;
        known = FALSE;

        /* Cancel shuffling */
        flavor_k_ptr = k_ptr;
    }

    /* Analyze the object */
    switch (o_ptr->tval)
    {
        /* Some objects are easy to describe */
        case TV_SKELETON:
        case TV_BOTTLE:
        case TV_JUNK:
        case TV_SPIKE:
        case TV_FLASK:
        case TV_CHEST:
        case TV_WHISTLE:
        {
            break;
        }

        case TV_CAPTURE:
        {
            monster_race *r_ptr = &r_info[o_ptr->pval];

            if (known)
            {
                if (!o_ptr->pval)
                {
                    modstr = " (empty)";
                }
                else
                {
                    cptr t = r_name + r_ptr->name;

                    if (!(r_ptr->flags1 & RF1_UNIQUE))
                    {
                        sprintf(tmp_val2, " (%s%s)", (is_a_vowel(*t) ? "an " : "a "), t);

                        modstr = tmp_val2;
                    }
                    else
                    {
                        sprintf(tmp_val2, " (%s)", t);

                        modstr = tmp_val2;
                    }
                }
            }
            break;
        }

        /* Figurines/Statues */
        case TV_FIGURINE:
        case TV_STATUE:
        {
            monster_race *r_ptr = &r_info[o_ptr->pval];

            cptr t = r_name + r_ptr->name;

            if (!(r_ptr->flags1 & RF1_UNIQUE))
            {
                sprintf(tmp_val2, "%s%s", (is_a_vowel(*t) ? "an " : "a "), t);

                modstr = tmp_val2;
            }
            else
            {
                modstr = t;
            }
            break;
        }

        /* Corpses */
        case TV_CORPSE:
        {
            if (o_ptr->sval >= SV_BODY_HEAD)
            {
                int luku = (o_ptr->sval == SV_BODY_EARS) ? o_ptr->pval : o_ptr->xtra4;
                if ((luku) && (luku < max_r_idx))
                {
                    monster_race *r_ptr = &r_info[luku];
                    if (r_ptr->name)
                    {
                        if (r_ptr->flags1 & RF1_UNIQUE)
                        {
                            modstr = r_name + r_ptr->name;
                            basenm = "The % of #";
                        }
                        else
                        {
                            modstr = r_name + r_ptr->name;
                            if (is_a_vowel(modstr[0])) basenm = "the % of an #";
                            else basenm = "the % of a #";
                        }
                    }
                    else
                    {
                        modstr = "a Software bug";
                        basenm = "the % of #";
                    }
                }
                else
                {
                    modstr = "an Igor";
                    basenm = "the % of #";
                }
                /* Body parts cannot be piled, so it's usually safe to use the
                 * article "the"... but it is possible to have 0 of them */
                if ((!number) || (mode & OD_OMIT_PREFIX)) basenm += 4;
            }
            else
            {
                monster_race *r_ptr = &r_info[o_ptr->pval];

                modstr = r_name + r_ptr->name;

                if (r_ptr->flags1 & RF1_UNIQUE)
                    basenm = "& % of #";
                else
                    basenm = "& # %";
            }
            break;
        }

        /* Missiles/ Bows/ Weapons */
        case TV_SHOT:
        case TV_BOLT:
        case TV_ARROW:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:
        {
            show_weapon = TRUE;
            break;
        }

        case TV_BOW:
        {
            if (!obj_is_fake_bow(o_ptr))
                show_weapon = TRUE;

            break;
        }
        case TV_QUIVER:
            break;

        /* Armour */
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_CROWN:
        case TV_HELM:
        case TV_SHIELD:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        {
            if (o_ptr->name1 == ART_HAND_OF_VECNA)
            {
                modstr = k_name + flavor_k_ptr->flavor_name;
                if (!flavor)    basenm = "& Hand~ of %";
                else if (aware) break;
                else            basenm = "& # Hand~";
            }
            else
                show_armour = TRUE;
            break;
        }

        /* Lites (including a few "Specials") */
        case TV_LITE:
        {
            if (o_ptr->name1 == ART_EYE_OF_VECNA)
            {
                modstr = k_name + flavor_k_ptr->flavor_name;
                if (!flavor)    basenm = "& Eye~ of %";
                else if (aware) break;
                else            basenm = "& # Eye~";
            }
            break;
        }

        case TV_AMULET:
        case TV_RING:
            if (o_ptr->to_h || o_ptr->to_d)
                show_weapon = TRUE;
            break;

        case TV_CARD:
        {
            break;
        }

        case TV_RUNE:
        {
            break;
        }

        case TV_STAFF:
        case TV_WAND:
        case TV_ROD:
            device = TRUE;
            break;

        case TV_SCROLL:
        {
            /* Color the object */
            modstr = k_name + flavor_k_ptr->flavor_name;

            if (!flavor)    basenm = "& Scroll~ of %";
            else if (aware) basenm = "& Scroll~ titled \"#\" of %";
            else            basenm = "& Scroll~ titled \"#\"";

            break;
        }

        case TV_POTION:
        {
            /* Color the object */
            modstr = k_name + flavor_k_ptr->flavor_name;

            if (!flavor)    basenm = "& Potion~ of %";
            else if (aware) basenm = "& # Potion~ of %";
            else            basenm = "& # Potion~";
            break;
        }

        case TV_FOOD:
        {
            /* Ordinary food is "boring" */
            if (!k_ptr->flavor_name) break;

            /* Color the object */
            modstr = k_name + flavor_k_ptr->flavor_name;

            if (!flavor)    basenm = "& Mushroom~ of %";
            else if (aware) basenm = "& # Mushroom~ of %";
            else            basenm = "& # Mushroom~";
            break;
        }

        case TV_PARCHMENT:
        {
            basenm = "& Parchment~ - %";
            break;
        }

        /* Magic Books */
        case TV_LIFE_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Life Magic %";
            else
                basenm = "& Life Spellbook~ %";
            break;
        }

        case TV_SORCERY_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Sorcery %";
            else
                basenm = "& Sorcery Spellbook~ %";
            break;
        }

        case TV_NATURE_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Nature Magic %";
            else
                basenm = "& Nature Spellbook~ %";
            break;
        }

        case TV_CHAOS_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Chaos Magic %";
            else
                basenm = "& Chaos Spellbook~ %";
            break;
        }

        case TV_DEATH_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Death Magic %";
            else
                basenm = "& Death Spellbook~ %";
            break;
        }

        case TV_TRUMP_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Trump Magic %";
            else
                basenm = "& Trump Spellbook~ %";
            break;
        }

        case TV_ARCANE_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Arcane Magic %";
            else
                basenm = "& Arcane Spellbook~ %";
            break;
        }

        case TV_CRAFT_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Craft Magic %";
            else
                basenm = "& Craft Spellbook~ %";
            break;
        }

        case TV_DAEMON_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Daemon Magic %";
            else
                basenm = "& Daemon Spellbook~ %";
            break;
        }

        case TV_CRUSADE_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Crusade Magic %";
            else
                basenm = "& Crusade Spellbook~ %";
            break;
        }

        case TV_LAW_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Legal Tricks %";
            else
                basenm = "& Law Book~ %";
            break;
        }

        case TV_NECROMANCY_BOOK:
        {
            basenm = "& Necromancy Spellbook~ %";
            break;
        }

        case TV_RAGE_BOOK:
        {
            basenm = "& Rage Spellbook~ %";
            break;
        }

        case TV_BURGLARY_BOOK:
        {
            basenm = "& Thieves' Guide~ %";
            break;
        }

        case TV_ARMAGEDDON_BOOK:
            basenm = "& Armageddon Spellbook~ %";
            break;

        case TV_MUSIC_BOOK:
        {
            basenm = "& Song Book~ %";
            break;
        }

        case TV_HISSATSU_BOOK:
        {
            basenm = "Book~ of Kendo %";
            break;
        }

        case TV_HEX_BOOK:
        {
            if (mp_ptr->spell_book == TV_LIFE_BOOK)
                basenm = "& Book~ of Hex Magic %";
            else
                basenm = "& Hex Spellbook~ %";
            break;
        }

        /* Hack -- Gold/Gems */
        case TV_GOLD:
        {
            if (mode & OD_COLOR_CODED)
                sprintf(buf, "<color:%c>%s</color>", tval_to_attr_char(o_ptr->tval), basenm);
            else
                strcpy(buf, basenm);
            return;
        }

        /* Used in the "inventory" routine */
        default:
        {
            strcpy(buf, "(nothing)");
            return;
        }
    }

    /* Use full name from k_info or a_info */
    if (aware && have_flag(flgs, OF_FULL_NAME))
    {
        if (known && o_ptr->name1) basenm = a_name + a_info[o_ptr->name1].name;
        else if (known && o_ptr->name2) basenm = e_name + e_info[o_ptr->name2].name;
        else basenm = kindname;
    }

    /* Start dumping the result */
    t = tmp_val;

    if (o_ptr->marked & OM_RESERVED)
    {
        if (mode & OD_COLOR_CODED)
            t = object_desc_str(t, "<color:B><<Hold>></color> ");
        else
            t = object_desc_str(t, "<<Hold>> ");
    }

    if (o_ptr->marked & OM_SLIPPING)
        t = object_desc_str(t, "<<Slipped>> ");

    if (o_ptr->marked & OM_WORN)
        t = object_desc_str(t, "<<Worn>> ");

    /* The object "expects" a "number" */
    if (basenm[0] == '&')
    {
        /* Skip the ampersand (and space) */
        s = basenm + 2;

        /* No prefix */
        if (mode & OD_OMIT_PREFIX)
        {
            /* Nothing */
        }

        /* Hack -- None left */
        else if (number <= 0)
        {
            t = object_desc_str(t, "no more ");
        }

        /* Extract the number */
        else if (number > 1)
        {
            t = object_desc_num(t, number);
            t = object_desc_chr(t, ' ');
        }

        /* Hack -- The only one of its kind */
        else if ((known && object_is_artifact(o_ptr)) ||
                 ((o_ptr->tval == TV_CORPSE) &&
                  (r_info[o_ptr->pval].flags1 & RF1_UNIQUE)))
        {
            if (o_ptr->name1 != ART_MOM) t = object_desc_str(t, "The ");
        }

        /* A single one */
        else
        {
            bool vowel;

            switch (*s)
            {
            case '#': vowel = is_a_vowel(modstr[0]); break;
            case '%': vowel = is_a_vowel(*kindname); break;
            default:  vowel = is_a_vowel(*s); break;
            }

            if (vowel)
            {
                /* A single one, with a vowel */
                t = object_desc_str(t, "an ");
            }
            else
            {
                /* A single one, without a vowel */
                t = object_desc_str(t, "a ");
            }
        }
    }

    /* Hack -- objects that "never" take an article */
    else
    {
        /* No ampersand */
        s = basenm;

        /* No pref */
        if (mode & OD_OMIT_PREFIX)
        {
            /* Nothing */
        }

        /* Hack -- all gone */
        else if (number <= 0)
        {
            t = object_desc_str(t, "no more ");
        }

        /* Prefix a number if required */
        else if (number > 1)
        {
            t = object_desc_num(t, number);
            t = object_desc_chr(t, ' ');
        }

        /* Hack -- The only one of its kind */
        else if (known && object_is_artifact(o_ptr))
        {
            if (o_ptr->name1 != ART_MOM) t = object_desc_str(t, "The ");
        }

        /* Hack -- single items get no prefix */
        else
        {
            /* Nothing */
        }
    }

    /* Copy the string */
    for (s0 = NULL; *s || s0; )
    {
        /* The end of the flavour/kind string. */
        if (!*s)
        {
            s = s0 + 1;
            s0 = NULL;
        }

        /* Begin to append the modifier (flavor) */
        else if ((*s == '#') && !s0)
        {
            s0 = s;
            s = modstr;

            /* Paranoia -- Never append multiple modstrs */
            modstr = "";
        }

        /* Begin to append the kind name */
        else if ((*s == '%') && !s0)
        {
            s0 = s;
            s = kindname;

            /* Paranoia -- Never append multiple kindnames */
            kindname = "";
        }

        /* Pluralizer */
        else if (*s == '~')
        {
            /* Add a plural if needed */
            if (!(mode & OD_NO_PLURAL) && ((number != 1) || (have_flag(o_ptr->flags, OF_PLURAL))))
            {
                char k = t[-1];

                /* XXX XXX XXX Mega-Hack */

                /* Hack -- "Cutlass-es" and "Torch-es", but "Photograph-s" */
                if ((k == 's') || ((k == 'h') && (o_ptr->tval != TV_STATUE))) *t++ = 'e';

                /* Add an 's' */
                *t++ = 's';
            }
            s++;
        }
        /* Normal */
        else
        {
            /* Copy */
            *t++ = *s++;
        }
    }

    /* Terminate */
    *t = '\0';


    if (object_is_smith(o_ptr))
    {
        t = object_desc_str(t,format(" of %s the Smith",player_name));
    }

    /* Hack -- Append "Artifact" or "Special" names */
    if (known && !have_flag(flgs, OF_FULL_NAME))
    {
        /* Is it a new random artifact ? */
        if (o_ptr->art_name)
        {
            if (o_ptr->art_name == 1) /* Out-of-room emergency quark */
            {
                char tmpbuf[20] = " 'Nameless Thing'";
                t = object_desc_str(t, tmpbuf);
            }
            else
            {
                t = object_desc_chr(t, ' ');
                t = object_desc_str(t, quark_str(o_ptr->art_name));
            }
        }

        /* Grab any artifact name */
        else if (object_is_fixed_artifact(o_ptr))
        {
            artifact_type *a_ptr = &a_info[o_ptr->name1];

            t = object_desc_chr(t, ' ');
            t = object_desc_str(t, a_name + a_ptr->name);
        }

        /* Grab any ego-item name */
        else
        {
            if (object_is_ego(o_ptr))
            {
                ego_type *e_ptr = &e_info[o_ptr->name2];

                t = object_desc_chr(t, ' ');
                t = object_desc_str(t, e_name + e_ptr->name);
            }

            if (o_ptr->inscription && my_strchr(quark_str(o_ptr->inscription), '#'))
            {
                /* Find the '#' */
                cptr str = my_strchr(quark_str(o_ptr->inscription), '#');

                /* Add the false name */
                t = object_desc_chr(t, ' ');
                t = object_desc_str(t, &str[1]);
            }
        }
    }

    if (known && object_is_device(o_ptr))
    {
        if (o_ptr->activation.type)
        {
            char buf[255];
            if (mode & OD_COLOR_CODED)
            {
                byte color = effect_color(&o_ptr->activation);
                sprintf(buf, ": <color:%c>%s</color>",
                        attr_to_attr_char(color),
                        do_effect(&o_ptr->activation, SPELL_NAME, 0));
            }
            else
            {
                sprintf(buf, ": %s", do_effect(&o_ptr->activation, SPELL_NAME, 0));
            }
            t = object_desc_str(t, buf);
        }
    }

    /* Hack - pluralize staves - only ever needed if there are none */
    if ((o_ptr->number < 1) && (o_ptr->tval == TV_STAFF))
    {
        int paikka = strpos("no more Staffs", tmp_val);
        if (paikka)
        {
            tmp_val[paikka + 10] = 'v';
            tmp_val[paikka + 11] = 'e';
        }
    }

    /* No more details wanted */
    if (mode & OD_NAME_ONLY) goto object_desc_done;

    /* Hack -- Chests must be described in detail */
    if (o_ptr->tval == TV_CHEST)
    {
        /* Not searched yet */
        if (!known)
        {
            /* Nothing */
        }

        /* May be "empty" */
        else if (!o_ptr->pval)
        {
            t = object_desc_str(t, " (empty)");
        }

        /* May be "disarmed" */
        else if (o_ptr->pval < 0)
        {
            if (chest_traps[0 - o_ptr->pval])
            {
                t = object_desc_str(t, " (disarmed)");
            }
            else
            {
                t = object_desc_str(t, " (unlocked)");
            }
        }

        /* Describe the traps, if any */
        else
        {
            /* Describe the traps */
            switch (chest_traps[o_ptr->pval])
            {
                case 0:
                {
                    t = object_desc_str(t, " (Locked)");
                    break;
                }
                case CHEST_LOSE_STR:
                {
                    t = object_desc_str(t, " (Poison Needle)");
                    break;
                }
                case CHEST_LOSE_CON:
                {
                    t = object_desc_str(t, " (Poison Needle)");
                    break;
                }
                case CHEST_POISON:
                {
                    t = object_desc_str(t, " (Gas Trap)");
                    break;
                }
                case CHEST_PARALYZE:
                {
                    t = object_desc_str(t, " (Gas Trap)");
                    break;
                }
                case CHEST_EXPLODE:
                {
                    t = object_desc_str(t, " (Explosion Device)");
                    break;
                }
                case CHEST_SUMMON:
                case CHEST_BIRD_STORM:
                case CHEST_E_SUMMON:
                case CHEST_H_SUMMON:
                {
                    t = object_desc_str(t, " (Summoning Runes)");
                    break;
                }
                case CHEST_RUNES_OF_EVIL:
                {
                    t = object_desc_str(t, " (Gleaming Black Runes)");
                    break;
                }
                case CHEST_ALARM:
                {
                    t = object_desc_str(t, " (Alarm)");
                    break;
                }
                default:
                {
                    t = object_desc_str(t, " (Multiple Traps)");
                    break;
                }
            }
        }
    }


    /* Display the item like a weapon */
    if (have_flag(flgs, OF_SHOW_MODS)) show_weapon = TRUE;

    /* Display the item like a weapon */
    if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;
    else if (o_ptr->to_h > 0) show_weapon = TRUE;
    else if (o_ptr->to_d < 0) show_weapon = TRUE;

    /* Display the item like armour */
    if (o_ptr->ac) show_armour = TRUE;


    /* Dump base weapon info */
    switch (o_ptr->tval)
    {
    /* Missiles and Weapons */
    case TV_SHOT:
    case TV_BOLT:
    case TV_ARROW:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_DIGGING:
        if (known)
        {
            int hand = equip_which_hand(o_ptr);
            int dd = o_ptr->dd;
            int ds = o_ptr->ds;

            if (p_ptr->big_shot && object_is_suitable_ammo(o_ptr))
                ds += 2;

            if (hand >= 0 && hand < MAX_HANDS && !(mode & OD_THROWING))
            {
                dd += p_ptr->weapon_info[hand].to_dd;
                ds += p_ptr->weapon_info[hand].to_ds;
            }

            /* Append a "damage" string */
            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, p1);
            t = object_desc_num(t, dd);
            t = object_desc_chr(t, 'd');
            t = object_desc_num(t, ds);
            t = object_desc_chr(t, p2);
        }
        /* All done */
        break;
    case TV_CORPSE:
    {
        if ((!o_ptr->dd) || (!o_ptr->xtra4)) break;
        /* Append a "damage" string */
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, p1);
        t = object_desc_num(t, o_ptr->dd);
        t = object_desc_chr(t, 'd');
        t = object_desc_num(t, o_ptr->ds);
        t = object_desc_chr(t, p2);
        break;
    }
    /* Bows get a special "damage string" */
    case TV_BOW:
    {
        char tmp[10];

        if (obj_is_fake_bow(o_ptr)) break;
        if (!known) break;

        /* Mega-Hack -- Extract the "base power" */
        power = o_ptr->mult;

        /* Are we describing a wielded bow? */
        if (equip_is_worn(o_ptr))
            power += p_ptr->shooter_info.to_mult;

        if (power % 100)
            sprintf(tmp, "x%d.%2.2d", power / 100, power % 100);
        else
            sprintf(tmp, "x%d", power / 100);

        /* Append a special "damage" string */
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, p1);
        t = object_desc_str(t, tmp);
        t = object_desc_chr(t, p2);

        /* All done */
        break;
    }
    case TV_QUIVER: /* show capacity */
        if (o_ptr->loc.where == INV_EQUIP)
            t = object_desc_str(t, format(" [%d of %d]", quiver_count(NULL), o_ptr->xtra4));
        else
            t = object_desc_str(t, format(" [%d]", o_ptr->xtra4));
        break;
    }

    if (mode & OD_NAME_AND_DICE) goto object_desc_done;

    /* Add the weapon bonuses */
    if (known)
    {
        if (obj_is_fake_bow(o_ptr))
        {
        }
        /* Show the tohit/todam on request */
        else if (show_weapon)
        {
            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, p1);
            t = object_desc_int(t, o_ptr->to_h);
            t = object_desc_chr(t, ',');
            t = object_desc_int(t, o_ptr->to_d);
            t = object_desc_chr(t, p2);
        }

        /* Show the tohit if needed */
        else if (o_ptr->to_h)
        {
            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, p1);
            t = object_desc_int(t, o_ptr->to_h);
            t = object_desc_chr(t, p2);
        }

        /* Show the todam if needed */
        else if (o_ptr->to_d)
        {
            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, p1);
            t = object_desc_int(t, o_ptr->to_d);
            t = object_desc_chr(t, p2);
        }
    }

    if ((player_is_ninja) && (o_ptr->tval == TV_SPIKE))
    {
        int avgdam = 1;
        s16b energy_fire = 100 - p_ptr->lev;

        avgdam += ((p_ptr->lev + 30) * (p_ptr->lev + 30) - 900) / 55;

        /* Display (shot damage/ avg damage) */
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, p1);
        t = object_desc_num(t, avgdam);
        t = object_desc_chr(t, '/');

        /* Calc effects of energy */
        avgdam = 100 * avgdam / energy_fire;

        t = object_desc_num(t, avgdam);
        t = object_desc_chr(t, p2);
    }

    /* Add the armor bonuses */
    if (known)
    {
        /* Show the armor class info */
        if (show_armour)
        {
            int ac = o_ptr->ac;
            int to_a = o_ptr->to_a;

            if (prace_is_(RACE_CENTAUR) && object_is_body_armour(o_ptr))
            {
                ac -= ac / 3;
                if (to_a > 0)
                    to_a -= to_a / 3;
            }

            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, b1);
            t = object_desc_num(t, ac);
            t = object_desc_chr(t, ',');
            t = object_desc_int(t, to_a);
            t = object_desc_chr(t, b2);
        }

        /* No base armor, but does increase armor */
        else if (o_ptr->to_a)
        {
            t = object_desc_chr(t, ' ');
            t = object_desc_chr(t, b1);
            t = object_desc_int(t, o_ptr->to_a);
            t = object_desc_chr(t, b2);
        }
    }

    /* Hack -- always show base armor
    else if (show_armour)
    {
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, b1);
        t = object_desc_num(t, o_ptr->ac);
        t = object_desc_chr(t, b2);
    } */

    if (o_ptr->rune)
    {
        t = object_desc_chr(t, ' ');
        if (mode & OD_COLOR_CODED)
            t = object_desc_str(t, "<color:B>");
        t = object_desc_str(t, rune_desc(o_ptr->rune));
        if (mode & OD_COLOR_CODED)
            t = object_desc_str(t, "</color>");
    }

    /* No more details wanted */
    if (mode & OD_NAME_AND_ENCHANT) goto object_desc_done;


    if (known)
    {
        if (object_is_device(o_ptr))
        {
            if (o_ptr->activation.cost)
            {
                t = object_desc_chr(t, ' ');
                t = object_desc_chr(t, p1);
                if (statistics_hack)
                {
                    double charges = (double)device_max_sp(o_ptr)/o_ptr->activation.cost;
                    t = object_desc_str(t, format("%.2f charges", charges));
                }
                else
                {
                    int  charges = device_sp(o_ptr) / o_ptr->activation.cost;
                    int  max_charges = device_max_sp(o_ptr) / o_ptr->activation.cost;

                    if (show_power)
                    {
                        int boost = (devicemaster_is_speciality(o_ptr)) ? (device_power_aux(100, p_ptr->device_power + p_ptr->lev / 10) - 100) : (device_power(100) - 100);
                        cptr info = do_device(o_ptr, SPELL_INFO, boost);
                        if ((info) && (strlen(info)) && ((obj_is_identified_fully(o_ptr)) || (o_ptr->known_xtra & OFL_DEVICE_POWER)) && (!strpos("dist", info)))
                        t = object_desc_str(t, format("%s, ", info));
                    }

                    if ((mode & OD_COLOR_CODED) && charges < max_charges)
                    {
                        if (!charges)
                            t = object_desc_str(t, format("<color:r>%d/%d%s</color>", charges, max_charges, show_power ? "" : " charges"));
                        else
                            t = object_desc_str(t, format("<color:y>%d/%d%s</color>", charges, max_charges, show_power ? "" : " charges"));
                    }
                    else
                        t = object_desc_str(t, format("%d/%d%s", charges, max_charges, show_power ? "" : " charges"));
                }
                t = object_desc_chr(t, p2);
            }
        }
        /* TODO: Devices need fixing, so we'll require Id for now */
        if (object_is_device(o_ptr) && o_ptr->pval)
        {
            if (o_ptr->name2 == EGO_DEVICE_POWER)
            {
            }
            else
            {
                t = object_desc_chr(t, ' ');
                t = object_desc_chr(t, p1);
                t = object_desc_int(t, o_ptr->pval);
                t = object_desc_chr(t, p2);
            }
        }
    }

    /* Learning about a pval flag on an unidentified object *should* display the pval!*/
    if (have_pval_flag(known_flgs) && !object_is_device(o_ptr) && o_ptr->pval)
    {
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, p1);
        t = object_desc_int(t, o_ptr->pval);
        t = object_desc_chr(t, p2);
    }
    if (have_flag(known_flgs, OF_DEVICE_POWER))
    {
        int pct = device_power_aux(100, (((prace_is_(RACE_MON_ARMOR)) && (o_ptr->loc.where == INV_EQUIP)) ? rag_effect_pval(o_ptr, -1, OF_DEVICE_POWER, FALSE) : o_ptr->pval)) - 100;
        if (pct >= 0)
            t = object_desc_str(t, format(" {+%d%%}", pct));
        else
            t = object_desc_str(t, format(" {%d%%}", pct));
    }
    else if (have_flag(known_flgs, OF_DEC_MAGIC_MASTERY))
    {
        int pct = device_power_aux(100, -o_ptr->pval) - 100;
        t = object_desc_str(t, format(" {%d%%}", pct));
    }

    if (have_flag(known_flgs, OF_SPELL_POWER))
    {
        int pct = spell_power_aux(100, o_ptr->pval) - 100;
        t = object_desc_str(t, format(" <+%d%%>", pct));
    }
    else if (have_flag(known_flgs, OF_DEC_SPELL_POWER))
    {
        int pct = spell_power_aux(100, -o_ptr->pval) - 100;
        t = object_desc_str(t, format(" <%d%%>", pct));
    }

    if (have_flag(known_flgs, OF_SPELL_CAP))
    {
        int pct = spell_cap_aux(100, o_ptr->pval) - 100;
        if (pct > 0)
            t = object_desc_str(t, format(" [+%d%%]", pct));
        else
            t = object_desc_str(t, format(" [%d%%]", pct));
    }
    else if (have_flag(known_flgs, OF_DEC_SPELL_CAP))
    {
        int pct = spell_cap_aux(100, -o_ptr->pval) - 100;
        t = object_desc_str(t, format(" [%d%%]", pct));
    }

    if (known)
    {
        /* Hack -- Process Lanterns/Torches */
        if ((object_needs_fuel(o_ptr)) && (!(o_ptr->name1 || o_ptr->art_name)))
        {
            /* Hack -- Turns of light for normal lites */
            t = object_desc_str(t, " (with ");

            if (o_ptr->name2 == EGO_LITE_DURATION) t = object_desc_num(t, o_ptr->xtra4 * 2);
            else t = object_desc_num(t, o_ptr->xtra4);
            t = object_desc_str(t, " turns of light)");
        }

        /* Indicate charging objects, but not rods. */
        if (o_ptr->timeout && (o_ptr->tval != TV_ROD))
        {
            t = object_desc_str(t, " (charging)");
        }
    }


    /* No more details wanted */
    if (mode & OD_OMIT_INSCRIPTION) goto object_desc_done;


    /* Prepare real inscriptions in a buffer */
    tmp_val2[0] = '\0';

    /* Auto abbreviation inscribe */
    if (abbrev_extra || abbrev_all)
    {
        if (!o_ptr->inscription || !my_strchr(quark_str(o_ptr->inscription), '%'))
        {
            bool all = abbrev_all;
            if (!obj_is_identified(o_ptr)) /* otherwise, this pseudo leaks the underlying name */
                all = TRUE;
            get_ability_abbreviation(tmp_val2, o_ptr, all);
        }
    }

    if ( have_flag(known_flgs, OF_ACTIVATE)
      && obj_has_effect(o_ptr)
      && !device
      && (abbrev_all || (abbrev_extra && o_ptr->activation.type) || !obj_is_identified(o_ptr)) )
    {
        char     buf[255];
        effect_t e = obj_get_effect(o_ptr);

        if (strlen(tmp_val2) > 0)
            strcat(tmp_val2, " ");

        if ((o_ptr->tval == TV_CAPTURE) && (o_ptr->pval > 0))
        { /* Special case */
            if (mode & OD_COLOR_CODED)
                sprintf(buf, "<color:B>A:Release Pet</color>");
            else
                sprintf(buf, "A:Release Pet");
        }
        else
        {            
            if (mode & OD_COLOR_CODED)
                sprintf(buf, "<color:B>A:%s</color>", do_effect(&e, SPELL_NAME, 0));
            else
                sprintf(buf, "A:%s", do_effect(&e, SPELL_NAME, 0));
        }
        strcat(tmp_val2, buf);
    }

    if ((o_ptr->tval == TV_CORPSE) && (o_ptr->xtra5))
    {
        gf_info_ptr gf = gf_lookup(o_ptr->xtra5);
        char buf[255];
        sprintf(buf, "%sU:Breathe %s%s", (mode & OD_COLOR_CODED) ? "<color:G>" : "", gf ? gf->name : "Buggy", (mode & OD_COLOR_CODED) ? "</color>" : "");
        if (strlen(tmp_val2) > 0)
            strcat(tmp_val2, " ");
        strcat(tmp_val2, buf);
    }

    if (object_is_device(o_ptr) && obj_is_identified_fully(o_ptr))
    {
        int  fail = device_calc_fail_rate(o_ptr);
        strcat(tmp_val2, format("%d%%", (fail + 5)/10));
        if ((statistics_hack) || ((!show_power) && (mode & OD_SHOW_DEVICE_INFO)))
        {
            cptr info = do_device(o_ptr, SPELL_INFO, 0);
            if (info && strlen(info))
            {
                char buf[255];
                sprintf(buf, " %s", info);
                strcat(tmp_val2, buf);
            }
        }
        if (statistics_hack)
        {
            char buf[255];
            sprintf(buf, " %dsp", o_ptr->activation.cost);
            strcat(tmp_val2, buf);
            sprintf(buf, " P%d D%d", o_ptr->activation.power, o_ptr->activation.difficulty);
            strcat(tmp_val2, buf);
        }
    }

    if (o_ptr->name3 && object_is_known(o_ptr) && abbrev_all)
    {
        cptr  t = a_name + a_info[o_ptr->name3].name;
        if ((strlen(t) > 2) && (t[0] == '&')) t += 2;

        if (!o_ptr->art_name || !strpos(t, quark_str(o_ptr->art_name)))
        {
            {
                char  buf[255];
                char *u = buf;
    
                /* of Hammerhand -> Hammerhand
                   'Thalkettoth' -> Thalkettoth
                   of the Dwarves -> Dwarves
                */
                if (*t == 'o' && *(t+1) == 'f')
                     t += 2;

                while (*t && *t == ' ')
                    t++;

                if (*t == 't' && *(t+1) == 'h' && *(t+2) == 'e')
                     t += 3;

                while (*t && *t == ' ')
                    t++;

                *u++ = ' ';
                while (*t)
                {
                    if (*t == '\'' || *t == '&' || *t == '~')
                        t++;
                    else
                        *u++ = *t++;
                }

                *u = '\0';
                strcat(tmp_val2, buf);
            }
        }
    }

    /* Use the standard inscription if available */
    if (o_ptr->inscription)
    {
        char buff[1024];

        if (tmp_val2[0]) strcat(tmp_val2, ", ");

        /* Get inscription and convert {%} */
        get_inscription(buff, o_ptr);

        /* strcat with correct treating of kanji */
        my_strcat(tmp_val2, buff, sizeof(tmp_val2));
    }


    /* No fake inscription yet */
    fake_insc_buf[0] = '\0';

    /* Use the game-generated "feeling" otherwise, if available */
    if (o_ptr->feeling)
    {
        strcpy(fake_insc_buf, game_inscriptions[o_ptr->feeling]);
    }

    else if ((p_ptr->munchkin_pseudo_id) && ((obj_can_sense1(o_ptr)) || (obj_can_sense2(o_ptr))) &&
             (!(o_ptr->ident & IDENT_SENSE)) && (!object_is_known(o_ptr)))
    {
        o_ptr->feeling = value_check_aux1(o_ptr, TRUE);
        if (!(o_ptr->ident & IDENT_KNOWN)) o_ptr->ident |= IDENT_SENSE; 
        if (o_ptr->feeling) strcpy(fake_insc_buf, game_inscriptions[o_ptr->feeling]);
    }

    /* Note "cursed" if the item is known to be cursed */
    else if (object_is_cursed(o_ptr) && (known || (o_ptr->ident & IDENT_SENSE)))
    {
        if (object_is_device(o_ptr) && !obj_is_identified_fully(o_ptr))
        {
            /* Hide cursed status of devices until *Identified* */
        }
        else
            strcpy(fake_insc_buf, "cursed");
    }

    /* Note "unidentified" if the item is unidentified */
    else if ( (o_ptr->tval == TV_LITE || o_ptr->tval == TV_FIGURINE || o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET)
           && aware 
           && !known
           && !(o_ptr->ident & IDENT_SENSE) )
    {
        strcpy(fake_insc_buf, "unidentified");
    }
    /* Mega-Hack -- note empty wands/staffs */
    else if (!known && (o_ptr->ident & IDENT_EMPTY))
    {
        strcpy(fake_insc_buf, "empty");
    }

    /* Note "tried" if the object has been tested unsuccessfully */
    else if (!known && object_is_device(o_ptr) && object_is_tried(o_ptr))
    {
        strcpy(fake_insc_buf, "tried");
    }
    else if (!aware && object_is_tried(o_ptr))
    {
        strcpy(fake_insc_buf, "tried");
    }
    else if ((shops_mark_unseen) && (aware) && (!object_is_aware(o_ptr)) && (object_is_flavor(o_ptr)) && (o_ptr->loc.where == INV_SHOP))
    {
        strcpy(fake_insc_buf, "unseen");
    }

    /* Note the discount, if any */
    if (o_ptr->discount)
    {
        if ((o_ptr->ident & IDENT_STORE) || show_discounts)
        {
            char discount_num_buf[4];

            /* Append to other fake inscriptions if any */
            if (fake_insc_buf[0]) strcat(fake_insc_buf, ", ");

            (void)object_desc_num(discount_num_buf, o_ptr->discount);
            strcat(fake_insc_buf, discount_num_buf);
            strcat(fake_insc_buf, "% off");
        }
    }


    /* Append the inscription, if any */
    if (fake_insc_buf[0] || tmp_val2[0])
    {
        /* Append the inscription */
        t = object_desc_chr(t, ' ');
        t = object_desc_chr(t, c1);

        /* Append fake inscriptions */
        if (fake_insc_buf[0])
        {
            t = object_desc_str(t, fake_insc_buf);
        }

        /* Append a separater */
        if (fake_insc_buf[0] && tmp_val2[0])
        {
            t = object_desc_chr(t, ',');
            t = object_desc_chr(t, ' ');
        }

        /* Append real inscriptions */
        if (tmp_val2[0])
        {
            t = object_desc_str(t, tmp_val2);
        }

        t = object_desc_chr(t, c2);
    }

object_desc_done:
    if (mode & OD_COLOR_CODED)
        sprintf(buf, "<color:%c>%s</color>", tval_to_attr_char(o_ptr->tval), tmp_val);
    else
        my_strcpy(buf, tmp_val, MAX_NLEN);
}


