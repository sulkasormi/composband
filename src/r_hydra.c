#include "angband.h"


static cptr _desc = 
    "Hydras are monsters with multiple heads. As they evolve, they grow "
    "additional heads, giving them extra attacks as well as the ability to "
    "wear more equipment (helmets and amulets). While not every head may "
    "wear a helmet and an amulet (imagine how powerful that would be!), "
    "each head is capable of attacking; so an 11-headed hydra can attack "
    "up to 11 times per round, provided it has enough strength "
    "and dexterity to command such a fearsome arsenal.\n \n"
    "Hydras are not very magical, and so play much like warriors but with a "
    "few extra abilities. Five- and seven-headed hydras resist poison and gain a few poison-based attacks. "
    "Nine- and eleven-headed hydras are creatures of flame: they resist fire, their attacks burn, and "
    "they are capable of breathing fire among other flame-based attacks. All hydras "
    "regenerate extremely rapidly.";

static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_TWO_HEADED_HYDRA;
    equip_on_change_race();
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_AMULET, 0));
    forge.name2 = EGO_JEWELRY_ELEMENTAL;
    add_flag(forge.flags, OF_RES_ACID);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_CROWN, SV_IRON_CROWN));
    forge.name2 = EGO_CROWN_MIGHT;
    forge.pval = 1;
    forge.to_a = 5;
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/**********************************************************************
 * Hydra Attacks: Each head can attack (stats permitting) for up to 11 blows!
 **********************************************************************/
static int _head_count(void)
{
    switch (p_ptr->current_r_idx)
    {
    case MON_TWO_HEADED_HYDRA: return 2;
    case MON_FOUR_HEADED_HYDRA: return 4;
    case MON_FIVE_HEADED_HYDRA: return 5;
    case MON_SEVEN_HEADED_HYDRA: return 7;
    case MON_NINE_HEADED_HYDRA: return 9;
    case MON_ELEVEN_HEADED_HYDRA: return 11;
    }
    return 2; /* paranoia */
}

static int _bite_effect(void)
{
    switch (p_ptr->current_r_idx)
    {
    case MON_FIVE_HEADED_HYDRA:
    case MON_SEVEN_HEADED_HYDRA: return GF_POIS;

    case MON_NINE_HEADED_HYDRA:
    case MON_ELEVEN_HEADED_HYDRA: return GF_FIRE;
    }
    return 0;
}

static void _calc_innate_attacks(void)
{
    innate_attack_t    a = {0};
    int l = p_ptr->lev;

    a.dd = 1 + (l+3)/14;
    a.ds = 4 + l/16;
    a.to_h = l/2;
    a.weight = 100 + l*2;
    a.effect[0] = GF_MISSILE;
    a.effect[1] = _bite_effect();

    calc_innate_blows(&a, _head_count() * 100);
    
    a.msg = "You bite.";
    a.name = "Bite";
    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

/**********************************************************************
 * Hydra Breath
 **********************************************************************/
static int _breath_effect(void)
{
    switch (p_ptr->current_r_idx)
    {
    case MON_NINE_HEADED_HYDRA:
    case MON_ELEVEN_HEADED_HYDRA: 
        return GF_FIRE;
    }
    return GF_POIS;
}

static int _breath_amount(void)
{
    int l = p_ptr->lev;
    return MIN(375, p_ptr->chp * (20 + l*l*l*15/125000) / 100);
}

static int _breath_cost(void)
{
    int l = p_ptr->lev;
    return MAX(l/2 + l*l*15/2500, 1);
}

static void _breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe");
        break;
    case SPELL_DESC:
    {
        char elname[32] = "software bugs";
        gf_info_ptr gf = gf_lookup(_breath_effect());
        if ((gf) && (gf->name))
        {
           strcpy(elname, gf->name);
           str_tolower(elname);
        }
        var_set_string(res, format("Breathes %s at your opponent.", elname));
        break;
    }
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
            msg_format("You breathe %s", gf_name(e));
            fire_ball(e, dir, _breath_amount(), -2);
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
 * Hydra Powers
 **********************************************************************/
static power_info _fire_powers[] = {
    { A_CHR, {  1,  3, 30, scare_monster_spell} },
    { A_STR, { 20,  5, 30, fire_bolt_spell} },
    { A_CON, { 30,  0, 50, _breathe_spell} },
    { A_STR, { 45, 10, 50, fire_ball_spell} },
    { A_STR, { 45, 15, 50, plasma_bolt_spell} },
    {    -1, { -1, -1, -1, NULL}}
};

static power_info _poison_powers[] = {
    { A_CHR, {  1,  3, 30, scare_monster_spell} },
    { A_STR, { 20,  3, 30, stinking_cloud_spell} },
    { A_CON, { 30,  0, 50, _breathe_spell} },
    {    -1, { -1, -1, -1, NULL}}
};

static power_info *_get_powers(void) 
{
    switch (p_ptr->current_r_idx)
    {
    case MON_NINE_HEADED_HYDRA:
    case MON_ELEVEN_HEADED_HYDRA: 
        return _fire_powers;
    }
    return _poison_powers;
}

/**********************************************************************
 * Hydra Bonuses
 **********************************************************************/
static void _calc_bonuses(void) 
{
    int ac = 20 + p_ptr->lev/10;

    p_ptr->skill_dig += 50;
    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;
    p_ptr->regen += 100 + 4*p_ptr->lev;

    switch (p_ptr->current_r_idx)
    {
    case MON_TWO_HEADED_HYDRA:
        break;
    case MON_FOUR_HEADED_HYDRA:
        p_ptr->pspeed += 2;
        break;
    case MON_FIVE_HEADED_HYDRA:
        p_ptr->pspeed += 3;
        res_add(RES_POIS);
        break;
    case MON_SEVEN_HEADED_HYDRA:
        p_ptr->pspeed += 5;
        res_add(RES_POIS);
        break;
    case MON_NINE_HEADED_HYDRA:
        p_ptr->pspeed += 7;
        res_add(RES_FIRE);
        break;
    case MON_ELEVEN_HEADED_HYDRA:
        p_ptr->pspeed += 10;
        res_add(RES_FIRE);
        break;
    }    
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_REGEN);
    switch (p_ptr->current_r_idx)
    {
    case MON_TWO_HEADED_HYDRA:
        break;
    case MON_FOUR_HEADED_HYDRA:
        add_flag(flgs, OF_SPEED);
        break;
    case MON_FIVE_HEADED_HYDRA:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_POIS);
        break;
    case MON_SEVEN_HEADED_HYDRA:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_POIS);
        break;
    case MON_NINE_HEADED_HYDRA:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_FIRE);
        break;
    case MON_ELEVEN_HEADED_HYDRA:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_FIRE);
        break;
    }    
}

/**********************************************************************
 * Hydra Evolution
 **********************************************************************/
static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_TWO_HEADED_HYDRA && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_FOUR_HEADED_HYDRA;
        equip_on_change_race();
        msg_print("You have evolved into a 4-headed hydra.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FOUR_HEADED_HYDRA && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_FIVE_HEADED_HYDRA;
        equip_on_change_race();
        msg_print("You have evolved into a 5-headed hydra.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FIVE_HEADED_HYDRA && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_SEVEN_HEADED_HYDRA;
        equip_on_change_race();
        msg_print("You have evolved into a 7-headed hydra.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_SEVEN_HEADED_HYDRA && new_level >= 37)
    {
        p_ptr->current_r_idx = MON_NINE_HEADED_HYDRA;
        equip_on_change_race();
        msg_print("You have evolved into a 9-headed hydra.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_NINE_HEADED_HYDRA && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_ELEVEN_HEADED_HYDRA;
        equip_on_change_race();
        msg_print("You have evolved into an 11-headed hydra.");
        p_ptr->redraw |= PR_MAP;
    }
}

/**********************************************************************
 * Public Methods
 **********************************************************************/
race_t *mon_hydra_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[6] =  {"2-headed hydra", "4-headed hydra", "5-headed hydra", "7-headed hydra", "9-headed hydra", "11-headed hydra"};    
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 37) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  19,  35,   1,  10,   7,  58,  30};
    skills_t xs = { 12,  10,  10,   0,   0,   0,  16,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Hydra";
        me.desc = _desc;

        me.infra = 5;
        me.exp = 130;
        me.base_hp = 45;
        me.shop_adjust = 130;

        me.get_powers_fn = _get_powers;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;

        me.flags = RACE_IS_MONSTER;
        me.boss_r_idx = MON_LERNEAN_HYDRA;
        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    me.subname = NULL;
    if (!birth_hack && !spoiler_hack)
        me.subname = titles[rank];
    me.stats[A_STR] = rank;
    me.stats[A_INT] = -2;
    me.stats[A_WIS] = -2;
    me.stats[A_DEX] = (rank + 1)/2;
    me.stats[A_CON] = rank;
    me.stats[A_CHR] =  0;
    me.life = 99 + 3*rank;
    me.equip_template = mon_get_equip_template();

    return &me;
}
