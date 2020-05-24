#include "angband.h"

static cptr _desc = 
    "Liches are the undead forms of former sorcerers. Their magic is strong, "
    "but their fighting skills weak. As undead, they quickly gain resistance to "
    "cold, poison and nether, and these resistances increase as the lich "
    "evolves.\n \n"
    "Liches are born with magical powers, and gain additional powers as they advance; of all the "
    "monster races, none can surpass the firepower of an Archlich, but managing "
    "to evolve that far can be a challenge. Intelligence is the primary spell stat.\n \n"
    "Liches are humanoid, and so use the standard set of equipment items; but should they "
    "forgo the use of a normal weapon, they may touch their opponents for "
    "various powerful effects. Even with this deadly touch, though, melee will "
    "never be their strong suit.";

static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_LICH;
    skills_innate_init("Finger", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_CROWN, SV_IRON_CROWN));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_STAFF, SV_ANY));
    if (device_init_fixed(&forge, EFFECT_ANIMATE_DEAD))
        py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_ROBE));
    py_birth_obj(&forge);

    py_birth_light();
}

/**********************************************************************
 * Innate Attacks
 **********************************************************************/
static void _calc_innate_attacks(void)
{
    if (p_ptr->weapon_ct == 0 && equip_find_empty_hand())
    {
        innate_attack_t    a = {0};
        int l = p_ptr->lev;
        int i = 0;

        a.dd = 1 + l / 12;
        a.ds = 6 + l / 15;
        a.weight = 2;
        a.to_h = p_ptr->lev/5;

        a.effect[i++] = GF_NETHER;
        if (p_ptr->lev >= 40)
            a.effect[i++] = GF_DISENCHANT;
        if (p_ptr->lev >= 25)
            a.effect[i++] = GF_OLD_DRAIN;
        
        calc_innate_blows(&a, 400);
        a.msg = "You touch.";
        a.name = "Finger";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Spells
 **********************************************************************/
static void _mana_bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Bolt");
        break;
    default:
        mana_bolt_II_spell(cmd, res);
        break;
    }
}

static void _mana_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mana Storm");
        break;
    default:
        mana_storm_II_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Archlich (Lich -> Master Lich -> Demilich -> Archlich)
 **********************************************************************/
static spell_info _get_spells[] = {
    {  1,  1, 20, magic_missile_spell},
    {  3,  2, 25, phase_door_spell},
    {  5,  3, 30, scare_spell},
    {  7,  4, 30, detect_life_spell},
    {  9,  4, 40, nether_bolt_spell},
    { 10,  5, 50, teleport_spell},
    { 11,  7, 40, slow_spell}, 
    { 14,  8, 50, paralyze_spell},
    { 17, 10, 50, drain_mana_spell},
    { 20, 12, 50, teleport_other_spell},
    { 23, 15, 50, brain_smash_spell},
    { 25, 15, 40, nether_ball_spell},
    { 27, 15, 50, confuse_spell}, 
    { 29, 20, 55, ice_bolt_spell},
    { 30, 20, 60, dispel_life_spell}, 
    { 32, 40, 60, summon_undead_spell}, 
    { 34, 15, 50, animate_dead_spell}, 
    { 36, 35, 80, _mana_bolt_spell},
    { 40, 70, 80, summon_hi_undead_spell}, 
    { 50, 50, 68, _mana_storm_spell},
    { -1, -1, -1, NULL}
};

static power_info _get_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void) {
    if (p_ptr->lev < 25)
        res_add_vuln(RES_LITE);

    p_ptr->align -= 200;
    p_ptr->see_inv++;
    p_ptr->slow_digest = TRUE;
    p_ptr->hold_life++;
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add(RES_NETHER);
    if (p_ptr->lev >= 25)
    {
        p_ptr->pspeed += 1;
        res_add(RES_CONF);
        res_add(RES_TELEPORT);
        p_ptr->free_act++;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add(RES_COLD);
        res_add(RES_POIS);
        res_add(RES_NETHER);
        p_ptr->telepathy = TRUE;
    }
    if (p_ptr->lev >= 50)
    {
        p_ptr->pspeed += 2;
        p_ptr->levitation = TRUE;
        res_add_immune(RES_NETHER);
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_NETHER);

    if (p_ptr->lev < 25)
        add_flag(flgs, OF_VULN_LITE);

    if (p_ptr->lev >= 25)
    {
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_TELEPATHY);
    }
    if (p_ptr->lev >= 50)
    {
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_IM_NETHER);
    }
}
static void _gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_LICH && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_MASTER_LICH;
        msg_print("You have evolved into a Master Lich.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MASTER_LICH && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_DEMILICH;
        msg_print("You have evolved into a Demilich.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEMILICH && new_level >= 50)
    {
        p_ptr->current_r_idx = MON_ARCHLICH;
        msg_print("You have evolved into an Archlich.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_archlich_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Lich", "Master Lich", "Demilich", "Archlich"};    
    int           rank = 0;

    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 40) rank++;
    if (p_ptr->lev >= 50) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  45,  38,   7,  20,  30,  34,  20 };
    skills_t xs = {  7,  15,  12,   0,   0,   0,   6,   7 };

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 275;
        me.base_hp = 20;

        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 - (p_ptr->lev / 10);
    me.stats[A_INT] =  3 + rank;
    me.stats[A_WIS] = -3 - rank;
    me.stats[A_DEX] =  1 + rank;
    me.stats[A_CON] =  0 - (rank+1)/2;
    me.stats[A_CHR] =  0 + rank;
    me.life = 100 - 2*rank;

    return &me;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "unholy power";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_lich_get_race(void)
{
    race_t *result = NULL;

    switch (p_ptr->psubrace)
    {
    /* TODO: Add subraces for Reaver (Lich -> Master Lich -> Lesser Black Reaver -> Black Reaver)
             in which case Archlich should lose the Manastorm.
             Also perhaps Iron Lich (Lich -> Master Lich -> Iron Lich)  */
    default: /* Birth Menus */
        result = _archlich_get_race_t();
    }

    result->name = "Lich";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER | RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_EATS_DEVICES;
    result->calc_innate_attacks = _calc_innate_attacks;
    result->birth = _birth;
    result->caster_info = _caster_info;
    result->pseudo_class_idx = CLASS_MAGE;
    result->shop_adjust = 135;

    result->boss_r_idx = MON_VECNA;

    if (birth_hack || spoiler_hack)
    {
        result->subname = NULL;
        result->subdesc = NULL;
    }
    return result;
}
