#include "angband.h"
#include "randname.h"

#include <assert.h>

/************************************************************************
 * Give birth to a new player.
 *
 * For reference, there are 3 possible game modes:
 * [1] Beginner: Limited Races and Classes
 *               No Wilderness
 *               No Personality
 *               No Birth Options
 * [2] Normal:   Standard Game with All Races/Classes/etc.
 * [3] Monster:  Monster Races
 *               No Class (CLASS_MONSTER is a bunch of zeros!)
 *
 * *_ui() functions run menu loops according to the following flow graph:
 ***********************************************************************/
extern int py_birth(void);
    static int _welcome_ui(void);
    static int _race_class_ui(void);
        static void _pers_ui(void);
        /* Normal Mode */
        static void _race_group_ui(void);
            static int _race_ui(int ids[]);
                static int _subrace_ui(void);
                    static int _demigod_ui(void);
                    static int _draconian_ui(void);
        static void _class_group_ui(void);
            static int _class_ui(int ids[]);
                static int _subclass_ui(void);
                    static int _warlock_ui(void);
                    static int _weaponmaster_ui(void);
                    static int _devicemaster_ui(void);
                    static int _gray_mage_ui(void);
                    static int _patron_ui(void);
        static int _realm1_ui(void);
            static int _realm2_ui(void);
        /* Monster Mode */
        static void _mon_race_group_ui(void);
            static int _mon_race_ui(int ids[]);
                static int _mon_subrace_ui(void);
                    static int _mon_demon_ui(void);
                    static int _mon_dragon_ui(void);
                        static int _dragon_realm_ui(void);
                    static int _mon_elemental_ui(void);
                    static int _mon_giant_ui(void);
                    static int _mon_golem_ui(void);
                    static int _mon_spider_ui(void);
                    static int _mon_troll_ui(void);
                    static int _mon_orc_ui(void);
        static int _speed_ui(void);
    static int _stats_ui(void);

extern void py_birth_obj(object_type *o_ptr);
extern void py_birth_obj_aux(int tval, int sval, int qty);
extern void py_birth_food(void);
extern void py_birth_light(void);
extern void py_birth_spellbooks(void);

/* I prefer to render menus to a document rather than directly to the terminal */
static doc_ptr _doc = NULL;

static void _birth_options(void);
static void _birth_finalize(void);
static int _inkey(void);
static void _sync_term(doc_ptr doc);
static int _count(int ids[]);

/************************************************************************
 * Public Entrypoints
 ***********************************************************************/ 
int py_birth(void)
{
    int result = UI_OK;

    /* Windows is not setting a player name for new files */
    if (0 == strlen(player_name))
        strcpy(player_name, "PLAYER");

    assert(!_doc);
    _doc = doc_alloc(80);

    msg_line_clear();
    Term_clear();
    Term_save();
    result = _welcome_ui();
    Term_load();

    doc_free(_doc);
    _doc = NULL;

    return result;
}

void py_birth_obj_aux(int tval, int sval, int qty)
{
    object_type forge;

    switch (tval)
    {
    case TV_WAND: case TV_ROD: case TV_STAFF:
    {
        int k_idx = lookup_kind(tval, SV_ANY);
        object_prep(&forge, k_idx);
        if (!device_init_fixed(&forge, sval))
            return;
        qty = 1;
        break;
    }
    case TV_QUIVER:
        object_prep(&forge, lookup_kind(tval, sval));
        apply_magic(&forge, 0, AM_AVERAGE);
        break;
    default:
        object_prep(&forge, lookup_kind(tval, sval));
    }

    forge.number = qty;
    py_birth_obj(&forge);
}

void py_birth_obj(object_type *o_ptr)
{
    int slot;

    if (spoiler_hack)
        return;

    /* Androids can hit CL9 or more with starting Chain Mail! */
    if (p_ptr->prace == RACE_ANDROID && object_is_body_armour(o_ptr))
        return;

    /* Weed out duplicate gear (e.g. Artemis Archer) but note
     * that centipedes start with duplicate boots (so allow
     * multiple objects provided they can also be equipped) */
    if ( object_is_wearable(o_ptr)
      && o_ptr->number == 1
      && p_ptr->prace != RACE_MON_RING /* Hack: Ring cannot wear rings, but can absorb them for powers */
      && equip_find_obj(o_ptr->tval, o_ptr->sval)
      && !equip_first_empty_slot(o_ptr) ) /* Hack: Centipede gets multiple boots */
    {
        return;
    }

    object_origins(o_ptr, ORIGIN_BIRTH);
    obj_identify_fully(o_ptr);

    /* Big hack for sexy players ... only wield the starting whip,
     * but carry the alternate weapon. Previously, sexy characters
     * would usually start off dual-wielding (ineffectual and confusing)*/
    if ( personality_includes_(PERS_SEXY)
      && p_ptr->prace != RACE_MON_SWORD
      && object_is_melee_weapon(o_ptr)
      && !object_is_(o_ptr, TV_HAFTED, SV_WHIP) )
    {
        pack_carry(o_ptr);
        return;
    }

    slot = equip_first_empty_slot(o_ptr);
    if ((slot) && (o_ptr->number == 1))
        equip_wield(o_ptr, slot);
    else if ((slot) && (thrall_mode))
    {
        equip_wield(o_ptr, slot);
        pack_carry(o_ptr);
    }
    else
        pack_carry(o_ptr);
}

/* Standard Food and Light */
void py_birth_food(void)
{
    py_birth_obj_aux(TV_FOOD, SV_FOOD_RATION, 2 + rand_range(3, 7));
}

void py_birth_light(void)
{
    if (!player_is_ninja)
    {
        object_type forge = {0};
        object_prep(&forge, lookup_kind(TV_LITE, SV_LITE_TORCH));
        forge.number = rand_range(3, 7);
        forge.xtra4 = rand_range(3, 7) * 500;
        py_birth_obj(&forge);
    }
}

void py_birth_spellbooks(void)
{
    if (p_ptr->realm1)
        py_birth_obj_aux(TV_LIFE_BOOK + p_ptr->realm1 - 1, 0, 1);
    if (p_ptr->realm2)
        py_birth_obj_aux(TV_LIFE_BOOK + p_ptr->realm2 - 1, 0, 1);
}

/********************************************************************
 * Roman numeral code
 *******************************************************************/
int _letter_roman_value(unsigned char mika)
{
    switch (mika)
    {
        case 'M': return 1000;
        case 'D': return 500;
        case 'C': return 100;
        case 'L': return 50;
        case 'X': return 10;
        case 'V': return 5;
        case 'I': return 1;
        default: return 0;
    }
}

int find_roman_numeral(char *nimi)
{
    int i, tyhja, pituus = strlen(nimi), uuspit, isoarvo = 0;
    int arvot[22] = {0};

    if (pituus <= 2) return 0;
    if (strpos(" ", nimi) <= 1) return 0;

    /* Scan for last space */
    for (tyhja = pituus - 2; tyhja >= 1; tyhja--)
    {
        if (nimi[tyhja] == ' ') break;
    }
    if (!tyhja) return 0; /* paranoia */
    uuspit = pituus - tyhja - 1;
    if (uuspit > PY_NAME_LEN) return 0;

    /* Check if name contains a Roman numeral */
    for (i = 0; i < uuspit; i++)
    {
        int _valu = _letter_roman_value(nimi[i + tyhja + 1]);
        if (!_valu) return 0;
        else arvot[i] = _valu;
        if ((i >= 2) && (_valu > arvot[i - 2])) return 0; /* Invalid numeral, e.g. LIM */
    }

    /* Calculate value of said numeral */
    for (i = 0; i < uuspit; i++)
    {
        if ((i < uuspit - 1) && (arvot[i + 1] > arvot[i])) isoarvo -= arvot[i];
        else isoarvo += arvot[i];
    }
    return MAX(0, isoarvo); /* paranoia */
}

#define _MAX_ROMAN_CAT 13
typedef struct _roman_cat_s {
   int val;
   char lisays[3];
} _roman_cat_t, *_roman_cat_ptr;

static _roman_cat_t _roman_cats[_MAX_ROMAN_CAT] = {
 { 1000, "M"},
 { 900, "CM"},
 { 500, "D"},
 { 400, "CD"},
 { 100, "C"},
 { 90, "XC"},
 { 50, "L"},
 { 40, "XL"},
 { 10, "X"},
 { 9, "IX"},
 { 5, "V"},
 { 4, "IV"},
 { 1, "I"}
};

bool num_to_roman(int _num, char *buf)
{
    int i, jaljella = _num, pituus = 0;
    while (jaljella > 0)
    {
        for (i = 0; i < _MAX_ROMAN_CAT; i++)
        {
            if (jaljella >= _roman_cats[i].val) break;
        }
        if ((pituus + (i % 2)) >= PY_NAME_LEN) return FALSE;
        if (!pituus) strcpy(buf, " ");
        strcat(buf, _roman_cats[i].lisays);
        pituus += (1 + (i % 2));
        jaljella -= _roman_cats[i].val;
    }
    return TRUE;
}

int find_arabic_numeral(char *nimi, int *paikka)
{
    s32b arvo = 0;
    int i, j, pituus = 0;
        
    for (i = strlen(nimi) - 1; i > 0; i--)
    {
        if (isdigit(nimi[i]))
        {
            int lisa = nimi[i] - '0';
            pituus++;
            for (j = 1; j < pituus; j++)
            {
                lisa *= 10;
            }
            arvo += lisa;
        }
        else
        {
            if (!pituus) return 0; /* No numeral */
            if (paikka != NULL) *paikka = i + 1;
            return arvo;
        }
    }
    return arvo;
}

void bump_numeral(char *nimi, int muutos)
{
    int _old_num = 0, _error = 0;
    if ((!nimi) || (!strlen(nimi))) return;
    if (!muutos) return; /* Nothing to do */
    _old_num = find_roman_numeral(nimi);
    while (_old_num > 0)
    {
        char luku[PY_NAME_LEN + 2] = "";
        if (!num_to_roman(_old_num, luku)) break;
        if (!clip_and_locate(luku, nimi)) break;
        _error = 1;
        if ((_old_num + muutos) <= 0) return;
        if (!num_to_roman(_old_num + muutos, luku)) break;
        if (strlen(nimi) + strlen(luku) > PY_NAME_LEN) break;
        strcat(nimi, luku);
        return;
    }
    if (_old_num > 0) /* There was a Roman numeral but something went wrong */
    {
        char luku[PY_NAME_LEN + 2] = "";
        if (!_error)
        {
            int tyhja = 0;
            for (tyhja = strlen(nimi) - 2; tyhja >= 0; tyhja--)
            {
                if (nimi[tyhja] == ' ') break;
            }
            if (!tyhja) return; /* Something weird is happening */
            if ((_old_num + muutos) <= 0) return;
            nimi[tyhja + 1] = '\0';
        }
        else if ((_old_num + muutos) <= 0) return;
        strcpy(luku, format("%d", _old_num + muutos));
        if (strlen(nimi) + strlen(luku) > PY_NAME_LEN) return;
        if ((_error) && (strlen(nimi) + strlen(luku) != PY_NAME_LEN) && (strlen(nimi) > 0) && (nimi[strlen(nimi) - 1] != ' ')) strcat(nimi, " ");
        strcat(nimi, luku);
        return;
    }
    else /* No Roman numeral - check for Arabic numeral */
    {
        char luku[PY_NAME_LEN + 2] = "";
        int paikka = -1;
        s32b arvo = find_arabic_numeral(nimi, &paikka);
        if (arvo < 1) return;
        if (paikka <= 0) return;
        nimi[paikka] = '\0';
        if ((arvo + muutos) < 1) return;
        strcpy(luku, format("%d", arvo + muutos));
        if (strlen(nimi) + strlen(luku) > PY_NAME_LEN) return;
        strcat(nimi, luku);
    }
}

bool name_is_numbered(char *nimi)
{
    if ((!nimi) || (strlen(nimi) < 2)) return FALSE;
    if (find_roman_numeral(nimi)) return TRUE;
    if (find_arabic_numeral(nimi, NULL)) return TRUE;
    return FALSE;
}

/************************************************************************
 * Welcome to FrogComposband!
 ***********************************************************************/ 
static void _set_mode(int mode);
static s16b _stats_changed = -1;
static bool lukittu = FALSE;

static int _welcome_ui(void)
{
    /* Mega-Hack */
    werewolf_init();
    beorning_init();
    p_ptr->chaos_patron = RANDOM_PATRON;

    for (;;)
    {
        int cmd;

        doc_clear(_doc);

        doc_insert(_doc,
            "Welcome to <color:keyword>FrogComposband</color>, a dungeon exploration "
            "role-playing game. Your goal is to defeat the dreaded <color:keyword>"
            "Serpent of Chaos</color>, but before you can face it, you must battle "
            "many foes. Your first step is to create a character for this quest. "
            "The following screens will guide you through this process so that you "
            "may quickly begin playing. You can get general help at any time by "
            "pressing <color:keypress>?</color>.\n\n"
        );

        doc_insert(_doc,
            "First, you must decide what type of game to play. <color:keyword>Beginner Mode</color> "
            "limits the options available, simplifying things for new players.\n\n "
        );

        doc_insert(_doc, "<color:G>Choose the Type of Game to Play</color>\n");
        doc_insert(_doc, "  <color:y>b</color>) Beginner\n");
        doc_insert(_doc, "  <color:y>n</color>) Normal\n");
        doc_insert(_doc, "  <color:y>m</color>) Monster\n");
        doc_newline(_doc);
        if (previous_char.quick_ok)
            doc_insert(_doc, "  <color:y>q</color>) Quick Start\n");
        if (game_mode != GAME_MODE_BEGINNER)
            doc_insert(_doc, "  <color:y>=</color>) Options\n");
        doc_insert(_doc, "  <color:y>?</color>) Help\n");
        doc_insert(_doc, "  <color:y>s</color>) View Scores\n");
        doc_insert(_doc, "<color:y>ESC</color>) <color:v>Quit</color>\n");


        doc_newline(_doc);
        doc_insert(_doc, "<color:G>Tip:</color> <indent>You can often get specific "
                         "help by entering the uppercase letter for a command. For "
                         "example, type <color:keypress>B</color> on this screen "
                         "to receive help on Beginner Mode.</indent>");
        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == '?')
            doc_display_help("birth.txt", NULL /*"Welcome"*/);
        else if (cmd == '!')
            doc_display_help("start.txt", NULL /*"Welcome"*/);
        else if (cmd == ESCAPE)
            return UI_CANCEL;
        else if (cmd == '=')
            _birth_options();
        else if (cmd == 'q' && previous_char.quick_ok)
        {
            int i;
            game_mode = previous_char.game_mode;
            p_ptr->psex = previous_char.psex;
            p_ptr->prace = previous_char.prace;
            p_ptr->psubrace = previous_char.psubrace;
            p_ptr->pclass = previous_char.pclass;
            p_ptr->psubclass = previous_char.psubclass;
            p_ptr->personality = previous_char.personality;
            p_ptr->realm1 = previous_char.realm1;
            p_ptr->realm2 = previous_char.realm2;
            p_ptr->dragon_realm = previous_char.dragon_realm;
            p_ptr->au = previous_char.au;            
            for (i = 0; i < MAX_STATS; i++)
            {
                p_ptr->stat_cur[i] = previous_char.stat_max[i];
                p_ptr->stat_max[i] = previous_char.stat_max[i];
            }
            _stats_changed = p_ptr->pclass; /* block default stat allocation via _stats_init */
            if (_race_class_ui() == UI_OK)
                return UI_OK;
        }
        else if (cmd == 'Q' && previous_char.quick_ok)
            doc_display_help("birth.txt", "QuickStart");
        else if (cmd == 'b')
        {
            _set_mode(GAME_MODE_BEGINNER);
            if (_race_class_ui() == UI_OK)
                return UI_OK;
        }
        else if (cmd == 'B')
            doc_display_help("birth.txt", "BeginnerMode");
        else if (cmd == 'n')
        {
            _set_mode(GAME_MODE_NORMAL);
            if (_race_class_ui() == UI_OK)
                return UI_OK;
        }
        else if (cmd == 'N')
            doc_display_help("birth.txt", "NormalMode");
        else if (cmd == 'm')
        {
            _set_mode(GAME_MODE_MONSTER);
            if (_race_class_ui() == UI_OK)
                return UI_OK;
        }
        else if (cmd == 'M')
            doc_display_help("birth.txt", "MonsterMode");
        else if (cmd == 's')
        {
            vec_ptr scores = scores_load(NULL);
            scores_display(scores);
            vec_free(scores);
        }
    }
}

static void _stats_init(void);

static void _set_mode(int mode)
{
    static bool first = TRUE;
    if (mode == GAME_MODE_BEGINNER)
    {
        if (first || game_mode != mode)
        {
            p_ptr->prace = RACE_HOBBIT;
            p_ptr->psubrace = 0;
            p_ptr->pclass = CLASS_ROGUE;
            p_ptr->psubclass = 0;
            p_ptr->realm1 = REALM_BURGLARY;
            p_ptr->realm2 = REALM_NONE;
            p_ptr->dragon_realm = DRAGON_REALM_NONE;
            p_ptr->personality = PERS_ORDINARY;
            _stats_init();
        }
        coffee_break = SPEED_COFFEE;
    }
    else if (mode == GAME_MODE_NORMAL)
    {
        if (first || game_mode == GAME_MODE_MONSTER)
        {
            p_ptr->prace = RACE_HOBBIT;
            p_ptr->psubrace = 0;
            p_ptr->pclass = CLASS_ROGUE;
            p_ptr->psubclass = 0;
            p_ptr->realm1 = REALM_BURGLARY;
            p_ptr->realm2 = REALM_NONE;
            p_ptr->dragon_realm = DRAGON_REALM_NONE;
            p_ptr->personality = PERS_ORDINARY;
            _stats_init();
        }
    }
    else if (mode == GAME_MODE_MONSTER)
    {
        if (first || game_mode != mode)
        {
            p_ptr->prace = RACE_MON_TROLL;
            p_ptr->psubrace = TROLL_ETTIN;
            p_ptr->pclass = CLASS_MONSTER;
            p_ptr->psubclass = 0;
            p_ptr->realm1 = REALM_NONE;
            p_ptr->realm2 = REALM_NONE;
            p_ptr->dragon_realm = DRAGON_REALM_NONE;
            _stats_init();
        }
    }
    if ((first || game_mode != mode) && mode == previous_char.game_mode && previous_char.quick_ok)
    {
        p_ptr->prace = previous_char.prace;
        p_ptr->psubrace = previous_char.psubrace;
        p_ptr->pclass = previous_char.pclass;
        p_ptr->psubclass = previous_char.psubclass;
        p_ptr->personality = previous_char.personality;
        p_ptr->realm1 = previous_char.realm1;
        p_ptr->realm2 = previous_char.realm2;
        p_ptr->dragon_realm = previous_char.dragon_realm;
        _stats_init();
    }
    game_mode = mode;
    first = FALSE;
}

/************************************************************************
 * 2) Race/Class/Personality
 *
 * Note: It assumed througout that the player starts with a valid
 * race, class and personality, and that throughout the birth process
 * these fields remain legal. Currently, player_wipe gives an Ordinary
 * Human Warrior to start.
 ***********************************************************************/ 
static void _race_class_top(doc_ptr doc);
static void _inc_rcp_state(void);
static void _change_name(void);

static int _beginner_races[] = {
  RACE_HUMAN,
  RACE_DWARF,
  RACE_HOBBIT,
  RACE_GNOME,
  RACE_DUNADAN,
  RACE_HIGH_ELF,
  RACE_HALF_TROLL,
  RACE_WEREWOLF,
  RACE_KLACKON,
  -1
};

static int _beginner_classes[] = {
  CLASS_WARRIOR,
  CLASS_MAGE,
  CLASS_PRIEST,
  CLASS_ROGUE,
  CLASS_RANGER,
  CLASS_PALADIN,
  CLASS_NINJA,
  CLASS_WEAPONSMITH,
  -1
};

static int _race_class_ui(void)
{
    if (!lukittu) bump_numeral(player_name, 1);
    lukittu = TRUE;
    for (;;)
    {
        int cmd;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(46);

        doc_insert(cols[0], "  <color:y>n</color>) Change Name\n");
        doc_insert(cols[0], "  <color:y>s</color>) Change Sex\n");
        if (game_mode != GAME_MODE_BEGINNER)
            doc_insert(cols[0], "  <color:y>p</color>) Change Personality\n");
        doc_insert(cols[0], "  <color:y>r</color>) Change Race\n");
        if (game_mode == GAME_MODE_MONSTER)
        {
            if (p_ptr->dragon_realm)
                doc_insert(cols[0], "  <color:y>m</color>) Change Magic\n");
        }
        else
        {
            doc_insert(cols[0], "  <color:y>c</color>) Change Class\n");
            if (p_ptr->realm1)
                doc_insert(cols[0], "  <color:y>m</color>) Change Magic\n");
        }
        if (game_mode != GAME_MODE_BEGINNER) doc_insert(cols[0], "  <color:y>g</color>) Change Game Speed\n");

        doc_insert(cols[1], "<color:y>  *</color>) Random Name\n");
        doc_insert(cols[1], "<color:y>  ?</color>) Help\n");
        if (game_mode != GAME_MODE_BEGINNER)
            doc_insert(cols[1], "<color:y>  =</color>) Options\n");
        doc_insert(cols[1], "<color:y>TAB</color>) More Info\n");
        doc_insert(cols[1], "<color:y>RET</color>) Next Screen\n");
        doc_insert(cols[1], "<color:y>ESC</color>) Prev Screen\n");

        doc_insert_cols(_doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, "<color:G>Tip:</color> <indent>You can often get specific "
                         "help by entering the uppercase letter for a command. For "
                         "example, type <color:keypress>R</color> on this screen "
                         "to receive help on your currently selected race.</indent>");
        _sync_term(_doc);

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            if ((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || ((personality_includes_(PERS_CHAOTIC)) && (p_ptr->pclass != CLASS_DISCIPLE)))
            {
                if (_patron_ui() != UI_OK) break;
            }
            if (_stats_ui() == UI_OK)
                return UI_OK;
            break;
        case ESCAPE:
            return UI_CANCEL;
        case '=':
            _birth_options();
            break;
        case '\t':
            _inc_rcp_state();
            break;
        case '?':
            doc_display_help("birth.txt", "RaceClass");
            break;
        case '!':
            doc_display_help("start.txt", NULL);
            break;
        case '*':
            if (one_in_(847)) sprintf(player_name, "Epic Space Hero");
            else if (one_in_(8)) randname_make(RANDNAME_SCROLL, 4 + randint0(3), 5 + damroll(2, 5), player_name, sizeof(player_name), name_sections);
            else {
                randname_make(RANDNAME_TOLKIEN, 4 + randint0(3), 5 + damroll(2, 5), player_name, sizeof(player_name), name_sections);
                if ((strlen(player_name) < 7) && (one_in_(2 * (strlen(player_name) - 1))))
                {
                    char barrel2[32];
                    randname_make(RANDNAME_TOLKIEN, 4, 14 - strlen(player_name), barrel2, sizeof(barrel2), name_sections);
                    strcat(player_name, "-");
                    strcat(player_name, barrel2);
                }
            }
            player_name[0] = toupper(player_name[0]);
            break;
        case 'n':
            _change_name();
            break;
        case 'N':
            doc_display_help("birth.txt", "CharacterName");
            break;
        case 'g':
            if (game_mode != GAME_MODE_BEGINNER)
                _speed_ui();
            break;
        case 'G':
            if (game_mode != GAME_MODE_BEGINNER)
                doc_display_help("speed.txt", NULL);
            break;
        case 'r':
            if (game_mode == GAME_MODE_BEGINNER)
                _race_ui(_beginner_races);
            else if (game_mode == GAME_MODE_MONSTER)
                _mon_race_group_ui();
            else
                _race_group_ui();
            break;
        case 'R':
        {
            race_t *race_ptr = get_race();
            if (p_ptr->prace == RACE_DEMIGOD)
                doc_display_help("Demigods.txt", race_ptr->subname);
            else if (p_ptr->prace == RACE_DRACONIAN)
                doc_display_help("Draconians.txt", race_ptr->subname);
            else if (p_ptr->prace == RACE_MON_DEMON)
                doc_display_help("Demons.txt", race_ptr->subname);
            else if (p_ptr->prace == RACE_MON_DRAGON)
                doc_display_help("Dragons.txt", race_ptr->subname);
            else if (game_mode == GAME_MODE_MONSTER)
                doc_display_help("MonsterRaces.txt", race_ptr->name);
            else
                doc_display_help("Races.txt", race_ptr->name);
            break;
        }
        case 'c':
            if (game_mode == GAME_MODE_BEGINNER)
                _class_ui(_beginner_classes);
            else if (game_mode != GAME_MODE_MONSTER)
                _class_group_ui();
            break;
        case 'C':
        {
            class_t *class_ptr = get_class();
            if (p_ptr->pclass == CLASS_WARLOCK)
                doc_display_help("Warlocks.txt", class_ptr->subname);
            else if (game_mode != GAME_MODE_MONSTER)
                doc_display_help("Classes.txt", class_ptr->name);
            break;
        }
        case 'p':
            if (game_mode != GAME_MODE_BEGINNER)
                _pers_ui();
            break;
        case 'P':
        {
            if (game_mode != GAME_MODE_BEGINNER)
            {
                personality_ptr pers_ptr = get_personality();
                doc_display_help("Personalities.txt", pers_ptr->name);
            }
            break;
        }
        case 'm':
            if (p_ptr->dragon_realm != DRAGON_REALM_NONE)
                _dragon_realm_ui();
            else if (p_ptr->realm1)
                _realm1_ui();
            break;
        case 'M':
            if (p_ptr->dragon_realm != DRAGON_REALM_NONE)
                doc_display_help("DragonRealms.txt", dragon_get_realm(p_ptr->dragon_realm)->name);
            else if (p_ptr->realm1)
                doc_display_help("magic.txt", realm_names[p_ptr->realm1]);
            break;
        case 's':
            if (p_ptr->psex == SEX_MALE)
                p_ptr->psex = SEX_FEMALE;
            else
                p_ptr->psex = SEX_MALE;
            break;
        }
    }
}

/************************************************************************
 * 2.1) Personality
 ***********************************************************************/ 
static vec_ptr _pers_choices(bool split);

static void _split_pers_ui(byte _old)
{
    byte laskuri = 0, _status[MAX_PERSONALITIES] = {0};
    vec_ptr v = _pers_choices(TRUE);
    split_copy_status(_status, FALSE);
    for (;;)
    {
        int cmd, i, split = vec_length(v) + 1;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(20);

        if (split > 7)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            personality_ptr pers_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:%c>%c</color>) <color:%c>%s</color>\n",
                _status[pers_ptr->id] ? 'y' : 'w',
                I2A(i),
                _status[pers_ptr->id] ? 'y' : 'w',
                pers_ptr->name
            );
        }

        doc_insert(_doc, "<color:G>Toggle Personalities (choose 2 or more; ENTER to accept)</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if ((cmd == '\r') || (cmd == ESCAPE))
        {
            int testi = 0;
            laskuri = 0;
            for (i = 0; ((i < MAX_PERSONALITIES) && (laskuri < 2)); i++)
            {
                if (_status[i])
                {
                    laskuri++;
                    testi = i;
                }
            }
            if (laskuri == 1)
            {
                p_ptr->personality = testi;
                break;
            }
            else if (laskuri == 0)
            {
                if (cmd == ESCAPE)
                {
                    p_ptr->personality = _old;
                    break;
                }
            }
            else break;
        }
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Personalities.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                doc_display_help("Personalities.txt", pers_ptr->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(vec_length(v));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                _status[pers_ptr->id] = 1 - _status[pers_ptr->id];
            }
        }
    }
    if (laskuri >= 2)
    {
        p_ptr->personality = PERS_SPLIT;
        split_copy_status(_status, TRUE);
    }
    else if (p_ptr->personality == PERS_SPLIT)
    {
        p_ptr->personality = _old;
        /* But what if _old was also Split... */
        if (p_ptr->personality == PERS_SPLIT) p_ptr->personality = PERS_ORDINARY;
    }
    vec_free(v);
}

static void _pers_ui(void)
{
    byte _old = p_ptr->personality;
    vec_ptr v = _pers_choices(FALSE);
    for (;;)
    {
        int cmd, i, split = vec_length(v) + 1;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(20);

        if (split > 7)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            personality_ptr pers_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                pers_ptr->id == p_ptr->personality ? 'B' : 'w',
                pers_ptr->name
            );
        }
        doc_insert(cols[vec_length(v) <= split ? 0 : 1], "  <color:y>*</color>) Random\n");

        doc_insert(_doc, "<color:G>Choose Your Personality</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) break;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Personalities.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                doc_display_help("Personalities.txt", pers_ptr->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(vec_length(v));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                p_ptr->personality = pers_ptr->id;
                break;
            }
        }
    }
    vec_free(v);

    if (p_ptr->personality == PERS_SPLIT)
    {
        _split_pers_ui(_old);
    }
//    if ((p_ptr->personality == PERS_CHAOTIC) && (p_ptr->pclass != CLASS_DISCIPLE)) (void)_patron_ui();
}

static int _pers_cmp(personality_ptr l, personality_ptr r)
{
    return strcmp(l->name, r->name);
}

static vec_ptr _pers_choices(bool split)
{
    vec_ptr v = vec_alloc(NULL);
    int i;
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        if ((split) && ((i == PERS_MUNCHKIN) || (i == PERS_SPLIT))) continue;
        vec_add(v, pers_ptr);
    }
    vec_sort(v, (vec_cmp_f)_pers_cmp);
    return v;
}

/************************************************************************
 * 2.2) Race
 ***********************************************************************/ 
static vec_ptr _get_races_aux(int ids[]);
static bool _is_valid_race_class(int race_id, int class_id);

b_race_group_t b_race_groups[B_MAX_RACE_GROUPS] = {
    { "Human",
        {RACE_AMBERITE, RACE_BARBARIAN, RACE_DEMIGOD, RACE_DUNADAN, RACE_HUMAN, RACE_IGOR, -1} },
    { "Elf",
        {RACE_DARK_ELF, RACE_HIGH_ELF, RACE_TOMTE, RACE_WOOD_ELF, -1} },
    { "Hobbit/Dwarf",
        {RACE_DWARF, RACE_GNOME, RACE_HOBBIT, RACE_NIBELUNG, -1} },
    { "Fairy",
        {RACE_SHADOW_FAIRY, RACE_SPRITE, -1} },
    { "Angel/Demon",
        {RACE_ARCHON, RACE_BALROG, RACE_IMP, -1} },
    { "Orc/Troll/Giant",
        {RACE_CYCLOPS, RACE_HALF_GIANT, RACE_HALF_ORC, RACE_HALF_TITAN,
         RACE_HALF_TROLL, RACE_KOBOLD, RACE_OGRE, RACE_SNOTLING, -1} },
    { "Shapeshifter",
        {RACE_BEORNING, RACE_DOPPELGANGER, RACE_WEREWOLF, -1} },
    { "Undead",
        {RACE_EINHERI, RACE_SKELETON, RACE_SPECTRE, RACE_VAMPIRE, RACE_ZOMBIE, -1} },
    { "Other",
        {RACE_ANDROID, RACE_BEASTMAN, RACE_BOIT, RACE_CENTAUR, RACE_DRACONIAN, RACE_ENT,
         RACE_GOLEM, RACE_KLACKON, RACE_KUTAR, RACE_MIND_FLAYER, RACE_TONBERRY, RACE_YEEK,-1 } },
};

b_race_group_t b_mon_race_groups[B_MAX_MON_RACE_GROUPS] = {
    { "Animal",
        {/*RACE_MON_ANT, RACE_MON_BEETLE, RACE_MON_BIRD, RACE_MON_CAT,*/ RACE_MON_CENTIPEDE,
            RACE_MON_HOUND, /*RACE_MON_HORSE, */ RACE_MON_HYDRA, RACE_MON_SPIDER, -1} },
    { "Angel/Demon",
        {RACE_MON_ANGEL, RACE_MON_DEMON, -1} },
    { "Beholder",
        {RACE_MON_BEHOLDER, -1} },
    { "Dragon",
        {RACE_MON_DRAGON, -1} },
    { "Elemental/Vortex",
        {RACE_MON_ELEMENTAL, RACE_MON_VORTEX, -1} },
    { "Golem",
        {RACE_MON_GOLEM, -1} },
    { "Jelly",
        {RACE_MON_JELLY, /*RACE_MON_MOLD,*/ RACE_MON_QUYLTHULG, -1} },
    { "Leprechaun",
        {RACE_MON_LEPRECHAUN, -1} },
    { "Mimic/Possessor",
        {RACE_MON_SWORD, RACE_MON_ARMOR, RACE_MON_MIMIC, RACE_MON_POSSESSOR, RACE_MON_RING, -1} },
    { "Orc/Troll/Giant",
        {RACE_MON_GIANT, /*RACE_MON_KOBOLD,*/ RACE_MON_ORC, RACE_MON_TROLL, -1} },
    { "Undead",
        {/*RACE_MON_GHOST,*/ RACE_MON_LICH, RACE_MON_MUMMY, RACE_MON_VAMPIRE, /*RACE_MON_WRAITH,*/ -1 } },
    { "Other",
        {RACE_MON_PUMPKIN, RACE_MON_XORN, -1} },
};

static void _race_group_ui(void)
{
    vec_ptr groups = vec_alloc(NULL);
    int     i;

    for (i = 0; i < B_MAX_RACE_GROUPS; i++)
    {
        b_race_group_ptr g_ptr = &b_race_groups[i];
        vec_ptr         races = _get_races_aux(g_ptr->ids);

        if (vec_length(races))
            vec_add(groups, g_ptr);
        vec_free(races);
    }

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose a Type of Race to Play</color>\n");
        for (i = 0; i < vec_length(groups); i++)
        {
            b_race_group_ptr g_ptr = vec_get(groups, i);
            doc_printf(_doc, "  <color:y>%c</color>) %s\n", I2A(i), g_ptr->name);
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) break;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Races.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else
        {
            if (cmd == '*') i = randint0(vec_length(groups));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(groups))
            {
                b_race_group_ptr g_ptr = vec_get(groups, i);
                if (_race_ui(g_ptr->ids) == UI_OK) break;
            }
        }
    }
    vec_free(groups);
}

static int _race_ui(int ids[])
{
    vec_ptr v = _get_races_aux(ids);
    int     result = UI_NONE;

    while (result == UI_NONE)
    {
        int cmd, i, split = vec_length(v) + 1;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        if (split > 7)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            race_t *race_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                race_ptr->id == p_ptr->prace ? 'B' : 'w',
                race_ptr->name
            );
        }
        doc_insert(cols[vec_length(v) < split ? 0 : 1], "  <color:y>*</color>) Random\n");

        doc_insert(_doc, "<color:G>Choose Your Race</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) result = UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Races.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                race_t *race_ptr = vec_get(v, i);
                doc_display_help("Races.txt", race_ptr->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(vec_length(v));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                int     old_id = p_ptr->prace;
                race_t *race_ptr = vec_get(v, i);

                if (p_ptr->prace != race_ptr->id)
                {
                    p_ptr->prace = race_ptr->id;
                    p_ptr->psubrace = 0;
                }
                result = _subrace_ui();
                if (result == UI_CANCEL)
                {
                    p_ptr->prace = old_id;
                    result = UI_NONE;
                }
            }
        }
    }
    vec_free(v);
    return result;
}

static vec_ptr _get_races_aux(int ids[])
{
    vec_ptr v = vec_alloc(NULL);
    int     i;

    for (i = 0; ; i++)
    {
        int id = ids[i];
        if (id == -1) break;
        if (!_is_valid_race_class(id, p_ptr->pclass)) continue;
        vec_add(v, get_race_aux(id, 0));
    }

    return v;
}

static bool _is_valid_race_class(int race_id, int class_id)
{
    if (class_id == CLASS_BLOOD_KNIGHT || class_id == CLASS_BLOOD_MAGE)
    {
        if (get_race_aux(race_id, 0)->flags & RACE_IS_NONLIVING)
            return FALSE;
    }
    if (class_id == CLASS_BEASTMASTER || class_id == CLASS_CAVALRY)
    {
        if (race_id == RACE_CENTAUR)
            return FALSE;
    }
    if (class_id == CLASS_DUELIST || class_id == CLASS_MAULER)
    {
        if (race_id == RACE_TONBERRY)
            return FALSE;
    }
    return TRUE;
}

/************************************************************************
 * 2.2.1) Subrace
 ***********************************************************************/ 
static int _subrace_ui(void)
{
    if (p_ptr->prace == RACE_DEMIGOD)
        return _demigod_ui();
    else if (p_ptr->prace == RACE_DRACONIAN)
        return _draconian_ui();
    else
    {
        p_ptr->psubrace = 0;
        return UI_OK;
    }
}

static int _subrace_ui_aux(int ct, cptr desc, cptr help, cptr topic)
{
    for (;;)
    {
        int cmd, i, split = ct + 1;
        doc_ptr cols[2];

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        _race_class_top(_doc);

        if (split > 7)
            split = (split + 1)/2;

        for (i = 0; i < ct; i++)
        {
            race_t *race_ptr = get_race_aux(p_ptr->prace, i);
            doc_printf(
                i < split ? cols[0] : cols[1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                i == p_ptr->psubrace ? 'B' : 'w',
                race_ptr->subname);
        }
        doc_insert(ct < split ? cols[0] : cols[1], "  <color:y>*</color>) Random\n");

        doc_printf(_doc, "<color:G>Choose %s</color>\n", desc);
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help(help, topic);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                race_t *race_ptr = get_race_aux(p_ptr->prace, i);
                doc_display_help(help, topic ? topic : race_ptr->subname);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(ct);
            else i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                p_ptr->psubrace = i;
                return UI_OK;
            }
        }
    }
}

static int _demigod_ui(void)
{
    assert(p_ptr->prace == RACE_DEMIGOD);
    return _subrace_ui_aux(DEMIGOD_MAX, "Demigod Parentage", "Demigods.txt", NULL);
}

static int _draconian_ui(void)
{
    assert(p_ptr->prace == RACE_DRACONIAN);
    return _subrace_ui_aux(DRACONIAN_MAX, "Draconian Subrace", "Draconians.txt", NULL);
}

/************************************************************************
 * 2.3) Class
 ***********************************************************************/ 
static vec_ptr _get_classes_aux(int ids[])
{
    vec_ptr v = vec_alloc(NULL);
    int     i;

    for (i = 0; ; i++)
    {
        int id = ids[i];
        if (id == -1) break;
        if (!_is_valid_race_class(p_ptr->prace, id)) continue;
        vec_add(v, get_class_aux(id, 0));
    }

    return v;
}

#define _MAX_CLASSES_PER_GROUP 20
#define _MAX_CLASS_GROUPS      11
typedef struct _class_group_s {
    cptr name;
    int ids[_MAX_CLASSES_PER_GROUP];
} _class_group_t, *_class_group_ptr;
static _class_group_t _class_groups[_MAX_CLASS_GROUPS] = {
    { "Melee", {CLASS_BERSERKER, CLASS_BLOOD_KNIGHT, CLASS_DUELIST, CLASS_MAULER,
                    CLASS_RUNE_KNIGHT, CLASS_SAMURAI, CLASS_WARRIOR, CLASS_WEAPONMASTER,
                    CLASS_WEAPONSMITH, -1} },
    { "Archery", {CLASS_ARCHER, CLASS_SNIPER, -1} },
    { "Martial Arts", {CLASS_FORCETRAINER, CLASS_MONK, CLASS_MYSTIC, -1} },
    { "Magic", {CLASS_BLOOD_MAGE, CLASS_BLUE_MAGE, CLASS_GRAY_MAGE, CLASS_HIGH_MAGE, CLASS_MAGE,
                    CLASS_NECROMANCER, CLASS_SORCERER, CLASS_YELLOW_MAGE, -1} },
    { "Devices", { CLASS_ALCHEMIST, CLASS_DEVICEMASTER, CLASS_MAGIC_EATER, -1} },
    { "Prayer", {CLASS_PRIEST, -1} },
    { "Stealth", {CLASS_NINJA, CLASS_ROGUE, CLASS_SCOUT, -1} },
    { "Hybrid", {CLASS_CHAOS_WARRIOR, CLASS_DISCIPLE, CLASS_NINJA_LAWYER, CLASS_PALADIN,
                    CLASS_RANGER, CLASS_RED_MAGE, CLASS_WARRIOR_MAGE, -1} },
    { "Riding", {CLASS_BEASTMASTER, CLASS_CAVALRY, -1} },
    { "Mind", {CLASS_MINDCRAFTER, CLASS_MIRROR_MASTER, CLASS_PSION,
                    CLASS_TIME_LORD, CLASS_WARLOCK, -1} },
    { "Other", {CLASS_ARCHAEOLOGIST, CLASS_BARD, CLASS_LAWYER, CLASS_POLITICIAN,
                CLASS_RAGE_MAGE, CLASS_SKILLMASTER, CLASS_TOURIST, CLASS_WILD_TALENT, -1} },
};

static void _class_group_ui(void)
{
    vec_ptr groups = vec_alloc(NULL);
    int     i;

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        _class_group_ptr g_ptr = &_class_groups[i];
        vec_ptr          classes = _get_classes_aux(g_ptr->ids);

        if (vec_length(classes))
            vec_add(groups, g_ptr);
        vec_free(classes);
    }

    for (;;)
    {
        int cmd, split = vec_length(groups) + 1;
        doc_ptr cols[2];

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        _race_class_top(_doc);

        if (split > 7)
            split = (split + 1)/2;

        for (i = 0; i < vec_length(groups); i++)
        {
            _class_group_ptr g_ptr = vec_get(groups, i);
            doc_printf(
                i < split ? cols[0] : cols[1],
                "  <color:y>%c</color>) %s\n", I2A(i), g_ptr->name);
        }
        doc_insert(cols[vec_length(groups) <= split ? 0 : 1], "  <color:y>*</color>) Random\n");

        doc_insert(_doc, "<color:G>Choose a Type of Class to Play</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) break;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Classes.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else
        {
            if (cmd == '*') i = randint0(vec_length(groups));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(groups))
            {
                _class_group_ptr g_ptr = vec_get(groups, i);
                if (_class_ui(g_ptr->ids) == UI_OK)
                    break;
            }
        }
    }
    vec_free(groups);
}

static int _class_ui(int ids[])
{
    vec_ptr v = _get_classes_aux(ids);
    int     result = UI_NONE;

    while (result == UI_NONE)
    {
        int cmd, i, split = vec_length(v) + 1;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        if (split > 7)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            class_t *class_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                class_ptr->id == p_ptr->pclass ? 'B' : 'w',
                class_ptr->name
            );
        }
        doc_insert(cols[vec_length(v) <= split ? 0 : 1], "  <color:y>*</color>) Random\n");

        doc_insert(_doc, "<color:G>Choose Your Class</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) result = UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Classes.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                class_t *class_ptr = vec_get(v, i);
                doc_display_help("Classes.txt", class_ptr->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(vec_length(v));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                class_t *class_ptr = vec_get(v, i);
                int      old_id = p_ptr->pclass;

                p_ptr->pclass = class_ptr->id;
                result = _subclass_ui();
                if (result == UI_CANCEL)
                {
                    p_ptr->pclass = old_id;
                    result = UI_NONE;
                }
            }
        }
    }
    vec_free(v);
    return result;
}

/************************************************************************
 * 2.3.1) Subclass
 ***********************************************************************/ 
static int _subclass_ui(void)
{
    for (;;)
    {
        int rc;
        bool has_subclass = TRUE;

        /* Prompt for a subclass */
        if (p_ptr->pclass == CLASS_WARLOCK)
            rc = _warlock_ui();
        else if (p_ptr->pclass == CLASS_WEAPONMASTER)
            rc = _weaponmaster_ui();
        else if (p_ptr->pclass == CLASS_DEVICEMASTER)
            rc = _devicemaster_ui();
        else if (p_ptr->pclass == CLASS_GRAY_MAGE)
            rc = _gray_mage_ui();
        else if (p_ptr->pclass == CLASS_DISCIPLE)
            rc = _patron_ui();
        else
        {
            p_ptr->psubclass = 0;
            rc = UI_OK;
            has_subclass = FALSE;
        }
        /* Cancel subclass returns to _class_ui */
        if (rc == UI_CANCEL) return UI_CANCEL;

        /* Prompt for magic */
        rc =_realm1_ui();

        /* Cancel magic stays here if there is a sublass ui */
        if (rc == UI_OK || !has_subclass) return rc;
    }
}

static int _warlock_ui(void)
{
    assert(p_ptr->pclass == CLASS_WARLOCK);
    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Warlock Pact</color>\n");
        for (i = 0; i < WARLOCK_MAX; i++)
        {
            class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                p_ptr->psubclass == i ? 'B' : 'w',
                class_ptr->subname
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Warlocks.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < WARLOCK_MAX)
            {
                class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
                doc_display_help("Warlocks.txt", class_ptr->subname);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(WARLOCK_MAX);
            else i = A2I(cmd);
            if (0 <= i && i < WARLOCK_MAX)
            {
                p_ptr->psubclass = i;
                return UI_OK;
            }
        }
    }
}

static int _weaponmaster_ui(void)
{
    assert(p_ptr->pclass == CLASS_WEAPONMASTER);
    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Speciality</color>\n");
        for (i = 0; i < WEAPONMASTER_MAX; i++)
        {
            class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                p_ptr->psubclass == i ? 'B' : 'w',
                class_ptr->subname
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Weaponmasters.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < WEAPONMASTER_MAX)
            {
                class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
                doc_display_help("Weaponmasters.txt", class_ptr->subname);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(WEAPONMASTER_MAX);
            else i = A2I(cmd);
            if (0 <= i && i < WEAPONMASTER_MAX)
            {
                p_ptr->psubclass = i;
                return UI_OK;
            }
        }
    }
}

static int _devicemaster_ui(void)
{
    assert(p_ptr->pclass == CLASS_DEVICEMASTER);
    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Speciality</color>\n");
        for (i = 0; i < DEVICEMASTER_MAX; i++)
        {
            class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                p_ptr->psubclass == i ? 'B' : 'w',
                class_ptr->subname
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Classes.txt", "Devicemaster");
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else
        {
            if (cmd == '*') i = randint0(DEVICEMASTER_MAX);
            else i = A2I(cmd);
            if (0 <= i && i < DEVICEMASTER_MAX)
            {
                p_ptr->psubclass = i;
                return UI_OK;
            }
        }
    }
}

static int _gray_mage_ui(void)
{
    assert(p_ptr->pclass == CLASS_GRAY_MAGE);
    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Bias</color>\n");
        for (i = 0; i < GRAY_MAGE_MAX; i++)
        {
            class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                p_ptr->psubclass == i ? 'B' : 'w',
                class_ptr->subname
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("Classes.txt", "Gray-Mage");
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else
        {
            if (cmd == '*') i = randint0(GRAY_MAGE_MAX);
            else i = A2I(cmd);
            if (0 <= i && i < GRAY_MAGE_MAX)
            {
                p_ptr->psubclass = i;
                return UI_OK;
            }
        }
    }
}

static int _random_patron(void)
{
    if (p_ptr->pclass == CLASS_DISCIPLE)
    {
        int patron = (MIN_PURPLE_PATRON + randint0(MAX_PURPLE_PATRON - MIN_PURPLE_PATRON));
        if ((p_ptr->psubclass >= MIN_PURPLE_PATRON) && (p_ptr->psubclass < MAX_PURPLE_PATRON)) patron = p_ptr->psubclass;
        else p_ptr->psubclass = patron;
        return patron;
    }
    return randint0(MAX_CHAOS_PATRON);
}

static int _patron_ui(void)
{
    for (;;)
    {
        int cmd, i, alku = 0, loppu = MAX_CHAOS_PATRON;

        if (p_ptr->pclass == CLASS_DISCIPLE)
        {
            alku = MIN_PURPLE_PATRON;
            loppu = MAX_PURPLE_PATRON;
            p_ptr->realm1 = REALM_NONE;
            p_ptr->realm2 = REALM_NONE;
        }

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Patron</color>\n");
        if (p_ptr->pclass != CLASS_DISCIPLE)
            doc_insert(_doc, "  <color:y>*</color>) Random\n");
        for (i = alku; i < loppu; i++)
        {
            cptr patron_name = chaos_patrons[i];
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n", I2A(i - alku), p_ptr->chaos_patron == i ? 'B' : 'w', patron_name);
        }
        if (p_ptr->pclass == CLASS_DISCIPLE)
            doc_insert(_doc, "  <color:y>*</color>) Random\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == ESCAPE) return UI_CANCEL;
        else if ((isupper(cmd)) && (p_ptr->pclass == CLASS_DISCIPLE))
        {
            i = A2I(tolower(cmd)) + alku;
            if (alku <= i && i < loppu)
            {
                class_t *class_ptr = get_class_aux(p_ptr->pclass, i);
                doc_display_help("Disciples.txt", class_ptr->subname);
            }
        }
        else
        {
            if (cmd == '*') i = RANDOM_PATRON;
            else if (cmd == '\r') i = (p_ptr->pclass == CLASS_DISCIPLE) ? DISCIPLE_YEQREZH : (((p_ptr->chaos_patron >= alku) && (p_ptr->chaos_patron < loppu)) ? p_ptr->chaos_patron : RANDOM_PATRON);
            else i = A2I(cmd) + alku;
            if ((alku <= i && i < loppu) || (i == RANDOM_PATRON))
            {
                if (i == RANDOM_PATRON)
                {
                    if (p_ptr->pclass == CLASS_DISCIPLE) p_ptr->psubclass = MAX_PURPLE_PATRON + 1;
                    i = _random_patron();
                }
                p_ptr->chaos_patron = i;
                if (p_ptr->pclass == CLASS_DISCIPLE) p_ptr->psubclass = i;
                return UI_OK;
            }
        }
    }
}

cptr _game_speed_text[GAME_SPEED_MAX] = {
 "Normal",
 "Coffee-Break",
 "Instant Coffee"
};

static int _speed_ui(void)
{
    for (;;)
    {
        int i, cmd;
        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Game Speed</color>\n");
        for (i = 0; i < GAME_SPEED_MAX; i++)
        {
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n", I2A(i), coffee_break == i ? 'B' : 'w', _game_speed_text[i]);
        }

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == ESCAPE) return UI_CANCEL;
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < GAME_SPEED_MAX)
            {
                char nimi[30];
                strcpy(nimi, "speed.txt#");
                strcat(nimi, _game_speed_text[i]);
                do {} while (clip_and_locate(" ", nimi));
                doc_display_help(nimi, NULL);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < GAME_SPEED_MAX)
            {
                coffee_break = i;
                return UI_OK;
            }
        }
    }
}

/************************************************************************
 * 2.3.2) Magic
 ***********************************************************************/ 
static int _realm1_ui(void)
{
    u32b bits = realm_choices1[p_ptr->pclass];
    int  choices[MAX_REALM];
    int  ct = 0, i;

    if (!bits)
    {
        p_ptr->realm1 = 0;
        p_ptr->realm2 = 0;
        return UI_OK;
    }

    for (i = 0; i < 32; i++)
    {
        if (bits & (1L << i))
            choices[ct++] = i+1;
    }
    choices[ct] = -1;

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Your Primary Magic Realm</color>\n");
        for (i = 0; i < ct; i++)
        {
            int id = choices[i];
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                id == p_ptr->realm1 ? 'B' : 'w',
                realm_names[id]
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "\n     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE)
            return UI_CANCEL;
        else if (cmd == '=')
            _birth_options();
        else if (cmd == '?')
            doc_display_help("magic.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                int id = choices[i];
                doc_display_help("magic.txt", realm_names[id]);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(ct);
            else i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                int id = choices[i];
                int old_id = p_ptr->realm1;
                int rc;

                p_ptr->realm1 = id;
                rc = _realm2_ui();
                if (rc == UI_CANCEL)
                    p_ptr->realm1 = old_id;
                else
                    return UI_OK;
            }
        }
    }
}

static int _realm2_ui(void)
{
    u32b bits = realm_choices2[p_ptr->pclass];
    int  choices[MAX_REALM];
    int  ct = 0, i;

    if (!bits)
    {
        p_ptr->realm2 = 0;
        return UI_OK;
    }

    if (p_ptr->pclass == CLASS_PRIEST)
    {
        if (is_good_realm(p_ptr->realm1))
            bits &= ~(CH_DEATH | CH_DAEMON);
        else
            bits &= ~(CH_LIFE | CH_CRUSADE);
    }

    for (i = 0; i < 32; i++)
    {
        int id = i + 1;
        if (bits & (1L << i) && p_ptr->realm1 != id)
            choices[ct++] = id;
    }
    choices[ct] = -1;

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Your Secondary Magic Realm</color>\n");
        for (i = 0; i < ct; i++)
        {
            int id = choices[i];
            doc_printf(_doc, "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                id == p_ptr->realm2 ? 'B' : 'w',
                realm_names[id]
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "\n     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE)
            return UI_CANCEL;
        else if (cmd == '=')
            _birth_options();
        else if (cmd == '?')
            doc_display_help("magic.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                int id = choices[i];
                doc_display_help("magic.txt", realm_names[id]);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(ct);
            else i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                int id = choices[i];
                p_ptr->realm2 = id;
                return UI_OK;
            }
        }
    }
}

/************************************************************************
 * 2.2') Monster Race (Replaces 2.2.* path when in monster mode)
 ***********************************************************************/ 

static void _mon_race_group_ui(void)
{
    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose a Type of Monster to Play</color>\n");
        for (i = 0; i < B_MAX_MON_RACE_GROUPS; i++)
        {
            b_race_group_ptr g_ptr = &b_mon_race_groups[i];
            doc_printf( _doc, "  <color:y>%c</color>) %s\n", I2A(i), g_ptr->name);
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) break;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("MonsterRaces.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(cmd);
            if (0 <= i && i < B_MAX_MON_RACE_GROUPS)
            {
                b_race_group_ptr g_ptr = &b_mon_race_groups[i];
                if (_count(g_ptr->ids) == 1)
                {
                    race_t *race_ptr = get_race_aux(p_ptr->prace, 0);
                    doc_display_help("MonsterRaces.txt", race_ptr->name);
                }
                else
                    doc_display_help("MonsterRaces.txt", NULL);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(B_MAX_MON_RACE_GROUPS);
            else i = A2I(cmd);
            if (0 <= i && i < B_MAX_MON_RACE_GROUPS)
            {
                b_race_group_ptr g_ptr = &b_mon_race_groups[i];
                if (_count(g_ptr->ids) == 1)
                {
                    int old_id = p_ptr->prace, old_sub = p_ptr->psubrace;
                    p_ptr->prace = g_ptr->ids[0];
                    if (_mon_subrace_ui() == UI_OK)
                        break;
                    else
                    {
                        p_ptr->prace = old_id;
                        p_ptr->psubrace = old_sub;
                    }
                }
                else if (_mon_race_ui(g_ptr->ids) == UI_OK)
                    break;
            }
        }
    }
}

static int _mon_race_ui(int ids[])
{
    for (;;)
    {
        int cmd, i, ct = 0;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Your Monster Race</color>\n");
        for (i = 0; ; i++)
        {
            int id = ids[i];
            race_t *race_ptr;

            if (id == -1) break;
            ct++;

            race_ptr = get_race_aux(id, 0);
            doc_printf(
                _doc,
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                race_ptr->id == p_ptr->prace ? 'B' : 'w',
                race_ptr->name
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("MonsterRaces.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                int     id = ids[i];
                race_t *race_ptr = get_race_aux(id, 0);
                doc_display_help("MonsterRaces.txt", race_ptr->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(ct);
            else i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                int     old_id = p_ptr->prace;
                int     id = ids[i];

                if (p_ptr->prace != id)
                {
                    p_ptr->prace = id;
                    p_ptr->psubrace = 0;
                }
                if (_mon_subrace_ui() == UI_CANCEL)
                {
                    p_ptr->prace = old_id;
                }
                else
                    return UI_OK;
            }
        }
    }
}

static int _mon_subrace_ui(void)
{
    if (p_ptr->prace != RACE_MON_DRAGON)
        p_ptr->dragon_realm = 0;

    if (p_ptr->prace == RACE_MON_DEMON)
        return _mon_demon_ui();
    else if (p_ptr->prace == RACE_MON_DRAGON)
        return _mon_dragon_ui();
    else if (p_ptr->prace == RACE_MON_ELEMENTAL)
        return _mon_elemental_ui();
    else if (p_ptr->prace == RACE_MON_GIANT)
        return _mon_giant_ui();
    else if (p_ptr->prace == RACE_MON_GOLEM)
        return _mon_golem_ui();
    else if (p_ptr->prace == RACE_MON_SPIDER)
        return _mon_spider_ui();
    else if (p_ptr->prace == RACE_MON_TROLL)
        return _mon_troll_ui();
    else if (p_ptr->prace == RACE_MON_ORC)
        return _mon_orc_ui();
    else
    {
        p_ptr->psubrace = 0;
        return UI_OK;
    }
}

static int _mon_demon_ui(void)
{
    assert(p_ptr->prace == RACE_MON_DEMON);
    return _subrace_ui_aux(DEMON_MAX, "Demon Subrace", "Demons.txt", NULL);
}

static int _mon_orc_ui(void)
{
    assert(p_ptr->prace == RACE_MON_ORC);
    return _subrace_ui_aux(ORC_MAX, "Orc Subrace", "Orcs.txt", NULL);
}

static int _mon_dragon_ui(void)
{
    int rc;
    assert(p_ptr->prace == RACE_MON_DRAGON);
    for (;;)
    {
        rc = _subrace_ui_aux(DRAGON_MAX, "Dragon Subrace", "Dragons.txt", NULL);
        if (rc == UI_OK)
            rc = _dragon_realm_ui();
        else return UI_CANCEL;
        if (rc == UI_OK) return rc;
    }
}

static int _dragon_realm_ui(void)
{
    vec_ptr v;
    int     i, rc = UI_NONE;

    if (p_ptr->psubrace == DRAGON_STEEL)
    {
        p_ptr->dragon_realm = DRAGON_REALM_NONE;
        return UI_OK;
    }

    v = vec_alloc(NULL);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        if (i == DRAGON_REALM_CRUSADE && p_ptr->psubrace != DRAGON_LAW && p_ptr->psubrace != DRAGON_GOLD)
            continue;
        if (i == DRAGON_REALM_DEATH && p_ptr->psubrace != DRAGON_NETHER && p_ptr->psubrace != DRAGON_CHAOS)
            continue;
        vec_add(v, realm);
    }
    assert(vec_length(v));

    while (rc == UI_NONE)
    {
        int cmd, i;

        doc_clear(_doc);
        _race_class_top(_doc);

        doc_insert(_doc, "<color:G>Choose Your Realm</color>\n");
        for (i = 0; i < vec_length(v); i++)
        {
            dragon_realm_ptr realm = vec_get(v, i);
            doc_printf(
                _doc,
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                realm->id == p_ptr->dragon_realm ? 'B' : 'w',
                realm->name
            );
        }
        doc_insert(_doc, "  <color:y>*</color>) Random\n");
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE) rc = UI_CANCEL;
        else if (cmd == '\t') _inc_rcp_state();
        else if (cmd == '=') _birth_options();
        else if (cmd == '?') doc_display_help("DragonRealms.txt", NULL);
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                dragon_realm_ptr realm = vec_get(v, i);
                doc_display_help("DragonRealms.txt", realm->name);
            }
        }
        else
        {
            if (cmd == '*') i = randint0(vec_length(v));
            else i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                dragon_realm_ptr realm = vec_get(v, i);
                p_ptr->dragon_realm = realm->id;
                rc = UI_OK;
            }
        }
    }

    vec_free(v);
    return rc;
}

static int _mon_elemental_ui(void)
{
    assert(p_ptr->prace == RACE_MON_ELEMENTAL);
    return _subrace_ui_aux(ELEMENTAL_MAX, "Elemental Subrace", "MonsterRaces.txt", "Elemental");
}

static int _mon_giant_ui(void)
{
    assert(p_ptr->prace == RACE_MON_GIANT);
    return _subrace_ui_aux(GIANT_MAX, "Giant Subrace", "MonsterRaces.txt", "Giant");
}

static int _mon_golem_ui(void)
{
    assert(p_ptr->prace == RACE_MON_GOLEM);
    return _subrace_ui_aux(GOLEM_MAX, "Golem Subrace", "MonsterRaces.txt", "Golem");
}

static int _mon_spider_ui(void)
{
    assert(p_ptr->prace == RACE_MON_SPIDER);
    return _subrace_ui_aux(SPIDER_MAX, "Spider Subrace", "MonsterRaces.txt", "Spider");
}

static int _mon_troll_ui(void)
{
    assert(p_ptr->prace == RACE_MON_TROLL);
    return _subrace_ui_aux(TROLL_MAX, "Troll Subrace", "MonsterRaces.txt", "Troll");
}


/************************************************************************
 * 3) Stats
 ***********************************************************************/ 

static int _stat_points[18] =
{ 0, 0, 0,
 -8,-7,-6,   /*  3  4  5 */
 -5,-4,-3,   /*  6  7  8 */
 -2,-1, 0,   /*  9 10 11 */
  1, 2, 3,   /* 12 13 14 */
  6,10,16 }; /* 15 16 17 */

#define _MAX_SCORE 30

static int _stats_score(void);
static void _stat_dec(int which);
static void _stat_inc(int which);
static cptr _stat_desc(int stat);

static cptr _stat_names[MAX_STATS] = { "STR", "INT", "WIS", "DEX", "CON", "CHR" };
static char _stat_to_char(int which);
static int _char_to_stat(char which);

static int _stats_ui(void)
{
    race_t         *race_ptr = get_race();
    class_t        *class_ptr = get_class();
    personality_ptr pers_ptr = get_personality();

    /* Initialize stats with reasonable defaults.
       If the user backs up and changes their class,
       pick new defaults (unless they manually made
       changes. */
    if ((_stats_changed == -1) || (_stats_changed != p_ptr->pclass))
        _stats_init();

    for (;;)
    {
        int cmd, score, i;
        doc_ptr cols[2];

        cols[0] = doc_alloc(32);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        _race_class_top(_doc);

        score = _stats_score();
        doc_insert(cols[0], "<color:G>Enter Your Starting Stats</color>\n");
        doc_insert(cols[0], "<tab:9>Base  Pts  Mod  Total\n");
        for (i = 0; i < MAX_STATS; i++)
        {
            int stat = p_ptr->stat_cur[i];
            int bonus = pers_ptr->stats[i] + race_ptr->stats[i] + class_ptr->stats[i];
            doc_printf(cols[0], "<color:y>%c</color>/<color:y>%c</color>) %-3.3s ",
                _stat_to_char(i),
                toupper(_stat_to_char(i)),
                _stat_names[i]
            );
            doc_printf(cols[0], "%4d  <color:w>%3d</color>  <color:R>%+3d</color> %6.6s\n",
                stat,
                _stat_points[stat],
                bonus,
                _stat_desc(stat + bonus)
            );
        }
        doc_printf(cols[0], "<tab:15><color:%c>%3d</color>\n",
            score <= _MAX_SCORE ? 'G' : 'r', score);

        doc_newline(cols[1]);
        doc_newline(cols[1]);
        doc_insert(cols[1], "<color:y>  n</color>) Change Name\n");
        doc_insert(cols[1], "<color:y>  ?</color>) Help\n");
        if (game_mode != GAME_MODE_BEGINNER)
            doc_insert(cols[1], "<color:y>  =</color>) Options\n");
        doc_insert(cols[1], "<color:y>TAB</color>) More Info\n");
        doc_printf(cols[1], "<color:%c>RET</color>) <color:%c>Begin Play</color>\n",
            score <= _MAX_SCORE ? 'y' : 'D',
            score <= _MAX_SCORE ? 'v' : 'D');
        doc_insert(cols[1], "<color:y>ESC</color>) Prev Screen\n");

        doc_insert_cols(_doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc,
            "<color:G>Note: </color><indent>"
            "You may adjust each starting stat using the indicated "
            "keys. The lower case key decrements while the upper case increments "
            "the starting value. Values may range from 8 to 17 and each value that "
            "you enter is charged the indicated number of points. You only have 30 "
            "points to spend, so be sure to adjust those stats that matter most."
            "</indent>");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == '\r' && score <= _MAX_SCORE)
        {
            _birth_finalize();
            return UI_OK;
        }
        else if (cmd == ESCAPE)
            return UI_CANCEL;
        else if (cmd == '?')
            doc_display_help("birth.txt", "Stats");
        else if (cmd == '!') doc_display_help("start.txt", NULL);
        else if (cmd == '\t')
            _inc_rcp_state();
        else if (cmd == '=')
            _birth_options();
        else if (cmd == 'n')
            _change_name();
        else if (cmd == 'N')
            doc_display_help("birth.txt", "CharacterName");
        else if (isupper(cmd))
        {
            i = _char_to_stat(tolower(cmd));
            if (i != A_NONE)
                _stat_inc(i);
        }
        else
        {
            i = _char_to_stat(cmd);
            if (i != A_NONE)
                _stat_dec(i);
        }
    }
}

static char _stat_cmd_map[MAX_STATS] = { 's', 'i', 'w', 'd', 'c', 'r' };
static char _stat_to_char(int which)
{
    return _stat_cmd_map[which];
}

static int _char_to_stat(char which)
{
    int i;
    for (i = 0; i < MAX_STATS; i++)
    {
        char c = _stat_cmd_map[i];
        if (c == which) return i;
    }
    return A_NONE;
}

static cptr _stat_desc(int stat)
{
    static char buf[10];
    if (stat < 3) stat = 3;
    if (stat > 40) stat = 40;
    if ((stat < 19) || (decimal_stats))
        sprintf(buf, "%2d", stat);
    else
        sprintf(buf, "18/%d", 10*(stat - 18));
    return buf;
}

static void _stats_init_aux(int stats[MAX_STATS])
{
    int i;
    for (i = 0; i < MAX_STATS; i++)
    {
        p_ptr->stat_cur[i] = stats[i];
        p_ptr->stat_max[i] = stats[i];
    }
}

static void _stats_init(void)
{
    if (p_ptr->pclass == CLASS_MONSTER)
    {
        switch (p_ptr->prace)
        {
        case RACE_MON_JELLY:
        case RACE_MON_XORN:
        case RACE_MON_HOUND:
        case RACE_MON_GIANT:
        case RACE_MON_VORTEX:
        case RACE_MON_CENTIPEDE:
        case RACE_MON_TROLL:
        case RACE_MON_HYDRA:
        case RACE_MON_SPIDER:
        case RACE_MON_LEPRECHAUN:
        case RACE_MON_ELEMENTAL:
        case RACE_MON_SWORD:
        case RACE_MON_ARMOR:
        case RACE_MON_GOLEM:
        case RACE_MON_ORC:
        case RACE_MON_PUMPKIN:
        {
            int stats[6] = { 17, 13, 8, 16, 15, 10 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_MUMMY:
        {
            int stats[6] = { 16, 12, 8, 16, 15, 15 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_ANGEL:
        {
            int stats[6] = { 16, 8, 17, 16, 11, 8 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_LICH:
        case RACE_MON_BEHOLDER:
        {
            int stats[6] = { 16, 17, 9, 9, 16, 9 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_RING:
        {
            int stats[6] = { 11, 17, 10, 10, 16, 15 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_VAMPIRE:
        {
            int stats[6] = { 16, 8, 8, 17, 11, 16 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_MIMIC:
        case RACE_MON_POSSESSOR:
        {
            int stats[6] = { 15, 15, 11, 15, 15, 15 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_QUYLTHULG:
        {
            int stats[6] = { 16, 8, 8, 11, 16, 17 };
            _stats_init_aux(stats);
            break;
        }
        case RACE_MON_DRAGON:
            if (p_ptr->dragon_realm == DRAGON_REALM_LORE)
            {
                int stats[6] = { 16, 16, 9, 16, 14, 10 };
                _stats_init_aux(stats);
            }
            else if (p_ptr->dragon_realm == DRAGON_REALM_BREATH)
            {
                int stats[6] = { 16, 13, 8, 15, 17, 10 };
                _stats_init_aux(stats);
            }
            else if (p_ptr->dragon_realm == DRAGON_REALM_CRAFT)
            {
                int stats[6] = { 16, 9, 16, 16, 14, 10 };
                _stats_init_aux(stats);
            }
            else if ( p_ptr->dragon_realm == DRAGON_REALM_DOMINATION
                   || p_ptr->dragon_realm == DRAGON_REALM_CRUSADE )
            {
                int stats[6] = { 16, 12 , 8, 15, 15, 16 };
                _stats_init_aux(stats);
            }
            else
            {
                int stats[6] = { 17, 13, 8, 16, 15, 10 };
                _stats_init_aux(stats);
            }
            break;
        case RACE_MON_DEMON:
            if (p_ptr->psubrace == DEMON_BALROG || p_ptr->psubrace == DEMON_MARILITH)
            {
                int stats[6] = { 16, 16, 9, 16, 14, 10 };
                _stats_init_aux(stats);
            }
            else if (p_ptr->psubrace == DEMON_CYBERDEMON)
            {
                int stats[6] = { 17, 13, 8, 15, 16, 10 };
                _stats_init_aux(stats);
            }
            else
            {
                int stats[6] = { 17, 13, 8, 16, 15, 10 };
                _stats_init_aux(stats);
            }
            break;
        }
    }
    else
    {
        switch (p_ptr->pclass)
        {
        case CLASS_BERSERKER:
        {
            int stats[6] = { 17, 8, 8, 17, 15, 9 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_WARRIOR:
        case CLASS_CAVALRY:
        case CLASS_MAULER:
        case CLASS_ARCHER:
        case CLASS_SAMURAI:
        case CLASS_WEAPONSMITH:
        case CLASS_WEAPONMASTER:
        case CLASS_NINJA:
        case CLASS_SNIPER:
        case CLASS_DUELIST:
        case CLASS_RAGE_MAGE:
        {
            int stats[6] = { 17, 13, 8, 16, 15, 10 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_LAWYER:
        case CLASS_NINJA_LAWYER:
        {
            int stats[6] = { 16, 8, 14, 16, 16, 11 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_ALCHEMIST:
        {
            int stats[6] = { 16, 16, 8, 14, 16, 11 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_BLOOD_KNIGHT:
        {
            int stats[6] = { 17, 13, 8, 15, 16, 10 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_MAGE:
        case CLASS_HIGH_MAGE:
        case CLASS_YELLOW_MAGE:
        case CLASS_MIRROR_MASTER:
        case CLASS_BLOOD_MAGE:
        case CLASS_BLUE_MAGE:
        case CLASS_NECROMANCER:
        {
            int stats[6] = { 16, 17, 9, 9, 16, 9 };
            _stats_init_aux(stats);
            break;
        }

        case CLASS_GRAY_MAGE:
        {
            int stats[6] = { 16, 17, 8, 13, 15, 10 };
            _stats_init_aux(stats);
            break;
        }

        case CLASS_SORCERER:
        {
            int stats[6] = { 16, 9, 9, 9, 16, 17 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_PRIEST:
        case CLASS_RANGER:
        case CLASS_PALADIN:
        case CLASS_MINDCRAFTER:
        case CLASS_FORCETRAINER:
        case CLASS_TIME_LORD:
        case CLASS_ARCHAEOLOGIST:
        case CLASS_SCOUT:
        {
            int stats[6] = { 16, 11, 16, 16, 14, 8};
            _stats_init_aux(stats);
            break;
        }
        case CLASS_ROGUE:
        case CLASS_WARRIOR_MAGE:
        case CLASS_CHAOS_WARRIOR:
        case CLASS_TOURIST:
        case CLASS_MAGIC_EATER:
        case CLASS_RED_MAGE:
        case CLASS_RUNE_KNIGHT:
        case CLASS_DEVICEMASTER:
        {
            int stats[6] = { 16, 16, 9, 16, 14, 10 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_MONK:
        {
            int stats[6] = { 16, 11, 16, 16, 14, 8 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_MYSTIC:
        {
            int stats[6] = { 16, 11, 8, 16, 14, 16 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_BEASTMASTER:
        case CLASS_BARD:
        case CLASS_POLITICIAN:
        case CLASS_WARLOCK:
        {
            int stats[6] = { 16, 8, 8, 16, 11, 17 };
            _stats_init_aux(stats);
            break;
        }
        case CLASS_DISCIPLE:
        {
            switch (p_ptr->psubclass)
            {
                case DISCIPLE_KARROT:
                {
                    int stats[6] = { 17, 15, 8, 15, 14, 13 };
                    _stats_init_aux(stats);
                    break;
                }
                case DISCIPLE_TROIKA:
                {
                    int stats[6] = { 17, 13, 8, 16, 15, 10 };
                    _stats_init_aux(stats);
                    break;
                }
                default:
                {
                    int stats[6] = { 16, 16, 9, 14, 16, 10 };
                    _stats_init_aux(stats);
                    break;
                }
            }
            break;
        }
        /* TODO */
        case CLASS_WILD_TALENT:
        case CLASS_PSION:
        default:
        {
            int stats[6] = { 15, 15, 15, 15, 15, 11 };
            _stats_init_aux(stats);
        }
        }
    }
    _stats_changed = -1;
}

static void _stat_dec(int which)
{
    int val = p_ptr->stat_cur[which];
    if (val > 8)
        val--;
    else
        val = 17;
    p_ptr->stat_cur[which] = val;
    p_ptr->stat_max[which] = val;
    _stats_changed = p_ptr->pclass;
}

static void _stat_inc(int which)
{
    int val = p_ptr->stat_cur[which];
    if (val < 17)
        val++;
    else
        val = 8;
    p_ptr->stat_cur[which] = val;
    p_ptr->stat_max[which] = val;
    _stats_changed = p_ptr->pclass;
}

static int _stats_score(void)
{
    int i, score = 0;

    for (i = 0; i < MAX_STATS; i++)
        score += _stat_points[p_ptr->stat_cur[i]];

    return score;
}

/************************************************************************
 * Low level utilities
 ***********************************************************************/ 
/* The UI is organized as follows:
    <player fields> | <info panel>
   -----------------------------------
              <menu choices>
   -----------------------------------
              <tips and help>
   ----------------------------------- */

static void _name_line(doc_ptr doc);
static void _race_class_header(doc_ptr doc);
static void _race_class_info(doc_ptr doc);

static void _race_class_top(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(25);
    cols[1] = doc_alloc(55);

    _name_line(doc);
    _race_class_header(cols[0]);
    _race_class_info(cols[1]);
    doc_insert_cols(doc, cols, 2, 1);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/* Player Fields */
static void _name_line(doc_ptr doc);
static void _sex_line(doc_ptr doc);
static void _pers_line(doc_ptr doc);
static void _race_line(doc_ptr doc);
static void _class_line(doc_ptr doc);
static void _magic_line(doc_ptr doc);

static void _race_class_header(doc_ptr doc)
{
    _sex_line(doc);
    if (game_mode != GAME_MODE_BEGINNER)
        _pers_line(doc);
    _race_line(doc);
    if (game_mode != GAME_MODE_MONSTER)
        _class_line(doc);
    _magic_line(doc);
}

static void _name_line(doc_ptr doc)
{
    int _attr = (coffee_break) ? TERM_VIOLET : TERM_VIOLET;
    doc_printf(doc, "Name : <color:B>%-32s</color>", player_name);
    if (coffee_break >= GAME_SPEED_MAX) coffee_break = 0; /* paranoia */
    doc_printf(doc, "<color:o>Game Speed:</color> ");
    doc_printf(doc, "<color:%c>%s</color>\n", attr_to_attr_char(_attr), _game_speed_text[coffee_break]);
}

static void _sex_line(doc_ptr doc)
{
    doc_printf(doc, "Sex  : <color:B>%s</color>\n", sex_info[p_ptr->psex].title);
}

static void _pers_line(doc_ptr doc)
{
    personality_ptr pers_ptr = get_personality();
    doc_printf(doc, "Pers : <color:B>%s</color>\n", pers_ptr->name);
}

static void _race_line(doc_ptr doc)
{
    race_t *race_ptr = get_race();
    if (game_mode == GAME_MODE_MONSTER)
    {
        if ( p_ptr->prace == RACE_MON_DRAGON
          || p_ptr->prace == RACE_MON_GIANT
          || p_ptr->prace == RACE_MON_ELEMENTAL
          || p_ptr->prace == RACE_MON_TROLL )
        {
            doc_printf(doc, "Race : <color:B>%s</color>\n", race_ptr->subname);
        }
        else
        {
            doc_printf(doc, "Race : <color:B>%s</color>\n", race_ptr->name);
            if (race_ptr->subname)
                doc_printf(doc, "       <color:B>%s</color>\n", race_ptr->subname);
        }
    }
    else
    {
        doc_insert(doc, "Race : <indent><color:B>");
        if (race_ptr->subname)
            doc_printf(doc, "%s ", race_ptr->subname);
        doc_printf(doc, "%s</indent></color>\n", race_ptr->name);
    }
}

static void _class_line(doc_ptr doc)
{
    class_t *class_ptr = get_class();
    doc_printf(doc, "Class: <color:B>%s</color>\n", class_ptr->name);
    if (class_ptr->subname)
    {
        if (p_ptr->pclass == CLASS_WARLOCK)
            doc_printf(doc, "Pact : <color:B>%s</color>\n", class_ptr->subname);
        else if (p_ptr->pclass == CLASS_WEAPONMASTER || p_ptr->pclass == CLASS_DEVICEMASTER)
            doc_printf(doc, "Spec : <color:B>%s</color>\n", class_ptr->subname);
        else if (p_ptr->pclass == CLASS_GRAY_MAGE)
            doc_printf(doc, "Bias : <color:B>%s</color>\n", class_ptr->subname);
        else
            doc_printf(doc, "       <color:B>%s</color>\n", class_ptr->subname);
    }
}

static void _magic_line(doc_ptr doc)
{
    if (p_ptr->dragon_realm)
    {
        dragon_realm_ptr realm = dragon_get_realm(p_ptr->dragon_realm);
        doc_printf(doc, "Magic: <color:B>%s</color>\n", realm->name);
    }
    else if (p_ptr->realm1)
    {
        doc_printf(doc, "Magic: <color:B>%s</color>\n", realm_names[p_ptr->realm1]);
        if (p_ptr->realm2)
            doc_printf(doc, "       <color:B>%s</color>\n", realm_names[p_ptr->realm2]);
    }
}

/* The Info Panel displays Stats and Skills as
   the player builds the character. Since this
   won't all fit in 80x27, we let the user <TAB>
   through the following sections */
enum {
    _RCP_STATS,
    _RCP_SKILLS1,
    _RCP_SKILLS2,
    _RCP_COUNT
};
static int _rcp_state = _RCP_STATS;

/* <TAB> rotates the state of the Info Panel */
static void _inc_rcp_state(void)
{
    ++_rcp_state;
    if (_rcp_state == _RCP_COUNT)
        _rcp_state = _RCP_STATS;
}

static void _stats_line(doc_ptr doc, s16b stats[MAX_STATS], int spell_stat, char color)
{
    int i;
    for (i = 0; i < MAX_STATS; i++)
    {
        doc_printf(doc, "<color:%c>%+3d</color>  ",
            i == spell_stat ? 'v' : color,
            stats[i]);
    }
}

static void _stats_add(s16b stats[MAX_STATS], s16b to_add[MAX_STATS])
{
    int i;
    for (i = 0; i < MAX_STATS; i++)
        stats[i] += to_add[i];
}

static void _race_class_info(doc_ptr doc)
{
    race_t *race_ptr = get_race();
    class_t *class_ptr = get_class();
    personality_ptr pers_ptr = get_personality();
    dragon_realm_ptr realm_ptr = dragon_get_realm(p_ptr->dragon_realm); /* 0 is OK */
    int spell_stat = get_spell_stat();
    
    if (_rcp_state == _RCP_STATS)
    {
        s16b stats[MAX_STATS] = {0};

        _stats_add(stats, race_ptr->stats);
        _stats_add(stats, class_ptr->stats);
        _stats_add(stats, pers_ptr->stats);
        if (p_ptr->dragon_realm)
            _stats_add(stats, realm_ptr->stats);

        doc_insert(doc, "<style:heading><color:w>  STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n");
        if (game_mode != GAME_MODE_BEGINNER)
        {
            doc_printf(doc, "  ");
            _stats_line(doc, pers_ptr->stats, spell_stat, 'G');
            doc_printf(doc, "%3d%%       %3d%%\n", pers_ptr->life, pers_ptr->exp);
        }
        doc_printf(doc, "  ");
        _stats_line(doc, race_ptr->stats, spell_stat, 'G');
        doc_printf(doc, "%3d%%  %+3d  %3d%%\n", race_ptr->life, race_ptr->base_hp, race_ptr->exp);
        if (game_mode != GAME_MODE_MONSTER)
        {
            doc_printf(doc, "  ");
            _stats_line(doc, class_ptr->stats, spell_stat, 'G');
            doc_printf(doc, "%3d%%  %+3d  %3d%%\n", class_ptr->life, class_ptr->base_hp, class_ptr->exp);
        }
        if (p_ptr->dragon_realm)
        {
            doc_printf(doc, "  ");
            _stats_line(doc, realm_ptr->stats, spell_stat, 'G');
            doc_printf(doc, "%3d%%       %3d%%\n", realm_ptr->life, realm_ptr->exp);
        }

        doc_printf(doc, "<color:R>==</color>");
        _stats_line(doc, stats, spell_stat, 'R');
        doc_printf(doc, "<color:R>%3d%%  %+3d  %3d%%</color>\n",
            race_ptr->life * class_ptr->life * pers_ptr->life * realm_ptr->life / 1000000,
            race_ptr->base_hp + class_ptr->base_hp,
            race_ptr->exp * class_ptr->exp * pers_ptr->exp * realm_ptr->exp / 1000000
        );
        doc_insert(doc, "</style>");
    }
    else if (_rcp_state == _RCP_SKILLS1 || _rcp_state == _RCP_SKILLS2)
    {
        skills_desc_t r_desc, c_desc, p_desc, dr_desc, tot_desc;
        skills_t      base, xtra;

        if (game_mode == GAME_MODE_MONSTER)
        {
            base = race_ptr->skills;
            skills_add(&base, &pers_ptr->skills);
            if (p_ptr->dragon_realm)
                skills_add(&base, &realm_ptr->skills);

            xtra = pers_ptr->skills;
            skills_scale(&xtra, 1, 5);
            if (p_ptr->dragon_realm)
            {
                skills_t tmp = realm_ptr->skills;
                skills_scale(&tmp, 1, 5);
                skills_add(&xtra, &tmp);
            }
            skills_add(&xtra, &race_ptr->extra_skills);

            skills_desc_mon_race(race_ptr, &r_desc);
            skills_desc_pers(pers_ptr, &p_desc);
            if (p_ptr->dragon_realm)
                skills_desc_realm(realm_ptr, &dr_desc);
            skills_desc_aux(&base, &xtra, &tot_desc);
        }
        else
        {
            base = class_ptr->base_skills;
            skills_add(&base, &race_ptr->skills);
            skills_add(&base, &pers_ptr->skills);

            xtra = pers_ptr->skills;
            skills_scale(&xtra, 1, 5);
            skills_add(&xtra, &class_ptr->extra_skills);

            skills_desc_race(race_ptr, &r_desc);
            skills_desc_class(class_ptr, &c_desc);
            skills_desc_pers(pers_ptr, &p_desc);
            skills_desc_aux(&base, &xtra, &tot_desc);
        }

        if (_rcp_state == _RCP_SKILLS1)
        {
            doc_printf(doc, "<color:w>   %-10.10s %-10.10s %-10.10s %-10.10s</color>\n",
                "Disarming", "Device", "Save", "Stealth");
            if (game_mode != GAME_MODE_BEGINNER)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    p_desc.dis, p_desc.dev, p_desc.sav, p_desc.stl);
            }
            doc_printf(doc, "   %s %s %s %s\n", /* Note: descriptions contain formatting tags, so %-10.10s won't work ... cf skills.c */
                r_desc.dis, r_desc.dev, r_desc.sav, r_desc.stl);
            if (game_mode != GAME_MODE_MONSTER)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    c_desc.dis, c_desc.dev, c_desc.sav, c_desc.stl);
            }
            if (p_ptr->dragon_realm)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    dr_desc.dis, dr_desc.dev, dr_desc.sav, dr_desc.stl);
            }
            doc_printf(doc, "<color:R>== </color>");
            doc_printf(doc, "%s %s %s %s\n",
                tot_desc.dis, tot_desc.dev, tot_desc.sav, tot_desc.stl);
        }
        else
        {
            doc_printf(doc, "<color:w>   %-10.10s %-10.10s %-10.10s %-10.10s</color>\n",
                "Searching", "Perception", "Melee", "Archery");
            if (game_mode != GAME_MODE_BEGINNER)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    p_desc.srh, p_desc.fos, p_desc.thn, p_desc.thb);
            }
            doc_printf(doc, "   %s %s %s %s\n",
                r_desc.srh, r_desc.fos, r_desc.thn, r_desc.thb);
            if (game_mode != GAME_MODE_MONSTER)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    c_desc.srh, c_desc.fos, c_desc.thn, c_desc.thb);
            }
            if (p_ptr->dragon_realm)
            {
                doc_printf(doc, "   %s %s %s %s\n",
                    dr_desc.srh, dr_desc.fos, dr_desc.thn, dr_desc.thb);
            }
            doc_printf(doc, "<color:R>== </color>");
            doc_printf(doc, "%s %s %s %s\n",
                tot_desc.srh, tot_desc.fos, tot_desc.thn, tot_desc.thb);
        }
    }
}

static void _change_name(void)
{
    if (!arg_lock_name)
    {
        char tmp[64];
        strcpy(tmp, player_name);
        Term_gotoxy(7, 0); /* Hack */
        if (askfor(tmp, PY_NAME_LEN))
            strcpy(player_name, tmp);
        if (0 == strlen(player_name))
            strcpy(player_name, "PLAYER");
    }
}

static int _inkey(void)
{
    return inkey_special(TRUE);
}

static void _sync_term(doc_ptr doc)
{
    rect_t r = ui_screen_rect();
    Term_load();
    doc_sync_term(doc, doc_range_top_lines(doc, r.cy), doc_pos_create(r.x, r.y));
}

static int _count(int ids[])
{
    int ct = 0, i;
    for (i = 0; ; i++)
    {
        if (ids[i] == -1) break;
        ct++;
    }
    return ct;
}

static void _birth_options(void)
{
    if (game_mode != GAME_MODE_BEGINNER)
    {
        Term_load();
        do_cmd_options_aux(OPT_PAGE_BIRTH, "Birth Option((*)s effect score)");
    }
}

static int _compare_rlvl(mon_race_ptr left, mon_race_ptr right)
{
    if (left->level < right->level)
        return -1;
    if (left->level > right->level)
        return 1;
    if (left->id < right->id)
        return -1;
    if (left->id > right->id)
        return 1;
    return 0;
}

bool _is_unique(int r_idx)
{
    mon_race_ptr race = &r_info[r_idx];
    return BOOL(race->flags1 & RF1_UNIQUE);
}

static void _bounty_uniques(void)
{
    vec_ptr v = vec_alloc(NULL);
    int     skip = 0, i;

    get_mon_num_prep(_is_unique, NULL);
    summon_specific_who = SUMMON_WHO_BIRTHER; /* XXX cf unique_count in get_mon_num_aux() */
    for (i = 0; i < MAX_KUBI; i++)
    {
        while (1)
        {
            int          id = get_mon_num(MAX_DEPTH - 1);
            mon_race_ptr race = &r_info[id];

            if ((id == MON_IMPLORINGTON) && (!no_wilderness)) continue;
            if (!(race->flags1 & RF1_UNIQUE)) continue;
            if (race->flags1 & RF1_NO_QUEST) continue;
            if (race->flagsx & RFX_WANTED) continue;
            if (!(race->flags9 & (RF9_DROP_CORPSE | RF9_DROP_SKELETON))) continue;
            if (race->rarity > 100) continue;

            race->flagsx |= RFX_WANTED;
            vec_add(v, race);
            break;
        }
    }
    summon_specific_who = SUMMON_WHO_NOBODY;

    assert(vec_length(v) == MAX_KUBI);
    vec_sort(v, (vec_cmp_f)_compare_rlvl);
    /* skip the first N wanted uniques in a reduce uniques game. we keep the
     * last and most difficult uniques only so that the player has a shot at
     * the scroll of artifact creation. */
    if (reduce_uniques_pct)
        skip = MAX_KUBI * (100 - reduce_uniques_pct) / 100;
    for (i = 0; i < MAX_KUBI; i++)
    {
        mon_race_ptr race = vec_get(v, i);
        if (i < skip)
        {
            kubi_r_idx[i] = 0;
            race->flagsx &= ~RFX_WANTED;
        }
        else
            kubi_r_idx[i] = race->id;
    }
    vec_free(v);
}

static int _boss_r_idx(void)
{
    race_ptr r = get_race();
    if (r->id == RACE_DEMIGOD)
    {
        switch (r->subid)
        {
        case DEMIGOD_ZEUS: return MON_ZEUS;
        case DEMIGOD_POSEIDON: return MON_POSEIDON;
        case DEMIGOD_HADES: return MON_HADES;
        case DEMIGOD_ATHENA: return MON_ATHENA;
        case DEMIGOD_ARES: return MON_ARES;
        case DEMIGOD_HERMES: return MON_HERMES;
        case DEMIGOD_APOLLO: return MON_APOLLO;
        case DEMIGOD_ARTEMIS: return MON_ARTEMIS;
        case DEMIGOD_HEPHAESTUS: return MON_HEPHAESTUS;
        case DEMIGOD_HERA: return MON_HERA;
        case DEMIGOD_DEMETER: return MON_DEMETER;
        case DEMIGOD_APHRODITE: return MON_APHRODITE;
        }
    }
    return r->boss_r_idx;
}

static void _reduce_uniques(void)
{
    vec_ptr  buckets[10] = {0};
    int      i, cull_pct;
    int      boss = _boss_r_idx();

    /* Perhaps there are too many uniques for your tastes? */
    if (!reduce_uniques) return;
    if (reduce_uniques_pct < 10 || reduce_uniques_pct > 90) return;

    /* I think it makes more sense that the user chooses the percentage
     * of uniques to face, not the percentage to remove. */
    cull_pct = 100 - reduce_uniques_pct;

    for (i = 0; i < 10; i++)
        buckets[i] = vec_alloc(NULL);

    /* Place all qualifying uniques into buckets by level */
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        int           bucket;

        if (!r_ptr->name) continue;
        if (r_ptr->flagsx & RFX_SUPPRESS) continue;  /* RF9_DEPRECATED */
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
        if (r_ptr->flags1 & RF1_FIXED_UNIQUE) continue;
        if (r_ptr->flagsx & RFX_WANTED) continue;
        if (!no_wilderness && (r_ptr->flags7 & RF7_GUARDIAN)) continue;
        if (r_ptr->flagsx & RFX_QUESTOR) continue; /* quests_on_birth() should be called before us */
        if (r_ptr->id == boss) continue;

        bucket = r_ptr->level / 10;
        if (bucket >= 10) continue;
        vec_add(buckets[bucket], r_ptr);
    }

    /* For each bucket, randomly suppress the requested percentage of uniques */
    for (i = 0; i < 10; i++)
    {
        vec_ptr v = buckets[i];
        int     total = vec_length(v);
        int     ct = total * cull_pct / 100;

        if (ct < total && randint0(100) < (total * cull_pct) % 100) ct++;
        if (!ct) continue;

        assert(ct <= total);
        while (ct)
        {
            int j = randint0(total);
            monster_race *r_ptr = vec_get(v, j);

            assert(r_ptr);
            assert(r_ptr->name);
            assert(!(r_ptr->flagsx & RFX_SUPPRESS));

            r_ptr->flagsx |= RFX_SUPPRESS;
            vec_delete(v, j);
            ct--;
            total--;
        }
    }

    for (i = 0; i < 10; i++)
        vec_free(buckets[i]);
}

static void _birth_finalize(void)
{
    int i;

    p_ptr->id = scores_next_id();

    /* Quick Start */
    previous_char.quick_ok = TRUE;
    previous_char.game_mode = game_mode;
    previous_char.psex = p_ptr->psex;
    previous_char.prace = p_ptr->prace;
    previous_char.psubrace = p_ptr->psubrace;
    previous_char.pclass = p_ptr->pclass;
    previous_char.psubclass = p_ptr->psubclass;
    previous_char.personality = p_ptr->personality;
    previous_char.realm1 = p_ptr->realm1;
    previous_char.realm2 = p_ptr->realm2;
    previous_char.dragon_realm = p_ptr->dragon_realm;
    previous_char.au = p_ptr->au;

    for (i = 0; i < MAX_STATS; i++)
        previous_char.stat_max[i] = p_ptr->stat_max[i];

    /* Other Initialization */
    if (game_mode == GAME_MODE_BEGINNER)
    {
        coffee_break = SPEED_COFFEE;
        effective_speed = TRUE;
        command_menu = TRUE;
        no_scrambling = TRUE;
        show_rogue_keys = TRUE;
    }

    if (coffee_break)
    {
        no_wilderness = TRUE;
        ironman_downward = TRUE;
        if (coffee_break == SPEED_INSTA_COFFEE)
        {
            thrall_mode = FALSE;
            reduce_uniques = TRUE;
            reduce_uniques_pct = 60;
            if (small_level_type != SMALL_LVL_INSTANT_COFFEE) small_level_type = SMALL_LVL_INSTANT_COFFEE_BIG;
        }
    }

    /* Confirm unusual settings
     * (players who play both coffee-break and normal games and start
     * new games by loading old savefiles sometimes turn coffee-break off,
     * but forget to turn no_wilderness and ironman_downward off) */
    if ((!coffee_break) && (ironman_downward))
    {
        if (thrall_mode) ironman_downward = FALSE; /* We start in R'lyeh... */
        else if (!get_check("Really play with ironman stairs? "))
        {
            ironman_downward = FALSE;
        }
    }
    if ((!coffee_break) && (no_wilderness))
    {
        if (!get_check("Really play with no wilderness? (This option and Normal game speed are not intended to be used together.) "))
        {
            no_wilderness = FALSE;
        }
        else
        {
            Term_clear();
            if (msg_prompt("The No Wilderness option is intended for coffee-break mode <color:v>only</color><color:w>. Normal game speed is balanced entirely around the presence of a</color> <color:G>wilderness</color> <color:w>and the</color> <color:B>many towns</color> and <color:B>dungeons</color> that come with it, while the no-wilderness option is balanced around the</color> <color:U>coffee-break</color> <color:w>and</color> <color:U>instant-coffee</color> <color:w>modes. Trying to combine Normal speed with the lack of a wilderness will make the game very <color:r>tedious</color> and <color:r>repetitive</color>, and you will miss out on much of what should make FrogComposband unique, enjoyable and engaging.\n\n</color><color:v>REALLY</color> <color:w>play with no wilderness?</color> <color:y>[y/n]</color>", "ny", PROMPT_DEFAULT) != 'y')
//        else if (!get_check("REALLY? (The no-wilderness option is intended for coffee-break mode only.)"))
            {
                no_wilderness = FALSE;
            }
        }
    }

    if ((reduce_uniques) && (coffee_break != SPEED_INSTA_COFFEE) && (reduce_uniques_pct == 60))
    {
        if (!get_check("Really reduce number of uniques by 40%? "))
        {
            reduce_uniques = FALSE;
            reduce_uniques_pct = 100;
        }
    }

    /* Can't have people sneaking out of R'lyeh by way of nexus attack... */
    if (thrall_mode) no_chris = TRUE;

    equip_init();
    pack_init();
    quiver_init();
    towns_init();
    home_init();
    virtue_init();
    cornucopia_init();
    wipe_labels();

    /* Suppress any deprecated monsters for this new game. Upgrading an existing
     * game should continue to generate deprecated monsters, since they may be wanted
     * or questors. We do this before choosing questors and bounty uniques, of course. */
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        if (r_ptr->flags9 & RF9_DEPRECATED)
            r_ptr->flagsx |= RFX_SUPPRESS;
    }

    quests_on_birth();
    _bounty_uniques(); /* go before reducing uniques for speed ... (e.g. 10% uniques) */
    _reduce_uniques(); /* quests go first, rolling up random quest uniques without restriction */

    p_ptr->au = randint1(300) + randint1(300) + 200;

    /* Everybody gets a chaos patron. The chaos warrior is obvious,
     * but anybody else can acquire MUT_CHAOS_GIFT during the game */
    if ((p_ptr->chaos_patron == RANDOM_PATRON) || ((p_ptr->pclass != CLASS_CHAOS_WARRIOR) &&
        (p_ptr->pclass != CLASS_DISCIPLE) && (!personality_includes_(PERS_CHAOTIC))) || ((p_ptr->pclass == CLASS_DISCIPLE) && (p_ptr->chaos_patron < MIN_PURPLE_PATRON)) ||
        ((p_ptr->pclass != CLASS_DISCIPLE) && (p_ptr->chaos_patron > MAX_CHAOS_PATRON)))
        p_ptr->chaos_patron = _random_patron();

    get_max_stats();
    do_cmd_rerate_aux();

    p_ptr->start_race = p_ptr->prace;
    p_ptr->start_sex = p_ptr->psex;
    p_ptr->expfact = calc_exp_factor();

    p_ptr->quest_seed = randint0(0x10000000);

    mp_ptr = &m_info[p_ptr->pclass];
    /* Hack ... external files always make easy stuff hard ... Burglary is natural for rogues!!!*/
    if (p_ptr->pclass == CLASS_ROGUE)
    {
        if (p_ptr->realm1 == REALM_BURGLARY)
            mp_ptr->spell_first = 1;
        else
            mp_ptr->spell_first = 5;
    }

    if (!ironman_shops) birth_shop_items();

    /* Rest Up to Max HP and SP */
    p_ptr->update |= PU_BONUS | PU_HP | PU_MANA;
    update_stuff();

    p_ptr->chp = p_ptr->mhp;
    p_ptr->csp = p_ptr->msp;
    process_player_name(FALSE);
}

