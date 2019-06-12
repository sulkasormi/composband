#include "angband.h"

#include <assert.h>

static cptr _desc = 
    "Dragons are powerful winged serpents. They are the strongest fighters "
    "with razor sharp claws and a bone crushing bite. Each dragon has a unique "
    "type of breath that becomes more deadly as the dragon grows and matures.\n \n"
    "Due to their non-humanoid bodies, dragons are unable to wear armor, gloves "
    "or boots. However, being creatures of magic, they are able to wear 6 rings. "
    "They can also wear a helmet, a light source, a cloak and an amulet. Because "
    "of these equipment restrictions, dragons may have a difficult time covering "
    "all resistances despite the fact that each dragon has one or more innate "
    "resistances (or even immunities).\n \n"
    "Dragons begin life in a weak form, being very young. As their bodies mature, "
    "their scales grow tough and their claws sharp. Their breath grows more "
    "deadly and they frequently gain additional magical powers and resistances. "
    "All dragons can fly, but younger dragons are not so quick as their elders.\n \n"
    "Dragons are magical creatures, and each dragon may choose to specialize in a "
    "specific type of magic. This magic does not require books to learn and also "
    "has a direct influence on the dragon's stats and skills.";

static dragon_realm_ptr _get_realm(void);

/**********************************************************************
 * Dragon Equipment
 **********************************************************************/
static void _dragon_birth(void) 
{ 
    object_type    forge;

    equip_on_change_race();
    skills_innate_init("Claw", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_h = 3;
    forge.to_d = 3;
    forge.pval = 1;
    add_flag(forge.flags, OF_STR);
    add_flag(forge.flags, OF_DEX);
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/**********************************************************************
 * Dragon Breath
 **********************************************************************/

static void _do_breathe(int effect, int dir, int dam)
{
    /* Dragon breath changes shape with maturity */
    if (p_ptr->lev < 20)
        fire_bolt(effect, dir, dam);
    else if (p_ptr->lev < 30)
        fire_beam(effect, dir, dam);
    else
        fire_ball(effect, dir, dam, -1 - (p_ptr->lev / 20));
}

cptr gf_name(int which)
{
    gf_info_ptr gf;
    switch (which)
    {
    case GF_ELDRITCH_HOWL: return "<color:R>Fear</color>";
    case GF_ANIM_DEAD: return "<color:D>Reanimation</color>";
    case GF_OLD_DRAIN: return "<color:D>Vampirism</color>";
    case GF_GENOCIDE: return "<color:D>Death</color>";
    case GF_OLD_POLY: return "<color:v>Change</color>";
    }
    gf = gf_lookup(which);
    if (gf)
        return format("<color:%c>%s</color>", attr_to_attr_char(gf->color), gf->name);
    return "something";
}

static int _count(int list[])
{
    int i;
    for (i = 0; ; i++)
    {
        if (list[i] == -1) return i;
    }
    /* return 0;  error: missing sentinel ... unreachable */
}

static int _random(int list[])
{
    return list[randint0(_count(list))];
}

static void _effect_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    int  idx = ((int*)cookie)[which];

    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, format("%s", gf_name(idx)));
        break;
    }
}

static int _choose_effect(int list[])
{
    int i;
    int ct = _count(list);

    if (REPEAT_PULL(&i))
    {
        if (i >= 0 && i < ct)
            return list[i];
    }

    {
        menu_t menu = { "Choose which effect?", NULL, NULL,
                        _effect_menu_fn, list, ct, 0};
        
        i = menu_choose(&menu);
        if (i >= 0)
        {
            REPEAT_PUSH(i);
            i = list[i];
        }
    }
    return i;
}

static int _get_effect(int list[]) /* va_args is probably a better sig ... */
{
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH && p_ptr->lev >= 35)
        return _choose_effect(list);
    else
        return _random(list);
}

static int _breath_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return GF_FIRE;
    case DRAGON_WHITE: return GF_COLD;
    case DRAGON_BLUE: return GF_ELEC;
    case DRAGON_BLACK: return GF_ACID;
    case DRAGON_GREEN: return GF_POIS;
    case DRAGON_BRONZE: return GF_CONFUSION;
    case DRAGON_GOLD: return GF_SOUND;
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 45)
        {
            int effects[] = { GF_NETHER, GF_NEXUS, GF_DISENCHANT, -1 };
            return _get_effect(effects);
        }
        return GF_NETHER;
    case DRAGON_LAW: 
    {
        int effects[] = { GF_SOUND, GF_SHARDS, -1 };
        return _get_effect(effects);
    }
    case DRAGON_CHAOS: 
    {
        int effects[] = { GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    case DRAGON_ETHEREAL: 
        if (p_ptr->lev < 40)
        {
            int effects[] = { GF_LITE, GF_DARK, -1 };
            return _get_effect(effects);
        }
        else
        {
            int effects[] = { GF_LITE, GF_DARK, GF_CONFUSION, -1 };
            return _get_effect(effects);
        }
    case DRAGON_CRYSTAL: return GF_SHARDS;
    case DRAGON_BALANCE:
    {
        int effects[] = { GF_SOUND, GF_SHARDS, GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    }
    return 0;
}
static int _breath_amount(void)
{
    int l = p_ptr->lev;
    int amt = 0;
    dragon_realm_ptr realm = _get_realm();

    switch (p_ptr->psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        amt = MIN(600, p_ptr->chp * (25 + l*l*l/2500) / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        amt = MIN(450, p_ptr->chp * (20 + l*l*l*30/125000) / 100);
        break;

    case DRAGON_BALANCE:
    case DRAGON_NETHER:
    case DRAGON_ETHEREAL:
        amt = MIN(400, p_ptr->chp * (20 + l*l*l*25/125000) / 100);
        break;

    case DRAGON_STEEL:
        return 0;
    }
    amt = MAX(1, amt * realm->breath / 100);
    return amt;
}

static int _breath_cost(void)
{
    int l = p_ptr->lev;
    int cost = l/2 + l*l*15/2500;
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH)
    {
        if (p_ptr->lev >= 40)
            cost = cost * 3 / 4;
    }
    else
    {
    /*    cost += 5; */
    }
    return MAX(cost, 1);
}

static cptr _breath_desc(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return "fire";
    case DRAGON_WHITE: return "cold";
    case DRAGON_BLUE: return "lightning";
    case DRAGON_BLACK: return "acid";
    case DRAGON_GREEN: return "poison";
    case DRAGON_BRONZE: return "confusion";
    case DRAGON_GOLD: return "sound";
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 40) return "nether, nexus or disenchantment";
        return "nether";
    case DRAGON_LAW: return "sound or shards";
    case DRAGON_CHAOS: return "chaos or disenchantment";
    case DRAGON_ETHEREAL: return "light, dark or confusion";
    case DRAGON_CRYSTAL: return "shards";
    case DRAGON_BALANCE: return "sound, shards, chaos or disenchantment";
    }
    return 0;
}

static void _breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            _do_breathe(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Dragon Melee
 * cf design/dragons.ods
 **********************************************************************/
static int _attack_level(void)
{
    int l = p_ptr->lev * 2;
    switch (p_ptr->psubrace)
    {
    case DRAGON_STEEL:
        l = MAX(1, l * 115 / 100);
        break;

    case DRAGON_RED:
    case DRAGON_WHITE:
        l = MAX(1, l * 105 / 100);
        break;

    case DRAGON_BLACK:
    case DRAGON_GREEN:
        break;

    case DRAGON_BLUE:
        l = MAX(1, l * 95 / 100);
        break;

    case DRAGON_ETHEREAL:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        l = MAX(1, l * 90 / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_NETHER:
    case DRAGON_BALANCE:
        l = MAX(1, l * 85 / 100);
        break;
    }

    l = MAX(1, l * _get_realm()->attack / 100);
    return l;
}

static void _calc_innate_attacks(void)
{
    int l = _attack_level();
    int l2 = p_ptr->lev; /* Note: Using attack_level() for both dd and ds gives too much variation */
    int to_d = 0;
    int to_h = l2*3/5;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 15;
        a.ds = 3 + l2 / 16; /* d6 max for everybody */
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 100 + l;
        calc_innate_blows(&a, 400);
        a.msg = "You claw.";
        a.name = "Claw";

        /*if (p_ptr->dragon_realm == DRAGON_REALM_ATTACK && p_ptr->lev >= 40)
            a.flags |= INNATE_VORPAL;*/

        if (p_ptr->dragon_realm == DRAGON_REALM_DEATH && p_ptr->lev >= 45)
            a.effect[1] = GF_OLD_DRAIN;

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l2 / 10; /* 6d max for everybody */
        a.ds = 4 + l / 6;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 200 + 2 * l;

        if (p_ptr->lev >= 40)
            calc_innate_blows(&a, 200);
        else if (p_ptr->lev >= 35)
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite.";
        a.name = "Bite";

        /*if (p_ptr->dragon_realm == DRAGON_REALM_ATTACK && p_ptr->lev >= 40)
            a.flags |= INNATE_VORPAL;*/

        if (p_ptr->dragon_realm == DRAGON_REALM_DEATH && p_ptr->lev >= 45)
            a.effect[1] = GF_OLD_DRAIN;

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Dragon Realms
 **********************************************************************/
static dragon_realm_t _realms[DRAGON_REALM_MAX] = {
    { DRAGON_REALM_NONE, "None", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0,  0,  0,  0,  0,  0}, {   0,   0,   0,   0,   0,   0,  0,  0}, 100, 100,   100,   100, A_NONE},

    { DRAGON_REALM_LORE, "Lore", 
        "Dragons specializing in lore are seekers of knowledge. They are the most "
        "intelligent of dragonkind and use their vast intellects to drive their magic. "
        "Armed with a vast array of detection and knowledge spells, dragons of lore "
        "seek power through knowledge. They eventually gain powers of telepathy and "
        "automatic object identification. Lore dragons are quick learners and gain "
        "maturity much more rapidly than the rest of their kind.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, +3,  0, -1, -1,  0}, {   3,   8,   2,   0,   5,   5, -8,  0}, 100,  80,   100,   100, A_INT},

    { DRAGON_REALM_BREATH, "Breath", 
        "Dragon breath is the stuff of legends, and this realm seeks to enhance this most "
        "powerful attribute of dragonkind. With this speciality, you will be able to shape "
        "and control your breaths to maximize deadliness for a given situation. In addition, "
        "dragons of this realm may choose their breath types if applicable, and breathing "
        "becomes less costly as they mature. This focus requires great fortitude to master "
        "and somewhat diminishes the dragon's defenses and melee.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0, -1, -1,  0, +3, +1}, {   0,   0,   3,  -1,   0,   0,  0,  0}, 103, 105,    90,   115, A_CON},

    { DRAGON_REALM_ATTACK, "Attack", 
        "Attack dragons seek melee supremacy above all else. This realm offers powerful attack "
        "spells to support a race that is already among the melee elite, and the result can "
        "be truly awe-inspiring. With this realm, the dragon may rend their opponents with "
        "extra sharp claws, may snatch an adjacent opponent in their powerful jaws and then "
        "toss them about like a rag doll, and may even augment their bite attacks with their "
        "breath element! Truly, a rampaging dragon is an awe inspiring sight, one that is "
        "seldom witnessed (or perhaps seldom survived?). This focus values strength above all else.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+3, -2, -2, +1, -1,  0}, {  -5,  -5,  -3,  -1,  -2,  -2, 15,  0},  97, 105,   115,    80, A_STR},

    { DRAGON_REALM_CRAFT, "Craft", 
        "The most powerful magical items have long been believed forged by dragonflame. The "
        "craft dragon gains powers of enchantment and may even reforge artifacts into the objects "
        "of their choosing! Otherwise, craft dragons are not particularly powerful as they trade "
        "melee and breath prowess for magical understanding. This focus requires great wisdom to "
        "master.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, +3, -1, -1, -1}, {   3,  15,   3,   0,   0,   0, -5,  0}, 100,  95,   100,   100, A_WIS},

    { DRAGON_REALM_ARMOR, "Armor", 
        "Dragon scales have thwarted many a would be dragonslayer. Naturally tough and resistant, "
        "the dragon's armor is even further enhanced by this realm. This specialization gives enhanced "
        "armor class, reflection, resistance to cuts and stunning, resistance to poison "
        "and life draining, and sustaining of several key stats, albeit not all at once. With all "
        "of these extra innate bonuses, the magic spells of this realm are few in number but serve "
        "to offer temporary defensive augmentations. Unlike their kin, dragons of this order prize "
        "agility above all else.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, +3, +1, +1}, {  -2,  -3,   7,   1,   0,   0,-10,  0}, 102, 105,    90,    90, A_DEX},

    { DRAGON_REALM_DOMINATION, "Domination", 
        "All dragons have a formidable presence and make fearsome opponents. But Domination dragons "
        "are truly a breed apart seeking to bend and control the will of all they meet. Convinced "
        "of their right to rule, these dragons may subjugate the weak, terrify the uncertain, "
        "and stun the unwary with their awesome presence. They are the best dragons at controlling "
        "minions and can summon mighty aid. Enemy monsters summoned to battle may switch sides "
        "when they notice whom they have been commanded to fight. And in the end, the dragon of "
        "domination may sever all oaths of allegiance, returning enemy summons to where they came. "
        "Needless to say, dragons of this order value force of will above all else.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, -1, -1, +3}, {  -2,  -3,  -2,   0,   0,   0, -7,  0},  95, 105,    95,    90, A_CHR},

    { DRAGON_REALM_CRUSADE, "Crusade", 
        "Crusade dragons are on a mission to destroy the forces of evil. As such, this realm is only "
        "available to Gold Dragons and Law Dragons. Being of a single focus, the Crusade dragon is not "
        "as powerful in melee or breaths as other dragons, but their spells more than make up for this "
        "deficit. At least against evil opponents. For one thing, Crusade dragons can breathe holy "
        "elements not normally accessible. At the sight of evil, they become enraged and may haste "
        "for battle. They can even heal a bit and ultimately they may smite the forces of evil with "
        "holy power, both in melee and in breath. Strong in personality, dragons of this order "
        "may summon like minded kin for the final battles.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+1, -1, -1, +1, -1, +2}, {  -5,   0,  -2,   0,  -2,  -2,  7,  0},  95, 107,    90,    90, A_CHR},

    { DRAGON_REALM_DEATH, "Death", 
        "Death dragons are enemies of life itself, seeking to destroy all living creatures. With this "
        "realm, the dragon may bend their breath weapon to suit their necromantic desires, eventually "
        "breathing mastery over both death and life. At high levels, the death dragon's melee attacks "
        "gain a powerful draining effect against living creatures. This focus values strength above "
        "all else. This foul realm is only available to Shadow and Chaos dragons.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+2, -2, -2,  0, -2, +1}, {  -5,  -3,  -3,   2,  -2,  -2,  5,  0},  95, 105,    90,    90, A_STR},
};

dragon_realm_ptr dragon_get_realm(int which)
{
    assert(0 <= which && which < DRAGON_REALM_MAX);
    return &_realms[which];
}

dragon_realm_ptr _get_realm(void)
{
    return dragon_get_realm(p_ptr->dragon_realm);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "dragon spell";
        me.encumbrance.max_wgt = 750;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    me.which_stat = _get_realm()->spell_stat; /* Careful: Birthing may invoke this multiple times with different realms */
    return &me;
}

/* Lore */
static spell_info _lore_spells[] = {
    {  1,  1, 30, detect_traps_spell },
    {  3,  2, 30, detect_treasure_spell },
    {  5,  3, 40, detect_monsters_spell },
    {  7,  5, 50, detect_objects_spell },
    { 12, 10, 60, identify_spell },
    { 15, 12, 60, sense_surroundings_spell },
    { 20, 15, 60, detection_spell },
    { 22, 17, 60, probing_spell },
    { 25, 20, 65, self_knowledge_spell },
    { 30, 25, 70, identify_fully_spell },
    { 40, 50, 90, clairvoyance_spell },
    { -1, -1, -1, NULL}
};

/* Breath */
static void _bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a bolt of %s at your opponent. This is quicker, though less deadly, as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(1, _breath_cost()/2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_bolt(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 101 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Beam");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a beam of %s at your opponent. This is quicker as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_beam(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 110 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cone_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cone");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a cone of %s at your opponent. This is quicker as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_ball(e, dir, dam, -2);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 120 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _split_beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Split Beam");
        break;
    case SPELL_DESC:
        var_set_string(res, format("You breathe a beam of %s a two chosen targets, albeit with reduced damage.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_beam(e, dir, dam);
            
            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */
            if (get_fire_dir(&dir))
                fire_beam(e, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _retreating_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Retreating Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent and then take a movement.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_ball(e, dir, dam, -2);

            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */

            if (get_rep_dir2(&dir) && dir != 5)
            {
                int y, x;
                y = py + ddy[dir];
                x = px + ddx[dir];
                if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
                    move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
            }

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _deadly_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Deadly Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s powerfully at your opponent.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount() * 125 / 100));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost() * 125 / 100);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount() * 125 / 100;
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            fire_ball(e, dir, dam, -3);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _star_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Star Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Unleash your breath uncontrollably and at random, though with devastating effect.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int num = damroll(5, 3);
        int dam = MAX(1, _breath_amount()/3);
        int e = _breath_effect();
        int y = py, x = px, i;
        int attempts;

        var_set_bool(res, FALSE);
        if (e < 0) return;

        for (i = 0; i < num; i++)
        {
            attempts = 1000;
            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);
                if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                if (!player_bold(y, x)) break;
            }
            project(0, 3, y, x, dam, e,
                (PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _breath_spells[] = {
    {  1,  0, 30, _bolt_spell },
    { 10,  0, 30, _beam_spell },
    { 20,  0, 30, _cone_spell },
    { 25, 10, 50, _split_beam_spell },
    { 30, 15, 50, _retreating_breath_spell },
    { 40, 15, 60, _deadly_breath_spell },
    { 50, 50, 70, _star_ball_spell },
    { -1, -1, -1, NULL}
};

/* Craft */
static void _detect_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Magic");
        break;
    case SPELL_DESC:
        var_set_string(res, "Locate nearby magical objects.");
        break;
    case SPELL_CAST:
        detect_objects_magic(DETECT_RAD_DEFAULT);    
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static obj_ptr _get_reforge_src(int max_power)
{
    obj_prompt_t prompt = {0};
    char buf[255];

    sprintf(buf, "Use what artifact for reforging (Max Power = %d)? ", max_power);
    prompt.prompt = buf;
    prompt.error = "You have no artifacts to reforge.";
    prompt.filter = object_is_artifact;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static obj_ptr _get_reforge_dest(int max_power)
{
    obj_prompt_t prompt = {0};
    char buf[255];

    sprintf(buf, "Reforge which object (Max Power = %d)? ", max_power);
    prompt.prompt = buf;
    prompt.error = "You have nothing to reforge.";
    prompt.filter = item_tester_hook_nameless_weapon_armour;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static void _reforging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reforging");
        break;
    case SPELL_DESC:
        var_set_string(res, "Reforge a chosen artifact.");
        break;
    case SPELL_CAST:
    {
        int cost;
        char o_name[MAX_NLEN];
        obj_ptr src, dest;
        int power = p_ptr->lev * 5 / 2 + (p_ptr->lev >= 50 ? 5 : 0);
        int src_max_power = power * power * 10;
        int dest_max_power = 0;

        var_set_bool(res, FALSE);
        src = _get_reforge_src(src_max_power);
        if (!src) return;
        if (!object_is_artifact(src)) /* paranoia */
        {
            msg_print("You must choose an artifact for reforging.");
            return;
        }
        if (obj_value_real(src) > src_max_power)
        {
            msg_print("You are not powerful enough to reforge that item.");
            return;
        }

        cost = obj_value_real(src);
    
        dest_max_power = cost / 2;
        if (dest_max_power < 1000) /* Reforging won't try to power match weak stuff ... */
            dest_max_power = 1000;

        object_desc(o_name, src, OD_NAME_ONLY);    
        if (!get_check(format("Really use %s? (It will be destroyed!) ", o_name))) 
            return;

        dest = _get_reforge_dest(dest_max_power);
        if (!dest) return;

        if (dest->number > 1)
        {
            msg_print("Don't be greedy! You may only reforge a single object.");
            return;
        }

        if (object_is_artifact(dest))
        {
            msg_print("This item is already an artifact!");
            return;
        }

        if (object_is_ego(dest))
        {
            msg_print("This item is already an ego item!");
            return;
        }

        if (!equip_first_slot(dest))
        {
            msg_print("You may only create items you can actually use.");
            return;
        }

        if (obj_value_real(dest) > dest_max_power)
        {
            msg_print("This item is too powerful for the source artifact you have chosen.");
            return;
        }

        if (!reforge_artifact(src, dest, power))
        {
            msg_print("The reforging failed!");
            return;
        }

        src->number--;
        obj_release(src, 0);

        obj_identify_fully(dest);

        p_ptr->update |= PU_BONUS;
        p_ptr->window |= (PW_INVEN | PW_EQUIP);
        handle_stuff();

        obj_display(dest);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _craft_spells[] = {
    {  1,  1, 30, _detect_magic_spell },
    {  5,  7, 60, minor_enchantment_spell },
    { 12, 10, 60, remove_curse_I_spell },
    { 17, 15, 60, identify_spell },
    { 30, 25, 70, enchantment_spell },
    { 32, 30, 70, recharging_spell },
    { 35, 90, 70, _reforging_spell },
    { 40, 30, 70, dispel_magic_spell },
    { -1, -1, -1, NULL}
};

/* Attack */
static void _war_cry_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon's Roar");
        break;
    case SPELL_DESC:
        var_set_string(res, "You will roar out mightily, alerting all nearby monsters of your presence.");
        break;
    case SPELL_CAST:
        msg_print("You roar out!");
        project_hack(GF_SOUND, randint1(p_ptr->lev));
        aggravate_monsters(0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rend_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rend");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with cutting blows.");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].flags |= INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags |= INNATE_VORPAL;
        var_set_bool(res, do_blow(DRAGON_REND));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].flags |= INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags |= INNATE_VORPAL;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].flags &= ~INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags &= ~INNATE_VORPAL;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rage");
        break;
    default:
        berserk_spell(cmd, res);
        break;
    }
}

static void _three_way_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "3-Way Attack");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack in chosen direction, and to either side of chosen direction, in a single action.");
        break;
    case SPELL_CAST:
    {
        int cdir, dir;
        int y, x;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        for (cdir = 0;cdir < 8; cdir++)
        {
            if (cdd[cdir] == dir) break;
        }

        if (cdir == 8) return;

        y = py + ddy_cdd[cdir];
        x = px + ddx_cdd[cdir];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");
        y = py + ddy_cdd[(cdir + 7) % 8];
        x = px + ddx_cdd[(cdir + 7) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");
        y = py + ddy_cdd[(cdir + 1) % 8];
        x = px + ddx_cdd[(cdir + 1) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("You attack the empty air.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _deadly_bite_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Deadly Bite");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent as usual, but augment your bite attacks with your breath element.");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[1].effect[1] = _breath_effect();
        var_set_bool(res, do_blow(DRAGON_DEADLY_BITE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[1].effect[1] = _breath_effect();
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[1].effect[1] = 0;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _snatch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Snatch");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to snatch an adjacent opponent in your jaws. If successful, you may toss the monster away from you.");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].flags |= INNATE_SKIP;
        var_set_bool(res, do_blow(DRAGON_SNATCH));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _charge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Charge");
        break;
    case SPELL_DESC:
        var_set_string(res, "Charge a nearby monster and attack in a single action.");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rapid_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rapid Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with extra blows.");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].blows += 50;
        p_ptr->innate_attacks[1].blows += 25;
        var_set_bool(res, do_blow(DRAGON_RAPID_STRIKE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].blows += 50;
        p_ptr->innate_attacks[1].blows += 25;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].blows -= 50;
        p_ptr->innate_attacks[1].blows -= 25;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _power_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Power Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with more powerful blows.");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].dd += 2;
        p_ptr->innate_attacks[1].dd += 2;
        var_set_bool(res, do_blow(DRAGON_POWER_STRIKE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;

    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].dd += 2;
        p_ptr->innate_attacks[1].dd += 2;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].dd -= 2;
        p_ptr->innate_attacks[1].dd -= 2;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _attack_spells[] = {
    {  1,  1, 20, _war_cry_spell },
    {  5,  3, 40, detect_menace_spell },
    {  7,  7,  0, _rend_spell },
    {  9,  9, 50, _rage_spell },
    { 12, 10,  0, _three_way_attack_spell },
    { 20, 15,  0, _deadly_bite_spell },
    { 22, 15,  0, _snatch_spell },
    { 25, 20, 50, _charge_spell },
    { 30, 25, 50, _rapid_strike_spell },
    { 40, 30, 60, _power_strike_spell },
    { -1, -1, -1, NULL}
};

/* Armor */
static void _shard_skin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shard Skin");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily gain an aura of shards which damages any monsters that strike you.");
        break;
    case SPELL_CAST:
        set_tim_sh_shards(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _dragon_cloak_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dragon Cloak");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily gain protective elemental auras for a bit.");
        break;
    case SPELL_CAST:
        set_tim_sh_elements(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _magic_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Resistance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily gain enhanced resistance to magic.");
        break;
    case SPELL_CAST:
        set_resist_magic(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _armor_spells[] = {
    { 10, 10, 50, stone_skin_spell },
    { 15, 12, 50, _shard_skin_spell },
    { 20, 15, 60, _dragon_cloak_spell },
    { 25, 20, 60, resistance_spell },
    { 30, 30, 70, _magic_resistance_spell },
    { -1, -1, -1, NULL}
};

/* Crusade */
static void _breathe_spell_aux(int effect, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = _breath_amount();

            msg_format("You breathe %s.", gf_name(effect));
            _do_breathe(effect, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_retribution_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Retribution");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes lightning at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_ELEC, cmd, res);
        break;
    }
}

static void _breathe_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes light at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_LITE, cmd, res);
        break;
    }
}

static void _healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Healing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Powerful healing magic:  heals hitpoints, cuts and stun.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Heals %d", 200));
        break;
    case SPELL_CAST:
        hp_player(200);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_holiness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Holiness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes holy fire at chosen target. This hurts evil monsters greatly, but non-evil monsters resist.");
        break;
    default:
        _breathe_spell_aux(GF_HOLY_FIRE, cmd, res);
        break;
    }
}

static void _smite_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smite Evil");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent evil opponent with holy fury!");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].effect[1] = GF_HOLY_FIRE;
        p_ptr->innate_attacks[1].effect[1] = GF_HOLY_FIRE;
        var_set_bool(res, do_blow(DRAGON_SMITE_EVIL));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _crusade_spells[] = {
    {  1,  1, 30, bless_spell },
    {  5,  3, 30, remove_fear_spell },
    {  7,  4, 40, detect_evil_spell },
    { 10,  5, 50, _breathe_retribution_spell },
    { 15,  7, 50, heroism_spell },
    { 25, 15, 60, haste_self_spell },
    { 30, 20, 60, curing_spell },
    { 32, 10, 60, _breathe_light_spell },
    { 35, 30, 60, _healing_spell },
    { 37, 30, 70, _breathe_holiness_spell },
    { 40, 60,  0, _smite_evil_spell },
    { 45,100, 90, summon_hi_dragon_spell },
    { -1, -1, -1, NULL}
};

/* Death */
static void _breathe_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Poison");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes poison at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_POIS, cmd, res);
        break;
    }
}

static void _breathe_fear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Fear");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes fear at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_ELDRITCH_HOWL, cmd, res);
        break;
    }
}

static void _breathe_dark_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Darkness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes darkness at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_DARK, cmd, res);
        break;
    }
}

static void _breathe_nether_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Nether");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes nether at chosen target.");
        break;
    default:
        _breathe_spell_aux(GF_NETHER, cmd, res);
        break;
    }
}

static void _breathe_reanimation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Reanimation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes reanimation. Any corpses or skeletons hit by this breath may come back to life to serve you.");
        break;
    case SPELL_INFO:
        break;
    default:
        _breathe_spell_aux(GF_ANIM_DEAD, cmd, res);
        break;
    }
}

int dragon_vamp_amt = 0;
bool dragon_vamp_hack = FALSE;

static void _breathe_vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Vampirism");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes vampirism. Any living creatures hit by this breath have their life energies stolen to regain your hitpoints.");
        break;
    default:
        dragon_vamp_hack = TRUE;
        dragon_vamp_amt = 0;
        _breathe_spell_aux(GF_OLD_DRAIN, cmd, res);
        dragon_vamp_hack = FALSE;
        if (dragon_vamp_amt)
        {
            int amt = MIN(500, (dragon_vamp_amt + 1) / 2);
            hp_player(amt);
        }
        break;
    }
}

static void _breathe_unholiness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Unholiness");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes hell fire at chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount() * 3 / 2));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost() * 3 / 2);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = _breath_amount() * 3 / 2;

            msg_print("You breathe hell fire.");
            fire_ball(GF_HELL_FIRE, dir, dam, -1 - (p_ptr->lev / 20));

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_genocide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Genocide");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes genocide. Any monsters hit by this breath may be removed from the level, but you take damage for each monster so removed.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(_breath_amount()));
        break;
    default:
        _breathe_spell_aux(GF_GENOCIDE, cmd, res);
        break;
    }
}

static spell_info _death_spells[] = {
    {  1,  1, 30, evil_bless_spell },
    {  5,  2, 40, detect_evil_spell },
    { 10,  5, 50, _breathe_poison_spell },
    { 15,  5, 50, _breathe_fear_spell },
    { 20, 10, 60, _breathe_dark_spell },
    { 25, 20, 60, restore_life_spell },
    { 27, 10, 60, _breathe_nether_spell },
    { 30, 25, 65, battle_frenzy_spell },
    { 32, 20, 65, _breathe_reanimation_spell },
    { 35, 20, 65, _breathe_vampirism_spell },
    { 37, 20, 70, _breathe_unholiness_spell },
    { 40, 35, 75, _breathe_genocide_spell },
    { -1, -1, -1, NULL}
};

/* Domination */
static void _frightful_presence_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Frightful Presence");
        break;
    default:
        scare_spell(cmd, res);
        break;
    }
}

static void _detect_minions_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Minions");
        break;
    default:
        detect_monsters_spell(cmd, res);
        break;
    }
}

static void _baffling_presence_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Baffling Presence");
        break;
    default:
        confuse_spell(cmd, res);
        break;
    }
}

static void _enslave_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Enslave");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to force a single monster to serve you.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        charm_monster(dir, p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _plev(void)
{
    if (p_ptr->lev <= 40)
        return 5 + p_ptr->lev;

    return 45 + (p_ptr->lev - 40)*2;
}

int subjugation_power(void)
{
    return MAX(1, _plev() + adj_stat_save[p_ptr->stat_ind[A_CHR]]);
}

static void _breathe_subjugation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Subjugation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes dominance at chosen target. Affected monsters may be forced to serve you or may be stunned or terrified by your awesome presence.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(subjugation_power()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = subjugation_power();

            msg_print("You breathe subjugation.");
            _do_breathe(GF_SUBJUGATION, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _aura_of_domination_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Aura of Domination");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily gain an aura of domination which attempts to enslave, terrify or stun any monster that strikes you.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(10, 20));
        break;
    case SPELL_CAST:
        set_tim_sh_domination(randint1(20) + 10, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _banish_summons_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Banish Summons");
        break;
    case SPELL_DESC:
        var_set_string(res, "All enemy summons in view are returned to where they came.");
        break;
    case SPELL_CAST:
    {
        int i;
        msg_print("You shatter all oaths of allegiance!");
        for (i = 1; i < m_max; i++)
        {
            monster_type *m_ptr = &m_list[i];

            if (!m_ptr->r_idx) continue;
            if (!m_ptr->parent_m_idx) continue;
            if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;
            if (is_pet(m_ptr)) continue;

            delete_monster_idx(i);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}


static spell_info _domination_spells[] = {
    {  1,  1, 30, _frightful_presence_spell },
    {  5,  2, 35, _detect_minions_spell }, 
    { 10,  5, 40, _baffling_presence_spell },
    { 15, 10, 50, slow_spell },
    { 20, 15, 55, _enslave_spell },
    { 25, 25, 60, summon_kin_spell },
    { 30, 10, 60, _breathe_subjugation_spell },
    { 35, 30, 60, _aura_of_domination_spell },
    { 40, 70, 90, summon_hi_dragon_spell },
    { 45, 80, 85, _banish_summons_spell },
    { -1, -1, -1, NULL}
};

int _realm_get_spells(spell_info* spells, int max)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        return get_spells_aux(spells, max, _lore_spells);
    case DRAGON_REALM_BREATH:
        return get_spells_aux(spells, max, _breath_spells);
    case DRAGON_REALM_ATTACK:
        return get_spells_aux(spells, max, _attack_spells);
    case DRAGON_REALM_ARMOR:
        return get_spells_aux(spells, max, _armor_spells);
    case DRAGON_REALM_CRAFT:
        return get_spells_aux(spells, max, _craft_spells);
    case DRAGON_REALM_CRUSADE:
        return get_spells_aux(spells, max, _crusade_spells);
    case DRAGON_REALM_DEATH:
        return get_spells_aux(spells, max, _death_spells);
    case DRAGON_REALM_DOMINATION:
        return get_spells_aux(spells, max, _domination_spells);
    }
    return 0;
}

static void _realm_calc_bonuses(void)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            p_ptr->telepathy = TRUE;
        if (p_ptr->lev >= 40)
            p_ptr->auto_id = TRUE;
        else
            p_ptr->auto_pseudo_id = TRUE;
        break;
    case DRAGON_REALM_BREATH:
        p_ptr->to_a -= p_ptr->lev/2;
        p_ptr->dis_to_a -= p_ptr->lev/2;
        break;
    case DRAGON_REALM_ATTACK:
        res_add(RES_FEAR);
        break;
    case DRAGON_REALM_ARMOR:
        p_ptr->to_a += p_ptr->lev;
        p_ptr->dis_to_a += p_ptr->lev;
        if (p_ptr->lev >= 5)
            p_ptr->sustain_dex = TRUE;
        if (p_ptr->lev >= 10)
            p_ptr->sustain_str = TRUE;
        if (p_ptr->lev >= 15)
            p_ptr->sustain_con = TRUE;
        if (p_ptr->lev >= 20)
            p_ptr->sustain_chr = TRUE;
        if (p_ptr->lev >= 25)
            p_ptr->hold_life++;
        if (p_ptr->lev >= 30)
            p_ptr->no_cut = TRUE;
        if (p_ptr->lev >= 35)
            res_add(RES_POIS);
        if (p_ptr->lev >= 40)
        {
            p_ptr->reflect = TRUE;
            p_ptr->no_stun = TRUE;
        }         /* v---- This is a timer but is not following the naming convention! */
        if (p_ptr->resist_magic && p_ptr->lev >= 30) 
            p_ptr->magic_resistance = 5 + (p_ptr->lev - 30) / 2;
        break;
    case DRAGON_REALM_CRUSADE:
        p_ptr->align += 200;
        if (p_ptr->lev >= 15)
            p_ptr->hold_life++;
        if (p_ptr->lev >= 30)
            res_add(RES_FEAR);
        break;
    case DRAGON_REALM_DEATH:
        p_ptr->align -= 200;
        break;
    case DRAGON_REALM_DOMINATION:
        res_add(RES_FEAR);
        if (p_ptr->lev >= 30)
            p_ptr->cult_of_personality = TRUE;
        if (p_ptr->lev >= 50)
            res_add_immune(RES_FEAR);
        break;
    }
}

static void _realm_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_TELEPATHY);
        break;
    case DRAGON_REALM_ATTACK:
        add_flag(flgs, OF_RES_FEAR);
        break;
    case DRAGON_REALM_ARMOR:
        if (p_ptr->lev >= 5)
            add_flag(flgs, OF_SUST_DEX);
        if (p_ptr->lev >= 10)
            add_flag(flgs, OF_SUST_STR);
        if (p_ptr->lev >= 15)
            add_flag(flgs, OF_SUST_CON);
        if (p_ptr->lev >= 20)
            add_flag(flgs, OF_SUST_CHR);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_HOLD_LIFE);
        /*if (p_ptr->lev >= 30)
            add_flag(flgs, TR_NO_CUT);*/
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_RES_POIS);
        if (p_ptr->lev >= 40)
        {
            add_flag(flgs, OF_REFLECT);
            /*add_flag(flgs, TR_NO_STUN);*/
        }
        if (p_ptr->resist_magic && p_ptr->lev >= 30) 
            add_flag(flgs, OF_MAGIC_RESISTANCE); /* s/b a temp flag ... */
        break;
    case DRAGON_REALM_CRUSADE:
        if (p_ptr->lev >= 15)
            add_flag(flgs, OF_HOLD_LIFE);
        if (p_ptr->lev >= 30)
            add_flag(flgs, OF_RES_FEAR);
        break;
    case DRAGON_REALM_DOMINATION:
        add_flag(flgs, OF_RES_FEAR);
        if (p_ptr->lev >= 50)
            add_flag(flgs, OF_IM_FEAR);
        break;
    }
}

/**********************************************************************
 * Dragon Bonuses (Common to all Types)
 **********************************************************************/
static void _dragon_calc_bonuses(void) 
{
    p_ptr->skill_dig += 100;
    p_ptr->levitation = TRUE;
    if (p_ptr->lev >= 20)
    {
        p_ptr->free_act++;
        p_ptr->see_inv++;
    }
    if (p_ptr->lev >= 30)
    {
        res_add(RES_CONF);
        /*Attack, Crusade, and Domination Realms: res_add(RES_FEAR);*/
    }
    _realm_calc_bonuses();
}

static void _dragon_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
    }
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_CONF);
        /*Attack, Crusade, and Domination Realms: add_flag(flgs, TR_RES_FEAR);*/
    }
    _realm_get_flags(flgs);
}

/**********************************************************************
 * Dragon Powers (Common to all Types)
 **********************************************************************/
void dragon_reach_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reach");
        break;
    case SPELL_DESC:
        var_set_string(res, "Reach out and bite a distant monster.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(2 + p_ptr->lev/40));
        break;
    case SPELL_CAST:
        {
            int dir = 5;
            var_set_bool(res, FALSE);
            project_length = 2 + p_ptr->lev/40;
            if (!get_fire_dir(&dir)) return;
            p_ptr->innate_attacks[0].flags |= INNATE_SKIP;
            project_hook(GF_ATTACK, dir, 0, PROJECT_STOP | PROJECT_KILL);
            p_ptr->innate_attacks[0].flags &= ~INNATE_SKIP;
            var_set_bool(res, TRUE);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dragon_tail_sweep_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Tail Sweep");
        break;
    case SPELL_DESC:
        var_set_string(res, "Sweep your tail in a semicircle clockwise from chosen direction. Monsters, if hit, may be flung back a square or two.");
        break;
    case SPELL_CAST:
        /* Hack: Replace normal tooth and claw attacks with a tail attack */
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attack_ct = 0;
        {
            int             l = _attack_level();
            innate_attack_t a = {0};

            a.dd = 1 + l / 30;
            a.ds = 3 + l / 10;
            a.to_h += l / 2;

            a.weight = 100 + l;
            a.blows = 100;
            a.msg = "You hit.";
            a.name = "Tail";

            p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        }     
        var_set_bool(res, do_blow(DRAGON_TAIL_SWEEP));
        p_ptr->innate_attack_lock = FALSE;
        /* Hack: Restore normal attacks */
        p_ptr->innate_attack_ct = 0;
        _calc_innate_attacks();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dragon_wing_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wing Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "This talent uses your wings to create massive wind gusts. Nearby foes are damaged, stunned, and blown about.");
        break;
    case SPELL_CAST:
        msg_print("You bring your wings down powerfully!");
        project(0, 5, py, px, randint1(p_ptr->lev * 3), GF_STORM, PROJECT_KILL | PROJECT_ITEM);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _dragon_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_DEX, { 20,  7,  0, dragon_reach_spell}},
    { A_DEX, { 25, 10,  0, dragon_tail_sweep_spell}},
    { A_DEX, { 30, 20,  0, dragon_wing_storm_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _steel_powers[] = {
    { A_DEX, { 20,  7,  0, dragon_reach_spell}},
    { A_DEX, { 25, 10,  0, dragon_tail_sweep_spell}},
    { A_DEX, { 30, 20,  0, dragon_wing_storm_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _dragon_get_powers(spell_info* spells, int max) {
    if (p_ptr->psubrace == DRAGON_STEEL)
        return get_powers_aux(spells, max, _steel_powers);
    else
        return get_powers_aux(spells, max, _dragon_powers);
}

/**********************************************************************
 * Elemental Dragon (Red, White, Blue, Black, Green)
 *   Baby -> Young -> Mature -> Ancient -> Great Foo Wyrm
 **********************************************************************/
 typedef struct {
    int  r_idx[5];
    cptr r_name[5];
    int  which_res;
    cptr name; /* For Birth and Helpfiles ... */
    cptr desc;
} _elemental_info_t;

static _elemental_info_t _elemental_info[5] = { /* relies on #define DRAGON_RED 0 ... */
    { {167, 563, 589, 644, 756},
      {"Baby Red Dragon", "Young Red Dragon", "Mature Red Dragon", "Ancient Red Dragon", "Great Hell Wyrm"},
      RES_FIRE, "Red Dragon",
        "Red Dragons are elemental dragons of fire and are the second strongest fighters among "
        "dragons. Their fiery breaths are the stuff of legends with damage unsurpassed. Even their "
        "bites are likely to burn their opponents rendering their melee damage quite impressive. "
        "As the Red Dragon matures, it becomes more and more resistant to fire, eventually gaining "
        "total immunity." },
    { {164, 460, 549, 617, 741},
      {"Baby White Dragon", "Young White Dragon", "Mature White Dragon", "Ancient White Dragon", "Great Ice Wyrm"},
      RES_COLD, "White Dragon",
        "White Dragons are to cold what Red Dragons are to fire. Their melee is truly awe-inspiring "
        "and their icy breath can be felt even in their bite. Like Red Dragons, White Dragons "
        "have the most deadly breath possible among dragonkind and they too become more and more "
        "resistant to cold as they mature." },
    { {163, 459, 560, 601, 728},
      {"Baby Blue Dragon", "Young Blue Dragon", "Mature Blue Dragon", "Ancient Blue Dragon", "Great Storm Wyrm"},
      RES_ELEC, "Blue Dragon",
        "Blue Dragons are elemental dragons of lightning. Their melee and breaths are not so "
        "strong as their Red and White brethren, but lightning is a bit more useful than fire or "
        "cold. Their bites eventually shock their foes. Blue Dragons become more and more resistant "
        "to lightning as they mature." },
    { {166, 546, 592, 624, 1066},
      {"Baby Black Dragon", "Young Black Dragon", "Mature Black Dragon", "Ancient Black Dragon", "Great Bile Wyrm"},
      RES_ACID, "Black Dragon",
        "Black Dragons are to acid what Blue Dragons are to lightning. Like the Blue Dragon, their "
        "breaths and melee fall short of their Red and White brethren. As they mature, their bites "
        "corrode their enemies and the Black Dragon also becomes more and more resistant to acid." },
    { {165, 461, 561, 618, 890},
      {"Baby Green Dragon", "Young Green Dragon", "Mature Green Dragon", "Ancient Green Dragon", "Great Venom Wyrm"},
      RES_POIS, "Green Dragon",
        "Green Dragons are elemental dragons of poison. They are not so strong as Red or White dragons, "
        "but are still fearsome opponents. As they mature, their bites poison their enemies. Also, "
        "Green Dragons become more and more resistant to poison." },
};

static void _elemental_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*5;
    int res = _elemental_info[p_ptr->psubrace].which_res;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(res);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(res);
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add_immune(res);
        res_add(RES_BLIND);
        switch (res)
        {
        case RES_FIRE: p_ptr->sh_fire++; break;
        case RES_COLD: p_ptr->sh_cold++; break;
        case RES_ELEC: p_ptr->sh_elec++; break;
        }
    }
    _dragon_calc_bonuses();
}
static void _elemental_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    int res = _elemental_info[p_ptr->psubrace].which_res;
    add_flag(flgs, res_get_object_flag(res));
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_RES_BLIND);
        switch (res)
        {
        case RES_FIRE: add_flag(flgs, OF_AURA_FIRE); break;
        case RES_COLD: add_flag(flgs, OF_AURA_COLD); break;
        case RES_ELEC: add_flag(flgs, OF_AURA_ELEC); break;
        }
        add_flag(flgs, res_get_object_immune_flag(res));
    }
    _dragon_get_flags(flgs);
}
static void _elemental_birth(void) { 
    p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[0]; 
    _dragon_birth();
}
static void _elemental_gain_level(int new_level) {
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[0] && new_level >= 10)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[1];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[1]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[1] && new_level >= 20)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[2];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[2]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[2] && new_level >= 30)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[3];
        msg_format("You have evolved into an %s.", _elemental_info[p_ptr->psubrace].r_name[3]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[3] && new_level >= 40)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[4];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[4]);
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_elemental_get_race_t(int subrace)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  70,  30};
    skills_t xs = {  8,   9,  10,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _elemental_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _elemental_calc_bonuses;
        me.get_flags = _elemental_get_flags;
        me.gain_level = _elemental_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = _elemental_info[subrace].name;
    else
        me.subname = _elemental_info[subrace].r_name[rank];
    me.subdesc = _elemental_info[subrace].desc;
    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -1 + rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] =  0 + rank;
    me.stats[A_CHR] = -1 + rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Nether: Shadow Drake -> Death Drake -> Spectral Wyrm
 **********************************************************************/
static void _nether_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_NETHER);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(RES_COLD);
        res_add(RES_TELEPORT);
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 45)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 2;
        p_ptr->sh_cold++;
        res_add(RES_POIS);
        res_add_immune(RES_NETHER);
        res_add(RES_NEXUS);
        res_add(RES_DISEN);
        res_add(RES_TELEPORT);
    }
    _dragon_calc_bonuses();
}
static void _nether_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_NETHER);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_COLD);
    }
    if (p_ptr->lev >= 45)
    {
        add_flag(flgs, OF_AURA_COLD);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_DISEN);
        add_flag(flgs, OF_IM_NETHER);
    }
    _dragon_get_flags(flgs);
}
static void _nether_birth(void) { 
    p_ptr->current_r_idx = MON_SHADOW_DRAKE; 
    _dragon_birth();
}

static void _nether_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_SHADOW_DRAKE && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_DEATH_DRAKE;
        msg_print("You have evolved into a Death Drake.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEATH_DRAKE && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_SPECTRAL_WYRM;
        msg_print("You have evolved into a Spectral Wyrm.");
        p_ptr->redraw |= PR_MAP;
    }
}

static race_t *_nether_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Shadow Drake", "Death Drake", "Spectral Wyrm"};    
    int           rank = 0;

    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   4,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Shadow Drakes are bit more stealthy than your average dragon. They are creatures of nether "
            "and eventually evolve into Death Drakes. Their melee is the weakest among dragonkind and "
            "their breaths also are lacking, but they still make fearsome opponents. As they advance, "
            "these dragons eventually gain the ability to pass through walls and also become more and "
            "more resistant to nether.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _nether_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _nether_calc_bonuses;
        me.get_flags = _nether_get_flags;
        me.gain_level = _nether_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Shadow Drake";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] = -1 + rank;
    me.stats[A_CHR] = -1 + 3*rank;
    me.life = 90 + 5*rank;

    return &me;
}

/**********************************************************************
 * Law: Law Drake -> Great Wyrm of Law
 **********************************************************************/
static void _law_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);

    if (p_ptr->lev >= 40)
    {
        p_ptr->align += 200;
        p_ptr->pspeed += 5;
        res_add(RES_SOUND);
        res_add(RES_SHARDS);
    }

    _dragon_calc_bonuses();
}
static void _law_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_SHARDS);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _law_birth(void) { 
    p_ptr->current_r_idx = MON_LAW_DRAKE; 
    _dragon_birth();
}
static void _law_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_LAW_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_LAW;
        msg_print("You have evolved into a Great Wyrm of Law.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_law_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Law Drake", "Great Wyrm of Law"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Law Drakes are powerful dragons of order. They can breathe sound or shards and eventually "
                    "evolve into Great Wyrms of Law, though not so quickly as you might hope. Their breaths "
                    "are much weaker than those of the elemental dragons but very few monsters resist sound "
                    "or shards. Their melee is among the weakest of all dragonkind but they still fight rather "
                    "well ... What dragon doesn't?";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 280;

        me.birth = _law_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _law_calc_bonuses;
        me.get_flags = _law_get_flags;
        me.gain_level = _law_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Law Drake";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Chaos: Chaos Drake -> Great Wyrm of Chaos
 **********************************************************************/
static void _chaos_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 5;
        res_add(RES_CHAOS);
        res_add(RES_DISEN);
    }

    _dragon_calc_bonuses();
}
static void _chaos_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_CHAOS);
    add_flag(flgs, OF_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _chaos_birth(void) { 
    p_ptr->current_r_idx = MON_CHAOS_DRAKE; 
    _dragon_birth();
}
static void _chaos_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CHAOS_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_CHAOS;
        msg_print("You have evolved into a Great Wyrm of Chaos.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_chaos_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Chaos Drake", "Great Wyrm of Chaos"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Chaos Drakes are powerful dragons of chaos. They can breathe chaos or disenchantment and eventually "
        "evolve into Great Wyrms of Chaos, though not so quickly as you might hope. Their breaths "
        "are much weaker than those of the elemental dragons but fewer monsters resist chaos "
        "or disenchantment. Their melee is among the weakest of all dragonkind but they still fight rather "
        "well ... What dragon doesn't?";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 280;

        me.birth = _chaos_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _chaos_calc_bonuses;
        me.get_flags = _chaos_get_flags;
        me.gain_level = _chaos_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Chaos Drake";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Balance: Balance Drake -> Great Wyrm of Balance
 **********************************************************************/
static void _balance_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 10 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);
    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
    }
    _dragon_calc_bonuses();
}
static void _balance_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_RES_CHAOS);
    add_flag(flgs, OF_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _balance_birth(void) { 
    p_ptr->current_r_idx = MON_BALANCE_DRAKE; 
    _dragon_birth();
}
static void _balance_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_BALANCE_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_BALANCE;
        msg_print("You have evolved into a Great Wyrm of Balance.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_balance_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Balance Drake", "Great Wyrm of Balance"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  35,   2,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  10,   0,   0,   0,  15,   7};

        me.subdesc = "Balance Drakes are a blend of Chaos and Law Drakes. They can breathe sound, shards, "
        "chaos or disenchantment and eventually evolve into Great Wyrms of Balance, though not "
        "so quickly as you might hope. Their breaths are much weaker than those of the elemental "
        "dragons and they are weaker than either of Chaos or Law Drakes, though not by much.";


        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _balance_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _balance_calc_bonuses;
        me.get_flags = _balance_get_flags;
        me.gain_level = _balance_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Balance Drake";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 4*rank;
    me.stats[A_INT] = -1 + 4*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 3*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 95 + 10*rank;

    return &me;
}

/**********************************************************************
 * Ethereal: Pseudo Dragon -> Ethereal Drake -> Ethereal Dragon
 **********************************************************************/
static void _ethereal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_LITE);
    res_add(RES_DARK);
    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
        res_add(RES_LITE);
        res_add(RES_DARK);
        res_add(RES_CONF);
    }
    _dragon_calc_bonuses();
}
static void _ethereal_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_RES_DARK);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _ethereal_birth(void) { 
    p_ptr->current_r_idx = MON_PSEUDO_DRAGON; 
    _dragon_birth();
}
static void _ethereal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_PSEUDO_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAKE;
        msg_print("You have evolved into an Ethereal Drake.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ETHEREAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAGON;
        msg_print("You have evolved into an Ethereal Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_ethereal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Pseudo Dragon", "Ethereal Drake", "Ethereal Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  37,   4,  25,  26,  52,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc =
        "Ethereal Drakes are dragons of light and darkness. They actually begin life as Pseudo "
        "Dragons but quickly evolve into Ethereal Drakes and then Ethereal Dragons. As they "
        "mature, they gain the ability to pass through walls and become more and more resistant "
        "to light, darkness and confusion. They are fairly weak fighters and have the weakest "
        "breaths in all of dragonkind (except for Steel Dragons which cannot breathe at all).";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _ethereal_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _ethereal_calc_bonuses;
        me.get_flags = _ethereal_get_flags;
        me.gain_level = _ethereal_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Ethereal Drake";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 95 + 7*rank;

    return &me;
}

/**********************************************************************
 * Crystal: Crystal Drake -> Great Crystal Drake
 **********************************************************************/
static void _crystal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level_aux(125, 1, 2, 2);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_COLD);
    res_add(RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        p_ptr->pspeed++;
    }    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add(RES_SHARDS);
        p_ptr->reflect = TRUE;
    }
    _dragon_calc_bonuses();
}
static void _crystal_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, OF_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_REFLECT);
    }
    _dragon_get_flags(flgs);
}
static void _crystal_birth(void) { 
    p_ptr->current_r_idx = MON_CRYSTAL_DRAKE; 
    _dragon_birth();
}
static void _crystal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CRYSTAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_CRYSTAL_DRAKE;
        msg_print("You have evolved into a Great Crystal Drake.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_crystal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Crystal Drake", "Great Crystal Drake"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  40,   1,  25,  26,  70,  30};
    skills_t xs = {  8,   7,  12,   0,   0,   0,  22,   7};

        me.subdesc =
        "Crystal Drakes are dragons of a strange crystalline form. They breathe shards and melee "
        "powerfully with razor sharp claws and teeth. At high levels, they gain the power of "
        "reflection.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 275;

        me.birth = _crystal_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _crystal_calc_bonuses;
        me.get_flags = _crystal_get_flags;
        me.gain_level = _crystal_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Crystal Drake";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  1 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] =  0 + 3*rank;
    me.stats[A_CON] =  0 + 4*rank;
    me.stats[A_CHR] =  0 + 3*rank;
    me.life = 100 + 15*rank;

    return &me;
}

/**********************************************************************
 * Bronze: Young -> Mature -> Ancient
 **********************************************************************/
static void _bronze_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CONF);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _bronze_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_CONF);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _bronze_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_BRONZE_DRAGON; 
    _dragon_birth();
}
static void _bronze_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_BRONZE_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_BRONZE_DRAGON;
        msg_print("You have evolved into a Mature Bronze Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_BRONZE_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_BRONZE_DRAGON;
        msg_print("You have evolved into an Ancient Bronze Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_bronze_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Young Bronze Dragon", "Mature Bronze Dragon", "Ancient Bronze Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   3,  25,  26,  55,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc =
        "Bronze Dragons are wyrms of confusion. While they are not quite as strong as most other "
        "dragons, they eventually confuse monsters with their bite attack. Also, they become "
        "more and more resistant to confusion as they mature.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _bronze_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _bronze_calc_bonuses;
        me.get_flags = _bronze_get_flags;
        me.gain_level = _bronze_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Bronze Dragon";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Gold: Young -> Mature -> Ancient
 **********************************************************************/
static void _gold_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SOUND);
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _gold_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _gold_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_GOLD_DRAGON; 
    _dragon_birth();
}
static void _gold_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_GOLD_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_GOLD_DRAGON;
        msg_print("You have evolved into a Mature Gold Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_GOLD_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_GOLD_DRAGON;
        msg_print("You have evolved into an Ancient Gold Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_gold_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Young Gold Dragon", "Mature Gold Dragon", "Ancient Gold Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  55,  30};
    skills_t xs = {  8,   9,  11,   0,   0,   0,  20,   7};

        me.subdesc =
        "Gold Dragons are wyrms of sound. While they are not quite as strong as most other "
        "dragons, they are able to breathe sound on command, stunning their foes. Also, they become "
        "more and more resistant to sound as they mature.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _gold_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _gold_calc_bonuses;
        me.get_flags = _gold_get_flags;
        me.gain_level = _gold_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Gold Dragon";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Steel: Stone Dragon -> Steel Dragon
 **********************************************************************/
static void _steel_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(144);
    int ac = 15 + (l/10)*2;

    p_ptr->skill_dig += 100;
    
    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    if (p_ptr->lev < 40)
        res_add_vuln(RES_COLD);

    res_add(RES_FIRE);
    res_add(RES_ELEC);
    res_add(RES_POIS);
    p_ptr->no_cut = TRUE;
    p_ptr->pspeed -= 1;
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->no_stun = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SHARDS);
    }
    _dragon_calc_bonuses();
}
static void _steel_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_RES_SHARDS);
    }
    if (p_ptr->lev < 40)
        add_flag(flgs, OF_VULN_COLD);

    _dragon_get_flags(flgs);
}
static void _steel_birth(void) { 
    p_ptr->current_r_idx = MON_STONE_DRAGON; 
    _dragon_birth();
}
static void _steel_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_STONE_DRAGON && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_STEEL_DRAGON;
        msg_print("You have evolved into a Steel Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_steel_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Stone Dragon", "Steel Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  18,  40,   0,  10,   7,  72,  30};
    skills_t xs = {  8,   7,  15,   0,   0,   0,  27,   7};

        me.subdesc =
        "Steel Dragons are magical dragons formed from rock. As they mature, their form hardens "
        "from stone into steel. Needless to say, their armor class is phenomenal, but their "
        "dexterity actually decreases with maturity. Steel dragons begin life being susceptible "
        "to cold damage, though they will eventually outgrow this vulnerability. They are not "
        "as fast as other dragons and they have no powers whatsoever, not even the ubiquitous "
        "dragon breath! But their fighting is impossibly strong, putting all the other dragons "
        "to complete and utter shame. They also have the most hitpoints of all dragons.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _steel_birth;
        me.calc_bonuses = _steel_calc_bonuses;
        me.get_flags = _steel_get_flags;
        me.get_powers = _dragon_get_powers;
        me.gain_level = _steel_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Steel Dragon";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  5 + (p_ptr->lev / 10);
    me.stats[A_INT] = -6;
    me.stats[A_WIS] = -6;
    me.stats[A_DEX] =  0 - (p_ptr->lev / 10);
    me.stats[A_CON] =  4 + (p_ptr->lev / 10);
    me.stats[A_CHR] =  0 + (p_ptr->lev / 10);
    me.life = 120 + (p_ptr->lev / 2);

    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_dragon_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        result = _elemental_get_race_t(psubrace);
        break;
    case DRAGON_NETHER:
        result = _nether_get_race_t();
        break;
    case DRAGON_LAW:
        result = _law_get_race_t();
        break;
    case DRAGON_CHAOS:
        result = _chaos_get_race_t();
        break;
    case DRAGON_BALANCE:
        result = _balance_get_race_t();
        break;
    case DRAGON_ETHEREAL:
        result = _ethereal_get_race_t();
        break;
    case DRAGON_CRYSTAL:
        result = _crystal_get_race_t();
        break;
    case DRAGON_BRONZE:
        result = _bronze_get_race_t();
        break;
    case DRAGON_GOLD:
        result = _gold_get_race_t();
        break;
    case DRAGON_STEEL:
        result = _steel_get_race_t();
        break;
    default: /* Birth Menus */
        result = _nether_get_race_t();
    }

    if (p_ptr->dragon_realm && !birth_hack && !spoiler_hack)
    {
        dragon_realm_ptr realm = _get_realm();
        int              i;

        for (i = 0; i < MAX_STATS; i++)
            result->stats[i] += realm->stats[i];

        result->caster_info = _caster_info;
        result->get_spells = _realm_get_spells;
    }
    else
    {
        result->caster_info = NULL;
        result->get_spells = NULL;
    }

    result->name = "Dragon";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->calc_innate_attacks = _calc_innate_attacks;
    result->equip_template = mon_get_equip_template();
    result->base_hp = 40;
    result->pseudo_class_idx = CLASS_BEASTMASTER;
    result->shop_adjust = 130;

    result->boss_r_idx = MON_GLAURUNG;
    return result;
}


