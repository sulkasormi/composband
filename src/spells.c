#include "angband.h"

#include "str-map.h"
#include <assert.h>

/***********************************************************************
 * Spell Skills, Stats and Savefile Support
 ***********************************************************************/

static str_map_ptr _spell_stats_map(void)
{
    static str_map_ptr _map = NULL;
    if (!_map)
        _map = str_map_alloc(free);
    return _map;
}

spell_stats_ptr spell_stats_aux(cptr name)
{
    str_map_ptr     map = _spell_stats_map();
    spell_stats_ptr result = str_map_find(map, name);

    if (!result)
    {
        result = malloc(sizeof(spell_stats_t));
        memset(result, 0, sizeof(spell_stats_t));
        str_map_add(map, name, result);
    }
    return result;
}

spell_stats_ptr spell_stats(spell_info *spell)
{
    cptr name = get_spell_stat_name(spell->fn);
    return spell_stats_aux(name);
}

void spell_stats_on_birth(void)
{
    str_map_ptr map = _spell_stats_map();
    str_map_clear(map);
}

void spell_stats_on_load(savefile_ptr file)
{
    str_map_ptr map = _spell_stats_map();
    int         ct, i;

    str_map_clear(map);
    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        char            name[255];
        spell_stats_ptr stats = malloc(sizeof(spell_stats_t));

        memset(stats, 0, sizeof(spell_stats_t));

        savefile_read_cptr(file, name, sizeof(name));
        stats->flags = savefile_read_u32b(file);
        stats->ct_cast = savefile_read_s32b(file);
        stats->ct_fail = savefile_read_s32b(file);
        stats->skill = savefile_read_s32b(file);
        stats->max_skill = savefile_read_s32b(file);

        str_map_add(map, name, stats);
    }
}

void spell_stats_on_save(savefile_ptr file)
{
    str_map_ptr      map = _spell_stats_map();
    str_map_iter_ptr iter;

    savefile_write_s32b(file, str_map_count(map));

    for (iter = str_map_iter_alloc(map);
            str_map_iter_is_valid(iter);
            str_map_iter_next(iter))
    {
        spell_stats_ptr stats = str_map_iter_current(iter);

        savefile_write_cptr(file, str_map_iter_current_key(iter));
        savefile_write_u32b(file, stats->flags);
        savefile_write_s32b(file, stats->ct_cast);
        savefile_write_s32b(file, stats->ct_fail);
        savefile_write_s32b(file, stats->skill);
        savefile_write_s32b(file, stats->max_skill);
    }
    str_map_iter_free(iter);
}

void spell_stats_on_learn(spell_info *spell, int max_skill)
{
    spell_stats_ptr stats = spell_stats(spell);

    stats->flags |= SPELL_FLAG_LEARNED;
    stats->max_skill = max_skill;
}

void spell_stats_on_cast(spell_info *spell)
{
    spell_stats_ptr stats = spell_stats(spell);

    stats->ct_cast++;
}

void spell_stats_on_fail(spell_info *spell)
{
    spell_stats_ptr stats = spell_stats(spell);
    stats->ct_fail++;
}

/* Legacy Spell System */
spell_stats_ptr spell_stats_old(int realm, int spell)
{
    cptr name = do_spell(realm, spell, SPELL_NAME);
    return spell_stats_aux(name);
}


void spell_stats_on_cast_old(int realm, int spell)
{
    spell_stats_ptr stats = spell_stats_old(realm, spell);
    stats->ct_cast++;
}

void spell_stats_on_fail_old(int realm, int spell)
{
    spell_stats_ptr stats = spell_stats_old(realm, spell);
    stats->ct_fail++;
}


/***********************************************************************
 * New Spell System ... Spells are objects (implemented as functions)
 * and can now be stored other data types (spell books, scrolls, etc).
 *
 * 'Spell' is misleading. This will be used by spells, racial powers,
 * mutations, potions, scrolls, etc.
 *
 * I'm attempting a grand unification of all player effects to allow
 * for some more code flexibility (e.g. scrolls that hold *any* spell and
 * Spells copied from scrolls into "books" for spellcasters, etc.)
 ***********************************************************************/

void default_spell(int cmd, variant *res) /* Base class */
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Unknown Spell");
        break;

    case SPELL_DESC:
    case SPELL_INFO:
    case SPELL_MUT_DESC:
        var_set_string(res, "");
        break;

    case SPELL_GAIN_MUT:
    case SPELL_LOSE_MUT:
        var_clear(res);
        break;

    case SPELL_FAIL:
    case SPELL_STOP:
    case SPELL_CONT:
        var_set_bool(res, TRUE);
        break;

    case SPELL_CAST:
        msg_print("Zap?");
        var_set_bool(res, TRUE);
        break;

    case SPELL_COST_EXTRA:
    case SPELL_FAIL_MIN:
        var_set_int(res, 0);
        break;

    case SPELL_ENERGY:
        var_set_int(res, 100);
        break;

    case SPELL_CALC_BONUS:
        var_set_bool(res, TRUE);
        break;

    case SPELL_COLOR:
        var_set_int(res, TERM_WHITE);
        break;

    case SPELL_ON_BROWSE:
        var_set_bool(res, FALSE);
        break;

    case SPELL_STAT_NAME: /* must return NULL so clients can requery with SPELL_NAME */
    default:
        var_clear(res);
        break;
    }
}

bool cast_spell(ang_spell spell)
{
    bool b;
    variant res;
    var_init(&res);
    spell(SPELL_CAST, &res);
    b = var_get_bool(&res);
    var_clear(&res);
    return b;
}

void fail_spell(ang_spell spell)
{
    variant res;
    var_init(&res);
    spell(SPELL_FAIL, &res);
    var_clear(&res);
}

int get_spell_energy(ang_spell spell)
{
    int n;
    variant res;
    var_init(&res);
    spell(SPELL_ENERGY, &res);
    n = var_get_int(&res);
    var_clear(&res);
    return n;
}

int get_spell_cost_extra(ang_spell spell)
{
    int n;
    variant res;
    var_init(&res);
    spell(SPELL_COST_EXTRA, &res);
    n = var_get_int(&res);
    var_clear(&res);
    return n;
}

int get_spell_flags(ang_spell spell)
{
    int n;
    variant res;
    var_init(&res);
    spell(SPELL_FLAGS, &res);
    n = var_get_int(&res);
    var_clear(&res);
    return n;
}

int get_spell_fail_min(ang_spell spell)
{
    int n;
    variant res;
    var_init(&res);
    spell(SPELL_FAIL_MIN, &res);
    n = var_get_int(&res);
    var_clear(&res);
    return n;
}

cptr get_spell_name(ang_spell spell)
{
    static char buf[255];
    variant v;
    var_init(&v);
    spell(SPELL_NAME, &v);
    sprintf(buf, "%s", var_get_string(&v));
    var_clear(&v);
    return buf;
}

cptr get_spell_stat_name(ang_spell spell)
{
    static char buf[255];
    variant v;
    var_init(&v);
    spell(SPELL_STAT_NAME, &v);
    if (var_is_null(&v)) /* cf default_spell above */
        spell(SPELL_NAME, &v);
    sprintf(buf, "%s", var_get_string(&v));
    var_clear(&v);
    return buf;
}

cptr get_spell_desc(ang_spell spell)
{
    static char buf[1024];
    variant v;
    var_init(&v);
    spell(SPELL_DESC, &v);
    sprintf(buf, "%s", var_get_string(&v));
    var_clear(&v);
    return buf;
}

cptr get_spell_spoiler_name(ang_spell spell)
{
    static char buf[255];
    variant v;
    var_init(&v);
    spell(SPELL_SPOIL_NAME, &v);

    if (var_is_null(&v))
        spell(SPELL_NAME, &v);

    sprintf(buf, "%s", var_get_string(&v));
    var_clear(&v);
    return buf;
}

/****************************************************************************************
 * UI Utilities
 *   choose_spell - prompt user with a list of spells, they choose one.
 *   browse_spell - show spell list, user picks spells repeatedly. describe each spell.
 ****************************************************************************************/

static int _col_height(int ct)
{
    int    result = ct;
    rect_t display = ui_menu_rect();

    display.cy -= 5; /* Room for browsing */
    if (result > display.cy)
    {
        result = (ct + 1)/2;
    }

    return result;
}

static void _list_spells(power_info* spells, int ct, int max_cost, char *labels, bool rage_hack, bool power)
{
    char temp[140];
    int  i;
    rect_t display = ui_menu_rect();
    int  col_height = _col_height(ct);
    int  col_width;
    variant name, info, color;
    bool poli = (p_ptr->pclass == CLASS_POLITICIAN);
    bool show_stats = (power || p_ptr->pclass == CLASS_WILD_TALENT);
    byte skipped = 0;

    var_init(&name);
    var_init(&info);
    var_init(&color);

    if (power) poli = FALSE;

    if (rage_hack)
    {
        ct = 8;
        col_height = _col_height(8);
    }

    Term_erase(display.x, display.y, display.cx);
    if (col_height == ct)
    {
        if (poli) put_str("Lvl   Cost   Fail   Desc", display.y, display.x + 29);
        else if (show_stats) put_str("Lvl Cost Fail Stat Desc", display.y, display.x + 29);
        else put_str("Lvl Cost Fail Desc", display.y, display.x + 29);
    }
    else if (!poli)
    {
        col_width = 42;
        put_str("Lvl Cost Fail", display.y, display.x + 29);
        put_str("Lvl Cost Fail", display.y, display.x + col_width + 29);
        show_stats = FALSE;
    }
    else
    {
        col_width = 48;
        put_str("Lvl   Cost   Fail", display.y, display.x + 29);
        put_str("Lvl   Cost   Fail", display.y, display.x + col_width + 29);
    }

    for (i = 0; i < ct; i++)
    {
        char letter = '\0';
        byte attr = TERM_WHITE;
        spell_info* spell = &spells[i].spell;
        int spell_cost = spell->cost;

        if ((rage_hack) && (spell->level == 99))
        {
            skipped++;
            continue;
        }

        var_set_int(&color, TERM_WHITE);

        (spell->fn)(SPELL_NAME, &name);
        (spell->fn)(SPELL_INFO, &info);
        (spell->fn)(SPELL_COLOR, &color);

        attr = var_get_int(&color);

        if ((labels) && (i < (int)strlen(labels))) letter = labels[i];
        else if (i < 26)
            letter = I2A(i);
        else if (i < 52)
            letter = 'A' + i - 26;
        else
            letter = '0' + i - 52;

        sprintf(temp, "  %c) ", letter);

        if (!poli)
        {
            strcat(temp, format("%-23.23s %3d %4d %3d%%",
                            var_get_string(&name),
                            spell->level,
                            spell->cost,
                            spell->fail));
        }
        else
        {
            char temp2[10];
            spell_cost = politician_get_cost(spell);
            big_num_display(spell_cost, temp2);
            strcat(temp, format("%-23.23s %3d %6s %5d%%",
                            var_get_string(&name),
                            spell->level,
                            temp2,
                            spell->fail));
            if (col_height == ct) strcat(temp, "  ");
        }

        if (show_stats)
        {
            if (spells[i].stat != A_NONE) strcat(temp, format("  %3.3s", stat_names_reduced[spells[i].stat]));
            else strcat(temp, " None");
        }

        if ((col_height == ct) && (spell->level <= p_ptr->lev))
            strcat(temp, format(" %s", var_get_string(&info)));

        if (spell->fail == 100)
            attr = TERM_L_DARK;

        if (spell_cost > max_cost)
            attr = TERM_L_DARK;

        if (spell->level > p_ptr->lev)
            attr = TERM_L_DARK;

        if ((i - skipped) < col_height)
        {
            Term_erase(display.x, display.y + i + 1 - skipped, display.cx);
            c_put_str(attr, temp, display.y + i + 1 - skipped, display.x);
        }
        else
        {
            c_put_str(attr, temp, display.y + (i - col_height) + 1 - skipped, display.x + col_width);
        }
    }
    Term_erase(display.x, display.y + col_height + 1 - skipped, display.cx);
    var_clear(&name);
    var_clear(&info);
    var_clear(&color);
}

static bool _describe_spell(spell_info *spell, int col_height)
{
    bool result = TRUE;
    variant info;

    var_init(&info);

    (spell->fn)(SPELL_ON_BROWSE, &info);
    if (!var_get_bool(&info))
    {
        char tmp[62*5];
        int i, line;
        rect_t display = ui_menu_rect();

        /* 2 lines below list of spells, 5 lines for description */
        for (i = 0; i < 7; i++)
            Term_erase(display.x, display.y + col_height + i + 2, display.cx);

        /* Get the description, and line break it (max 5 lines) */
        (spell->fn)(SPELL_DESC, &info);
        roff_to_buf(var_get_string(&info), 62, tmp, sizeof(tmp));

        line = display.y + col_height + 3;
        for(i = 0; tmp[i]; i += 1+strlen(&tmp[i]))
        {
            put_str(&tmp[i], line, display.x + 2);
            line++;
        }

        if (spell->level <= p_ptr->lev)
        {
            (spell->fn)(SPELL_INFO, &info);
            put_str(format("%^s", var_get_string(&info)), line, display.x + 2);
        }
        result = FALSE;
    }
    var_clear(&info);
    return result;
}

static void _make_sticky_label(spell_info *spell, int paikka)
{
    my_strcpy(power_labels[paikka], get_spell_spoiler_name(spell->fn), 15);
}

void wipe_labels(void)
{
    memset(power_labels, 0, sizeof(power_labels));
}

/* Mega-hack */
static int _rage_mage_count_spells(power_info *spells)
{
    int i, ct = 0;
    for (i = 0; i < 8; i++)
    {
        if ((spells[i].spell.level < 99) && (spells[i].spell.fn)) ct++;
    }
    return ct;
}

static int _choose_spell(power_info* spells, int ct, cptr verb, cptr desc, int max_cost, bool power, bool force_browsing)
{
    int choice = -1;
    int korkeus = 0;
    char prompt1[140];
    char prompt2[140];
    char prompt3[140];
    bool describe = force_browsing;
    bool inscribe = FALSE;
    char labels[100] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$%&'()*+,-./:;<=>{|}...............";
    bool rage_hack = ((desc) && (!power) && (streq("rage", desc)) && (ct == 8));

    if (power)
    {
        int i;
        bool compute_labels = FALSE;
        bool auto_labels[98] = {0};
        bool exists[MAX_POWER_LABEL] = {0};
        bool kaytetty[MAX_POWER_LABEL] = {0};
        for (i = 0; i < MAX_POWER_LABEL; i++)
        {
            if (power_labels[i][0])
            {
                compute_labels = TRUE;
                exists[i] = TRUE;
            }
        }
        if (compute_labels)
        {
            int kaytossa = 0;
            for (i = 0; i < MIN(ct, 98); i++)
            {
                char mininimi[15];
                int j;
                my_strcpy(mininimi, get_spell_spoiler_name(spells[i].spell.fn), 15);
                for (j = 0; j < MAX_POWER_LABEL; j++)
                {
                    if (!exists[j]) continue;
                    if (kaytetty[j]) continue;
                    if (streq(mininimi, power_labels[j]))
                    {
                        kaytetty[j] = TRUE;
                        auto_labels[i] = TRUE;
                        if (multicase[j] != labels[i])
                        {
                            int paikka = chrpos(multicase[j], labels);
                            if (paikka) labels[paikka - 1] = labels[i];
                            labels[i] = multicase[j];
                        }
                        break;
                    }
                }
                if ((!auto_labels[i]) && (i < MAX_POWER_LABEL))
                {
                    int paikka = chrpos(labels[i], multicase);
                    if ((paikka) && (exists[paikka - 1]))
                    {
                        int laskuri = 0;
                        for (j = 0; j < MAX_POWER_LABEL; j++)
                        {
                            int paikka2;
                            if (exists[j]) continue;
                            laskuri++;
                            if (laskuri <= kaytossa) continue;
                            kaytossa++;
                            paikka2 = chrpos(multicase[j], labels);
                            if (!paikka2)
                            {
                                labels[i] = multicase[j];
                                break;
                            }
                            else
                            {
                                char muisti = labels[i];
                                labels[i] = multicase[j];
                                labels[paikka2 - 1] = muisti;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (auto_sticky_labels)
        {
            int paikka;
            for (i = 0; i < MIN(ct, 98); i++)
            {
                if (auto_labels[i]) continue;
                paikka = chrpos(labels[i], multicase);
                if ((paikka) && (paikka <= MAX_POWER_LABEL)) _make_sticky_label(&spells[i].spell, paikka - 1);
            }
        }
    }
    else if (disciple_is_(DISCIPLE_TROIKA))
    {
        int i;
        for (i = troika_spell_hack; i < ct; i++)
        {
            labels[i] = '0' + i - troika_spell_hack;
        }
    }

    labels[MIN(ct, 98)] = '\0';

    if (force_browsing)
    {
        strnfmt(prompt2, 78, "Browse which %s?", desc);
    }
    else if (power)
    {
        strnfmt(prompt1, 78, "Use which %s? ('?' to Browse, '!' to Label) ", desc);
        strnfmt(prompt2, 78, "Browse which %s? ('?' to Use, '!' to Label) ", desc);
        strnfmt(prompt3, 78, "Label which %s? ('!' to Use, '=' to wipe inactive) ", desc);
    }
    else
    {
        strnfmt(prompt1, 78, "%s which %s? (Type '?' to Browse) ", verb, desc);
        strnfmt(prompt2, 78, "Browse which %s? (Type '?' to %s) ", desc, verb);
    }

    if (rage_hack)
    {
        int i;
        for (i = 0; i < ct; i++)
        {
            if (spells[i].spell.level == 99) labels[i] = 'i';
        }
    }

    korkeus = _col_height(rage_hack ? _rage_mage_count_spells(spells) : ct);

    for (;;)
    {
        char ch = '\0';
        int paikka;

        _list_spells(spells, ct, max_cost, labels, rage_hack, power);

        /* Prompt User */
        choice = -1;

        if (!get_com(describe ? prompt2 : inscribe ? prompt3 : prompt1, &ch, FALSE)) break;

        if ((ch == '?') && (!force_browsing))
        {
            describe = !describe;
            inscribe = FALSE;
            if (!get_com(describe ? prompt2 : prompt1, &ch, FALSE)) break;
        }

        if ((ch == '!') && (power) && (!force_browsing))
        {
            inscribe = !inscribe;
            describe = FALSE;
            if (!get_com(inscribe ? prompt3 : prompt1, &ch, FALSE)) break;
        }

        if ((rage_hack) && (ch == 'i')) continue;

        if ((inscribe) && (ch == '='))
        {
            int i;
            wipe_labels();
            for (i = 0; i < ct; i++)
            {
                int paikka = chrpos(labels[i], multicase);
                if ((paikka) && (paikka <= MAX_POWER_LABEL)) _make_sticky_label(&spells[i].spell, paikka - 1);
            }
            continue;
        }

        paikka = chrpos(ch, labels);
        if (paikka) choice = paikka - 1;
        else if (isupper(ch))
        {
            paikka = chrpos(tolower(ch), labels);
            if (paikka)
            {
                choice = paikka - 1;
               _describe_spell(&spells[choice].spell, korkeus);
                continue;
            }
        }

        /* Valid Choice? */
        if (choice < 0 || choice >= ct)
        {
            bell();
            continue;
        }

        if (describe)
        {
            _describe_spell(&spells[choice].spell, korkeus);
            continue;
        }
        else if ((inscribe) && (get_com("New label ('!' to unstick): ", &ch, FALSE)))
        {
            char muisti = labels[choice];
            paikka = chrpos(ch, multicase);
            if ((paikka) && (paikka <= MAX_POWER_LABEL))
            {
                int vanha = chrpos(ch, labels);
                if (vanha != choice + 1) /* no change needed */
                {
                    int paikka2 = chrpos(muisti, multicase);
                    if ((paikka2) && (paikka2 <= MAX_POWER_LABEL)) strcpy(power_labels[paikka2 - 1], "\0");
                    if ((vanha) && (vanha <= ct))
                    {
                        int paikka3 = chrpos(vanha, multicase);
                        labels[vanha - 1] = muisti;
                        if ((paikka3) && (paikka3 <= MAX_POWER_LABEL)) _make_sticky_label(&spells[vanha - 1].spell, paikka3 - 1);
                    }
                    labels[choice] = ch;
                    _make_sticky_label(&spells[choice].spell, paikka - 1);
                }
            }
            else if (ch == '!')
            {
                int paikka2 = chrpos(muisti, multicase);
                if ((paikka2) && (paikka2 <= MAX_POWER_LABEL)) strcpy(power_labels[paikka2 - 1], "\0");
                inscribe = FALSE;
            }
            continue;
        }

        /* Good to go! */
        break;
    }

    return choice;
}

int choose_spell(power_info* spells, int ct, cptr verb, cptr desc, int max_cost, bool power)
{
    int choice = -1;

    if (REPEAT_PULL(&choice))
    {
        if (choice >= 0 && choice < ct)
            return choice;
    }

    screen_save();

    choice = _choose_spell(spells, ct, verb, desc, max_cost, power, FALSE);
    REPEAT_PUSH(choice);

    screen_load();

    return choice;
}

void browse_spells(power_info* spells, int ct, cptr desc)
{
    screen_save();

    for(;;)
    {
        int choice = -1;

        choice = _choose_spell(spells, ct, "Use", desc, 10000, FALSE, TRUE);
        if (choice < 0 || choice >= ct) break;
        if (p_ptr->pclass == CLASS_RAGE_MAGE)
        {
            if (spells[choice].spell.level == 99) continue;
            if (_describe_spell(&spells[choice].spell, _col_height(_rage_mage_count_spells(spells)))) break;
        }
        else if (_describe_spell(&spells[choice].spell, _col_height(ct)))
            break;
    }
    screen_load();
}

int calculate_cost(int cost)
{
    int result = cost;

    if (p_ptr->dec_mana && cost > 0)
        result = MAX(1, result * 3 / 4);

    return result;
}

int calculate_fail_rate_aux(int caster_lvl, int spell_lvl, int base_fail, int stat_idx)
{
    int fail = base_fail;
    int min = 0;
    caster_info *caster_ptr = get_caster_info();

    if (caster_lvl < spell_lvl)
        return 100;

    if (caster_ptr && (caster_ptr->options & CASTER_NO_SPELL_FAIL))
        return 0;

    if (base_fail == 0)
        return 0;

    /* Adjust Fail Rate */
    fail -= 3 * (caster_lvl - spell_lvl);
    fail += p_ptr->to_m_chance;
    fail -= 3 * (adj_mag_stat[stat_idx] - 1);
    if (p_ptr->heavy_spell) fail += 20;
    if (p_ptr->easy_spell) fail -= 4;

    /* Apply Min Fail Rate */
    min = adj_mag_fail[stat_idx];

    if (caster_ptr && min < caster_ptr->min_fail)
        min = caster_ptr->min_fail;

    if (mut_present(MUT_ARCANE_MASTERY))
        fail -= 3;

    if (demigod_is_(DEMIGOD_ATHENA))
    {
        fail -= 2;
        if (min > 0)
            min -= 1;
    }

    if (fail < min) fail = min;
    if (p_ptr->stun)
        fail += 50 * p_ptr->stun / 100;

    if (fail > 95) fail = 95;

    /* Some effects violate the Min/Max Fail Rates */
    if (p_ptr->heavy_spell) fail += 5; /* Fail could go to 100% */

    if (p_ptr->easy_spell) fail--; /* 5% casters could get 4% fail rates */

    if (fail < 0) fail = 0;
    if (fail > 100) fail = 100;
    return fail;
}

int calculate_fail_rate(int level, int base_fail, int stat_idx)
{
    return calculate_fail_rate_aux(p_ptr->lev, level, base_fail, stat_idx);
}

/****************************************************************
 * Entrypoints for the world
 ****************************************************************/

static void _add_extra_costs(power_info* spells, int max)
{
    int i;
    /* Some spells give extra abilities depending on player level ...
       Ideally, these spells should scale the costs as well! */
    for (i = 0; i < max; i++)
    {
        spell_info* current = &spells[i].spell;
        current->cost += get_spell_cost_extra(current->fn);
        current->fail = MAX(current->fail, get_spell_fail_min(current->fn));
        current->cost = calculate_cost(current->cost);
    }
}

static void _add_extra_costs_powers(power_info* spells, int max)
{
    int i;
    /* Some spells give extra abilities depending on player level ...
       Ideally, these spells should scale the costs as well! */
    for (i = 0; i < max; i++)
    {
        spell_info* current = &spells[i].spell;
        current->cost += get_spell_cost_extra(current->fn);
        current->fail = MAX(current->fail, get_spell_fail_min(current->fn));
        /*Oops: Powers should not benefit from DEC_MANA or CASTER_NO_SPELL_COST!!!
        current->cost = calculate_cost(current->cost);*/
    }
}

int get_spell_table(power_info* spells, int max, bool with_class)
{
    int ct = 0;
    class_t *class_ptr = get_class();
    race_t  *race_ptr = get_race();

    if (race_ptr->get_spells != NULL) /* Monster Races ... */
    {
        if (with_class) inkey_xtra = FALSE;
        ct = get_spells_aux(spells, max, race_ptr->get_spells, TRUE);
    }
    else if (race_ptr->get_spells_fn != NULL)
    {
        if (with_class) inkey_xtra = FALSE;
        ct = get_spells_aux(spells, max, race_ptr->get_spells_fn(), TRUE);
    }
    else if ((with_class) && (p_ptr->pclass == CLASS_WILD_TALENT))
    {
        inkey_xtra = FALSE;
        ct = wild_talent_get_spells(spells);
    }
    else if ((with_class) && (class_ptr->get_spells != NULL))
    {
        if (with_class) inkey_xtra = FALSE;
        ct = get_spells_aux(spells, max, class_ptr->get_spells, TRUE);
    }
    else if ((with_class) && (class_ptr->get_spells_fn != NULL))
    {
        if (with_class) inkey_xtra = FALSE;
        ct = get_spells_aux(spells, max, class_ptr->get_spells_fn(), TRUE);
    }

    _add_extra_costs(spells, ct);
    return ct;
}

static int _puhdista(power_info* spells, int ct, byte liput)
{
    int i, confirmed_spells = 0;
    byte vanha_paikka[MAX_SPELLS] = {0};
    if (!liput) return ct; /* paranoia */
    for (i = 0; i < ct; i++)
    {
        if ((get_spell_flags(spells[i].spell.fn) & liput) == liput)
        {
            vanha_paikka[confirmed_spells] = i;
            confirmed_spells++;
        }
    }
    for (i = 0; i < ct; i++)
    {
        if ((i < confirmed_spells) && (i != vanha_paikka[i]))
        {
            spells[i].spell.level = spells[vanha_paikka[i]].spell.level;
            spells[i].spell.fail = spells[vanha_paikka[i]].spell.fail;
            spells[i].spell.cost = spells[vanha_paikka[i]].spell.cost;
            spells[i].spell.fn = spells[vanha_paikka[i]].spell.fn;
            spells[i].stat = spells[vanha_paikka[i]].stat;
        }
    }
    return confirmed_spells;
}

void do_cmd_spell_browse(void)
{
    power_info spells[MAX_SPELLS];
    caster_info *caster = get_caster_info();
    int ct = get_spell_table(spells, MAX_SPELLS, TRUE);

    if (ct == 0)
    {
        /* User probably canceled the prompt for a spellbook */
        return;
    }
    browse_spells(spells, ct, caster->magic_desc);
}

void do_cmd_spell(void)
{
    power_info spells[MAX_SPELLS];
    caster_info *caster = get_caster_info();
    bool hp_caster;
    int ct = 0;
    int choice = 0;
    int max_cost = 0;
    bool poli = (p_ptr->pclass == CLASS_POLITICIAN);
    bool _old_inkey_xtra = inkey_xtra;
    spell_problem = 0;

    inkey_xtra = TRUE;

    if (!caster)
    {
        msg_print("You cannot cast spells.");
        return;
    }

    if ((poli) && (p_ptr->lev < POLITICIAN_FIRST_SPELL)) 
    {
        msg_print("You haven't learned any political skills yet.");
        return;
    }

    if ((p_ptr->pclass == CLASS_BLOOD_KNIGHT) && ((get_race()->flags & RACE_IS_NONLIVING) || (p_ptr->no_cut)))
    {
        if (get_true_race()->flags & RACE_IS_NONLIVING) msg_print("You can no longer use bloodcraft!");
        else msg_print("You cannot use bloodcraft while transformed into a nonliving creature.");
        return;
    }

    hp_caster = ((caster->options & CASTER_USE_HP) || (p_ptr->pclass == CLASS_NINJA_LAWYER));
    if (poli) hp_caster = (politician_get_toggle() == POLLY_TOGGLE_HPCAST);

    if ((p_ptr->riding) && (player_is_ninja))
    {
        msg_print("You cannot use ninjutsu while riding!");
        return;
    }

    if (p_ptr->special_defense & KATA_MASK)
    {
        set_action(ACTION_NONE);
    }

    if (!fear_allow_magic()) spell_problem |= PWR_AFRAID;
    if (p_ptr->confused) spell_problem |= PWR_CONFUSED;

    ct = get_spell_table(spells, MAX_SPELLS, TRUE);
    if (ct == 0)
    {
        /* User probably canceled the prompt for a spellbook */
        spell_problem = 0;
        switch (p_ptr->pclass)
        {
            case CLASS_DISCIPLE:
                msg_print("The Purples have not taught you any spells yet!");
                break;
            case CLASS_BERSERKER:
                msg_print("Rargh! Go kill something for more experience!");
                break;
            case CLASS_MAULER:
                msg_print("Rargh! Go maul something for more experience!");
                break;
            case CLASS_SCOUT:
                msg_print("You have no powers yet! Why not go kill stuff?");
                break;
            case CLASS_WEAPONMASTER:
                msg_print("You need more experience. Why not kill something?");
                break;
            default: break;
        }
        return;
    }

    inkey_xtra = TRUE;

    if (spell_problem)
    {
        ct = _puhdista(spells, ct, spell_problem);
        if (ct == 0)
        {
            if (spell_problem & PWR_CONFUSED) msg_print("You are too confused!");
            else if (spell_problem & PWR_AFRAID) msg_print("You are too scared!");
            return;
        }
    }

    inkey_xtra = _old_inkey_xtra;

    if (caster->options & CASTER_USE_CONCENTRATION)
        max_cost = p_ptr->concent;
    else if (hp_caster)
        max_cost = p_ptr->chp;
    else if (poli)
        max_cost = politician_max_cost();
    else if (caster->options & CASTER_USE_AU)
        max_cost = p_ptr->au;
    else
        max_cost = p_ptr->csp;
    choice = choose_spell(spells, ct, "Use", caster->magic_desc, max_cost, FALSE);

    if (choice >= 0 && choice < ct)
    {
        spell_info *spell = &spells[choice].spell;
        int ongelma = 0;

        if (spell->level > p_ptr->lev)
        {
            msg_print("You can't use that spell yet!");
            return;
        }

        if (poli) ongelma = politician_verify_spell(spell);

        /* Verify Cost ... Note, I'm removing options for over exertion
           Also note we now pay casting costs up front for mana casters.
           If the user cancels, then we return the cost below.
        */
        if (caster->options & CASTER_USE_CONCENTRATION)
        {
            if (spell->cost > p_ptr->concent)
            {
                msg_print("You need to concentrate more to use this power.");
                return;
            }
        }
        else if (hp_caster)
        {
            if (spell->cost > p_ptr->chp)
            {
                msg_print("You do not have enough hp to use this power.");
                return;
            }
        }
        else if (caster->options & CASTER_USE_AU)
        {
            if (spell->cost > p_ptr->au)
            {
                msg_print("You do not have enough gold to use this power.");
                return;
            }
        }
        else if (ongelma == POLLY_TOGGLE_AUCAST)
        {
            msg_print("You do not have enough disposable gold to use this power.");
            return;
        }
        else if (ongelma == POLLY_TOGGLE_XPCAST)
        {
            if (prace_is_(RACE_ANDROID)) msg_print("You have taxed your construction too much to use this power now.");
            else msg_print("You do not have enough experience to draw on.");
            return;
        }
        else if (!poli)
        {
            if (spell->cost > p_ptr->csp)
            {
                msg_print("You do not have enough mana to use this power.");
                return;
            }
            p_ptr->csp -= spell->cost;
        }

        /* Check for Failure */
        if (randint0(100) < spell->fail)
        {
            sound(SOUND_FAIL); /* Doh! */
            spell_stats_on_fail(spell);
            fail_spell(spell->fn);
            if (flush_failure) flush();
            msg_print("You failed to concentrate hard enough!");
            if (prompt_on_failure) msg_print(NULL);
            if (!(caster->options & CASTER_USE_AU) && !hp_caster && !poli && demigod_is_(DEMIGOD_ATHENA) )
                p_ptr->csp += spell->cost/2;
            if (caster->on_fail != NULL)
                (caster->on_fail)(spell);
        }
        else
        {
            if (!cast_spell(spell->fn))
            {	
                /* Give back the spell cost, since the user canceled the spell
                 * There is no CASTER_USE_SP flag so we need to check all the alternatives */
                if ((!hp_caster) && (!poli) && (!(caster->options & (CASTER_USE_AU | CASTER_USE_CONCENTRATION))))
                {
                    p_ptr->csp += spell->cost;
                    p_ptr->redraw |= PR_MANA;
                }
                return;
            }
            spell_stats_on_cast(spell);
            sound(SOUND_ZAP); /* Wahoo! */
        }

        energy_use = get_spell_energy(spell->fn);
        p_inc_fatigue(MUT_EASY_TIRING2, 50 + MIN(50, spell->cost / 2));

        if (poli) /* Do nothing - let on_cast handle it */
        {
        }
        else if ((hp_caster) && spell->cost > 0)
            take_hit(DAMAGE_USELIFE, spell->cost, "concentrating too hard");
        else if ((caster->options & CASTER_USE_AU) && spell->cost > 0)
        {
            p_ptr->au -= spell->cost;
            stats_on_gold_services(spell->cost); /* ? */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
            p_ptr->redraw |= (PR_GOLD);
        }

        if (caster->on_cast != NULL)
            (caster->on_cast)(spell);

        p_ptr->redraw |= PR_MANA;
        p_ptr->redraw |= PR_HP;
        p_ptr->window |= PW_SPELL;
        spell_problem = 0; /* successful cast */
        if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_CAST);
    }
}

int get_power_table(power_info *spells)
{
    int ct = 0;
    race_t *race_ptr = get_race();
    class_t *class_ptr = get_class();

    /* Hack ... Rethink this a bit, but the alternative of hacking into
       the 'm' command is a million times worse!
       Doppelgangers need to be able to cancel their current mimicry.
       Also, add Mimic power back first so it always stays in the 'a' slot. */
    if (race_ptr->mimic && p_ptr->prace == RACE_DOPPELGANGER)
    {
        ct += get_powers_aux(spells + ct, MAX_SPELLS - ct, get_true_race()->get_powers, TRUE);
    }

    if (race_ptr->get_powers != NULL)
    {
        ct += get_powers_aux(spells + ct, MAX_SPELLS - ct, race_ptr->get_powers, TRUE);
    }
    else if (race_ptr->get_powers_fn != NULL)
    {
        ct += get_powers_aux(spells + ct, MAX_SPELLS - ct, race_ptr->get_powers_fn(), TRUE);
    }

    if (class_ptr != NULL && class_ptr->get_powers != NULL)
    {
        ct += get_powers_aux(spells + ct, MAX_SPELLS - ct, class_ptr->get_powers, TRUE);
    }
    else if (class_ptr != NULL && class_ptr->get_powers_fn != NULL)
    {
        ct += get_powers_aux(spells + ct, MAX_SPELLS - ct, class_ptr->get_powers_fn(), TRUE);
    }

    ct += mut_get_powers(spells + ct, MAX_SPELLS - ct);
    return ct;
}

byte do_cmd_power(void)
{
    power_info spells[MAX_SPELLS];
    int ct = 0;
    int choice = 0;
    int budget = p_ptr->chp;
    byte ongelma = 0;
    bool hp_only = (elemental_is_(ELEMENTAL_WATER));

    if (!fear_allow_magic()) ongelma |= PWR_AFRAID;
    if (p_ptr->confused) ongelma |= PWR_CONFUSED;

    if (!hp_only) budget += p_ptr->csp;

    ct = get_power_table(spells);

    if (ct == 0)
    {
        msg_print("You have no powers.");
        return 0; /* No energy use even if frightened! */
    }

    if (ongelma)
    {
        ct = _puhdista(spells, ct, ongelma);
        if (ct == 0)
        {
             if (ongelma & PWR_AFRAID) msg_print("You are too scared!");
             else if (ongelma & PWR_CONFUSED) msg_print("You are too confused!");
             if (flush_failure) flush();
             return ongelma;
        }
    }
    
    _add_extra_costs_powers(spells, ct);

    choice = choose_spell(spells, ct, "Use", "power", budget, TRUE);

    if (p_ptr->special_defense & (KATA_MUSOU | KATA_KOUKIJIN))
    {
        set_action(ACTION_NONE);
    }

    if (choice >= 0 && choice < ct)
    {
        spell_info *spell = &spells[choice].spell;

        if (spell->level > p_ptr->lev)
        {
            msg_print("You can't use that power yet!");
            return ongelma;
        }

        if (spell->cost > budget)
        {
            msg_print("Using this power will kill you!  Why not rest a bit first?");
            return ongelma;
        }

        /* Check for Failure */
        if (randint0(100) < spell->fail)
        {
            spell_stats_on_fail(spell);
            sound(SOUND_FAIL); /* Doh! */
            fail_spell(spell->fn);
            if (flush_failure) flush();
            msg_print("You failed to concentrate hard enough!");
            if (prompt_on_failure) msg_print(NULL);
        }
        else
        {
            if (!cast_spell(spell->fn))
                return ongelma;
            spell_stats_on_cast(spell);
            sound(SOUND_ZAP); /* Wahoo! */
        }

        energy_use = get_spell_energy(spell->fn);
        p_inc_fatigue(MUT_EASY_TIRING2, 50 + MIN(50, spell->cost / 2));

        /* Casting costs spill over into hit points */
        if (hp_only)
        {
            take_hit(DAMAGE_NOESCAPE, spell->cost, "concentrating too hard");
        }
        else if (p_ptr->csp < spell->cost)
        {
            int cost = spell->cost - p_ptr->csp;
            p_ptr->csp = 0;
            take_hit(DAMAGE_USELIFE, cost, "concentrating too hard");
        }
        else
            p_ptr->csp -= spell->cost;

        p_ptr->redraw |= (PR_MANA);
        p_ptr->redraw |= (PR_HP);
        p_ptr->window |= (PW_SPELL);
    }
    return ongelma;
}

/***********************************************************************
 * Utilities
 ***********************************************************************/
int get_powers_aux(power_info* spells, int max, power_info* table, bool calc_fail)
{
    int i;
    int ct = 0;

    if ((!table) || (!table[0].spell.fn)) return 0;

    for (i = 0; ; i++)
    {
        power_info *base = &table[i];

        if (ct >= max) break;
        if (!base->spell.fn) break;

        if ((base->spell.level <= p_ptr->lev) || (show_future_powers))
        {
            power_info* current = &spells[ct];
            int idx = (base->stat == A_NONE) ? 37 : p_ptr->stat_ind[base->stat];
            current->spell.fn = base->spell.fn;
            current->spell.level = base->spell.level;
            current->spell.cost = base->spell.cost;
            current->stat = base->stat;
            current->spell.fail = ((!calc_fail) ? base->spell.fail : calculate_fail_rate(
                base->spell.level,
                base->spell.fail,
                idx));
            ct++;
        }
    }
    return ct;
}

int get_spells_aux(power_info* spells, int max, spell_info* table, bool calc_fail)
{
    int i;
    int ct = 0;
    caster_info *caster_ptr = get_caster_info();
    int idx = p_ptr->stat_ind[(p_ptr->pclass == CLASS_NINJA_LAWYER) ? A_DEX : caster_ptr->which_stat];

    if ((!table) || (!table[0].fn)) return 0;

    /* Check for precomputed custom failrate calculations */
    if ((disciple_is_(DISCIPLE_YEQREZH)) || (p_ptr->pclass == CLASS_FORCETRAINER) ||
        (p_ptr->pclass == CLASS_PSION) || (p_ptr->pclass == CLASS_RAGE_MAGE))
    {
        calc_fail = FALSE;
    }

    for (i = 0; ; i++)
    {
        spell_info *base = &table[i];
        if (ct >= max) break;
        if (!base->fn) break;

        if ((base->level <= p_ptr->lev) || (show_future_spells) || (p_ptr->pclass == CLASS_RAGE_MAGE))
        {
            power_info* current = &spells[ct];
            current->spell.fn = base->fn;
            current->spell.level = base->level;
            current->spell.cost = base->cost;

            if (!calc_fail) current->spell.fail = base->fail;
            else current->spell.fail = calculate_fail_rate(base->level, base->fail, idx);
            current->stat = A_NONE;
            ct++;
        }
    }
    return ct;
}

int spell_stats_fail(spell_stats_ptr stats)
{
    int result = 0;
    int attempts = stats->ct_cast + stats->ct_fail;
    if (attempts)
    {
        int pct = stats->ct_fail * 1000 / attempts;
        result = pct / 10;
        if (pct % 10 >= 5)
            result++;
    }
    return result;
}

void dump_spells_aux(FILE *fff, power_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;

    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    if (character_dump_hack)
    {
        fprintf(fff, "=================================== Spells ====================================\n\n");
        fprintf(fff, "%-20.20s Lvl Cost Fail %-15.15s Cast Fail\n", "", "Desc");
    }
    else
    {
        fprintf(fff, "\n[[[[r|%-20.20s Lvl Cost Fail %-15.15s Cast Fail\n", "Spells", "Desc");
    }
    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i].spell;
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        fprintf(fff, "%-20.20s %3d %4d %3d%% %-15.15s %4d %4d %3d%%\n",
            var_get_string(&vn),
            spell->level, calculate_cost(spell->cost + var_get_int(&vc)), MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail,
            spell_stats_fail(stats)
        );
    }

    var_clear(&vn);
    var_clear(&vd);
    var_clear(&vc);
    var_clear(&vfm);
}

static void _dump_book(doc_ptr doc, int realm, int book)
{
    int          k_idx = lookup_kind(realm2tval(realm), book);
    int          i, increment = 64;
    caster_info *caster_ptr = get_caster_info();

    if ((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE)) increment = 0;
    else if (realm == p_ptr->realm1) increment = 0;
    else if (realm == p_ptr->realm2) increment = 32;

    if (realm == REALM_HISSATSU)
    {
        doc_printf(doc, "<color:G>    %-25.25s Lvl  SP %-15.15s  Cast</color>\n", k_name + k_info[k_idx].name, "Desc");
    }
    else
    {
        if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
            doc_printf(doc, "<color:G>    %-23.23s Profic Lvl  HP Fail %-15.15s  Cast Fail</color>\n", k_name + k_info[k_idx].name, "Desc");
        else
            doc_printf(doc, "<color:G>    %-23.23s Profic Lvl  SP Fail %-15.15s  Cast Fail</color>\n", k_name + k_info[k_idx].name, "Desc");
    }

    for (i = 0; i < 8; i++)
    {
        int         s_idx = book * 8 + i;
        magic_type *s_ptr;
        int	    vaikeustaso;
        int         cost;
        bool        max = FALSE;
        char        proficiency[10];
        char        info[80];
        cptr        comment;
        char        line[160];
        char        color = 'w';

        if (is_magic(realm))
            s_ptr = &mp_ptr->info[realm - 1][s_idx];
        else
            s_ptr = &technic_info[realm - MIN_TECHNIC][s_idx];

        vaikeustaso = lawyer_hack(s_ptr, LAWYER_HACK_LEVEL);
        if (vaikeustaso >= 99) continue;

        if (realm == REALM_HISSATSU)
            cost = s_ptr->smana;
        else
        {
            s16b exp = experience_of_spell(s_idx, realm);
            int  exp_level = spell_exp_level(exp);

            cost = mod_need_mana(lawyer_hack(s_ptr, LAWYER_HACK_MANA), s_idx, realm);

            max = FALSE;
            if (!increment && (exp_level == EXP_LEVEL_MASTER)) max = TRUE;
            else if ((increment == 32) && (exp_level >= EXP_LEVEL_EXPERT)) max = TRUE;
            else if ((p_ptr->pclass == CLASS_RED_MAGE) && (exp_level >= EXP_LEVEL_SKILLED)) max = TRUE;

            strncpy(proficiency, exp_level_str[exp_level], 4);
            proficiency[3] = ']';
            proficiency[4] = '\0';
        }

        strcpy(info, do_spell(realm, s_idx, SPELL_INFO));
        comment = info;

        if (p_ptr->pclass == CLASS_SORCERER || p_ptr->pclass == CLASS_RED_MAGE)
        {
            if (vaikeustaso > p_ptr->max_plv)
            {
                comment = "unknown";
                color = 'D';
            }
            else if (vaikeustaso > p_ptr->lev)
            {
                comment = "forgotten";
                color = 'y';
            }
        }
        else if ((realm == p_ptr->realm1) ?
            ((p_ptr->spell_forgotten1 & (1L << s_idx))) :
            ((p_ptr->spell_forgotten2 & (1L << s_idx))))
        {
            comment = "forgotten";
            color = 'y';
        }
        else if (!((realm == p_ptr->realm1) ?
            (p_ptr->spell_learned1 & (1L << s_idx)) :
            (p_ptr->spell_learned2 & (1L << s_idx))))
        {
            comment = "unknown";
            if (vaikeustaso > p_ptr->lev)
                color = 'D';
            else
                color = 'B';
        }
        else if (!((realm == p_ptr->realm1) ?
            (p_ptr->spell_worked1 & (1L << s_idx)) :
            (p_ptr->spell_worked2 & (1L << s_idx))))
        {
            comment = "untried";
        }

        sprintf(line, " %c) ", I2A(i));
        if (realm == REALM_HISSATSU)
        {
            spell_stats_ptr stats = spell_stats_old(realm, s_idx);
            strcat(
                line,
                format(
                    "<color:%c>%-25s %3d %3d %-15.15s %5d</color>",
                    color,
                    do_spell(realm, s_idx, SPELL_NAME),
                    vaikeustaso,
                    cost,
                    comment,
                    stats->ct_cast
                )
            );
        }
        else
        {
            spell_stats_ptr stats = spell_stats_old(realm, s_idx);
            strcat(
                line,
                format(
                    "<color:%c>%-25s%c%-4s %3d %3d %3d%% %-15.15s %5d %4d %3d%%</color>",
                    color,
                    do_spell(realm, s_idx, SPELL_NAME),
                    (max ? '!' : ' '),
                    proficiency,
                    vaikeustaso,
                    cost,
                    spell_chance(s_idx, realm),
                    comment,
                    stats->ct_cast,
                    stats->ct_fail,
                    spell_stats_fail(stats)
                )
            );
        }

        doc_printf(doc, "%s\n", line);
    }
    doc_newline(doc);
}

static bool _has_spells(int realm, int book)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        int            s_idx = book * 8 + i;
        magic_type *s_ptr;
        int vaikeustaso;

        if (is_magic(realm))
            s_ptr = &mp_ptr->info[realm - 1][s_idx];
        else
            s_ptr = &technic_info[realm - MIN_TECHNIC][s_idx];

        vaikeustaso = lawyer_hack(s_ptr, LAWYER_HACK_LEVEL);
        if (vaikeustaso >= 99) continue;

        if (p_ptr->pclass == CLASS_SORCERER || p_ptr->pclass == CLASS_RED_MAGE)
        {
            if (vaikeustaso > p_ptr->max_plv) continue;
            else if (vaikeustaso > p_ptr->lev) return TRUE;
        }
        else if ((realm == p_ptr->realm1) ?
            ((p_ptr->spell_forgotten1 & (1L << s_idx))) :
            ((p_ptr->spell_forgotten2 & (1L << s_idx))))
        {
            return TRUE;
        }
        else if (realm == p_ptr->realm1 ?
                    (p_ptr->spell_learned1 & (1L << s_idx)) :
                    (p_ptr->spell_learned2 & (1L << s_idx)))
        {
            return TRUE;
        }
    }
    return FALSE;
}

static bool _has_book(int realm, int book)
{
    int tval = realm2tval(realm);
    int sval = book;

    return pack_find_obj(tval, sval);
}

static void _dump_realm(doc_ptr doc, int realm)
{
    int i;
    bool first = TRUE;
    for (i = 0; i < 4; i++)
    {
        bool dump = FALSE;

        /* Red Mages only learn from first 2 books (except for Arcane) */
        if (p_ptr->pclass == CLASS_RED_MAGE && realm != REALM_ARCANE && i > 1) break;

        /* Necromancy probably should be a technique ... Too late now :( */
        if (p_ptr->pclass == CLASS_RED_MAGE && realm == REALM_NECROMANCY) break;

        /* Red Mages and Sorcerers don't learn spells, so make sure the user actually
           has the book in their pack (otherwise we spoil ... ) */
        if (p_ptr->pclass == CLASS_RED_MAGE || p_ptr->pclass == CLASS_SORCERER)
            dump = _has_book(realm, i);
        else
            dump = _has_spells(realm, i) || _has_book(realm, i);

        if (dump)
        {
            if (first)
            {
                doc_printf(doc, "<color:r>Realm:</color> <color:B>%s</color>\n\n", realm_names[realm]);
                first = FALSE;
            }
            _dump_book(doc, realm, i);
        }
    }
    i = virtue_mod_spell_fail(realm, 0);
    if (!first && i)
        doc_printf(doc, " Your alignment is adding <color:R>%+d%%</color> to your fail rates in this realm.\n\n", i);
}

void spellbook_character_dump(doc_ptr doc)
{
    if (p_ptr->pclass == CLASS_RAGE_MAGE)
        return; /* TODO */

    doc_printf(doc, "<topic:Spells>==================================== <color:keypress>S</color>pells ===================================\n\n");

    if (p_ptr->pclass == CLASS_RED_MAGE || p_ptr->pclass == CLASS_SORCERER)
    {
        int realm;
        for (realm = REALM_LIFE; realm <= MAX_MAGIC; realm++)
            _dump_realm(doc, realm);
    }
    else
    {
        if (p_ptr->realm1)
            _dump_realm(doc, p_ptr->realm1);
        if (p_ptr->realm2)
            _dump_realm(doc, p_ptr->realm2);
    }

    if (p_ptr->old_realm)
    {
        int i;
        for (i = 0; i < MAX_MAGIC; i++)
        {
            if (!(p_ptr->old_realm & 1L << i)) continue;
            doc_printf(doc, " You were able to use %s magic before.\n", realm_names[i+1]);
        }
        doc_newline(doc);
    }

    if (p_ptr->spells_per_round > 100)
    {
        doc_printf(doc, " You may cast %d.%02d spells per round.\n\n", p_ptr->spells_per_round/100, p_ptr->spells_per_round%100);
    }
}

void spellbook_destroy(obj_ptr obj)
{
    bool gain_expr = FALSE;
    if (!high_level_book(obj)) return;

    if (p_ptr->prace == RACE_ANDROID)
    {
    }
    else if (p_ptr->pclass == CLASS_WARRIOR || p_ptr->pclass == CLASS_BERSERKER)
    {
        gain_expr = TRUE;
    }
    else if (p_ptr->pclass == CLASS_PALADIN)
    {
        if (is_good_realm(p_ptr->realm1))
        {
            if (!is_good_realm(tval2realm(obj->tval))) gain_expr = TRUE;
        }
        else
        {
            if (is_good_realm(tval2realm(obj->tval))) gain_expr = TRUE;
        }
    }

    if (gain_expr && (p_ptr->exp < PY_MAX_EXP))
    {
        s32b tester_exp = p_ptr->max_exp / 20;
        if (tester_exp > 10000) tester_exp = 10000;
        if (obj->sval < 3) tester_exp /= 4;
        if (tester_exp<1) tester_exp = 1;

        msg_print("You feel more experienced.");
        gain_exp(tester_exp * obj->number);
    }

    if (obj->tval == TV_LIFE_BOOK)
    {
        virtue_add(VIRTUE_UNLIFE, 1);
        virtue_add(VIRTUE_VITALITY, -1);
    }
    else if ( high_level_book(obj)
           && (obj->tval == TV_DEATH_BOOK || obj->tval == TV_NECROMANCY_BOOK) )
    {
        virtue_add(VIRTUE_UNLIFE, -1);
        virtue_add(VIRTUE_VITALITY, 1);
    }
}

