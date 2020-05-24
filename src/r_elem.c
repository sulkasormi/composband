#include "angband.h"

static cptr _desc = 
    "Elementals are mindless creatures animated from a single "
    "elemental form. As a species, elementals can never be cut, "
    "and being virtually mindless they are immune to sanity blasting. "
    "They rarely feel fear, and are resistant to confusion and poison. In addition, they "
    "gain specific powers and abilities depending on their form.\n \n"
    "Earth Elementals are slow, but gain AC bonuses due to their tough bodies. "
    "They are resistant to shards, and may even turn their skins to stone. "
    "At home in elemental earth, they may travel freely through rocky confines. "
    "Unfortunately, as they are made of earth, their magic potions frequently turn to mud!\n \n"
    "Air Elementals are shockingly fast, but perhaps that is just the "
    "crackle of their electrified bodies? They may hurl bolts and balls of "
    "electricity at their enemies, and may even imbue their weapons with "
    "deadly lightning. However, being surrounded by lightning, rings, "
    "amulets, wands and rods are quickly destroyed!\n \n"
    "Fire Elementals are somewhat fast (enough to run circles around "
    "their earthen brethren) and are cloaked in flames. They may attack "
    "with hell's fury, but need to be on the lookout for cold-wielding foes. "
    "Being surrounded by fire, they rapidly burn scrolls and staves to ash!\n \n"
    "Finally, there are the Water Elementals, creatures able to conjure deadly "
    "water bolts. They are immune to stunning. Their attacks can be quite corrosive; "
    "but alas, sometimes their own armor corrodes as well. Water Elementals also have a unique "
    "energy system based on the speed of their internal water flow; fighting and "
    "the use of elemental powers excite them and turn them into a raging torrent, "
    "while resting or aimlessly splashing about slows them down again. High flow "
    "rates make water elementals stronger, but their armor sometimes gets caught in "
    "the flow and slips off!";

static void _calc_bonuses(void) 
{
    res_add(RES_CONF);
    res_add(RES_FEAR);
    p_ptr->no_cut = TRUE;
    p_ptr->no_eldritch = TRUE;
    p_ptr->levitation = TRUE;
    p_ptr->slow_digest = TRUE;

    if (p_ptr->lev >= 5)
        res_add(RES_POIS);
    if (p_ptr->lev >= 10)
        p_ptr->see_inv++;
    if (p_ptr->lev >= 15)
        p_ptr->free_act++;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_SLOW_DIGEST);
    if (p_ptr->lev >= 5)
        add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->lev >= 15)
        add_flag(flgs, OF_FREE_ACT);
}

static bool _elemental_travel(int flag)
{
    int  rng = p_ptr->lev / 2 + 10;
    int  x, y;

    if (!tgt_pt(&x, &y, rng)) return FALSE;
    if (!in_bounds(y, x)) return FALSE;

    if (!cave_have_flag_bold(y, x, flag))
    {
        msg_print("Failed! You are out of your element!");
        teleport_player((p_ptr->lev + 2) * 2, TELEPORT_PASSIVE);
    }
    else if (one_in_(7))
    {
        msg_print("You failed to travel correctly!");
        teleport_player((p_ptr->lev + 2) * 2, TELEPORT_PASSIVE);
    }
    else
    {
        /* Note: teleport_player_to requires FF_TELEPORTABLE, which won't work for walls */
        if (flag == FF_WALL && !cave_have_flag_bold(y, x, FF_PERMANENT))
            move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
        else
            teleport_player_to(y, x, 0);
    }
    water_mana_action(1, 25);
    return TRUE;
}

static bool _elemental_healing(int flag)
{
    int dir, ct = 0;

    if (!cave_have_flag_bold(py, px, flag))
    {
        msg_print("Failed! You are out of your element!");
        return FALSE;
    }

    for (dir = 0; dir < 8; dir++)
    {
        int x = px + ddx_ddd[dir];
        int y = py + ddy_ddd[dir];
        
        if (!in_bounds(y, x)) continue;
        if (cave_have_flag_bold(y, x, flag)) ct++;
    }

    if (ct < 4)
    {
        msg_print("Failed! You need to be surrounded by your element!");
        return FALSE;
    }

    msg_print("You bask in your element and slowly feel your life returning ... ");
    if (elemental_is_(ELEMENTAL_WATER)) 
    {
        hp_player(50 + water_flow_rate() * 2);
        water_mana_action(1, 20);
    }
    else hp_player(100 + p_ptr->lev * 3);
    return TRUE;
}

static void _elemental_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Elemental Rage");
        break;
    default:
        berserk_spell(cmd, res);
    }
}

static void _destroy_aux(obj_ptr obj, cptr fmt)
{
    char o_name[MAX_NLEN];

    object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED | OD_SINGULAR);
    msg_format(fmt, o_name);

    stats_on_p_destroy(obj, 1);
    obj_dec_number(obj, 1, TRUE);
    obj_release(obj, 0);
}

static void _elemental_pack_destroy(object_p p, cptr destroy_fmt, int chance)
{
    int inven_ct = 0;
    int equip_ct = 0;
    slot_t slot;
    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = pack_obj(slot);

        if (!obj) continue;
        if (!p(obj)) continue;
        
        if (randint0(1000) >= MAX(1, chance)) continue;
        _destroy_aux(obj, destroy_fmt);
        ++inven_ct;
   }
    for (slot = 1; slot <= equip_max(); slot++)
    {
        obj_ptr obj = equip_obj(slot);

        if (!obj) continue;
        if (!p(obj)) continue;
        
        if (randint0(1000) >= MAX(1, chance/3)) continue;
        _destroy_aux(obj, destroy_fmt);
        ++equip_ct;
    }

    if (equip_ct)
    {
        p_ptr->update |= PU_BONUS | PU_TORCH | PU_MANA;
        p_ptr->redraw |= PR_EQUIPPY;
        p_ptr->window |= PW_EQUIP;
    }
    if (inven_ct)
        p_ptr->window |= PW_INVEN;

    if (equip_ct + inven_ct)
        disturb(1, 0);
}

/**********************************************************************
 *             25
 * Earth Spirit -> Earth Elemental
 **********************************************************************/
static void _earth_birth(void) 
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 6;
    forge.pval = 3;
    add_flag(forge.flags, OF_STR);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HAFTED, SV_CLUB));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    py_birth_obj(&forge);

    p_ptr->current_r_idx = MON_EARTH_SPIRIT; 

    py_birth_light();
}

static void _earth_gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_EARTH_SPIRIT && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_EARTH_ELEMENTAL;
        msg_print("You have evolved into an Earth Elemental.");
        p_ptr->redraw |= PR_MAP;
    }
}

static void _shard_bolt_spell(int cmd, variant *res)
{
    int dd = 1 + p_ptr->lev / 3;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shard Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of shards at chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, ds, 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_SHARDS, dir, damroll(dd, ds));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _earthen_healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earthen Healing");
        break;
    case SPELL_DESC:
        var_set_string(res, "If you are surrounded by rock, you may heal yourself at the cost of several acts.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, 100 + p_ptr->lev * 3));
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_healing(FF_WALL));
        break;
    case SPELL_ENERGY:
        var_set_int(res, 300);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _earthen_portal_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Earthen Portal");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move instantaneously to chosen rocky locale.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_travel(FF_WALL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shard_ball_spell(int cmd, variant *res)
{
    int dam = py_prorata_level(300);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shard Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of shards at chosen target");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SHARDS, dir, dam, 2);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, dam / 6);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wall_of_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wall of Earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "Create walls on all open, surrounding squares.");
        break;
    case SPELL_CAST:
        wall_stone();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _earth_get_powers[] = 
{
    { A_STR, {  1,  1, 35, eat_rock_spell}},
    { A_STR, {  7,  5, 35, _shard_bolt_spell}},
    { A_CON, { 10, 10, 40, stone_skin_spell}},
    { A_CON, { 15, 10, 40, sense_surroundings_spell}},
    { A_STR, { 20, 15, 40, earthquake_spell}},
    { A_STR, { 25,  0, 50, _shard_ball_spell}},
    { A_CON, { 30, 50, 60, _wall_of_earth_spell}},
    { A_CON, { 35,  0, 65, _earthen_healing_spell}},
    { A_DEX, { 42, 75, 70, _earthen_portal_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

static void _earth_calc_bonuses(void) 
{
    int to_a = py_prorata_level(50);

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_FIRE);
    res_add(RES_COLD);
    res_add(RES_ELEC);
    res_add(RES_SHARDS);
    p_ptr->pass_wall = TRUE;
    p_ptr->no_passwall_dam = TRUE;
    p_ptr->regen += 100;

    p_ptr->pspeed--;
    if (p_ptr->lev >= 25)
        p_ptr->pspeed--;

    _calc_bonuses();
}

static void _earth_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_DEC_SPEED);
    add_flag(flgs, OF_REGEN);

    _get_flags(flgs);
}

static bool _earth_p(object_type *o_ptr)
{
    if (object_is_artifact(o_ptr)) return FALSE;
    if (o_ptr->tval != TV_POTION) return FALSE;
    return TRUE;
}

static void _earth_process_world(void)
{
    int chance = 40 - p_ptr->lev/2;
    _elemental_pack_destroy(_earth_p, "Your %s turns to mud.", chance);
}

static race_t *_earth_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Earth Spirit", "Earth Elemental"};
    int           rank = 0;

    if (p_ptr->lev >= 25) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  18,  40,   5,  25,  16,  70,  25};
    skills_t xs = {  8,   8,  12,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 170;

        me.birth = _earth_birth;
        me.get_powers = _earth_get_powers;
        me.calc_bonuses = _earth_calc_bonuses;
        me.get_flags = _earth_get_flags;
        me.gain_level = _earth_gain_level;
        me.process_world = _earth_process_world;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  2 + 3*rank;
    me.stats[A_INT] = -5;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] = -2 - 2*rank;
    me.stats[A_CON] =  2 + 4*rank;
    me.stats[A_CHR] =  0;
    me.life = 105 + 15*rank;
    me.boss_r_idx = MON_QUAKER;

    return &me;
}

/**********************************************************************
 *           25
 * Air Spirit -> Air Elemental
 **********************************************************************/
static void _air_birth(void) 
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_JEWELRY_ELEMENTAL;
    forge.to_a = 15;
    add_flag(forge.flags, OF_RES_ELEC);
    add_flag(forge.flags, OF_AURA_ELEC);
    add_flag(forge.flags, OF_IGNORE_ELEC);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_LONG_SWORD));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_STAFF, SV_ANY));
    if (device_init_fixed(&forge, EFFECT_NOTHING))
        py_birth_obj(&forge);
    py_birth_light();

    p_ptr->current_r_idx = MON_AIR_SPIRIT; 
}

static void _air_gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_AIR_SPIRIT && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_AIR_ELEMENTAL;
        msg_print("You have evolved into an Air Elemental.");
        p_ptr->redraw |= PR_MAP;
    }
}

static void _confusing_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confusing Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a confusing blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_CONF));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _lightning_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a shocking blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_ELEC));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _lightning_storm_spell(int cmd, variant *res)
{
    int dam = py_prorata_level(350);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of electricity.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ELEC, dir, dam, 4);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, dam / 10);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _whirlwind_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Whirlwind");
        break;
    default:
        massacre_spell(cmd, res);
    }
}

static void _sky_gate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sky Gate");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move instantaneously to chosen open location.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_travel(FF_FLOOR));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _air_get_powers[] = 
{
    { A_STR, {  2,  3, 25, lightning_bolt_spell}},
    { A_DEX, {  5,  3, 25, phase_door_spell}},
    { A_DEX, { 10,  5, 35, teleport_spell}},
    { A_DEX, { 15,  7,  0, _confusing_strike_spell}},
    { A_STR, { 17, 10, 45, lightning_ball_spell}},
    { A_DEX, { 25, 20, 50, haste_self_spell}},
    { A_STR, { 32, 25,  0, _lightning_strike_spell}},
    { A_DEX, { 35, 40, 55, _whirlwind_spell}},
    { A_STR, { 37,  0, 55, _lightning_storm_spell}},
    { A_DEX, { 42, 40, 60, _sky_gate_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

static void _air_calc_bonuses(void) 
{
    res_add(RES_ELEC);

    p_ptr->pspeed += 2;
    if (p_ptr->lev >= 25)
    {
        p_ptr->pspeed += 3;
        p_ptr->pspeed += (p_ptr->lev - 25) / 5; /* up to +10 speed */
        res_add(RES_ELEC);
        res_add(RES_ACID);
        res_add(RES_FIRE);
        res_add(RES_COLD);
        p_ptr->sh_elec++;
    }
    if (p_ptr->lev >= 50)
        res_add_immune(RES_ELEC);

    _calc_bonuses();
}

static void _air_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 25)
    {
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_AURA_ELEC);
    }

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_ELEC);

    _get_flags(flgs);
}

static bool _air_p(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (object_is_artifact(o_ptr)) return FALSE;
    if ( o_ptr->tval != TV_RING 
      && o_ptr->tval != TV_AMULET 
      && o_ptr->tval != TV_WAND 
      && o_ptr->tval != TV_ROD) 
    {
        return FALSE;
    }
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_ELEC)) return FALSE;
    return TRUE;
}

static void _air_process_world(void)
{
    _elemental_pack_destroy(_air_p, "Your shocking touch destroys your %s.", 40);
}

static race_t *_air_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Air Spirit", "Air Elemental"};
    int           rank = 0;

    if (p_ptr->lev >= 25) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  25,  35,   6,  25,  16,  55,  35};
    skills_t xs = {  8,  10,   9,   0,   0,   0,  20,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 225;

        me.birth = _air_birth;
        me.get_powers = _air_get_powers;
        me.calc_bonuses = _air_calc_bonuses;
        me.get_flags = _air_get_flags;
        me.gain_level = _air_gain_level;
        me.process_world = _air_process_world;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -4;
    me.stats[A_WIS] = -4;
    me.stats[A_DEX] =  3 + 3*rank;
    me.stats[A_CON] =  0 + 2*rank;
    me.stats[A_CHR] =  0;
    me.life = 90 + 10*rank;
    me.boss_r_idx = MON_ARIEL;

    return &me;
}

/**********************************************************************
 *             25
 * Water Spirit -> Water Elemental
 **********************************************************************/
int water_flow_rate(void)
{
    return (p_ptr->csp / 10);
}

static bool _armor_melt_hack = FALSE;
static byte _toistot = 0;

void water_mana_action(byte check_hurt_mode, int mana)
{
    static s32b kieppi = 0;
    static u16b kieppi_mana = 0;
    bool uuskieppi = TRUE;
    if (!elemental_is_(ELEMENTAL_WATER)) return;
    if (!mana) return;
    if ((check_hurt_mode == 2) && (!monsters_damaged_hack)) return;
    if (check_hurt_mode == 1) _toistot = MIN(10, _toistot + 1);
    else if ((monsters_damaged_hack) || (mana < 25) || (kieppi_mana)) _toistot = 0;
    if (_toistot > 0) mana /= _toistot;
    if ((kieppi == game_turn) && (mana > 0)) /* Multiple blows in one round - diminishing returns */
    {
        mana -= mana * (MIN(50, kieppi_mana * 3 / 4)) / 100;
        uuskieppi = FALSE;
    }
    else kieppi_mana = 0;
    kieppi = game_turn;
    kieppi_mana += mana;
    p_ptr->csp += mana;
    if (p_ptr->csp > 1000) p_ptr->csp = 1000;
    if (p_ptr->csp < 0) p_ptr->csp = 0;
    p_ptr->update |= (PU_BONUS);
    p_ptr->redraw |= (PR_MANA); 
    if ((mana > 0) && ((uuskieppi) || (one_in_(5))) && (randint0(2178) < MIN(76, ((water_flow_rate() * 9 / 5) - 83))))
    {
        slot_t slot = equip_find_first(object_is_body_armour);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            char o_name[MAX_NLEN];
            if (o_ptr->marked & OM_SLIPPING) return;
            object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_OMIT_PREFIX | OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
            o_ptr->marked |= OM_SLIPPING;
            msg_format("You flow too fast! Your %s is caught in the flow and slips off!", o_name);
            if ((object_is_(o_ptr, TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI)) && (personality_is_(PERS_SEXY)))
            {
                msg_print("You roar!");
                p_ptr->csp = 1000;
                set_fast(p_ptr->fast + 15, FALSE);
                set_hero(p_ptr->hero + 15, FALSE);
            }
            p_ptr->update |= (PU_HP | PU_SPELLS);
            p_ptr->redraw |= (PR_STATS);
            p_ptr->window |= (PW_EQUIP);
            msg_print(NULL);
        }
    }
    handle_stuff();
}

static void _water_birth(void) 
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_JEWELRY_ELEMENTAL;
    forge.to_a = 15;
    add_flag(forge.flags, OF_RES_ACID);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 5;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_POLEARM, SV_TRIDENT));
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_POTION, SV_POTION_WATER, rand_range(15, 23));
    py_birth_light();

    p_ptr->current_r_idx = MON_WATER_SPIRIT; 
    p_ptr->csp = 0;
}

static void _water_gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_WATER_SPIRIT && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_WATER_ELEMENTAL;
        msg_print("You have evolved into a Water Elemental.");
        p_ptr->redraw |= PR_MAP;
    }
}

static bool _adjust_armor_aux(void)
{
    slot_t slot = equip_find_first(object_is_body_armour);
    if (!slot)
    {
        msg_print("You're not even attempting to wear body armour.");
        return FALSE;
    }
    else
    {
        object_type *o_ptr = equip_obj(slot);
//        char o_name[MAX_NLEN];
        if (!(o_ptr->marked & OM_SLIPPING))
        {
            msg_print("Your armour does not need adjusting.");
            return FALSE;
        }
        o_ptr->marked &= ~OM_SLIPPING;
//        object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_OMIT_PREFIX | OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
        obj_release(o_ptr, 0);
    }
    p_ptr->update |= (PU_BONUS | PU_HP | PU_SPELLS);
    p_ptr->redraw |= (PR_STATS);
    p_ptr->window |= (PW_EQUIP);
    handle_stuff();
    return TRUE;
}

static void _adjust_armor_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Adjust Armour");
        break;
    case SPELL_DESC:
        var_set_string(res, "Flow back into a slipping body armour, at the cost of two turns.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _adjust_armor_aux());
        break;
    case SPELL_ENERGY:
        var_set_int(res, 200);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _acid_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Acid Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a corrosive blow.");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 25) 
        {
            p_ptr->melt_armor = TRUE;
            _armor_melt_hack = TRUE;
        }
        var_set_bool(res, do_blow(PY_ATTACK_ACID));
        p_ptr->melt_armor = FALSE;
        _armor_melt_hack = FALSE;        
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev >= 25) ? 5 : 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _water_ball_spell(int cmd, variant *res)
{
    int dam = 28 + py_prorata_level(144);
    dam += MIN(dam, water_flow_rate());

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of water.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_WATER2, dir, dam, 4);
        var_set_bool(res, TRUE);
        water_mana_action(2, dam * 2 / 5);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, dam / 5);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _water_gate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Water Gate");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move instantaneously to chosen watery location.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_travel(FF_WATER));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _water_healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Healing Bath");
        break;
    case SPELL_DESC:
        var_set_string(res, "If you are surrounded by water, you may heal yourself at the cost of two turns.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, 50 + water_flow_rate()*2));
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_healing(FF_WATER));
        break;
    case SPELL_ENERGY:
        var_set_int(res, 200);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _water_get_powers[] = 
{
    { A_DEX, {  1,  0,  0, _adjust_armor_spell}},
    { A_DEX, {  5,  5,  0, _acid_strike_spell}},
    { A_STR, { 12,  7, 35, acid_bolt_spell}},
    { A_STR, { 17, 10, 40, _elemental_rage_spell}},
    { A_STR, { 23, 16, 65, water_bolt_spell}},
    { A_STR, { 32,  0, 65, _water_ball_spell}},
    { A_CON, { 38,  0, 75, _water_healing_spell}},
    { A_DEX, { 42, 50, 75, _water_gate_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

static void _water_calc_bonuses(void) 
{
    res_add(RES_ACID);
    p_ptr->no_stun = TRUE;
    p_ptr->pspeed += water_flow_rate() / 27;
    
    if (p_ptr->lev >= 25)
    {
        res_add(RES_ACID);
        p_ptr->melt_armor = _armor_melt_hack;
    }

    if (p_ptr->lev >= 50)
        res_add_immune(RES_ACID);

    _calc_bonuses();
}

static void _water_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_RES_ACID);

    if (water_flow_rate() >= 27)
        add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_ACID);

    _get_flags(flgs);
}

static void _water_damage(obj_ptr obj)
{
    u32b flgs[OF_ARRAY_SIZE];
    char o_name[MAX_NLEN];
    int  chance = 15;

    if (!object_is_armour(obj)) return;
    if (randint0(1452) >= chance) return;
    if (obj->ac + obj->to_a <= 0) return;

    obj_flags(obj, flgs);
    if (have_flag(flgs, OF_IM_ACID)) return;
    if (have_flag(flgs, OF_RES_ACID)) return;
    if (have_flag(flgs, OF_IGNORE_ACID)) return;
//    if (have_flag(flgs, OF_IGNORE_ACID) && !(one_in_(10))) return;
//    if (object_is_artifact(obj) && !(one_in_(2))) return;

    object_desc(o_name, obj, OD_OMIT_PREFIX | OD_NAME_ONLY | OD_COLOR_CODED);
    msg_format("Your watery touch corrodes your %s!", o_name);
    obj->to_a--;

    if (obj->loc.where == INV_EQUIP)
    {
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
    }
    else if (obj->loc.where == INV_PACK)
        p_ptr->window |= PW_INVEN;
    disturb(1, 0);
}

static void _water_process_world(void)
{
    equip_for_each(_water_damage);
    pack_for_each(_water_damage);
}

static caster_info * _water_caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "elemental power";
        me.which_stat = A_STR;
        me.options = CASTER_USE_HP;
        init = TRUE;
    }
    return &me;
}

static void _water_load_player(savefile_ptr file)
{
    if (savefile_is_older_than(file, 7, 0, 9, 3)) _toistot = 0;
    else _toistot = savefile_read_byte(file);
}

static void _water_save_player(savefile_ptr file)
{
    savefile_write_byte(file, _toistot);
}

static race_t *_water_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Water Spirit", "Water Elemental"};
    int           rank = 0;

    if (p_ptr->lev >= 25) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  25,  35,   5,  25,  16,  62,  35};
    skills_t xs = {  8,  10,   9,   0,   0,   0,  18,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 200;

        me.birth = _water_birth;
        me.get_powers = _water_get_powers;
        me.calc_bonuses = _water_calc_bonuses;
        me.get_flags = _water_get_flags;
        me.gain_level = _water_gain_level;
        me.process_world = _water_process_world;
        me.caster_info = _water_caster_info;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  2 + 2*rank;
    me.stats[A_INT] = -4;
    me.stats[A_WIS] = -4;
    me.stats[A_DEX] =  1 + rank;
    me.stats[A_CON] =  2 + 2*rank;
    me.stats[A_CHR] =  1;
    me.life = 96 + MIN(6, p_ptr->lev/5);
    me.boss_r_idx = MON_MOIRE;
    me.load_player = _water_load_player;
    me.save_player = _water_save_player;

    return &me;
}

/**********************************************************************
 *            25                40
 * Fire Spirit -> Fire Elemental -> Magma Elemental
 **********************************************************************/
static void _fire_birth(void) 
{ 
    object_type forge;
    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_JEWELRY_ELEMENTAL;
    forge.to_a = 15;
    add_flag(forge.flags, OF_RES_FIRE);
    add_flag(forge.flags, OF_AURA_FIRE);
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HAFTED, SV_WHIP));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_FLASK, SV_ANY));
    apply_magic(&forge, 1, AM_NO_FIXED_ART);
    forge.number = (byte)rand_range(7, 12);
    py_birth_obj(&forge);

    py_birth_light();

    p_ptr->current_r_idx = MON_FIRE_SPIRIT; 
}

static void _fire_gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_FIRE_SPIRIT && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_FIRE_ELEMENTAL;
        msg_print("You have evolved into a Fire Elemental.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FIRE_ELEMENTAL && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_MAGMA_ELEMENTAL;
        msg_print("You have evolved into a Magma Elemental.");
        p_ptr->redraw |= PR_MAP;
    }
}

static void _fire_whip_spell(int cmd, variant *res)
{
    int dd = 3 + p_ptr->lev / 7;
    int ds = 6;
    int range = 2 + p_ptr->lev / 6;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Whip");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a short beam of fire.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("dam %dd%d (rng %d)", dd, ds, range));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        project_length = range;
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_FIRE, dir, damroll(dd, ds));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _fire_storm_spell(int cmd, variant *res)
{
    int dam = py_prorata_level(400);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a huge ball of fire.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_FIRE, dir, dam, 4);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, dam / 10);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _flaming_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Flaming Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a fiery blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_FIRE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _fire_door_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Fire Door");
        break;
    case SPELL_DESC:
        var_set_string(res, "Move instantaneously to chosen fiery location.");
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_travel(FF_LAVA));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _fire_healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Healing Flames");
        break;
    case SPELL_DESC:
        var_set_string(res, "If you are surrounded by lava, you may heal yourself at the cost of several acts.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, 100 + p_ptr->lev * 3));
        break;
    case SPELL_CAST:
        var_set_bool(res, _elemental_healing(FF_LAVA));
        break;
    case SPELL_ENERGY:
        var_set_int(res, 250);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _fire_get_powers[] = 
{
    { A_STR, {  2,  3, 25, _fire_whip_spell}},
    { A_STR, {  7,  5, 35, fire_bolt_spell}},
    { A_STR, { 12,  7,  0, _flaming_strike_spell}},
    { A_DEX, { 15, 10, 40, hide_in_flame_spell}},
    { A_STR, { 20, 12, 45, fire_ball_spell}},
    { A_STR, { 22, 15, 45, plasma_bolt_spell}},
    { A_DEX, { 25, 20, 55, flow_of_lava_spell}},
    { A_STR, { 27, 25, 65, plasma_ball_spell}},
    { A_CON, { 35,  0, 65, _fire_healing_spell}},
    { A_STR, { 37,  0, 55, _fire_storm_spell}},
    { A_DEX, { 42, 50, 60, _fire_door_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

static void _fire_calc_bonuses(void) 
{
    res_add(RES_FIRE);
    res_add_vuln(RES_COLD);
    p_ptr->sh_fire++;

    if (p_ptr->lev >= 25)
    {
        p_ptr->pspeed += 2;
        res_add(RES_FIRE);
    }

    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 3;
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
        res_add(RES_FIRE);
        res_add(RES_ELEC);
    }

    if (p_ptr->lev >= 50)
        res_add_immune(RES_FIRE);

    _calc_bonuses();
}

static void _fire_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_VULN_COLD);
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_AURA_FIRE);

    if (p_ptr->lev >= 25)
        add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_ELEC);

    if (p_ptr->lev >= 50)
        add_flag(flgs, OF_IM_FIRE);

    _get_flags(flgs);
}

static bool _fire_p(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (object_is_artifact(o_ptr)) return FALSE;
    if (o_ptr->tval != TV_SCROLL && o_ptr->tval != TV_STAFF) return FALSE;
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_FIRE)) return FALSE;
    return TRUE;
}

static void _fire_process_world(void)
{
    _elemental_pack_destroy(_fire_p, "Your fiery touch burns your %s.", 40);
}

static race_t *_fire_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Fire Spirit", "Fire Elemental", "Magma Elemental"};
    int           rank = 0;

    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  25,  35,   4,  25,  16,  65,  35};
    skills_t xs = {  8,  10,   9,   0,   0,   0,  25,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 200;

        me.birth = _fire_birth;
        me.get_powers = _fire_get_powers;
        me.calc_bonuses = _fire_calc_bonuses;
        me.get_flags = _fire_get_flags;
        me.gain_level = _fire_gain_level;
        me.process_world = _fire_process_world;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  1 + 2*rank;
    me.stats[A_INT] = -4;
    me.stats[A_WIS] = -4;
    me.stats[A_DEX] =  0 + rank;
    me.stats[A_CON] =  1 + rank;
    me.stats[A_CHR] =  0;
    me.life = 100 + 5*rank;
    me.boss_r_idx = MON_LOGE;

    return &me;
}

static name_desc_t _info[ELEMENTAL_MAX] = {
    { "Earth Elemental",
        "Earth Elementals are creatures of rock: Strong, tough and slow. "
        "They may move freely through the earth and are capable of conjuring "
        "sharp clods of earth to hurl at their foes. Their skin is very tough, "
        "and they can even turn their bodies to stone. However, being made of "
        "earth, their potions frequently turn to mud." },
    { "Air Elemental",
        "Air Elementals are creatures of electricity. They are incredibly fast, "
        "blinking in and out of sight as they shower their enemies with confusing "
        "and shocking blows. Electricity crackles menacingly about their nimble frames, "
        "tending to destroy rings, amulets, wands and rods." },
    { "Water Elemental",
        "Water Elementals are creatures of water, able to modify this ubiquitous "
        "liquid into a deadly and often corrosive weapon of destruction. Fear their "
        "rage! They cannot be stunned. Their corrosive nature erodes any armor that "
        "gets too close." },
    { "Fire Elemental",
        "Fire Elementals are creatures of flame. They have a vast arsenal of "
        "flaming attacks with which to singe the fiercest of foes. However, they "
        "must beware of cold based attacks! Being wreathed in flames, scrolls and "
        "staves are quickly burned to ash." },
};

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_elemental_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case ELEMENTAL_EARTH:
        result = _earth_get_race_t();
        break;
    case ELEMENTAL_AIR:
        result = _air_get_race_t();
        break;
    case ELEMENTAL_WATER:
        result = _water_get_race_t();
        break;
    case ELEMENTAL_FIRE:
        result = _fire_get_race_t();
        break;
    default: /* ?? */
        result = _earth_get_race_t();
    }

    result->name = "Elemental";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER | RACE_IS_NONLIVING;
    if (psubrace == ELEMENTAL_AIR) result->flags |= RACE_EATS_DEVICES;
    result->base_hp = 30;
    result->pseudo_class_idx = CLASS_WARRIOR;
    result->shop_adjust = 120;

    if (birth_hack || spoiler_hack)
    {
        result->subname = _info[psubrace].name;
        result->subdesc = _info[psubrace].desc;
    }

    return result;
}


