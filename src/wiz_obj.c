#include "angband.h"
#include <assert.h>

#define _NONE  -1
#define _OK     0
#define _CANCEL 1

static doc_ptr _doc = NULL;

static int _inkey(void)
{
    return inkey_special(TRUE);
}

/***********************************************************************
 * Object Creation
 **********************************************************************/
void wiz_obj_create(void)
{
}

/***********************************************************************
 * Object Modification (Smithing)
 **********************************************************************/

/* Low level helpers for bit twiddling */
typedef struct {
    int flag;
    cptr name;
    object_p pred;
} _flag_info_t, *_flag_info_ptr;

static void _obj_identify_fully(object_type *o_ptr)
{
    int old_hack_status = no_karrot_hack;
    no_karrot_hack = TRUE;
    obj_identify_fully(o_ptr);
    no_karrot_hack = old_hack_status;
}

static void _toggle(object_type *o_ptr, int flag)
{
    if (have_flag(o_ptr->flags, flag)) remove_flag(o_ptr->flags, flag);
    else add_flag(o_ptr->flags, flag);
    if (is_pval_flag(flag) && have_flag(o_ptr->flags, flag) && o_ptr->pval == 0)
        o_ptr->pval = 1;
    _obj_identify_fully(o_ptr);
}

/* Level 2 Smithing functions */
static int _smith_plusses(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        if (object_is_melee_weapon(o_ptr) || object_is_ammo(o_ptr))
        {
            doc_insert(_doc, " <color:y>x</color>/<color:y>X</color>) Adjust damage dice\n");
            doc_insert(_doc, " <color:y>y</color>/<color:y>Y</color>) Adjust damage sides\n");
        }
        else if (o_ptr->tval == TV_BOW)
            doc_insert(_doc, " <color:y>x</color>/<color:y>X</color>) Adjust multiplier\n");
        else
            doc_insert(_doc, " <color:y>x</color>/<color:y>X</color>) Adjust base AC\n");
        if (!object_is_ammo(o_ptr))
            doc_insert(_doc, " <color:y>a</color>/<color:y>A</color>) Adjust AC bonus\n");
        doc_insert(_doc, " <color:y>h</color>/<color:y>H</color>) Adjust melee accuracy\n");
        doc_insert(_doc, " <color:y>d</color>/<color:y>D</color>) Adjust melee damage\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'x':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.dd > 0) copy.dd--;
                else copy.dd = 99;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult > 0) copy.mult -= 5;
                else copy.mult = 700;
            }
            else
            {
                if (copy.ac > 0) copy.ac--;
                else copy.ac = 50;
            }
            break;
        case 'X':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.dd < 99) copy.dd++;
                else copy.dd = 0;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult < 696) copy.mult += 5;
                else copy.mult = 0;
            }
            else
            {
                if (copy.ac < 50) copy.ac++;
                else copy.ac = 0;
            }
            break;
        case 'y':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.ds > 0) copy.ds--;
                else copy.ds = 99;
            }
            break;
        case 'Y':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.ds < 99) copy.ds++;
                else copy.ds = 0;
            }
            break;
        case 'h':
            if (copy.to_h > -50) copy.to_h--;
            else copy.to_h = 50;
            break;
        case 'H':
            if (copy.to_h < 50) copy.to_h++;
            else copy.to_h = -50;
            break;
        case 'd':
            if (copy.to_d > -50) copy.to_d--;
            else copy.to_d = 50;
            break;
        case 'D':
            if (copy.to_d < 50) copy.to_d++;
            else copy.to_d = -50;
            break;
        case 'a':
            if (!object_is_ammo(o_ptr))
            {
                if (copy.to_a > -50) copy.to_a--;
                else copy.to_a = 50;
            }
            break;
        case 'A':
            if (!object_is_ammo(o_ptr))
            {
                if (copy.to_a < 50) copy.to_a++;
                else copy.to_a = -50;
            }
            break;
        }
    }
}

static int _smith_stats(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < MAX_STATS; i++)
        {
            doc_printf(_doc, "   <color:y>%c</color>) %s\n", I2A(i), stat_name_true[i]);
        }
        doc_insert(_doc, " <color:y>p</color>/<color:y>P</color>) Adjust pval\n");
        doc_newline(_doc);
        doc_insert(_doc, "      Use SHIFT+choice to toggle decrement flag\n");
        doc_insert(_doc, "      Use CTRL+choice to toggle sustain flag\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        
        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'p':
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
            break;
        case 'P':
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
            break;
        }
       
        /* Toggle inc stat? */
        i = A2I(cmd);
        if (0 <= i && i < MAX_STATS)
        {
            _toggle(&copy, OF_STR + i);
            continue;
        }

        /* Toggle dec stat? */
        if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_DEC_STR + i);
                continue;
            }
        }

        /* Toggle sustain stat? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            i = A2I(c);
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_SUST_STR + i);
                continue;
            }
        }
    }
}

static bool _blows_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && o_ptr->tval != TV_BOW;
}

static bool _shots_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr);
}

static bool _weaponmastery_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr)
        && o_ptr->tval != TV_BOW;
}

typedef struct { /* Bonuses need to support DEC_* flags */
    int flag;
    int dec_flag;
    cptr name;
    object_p pred;
} _flagx_info_t, *_flagx_info_ptr;

static _flagx_info_t _bonus_flags[] = {
    { OF_BLOWS, OF_DEC_BLOWS, "Attack Speed", _blows_p },
    { OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY, "Device Skill" },
    { OF_DEVICE_POWER, OF_INVALID, "Device Power" },
    { OF_TUNNEL, OF_INVALID, "Digging" },
    { OF_XTRA_MIGHT, OF_INVALID, "Extra Might", _shots_p },
    { OF_XTRA_SHOTS, OF_INVALID, "Extra Shots", _shots_p },
    { OF_INFRA, OF_INVALID, "Infravision" },
    { OF_LIFE, OF_DEC_LIFE, "Life Rating" },
    { OF_MAGIC_RESISTANCE, OF_INVALID, "Magic Resistance" },
    { OF_SEARCH, OF_INVALID, "Searching" },
    { OF_SPEED, OF_DEC_SPEED, "Speed" },
    { OF_SPELL_POWER, OF_DEC_SPELL_POWER, "Spell Power" },
    { OF_SPELL_CAP, OF_DEC_SPELL_CAP, "Spell Capacity" },
    { OF_STEALTH, OF_DEC_STEALTH, "Stealth" },
    { OF_WEAPONMASTERY, OF_INVALID, "Weaponmastery", _weaponmastery_p },
    { OF_INVALID }
};

static int _smith_bonuses(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    vec_ptr     v = vec_alloc(NULL);
    int         result = _NONE, i;

    for (i = 0; ; i++)
    {
        _flagx_info_ptr fi = &_bonus_flags[i];
        if (fi->flag == OF_INVALID) break;
        if (fi->pred && !fi->pred(o_ptr)) continue;
        vec_add(v, fi);
    }

    while (result == _NONE)
    {
        int     cmd, split = vec_length(v); /* default to no split ... cols[1] remains empty */
        doc_ptr cols[2];

        cols[0] = doc_alloc(23);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1) / 2;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _flagx_info_ptr fi = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "   <color:y>%c</color>) %s%c\n",
                I2A(i),
                fi->name,
                fi->dec_flag != OF_INVALID ? '*' : ' '
            );
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, " <color:y>p</color>/<color:y>P</color>) Adjust pval\n");
        doc_insert(_doc, "   (*)Use SHIFT+choice to toggle decrement flag\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
        {
            *o_ptr = copy;
            result =  _OK;
        }
        else if (cmd == ESCAPE)
        {
            result = _CANCEL;
        }
        else if (cmd == 'p')
        {
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
        }
        else if (cmd == 'P')
        {
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
        }
        else if (isupper(cmd))
        {
            i = cmd - 'A';
            if (0 <= i && i < vec_length(v))
            {
                _flagx_info_ptr fi = vec_get(v, i);
                if (fi->dec_flag != OF_INVALID)
                    _toggle(&copy, fi->dec_flag);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                _flagx_info_ptr fi = vec_get(v, i);
                _toggle(&copy, fi->flag);
            }
        }
    }
    vec_free(v);
    return result;
}

static int _smith_flags(object_type* o_ptr, _flag_info_ptr flags)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    vec_ptr     v = vec_alloc(NULL);
    int         result = _NONE, i;

    for (i = 0; ; i++)
    {
        _flag_info_ptr fi = &flags[i];
        if (fi->flag == OF_INVALID) break;
        if (fi->pred && !fi->pred(o_ptr)) continue;
        vec_add(v, fi);
    }

    while (result == _NONE && vec_length(v) > 0)
    {
        int     cmd, split = vec_length(v); /* default to no split ... cols[1] remains empty */
        doc_ptr cols[2];

        cols[0] = doc_alloc(23);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1) / 2;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _flag_info_ptr fi = vec_get(v, i);
            doc_printf(cols[i < split ? 0 : 1], "   <color:y>%c</color>) %s\n", I2A(i), fi->name);
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
        {
            *o_ptr = copy;
            result =  _OK;
        }
        else if (cmd == ESCAPE)
        {
            result = _CANCEL;
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                _flag_info_ptr fi = vec_get(v, i);
                _toggle(&copy, fi->flag);
            }
        }
    }
    vec_free(v);
    return result;
}

static _flag_info_t _ability_flags[] = {
    { OF_FREE_ACT, "Free Action" },
    { OF_SEE_INVIS, "See Invisible" },
    { OF_HOLD_LIFE, "Hold Life" },
    { OF_SLOW_DIGEST, "Slow Digestion" },
    { OF_REGEN, "Regeneration" },
    { OF_DUAL_WIELDING, "Dual Wielding", object_is_gloves },
    { OF_NO_MAGIC, "Antimagic" },
    { OF_WARNING, "Warning" },
    { OF_LEVITATION, "Levitation" },
    { OF_REFLECT, "Reflection" },
    { OF_AURA_FIRE, "Aura Fire" },
    { OF_AURA_ELEC, "Aura Elec" },
    { OF_AURA_COLD, "Aura Cold" },
    { OF_AURA_SHARDS, "Aura Shards" },
    { OF_AURA_REVENGE, "Aura Revenge" },
    { OF_LITE, "Extra Light" },
    { OF_INVALID }
};

static int _smith_abilities(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _ability_flags);
}

static _flag_info_t _telepathy_flags[] = {
    { OF_TELEPATHY,     "Telepathy" },
    { OF_ESP_ANIMAL,    "Sense Animals" },
    { OF_ESP_UNDEAD,    "Sense Undead" },
    { OF_ESP_DEMON,     "Sense Demon" },
    { OF_ESP_ORC,       "Sense Orc" },
    { OF_ESP_TROLL,     "Sense Troll" },
    { OF_ESP_GIANT,     "Sense Giant" },
    { OF_ESP_DRAGON,    "Sense Dragon" },
    { OF_ESP_HUMAN,     "Sense Human" },
    { OF_ESP_EVIL,      "Sense Evil" },
    { OF_ESP_GOOD,      "Sense Good" },
    { OF_ESP_NONLIVING, "Sense Nonliving" },
	{ OF_ESP_LIVING,    "Sense Living" },
    { OF_ESP_UNIQUE,    "Sense Unique" },
    { OF_INVALID }
};

static int _smith_telepathies(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _telepathy_flags);
}

static _flag_info_t _slay_flags[] = {
    { OF_SLAY_EVIL,   "Slay Evil" },
    { OF_SLAY_GOOD,   "Slay Good" },
    { OF_SLAY_LIVING, "Slay Living" },
    { OF_SLAY_UNDEAD, "Slay Undead" },
    { OF_SLAY_DEMON,  "Slay Demon" },
    { OF_SLAY_DRAGON, "Slay Dragon" },
    { OF_SLAY_HUMAN,  "Slay Human" },
    { OF_SLAY_ANIMAL, "Slay Animal" },
    { OF_SLAY_ORC,    "Slay Orc" },
    { OF_SLAY_TROLL,  "Slay Troll" },
    { OF_SLAY_GIANT,  "Slay Giant" },
    { OF_KILL_EVIL,   "Kill Evil" },
	{ OF_KILL_GOOD,   "Kill Good" },
	{ OF_KILL_LIVING, "Kill Living" },
    { OF_KILL_UNDEAD, "Kill Undead" },
    { OF_KILL_DEMON,  "Kill Demon" },
    { OF_KILL_DRAGON, "Kill Dragon" },
    { OF_KILL_HUMAN,  "Kill Human" },
    { OF_KILL_ANIMAL, "Kill Animal" },
    { OF_KILL_ORC,    "Kill Orc" },
    { OF_KILL_TROLL,  "Kill Troll" },
    { OF_KILL_GIANT,  "Kill Giant" },
    { OF_INVALID }
};

static int _smith_slays(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _slay_flags);
}

static _flag_info_t _brand_flags[] = {
    { OF_BRAND_ACID,    "Brand Acid" },
    { OF_BRAND_ELEC,    "Brand Elec" },
    { OF_BRAND_FIRE,    "Brand Fire" },
    { OF_BRAND_COLD,    "Brand Cold" },
    { OF_BRAND_POIS,    "Brand Poison" },
    { OF_BRAND_MANA,    "Brand Mana", object_is_melee_weapon },
    { OF_BRAND_CHAOS,   "Chaotic", object_is_melee_weapon },
    { OF_BRAND_VAMP,    "Vampiric" },
    { OF_IMPACT,        "Impact", object_is_melee_weapon },
    { OF_STUN,          "Stun", object_is_melee_weapon },
    { OF_VORPAL,        "Vorpal", object_is_melee_weapon },
    { OF_VORPAL2,       "*Vorpal*", object_is_melee_weapon },
    { OF_INVALID }
};

static int _smith_brands(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _brand_flags);
}

static void _reroll_aux(object_type *o_ptr, int flags, int min)
{
    int attempts = 1 * 1000; /* param?  better gcc -O2 if more than 1k */
    int i, score, best_score = -1; /* scores are never negative */
    object_type forge, best = {0};
    bool igor_hack = ((prace_is_(RACE_IGOR)) && (o_ptr->tval == TV_CORPSE));

    for (i = 0; i < attempts; i++)
    {
        object_prep(&forge, igor_hack ? lookup_kind(TV_CORPSE, SV_CORPSE) : o_ptr->k_idx);
        if (igor_hack)
        {
            forge.pval = o_ptr->pval;
            if ((o_ptr->sval != SV_CORPSE) && (o_ptr->sval != SV_BODY_EARS)) forge.pval = o_ptr->xtra4;
            igor_dissect_corpse(&forge);
        }
        else apply_magic(&forge, dun_level, AM_NO_FIXED_ART | flags);
        _obj_identify_fully(&forge);

        score = obj_value_real(&forge);
        if (score > min)
        {
            forge.number = o_ptr->number; /* ammo */
            *o_ptr = forge;
            return;
        }
        else if (object_is_melee_weapon(o_ptr))
        {
            if (forge.dd * forge.ds > best_score)
            {
                best_score = forge.dd * forge.ds;
                best = forge;
            }
        }
        else if (score > best_score)
        {
            best_score = score;
            best = forge;
        }
    }
    if (!igor_hack) assert(best.k_idx == o_ptr->k_idx);
    best.number = o_ptr->number; /* ammo */
    object_origins(&best, ORIGIN_CHEAT);
    *o_ptr = best;
}

static char _score_color(int score)  /* XXX duplicated in wizard2.c */
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

void wiz_create_objects(obj_create_f creator, u32b mode)
{
    doc_ptr doc = doc_alloc(120);
    inv_ptr inv = inv_alloc("Temp", INV_PACK, 0);
    int     i;

    statistics_hack = TRUE;
    for (i = 1; i < 1000;)
    {
        object_type forge;
        if (creator(&forge, mode))
        {
            _obj_identify_fully(&forge);
            inv_add(inv, &forge);
            i++;
        }
    }
    inv_optimize(inv);
    doc_insert(doc, "<style:table>");
    for (i = 1; i <= inv_max(inv); i++)
    {
        obj_ptr obj = inv_obj(inv, i);
        int     score;
        char    name[MAX_NLEN];

        if (!obj) continue;
        score = obj_value_real(obj);
        object_desc(name, obj, OD_COLOR_CODED);

        doc_printf(doc, "%3d <color:%c>%5d</color>) <indent><style:indent>%s</style></indent>\n",
            i, _score_color(score), score, name);
    }
    doc_insert(doc, "</style>");
    statistics_hack = FALSE;

    doc_display(doc, "Objects", 0);
    inv_free(inv);
    doc_free(doc);
}

static obj_ptr _reroll_obj = NULL;
static bool _reroll_creator(obj_ptr obj, u32b mode)
{
    bool igor_hack = ((prace_is_(RACE_IGOR)) && (_reroll_obj->tval == TV_CORPSE));
    object_prep(obj, igor_hack ? lookup_kind(TV_CORPSE, SV_CORPSE) : _reroll_obj->k_idx);
    object_origins(obj, ORIGIN_CHEAT);
    if (igor_hack)
    {
        if ((_reroll_obj->sval == SV_CORPSE) || (_reroll_obj->sval == SV_BODY_EARS)) obj->pval = _reroll_obj->pval;
        else obj->pval = _reroll_obj->xtra4;
    }
    if ((prace_is_(RACE_IGOR)) && (obj->tval == TV_CORPSE)) return igor_dissect_corpse(obj);
    return apply_magic(obj, dun_level, mode);
}
static void _reroll_stats_aux(object_type *o_ptr, int flags)
{
    _reroll_obj = o_ptr;
    wiz_create_objects(_reroll_creator, AM_NO_FIXED_ART | flags);
    _reroll_obj = NULL;
}

static int _smith_reroll(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    static int  min = 0;

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "   <color:y>w</color>) Awful\n");
        doc_insert(_doc, "   <color:y>b</color>) Bad\n");
        doc_insert(_doc, "   <color:y>a</color>) Average*\n");
        doc_insert(_doc, "   <color:y>g</color>) Good*\n");
        doc_insert(_doc, "   <color:y>e</color>) Excellent*\n");
        doc_insert(_doc, "   <color:y>r</color>) Random Artifact*\n");
        if (o_ptr->name1 || o_ptr->name3)
            doc_insert(_doc, "   <color:y>X</color>) Replacement Artifact\n");

        doc_newline(_doc);
        doc_printf(_doc, "   <color:y>m</color>) Min Score = %d\n", min);

        doc_insert(_doc, "\n   (*)SHIFT+choice to view statistics\n");
        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'm':
        {
            char buf[51];
            sprintf(buf, "%d", min);
            if (get_string("Min Score: ", buf, 50))
            {
                min = atoi(buf);
                if (min < 0) min = 0;
                else if (min > 500000) min = 500000;
            }
            break;
        }
        case 'w': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_CURSED, min); break;
        case 'b': _reroll_aux(&copy, AM_GOOD | AM_CURSED, min); break;
        case 'a': _reroll_aux(&copy, AM_AVERAGE, min); break;
        case 'A': _reroll_stats_aux(&copy, AM_AVERAGE); break; /* only makes sense for devices */
        case 'g': _reroll_aux(&copy, AM_GOOD, min); break;
        case 'G': _reroll_stats_aux(&copy, AM_GOOD); break; /* only makes sense for devices */
        case 'e': _reroll_aux(&copy, AM_GOOD | AM_GREAT, min); break;
        case 'E': _reroll_stats_aux(&copy, AM_GOOD | AM_GREAT); break;
        case 'r': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_SPECIAL, min); break;
        case 'R': _reroll_stats_aux(&copy, AM_GOOD | AM_GREAT | AM_SPECIAL); break;
        case 'X': {
            int which = o_ptr->name1;
            if (!which) which = o_ptr->name3;
            create_replacement_art(which, &copy, ORIGIN_CHEAT);
            _obj_identify_fully(&copy);
            break;}
        }
    }
}

static int _smith_resistances(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int  cmd, which;
        doc_ptr cols[2];

        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (which = RES_BEGIN; which < RES_END; which++)
        {
            doc_printf(cols[which < RES_NEXUS ? 0 : 1], "   <color:y>%c</color>) %s%c\n",
                I2A(which - RES_BEGIN), res_name(which),
                res_get_object_immune_flag(which) != OF_INVALID ? '*' : ' ');
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, "      SHIFT+choice toggle vulnerability\n");
        doc_insert(_doc, "   (*)CTRL+choice toggle immunity\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();

        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        if (cmd == '\r')
        {
            *o_ptr = copy;
            return _OK;
        }
        else if (cmd == ESCAPE)
            return _CANCEL;

        /* Toggle resistance? */
        which = A2I(cmd) + RES_BEGIN;
        if (RES_BEGIN <= which && which < RES_END)
        {
            _toggle(&copy, res_get_object_flag(which));
            continue;
        }

        /* Toggle vulnerability? */
        if (isupper(cmd))
        {
            which = A2I(tolower(cmd)) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flag = res_get_object_vuln_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
                    continue;
                }
            }
        }

        /* Toggle immunity? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            which = A2I(c) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flag = res_get_object_immune_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
                    continue;
                }
            }
        }
    }
}

/* Devices */
static device_effect_info_ptr _device_find_effect(device_effect_info_ptr tbl, int effect)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &tbl[i];

        if (!entry->type) break;
        if (entry->type == effect) return entry;
    }

    return NULL;
}

static device_effect_info_ptr _device_effect_tbl(object_type *o_ptr)
{
    assert(object_is_device(o_ptr));
    switch (o_ptr->tval)
    {
    case TV_WAND: return wand_effect_table;
    case TV_STAFF: return staff_effect_table;
    case TV_ROD: return rod_effect_table;
    }
    assert(FALSE);
    return NULL;
}

static device_effect_info_ptr _choose_effect(object_type *o_ptr)
{
    rect_t                 r = ui_map_rect();
    device_effect_info_ptr tbl = _device_effect_tbl(o_ptr);
    device_effect_info_ptr effect = _device_find_effect(tbl, o_ptr->activation.type);

    for (;;)
    {
        int cmd, i, ct = 0, split;
        doc_ptr cols[2];

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        /*obj_display_smith(o_ptr, _doc);*/
        doc_insert(_doc, "   <color:G>Choose an Effect:</color>\n");

        for (i = 0; ; i++)
        {
            if (!tbl[i].type) break;
            ct++;
        }
        split = ct;
        if (split > 10)
            split = (ct + 1) / 2;

        for (i = 0; ; i++)
        {
            device_effect_info_ptr e = &tbl[i];
            effect_t               dummy = {0};
            char                   choice = '?';
            char                   color = 'w';

            if (!e->type) break;
            dummy.type = e->type;
            if (i < 26) choice = 'a' + i;
            else if (i < 52) choice = 'A' + (i - 26);
            if (e->type == o_ptr->activation.type) color = 'B';
            doc_printf(cols[i < split ? 0 : 1], "   <color:y>%c</color>) <color:%c>%s</color>\n",
                choice, color, do_effect(&dummy, SPELL_NAME, 0));
        }
        doc_insert_cols(_doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == ESCAPE)
            break;
        else
        {
            if (isupper(cmd)) i = cmd - 'A' + 26;
            else i = cmd - 'a';
            if (0 <= i && i < ct)
            {
                effect = &tbl[i];
                break;
            }
        }
    }
    return effect;
}

static int _smith_device_effect(object_type *o_ptr)
{
    rect_t                 r = ui_map_rect();
    object_type            copy = *o_ptr;
    device_effect_info_ptr tbl = _device_effect_tbl(o_ptr);
    device_effect_info_ptr effect = _device_find_effect(tbl, copy.activation.type);

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "   <color:y>e</color>) Change effect\n");

        doc_insert(_doc, " <color:y>p</color>/<color:y>P</color>) Adjust power\n");
        doc_insert(_doc, " <color:y>m</color>/<color:y>M</color>) Adjust mana\n");
        doc_insert(_doc, " <color:y>l</color>/<color:y>L</color>) Adjust effect level\n");
        doc_insert(_doc, " <color:y>c</color>/<color:y>C</color>) Adjust effect cost\n");
        doc_insert(_doc, "   <color:y>r</color>) Recharge\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'l':
            if (copy.activation.difficulty > 1) copy.activation.difficulty--;
            else copy.activation.difficulty = 100;
            break;
        case 'L':
            if (copy.activation.difficulty < 100) copy.activation.difficulty++;
            else copy.activation.difficulty = 1;
            break;
        case 'c':
            if (copy.activation.cost > 1) copy.activation.cost--;
            else copy.activation.cost = 150;
            break;
        case 'C':
            if (copy.activation.cost < 150) copy.activation.cost++;
            else copy.activation.cost = 1;
            break;
        /* Note: We break encapsulation here ... */
        case 'p':
            if (copy.xtra3 > effect->level) copy.xtra3--;
            else copy.xtra3 = 100;
            copy.activation.power = copy.xtra3;
            break;
        case 'P':
            if (copy.xtra3 < 100) copy.xtra3++;
            else copy.xtra3 = effect->level;
            copy.activation.power = copy.xtra3;
            break;
        case 'm':
            if (copy.xtra4 > copy.activation.cost + 4) copy.xtra4 -= 5;
            else copy.xtra4 = 500;
            if (copy.xtra5/100 > copy.xtra4)
                copy.xtra5 = copy.xtra4*100;
            break;
        case 'M':
            if (copy.xtra4 < 496) copy.xtra4 += 5;
            else copy.xtra4 = copy.activation.cost;
            if (copy.xtra5/100 > copy.xtra4)
                copy.xtra5 = copy.xtra4*100;
            break;
        case 'e':
        {
            device_effect_info_ptr new_effect = _choose_effect(&copy);
            if (new_effect->type != effect->type)
            {
                effect = new_effect;
                copy.activation.type = effect->type;
                if (effect->level > copy.xtra3)
                {
                    copy.xtra3 = effect->level;
                    copy.activation.power = effect->level;
                }
                copy.activation.difficulty = effect->level;
                copy.activation.cost = effect->cost;
                if (effect->cost > copy.xtra4)
                {
                    copy.xtra4 = effect->cost;
                    copy.xtra5 = effect->cost * 100;
                }
            }
            break;
        }
        case 'r':
            device_regen_sp(&copy, 1000);
            break;
        }
    }
}

static int _smith_device_bonus(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "   <color:y>q</color>) Quickness\n");
        doc_insert(_doc, "   <color:y>p</color>) Power\n");
        doc_insert(_doc, "   <color:y>e</color>) Easy Use\n");
        doc_insert(_doc, "   <color:y>r</color>) Regeneration\n");
        doc_insert(_doc, "   <color:y>h</color>) Hold Charges\n");
        doc_insert(_doc, " <color:y>x</color>/<color:y>X</color>) Adjust amount of bonus\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'q':
            _toggle(&copy, OF_SPEED);
            break;
        case 'p':
            _toggle(&copy, OF_DEVICE_POWER);
            break;
        case 'e':
            _toggle(&copy, OF_EASY_SPELL);
            if (!copy.pval) /* not normally a pval flag */
                copy.pval = 1;
            break;
        case 'r':
            _toggle(&copy, OF_REGEN);
            if (!copy.pval) /* not normally a pval flag */
                copy.pval = 1;
            break;
        case 'h':
            _toggle(&copy, OF_HOLD_LIFE);
            break;
        case 'x':
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
            break;
        case 'X':
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
            break;
        }
    }
}

/* Top Level Smithing UI */
typedef int (*_smith_fn)(object_type *o_ptr);

typedef struct {
    char choice;
    cptr name;
    _smith_fn smithee;
    object_p pred;
} _command_t, *_command_ptr;

static bool _slays_p(object_type *o_ptr)
{
    return object_is_melee_weapon(o_ptr)
        || object_is_ammo(o_ptr);
}

static bool _brands_p(object_type *o_ptr)
{
    return object_is_melee_weapon(o_ptr)
        || object_is_ammo(o_ptr)
        || object_is_bow(o_ptr)
        || o_ptr->tval == TV_RING;
}

static _command_t _commands[] = {
    { 'p', "Plusses", _smith_plusses, object_is_weapon_armour_ammo },
    { 's', "Stats", _smith_stats, object_is_wearable },
    { 'b', "Bonuses", _smith_bonuses, object_is_wearable },
    { 'r', "Resistances", _smith_resistances, object_is_wearable },
    { 'a', "Abilities", _smith_abilities, object_is_wearable },
    { 't', "Telepathies", _smith_telepathies, object_is_wearable },
    { 'S', "Slays", _smith_slays, _slays_p },
    { 'B', "Brands", _smith_brands, _brands_p },
/*  { 'A', "Activation", _smith_activation, object_is_wearable },*/
    { 'e', "Effects", _smith_device_effect, object_is_device },
    { 'b', "Bonuses", _smith_device_bonus, object_is_device },
    { 'R', "Re-roll", _smith_reroll },
/*  { 'i', "Ignore", _smith_ignore },*/
    { 0 }
};

static int _smith_object_aux(object_type *o_ptr)
{
    rect_t  r = ui_map_rect();
    vec_ptr v = vec_alloc(NULL);
    int     result = _NONE, i; 

    for (i = 0; ; i++)
    {
        _command_ptr c_ptr = &_commands[i];
        if (!c_ptr->smithee) break;
        if (c_ptr->pred && !c_ptr->pred(o_ptr)) continue;
        vec_add(v, c_ptr);
    }

    while (result == _NONE && vec_length(v))
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _command_ptr c_ptr = vec_get(v, i);
            doc_printf(_doc, "   <color:y>%c</color>) %s\n",
                c_ptr->choice, c_ptr->name);
        }

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>RET</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>ESC</color>) Cancel changes\n");
        doc_newline(_doc);
        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
            result = _OK;
        else if (cmd == ESCAPE)
            result = _CANCEL;
        else
        {
            for (i = 0; i < vec_length(v); i++)
            {
                _command_ptr c_ptr = vec_get(v, i);
                if (c_ptr->choice == cmd)
                {
                    assert(c_ptr->smithee);
                    c_ptr->smithee(o_ptr);
                    break;
                }
            }
        }
    }
    vec_free(v);
    return result;
}

static int _smith_object(object_type *o_ptr)
{
    int result = _OK;
    assert(!_doc);
    _doc = doc_alloc(72);
    msg_line_clear();
    Term_save();

    result = _smith_object_aux(o_ptr);

    Term_load();
    doc_free(_doc);
    _doc = NULL;
    return result;
}

static bool _smith_p(object_type *o_ptr)
{
    if (object_is_wearable(o_ptr)) return TRUE;
    if (object_is_ammo(o_ptr)) return TRUE;
    if (object_is_device(o_ptr)) return TRUE;
    if ((prace_is_(RACE_IGOR)) && (object_is_(o_ptr, TV_CORPSE, SV_CORPSE))) return TRUE;
    return FALSE;
}

void wiz_obj_smith(void)
{
    obj_t        copy;
    obj_prompt_t prompt = {0};

    prompt.prompt = "Smith which object?";
    prompt.error = "You have nothing to work with.";
    prompt.filter = _smith_p;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    copy = *prompt.obj;
    _obj_identify_fully(&copy);

    msg_line_clear();
    if (_smith_object(&copy) == _OK)
    {
        obj_loc_t loc = prompt.obj->loc; /* re-roll will erase this ... */
        *prompt.obj = copy;
        prompt.obj->loc = loc;
        obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
    }
}

