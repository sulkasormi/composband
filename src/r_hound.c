#include "angband.h"

/**********************************************************************
 * Hound Evolution
 **********************************************************************/
#define _MAX_PER_TIER 10
#define _MAX_TIERS     7

typedef struct {
    int level;
    int r_ids[_MAX_PER_TIER];
} _tier_t;

static _tier_t _tiers[_MAX_TIERS] = {
    {  1, { MON_CLEAR_HOUND, -1 } },
    {  8, { MON_LIGHT_HOUND, MON_SHADOW_HOUND, -1 } },
    { 15, { MON_FIRE_HOUND, MON_COLD_HOUND, MON_ENERGY_HOUND, MON_AIR_HOUND, MON_WATER_HOUND, -1 } },
    { 23, { MON_EARTH_HOUND, MON_VIBRATION_HOUND, MON_NEXUS_HOUND, MON_MULTI_HUED_HOUND, -1 } },
    { 31, { MON_INERTIA_HOUND, MON_IMPACT_HOUND, MON_NETHER_HOUND, -1 } },
    { 39, { MON_GRAVITY_HOUND, MON_TIME_HOUND, MON_PLASMA_HOUND, MON_CHAOS_HOUND, -1 } },
    { 47, { MON_HOUND_OF_TINDALOS, MON_MANA_HOUND, MON_AETHER_HOUND, -1 } },
};

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
    if (spoiler_hack)
        return list[0];
    return list[randint0(_count(list))];
}

static int _random_weights(int list[])
{
    int i, k;
    int tot = 0;

    for (i = 0; ; i+=2)
    {
        if (list[i] == -1) break;
        tot += list[i+1];
    }
    k = randint1(tot);

    for (i = 0; ; i+=2)
    {
        k -= list[i+1];
        if (k <= 0) return list[i];
    }
    /*return 0;  error */
}

static int _find(int what, int list[])
{
    int i;
    for (i = 0; ; i++)
    {
        int n = list[i];
        if (n == -1) return -1;
        if (n == what) return i;
    }
    /* return -1;  unreachable */
}

static int _find_tier(int r_idx)
{
    int i;
    for (i = 0; i < _MAX_TIERS; i++)
    {
        if (_find(r_idx, _tiers[i].r_ids) >= 0) return i;
    }
    return -1;
}

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

/**********************************************************************
 * Hound Equipment
 **********************************************************************/
static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_CLEAR_HOUND;
    equip_on_change_race();
    skills_innate_init("Claw", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 3;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_BOOTS, SV_PAIR_OF_METAL_SHOD_BOOTS));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/**********************************************************************
 * Hound Attacks
 **********************************************************************/
void hound_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_d = py_prorata_level(15);
    int to_h = l/2;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 17;
        a.ds = 3 + l / 21;
        a.to_d += to_d;
        a.to_h += to_h;

        a.weight = 100;
        calc_innate_blows(&a, 200);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 23;
        a.ds = 4 + l / 5;
        a.to_d += to_d;
        a.to_h += to_h;

        a.weight = 200;
        a.effect[0] = GF_MISSILE;
        if (p_ptr->current_r_idx == MON_HOUND_OF_TINDALOS)
        {
            a.effect[1] = GF_TIME;
            a.effect_chance[1] = 50;
            a.dd = 2; /* 3d14+15 -> 2d14+10 with 50% 2d14+10 additional time damage */
            a.to_d = py_prorata_level(10);
        }

        calc_innate_blows(&a, 300);
        a.msg = "You bite.";
        a.name = "Bite";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Hound Breath
 **********************************************************************/
static int _breath_effect(void)
{
    switch (p_ptr->current_r_idx)
    {
    case MON_LIGHT_HOUND: return GF_LITE;
    case MON_SHADOW_HOUND: return GF_DARK;
    case MON_FIRE_HOUND: return GF_FIRE;
    case MON_COLD_HOUND: return GF_COLD;
    case MON_ENERGY_HOUND: return GF_ELEC;
    case MON_AIR_HOUND: return GF_POIS;
    case MON_WATER_HOUND: return GF_ACID;
    case MON_EARTH_HOUND: return GF_SHARDS;
    case MON_VIBRATION_HOUND: return GF_SOUND;
    case MON_NEXUS_HOUND: return GF_NEXUS;
    case MON_MULTI_HUED_HOUND:
    {
        int choices[] = {GF_FIRE, GF_COLD, GF_ELEC, GF_ACID, GF_POIS, -1};
        return _random(choices);
    }
    case MON_INERTIA_HOUND: return GF_INERT;
    case MON_IMPACT_HOUND: return GF_FORCE;
    case MON_NETHER_HOUND: return GF_NETHER;
    case MON_GRAVITY_HOUND: return GF_GRAVITY;
    case MON_TIME_HOUND: return GF_TIME;
    case MON_PLASMA_HOUND: return GF_PLASMA;
    case MON_CHAOS_HOUND: return GF_CHAOS;
    case MON_HOUND_OF_TINDALOS:
    {
        int choices[] = {GF_NETHER, GF_TIME, -1};
        return _random(choices);
    }
    case MON_MANA_HOUND: return GF_MANA;
    case MON_AETHER_HOUND:
    {
        int choices[] = {GF_FIRE, 1,
                         GF_COLD, 1,
                         GF_ELEC, 2,
                         GF_ACID, 2,
                         GF_POIS, 1,
                         GF_LITE, 2,
                         GF_DARK, 2,
                         GF_CONFUSION, 1,
                         GF_NETHER, 1,
                         GF_NEXUS, 2,
                         GF_SOUND, 3,
                         GF_SHARDS, 5,
                         GF_CHAOS, 4,
                         GF_DISENCHANT, 4,
                         GF_TIME, 3,
                         GF_INERT, 3,
                         GF_FORCE, 3,
                         GF_GRAVITY, 4,
                         GF_PLASMA, 5,
                         -1, 0};
        return _random_weights(choices);
    }
    }
    return GF_FIRE; /* ?? */
}

static int _breath_amount(void)
{
    int pct = 15 + py_prorata_level(15);
    return MAX(5, p_ptr->chp * pct / 100);
}

static cptr _breath_desc(void)
{
    gf_info_ptr gf;
    switch (p_ptr->current_r_idx)
    {
    case MON_AETHER_HOUND: return "almost anything";
    case MON_HOUND_OF_TINDALOS: return "nether or time";
    case MON_MULTI_HUED_HOUND: return "acid, fire, cold, lightning or poison";
    }
    gf = gf_lookup(_breath_effect());
    if (gf) return gf->name;
    return "something";
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
        static char elly[40];
        strcpy(elly, _breath_desc());
        str_tolower(elly);
        var_set_string(res, format("Breathes %s at your opponent.", elly));
        break;
    }
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
    {
        int l = p_ptr->lev;
        int cst = l*l/100;
        var_set_int(res, cst);
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            msg_format("You breathe %s", gf_name(e));
            fire_ball(e, dir, _breath_amount(), -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _bark_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bark");
        break;
    case SPELL_DESC:
        var_set_string(res, "You will make enough noise to wake the neighbors.");
        break;
    case SPELL_CAST:
        msg_print("Woof! Woof!");
        aggravate_monsters(0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hound_leap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Pounce");
        break;
    case SPELL_DESC:
        var_set_string(res, "Leap towards a nearby prey and attack in a single action.");
        break;
    default:
        rush_attack_spell(cmd, res);
        break;
    }
}

void hound_run_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Run");
        break;
    case SPELL_DESC:
        var_set_string(res, "By moving quickly, you will be able to run down your prey or to flee quickly in a pinch.");
        break;
    default:
        quick_walk_spell(cmd, res);
        break;
    }
}

void hound_sniff_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sniff");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to smell a nearby meal.");
        break;
    case SPELL_CAST:
        detect_monsters_living(DETECT_RAD_DEFAULT, "You smell nearby monsters.");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hound_stalk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stalk");
        break;
    case SPELL_DESC:
        var_set_string(res, "By moving slowly, you will be able to sneak up on your prey very effectively.");
        break;
    case SPELL_CAST:
        if (p_ptr->action == ACTION_STALK) set_action(ACTION_NONE);
        else set_action(ACTION_STALK);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Hounds: Evolution is tier based with a random choice from each tier.
 **********************************************************************/
static power_info _powers[] = {
    { A_DEX, {  1,  0,  0, _bark_spell } },
    { A_DEX, {  1,  1, 30, hound_sniff_spell } },
    { A_CON, {  8,  5, 30, _breathe_spell}},
    { A_DEX, { 10,  0,  0, hound_stalk_spell}},
    { A_DEX, { 15,  0,  0, hound_run_spell}},
    { A_DEX, { 20, 10, 30, hound_leap_spell}},
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _tindalos_powers[] = {
    { A_DEX, {  5,  2, 30, phase_door_spell}},
    { A_DEX, { 25, 15, 50, teleport_to_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static power_info *_get_powers(void) {
    static power_info spells[MAX_SPELLS];
    int max = MAX_SPELLS;
    int ct = get_powers_aux(spells, max, _powers, FALSE);
    if (p_ptr->current_r_idx == MON_HOUND_OF_TINDALOS)
        ct += get_powers_aux(spells + ct, max - ct, _tindalos_powers, FALSE);
    spells[ct].spell.fn = NULL;
    return spells;
}
static void _calc_bonuses(void) {
    int to_a = py_prorata_level_aux(25, 1, 2, 2);
    int tier = _find_tier(p_ptr->current_r_idx);

    p_ptr->skill_dig += 100;
    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    if (tier > 3)
        p_ptr->pspeed += (tier - 3);

    switch (p_ptr->current_r_idx)
    {
    /* Tier 1 */
    case MON_LIGHT_HOUND:
        res_add(RES_LITE);
        break;
    case MON_SHADOW_HOUND:
        res_add(RES_DARK);
        res_add_vuln(RES_LITE);
        break;
    /* Tier 2 */
    case MON_FIRE_HOUND:
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add_vuln(RES_COLD);
        break;
    case MON_COLD_HOUND:
        res_add(RES_COLD);
        res_add(RES_COLD);
        res_add_vuln(RES_FIRE);
        break;
    case MON_ENERGY_HOUND:
        res_add(RES_ELEC);
        res_add(RES_ELEC);
        break;
    case MON_AIR_HOUND:
        res_add(RES_POIS);
        res_add(RES_POIS);
        break;
    case MON_WATER_HOUND:
        res_add(RES_ACID);
        res_add(RES_ACID);
        break;
    /* Tier 3 */
    case MON_EARTH_HOUND:
        res_add(RES_SHARDS);
        break;
    case MON_VIBRATION_HOUND:
        res_add(RES_SOUND);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_NEXUS_HOUND:
        res_add(RES_NEXUS);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_MULTI_HUED_HOUND:
        p_ptr->pspeed += 3;
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ACID);
        res_add(RES_ELEC);
        res_add(RES_POIS);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    /* Tier 4 */
    case MON_INERTIA_HOUND:
        res_add(RES_CONF);
        p_ptr->free_act++;
        p_ptr->no_slow = TRUE;
        break;
    case MON_IMPACT_HOUND:
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_NETHER_HOUND:
        p_ptr->pspeed += 5;
        res_add(RES_NETHER);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    /* Tier 5 */
    case MON_GRAVITY_HOUND:
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_TIME_HOUND:
        p_ptr->pspeed += 7;
        res_add(RES_TIME);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_PLASMA_HOUND:
        p_ptr->pspeed += 5;
        res_add(RES_ELEC);
        res_add(RES_FIRE);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_CHAOS_HOUND:
        p_ptr->pspeed += 5;
        res_add(RES_CHAOS);
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    /* Tier 6 */
    case MON_HOUND_OF_TINDALOS:
        p_ptr->pspeed += 7;
        res_add(RES_NETHER);
        res_add(RES_TIME);
        res_add(RES_CONF);
        p_ptr->free_act++;
        p_ptr->levitation = TRUE;
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
        break;
    case MON_MANA_HOUND:
        p_ptr->pspeed += 10;
        res_add(RES_CONF);
        p_ptr->free_act++;
        break;
    case MON_AETHER_HOUND:
        p_ptr->pspeed += 5;
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ACID);
        res_add(RES_ELEC);
        res_add(RES_POIS);
        res_add(RES_LITE);
        res_add(RES_DARK);
        res_add(RES_CONF);
        res_add(RES_NETHER);
        res_add(RES_NEXUS);
        res_add(RES_SOUND);
        res_add(RES_SHARDS);
        res_add(RES_CHAOS);
        res_add(RES_DISEN);
        res_add(RES_TIME);
        p_ptr->sh_cold++;
        p_ptr->sh_fire++;
        p_ptr->sh_elec++;
        p_ptr->free_act++;
        break;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    int tier = _find_tier(p_ptr->current_r_idx);
    if (tier > 3)
        add_flag(flgs, OF_SPEED);

    switch (p_ptr->current_r_idx)
    {
    case MON_LIGHT_HOUND:
        add_flag(flgs, OF_RES_LITE);
        break;
    case MON_SHADOW_HOUND:
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_VULN_LITE);
        break;
    case MON_FIRE_HOUND:
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_VULN_COLD);
        break;
    case MON_COLD_HOUND:
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_VULN_FIRE);
        break;
    case MON_ENERGY_HOUND:
        add_flag(flgs, OF_RES_ELEC);
        break;
    case MON_AIR_HOUND:
        add_flag(flgs, OF_RES_POIS);
        break;
    case MON_WATER_HOUND:
        add_flag(flgs, OF_RES_ACID);
        break;
    case MON_EARTH_HOUND:
        add_flag(flgs, OF_RES_SHARDS);
        break;
    case MON_VIBRATION_HOUND:
        add_flag(flgs, OF_RES_SOUND);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_NEXUS_HOUND:
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_MULTI_HUED_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_INERTIA_HOUND:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_IMPACT_HOUND:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_NETHER_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_GRAVITY_HOUND:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_TIME_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_PLASMA_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_CHAOS_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_HOUND_OF_TINDALOS:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_LEVITATION);
        break;
    case MON_MANA_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        break;
    case MON_AETHER_HOUND:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_LITE);
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_SOUND);
        add_flag(flgs, OF_RES_SHARDS);
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_RES_DISEN);
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_AURA_COLD);
        add_flag(flgs, OF_AURA_FIRE);
        add_flag(flgs, OF_AURA_ELEC);
        add_flag(flgs, OF_FREE_ACT);
        break;
    }
}
static void _gain_level(int new_level) {
    int tier = _find_tier(p_ptr->current_r_idx);
    if (tier < 0 || tier == _MAX_TIERS - 1) return;
    if (p_ptr->lev >= _tiers[tier + 1].level)
    {
        p_ptr->current_r_idx = _random(_tiers[tier+1].r_ids);
        msg_format("You have evolved into a %s.", _mon_name(p_ptr->current_r_idx));
        p_ptr->redraw |= PR_MAP;
    }
}
race_t *mon_hound_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  20,  31,   4,  20,  15,  56,  30};
    skills_t xs = {  8,   8,  10,   1,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Hound";
        me.desc = "While Zephyr Hounds typically hunt in packs, you have chosen to go it alone. "
                    "You will begin life in the weak form of a Clear Hound. As you mature, "
                    "you will take a number of evolutionary steps. At each such step, the "
                    "form you evolve into will be randomly determined; but each tier offers "
                    "more powerful choices than the last. Who knows - you may even evolve "
                    "into an Aether Hound some day.\n \n"
                    "Hounds are not magical, and so cannot cast spells; however, they are "
                    "fantastic hunters, and learn a number of special abilities that will aid them "
                    "in this endeavour. And as you probably know all too well, hounds love to breathe!\n \n"
                    "Due to their canine bodies, Hounds cannot wield weapons; instead, they "
                    "attack with their vicious bite and razor-sharp claws. While their melee is "
                    "not as powerful as that of dragons, it usually gets the job done. "
                    "Hounds can wear four rings on their front paws, a pair of boots on their "
                    "hind legs, an amulet around their neck, and even a cloak and a suit of "
                    "body armor.";


        me.infra = 5;
        me.exp = 150;
        me.life = 100;
        me.base_hp = 22;
        me.shop_adjust = 110;

        me.calc_innate_attacks = hound_calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers_fn = _get_powers;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;

        me.flags = RACE_IS_MONSTER; /* | RACE_IS_ILLITERATE? */
        me.boss_r_idx = MON_CARCHAROTH;

        me.pseudo_class_idx = CLASS_ROGUE;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    me.stats[A_STR] =  1 + p_ptr->lev/12;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  2 + p_ptr->lev/15;
    me.stats[A_CON] =  1 + p_ptr->lev/15;
    me.stats[A_CHR] =  0 + p_ptr->lev/25;
    me.equip_template = mon_get_equip_template();

    return &me;
}
