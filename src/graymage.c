#include "angband.h"

#include <assert.h>

/**********************************************************************
 * The Gray Mage learn spells from books, but commits these spells
 * to memory (Gray Matter). They can only remember a certain number
 * of spells at a time, depending on level or intelligence.
 *
 * They may access multiple realms, depending on their subclass. All
 * can use Sorcery, Chaos, Trump, Arcane, Craft, and Armageddon. Good
 * allows Life and Crusade; Neutral adds Nature; and Evil adds Death
 * and Daemon.
 *
 * There is no spell proficiency system for us. Also, we can't use
 * any of the stock routines for gaining, browsing and casting spells.
 **********************************************************************/

#define _MAX_SLOTS 10
#define _INVALID_SLOT -1

#define _ALLOW_EMPTY    0x01
#define _ALLOW_EXCHANGE 0x02
#define _SHOW_INFO      0x04
#define _SHOW_STATS     0x08
#define _FROM_BOOK      0x10

static int _browse_choice = _INVALID_SLOT;

typedef struct {
    int realm;
    int spell;
} _slot_info_t, *_slot_info_ptr;

static _slot_info_t _spells[_MAX_SLOTS];

static magic_type *_get_spell_info(int realm, int spell)
{
    return &mp_ptr->info[realm - 1][spell];
}

static void _list_spell(doc_ptr doc, int realm, int spell, int choice, int options)
{
    magic_type *spell_ptr = _get_spell_info(realm, spell);
    int         sp_level = lawyer_hack(spell_ptr, LAWYER_HACK_LEVEL);
    int         cost = calculate_cost(lawyer_hack(spell_ptr, LAWYER_HACK_MANA));
    int         fail = calculate_fail_rate(sp_level, lawyer_hack(spell_ptr, LAWYER_HACK_FAILRATE), p_ptr->stat_ind[A_INT]);

    if (cost > p_ptr->csp)
        doc_insert(doc, "<color:D>");
    else if (choice == _browse_choice)
        doc_insert(doc, "<color:B>");
    else if (sp_level > p_ptr->lev)
    {
        if (options & _FROM_BOOK)
            doc_insert(doc, "<color:D>");
        else
            doc_insert(doc, "<color:y>");
    }
    else
        doc_insert(doc, "<color:w>");

    if (sp_level > p_ptr->lev)
        doc_printf(doc, " <color:D>%c)</color> ", I2A(choice));
    else
        doc_printf(doc, " %c) ", I2A(choice));

    doc_printf(doc, "%-20.20s ", do_spell(realm, spell, SPELL_NAME));
    doc_printf(doc, "%3d %3d %3d%% ", sp_level, cost, fail);

    if (sp_level > p_ptr->lev)
    {
        if (options & _FROM_BOOK)
            doc_printf(doc, "%-15.15s", "");
        else
            doc_printf(doc, "%-15.15s", "Forgotten");
    }
    else if (options & _SHOW_INFO)
        doc_printf(doc, "%-15.15s", do_spell(realm, spell, SPELL_INFO));

    if (options & _SHOW_STATS)
    {
        spell_stats_ptr stats = spell_stats_old(realm, spell);
        if (stats->ct_cast + stats->ct_fail)
        {
            doc_printf(doc, " %5d %4d %3d%%",
                stats->ct_cast,
                stats->ct_fail,
                spell_stats_fail(stats)
            );
        }
    }

    doc_insert(doc, "</color>\n");
}

static void _list_spells(doc_ptr doc, int options)
{
    int i;

    doc_insert(doc, "<style:table>");
    doc_printf(doc, "<color:G>    %-20.20s Lvl  SP Fail %-15.15s", "Name", "Desc");
    if (options & _SHOW_STATS)
        doc_insert(doc, "  Cast Fail");
    doc_insert(doc, "</color>\n");

    for (i = 0; i < _MAX_SLOTS; i++)
    {
        if (_spells[i].realm != REALM_NONE)
            _list_spell(doc, _spells[i].realm, _spells[i].spell, i, options);
        else
        {
            if (options & _ALLOW_EMPTY)
                doc_printf(doc, " %c) <color:D>(Empty)</color>\n", I2A(i));
            else
                doc_printf(doc, " <color:D>%c) (Empty)</color>\n", I2A(i));
        }
    }
    doc_insert(doc, "</style>");

    if (_browse_choice != -1 && _spells[_browse_choice].realm != REALM_NONE)
    {
        doc_newline(doc);
        doc_printf(doc, "    <indent>%s</indent>\n\n",
            do_spell(_spells[_browse_choice].realm, _spells[_browse_choice].spell, SPELL_DESC));
    }
}

static void _display(rect_t r, int options)
{
    doc_ptr doc = doc_alloc(r.cx);
    _list_spells(doc, options);
    doc_sync_term(doc, doc_range_all(doc), doc_pos_create(r.x, r.y));
    doc_free(doc);
}

static rect_t _menu_rect(void)
{
    rect_t r = ui_menu_rect();
    if (r.cx > 80)
        r.cx = 80;
    return r;
}

static _slot_info_ptr _choose(cptr verb, int options)
{
    _slot_info_ptr result = NULL;
    int            slot = 0;
    int            cmd;
    rect_t         r = _menu_rect();
    string_ptr     prompt = NULL;
    bool           done = FALSE;
    bool           exchange = FALSE;
    int            slot1 = _INVALID_SLOT, slot2 = _INVALID_SLOT;

    if (REPEAT_PULL(&cmd))
    {
        slot = A2I(cmd);
        if (0 <= slot && slot < _MAX_SLOTS)
            return &_spells[slot];
    }

    prompt = string_alloc();
    screen_save();
    while (!done)
    {
        string_clear(prompt);

        if (exchange)
        {
            if (slot1 == _INVALID_SLOT)
                string_append_s(prompt, "Select the first spell:");
            else
                string_append_s(prompt, "Select the second spell:");
        }
        else
        {
            string_printf(prompt, "%s which spell", verb);
            if (options & _ALLOW_EXCHANGE)
                string_append_s(prompt, " [Press 'X' to Exchange]");
            string_append_c(prompt, ':');
        }
        prt(string_buffer(prompt), 0, 0);
        _display(r, options);

        cmd = inkey_special(FALSE);

        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q')
            done = TRUE;

        if (options & _ALLOW_EXCHANGE)
        {
            if (!exchange && (cmd == 'x' || cmd == 'X'))
            {
                exchange = TRUE;
                slot1 = slot2 = _INVALID_SLOT;
            }
        }

        if ('a' <= cmd && cmd < 'a' + _MAX_SLOTS)
        {
            slot = A2I(cmd);
            if (exchange)
            {
                if (slot1 == _INVALID_SLOT)
                    slot1 = slot;
                else
                {
                    slot2 = slot;
                    if (slot1 != slot2)
                    {
                        _slot_info_t  tmp = _spells[slot1];
                        _spells[slot1] = _spells[slot2];
                        _spells[slot2] = tmp;
                    }
                    exchange = FALSE;
                    slot1 = slot2 = _INVALID_SLOT;
                }
            }
            else
            {
                if (_spells[slot].realm != REALM_NONE || (options & _ALLOW_EMPTY))
                {
                    result = &_spells[slot];
                    done = TRUE;
                }
            }
        }
    }

    if (result)
    {
        REPEAT_PUSH(I2A(slot));
    }

    screen_load();
    string_free(prompt);
    return result;
}

/**********************************************************************
 * Birth
 **********************************************************************/
static void _birth(void)
{
    object_type forge;
    int i;

    object_prep(&forge, lookup_kind(TV_SWORD, SV_DAGGER));
    py_birth_obj(&forge);

    if (p_ptr->psubclass == GRAY_MAGE_GOOD)
    {
        object_prep(&forge, lookup_kind(TV_LIFE_BOOK, 0));
        py_birth_obj(&forge);

        object_prep(&forge, lookup_kind(TV_CRUSADE_BOOK, 0));
        py_birth_obj(&forge);
    }
    else if (p_ptr->psubclass == GRAY_MAGE_NEUTRAL)
    {
        object_prep(&forge, lookup_kind(TV_NATURE_BOOK, 0));
        py_birth_obj(&forge);
    }
    else if (p_ptr->psubclass == GRAY_MAGE_EVIL)
    {
        object_prep(&forge, lookup_kind(TV_DEATH_BOOK, 0));
        py_birth_obj(&forge);

        object_prep(&forge, lookup_kind(TV_DAEMON_BOOK, 0));
        py_birth_obj(&forge);
    }

    object_prep(&forge, lookup_kind(TV_ARCANE_BOOK, 0));
    py_birth_obj(&forge);

    /* Restart? player_wipe doesn't know about this stuff, of course ... */
    for (i = 0; i < _MAX_SLOTS; i++)
    {
        _spells[i].realm = REALM_NONE;
        _spells[i].spell = 0;
    }
}

/**********************************************************************
 * Private Helpers
 **********************************************************************/
static void _character_dump(doc_ptr doc)
{
    doc_printf(doc, "<topic:Spells>==================================== <color:keypress>S</color>pells ===================================\n\n");
    _list_spells(doc, _SHOW_INFO | _SHOW_STATS);
    doc_newline(doc);
}

static void _load_player(savefile_ptr file)
{
    int i;
    for (i = 0; i < _MAX_SLOTS; i++)
    {
        _spells[i].realm = REALM_NONE;
        _spells[i].spell = 0;
    }

    while (1)
    {
        i = savefile_read_u16b(file);
        if (i == 0xFFFF) break;
        assert(0 <= i && i < _MAX_SLOTS);
        _spells[i].realm = savefile_read_byte(file);
        _spells[i].spell = savefile_read_byte(file);
    }
}

static void _save_player(savefile_ptr file)
{
    int i;
    for (i = 0; i < _MAX_SLOTS; i++)
    {
        if (_spells[i].realm != REALM_NONE)
        {
            savefile_write_u16b(file, (u16b)i);
            savefile_write_byte(file, _spells[i].realm); /* 1 to 12 */
            savefile_write_byte(file, _spells[i].spell); /* 0 to 31 */
        }
    }
    savefile_write_u16b(file, 0xFFFF); /* sentinel */
}

static power_info _get_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static bool _is_allowed_realm(int realm)
{
    switch (realm)
    {
    case REALM_LIFE:
    case REALM_CRUSADE:
        return p_ptr->psubclass == GRAY_MAGE_GOOD;

    case REALM_NATURE:
        return p_ptr->psubclass == GRAY_MAGE_NEUTRAL;

    case REALM_DEATH:
    case REALM_DAEMON:
        return p_ptr->psubclass == GRAY_MAGE_EVIL;

    case REALM_SORCERY:
    case REALM_CHAOS:
    case REALM_TRUMP:
    case REALM_ARCANE:
    case REALM_CRAFT:
    case REALM_ARMAGEDDON:
        return TRUE;
    }
    return FALSE;
}

static bool _is_spellbook(int tval)
{
    if (tval < TV_BOOK_BEGIN || tval > TV_BOOK_END) return FALSE;
    return TRUE;
}

bool gray_mage_is_allowed_book(int tval, int sval) /* For autopick.c */
{
    if (!_is_spellbook(tval)) return FALSE;
    return _is_allowed_realm(tval2realm(tval));
}

static bool _spell_book_p(object_type *o_ptr)
{
    if (!_is_spellbook(o_ptr->tval)) return FALSE;
    return gray_mage_is_allowed_book(o_ptr->tval, o_ptr->sval);
}

/* cmd5.c get_spell() was blowing up when I attempted code reuse ...
   so roll our own (much simpler) version */
#define _SPELLS_PER_BOOK 8
static void _display_spells_to_gain(object_type *o_ptr, rect_t r, int tutki)
{
    doc_ptr doc = doc_alloc(r.cx);
    int     i;
    int     realm = tval2realm(o_ptr->tval);
    int     start_idx = o_ptr->sval * _SPELLS_PER_BOOK;

    r.x = 0;
    r.y = 0;
    doc_insert(doc, "Memorize which spell [<color:keypress>A</color>-<color:keypress>H</color> to browse]:\n");
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "<color:G>    %-20.20s Lvl  SP Fail Desc</color>\n", "Name");

    for (i = start_idx; i < start_idx + _SPELLS_PER_BOOK; i++)
        _list_spell(doc, realm, i, i - start_idx, (_FROM_BOOK | _SHOW_INFO));
    doc_insert(doc, "</style>");

    if ((tutki >= 0) && (tutki < _SPELLS_PER_BOOK))
    {
        doc_printf(doc, "    <indent>%s</indent>\n",
            do_spell(realm, start_idx + tutki, SPELL_DESC));
    }

    doc_sync_term(doc, doc_range_all(doc), doc_pos_create(r.x, r.y));
    doc_free(doc);
}

static int _choose_spell_to_gain(object_type *o_ptr)
{
    rect_t r = _menu_rect();
    int    result = -1;
    int    tutki = -1;
    int    cmd;
    bool   done = FALSE;

    screen_save();
    while (!done)
    {
        _display_spells_to_gain(o_ptr, r, tutki);

        cmd = inkey_special(FALSE);

        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q')
            done = TRUE;

        tutki = -1;

        if ('a' <= cmd && cmd < 'a' + _SPELLS_PER_BOOK)
        {
            int         spell_idx = o_ptr->sval * _SPELLS_PER_BOOK + A2I(cmd);
            magic_type *spell_ptr = _get_spell_info(tval2realm(o_ptr->tval), spell_idx);

            if (lawyer_hack(spell_ptr, LAWYER_HACK_LEVEL) <= p_ptr->lev) /* Note: Illegible spells have slevel == 99 in m_info.txt */
            {
                done = TRUE;
                result = spell_idx;
            }
        }        
        else 
        {
            screen_load();
            screen_save();
            if (isupper(cmd))
            {
                cmd = tolower(cmd);
                if ('a' <= cmd && cmd < 'a' + _SPELLS_PER_BOOK)
                {
                    tutki = A2I(cmd);
                }
            }
        }
    }
    screen_load();
    return result;
}

/**********************************************************************
 * Public
 **********************************************************************/
void gray_mage_browse_spell(void)
{
    bool done = FALSE;

    screen_save();
    _browse_choice = 0;
    while (!done)
    {
        _slot_info_ptr slot = _choose("Browse", _ALLOW_EXCHANGE | _SHOW_INFO | _SHOW_STATS);
        if (!slot)
            done = TRUE;
        else
            _browse_choice = slot - _spells;
    }
    _browse_choice = -1;
    screen_load();
}

void gray_mage_cast_spell(void)
{
    _slot_info_ptr slot_ptr;

    /* Blind is OK!!! */

    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        return;
    }

    if (pelko()) return;

    slot_ptr = _choose("Cast", _ALLOW_EXCHANGE | _SHOW_INFO);
    if (slot_ptr)
    {
        magic_type *spell_ptr = _get_spell_info(slot_ptr->realm, slot_ptr->spell);
        int         sp_level = lawyer_hack(spell_ptr, LAWYER_HACK_LEVEL);
        int         cost = calculate_cost(lawyer_hack(spell_ptr, LAWYER_HACK_MANA));
        int         fail = calculate_fail_rate(sp_level, lawyer_hack(spell_ptr, LAWYER_HACK_FAILRATE), p_ptr->stat_ind[A_INT]);

        if (sp_level > p_ptr->lev) /* Experience Drain? */
        {
            msg_format("You need to be level %d to use that spell.", sp_level);
            return;
        }

        if (cost > p_ptr->csp)
        {
            msg_print("You do not have enough mana to cast this spell.");
            return;
        }

        p_ptr->csp -= cost;
        energy_use = 100;

        if (randint0(100) < fail)
        {
            if (flush_failure) flush();

            cmsg_format(TERM_VIOLET, "You failed to cast %s!", do_spell(slot_ptr->realm, slot_ptr->spell, SPELL_NAME));
            if (prompt_on_failure) msg_print(NULL);
            if (demigod_is_(DEMIGOD_ATHENA))
                p_ptr->csp += cost/2;
            spell_stats_on_fail_old(slot_ptr->realm, slot_ptr->spell);
            sound(SOUND_FAIL);
            do_spell(slot_ptr->realm, slot_ptr->spell, SPELL_FAIL);
        }
        else
        {
            if (!do_spell(slot_ptr->realm, slot_ptr->spell, SPELL_CAST))
            {  /* Canceled */
                p_ptr->csp += cost;
                energy_use = 0;
                return;
            }
            sound(SOUND_ZAP);
            spell_stats_on_cast_old(slot_ptr->realm, slot_ptr->spell);
        }
        p_inc_fatigue(MUT_EASY_TIRING2, 50 + MIN(50, cost / 2));
    }
    p_ptr->redraw |= PR_MANA;
    p_ptr->window |= PW_SPELL;
}

void gray_mage_gain_spell(void)
{
    obj_prompt_t    prompt = {0};
    int             spell_idx;
    _slot_info_ptr  slot_ptr;

    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");
        return;
    }

    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        return;
    }

    if (!p_ptr->new_spells)
    {
        msg_print("You cannot learn any new spells!");
        return;
    }

    prompt.prompt = "Study which book?";
    prompt.error = "You have no books that you can read.";
    prompt.filter = _spell_book_p;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    /* Pick a spell to learn */
    spell_idx = _choose_spell_to_gain(prompt.obj);
    if (spell_idx == -1) return;

    /* Pick a slot for storage (possibly replacing an already learned spell) */
    slot_ptr = _choose("Replace", _ALLOW_EMPTY | _SHOW_INFO);
    if (!slot_ptr) return;

    if (slot_ptr->realm != REALM_NONE)
    {
        char       c;
        string_ptr prompt = string_alloc_format(
            "Really replace %s? <color:y>[y/N]</color>",
            do_spell(slot_ptr->realm, slot_ptr->spell, SPELL_NAME));

        c = msg_prompt(string_buffer(prompt), "ny", PROMPT_DEFAULT);
        string_free(prompt);
        if (c == 'n') return;
    }

    /* Learn the spell: Note, we don't bother with spell_learned# and spell_order[], since
       these are hard coded for 2 spell realms. Hopefully, ticking up learned_spells is enough? */
    p_ptr->learned_spells++;
    slot_ptr->realm = tval2realm(prompt.obj->tval);
    slot_ptr->spell = spell_idx;
    msg_format("You have learned the spell '%s'.", do_spell(slot_ptr->realm, slot_ptr->spell, SPELL_NAME));
    p_ptr->update |= PU_SPELLS;
    p_ptr->redraw |= PR_EFFECTS;
    energy_use = 100;
}

extern cptr gray_mage_speciality_name(int psubclass)
{
    switch (psubclass)
    {
    case GRAY_MAGE_GOOD: return "Good Bias";
    case GRAY_MAGE_NEUTRAL: return "Neutral Bias";
    case GRAY_MAGE_EVIL: return "Evil Bias";
    }
    return "";
}

extern cptr gray_mage_speciality_desc(int psubclass)
{
    switch (psubclass)
    {
    case GRAY_MAGE_GOOD: return "You gain access to Life and Crusade magic.";
    case GRAY_MAGE_NEUTRAL: return "You gain access to Nature magic.";
    case GRAY_MAGE_EVIL: return "You gain access to Daemon and Death magic.";
    }
    return "";
}

class_t *gray_mage_get_class(int psubclass)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  38,   3,  16,  20,  34,  20};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "Gray-Mage";
        me.desc = "The Gray-Mage casts spells from memory rather than books; a "
                    "spellbook is only required for the initial learning process. "
                    "However, only ten spells may be memorized at any given time; and "
                    "while a Gray-Mage may replace old spells with new ones, the total "
                    "number of spells they can study is limited.\n\n"
                    "Gray-Mages do not choose specific realms like book spellcasters; "
                    "instead, they choose a general bias towards either Good, Neutral "
                    "or Evil magic. So while all Gray-Mages may learn spells from the Arcane, "
                    "Armageddon, Chaos, Craft, Sorcery and Trump realms, only a Good Bias allows "
                    "access to Life and Crusade magic; only a Neutral Bias allows access to Nature "
                    "magic; and only an Evil Bias allows access to Death and Daemon magic. At any one "
                    "time, a Gray-Mage has relatively few spells directly at their disposal; but their ability "
                    "to pick the best spells from an extremely large pool more than compensates for this. "
                    "As with most mages, the key stat is Intelligence.";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 95;
        me.base_hp = 0;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG |
                   CLASS_REGEN_MANA;

        me.caster_info = _caster_info;
        me.character_dump = _character_dump;
        me.get_powers = _get_powers;
        me.birth = _birth;

        me.load_player = _load_player;
        me.save_player = _save_player;

        init = TRUE;
    }
    me.subname = gray_mage_speciality_name(psubclass);
    me.subdesc = gray_mage_speciality_desc(psubclass);
    return &me;
}
