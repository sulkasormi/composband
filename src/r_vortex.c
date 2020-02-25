#include "angband.h"

/**********************************************************************
 * Evolution
 **********************************************************************/
#define _MAX_PER_TIER 10
#define _MAX_TIERS     5

typedef struct {
    int level;
    int r_ids[_MAX_PER_TIER];
} _tier_t;

static _tier_t _tiers[_MAX_TIERS] = {
    {  1, { MON_FIRE_VORTEX, MON_COLD_VORTEX, -1 } },
    { 14, { MON_WATER_VORTEX, MON_ENERGY_VORTEX, -1 } },
    { 27, { MON_NEXUS_VORTEX, MON_PLASMA_VORTEX, MON_SHIMMERING_VORTEX, -1 } },
    { 39, { MON_TIME_VORTEX, MON_SHARD_VORTEX, MON_CHAOS_VORTEX, MON_DISINTEGRATE_VORTEX, -1 } },
    { 50, { MON_AETHER_VORTEX, -1 } },
};

static int _vortex_mut_is(void)
{
    if (p_ptr->max_plv < 50) return 0;
    if (mut_present(MUT_VORTEX_MELEE)) return MUT_VORTEX_MELEE;
    if (mut_present(MUT_VORTEX_SPEED)) return MUT_VORTEX_SPEED;
    if (mut_present(MUT_VORTEX_CONTROL)) return MUT_VORTEX_CONTROL;
    return 0;
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
    if (spoiler_hack)
        return list[0];
    return list[randint0(_count(list))];
}

static int _laskuri(int list[])
{
    int i, elem_ct = 0;

    for (i = 0; ; i+=2)
    {
        if (list[i] == -1) break;
        elem_ct++;
    }
    return elem_ct;
}

static void _display(rect_t r, int list[])
{
    doc_ptr doc = doc_alloc(r.cx);
    int i, elem_ct = 0;

    elem_ct = _laskuri(list);

    doc_insert(doc, "<style:table>");
    for (i = 0; i < elem_ct; i++)
    {
        gf_info_ptr this_gf = gf_lookup(list[i*2]);
        if (this_gf) doc_printf(doc, " %c) <color:%c>%s</color>%s\n", I2A(i), attr_to_attr_char(this_gf->color), this_gf->name, (i > elem_ct - 6) ? strpos("Time", this_gf->name) ? " (-50% power)" : " (-33% power)" : "");
    }
    doc_insert(doc, "</style>");
    doc_sync_term(doc, doc_range_all(doc), doc_pos_create(r.x, r.y));
    doc_free(doc);
}

static bool _allow_control_hack = FALSE;

static int _random_weights(int list[])
{
    int i, k;
    int tot = 0;

    if ((mut_present(MUT_VORTEX_CONTROL)) && (_allow_control_hack))
    {
        int cmd, valinta, elem_ct = _laskuri(list);
        rect_t r = ui_menu_rect();
        bool valmis = FALSE;

        if (REPEAT_PULL(&cmd))
        {
            valinta = A2I(cmd);
            if (0 <= valinta && valinta < elem_ct)
                return list[valinta*2];
        }

        if (r.cx > 80)
            r.cx = 80;
        screen_save();
        prt("Breathe which element?", 0, 0);
        valinta = -1;
        while (!valmis)
        {
            _display(r, list);

            cmd = inkey_special(FALSE);

            if (cmd == ESCAPE)
            {
                valmis = TRUE;
            }
            if ('a' <= cmd && cmd < 'a' + elem_ct)
            {
                valinta = A2I(cmd);
                valmis = TRUE;
            }
        }
        screen_load();
        if ((valinta >= 0) && (valinta < elem_ct))
        {
            REPEAT_PUSH(I2A(valinta));
            return list[valinta*2];
        } /* otherwise just pick one at random */
    }

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

static int _rank(void)
{
    int r = 0;
    if (p_ptr->lev >= 14) r++;
    if (p_ptr->lev >= 27) r++;
    if (p_ptr->lev >= 39) r++;
    if (p_ptr->lev >= 50) r++;
    return r;
}

static cptr _mon_name(int r_idx)
{
    if (r_idx == MON_CHAOS_VORTEX) /* This became a Death vortex, but we are being nostalgic ... */
        return "Chaos vortex";
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static void _gain_power(void)
{
    if (!_vortex_mut_is())
    {
        int idx = mut_gain_choice(mut_vortex_pred);
        mut_lock(idx);
    }
}

static void _gain_level(int new_level)
{
    int tier = _find_tier(p_ptr->current_r_idx);
    if (tier < 0 || tier == _MAX_TIERS - 1) return;
    if (p_ptr->lev >= _tiers[tier + 1].level)
    {
        p_ptr->current_r_idx = _random(_tiers[tier+1].r_ids);
        msg_format("You have evolved into a %s.", _mon_name(p_ptr->current_r_idx));
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    if (new_level == 50) _gain_power();
}

/**********************************************************************
 * Birth
 **********************************************************************/
static void _birth(void)
{
    object_type forge;

    p_ptr->current_r_idx = _random(_tiers[0].r_ids);
    msg_format("You are born a %s.", _mon_name(p_ptr->current_r_idx));
    p_ptr->redraw |= PR_MAP;
    equip_on_change_race();

    skills_innate_init("Engulf", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_h = 7;
    forge.to_d = 2;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_JACK));
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
}

/**********************************************************************
 * Attacks
 **********************************************************************/
int vortex_get_effect(void)
{
    switch (p_ptr->current_r_idx)
    {
    case MON_FIRE_VORTEX: return GF_FIRE;
    case MON_COLD_VORTEX: return GF_COLD;

    case MON_WATER_VORTEX: return GF_ACID;
    case MON_ENERGY_VORTEX: return GF_ELEC;

    case MON_NEXUS_VORTEX: return GF_NEXUS;
    case MON_PLASMA_VORTEX: return GF_PLASMA;
    case MON_SHIMMERING_VORTEX: return GF_LITE;

    case MON_TIME_VORTEX: return GF_TIME;
    case MON_SHARD_VORTEX: return GF_SHARDS;
    case MON_CHAOS_VORTEX: return GF_CHAOS;
    case MON_DISINTEGRATE_VORTEX: return GF_DISINTEGRATE;

    case MON_AETHER_VORTEX:
    {
        int choices[] = {GF_FIRE, 1,
                         GF_COLD, 1,
                         GF_ELEC, 2,
                         GF_ACID, 2,
                         GF_POIS, 1,
                         GF_LITE, 3,
                         GF_DARK, 3,
                         GF_CONFUSION, 1,
                         GF_NETHER, 1,
                         GF_NEXUS, 1, /* Teleportation effects can be frustrating during melee! */
                         GF_SOUND, 4,
                         GF_SHARDS, 5,
                         GF_CHAOS, 4,
                         GF_DISENCHANT, 4,
                         GF_TIME, 3,
                         GF_INERT, 3,
                         GF_FORCE, 4,
                         GF_GRAVITY, 1, /* Teleportation effects can be frustrating during melee! */
                         GF_PLASMA, 5,
                         -1, 0};
        return _random_weights(choices);
    }
    }
    return GF_MISSILE;
}

static void _calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int r = MIN(3, _rank());
    int to_d = r + l/5;
    int to_h = l/2;

    /* Engulf */
    {
        innate_attack_t a = {0};

        a.dd = 3 + r;
        a.ds = 3 + r;
        a.to_d += to_d;
        a.to_h += to_h;

        if (mut_present(MUT_VORTEX_MELEE))
        {
            a.ds += 1;
            a.to_d += 25;
            a.to_h += 25;
        }

        a.weight = 150;
        if (p_ptr->current_r_idx == MON_AETHER_VORTEX)
        {
            /* Note: see cmd1.c innate_attacks() ... the effect will
               be randomly chosen on each hit. These are just placeholders. */
            a.effect[0] = GF_ELEC;
            a.effect[1] = GF_FIRE; a.effect_chance[1] = 75;
            a.effect[2] = GF_ACID; a.effect_chance[2] = 50;
        }
        else
            a.effect[0] = vortex_get_effect();

        if (p_ptr->current_r_idx == MON_SHARD_VORTEX)
            a.flags |= INNATE_VORPAL;

        calc_innate_blows(&a, (p_ptr->current_r_idx == MON_AETHER_VORTEX) ? 363 : 375);
        a.msg = "You engulf.";
        a.name = "Engulf";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Breath
 **********************************************************************/
static int _breath_effect(void)
{
    return vortex_get_effect();
}

static int _breath_amount(void)
{
    int pct = 10 + py_prorata_level(15);
    return MAX(5, p_ptr->chp * pct / 100);
}

static cptr _breath_desc(void)
{
    gf_info_ptr gf;
    if (p_ptr->current_r_idx == MON_AETHER_VORTEX)
        return "almost anything";
    /* XXX gf_name() returns a color coded result, but the 'powers' menu is still
     * old school and does not use a z-doc for formatting. */
    gf = gf_lookup(_breath_effect());
    if (gf)
        return gf->name;
    return "";
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
    {
        int l = p_ptr->lev;
        int cst = l*l/((mut_present(MUT_VORTEX_CONTROL)) ? 75 : 150);
        var_set_int(res, cst);
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        int e, voima;
        bool early_effect = mut_present(MUT_VORTEX_CONTROL);
        var_set_bool(res, FALSE);
        voima = _breath_amount();
        if (early_effect)
        {
            _allow_control_hack = TRUE;
            e = _breath_effect();
            switch (e)
            {
                case GF_PLASMA:
                case GF_GRAVITY:
                case GF_INERT:
                case GF_FORCE:
                    voima -= voima / 3;
                    break;
                case GF_TIME:
                    voima -= voima / 2;
                    break;
                default: break;
            }
            _allow_control_hack = FALSE;
        }
        if (get_fire_dir(&dir))
        {
            if (!early_effect) e = _breath_effect();
            msg_format("You breathe %s.", gf_name(e));
            fire_ball(e, dir, voima, -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _escape_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Elemental Escape");
        break;
    case SPELL_DESC:
        var_set_string(res, "Flee amidst a maelstrom of the elements.");
        break;
    case SPELL_COST_EXTRA:
        _breathe_spell(cmd, res);
        break;
    case SPELL_CAST:
    {
        int d = _breath_amount();
        int ct = 3;
        int i;
        for (i = 0; i < ct; i++)
            fire_ball(_breath_effect(), 0, d, 6);
        teleport_player(30, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _explode_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Elemental Explosion");
        break;
    case SPELL_DESC: {
        char buf[255];
        sprintf(buf, "Generates a large ball of %s.", _breath_desc());
        var_set_string(res, buf);
        break; }
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        _breathe_spell(cmd, res);
        break;
    case SPELL_CAST:
    {
        int d = _breath_amount() * 2;
        int e = _breath_effect();
        msg_format("You explode in %s.", gf_name(e));
        fire_ball(e, 0, d, 8);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spin_away_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spin Away");
        break;
    case SPELL_DESC:
        var_set_string(res, "Spin rapidly towards a random, nearby, visible location.");
        break;
    default:
        strafing_spell(cmd, res);
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
    case SPELL_DESC:
        var_set_string(res, "Spin rapidly in place, attacking all adjacent monsters.");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

static void _unleash_elements_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Unleash Elements");
        break;
    case SPELL_DESC:
        var_set_string(res, "Unleash your breath uncontrollably and at random, though with devastating effect.");
        break;
    case SPELL_COST_EXTRA:
        _breathe_spell(cmd, res);
        break;
    case SPELL_CAST:
    {
        int num = damroll(5, 3);
        int dam = MAX(1, _breath_amount()/3);
        int y = py, x = px, i;

        var_set_bool(res, FALSE);

        for (i = 0; i < num; i++)
        {
            int attempts = 1000;
            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);
                if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                if (!player_bold(y, x)) break;
            }
            project(0, 3, y, x, dam, _breath_effect(),
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

static power_info _powers[] = {
    { A_CON, {  1,  3, 30, _breathe_spell}},
    { A_DEX, {  5,  5, 30, _spin_away_spell}},
    { A_DEX, { 12,  7, 35, _whirlwind_spell}},
    { A_CON, { 25,  7, 40, _explode_spell}},
    { A_DEX, { 30, 10, 40, _escape_spell}},
    { A_CON, { 50, 50, 65, _unleash_elements_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static int _get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _powers);
}

/**********************************************************************
 * Bonuses
 **********************************************************************/
static void _calc_bonuses(void)
{
    int r = _rank();

    p_ptr->to_a += 5 + 5*r;
    p_ptr->dis_to_a += 5 + 5*r;

    p_ptr->free_act++;
    res_add(RES_CONF);
    res_add_immune(RES_BLIND);
    res_add_immune(RES_FEAR);
    p_ptr->see_nocto = TRUE;
    p_ptr->no_stun = TRUE;
    p_ptr->no_cut = TRUE;
    p_ptr->no_eldritch = TRUE;
    p_ptr->hold_life++;
    p_ptr->levitation = TRUE;

    switch (p_ptr->current_r_idx)
    {
    case MON_FIRE_VORTEX:
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add_vuln(RES_COLD);
        p_ptr->sh_fire++;
        break;
    case MON_COLD_VORTEX:
        res_add(RES_COLD);
        res_add(RES_COLD);
        res_add_vuln(RES_FIRE);
        p_ptr->sh_cold++;
        break;

    case MON_WATER_VORTEX:
        p_ptr->pspeed += 1;
        res_add(RES_ACID);
        res_add(RES_ACID);
        break;
    case MON_ENERGY_VORTEX:
        p_ptr->pspeed += 1;
        res_add(RES_ELEC);
        res_add(RES_ELEC);
        p_ptr->sh_elec++;
        break;

    case MON_NEXUS_VORTEX:
        p_ptr->pspeed += 3;
        res_add(RES_NEXUS);
        res_add(RES_TELEPORT);
        break;
    case MON_PLASMA_VORTEX:
        p_ptr->pspeed += 3;
        res_add(RES_FIRE);
        res_add(RES_ELEC);
        p_ptr->sh_fire++;
        p_ptr->sh_elec++;
        break;
    case MON_SHIMMERING_VORTEX:
        p_ptr->pspeed += 3;
        res_add(RES_LITE);
        break;

    case MON_TIME_VORTEX:
        p_ptr->pspeed += 7;
        res_add(RES_TIME);
        break;
    case MON_SHARD_VORTEX:
        p_ptr->pspeed += 5;
        p_ptr->skill_dig += 100;
        res_add(RES_SHARDS);
        p_ptr->sh_shards++;
        break;
    case MON_CHAOS_VORTEX:
        p_ptr->pspeed += 7;
        res_add(RES_CHAOS);
        break;
    case MON_DISINTEGRATE_VORTEX:
        p_ptr->pspeed += 5;
        p_ptr->kill_wall = TRUE;
        break;

    case MON_AETHER_VORTEX:
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
        break;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_IM_BLIND);
    add_flag(flgs, OF_IM_FEAR);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_LEVITATION);

    switch (p_ptr->current_r_idx)
    {
    case MON_FIRE_VORTEX:
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_VULN_COLD);
        add_flag(flgs, OF_AURA_FIRE);
        break;
    case MON_COLD_VORTEX:
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_VULN_FIRE);
        add_flag(flgs, OF_AURA_COLD);
        break;

    case MON_WATER_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_ACID);
        break;
    case MON_ENERGY_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_AURA_ELEC);
        break;

    case MON_NEXUS_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_NEXUS);
        break;
    case MON_PLASMA_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_AURA_FIRE);
        add_flag(flgs, OF_AURA_ELEC);
        break;
    case MON_SHIMMERING_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_LITE);
        break;

    case MON_TIME_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_TIME);
        break;
    case MON_SHARD_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_SHARDS);
        add_flag(flgs, OF_AURA_SHARDS);
        break;
    case MON_CHAOS_VORTEX:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_CHAOS);
        break;
    case MON_DISINTEGRATE_VORTEX:
        add_flag(flgs, OF_SPEED);
        break;

    case MON_AETHER_VORTEX:
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
        break;
    }
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_vortex_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           r = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  40,   4,   5,   5,  60,   0};
    skills_t xs = {  8,   7,  15,   0,   0,   0,  20,   0};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "Vortex";
        me.desc = "Vortices are mindless whirlwinds of elemental forces twirling their way "
                    "through the dungeon in search of prey. They are seldom confused "
                    "and never blinded since they lack vision in the normal sense; indeed, they "
                    "seem to be aware of their surroundings even without eyes, and need not wear a "
                    "light source. They can be neither stunned nor cut, and their mindless "
                    "nature renders them immune to fear. But, alas! they are hopeless with "
                    "magical devices and somewhat weak in melee; and while they may breathe "
                    "almost effortlessly, their breath attacks are not particularly strong.\n \n"
                    "Vortices do not have normal bodies; rather, they are simply a rapidly "
                    "spinning elemental mass. As such, they may incorporate objects into their "
                    "rotating essence, gaining bonuses and protection in the process. "
                    "The number of objects they may use increases with level, and there are no "
                    "restrictions on the types of equipment they may 'wield' in each slot. "
                    "While they may equip weapons this way, they cannot attack with them; "
                    "instead, the vortex may engulf nearby monsters with their elemental essence.\n \n"
                    "Vortices evolve randomly, though all seem to end up with the evolutionary "
                    "perfection of the Aether vortex. At level 50 they gain a special talent "
                    "to augment their power.";

        me.infra = 0;
        me.exp = 125;
        me.base_hp = 25;
        me.shop_adjust = 120;

        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;
        me.boss_r_idx = MON_WIRUIN;

        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING;

        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    me.stats[A_STR] =  0 + r;
    me.stats[A_INT] = -5;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  2 + r;
    me.stats[A_CON] =  1 + r/2;
    me.stats[A_CHR] =  0;
    me.life = 93 + r;
    if (r == 4) me.flags |= RACE_DEMI_TALENT;
    me.equip_template = mon_get_equip_template();

    return &me;
}
