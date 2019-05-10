#include "angband.h"

#include <stdlib.h>
#include <assert.h>

/* Build & Display the "Character Sheet" */

extern void py_display(void);
extern void py_display_birth(void);
extern void py_display_spells(doc_ptr doc, spell_info *table, int ct);
extern void py_display_powers(doc_ptr doc, spell_info *table, int ct);
extern void py_display_character_sheet(doc_ptr doc);
extern void py_display_dungeons(doc_ptr doc);

static void _build_general(doc_ptr doc);
static void _build_equipment(doc_ptr doc);
static void _build_melee(doc_ptr doc);
static void _build_shooting(doc_ptr doc);
static void _build_powers(doc_ptr doc);
static void _build_spells(doc_ptr doc);
static void _build_dungeons(doc_ptr doc);
static void _build_uniques(doc_ptr doc);
static void _build_virtues(doc_ptr doc);
static void _build_race_history(doc_ptr doc);
static void _build_mutations(doc_ptr doc);
static void _build_pets(doc_ptr doc);
static void _build_allies(doc_ptr doc);
static void _build_inventory(doc_ptr doc);
static void _build_quiver(doc_ptr doc);
static void _build_home(doc_ptr doc);
static void _build_museum(doc_ptr doc);
static void _build_statistics(doc_ptr doc);

static void _build_messages(doc_ptr doc);
static void _build_options(doc_ptr doc);

/********************************** Page 1 ************************************/
static void _build_general1(doc_ptr doc)
{
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    bool             patron_listed = FALSE;

    doc_printf(doc, " Name       : <color:B>%s</color>\n", player_name);
    doc_printf(doc, " Sex        : <color:B>%s</color>\n", sex_info[p_ptr->psex].title);
    doc_printf(doc, " Personality: <color:B>%s</color>\n", pers_ptr->name);

    if (race_ptr->mimic)
        doc_printf(doc, " Race       : <color:B>[%s]</color>\n", race_ptr->name);
    else
        doc_printf(doc, " Race       : <color:B>%s</color>\n", race_ptr->name);

    if (race_ptr->subname)
    {
        char nimi[26];
        int paikka;
        bool ok_name = FALSE;
        strncpy(nimi, get_race()->subname, sizeof(nimi));
        if (strlen(get_race()->subname) < 25) ok_name = TRUE;
        while (!ok_name)
        {
            paikka = strpos(",", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" the ", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" the", nimi);
            if (paikka >= 20)
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" of ", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            nimi[25] = '\0';
            break;
        }

        if (p_ptr->prace == RACE_MON_RING)
            doc_printf(doc, " Controlling: <color:B>%-26.26s</color>\n", nimi);
        else if (p_ptr->prace == RACE_MON_MIMIC)
        {
            if (p_ptr->current_r_idx == MON_MIMIC)
                doc_printf(doc, " Mimicking  : <color:B>%-26.26s</color>\n", "Nothing");
            else
                doc_printf(doc, " Mimicking  : <color:B>%-26.26s</color>\n", nimi);
        }
        else
            doc_printf(doc, " Subrace    : <color:B>%-26.26s</color>\n", nimi);
    }
    else
        doc_printf(doc, " Subrace    : <color:B>%-26.26s</color>\n", "None");

    doc_printf(doc, " Class      : <color:B>%s</color>\n", class_ptr->name);

    /* Assume Subclass and Magic are mutually exclusive ... */
    if (class_ptr->subname)
        doc_printf(doc, " Subclass   : <color:B>%-26.26s</color>\n", class_ptr->subname);
    else if (p_ptr->prace == RACE_MON_DRAGON)
    {
        dragon_realm_ptr realm = dragon_get_realm(p_ptr->dragon_realm);
        doc_printf(doc, " Realm      : <color:B>%-26.26s</color>\n", realm->name);
    }
    else if ((p_ptr->realm1) && (p_ptr->pclass != CLASS_LAWYER) && (p_ptr->pclass != CLASS_NINJA_LAWYER))
    {
        if (p_ptr->realm2)
            doc_printf(doc, " Realm      : <color:B>%s, %s</color>\n", realm_names[p_ptr->realm1], realm_names[p_ptr->realm2]);
        else
            doc_printf(doc, " Realm      : <color:B>%s</color>\n", realm_names[p_ptr->realm1]);
    }
    else if ((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || mut_present(MUT_CHAOS_GIFT))
    {
        doc_printf(doc, " Patron     : <color:B>%s</color>\n", chaos_patrons[p_ptr->chaos_patron]);
        patron_listed = TRUE;
    }
    else
        doc_newline(doc);

    if (((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || mut_present(MUT_CHAOS_GIFT)) && (!patron_listed))
        doc_printf(doc, " Patron     : <color:B>%s</color>\n", chaos_patrons[p_ptr->chaos_patron]);
    else
        doc_newline(doc);


    doc_printf(doc, " Level      : <color:G>%8d</color>\n", p_ptr->lev);
    if (p_ptr->prace == RACE_ANDROID)
    {
        doc_printf(doc, " Construct  : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_newline(doc);
    }
    else
    {
        doc_printf(doc, " Cur Exp    : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_printf(doc, " Max Exp    : <color:G>%8d</color>\n", p_ptr->max_exp);
    }
    doc_printf(doc, " Adv Exp    : <color:G>%8.8s</color>\n", p_ptr->lev >= PY_MAX_LEVEL ? "*****" : format("%d", exp_requirement(p_ptr->lev)));
    doc_newline(doc);
    doc_newline(doc);

    doc_printf(doc, " Gold       : <color:G>%8d</color>\n", p_ptr->au);
    doc_printf(doc, " Kills      : <color:G>%8d</color>\n", ct_kills());
    doc_printf(doc, " Uniques    : <color:G>%8d</color>\n", ct_uniques());
    doc_printf(doc, " Artifacts  : <color:G>%8.8s</color>\n",
                            no_artifacts ? "N/A" : format("%d+%d" , ct_artifacts(), stats_rand_art_counts.found));
    doc_newline(doc);

    {
        int day, hour, min;
        extract_day_hour_min(&day, &hour, &min);

        doc_printf(doc, " Game Day   : <color:G>%8d</color>\n", day);
        doc_printf(doc, " Game Time  : <color:G>%8.8s</color>\n", format("%d:%02d", hour, min));
    }

    update_playtime();
    doc_printf(doc, " Play Time  : <color:G>%8.8s</color>\n",
                            format("%.2lu:%.2lu", playtime/(60*60), (playtime/60)%60));
}

static void _display_skill(doc_ptr doc, cptr name, int amt, int div)
{
    skill_desc_t desc = skills_describe(amt, div);
    doc_printf(doc, "   %-11.11s: <color:%c>%s</color>", name, attr_to_attr_char(desc.color), desc.desc);
    if (p_ptr->wizard || display_skill_num || 0)
        doc_printf(doc, " (%d)", amt);
    doc_newline(doc);
}

static char _stat_color(int i, int mode)
{
    if ((!p_ptr->unwell) || ((i != A_DEX) && (i != A_CON)) || (!unwell_effect(p_ptr->unwell)))
        return ((mode == 2) || ((p_ptr->stat_use[i] < p_ptr->stat_top[i]) && (mode != 1))) ? 'y' : 'G';
    else
        return ((mode == 2) || ((p_ptr->stat_use[i] < p_ptr->stat_top[i]) && (mode != 1))) ? 'B' : 'W';
}

static void _build_general2(doc_ptr doc)
{
    string_ptr s = string_alloc();
    char       buf[255];
    int        i;

    doc_insert(doc, "   ========== Stats ==========\n");
    for (i = 0; i < MAX_STATS; i++)
    {
        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
            doc_printf(doc, "<tab:9>%3.3s", stat_names_reduced[i]);
        else
            doc_printf(doc, "<tab:9>%3.3s", stat_names[i]);

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        cnv_stat(p_ptr->stat_use[i], buf);
        doc_printf(doc, "<color:%c>%9.9s</color>\n", _stat_color(i, 0), buf);
    }

    doc_newline(doc);

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->chp , p_ptr->mmhp);
    doc_printf(doc, "<tab:9>HP   : <color:%c>%9.9s</color>\n",
                    p_ptr->chp >= p_ptr->mhp ? 'G' :
                        p_ptr->chp > (p_ptr->mmhp * hitpoint_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->csp , p_ptr->msp);
    if (elemental_is_(ELEMENTAL_WATER))
        doc_printf(doc, "<tab:9>Flow : <color:G>%9.9s</color>\n", string_buffer(s));
    else
        doc_printf(doc, "<tab:9>SP   : <color:%c>%9.9s</color>\n",
                    p_ptr->csp >= p_ptr->msp ? 'G' :
                        p_ptr->csp > (p_ptr->msp * mana_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    doc_printf(doc, "<tab:9>AC   : <color:G>%9d</color>\n", p_ptr->dis_ac + p_ptr->dis_to_a);

    /* Dump speed ... What a monster! */
    {
        int  tmp_speed = 0;
        byte attr;
        int  speed = p_ptr->pspeed-110;

        /* Hack -- Visually "undo" the Search Mode Slowdown */
        if (p_ptr->action == ACTION_SEARCH) speed += 10;

        if (speed > 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_GREEN;
            else
                attr = TERM_GREEN;
        }
        else if (i == 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_BLUE;
            else
                attr = TERM_GREEN;
        }
        else
        {
            if (!p_ptr->riding)
                attr = TERM_L_UMBER;
            else
                attr = TERM_RED;
        }

        if (!p_ptr->riding)
        {
            if (IS_FAST()) tmp_speed += 10;
            tmp_speed -= player_slow();
            if (IS_LIGHT_SPEED()) tmp_speed = 99;
        }
        else
        {
            if (MON_FAST(&m_list[p_ptr->riding])) tmp_speed += 10;
            tmp_speed -= monster_slow(&m_list[p_ptr->riding]);
        }

        string_clear(s);
        if (tmp_speed)
        {
            string_printf(s, "%+d%+d", speed-tmp_speed, tmp_speed);
            if (tmp_speed > 0)
                attr = TERM_YELLOW;
            else
                attr = TERM_VIOLET;
        }
        else
        {
            string_printf(s, "%+d", speed);
        }

        doc_printf(doc, "<tab:9>Speed: <color:%c>%9.9s</color>\n", attr_to_attr_char(attr), string_buffer(s));
    }

    doc_newline(doc);
    doc_insert(doc, "   ========== Skills =========\n");

    {
        skills_t skills = p_ptr->skills;
        int      slot = equip_find_obj(TV_BOW, SV_ANY);

        /* Patch Up Skills a bit */
        skills.thn += p_ptr->to_h_m * BTH_PLUS_ADJ;
        if (slot)
        {
            object_type *bow = equip_obj(slot);
            if (bow)
                skills.thb += (p_ptr->shooter_info.to_h + bow->to_h) * BTH_PLUS_ADJ;
        }
        if (!skills.stl)
            skills.stl = -1; /* Force "Very Bad" */

        /* Display */
        _display_skill(doc, "Melee", skills.thn, 12);
        _display_skill(doc, "Archery", skills.thb, 12);
        _display_skill(doc, "SavingThrow", skills.sav, 7);
        _display_skill(doc, "Stealth", skills.stl, 1);
        _display_skill(doc, "Perception", skills.fos, 6);
        _display_skill(doc, "Searching", skills.srh, 6);
        _display_skill(doc, "Disarming", skills.dis, 8);
        _display_skill(doc, "Device", skills.dev, 7);
    }

    string_free(s);
}

static void _build_general(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(40);
    cols[1] = doc_alloc(40);

    _build_general1(cols[0]);
    _build_general2(cols[1]);
    doc_insert_cols(doc, cols, 2, 0);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/********************************** Equipment *********************************/
static void _equippy_chars(doc_ptr doc, int col)
{
    if (equippy_chars)
    {
        int i;
        doc_printf(doc, "<tab:%d>", col);
        for (i = 1; i <= equip_max(); i++)
        {
            object_type *o_ptr = equip_obj(i);

            if (o_ptr)
            {
                byte a;
                char c;

                a = object_attr(o_ptr);
                c = object_char(o_ptr);

                doc_insert_char(doc, a, c);
            }
            else
                doc_insert_char(doc, TERM_WHITE, ' ');
        }
        doc_newline(doc);
    }
}

static void _equippy_heading_aux(doc_ptr doc, cptr heading, int col)
{
    int i;
    doc_printf(doc, " <color:G>%-11.11s</color><tab:%d>", heading, col);
    for (i = 1; i <= equip_max(); i++)
        doc_insert_char(doc, TERM_WHITE, 'a' + i - 1);
    doc_insert_char(doc, TERM_WHITE, '@');
}

static void _equippy_heading(doc_ptr doc, cptr heading, int col)
{
    _equippy_heading_aux(doc, heading, col);
    doc_newline(doc);
}

typedef struct {
    u32b py_flgs[OF_ARRAY_SIZE];
    u32b tim_py_flgs[OF_ARRAY_SIZE];
    u32b obj_flgs[EQUIP_MAX + 1][OF_ARRAY_SIZE];
} _flagzilla_t, *_flagzilla_ptr;

static _flagzilla_ptr _flagzilla_alloc(void)
{
    _flagzilla_ptr flagzilla = malloc(sizeof(_flagzilla_t));
    int            i;

    memset(flagzilla, 0, sizeof(_flagzilla_t));

    player_flags(flagzilla->py_flgs);
    tim_player_flags(flagzilla->tim_py_flgs);
    for (i = 1; i <= equip_max(); i++)
    {
        object_type *o_ptr = equip_obj(i);

        if (o_ptr)
        {
            obj_flags_display(o_ptr, flagzilla->obj_flgs[i]);
            switch (o_ptr->rune)
            {
            case RUNE_ABSORPTION:
                add_flag(flagzilla->obj_flgs[i], OF_MAGIC_RESISTANCE);
                break;
            case RUNE_SHADOW:
                if (object_is_body_armour(o_ptr) || o_ptr->tval == TV_CLOAK)
                    add_flag(flagzilla->obj_flgs[i], OF_STEALTH);
                break;
            case RUNE_HASTE:
                if (o_ptr->tval == TV_BOOTS)
                    add_flag(flagzilla->obj_flgs[i], OF_SPEED);
                break;
            }
        }
    }

    return flagzilla;
}

static void _flagzilla_free(_flagzilla_ptr flagzilla)
{
    free(flagzilla);
}

static void _build_res_flags(doc_ptr doc, int which, _flagzilla_ptr flagzilla)
{
    int i;
    int flg = res_get_object_flag(which);
    int im_flg = res_get_object_immune_flag(which);
    int vuln_flg = res_get_object_vuln_flag(which);
    int pct = res_pct_known(which);
    char color = 'w';

    doc_printf(doc, " %-11.11s: ", res_name(which));

    for (i = 1; i <= equip_max(); i++)
    {
        if (im_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], im_flg))
            doc_insert_char(doc, TERM_VIOLET, '*');
        else if (vuln_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], vuln_flg) && have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_L_DARK, '.');
        else if (vuln_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], vuln_flg))
            doc_insert_char(doc, TERM_L_RED, '-');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }

    if (im_flg != OF_INVALID && have_flag(flagzilla->py_flgs, im_flg))
        doc_insert_char(doc, TERM_VIOLET, '*');
    else if (im_flg != OF_INVALID && have_flag(flagzilla->tim_py_flgs, im_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
    {
        if (vuln_flg != OF_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
            doc_insert_char(doc, TERM_ORANGE, '#');
        else
            doc_insert_char(doc, TERM_YELLOW, '#');
    }
    else if (vuln_flg != OF_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
        doc_insert_char(doc, TERM_RED, 'v');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    if (pct == 100)
        color = 'v';
    else if (pct < 0)
        color = 'D';
    else if (res_is_low(which))
    {
        if (pct >= 72)
            color =  'r';
        else if (pct >= 65)
            color =  'R';
        else if (pct >= 50)
            color =  'y';
    }
    else
    {
        if (pct >= 45)
            color =  'r';
        else if (pct >= 40)
            color =  'R';
        else if (pct >= 30)
            color =  'y';
    }

    if (which == RES_FEAR && res_pct(RES_FEAR) < 100)
        doc_printf(doc, " %3dx", res_ct_known(which));
    else
        doc_printf(doc, " <color:%c>%3d%%</color>", color, pct);
    doc_newline(doc);
}

static void _build_curse_flags(doc_ptr doc, cptr name)
{
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 1; i <= equip_max(); i++)
    {
        object_type *o_ptr = equip_obj(i);

        if (o_ptr)
        {
            if (o_ptr->curse_flags & OFC_PERMA_CURSE)
                doc_insert_char(doc, TERM_VIOLET, '*');
            else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
                doc_insert_char(doc, TERM_L_RED, '+');
            else if (o_ptr->curse_flags & OFC_CURSED)
                doc_insert_char(doc, TERM_WHITE, '+');
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    doc_insert_char(doc, TERM_L_DARK, '.');
    doc_newline(doc);
}

static void _build_slays_imp(doc_ptr doc, cptr name, int flg, int kill_flg, _flagzilla_ptr flagzilla)
{
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 1; i <= equip_max(); i++)
    {
        if (kill_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], kill_flg))
            doc_insert_char(doc, TERM_RED, '*');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (kill_flg != OF_INVALID && have_flag(flagzilla->tim_py_flgs, kill_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
        doc_insert_char(doc, TERM_YELLOW, '+');
    else if (kill_flg != OF_INVALID && have_flag(flagzilla->py_flgs, kill_flg))
        doc_insert_char(doc, TERM_RED, '*');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    doc_newline(doc);
}

static int _build_flags_imp(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    int result = 0;
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 1; i <= equip_max(); i++)
    {
        if (have_flag(flagzilla->obj_flgs[i], flg))
        {
            doc_insert_char(doc, TERM_WHITE, '+');
            result++;
        }
        else if (dec_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], dec_flg))
        {
            doc_insert_char(doc, TERM_L_RED, '-');
            result--;
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (have_flag(flagzilla->tim_py_flgs, flg))
    {
       doc_insert_char(doc, TERM_YELLOW, '#');
       result++;
    }
    else if (have_flag(flagzilla->py_flgs, flg))
    {
        doc_insert_char(doc, TERM_WHITE, '+');
        result++;
    }
    else if (dec_flg != OF_INVALID && have_flag(flagzilla->py_flgs, dec_flg))
    {
        doc_insert_char(doc, TERM_L_RED, '-');
        result++;
    }
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    return result;
}

static void _build_flags_aura(doc_ptr doc, cptr name, int flg, _flagzilla_ptr flagzilla)
{
    if (_build_flags_imp(doc, name, flg, OF_INVALID, flagzilla))
    {
        if (flg == OF_AURA_FIRE)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_fire - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_COLD)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_cold - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_ELEC)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_elec - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_SHARDS)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_shards - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else doc_printf(doc, " %dd%d+2", 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
    }
    doc_newline(doc);
}

static void _build_flags(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    _build_flags_imp(doc, name, flg, dec_flg, flagzilla);
    doc_newline(doc);
}

static void _build_flags1(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int i;
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Resistances", 14);

    for (i = RES_BEGIN; i < RES_END; i++)
        _build_res_flags(doc, i, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Auras", 14);
    _build_flags_aura(doc, "Aura Elec", OF_AURA_ELEC, flagzilla);
    _build_flags_aura(doc, "Aura Fire", OF_AURA_FIRE, flagzilla);
    _build_flags_aura(doc, "Aura Cold", OF_AURA_COLD, flagzilla);
    _build_flags_aura(doc, "Aura Shards", OF_AURA_SHARDS, flagzilla);
    _build_flags(doc, "Revenge", OF_AURA_REVENGE, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Slays", 14);
    _build_slays_imp(doc, "Slay Evil", OF_SLAY_EVIL, OF_KILL_EVIL, flagzilla);
    _build_slays_imp(doc, "Slay Undead", OF_SLAY_UNDEAD, OF_KILL_UNDEAD, flagzilla);
    _build_slays_imp(doc, "Slay Demon", OF_SLAY_DEMON, OF_KILL_DEMON, flagzilla);
    _build_slays_imp(doc, "Slay Dragon", OF_SLAY_DRAGON, OF_KILL_DRAGON, flagzilla);
    _build_slays_imp(doc, "Slay Human", OF_SLAY_HUMAN, OF_KILL_HUMAN, flagzilla);
    _build_slays_imp(doc, "Slay Animal", OF_SLAY_ANIMAL, OF_KILL_ANIMAL, flagzilla);
    _build_slays_imp(doc, "Slay Orc", OF_SLAY_ORC, OF_KILL_ORC, flagzilla);
    _build_slays_imp(doc, "Slay Troll", OF_SLAY_TROLL, OF_KILL_TROLL, flagzilla);
    _build_slays_imp(doc, "Slay Giant", OF_SLAY_GIANT, OF_KILL_GIANT, flagzilla);
    _build_slays_imp(doc, "Slay Good", OF_SLAY_GOOD, OF_KILL_GOOD, flagzilla);
    _build_slays_imp(doc, "Slay Living", OF_SLAY_LIVING, OF_KILL_LIVING, flagzilla);
    _build_slays_imp(doc, "Acid Brand", OF_BRAND_ACID, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Elec Brand", OF_BRAND_ELEC, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Fire Brand", OF_BRAND_FIRE, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Cold Brand", OF_BRAND_COLD, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Pois Brand", OF_BRAND_POIS, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Mana Brand", OF_BRAND_MANA, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Sharpness", OF_VORPAL, OF_VORPAL2, flagzilla);
    _build_slays_imp(doc, "Stunning", OF_STUN, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Quake", OF_IMPACT, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Vampiric", OF_BRAND_VAMP, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "Chaotic", OF_BRAND_CHAOS, OF_INVALID, flagzilla);
    _build_flags(doc, "Add Blows", OF_BLOWS, OF_DEC_BLOWS, flagzilla);
    _build_flags(doc, "Blessed", OF_BLESSED, OF_INVALID, flagzilla);
    _build_flags(doc, "Riding", OF_RIDING, OF_INVALID, flagzilla);
    _build_flags(doc, "Tunnel", OF_TUNNEL, OF_INVALID, flagzilla);
    _build_flags(doc, "Throwing", OF_THROWING, OF_INVALID, flagzilla);
}

static void _display_known_count(doc_ptr doc, int total, int flg)
{
    int ct = total;
    slot_t slot;
    for (slot = 1; slot <= equip_max(); slot++)
    {
        obj_ptr obj = equip_obj(slot);
        u32b    flgs[OF_ARRAY_SIZE];
        u32b    flgs_known[OF_ARRAY_SIZE];

        if (!obj) continue;
        obj_flags(obj, flgs);
        obj_flags_known(obj, flgs_known);
        if (have_flag(flgs, flg) && !have_flag(flgs_known, flg))
            ct--;
    }
    if (ct)
        doc_printf(doc, " %3dx", ct);
    doc_newline(doc);
}
static void _build_flags2(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Abilities", 14);

    _build_flags(doc, "Speed", OF_SPEED, OF_DEC_SPEED, flagzilla);

    _build_flags_imp(doc, "Free Act", OF_FREE_ACT, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->free_act, OF_FREE_ACT);

    _build_flags_imp(doc, "See Invis", OF_SEE_INVIS, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->see_inv, OF_SEE_INVIS);

    _build_flags(doc, "Warning", OF_WARNING, OF_INVALID, flagzilla);
    _build_flags(doc, "Slow Digest", OF_SLOW_DIGEST, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "Regenerate", OF_REGEN, OF_INVALID, flagzilla);
    doc_printf(doc, " %3d%%\n", p_ptr->regen); /* TODO: Only display known amount ... but then again, you can feel this, no? */

    _build_flags(doc, "Levitation", OF_LEVITATION, OF_INVALID, flagzilla);
    _build_flags(doc, "Perm Lite", OF_LITE, OF_DARKNESS, flagzilla);
    _build_flags(doc, "Reflection", OF_REFLECT, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "Hold Life", OF_HOLD_LIFE, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->hold_life, OF_HOLD_LIFE);

    _build_flags(doc, "Life Rating", OF_LIFE, OF_DEC_LIFE, flagzilla);

    _build_flags(doc, "Dec Mana", OF_DEC_MANA, OF_INVALID, flagzilla);
    _build_flags(doc, "Easy Spell", OF_EASY_SPELL, OF_INVALID, flagzilla);
    _build_flags(doc, "Anti Magic", OF_NO_MAGIC, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "Magic Skill", OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY, flagzilla);
    if (p_ptr->device_power)
    {
        int pow = device_power_aux(100, p_ptr->device_power) - 100;
        doc_printf(doc, " %+3d%%", pow);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Spell Power", OF_SPELL_POWER, OF_DEC_SPELL_POWER, flagzilla))
    {
        int  pow = spell_power_aux(100, p_ptr->spell_power) - 100;
        doc_printf(doc, " %+3d%%", pow);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Spell Cap", OF_SPELL_CAP, OF_DEC_SPELL_CAP, flagzilla))
    {
        int cap = spell_cap_aux(100, p_ptr->spell_cap) - 100;
        doc_printf(doc, " %+3d%%", cap);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Magic Res", OF_MAGIC_RESISTANCE, OF_INVALID, flagzilla))
        doc_printf(doc, " %+3d%%", p_ptr->magic_resistance);
    doc_newline(doc);

    if (_build_flags_imp(doc, "Infravision", OF_INFRA, OF_INVALID, flagzilla))
        doc_printf(doc, " %3d'", p_ptr->see_infra * 10);
    doc_newline(doc);

    _build_flags(doc, "Stealth", OF_STEALTH, OF_DEC_STEALTH, flagzilla);
    _build_flags(doc, "Searching", OF_SEARCH, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Sustains", 14);
    _build_flags(doc, "Sust Str", OF_SUST_STR, OF_INVALID, flagzilla);
    _build_flags(doc, "Sust Int", OF_SUST_INT, OF_INVALID, flagzilla);
    _build_flags(doc, "Sust Wis", OF_SUST_WIS, OF_INVALID, flagzilla);
    _build_flags(doc, "Sust Dex", OF_SUST_DEX, OF_INVALID, flagzilla);
    _build_flags(doc, "Sust Con", OF_SUST_CON, OF_INVALID, flagzilla);
    _build_flags(doc, "Sust Chr", OF_SUST_CHR, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Detection", 14);
    _build_flags(doc, "Telepathy", OF_TELEPATHY, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Evil", OF_ESP_EVIL, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Nonliv", OF_ESP_NONLIVING, OF_INVALID, flagzilla);
	_build_flags(doc, "ESP Living", OF_ESP_LIVING, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Good", OF_ESP_GOOD, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Undead", OF_ESP_UNDEAD, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Demon", OF_ESP_DEMON, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Dragon", OF_ESP_DRAGON, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Human", OF_ESP_HUMAN, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Animal", OF_ESP_ANIMAL, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Orc", OF_ESP_ORC, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Troll", OF_ESP_TROLL, OF_INVALID, flagzilla);
    _build_flags(doc, "ESP Giant", OF_ESP_GIANT, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Curses", 14);
    _build_curse_flags(doc, "Cursed");
    _build_flags(doc, "Rnd Tele", OF_TELEPORT, OF_INVALID, flagzilla);
    _build_flags(doc, "No Tele", OF_NO_TELE, OF_INVALID, flagzilla);
    _build_flags(doc, "Drain Exp", OF_DRAIN_EXP, OF_INVALID, flagzilla);
    _build_flags(doc, "Aggravate", OF_AGGRAVATE, OF_INVALID, flagzilla);
    _build_flags(doc, "TY Curse", OF_TY_CURSE, OF_INVALID, flagzilla);
}

static void _build_stats(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int              i, j;
    char             buf[255];
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    s16b             stats[MAX_STATS] = {0};
    s16b             tim_stats[MAX_STATS] = {0};

    mut_calc_stats(stats);
    if (race_ptr->calc_stats)
        race_ptr->calc_stats(stats);
    if (class_ptr->calc_stats)
        class_ptr->calc_stats(stats);

    tim_player_stats(tim_stats);

    _equippy_chars(doc, 14);
    _equippy_heading_aux(doc, "", 14);
    doc_insert(doc, "   Base  R  C  P  E  Total\n");

    for (i = 0; i < MAX_STATS; i++)
    {
        int flg = OF_STR + i;
        int dec_flg = OF_DEC_STR + i;
        int sust_flg = OF_SUST_STR + i;
        int e_adj = 0;

        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
            doc_printf(doc, "<tab:7>%3.3s", stat_names_reduced[i]);
        else
            doc_printf(doc, "<tab:7>%3.3s", stat_names[i]);

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        /* abcdefghijkl */
        for (j = 1; j <= equip_max(); j++)
        {
            object_type *o_ptr = equip_obj(j);

            if (o_ptr)
            {
                int adj = 0;
                bool slipping = (o_ptr->marked & OM_SLIPPING) ? TRUE : FALSE;

                if (o_ptr->rune)
                {
                    s16b stats[MAX_STATS] = {0};
                    rune_calc_stats(o_ptr, stats);
                    adj += stats[i];
                }
                if (have_flag(flagzilla->obj_flgs[j], dec_flg))
                    adj = -o_ptr->pval;
                else if (have_flag(flagzilla->obj_flgs[j], flg))
                {
                    adj += o_ptr->pval;
                    if (p_ptr->prace == RACE_MON_ARMOR) adj += rag_effect_pval(o_ptr, -1, flg, FALSE);
                }

                if (adj)
                {
                    byte a = TERM_WHITE;
                    char c = '*';
                    if (abs(adj) < 10)
                        c = '0' + abs(adj);

                    if (adj < 0)
                        a = TERM_RED;
                    else
                    {
                        a = TERM_L_GREEN;
                        if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                            a = TERM_GREEN;
                    }
                    if (slipping)
                    {
                        switch (a)
                        {
                            case TERM_L_GREEN: a = TERM_L_BLUE; break;
                            case TERM_GREEN: a = TERM_BLUE; break;
                            case TERM_RED: a = TERM_L_RED; break;
                            default: break;
                        }
                    }
                    doc_insert_char(doc, a, c);
                }
                else
                {
                    byte a = TERM_L_DARK;
                    char c = '.';

                    if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                    {
                        a = TERM_GREEN;
                        c = 's';
                    }
                    doc_insert_char(doc, a, c);
                }
                if (!slipping) e_adj += adj;
            }
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        /* @ */
        if (stats[i] + tim_stats[i] != 0)
        {
            byte a = TERM_WHITE;
            char c = '*';
            int  adj = stats[i] + tim_stats[i];

            if (abs(adj) < 10)
                c = '0' + abs(adj);

            if (tim_stats[i])
                a = TERM_YELLOW;
            else if (adj > 0)
            {
                a = TERM_L_GREEN;
                if (have_flag(flagzilla->tim_py_flgs, sust_flg) || have_flag(flagzilla->py_flgs, sust_flg))
                    a = TERM_GREEN;
            }
            else if (adj < 0)
                a = TERM_RED;

            doc_insert_char(doc, a, c);
        }
        else
        {
            byte a = TERM_L_DARK;
            char c = '.';

            if (have_flag(flagzilla->tim_py_flgs, sust_flg))
            {
                a = TERM_YELLOW;
                c = 's';
            }
            else if (have_flag(flagzilla->py_flgs, sust_flg))
            {
                a = TERM_GREEN;
                c = 's';
            }
            doc_insert_char(doc, a, c);
        }

        /* Base R C P E */
        cnv_stat(p_ptr->stat_max[i], buf);
        doc_printf(doc, " <color:B>%6.6s%3d%3d%3d%3d</color>",
                    buf, race_ptr->stats[i], class_ptr->stats[i], pers_ptr->stats[i], e_adj);

        /* Total */
        cnv_stat(p_ptr->stat_top[i], buf);
        doc_printf(doc, " <color:%c>%6.6s</color>", _stat_color(i, 1), buf);

        /* Current */
        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
        {
            cnv_stat(p_ptr->stat_use[i], buf);
            doc_printf(doc, " <color:%c>%6.6s</color>", _stat_color(i, 2), buf);
        }

        doc_newline(doc);
    }
    doc_newline(doc);
}

static void _build_equipment(doc_ptr doc)
{
    bool old_use_graphics = use_graphics;

    /* It always bugged me that equippy characters didn't show in character dumps!*/
    if (equippy_chars && old_use_graphics)
    {
        use_graphics = FALSE;
        reset_visuals();
    }

    if (equip_count_used())
    {
        int slot;
        char o_name[MAX_NLEN];
        _flagzilla_ptr flagzilla = 0;

        doc_insert(doc, "<topic:Equipment>============================= Character <color:keypress>E</color>quipment =============================\n\n");
        for (slot = 1; slot <= equip_max(); slot++)
        {
            object_type *o_ptr = equip_obj(slot);
            if (!o_ptr) continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", slot - 1 + 'a', o_name);
            if (((always_dump_origins) || ((final_dump_origins) && ((p_ptr->total_winner) || (p_ptr->is_dead))))
              && (o_ptr->origin_type != ORIGIN_NONE) && (o_ptr->origin_type != ORIGIN_MIXED)
              && (!prace_is_(RACE_MON_SWORD)) && (!prace_is_(RACE_MON_ARMOR)) && (!prace_is_(RACE_MON_RING)))
            {
                doc_printf(doc, "    <indent><style:indent><color:W>");
                (void)display_origin(o_ptr, doc);
                doc_printf(doc, "</color></style></indent>\n");
            }
        }
        doc_newline(doc);


        /* Flags */
        flagzilla = _flagzilla_alloc();
        {
            doc_ptr cols[2];

            cols[0] = doc_alloc(40);
            cols[1] = doc_alloc(40);
            _build_flags1(cols[0], flagzilla);
            _build_flags2(cols[1], flagzilla);
            doc_insert_cols(doc, cols, 2, 0);
            doc_free(cols[0]);
            doc_free(cols[1]);
        }

        /* Stats */
        _build_stats(doc, flagzilla);

        _flagzilla_free(flagzilla);
    }

    if (equippy_chars && old_use_graphics)
    {
        use_graphics = TRUE;
        reset_visuals();
    }
}

/****************************** Combat ************************************/
static void _build_melee(doc_ptr doc)
{
    if (p_ptr->prace == RACE_MON_RING) return;
    if (possessor_can_attack() && !p_ptr->weapon_ct && !p_ptr->innate_attack_ct) return;
    {
        int i;
        doc_insert(doc, "<topic:Melee>==================================== <color:keypress>M</color>elee ====================================\n\n");
        for (i = 0; i < MAX_HANDS; i++)
        {
            if (p_ptr->weapon_info[i].wield_how == WIELD_NONE) continue;
            if (p_ptr->weapon_info[i].bare_hands)
                monk_display_attack_info(doc, i);
            else
                display_weapon_info(doc, i);
        }

        for (i = 0; i < p_ptr->innate_attack_ct; i++)
        {
            display_innate_attack_info(doc, i);
        }
    }
}

static void _build_shooting(doc_ptr doc)
{
    if (equip_find_obj(TV_BOW, SV_ANY) && !prace_is_(RACE_MON_JELLY) && p_ptr->shooter_info.tval_ammo != TV_NO_AMMO)
    {
        doc_insert(doc, "<topic:Shooting>=================================== <color:keypress>S</color>hooting ==================================\n\n");
        display_shooter_info(doc);
    }
}

/****************************** Magic ************************************/
void py_display_powers(doc_ptr doc, spell_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;
    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    doc_printf(doc, "<topic:Powers>==================================== <color:keypress>P</color>owers ===================================\n\n");
    doc_printf(doc, "<color:G>%-20.20s Lvl Cost Fail %-15.15s Cast Fail</color>\n", "", "Desc");
    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i];
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        doc_printf(doc, "%-20.20s %3d %4d %3d%% %-15.15s %4d %4d %3d%%\n",
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

    doc_newline(doc);
}

static void _build_powers(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = 0;
    race_t    *race_ptr = get_race();
    class_t   *class_ptr = get_class();

    if (race_ptr->get_powers)
        ct += (race_ptr->get_powers)(spells + ct, MAX_SPELLS - ct);

    if (class_ptr->get_powers)
        ct += (class_ptr->get_powers)(spells + ct, MAX_SPELLS - ct);

    ct += mut_get_powers(spells + ct, MAX_SPELLS - ct);

    py_display_powers(doc, spells, ct);
}

void py_display_spells(doc_ptr doc, spell_info *table, int ct)
{
    if (!ct) return;
    doc_printf(doc, "<topic:Spells>=================================== <color:keypress>S</color>pells ====================================\n\n");
    py_display_spells_aux(doc, table, ct);
}

void py_display_spells_aux(doc_ptr doc, spell_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;

    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    doc_printf(doc, "    <color:G>%-25.25s Lvl Cost Fail %-15.15s  Cast Fail</color>\n", "", "Desc");

    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i];
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        doc_printf(doc, " %c) %-25.25s %3d %4d %3d%% %-15.15s %5d %4d %3d%%\n",
            I2A(i),
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

    doc_newline(doc);
}

static void _build_spells(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = 0;
    race_t    *race_ptr = get_race();
    /*class_t   *class_ptr = get_class_t();*/

    if (race_ptr->get_spells)
        ct += (race_ptr->get_spells)(spells + ct, MAX_SPELLS - ct);

    /* TODO: Some classes prompt the user at this point ...
    if (class_ptr->get_spells)
        ct += (class_ptr->get_spells)(spells + ct, MAX_SPELLS - ct); */

    py_display_spells(doc, spells, ct);
}

/****************************** Miscellaneous ************************************/
static void _build_race_history(doc_ptr doc)
{
    if (p_ptr->old_race1 || p_ptr->old_race2 || p_ptr->old_race3)
    {
        int i;
        const char *slaji = get_race_aux(p_ptr->start_race, 0)->name;

        if (p_ptr->psex == p_ptr->start_sex) doc_printf(doc, "\n You were born as %s %s.\n", is_a_vowel(slaji[0]) ? "an" : "a", slaji);
        else doc_printf(doc, "\n You were born as a %s %s.\n", p_ptr->start_sex == SEX_FEMALE ? "female" : "male", slaji);
        for (i = 0; i < MAX_RACES; i++)
        {
            if (p_ptr->start_race == i) continue;
            if (i < 32)
            {
                if (!(p_ptr->old_race1 & 1L << i)) continue;
            }
            else if (i < 64)
            {
                if (!(p_ptr->old_race2 & 1L << (i-32))) continue;
            }
            else
            {
                if (!(p_ptr->old_race3 & 1L << (i-64))) continue;
            }
            {
                const char *laji = get_race_aux(i, 0)->name;
                doc_printf(doc, " You were %s %s before.\n", is_a_vowel(laji[0]) ? "an" : "a", laji);
            }
        }
        doc_newline(doc);
    }
}

static int _compare_monsters_counts(monster_race *left, monster_race *right)
{
    if (left->r_pkills < right->r_pkills)
        return -1;
    if (left->r_pkills > right->r_pkills)
        return 1;
    if (left->level < right->level)
        return -1;
    if (left->level > right->level)
        return 1;
    if (left->mexp < right->mexp)
        return -1;
    if (left->mexp > right->mexp)
        return 1;
    return 0;
}
static int _compare_monsters(monster_race *left, monster_race *right)
{
    if (left->level < right->level)
        return -1;
    if (left->level > right->level)
        return 1;
    if (left->mexp < right->mexp)
        return -1;
    if (left->mexp > right->mexp)
        return 1;
    if (left->r_pkills < right->r_pkills)
        return -1;
    if (left->r_pkills > right->r_pkills)
        return 1;
    return 0;
}
static void _build_uniques(doc_ptr doc)
{
    int ct = ct_kills();

    if (ct)
    {
        int     i;
        vec_ptr v = vec_alloc(NULL);
        int     ct_uniques_dead, ct_uniques_alive = 0;

        doc_printf(doc, "<topic:Kills>================================ Monster <color:keypress>K</color>ills ================================\n\n");

        for (i = 0; i < max_r_idx; i++)
        {
            monster_race *r_ptr = &r_info[i];
            if (!r_ptr->name) continue;
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                if (r_ptr->max_num == 0)
                    vec_add(v, r_ptr);
                /* When playing with reduce_uniques, it is helpful to know just
                 * how many you are dealing with. Skip the Arena uniques and any
                 * uniques suppressed this game, but count up the rest. */
                else if ( 0 < r_ptr->rarity && r_ptr->rarity <= 100
                       && !(r_ptr->flagsx & RFX_SUPPRESS) )
                {
                    ct_uniques_alive++;
                }
            }
        }

        ct_uniques_dead = vec_length(v);
        if (ct_uniques_dead)
        {
            doc_printf(doc, "You have defeated %d %s including %d unique monster%s in total. ",
                ct, ct == 1 ? "enemy" : "enemies",
                ct_uniques_dead, ct_uniques_dead == 1 ? "" : "s");

            if (ct_uniques_alive == 1)
                doc_insert(doc, "There is 1 unique remaining.");
            else
                doc_printf(doc, "There are %d uniques remaining.", ct_uniques_alive);

            doc_insert(doc, "\n\n");

            vec_sort(v, (vec_cmp_f)_compare_monsters);

            doc_printf(doc, "  <color:G>%-44.44s <color:R>%3s</color></color>\n", "Uniques", "Lvl");
            for (i = ct_uniques_dead - 1; i >= 0 && i >= ct_uniques_dead - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(doc, "  %-44.44s %3d\n", (r_name + r_ptr->name), r_ptr->level);
            }
        }
        else
            doc_printf(doc,"You have defeated %d %s.\n", ct, ct == 1 ? "enemy" : "enemies");

        doc_newline(doc);


        vec_clear(v);
        for (i = 0; i < max_r_idx; i++)
        {
            monster_race *r_ptr = &r_info[i];
            if (!r_ptr->name) continue;
            if (!(r_ptr->flags1 & RF1_UNIQUE))
            {
                if (r_ptr->r_pkills)
                    vec_add(v, r_ptr);
            }
        }
        ct = vec_length(v);
        if (ct)
        {
            doc_ptr cols[2] = {0};

            cols[0] = doc_alloc(40);
            cols[1] = doc_alloc(40);

            vec_sort(v, (vec_cmp_f)_compare_monsters);

            doc_printf(cols[0], "  <color:G>%-25.25s <color:R>%3s</color> %5s</color>\n", "Non-uniques", "Lvl", "Count");
            for (i = ct - 1; i >= 0 && i >= ct - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(cols[0], "  %-25.25s %3d %5d\n", (r_name + r_ptr->name), r_ptr->level, r_ptr->r_pkills);
            }
            doc_newline(cols[0]);

            vec_sort(v, (vec_cmp_f)_compare_monsters_counts);
            doc_printf(cols[1], "  <color:G>%-25.25s %3s <color:R>%5s</color></color>\n", "Non-uniques", "Lvl", "Count");
            for (i = ct - 1; i >= 0 && i >= ct - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(cols[1], "  %-25.25s %3d %5d\n", (r_name + r_ptr->name), r_ptr->level, r_ptr->r_pkills);
            }
            doc_newline(cols[1]);

            doc_insert_cols(doc, cols, 2, 0);
            doc_free(cols[0]);
            doc_free(cols[1]);
        }

        vec_free(v);
    }
}

static void _build_virtues(doc_ptr doc)
{
    if (enable_virtues)
    {	
        doc_printf(doc, "<topic:Virtues>=================================== <color:keypress>V</color>irtues ===================================\n\n");
        virtue_display(doc);
        doc_newline(doc);
    }	
}

static void _build_mutations(doc_ptr doc)
{
    if (mut_count(NULL))
    {
        doc_printf(doc, "<topic:Mutations>================================== <color:keypress>M</color>utations ==================================\n\n");
        mut_display(doc);
        doc_newline(doc);
    }
}

static void _build_pets(doc_ptr doc)
{
    int i;
    bool pet = FALSE;
    bool pet_settings = FALSE;
    char pet_name[MAX_NLEN];

    for (i = m_max - 1; i >= 1; i--)
    {
        monster_type *m_ptr = &m_list[i];

        if (!m_ptr->r_idx) continue;
        if (!is_pet(m_ptr)) continue;

        pet_settings = TRUE;
        /*if (!m_ptr->nickname && (p_ptr->riding != i)) continue;*/
        if (!pet)
        {
            doc_printf(doc, "<topic:Pets>==================================== <color:keypress>P</color>ets =====================================\n\n");
            doc_printf(doc, "  <color:G>Leading Pets</color>\n");
            pet = TRUE;
        }
        monster_desc(pet_name, m_ptr, MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE | MD_NO_PET_ABBREV);
        doc_printf(doc, "  <indent><style:indent>%s</style></indent>\n", pet_name);
    }

    if (pet_settings)
    {
        doc_printf(doc, "\n  <color:G>Options</color>\n");
        doc_printf(doc, "  Pets open doors:                    %s\n", (p_ptr->pet_extra_flags & PF_OPEN_DOORS) ? "ON" : "OFF");
        doc_printf(doc, "  Pets pick up items:                 %s\n", (p_ptr->pet_extra_flags & PF_PICKUP_ITEMS) ? "ON" : "OFF");
        doc_printf(doc, "  Allow teleport:                     %s\n", (p_ptr->pet_extra_flags & PF_TELEPORT) ? "ON" : "OFF");
        doc_printf(doc, "  Allow cast attack spell:            %s\n", (p_ptr->pet_extra_flags & PF_ATTACK_SPELL) ? "ON" : "OFF");
        doc_printf(doc, "  Allow cast summon spell:            %s\n", (p_ptr->pet_extra_flags & PF_SUMMON_SPELL) ? "ON" : "OFF");
        doc_printf(doc, "  Allow involve player in area spell: %s\n", (p_ptr->pet_extra_flags & PF_BALL_SPELL) ? "ON" : "OFF");
        if (p_ptr->wizard || easy_damage)
        {
            doc_printf(doc, "  Riding Skill:                       %d\n", skills_riding_current());
        }

        doc_newline(doc);
    }
}

static void _build_allies(doc_ptr doc)
{
    int i;
    int ally_counter = 0, guardian_counter = 0;
    vec_ptr v = vec_alloc(NULL);

    if (p_ptr->lev < 2)
    {
        vec_free(v);
        return;
    }

    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
        if (!unique_is_friend(i)) continue;
        vec_add(v, r_ptr);
        ally_counter++; 
        if (r_ptr->flags7 & RF7_GUARDIAN)
        {
            int j;
            for (j = 1; j < max_d_idx; j++)
            {
                dungeon_info_type *d_ptr = &d_info[j];
                if ((!d_ptr) || (!d_ptr->final_guardian)) continue;
                if (r_ptr->name == r_info[d_ptr->final_guardian].name)
                {
                    guardian_counter++;
                    break;
                }
            }
        }
    }

    if (ally_counter)
    {
        doc_printf(doc, "<topic:Allies>=================================== <color:keypress>A</color>llies ====================================\n\n");

        if ((!guardian_counter) || (ally_counter == 1))
        {
            doc_printf(doc, "<color:w>%d unique%s allied with you:\n\n", ally_counter, (ally_counter == 1) ? "</color> is" : "s</color> are");
        }
        else
        {
            doc_printf(doc, "%d uniques, including <color:w>%d dungeon guardian%s</color> are allied with you:\n\n", ally_counter, guardian_counter, (guardian_counter == 1) ? "," : "s,");
        }

        doc_printf(doc, "  <color:G>Uniques%39s</color>\n", "Lvl");
        vec_sort(v, (vec_cmp_f)_compare_monsters);

        for (i = ally_counter - 1; i >= 0; i--)
        {
            monster_race *r_ptr = vec_get(v, i);
            bool osuma = FALSE;
            int j;

            doc_printf(doc, "  %-42.42s %3d", (r_name + r_ptr->name), r_ptr->level);
            if (r_ptr->flags7 & RF7_GUARDIAN)
            {
                for (j = 1; j < max_d_idx && !osuma; j++)
                {
                    dungeon_info_type *d_ptr = &d_info[j];
                    if ((!d_ptr) || (!d_ptr->final_guardian)) continue;
                    if (r_ptr->name == r_info[d_ptr->final_guardian].name)
                    {
                        doc_printf(doc, " (%s)", d_name + d_ptr->name);
                        osuma = TRUE;
                    }
                }
            }
            if (!osuma)
            {
                vec_ptr qv = quests_get_random();
                for (j = 0; j < vec_length(qv) && !osuma; j++)
                {
                    quest_ptr q_ptr = vec_get(qv, j);
                    if ((!q_ptr) || (q_ptr->goal != QG_KILL_MON)) continue;
                    if (r_ptr->name == r_info[q_ptr->goal_idx].name)
                    {
                        doc_printf(doc, " (Angband %d)", q_ptr->level);
                        osuma = TRUE;
                    }
                }
                vec_free(qv);
            }
            doc_newline(doc);
        }

        doc_newline(doc);

    }
    vec_free(v);
}

/****************************** Objects ************************************/
static void _build_inventory(doc_ptr doc)
{
    slot_t slot;
    char o_name[MAX_NLEN];

    doc_printf(doc, "<topic:Inventory>============================= Character <color:keypress>I</color>nventory =============================\n\n");

    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = pack_obj(slot);
        if (!obj) continue;
        object_desc(o_name, obj, OD_COLOR_CODED);
        doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", o_name);
    }

    doc_newline(doc);
}

static void _build_quiver(doc_ptr doc)
{
    if (quiver_count(NULL))
    {
        slot_t slot;
        char o_name[MAX_NLEN];

        doc_printf(doc, "<topic:vQuiver>============================== Character Qui<color:keypress>v</color>er ===============================\n\n");

        for (slot = 1; slot <= quiver_max(); slot++)
        {
            obj_ptr obj = quiver_obj(slot);
            if (!obj) continue;
            object_desc(o_name, obj, OD_COLOR_CODED);
            doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", o_name);
        }

        doc_newline(doc);
    }
}

static void _build_home(doc_ptr doc)
{
    if (home_count(NULL))
    {
        doc_printf(doc, "<topic:Home>================================ <color:keypress>H</color>ome Inventory ===============================\n");
        doc_newline(doc);
        home_display(doc, obj_exists, 0);
        doc_newline(doc);
    }
}

static void _build_museum(doc_ptr doc)
{
    if (museum_count(NULL))
    {
        doc_printf(doc, "<topic:Museum>==================================== <color:keypress>M</color>useum ===================================\n");
        doc_newline(doc);
        museum_display(doc, obj_exists, 0);
        doc_newline(doc);
    }
}

/****************************** Statistics ************************************/
static void _object_counts_imp(doc_ptr doc, int tval, int sval)
{
    int          k_idx = lookup_kind(tval, sval);
    object_kind *k_ptr = &k_info[k_idx];

    if (k_ptr->counts.found || k_ptr->counts.bought || k_ptr->counts.used || k_ptr->counts.destroyed)
    {
        doc_printf(
            doc,
            "  %-20.20s %5d %6d %5d %5d\n",
            k_name + k_ptr->name,
            k_ptr->counts.found,
            k_ptr->counts.bought,
            k_ptr->counts.used,
            k_ptr->counts.destroyed
        );
    }
}

static void _device_counts_imp(doc_ptr doc, int tval, int effect)
{
    device_effect_info_ptr entry = device_get_effect_info(tval, effect);

    if (!entry)
        return;

    if (entry->counts.found || entry->counts.bought || entry->counts.used || entry->counts.destroyed)
    {
        effect_t effect;

        effect.power = entry->level;
        effect.difficulty = entry->level;
        effect.type = entry->type;
        effect.extra = 0;

        doc_printf(
            doc,
            "  %-20.20s %5d %6d %5d %5d\n",
            do_effect(&effect, SPELL_NAME, 0),
            entry->counts.found,
            entry->counts.bought,
            entry->counts.used,
            entry->counts.destroyed
        );
    }
}

typedef bool (*_kind_p)(int k_idx);
bool _kind_is_third_book(int k_idx) {
    if (k_info[k_idx].tval == TV_ARCANE_BOOK) return FALSE;
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END
      && k_info[k_idx].sval == 2 )
    {
        return TRUE;
    }
    return FALSE;
}
bool _kind_is_fourth_book(int k_idx) {
    if (k_info[k_idx].tval == TV_ARCANE_BOOK) return FALSE;
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END
      && k_info[k_idx].sval == 3 )
    {
        return TRUE;
    }
    return FALSE;
}

static bool _kind_is_equipment(int i) {
    int tval = k_info[i].tval;
    if ( kind_is_weapon(i)
      || tval == TV_SHIELD
      || tval == TV_BOW
      || tval == TV_RING
      || tval == TV_AMULET
      || tval == TV_LITE
      || kind_is_body_armor(i)
      || tval == TV_HELM
      || tval == TV_CLOAK
      || kind_is_helm(i)
      || tval == TV_GLOVES
      || tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}

static bool _kind_is_device(int i) {
    int tval = k_info[i].tval;
    if ( tval == TV_WAND
      || tval == TV_ROD
      || tval == TV_STAFF
      || tval == TV_POTION
      || tval == TV_SCROLL )
    {
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_corpse(int k_idx) {
    if (k_info[k_idx].tval == TV_CORPSE && k_info[k_idx].sval == SV_CORPSE)
        return TRUE;
    return FALSE;
}
static bool _kind_is_skeleton(int k_idx) {
    if (k_info[k_idx].tval == TV_CORPSE && k_info[k_idx].sval == SV_SKELETON)
        return TRUE;
    return FALSE;
}
static bool _kind_is_spellbook(int k_idx) {
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END )
    {
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_other(int k_idx) {
    int tval = k_info[k_idx].tval;
    if ( tval == TV_FOOD
      || _kind_is_corpse(k_idx)
      || _kind_is_skeleton(k_idx)
      || _kind_is_spellbook(k_idx)
      || tval == TV_SHOT
      || tval == TV_ARROW
      || tval == TV_BOLT 
      || tval == TV_CHEST )
    {
        return TRUE;
    }
    return FALSE;
}

static void _group_counts_imp(doc_ptr doc, _kind_p p, cptr text)
{
    int i;
    counts_t totals = {0};
    for (i = 0; i < max_k_idx; i++)
    {
        if (p(i))
        {
            totals.generated += k_info[i].counts.generated;
            totals.found += k_info[i].counts.found;
            totals.bought += k_info[i].counts.bought;
            totals.used += k_info[i].counts.used;
            totals.destroyed += k_info[i].counts.destroyed;
        }
    }

    if (totals.found || totals.bought || totals.used || totals.destroyed)
    {
        doc_printf(
            doc,
            "  %-20.20s %5d %6d %5d %5d\n",
            text,
            totals.found,
            totals.bought,
            totals.used,
            totals.destroyed
        );
    }
}

static void _group_counts_tval_imp(doc_ptr doc, int tval, cptr text)
{
    int i;
    counts_t totals = {0};
    for (i = 0; i < max_k_idx; i++)
    {
        if (k_info[i].tval == tval)
        {
            totals.generated += k_info[i].counts.generated;
            totals.found += k_info[i].counts.found;
            totals.bought += k_info[i].counts.bought;
            totals.used += k_info[i].counts.used;
            totals.destroyed += k_info[i].counts.destroyed;
        }
    }

    if (totals.found || totals.bought || totals.used || totals.destroyed)
    {
        doc_printf(
            doc,
            "  %-20.20s %5d %6d %5d %5d\n",
            text,
            totals.found,
            totals.bought,
            totals.used,
            totals.destroyed
        );
    }
}

static void _ego_counts_imp(doc_ptr doc, int idx, cptr text)
{
    ego_type *e_ptr = &e_info[idx];

    if (e_ptr->counts.found || e_ptr->counts.bought || e_ptr->counts.destroyed)
    {
        doc_printf(
            doc,
            "  %-20.20s %5d %6d %5d\n",
            text,
            e_ptr->counts.found,
            e_ptr->counts.bought,
            e_ptr->counts.destroyed
        );
    }
}

typedef bool (*_mon_p)(int r_idx);
static bool _mon_is_animal(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_ANIMAL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_breeder(int r_idx)
{
    if (r_info[r_idx].flags2 & RF2_MULTIPLY)
        return TRUE;
    return FALSE;
}
static bool _mon_is_demon(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_DEMON)
        return TRUE;
    return FALSE;
}
static bool _mon_is_dragon(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_DRAGON)
        return TRUE;
    return FALSE;
}
static bool _mon_is_evil(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_EVIL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_giant(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GIANT)
        return TRUE;
    return FALSE;
}
static bool _mon_is_good(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GOOD)
        return TRUE;
    return FALSE;
}
static bool _mon_is_hound(int r_idx)
{
    if (r_info[r_idx].d_char == 'Z')
        return TRUE;
    return FALSE;
}
static bool _mon_is_human(int r_idx)
{
    if (r_info[r_idx].flags2 & RF2_HUMAN)
        return TRUE;
    return FALSE;
}
static bool _mon_is_neutral(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GOOD)
        return FALSE;
    if (r_info[r_idx].flags3 & RF3_EVIL)
        return FALSE;
    return TRUE;
}
static bool _mon_is_orc(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_ORC)
        return TRUE;
    return FALSE;
}
static bool _mon_is_troll(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_TROLL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_undead(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_UNDEAD)
        return TRUE;
    return FALSE;
}
static bool _mon_is_unique(int r_idx)
{
    if (r_info[r_idx].flags1 & RF1_UNIQUE)
        return TRUE;
    return FALSE;
}
static bool _mon_is_pact(int r_idx)
{
    return warlock_is_pact_monster(&r_info[r_idx]);
}
static void _kill_counts_imp(doc_ptr doc, _mon_p p, cptr text, int total)
{
    int i;
    int kills = 0;
    for (i = 0; i < max_r_idx; i++)
    {
        if (p(i))
        {
            if (_mon_is_unique(i))
            {
                if (r_info[i].max_num == 0)
                    kills++;   /* Perhaps The Cloning Pits is messing up r_akills? */
            }
            else
                kills += r_info[i].r_pkills;
        }
    }

    if (kills)
    {
        doc_printf(
            doc,
            "  %-20.20s %5d %3d.%1d%%\n",
            text,
            kills,
            kills*100/total,
            (kills*1000/total)%10
        );
    }
}

static void _build_mon_kill_stats(doc_ptr doc)
{
    int total_kills = ct_kills_all();
    doc_printf(doc, "  <color:G>Monsters             Kills   Pct</color>\n");
    _kill_counts_imp(doc, _mon_is_animal, "Animals", total_kills);
    _kill_counts_imp(doc, _mon_is_breeder, "Breeders", total_kills);
    _kill_counts_imp(doc, _mon_is_demon, "Demons", total_kills);
    _kill_counts_imp(doc, _mon_is_dragon, "Dragons", total_kills);
    _kill_counts_imp(doc, _mon_is_giant, "Giants", total_kills);
    _kill_counts_imp(doc, _mon_is_hound, "Hounds", total_kills);
    _kill_counts_imp(doc, _mon_is_human, "Humans", total_kills);
    _kill_counts_imp(doc, _mon_is_orc, "Orcs", total_kills);
    _kill_counts_imp(doc, _mon_is_troll, "Trolls", total_kills);
    _kill_counts_imp(doc, _mon_is_undead, "Undead", total_kills);
    _kill_counts_imp(doc, _mon_is_unique, "Uniques", total_kills);
    if (p_ptr->pclass == CLASS_WARLOCK)
        _kill_counts_imp(doc, _mon_is_pact, "Pact", total_kills);
    doc_newline(doc);
    _kill_counts_imp(doc, _mon_is_evil, "Evil Monsters", total_kills);
    _kill_counts_imp(doc, _mon_is_good, "Good Monsters", total_kills);
    _kill_counts_imp(doc, _mon_is_neutral, "Neutral Monsters", total_kills);
    doc_printf(doc, "\n  %-20.20s %5d\n", "Totals", total_kills);
}

static void _spell_count_imp(doc_ptr doc, cptr heading, int ct, int total)
{
    if (!total) return;
    doc_printf(doc, "  %-20.20s %5d %3d.%1d%%\n", heading, ct,
        ct*100/total,
        (ct*1000/total)%10);
}
static void _build_mon_spell_stats(doc_ptr doc, cptr heading, mon_race_p filter)
{
    int ct_total_moves = 0;
    int ct_spell_moves = 0;
    int ct_spells = 0;
    int total_freq = 0;
    double expected_freq;
    int expected_spells;
    int allocation[MST_COUNT] = {0};
    int i, j, k;

    for (i = 1; i < max_r_idx; i++)
    {
        mon_race_ptr race = &r_info[i];
        int          moves;
        if (!race->name) continue;
        if (!race->spells) continue;
        if (filter && !filter(race)) continue;
        moves = race->r_move_turns + race->r_spell_turns;
        ct_total_moves += moves;
        ct_spell_moves += race->r_spell_turns;
        total_freq += race->spells->freq * moves;
        for (j = 0; j < MST_COUNT; j++)
        {
            mon_spell_group_ptr group = race->spells->groups[j];
            if (!group) continue;
            for (k = 0; k < group->count; k++)
            {
                mon_spell_ptr spell = &group->spells[k];
                allocation[j] += spell->lore;
                ct_spells += spell->lore; /* ct_spell_moves includes spell failures */
            }
        }
    }
    if (!ct_total_moves) return;
    doc_printf(doc, "  <color:G>%-20.20s Count   Pct</color>\n", heading);
    _spell_count_imp(doc, "Observed", ct_spell_moves, ct_total_moves);
    expected_freq = (double)total_freq/(ct_total_moves * 100.0);
    expected_spells = ct_total_moves * expected_freq; 
    doc_printf(doc, "  %-20.20s %5d %5.1f%%\n", "Expected", expected_spells, expected_freq * 100.0);
    _spell_count_imp(doc, "Failures", ct_spell_moves - ct_spells, ct_spell_moves);
    _spell_count_imp(doc, "Summon", allocation[MST_SUMMON], ct_spell_moves);
    _spell_count_imp(doc, "Heal", allocation[MST_HEAL], ct_spell_moves);
    _spell_count_imp(doc, "Escape", allocation[MST_ESCAPE], ct_spell_moves);
    _spell_count_imp(doc, "Offense",
        allocation[MST_BREATH] + allocation[MST_BALL] + allocation[MST_BOLT]
            + allocation[MST_BEAM] + allocation[MST_CURSE],
        ct_spell_moves);
    _spell_count_imp(doc, "Other",
        allocation[MST_BUFF] + allocation[MST_BIFF] + allocation[MST_ANNOY]
            + allocation[MST_TACTIC] + allocation[MST_WEIRD],
        ct_spell_moves);
}
static bool _is_unique(mon_race_ptr race) { return BOOL(race->flags1 & RF1_UNIQUE); }
/*static bool _is_hound(mon_race_ptr race) { return race->d_char == 'Z'; }*/
/*static bool _is_deep(mon_race_ptr race) { return race->level >= 60; }*/
static void _build_monster_stats(doc_ptr doc)
{
    doc_ptr cols[2];
    cols[0] = doc_alloc(40);
    cols[1] = doc_alloc(40);
    _build_mon_kill_stats(cols[0]);
    _build_mon_spell_stats(cols[1], "Spells", NULL);
    doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "Unique Spells", _is_unique);
    /*doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "Hound Spells", _is_hound);
    doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "Deep Spells", _is_deep);*/
    doc_insert_cols(doc, cols, 2, 0);
    doc_free(cols[0]);
    doc_free(cols[1]);
}
static void _build_monster_histogram(doc_ptr doc)
{
    int histogram[20] = {0};
    int i, max_bucket = 0, total = 0, running = 0;

    for (i = 0; i < max_r_idx; i++)
    {
        mon_race_ptr race = &r_info[i];
        int          bucket = MIN(19, race->level/5);
        int          amt = 0;
        if (!race->name) continue;

        if (race->flags1 & RF1_UNIQUE) /* XXX problem with r_pkills and uniques */
        {
            if (race->max_num == 0)
                amt = 1;
        }
        else
            amt = race->r_pkills;

        if (amt)
        {
            histogram[bucket] += amt;
            total += amt;
            max_bucket = MAX(bucket, max_bucket);
        }
    }
    if (!total) return;

    doc_insert(doc, "  <color:G>Level   Count</color>\n");
    for (i = 0; i <= max_bucket; i++)
    {
        int min = i*5;
        int max = min + 4;
        int ct = histogram[i];
        running += ct;
        doc_printf(doc, "  %2d - ", min);
        if (i < 19)
            doc_printf(doc, "%2d", max);
        else
            doc_insert(doc, "**");
        doc_printf(doc, " %5d %2d.%02d%% %3d.%02d%%\n", ct,
            ct * 100 / total, (ct * 10000 / total) % 100,
            running * 100 / total, (running * 10000 / total) % 100);
    }
    doc_newline(doc);
}
static void _build_statistics(doc_ptr doc)
{
    int i;
    counts_t totals = {0};

    doc_printf(doc, "<topic:Statistics>================================== <color:keypress>S</color>tatistics =================================\n\n");

    /* Gold */
    doc_insert(doc, "             <color:y>    Gold</color>\n");
    doc_printf(doc, "  Found    : <color:w>%8d</color>\n", stats_gold_counts.found);
    if (!no_selling)
        doc_printf(doc, "  Selling  : <color:w>%8d</color>\n", stats_gold_counts.selling);
    else
        doc_printf(doc, "  Alchemy  : <color:w>%8d</color>\n", stats_gold_counts.selling);
    doc_printf(doc, "  Winnings : <color:w>%8d</color> <color:G>%8d</color>\n",
        stats_gold_counts.winnings,
        stats_gold_counts.found + stats_gold_counts.selling + stats_gold_counts.winnings);
    doc_printf(doc, "  Purchases: <color:w>%8d</color>\n", stats_gold_counts.buying);
    doc_printf(doc, "  Services : <color:w>%8d</color>\n", stats_gold_counts.services);
    doc_printf(doc, "  Stolen   : <color:w>%8d</color> <color:R>%8d</color>\n",
        stats_gold_counts.stolen,
        stats_gold_counts.buying + stats_gold_counts.services + stats_gold_counts.stolen);
    cornucopia_print_stats(doc);
    doc_printf(doc, "                      <color:y>%8d</color>\n\n", p_ptr->au);

    /* Objects */
    for (i = 0; i < max_k_idx; i++)
    {
        totals.generated += k_info[i].counts.generated;
        totals.found += k_info[i].counts.found;
        totals.bought += k_info[i].counts.bought;
        totals.used += k_info[i].counts.used;
        totals.destroyed += k_info[i].counts.destroyed;
    }

    doc_printf(doc, "  <color:R>Objects Found    :</color> %6d\n", totals.found);
    doc_printf(doc, "  <color:R>Objects Bought   :</color> %6d\n", totals.bought);
    doc_printf(doc, "  <color:R>Objects Destroyed:</color> %6d\n", totals.destroyed);


    doc_printf(doc, "\n  <color:G>Equipment            Found Bought  Used  Dest</color>\n");
    _group_counts_imp(doc, kind_is_weapon, "Weapons");
    _group_counts_tval_imp(doc, TV_SHIELD, "Shields");
    _group_counts_tval_imp(doc, TV_BOW, "Bows");
    _group_counts_tval_imp(doc, TV_QUIVER, "Quivers");
    _group_counts_tval_imp(doc, TV_RING, "Rings");
    _group_counts_tval_imp(doc, TV_AMULET, "Amulets");
    _group_counts_tval_imp(doc, TV_LITE, "Lights");
    _group_counts_imp(doc, kind_is_body_armor, "Body Armor");
    _group_counts_tval_imp(doc, TV_CLOAK, "Cloaks");
    _group_counts_imp(doc, kind_is_helm, "Helmets");
    _group_counts_tval_imp(doc, TV_GLOVES, "Gloves");
    _group_counts_tval_imp(doc, TV_BOOTS, "Boots");
    _group_counts_imp(doc, _kind_is_equipment, "Totals");

    doc_printf(doc, "\n  <color:G>Devices              Found Bought  Used  Dest</color>\n");
    _group_counts_tval_imp(doc, TV_WAND, "Wands");
    _group_counts_tval_imp(doc, TV_STAFF, "Staves");
    _group_counts_tval_imp(doc, TV_ROD, "Rods");
    _group_counts_tval_imp(doc, TV_POTION, "Potions");
    _group_counts_tval_imp(doc, TV_SCROLL, "Scrolls");
    _group_counts_imp(doc, _kind_is_device, "Totals");

    doc_printf(doc, "\n  <color:G>Other                Found Bought  Used  Dest</color>\n");
    _group_counts_tval_imp(doc, TV_SHOT, "Shots");
    _group_counts_tval_imp(doc, TV_ARROW, "Arrows");
    _group_counts_tval_imp(doc, TV_BOLT, "Bolts");
    if (player_is_ninja)
        _group_counts_tval_imp(doc, TV_SPIKE, "Syuriken");
    _group_counts_imp(doc, _kind_is_spellbook, "Spellbooks");
    _group_counts_tval_imp(doc, TV_FOOD, "Food");
    _group_counts_imp(doc, _kind_is_corpse, "Corpses");
    _group_counts_imp(doc, _kind_is_skeleton, "Skeletons");
    _group_counts_tval_imp(doc, TV_CHEST, "Chests");
    _group_counts_imp(doc, _kind_is_other, "Totals");

    doc_printf(doc, "\n  <color:G>Potions              Found Bought  Used  Dest</color>\n");
    _object_counts_imp(doc, TV_POTION, SV_POTION_CURE_CRITICAL);
    _object_counts_imp(doc, TV_POTION, SV_POTION_CURING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_SPEED);
    _object_counts_imp(doc, TV_POTION, SV_POTION_CLARITY);
    _object_counts_imp(doc, TV_POTION, SV_POTION_HEALING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_STAR_HEALING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_LIFE);
    _object_counts_imp(doc, TV_POTION, SV_POTION_RESTORE_MANA);
    _object_counts_imp(doc, TV_POTION, SV_POTION_RESTORE_EXP);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_STR);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_INT);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_WIS);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_DEX);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_CON);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_CHR);
    _object_counts_imp(doc, TV_POTION, SV_POTION_NEW_LIFE);
    _object_counts_imp(doc, TV_POTION, SV_POTION_EXPERIENCE);
    _group_counts_tval_imp(doc, TV_POTION, "Totals");

    doc_printf(doc, "\n  <color:G>Scrolls              Found Bought  Used  Dest</color>\n");
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_PHASE_DOOR);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_IDENTIFY);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_IDENTIFY);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_REMOVE_CURSE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_REMOVE_CURSE);
    if (class_uses_spell_scrolls(p_ptr->pclass))
        _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_SPELL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_TELEPORT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_TELEPORT_LEVEL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_GENOCIDE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_MASS_GENOCIDE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_FOREST_CREATION);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_BANISHMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_CRAFTING);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_ACQUIREMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_ARTIFACT);
    _group_counts_tval_imp(doc, TV_SCROLL, "Totals");

    doc_printf(doc, "\n  <color:G>Wands                Found Bought  Used  Dest</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &wand_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_WAND, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_WAND, EFFECT_STONE_TO_MUD);
        _device_counts_imp(doc, TV_WAND, EFFECT_TELEPORT_AWAY);
        _device_counts_imp(doc, TV_WAND, EFFECT_DRAIN_LIFE);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_COLD);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_FIRE);
		_device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_WATER);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_ONE_MULTIHUED);
        _device_counts_imp(doc, TV_WAND, EFFECT_METEOR);
        _device_counts_imp(doc, TV_WAND, EFFECT_GENOCIDE_ONE);
        _device_counts_imp(doc, TV_WAND, EFFECT_BALL_WATER);
        _device_counts_imp(doc, TV_WAND, EFFECT_BALL_DISINTEGRATE);
        _device_counts_imp(doc, TV_WAND, EFFECT_ROCKET);
        _device_counts_imp(doc, TV_WAND, EFFECT_WALL_BUILDING);
    }
    _group_counts_tval_imp(doc, TV_WAND, "Totals");

    doc_printf(doc, "\n  <color:G>Staves               Found Bought  Used  Dest</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &staff_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_STAFF, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_STAFF, EFFECT_IDENTIFY);
        _device_counts_imp(doc, TV_STAFF, EFFECT_ENLIGHTENMENT);
        _device_counts_imp(doc, TV_STAFF, EFFECT_CURING);
        _device_counts_imp(doc, TV_STAFF, EFFECT_HEAL);
        _device_counts_imp(doc, TV_STAFF, EFFECT_TELEPATHY);
        _device_counts_imp(doc, TV_STAFF, EFFECT_SPEED);
        _device_counts_imp(doc, TV_STAFF, EFFECT_IDENTIFY_FULL);
        _device_counts_imp(doc, TV_STAFF, EFFECT_DESTRUCTION);
        _device_counts_imp(doc, TV_STAFF, EFFECT_HEAL_CURING);
        _device_counts_imp(doc, TV_STAFF, EFFECT_GENOCIDE);
        _device_counts_imp(doc, TV_STAFF, EFFECT_CONFUSING_LITE);
        _device_counts_imp(doc, TV_STAFF, EFFECT_MANA_STORM);
        _device_counts_imp(doc, TV_STAFF, EFFECT_STARBURST);
        _device_counts_imp(doc, TV_STAFF, EFFECT_DARKNESS_STORM);
        _device_counts_imp(doc, TV_STAFF, EFFECT_RESTORE_MANA);
    }
    _group_counts_tval_imp(doc, TV_STAFF, "Totals");

    doc_printf(doc, "\n  <color:G>Rods                 Found Bought  Used  Dest</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &rod_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_ROD, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_TRAPS);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_DOOR_STAIRS);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_MONSTERS);
        _device_counts_imp(doc, TV_ROD, EFFECT_LITE_AREA);
        _device_counts_imp(doc, TV_ROD, EFFECT_RECALL);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_ALL);
        _device_counts_imp(doc, TV_ROD, EFFECT_ENLIGHTENMENT);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_SOUND);
        _device_counts_imp(doc, TV_ROD, EFFECT_SPEED_HERO);
        _device_counts_imp(doc, TV_ROD, EFFECT_HEAL_CURING_HERO);
        _device_counts_imp(doc, TV_ROD, EFFECT_RESTORING);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_MANA);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_SHARDS);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_CHAOS);
        _device_counts_imp(doc, TV_ROD, EFFECT_CLAIRVOYANCE);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_LITE);
    }
    _group_counts_tval_imp(doc, TV_ROD, "Totals");

    doc_printf(doc, "\n  <color:G>Spellbooks           Found Bought  Used  Dest</color>\n");
    _group_counts_imp(doc, _kind_is_third_book, "Third Spellbooks");
    _group_counts_imp(doc, _kind_is_fourth_book, "Fourth Spellbooks");
    _group_counts_imp(doc, kind_is_book, "Totals");

    /* Egos */
    WIPE(&totals, counts_t);
    for (i = 0; i < max_e_idx; i++)
    {
        totals.generated += e_info[i].counts.generated;
        totals.found += e_info[i].counts.found;
        totals.bought += e_info[i].counts.bought;
        totals.destroyed += e_info[i].counts.destroyed;
    }

    doc_printf(doc, "\n  <color:R>Egos Found    :</color> %6d\n", totals.found);
    doc_printf(doc,   "  <color:R>Egos Bought   :</color> %6d\n", totals.bought);
    doc_printf(doc,   "  <color:R>Egos Destroyed:</color> %6d\n", totals.destroyed);

    doc_printf(doc, "\n  <color:G>Egos                 Found Bought  Dest</color>\n");
    _ego_counts_imp(doc, EGO_RING_SPEED, "Ring of Speed");
    _ego_counts_imp(doc, EGO_JEWELRY_DEFENDER, "Jewelry (Defender)");
    _ego_counts_imp(doc, EGO_BOOTS_ELVENKIND, "Boots of Elvenkind");
    _ego_counts_imp(doc, EGO_BOOTS_SPRITE, "Boots of the Sprite");
    _ego_counts_imp(doc, EGO_BOOTS_SPEED, "Boots of Speed");
    _ego_counts_imp(doc, EGO_BOOTS_FEANOR, "Boots of Feanor");

    doc_newline(doc);
    _build_monster_stats(doc);
    if (0 || p_ptr->wizard)
        _build_monster_histogram(doc);

    if ((p_ptr->is_dead) || (p_ptr->knowledge & KNOW_HPRATE))
    {
        doc_printf(doc, "  <color:G>Life Rating</color>:  %d%%\n\n", life_rating());
    }
}

/****************************** Dungeons ************************************/
typedef dungeon_info_type *dun_ptr;
static int _cmp_d_lvl(dun_ptr l, dun_ptr r)
{
    if (l->maxdepth < r->maxdepth) return -1;
    if (l->maxdepth > r->maxdepth) return 1;
    if (l->id < r->id) return -1;
    if (l->id > r->id) return 1;
    return 0;
}
void py_display_dungeons(doc_ptr doc)
{
    int     i;
    vec_ptr v = vec_alloc(NULL);
    for (i = 1; i < max_d_idx; i++)
    {
        dun_ptr d_ptr = &d_info[i];
        if (!d_ptr->maxdepth) continue;
        if (!max_dlv[i]) continue;
        vec_add(v, d_ptr);
    }
    vec_sort(v, (vec_cmp_f)_cmp_d_lvl);
    for (i = 0; i < vec_length(v); i++)
    {
        bool    conquered;
        dun_ptr d_ptr = vec_get(v, i);

        conquered = dungeon_conquered(d_ptr->id);

        if (conquered)
            doc_printf(doc, "!<color:G>%-16s</color>: level %3d\n", d_name+d_ptr->name, max_dlv[d_ptr->id]);
        else
            doc_printf(doc, " %-16s: level %3d\n", d_name+d_ptr->name, max_dlv[d_ptr->id]);
    }
    vec_free(v);
    doc_newline(doc);

    if (p_ptr->is_dead)
    {
        if (p_ptr->total_winner)
        {
            doc_printf(doc, "<color:v>You %s after winning.</color>\n",
                streq(p_ptr->died_from, "Seppuku") ? "did Seppuku" : "retired from the adventure");
        }
        else if (py_on_surface())
        {
            doc_printf(doc, "You were killed by %s in %s.\n", p_ptr->died_from, map_name());
        }
        else if (py_in_dungeon())
        {
            doc_printf(doc, "You were killed by %s on level %d of %s.\n",
                p_ptr->died_from, dun_level, map_name());
        }
        else if (quests_get_current())
        {
            cptr quest_name;
            doc_printf(doc, "You were killed by %s in the quest '%s'.\n",
                p_ptr->died_from, lyhytnimi(quests_get_current(), &quest_name));
            free((vptr)quest_name);
        }
        else /* ??? */
        {
            doc_printf(doc, "You were killed by %s in %s.\n", p_ptr->died_from, map_name());
        }
    }
    else if (character_dungeon)
    {
        if (py_on_surface())
            doc_printf(doc, "Now, you are in %s.\n", map_name());
        else if (py_in_dungeon())
            doc_printf(doc, "Now, you are exploring level %d of %s.\n", dun_level, map_name());
        else if (quests_get_current())
        {
            cptr quest_name;
            doc_printf(doc, "Now, you are in the quest '%s'.\n", lyhytnimi(quests_get_current(), &quest_name));
            free((vptr)quest_name);
        }
        else
            doc_insert(doc, "Hmmm ... Where are you?");
    }

    if (p_ptr->last_message)
    {
        if (p_ptr->is_dead)
            doc_printf(doc, "\n Last Message: %s\n", p_ptr->last_message);
        else if (p_ptr->total_winner)
            doc_printf(doc, "\n *Winning* Message: %s\n", p_ptr->last_message);
    }
    doc_newline(doc);
}

static void _build_dungeons(doc_ptr doc)
{
    doc_printf(doc, "<topic:Dungeons>=================================== <color:keypress>D</color>ungeons ==================================\n\n");
    py_display_dungeons(doc);
}

/****************************** Messages ************************************/
static void _build_messages(doc_ptr doc)
{
    int i;
    int current_turn = 0;
    int current_row = 0;

    doc_insert(doc, "<topic:LastMessages>================================ <color:keypress>L</color>ast Messages ================================\n");
    doc_insert(doc, "<style:normal>");
    for (i = MIN(msg_count() - 1, 30); i >= 0; i--)
    {
        msg_ptr m = msg_get(i);

        if (m->turn != current_turn)
        {
            if (doc_cursor(doc).y > current_row + 1)
                doc_newline(doc);
            current_turn = m->turn;
            current_row = doc_cursor(doc).y;
        }

        doc_insert_text(doc, m->color, string_buffer(m->msg));
        if (m->count > 1)
        {
            char buf[10];
            sprintf(buf, " (x%d)", m->count);
            doc_insert_text(doc, m->color, buf);
        }
        doc_newline(doc);
    }
    doc_insert(doc, "</style>\n");
}

/******************************** Options ************************************/
static cptr _game_mode_text[GAME_MODE_MAX] = {
    "<color:G>Beginner</color>",
    "Normal",
    "XXX", 
    "<color:r>Monster</color>"
};
static void _build_options(doc_ptr doc)
{
    doc_printf(doc, "<topic:Options>=================================== <color:keypress>O</color>ptions ===================================\n\n");

    if (game_mode != GAME_MODE_NORMAL)
        doc_printf(doc, " Game Mode:          %s\n", _game_mode_text[game_mode]);

    if (coffee_break)
        doc_printf(doc, " Coffeebreak Mode:   On\n");

    if ((p_ptr->coffee_lv_revisits) || (coffee_break && p_ptr->total_winner))
    {
        if (!p_ptr->coffee_lv_revisits)
            doc_printf(doc, " Depth Revisits:     None\n");
        else if (p_ptr->coffee_lv_revisits > 250)
            doc_printf(doc, " Depth Revisits:     250+\n");
        else
            doc_printf(doc, " Depth Revisits:     %d\n", p_ptr->coffee_lv_revisits);
    }

    doc_printf(doc, " Preserve Mode:      %s\n", preserve_mode ? "On" : "Off");

    if (small_level_type <= SMALL_LVL_MAX)
         doc_printf(doc, " Level Size:         %s\n", lv_size_options[small_level_type]);

    if (easy_damage)
		doc_printf(doc, " Easy Damage Info:   On\n");

	if (easy_id)
		doc_printf(doc, " Easy Identify:      On\n");
	
	if (easy_lore)
		doc_printf(doc, " Easy Lore:          On\n");

    if (no_wilderness)
        doc_printf(doc, " Wilderness:         Off\n");

    if (ironman_shops)
        doc_printf(doc, " No Shops:           On\n");

    if ((ironman_downward) && (!coffee_break))
        doc_printf(doc, " Diving Only:        On\n");

    if (increase_density)
        doc_printf(doc, " Dense Small Levels: On\n");

    if (no_big_dungeons)
        doc_printf(doc, " Large Dungeons:     Arena Only\n");

    if (ironman_nightmare)
        doc_printf(doc, " <color:v>Nightmare Mode</color>:     On\n");

    doc_printf(doc, " Arena Levels:       %s\n", ironman_empty_levels ? "*Always*" :
                                                    empty_levels ? "Sometimes" : "Never");

    if ((single_pantheon) && (game_pantheon > 0) && (game_pantheon < PANTHEON_MAX))
        doc_printf(doc, " Pantheon:           %s\n", pant_list[game_pantheon].name);

    if (no_artifacts)
        doc_printf(doc, " No Artifacts:       On\n");
    else if (random_artifacts)
        doc_printf(doc, " Random Artifacts:   %d%%\n", random_artifact_pct);

    if (no_egos)
        doc_printf(doc, " No Egos:            On\n");

    if (reduce_uniques)
        doc_printf(doc, " Reduce Uniques:     %d%%\n", reduce_uniques_pct);

    if (no_selling)
        doc_printf(doc, " No Selling:         On\n");

    if (comp_mode)
        doc_printf(doc, " Competition Mode:   On\n");

    if (p_ptr->noscore)
        doc_printf(doc, "\n <color:v>You have done something illegal.</color>\n");

    doc_newline(doc);
}

/****************************** Character Sheet ************************************/
static void _build_quests(doc_ptr doc)
{
    doc_printf(doc, "<topic:uQuests>==================================== Q<color:keypress>u</color>ests ===================================\n\n");
    quests_doc(doc);
    if (!no_wilderness || !ironman_downward || coffee_break)
    {
        doc_newline(doc);
        if (p_ptr->arena_number < 0)
        {
            if (p_ptr->arena_number <= ARENA_DEFEATED_OLD_VER)
            {
                doc_printf(doc, "  <color:G>Arena</color>: <color:v>Defeated</color>\n");
            }
            else
            {
                doc_printf(doc, "  <color:G>Arena</color>: <color:v>Defeated</color> by %s in the %d%s fight\n",
                    r_name + r_info[arena_info[-1 - p_ptr->arena_number].r_idx].name,
                    -p_ptr->arena_number, get_ordinal_number_suffix(-p_ptr->arena_number));
            }
        }
        else if (p_ptr->arena_number > MAX_ARENA_MONS + 2)
        {
            doc_printf(doc, "  <color:G>Arena</color>: <color:B>True Champion</color>\n");
        }
        else if (p_ptr->arena_number > MAX_ARENA_MONS - 1)
        {
            doc_printf(doc, "  <color:G>Arena</color>: <color:R>Champion</color>\n");
        }
        else
        {
            doc_printf(doc, "  <color:G>Arena</color>: %2d Victor%s\n",
                p_ptr->arena_number > MAX_ARENA_MONS ? MAX_ARENA_MONS : p_ptr->arena_number,
                p_ptr->arena_number != 1 ? "ies" : "y");
        }
    }
}

static int _max_depth(void)
{
    int result = 0;
    int i;

    for(i = 1; i < max_d_idx; i++)
    {
        if (!d_info[i].maxdepth) continue;
        if (d_info[i].flags1 & DF1_RANDOM) continue;
        if (!max_dlv[i]) continue;
        result = MAX(result, max_dlv[i]);
    }

    return result;
}

static bool _is_retired(void)
{
    if (p_ptr->total_winner && p_ptr->is_dead && strcmp(p_ptr->died_from, "Ripe Old Age") == 0)
        return TRUE;
    return FALSE;
}

int oook_score(void)
{
    int tulos = p_ptr->max_max_exp;
    if ((easy_damage) && ((p_ptr->total_winner) || (tulos > 6500000L))) tulos = (p_ptr->total_winner ? (tulos / 2) : ((tulos - 6500000L) / 2)) + 6500000L;
    return tulos;
}

char *version_modifier(void)
{
    if (coffee_break) return " (coffee)";
    return "";
}

static void _add_html_header(doc_ptr doc)
{
    string_ptr s = string_alloc_format("%s.html", player_base);
    string_ptr header = string_alloc();

    doc_change_name(doc, string_buffer(s));

    string_append_s(header, "<head>\n");
    string_append_s(header, " <meta name='filetype' value='character dump'>\n");
    string_printf(header,  " <meta name='variant' value='%s'>\n", VERSION_NAME);
    string_printf(header,  " <meta name='variant_version' value='%d.%d.%s%s'>\n", VER_MAJOR, VER_MINOR, VER_PATCH, version_modifier());
    string_printf(header,  " <meta name=\"character_name\" value=\"%s\">\n", player_name);
    string_printf(header,  " <meta name='race' value='%s'>\n", get_true_race()->name);
    string_printf(header,  " <meta name='class' value='%s'>\n", get_class()->name);
    string_printf(header,  " <meta name='level' value='%d'>\n", p_ptr->max_plv);
    string_printf(header,  " <meta name='experience' value='%d'>\n", oook_score());
    string_printf(header,  " <meta name='turncount' value='%d'>\n", game_turn);
    string_printf(header,  " <meta name='max_depth' value='%d'>\n", _max_depth());
    string_printf(header,  " <meta name='score' value='%d'>\n", oook_score()); /* ?? Does oook need this? */
    string_printf(header,  " <meta name='fame' value='%d'>\n", p_ptr->fame);

    /* For angband.oook.cz ... I'm not sure what is best for proper display of html dumps so I'll need to ask pav
     * Note: A retired winning player is_dead, but has died_from set to 'Ripe Old Age'.
     * Approach #1: Give oook a string status field */
    string_printf(header,  " <meta name='status' value='%s'>\n",
        p_ptr->total_winner ? "winner" : (p_ptr->is_dead ? "dead" : "alive"));
    /* Approach #2: Give oook some boolean fields */
    string_printf(header,  " <meta name='winner' value='%d'>\n", p_ptr->total_winner ? 1 : 0);
    string_printf(header,  " <meta name='dead' value='%d'>\n", p_ptr->is_dead ? 1 : 0);
    string_printf(header,  " <meta name='retired' value='%d'>\n", _is_retired() ? 1 : 0);

    if (p_ptr->is_dead)
        string_printf(header,  " <meta name='killer' value='%s'>\n", p_ptr->died_from);
    string_append_s(header, "</head>");

    doc_change_html_header(doc, string_buffer(header));

    string_free(s);
    string_free(header);
}

void py_display_character_sheet(doc_ptr doc)
{
    _add_html_header(doc);

    doc_insert(doc, "<style:wide>  [FrogComposband <$:version> Character Dump]\n");
    if (p_ptr->total_winner)
        doc_insert(doc, "              <color:B>***WINNER***</color>\n");
    else if (p_ptr->is_dead)
        doc_insert(doc, "              <color:v>***LOSER***</color>\n");
    else
        doc_newline(doc);
    doc_newline(doc);

    _build_general(doc);
    _build_equipment(doc);
    _build_melee(doc);
    _build_shooting(doc);
    _build_powers(doc);
    _build_spells(doc);

    {
        class_t *class_ptr = get_class();
        race_t  *race_ptr = get_race();
        if (class_ptr->character_dump)
            (class_ptr->character_dump)(doc);
        if (race_ptr && race_ptr->character_dump)
            race_ptr->character_dump(doc);
    }

    _build_dungeons(doc);
    _build_quests(doc);
    _build_uniques(doc);
    _build_virtues(doc);
    _build_race_history(doc);
    _build_mutations(doc);
    _build_pets(doc);
    _build_allies(doc);
    _build_inventory(doc);
    _build_quiver(doc);
    _build_home(doc);
    _build_museum(doc);
    _build_statistics(doc);
    _build_messages(doc);
    _build_options(doc);

    doc_insert(doc, "</style>");
}

void py_display(void)
{
    doc_ptr d = doc_alloc(80);

    py_display_character_sheet(d);

    screen_save();
    doc_display(d, "Character Sheet", 0);
    screen_load();

    doc_free(d);
}

/* This is used by the birth process ... Note that there
   is birth code that assumes the location of fields on the screen,
   and probably that should be cleaned up some day */
void py_display_birth(void)
{
    doc_ptr d = doc_alloc(80);
    _build_general(d);
    doc_sync_term(d, doc_range_all(d), doc_pos_create(0, 1));
    doc_free(d);
}
