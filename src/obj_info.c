#include "angband.h"

#include <stdlib.h>
#include <assert.h>

/* Display detailed object information to the user */
extern void obj_display(object_type *o_ptr);
extern void obj_display_rect(object_type *o_ptr, rect_t display);
extern void obj_display_doc(object_type *o_ptr, doc_ptr doc);
extern void obj_display_smith(object_type *o_ptr, doc_ptr doc);
extern void device_display_doc(object_type *o_ptr, doc_ptr doc);
extern void device_display_smith(object_type *o_ptr, doc_ptr doc);
extern bool display_origin(object_type *o_ptr, doc_ptr doc);

static void _display_name(object_type *o_ptr, doc_ptr doc);
static void _display_desc(object_type *o_ptr, doc_ptr doc);
static void _display_stats(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_sustains(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_other_pval(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_brands(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_slays(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_resists(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_abilities(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_auras(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_extra(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_insurance(object_type *o_ptr, doc_ptr doc);
static void _display_curses(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_activation(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_activation_aux(effect_t *effect, bool full_info, doc_ptr doc, bool cb_hack);
static void _display_ignore(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_autopick(object_type *o_ptr, doc_ptr doc);
static void _display_score(object_type *o_ptr, doc_ptr doc);
static void _lite_display_doc(object_type *o_ptr, doc_ptr doc);

/* Ego Object Knowledge is very similar to obj_display() ... I had to hack a bit
   to make it work, though :( */
extern void ego_display(ego_type *e_ptr);
extern void ego_display_rect(ego_type *e_ptr, rect_t display);
extern void ego_display_doc(ego_type *e_ptr, doc_ptr doc);

static void _ego_display_name(ego_type *e_ptr, doc_ptr doc);
static void _ego_display_stats(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _ego_display_other_pval(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _ego_display_extra(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);

static int _calc_net_bonus(int amt, u32b flgs[OF_ARRAY_SIZE], int flg, int flg_dec);

static void _print_list(vec_ptr v, doc_ptr doc, char sep, char term);
static string_ptr _get_res_name(int res);

/* Low level helpers.
   Note: We have bonus flags, and penalty flags. For example
   TR_STR and TR_DEC_STR. With Rand-arts and the new Cursed implementation, it
   is possible for an object to have both. Also, pvals can be negative and use
   the bonus flag (rather than being positive and use the penalty flag).
   Note: vec_ptr is always vec<string_ptr> */
static int _calc_net_bonus(int amt, u32b flgs[OF_ARRAY_SIZE], int flg, int flg_dec)
{
    int net = 0;

    if (flg != OF_INVALID && have_flag(flgs, flg))
        net += amt;
    if (flg_dec != OF_INVALID && have_flag(flgs, flg_dec))
        net -= amt;

    return net;
}

static void _print_list(vec_ptr v, doc_ptr doc, char sep, char term)
{
    int ct = vec_length(v);
    int i;
    for (i = 0; i < ct; i++)
    {
        string_ptr s = vec_get(v, i);
        if (i < ct - 1 && sep)
            doc_printf(doc, "%s%c ", string_buffer(s), sep);
        else if (i == ct - 1 && term)
            doc_printf(doc, "%s%c", string_buffer(s), term);
        else
            doc_insert(doc, string_buffer(s));
    }
}

static string_ptr _get_res_name(int res)
{
    return string_alloc_format(
        "<color:%c>%s</color>",
        attr_to_attr_char(res_color(res)),
        res_name(res)
    );
}

/* Mid level helpers. Output one block of information at a time. */
static void _display_name(object_type *o_ptr, doc_ptr doc)
{
    char o_name[MAX_NLEN];
    char o_name2[MAX_NLEN];
    int leveys = MIN(72, doc->width);

    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_AND_ENCHANT | OD_NO_FLAVOR);
    object_desc(o_name2, o_ptr, OD_NAME_AND_ENCHANT | OD_NO_FLAVOR);
    if ((int)strlen(o_name2) > leveys - 10)
    {
        doc_printf(doc, "%s\n", o_name);
    }
    else doc_printf(doc, "%s%*c%2d.%d lbs\n", o_name, leveys - (strlen(o_name2) + 9), ' ', o_ptr->weight / 10, o_ptr->weight % 10);
}

static void _selita_paikka(char *paikka_text, byte paikka, byte taso, byte origin)
{
    bool tehtava = ((origin == ORIGIN_QUEST) || (origin == ORIGIN_QUEST_DROP) || (origin == ORIGIN_QUEST_REWARD));
    bool kaupunki = ((!tehtava) && (paikka < ORIGIN_DUNGEONS_AFTER));
    if (paikka > ORIGIN_QUESTS_AFTER)
    {
        paikka -= ORIGIN_QUESTS_AFTER;
        tehtava = TRUE;
    }

    if (kaupunki)
    {
        byte mika = ORIGIN_DUNGEONS_AFTER - paikka;
        strcpy(paikka_text, "in ");
        if (mika > TOWN_MAX)
        {
            strcat(paikka_text, "a bizarre town");
        }
        else
        {
            strcat(paikka_text, town_name(mika));
        }
        return;
    }
    else if (tehtava)
    {
        if (!paikka)
        {
            strcpy(paikka_text, "in a bizarre quest");
            return;
        }
        else
        {
            quest_ptr q = quests_get(paikka);
            cptr nimi;
            if ((!q) || (!q->id))
            {
                strcpy(paikka_text, "in a bizarre quest");
                return;
            }
            strcpy(paikka_text, (origin == ORIGIN_QUEST_REWARD) ? "the quest '" : "in the quest '");
            strcat(paikka_text, lyhytnimi(q, &nimi));
            free((vptr)nimi);
            strcat(paikka_text, "'");
            return;
        }
    }
    else if ((paikka == ORIGIN_DUNGEONS_AFTER) || (!taso))
    {
        strcpy(paikka_text, "in the wilderness");
        return;
    }
    else /* originated in a regular dungeon */
    {
        int dung = paikka - ORIGIN_DUNGEONS_AFTER;
        int aitotaso = taso;
        string_ptr apu;
        char *dung_name;
        if (dung == DUNGEON_HEAVEN) aitotaso += 512;
        else if (dung == DUNGEON_HELL) aitotaso += 640;
        if (dung > DUNGEON_MAX) dung = DUNGEON_MAX; /* desperation */
        dung_name = d_name + d_info[dung].name;
        apu = string_alloc_format("on level %d of %s", aitotaso, dung_name);
        strcpy(paikka_text, string_buffer(apu));
        string_free(apu);
        return;
    }
}

bool display_origin(object_type *o_ptr, doc_ptr doc)
{
    byte origin = o_ptr->origin_type;
    byte paikka = o_ptr->origin_place / ORIGIN_MODULO;
    byte taso = o_ptr->origin_place % ORIGIN_MODULO;
    char paikka_text[80];
    char pudottaja[80];

    if ((origin == ORIGIN_NONE) || (origin == ORIGIN_MIXED)) return FALSE;
    if ((!show_discovery) && (o_ptr->mitze_type & MITZE_MIXED)) return FALSE;
    if ((origin != ORIGIN_CHEAT) && (origin != ORIGIN_PLAYER_MADE) && (origin != ORIGIN_GAMBLE) && (origin != ORIGIN_ENDLESS)
     && (origin != ORIGIN_REFORGE) && (origin != ORIGIN_BIRTH) && (origin != ORIGIN_ARENA_REWARD) && (origin != ORIGIN_WANTED)
     && (origin != ORIGIN_CORNUCOPIA))
    {
        _selita_paikka(paikka_text, paikka, taso, origin);
    }
    if ((origin == ORIGIN_DROP) || (origin == ORIGIN_QUEST_DROP) || (origin == ORIGIN_STOLEN) || (origin == ORIGIN_ARENA_REWARD)
     || (origin == ORIGIN_WANTED))
    {
        monster_race *r_ptr = &r_info[o_ptr->origin_xtra];
        if ((o_ptr->origin_xtra) && (o_ptr->origin_xtra < max_r_idx) && (r_ptr) && (r_ptr->name))
        {
            cptr mon_name = r_name + r_ptr->name;
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                strcpy(pudottaja, mon_name);
            }
            else
            {
                strcpy(pudottaja, is_a_vowel(mon_name[0]) ? "an " : "a ");
                strcat(pudottaja, mon_name);
            }
        }
        else strcpy(pudottaja, "a bizarre monster");
    }

    switch (origin)
    {
        case ORIGIN_FLOOR:
        case ORIGIN_QUEST:
        {
            doc_printf(doc, "Found lying on the floor %s.", paikka_text);
            break;
        }
        case ORIGIN_DROP:
        case ORIGIN_QUEST_DROP:
        {
            doc_printf(doc, "Dropped by %s %s.", pudottaja, paikka_text);
            break;
        }
        case ORIGIN_CHEST:
        {
            doc_printf(doc, "Found in a chest %s.", paikka_text);
            break;
        }
        case ORIGIN_SPECIAL:
        {
            doc_printf(doc, "Found lying on the floor of a special room %s.", paikka_text);
            break;
        }
        case ORIGIN_VAULT:
        {
            doc_printf(doc, "Found lying on the floor in a vault %s.", paikka_text);
            break;
        }
        case ORIGIN_RUBBLE:
        {
            doc_printf(doc, "Found under some rubble %s.", paikka_text);
            break;
        }
        case ORIGIN_ACQUIRE:
        {
            if ((o_ptr->tval == TV_FOOD) && (o_ptr->sval == SV_FOOD_RATION)) /* Produce Food/Create Food */
            {
                if (p_ptr->prace == RACE_HOBBIT) doc_printf(doc, "Conjured forth by an expert cook %s.", paikka_text);
                else doc_printf(doc, "Manna from heaven.");
            }
            else
                doc_printf(doc, "Conjured forth by magic %s.", paikka_text);
            break;
        }
        case ORIGIN_STORE:
        {
            doc_printf(doc, "Bought from a store %s.", paikka_text);
            break;
        }
        case ORIGIN_BIRTH:
        {
            if ((p_ptr->pclass == CLASS_MONSTER) && (have_flag(o_ptr->flags, OF_NO_REMOVE))) return FALSE; /* Hack - Death-Swords, Rings, Filthy Rags */
            else if (!object_plural(o_ptr)) doc_printf(doc, "You can't remember ever not having it.");
            else doc_printf(doc, "You can't remember ever not having them.");
            break;
        }
        case ORIGIN_DROP_UNKNOWN:
        {
            doc_printf(doc, "Dropped by an unknown creature %s.", paikka_text);
            break;
        }
        case ORIGIN_CHEAT:
        {
            doc_printf(doc, "Created by a debug option.");
            break;
        }
        case ORIGIN_QUEST_REWARD:
        {
            doc_printf(doc, "Received as a reward for completing %s.", paikka_text);
            break;
        }
        case ORIGIN_ANGBAND_REWARD:
        {
            doc_printf(doc, "Dropped by the level guardian %s.", paikka_text);
            break;
        }
        case ORIGIN_ARENA_REWARD:
        {
            if (!no_wilderness) doc_printf(doc, "Dropped by %s in the Thalos Arena.", pudottaja);
            else doc_printf(doc, "Dropped by %s in the Arena.", pudottaja);
            break;
        }
        case ORIGIN_NAGA:
        {
            doc_printf(doc, "Received as a gift from Spiritnaga & Co. %s.", paikka_text);
            break;
        }
        case ORIGIN_PATTERN:
        {
            doc_printf(doc, "Received from the Pattern %s.", paikka_text);
            break;
        }
        case ORIGIN_PLAYER_MADE:
        {
            doc_printf(doc, "Your own handiwork.", paikka_text);
            break;
        }
        case ORIGIN_ART_CREATION:
        {
            doc_printf(doc, "Magically improved by a scroll %s.", paikka_text);
            break;
        }
        case ORIGIN_STOLEN:
        {
            doc_printf(doc, "Stolen from %s %s.", pudottaja, paikka_text);
            break;
        }
        case ORIGIN_REFORGE:
        {
            if (!no_wilderness) doc_printf(doc, "Reforged in Morivant.");
            else doc_printf(doc, "Reforged at the Fighters' Hall.");
            break;
        }
        case ORIGIN_GAMBLE:
        {
            doc_printf(doc, "You won it in an object lottery.");
            break;
        }
        case ORIGIN_ENDLESS:
        {
            doc_printf(doc, "Conjured forth by an endless quiver.");
            break;
        }
        case ORIGIN_PATRON:
        {
            if (o_ptr->origin_xtra >= MAX_PATRON) /* bizarre patron */
                 doc_printf(doc, "Received as a gift from your chaos patron %s.", paikka_text);
            else if (disciple_is_(DISCIPLE_TROIKA)) doc_printf(doc, "Received as a gift from the Troika %s.", paikka_text);
            else doc_printf(doc, "Received as a gift from %s %s.", chaos_patrons[o_ptr->origin_xtra], paikka_text);
            break;
        }
        case ORIGIN_PHOTO:
        {
            doc_printf(doc, "Taken %s.", paikka_text);
            break;
        }
        case ORIGIN_KAWARIMI:
        {
            doc_printf(doc, "Left behind %s.", paikka_text);
            break;
        }
        case ORIGIN_WANTED:
        {
            doc_printf(doc, "Received as a reward for turning in the corpse of %s.", pudottaja);
            break;
        }
        case ORIGIN_CAN_OF_TOYS:
        {
            doc_printf(doc, "Found in a Can of Toys %s.", paikka_text);
            break;
        }
        case ORIGIN_BLOOD:
        {
            if (object_is_(o_ptr, TV_POTION, SV_POTION_BLOOD))
                doc_printf(doc, "Created from your own blood %s.", paikka_text);
            else
                doc_printf(doc, "Resulted from %s going sour %s.", object_plural(o_ptr) ? "Potions of Blood" : "a Potion of Blood", paikka_text);
            break;
        }
        case ORIGIN_CORNUCOPIA:
        {
            doc_printf(doc, "Received from Cornucopia of Anambar as a replacement item.");
            break;
        }
        case ORIGIN_CRAFTING:
        {
            doc_printf(doc, "Crafted %s.", paikka_text);
            break;
        }
        case ORIGIN_MUNDANITY:
        {
            doc_printf(doc, "Mundanized %s.", paikka_text);
            break;
        }
    }
    if ((show_discovery) && (o_ptr->mitze_type) && (o_ptr->mitze_turn) && (origin != ORIGIN_BIRTH))
    {
        int day = 0, hour = 0, min = 0;
        doc_printf(doc, "\n");
        if (origin == ORIGIN_STORE) doc_printf(doc, "Purchased: ");
        else if (origin == ORIGIN_PHOTO) doc_printf(doc, "Snapped: ");
        else if (o_ptr->mitze_type & MITZE_REFORGE) doc_printf(doc, "Received: ");
        else if (o_ptr->mitze_type & MITZE_ID) doc_printf(doc, "Identified: ");
        else if (o_ptr->mitze_type & MITZE_PICKUP) doc_printf(doc, "Picked up: ");
        extract_day_hour_min_imp(o_ptr->mitze_turn, &day, &hour, &min);
        doc_printf(doc, "Day %d, %d:%02d, at CL %d.", day, hour, min, o_ptr->mitze_level);
        if (o_ptr->mitze_type & MITZE_MIXED) doc_printf(doc, "\n(Details may apply to the first item in a pile of similar items.)");
    }

    return TRUE;
}


static void _display_desc(object_type *o_ptr, doc_ptr doc)
{
    cptr text;

    if (o_ptr->name1 && object_is_known(o_ptr))
        text = a_text + a_info[o_ptr->name1].text;
    else
        text = k_text + k_info[o_ptr->k_idx].text;

    if (strlen(text))
        doc_printf(doc, "%s\n\n", text);
}

/* For convenience, the stats section will include a few other bonuses, like stealh,
   searching, tunneling, and speed. These are things where the pval is readily
   interpreted by the user (unlike, say, device power). */
struct _flag_info_s
{
    int flg;
    int flg_dec;
    cptr name;
};
typedef struct _flag_info_s _flag_info_t, *_flag_info_ptr;
static _flag_info_t _stats_flags[] =
{
    { OF_STR,           OF_DEC_STR,             "STR" },
    { OF_INT,           OF_DEC_INT,             "INT" },
    { OF_WIS,           OF_DEC_WIS,             "WIS" },
    { OF_DEX,           OF_DEC_DEX,             "DEX" },
    { OF_CON,           OF_DEC_CON,             "CON" },
    { OF_CHR,           OF_DEC_CHR,             "CHR" },
    { OF_SPEED,         OF_DEC_SPEED,           "Speed" },
    { OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY,   "Devices" },
    { OF_STEALTH,       OF_DEC_STEALTH,         "Stealth" },
    { OF_SEARCH,        OF_INVALID,             "Searching" },
    { OF_INFRA,         OF_INVALID,             "Infravision" },
    { OF_TUNNEL,        OF_INVALID,             "Digging" },
    { OF_XTRA_SHOTS,    OF_INVALID,             "Shooting Speed" },
    { OF_INVALID,       OF_INVALID,             NULL }
};

static int _opposite_flag(int i)
{
    static s16b loydetty[OF_COUNT - 1] = {0};
    _flag_info_ptr table = _stats_flags;
    if (i < 1) return OF_INVALID;
    if (i >= OF_COUNT) return OF_INVALID;
    if (loydetty[i - 1]) return loydetty[i - 1];
    while (table->flg != OF_INVALID)
    {
        if (i == table->flg)
        {
            loydetty[i - 1] = table->flg_dec;
            return table->flg_dec;
        }
        if (i == table->flg_dec)
        {
            loydetty[i - 1] = table->flg;
            return table->flg;
        }
        table++;
    }
    if (i >= OF_RES_ACID)
    {
        int tulos = resist_opposite_flag(i);
        loydetty[i - 1] = tulos;
        return tulos;
    }
    loydetty[i - 1] = OF_INVALID;
    return OF_INVALID;
}

void remove_opposite_flags(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = 1; i < OF_COUNT; i++)
    {
        int j;
        if (!have_flag(flgs, i)) continue;
        j = _opposite_flag(i);
        if ((j != OF_INVALID) && have_flag(flgs, j))
        {
            remove_flag(flgs, i);
            remove_flag(flgs, j);
        }
    }
}

static void _build_bonus_list(int pval, u32b flgs[OF_ARRAY_SIZE], _flag_info_ptr table, vec_ptr v)
{
    while (table->flg != OF_INVALID)
    {
        int net = _calc_net_bonus(pval, flgs, table->flg, table->flg_dec);
        if (net > 0)
            vec_add(v, string_copy_s(table->name));

        table++;
    }
}

static void _build_penalty_list(int pval, u32b flgs[OF_ARRAY_SIZE], _flag_info_ptr table, vec_ptr v)
{
    while (table->flg != OF_INVALID)
    {
        int net = _calc_net_bonus(pval, flgs, table->flg, table->flg_dec);
        if (net < 0)
            vec_add(v, string_copy_s(table->name));

        table++;
    }
}

static void _display_stats(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Pass 1: Positive Bonus */
    _build_bonus_list(o_ptr->pval, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_printf(doc, "<color:G>%+d</color> to ", abs(o_ptr->pval));
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Pass 2: Penalty Phase */
    vec_clear(v);
    _build_penalty_list(o_ptr->pval, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_printf(doc, "<color:r>%+d</color> to ", -abs(o_ptr->pval));
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}
static void _ego_display_stats(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Pass 1: Positive Bonus */
    _build_bonus_list(1, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:G>Increase</color> ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Pass 2: Penalty Phase */
    vec_clear(v);
    _build_penalty_list(1, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:r>Decrease</color> ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static _flag_info_t _sustains_flags[] =
{
    { OF_SUST_STR, OF_INVALID, "STR" },
    { OF_SUST_INT, OF_INVALID, "INT" },
    { OF_SUST_WIS, OF_INVALID, "WIS" },
    { OF_SUST_DEX, OF_INVALID, "DEX" },
    { OF_SUST_CON, OF_INVALID, "CON" },
    { OF_SUST_CHR, OF_INVALID, "CHR" },
    { OF_INVALID,  OF_INVALID, NULL }
};

static void _display_sustains(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    _build_bonus_list(1, flgs, _sustains_flags, v);
    if (vec_length(v) > 0)
    {
        if (vec_length(v) == 6)
            doc_insert(doc, "<color:B>Sustains All Stats</color>");
        else
        {
            doc_insert(doc, "<color:B>Sustain</color> ");
            _print_list(v, doc, ',', '\0');
        }
        doc_newline(doc);
    }
    vec_free(v);
}

static _flag_info_t _other_flags[] =
{
    { OF_BLOWS,             OF_INVALID, "Attack Speed" },
//    { OF_XTRA_SHOTS,        OF_INVALID, "Shooting Speed" },
    { OF_DEVICE_POWER,      OF_INVALID, "Device Power" },
    { OF_MAGIC_RESISTANCE,  OF_INVALID, "Magic Resistance" },
    { OF_SPELL_POWER,       OF_INVALID, "Spell Power" },
    { OF_SPELL_CAP,         OF_INVALID, "Spell Capacity" },
    { OF_LIFE,              OF_INVALID, "Life Rating" },
    { OF_INVALID,           OF_INVALID, NULL }
};
static void _ego_display_other_pval(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);
    _build_bonus_list(1, flgs, _other_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:G>Increase</color> ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }
    vec_free(v);
}
/* These pval flags require detailed explanations (e.g. +21% to Spell Power or -1.50 to Attack Speed) */
static void _display_other_pval(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int net = 0;

    if (!o_ptr->pval) return;

    if (have_flag(flgs, OF_BLOWS) || have_flag(flgs, OF_DEC_BLOWS))
    {
        int num = 0;

        if (have_flag(flgs, OF_BLOWS))
            num +=  o_ptr->pval * 50;
        if (have_flag(flgs, OF_DEC_BLOWS))
            num -=  o_ptr->pval * 100;

        doc_printf(doc, "<color:%c>%+d.%2.2d</color> to Attack Speed\n",
                    (net > 0) ? 'G' : 'r', num / 100, num % 100);
    }
/*    if (have_flag(flgs, OF_XTRA_SHOTS))
    {
        int num = o_ptr->pval * 15;
        doc_printf(doc, "<color:%c>%+d.%2.2d</color> to Shooting Speed\n",
                    (net > 0) ? 'G' : 'r', num / 100, num % 100);
    }*/

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_DEC_MAGIC_MASTERY);
    if (net)
    {
        int pct = device_power_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> to Device Power\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    if (have_flag(flgs, OF_MAGIC_RESISTANCE))
    {
        int pct = o_ptr->pval * 5;
        doc_printf(doc, "<color:%c>%+d%%</color> to Magic Resistance\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPELL_POWER, OF_DEC_SPELL_POWER);
    if (net)
    {
        int pct = spell_power_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> to Spell Power\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPELL_CAP, OF_DEC_SPELL_CAP);
    if (net)
    {
        int pct = spell_cap_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> to Spell Capacity\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_LIFE, OF_DEC_LIFE);
    if (net)
    {
        int pct = 3 * net;
        doc_printf(doc, "<color:%c>%+d%%</color> to Life Rating\n",
                    (net > 0) ? 'G' : 'r', pct);
    }
}

static void _display_brands(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_BRAND_ACID))
        vec_add(v, string_copy_s("<color:g>Acid Brand</color>"));
    if (have_flag(flgs, OF_BRAND_ELEC))
        vec_add(v, string_copy_s("<color:b>Lightning Brand</color>"));
    if (have_flag(flgs, OF_BRAND_FIRE))
        vec_add(v, string_copy_s("<color:r>Flame Tongue</color>"));
    if (have_flag(flgs, OF_BRAND_COLD))
        vec_add(v, string_copy_s("<color:W>Frost Brand</color>"));
    if (have_flag(flgs, OF_BRAND_POIS))
        vec_add(v, string_copy_s("<color:G>Viper's Fang</color>"));
    if (have_flag(flgs, OF_BRAND_CHAOS))
        vec_add(v, string_copy_s("<color:v>Mark of Chaos</color>"));
    if (have_flag(flgs, OF_BRAND_VAMP))
        vec_add(v, string_copy_s("<color:D>Vampiric</color>"));
    if (have_flag(flgs, OF_BRAND_DARK))
        vec_add(v, string_copy_s("<color:D>Shadow Sweep</color>"));
    if (have_flag(flgs, OF_IMPACT))
        vec_add(v, string_copy_s("<color:U>Earthquakes</color>"));
    if (have_flag(flgs, OF_VORPAL2))
        vec_add(v, string_copy_s("<color:v>*Sharpness*</color>"));
    else if (have_flag(flgs, OF_VORPAL))
        vec_add(v, string_copy_s("<color:R>Sharpness</color>"));
    if (have_flag(flgs, OF_STUN))
        vec_add(v, string_copy_s("<color:o>Stuns</color>"));
    if (have_flag(flgs, OF_BRAND_ORDER))
        vec_add(v, string_copy_s("<color:W>Weapon of Order</color>"));
    if (have_flag(flgs, OF_BRAND_WILD))
        vec_add(v, string_copy_s("<color:o>Wild Weapon</color>"));
    if (have_flag(flgs, OF_BRAND_MANA))
        vec_add(v, string_copy_s("<color:B>Mana Brand</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_slays(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_KILL_EVIL))
        vec_add(v, string_copy_s("<color:y>*Evil*</color>"));
    else if (have_flag(flgs, OF_SLAY_EVIL))
        vec_add(v, string_copy_s("<color:y>Evil</color>"));

	if (have_flag(flgs, OF_KILL_GOOD))
		vec_add(v, string_copy_s("<color:y>*Good*</color>"));
	else if (have_flag(flgs, OF_SLAY_GOOD))
        vec_add(v, string_copy_s("<color:W>Good</color>"));

	if (have_flag(flgs, OF_KILL_LIVING))
		vec_add(v, string_copy_s("<color:y>*Living*</color>"));
	else if (have_flag(flgs, OF_SLAY_LIVING))
        vec_add(v, string_copy_s("<color:o>Living</color>"));

    if (have_flag(flgs, OF_KILL_DRAGON))
        vec_add(v, string_copy_s("<color:r>*Dragons*</color>"));
    else if (have_flag(flgs, OF_SLAY_DRAGON))
        vec_add(v, string_copy_s("<color:r>Dragons</color>"));

    if (have_flag(flgs, OF_KILL_DEMON))
        vec_add(v, string_copy_s("<color:R>*Demons*</color>"));
    else if (have_flag(flgs, OF_SLAY_DEMON))
        vec_add(v, string_copy_s("<color:R>Demons</color>"));

    if (have_flag(flgs, OF_KILL_UNDEAD))
        vec_add(v, string_copy_s("<color:D>*Undead*</color>"));
    else if (have_flag(flgs, OF_SLAY_UNDEAD))
        vec_add(v, string_copy_s("<color:D>Undead</color>"));

    if (have_flag(flgs, OF_KILL_ANIMAL))
        vec_add(v, string_copy_s("<color:g>*Animals*</color>"));
    else if (have_flag(flgs, OF_SLAY_ANIMAL))
        vec_add(v, string_copy_s("<color:g>Animals</color>"));

    if (have_flag(flgs, OF_KILL_HUMAN))
        vec_add(v, string_copy_s("<color:s>*Humans*</color>"));
    else if (have_flag(flgs, OF_SLAY_HUMAN))
        vec_add(v, string_copy_s("<color:s>Humans</color>"));

    if (have_flag(flgs, OF_KILL_ORC))
        vec_add(v, string_copy_s("<color:U>*Orcs*</color>"));
    else if (have_flag(flgs, OF_SLAY_ORC))
        vec_add(v, string_copy_s("<color:U>Orcs</color>"));

    if (have_flag(flgs, OF_KILL_TROLL))
        vec_add(v, string_copy_s("<color:g>*Trolls*</color>"));
    else if (have_flag(flgs, OF_SLAY_TROLL))
        vec_add(v, string_copy_s("<color:g>Trolls</color>"));

    if (have_flag(flgs, OF_KILL_GIANT))
        vec_add(v, string_copy_s("<color:u>*Giants*</color>"));
    else if (have_flag(flgs, OF_SLAY_GIANT))
        vec_add(v, string_copy_s("<color:u>Giants</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "Slay ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_resists(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int     i;
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Immunities */
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg = res_get_object_immune_flag(i);
        if (flg != OF_INVALID && have_flag(flgs, flg))
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "Immunity to ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Resistances */
    vec_clear(v);
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg_im = res_get_object_immune_flag(i);
        int net = 0;

        if (flg_im != OF_INVALID && have_flag(flgs, flg_im)) continue;
        net = _calc_net_bonus(1, flgs, res_get_object_flag(i), res_get_object_vuln_flag(i));
        if (net > 0)
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "Resist ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Vulnerabilities */
    vec_clear(v);
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg_im = res_get_object_immune_flag(i);
        int net = 0;

        if (flg_im != OF_INVALID && have_flag(flgs, flg_im)) continue;
        net = _calc_net_bonus(1, flgs, res_get_object_flag(i), res_get_object_vuln_flag(i));
        if (net < 0)
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "Vulnerability to ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_abilities(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_LORE2))
        vec_add(v, string_copy_s("<color:B>Auto Identify</color>"));
    else if (have_flag(flgs, OF_LORE1))
        vec_add(v, string_copy_s("<color:B>Auto Pseudo-Identify</color>"));

    if (have_flag(flgs, OF_FREE_ACT))
        vec_add(v, string_copy_s("<color:R>Free Action</color>"));
    if (have_flag(flgs, OF_SEE_INVIS))
        vec_add(v, string_copy_s("<color:B>See Invisible</color>"));
    if (have_flag(flgs, OF_REGEN))
        vec_add(v, string_copy_s("<color:g>Regeneration</color>"));
    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>Hold Life</color>"));
    if (have_flag(flgs, OF_REFLECT))
        vec_add(v, string_copy_s("<color:o>Reflection</color>"));
    if (have_flag(flgs, OF_LEVITATION))
        vec_add(v, string_copy_s("<color:B>Levitation</color>"));
    if (have_flag(flgs, OF_SLOW_DIGEST))
        vec_add(v, string_copy_s("<color:g>Slow Digestion</color>"));
    if (have_flag(flgs, OF_WARNING))
        vec_add(v, string_copy_s("<color:y>Warning</color>"));
    if (have_flag(flgs, OF_NO_MAGIC))
        vec_add(v, string_copy_s("<color:r>Anti-Magic</color>"));
    if (have_flag(flgs, OF_NO_SUMMON))
        vec_add(v, string_copy_s("<color:v>Prevents Summoning</color>"));
    if (have_flag(flgs, OF_NO_TELE))
        vec_add(v, string_copy_s("<color:r>Prevents Teleportation</color>"));
    if (have_flag(flgs, OF_THROWING))
        vec_add(v, string_copy_s("<color:D>Throwing</color>"));
    if (have_flag(flgs, OF_BLESSED))
        vec_add(v, string_copy_s("<color:B>Blessed</color>"));
    if (have_flag(flgs, OF_RIDING))
        vec_add(v, string_copy_s("<color:o>Riding</color>"));
    if (have_flag(flgs, OF_DARKNESS))
        vec_add(v, string_copy_s("<color:D>Permanent Darkness</color>"));
    else if (have_flag(flgs, OF_LITE))
        vec_add(v, string_copy_s("<color:y>Permanent Light</color>"));
    if (have_flag(flgs, OF_NIGHT_VISION))
        vec_add(v, string_copy_s("<color:D>Night Vision</color>"));
    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_clear(v);
    if (have_flag(flgs, OF_TELEPATHY))
        vec_add(v, string_copy_s("<color:y>Telepathy</color>"));
    if (have_flag(flgs, OF_ESP_ANIMAL))
        vec_add(v, string_copy_s("<color:B>Sense Animals</color>"));
    if (have_flag(flgs, OF_ESP_UNDEAD))
        vec_add(v, string_copy_s("<color:D>Sense Undead</color>"));
    if (have_flag(flgs, OF_ESP_DEMON))
        vec_add(v, string_copy_s("<color:R>Sense Demons</color>"));
    if (have_flag(flgs, OF_ESP_ORC))
        vec_add(v, string_copy_s("<color:U>Sense Orcs</color>"));
    if (have_flag(flgs, OF_ESP_TROLL))
        vec_add(v, string_copy_s("<color:g>Sense Trolls</color>"));
    if (have_flag(flgs, OF_ESP_GIANT))
        vec_add(v, string_copy_s("<color:u>Sense Giants</color>"));
    if (have_flag(flgs, OF_ESP_DRAGON))
        vec_add(v, string_copy_s("<color:r>Sense Dragons</color>"));
    if (have_flag(flgs, OF_ESP_HUMAN))
        vec_add(v, string_copy_s("<color:s>Sense Humans</color>"));
    if (have_flag(flgs, OF_ESP_EVIL))
        vec_add(v, string_copy_s("<color:y>Sense Evil</color>"));
    if (have_flag(flgs, OF_ESP_GOOD))
        vec_add(v, string_copy_s("<color:w>Sense Good</color>"));
    if (have_flag(flgs, OF_ESP_NONLIVING))
        vec_add(v, string_copy_s("<color:B>Sense Nonliving</color>"));
	if (have_flag(flgs, OF_ESP_LIVING))
		vec_add(v, string_copy_s("<color:W>Sense Living</color>"));
    if (have_flag(flgs, OF_ESP_UNIQUE))
        vec_add(v, string_copy_s("<color:v>Sense Uniques</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_auras(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_AURA_FIRE))
        vec_add(v, _get_res_name(RES_FIRE));
    if (have_flag(flgs, OF_AURA_COLD))
        vec_add(v, _get_res_name(RES_COLD));
    if (have_flag(flgs, OF_AURA_ELEC))
        vec_add(v, _get_res_name(RES_ELEC));
    if (have_flag(flgs, OF_AURA_SHARDS))
        vec_add(v, string_copy_s("<color:U>Shards</color>"));
    if (have_flag(flgs, OF_AURA_REVENGE))
        vec_add(v, string_copy_s("<color:v>Retaliation</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "Aura of ");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_extra(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int net = 0;

    switch (o_ptr->name2)
    {
    case EGO_AMMO_RETURNING:
        doc_insert(doc, "It often returns to your pack after being fired.\n");
        break;
    case EGO_AMMO_ENDURANCE:
        doc_insert(doc, "It endures almost anything without being destroyed.\n");
        break;
    case EGO_QUIVER_PHASE:
        doc_insert(doc, "This quiver and its contents weigh absolutely nothing at all.\n");
        break;
    case EGO_QUIVER_PROTECTION:
        doc_insert(doc, "This quiver protects its contents from accidental destruction.\n");
        break;
    case EGO_QUIVER_HOLDING:
        doc_insert(doc, "This quiver has an increased carrying capacity.\n");
        break;
    }

    if (have_flag(flgs, OF_EASY_SPELL))
        doc_insert(doc, "It affects your ability to cast spells.\n");

    if (have_flag(flgs, OF_DEC_MANA))
    {
        doc_insert(doc, "It decreases your mana consumption.\n");
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_WEAPONMASTERY, OF_INVALID);
    if (net)
    {
        doc_printf(doc, "It %s the damage dice of your melee weapon.\n",
            (net > 0) ? "increases" : "<color:R>decreases</color>");
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_XTRA_MIGHT, OF_INVALID);
    if (net)
    {
        doc_printf(doc, "It %s the multiplier of your bow.\n",
            (net > 0) ? "increases" : "<color:R>decreases</color>");
    }

    switch (o_ptr->name1)
    {
    case ART_STONE_OF_NATURE:
        doc_insert(doc, "It greatly enhances Nature magic.\n");
        break;
    case ART_STONE_OF_LIFE:
        doc_insert(doc, "It greatly enhances Life magic.\n");
        break;
    case ART_STONE_OF_SORCERY:
        doc_insert(doc, "It greatly enhances Sorcery magic.\n");
        break;
    case ART_STONE_OF_CHAOS:
        doc_insert(doc, "It greatly enhances Chaos magic.\n");
        break;
    case ART_STONE_OF_DEATH:
        doc_insert(doc, "It greatly enhances Death magic.\n");
        break;
    case ART_STONE_OF_TRUMP:
        doc_insert(doc, "It greatly enhances Trump magic.\n");
        break;
    case ART_STONE_OF_DAEMON:
        doc_insert(doc, "It greatly enhances Daemon magic.\n");
        break;
    case ART_STONE_OF_CRUSADE:
        doc_insert(doc, "It greatly enhances Crusade magic.\n");
        break;
    case ART_STONE_OF_CRAFT:
        doc_insert(doc, "It greatly enhances Craft magic.\n");
        break;
    case ART_STONE_OF_ARMAGEDDON:
        doc_insert(doc, "It greatly enhances Armageddon magic.\n");
        break;
    case ART_STONEMASK:
        doc_insert(doc, "It makes you turn into a vampire permanently.\n");
        break;
    }

    if (object_is_(o_ptr, TV_SWORD, SV_POISON_NEEDLE))
        doc_insert(doc, "It will attempt to kill a monster instantly.\n");

    if (object_is_(o_ptr, TV_POLEARM, SV_DEATH_SCYTHE))
        doc_insert(doc, "It causes you to strike yourself sometimes.\nIt always penetrates invulnerability barriers.\n");

    if (have_flag(flgs, OF_IGNORE_INVULN))
        doc_insert(doc, "It negates invulnerability spheres.\n");

    if (have_flag(flgs, OF_DUAL_WIELDING))
        doc_insert(doc, "It affects your ability to hit when you are wielding two weapons.\n");

    if (o_ptr->tval == TV_STATUE)
    {
        if (o_ptr->pval == MON_BULLGATES)
            doc_insert(doc, "It is shameful.\n");
        else if (r_info[o_ptr->pval].flags2 & RF2_ELDRITCH_HORROR)
            doc_insert(doc, "It is fearful.\n");
        else
            doc_insert(doc, "It is cheerful.\n");
    }

    if (o_ptr->tval == TV_FIGURINE)
        doc_insert(doc, "It will transform into a pet when thrown.\n");

    if (o_ptr->name3)
    {
        cptr nimi = a_name + a_info[o_ptr->name3].name;
        doc_printf(doc, "It reminds you of the artifact <color:R>%s</color>.\n", ((nimi[0] == '&') && (strlen(nimi) > 2)) ? nimi + 2 : nimi);
    }
}

static void _display_insurance(object_type *o_ptr, doc_ptr doc)
{
    if (cornucopia_item_policy(o_ptr) >= 0)
    {
        int vakuutettu = (o_ptr->insured % 100);
        if (o_ptr->number == vakuutettu) doc_printf(doc, "\nYou have insured %s.\n", (vakuutettu == 1) ? "it" : "them");
        else if (vakuutettu) doc_printf(doc, "\n%d of the items in this pile %s insured.\n", vakuutettu, (vakuutettu == 1) ? "is": "are");
    }
}

static void _ego_display_extra(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (have_flag(flgs, OF_EASY_SPELL))
        doc_insert(doc, "It affects your ability to cast spells.\n");

    if (have_flag(flgs, OF_DEC_MANA))
    {
        caster_info *caster_ptr = get_caster_info();
        if (caster_ptr && (caster_ptr->options & CASTER_ALLOW_DEC_MANA))
            doc_insert(doc, "It decreases your mana consumption.\n");
    }

    if (have_flag(flgs, OF_WEAPONMASTERY))
        doc_insert(doc, "It increases the damage dice of your melee weapon.\n");

    if (have_flag(flgs, OF_XTRA_MIGHT))
        doc_insert(doc, "It increases the multiplier of your bow.\n");

    if (have_flag(flgs, OF_DUAL_WIELDING))
        doc_insert(doc, "It affects your ability to hit when you are wielding two weapons.\n");
}

static void _display_curses(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v;

    if (object_is_device(o_ptr)) return;
    if (!(o_ptr->ident & (IDENT_KNOWN | IDENT_SENSE))) return;

    v = vec_alloc((vec_free_f)string_free);

    /* Note: Object may not actually be cursed, but still might have
       Aggravate or TY Curse. */
    if (obj_is_identified(o_ptr))
    {
        /* Basic Curse Status is always obvious (light, heavy, permanent) */
        if (o_ptr->curse_flags & OFC_PERMA_CURSE)
            doc_insert(doc, "It is <color:v>Permanently Cursed</color>.\n");
        else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
            doc_insert(doc, "It is <color:r>Heavily Cursed</color>.\n");
        else if (o_ptr->curse_flags & OFC_CURSED)
            doc_insert(doc, "It is <color:D>Cursed</color>.\n");
    }
    else if (o_ptr->curse_flags & OFC_CURSED)
        doc_insert(doc, "It has <color:v>unknown curses</color>.\n");

    /* The precise nature of the curse, however, must be learned either by
       experience or by *identification* */
    if (have_flag(flgs, OF_TY_CURSE) || o_ptr->known_curse_flags & OFC_TY_CURSE)
        vec_add(v, string_copy_s("<color:v>*Ancient Foul Curse*</color>"));
    if (have_flag(flgs, OF_AGGRAVATE) || o_ptr->known_curse_flags & OFC_AGGRAVATE)
        vec_add(v, string_copy_s("<color:r>Aggravates</color>"));
    if (have_flag(flgs, OF_DRAIN_EXP) || o_ptr->known_curse_flags & OFC_DRAIN_EXP)
        vec_add(v, string_copy_s("<color:y>Drains Experience</color>"));
    if (o_ptr->known_curse_flags & OFC_SLOW_REGEN)
        vec_add(v, string_copy_s("<color:o>Slow Regeneration</color>"));
    if (o_ptr->known_curse_flags & OFC_ADD_L_CURSE)
        vec_add(v, string_copy_s("<color:w>Adds Weak Curses</color>"));
    if (o_ptr->known_curse_flags & OFC_ADD_H_CURSE)
        vec_add(v, string_copy_s("<color:b>Adds Heavy Curses</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_ANIMAL)
        vec_add(v, string_copy_s("<color:g>Attracts Animals</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_DEMON)
        vec_add(v, string_copy_s("<color:R>Attracts Demons</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_DRAGON)
        vec_add(v, string_copy_s("<color:r>Attracts Dragons</color>"));
    if (o_ptr->known_curse_flags & OFC_COWARDICE)
        vec_add(v, string_copy_s("<color:y>Cowardice</color>"));
    if (have_flag(flgs, OF_TELEPORT) || o_ptr->known_curse_flags & OFC_TELEPORT)
        vec_add(v, string_copy_s("<color:B>Random Teleportation</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_MELEE)
        vec_add(v, string_copy_s("<color:G>Miss Blows</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_AC)
        vec_add(v, string_copy_s("<color:R>Low AC</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_MAGIC)
        vec_add(v, string_copy_s("<color:y>Increased Fail Rates</color>"));
    if (o_ptr->known_curse_flags & OFC_FAST_DIGEST)
        vec_add(v, string_copy_s("<color:r>Fast Digestion</color>"));
    if (o_ptr->known_curse_flags & OFC_DRAIN_HP)
        vec_add(v, string_copy_s("<color:o>Drains You</color>"));
    if (o_ptr->known_curse_flags & OFC_DRAIN_MANA)
        vec_add(v, string_copy_s("<color:B>Drains Mana</color>"));
    if (object_is_unenchantable(o_ptr))
        vec_add(v, string_copy_s("<color:r>Unenchantable</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_activation_aux(effect_t *effect, bool full_info, doc_ptr doc, bool cb_hack)
{
    cptr res = cb_hack ? "Release Pet" : do_effect(effect, SPELL_NAME, 0);

    doc_newline(doc);
    doc_printf(doc, "<color:U>Activation:</color><tab:12><color:B>%s</color>\n", res);

    if (full_info)
    {
        int fail = effect_calc_fail_rate(effect);

        res = do_effect(effect, SPELL_INFO, 0);
        if (res && strlen(res))
            doc_printf(doc, "<color:U>Info:</color><tab:12>%s\n", res);
        doc_printf(doc, "<color:U>Fail:</color><tab:12>%d.%d%%\n", fail/10, fail%10);
        if (effect->cost)
            doc_printf(doc, "<color:U>Timeout:</color><tab:12>%d\n", effect->cost);
    }

}

static void _display_activation(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (obj_has_effect(o_ptr))
    {
        if (have_flag(flgs, OF_ACTIVATE))
        {
            effect_t e = obj_get_effect(o_ptr);
            bool capture_ball_hack = ((o_ptr->tval == TV_CAPTURE) && (o_ptr->pval > 0));
            _display_activation_aux(&e, obj_is_identified_fully(o_ptr), doc, capture_ball_hack);
            doc_newline(doc);
        }
        else
        {
            doc_newline(doc);
            doc_insert(doc, "<color:U>Activation:</color><tab:12><color:y>?</color>\n");
            doc_newline(doc);
        }
    }
}

static void _display_ignore(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (have_flag(flgs, OF_IGNORE_ACID) &&
        have_flag(flgs, OF_IGNORE_ELEC) &&
        have_flag(flgs, OF_IGNORE_FIRE) &&
        have_flag(flgs, OF_IGNORE_COLD))
    {
        doc_insert(doc, "<color:B>It cannot be harmed by the elements.</color>\n");
    }
    else
    {
        vec_ptr v = vec_alloc((vec_free_f)string_free);

        if (have_flag(flgs, OF_IGNORE_ACID))
            vec_add(v, _get_res_name(RES_ACID));
        if (have_flag(flgs, OF_IGNORE_ELEC))
            vec_add(v, _get_res_name(RES_ELEC));
        if (have_flag(flgs, OF_IGNORE_FIRE))
            vec_add(v, _get_res_name(RES_FIRE));
        if (have_flag(flgs, OF_IGNORE_COLD))
            vec_add(v, _get_res_name(RES_COLD));

        if (vec_length(v) > 0)
        {
            doc_insert(doc, "It cannot be harmed by ");
            _print_list(v, doc, ',', '.');
            doc_newline(doc);
        }

        vec_free(v);
    }
}

static void _display_autopick(object_type *o_ptr, doc_ptr doc)
{
    if (destroy_debug)
    {
        int idx = is_autopick(o_ptr);
        if (idx >= 0)
        {
            string_ptr s = autopick_line_from_entry(&autopick_list[idx], AUTOPICK_COLOR_CODED);
            doc_printf(doc, "<color:r>Autopick:</color> <indent><style:indent>%s</style></indent>\n", string_buffer(s));
            string_free(s);
        }
    }
}

#if 0
/* Debugging Object Pricing */
static doc_ptr _dbg_doc = NULL;
static void _cost_dbg_hook(cptr msg)
{
    doc_printf(_dbg_doc, "%s\n", msg);
}
#else
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
#endif

static void _display_score(object_type *o_ptr, doc_ptr doc)
{
#if 0
    _dbg_doc = doc;
    cost_calc_hook = _cost_dbg_hook;

    doc_newline(doc);
    new_object_cost(o_ptr, 0);

    cost_calc_hook = NULL;
    _dbg_doc = NULL;
#else
    int score = obj_value(o_ptr);
    char buf[10];
    big_num_display(score, buf);
    doc_printf(doc, "<color:B>Score:</color> <color:%c>%s</color>", _score_color(score), buf);
    if (o_ptr->level)
        doc_printf(doc, " (L%d)", o_ptr->level);
    if (o_ptr->discount)
        doc_printf(doc, " (%d%% discount)", o_ptr->discount);
    doc_newline(doc);

    if (p_ptr->prace == RACE_ANDROID && obj_is_identified(o_ptr))
    {
        score = android_obj_exp(o_ptr);
        big_num_display(score, buf);
        if (score)
            doc_printf(doc, "<color:B>Const:</color> <color:%c>%s</color>\n", _score_color(score), buf);
    }

#endif
}

static void _lite_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (o_ptr->tval != TV_LITE) return;
    obj_flags_known(o_ptr, flgs);
    if (have_flag(flgs, OF_DARKNESS))
    {
        doc_insert(doc, "It provides no light.\n");

        if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "It decreases radius of light source by 2.\n");
        else if (o_ptr->sval == SV_LITE_TORCH)
            doc_insert(doc, "It decreases radius of light source by 1.\n");
        else
            doc_insert(doc, "It decreases radius of light source by 3.\n");
    }
    else if (have_flag(flgs, OF_NIGHT_VISION))
    {
        doc_insert(doc, "It allows you to see in the dark.\n");
    }
    else if (o_ptr->name1 || o_ptr->art_name)
    {
        doc_insert(doc, "It provides light (radius 3) forever.\n");
    }
    else if (o_ptr->name2 == EGO_LITE_EXTRA_LIGHT)
    {
        if (o_ptr->sval == SV_LITE_FEANOR)
            doc_insert(doc, "It provides light (radius 3) forever.\n");
        else if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "It provides light (radius 3) when fueled.\n");
        else
            doc_insert(doc, "It provides light (radius 2) when fueled.\n");
    }
    else
    {
        if (o_ptr->sval == SV_LITE_FEANOR)
            doc_insert(doc, "It provides light (radius 2) forever.\n");
        else if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "It provides light (radius 2) when fueled.\n");
        else
            doc_insert(doc, "It provides light (radius 1) when fueled.\n");
    }
    if (o_ptr->name2 == EGO_LITE_DURATION)
        doc_insert(doc, "It provides light for much longer time.\n");
}

/* Public Interface */
void obj_display(object_type *o_ptr)
{
    obj_display_rect(o_ptr, ui_menu_rect());
}

void obj_display_rect(object_type *o_ptr, rect_t display)
{
    doc_ptr doc = doc_alloc(MIN(display.cx, 72));

    if (display.cx > 80)
        display.cx = 80;

    obj_display_doc(o_ptr, doc);

    screen_save();
    if (doc_cursor(doc).y < display.cy - 3)
    {
        doc_insert(doc, "\n<color:B>[Press <color:y>Any Key</color> to Continue]</color>\n\n");
        doc_sync_term(doc, doc_range_all(doc), doc_pos_create(display.x, display.y));
        inkey();
    }
    else
    {
        doc_display_aux(doc, "Object Info", 0, display);
    }
    screen_load();

    doc_free(doc);
}

void obj_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];

    /* Devices need special handling. For one thing, they cannot be equipped, so
       that most flags are not used, and those that are generally mean something
       different (e.g. TR_HOLD_LIFE means no charge draining). */
    if (object_is_device(o_ptr) || o_ptr->tval == TV_POTION || o_ptr->tval == TV_SCROLL)
    {
        device_display_doc(o_ptr, doc);
        return;
    }

    obj_flags_display(o_ptr, flgs);

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");
    if (show_origins || show_discovery)
    {
        if (display_origin(o_ptr, doc)) doc_printf(doc, "\n\n");

    }
    _display_desc(o_ptr, doc);
    doc_insert(doc, "<style:indent>"); /* Indent a bit when word wrapping long lines */

    if (o_ptr->tval == TV_LITE)
        _lite_display_doc(o_ptr, doc);

    _display_stats(o_ptr, flgs, doc);
    _display_sustains(flgs, doc);
    _display_other_pval(o_ptr, flgs, doc);
    _display_slays(flgs, doc);
    _display_brands(flgs, doc);
    _display_resists(flgs, doc);
    _display_abilities(flgs, doc);
    _display_auras(flgs, doc);
    _display_extra(o_ptr, flgs, doc);
    _display_insurance(o_ptr, doc);
    _display_activation(o_ptr, flgs, doc);
    _display_curses(o_ptr, flgs, doc);
    _display_ignore(flgs, doc);
    _display_score(o_ptr, doc);

    if (object_is_wearable(o_ptr))
    {
        doc_newline(doc);
        if (obj_is_identified(o_ptr))
        {
            if (!obj_is_identified_fully(o_ptr))
            {
                if (object_is_artifact(o_ptr))
                    doc_printf(doc, "This object is an artifact, a unique object whose powers you must either learn by direct experience or by *identifying* or selling this object.\n");
                else
                    doc_printf(doc, "This object may have additional powers which you may learn either by experience or by *identifying* or selling this object.\n");
            }
        }
        else
        {
            doc_printf(doc, "This object is unknown. You should either identify it, or, if you are truly bold (foolish?), then you might learn more by equipping it!\n");
        }
    }
    _display_autopick(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

void obj_display_smith(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];

    if (object_is_device(o_ptr))
    {
        device_display_smith(o_ptr, doc);
        return;
    }

    obj_flags_display(o_ptr, flgs);

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent><style:indent>");

    if (!object_is_known(o_ptr) && (o_ptr->ident & IDENT_SENSE))
    {
        switch (o_ptr->feeling)
        {
        case FEEL_TERRIBLE:
            doc_insert(doc, "This item appears to be something truly <color:v>terrible</color>.\n");
            break;
        case FEEL_SPECIAL:
            doc_insert(doc, "This item appears to be something truly <color:B>special</color>.\n");
            break;
        case FEEL_AWFUL:
            doc_insert(doc, "This item appears to be something <color:r>awful</color>.\n");
            break;
        case FEEL_EXCELLENT:
            doc_insert(doc, "This item appears to be an <color:y>excellent</color> piece of work.\n");
            break;
        case FEEL_BAD:
            doc_insert(doc, "This item appears to be a <color:R>shoddy</color> piece of work.\n");
            break;
        case FEEL_GOOD:
            doc_insert(doc, "This item displays <color:G>good</color> craftmanship.\n");
            break;
        case FEEL_AVERAGE:
            doc_insert(doc, "This item appears normal.\n");
            break;
        }
    }


    _display_stats(o_ptr, flgs, doc);
    _display_sustains(flgs, doc);
    _display_other_pval(o_ptr, flgs, doc);
    _display_slays(flgs, doc);
    _display_brands(flgs, doc);
    _display_resists(flgs, doc);
    _display_abilities(flgs, doc);
    _display_auras(flgs, doc);
    _display_extra(o_ptr, flgs, doc);
    _display_curses(o_ptr, flgs, doc);
    _display_ignore(flgs, doc);
    _display_score(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

void device_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b    flgs[OF_ARRAY_SIZE];
    int     net = 0;
    int     boost = 0;
    vec_ptr v = NULL;

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");
    if (show_origins || show_discovery)
    {
        if (display_origin(o_ptr, doc)) doc_printf(doc, "\n\n");

    }
    _display_desc(o_ptr, doc);

    if (o_ptr->tval == TV_SCROLL || o_ptr->tval == TV_POTION)
    {
        if (!object_is_known(o_ptr)) doc_printf(doc, "You have no special knowledge about this item.\n\n");
        else doc_printf(doc, "%s\n\n", do_device(o_ptr, SPELL_DESC, 0));
        if (obj_is_identified_fully(o_ptr))
        {
            cptr info = do_device(o_ptr, SPELL_INFO, 0);
            if (info && strlen(info))
                doc_printf(doc, "<color:U>Info: </color>%s\n", info);
            if (o_ptr->tval == TV_SCROLL)
            {
                int fail = device_calc_fail_rate(o_ptr);
                doc_printf(doc, "<color:U>Fail: </color>%d.%d%%\n", fail/10, fail%10);
            }
        }
        _display_insurance(o_ptr, doc);
        _display_autopick(o_ptr, doc);
        doc_insert(doc, "</indent>\n");
        return;
    }

    if (!object_is_device(o_ptr) || !object_is_known(o_ptr))
    {
        _display_insurance(o_ptr, doc);
        _display_autopick(o_ptr, doc);
        doc_insert(doc, "</indent>\n");
        return;
    }

    obj_flags_known(o_ptr, flgs);
    if (devicemaster_is_speciality(o_ptr))
        boost = device_power_aux(100, p_ptr->device_power + p_ptr->lev/10) - 100;
    else
        boost = device_power(100) - 100;

    doc_insert(doc, "<color:U>This device has the following magical strength:</color>\n");
    if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_POWER))
        doc_printf(doc, "Power  : <color:G>%d</color>\n", device_level(o_ptr));
    else
        doc_insert(doc, "Power  : <color:G>?</color>\n");
    if (obj_is_identified_fully(o_ptr))
    {
        int sp = device_sp(o_ptr);
        int max_sp = device_max_sp(o_ptr);

        doc_printf(doc, "Mana   : <color:%c>%d</color>/<color:G>%d</color>\n",
                    (sp < max_sp) ? 'y' : 'G', sp, max_sp);
    }
    else
    {
        doc_insert(doc, "Mana   : <color:G>?</color>/<color:G>?</color>\n");
    }

    v = vec_alloc((vec_free_f)string_free);
    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPEED, OF_DEC_SPEED);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d Quickness</color>", (net > 0) ? 'G' : 'r', net));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_INVALID);
    if (net)
    {
        int        pct = device_power_aux(100, net) - 100;
        string_ptr s = string_alloc_format("<color:%c>%+d%% to Power</color>", (net > 0) ? 'G' : 'r', pct);
        vec_add(v, s);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_EASY_SPELL, OF_INVALID);
    if (net > 0)
        vec_add(v, string_alloc_format("<color:G>%+d Easy Use</color>", net));
    else if (net < 0)
        vec_add(v, string_copy_s("<color:r>Hard Use</color>"));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_REGEN, OF_INVALID);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d to Regeneration</color>", (net > 0) ? 'G' : 'r', net));

    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>Hold Charges</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "Extra  : <indent>");
        _print_list(v, doc, ';', '\0');
        doc_insert(doc, "</indent>\n\n");
    }
    else
        doc_newline(doc);

    vec_free(v);

    if (o_ptr->activation.type != EFFECT_NONE)
    {
        doc_insert(doc, "<color:U>This device is loaded with a spell:</color>\n");
        doc_printf(doc, "Spell  : <color:B>%s</color>\n", do_device(o_ptr, SPELL_NAME, boost));
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_POWER))
        {
            cptr desc;
            desc = do_device(o_ptr, SPELL_INFO, boost);
            if (desc && strlen(desc))
                doc_printf(doc, "Info   : <color:w>%s</color>\n", desc);
        }
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_FAIL))
            doc_printf(doc, "Level  : <color:G>%d</color>\n", o_ptr->activation.difficulty);

        if (obj_is_identified_fully(o_ptr))
        {
            int  charges = device_sp(o_ptr) / o_ptr->activation.cost;
            int  max_charges = device_max_sp(o_ptr) / o_ptr->activation.cost;
            doc_printf(doc, "Cost   : <color:G>%d</color>\n", o_ptr->activation.cost);
            doc_printf(doc, "Charges: <color:%c>%d</color>/<color:G>%d</color>\n",
                        (charges < max_charges) ? 'y' : 'G', charges, max_charges);
            if (p_ptr->pclass == CLASS_MAGIC_EATER)
            {
                int       per_mill = magic_eater_regen_amt(o_ptr->tval);
                const int scale = 100;
                int       cost = o_ptr->activation.cost * scale;
                int       sp, turns;

                if (have_flag(flgs, OF_REGEN))
                    per_mill += o_ptr->pval * per_mill / 5;

                sp = device_max_sp(o_ptr) * per_mill * scale / 1000;
                turns = cost * scale / sp;

                doc_printf(doc, "Regen  : <color:G>%d.%02d</color> rounds per charge\n", turns / scale, turns % scale);
            }
        }
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_FAIL))
        {
            int  fail = device_calc_fail_rate(o_ptr);
            doc_printf(doc, "Fail   : <color:G>%d.%d%%</color>\n", fail/10, fail%10);
        }
        doc_printf(doc, "Desc   : <indent>%s</indent>\n\n", do_device(o_ptr, SPELL_DESC, boost));
    }

    _display_insurance(o_ptr, doc);
        
    doc_insert(doc, "<style:indent>"); /* Indent a bit when word wrapping long lines */
    _display_score(o_ptr, doc);

    if (!obj_is_identified_fully(o_ptr))
        doc_printf(doc, "\nYou may *identify* or sell this object to learn more about this device. Also, for many device types, you can learn more by actually using this device long enough.\n");

    _display_ignore(flgs, doc);
    _display_autopick(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

/* For wiz_smithing ... TODO: Consolidate common code with device_display_doc */
void device_display_smith(object_type *o_ptr, doc_ptr doc)
{
    u32b    flgs[OF_ARRAY_SIZE];
    int     net = 0;
    int     boost = 0;
    vec_ptr v = NULL;

    assert(object_is_device(o_ptr));
    assert(obj_is_identified_fully(o_ptr));

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");

    obj_flags_known(o_ptr, flgs);
    if (devicemaster_is_speciality(o_ptr))
        boost = device_power_aux(100, p_ptr->device_power + p_ptr->lev/10) - 100;
    else
        boost = device_power(100) - 100;

    {
        int sp = device_sp(o_ptr);
        int max_sp = device_max_sp(o_ptr);

        doc_insert(doc, "<color:U>This device has the following magical strength:</color>\n");
        doc_printf(doc, "Power  : <color:G>%d</color>\n", device_level(o_ptr));
        doc_printf(doc, "Mana   : <color:%c>%d</color>/<color:G>%d</color>\n",
                    (sp < max_sp) ? 'y' : 'G', sp, max_sp);
    }

    v = vec_alloc((vec_free_f)string_free);
    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPEED, OF_DEC_SPEED);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d Quickness</color>", (net > 0) ? 'G' : 'r', net));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_INVALID);
    if (net)
    {
        int        pct = device_power_aux(100, net) - 100;
        string_ptr s = string_alloc_format("<color:%c>%+d%% to Power</color>", (net > 0) ? 'G' : 'r', pct);
        vec_add(v, s);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_EASY_SPELL, OF_INVALID);
    if (net > 0)
        vec_add(v, string_alloc_format("<color:G>%+d Easy Use</color>", net));
    else if (net < 0)
        vec_add(v, string_copy_s("<color:r>Hard Use</color>"));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_REGEN, OF_INVALID);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d to Regeneration</color>", (net > 0) ? 'G' : 'r', net));

    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>Hold Charges</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "Extra  : <indent>");
        _print_list(v, doc, ';', '\0');
        doc_insert(doc, "</indent>\n\n");
    }
    else
        doc_newline(doc);

    vec_free(v);

    if (o_ptr->activation.type != EFFECT_NONE)
    {
        int  fail = device_calc_fail_rate(o_ptr);
        cptr desc;

        doc_insert(doc, "<color:U>This device is loaded with a spell:</color>\n");
        doc_printf(doc, "Spell  : <color:B>%s</color>\n", do_device(o_ptr, SPELL_NAME, boost));
        desc = do_device(o_ptr, SPELL_INFO, boost);
        if (desc && strlen(desc))
            doc_printf(doc, "Info   : <color:w>%s</color>\n", desc);
        doc_printf(doc, "Level  : <color:G>%d</color>\n", o_ptr->activation.difficulty);
        doc_printf(doc, "Cost   : <color:G>%d</color>\n", o_ptr->activation.cost);
        doc_printf(doc, "Fail   : <color:G>%d.%d%%</color>\n", fail/10, fail%10);
    }

    _display_score(o_ptr, doc);
    _display_ignore(flgs, doc);
    doc_insert(doc, "</indent>\n");
}

void ego_display(ego_type *e_ptr)
{
    ego_display_rect(e_ptr, ui_menu_rect());
}

void ego_display_rect(ego_type *e_ptr, rect_t display)
{
    doc_ptr doc = doc_alloc(MIN(display.cx, 72));

    if (display.cx > 80)
        display.cx = 80;

    ego_display_doc(e_ptr, doc);

    screen_save();
    if (doc_cursor(doc).y < display.cy - 3)
    {
        doc_insert(doc, "\n[Press Any Key to Continue]\n\n");
        doc_sync_term(doc, doc_range_all(doc), doc_pos_create(display.x, display.y));
        inkey();
    }
    else
    {
        doc_display_aux(doc, "Ego Info", 0, display);
    }
    screen_load();

    doc_free(doc);
}

static bool _have_flag(u32b flgs[OF_ARRAY_SIZE])
{
    int  i;
    for (i = 0; i < OF_ARRAY_SIZE; i++)
    {
        if (flgs[i])
            return TRUE;
    }
    return FALSE;
}

static void _ego_display_name(ego_type *e_ptr, doc_ptr doc)
{
    char name[255];

    /* Major hackage. Many egos can span multiple types, and
       I'd rather not always list them all. For example, 'Armor of Protection'
       is enough, rather than 'Body Armor/Shield/Cloak/Helmet/Gloves/Boots of Protection' */
    if (e_ptr->type & EGO_TYPE_WEAPON)
        doc_insert(doc, "Weapon ");
    else if (e_ptr->type & EGO_TYPE_DIGGER)
        doc_insert(doc, "Digger ");
    else if (e_ptr->type & EGO_TYPE_BODY_ARMOR)
        doc_insert(doc, "Armor ");
    else if (e_ptr->type & EGO_TYPE_DRAGON_ARMOR)
        doc_insert(doc, "Dragon Armor ");
    else if (e_ptr->type == (EGO_TYPE_CLOAK | EGO_TYPE_BOOTS))
        doc_insert(doc, "Cloak/Boots ");
    else if (e_ptr->type == (EGO_TYPE_GLOVES | EGO_TYPE_BOOTS))
        doc_insert(doc, "Golves/Boots ");
    else if (e_ptr->type == (EGO_TYPE_HELMET | EGO_TYPE_CROWN))
        doc_insert(doc, "Helmet/Crown ");
    else if (e_ptr->type & EGO_TYPE_SHIELD)
        doc_insert(doc, "Shield ");
    else if (e_ptr->type & EGO_TYPE_ROBE)
        doc_insert(doc, "Robe ");
    else if (e_ptr->type & EGO_TYPE_CLOAK)
        doc_insert(doc, "Cloak ");
    else if (e_ptr->type & EGO_TYPE_HELMET)
        doc_insert(doc, "Helmet ");
    else if (e_ptr->type & EGO_TYPE_CROWN)
        doc_insert(doc, "Crown ");
    else if (e_ptr->type & EGO_TYPE_GLOVES)
        doc_insert(doc, "Gloves ");
    else if (e_ptr->type & EGO_TYPE_BOOTS)
        doc_insert(doc, "Boots ");
    else if (e_ptr->type & EGO_TYPE_BOW)
        doc_insert(doc, "Bow ");
    else if (e_ptr->type & EGO_TYPE_AMMO)
        doc_insert(doc, "Ammo ");
    else if (e_ptr->type & EGO_TYPE_HARP)
        doc_insert(doc, "Harp ");
    else if (e_ptr->type == (EGO_TYPE_RING | EGO_TYPE_AMULET))
        doc_insert(doc, "Ring/Amulet ");
    else if (e_ptr->type & EGO_TYPE_RING)
        doc_insert(doc, "Ring ");
    else if (e_ptr->type & EGO_TYPE_AMULET)
        doc_insert(doc, "Amulet ");
    else if (e_ptr->type & EGO_TYPE_LITE)
        doc_insert(doc, "Light ");
    else if (e_ptr->type & EGO_TYPE_DEVICE)
        doc_insert(doc, "Device ");

    strip_name_aux(name, e_name + e_ptr->name);
    doc_printf(doc, "%s\n\n", name);
}

void ego_display_doc(ego_type *e_ptr, doc_ptr doc)
{
    int  i;
    u32b flgs[OF_ARRAY_SIZE];

    _ego_display_name(e_ptr, doc);

    /* First, the fixed flags always present */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = e_ptr->known_flags[i] & e_ptr->flags[i];
    remove_flag(flgs, OF_HIDE_TYPE);
    remove_flag(flgs, OF_SHOW_MODS);
    remove_flag(flgs, OF_FULL_NAME);

    if (_have_flag(flgs))
    {
        doc_insert(doc, "<color:B>Fixed Bonuses</color>\n");
        doc_insert(doc, "  <indent><style:indent>");
        _ego_display_stats(flgs, doc);
        _display_sustains(flgs, doc);
        _ego_display_other_pval(flgs, doc);
        _display_slays(flgs, doc);
        _display_brands(flgs, doc);
        _display_resists(flgs, doc);
        _display_abilities(flgs, doc);
        _display_auras(flgs, doc);
        _ego_display_extra(flgs, doc);
        if (have_flag(flgs, OF_ACTIVATE))
            _display_activation_aux(&e_ptr->activation, FALSE, doc, FALSE); /* no ego capture balls */
        _display_ignore(flgs, doc);
        doc_insert(doc, "</style></indent>\n");
    }

    /* Next, the optional flags */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = e_ptr->known_flags[i] & e_ptr->xtra_flags[i];
    remove_flag(flgs, OF_HIDE_TYPE);
    remove_flag(flgs, OF_SHOW_MODS);
    remove_flag(flgs, OF_FULL_NAME);

    if (_have_flag(flgs))
    {
        doc_insert(doc, "<color:B>Optional Bonuses</color>\n");
        doc_insert(doc, "  <indent><style:indent>");
        _ego_display_stats(flgs, doc);
        _display_sustains(flgs, doc);
        _ego_display_other_pval(flgs, doc);
        _display_slays(flgs, doc);
        _display_brands(flgs, doc);
        _display_resists(flgs, doc);
        _display_abilities(flgs, doc);
        _display_auras(flgs, doc);
        _ego_display_extra(flgs, doc);
        if (have_flag(flgs, OF_ACTIVATE))
            doc_insert(doc, "It grants an extra activation.\n");
        _display_ignore(flgs, doc);
        doc_insert(doc, "</style></indent>\n");
    }
}
