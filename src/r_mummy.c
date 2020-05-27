#include "angband.h"

/* Largely based on Maledicts from PosChengband R */

static int _curse_boost = 0;
static int _curse_boost_capped = 0;
static int _curse_boost_removable = 0;

static byte UglyBitTable[256] = {
0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 
2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 
4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 
3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 
4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

typedef struct
{
    u32b flag;
    int lev;
    char desc[25];
    byte attr;
    char *help;
} _curse_type;

#define _MAX_MUMMY_CURSE 25

static _curse_type _mummy_curses[_MAX_MUMMY_CURSE] =
{
    { OFC_NORMALITY, 10, "Dispel Magic", TERM_L_BLUE, "Light curse of power 1. Occasionally dispels temporary status buffs." },
    { OFC_CALL_ANIMAL, 10, "Call Animals", TERM_GREEN, "Light curse of power 1. Occasionally summons hostile animals." },
    { OFC_COWARDICE, 10, "Cowardice", TERM_YELLOW, "LIght curse of power 1. Occasionally makes you very afraid." },
    { OFC_CATLIKE, 10, "Catlike Tread", TERM_RED, "Light curse of power 1. Reduces your stealth." },
    { OFC_FAST_DIGEST, 10, "Fast Digestion", TERM_ORANGE, "Light curse of power 1. Increases your food consumption." },
    { OFC_OPEN_WOUNDS, 10, "Open Wounds", TERM_RED, "Light curse of power 1. Reduces the speed of cut healing." },
    { OFC_ADD_L_CURSE, 20, "Add Weak Curses", TERM_WHITE, "Light curse of power 1. Occasionally adds new light curses to your equipment." },
    { OFC_LOW_AC, 20, "Low AC", TERM_L_RED, "Light curse of power 1. Reduces your AC." },
    { OFC_LOW_MELEE, 20, "Miss Blows", TERM_L_GREEN, "Light curse of power 1. Reduces your melee accuracy." },
    { OFC_DRAIN_HP, 20, "Drain HP", TERM_ORANGE, "Light curse of power 1. Occasionally drains your HP." },
    { OFC_DRAIN_MANA, 20, "Drain Mana", TERM_L_BLUE, "Light curse of power 1. Occasionally drains your mana." },
    { OFC_DRAIN_PACK, 20, "Drain Pack", TERM_GREEN, "Light curse of power 1. Occasionally drains your devices." },
    { OFC_LOW_MAGIC, 20, "Induce Spell Fails", TERM_YELLOW, "Light curse of power 1. Increases your spell failure chance." },
    { OFC_LOW_DEVICE, 20, "Induce Device Fails", TERM_YELLOW, "Light curse of power 1. Increases your device failure chance." },
    { OFC_CALL_DEMON, 30, "Call Demons", TERM_L_RED, "Heavy curse of power 2. Occasionally summons hostile demons." },
    { OFC_CALL_DRAGON, 30, "Call Dragons", TERM_L_GREEN, "Heavy curse of power 2. Occasionally summons hostile dragons." },
    { OFC_TELEPORT, 30, "Random Teleportation", TERM_L_BLUE, "Heavy curse of power 2. Occasionally teleports you randomly." },
    { OFC_DRAIN_EXP, 30, "Drain Experience", TERM_YELLOW, "Heavy curse of power 2. Occasionally drains your experience." },
    { OFC_ADD_H_CURSE, 30, "Add Heavy Curses", TERM_BLUE, "Heavy curse of power 2. Occasionally adds new heavy curses to your equipment." },
    { OFC_CRAPPY_MUT, 30, "Induce Mutations", TERM_L_RED, "Heavy curse of power 2. Occasionally causes harmful mutations." },
    { OFC_AGGRAVATE, 40, "Aggravation", TERM_RED, "Heavy curse of power 3. Makes you aggravating." },
    { OFC_DANGER, 40, "Invite Danger", TERM_L_RED, "Heavy curse of power 3. Causes you to meet more dangerous monsters." },
    { OFC_BY_CURSE, 40, "Baby Foul Curse", TERM_PINK, "Heavy curse of power 3. Occasionally triggers the Baby Foul Curse." },
    { OFC_TY_CURSE, 45, "*Ancient Foul Curse*", TERM_VIOLET, "Heavy curse of power 4. Occasionally triggers the Ancient Foul Curse." },
    { OFC_PERMA_CURSE, 50, "Permanent Curse", TERM_VIOLET, "Permanent curse of power 1. Cannot be removed except by mundanizing the object. Prevents the removal and draining of Cursed and Heavily Cursed, but not of specific curses." },
};

/* The first two are just the flaggy mask; Allergy has no effect on mummies
 * anyway; Slow Regen, well, not ignoring it might just encourage some poor
 * soul to actually put it on something... */
#define _IGNORE_MASK \
    (OFC_TELEPORT_SELF | OFC_CHAINSWORD | OFC_SLOW_REGEN | OFC_ALLERGY)

/* Not sure why we don't just use count_bits()... this *is* somewhat faster
 * with a high number of curses, but count_bits() is "fast enough" */
static int _count_curses(u32b flg)
{
    int tulos = 0;
    while (flg)
    {
        tulos += UglyBitTable[(flg & 0xff)];
        flg>>=8;
    }
    return tulos;
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return result;
}


static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);   
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _nether_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 39 + (_curse_boost_capped * 8) + p_ptr->to_d_spell);
    int rad = spell_power(p_ptr->lev / 20 + 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nether Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large ball of nether.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NETHER, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static byte _boost_cap(void)
{
    return 5 + (p_ptr->lev / 5);
}

static bool _purge_curse_which(object_type *o_ptr)
{
    char o_name[MAX_NLEN];

    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_ONLY | OD_OMIT_PREFIX);

    if (!mummy_can_remove(o_ptr))
    {
        msg_format("You are not powerful enough to uncurse this object.");
        return FALSE;
    }
    else if (o_ptr->curse_flags & OFC_PERMA_CURSE)
    {
        if (o_ptr->curse_flags == (o_ptr->curse_flags & (OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED)))
        {
            msg_format("Your %s %s no removable curses.", o_name, object_plural(o_ptr) ? "have" : "has");
            return FALSE;
        }
        o_ptr->curse_flags = OFC_PERMA_CURSE;
        o_ptr->known_curse_flags = OFC_PERMA_CURSE; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("The curse on your %s is permanent. Lesser curses are stripped away.", o_name);
        return TRUE;
    }
    else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
    {
        o_ptr->curse_flags = 0;
        o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("You feel a heavy curse being lifted from your %s.", o_name);
        return TRUE;
    }
    else if (o_ptr->curse_flags & OFC_CURSED)
    {
        o_ptr->curse_flags = 0;
        o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("You feel a curse being lifted from your %s.", o_name);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool _purge_curse(void){

    obj_prompt_t prompt = {0};
    prompt.prompt = "Uncurse which item?";
    prompt.error = "There is nothing to uncurse.";
    prompt.filter = object_is_cursed;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    obj_prompt(&prompt);
    if ((!prompt.obj) || (!prompt.obj->number)) return FALSE;
    _purge_curse_which(prompt.obj);
    
    return TRUE;
}

static void _mummy_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    static char _mummy_menu_options[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int idx = ((int*)cookie)[which];
    switch (cmd)
    {
    case MENU_KEY:
    {
        var_set_int(res, _mummy_menu_options[which]);
        break;
    }
    case MENU_TEXT:
    {
        var_set_string(res, format("%-20.20s (Level %2d)", _mummy_curses[idx].desc, _mummy_curses[idx].lev));
        break;
    }
    case MENU_COLOR:
    {
        int attr = TERM_L_BLUE;
        if (p_ptr->cursed & _mummy_curses[idx].flag) attr = TERM_SLATE;
        else if (_mummy_curses[idx].lev > 40) attr = TERM_VIOLET;
        else if (_mummy_curses[idx].lev > 30) attr = TERM_L_RED;
        else if (_mummy_curses[idx].lev > 20) attr = TERM_ORANGE;
        else if (_mummy_curses[idx].lev > 10) attr = TERM_YELLOW;
        var_set_int(res, attr);
        break;
    }
    case MENU_HELP:
    {
        char buf[255];
        strcpy(buf, _mummy_curses[idx].help);
        var_set_string(res, buf);
        break;
    }
    case MENU_COLUMN:
    {
        var_set_int(res, (which >= Term->hgt - 6) ? 44 : 8);
        break;
    }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static bool _mummy_pick_curse(object_type *o_ptr)
{
    int choices[_MAX_MUMMY_CURSE];
    int i, ct = 0;
    menu_t menu = { "Add which curse?", "Browse which curse?", NULL,
                    _mummy_menu_fn, choices, 0, Term->hgt - 6};

    if ((!o_ptr) || (!o_ptr->k_idx)) return FALSE;

    for (i = 0; i < _MAX_MUMMY_CURSE; i++)
    {
        _curse_type *_curse = &_mummy_curses[i];
        if ((_curse->flag == OFC_LOW_MELEE) && (!object_is_melee_weapon(o_ptr))) continue;
        if ((_curse->flag == OFC_LOW_AC) && (!object_is_armour(o_ptr))) continue;
        if (_curse->lev > p_ptr->lev) continue;
        if (o_ptr->curse_flags & _curse->flag) continue;
        choices[ct++] = i;
    }

    if (ct == 0)
    {
        msg_print("You cannot add any further curses to this item at the moment.");
        return FALSE;
    }

    menu.count = ct;

    for (;;)
    {
        i = menu_choose(&menu);
        if (i >= 0)
        {
            char o_name[MAX_NLEN];
            int idx = choices[i];
            _curse_type *_curse = &_mummy_curses[idx];
            char c, buf[256];
            strcpy(buf, format("Add the curse of <color:%c>%s</color>? <color:y>[y/n]</color>", attr_to_attr_char(_curse->attr), _curse->desc));
            c = msg_prompt(buf, "ny", PROMPT_NEW_LINE | PROMPT_ESCAPE_DEFAULT);
            if (c == 'n') continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_OMIT_PREFIX | OD_NAME_ONLY);
            o_ptr->curse_flags |= _curse->flag;
            o_ptr->known_curse_flags |= _curse->flag;

            if (_curse->lev >= 30)
            {
                msg_format("A terrible black aura blasts your %s!", o_name);
                o_ptr->curse_flags |= (OFC_HEAVY_CURSE | OFC_CURSED);
                o_ptr->known_curse_flags |= (OFC_HEAVY_CURSE | OFC_CURSED);
            }
            else
            {
                msg_format("A black aura surrounds your %s!", o_name);
                o_ptr->curse_flags |= (OFC_CURSED);
                o_ptr->known_curse_flags |= (OFC_CURSED);
            }
            if ((!object_is_known(o_ptr)) && (o_ptr->ident & IDENT_SENSE))
            {
                o_ptr->feeling = value_check_aux1(o_ptr, TRUE);
            }
            p_ptr->update |= PU_BONUS;
            p_ptr->window |= (PW_EQUIP | PW_INVEN);
            p_ptr->redraw |= PR_EFFECTS;
            return TRUE;
        }
        return FALSE;
    }
}

static bool _curse_item_aux(void)
{
    obj_prompt_t prompt = {0};
    prompt.prompt = "Curse which item?";
    prompt.error = "There is nothing to curse.";
    prompt.filter = object_is_equipment;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;

    obj_prompt(&prompt);
    if (!prompt.obj || !prompt.obj->number) return FALSE;
    if (!_mummy_pick_curse(prompt.obj)) return FALSE;

    p_ptr->update |= (PU_BONUS);
    p_ptr->window |= (PW_INVEN | PW_EQUIP);
    p_ptr->redraw |= (PR_EFFECTS);
    handle_stuff();
    return TRUE;
}

static void _purge_curse_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Purge Curse");break;
    case SPELL_DESC: var_set_string(res, "Removes a curse from a single item. At high levels you can dispel even heavy curses."); break;
    case SPELL_CAST: var_set_bool(res, _purge_curse()); break;
    default: default_spell(cmd, res);break;
    }
}

static void _curse_item_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Curse Item"); break;
    case SPELL_DESC: var_set_string(res, "Curses a single item, consuming 3 turns."); break;
    case SPELL_CAST: var_set_bool(res,_curse_item_aux()); break;
    case SPELL_ENERGY: var_set_int(res, 300); break;
    default: default_spell(cmd, res);break;}
}

static int _curse_plev(void)
{
    switch (p_ptr->current_r_idx)
    {
        case MON_MUMMY_SORC:
            return (p_ptr->lev + 50) / 2;
        case MON_MUMMY_KING:
        case MON_GHOUL:
        case MON_GREATER_MUMMY:
            return p_ptr->lev;
        default:
            return MAX((p_ptr->lev + 1) / 2, p_ptr->lev - 10);
    }
    return 1;
}

static int _get_curse_rolls(int pow)
{
    int plev = _curse_plev();
    if (pow == 2) return 4 + (plev / 10) + (_curse_boost_capped / 3); // max 4 + 5 + 5 = 14
    else if (pow == 1) return 2 + (plev / 10) + (_curse_boost_capped * 4 / 15); // max 2 + 5 + 4 = 11
    return 2 + (plev + 10) / 20 + (_curse_boost_capped / 5); // max 2 + 3 + 3 = 8
}


static bool _inflict_curse_aux(int pow, monster_type *m_ptr, int m_idx, bool DoDamage){
    int ct = 0; int p = 0;
    int highest_power = 0;
    int dType = -1;
    int refunds = 0;
    int dmg = 0;
    int rolls = 1;
    int plev = _curse_plev();
    byte nopat[15];

    char m_name[MAX_NLEN];
    monster_desc(m_name, m_ptr, 0);

    rolls = _get_curse_rolls(pow);
    if (one_in_(3)) rolls++;

    while (ct < rolls)
    {
        if (one_in_(666) && plev > 40) nopat[ct] = 13;
        else if (one_in_(66) && plev > 40) nopat[ct] = 11;
        else if (one_in_(22) && plev > 35) nopat[ct] = 9;
        else if (one_in_(18) && plev > 30) nopat[ct] = 7;
        else if (one_in_(15) && plev > 10) nopat[ct] = 6;
        else if (one_in_(12) && plev > 10) nopat[ct] = 5;
        else if (one_in_(10)) nopat[ct] = 4;
        else if (one_in_(8)) nopat[ct] = 3;
        else if (one_in_(6)) nopat[ct] = 2;
        else if (one_in_(4)) nopat[ct] = 1;
        else nopat[ct] = 0;
        if (nopat[ct] > highest_power) highest_power = MAX(highest_power, (nopat[ct] + 1) / 2);
        ct++;
    }

    switch (highest_power){
    case 1: msg_format("%^s is cursed.", m_name); break;
    case 2: msg_format("An evil curse reaches out for %s.", m_name); break;
    case 3: msg_format("%^s is subjected to an evil curse.", m_name); break;
    case 4: msg_format("%^s takes a heavy toll from the curse.", m_name); break;
    case 5: msg_format("%^s is blasted by a mighty curse!", m_name); break;
    case 6: msg_format("<color:D>Black Omen!</color> A terrible fate awaits %s!", m_name); break;
    case 7: msg_format("The hand of death reaches for %s!", m_name); break;
    default: msg_format("A weak black aura briefly surrounds %s.", m_name); break;
    }

    ct = 0;
    while (ct <= rolls)
    {
        if ((!m_ptr) || (!m_ptr->r_idx)) return TRUE;
        ct++;
        dmg = plev + _curse_boost_capped;
        dType = -1;
        switch (nopat[ct-1])
        {
            case 13: dType = GF_DEATH_RAY; dmg = spell_power(plev * 200); break;
            case 11: dType = GF_BLOOD_CURSE; break;
            case 9:  dType = GF_HAND_DOOM; break;
            case 7:  dType = GF_ANTIMAGIC; break;
            case 6:  dType = GF_STASIS; break;
            case 5:  dType = GF_PARALYSIS; break;
            case 4:  dType = GF_OLD_CONF; break;
            case 3: dType = GF_STUN; break;
            case 2: dType = GF_TURN_ALL; break;
            case 1: dType = GF_OLD_SLOW; break;
            default: break;
        }
        p = (nopat[ct-1] + 1) / 2;

        if (r_info[m_idx].flags1 & RF1_UNIQUE){ // if it is an unique, give some refund for high-powered ones...
            if (p == 6 || p == 3 || p == 7){ refunds++; continue; }
        }
        if (dType > 0){
            u32b liput = PROJECT_KILL | PROJECT_HIDE | PROJECT_JUMP;
            if ((ct < rolls) || (DoDamage)) liput |= PROJECT_NO_PAIN;
            project(0, 0, m_ptr->fy, m_ptr->fx, dmg, dType, liput);
        }
    }
    // In addition to these things, we also have some damage
    if ((DoDamage) && (m_ptr) && (m_ptr->r_idx))
    {
        dmg = (plev / 2) * refunds + plev * (pow + 1) + _curse_boost_capped;
        project(0, 0, m_ptr->fy, m_ptr->fx, dmg, GF_NETHER, (PROJECT_KILL | PROJECT_HIDE | PROJECT_JUMP));
    }

    return TRUE;
}

static bool _inflict_curse(int pow){ 
    int m_idx = 0;
    monster_type *m_ptr;
    char m_name[MAX_NLEN];

    if (!get_direct_target()) return FALSE;

    m_idx = cave[target_row][target_col].m_idx;
    if (!m_idx) return FALSE;
    if (m_idx == p_ptr->riding) return FALSE;
    if (!player_has_los_bold(target_row, target_col)) return FALSE;
    m_ptr = &m_list[m_idx];

    if ((m_ptr) && (m_ptr->r_idx))
    {
        monster_desc(m_name, m_ptr, 0);

        if(pow==0) msg_format("<color:R>You curse %s.</color>", m_name);
        else if (pow == 1) msg_format("<color:R>You curse %s.</color>", m_name);
        else msg_format("<color:R>You curse %s.</color>", m_name);
        _inflict_curse_aux(pow, m_ptr, m_idx, TRUE);
        energy_use = 100;
        return TRUE;
    }

    return FALSE;
}

static bool _blasphemy(void)
{
    int i, afflicted = 0;
    monster_type *m_ptr;
    msg_print("You utter an ancient and terrible word.");

    for (i = 1; i < m_max; i++)
    {
        if (!m_list[i].r_idx) continue;
        m_ptr = &m_list[i];
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx)){ // Not seen
            _inflict_curse_aux(1, m_ptr, i, FALSE);
            afflicted++;
        }

    }
    if (afflicted == 0){ msg_print("Nobody hears it..."); return FALSE; }
    energy_use = 100;
    return TRUE;
}

/* BLASPHEMY */
static void _blasphemy_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Blasphemy"); break;
    case SPELL_INFO: var_set_string(res, format("%d*Curse + dam 0; dam %dd%d",  _get_curse_rolls(1), _curse_plev(), 8)); break;
    case SPELL_DESC: var_set_string(res, "Utters an accursed word, inflicting a curse on all monsters in line of sight and generating a ball of nether centered on yourself."); break;
    case SPELL_CAST: 
        if (_blasphemy()){
            project(0, 4, py, px, damroll(_curse_plev(), 8), GF_NETHER, (PROJECT_FULL_DAM | PROJECT_KILL));
            var_set_bool(res, TRUE);
        } else var_set_bool(res, FALSE);
        break;
    default:default_spell(cmd, res); break;
    }
}

static int _curse_pow(int pow)
{
    return _curse_plev() * (pow + 1) + _curse_boost_capped;
}

/* MINOR CURSE */
static void _minor_curse(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Lesser Curse"); break;
    case SPELL_INFO: var_set_string(res, format("%d*Curse + dam %d", _get_curse_rolls(0), _curse_pow(0))); break;
    case SPELL_DESC: var_set_string(res, "Invokes a minor curse on a single monster."); break;
    case SPELL_COST_EXTRA: var_set_int(res, p_ptr->lev / 20); break;
    case SPELL_CAST: var_set_bool(res, _inflict_curse(0)); break;
    default:default_spell(cmd, res);break;}
}

/* CURSE */
static void _curse_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Evil Curse"); break;
    case SPELL_INFO: var_set_string(res, format("%d*Curse + dam %d", _get_curse_rolls(1), _curse_pow(1))); break;
    case SPELL_DESC: var_set_string(res, "Invokes a curse on a single monster."); break;
    case SPELL_COST_EXTRA: var_set_int(res, MIN(4, p_ptr->lev / 10)); break;
    case SPELL_CAST: var_set_bool(res, _inflict_curse(1)); break;
    default:default_spell(cmd, res); break;}
}

/* MAJOR CURSE */
static void _major_curse(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Mighty Curse"); break;
    case SPELL_INFO: var_set_string(res, format("%d*Curse + dam %d", _get_curse_rolls(2), _curse_pow(2))); break;
    case SPELL_DESC: var_set_string(res, "Invokes a terrible curse on a single monster."); break;
    case SPELL_COST_EXTRA: var_set_int(res, MIN(12, p_ptr->lev / 4)); break;
    case SPELL_CAST: var_set_bool(res,_inflict_curse(2)); break;
    default:default_spell(cmd, res); break;
    }
}
/*static void _sense_misfortune(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Sense Misfortune"); break;
    case SPELL_DESC: var_set_string(res, "Senses monsters and traps in range. At high level, also maps the area."); break;
    case SPELL_INFO: var_set_string(res, info_radius(26 + _curse_boost_capped)); break;
    case SPELL_CAST: 
        msg_print("You attempt to sense misfortune... \n");
        detect_monsters_evil( 26 + _curse_boost_capped );
        detect_traps(26 + _curse_boost_capped, FALSE);
        if (p_ptr->lev > 40){ map_area(26 + _curse_boost_capped); }
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}*/
static void _curse_of_impotence(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Curse of Impotence"); break;
    case SPELL_DESC: var_set_string(res, "Curses all creatures with impotence."); break;
    case SPELL_CAST: 
        num_repro += MAX_REPRO; 
        msg_print("You feel a tangible increase in abstinence...");
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}

static void _dimensional_anchor(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Dimensional Lock"); break;
    case SPELL_DESC: var_set_string(res, "Locks things in place, preventing almost all teleportation."); break;
    case SPELL_CAST:{
        _toggle_spell(MUMMY_TOGGLE_ANTITELE, cmd, res);
        if (_get_toggle() == MUMMY_TOGGLE_ANTITELE) msg_print("Everything is locked down in space.");
        else msg_print("Dimensional anchoring vanishes.");
        var_set_bool(res, TRUE);
        break;
    }
    default:default_spell(cmd, res); break;
    }
}

static void _absorb_curse_pow(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Absorb Curse Power"); break;
    case SPELL_DESC: var_set_string(res, "Purges all curses from equipment to heal self. A successful casting consumes less time (down to 0.25 turns) at higher power."); break;
    case SPELL_INFO: var_set_string(res, info_heal(0, 0, _curse_boost_removable * 70)); break;
    case SPELL_CAST:{
        int old_cursepow = _curse_boost_removable;
        if (old_cursepow == 0){ msg_print("You are not carrying any dispellable curses."); var_set_bool(res, FALSE); break; }
        msg_print("You absorb the power of evil curses!");
        hp_player(old_cursepow *70);
        remove_all_curse();
        if (old_cursepow >= 5)
        {
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            set_blind(0, TRUE);
            energy_use = 100 - (old_cursepow * 5); // super-quick too.
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:default_spell(cmd, res); break;
    }
}

static void _drain_curse_pow(int cmd, variant *res)
{
    int pow = _curse_plev() / 2;
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Drain Curse Power"); break;
    case SPELL_DESC: var_set_string(res, "Drains cursed equipment to replenish mana."); break;
    case SPELL_INFO: var_set_string(res, format("pow %d+1d%d", pow, pow)); break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        prompt.prompt = "Drain which item?";
        prompt.error = "You have no cursed equipment to drain.";
        prompt.filter = object_is_cursed;
        prompt.where[0] = INV_EQUIP;
        obj_prompt(&prompt);
        var_set_bool(res, FALSE);
        if ((!prompt.obj) || (!(prompt.obj->number))) break;

        if (_purge_curse_which(prompt.obj))
        {
            p_ptr->csp += (pow) + randint1(pow);
            if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;
            var_set_bool(res, TRUE);
        }
        else
        {
            var_set_bool(res, FALSE);
        }
        break;
    }
    default: default_spell(cmd, res); break;
    }
}

static void _umbra_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Umbra"); break;
    case SPELL_DESC: var_set_string(res, "Shrouds you in shadows, making you stealthier and mitigating the effects of aggravation."); break;
    case SPELL_INFO: var_set_string(res, info_duration(p_ptr->lev / 3 + _curse_boost_capped * 3, p_ptr->lev / 3 + _curse_boost_capped * 3)); break;
    case SPELL_CAST:
        if (p_ptr->cur_lite > 0)
        {
            msg_print("You are carrying too much light to hide yourself in shadows!");
            var_set_bool(res, FALSE);
        }
        set_tim_dark_stalker(spell_power(p_ptr->lev / 3 + _curse_boost_capped * 3 + randint1(p_ptr->lev / 3 + _curse_boost_capped * 3)), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}

static void _assess_curses_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Assess Curses"); break;
    case SPELL_DESC: var_set_string(res, "Analyzes the power of the evil enchantments surrounding you."); break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_curse_boost < 1)
        {
            msg_print("You do not currently receive any bonuses from cursed equipment.");
            break;
        }
        else
        {
            int hat = _boost_cap();
            if (_curse_boost_capped == hat)
            {
                msg_format("The curses on your equipment have a total power of <color:o>%d</color>, giving you a <color:o>Level %d</color> boost, the highest%s possible.", _curse_boost, _curse_boost_capped, (p_ptr->lev == 50) ? "" : "currently");
            }
            else
            {
                if (p_ptr->cur_lite > 0) msg_format("The curses on your equipment have a nominal total power of <color:o>%d</color>, but this is weakened by the permanent light around you. You receive a <color:o>Level %d</color> boost.", _curse_boost, _curse_boost_capped);
                else msg_format("The curses on your equipment have a total power of <color:o>%d</color>, giving you a <color:o>Level %d</color> boost. With additional curses you could reach a boost of <color:o>Level %d</color>.", _curse_boost, _curse_boost_capped, hat);
            }
            var_set_bool(res, TRUE);
        }
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:default_spell(cmd, res); break;
    }
}

static bool _unleash(void)
{    
    msg_print("All malice is released!");

    if(_curse_boost_removable >= 6) cast_destruction();
    if(_curse_boost_removable >= 5) project_hack(GF_BLOOD_CURSE, _curse_boost_removable * 25);
    project(0, 10, py, px, damroll(_curse_boost_removable, 25), GF_MANA, (PROJECT_FULL_DAM | PROJECT_KILL));
    hp_player(_curse_boost_removable * 50);
    set_stun(0, TRUE);
    set_cut(0, TRUE);
    set_blind(0, TRUE);

    remove_all_curse();
    energy_use = 100 - (_curse_boost_removable * 5);
    return TRUE;
}

static void _unleash_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Unleash Malice"); break;
    case SPELL_DESC: var_set_string(res, "Releases the evil power of your equipment curses. Destroys the area at high powers. A successful casting consumes less time (down to 0.25 turns) at higher power."); break;
    case SPELL_INFO: {
        if (_curse_boost_removable > 2){
            if (_curse_boost_removable >= 5)
            var_set_string(res, format("dam %dd25+%d; heal %d", _curse_boost_removable, _curse_boost_removable * 25, _curse_boost_removable * 50));
            else var_set_string(res, format("dam %dd25; heal %d", _curse_boost_removable, _curse_boost_removable * 50));
        }
        else var_set_string(res, "");
        break;
    }
    case SPELL_CAST:{ 
        if (_curse_boost_capped > 2) var_set_bool(res, _unleash()); 
        else
        {
            msg_print("There isn't enough malice in you... ");
            var_set_bool(res, FALSE);
        }
        break;
    }
    default:default_spell(cmd, res); break;
    }
}
static power_info _powers[] = {
    { A_CHR, { 3,  0, 20, _purge_curse_spell } },
    { A_CHR, { 10,  0, 35, _curse_item_spell } },
    { A_CHR, { 10,  0, 0, _assess_curses_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _draugr_powers[] = {
    { A_STR, { 36, 15, 30, building_up_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _sorc_powers[] = {
    { A_CHR, { 36, 30, 30, _nether_ball_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info *_get_powers(void) {
    static power_info spells[MAX_SPELLS];
    int max = MAX_SPELLS;
    int ct = get_powers_aux(spells, max, _powers, FALSE);
    if (p_ptr->current_r_idx == MON_DRAUGR)
        ct += get_powers_aux(spells + ct, max - ct, _draugr_powers, FALSE);
    else if (p_ptr->current_r_idx == MON_MUMMY_SORC)
        ct += get_powers_aux(spells + ct, max - ct, _sorc_powers, FALSE);
    spells[ct].spell.fn = NULL;
    return spells;
}

bool mummy_ty_protection(void){
    if (p_ptr->prace != RACE_MON_MUMMY) return FALSE;
    if (p_ptr->lev > 30){
        if (one_in_(2)) return TRUE;
    }
    return FALSE;
}

bool mummy_can_remove(object_type *o_ptr)
{
    if (p_ptr->prace != RACE_MON_MUMMY) return FALSE;
    if (o_ptr->curse_flags & OFC_PERMA_CURSE) return FALSE;
    if ((o_ptr->curse_flags & OFC_HEAVY_CURSE) && (p_ptr->lev < 25)) return FALSE;
    if (p_ptr->lev < 3) return FALSE;
    return TRUE;
}

int mummy_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->prace == RACE_MON_MUMMY)
        result = _get_toggle();
    return result;
}

static spell_info _get_spells[] =
{
    /*lvl cst fail spell */
    { 1, 8, 30, _minor_curse }, // debuff
//    { 5, 5, 40, _sense_misfortune }, // detect traps / monsters
    { 10, 15, 40, _curse_of_impotence }, // no breeding
    { 15, 8, 45,  _drain_curse_pow }, // attempts to remove curse from one equipment, restores mana based on curses.
    { 20, 20, 45, _umbra_spell}, // Stealth buff, cancels out aggravation
    { 24, 16, 45, _curse_spell}, // stronger debuff + damage
    { 28, 0, 0, _dimensional_anchor }, // -TELE on self and everyone on sight
    { 32, 20, 40, animate_dead_spell },
    { 36, 30, 45, _absorb_curse_pow }, // Remove curse & heal
    { 40, 110, 45, _blasphemy_spell },
    { 45, 48, 45, _major_curse }, // crippling debuff
    { 50, 100, 50, _unleash_spell }, // *remove curse, heal, LOS effects depending on the curse_power. Requires curses to be present.
    { -1, -1, -1, NULL }
};

/**********************************************************************
 * Mummy Equipment
 **********************************************************************/
static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_ZOMBIE_H;
    equip_on_change_race();
    skills_innate_init("Claw", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Gaze", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Crush", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Hit", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_ROBE));
    py_birth_obj(&forge);
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
}

/**********************************************************************
 * Mummy Attacks
 **********************************************************************/

/* Not a physical attack, so undo physical bonuses */
void _gaze_adjustments(innate_attack_ptr a)
{
    a->to_d -= ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
    a->to_h -= ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);
    a->to_h -= ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
    if (IS_HERO()) a->to_h -= 12;
    if (IS_SHERO())
    {
        a->to_d -= 3+(p_ptr->lev/5);
        a->to_h -= 12;
    }
}

void _zombie_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Hit */
    {
        innate_attack_t    a = {0};

        a.dd = 3 + l / 15;
        a.ds = 3 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += 10 + _curse_boost_capped;

        a.effect[0] = GF_MISSILE;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You hit.";
        a.name = "Hit";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _ghoul_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 13;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;

        a.weight = 100;
        calc_innate_blows(&a, 164);
        a.blows += py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 2 + l / 12;
        a.ds = 2 + l / 12;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_PARALYSIS;
        a.effect_chance[1] = 25;

        a.weight = 100;
        calc_innate_blows(&a, 164);
        a.blows += py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You bite.";
        a.name = "Bite";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _sorc_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 12;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;
        a.effect_chance[1] = 50;
        a.effect[2] = GF_DISENCHANT;
        a.effect_chance[2] = 25;
        a.effect[3] = GF_BLIND;
        a.effect_chance[3] = 10;

        a.weight = 100;
        calc_innate_blows(&a, 200);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT | INNATE_NO_DAM;

        a.weight = 150;
        a.effect[0] = GF_TURN_ALL;

        a.blows = 100 + py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "Gaze";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _draugr_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 100;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 40;
        a.effect[2] = GF_DRAIN_MANA;
        a.effect_chance[2] = 20;

        a.blows = 100 + py_prorata_level_aux(_curse_boost_capped*5, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "Gaze";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 2 + l / 9;
        a.ds = 1 + l / 9;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;

        a.weight = 150;
        calc_innate_blows(&a, 150);
        a.blows += py_prorata_level_aux(_curse_boost_capped*5, 1, 1, 0);
        a.msg = "You bite.";
        a.name = "Bite";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Crush */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 9;
        a.ds = 2 + l / 9;
        a.to_d += _curse_boost_capped;
        a.to_h += 5 + _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_BABY_SLOW;
        a.effect_chance[1] = 40;
        a.effect[2] = GF_STUN;
        a.effect_chance[2] = 20;

        a.weight = 250;
        a.blows = 100;
        a.msg = "You crush.";
        a.name = "Crush";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _gmum_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_OLD_DRAIN;
        a.effect_chance[1] = 50;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 150;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 50;

        a.blows = 200 + py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "Gaze";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _king_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_OLD_DRAIN;
        a.effect[2] = GF_BLIND;
        a.effect_chance[2] = 10;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 150;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 50;
        a.effect[2] = GF_DRAIN_MANA;
        a.effect_chance[2] = 25;

        a.blows = 200 + py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "Gaze";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}
 
void _calc_innate_attacks(void)
{
    if ((p_ptr->weapon_ct > 0) || (!equip_find_empty_hand())) return;
    switch (p_ptr->current_r_idx)
    {
        case MON_ZOMBIE_H:
        case MON_MUMMY_H:
            _zombie_innate_attacks();
            break;
        case MON_GHOUL:
            _ghoul_innate_attacks();
            break;
        case MON_GREATER_MUMMY:
            _gmum_innate_attacks();
            break;
        case MON_DRAUGR:
            _draugr_innate_attacks();
            break;
        case MON_MUMMY_SORC:
            _sorc_innate_attacks();
            break;
        case MON_MUMMY_KING:
            _king_innate_attacks();
            break;
        default: break;
    }
}

static void _calc_bonuses_aux(void)
{
    int slot;
    u32b checklist = 0, perm_flags = 0;
    int basePow = 0;
    int perm_ct = 0;
    int osumat = 0;
    int boost_cap = _boost_cap();
    int boost = 0;
    int removable = 0;

    if (!p_ptr->cursed)
    {
        _curse_boost = 0;
        _curse_boost_capped = 0;
        _curse_boost_removable = 0;
        return; /* Easy */
    }

    for (slot = equip_find_first(object_is_cursed); slot; slot = equip_find_next(object_is_cursed, slot))
    {
        object_type *o_ptr = equip_obj(slot);
        u32b flgs[OF_ARRAY_SIZE];
        u32b liput = o_ptr->curse_flags;

        obj_flags(o_ptr, flgs);

        if (o_ptr->curse_flags & OFC_PERMA_CURSE)
        {
            basePow += 3;
            perm_ct += 3;
            liput &= ~(OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED);
        }
        else if (o_ptr->curse_flags & OFC_HEAVY_CURSE) basePow += 2;
        else if (o_ptr->curse_flags & OFC_CURSED) basePow++;
        if (obj_is_blessed(o_ptr)) basePow -= 2;
        if (have_flag(flgs, OF_TY_CURSE)) perm_flags |= OFC_TY_CURSE;
        if (have_flag(flgs, OF_AGGRAVATE)) perm_flags |= OFC_AGGRAVATE;
        if (have_flag(flgs, OF_DRAIN_EXP)) perm_flags |= OFC_DRAIN_EXP;
        checklist |= (liput);
    }
    checklist &= ~(_IGNORE_MASK);
    osumat = _count_curses(checklist);
    /* Special curses
     * Note: We don't reward perma-curses very heavily to prevent constant
     * high power from curses that aren't actually removed */
    if (checklist & OFC_TY_CURSE) osumat += 2;
    else if (perm_flags & OFC_TY_CURSE) boost += 4; /* 2 for TY bonus, 1 for extra curse and 1 for extra heavy curse */
    if (checklist & OFC_AGGRAVATE) osumat += 1;
    else if (perm_flags & OFC_AGGRAVATE) boost += 3;
    if (checklist & OFC_DANGER) osumat += 1;
    if (checklist & OFC_BY_CURSE) osumat += 1;
    if ((perm_flags & OFC_DRAIN_EXP) && (!(checklist & OFC_DRAIN_EXP))) boost += 2;
    boost += osumat + (basePow / 2);
    removable += osumat + ((basePow - perm_ct) / 2);
    if (perm_ct > 0)
    {
        /* Check for curses that were hidden but are actually present */
        boost++; /* Perma-curse itself is such a curse, it's always hidden! */
        if (!(checklist & OFC_HEAVY_CURSE)) boost++;
        if (!(checklist & OFC_CURSED)) boost++;
    }
    checklist &= (TRC_HEAVY_MASK);
    osumat = _count_curses(checklist);
    boost += osumat;
    removable += osumat;

    _curse_boost = boost; // save up the uncapped boost.
    boost = MAX(0, MIN(boost_cap, boost / 3));
    removable = MAX(0, MIN(boost_cap, removable / 3));
    if (p_ptr->cur_lite > 0)
    {
        int malus = p_ptr->cur_lite + 5;
        if (malus >= 10)
        {
            boost = 0;
            removable = 0;
        }
        else
        {
            boost -= (boost * malus / 10);
            removable -= (removable * malus / 10);
        }
        if (p_ptr->tim_dark_stalker)
        {
            set_tim_dark_stalker(0, TRUE);
        }
    }
    _curse_boost_capped = boost;
    _curse_boost_removable = removable;
}

static void _calc_bonuses(void)
{
    int boost;

    _calc_bonuses_aux();

    boost = _curse_boost_capped;
    p_ptr->pspeed += boost / 3;
    if (p_ptr->current_r_idx == MON_MUMMY_SORC) p_ptr->pspeed += boost / 3;

    p_ptr->weapon_info[0].xtra_blow += py_prorata_level_aux(boost*10, 1, 1, 1);
    p_ptr->weapon_info[1].xtra_blow += py_prorata_level_aux(boost*10, 1, 1, 1);

    p_ptr->weapon_info[0].to_h += boost; p_ptr->weapon_info[1].to_h += boost;
    p_ptr->to_h_m += boost;
    p_ptr->weapon_info[0].dis_to_h += boost; p_ptr->weapon_info[1].dis_to_h += boost;

    p_ptr->weapon_info[0].to_d += boost; p_ptr->weapon_info[1].to_d += boost;
    p_ptr->to_d_m += boost;
    p_ptr->weapon_info[0].dis_to_d += boost; p_ptr->weapon_info[1].dis_to_d += boost;
    p_ptr->see_nocto = TRUE;

    res_add(RES_POIS);
    res_add(RES_COLD);
    res_add(RES_NETHER);
    if (p_ptr->lev < 10) p_ptr->regen -= (p_ptr->regen / 4);
    if (p_ptr->lev >= 27)
    {
        res_add(RES_ELEC);
        p_ptr->slow_digest = TRUE;
    }
    if (p_ptr->lev >= 36)
    {
        res_add(RES_DARK);
        res_add(RES_ACID);
    }
    switch (p_ptr->current_r_idx)
    {
        case MON_MUMMY_SORC:
            p_ptr->skills.dev += 16;
            res_add_vuln(RES_LITE);
            break;
        case MON_MUMMY_KING:
            p_ptr->skills.dev += 8;
            break;
        case MON_DRAUGR:
            p_ptr->skills.dev -= 8;
            break;
        case MON_GHOUL:
            res_add_vuln(RES_LITE);
            break;
        default: break;
    }
    if (mummy_get_toggle() == MUMMY_TOGGLE_ANTITELE) p_ptr->anti_tele = TRUE;
    p_ptr->regen += _curse_boost_capped * 10;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_NIGHT_VISION);
    if (p_ptr->lev >= 27)
    {
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_SLOW_DIGEST);
    }
    if (p_ptr->lev >= 36)
    {
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_RES_ACID);
    }
    if ((p_ptr->current_r_idx == MON_MUMMY_SORC) || (p_ptr->current_r_idx == MON_GHOUL))
    {
        add_flag(flgs, OF_VULN_LITE);
    }
    if (_curse_boost_capped) add_flag(flgs, OF_REGEN);
}

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_ZOMBIE_H && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_MUMMY_H;
        msg_print("You have evolved into a Mummified human.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MUMMY_H && new_level >= 18)
    {
        p_ptr->current_r_idx = MON_GHOUL;
        msg_print("You have evolved into a Ghoul.");
    }
    if ((p_ptr->current_r_idx == MON_MUMMY_H || p_ptr->current_r_idx == MON_GHOUL) && (new_level >= 27))
    {
        p_ptr->current_r_idx = MON_GREATER_MUMMY;
        msg_print("You have evolved into a Greater mummy.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_GREATER_MUMMY && new_level >= 36)
    {
        if (one_in_(2))
        {
            p_ptr->current_r_idx = MON_DRAUGR;
            msg_print("You have evolved into a Draugr.");
        }
        else
        {
            p_ptr->current_r_idx = MON_MUMMY_SORC;
            msg_print("You have evolved into a Mummified sorcerer.");
        }
        p_ptr->redraw |= PR_MAP;
    }
    if ((p_ptr->current_r_idx != MON_MUMMY_KING) && (new_level >= 45))
    {
        p_ptr->current_r_idx = MON_MUMMY_KING;
        msg_print("You have evolved into a Mummy king.");
        p_ptr->redraw |= PR_MAP;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = { 0 };
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "curse power";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 420;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.min_fail = 5;
        init = TRUE;
    }
    if ((me.min_fail == 5) && ((p_ptr->current_r_idx == MON_MUMMY_SORC) ||
        (p_ptr->current_r_idx == MON_MUMMY_KING)))
    {
        me.min_fail = 3;
    }
    return &me;
}

race_t *mon_mummy_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static int    init_race = -1;

    if (!init)
    {   /* dis, dev, sav, stl, srh, fos, thn, thb */
        skills_t bs = { 20, 18, 32, 1, 12, 6, 60, 35 };
        skills_t xs = { 7,  6,  12, 0,  0, 0, 20, 17 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Mummy";
        me.desc = "Zombies and mummies are among the most feared of all undead races. "
                    "Evil curses are a mummy's bread and butter; not only can they hex their "
                    "opponents, they can also curse and uncurse their own equipment at will; "
                    "and the stronger and more numerous the curses they are wrapped in, the "
                    "greater their magical powers. Of course, they will also have to cope "
                    "with being cursed...\n\n"
                    "You begin unlife as a humble Zombified human, with few curses or other "
                    "abilities at your disposal; but soon you will evolve into a a "
                    "Mummified human, and things really start picking up. Mummy evolution is "
                    "somewhat random - you might find yourself a strong but stupid Draugr or a wily "
                    "but weak Mummified sorcerer - but ultimately, you will combine strength "
                    "and magic in the spine-chilling final form of a Mummy king.\n\n"
                    "Being creatures of tombs and the night, zombies and mummies can easily see "
                    "without a light; indeed, carrying a light source saps the dark energy of "
                    "their curses. In melee, they prefer to rely on their innate attacks; "
                    "equipping a weapon serves only to distract them from their real fighting "
                    "skills, and is only a viable offensive option in the earliest parts of the game.";

        me.infra = 5;
        me.exp = 135;
        me.base_hp = 22;
        me.shop_adjust = 120;

        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers_fn = _get_powers;
        me.get_spells = _get_spells;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.caster_info = _caster_info;
        me.birth = _birth;
        me.boss_r_idx = MON_OSIRIS;

        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START | RACE_EATS_DEVICES;
        me.pseudo_class_idx = CLASS_RAGE_MAGE;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    if ((init_race != p_ptr->current_r_idx) || (birth_hack) || (spoiler_hack))
    {
        int _my_race = p_ptr->current_r_idx;
        if (spoiler_hack) _my_race = MON_GREATER_MUMMY;
        else if (birth_hack)
        {
            _my_race = MON_ZOMBIE_H;
            init_race = -1; /* paranoia */
        }
        else if (_my_race) init_race = _my_race;
        switch (_my_race)
        {
            case MON_DRAUGR:
                me.life = 110;
                me.stats[A_STR] = 2;
                me.stats[A_INT] = -3;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -1;
                me.stats[A_CON] = 2;
                me.stats[A_CHR] = -3;
            break;
            case MON_MUMMY_SORC:
                me.life = 92;
                me.stats[A_STR] = -3;
                me.stats[A_INT] = 2;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -2;
                me.stats[A_CON] = -2;
                me.stats[A_CHR] = 1;
            break;
            case MON_MUMMY_KING:
                me.life = 105;
                me.stats[A_STR] = 1;
                me.stats[A_INT] = 0;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -1;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = 0;
            break;
            case MON_GREATER_MUMMY:
                me.life = 100;
                me.stats[A_STR] = -1;
                me.stats[A_INT] = -2;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -2;
                me.stats[A_CON] = 0;
                me.stats[A_CHR] = -1;
            break;
            case MON_GHOUL:
                me.life = 95;
                me.stats[A_STR] = -2;
                me.stats[A_INT] = 0;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -3;
                me.stats[A_CON] = -2;
                me.stats[A_CHR] = -1;
            break;
            case MON_MUMMY_H:
                me.life = 100;
                me.stats[A_STR] = 0;
                me.stats[A_INT] = -3;
                me.stats[A_WIS] = -4;
                me.stats[A_DEX] = 0;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = -3;
            break;
            default:
                me.life = 100;
                me.stats[A_STR] = 0;
                me.stats[A_INT] = -4;
                me.stats[A_WIS] = -6;
                me.stats[A_DEX] = 0;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = -3;
            break;
        }
    }

    return &me;
}
