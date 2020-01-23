/* File: autopick.c */

/* Purpose: The Mogaminator! */

/*
 * Copyright (c) 2002  Mogami
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include <assert.h>

#define MAX_LINELEN 1024

bool autopick_auto_id(object_type *o_ptr);

/*
  Rules have one of the following two syntactic forms:
  [Commands] Adjective* Noun [Special-Clause] [:Search-String] [#Inscription-String]
  [Commands] Search-String [#Inscription-String]
*/

/*
 * Macros for Keywords
 */
enum _keyword_e {
/* Adjectives */
    FLG_ALL = 0,

    /* Object Knowledge */
    FLG_UNSENSED,
    FLG_UNIDENTIFIED,
    FLG_IDENTIFIED,
    FLG_STAR_IDENTIFIED,
    FLG_UNAWARE,

    /* Object Quality */
    FLG_AVERAGE,
    FLG_GOOD,
    FLG_EGO,
    FLG_ARTIFACT,
    FLG_NAMELESS,
    FLG_RARE,
    FLG_COMMON,
    FLG_CURSED,
    FLG_WORTHLESS,

    /* Object Attributes */
    FLG_BOOSTED,
    FLG_ICKY,

    /* Spellbooks */
    FLG_UNREADABLE,
    FLG_REALM1,
    FLG_REALM2,
    FLG_FIRST,
    FLG_SECOND,
    FLG_THIRD,
    FLG_FOURTH,

    /* Miscellaneous */
    FLG_COLLECTING,
    FLG_SPECIAL,
    FLG_UNUSABLE,  /* For example, a Dragon cannot wear boots! */
    FLG_WANTED,
    FLG_UNIQUE,
    FLG_HUMAN,

/* Special Forms */
    FLG_MORE_DICE,
    FLG_MORE_BONUS,
    FLG_MORE_LEVEL,
    FLG_MORE_WEIGHT,
    FLG_MORE_CHARGES,
    FLG_MORE_VALUE,

/* Nouns */
    FLG_ITEMS,

    /* Wearable Items */
    FLG_WEAPONS,
        FLG_FAVORITE_WEAPONS,
        FLG_HAFTED,
        FLG_DIGGERS,
    FLG_SHOOTERS,
    FLG_AMMO,
    FLG_ARMORS,
        FLG_SHIELDS,
        FLG_SUITS,
        FLG_CLOAKS,
        FLG_HELMS,
        FLG_GLOVES,
        FLG_BOOTS,
    FLG_LIGHTS,
    FLG_RINGS,
    FLG_AMULETS,

    FLG_SPELLBOOKS,

    /* Devices */
    FLG_WANDS,
    FLG_STAVES,
    FLG_RODS,
    FLG_POTIONS,
    FLG_SCROLLS,

    FLG_JUNKS,
    FLG_CORPSES,
    FLG_SKELETONS,

    FLG_MAX, /* watch out for AUTOPICK_FLAG_SIZE */
};

#define FLG_ADJECTIVE_BEGIN     FLG_ALL
#define FLG_ADJECTIVE_END       FLG_HUMAN
#define FLG_KNOWLEDGE_BEGIN     FLG_UNSENSED
#define FLG_KNOWLEDGE_END       FLG_UNAWARE
#define FLG_QUALITY_BEGIN       FLG_AVERAGE
#define FLG_QUALITY_END         FLG_NAMELESS
#define FLG_SPECIAL_FORM_BEGIN  FLG_MORE_DICE
#define FLG_SPECIAL_FORM_END    FLG_MORE_VALUE
#define FLG_NOUN_BEGIN          FLG_ITEMS
#define FLG_NOUN_END            (FLG_MAX - 1)


static char KEY_ALL[] = "all";

static char KEY_UNSENSED[] = "unsensed";
static char KEY_UNIDENTIFIED[] = "unidentified";
static char KEY_IDENTIFIED[] = "identified";
static char KEY_STAR_IDENTIFIED[] = "*identified*";
static char KEY_UNAWARE[] = "unaware";

static char KEY_AVERAGE[] = "average";
static char KEY_GOOD[] = "good";
static char KEY_CURSED[] = "cursed";
static char KEY_EGO[] = "ego";
static char KEY_ARTIFACT[] = "artifact";
static char KEY_NAMELESS[] = "nameless";
static char KEY_RARE[] = "rare";
static char KEY_COMMON[] = "common";
static char KEY_WORTHLESS[] = "worthless";

static char KEY_BOOSTED[] = "dice boosted";
static char KEY_ICKY[] = "icky";

static char KEY_UNREADABLE[] = "unreadable";
static char KEY_REALM1[] = "first realm's";
static char KEY_REALM2[] = "second realm's";
static char KEY_FIRST[] = "first";
static char KEY_SECOND[] = "second";
static char KEY_THIRD[] = "third";
static char KEY_FOURTH[] = "fourth";

static char KEY_COLLECTING[] = "collecting";
static char KEY_SPECIAL[] = "special";
static char KEY_UNUSABLE[] = "unusable";
static char KEY_WANTED[] = "wanted";
static char KEY_UNIQUE[] = "unique";
static char KEY_HUMAN[] = "human";

static char KEY_MORE_DICE[] =  "more dice than";
static char KEY_MORE_BONUS[] =  "more bonus than";
static char KEY_MORE_LEVEL[] =  "more level than";
static char KEY_MORE_WEIGHT[] =  "more weight than";
static char KEY_MORE_CHARGES[] =  "more charges than";
static char KEY_MORE_VALUE[] =  "more value than";

static char KEY_ITEMS[] = "items";

static char KEY_WEAPONS[] = "weapons";
    static char KEY_FAVORITE_WEAPONS[] = "favorite weapons";
    static char KEY_HAFTED[] = "hafted weapons";
    static char KEY_DIGGERS[] = "diggers";

static char KEY_SHOOTERS[] = "shooters";
static char KEY_AMMO[] = "ammo";

static char KEY_ARMORS[] = "armors";
    static char KEY_SHIELDS[] = "shields";
    static char KEY_SUITS[] = "suits";
    static char KEY_CLOAKS[] = "cloaks";
    static char KEY_HELMS[] = "helms";
    static char KEY_GLOVES[] = "gloves";
    static char KEY_BOOTS[] = "boots";

static char KEY_LIGHTS[] = "lights";
static char KEY_RINGS[] = "rings";
static char KEY_AMULETS[] = "amulets";

static char KEY_SPELLBOOKS[] = "spellbooks";

static char KEY_WANDS[] = "wands";
static char KEY_STAVES[] = "staves";
static char KEY_RODS[] = "rods";
static char KEY_POTIONS[] = "potions";
static char KEY_SCROLLS[] = "scrolls";

static char KEY_JUNKS[] = "junk";
static char KEY_CORPSES[] = "corpses";
static char KEY_SKELETONS[] = "skeletons";


#define MATCH_KEY(KEY) (!strncmp(ptr, KEY, sizeof(KEY)-1)\
     ? (ptr += sizeof(KEY)-1, (' '==*ptr) ? ptr++ : 0, TRUE) : FALSE)
#define MATCH_KEY2(KEY) (!strncmp(ptr, KEY, sizeof(KEY)-1)\
     ? (prev_ptr = ptr, ptr += sizeof(KEY)-1, (' '==*ptr) ? ptr++ : 0, TRUE) : FALSE)

#define ADD_FLG(FLG) (entry->flag[FLG / 32] |= (1L << (FLG % 32)))
#define REM_FLG(FLG) (entry->flag[FLG / 32] &= ~(1L << (FLG % 32)))
#define ADD_FLG_NOUN(FLG) (ADD_FLG(FLG), prev_flg = FLG)
#define IS_FLG(FLG) (entry->flag[FLG / 32] & (1L << (FLG % 32)))


/*
 * A function to create new entry
 */
static bool autopick_new_entry(autopick_type *entry, cptr str, bool allow_default)
{
    cptr insc;
    int i;
    byte act = 0;
    char buf[MAX_LINELEN];
    cptr prev_ptr, ptr, old_ptr;
    int prev_flg;

    if (str[0] && str[1] == ':') switch (str[0])
    {
    case '?': case '%':
    case 'A': case 'P': case 'C':
        return FALSE;
    }

    entry->flag[0] = entry->flag[1] = 0L;
    entry->dice = 0;
    entry->bonus = 0;

    act = DO_AUTOPICK | DO_DISPLAY;
    while (TRUE)
    {
        if ((act & DO_AUTOPICK) && *str == '!')
        {
            act &= ~DO_AUTOPICK;
            act |= DO_AUTODESTROY;
            str++;
        }
        else if ((act & DO_AUTOPICK) && *str == '~')
        {
            act &= ~DO_AUTOPICK;
            act |= DONT_AUTOPICK;
            str++;
        }
        else if ((act & DO_AUTOPICK) && *str == ';')
        {
            act &= ~DO_AUTOPICK;
            act |= DO_QUERY_AUTOPICK;
            str++;
        }
        else if ((act & DO_DISPLAY) && *str == '(')
        {
            act &= ~DO_DISPLAY;
            str++;
        }
        else if (*str == '?')
        {
            act |= DO_AUTO_ID;
            str++;
        }
        else
            break;
    }

    /* don't mind upper or lower case */
    insc = NULL;
    for (i = 0; *str; i++)
    {
        char c = *str++;
        /* Auto-inscription? */
        if (c == '#')
        {
            buf[i] = '\0';
            insc = str;
            break;
        }

        if (isupper(c)) c = tolower(c);

        buf[i] = c;
    }
    buf[i] = '\0';

    /* Skip empty line unless allow_default */
    if (!allow_default && *buf == 0) return FALSE;

    /* Skip comment line */
    if (*buf == 0 && insc) return FALSE;

    ptr = prev_ptr = buf;
    old_ptr = NULL;

    /* Adjective* */
    while (old_ptr != ptr)
    {
        /* Save current location */
        old_ptr = ptr;

        if (MATCH_KEY(KEY_ALL)) ADD_FLG(FLG_ALL);

        if (MATCH_KEY(KEY_UNSENSED)) ADD_FLG(FLG_UNSENSED);
        if (MATCH_KEY(KEY_UNIDENTIFIED)) ADD_FLG(FLG_UNIDENTIFIED);
        if (MATCH_KEY(KEY_IDENTIFIED)) ADD_FLG(FLG_IDENTIFIED);
        if (MATCH_KEY(KEY_STAR_IDENTIFIED)) ADD_FLG(FLG_STAR_IDENTIFIED);
        if (MATCH_KEY(KEY_UNAWARE)) ADD_FLG(FLG_UNAWARE);

        if (MATCH_KEY(KEY_AVERAGE)) ADD_FLG(FLG_AVERAGE);
        if (MATCH_KEY(KEY_GOOD)) ADD_FLG(FLG_GOOD);
        if (MATCH_KEY(KEY_EGO)) ADD_FLG(FLG_EGO);
        if (MATCH_KEY(KEY_ARTIFACT)) ADD_FLG(FLG_ARTIFACT);
        if (MATCH_KEY(KEY_CURSED)) ADD_FLG(FLG_CURSED);
        if (MATCH_KEY(KEY_NAMELESS)) ADD_FLG(FLG_NAMELESS);
        if (MATCH_KEY(KEY_RARE)) ADD_FLG(FLG_RARE);
        if (MATCH_KEY(KEY_COMMON)) ADD_FLG(FLG_COMMON);
        if (MATCH_KEY(KEY_WORTHLESS)) ADD_FLG(FLG_WORTHLESS);

        if (MATCH_KEY(KEY_BOOSTED)) ADD_FLG(FLG_BOOSTED);
        if (MATCH_KEY(KEY_ICKY)) ADD_FLG(FLG_ICKY);

        if (MATCH_KEY(KEY_UNREADABLE)) ADD_FLG(FLG_UNREADABLE);
        if (MATCH_KEY(KEY_REALM1)) ADD_FLG(FLG_REALM1);
        if (MATCH_KEY(KEY_REALM2)) ADD_FLG(FLG_REALM2);
        if (MATCH_KEY(KEY_FIRST)) ADD_FLG(FLG_FIRST);
        if (MATCH_KEY(KEY_SECOND)) ADD_FLG(FLG_SECOND);
        if (MATCH_KEY(KEY_THIRD)) ADD_FLG(FLG_THIRD);
        if (MATCH_KEY(KEY_FOURTH)) ADD_FLG(FLG_FOURTH);

        if (MATCH_KEY(KEY_COLLECTING)) ADD_FLG(FLG_COLLECTING);
        if (MATCH_KEY(KEY_SPECIAL)) ADD_FLG(FLG_SPECIAL);
        if (MATCH_KEY(KEY_UNUSABLE)) ADD_FLG(FLG_UNUSABLE);
        if (MATCH_KEY(KEY_WANTED)) ADD_FLG(FLG_WANTED);
        if (MATCH_KEY(KEY_UNIQUE)) ADD_FLG(FLG_UNIQUE);
        if (MATCH_KEY(KEY_HUMAN)) ADD_FLG(FLG_HUMAN);
    }

    /* Not yet found any noun */
    prev_flg = -1;

    /* Noun */
    if (MATCH_KEY2(KEY_ITEMS)) ADD_FLG_NOUN(FLG_ITEMS);

    else if (MATCH_KEY2(KEY_WEAPONS)) ADD_FLG_NOUN(FLG_WEAPONS);
    else if (MATCH_KEY2(KEY_FAVORITE_WEAPONS)) ADD_FLG_NOUN(FLG_FAVORITE_WEAPONS);
    else if (MATCH_KEY2(KEY_HAFTED)) ADD_FLG_NOUN(FLG_HAFTED);
    else if (MATCH_KEY2(KEY_DIGGERS)) ADD_FLG_NOUN(FLG_DIGGERS);

    else if (MATCH_KEY2(KEY_SHOOTERS)) ADD_FLG_NOUN(FLG_SHOOTERS);
    else if (MATCH_KEY2(KEY_AMMO)) ADD_FLG_NOUN(FLG_AMMO);

    else if (MATCH_KEY2(KEY_ARMORS)) ADD_FLG_NOUN(FLG_ARMORS);
    else if (MATCH_KEY2(KEY_SHIELDS)) ADD_FLG_NOUN(FLG_SHIELDS);
    else if (MATCH_KEY2(KEY_SUITS)) ADD_FLG_NOUN(FLG_SUITS);
    else if (MATCH_KEY2(KEY_CLOAKS)) ADD_FLG_NOUN(FLG_CLOAKS);
    else if (MATCH_KEY2(KEY_HELMS)) ADD_FLG_NOUN(FLG_HELMS);
    else if (MATCH_KEY2(KEY_GLOVES)) ADD_FLG_NOUN(FLG_GLOVES);
    else if (MATCH_KEY2(KEY_BOOTS)) ADD_FLG_NOUN(FLG_BOOTS);

    else if (MATCH_KEY2(KEY_LIGHTS)) ADD_FLG_NOUN(FLG_LIGHTS);
    else if (MATCH_KEY2(KEY_RINGS)) ADD_FLG_NOUN(FLG_RINGS);
    else if (MATCH_KEY2(KEY_AMULETS)) ADD_FLG_NOUN(FLG_AMULETS);

    else if (MATCH_KEY2(KEY_SPELLBOOKS)) ADD_FLG_NOUN(FLG_SPELLBOOKS);

    else if (MATCH_KEY2(KEY_WANDS)) ADD_FLG_NOUN(FLG_WANDS);
    else if (MATCH_KEY2(KEY_STAVES)) ADD_FLG_NOUN(FLG_STAVES);
    else if (MATCH_KEY2(KEY_RODS)) ADD_FLG_NOUN(FLG_RODS);
    else if (MATCH_KEY2(KEY_POTIONS)) ADD_FLG_NOUN(FLG_POTIONS);
    else if (MATCH_KEY2(KEY_SCROLLS)) ADD_FLG_NOUN(FLG_SCROLLS);

    else if (MATCH_KEY2(KEY_JUNKS)) ADD_FLG_NOUN(FLG_JUNKS);
    else if (MATCH_KEY2(KEY_CORPSES)) ADD_FLG_NOUN(FLG_CORPSES);
    else if (MATCH_KEY2(KEY_SKELETONS)) ADD_FLG_NOUN(FLG_SKELETONS);

    /* Special-Clause */
    /*** Weapons whose dd*ds is more than nn ***/
    if (MATCH_KEY2(KEY_MORE_DICE))
    {
        int k = 0;
        entry->dice = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->dice = 10 * entry->dice + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 2)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_DICE);
        }
        else
            ptr = prev_ptr;
    }

    /*** Items whose magical bonus is more than n ***/
    if (MATCH_KEY2(KEY_MORE_BONUS))
    {
        int k = 0;
        entry->bonus = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->bonus = 10 * entry->bonus + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 2)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_BONUS);
        }
        else
            ptr = prev_ptr;
    }

    if (MATCH_KEY2(KEY_MORE_LEVEL))
    {
        int k = 0;
        entry->level = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->level = 10 * entry->level + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 2)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_LEVEL);
        }
        else
            ptr = prev_ptr;
    }

    if (MATCH_KEY2(KEY_MORE_WEIGHT))
    {
        int k = 0;
        entry->weight = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->weight = 10 * entry->weight + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 2)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_WEIGHT);
        }
        else
            ptr = prev_ptr;
    }

    if (MATCH_KEY2(KEY_MORE_CHARGES))
    {
        int k = 0;
        entry->charges = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->charges = 10 * entry->charges + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 3)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_CHARGES);
        }
        else
            ptr = prev_ptr;
    }

    if (MATCH_KEY2(KEY_MORE_VALUE))
    {
        int k = 0;
        entry->value = 0;

        /* Drop leading spaces */
        while (' ' == *ptr) ptr++;

        /* Read number */
        while ('0' <= *ptr && *ptr <= '9')
        {
            entry->value = 10 * entry->value + (*ptr - '0');
            ptr++;
            k++;
        }

        if (k > 0 && k <= 6)
        {
            if (' ' == *ptr) ptr++;
            ADD_FLG(FLG_MORE_VALUE);
        }
        else
            ptr = prev_ptr;
    }

    /* Last 'keyword' must be at the correct location */
    if (*ptr == ':')
        ptr++;
    else if (*ptr == '\0')
    {
        /* There was no noun */
        if (prev_flg == -1)

        /* Add extra word "items" */
        ADD_FLG_NOUN(FLG_ITEMS);
    }
    else
    {
        /* Noun type? */
        if (prev_flg != -1)
        {
            /* A noun type keyword didn't end correctly */
            entry->flag[prev_flg/32] &= ~(1L<< (prev_flg%32));
            ptr = prev_ptr;
        }
    }

    /* Save this auto-picker entry line */
    entry->name = z_string_make(ptr);
    entry->action = act;
    entry->insc = z_string_make(insc);

    return TRUE;
}

/* Check object's ickiness */
static bool _object_is_icky(object_type *o_ptr)
{
    if (!object_is_wearable(o_ptr)) return FALSE;
    else {
        class_t *class_ptr = get_class();
        if ((o_ptr->tval == TV_GLOVES) && (class_ptr->caster_info) && ((obj_is_identified(o_ptr)) || (o_ptr->feeling == FEEL_AVERAGE) || (o_ptr->feeling == FEEL_GOOD)))
        {
            if (get_caster_info()->options & CASTER_GLOVE_ENCUMBRANCE)
            {
                u32b flgs[OF_ARRAY_SIZE];
                obj_flags(o_ptr, flgs);
                if (!(have_flag(flgs, OF_FREE_ACT)) && !(have_flag(flgs, OF_MAGIC_MASTERY)) && !((have_flag(flgs, OF_DEX)) && (o_ptr->pval > 0))) return TRUE;
            }
        }
        if (class_ptr->known_icky_object) return class_ptr->known_icky_object(o_ptr);
        return FALSE;
    }
}

/*
 * Get auto-picker entry from o_ptr.
 */
static void autopick_entry_from_object(autopick_type *entry, object_type *o_ptr)
{
    /* Assume that object name is to be added */
    bool name = TRUE;

    char name_str[MAX_NLEN];

    /* Initialize name string */
    name_str[0] = '\0';

    entry->insc = z_string_make(quark_str(o_ptr->inscription));
    entry->action = DO_AUTOPICK | DO_DISPLAY;
    entry->flag[0] = entry->flag[1] = 0L;
    entry->dice = 0;

    /* Unaware */
    if (!object_is_aware(o_ptr))
    {
        ADD_FLG(FLG_UNAWARE);
    }
    /* Not really identified */
    else if (!object_is_known(o_ptr))
    {
        if (!(o_ptr->ident & IDENT_SENSE))
        {
            ADD_FLG(FLG_UNIDENTIFIED);
        }
        else
        {
            /* Pseudo-identified */
            switch (o_ptr->feeling)
            {
            case FEEL_CURSED:
                ADD_FLG(FLG_CURSED);
                break;

            case FEEL_ENCHANTED:
                /* XXX No appropriate flag */
                /* ADD_FLG(); */
                break;

            case FEEL_AVERAGE:
                ADD_FLG(FLG_AVERAGE);
                break;

            case FEEL_GOOD:
                ADD_FLG(FLG_GOOD);
                break;

            case FEEL_BAD:
                ADD_FLG(FLG_CURSED);
                break;

            case FEEL_EXCELLENT:
                ADD_FLG(FLG_EGO);
                break;

            case FEEL_AWFUL:
                ADD_FLG(FLG_CURSED);
                ADD_FLG(FLG_EGO);
                break;

            case FEEL_SPECIAL:
                ADD_FLG(FLG_ARTIFACT);
                break;

            case FEEL_TERRIBLE:
                ADD_FLG(FLG_CURSED);
                ADD_FLG(FLG_ARTIFACT);
                break;

            case FEEL_BROKEN:
                ADD_FLG(FLG_NAMELESS);
                ADD_FLG(FLG_WORTHLESS);
                break;

            default:
                /* Never reach here */
                break;
            }
        }
    }

    /* Identified */
    else
    {
        /* Ego objects */
        if (object_is_ego(o_ptr))
        {
            if (object_is_weapon_armour_ammo(o_ptr))
            {
                /*
                 * Base name of ego weapons and armors
                 * are almost meaningless.
                 * Register the ego type only.
                 */
                ego_type *e_ptr = &e_info[o_ptr->name2];
                /* We ommit the basename and cannot use the ^ mark */
                strcpy(name_str, e_name + e_ptr->name);

                /* Don't use the object description */
                name = FALSE;

                /* Restrict to 'common' equipments */
                if (!object_is_rare(o_ptr)) ADD_FLG(FLG_COMMON);
            }

            if (!object_is_device(o_ptr))
                ADD_FLG(FLG_EGO);
        }

        /* Artifact */
        else if (object_is_artifact(o_ptr))
            ADD_FLG(FLG_ARTIFACT);

        /* Non-ego, non-artifact */
        else
        {
            /* Wearable nameless object */
            if (object_is_ammo(o_ptr))
            {
                if (o_ptr->to_h || o_ptr->to_d)
                    ADD_FLG(FLG_GOOD);
                else
                    ADD_FLG(FLG_AVERAGE);
            }
            else if (object_is_equipment(o_ptr))
                ADD_FLG(FLG_NAMELESS);
        }

        /*Devices work better if we just use the effect name */
        if (object_is_device(o_ptr) && o_ptr->activation.type != EFFECT_NONE)
        {
            strcpy(name_str, do_device(o_ptr, SPELL_NAME, 0));
            strcat(name_str, "$");
            name = FALSE;
        }
    }

    /* Melee weapon with boosted dice */
    if (object_is_melee_weapon(o_ptr))
    {
        object_kind *k_ptr = &k_info[o_ptr->k_idx];

        if ((o_ptr->dd != k_ptr->dd) || (o_ptr->ds != k_ptr->ds))
            ADD_FLG(FLG_BOOSTED);
    }

    /* Wanted monster's corpse */
    if (object_is_shoukinkubi(o_ptr))
    {
        REM_FLG(FLG_WORTHLESS);
        ADD_FLG(FLG_WANTED);
    }

    if ((o_ptr->tval == TV_CORPSE || o_ptr->tval == TV_STATUE)
        && (r_info[o_ptr->pval].flags1 & RF1_UNIQUE))
    {
        ADD_FLG(FLG_UNIQUE);
    }

    if (o_ptr->tval == TV_CORPSE && my_strchr("pht", r_info[o_ptr->pval].d_char))
    {
        ADD_FLG(FLG_HUMAN);
    }

    if (o_ptr->tval >= TV_LIFE_BOOK &&
        !check_book_realm(o_ptr->tval, o_ptr->sval))
    {
        ADD_FLG(FLG_UNREADABLE);
        if (o_ptr->tval != TV_ARCANE_BOOK) name = FALSE;
    }

    if (REALM1_BOOK == o_ptr->tval &&
        p_ptr->pclass != CLASS_SORCERER &&
        p_ptr->pclass != CLASS_RED_MAGE)
    {
        ADD_FLG(FLG_REALM1);
        name = FALSE;
    }

    if (REALM2_BOOK == o_ptr->tval &&
        p_ptr->pclass != CLASS_SORCERER &&
        p_ptr->pclass != CLASS_RED_MAGE)
    {
        ADD_FLG(FLG_REALM2);
        name = FALSE;
    }

    if (_object_is_icky(o_ptr)) ADD_FLG(FLG_ICKY);

    if (o_ptr->tval >= TV_LIFE_BOOK && 0 == o_ptr->sval)
        ADD_FLG(FLG_FIRST);
    if (o_ptr->tval >= TV_LIFE_BOOK && 1 == o_ptr->sval)
        ADD_FLG(FLG_SECOND);
    if (o_ptr->tval >= TV_LIFE_BOOK && 2 == o_ptr->sval)
        ADD_FLG(FLG_THIRD);
    if (o_ptr->tval >= TV_LIFE_BOOK && 3 == o_ptr->sval)
        ADD_FLG(FLG_FOURTH);

    if (o_ptr->tval == TV_DIGGING)
        ADD_FLG(FLG_DIGGERS);
    else if (object_is_melee_weapon(o_ptr))
        ADD_FLG(FLG_WEAPONS);
    else if (o_ptr->tval == TV_BOW)
        ADD_FLG(FLG_SHOOTERS);
    else if (object_is_ammo(o_ptr))
        ADD_FLG(FLG_AMMO);
    else if (o_ptr->tval == TV_WAND)
        ADD_FLG(FLG_WANDS);
    else if (o_ptr->tval == TV_STAFF)
        ADD_FLG(FLG_STAVES);
    else if (o_ptr->tval == TV_ROD)
        ADD_FLG(FLG_RODS);
    else if (o_ptr->tval == TV_POTION)
        ADD_FLG(FLG_POTIONS);
    else if (o_ptr->tval == TV_SCROLL)
        ADD_FLG(FLG_SCROLLS);
    else if (o_ptr->tval == TV_LITE)
        ADD_FLG(FLG_LIGHTS);
    else if (o_ptr->tval == TV_SKELETON || o_ptr->tval == TV_BOTTLE
         || o_ptr->tval == TV_JUNK || o_ptr->tval == TV_STATUE)
        ADD_FLG(FLG_JUNKS);
    else if (o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_CORPSE)
        ADD_FLG(FLG_CORPSES);
    else if (o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_SKELETON)
        ADD_FLG(FLG_SKELETONS);
    else if (o_ptr->tval >= TV_LIFE_BOOK)
        ADD_FLG(FLG_SPELLBOOKS);
    else if (o_ptr->tval == TV_SHIELD)
        ADD_FLG(FLG_SHIELDS);
    else if (o_ptr->tval == TV_RING)
        ADD_FLG(FLG_RINGS);
    else if (o_ptr->tval == TV_AMULET)
        ADD_FLG(FLG_AMULETS);
    else if (o_ptr->tval == TV_DRAG_ARMOR || o_ptr->tval == TV_HARD_ARMOR ||
         o_ptr->tval == TV_SOFT_ARMOR)
        ADD_FLG(FLG_SUITS);
    else if (o_ptr->tval == TV_CLOAK)
        ADD_FLG(FLG_CLOAKS);
    else if (o_ptr->tval == TV_HELM || o_ptr->tval == TV_CROWN)
        ADD_FLG(FLG_HELMS);
    else if (o_ptr->tval == TV_GLOVES)
        ADD_FLG(FLG_GLOVES);
    else if (o_ptr->tval == TV_BOOTS)
        ADD_FLG(FLG_BOOTS);

    /* Prepare the object description */
    if (name)
    {
        char o_name[MAX_NLEN];

        object_desc(o_name, o_ptr, (OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL | OD_NAME_ONLY));
        sprintf(name_str, "^%s$", o_name);
    }

    /* Register the name in lowercase */
    str_tolower(name_str);
    entry->name = z_string_make(name_str);

    return;
}


/*
 * A function to delete entry
 */
static void autopick_free_entry(autopick_type *entry)
{
    z_string_free(entry->name);
    z_string_free(entry->insc);
    entry->name = NULL;
    entry->insc = NULL;
}


#define MAX_AUTOPICK_DEFAULT 200

/*
 * Initialize the autopick
 */
static void init_autopick(void)
{
    static const char easy_autopick_inscription[] = "(:=g";
    autopick_type entry = {0};
    int i;

    if (!autopick_list)
    {
        max_max_autopick = MAX_AUTOPICK_DEFAULT;
        C_MAKE(autopick_list, max_max_autopick, autopick_type);
        max_autopick = 0;
    }

    /* Clear old entries */
    for( i = 0; i < max_autopick; i++)
        autopick_free_entry(&autopick_list[i]);

    max_autopick = 0;

    /* There is always one entry "=g" */
    autopick_new_entry(&entry, easy_autopick_inscription, TRUE);
    autopick_list[max_autopick++] = entry;
}


#define PT_DEFAULT 0
#define PT_WITH_PNAME 1
#define PT_WITH_OTHERNAME 2

/*
 *  Get file name for autopick preference
 */
static cptr pickpref_filename(int filename_mode, char *other_base)
{
    static const char namebase[] = "pickpref";

    switch (filename_mode)
    {
    case PT_DEFAULT:
        return format("%s.prf", namebase);

    case PT_WITH_PNAME:
        return format("%s-%s.prf", namebase, player_base);

    case PT_WITH_OTHERNAME:
    {
        if (!other_base) return format("%s.prf", namebase);
        if (streq(namebase, other_base)) return format("%s.prf", namebase);
        return format("%s-%s.prf", namebase, other_base);
    }

    default:
        return NULL;
    }
}

static bool write_text_lines(cptr filename, cptr *lines_list);
static cptr *read_text_lines(cptr filename);
static void free_text_lines(cptr *lines_list);

/*
 * Load an autopick preference file
 */
void autopick_load_pref(byte mode)
{
    char buf[80];
    errr err;
    bool disp_mes = (mode & ALP_DISP_MES) ? TRUE : FALSE;

    /* Free old entries */
    init_autopick();

    /* Try a filename with player name */
    my_strcpy(buf, pickpref_filename(PT_WITH_PNAME, NULL), sizeof(buf));

    /* Load the file */
    err = process_autopick_file(buf);

    if (err == 0 && disp_mes)
    {
        /* Success */
        msg_format("Loaded '%s'.", buf);
    }

    /* No file found */
    if (0 > err)
    {
        /* Use default name */
        my_strcpy(buf, pickpref_filename(PT_DEFAULT, NULL), sizeof(buf));

        /* Load the file */
        err = process_autopick_file(buf);

        if (err == 0 && disp_mes)
        {
            /* Success */
            msg_format("Loaded '%s'.", buf);
        }
    }

    /* Load old preferences after ordinal bump */
    if ((err) && (mode & ALP_CHECK_NUMERALS) && (name_is_numbered(player_name)))
    {
        char old_py_name[32];
        cptr *lines_list = NULL;
        strcpy(old_py_name, player_name);

        while (err != 0)
        {
            bump_numeral(player_name, -1);
            if (!name_is_numbered(player_name)) break;
            process_player_name(FALSE);

            /* Try a filename with old player name */
            my_strcpy(buf, pickpref_filename(PT_WITH_PNAME, NULL), sizeof(buf));

            /* Load the file */
            err = process_autopick_file(buf);
            if (err == 0 && disp_mes)
            {
                /* Success */
                msg_format("Loaded '%s'.", buf);
            }
            if (err == 0)
            {
                lines_list = read_text_lines(buf);
                if (!lines_list) err = 1;
            }
        }
        strcpy(player_name, old_py_name);
        process_player_name(FALSE);
        if ((!err) && (lines_list))
        {
            my_strcpy(buf, pickpref_filename(PT_WITH_PNAME, NULL), sizeof(buf));
            (void)write_text_lines(buf, lines_list);
        }
        if (lines_list) free_text_lines(lines_list);
    }

    if (err && disp_mes)
    {
        /* Failed */
        msg_print("Failed to reload autopick preference.");
    }
}


/*
 * Add one line to autopick_list[]
 */
static void add_autopick_list(autopick_type *entry)
{
    /* There is no enough space to add one line */
    if (max_autopick >= max_max_autopick)
    {
        int old_max_max_autopick = max_max_autopick;
        autopick_type *old_autopick_list = autopick_list;

        /* Increase size of list */
        max_max_autopick += MAX_AUTOPICK_DEFAULT;

        /* Allocate */
        C_MAKE(autopick_list, max_max_autopick, autopick_type);

        /* Copy from old list to new list */
        C_COPY(autopick_list, old_autopick_list, old_max_max_autopick, autopick_type);

        /* Kill old list */
        C_FREE(old_autopick_list, old_max_max_autopick, autopick_type);
    }

    /* Add one line */
    autopick_list[max_autopick] = *entry;

    max_autopick++;
}


/*
 *  Process line for auto picker/destroyer.
 */
errr process_autopick_file_command(char *buf)
{
    autopick_type an_entry = {0}, *entry = &an_entry;
    int i;

    /* Nuke illegal char */
    for(i = 0; buf[i]; i++)
    {
        if (isspace(buf[i]) && buf[i] != ' ')
            break;
    }
    buf[i] = 0;

    if (!autopick_new_entry(entry, buf, FALSE)) return 0;

    /* Already has the same entry? */
    for(i = 0; i < max_autopick; i++)
        if(!strcmp(entry->name, autopick_list[i].name)
           && entry->flag[0] == autopick_list[i].flag[0]
           && entry->flag[1] == autopick_list[i].flag[1]
           && entry->dice == autopick_list[i].dice
           && entry->bonus == autopick_list[i].bonus
           && entry->weight == autopick_list[i].weight)
        {
            autopick_free_entry(entry);
            return 0;
        }

    add_autopick_list(entry);
    return 0;
}


/*
 * Reconstruct preference line from entry
 */
string_ptr autopick_line_from_entry(autopick_type *entry, int options)
{
    string_ptr s = string_alloc();
    bool sepa_flag = TRUE;

    if (!(entry->action & DO_DISPLAY)) string_append_c(s, '(');
    if (entry->action & DO_QUERY_AUTOPICK) string_append_c(s, ';');
    if (entry->action & DO_AUTODESTROY) string_append_c(s, '!');
    if (entry->action & DONT_AUTOPICK) string_append_c(s, '~');
    if (entry->action & DO_AUTO_ID) string_append_c(s, '?');

    /* Adjective* */
    if (options & AUTOPICK_COLOR_CODED)
        string_append_s(s, "<color:G>");

    if (IS_FLG(FLG_ALL)) string_printf(s, "%s ", KEY_ALL);

    if (IS_FLG(FLG_UNSENSED)) string_printf(s, "%s ", KEY_UNSENSED);
    if (IS_FLG(FLG_UNIDENTIFIED)) string_printf(s, "%s ", KEY_UNIDENTIFIED);
    if (IS_FLG(FLG_IDENTIFIED)) string_printf(s, "%s ", KEY_IDENTIFIED);
    if (IS_FLG(FLG_STAR_IDENTIFIED)) string_printf(s, "%s ", KEY_STAR_IDENTIFIED);
    if (IS_FLG(FLG_UNAWARE)) string_printf(s, "%s ", KEY_UNAWARE);

    if (IS_FLG(FLG_AVERAGE)) string_printf(s, "%s ", KEY_AVERAGE);
    if (IS_FLG(FLG_GOOD)) string_printf(s, "%s ", KEY_GOOD);
    if (IS_FLG(FLG_EGO)) string_printf(s, "%s ", KEY_EGO);
    if (IS_FLG(FLG_ARTIFACT)) string_printf(s, "%s ", KEY_ARTIFACT);
    if (IS_FLG(FLG_CURSED)) string_printf(s, "%s ", KEY_CURSED);
    if (IS_FLG(FLG_NAMELESS)) string_printf(s, "%s ", KEY_NAMELESS);
    if (IS_FLG(FLG_RARE)) string_printf(s, "%s ", KEY_RARE);
    if (IS_FLG(FLG_COMMON)) string_printf(s, "%s ", KEY_COMMON);
    if (IS_FLG(FLG_WORTHLESS)) string_printf(s, "%s ", KEY_WORTHLESS);
    if (IS_FLG(FLG_ICKY)) string_printf(s, "%s ", KEY_ICKY);

    if (IS_FLG(FLG_BOOSTED)) string_printf(s, "%s ", KEY_BOOSTED);

    if (IS_FLG(FLG_UNREADABLE)) string_printf(s, "%s ", KEY_UNREADABLE);
    if (IS_FLG(FLG_REALM1)) string_printf(s, "%s ", KEY_REALM1);
    if (IS_FLG(FLG_REALM2)) string_printf(s, "%s ", KEY_REALM2);
    if (IS_FLG(FLG_FIRST)) string_printf(s, "%s ", KEY_FIRST);
    if (IS_FLG(FLG_SECOND)) string_printf(s, "%s ", KEY_SECOND);
    if (IS_FLG(FLG_THIRD)) string_printf(s, "%s ", KEY_THIRD);
    if (IS_FLG(FLG_FOURTH)) string_printf(s, "%s ", KEY_FOURTH);

    if (IS_FLG(FLG_COLLECTING)) string_printf(s, "%s ", KEY_COLLECTING);
    if (IS_FLG(FLG_SPECIAL)) string_printf(s, "%s ", KEY_SPECIAL);
    if (IS_FLG(FLG_UNUSABLE)) string_printf(s, "%s ", KEY_UNUSABLE);
    if (IS_FLG(FLG_WANTED)) string_printf(s, "%s ", KEY_WANTED);
    if (IS_FLG(FLG_UNIQUE)) string_printf(s, "%s ", KEY_UNIQUE);
    if (IS_FLG(FLG_HUMAN)) string_printf(s, "%s ", KEY_HUMAN);

    /* Noun */
    if (options & AUTOPICK_COLOR_CODED)
        string_append_s(s, "</color><color:R>");

    if (IS_FLG(FLG_ITEMS)) string_append_s(s, KEY_ITEMS);

    else if (IS_FLG(FLG_WEAPONS)) string_append_s(s, KEY_WEAPONS);
    else if (IS_FLG(FLG_FAVORITE_WEAPONS)) string_append_s(s, KEY_FAVORITE_WEAPONS);
    else if (IS_FLG(FLG_HAFTED)) string_append_s(s, KEY_HAFTED);
    else if (IS_FLG(FLG_DIGGERS)) string_append_s(s, KEY_DIGGERS);

    else if (IS_FLG(FLG_SHOOTERS)) string_append_s(s, KEY_SHOOTERS);
    else if (IS_FLG(FLG_AMMO)) string_append_s(s, KEY_AMMO);

    else if (IS_FLG(FLG_ARMORS)) string_append_s(s, KEY_ARMORS);
    else if (IS_FLG(FLG_SHIELDS)) string_append_s(s, KEY_SHIELDS);
    else if (IS_FLG(FLG_SUITS)) string_append_s(s, KEY_SUITS);
    else if (IS_FLG(FLG_CLOAKS)) string_append_s(s, KEY_CLOAKS);
    else if (IS_FLG(FLG_HELMS)) string_append_s(s, KEY_HELMS);
    else if (IS_FLG(FLG_GLOVES)) string_append_s(s, KEY_GLOVES);
    else if (IS_FLG(FLG_BOOTS)) string_append_s(s, KEY_BOOTS);

    else if (IS_FLG(FLG_LIGHTS)) string_append_s(s, KEY_LIGHTS);
    else if (IS_FLG(FLG_RINGS)) string_append_s(s, KEY_RINGS);
    else if (IS_FLG(FLG_AMULETS)) string_append_s(s, KEY_AMULETS);

    else if (IS_FLG(FLG_SPELLBOOKS)) string_append_s(s, KEY_SPELLBOOKS);

    else if (IS_FLG(FLG_WANDS)) string_append_s(s, KEY_WANDS);
    else if (IS_FLG(FLG_STAVES)) string_append_s(s, KEY_STAVES);
    else if (IS_FLG(FLG_RODS)) string_append_s(s, KEY_RODS);
    else if (IS_FLG(FLG_POTIONS)) string_append_s(s, KEY_POTIONS);
    else if (IS_FLG(FLG_SCROLLS)) string_append_s(s, KEY_SCROLLS);

    else if (IS_FLG(FLG_JUNKS)) string_append_s(s, KEY_JUNKS);
    else if (IS_FLG(FLG_CORPSES)) string_append_s(s, KEY_CORPSES);
    else if (IS_FLG(FLG_SKELETONS)) string_append_s(s, KEY_SKELETONS);
    else sepa_flag = FALSE;

    /* Special-Clause */
    if (options & AUTOPICK_COLOR_CODED)
        string_append_s(s, "</color>");

    if (IS_FLG(FLG_MORE_DICE))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_DICE, entry->dice);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    if (IS_FLG(FLG_MORE_BONUS))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_BONUS, entry->bonus);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    if (IS_FLG(FLG_MORE_LEVEL))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_LEVEL, entry->level);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    if (IS_FLG(FLG_MORE_WEIGHT))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_WEIGHT, entry->weight);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    if (IS_FLG(FLG_MORE_CHARGES))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_CHARGES, entry->charges);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    if (IS_FLG(FLG_MORE_VALUE))
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:o>");
        string_printf(s, " %s %d", KEY_MORE_VALUE, entry->value);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    /* Search-String */
    if (entry->name && entry->name[0])
    {
        if (sepa_flag)
            string_append_c(s, ':');

        string_append_s(s, entry->name);
    }

    /* Inscription-String */
    if (entry->insc)
    {
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "<color:U>");
        string_append_c(s, '#');
        string_append_s(s, entry->insc);
        if (options & AUTOPICK_COLOR_CODED)
            string_append_s(s, "</color>");
    }

    return s;
}


/*
 * Reconstruct preference line from entry and kill entry
 */
static cptr autopick_line_from_entry_kill(autopick_type *entry)
{
    string_ptr s = autopick_line_from_entry(entry, 0);
    cptr       result = z_string_make(string_buffer(s));

    autopick_free_entry(entry);

    string_free(s); /*TODO: The editor structure needs conversion ... */
    return result;
}


/*
 * A function for Auto-picker/destroyer
 * Examine whether the object matches to the entry
 */
static bool _is_aware(object_type *o_ptr)
{
    return object_is_aware(o_ptr);
}

static bool _string_match_aux(cptr source, cptr pattern)
{
    cptr ss = source, ps = pattern;

    while (*ss && *ps)
    {
        int s = toupper(*ss), p = toupper(*ps);

        if (p == '$')
            break;

        if (s != p)
            return FALSE;

        ss++;
        ps++;
    }

    /* Did we exhaust the pattern string? */
    if (*ps)
    {
        /* anchor to end of string? */
        if (*ps == '$' && !*ss)
            return TRUE;

        return FALSE;
    }

    return TRUE;
}

static bool _string_match(cptr source, cptr pattern)
{
    cptr p = pattern;
    cptr s;

    /* anchor to start of string? */
    if (*p == '^')
        return _string_match_aux(source, pattern + 1);

    for (s = source; *s; s++)
    {
        if (_string_match_aux(s, pattern))
            return TRUE;
    }
    return FALSE;
}
/*  There has been a long standing Hengband bug with autoregistering
    objects for destruction greedily destroying too much. The classic
    example is autoregistering a mace only to find this also destroys
    that uber rare mace of disruption you just found!

    assert(_string_match("wand of light", "wand"));
    assert(!_string_match("wand of light", "mace"));
    assert(!_string_match("wand of light", "lightning balls"));
    assert(_string_match("wand of light", "light"));
    assert(_string_match("wand of light", "nd of li"));
    assert(_string_match("wand of light", "^wand of light"));
    assert(!_string_match("wand of light", "^of light"));
    assert(_string_match("wand of lightning balls", "^wand of light"));
    assert(!_string_match("wand of lightning balls", "^wand of light$"));
    assert(_string_match("wand of light", "^wand of light$"));
    assert(_string_match("wand of light", "of light$"));
    assert(!_string_match("wand of lightning balls", "of light$"));
    assert(!_string_match("wand of light", "^wand of lightning balls$"));
*/

static bool _is_polymorphed_demon(void)
{
    if (!p_ptr->mimic_form) return FALSE;
    else return (get_true_race()->flags & RACE_IS_DEMON);
}

static bool is_autopick_aux(object_type *o_ptr, autopick_type *entry, cptr o_name)
{
    int j;

    /*** Unaware items ***/
    if (IS_FLG(FLG_UNAWARE) && _is_aware(o_ptr))
        return FALSE;

    /*** Unsensed items ***/
    if (IS_FLG(FLG_UNSENSED) && (object_is_known(o_ptr) || (o_ptr->ident & IDENT_SENSE)))
        return FALSE;

    /*** Unidentified ***/
    if (IS_FLG(FLG_UNIDENTIFIED) && object_is_known(o_ptr))
        return FALSE;

    /*** Identified ***/
    if (IS_FLG(FLG_IDENTIFIED) && !object_is_known(o_ptr))
        return FALSE;

    /*** *Identified* ***/
    if (IS_FLG(FLG_STAR_IDENTIFIED) &&
        (!object_is_known(o_ptr) || !obj_is_identified_fully(o_ptr)))
        return FALSE;

    /*** Dice boosted (weapon of slaying) ***/
    if (IS_FLG(FLG_BOOSTED))
    {
        object_kind *k_ptr = &k_info[o_ptr->k_idx];

        if (!object_is_known(o_ptr)) return FALSE;

        if (object_is_bow(o_ptr))
        {
            if (o_ptr->mult == k_ptr->mult)
                return FALSE;
        }
        else if (object_is_melee_weapon(o_ptr))
        {
            if (o_ptr->dd == k_ptr->dd && o_ptr->ds == k_ptr->ds)
                return FALSE;
        }
        else
            return FALSE;
    }

    /*** Weapons which dd*ds is more than nn ***/
    if (IS_FLG(FLG_MORE_DICE))
    {
        if (!object_is_known(o_ptr)) return FALSE;
        if (o_ptr->dd * o_ptr->ds <= entry->dice)
            return FALSE;
    }

    /*** Objects with pval is more than nn ***/
    if (IS_FLG(FLG_MORE_BONUS))
    {
        if (!object_is_known(o_ptr)) return FALSE;

        if (o_ptr->to_h <= entry->bonus &&
            o_ptr->to_d <= entry->bonus &&
            o_ptr->to_a <= entry->bonus &&
            o_ptr->pval <= entry->bonus)
        {
            return FALSE;
        }
    }

    /* I added this for The Possessor, who quickly loses interest in
       the corpses of weak monsters ... */
    if (IS_FLG(FLG_MORE_LEVEL))
    {
        if (o_ptr->tval == TV_CORPSE)
        {
            monster_race *r_ptr = &r_info[o_ptr->pval];
            if (r_ptr->level <= entry->level)
                return FALSE;
        }
        /* ... but perhaps it might also be useful for other kinds of objects? */
        else if (object_is_known(o_ptr) && object_is_device(o_ptr))
        {             /* v--- This is leaking information, since normally, the object must be *Identified* first.*/
            int level = device_level(o_ptr);
            if (level <= entry->level)
                return FALSE;
        }
        else
        {
            int level = k_info[o_ptr->k_idx].level;

            if (object_is_known(o_ptr) && o_ptr->name2 && e_info[o_ptr->name2].level > level)
                level = e_info[o_ptr->name2].level;

            if (level <= entry->level)
                return FALSE;
        }
    }

    if (IS_FLG(FLG_MORE_WEIGHT))
    {
        if (o_ptr->weight <= entry->weight * 10)
            return FALSE;
    }

    if (IS_FLG(FLG_MORE_CHARGES))
    {
        if (!object_is_device(o_ptr)) return FALSE;
        if ((device_max_sp(o_ptr) / o_ptr->activation.cost) <= entry->charges) return FALSE;
    }

    if (IS_FLG(FLG_MORE_VALUE))
    {
        int value = obj_value(o_ptr);
        if (value <= entry->value)
            return FALSE;
    }

    /*** Worthless items ***/
    if (IS_FLG(FLG_WORTHLESS) && obj_value(o_ptr) > 0)
        return FALSE;

    /*** Icky items ***/
    if (IS_FLG(FLG_ICKY) && !_object_is_icky(o_ptr))
        return FALSE;

    /*** Artifact object ***/
    if (IS_FLG(FLG_ARTIFACT))
    {
        if (object_is_known(o_ptr))
        {
            if (!object_is_artifact(o_ptr)) return FALSE;
        }
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_SPECIAL:
            case FEEL_TERRIBLE:
                break;
            default:
                return FALSE;
            }
        }
        else
            return FALSE;
    }

    /*** Ego object ***/
    if (IS_FLG(FLG_EGO))
    {
        if (object_is_known(o_ptr))
        {
            if (!object_is_ego(o_ptr)) return FALSE;
        }
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_AWFUL:
            case FEEL_EXCELLENT:
                break;
            default:
                return FALSE;
            }
        }
        else
            return FALSE;
    }
    if (IS_FLG(FLG_SPECIAL)) /* leave_special ... I'm trying to obsolesce the easy destroyer. */
    {
        bool is_special = FALSE;
        if ( (get_race()->flags & RACE_IS_DEMON)
          || p_ptr->realm1 == REALM_DAEMON
          || p_ptr->realm2 == REALM_DAEMON || _is_polymorphed_demon())
        {
            if (o_ptr->tval == TV_CORPSE &&
                o_ptr->sval == SV_CORPSE &&
                my_strchr("pht", r_info[o_ptr->pval].d_char))
            {
                is_special = TRUE;
            }
        }
        if (p_ptr->prace == RACE_IGOR)
        {
            if ((o_ptr->tval == TV_CORPSE) && (o_ptr->sval >= SV_CORPSE)) is_special = TRUE;
        }
        if (p_ptr->pclass == CLASS_ARCHER)
        {
            if (o_ptr->tval == TV_SKELETON ||
                (o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_SKELETON))
            {
                is_special = TRUE;
            }
        }
        else if (player_is_ninja)
        {
            if ( o_ptr->tval == TV_LITE
              && (o_ptr->name2 == EGO_LITE_DARKNESS || have_flag(o_ptr->flags, OF_DARKNESS))
              && object_is_known(o_ptr))
            {
                is_special = TRUE;
            }
        }
        else if (p_ptr->pclass == CLASS_BEASTMASTER ||
             p_ptr->pclass == CLASS_CAVALRY)
        {
            if (o_ptr->tval == TV_WAND &&
                o_ptr->activation.type == EFFECT_HEAL_MONSTER && object_is_aware(o_ptr))
            {
                is_special = TRUE;
            }
        }
        else if (p_ptr->pclass == CLASS_ALCHEMIST)
        {
            if (o_ptr->tval == TV_POTION) is_special = TRUE;
        }
        else if (weaponmaster_is_(WEAPONMASTER_DIGGERS))
        {
            if (o_ptr->tval == TV_CORPSE || o_ptr->tval == TV_SKELETON)
                is_special = TRUE;
        }

        if (!is_special)
            return FALSE;
    }

    if (IS_FLG(FLG_UNUSABLE))
    {
        if (!object_is_wearable(o_ptr))
            return FALSE;
        if (equip_can_wield_kind(o_ptr->tval, o_ptr->sval))
            return FALSE;
    }

    /*** Cursed ***/
    if (IS_FLG(FLG_CURSED))
    {
        if (!object_is_equipment(o_ptr)) return FALSE;
        if (object_is_known(o_ptr))
        {
            if (!object_is_cursed(o_ptr)) return FALSE;
        }
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_BROKEN:
            case FEEL_BAD:
            case FEEL_AWFUL:
            case FEEL_TERRIBLE:
            case FEEL_CURSED:
                break;
            default:
                return FALSE;
            }
        }
        else
            return FALSE;
    }

    /*** Good ***/
    if (IS_FLG(FLG_GOOD))
    {
        if (!object_is_equipment(o_ptr)) return FALSE;

        /* Identified */
        if (object_is_known(o_ptr))
        {
            /* Artifacts and Ego objects are not okay */
            if (!object_is_nameless(o_ptr))
                return FALSE;

            /* Average are not okay */
            if (o_ptr->to_a <= 0 && (o_ptr->to_h + o_ptr->to_d) <= 0)
                return FALSE;
        }

        /* Pseudo-identified */
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_GOOD:
            case FEEL_ENCHANTED:
                /* It's good */
                break;

            default:
                /* It's not good */
                return FALSE;
            }
        }

        /* Unidentified */
        else
        {
            /* Not known to be good */
            return FALSE;
        }
    }

    /*** Nameless ***/
    if (IS_FLG(FLG_NAMELESS))
    {
        if (!object_is_equipment(o_ptr) && !object_is_device(o_ptr)) return FALSE;

        /* Identified */
        if (object_is_known(o_ptr))
        {
            /* Artifacts and Ego objects are not okay */
            if (!object_is_nameless(o_ptr))
                return FALSE;
        }

        /* Pseudo-identified */
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_AVERAGE:
            case FEEL_GOOD:
            case FEEL_BAD:
                /* It's nameless */
                break;

            default:
                /* It's not nameless */
                return FALSE;
            }
        }

        /* Unidentified */
        else
        {
            /* Not known to be nameless */
            return FALSE;
        }
    }

    /*** Average ***/
    if (IS_FLG(FLG_AVERAGE))
    {
        if (!object_is_equipment(o_ptr)) return FALSE;

        /* Identified */
        if (object_is_known(o_ptr))
        {
            /* Artifacts and Ego objects are not okay */
            if (!object_is_nameless(o_ptr))
                return FALSE;

            /* Cursed or broken objects are not okay */
            if (object_is_cursed(o_ptr) || object_is_broken(o_ptr))
                return FALSE;

            /* Good are not okay */
            if (o_ptr->to_a > 0 || (o_ptr->to_h + o_ptr->to_d) > 0)
                return FALSE;
        }

        /* Pseudo-identified */
        else if (o_ptr->ident & IDENT_SENSE)
        {
            switch (o_ptr->feeling)
            {
            case FEEL_AVERAGE:
                /* It's average */
                break;

            default:
                /* It's not average */
                return FALSE;
            }
        }

        /* Unidentified */
        else
        {
            /* Not known to be average */
            return FALSE;
        }
    }

    /*** Rere equipments ***/
    if (IS_FLG(FLG_RARE) && !object_is_rare(o_ptr))
        return FALSE;

    /*** Common equipments ***/
    if (IS_FLG(FLG_COMMON) && object_is_rare(o_ptr))
        return FALSE;

    /*** Wanted monster's corpse/skeletons ***/
    if (IS_FLG(FLG_WANTED) && !object_is_shoukinkubi(o_ptr))
        return FALSE;

    /*** Unique monster's corpse/skeletons/statues ***/
    if (IS_FLG(FLG_UNIQUE) &&
        ((o_ptr->tval != TV_CORPSE && o_ptr->tval != TV_STATUE) ||
         !(r_info[o_ptr->pval].flags1 & RF1_UNIQUE)))
        return FALSE;

    /*** Human corpse/skeletons (for Daemon magic) ***/
    if (IS_FLG(FLG_HUMAN) &&
        (o_ptr->tval != TV_CORPSE ||
         !my_strchr("pht", r_info[o_ptr->pval].d_char)))
        return FALSE;

    /*** Unreadable spellbooks ***/
    if (IS_FLG(FLG_UNREADABLE) &&
        (o_ptr->tval < TV_LIFE_BOOK ||
         check_book_realm(o_ptr->tval, o_ptr->sval)))
        return FALSE;

    /*** First realm spellbooks ***/
    if (IS_FLG(FLG_REALM1) &&
        (REALM1_BOOK != o_ptr->tval ||
         p_ptr->pclass == CLASS_SORCERER ||
         p_ptr->pclass == CLASS_RED_MAGE))
        return FALSE;

    /*** Second realm spellbooks ***/
    if (IS_FLG(FLG_REALM2) &&
        (REALM2_BOOK != o_ptr->tval ||
         p_ptr->pclass == CLASS_SORCERER ||
         p_ptr->pclass == CLASS_RED_MAGE))
        return FALSE;

    /*** First rank spellbooks ***/
    if (IS_FLG(FLG_FIRST) &&
        (o_ptr->tval < TV_LIFE_BOOK || 0 != o_ptr->sval))
        return FALSE;

    /*** Second rank spellbooks ***/
    if (IS_FLG(FLG_SECOND) &&
        (o_ptr->tval < TV_LIFE_BOOK || 1 != o_ptr->sval))
        return FALSE;

    /*** Third rank spellbooks ***/
    if (IS_FLG(FLG_THIRD) &&
        (o_ptr->tval < TV_LIFE_BOOK || 2 != o_ptr->sval))
        return FALSE;

    /*** Fourth rank spellbooks ***/
    if (IS_FLG(FLG_FOURTH) &&
        (o_ptr->tval < TV_LIFE_BOOK || 3 != o_ptr->sval))
        return FALSE;

    /*** Items ***/
    if (IS_FLG(FLG_WEAPONS))
    {
        if (!object_is_melee_weapon(o_ptr))
            return FALSE;
    }
    else if (IS_FLG(FLG_FAVORITE_WEAPONS))
    {
        if (!object_is_favorite(o_ptr))
            return FALSE;
    }
    else if (IS_FLG(FLG_HAFTED))
    {
        if (!(o_ptr->tval == TV_HAFTED))
            return FALSE;
    }
    else if (IS_FLG(FLG_DIGGERS))
    {
        if (!(o_ptr->tval == TV_DIGGING))
            return FALSE;
    }
    else if (IS_FLG(FLG_SHOOTERS))
    {
        if (o_ptr->tval != TV_BOW)
            return FALSE;
    }
    else if (IS_FLG(FLG_AMMO))
    {
        if (!object_is_ammo(o_ptr)) return FALSE;
    }
    else if (IS_FLG(FLG_ARMORS))
    {
        if (!object_is_armour(o_ptr))
            return FALSE;
    }
    else if (IS_FLG(FLG_SHIELDS))
    {
        if (!(o_ptr->tval == TV_SHIELD))
            return FALSE;
    }
    else if (IS_FLG(FLG_SUITS))
    {
        if (!(o_ptr->tval == TV_DRAG_ARMOR ||
              o_ptr->tval == TV_HARD_ARMOR ||
              o_ptr->tval == TV_SOFT_ARMOR))
            return FALSE;
    }
    else if (IS_FLG(FLG_CLOAKS))
    {
        if (!(o_ptr->tval == TV_CLOAK))
            return FALSE;
    }
    else if (IS_FLG(FLG_HELMS))
    {
        if (!(o_ptr->tval == TV_CROWN || o_ptr->tval == TV_HELM))
            return FALSE;
    }
    else if (IS_FLG(FLG_GLOVES))
    {
        if (!(o_ptr->tval == TV_GLOVES))
            return FALSE;
    }
    else if (IS_FLG(FLG_BOOTS))
    {
        if (!(o_ptr->tval == TV_BOOTS))
            return FALSE;
    }
    else if (IS_FLG(FLG_LIGHTS))
    {
        if (!(o_ptr->tval == TV_LITE))
            return FALSE;
    }
    else if (IS_FLG(FLG_RINGS))
    {
        if (!(o_ptr->tval == TV_RING))
            return FALSE;
    }
    else if (IS_FLG(FLG_AMULETS))
    {
        if (!(o_ptr->tval == TV_AMULET))
            return FALSE;
    }
    else if (IS_FLG(FLG_WANDS))
    {
        if (o_ptr->tval != TV_WAND)
            return FALSE;
    }
    else if (IS_FLG(FLG_STAVES))
    {
        if (o_ptr->tval != TV_STAFF)
            return FALSE;
    }
    else if (IS_FLG(FLG_RODS))
    {
        if (o_ptr->tval != TV_ROD)
            return FALSE;
    }
    else if (IS_FLG(FLG_POTIONS))
    {
        if (o_ptr->tval != TV_POTION)
            return FALSE;
    }
    else if (IS_FLG(FLG_SCROLLS))
    {
        if (o_ptr->tval != TV_SCROLL)
            return FALSE;
    }
    else if (IS_FLG(FLG_JUNKS))
    {
        switch(o_ptr->tval)
        {
        case TV_SKELETON: case TV_BOTTLE:
        case TV_JUNK: case TV_STATUE:
            break;
        default: return FALSE;
        }
    }
    else if (IS_FLG(FLG_CORPSES))
    {
        if (!object_is_(o_ptr, TV_CORPSE, SV_CORPSE))
            return FALSE;
    }
    else if (IS_FLG(FLG_SKELETONS))
    {
        if ( !object_is_(o_ptr, TV_CORPSE, SV_SKELETON)
          && o_ptr->tval != TV_SKELETON )
        {
            return FALSE;
        }
    }
    else if (IS_FLG(FLG_SPELLBOOKS))
    {
        if (!(o_ptr->tval >= TV_LIFE_BOOK))
            return FALSE;
    }

    /* Search-String */
    if (!_string_match(o_name, entry->name))
        return FALSE;

    /* TRUE when it need not to be 'collecting' */
    if (!IS_FLG(FLG_COLLECTING)) return TRUE;

    /* Check if there is a same item */
    for (j = 1; j <= pack_max(); j++)
    {
        obj_ptr obj = pack_obj(j);
        if (obj && obj_can_combine(obj, o_ptr, 0))
            return TRUE;
    }
    for (j = 1; j <= quiver_max(); j++)
    {
        obj_ptr obj = quiver_obj(j);
        if (obj && obj_can_combine(obj, o_ptr, 0))
            return TRUE;
    }

    /* Not collecting */
    return FALSE;
}

/*
 * A function for Auto-picker/destroyer
 * Examine whether the object matches to the list of keywords or not.
 */
int is_autopick(object_type *o_ptr)
{
    int i;
    char o_name[MAX_NLEN];

    if (o_ptr->tval == TV_GOLD) return -1;

    if (no_mogaminator) return -1;

    if (o_ptr->inscription && my_strstr(quark_str(o_ptr->inscription), "=g"))
    {
        /* see init_autopick ... I think at some point I broke this line: we
           no longer include the inscription in the match logic. Indeed, doing
           so would cause problems with the new end of string match character.
           For example: "^wand of light$" won't match if there is an inscription!
           But having that example match "wand of lightning balls" too was just
           plain intolerable! */
        return 0;
    }

    /* Prepare object name string first */
    object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL));

    /* Convert the string to lower case */
    str_tolower(o_name);

    /* Look for a matching entry in the list */
    for (i=0; i < max_autopick; i++)
    {
        autopick_type *entry = &autopick_list[i];

        /* Hack - ignore the entry "items" if obj_list_autopick_hack is on */
        if ((obj_list_autopick_hack) && (entry)  && (entry->action == (DO_AUTOPICK | DO_DISPLAY)) &&
            ((!entry->name) || (!entry->name[0])) &&
            (!entry->insc) && (!entry->level) && (!entry->dice) && (!entry->weight) &&
            (!entry->charges) && (!entry->value) && (!entry->bonus))
        {
            u32b cmp_liput[AUTOPICK_FLAG_SIZE] = {0};
            int ii;
            bool notsame = FALSE;
            cmp_liput[FLG_ITEMS / 32] |= (1L << (FLG_ITEMS % 32));
            for (ii = 0; ii < AUTOPICK_FLAG_SIZE; ii++)
            {
                if (cmp_liput[ii] != entry->flag[ii])
                {
                    notsame = TRUE;
                    break;
                }
            }
            if (!notsame) continue;
        }
        if (is_autopick_aux(o_ptr, entry, o_name))
            return i;
    }

    /* No matching entry */
    return -1;
}


/*
 *  Auto inscription
 */
static void auto_inscribe_item(object_type *o_ptr, int idx)
{
    /* Are there auto-inscription? */
    if (idx < 0 || !autopick_list[idx].insc) return;

    if (!o_ptr->inscription)
        o_ptr->inscription = quark_add(autopick_list[idx].insc);

    /* Redraw inscription */
    p_ptr->window |= (PW_EQUIP | PW_INVEN);

    /* {.} and {$} effect p_ptr->warning and TRC_TELEPORT_SELF */
    p_ptr->update |= (PU_BONUS);
}


/*
 * Automatically destroy items in this grid.
 */
static bool is_opt_confirm_destroy(object_type *o_ptr)
{
    if (!destroy_items) return FALSE;

    if (leave_worth) /* leave worthy items ... worth could also stand for worthless, no? Sigh ... */
    {
        if ( !object_is_known(o_ptr)
          && !object_is_rare(o_ptr)
          && (o_ptr->ident & IDENT_SENSE)
          && o_ptr->feeling == FEEL_BAD )
        {
            /* Bad items should generally be destroyed (even if they have
               non-zero values). However, a subsequent option may keep them
               around (e.g. leave_special or leave_equip) */
        }
        else if (obj_value(o_ptr) > 0)
            return FALSE;
    }

    if (leave_equip)
        if (object_is_weapon_armour_ammo(o_ptr)) return FALSE;

    if (leave_chest)
        if ((o_ptr->tval == TV_CHEST) && o_ptr->pval) return FALSE;

    if (leave_wanted)
    {
        if (object_is_shoukinkubi(o_ptr)) return FALSE;
    }

    if (leave_corpse)
        if (o_ptr->tval == TV_CORPSE) return FALSE;

    if (leave_junk)
        if ((o_ptr->tval == TV_SKELETON) || (o_ptr->tval == TV_BOTTLE) || (o_ptr->tval == TV_JUNK) || (o_ptr->tval == TV_STATUE)) return FALSE;

    if (leave_special)
    {
        if (prace_is_(RACE_BALROG) || prace_is_(RACE_MON_DEMON)
            || p_ptr->realm1 == REALM_DAEMON || p_ptr->realm2 == REALM_DAEMON)
        {
            if (o_ptr->tval == TV_CORPSE &&
                o_ptr->sval == SV_CORPSE &&
                my_strchr("pht", r_info[o_ptr->pval].d_char))
                return FALSE;
        }
        if (p_ptr->prace == RACE_MON_POSSESSOR && o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_CORPSE)
        {
            return FALSE;
        }
        if ((p_ptr->prace == RACE_IGOR) && (o_ptr->tval == TV_CORPSE) && (o_ptr->sval != SV_SKELETON))
        {
            return FALSE;
        }
        if (p_ptr->pclass == CLASS_ARCHER)
        {
            if (o_ptr->tval == TV_SKELETON ||
                (o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_SKELETON))
                return FALSE;
        }
        else if (player_is_ninja)
        {
            if ( o_ptr->tval == TV_LITE
              && (o_ptr->name2 == EGO_LITE_DARKNESS || have_flag(o_ptr->flags, OF_DARKNESS))
              && object_is_known(o_ptr))
            {
                return FALSE;
            }
        }
        else if (p_ptr->pclass == CLASS_BEASTMASTER ||
             p_ptr->pclass == CLASS_CAVALRY)
        {
            if (o_ptr->tval == TV_WAND &&
                o_ptr->activation.type == EFFECT_HEAL_MONSTER && object_is_aware(o_ptr))
            {
                return FALSE;
            }
        }
    }

    if (o_ptr->tval == TV_GOLD) return FALSE;

    return TRUE;
}

static void msg_autopick(int idx, cptr action)
{
    if (idx >= 0)
    {
        string_ptr s = autopick_line_from_entry(&autopick_list[idx], AUTOPICK_COLOR_CODED);
        if (action)
            msg_format("<color:B>(%s:</color>%s<color:B>)</color>", action, string_buffer(s));
        else
            msg_print(string_buffer(s));
        string_free(s);
    }
}

/*
 * Automatically destroy an item if it is to be destroyed
 *
 * When always_pickup is 'yes', we disable auto-destroyer function of
 * auto-picker/destroyer, and do only easy-auto-destroyer.
 */
static object_type autopick_last_destroyed_object;

static void auto_destroy_obj(object_type *o_ptr, int autopick_idx)
{
    bool destroy = FALSE;
    char name[MAX_NLEN];

    /* Never autodestroy an insured item */
    if (o_ptr->insured) return;

    /* Never autodestroy a mundanified item */
    if (o_ptr->origin_type == ORIGIN_MUNDANITY) return;

    /* Easy-Auto-Destroyer (3rd priority) */
    if (is_opt_confirm_destroy(o_ptr)) destroy = TRUE;

    /* Protected by auto-picker (2nd priotity) */
    if ((!no_mogaminator) && (autopick_idx >= 0) &&
        (!(autopick_list[autopick_idx].action & DO_AUTODESTROY)))
        destroy = FALSE;

    /* Auto-destroyer works only when !always_pickup */
    if ((!always_pickup) && (!no_mogaminator) && (!leave_mogaminator))
    {
        /* Auto-picker/destroyer (1st priority) */
        if (autopick_idx >= 0 &&
            (autopick_list[autopick_idx].action & DO_AUTODESTROY))
            destroy = TRUE;
    }

    /* Not to be destroyed */
    if (!destroy)
    {
        if (destroy_debug && !no_mogaminator && autopick_idx >= 0 && (autopick_list[autopick_idx].action & DONT_AUTOPICK))
            msg_autopick(autopick_idx, "Leave");
        return;
    }
    /* Now decided to destroy
    disturb(0,0);*/
    obj_delayed_describe(o_ptr); /* mass identify */

    /* Artifact? */
    if (!can_player_destroy_object(o_ptr))
    {
        /* Describe the object (with {terrible/special}) */
        object_desc(name, o_ptr, OD_COLOR_CODED);

        /* Message */
        msg_format("You cannot auto-destroy %s.", name);

        /* Done */
        return;
    }

    /* Record name of destroyed item */
    autopick_last_destroyed_object = *o_ptr;

    if (destroy_debug)
    {
        int idx = is_autopick(o_ptr);
        msg_autopick(idx, "Destroy");
    }
    if (o_ptr->k_idx)
    {
        object_desc(name, o_ptr, OD_COLOR_CODED);
        msg_format("Auto-destroying %s.", name);
    }
    /* Note: It turns out to be convenient to delay destruction after
     * all. But rather than waiting until notice_stuff(), we need
     * only wait until obj_release(). Otherwise, clients need to think
     * a bit to avoid accidentally using destroyed memory. */
    o_ptr->marked |= OM_AUTODESTROY; /* clients *must* obj_release() */
    /*obj_destroy(o_ptr, o_ptr->number);*/

    return;
}

/*
 * Auto-inscription and/or destroy
 *
 * Auto-destroyer works only on inventory or on floor stack only when
 * requested.
 */
void autopick_alter_obj(obj_ptr o_ptr, bool allow_destroy)
{
    /* Get the index in the auto-pick/destroy list */
    int idx = is_autopick(o_ptr);

    if (no_mogaminator) return;

    /* Auto-id: Try "?unidentified good" for a L30 monk ... */
    if (idx >= 0 && autopick_list[idx].action & DO_AUTO_ID)
    {
        if (autopick_auto_id(o_ptr))
        {
            int new_idx = is_autopick(o_ptr); /* requery for destroy/pickup/inscribe once known */
            if (new_idx >= 0)
                idx = new_idx;
        }
    }

    /* Do auto-inscription */
    auto_inscribe_item(o_ptr, idx);

    /* Do auto-destroy if needed */
    if (allow_destroy && o_ptr->loc.where != INV_EQUIP)
        auto_destroy_obj(o_ptr, idx);
}

static void _sense_object_floor(object_type *o_ptr)
{
    if (o_ptr->ident & IDENT_SENSE) return;
    if (object_is_known(o_ptr)) return;
    if ((!obj_can_sense1(o_ptr)) && (!obj_can_sense2(o_ptr))) return;

    o_ptr->ident |= IDENT_SENSE;
    o_ptr->feeling = value_check_aux1(o_ptr);
}

/* Automatically identify objects, consuming requisite resources.
   We support scrolls and devices as the source for this convenience.
   We ignore fail rates and don't even charge the player energy for
   this boon! This is generous, but it is assumed that the player could
   (laboriously) locate a quiet safe place and then repeat until successful anyway. */
bool autopick_auto_id(object_type *o_ptr)
{
    int     class_idx = p_ptr->pclass;
    race_t *race = get_race();

    if (class_idx == CLASS_MONSTER)
        class_idx = race->pseudo_class_idx;

    if (!object_is_known(o_ptr) && class_idx != CLASS_BERSERKER && !beorning_is_(BEORNING_FORM_BEAR))
    {
        slot_t slot;

        if (p_ptr->pclass == CLASS_MAGIC_EATER && magic_eater_auto_id(o_ptr))
            return TRUE;

        if (p_ptr->pclass == CLASS_ALCHEMIST && o_ptr->tval == TV_POTION)
            return TRUE;

        slot = pack_find_device(EFFECT_IDENTIFY);
        if (slot)
        {
            obj_ptr device = pack_obj(slot);
            identify_item(o_ptr);
            stats_on_use(device, 1);
            device_decrease_sp(device, device->activation.cost);
            return TRUE;
        }

        slot = pack_find_obj(TV_SCROLL, SV_SCROLL_IDENTIFY);
        if (slot && !p_ptr->blind && !(race->flags & RACE_IS_ILLITERATE))
        {
            obj_ptr scroll = pack_obj(slot);
            if (obj_is_known(scroll))
            {
                identify_item(o_ptr);
                stats_on_use(scroll, 1);
                scroll->number--;
                obj_release(scroll, OBJ_RELEASE_DELAYED_MSG);
                return TRUE;
            }
        }

        if (p_ptr->auto_id_sp && p_ptr->csp >= p_ptr->auto_id_sp)
        {
            identify_item(o_ptr);
            sp_player(-p_ptr->auto_id_sp);
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Automatically pickup/destroy items in player's current grid
 */
static void _get_obj(obj_ptr obj)
{
    int idx;
    assert(obj);

    /* Gold always picks up */
    if (obj->tval == TV_GOLD)
    {
        pack_get(obj);
        return;
    }

    /* Player Sensing */
    if (p_ptr->auto_id)
    {
        identify_item(obj);
        equip_learn_flag(OF_LORE2);
    }
    else if ((p_ptr->pclass == CLASS_ALCHEMIST) && (obj->tval == TV_POTION))
    {
        identify_item(obj);
    }
    else if ((p_ptr->auto_pseudo_id) || (p_ptr->munchkin_pseudo_id))
    {
        _sense_object_floor(obj);
        equip_learn_flag(OF_LORE1);
    }

    if ((!obj) || (!obj->k_idx)) return;

    if (no_mogaminator)
    {
        if ((destroy_items) && (obj->loc.where != INV_EQUIP))
        {
            auto_destroy_obj(obj, -1);
            obj_release(obj, OBJ_RELEASE_QUIET);
        }
        return;
    }

    idx = is_autopick(obj);

    /* AutoID (?unaware items) */
    if (idx >= 0 && autopick_list[idx].action & DO_AUTO_ID)
    {
        if (autopick_auto_id(obj))
        {
            int new_idx = is_autopick(obj); /* requery for destroy/pickup/inscribe once known */
            if (destroy_debug)
                msg_autopick(idx, "AutoID");
            if (new_idx >= 0)
                idx = new_idx;
        }
    }

    if ((!obj) || (!obj->k_idx)) return;

    /* Inscribe */
    auto_inscribe_item(obj, idx);

    if (delay_autopick_hack)
    {
        auto_destroy_obj(obj, idx);
        obj_release(obj, OBJ_RELEASE_QUIET);
        return;
    }

    /* Pickup */
    if (idx >= 0 &&
        (autopick_list[idx].action & (DO_AUTOPICK | DO_QUERY_AUTOPICK)))
    {
        disturb(0,0);

        if (destroy_debug)
            msg_autopick(idx, "Pickup");

        if (autopick_list[idx].action & DO_QUERY_AUTOPICK)
        {
            char out_val[MAX_NLEN+20];
            char o_name[MAX_NLEN];

            if (obj->marked & OM_NO_QUERY)
            {
                /* Already answered as 'No' */
                return;
            }

            /* Describe the object */
            object_desc(o_name, obj, OD_COLOR_CODED);

            sprintf(out_val, "Pick up %s? ", o_name);

            if (!get_check(out_val))
            {
                obj->marked |= (OM_NOMSG | OM_NO_QUERY);
                return;
            }

        }
        pack_get(obj);
        return;
    }

    /* Autodestroy */
    auto_destroy_obj(obj, idx);
    obj_release(obj, OBJ_RELEASE_QUIET);
}

static bool _obj_allows_unid_pickup(obj_ptr obj)
{
    int j;
    if ((!obj) || (!obj->k_idx)) return FALSE;
    if (obj->tval == TV_GOLD) return TRUE;
    if ((obj->tval == TV_POTION) && /* Mega-hack - disturb for healing potions */
       ((obj->sval == SV_POTION_HEALING) ||
        (obj->sval == SV_POTION_STAR_HEALING) ||
        (obj->sval == SV_POTION_LIFE))) return TRUE;
    /* Check if we are carrying an identical item */
    for (j = 1; j <= pack_max(); j++)
    {
        obj_ptr o_ptr = pack_obj(j);
        if (o_ptr && obj_can_combine(o_ptr, obj, 0))
            return TRUE;
    }
    if (obj->marked & OM_FOUND) return FALSE;
    if (obj_is_known(obj)) return FALSE;
    if ((obj->ident & IDENT_SENSE) && (!obj_is_device(obj)) &&
        (obj->feeling != FEEL_EXCELLENT) && (obj->feeling != FEEL_SPECIAL) &&
        (obj->feeling != FEEL_TERRIBLE) && (obj->feeling != FEEL_ENCHANTED) &&
        (obj->feeling != FEEL_AWFUL)) return FALSE;
    return TRUE;
}

void autopick_get_floor(bool allow_identified)
{
    inv_ptr floor = inv_filter_floor(point(px, py), allow_identified ? NULL : _obj_allows_unid_pickup);
    inv_for_each(floor, _get_obj);
    inv_free(floor);
}


static const char autoregister_header[] = "?:$AUTOREGISTER";

/*
 *  Clear auto registered lines in the picktype.prf .
 */
static bool clear_auto_register(void)
{
    char tmp_file[1024];
    char pref_file[1024];
    char buf[1024];
    FILE *pref_fff;
    FILE *tmp_fff;
    int num = 0;
    bool autoregister = FALSE;
    bool okay = TRUE;

    path_build(pref_file, sizeof(pref_file), ANGBAND_DIR_USER, pickpref_filename(PT_WITH_PNAME, NULL));
    pref_fff = my_fopen(pref_file, "r");

    if (!pref_fff)
    {
        path_build(pref_file, sizeof(pref_file), ANGBAND_DIR_USER, pickpref_filename(PT_DEFAULT, NULL));
        pref_fff = my_fopen(pref_file, "r");
    }

    if (!pref_fff)
    {
        /* No file yet */
        return TRUE;
    }

    /* Open a new (temporary) file */
    tmp_fff = my_fopen_temp(tmp_file, sizeof(tmp_file));

    if (!tmp_fff)
    {
        /* Close the preference file */
        fclose(pref_fff);

        msg_format("Failed to create temporary file %s.", tmp_file);
        msg_print(NULL);
        return FALSE;
    }


    /* Loop for every line */
    while (TRUE)
    {
        /* Read a line */
        if (my_fgets(pref_fff, buf, sizeof(buf))) break;

        if (autoregister)
        {
            /* Delete auto-registered line */

            /* Count auto-destroy preference lines */
            if (buf[0] != '#' && buf[0] != '?') num++;
        }

        /* We are looking for auto-registered line */
        else
        {
            if (streq(buf, autoregister_header))
            {
                /* Delete all further lines */
                autoregister = TRUE;
            }
            else
            {
                /* Copy orginally lines */
                fprintf(tmp_fff, "%s\n", buf);
            }
        }
    }

    /* Close files */
    my_fclose(pref_fff);
    my_fclose(tmp_fff);

    if (num)
    {
        msg_format("Auto registered lines (%d lines) for previous character are remaining.", num);
        strcpy(buf, "These lines will be deleted. Are you sure? ");

        /* You can cancel it */
        if (!get_check(buf))
        {
            okay = FALSE;
            autoregister = FALSE;

            msg_print("Use cut & paste of auto picker editor (_) to keep old prefs.");
        }
    }


    /* If there are some changes, overwrite the original file with new one */
    if (autoregister)
    {
        /* Copy contents of temporary file */

        tmp_fff = my_fopen(tmp_file, "r");
        pref_fff = my_fopen(pref_file, "w");

        while (!my_fgets(tmp_fff, buf, sizeof(buf)))
            fprintf(pref_fff, "%s\n", buf);

        my_fclose(pref_fff);
        my_fclose(tmp_fff);
    }

    /* Kill the temporary file */
    fd_kill(tmp_file);

    return okay;
}


/*
 *  Automatically register an auto-destroy preference line
 */
bool autopick_autoregister(object_type *o_ptr)
{
    char buf[1024];
    char pref_file[1024];
    FILE *pref_fff;
    autopick_type an_entry, *entry = &an_entry;
    string_ptr line = 0;

    int match_autopick = is_autopick(o_ptr);

    if (no_mogaminator)
    {
        msg_print("The Mogaminator is not currently active.");
        return FALSE;
    }

    /* Already registered */
    if (match_autopick != -1)
    {
        cptr what;
        byte act = autopick_list[match_autopick].action;
        string_ptr s = autopick_line_from_entry(&autopick_list[match_autopick], AUTOPICK_COLOR_CODED);

        if (act & DO_AUTOPICK) what = "auto-pickup";
        else if (act & DO_AUTODESTROY) what = "auto-destroy";
        else if (act & DONT_AUTOPICK) what = "leave on floor";
        else /* if (act & DO_QUERY_AUTOPICK) */ what = "query auto-pickup";

        msg_format("The object is already registered to %s by the rule %s.", what, string_buffer(s));

        string_free(s);
        return FALSE;
    }

    /* Known to be an artifact? */
    if ((object_is_known(o_ptr) && object_is_artifact(o_ptr)) ||
        ((o_ptr->ident & IDENT_SENSE) &&
         (o_ptr->feeling == FEEL_SPECIAL)))
    {
        char o_name[MAX_NLEN];

        /* Describe the object (with {terrible/special}) */
        object_desc(o_name, o_ptr, 0);

        /* Message */
        msg_format("You cannot auto-destroy %s.", o_name);

        /* Done */
        return FALSE;
    }


    if (!p_ptr->autopick_autoregister)
    {
        /* Clear old auto registered lines */
        if (!clear_auto_register()) return FALSE;
    }

    /* Try a filename with player name */
    path_build(pref_file, sizeof(pref_file), ANGBAND_DIR_USER, pickpref_filename(PT_WITH_PNAME, NULL));
    pref_fff = my_fopen(pref_file, "r");

    if (!pref_fff)
    {
        /* Use default name */
        path_build(pref_file, sizeof(pref_file), ANGBAND_DIR_USER, pickpref_filename(PT_DEFAULT, NULL));
        pref_fff = my_fopen(pref_file, "r");
        if (!pref_fff)
        {
            msg_print("Initialize the auto-pick preferences first (Type '_').");
            return FALSE;
        }
    }

    /* Check the header */
    while (TRUE)
    {
        /* Read a line */
        if (my_fgets(pref_fff, buf, sizeof(buf)))
        {
            /* No header found */
            p_ptr->autopick_autoregister = FALSE;

            break;
        }

        if (streq(buf, autoregister_header))
        {
            /* Found the header */
            p_ptr->autopick_autoregister = TRUE;

            break;
        }
    }

    /* Close read only FILE* */
    fclose(pref_fff);

    /* Open for append */
    pref_fff = my_fopen(pref_file, "a");

    /* Failure */
    if (!pref_fff) {
        msg_format("Failed to open %s.", pref_file);
        msg_print(NULL);

        /* Failed */
        return FALSE;
    }

    if (!p_ptr->autopick_autoregister)
    {
        /* Add the header */
        fprintf(pref_fff, "%s\n", autoregister_header);

        fprintf(pref_fff, "%s\n", "# *Warning!* The lines below will be deleted later.");
        fprintf(pref_fff, "%s\n", "# Keep them by cut & paste if you need them for future characters.");

        /* Now auto register is in-use */
        p_ptr->autopick_autoregister = TRUE;
    }

    /* Get a preference entry */
    autopick_entry_from_object(entry, o_ptr);

    /* Set to auto-destroy (with no-display) */
    entry->action = DO_AUTODESTROY;

    /* Load the new line as preference */
    add_autopick_list(entry);

    /* Add a line to the file */
    /* Don't kill "entry" */
    line = autopick_line_from_entry(entry, 0);
    fprintf(pref_fff, "%s\n", string_buffer(line));
    string_free(line);

    /* Close the file */
    fclose(pref_fff);

    return TRUE;
}


/********  Auto-picker/destroyer editor  **********/

#define MAX_YANK MAX_LINELEN
#define MAX_LINES 3000

#define MARK_MARK     0x01
#define MARK_BY_SHIFT 0x02

#define LSTAT_BYPASS        0x01
#define LSTAT_EXPRESSION    0x02
#define LSTAT_AUTOREGISTER  0x04

#define QUIT_WITHOUT_SAVE 1
#define QUIT_AND_SAVE     2

/*
 * Struct for yank buffer
 */
typedef struct chain_str {
    struct chain_str *next;
    char s[1];
} chain_str_type;


/*
 * Data struct for text editor
 */
typedef struct {
    int wid, hgt;
    int cx, cy;
    int upper, left;
    int old_wid, old_hgt;
    int old_cy;
    int old_upper, old_left;
    int mx, my;
    byte mark;

    object_type *search_o_ptr;
    cptr search_str;
    cptr last_destroyed;

    chain_str_type *yank;
    bool yank_eol;

    cptr *lines_list;
    byte states[MAX_LINES];

    u16b dirty_flags;
    int dirty_line;
    int filename_mode;
    int old_com_id;

    bool changed;
} text_body_type;


/*
 * Dirty flag for text editor
 */
#define DIRTY_ALL           0x0001
#define DIRTY_MODE          0x0004
#define DIRTY_SCREEN        0x0008
#define DIRTY_NOT_FOUND     0x0010
#define DIRTY_NO_SEARCH     0x0020
#define DIRTY_EXPRESSION    0x0040
#define DIRTY_SKIP_INACTIVE 0x0080
#define DIRTY_INACTIVE      0x0100

/*
 * Describe which kind of object is Auto-picked/destroyed
 */
static void describe_autopick(char *buff, autopick_type *entry)
{
    cptr str = entry->name;
    byte act = entry->action;
    cptr insc = entry->insc;
    int i;

    bool top = FALSE;


    cptr before_str[20], after_str[20], which_str[20], whose_str[20], body_str;
    int before_n = 0, after_n = 0, which_n = 0, whose_n = 0;

    body_str = "items";

    /*** Collecting items ***/
    /*** Which can be absorbed into a slot as a bundle ***/
    if (IS_FLG(FLG_COLLECTING))
        which_str[which_n++] = "can be absorbed into an existing inventory slot";

    /*** Unaware items ***/
    if (IS_FLG(FLG_UNAWARE))
    {
        before_str[before_n++] = "unidentified";
        whose_str[whose_n++] = "basic abilities are not known";
    }

    /*** Unsensed ***/
    if (IS_FLG(FLG_UNSENSED))
        before_str[before_n++] = "unsensed";

    /*** Unidentified ***/
    if (IS_FLG(FLG_UNIDENTIFIED))
        before_str[before_n++] = "unidentified";

    /*** Identified ***/
    if (IS_FLG(FLG_IDENTIFIED))
        before_str[before_n++] = "identified";

    /*** *Identified* ***/
    if (IS_FLG(FLG_STAR_IDENTIFIED))
        before_str[before_n++] = "fully identified";

    /*** Rare equipments ***/
    if (IS_FLG(FLG_RARE))
    {
        before_str[before_n++] = "very rare";
        body_str = "equipments";
        after_str[after_n++] = "like Dragon armors, Blades of Chaos, etc.";
    }

    /*** Common equipments ***/
    if (IS_FLG(FLG_COMMON))
    {
        before_str[before_n++] = "relatively common";
        body_str = "equipments";
        after_str[after_n++] = "compared to very rare Dragon armors, Blades of Chaos, etc.";
    }

    /*** Worthless items ***/
    if (IS_FLG(FLG_WORTHLESS))
    {
        before_str[before_n++] = "worthless";
        which_str[which_n++] = "can not be sold at stores";
    }

    /*** Icky/uncomfortable item ***/
    if (IS_FLG(FLG_ICKY))
    {
        before_str[before_n++] = "uncomfortable";
    }

    /*** Artifacto ***/
    if (IS_FLG(FLG_ARTIFACT))
    {
        before_str[before_n++] = "artifact";
    }

    /*** Ego ***/
    if (IS_FLG(FLG_EGO))
    {
        before_str[before_n++] = "ego";
    }

    /*** Good ***/
    if (IS_FLG(FLG_GOOD))
    {
        body_str = "equipment";
        which_str[which_n++] = "have good quality";
    }

    if (IS_FLG(FLG_CURSED))
    {
        body_str = "equipment";
        which_str[which_n++] = "are cursed";
    }

    if (IS_FLG(FLG_SPECIAL))
    {
        body_str = "item";
        which_str[which_n++] = "your race/class needs";
    }

    if (IS_FLG(FLG_UNUSABLE))
    {
        body_str = "equipment";
        which_str[which_n++] = "you cannot wear";
    }

    /*** Nameless ***/
    if (IS_FLG(FLG_NAMELESS))
    {
        body_str = "equipment";
        which_str[which_n++] = "are neither ego-item nor artifact";
    }

    /*** Average ***/
    if (IS_FLG(FLG_AVERAGE))
    {
        body_str = "equipment";
        which_str[which_n++] = "have average quality";
    }

    /*** Dice boosted (weapon of slaying) ***/
    if (IS_FLG(FLG_BOOSTED))
    {
        body_str = "weapons";
        whose_str[whose_n++] = "damage dice is bigger than normal";
    }

    /*** Weapons whose dd*ds is more than nn ***/
    if (IS_FLG(FLG_MORE_DICE))
    {
        static char more_than_desc_str[] =
            "maximum damage from dice is more than __";
        body_str = "weapons";

        sprintf(more_than_desc_str + sizeof(more_than_desc_str) - 3,
            "%d", entry->dice);
        whose_str[whose_n++] = more_than_desc_str;
    }

    /*** Items whose magical bonus is more than nn ***/
    if (IS_FLG(FLG_MORE_BONUS))
    {
        static char more_bonus_desc_str[] =
            "magical bonus is larger than (+__)";

        sprintf(more_bonus_desc_str + sizeof(more_bonus_desc_str) - 4,
            "%d)", entry->bonus);
        whose_str[whose_n++] = more_bonus_desc_str;
    }

    if (IS_FLG(FLG_MORE_LEVEL))
    {
        static char more_level_desc_str[50];
        sprintf(more_level_desc_str, "level is higher than %d", entry->level);
        whose_str[whose_n++] = more_level_desc_str;
    }
    if (IS_FLG(FLG_MORE_WEIGHT))
    {
        static char more_weight_desc_str[50];
        sprintf(more_weight_desc_str, "weight is more than %d lbs", entry->weight);
        whose_str[whose_n++] = more_weight_desc_str;
    }
    if (IS_FLG(FLG_MORE_CHARGES))
    {
        static char more_charges_desc_str[50];
        sprintf(more_charges_desc_str, "number of charges is higher than %d", entry->charges);
        whose_str[whose_n++] = more_charges_desc_str;
    }
    if (IS_FLG(FLG_MORE_VALUE))
    {
        static char more_value_desc_str[50];
        sprintf(more_value_desc_str, "known value is more than %d", entry->value);
        whose_str[whose_n++] = more_value_desc_str;
    }

    /*** Wanted monster's corpse/skeletons ***/
    if (IS_FLG(FLG_WANTED))
    {
        body_str = "corpse or skeletons";
        which_str[which_n++] = "are wanted at the Hunter's Office";
    }

    /*** Human corpse/skeletons (for Daemon magic) ***/
    if (IS_FLG(FLG_HUMAN))
    {
        before_str[before_n++] = "humanoid";
        body_str = "corpse or skeletons";
        which_str[which_n++] = "can be used for Daemon magic";
    }

    /*** Unique monster's corpse/skeletons/statues ***/
    if (IS_FLG(FLG_UNIQUE))
    {
        before_str[before_n++] = "unique monster's";
        body_str = "corpse or skeletons";
    }

    /*** Unreadable spellbooks ***/
    if (IS_FLG(FLG_UNREADABLE))
    {
        body_str = "spellbooks";
        after_str[after_n++] = "of different realms from yours";
    }

    /*** First realm spellbooks ***/
    if (IS_FLG(FLG_REALM1))
    {
        body_str = "spellbooks";
        after_str[after_n++] = "of your first realm";
    }

    /*** Second realm spellbooks ***/
    if (IS_FLG(FLG_REALM2))
    {
        body_str = "spellbooks";
        after_str[after_n++] = "of your second realm";
    }

    /*** First rank spellbooks ***/
    if (IS_FLG(FLG_FIRST))
    {
        before_str[before_n++] = "first one of four";
        body_str = "spellbooks";
    }

    /*** Second rank spellbooks ***/
    if (IS_FLG(FLG_SECOND))
    {
        before_str[before_n++] = "second one of four";
        body_str = "spellbooks";
    }

    /*** Third rank spellbooks ***/
    if (IS_FLG(FLG_THIRD))
    {
        before_str[before_n++] = "third one of four";
        body_str = "spellbooks";
    }

    /*** Fourth rank spellbooks ***/
    if (IS_FLG(FLG_FOURTH))
    {
        before_str[before_n++] = "fourth one of four";
        body_str = "spellbooks";
    }

    /*** Items ***/
    if (IS_FLG(FLG_ITEMS))
        ; /* Nothing to do */
    else if (IS_FLG(FLG_WEAPONS))
        body_str = "melee weapons";
    else if (IS_FLG(FLG_FAVORITE_WEAPONS))
        body_str = "favorite weapons";
    else if (IS_FLG(FLG_HAFTED))
        body_str = "hafted weapons";
    else if (IS_FLG(FLG_DIGGERS))
        body_str = "digging implements";
    else if (IS_FLG(FLG_SHOOTERS))
        body_str = "slings, bows or crossbows";
    else if (IS_FLG(FLG_AMMO))
        body_str = "shots, arrows or bolts";
    else if (IS_FLG(FLG_ARMORS))
        body_str = "armors";
    else if (IS_FLG(FLG_WANDS))
        body_str = "wands";
    else if (IS_FLG(FLG_STAVES))
        body_str = "staves";
    else if (IS_FLG(FLG_RODS))
        body_str = "rods";
    else if (IS_FLG(FLG_POTIONS))
        body_str = "potions";
    else if (IS_FLG(FLG_SCROLLS))
        body_str = "scrolls";
    else if (IS_FLG(FLG_LIGHTS))
        body_str = "light sources";
    else if (IS_FLG(FLG_JUNKS))
        body_str = "junk such as broken sticks";
    else if (IS_FLG(FLG_CORPSES))
        body_str = "corpses";
    else if (IS_FLG(FLG_SKELETONS))
        body_str = "skeletons";
    else if (IS_FLG(FLG_SPELLBOOKS))
        body_str = "spellbooks";
    else if (IS_FLG(FLG_SHIELDS))
        body_str = "shields";
    else if (IS_FLG(FLG_RINGS))
        body_str = "rings";
    else if (IS_FLG(FLG_AMULETS))
        body_str = "amulets";
    else if (IS_FLG(FLG_SUITS))
        body_str = "body armors";
    else if (IS_FLG(FLG_CLOAKS))
        body_str = "cloaks";
    else if (IS_FLG(FLG_HELMS))
        body_str = "helms or crowns";
    else if (IS_FLG(FLG_GLOVES))
        body_str = "gloves";
    else if (IS_FLG(FLG_BOOTS))
        body_str = "boots";

    /* Prepare a string for item name */
    if (*str)
    {
        if (*str == '^')
        {
            str++;
            top = TRUE;
            whose_str[whose_n++] = "name begins with \"";
        }
        else
            which_str[which_n++] = "have \"";
    }


    /* Describe action flag */
    if (act & DONT_AUTOPICK)
        strcpy(buff, "Leave on floor ");
    else if (act & DO_AUTODESTROY)
        strcpy(buff, "Destroy ");
    else if (act & DO_QUERY_AUTOPICK)
        strcpy(buff, "Ask to pick up ");
    else
        strcpy(buff, "Pick up ");

    if (act & DO_AUTO_ID)
        strcat(buff, "and automatically identify ");

    /* Auto-insctiption */
    if (insc)
    {
        strncat(buff, format("and inscribe \"%s\"", insc), 80);

        if (my_strstr(insc, "%all"))
            strcat(buff, ", replacing %all with code string representing all abilities,");
        else if (my_strstr(insc, "%"))
            strcat(buff, ", replacing % with code string representing extra random abilities,");

        strcat(buff, " on ");
    }

    /* Adjective */
    if (!before_n)
        strcat(buff, "all ");
    else for (i = 0; i < before_n && before_str[i]; i++)
    {
        strcat(buff, before_str[i]);
        strcat(buff, " ");
    }

    /* Item class */
    strcat(buff, body_str);

    /* of ... */
    for (i = 0; i < after_n && after_str[i]; i++)
    {
        strcat(buff, " ");
        strcat(buff, after_str[i]);
    }

    /* which ... */
    for (i = 0; i < whose_n && whose_str[i]; i++)
    {
        if (i == 0)
            strcat(buff, " whose ");
        else
            strcat(buff, ", and ");

        strcat(buff, whose_str[i]);
    }

    /* Item name ; whose name is beginning with "str" */
    if (*str && top)
    {
        strcat(buff, str);
        strcat(buff, "\"");
    }

    /* whose ..., and which .... */
    if (whose_n && which_n)
        strcat(buff, ", and");

    /* which ... */
    for (i = 0; i < which_n && which_str[i]; i++)
    {
        if (i == 0)
            strcat(buff, " which ");
        else
            strcat(buff, ", and ");

        strcat(buff, which_str[i]);
    }

    /* Item name ; which have "str" as part of its name */
    if (*str && !top)
    {
        strncat(buff, str, 80);
        strcat(buff, "\" as part of their name");
    }
    strcat(buff, ".");

    /* Describe whether it will be displayed on the full map or not */
    if (act & (DO_AUTOPICK | DO_QUERY_AUTOPICK))
    {
        strcat(buff, " Display these items as wanted in object list.");
    }
    else
        strcat(buff, " Do not display as wanted.");

}


/*
 * Read whole lines of a file to memory
 */
static cptr *read_text_lines(cptr filename)
{
    cptr *lines_list = NULL;
    FILE *fff;

    int lines = 0;
    char buf[1024];

    path_build(buf, sizeof(buf), (streq("pickpref.prf", filename) ? ANGBAND_DIR_PREF : ANGBAND_DIR_USER), filename);

    /* Open the file */
    fff = my_fopen(buf, "r");

    if (fff)
    {
        /* Allocate list of pointers */
        C_MAKE(lines_list, MAX_LINES, cptr);

        /* Parse it */
        while (0 == my_fgets(fff, buf, sizeof(buf)))
        {
            lines_list[lines++] = z_string_make(buf);
            if (lines >= MAX_LINES - 1) break;
        }
        if (lines == 0)
            lines_list[0] = z_string_make("");

        my_fclose(fff);
    }

    if (!fff) return NULL;
    return lines_list;
}


/*
 * Copy the default autopick file to the user directory
 */
static void prepare_default_pickpref(void)
{
    static char *messages[] = {
        "You have activated the Auto-Picker Editor for the first time.",
        "Since user pref file for autopick is not yet created,",
        "the default setting is loaded from lib/pref/pickpref.prf .",
        NULL
    };

    char buf[1024];
    char buf_src[255];
    char buf_dest[255];
    FILE *pref_fp;
    FILE *user_fp;
    int i;

    sprintf(buf_src, "%s", pickpref_filename(PT_DEFAULT, NULL));
    sprintf(buf_dest, "%s", pickpref_filename(PT_WITH_PNAME, NULL));

    /* Display messages */
    for (i = 0; messages[i]; i++) msg_print(messages[i]);
    msg_print(NULL);


    /* Open new file */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, buf_dest);
    user_fp = my_fopen(buf, "w");

    /* Failed */
    if (!user_fp) return;

    /* Write header messages for a notification */
    fprintf(user_fp, "#***\n");
    for (i = 0; messages[i]; i++)
    {
        fprintf(user_fp, "#***  %s\n", messages[i]);
    }
    fprintf(user_fp, "#***\n\n\n");


    /* Open the default file */
    path_build(buf, sizeof(buf), ANGBAND_DIR_PREF, buf_src);
    pref_fp = my_fopen(buf, "r");

    /* Failed */
    if (!pref_fp)
    {
        my_fclose(user_fp);
        return;
    }

    /* Copy the contents of default file */
    while (!my_fgets(pref_fp, buf, sizeof(buf)))
        fprintf(user_fp, "%s\n", buf);

    my_fclose(user_fp);
    my_fclose(pref_fp);
}

/*
 * Read an autopick prefence file to memory
 * Prepare default if no user file is found
 */
static cptr *read_pickpref_text_lines(int *filename_mode_p, char *other_base)
{
    char buf[1024];
    cptr *lines_list;

    if (((*filename_mode_p) == PT_WITH_OTHERNAME) && (other_base))
    {
        strcpy(buf, pickpref_filename(*filename_mode_p, other_base));
        lines_list = read_text_lines(buf);
        if (lines_list) return lines_list;
    }

    /* Try a filename with player name */
    *filename_mode_p = PT_WITH_PNAME;
    strcpy(buf, pickpref_filename(*filename_mode_p, NULL));
    lines_list = read_text_lines(buf);

    if (!lines_list)
    {
        /* Use default name */
        *filename_mode_p = PT_DEFAULT;
        strcpy(buf, pickpref_filename(*filename_mode_p, NULL));
        lines_list = read_text_lines(buf);

        if (lines_list)
        {
            *filename_mode_p = PT_WITH_PNAME;
            prepare_default_pickpref();
            return lines_list;
        }
    }

    if (!lines_list)
    {
        /* There is no preference file in the user directory */
        *filename_mode_p = PT_WITH_PNAME;
        strcpy(buf, pickpref_filename(*filename_mode_p, NULL));

        /* Copy the default autopick file to the user directory */
        prepare_default_pickpref();

        /* Use default name again */
        lines_list = read_text_lines(buf);
    }

    if (!lines_list)
    {
        /* Allocate list of pointers */
        C_MAKE(lines_list, MAX_LINES, cptr);
        lines_list[0] = z_string_make("");
    }
    return lines_list;
}


/*
 * Write whole lines of memory to a file.
 */
static bool write_text_lines(cptr filename, cptr *lines_list)
{
    FILE *fff;

    int lines = 0;
    char buf[1024];

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, filename);

    /* Open the file */
    fff = my_fopen(buf, "w");
    if (fff)
    {
        for (lines = 0; lines_list[lines]; lines++)
            my_fputs(fff, lines_list[lines], 1024);

        my_fclose(fff);
    }

    if (!fff) return FALSE;
    return TRUE;
}


/*
 * Free memory of lines_list.
 */
static void free_text_lines(cptr *lines_list)
{
    int lines;

    for (lines = 0; lines_list[lines]; lines++)
        z_string_free(lines_list[lines]);

    /* free list of pointers */
    C_FREE((char **)lines_list, MAX_LINES, char *);
}


/*
 * Delete or insert string
 */
static void toggle_keyword(text_body_type *tb, int flg)
{
    int by1, by2, y;
    bool add = TRUE;
    bool fixed = FALSE;

    /* Some lines are selected */
    if (tb->mark)
    {
        by1 = MIN(tb->my, tb->cy);
        by2 = MAX(tb->my, tb->cy);
    }

    /* No mark -- Select current line */
    else /* if (!tb->mark) */
    {
        by1 = by2 = tb->cy;
    }


    /* Set/Reset flag of each line */
    for (y = by1; y <= by2; y++)
    {
        autopick_type an_entry = {0}, *entry = &an_entry;

        if (!autopick_new_entry(entry, tb->lines_list[y], !fixed)) continue;

        z_string_free(tb->lines_list[y]);

        if (!fixed)
        {
            /* Add? or Remove? */
            if (!IS_FLG(flg)) add = TRUE;
            else add = FALSE;

            /* No more change */
            fixed = TRUE;
        }

        /* You can use only one noun flag */
        if (FLG_NOUN_BEGIN <= flg && flg <= FLG_NOUN_END)
        {
            int i;
            for (i = FLG_NOUN_BEGIN; i <= FLG_NOUN_END; i++)
                REM_FLG(i);
        }

        /* You can use only one identify state flag */
        else if (FLG_KNOWLEDGE_BEGIN <= flg && flg <= FLG_KNOWLEDGE_END)
        {
            int i;
            for (i = FLG_KNOWLEDGE_BEGIN; i <= FLG_KNOWLEDGE_END; i++)
                REM_FLG(i);
        }

        /* You can use only one flag in artifact/ego/nameless */
        else if (FLG_QUALITY_BEGIN <= flg && flg <= FLG_QUALITY_END)
        {
            int i;
            for (i = FLG_QUALITY_BEGIN; i <= FLG_QUALITY_END; i++)
                REM_FLG(i);
        }

        /* You can use only one flag in rare/common */
        else if (FLG_RARE <= flg && flg <= FLG_COMMON)
        {
            int i;
            for (i = FLG_RARE; i <= FLG_COMMON; i++)
                REM_FLG(i);
        }

        if (add) ADD_FLG(flg);
        else
        {
            REM_FLG(flg);
            if (FLG_NOUN_BEGIN <= flg && flg <= FLG_NOUN_END)
                ADD_FLG(FLG_ITEMS);
        }

        tb->lines_list[y] = autopick_line_from_entry_kill(entry);

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Text is changed */
        tb->changed = TRUE;
    }
}


/*
 * Change command letter
 */
static void toggle_command_letter(text_body_type *tb, byte flg)
{
    autopick_type an_entry = {0}, *entry = &an_entry;
    int by1, by2, y;
    bool add = TRUE;
    bool fixed = FALSE;

    /* Some lines are selected */
    if (tb->mark)
    {
        by1 = MIN(tb->my, tb->cy);
        by2 = MAX(tb->my, tb->cy);
    }

    /* No mark -- Select current line */
    else /* if (!tb->mark) */
    {
        by1 = by2 = tb->cy;
    }


    /* Set/Reset flag of each line */
    for (y = by1; y <= by2; y++)
    {
        int wid = 0;

        if (!autopick_new_entry(entry, tb->lines_list[y], FALSE)) continue;

        z_string_free(tb->lines_list[y]);

        if (!fixed)
        {
            /* Add? or Remove? */
            if (!(entry->action & flg)) add = TRUE;
            else add = FALSE;

            /* No more change */
            fixed = TRUE;
        }

        /* Count number of letter (by negative number) */
        if (entry->action & DONT_AUTOPICK) wid--;
        else if (entry->action & DO_AUTODESTROY) wid--;
        else if (entry->action & DO_QUERY_AUTOPICK) wid--;

        if (entry->action & DO_AUTO_ID) wid--;
        if (!(entry->action & DO_DISPLAY)) wid--;

        /* Set/Reset the flag */
        if (flg == DO_DISPLAY)
        {
            entry->action &= ~(DO_DISPLAY);
            if (add) entry->action |= flg;
        }
        else if (flg == DO_AUTO_ID)
        {
            entry->action &= ~(DO_AUTO_ID);
            if (add) entry->action |= flg;
        }
        else
        {
            entry->action &= ~(DO_AUTOPICK | DONT_AUTOPICK | DO_AUTODESTROY | DO_QUERY_AUTOPICK);
            if (add) entry->action |= flg;
            else entry->action |= DO_AUTOPICK;
        }

        /* Correct cursor location */
        if (tb->cy == y)
        {
            if (entry->action & DONT_AUTOPICK) wid++;
            else if (entry->action & DO_AUTODESTROY) wid++;
            else if (entry->action & DO_QUERY_AUTOPICK) wid++;
            if (entry->action & DO_AUTO_ID) wid++;
            if (!(entry->action & DO_DISPLAY)) wid++;

            if (wid > 0) tb->cx++;
            if (wid < 0 && tb->cx > 0) tb->cx--;
        }

        tb->lines_list[y] = autopick_line_from_entry_kill(entry);

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Text is changed */
        tb->changed = TRUE;
    }
}

/*
 * Delete or insert string
 */
static void add_keyword(text_body_type *tb, int flg)
{
    int by1, by2, y;

    /* Some lines are selected */
    if (tb->mark)
    {
        by1 = MIN(tb->my, tb->cy);
        by2 = MAX(tb->my, tb->cy);
    }

    /* No mark -- Select current line */
    else /* if (!tb->mark) */
    {
        by1 = by2 = tb->cy;
    }


    /* Set/Reset flag of each line */
    for (y = by1; y <= by2; y++)
    {
        autopick_type an_entry = {0}, *entry = &an_entry;

        if (!autopick_new_entry(entry, tb->lines_list[y], FALSE)) continue;

        /* There is the flag already */
        if (IS_FLG(flg))
        {
            /* Free memory for the entry */
            autopick_free_entry(entry);

            continue;
        }

        z_string_free(tb->lines_list[y]);

        /* Remove all noun flag */
        if (FLG_NOUN_BEGIN <= flg && flg <= FLG_NOUN_END)
        {
            int i;
            for (i = FLG_NOUN_BEGIN; i <= FLG_NOUN_END; i++)
                REM_FLG(i);
        }

        ADD_FLG(flg);

        tb->lines_list[y] = autopick_line_from_entry_kill(entry);

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Text is changed */
        tb->changed = TRUE;
    }
}


/*
 * Check if this line is expression or not.
 * And update it if it is.
 */
static void check_expression_line(text_body_type *tb, int y)
{
    cptr s = tb->lines_list[y];

    if ((s[0] == '?' && s[1] == ':') ||
        (tb->states[y] & LSTAT_BYPASS))
    {
        /* Expressions need re-evaluation */
        tb->dirty_flags |= DIRTY_EXPRESSION;
    }
}


/*
 * Add an empty line at the last of the file
 */
static bool add_empty_line(text_body_type *tb)
{
    int k;

    for (k = 0; tb->lines_list[k]; k++)
        /* count number of lines */ ;

    /* Too many lines! */
    if (k >= MAX_LINES - 2) return FALSE;

    /* The last line is already empty */
    if (!tb->lines_list[k-1][0]) return FALSE;

    /* Create new empty line */
    tb->lines_list[k] = z_string_make("");

    /* Expressions need re-evaluation */
    tb->dirty_flags |= DIRTY_EXPRESSION;

    /* Text is changed */
    tb->changed = TRUE;

    /* A line is added */
    return TRUE;
}


/*
 * Insert return code and split the line
 */
static bool insert_return_code(text_body_type *tb)
{
    char buf[MAX_LINELEN];
    int i, j, k;

    for (k = 0; tb->lines_list[k]; k++)
        /* count number of lines */ ;

    if (k >= MAX_LINES - 2) return FALSE;
    k--;

    /* Move down lines */
    for (; tb->cy < k; k--)
    {
        tb->lines_list[k+1] = tb->lines_list[k];
        tb->states[k+1] = tb->states[k];
    }

    /* Split current line */
    for (i = j = 0; tb->lines_list[tb->cy][i] && i < tb->cx; i++)
    {
        buf[j++] = tb->lines_list[tb->cy][i];
    }
    buf[j] = '\0';
    tb->lines_list[tb->cy+1] = z_string_make(&tb->lines_list[tb->cy][i]);
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(buf);

    /* Expressions need re-evaluation */
    tb->dirty_flags |= DIRTY_EXPRESSION;

    /* Text is changed */
    tb->changed = TRUE;

    return TRUE;
}


/*
 * Choose an item and get auto-picker entry from it.
 */
static bool entry_from_choosed_object(autopick_type *entry)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Enter which item?";
    prompt.error = "You have nothing to enter.";
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    autopick_entry_from_object(entry, prompt.obj);
    return TRUE;
}


/*
 * Choose an item for search
 */
static byte get_object_for_search(object_type **o_handle, cptr *search_strp)
{
    char buf[MAX_NLEN+20];
    obj_prompt_t prompt = {0};

    prompt.prompt = "Enter which item?";
    prompt.error = "You have nothing to enter.";
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return 0;

    *o_handle = prompt.obj;

    z_string_free(*search_strp);
    object_desc(buf, *o_handle, (OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL));
    *search_strp = z_string_make(format("<%s>", buf));
    return 1;
}


/*
 * Prepare for search by destroyed object
 */
static byte get_destroyed_object_for_search(object_type **o_handle, cptr *search_strp)
{
    char buf[MAX_NLEN+20];

    if (!autopick_last_destroyed_object.k_idx) return 0;

    *o_handle = &autopick_last_destroyed_object;

    z_string_free(*search_strp);
    object_desc(buf, *o_handle, (OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL));
    *search_strp = z_string_make(format("<%s>", buf));
    return 1;
}


/*
 * Choose an item or string for search
 */
static byte get_string_for_search(object_type **o_handle, cptr *search_strp)
{
    int pos = 0;

    /*
     * Text color
     * TERM_YELLOW : Overwrite mode
     * TERM_WHITE : Insert mode
     */
    byte color = TERM_YELLOW;
    char buf[MAX_NLEN+20];
    const int len = 80;

    char prompt[] = "Search key(^I:inven ^L:destroyed): ";
    int col = sizeof(prompt) - 1;

    /* Prepare string buffer for edit */
    if (*search_strp) strcpy(buf, *search_strp);
    else buf[0] = '\0';

    /* Object searching mode */
    if (*o_handle)
    {
        color = TERM_L_GREEN;
    }

    /* Display prompt */
    prt(prompt, 0, 0);


    /* Process input */
    while (TRUE)
    {
        bool back = FALSE;
        int skey;

        /* Display the string */
        Term_erase(col, 0, 255);
        Term_putstr(col, 0, -1, color, buf);

        /* Place cursor */
        Term_gotoxy(col + pos, 0);

        /* Get a special key code */
        skey = inkey_special(TRUE);

        /* Analyze the key */
        switch (skey)
        {
        case SKEY_LEFT:
        case KTRL('b'):
        {
            int i = 0;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at beginning of line */
            if (0 == pos) break;

            while (TRUE)
            {
                int next_pos = i + 1;


                /* Is there the cursor at next position? */
                if (next_pos >= pos) break;

                /* Move to next */
                i = next_pos;
            }

            /* Get previous position */
            pos = i;

            break;
        }

        case SKEY_RIGHT:
        case KTRL('f'):
            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at end of line */
            if ('\0' == buf[pos]) break;

            pos++;

            break;

        case ESCAPE:
            return 0;

        case KTRL('r'):
            back = TRUE;
            /* Fall through */

        case '\n':
        case '\r':
        case KTRL('s'):
            if (*o_handle) return (back ? -1 : 1);
            z_string_free(*search_strp);
            *search_strp = z_string_make(buf);
            *o_handle = NULL;
            return (back ? -1 : 1);

        case KTRL('i'):
            /* Get an item */
            return get_object_for_search(o_handle, search_strp);

        case KTRL('l'):
            /* Prepare string for destroyed object if there is one. */
            if (get_destroyed_object_for_search(o_handle, search_strp))
                return 1;
            break;

        case '\010':
        {
            /* Backspace */

            int i = 0;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at beginning of line */
            if (0 == pos) break;

            while (TRUE)
            {
                int next_pos = i + 1;


                /* Is there the cursor at next position? */
                if (next_pos >= pos) break;

                /* Move to next */
                i = next_pos;
            }

            /* Get previous position */
            pos = i;

            /* Fall through to 'Delete key' */
        }

        case 0x7F:
        case KTRL('d'):
            /* Delete key */
        {
            int dst, src;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at end of line */
            if ('\0' == buf[pos]) break;

            /* Position of next character */
            src = pos + 1;


            dst = pos;

            /* Move characters at src to dst */
            while ('\0' != (buf[dst++] = buf[src++]))
                /* loop */;

            break;
        }

        default:
        {
            /* Insert a character */

            char tmp[100];
            char c;

            /* Ignore special keys */
            if (skey & SKEY_MASK) break;

            /* Get a character code */
            c = (char)skey;

            /* Was non insert mode? */
            if (color != TERM_WHITE)
            {
                /* Was object searching mode */
                if (color == TERM_L_GREEN)
                {
                    /* Cancel the mode */
                    *o_handle = NULL;

                    /* Remove indicating string */
                    z_string_free(*search_strp);
                    *search_strp = NULL;
                }

                /* Overwrite default string */
                buf[0] = '\0';

                /* Go to insert mode */
                color = TERM_WHITE;
            }

            /* Save right part of string */
            strcpy(tmp, buf + pos);
            if (pos < len && isprint(c))
            {
                buf[pos++] = c;
            }
            else
            {
                bell();
            }

            /* Terminate */
            buf[pos] = '\0';

            /* Write back the left part of string */
            my_strcat(buf, tmp, len + 1);

            break;
        } /* default: */

        }

        /* Object searching mode was cancelled? */
        if (*o_handle && color != TERM_L_GREEN)
        {
            /* Cancel the mode */
            *o_handle = NULL;

            /* Remove indicating string */
            buf[0] = '\0';
            z_string_free(*search_strp);
            *search_strp = NULL;

        }


    } /* while (TRUE) */
}


/*
 * Search next line matches for o_ptr
 */
static void search_for_object(text_body_type *tb, object_type *o_ptr, bool forward)
{
    autopick_type an_entry = {0}, *entry = &an_entry;
    char o_name[MAX_NLEN];
    int bypassed_cy = -1;

    /* Start searching from current cursor position */
    int i = tb->cy;

    /* Prepare object name string first */
    object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_NO_FLAVOR | OD_OMIT_PREFIX | OD_NO_PLURAL));

    /* Convert the string to lower case */
    str_tolower(o_name);

    while (TRUE)
    {
        bool match;

        /* End of list? */
        if (forward)
        {
            if (!tb->lines_list[++i]) break;
        }
        else
        {
            if (--i < 0) break;
        }

        /* Is this line is a correct entry? */
        if (!autopick_new_entry(entry, tb->lines_list[i], FALSE)) continue;

        /* Does this line match to the object? */
        match = is_autopick_aux(o_ptr, entry, o_name);
        autopick_free_entry(entry);
        if (!match)    continue;

        /* Found a line but it's inactive */
        if (tb->states[i] & LSTAT_BYPASS)
        {
            /* If it is first found, remember it */
            if (bypassed_cy == -1) bypassed_cy = i;
        }

        /* Found an active line! */
        else
        {
            /* Move to this line */
            tb->cx = 0;
            tb->cy = i;

            if (bypassed_cy != -1)
            {
                /* Mark as some lines are skipped */
                tb->dirty_flags |= DIRTY_SKIP_INACTIVE;
            }

            /* Found it! */
            return;
        }
    }

    if (bypassed_cy != -1)
    {
        /* Move to the remembered line */
        tb->cx = 0;
        tb->cy = bypassed_cy;

        /* Mark as this line is inactive */
        tb->dirty_flags |= DIRTY_INACTIVE;
    }

    else
    {
        /* Mark as NOT FOUND */
        tb->dirty_flags |= DIRTY_NOT_FOUND;
    }

    return;
}


/*
 * Search next line matches to the string
 */
static void search_for_string(text_body_type *tb, cptr search_str, bool forward)
{
    int bypassed_cy = -1;
    int bypassed_cx = 0;

    /* Start searching from current cursor position */
    int i = tb->cy;

    while (TRUE)
    {
        cptr pos;

        /* End of list? */
        if (forward)
        {
            if (!tb->lines_list[++i]) break;
        }
        else
        {
            if (--i < 0) break;
        }

        /* Look for the string pattern */
        pos = my_strstr(tb->lines_list[i], search_str);

        /* Not found! */
        if (!pos) continue;

        /* Found a line but it's inactive */
        if ((tb->states[i] & LSTAT_BYPASS) &&
            !(tb->states[i] & LSTAT_EXPRESSION))
        {
            /* If it is first found, remember it */
            if (bypassed_cy == -1)
            {
                bypassed_cy = i;
                bypassed_cx = (int)(pos - tb->lines_list[i]);
            }
        }

        /* Found an active line! */
        else
        {
            /* Move to this location */
            tb->cx = (int)(pos - tb->lines_list[i]);
            tb->cy = i;

            if (bypassed_cy != -1)
            {
                /* Mark as some lines are skipped */
                tb->dirty_flags |= DIRTY_SKIP_INACTIVE;
            }

            /* Found it! */
            return;
        }
    }

    if (bypassed_cy != -1)
    {
        /* Move to the remembered line */
        tb->cx = bypassed_cx;
        tb->cy = bypassed_cy;

        /* Mark as this line is inactive */
        tb->dirty_flags |= DIRTY_INACTIVE;
    }

    else
    {
        /* Mark as NOT FOUND */
        tb->dirty_flags |= DIRTY_NOT_FOUND;
    }

    return;
}




/*
 * Editor command id's
 */
enum {
    EC_QUIT = 1,
    EC_SAVEQUIT,
    EC_REVERT,
    EC_LOAD,
    EC_HELP,
    EC_RETURN,
    EC_LEFT,
    EC_DOWN,
    EC_UP,
    EC_RIGHT,
    EC_BOL,
    EC_EOL,
    EC_PGUP,
    EC_PGDOWN,
    EC_TOP,
    EC_BOTTOM,
    EC_CUT,
    EC_COPY,
    EC_PASTE,
    EC_BLOCK,
    EC_KILL_LINE,
    EC_DELETE_CHAR,
    EC_BACKSPACE,
    EC_SEARCH_STR,
    EC_SEARCH_FORW,
    EC_SEARCH_BACK,
    EC_SEARCH_OBJ,
    EC_SEARCH_DESTROYED,
    EC_INSERT_OBJECT,
    EC_INSERT_DESTROYED,
    EC_INSERT_BLOCK,
    EC_INSERT_MACRO,
    EC_INSERT_KEYMAP,
    EC_CL_AUTOPICK,
    EC_CL_DESTROY,
    EC_CL_LEAVE,
    EC_CL_QUERY,
    EC_CL_NO_DISP,
    EC_CL_AUTO_ID,

    EC_OK_COLLECTING,
    EC_IK_UNAWARE,
    EC_IK_UNSENSED,
    EC_IK_UNIDENTIFIED,
    EC_IK_IDENTIFIED,
    EC_IK_STAR_IDENTIFIED,
    EC_OK_BOOSTED,
    EC_OK_ICKY,
    EC_OK_MORE_DICE,
    EC_OK_MORE_BONUS,
    EC_OK_MORE_LEVEL,
    EC_OK_MORE_WEIGHT,
    EC_OK_MORE_CHARGES,
    EC_OK_MORE_VALUE,
    EC_OK_WORTHLESS,
    EC_OK_ARTIFACT,
    EC_OK_EGO,
    EC_OK_GOOD,
    EC_OK_NAMELESS,
    EC_OK_AVERAGE,
    EC_OK_CURSED,
    EC_OK_SPECIAL,
    EC_OK_UNUSABLE,
    EC_OK_RARE,
    EC_OK_COMMON,
    EC_OK_WANTED,
    EC_OK_UNIQUE,
    EC_OK_HUMAN,
    EC_OK_UNREADABLE,
    EC_OK_REALM1,
    EC_OK_REALM2,
    EC_OK_FIRST,
    EC_OK_SECOND,
    EC_OK_THIRD,
    EC_OK_FOURTH,

    EC_KK_WEAPONS,
    EC_KK_FAVORITE_WEAPONS,
    EC_KK_DIGGERS,
    EC_KK_SHOOTERS,
    EC_KK_AMMO,
    EC_KK_ARMORS,
    EC_KK_WANDS,
    EC_KK_STAVES,
    EC_KK_RODS,
    EC_KK_POTIONS,
    EC_KK_SCROLLS,
    EC_KK_LIGHTS,
    EC_KK_JUNKS,
    EC_KK_CORPSES,
    EC_KK_SPELLBOOKS,
    EC_KK_SHIELDS,
    EC_KK_RINGS,
    EC_KK_AMULETS,
    EC_KK_SUITS,
    EC_KK_CLOAKS,
    EC_KK_HELMS,
    EC_KK_GLOVES,
    EC_KK_BOOTS,
    EC_KK_SKELETONS,
};

/* Manu names */

static char MN_QUIT[] = "Quit without save";
static char MN_SAVEQUIT[] = "Save & Quit";
static char MN_REVERT[] = "Revert all changes";
static char MN_LOAD[] = "Load preferences";
static char MN_HELP[] = "Help";

static char MN_MOVE[] =   "Move cursor";
static char MN_LEFT[] =   "Left     (Left Arrow key)";
static char MN_DOWN[] =   "Down     (Down Arrow key)";
static char MN_UP[] =     "Up       (Up Arrow key)";
static char MN_RIGHT[] =  "Right    (Right Arrow key)";
static char MN_BOL[] =    "Beginning of line";
static char MN_EOL[] =    "End of line";
static char MN_PGUP[] =   "Page up  (PageUp key)";
static char MN_PGDOWN[] = "Page down(PageDown key)";
static char MN_TOP[] =    "Top      (Home key)";
static char MN_BOTTOM[] = "Bottom   (End key)";

static char MN_EDIT[] = "Edit";
static char MN_CUT[] = "Cut";
static char MN_COPY[] = "Copy";
static char MN_PASTE[] = "Paste";
static char MN_BLOCK[] = "Select block";
static char MN_KILL_LINE[] = "Kill rest of line";
static char MN_DELETE_CHAR[] = "Delete character";
static char MN_BACKSPACE[] = "Backspace";
static char MN_RETURN[] = "Return";

static char MN_SEARCH[] = "Search";
static char MN_SEARCH_STR[] = "Search by string";
static char MN_SEARCH_FORW[] = "Search forward";
static char MN_SEARCH_BACK[] = "Search backward";
static char MN_SEARCH_OBJ[] = "Search by inventory object";
static char MN_SEARCH_DESTROYED[] = "Search by destroyed object";

static char MN_INSERT[] = "Insert...";
static char MN_INSERT_OBJECT[] = "Insert name of choosen object";
static char MN_INSERT_DESTROYED[] = "Insert name of destroyed object";
static char MN_INSERT_BLOCK[] = "Insert conditional block";
static char MN_INSERT_MACRO[] = "Insert a macro definition";
static char MN_INSERT_KEYMAP[] = "Insert a keymap definition";

static char MN_COMMAND_LETTER[] = "Action letter";
static char MN_CL_AUTOPICK[] = "' ' (Pick up)";
static char MN_CL_DESTROY[] = "'!' (Destroy)";
static char MN_CL_LEAVE[] = "'~' (Leave on the floor)";
static char MN_CL_QUERY[] = "';' (Query to pick up)";
static char MN_CL_NO_DISP[] = "'(' (Hide on M map)";
static char MN_CL_AUTO_ID[] = "'?' (Identify)";

static char MN_ADJECTIVE_GEN[] = "Adjective (general)";
static char MN_RARE[] = "rare (equipments)";
static char MN_COMMON[] = "common (equipments)";
static char MN_ICKY[] = "icky (equipments)";

static char MN_ADJECTIVE_SPECIAL[] = "Adjective (special)";
static char MN_BOOSTED[] = "dice boosted (weapons)";
static char MN_MORE_DICE[] = "more dice than # (weapons/shooters)";
static char MN_MORE_BONUS[] = "more bonus than # (rings etc.)";
static char MN_MORE_LEVEL[] = "more level than # (corpses)";
static char MN_MORE_WEIGHT[] = "more weight than #";
static char MN_MORE_VALUE[] = "more value than #";
static char MN_MORE_CHARGES[] = "more charges than #";
static char MN_WANTED[] = "wanted (corpse)";
static char MN_UNIQUE[] = "unique (corpse)";
static char MN_HUMAN[] = "human (corpse)";
static char MN_UNREADABLE[] = "unreadable (spellbooks)";
static char MN_REALM1[] = "realm1 (spellbooks)";
static char MN_REALM2[] = "realm2 (spellbooks)";
static char MN_FIRST[] = "first (spellbooks)";
static char MN_SECOND[] = "second (spellbooks)";
static char MN_THIRD[] = "third (spellbooks)";
static char MN_FOURTH[] = "fourth (spellbooks)";

static char MN_NOUN[] = "Keywords (noun)";



typedef struct {
    cptr name;
    int level;
    int key;
    int com_id;
} command_menu_type;


command_menu_type menu_data[] =
{
    {MN_HELP, 0, -1, EC_HELP},
    {MN_QUIT, 0, KTRL('q'), EC_QUIT},
    {MN_SAVEQUIT, 0, KTRL('w'), EC_SAVEQUIT},
    {MN_REVERT, 0, KTRL('z'), EC_REVERT},
    {MN_LOAD, 0, KTRL('j'), EC_LOAD},

    {MN_EDIT, 0, -1, -1},
    {MN_CUT, 1, KTRL('x'), EC_CUT},
    {MN_COPY, 1, KTRL('c'), EC_COPY},
    {MN_PASTE, 1, KTRL('v'), EC_PASTE},
    {MN_BLOCK, 1, KTRL('g'), EC_BLOCK},
    {MN_KILL_LINE, 1, KTRL('k'), EC_KILL_LINE},
    {MN_DELETE_CHAR, 1, KTRL('d'), EC_DELETE_CHAR},
    {MN_BACKSPACE, 1, KTRL('h'), EC_BACKSPACE},
    {MN_RETURN, 1, KTRL('m'), EC_RETURN},

    {MN_SEARCH, 0, -1, -1},
    {MN_SEARCH_STR, 1, KTRL('s'), EC_SEARCH_STR},
    {MN_SEARCH_FORW, 1, -1, EC_SEARCH_FORW},
    {MN_SEARCH_BACK, 1, KTRL('r'), EC_SEARCH_BACK},
    {MN_SEARCH_OBJ, 1, KTRL('y'), EC_SEARCH_OBJ},
    {MN_SEARCH_DESTROYED, 1, -1, EC_SEARCH_DESTROYED},

    {MN_MOVE, 0, -1, -1},
    {MN_LEFT, 1, KTRL('b'), EC_LEFT},
    {MN_DOWN, 1, KTRL('n'), EC_DOWN},
    {MN_UP, 1, KTRL('p'), EC_UP},
    {MN_RIGHT, 1, KTRL('f'), EC_RIGHT},
    {MN_BOL, 1, KTRL('a'), EC_BOL},
    {MN_EOL, 1, KTRL('e'), EC_EOL},
    {MN_PGUP, 1, KTRL('o'), EC_PGUP},
    {MN_PGDOWN, 1, KTRL('l'), EC_PGDOWN},
    {MN_TOP, 1, KTRL('t'), EC_TOP},
    {MN_BOTTOM, 1, KTRL('u'), EC_BOTTOM},

    {MN_INSERT, 0, -1, -1},
    {MN_INSERT_OBJECT, 1, KTRL('i'), EC_INSERT_OBJECT},
    {MN_INSERT_DESTROYED, 1, -1, EC_INSERT_DESTROYED},
    {MN_INSERT_BLOCK, 1, -1, EC_INSERT_BLOCK},
    {MN_INSERT_MACRO, 1, -1, EC_INSERT_MACRO},
    {MN_INSERT_KEYMAP, 1, -1, EC_INSERT_KEYMAP},

     {MN_ADJECTIVE_GEN, 0, -1, -1},
    {KEY_UNAWARE, 1, -1, EC_IK_UNAWARE},
    {KEY_UNSENSED, 1, -1, EC_IK_UNSENSED},
    {KEY_UNIDENTIFIED, 1, -1, EC_IK_UNIDENTIFIED},
    {KEY_IDENTIFIED, 1, -1, EC_IK_IDENTIFIED},
    {KEY_STAR_IDENTIFIED, 1, -1, EC_IK_STAR_IDENTIFIED},
    {KEY_COLLECTING, 1, -1, EC_OK_COLLECTING},
    {KEY_ARTIFACT, 1, -1, EC_OK_ARTIFACT},
    {KEY_EGO, 1, -1, EC_OK_EGO},
    {KEY_GOOD, 1, -1, EC_OK_GOOD},
    {KEY_CURSED, 1, -1, EC_OK_CURSED},
    {KEY_SPECIAL, 1, -1, EC_OK_SPECIAL},
    {KEY_UNUSABLE, 1, -1, EC_OK_UNUSABLE},
    {KEY_NAMELESS, 1, -1, EC_OK_NAMELESS},
    {KEY_AVERAGE, 1, -1, EC_OK_AVERAGE},
    {KEY_WORTHLESS, 1, -1, EC_OK_WORTHLESS},
    {MN_RARE, 1, -1, EC_OK_RARE},
    {MN_COMMON, 1, -1, EC_OK_COMMON},
    {MN_ICKY, 1, -1, EC_OK_ICKY},

     {MN_ADJECTIVE_SPECIAL, 0, -1, -1},
    {MN_BOOSTED, 1, -1, EC_OK_BOOSTED},
    {MN_MORE_DICE, 1, -1, EC_OK_MORE_DICE},
    {MN_MORE_BONUS, 1, -1, EC_OK_MORE_BONUS},
    {MN_MORE_LEVEL, 1, -1, EC_OK_MORE_LEVEL},
    {MN_MORE_WEIGHT, 1, -1, EC_OK_MORE_WEIGHT},
    {MN_MORE_CHARGES, 1, -1, EC_OK_MORE_CHARGES},
    {MN_MORE_VALUE, 1, -1, EC_OK_MORE_VALUE},
    {MN_WANTED, 1, -1, EC_OK_WANTED},
    {MN_UNIQUE, 1, -1, EC_OK_UNIQUE},
    {MN_HUMAN, 1, -1, EC_OK_HUMAN},
    {MN_UNREADABLE, 1, -1, EC_OK_UNREADABLE},
    {MN_REALM1, 1, -1, EC_OK_REALM1},
    {MN_REALM2, 1, -1, EC_OK_REALM2},
    {MN_FIRST, 1, -1, EC_OK_FIRST},
    {MN_SECOND, 1, -1, EC_OK_SECOND},
    {MN_THIRD, 1, -1, EC_OK_THIRD},
    {MN_FOURTH, 1, -1, EC_OK_FOURTH},

     {MN_NOUN, 0, -1, -1},
    {KEY_WEAPONS, 1, -1, EC_KK_WEAPONS},
    {KEY_FAVORITE_WEAPONS, 1, -1, EC_KK_FAVORITE_WEAPONS},
    {KEY_DIGGERS, 1, -1, EC_KK_DIGGERS},

    {KEY_SHOOTERS, 1, -1, EC_KK_SHOOTERS},
    {KEY_AMMO, 1, -1, EC_KK_AMMO},

    {KEY_ARMORS, 1, -1, EC_KK_ARMORS},
    {KEY_SHIELDS, 1, -1, EC_KK_SHIELDS},
    {KEY_SUITS, 1, -1, EC_KK_SUITS},
    {KEY_CLOAKS, 1, -1, EC_KK_CLOAKS},
    {KEY_HELMS, 1, -1, EC_KK_HELMS},
    {KEY_GLOVES, 1, -1, EC_KK_GLOVES},
    {KEY_BOOTS, 1, -1, EC_KK_BOOTS},

    {KEY_LIGHTS, 1, -1, EC_KK_LIGHTS},
    {KEY_RINGS, 1, -1, EC_KK_RINGS},
    {KEY_AMULETS, 1, -1, EC_KK_AMULETS},

    {KEY_SPELLBOOKS, 1, -1, EC_KK_SPELLBOOKS},

    {KEY_WANDS, 1, -1, EC_KK_WANDS},
    {KEY_STAVES, 1, -1, EC_KK_STAVES},
    {KEY_RODS, 1, -1, EC_KK_RODS},
    {KEY_POTIONS, 1, -1, EC_KK_POTIONS},
    {KEY_SCROLLS, 1, -1, EC_KK_SCROLLS},

    {KEY_JUNKS, 1, -1, EC_KK_JUNKS},
    {KEY_CORPSES, 1, -1, EC_KK_CORPSES},
    {KEY_SKELETONS, 1, -1, EC_KK_SKELETONS},

     {MN_COMMAND_LETTER, 0, -1, -1},
    {MN_CL_AUTOPICK, 1, -1, EC_CL_AUTOPICK},
    {MN_CL_DESTROY, 1, -1, EC_CL_DESTROY},
    {MN_CL_LEAVE, 1, -1, EC_CL_LEAVE},
    {MN_CL_QUERY, 1, -1, EC_CL_QUERY},
    {MN_CL_NO_DISP, 1, -1, EC_CL_NO_DISP},
    {MN_CL_AUTO_ID, 1, -1, EC_CL_AUTO_ID},

    {MN_DELETE_CHAR, -1, 0x7F, EC_DELETE_CHAR},

    {NULL, -1, -1, 0}
};


/*
 * Find a command by 'key'.
 */
static int get_com_id(char key)
{
    int i;

    for (i = 0; menu_data[i].name; i++)
    {
        if (menu_data[i].key == key)
        {
            return menu_data[i].com_id;
        }
    }

    return 0;
}


/*
 * Display the menu, and get a command
 */
static int do_command_menu(int level, int start)
{
    int i;
    int max_len = 0;
    int max_menu_wid;
    int col0 = 5 + level*7;
    int row0 = 1 + level*3;
    byte menu_key = 0;
    int menu_id_list[26];
    bool redraw = TRUE;
    char linestr[MAX_LINELEN];

    /* Get max length */
    menu_key = 0;
    for (i = start; menu_data[i].level >= level; i++)
    {
        int len;

        /* Ignore lower level sub menus */
        if (menu_data[i].level > level) continue;

        len = strlen(menu_data[i].name);
        if (len > max_len) max_len = len;

        menu_id_list[menu_key] = i;
        menu_key++;
    }

    while (menu_key < 26)
    {
        menu_id_list[menu_key] = -1;
        menu_key++;
    }

    /* Extra space for displaying menu key and command key */
    max_menu_wid = max_len + 3 + 3;

    /* Prepare box line */
    linestr[0] = '\0';
    strcat(linestr, "+");
    for (i = 0; i < max_menu_wid + 2; i++)
    {
        strcat(linestr, "-");
    }
    strcat(linestr, "+");

    while (TRUE)
    {
        int com_id;
        char key;
        int menu_id;

        if (redraw)
        {
            int row1 = row0 + 1;

            /* Draw top line */
            Term_putstr(col0, row0, -1, TERM_BLUE, linestr);

            /* Draw menu items */
            menu_key = 0;
            for (i = start; menu_data[i].level >= level; i++)
            {
                char com_key_str[3];
                cptr str;

                /* Ignore lower level sub menus */
                if (menu_data[i].level > level) continue;

                if (menu_data[i].com_id == -1)
                {
                    strcpy(com_key_str, ">");
                }
                else if (menu_data[i].key != -1)
                {
                    com_key_str[0] = '^';
                    com_key_str[1] = menu_data[i].key + '@';
                    com_key_str[2] = '\0';
                }
                else
                {
                    com_key_str[0] = '\0';
                }

                str = format(" %c) %-*s %2s ", menu_key + 'a', max_len, menu_data[i].name, com_key_str);

                Term_putch(col0, row1, TERM_BLUE, '|');
                Term_putstr(col0 + 1, row1, -1, TERM_L_BLUE, str);
                Term_addch(TERM_BLUE, '|');
                row1++;

                menu_key++;
            }

            /* Draw bottom line */
            Term_putstr(col0, row1, -1, TERM_BLUE, linestr);

            /* The menu was shown */
            redraw = FALSE;
        }
        c_prt(TERM_L_BLUE, format("(a-%c) Command:", menu_key + 'a' - 1), 0, 0);
        key = inkey();

        if (key == ESCAPE) return 0;

        if ('a' <= key && key <= 'z')
        {
            menu_id = menu_id_list[key - 'a'];

            if (menu_id >= 0)
            {
                com_id = menu_data[menu_id].com_id;

                if (com_id == -1)
                {
                    com_id = do_command_menu(level + 1, menu_id + 1);

                    if (com_id) return com_id;
                    else redraw = TRUE;
                }
                else if (com_id)
                {
                    return com_id;
                }
            }
        }

        else
        {
            com_id = get_com_id(key);
            if (com_id) return com_id;
            else continue;
        }
    }
}


static chain_str_type *new_chain_str(cptr str)
{
    chain_str_type *chain;

    size_t len = strlen(str);

    chain = (chain_str_type *)ralloc(sizeof(chain_str_type) + len * sizeof(char));

    strcpy(chain->s, str);
    chain->next = NULL;

    return chain;
}


static void kill_yank_chain(text_body_type *tb)
{
    chain_str_type *chain = tb->yank;
    tb->yank = NULL;
    tb->yank_eol = TRUE;

    while (chain)
    {
        chain_str_type *next = chain->next;
        size_t len = strlen(chain->s);

        rnfree(chain, sizeof(chain_str_type) + len * sizeof(char));

        chain = next;
    }
}


static void add_str_to_yank(text_body_type *tb, cptr str)
{
    chain_str_type *chain;

    tb->yank_eol = FALSE;

    if (NULL == tb->yank)
    {
        tb->yank = new_chain_str(str);
        return;
    }

    chain = tb->yank;

    while (TRUE)
    {
        if (!chain->next)
        {
            chain->next = new_chain_str(str);
            return;
        }

        /* Go to next */
        chain = chain->next;
    }
}


/*
 * Do work for the copy editor-command
 */
static void copy_text_to_yank(text_body_type *tb)
{
    int len = strlen(tb->lines_list[tb->cy]);

    /* Correct cursor location */
    if (tb->cx > len) tb->cx = len;

    /* Use single line? */
    if (!tb->mark)
    {
        /* Select a single line */
        tb->cx = 0;
        tb->my = tb->cy;
        tb->mx = len;
    }

    /* Kill old yank buffer */
    kill_yank_chain(tb);


    /* Single line case */
    if (tb->my == tb->cy)
    {
        int i;
        char buf[MAX_LINELEN];
        int bx1 = MIN(tb->mx, tb->cx);
        int bx2 = MAX(tb->mx, tb->cx);

        /* Correct fake cursor position */
        if (bx2 > len) bx2 = len;

        /* Whole part of this line is selected */
        if (bx1 == 0 && bx2 == len)
        {
            /* Copy this line */
            add_str_to_yank(tb, tb->lines_list[tb->cy]);

            /* Add end of line to the buffer */
            add_str_to_yank(tb, "");
        }

        /* Segment of this line is selected */
        else
        {
            for (i = 0; i < bx2 - bx1; i++)
            {
                buf[i] = tb->lines_list[tb->cy][bx1 + i];
            }
            buf[i] = '\0';

            /* Copy this segment of line */
            add_str_to_yank(tb, buf);
        }
    }

    /* Multiple lines case */
    else /* if (tb->my != tb->cy) */
    {
        int y;

        int by1 = MIN(tb->my, tb->cy);
        int by2 = MAX(tb->my, tb->cy);

        /* Copy lines */
        for (y = by1; y <= by2; y++)
        {
            /* Copy this line */
            add_str_to_yank(tb, tb->lines_list[y]);
        }

        /* Add final end of line to the buffer */
        add_str_to_yank(tb, "");
    }

    /* Disable selection */
    tb->mark = 0;

    /* Now dirty */
    tb->dirty_flags |= DIRTY_ALL;
}


#define DESCRIPT_HGT 3

/*
 * Draw text
 */
static void draw_text_editor(text_body_type *tb)
{
    int i;
    int by1 = 0, by2 = 0;
    doc_ptr doc = 0;

    /* Get size */
    Term_get_size(&tb->wid, &tb->hgt);
    doc = doc_alloc(tb->wid);
    doc_insert(doc, "<style:screenshot>"); /* No linebreak and as wide as possible ... */

    /*
     * Top line (-1), description line (-3), separator (-1)
     *  == -5
     */
    tb->hgt -= 2 + DESCRIPT_HGT;


    /* Scroll if necessary */
    if (tb->cy < tb->upper || tb->upper + tb->hgt <= tb->cy)
        tb->upper = tb->cy - (tb->hgt)/2;
    if (tb->upper < 0)
        tb->upper = 0;
    if ((tb->cx < tb->left + 10 && tb->left > 0) || tb->left + tb->wid - 5 <= tb->cx)
        tb->left = tb->cx - (tb->wid)*2/3;
    if (tb->left < 0)
        tb->left = 0;

    /* Redraw whole window after resize */
    if (tb->old_wid != tb->wid || tb->old_hgt != tb->hgt)
        tb->dirty_flags |= DIRTY_SCREEN;

    /* Redraw all text after scroll */
    else if (tb->old_upper != tb->upper || tb->old_left != tb->left)
        tb->dirty_flags |= DIRTY_ALL;


    if (tb->dirty_flags & DIRTY_SCREEN)
    {
        tb->dirty_flags |= (DIRTY_ALL | DIRTY_MODE);

        /* Clear screen */
        Term_clear();
    }

    /* Redraw mode line */
    if (tb->dirty_flags & DIRTY_MODE)
    {
        char buf[MAX_LINELEN];

        int sepa_length = tb->wid;

        /* Separator */
        for (i = 0; i < sepa_length; i++)
            buf[i] = '-';
        buf[i] = '\0';

        Term_putstr(0, tb->hgt + 1, sepa_length, TERM_L_BLUE, buf);
    }

    if (tb->dirty_flags & DIRTY_EXPRESSION)
    {
        int y;
        byte state = 0;

        for (y = 0; tb->lines_list[y]; y++)
        {
            char f;
            cptr v;
            cptr s = tb->lines_list[y];
            char *ss, *s_keep;
            int s_len;

            /* Update this line's state */
            tb->states[y] = state;

            if (*s++ != '?') continue;
            if (*s++ != ':') continue;

            /* Lines below this line are auto-registered */
            if (streq(s, "$AUTOREGISTER"))
                state |= LSTAT_AUTOREGISTER;

            s_len = strlen(s);
            ss = (char *)z_string_make(s);
            s_keep = ss;

            /* Parse the expr */
            v = process_pref_file_expr(&ss, &f);

            /* Set flag */
            if (streq(v, "0")) state |= LSTAT_BYPASS;
            else state &= ~LSTAT_BYPASS;

            /* Cannot use z_string_free() because the string was "destroyed" */
            C_FREE(s_keep, s_len + 1, char);

            /* Re-update this line's state */
            tb->states[y] = state | LSTAT_EXPRESSION;
        }

        tb->dirty_flags |= DIRTY_ALL;
    }

    if (tb->mark)
    {
        tb->dirty_flags |= DIRTY_ALL;

        by1 = MIN(tb->my, tb->cy);
        by2 = MAX(tb->my, tb->cy);
    }

    /* Dump up to tb->hgt lines of messages */
    for (i = 0; i < tb->hgt; i++)
    {
        int j;
        int leftcol = 0;
        cptr msg;
        byte color;
        int y = tb->upper+i;

        /* clean or dirty?                                              v--- Never syntax color the current line
                                                                             At least until we improve the parser! */
        if (!(tb->dirty_flags & DIRTY_ALL) && (tb->dirty_line != y) && (tb->cy != y))
            continue;

        msg = tb->lines_list[y];
        if (!msg) break;

        /* Apply horizontal scroll */
        for (j = 0; *msg; msg++, j++)
        {
            if (j == tb->left) break;
        }

        /* Erase line */
        Term_erase(0, i + 1, tb->wid);

        if (tb->states[y] & LSTAT_AUTOREGISTER)
        {
            /* Warning color -- These lines will be deleted later */
            color = TERM_L_RED;
        }
        else
        {
            if (msg[0] == '#') color = TERM_L_GREEN;
            else if (msg[0] == '?' && msg[1] == ':') color = TERM_VIOLET;
            else if (tb->states[y] & LSTAT_BYPASS) color = TERM_L_DARK;
            else color = TERM_WHITE;
        }

        /* No mark or Out of mark */
        if (!tb->mark || (y < by1 || by2 < y))
        {
            /* Dump the messages, bottom to top */
            /* This is a difficult hack, but syntax coloring is *very* useful! */
            /*                         v--- Only Color Lines for Rules
                                                          v--- Don't color while the user (possibly) edits */
            if (color == TERM_WHITE && strlen(msg) > 0 && y != tb->cy)
            {
                autopick_type entry = {0};
                string_ptr s = 0;

                autopick_new_entry(&entry, msg, FALSE);
                s = autopick_line_from_entry(&entry, AUTOPICK_COLOR_CODED);
                doc_rollback(doc, doc_pos_create(0, 0));
                doc_insert(doc, string_buffer(s));
                doc_sync_term(doc, doc_range_top_lines(doc, 1), doc_pos_create(leftcol, i+1));
                autopick_free_entry(&entry);
                string_free(s);
            }
            else
                Term_putstr(leftcol, i + 1, tb->wid - 1, color, msg);
        }

        /* Multiple lines selected */
        else if (by1 != by2)
        {
            /* Dump the messages, bottom to top */
            Term_putstr(leftcol, i + 1, tb->wid - 1, TERM_YELLOW, msg);
        }

        /* Single line selected */
        else
        {
            int x0 = leftcol + tb->left;
            int len = strlen(tb->lines_list[tb->cy]);
            int bx1 = MIN(tb->mx, tb->cx);
            int bx2 = MAX(tb->mx, tb->cx);

            /* Correct cursor location */
            if (bx2 > len) bx2 = len;

            Term_gotoxy(leftcol, i + 1);
            if (x0 < bx1) Term_addstr(bx1 - x0, color, msg);
            if (x0 < bx2) Term_addstr(bx2 - bx1, TERM_YELLOW, msg + (bx1 - x0));
            Term_addstr(-1, color, msg + (bx2 - x0));
        }
    }

    for (; i < tb->hgt; i++)
    {
        /* Erase line */
        Term_erase(0, i + 1, tb->wid);
    }

    /* Display information when updated */
    if (tb->old_cy != tb->cy || (tb->dirty_flags & (DIRTY_ALL | DIRTY_NOT_FOUND | DIRTY_NO_SEARCH)) || tb->dirty_line == tb->cy)
    {
        autopick_type an_entry = {0}, *entry = &an_entry;
        cptr str1 = NULL, str2 = NULL;


        /* Clear information line */
        for (i = 0; i < DESCRIPT_HGT; i++)
        {
            /* Erase line */
            Term_erase(0, tb->hgt + 2 + i, tb->wid);
        }

        /* Display information */
        if (tb->dirty_flags & DIRTY_NOT_FOUND)
        {
            str1 = format("Pattern not found: %s", tb->search_str);
        }
        else if (tb->dirty_flags & DIRTY_SKIP_INACTIVE)
        {
            str1 = format("Some inactive lines are skipped. (Searching %s)", tb->search_str);
        }
        else if (tb->dirty_flags & DIRTY_INACTIVE)
        {
            str1 = format("Found only an inactive line. (Searching %s)", tb->search_str);
        }
        else if (tb->dirty_flags & DIRTY_NO_SEARCH)
        {
            str1 = "No pattern to search. (Press ^S to search.)";
        }
        else if (tb->lines_list[tb->cy][0] == '#')
        {
            str1 = "This line is a comment.";
        }
        else if (tb->lines_list[tb->cy][0] && tb->lines_list[tb->cy][1] == ':')
        {
            switch(tb->lines_list[tb->cy][0])
            {
            case '?':
                str1 = "This line is a Conditional Expression.";

                break;
            case 'A':
                str1 = "This line defines a Macro action.";
                break;
            case 'P':
                str1 = "This line defines a Macro trigger key.";
                break;
            case 'C':
                str1 = "This line defines a Keymap.";
                break;
            }

            switch(tb->lines_list[tb->cy][0])
            {
            case '?':
                if (tb->states[tb->cy] & LSTAT_BYPASS)
                {
                    str2 = "The expression is 'False'(=0) currently.";
                }
                else
                {
                    str2 = "The expression is 'True'(=1) currently.";
                }
                break;

            default:
                if (tb->states[tb->cy] & LSTAT_AUTOREGISTER)
                {
                    str2 = "This line will be deleted later.";
                }

                else if (tb->states[tb->cy] & LSTAT_BYPASS)
                {
                    str2 = "This line is bypassed currently.";
                }
                break;
            }
        }

        /* Get description of an autopicker preference line */
        else if (autopick_new_entry(entry, tb->lines_list[tb->cy], FALSE))
        {
            char buf[MAX_LINELEN];
            char temp[MAX_LINELEN];
            cptr t;

            describe_autopick(buf, entry);

            if (tb->states[tb->cy] & LSTAT_AUTOREGISTER)
            {
                strcat(buf, "  This line will be deleted later.");
            }

            if (tb->states[tb->cy] & LSTAT_BYPASS)
            {
                strcat(buf, "  This line is bypassed currently.");
            }

            roff_to_buf(buf, 81, temp, sizeof(temp));
            t = temp;
            for (i = 0; i < 3; i++)
            {
                if(t[0] == 0)
                    break;
                else
                {
                    prt(t, tb->hgt +1 + 1 + i, 0);
                    t += strlen(t) + 1;
                }
            }
            autopick_free_entry(entry);
        }

        /* Draw the first line */
        if (str1) prt(str1, tb->hgt +1 + 1, 0);

        /* Draw the second line */
        if (str2) prt(str2, tb->hgt +1 + 2, 0);
    }
    doc_free(doc);
}


/*
 * Kill segment of a line
 */
static void kill_line_segment(text_body_type *tb, int y, int x0, int x1, bool whole)
{
    char buf[MAX_LINELEN];
    cptr s = tb->lines_list[y];
    char *d = buf;
    int x;

    /* Kill whole line? */
    if (whole && x0 == 0 && s[x1] == '\0' && tb->lines_list[y+1])
    {
        int i;

        z_string_free(tb->lines_list[y]);

        /* Shift lines up */
        for (i = y; tb->lines_list[i+1]; i++)
            tb->lines_list[i] = tb->lines_list[i+1];
        tb->lines_list[i] = NULL;

        /* Expressions need re-evaluation */
        tb->dirty_flags |= DIRTY_EXPRESSION;

        return;
    }

    /* No segment? */
    if (x0 == x1) return;

    /* Before the segment */
    for (x = 0; x < x0; x++)
        *(d++) = s[x];

    /* After the segment */
    for (x = x1; s[x]; x++)
        *(d++) = s[x];

    *d = '\0';

    /* Replace */
    z_string_free(tb->lines_list[y]);
    tb->lines_list[y] = z_string_make(buf);

    /* Expressions may need re-evaluation */
    check_expression_line(tb, y);

    /* Text is changed */
    tb->changed = TRUE;
}


/*
 * Get a trigger key and insert ASCII string for the trigger
 */
static bool insert_macro_line(text_body_type *tb)
{
    char tmp[1024];
    char buf[1024];
    int i, n = 0;

    /* Flush */
    flush();

    /* Do not process macros */
    inkey_base = TRUE;

    /* First key */
    i = inkey();

    /* Read the pattern */
    while (i)
    {
        /* Save the key */
        buf[n++] = i;

        /* Do not process macros */
        inkey_base = TRUE;

        /* Do not wait for keys */
        inkey_scan = TRUE;

        /* Attempt to read a key */
        i = inkey();
    }

    /* Terminate */
    buf[n] = '\0';

    /* Flush */
    flush();

    /* Convert the trigger */
    ascii_to_text(tmp, buf);

    /* Null */
    if(!tmp[0]) return FALSE;

    tb->cx = 0;

    /* Insert preference string */
    insert_return_code(tb);
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(format("P:%s", tmp));

    /* Acquire action */
    i = macro_find_exact(buf);

    if (i == -1)
    {
        /* Nothing defined */
        tmp[0] = '\0';
    }
    else
    {
        /* Analyze the current action */
        ascii_to_text(tmp, macro__act[i]);
    }

    /* Insert blank action preference line */
    insert_return_code(tb);
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(format("A:%s", tmp));

    return TRUE;
}


/*
 * Get a command key and insert ASCII string for the key
 */
static bool insert_keymap_line(text_body_type *tb)
{
    char tmp[1024];
    char buf[2];
    int mode;
    cptr act;

    /* Roguelike */
    if (rogue_like_commands)
    {
        mode = KEYMAP_MODE_ROGUE;
    }

    /* Original */
    else
    {
        mode = KEYMAP_MODE_ORIG;
    }

    /* Flush */
    flush();

    /* Get a key */
    buf[0] = inkey();
    buf[1] = '\0';

    /* Flush */
    flush();

    /* Convert the trigger */
    ascii_to_text(tmp, buf);

    /* Null */
    if(!tmp[0]) return FALSE;

    tb->cx = 0;

    /* Insert preference string */
    insert_return_code(tb);
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(format("C:%d:%s", mode, tmp));

    /* Look up the keymap */
    act = keymap_act[mode][(byte)(buf[0])];

    if (act)
    {
        /* Analyze the current action */
        ascii_to_text(tmp, act);
    }
    else
    {
        /* No keymap defined -- Use trigger key itself as a default */

        /* Nothing to do (use tmp) */
    }

    /* Insert blank action preference line */
    insert_return_code(tb);
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(format("A:%s", tmp));

    return TRUE;
}


/*
 * Execute a single editor command
 */
static bool do_editor_command(text_body_type *tb, int com_id)
{
    switch(com_id)
    {
    case EC_QUIT:
        if (tb->changed)
        {
            if (!get_check("Discard all changes and quit. Are you sure? ")) break;
        }
        return QUIT_WITHOUT_SAVE;

    case EC_SAVEQUIT:
        return QUIT_AND_SAVE;

    case EC_REVERT:
        /* Revert to original */
        if (!get_check("Discard all changes and revert to original file. Are you sure? ")) break;

        free_text_lines(tb->lines_list);
        tb->lines_list = read_pickpref_text_lines(&tb->filename_mode, NULL);
        tb->dirty_flags |= DIRTY_ALL | DIRTY_MODE | DIRTY_EXPRESSION;
        tb->cx = tb->cy = 0;
        tb->mark = 0;

        /* Text is not changed */
        tb->changed = FALSE;
        break;

    case EC_LOAD:
        {
            char lataa_minut[80];
            int pituus;
            strcpy(lataa_minut, player_base);
            if (!get_string("Load: ", lataa_minut, 78)) break;
            pituus = strlen(lataa_minut);

            /* Identify format
             * Acceptable formats include [charname], [charname.prf],
             * [pickpref-charname] and [pickpref-charname.prf] */
            if (pituus > 9)
            {
                if (clip_and_locate("pickpref-", lataa_minut)) pituus -= 9;
            }
            if (pituus > 4 && strcmp(lataa_minut + pituus - 4, ".prf") == 0) lataa_minut[pituus - 4] = '\0';

            free_text_lines(tb->lines_list);
            tb->filename_mode = PT_WITH_OTHERNAME;
            tb->lines_list = read_pickpref_text_lines(&tb->filename_mode, lataa_minut);
            tb->dirty_flags |= DIRTY_ALL | DIRTY_MODE | DIRTY_EXPRESSION;
            tb->cx = tb->cy = 0;
            tb->mark = 0;
            tb->changed = TRUE;
            tb->filename_mode = PT_WITH_PNAME;
            break;
        }

    case EC_HELP:
        /* Peruse the main help file */
        doc_display_help("editor.txt", NULL);
        /* Redraw all */
        tb->dirty_flags |= DIRTY_SCREEN;

        break;

    case EC_RETURN:
        /* Split a line or insert end of line */

        /* Ignore selection */
        if (tb->mark)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }

        insert_return_code(tb);
        tb->cy++;
        tb->cx = 0;

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;
        break;

    case EC_LEFT:
        /* Back */
        if (0 < tb->cx)
        {
            int len;

            tb->cx--;
            len = strlen(tb->lines_list[tb->cy]);
            if (len < tb->cx) tb->cx = len;

        }
        else if (tb->cy > 0)
        {
            tb->cy--;
            tb->cx = strlen(tb->lines_list[tb->cy]);
        }
        break;

    case EC_DOWN:
        /* Next line */

        /* Is this the last line? */
        if (!tb->lines_list[tb->cy + 1])
        {
            /* Add one more empty line if possible */
            if (!add_empty_line(tb)) break;
        }

        /* Go down */
        tb->dirty_line = tb->cy; /* Hack for Syntax Coloring: Redraw old current line in case it was edited */
        tb->cy++;

        break;

    case EC_UP:
        /* Previous line */
        if (tb->cy > 0)
        {
            tb->dirty_line = tb->cy; /* Hack for Syntax Coloring: Redraw old current line in case it was edited */
            tb->cy--;
        }
        break;

    case EC_RIGHT:
    {
        /* Forward */

        int len;
        tb->cx++;
        len = strlen(tb->lines_list[tb->cy]);
        if (len < tb->cx)
        {
            /* Correct the cursor position */
            tb->cx = len;

            /* Is this the last line? */
            if (!tb->lines_list[tb->cy + 1])
            {
                /* Add one more empty line if possible */
                if (!add_empty_line(tb)) break;
            }

            /* Move to the beginning of next line */
            tb->dirty_line = tb->cy; /* Hack for Syntax Coloring: Redraw old current line in case it was edited */
            tb->cy++;
            tb->cx = 0;
        }
        break;
    }

    case EC_BOL:
        /* Beginning of line */
        tb->cx = 0;
        break;

    case EC_EOL:
        /* End of line */
        tb->cx = strlen(tb->lines_list[tb->cy]);
        break;

    case EC_PGUP:
        while (0 < tb->cy && tb->upper <= tb->cy)
            tb->cy--;
        while (0 < tb->upper && tb->cy + 1 < tb->upper + tb->hgt)
            tb->upper--;
        break;

    case EC_PGDOWN:
        /* Page down */
        while (tb->cy < tb->upper + tb->hgt)
        {
            /* Is this the last line? */
            if (!tb->lines_list[tb->cy + 1])
            {
                /* Add one more empty line if possible */
                if (!add_empty_line(tb)) break;
            }

            tb->cy++;
        }

        tb->upper = tb->cy;
        break;

    case EC_TOP:
        tb->cy = 0;
        break;

    case EC_BOTTOM:
        while (TRUE)
        {
            /* Is this the last line? */
            if (!tb->lines_list[tb->cy + 1])
            {
                /* Add one more empty line if possible */
                if (!add_empty_line(tb)) break;
            }

            tb->cy++;
        }

        /* Always at the biginning of the last line */
        tb->cx = 0;

        break;

    case EC_CUT:
    {
        /* Copy the text first */
        copy_text_to_yank(tb);

        /* Single line case */
        if (tb->my == tb->cy)
        {
            int bx1 = MIN(tb->mx, tb->cx);
            int bx2 = MAX(tb->mx, tb->cx);
            int len = strlen(tb->lines_list[tb->cy]);

            /* Correct fake cursor position */
            if (bx2 > len) bx2 = len;

            kill_line_segment(tb, tb->cy, bx1, bx2, TRUE);

            /* New cursor position */
            tb->cx = bx1;
        }

        /* Multiple lines case */
        else /* if (tb->my != tb->cy) */
        {
            int y;

            int by1 = MIN(tb->my, tb->cy);
            int by2 = MAX(tb->my, tb->cy);

            /* Kill lines in reverse order */
            for (y = by2; y >= by1; y--)
            {
                int len = strlen(tb->lines_list[y]);

                kill_line_segment(tb, y, 0, len, TRUE);
            }

            /* New cursor position */
            tb->cy = by1;
            tb->cx = 0;
        }


        /* Disable selection */
        tb->mark = 0;

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Text is changed */
        tb->changed = TRUE;

        break;
    }

    case EC_COPY:
        copy_text_to_yank(tb);

        /*
         * Move cursor position to the end of the selection
         *
         * Pressing ^C ^V correctly duplicates the selection.
         */
        if (tb->my == tb->cy)
        {
            tb->cx = MAX(tb->cx, tb->mx);

            /*
             * When whole line is selected, the end of
             * line code is also copyed.
             */
            if (!tb->lines_list[tb->cy][tb->cx])
            {
                /* Is this the last line? */
                if (!tb->lines_list[tb->cy + 1])
                {
                    /* Add one more empty line if possible */
                    if (!add_empty_line(tb)) break;
                }

                /* Go to the beginning of next line */
                tb->cy++;
                tb->cx = 0;
            }
        }
        else
        {
            tb->cy = MAX(tb->cy, tb->my);

            /* Is this the last line? */
            if (!tb->lines_list[tb->cy + 1])
            {
                /* Add one more empty line if possible */
                if (!add_empty_line(tb)) break;
            }

            /* Go down */
            tb->cy++;
        }

        break;

    case EC_PASTE:
    {
        /* Paste killed text */

        chain_str_type *chain = tb->yank;
        int len = strlen(tb->lines_list[tb->cy]);

        /* Nothing to do? */
        if (!chain) break;

        /* Correct cursor location */
        if (tb->cx > len) tb->cx = len;

        /* Ignore selection */
        if (tb->mark)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }

        /* Paste text */
        while (chain)
        {
            cptr yank_str = chain->s;

            char buf[MAX_LINELEN];
            int i;
            char rest[MAX_LINELEN], *rest_ptr = rest;

            /* Save preceding string */
            for(i = 0; i < tb->cx; i++)
                buf[i] = tb->lines_list[tb->cy][i];

            strcpy(rest, &(tb->lines_list[tb->cy][i]));

            /* Paste yank buffer */
            while (*yank_str && i < MAX_LINELEN-1)
            {
                buf[i++] = *yank_str++;
            }

            /* Terminate */
            buf[i] = '\0';

            chain = chain->next;

            if (chain || tb->yank_eol)
            {
                /* There is an end of line between chain nodes */

                insert_return_code(tb);

                /* Replace this line with new one */
                z_string_free(tb->lines_list[tb->cy]);
                tb->lines_list[tb->cy] = z_string_make(buf);

                /* Move to next line */
                tb->cx = 0;
                tb->cy++;

                continue;
            }

            /* Final line doesn't have end of line */

            tb->cx = strlen(buf);

            /* Rest of original line */
            while (*rest_ptr && i < MAX_LINELEN-1)
            {
                buf[i++] = *rest_ptr++;
            }

            /* Terminate */
            buf[i] = '\0';

            /* Replace this line with new one */
            z_string_free(tb->lines_list[tb->cy]);
            tb->lines_list[tb->cy] = z_string_make(buf);

            /* Finish */
            break;
        }

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Expressions need re-evaluation */
        tb->dirty_flags |= DIRTY_EXPRESSION;

        /* Text is changed */
        tb->changed = TRUE;

        break;
    }

    case EC_BLOCK:
        if (tb->mark)
        {
            /* Disable the selection */
            tb->mark = 0;

            /* Redraw text */
            tb->dirty_flags |= DIRTY_ALL;
        }
        else
        {
            tb->mark = MARK_MARK;

            /* Repeating this command swaps cursor position */
            if (com_id == tb->old_com_id)
            {
                int tmp;

                tmp = tb->cy;
                tb->cy = tb->my;
                tb->my = tmp;
                tmp = tb->cx;
                tb->cx = tb->mx;
                tb->mx = tmp;

                /* Redraw text */
                tb->dirty_flags |= DIRTY_ALL;
            }
            else
            {
                int len = strlen(tb->lines_list[tb->cy]);

                /* Mark the point 1 */
                tb->my = tb->cy;
                tb->mx = tb->cx;

                /* Correct cursor location */
                if (tb->cx > len) tb->mx = len;
            }
        }
        break;

    case EC_KILL_LINE:
    {
        /* Kill rest of line */

        int len = strlen(tb->lines_list[tb->cy]);

        /* Correct cursor location */
        if (tb->cx > len) tb->cx = len;

        /* Ignore selection */
        if (tb->mark)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }

        /* Append only if this command is repeated. */
        if (tb->old_com_id != com_id)
        {
            kill_yank_chain(tb);
            tb->yank = NULL;
        }

        /* Really deleted some text */
        if (tb->cx < len)
        {
            /* Add deleted string to yank buffer */
            add_str_to_yank(tb, &(tb->lines_list[tb->cy][tb->cx]));

            kill_line_segment(tb, tb->cy, tb->cx, len, FALSE);

            /* Now dirty */
            tb->dirty_line = tb->cy;

            /* Leave end of line character */
            break;
        }

        /* Cut the end of line character only */
        if (tb->yank_eol) add_str_to_yank(tb, "");

        /* Cut end of line */
        tb->yank_eol = TRUE;

        do_editor_command(tb, EC_DELETE_CHAR);
        break;
    }

    case EC_DELETE_CHAR:
    {
        /* DELETE == go forward + BACK SPACE */

        int len;

        /* Ignore selection */
        if (tb->mark)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }

        tb->cx++;

        /* Pass through the end of line to next line */
        len = strlen(tb->lines_list[tb->cy]);
        if (len < tb->cx)
        {
            if (tb->lines_list[tb->cy + 1])
            {
                tb->cy++;
                tb->cx = 0;
            }
            else
            {
                tb->cx = len;
                break;
            }
        }

        do_editor_command(tb, EC_BACKSPACE);
        break;
    }

    case EC_BACKSPACE:
    {
        /* BACK SPACE */

        int len, i, j, k;
        char buf[MAX_LINELEN];

        /* Ignore selection */
        if (tb->mark)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }

        /* Move to correct collumn */
        len = strlen(tb->lines_list[tb->cy]);
        if (len < tb->cx) tb->cx = len;

        if (tb->cx == 0)
        {
            /* delete a return code and union two lines */
            if (tb->cy == 0) break;
            tb->cx = strlen(tb->lines_list[tb->cy-1]);
            strcpy(buf, tb->lines_list[tb->cy-1]);
            strcat(buf, tb->lines_list[tb->cy]);
            z_string_free(tb->lines_list[tb->cy-1]);
            z_string_free(tb->lines_list[tb->cy]);
            tb->lines_list[tb->cy-1] = z_string_make(buf);

            for (i = tb->cy; tb->lines_list[i+1]; i++)
                tb->lines_list[i] = tb->lines_list[i+1];

            tb->lines_list[i] = NULL;
            tb->cy--;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;

            /* Expressions need re-evaluation */
            tb->dirty_flags |= DIRTY_EXPRESSION;

            /* Text is changed */
            tb->changed = TRUE;

            break;
        }

        for (i = j = k = 0; tb->lines_list[tb->cy][i] && i < tb->cx; i++)
        {
            k = j;
            buf[j++] = tb->lines_list[tb->cy][i];
        }
        while (j > k)
        {
            tb->cx--;
            j--;
        }
        for (; tb->lines_list[tb->cy][i]; i++)
            buf[j++] = tb->lines_list[tb->cy][i];
        buf[j] = '\0';
        z_string_free(tb->lines_list[tb->cy]);
        tb->lines_list[tb->cy] = z_string_make(buf);

        /* Now dirty */
        tb->dirty_line = tb->cy;

        /* Expressions may need re-evaluation */
        check_expression_line(tb, tb->cy);

        /* Text is changed */
        tb->changed = TRUE;

        break;
    }

    case EC_SEARCH_STR:
    {
        byte search_dir;

        /* Become dirty because of item/equip menu */
        tb->dirty_flags |= DIRTY_SCREEN;

        search_dir = get_string_for_search(&tb->search_o_ptr, &tb->search_str);

        if (!search_dir) break;

        if (search_dir == 1) do_editor_command(tb, EC_SEARCH_FORW);
        else do_editor_command(tb, EC_SEARCH_BACK);
        break;
    }

    case EC_SEARCH_FORW:
        if (tb->search_o_ptr)
        {
            search_for_object(tb, tb->search_o_ptr, TRUE);
        }
        else if (tb->search_str && tb->search_str[0])
        {
            search_for_string(tb, tb->search_str, TRUE);
        }
        else
        {
            tb->dirty_flags |= DIRTY_NO_SEARCH;
        }
        break;

    case EC_SEARCH_BACK:
        if (tb->search_o_ptr)
        {
            search_for_object(tb, tb->search_o_ptr, FALSE);
        }
        else if (tb->search_str && tb->search_str[0])
        {
            search_for_string(tb, tb->search_str, FALSE);
        }
        else
        {
            tb->dirty_flags |= DIRTY_NO_SEARCH;
        }
        break;

    case EC_SEARCH_OBJ:
        /* Become dirty because of item/equip menu */
        tb->dirty_flags |= DIRTY_SCREEN;

        if (!get_object_for_search(&tb->search_o_ptr, &tb->search_str)) break;

        do_editor_command(tb, EC_SEARCH_FORW);
        break;

    case EC_SEARCH_DESTROYED:
        if (!get_destroyed_object_for_search(&tb->search_o_ptr, &tb->search_str))
        {
            /* There is no object to search */
            tb->dirty_flags |= DIRTY_NO_SEARCH;

            break;
        }

        do_editor_command(tb, EC_SEARCH_FORW);
        break;

    case EC_INSERT_OBJECT:
    {
        /* Insert choosen item name */

        autopick_type an_entry, *entry = &an_entry;

        if (!entry_from_choosed_object(entry))
        {
            /* Now dirty because of item/equip menu */
            tb->dirty_flags |= DIRTY_SCREEN;
            break;
        }

        tb->cx = 0;
        insert_return_code(tb);
        z_string_free(tb->lines_list[tb->cy]);
        tb->lines_list[tb->cy] = autopick_line_from_entry_kill(entry);

        /* Now dirty because of item/equip menu */
        tb->dirty_flags |= DIRTY_SCREEN;

        break;
    }

    case EC_INSERT_DESTROYED:
        /* Insert a name of last destroyed item */
        if (tb->last_destroyed)
        {
            tb->cx = 0;
            insert_return_code(tb);
            z_string_free(tb->lines_list[tb->cy]);
            tb->lines_list[tb->cy] = z_string_make(tb->last_destroyed);

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;

            /* Text is changed */
            tb->changed = TRUE;
        }
        break;

    case EC_INSERT_BLOCK:
    {
        /* Insert a conditinal expression line */
        char expression[80];
        race_t *race_ptr = get_race();
        class_t *class_ptr = get_class();

        /* Conditional Expression for Class and Race */
        sprintf(expression, "?:[AND [EQU $RACE %s] [EQU $CLASS %s] [GEQ $LEVEL %02d]]",
            race_ptr->name, class_ptr->name, p_ptr->lev);

        tb->cx = 0;
        insert_return_code(tb);
        z_string_free(tb->lines_list[tb->cy]);
        tb->lines_list[tb->cy] = z_string_make(expression);
        tb->cy++;
        insert_return_code(tb);
        z_string_free(tb->lines_list[tb->cy]);
        tb->lines_list[tb->cy] = z_string_make("?:1");

        /* Now dirty */
        tb->dirty_flags |= DIRTY_ALL;

        /* Text is changed */
        tb->changed = TRUE;

        break;
    }

    case EC_INSERT_MACRO:
        /* Draw_everythig (delete menu) */
        draw_text_editor(tb);

        /* Erase line */
        Term_erase(0, tb->cy - tb->upper + 1, tb->wid);

        /* Prompt */
        Term_putstr(0, tb->cy - tb->upper + 1, tb->wid - 1, TERM_YELLOW, "P:<Trigger key>: ");
        if (insert_macro_line(tb))
        {
            /* Prepare to input action */
            tb->cx = 2;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;

            /* Text is changed */
            tb->changed = TRUE;
        }

        break;

    case EC_INSERT_KEYMAP:
        /* Draw_everythig (delete menu) */
        draw_text_editor(tb);

        /* Erase line */
        Term_erase(0, tb->cy - tb->upper + 1, tb->wid);

        /* Prompt */
        Term_putstr(0, tb->cy - tb->upper + 1, tb->wid - 1, TERM_YELLOW, format("C:%d:<Keypress>: ", (rogue_like_commands ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG)));

        if (insert_keymap_line(tb))
        {
            /* Prepare to input action */
            tb->cx = 2;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;

            /* Text is changed */
            tb->changed = TRUE;
        }
        break;

    case EC_CL_AUTOPICK: toggle_command_letter(tb, DO_AUTOPICK); break;
    case EC_CL_DESTROY: toggle_command_letter(tb, DO_AUTODESTROY); break;
    case EC_CL_LEAVE: toggle_command_letter(tb, DONT_AUTOPICK); break;
    case EC_CL_QUERY: toggle_command_letter(tb, DO_QUERY_AUTOPICK); break;
    case EC_CL_NO_DISP: toggle_command_letter(tb, DO_DISPLAY); break;
    case EC_CL_AUTO_ID: toggle_command_letter(tb, DO_AUTO_ID); break;

    case EC_IK_UNAWARE: toggle_keyword(tb, FLG_UNAWARE); break;
    case EC_IK_UNSENSED: toggle_keyword(tb, FLG_UNSENSED); break;
    case EC_IK_UNIDENTIFIED: toggle_keyword(tb, FLG_UNIDENTIFIED); break;
    case EC_IK_IDENTIFIED: toggle_keyword(tb, FLG_IDENTIFIED); break;
    case EC_IK_STAR_IDENTIFIED: toggle_keyword(tb, FLG_STAR_IDENTIFIED); break;
    case EC_KK_WEAPONS: toggle_keyword(tb, FLG_WEAPONS); break;
    case EC_KK_SHOOTERS: toggle_keyword(tb, FLG_SHOOTERS); break;
    case EC_KK_AMMO: toggle_keyword(tb, FLG_AMMO); break;
    case EC_KK_FAVORITE_WEAPONS: toggle_keyword(tb, FLG_FAVORITE_WEAPONS); break;
    case EC_KK_DIGGERS: toggle_keyword(tb, FLG_DIGGERS); break;
    case EC_KK_ARMORS: toggle_keyword(tb, FLG_ARMORS); break;
    case EC_KK_WANDS: toggle_keyword(tb, FLG_WANDS); break;
    case EC_KK_STAVES: toggle_keyword(tb, FLG_STAVES); break;
    case EC_KK_RODS: toggle_keyword(tb, FLG_RODS); break;
    case EC_KK_POTIONS: toggle_keyword(tb, FLG_POTIONS); break;
    case EC_KK_SCROLLS: toggle_keyword(tb, FLG_SCROLLS); break;
    case EC_KK_LIGHTS: toggle_keyword(tb, FLG_LIGHTS); break;
    case EC_KK_JUNKS: toggle_keyword(tb, FLG_JUNKS); break;
    case EC_KK_CORPSES: toggle_keyword(tb, FLG_CORPSES); break;
    case EC_KK_SKELETONS: toggle_keyword(tb, FLG_SKELETONS); break;
    case EC_KK_SPELLBOOKS: toggle_keyword(tb, FLG_SPELLBOOKS); break;
    case EC_KK_SHIELDS: toggle_keyword(tb, FLG_SHIELDS); break;
    case EC_KK_RINGS: toggle_keyword(tb, FLG_RINGS); break;
    case EC_KK_AMULETS: toggle_keyword(tb, FLG_AMULETS); break;
    case EC_KK_SUITS: toggle_keyword(tb, FLG_SUITS); break;
    case EC_KK_CLOAKS: toggle_keyword(tb, FLG_CLOAKS); break;
    case EC_KK_HELMS: toggle_keyword(tb, FLG_HELMS); break;
    case EC_KK_GLOVES: toggle_keyword(tb, FLG_GLOVES); break;
    case EC_KK_BOOTS: toggle_keyword(tb, FLG_BOOTS); break;
    case EC_OK_COLLECTING: toggle_keyword(tb, FLG_COLLECTING); break;
    case EC_OK_BOOSTED: toggle_keyword(tb, FLG_BOOSTED); break;
    case EC_OK_ICKY: toggle_keyword(tb, FLG_ICKY); break;
    case EC_OK_MORE_DICE: toggle_keyword(tb, FLG_MORE_DICE); break;
    case EC_OK_MORE_BONUS: toggle_keyword(tb, FLG_MORE_BONUS); break;
    case EC_OK_MORE_LEVEL: toggle_keyword(tb, FLG_MORE_LEVEL); break;
    case EC_OK_MORE_WEIGHT: toggle_keyword(tb, FLG_MORE_WEIGHT); break;
    case EC_OK_MORE_CHARGES: toggle_keyword(tb, FLG_MORE_CHARGES); break;
    case EC_OK_MORE_VALUE: toggle_keyword(tb, FLG_MORE_VALUE); break;
    case EC_OK_WORTHLESS: toggle_keyword(tb, FLG_WORTHLESS); break;
    case EC_OK_ARTIFACT: toggle_keyword(tb, FLG_ARTIFACT); break;
    case EC_OK_EGO: toggle_keyword(tb, FLG_EGO); break;
    case EC_OK_GOOD: toggle_keyword(tb, FLG_GOOD); break;
    case EC_OK_CURSED: toggle_keyword(tb, FLG_CURSED); break;
    case EC_OK_SPECIAL: toggle_keyword(tb, FLG_SPECIAL); break;
    case EC_OK_UNUSABLE: toggle_keyword(tb, FLG_UNUSABLE); break;
    case EC_OK_NAMELESS: toggle_keyword(tb, FLG_NAMELESS); break;
    case EC_OK_AVERAGE: toggle_keyword(tb, FLG_AVERAGE); break;
    case EC_OK_RARE: toggle_keyword(tb, FLG_RARE); break;
    case EC_OK_COMMON: toggle_keyword(tb, FLG_COMMON); break;
    case EC_OK_WANTED: toggle_keyword(tb, FLG_WANTED); break;
    case EC_OK_UNIQUE: toggle_keyword(tb, FLG_UNIQUE); break;
    case EC_OK_HUMAN: toggle_keyword(tb, FLG_HUMAN); break;
    case EC_OK_UNREADABLE:
        toggle_keyword(tb, FLG_UNREADABLE);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_REALM1:
        toggle_keyword(tb, FLG_REALM1);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_REALM2:
        toggle_keyword(tb, FLG_REALM2);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_FIRST:
        toggle_keyword(tb, FLG_FIRST);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_SECOND:
        toggle_keyword(tb, FLG_SECOND);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_THIRD:
        toggle_keyword(tb, FLG_THIRD);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    case EC_OK_FOURTH:
        toggle_keyword(tb, FLG_FOURTH);
        add_keyword(tb, FLG_SPELLBOOKS);
        break;
    }

    /* Save old command */
    tb->old_com_id = com_id;

    return FALSE;
}


/*
 * Insert single letter at cursor position.
 */
static void insert_single_letter(text_body_type *tb, int key)
{
    int i, j, len;
    char buf[MAX_LINELEN];

    /* Save preceding string */
    for (i = j = 0; tb->lines_list[tb->cy][i] && i < tb->cx; i++)
        buf[j++] = tb->lines_list[tb->cy][i];

    /* Add a character */
    if (j+1 < MAX_LINELEN)
        buf[j++] = key;
    tb->cx++;

    /* Add following */
    for (; tb->lines_list[tb->cy][i] && j + 1 < MAX_LINELEN; i++)
        buf[j++] = tb->lines_list[tb->cy][i];
    buf[j] = '\0';

    /* Replace current line with new line */
    z_string_free(tb->lines_list[tb->cy]);
    tb->lines_list[tb->cy] = z_string_make(buf);

    /* Move to correct column */
    len = strlen(tb->lines_list[tb->cy]);
    if (len < tb->cx) tb->cx = len;

    /* Now dirty */
    tb->dirty_line = tb->cy;

    /* Expressions may need re-evaluation */
    check_expression_line(tb, tb->cy);

    /* Text is changed */
    tb->changed = TRUE;
}


/*
 * Check special key code and get a movement command id
 */
static int analyze_move_key(text_body_type *tb, int skey)
{
    int com_id;

    /* Not a special key */
    if (!(skey & SKEY_MASK)) return 0;

    /* Convert from a special key code to an editor command */
    switch(skey & ~SKEY_MOD_MASK)
    {
    case SKEY_DOWN:   com_id = EC_DOWN;   break;
    case SKEY_LEFT:   com_id = EC_LEFT;   break;
    case SKEY_RIGHT:  com_id = EC_RIGHT;  break;
    case SKEY_UP:     com_id = EC_UP;     break;
    case SKEY_PGUP:   com_id = EC_PGUP;   break;
    case SKEY_PGDOWN: com_id = EC_PGDOWN; break;
    case SKEY_TOP:    com_id = EC_TOP;    break;
    case SKEY_BOTTOM: com_id = EC_BOTTOM; break;

    default:
        /* Not a special movement key */
        return 0;
    }

    /* Without shift modifier */
    if (!(skey & SKEY_MOD_SHIFT))
    {
        /*
         * Un-shifted cursor keys cancells
         * selection created by shift+cursor.
         */
        if (tb->mark & MARK_BY_SHIFT)
        {
            tb->mark = 0;

            /* Now dirty */
            tb->dirty_flags |= DIRTY_ALL;
        }
    }

    /* With shift modifier */
    else
    {
        /* Start selection by shift + cursor keys */
        if (!tb->mark)
        {
            int len = strlen(tb->lines_list[tb->cy]);

            tb->mark = MARK_MARK | MARK_BY_SHIFT;
            tb->my = tb->cy;
            tb->mx = tb->cx;

            /* Correct cursor location */
            if (tb->cx > len) tb->mx = len;

            /* Need to redraw text */
            if (com_id == EC_UP || com_id == EC_DOWN)
            {
                /* Redraw all text */
                tb->dirty_flags |= DIRTY_ALL;
            }
            else
            {
                tb->dirty_line = tb->cy;
            }
        }
    }

    return com_id;
}

/*
 * In-game editor of Object Auto-picker/Destoryer
 */
void do_cmd_edit_autopick(void)
{
    static int cx_save = 0;
    static int cy_save = 0;

    text_body_type text_body, *tb = &text_body;

    autopick_type an_entry, *entry = &an_entry;
    char buf[MAX_LINELEN];

    int i;
    int key = -1;

    static s32b old_autosave_turn = 0L;
    byte quit = 0;

    tb->changed = FALSE;
    tb->cx = cx_save;
    tb->cy = cy_save;
    tb->upper = tb->left = 0;
    tb->mark = 0;
    tb->mx = tb->my = 0;
    tb->old_cy = tb->old_upper = tb->old_left = -1;
    tb->old_wid = tb->old_hgt = -1;
    tb->old_com_id = 0;

    tb->yank = NULL;
    tb->search_o_ptr = NULL;
    tb->search_str = NULL;
    tb->last_destroyed = NULL;
    tb->dirty_flags = DIRTY_ALL | DIRTY_MODE | DIRTY_EXPRESSION;
    tb->dirty_line = -1;
    tb->filename_mode = PT_WITH_PNAME;

    if (game_turn < old_autosave_turn)
    {
        while (old_autosave_turn > game_turn) old_autosave_turn -= TURNS_PER_TICK * TOWN_DAWN;
    }

    /* Autosave */
    if (game_turn > old_autosave_turn + 100L)
    {
        do_cmd_save_game(TRUE);
        old_autosave_turn = game_turn;
    }

    /* HACK -- Reset start_time to stop counting playtime while edit */
    update_playtime();

    /* Free old entries */
    init_autopick();

    /* Command Description of the 'Last Destroyed Item' */
    if (autopick_last_destroyed_object.k_idx)
    {
        autopick_entry_from_object(entry, &autopick_last_destroyed_object);
        tb->last_destroyed = autopick_line_from_entry_kill(entry);
    }

    /* Read or initialize whole text */
    tb->lines_list = read_pickpref_text_lines(&tb->filename_mode, NULL);

    /* Reset cursor position if needed */
    for (i = 0; i < tb->cy; i++)
    {
        if (!tb->lines_list[i])
        {
            tb->cy = tb->cx = 0;
            break;
        }
    }

    /* Save the screen */
    Term_clear();
    /*screen_save(); is currently very broken ... */

    /* Process requests until done */
    while (!quit)
    {
        int com_id = 0;

        /* Draw_everything */
        draw_text_editor(tb);

        /* Display header line */
        c_prt(TERM_L_BLUE, "(^Q:Quit, ^W:Save&Quit, ESC:Menu, Other:Input text)", 0, 0);
        if (tb->changed)
            c_prt(TERM_YELLOW, "*", 0, 58);

        if (!tb->mark)
        {
            /* Display current position */
            prt(format("(%d,%d)", tb->cx, tb->cy), 0, 60);
        }
        else
        {
            /* Display current position and mark position */
            prt(format("(%d,%d)-(%d,%d)", tb->mx, tb->my, tb->cx, tb->cy), 0, 60);
        }

        /* Place cursor */
        Term_gotoxy(tb->cx - tb->left, tb->cy - tb->upper + 1);

        /* Now clean */
        tb->dirty_flags = 0;
        tb->dirty_line = -1;

        /* Save old key and location */
        tb->old_cy = tb->cy;
        tb->old_upper = tb->upper;
        tb->old_left = tb->left;
        tb->old_wid = tb->wid;
        tb->old_hgt = tb->hgt;

        if (numpad_as_cursorkey) autopick_inkey_hack = 1;

        /* Get a command */
        key = inkey_special(TRUE);

        /* Special keys */
        if (key & SKEY_MASK)
        {
            /* Get a movement command */
            com_id = analyze_move_key(tb, key);
        }

        /* Open the menu */
        else if (key == ESCAPE)
        {
            com_id = do_command_menu(0, 0);

            /* Redraw all text later */
            tb->dirty_flags |= DIRTY_SCREEN;
        }

        /* Insert a character */
        else if (!iscntrl((unsigned char)key))
        {
            /* Ignore selection */
            if (tb->mark)
            {
                tb->mark = 0;

                /* Now dirty */
                tb->dirty_flags |= DIRTY_ALL;
            }

            insert_single_letter(tb, key);

            /* Next loop */
            continue;
        }

        /* Other commands */
        else
        {
            com_id = get_com_id(key);
        }

        if (com_id) quit = do_editor_command(tb, com_id);
    } /* while (TRUE) */

    /* Restore the screen
    screen_load(); is currently very broken.*/
    Term_clear();
    do_cmd_redraw();

    /* Get the filename of preference */
    strcpy(buf, pickpref_filename(tb->filename_mode, NULL));

    if (quit == QUIT_AND_SAVE)
        write_text_lines(buf, tb->lines_list);

    free_text_lines(tb->lines_list);

    z_string_free(tb->search_str);
    z_string_free(tb->last_destroyed);

    /* Destroy string chain */
    kill_yank_chain(tb);

    /* Reload autopick pref */
    process_autopick_file(buf);

    /* HACK -- reset start_time so that playtime is not increase while edit */
    start_time = time(NULL);

    /* Save cursor location */
    cx_save = tb->cx;
    cy_save = tb->cy;
}
