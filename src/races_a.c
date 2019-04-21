#include "angband.h"

/****************************************************************
 * Amberite
 ****************************************************************/
static power_info _amberite_powers[] =
{
    { A_INT, {30, 50, 70, shadow_shifting_spell}},
    { A_WIS, {40, 75, 75, pattern_mindwalk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _amberite_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _amberite_powers);
}
static void _amberite_calc_bonuses(void)
{
    p_ptr->sustain_con = TRUE;
    p_ptr->regen += 100;
}
static void _amberite_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_CON);
    add_flag(flgs, OF_REGEN);
}
race_t *amberite_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Amberite";
        me.desc = "The Amberites are a reputedly immortal race, who are endowed with numerous "
                    "advantages in addition to their longevity. They are very tough and their "
                    "constitution cannot be reduced, and their ability to heal wounds far "
                    "surpasses that of any other race. Having seen virtually everything, "
                    "very little is new to them, and they gain levels much slower than the "
                    "other races.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  4;
        me.skills.dev =  3;
        me.skills.sav =  3;
        me.skills.stl =  2;
        me.skills.srh =  3;
        me.skills.fos = 13;
        me.skills.thn = 15;
        me.skills.thb =  7;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 190;
        me.infra = 0;
        me.shop_adjust = 100;


        me.calc_bonuses = _amberite_calc_bonuses;
        me.get_powers = _amberite_get_powers;
        me.get_flags = _amberite_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Android
 ****************************************************************/
static int _obj_value(object_type *o_ptr)
{
    object_type  copy = *o_ptr;
    copy.discount = 0;
    copy.curse_flags = 0;
    return obj_value_real(&copy);
}
int android_obj_exp(object_type *o_ptr)
{
    int value, exp, level;

    if (!o_ptr) return 0;
    if (!object_is_wearable(o_ptr)) return 0;
    if (object_is_jewelry(o_ptr)) return 0;
    if (o_ptr->tval == TV_LITE) return 0;

    value = _obj_value(o_ptr);
    if (value <= 0) return 0;
    if (object_is_(o_ptr, TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI) && p_ptr->personality != PERS_SEXY
        && p_ptr->pclass != CLASS_POLITICIAN) value /= 32;
    if (value > 5000000) value = 5000000;

    level = MAX(k_info[o_ptr->k_idx].level - 8, 1);

    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        int            a_lvl = MAX(a_ptr->level - 8, 5);
        int            r_div = a_ptr->gen_flags & OFG_INSTA_ART ? 10 : 3;

        level = (level + a_lvl) / 2;
        level += MIN(20, a_ptr->rarity/r_div);
    }
    else if (o_ptr->art_name || o_ptr->name2)
    {
        int fake_level = 10 + value / 1500;

        if (fake_level > 90)
            fake_level = 90;

        fake_level = MAX(fake_level - 8, 5);
        level = MAX(level, (level + fake_level) / 2 + 3);
    }

    if (o_ptr->tval == TV_DRAG_ARMOR || o_ptr->tval == TV_CARD) level /= 2;

    if ( object_is_artifact(o_ptr)
      || object_is_ego(o_ptr)
      || o_ptr->tval == TV_DRAG_ARMOR
      || object_is_dragon_armor(o_ptr)
      || object_is_(o_ptr, TV_SWORD, SV_DIAMOND_EDGE) )
    {
        if (level > 65) level = 35 + (level - 65) / 5;
        else if (level > 35) level = 25 + (level - 35) / 3;
        else if (level > 15) level = 15 + (level - 15) / 2;
        exp = MIN(100000L, value) * level * level / 2;
        if (value > 100000L)
            exp += (value - 100000L) * level * level / 8;
    }
    else
    {
        exp = MIN(100000L, value) * level;
        if (value > 100000L)
            exp += (value - 100000L) * level / 4;
    }
    if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_BOW)
        return exp / 48;
    else if (object_is_body_armour(o_ptr))
        return 3 * exp / 32;
    else
        return exp / 16;
}

void android_calc_exp(void)
{
    int slot;
    s32b total_exp = 0;

    if (p_ptr->is_dead) return;

    if (p_ptr->prace != RACE_ANDROID) return;

    for (slot = 1; slot <= equip_max(); slot++)
    {
        object_type *o_ptr = equip_obj(slot);
        total_exp += android_obj_exp(o_ptr);
    }
    p_ptr->exp = p_ptr->max_exp = total_exp;
    if (p_ptr->pclass == CLASS_POLITICIAN) politician_android_experience();
    check_experience();
}


static int _android_get_powers(spell_info* spells, int max)
{
    int         ct = 0;
    spell_info *spell = &spells[ct++];

    if (p_ptr->lev < 10)
    {
        spell->level = 1;
        spell->cost = 7;
        spell->fail = calculate_fail_rate(1, 30, p_ptr->stat_ind[A_STR]);
        spell->fn = android_ray_gun_spell;
    }
    else if (p_ptr->lev < 25)
    {
        spell->level = 10;
        spell->cost = 13;
        spell->fail = calculate_fail_rate(10, 30, p_ptr->stat_ind[A_STR]);
        spell->fn = android_blaster_spell;
    }
    else if (p_ptr->lev < 35)
    {
        spell->level = 25;
        spell->cost = 26;
        spell->fail = calculate_fail_rate(25, 40, p_ptr->stat_ind[A_STR]);
        spell->fn = android_bazooka_spell;
    }
    else if (p_ptr->lev < 45)
    {
        spell->level = 35;
        spell->cost = 40;
        spell->fail = calculate_fail_rate(35, 50, p_ptr->stat_ind[A_STR]);
        spell->fn = android_beam_cannon_spell;
    }
    else
    {
        spell->level = 45;
        spell->cost = 60;
        spell->fail = calculate_fail_rate(45, 70, p_ptr->stat_ind[A_STR]);
        spell->fn = android_rocket_spell;
    }
    return ct;
}
static void _android_calc_bonuses(void)
{
    int ac = 10 + (p_ptr->lev * 2 / 5);

    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;

    p_ptr->slow_digest = TRUE;
    p_ptr->free_act++;
    res_add(RES_POIS);
    /*res_add_vuln(RES_ELEC); cf resists.c res_pct_aux() for an alternative*/
    p_ptr->hold_life++;
}
static void _android_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_HOLD_LIFE);
    /*add_flag(flgs, TR_VULN_ELEC);*/
}
static void _android_birth(void)
{
    object_type forge = {0};
    object_prep(&forge, lookup_kind(TV_FLASK, SV_ANY));
    apply_magic(&forge, 1, AM_NO_FIXED_ART); /* Hack (pval->xtra4) */
    forge.number = rand_range(7, 12);
    py_birth_obj(&forge);

    py_birth_light();

    p_ptr->au /= 5;
}
race_t *android_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Android";
        me.desc = "An android is a artificial creation with a body of machinery. Over the millenia, artificial "
                    "intelligence has improved to the point where androids are nearly as smart as humans, though "
                    "perhaps not so wise. Of course, their mechanical body offers great physical advantages, far "
                    "surpassing the powers of man. Androids don't acquire experience like other "
                    "races, but rather gain in power as they attach new equipment to their frame. "
                    "Rings, amulets, and lights do not influence growth. Androids are resistant to "
                    "poison, can move freely, and are immune to life-draining attacks. Moreover, "
                    "because of their hard metallic bodies, they get a bonus to AC. Androids have "
                    "electronic circuits throughout their body and must beware of electric shocks. "
                    "They gain very little nutrition from the food of mortals, but they can use flasks "
                    "of oil as their energy source.";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  0;
        me.skills.dev = -3;
        me.skills.sav =  0;
        me.skills.stl = -2;
        me.skills.srh =  3;
        me.skills.fos = 14;
        me.skills.thn = 20;
        me.skills.thb =  6;

        me.life = 108;
        me.base_hp = 26;
        me.exp = 200;
        me.infra = 0;
        me.shop_adjust = 120;


        me.birth = _android_birth;
        me.calc_bonuses = _android_calc_bonuses;
        me.get_powers = _android_get_powers;
        me.get_flags = _android_get_flags;
        me.flags = RACE_IS_NONLIVING | RACE_NO_POLY;

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Archon
 ****************************************************************/
static void _archon_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->see_inv++;
    p_ptr->align += 200;
}
static void _archon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_SEE_INVIS);
}
race_t *archon_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Archon";
        me.desc = "Archons are a higher class of angels. They are good at all skills, and are strong, "
                    "wise, and are a favorite with any people. They are able to see the unseen, and "
                    "their wings allow them to safely fly over traps and other dangerous places. However, "
                    "belonging to a higher plane as they do, the experiences of this world do not leave "
                    "a strong impression on them and they gain levels slowly.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  4;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  3;

        me.skills.dis =  0;
        me.skills.dev =  8;
        me.skills.sav =  8;
        me.skills.stl =  2;
        me.skills.srh =  2;
        me.skills.fos = 11;
        me.skills.thn = 10;
        me.skills.thb =  7;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 200;
        me.infra = 3;
        me.shop_adjust = 90;

        me.calc_bonuses = _archon_calc_bonuses;
        me.get_flags = _archon_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Balrog
 ****************************************************************/
static power_info _balrog_powers[] =
{
    { A_CON, {15, 10, 70, demon_breath_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _balrog_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _balrog_powers);
}
static void _balrog_calc_bonuses(void)
{
    res_add(RES_FIRE);
    res_add(RES_NETHER);
    p_ptr->hold_life++;
    if (p_ptr->lev >= 10) p_ptr->see_inv++;
    if (p_ptr->lev >= 45) res_add(RES_FIRE);
    p_ptr->align -= 200;
}
static void _balrog_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
}
static void _balrog_birth(void)
{
    int i, ct = rand_range(3, 4);
    get_mon_num_prep(monster_hook_human, NULL);
    for (i = 0; i < ct; i++)
    {
        object_type forge = {0};
        object_prep(&forge, lookup_kind(TV_CORPSE, SV_CORPSE));
        forge.pval = get_mon_num(2);
        py_birth_obj(&forge);
    }
    py_birth_light();
}
race_t *balrog_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Balrog";
        me.desc = "Balrogs are a higher class of demons. They are strong, intelligent and tough. They do "
                    "not believe in gods, and are not suitable for priest at all. Balrog are resistant to "
                    "fire and nether, and have a firm hold on their life force. They also eventually learn "
                    "to see invisible things. They are good at almost all skills except stealth. They gain "
                    "very little nutrition from the food of mortals, and need human corpses as sacrifices "
                    "to regain their vitality.";

        me.stats[A_STR] =  4;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =-10;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  2;

        me.skills.dis = -3;
        me.skills.dev =  8;
        me.skills.sav = 15;
        me.skills.stl = -2;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn = 20;
        me.skills.thb =  0;

        me.life = 106;
        me.base_hp = 24;
        me.exp = 180;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_DEMON;
        me.shop_adjust = 140;

        me.birth = _balrog_birth;
        me.calc_bonuses = _balrog_calc_bonuses;
        me.get_powers = _balrog_get_powers;
        me.get_flags = _balrog_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Barbarian
 ****************************************************************/
static void _barbarian_gain_level(int new_level)
{
	if ((new_level >= 30) && (p_ptr->prace != RACE_DOPPELGANGER)) 
      /* bostock says doppel barbies are strong enough without a demigod power */
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static power_info _barbarian_powers[] =
{
    { A_STR, {8, 10, 30, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _barbarian_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _barbarian_powers);
}
static void _barbarian_calc_bonuses(void)
{
    res_add(RES_FEAR);
}
static void _barbarian_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FEAR);
}
race_t *barbarian_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Barbarian";
        me.desc = "Barbarians are hardy men of the north. They are fierce in combat, and their wrath "
                    "is feared throughout the world. Combat is their life: they feel no fear, and they "
                    "learn to enter battle frenzy at will even sooner than half-trolls. Barbarians are, "
                    "however, suspicious of magic, which makes magic devices fairly hard for them to use.";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;

        me.skills.dis = -2;
        me.skills.dev = -7;
        me.skills.sav = 2;
        me.skills.stl = -1;
        me.skills.srh = 1;
        me.skills.fos = 7;
        me.skills.thn = 12;
        me.skills.thb =  6;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 135;
        me.infra = 0;
        me.shop_adjust = 120;

        me.calc_bonuses = _barbarian_calc_bonuses;
        me.get_powers = _barbarian_get_powers;
        me.get_flags = _barbarian_get_flags;
		me.gain_level = _barbarian_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Beastman
 ****************************************************************/
static void _beastman_gain_level(int new_level)
{
    if (one_in_(5))
    {
        msg_print("You feel different...");
        mut_gain_random(NULL);
    }
}
static void _beastman_calc_bonuses(void)
{
    res_add(RES_CONF);
    res_add(RES_SOUND);
}
static void _beastman_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_CONF);
}
static void _beastman_birth(void)
{
    mut_gain_random(mut_good_pred);
    py_birth_food();
    py_birth_light();
}
race_t *beastman_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Beastman";
        me.desc = "This race is a blasphemous abomination produced by Chaos. It is not an independent "
                    "race but rather a humanoid creature, most often a human, twisted by the Chaos, "
                    "or a nightmarish crossbreed of a human and a beast. All Beastmen are accustomed "
                    "to Chaos so much that they are untroubled by confusion and sound, although raw "
                    "logrus can still have effects on them. Beastmen revel in chaos, as it twists them "
                    "more and more. Beastmen are subject to mutations: when they have been created, "
                    "they receive a random mutation. After that, every time they advance a level "
                    "they have a small chance of gaining yet another mutation.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = -5;
        me.skills.dev = -1;
        me.skills.sav = -1;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 5;
        me.skills.thn = 12;
        me.skills.thb = 3;

        me.life = 102;
        me.base_hp = 22;
        me.exp = 150;
        me.infra = 0;
        me.shop_adjust = 130;

        me.birth = _beastman_birth;
        me.calc_bonuses = _beastman_calc_bonuses;
        me.gain_level = _beastman_gain_level;
        me.get_flags = _beastman_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Boit
 ****************************************************************/

static void _boit_vomit_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vomit");
        break;
    case SPELL_DESC:
        var_set_string(res, "Ejects the contents of your stomach, making you extremely hungry and curing poison. Vomiting on an already empty stomach hurts a bit.");
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_AFRAID | PWR_CONFUSED);
        break;
    case SPELL_CAST:
        msg_print("You throw up!");
        if (p_ptr->food < PY_FOOD_FAINT + 24)
        {
            take_hit(DAMAGE_NOESCAPE, 10, "vomiting on an empty stomach");
            energy_use += 15;
        }
        set_food(MAX(1, MIN(p_ptr->food - 100, PY_FOOD_FAINT + 12)));
        fire_ball(GF_POIS, 0, p_ptr->poisoned * 2 / 7, 1);
        set_poisoned(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _boit_powers[] =
{
    { A_STR, {1, 0, 0, _boit_vomit_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _boit_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _boit_powers);
}
static void _boit_calc_bonuses(void)
{
    p_ptr->pspeed += 2;
    p_ptr->skill_tht += 25;
}
static void _boit_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
}
race_t *boit_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Boit";
        me.desc = "Boits are small bipedal creatures covered in yellow, lilac, brown or green "
                    "fur, distantly related to the Yeeks. Boits move speedily, and can eject "
                    "the contents of their stomachs at will, but they are not particularly "
                    "well-suited for melee, ranged combat, use of magical devices, or spellcasting.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] = -2;

        me.skills.dis = 2;
        me.skills.dev = -5;
        me.skills.sav = -1;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = -8;
        me.skills.thb = -8;

        me.life = 95;
        me.base_hp = 15;
        me.exp = 80;
        me.infra = 1;
        me.shop_adjust = 105;

        me.calc_bonuses = _boit_calc_bonuses;
        me.get_powers = _boit_get_powers;
        me.get_flags = _boit_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Centaur
 ****************************************************************/
static void _centaur_birth(void)
{
    equip_on_change_race();
    skills_innate_init("Hooves", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    py_birth_food();
    py_birth_light();
}

void jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Jump");
        break;
    case SPELL_DESC:
        var_set_string(res, "Leap a short distance, clearing any intervening monsters or obstacles.");
        break;
    case SPELL_CAST:
    {
        int x, y;
        int len = 2 + p_ptr->lev/35;

        var_set_bool(res, FALSE);

        if (!tgt_pt(&x, &y, len)) return;

        if (distance(y, x, py, px) > len)
        {
            msg_print("You can't jump that far.");
            return;
        }
        if (!los(py, px, y, x))
        {
            msg_print("You can't see that location.");
            return;
        }
        if (!cave_player_teleportable_bold(y, x, 0L))
        {
            msg_print("You can't leap there!");
            return;
        }
        teleport_player_to(y, x, 0L);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _centaur_powers[] =
{
    { A_DEX, {15, 10, 50, jump_spell}},
    { -1, {-1, -1, -1, NULL} }
};

static int _centaur_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _centaur_powers);
}

static void _centaur_calc_bonuses(void)
{
    int slot = equip_find_first(object_is_body_armour);
    p_ptr->pspeed += p_ptr->lev / 10;

    if (slot)
    {
        object_type *o_ptr = equip_obj(slot);
        p_ptr->to_a -= o_ptr->ac / 3;
        p_ptr->dis_to_a -= o_ptr->ac / 3;

        if (o_ptr->to_a > 0)
        {
            p_ptr->to_a -= o_ptr->to_a / 3;
            p_ptr->dis_to_a -= o_ptr->to_a / 3;
        }
    }
}

static void _centaur_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SPEED);
}

static void _centaur_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_d = py_prorata_level(15);
    int to_h = l/2;
    innate_attack_t    a = {0};

    a.dd = 1 + l / 16;
    a.ds = 4 + l / 21;
    a.to_d += to_d;
    a.to_h += to_h;

    a.weight = 150;
    calc_innate_blows(&a, 200);
    a.msg = "You kick.";
    a.name = "Hooves";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

race_t *centaur_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Centaur";
        me.desc = "Centaurs are creatures with the head, arms and torso of a human combined with the "
                    "body and legs of a horse. As such, they are able to move more quickly as "
                    "they gain experience and are capable of leaping great distances. They may "
                    "attack monsters with their hooves in addition to any normal melee weapons. "
                    "Centaurs are strong, agile, and wise, but not so smart. "
                    "They are skilled in fighting and archery but are rather distrustful of magic. "
                    "Finally, being at home in the forests of the world, Centaurs "
                    "are able to move quickly through foliage.";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;

        me.skills.dis =  0;
        me.skills.dev = -3;
        me.skills.sav =  2;
        me.skills.stl =  1;
        me.skills.srh =  3;
        me.skills.fos =  5;
        me.skills.thn = 10;
        me.skills.thb =  8;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 190;
        me.infra = 0;
        me.shop_adjust = 95;

        me.birth = _centaur_birth;
        me.calc_innate_attacks = _centaur_calc_innate_attacks;
        me.get_powers = _centaur_get_powers;
        me.calc_bonuses = _centaur_calc_bonuses;
        me.get_flags = _centaur_get_flags;

        me.equip_template = &b_info[46];

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Cyclops
 ****************************************************************/
static power_info _cyclops_powers[] =
{
    { A_STR, {20, 0, 50, throw_boulder_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _cyclops_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _cyclops_powers);
}
static void _cyclops_calc_bonuses(void)
{
    res_add(RES_SOUND);
}
static void _cyclops_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SOUND);
}
race_t *cyclops_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Cyclops";
        me.desc = "With but one eye, a Cyclops can see more than many with two eyes. They are "
                    "headstrong, and loud noises bother them very little. They are not quite "
                    "qualified for the magic using professions, but as a certain Mr. Ulysses "
                    "can testify, their accuracy with thrown rocks can be deadly!";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] = -1;

        me.skills.dis = -4;
        me.skills.dev = -3;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -2;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb = 10;

        me.life = 108;
        me.base_hp = 24;
        me.exp = 155;
        me.infra = 1;
        me.shop_adjust = 135;

        me.calc_bonuses = _cyclops_calc_bonuses;
        me.get_powers = _cyclops_get_powers;
        me.get_flags = _cyclops_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Dark-Elf
 ****************************************************************/
static power_info _dark_elf_powers[] =
{
    { A_INT, {1, 2, 30, magic_missile_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _dark_elf_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _dark_elf_powers);
}
static void _dark_elf_calc_bonuses(void)
{
    res_add(RES_DARK);
    p_ptr->spell_cap += 3;
    if (p_ptr->lev >= 20) p_ptr->see_inv++;
}
static void _dark_elf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_SPELL_CAP);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_SEE_INVIS);
}
race_t *dark_elf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Dark-Elf";
        me.desc = "Another dark, cave-dwelling race, likewise unhampered by darkness attacks, "
                    "the Dark Elves have a long tradition and knowledge of magic. They have an "
                    "inherent magic missile attack available to them at a low level. With their "
                    "keen sight, they also learn to see invisible things as their relatives "
                    "High-Elves do, but at a higher level.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] =  3;

        me.skills.dis = 5;
        me.skills.dev = 7;
        me.skills.sav = 12;
        me.skills.stl = 3;
        me.skills.srh = 8;
        me.skills.fos = 12;
        me.skills.thn = -5;
        me.skills.thb =  6;

        me.life = 97;
        me.base_hp = 18;
        me.exp = 155;
        me.infra = 5;
        me.shop_adjust = 120;

        me.calc_bonuses = _dark_elf_calc_bonuses;
        me.get_powers = _dark_elf_get_powers;
        me.get_flags = _dark_elf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Draconian
 ****************************************************************/
static int _draconian_breath_amount(void)
{
    int l = p_ptr->lev;
    int amt = 0;

    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
    case DRACONIAN_WHITE:
    case DRACONIAN_BLUE:
    case DRACONIAN_BLACK:
    case DRACONIAN_GREEN:
        amt = MIN(500, p_ptr->chp * (25 + l*l*l/2500) / 100);
        break;

    case DRACONIAN_SHADOW:
        amt = MIN(400, p_ptr->chp * (20 + l*l*l*35/125000) / 100);
        break;

    case DRACONIAN_CRYSTAL:
    case DRACONIAN_GOLD:
        amt = MIN(350, p_ptr->chp * (20 + l*l*l*30/125000) / 100);
        break;

    case DRACONIAN_BRONZE:
        amt = MIN(300, p_ptr->chp * (17 + l*l*l*25/125000) / 100);
        break;
    }

    if (!mut_present(MUT_DRACONIAN_BREATH))
        amt /= 2;

    return MAX(amt, 1);
}

static int _draconian_breath_cost(void)
{
    int l = p_ptr->lev;
    int cost = l/2 + l*l*15/2500;
    if (!mut_present(MUT_DRACONIAN_BREATH))
        cost = cost * 2 / 3;
    return MAX(cost, 1);
}

static cptr _draconian_breath_desc(void)
{
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED: return "fire";
    case DRACONIAN_WHITE: return "cold";
    case DRACONIAN_BLUE: return "lightning";
    case DRACONIAN_BLACK: return "acid";
    case DRACONIAN_GREEN: return "poison";
    case DRACONIAN_CRYSTAL: return "shards";
    case DRACONIAN_BRONZE: return "confusion";
    case DRACONIAN_GOLD: return "sound";
    case DRACONIAN_SHADOW: return "nether";
    }
    return 0;
}

static int _draconian_breath_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED: return GF_FIRE;
    case DRACONIAN_WHITE: return GF_COLD;
    case DRACONIAN_BLUE: return GF_ELEC;
    case DRACONIAN_BLACK: return GF_ACID;
    case DRACONIAN_GREEN: return GF_POIS;
    case DRACONIAN_BRONZE: return GF_CONFUSION;
    case DRACONIAN_GOLD: return GF_SOUND;
    case DRACONIAN_SHADOW: return GF_NETHER;
    case DRACONIAN_CRYSTAL: return GF_SHARDS;
    }
    return 0;
}

static void _draconian_do_breathe(int effect, int dir, int dam)
{
    /* Dragon breath changes shape with maturity */
    if (p_ptr->lev < 20)
        fire_bolt(effect, dir, dam);
    else if (p_ptr->lev < 30)
        fire_beam(effect, dir, dam);
    else
        fire_ball(effect, dir, dam, -1 - (p_ptr->lev / 20));
}

static void _draconian_breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent.", _draconian_breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _draconian_breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _draconian_breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _draconian_breath_effect();
            int dam = _draconian_breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s.", gf_name(e));
            _draconian_do_breathe(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _draconian_powers[] =
{
    { A_CON, {1, 0, 70, _draconian_breathe_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _draconian_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _draconian_powers);
}
static void _draconian_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
        res_add(RES_FIRE);
        break;
    case DRACONIAN_WHITE:
        res_add(RES_COLD);
        break;
    case DRACONIAN_BLUE:
        res_add(RES_ELEC);
        break;
    case DRACONIAN_BLACK:
        res_add(RES_ACID);
        break;
    case DRACONIAN_GREEN:
        res_add(RES_POIS);
        break;
    case DRACONIAN_BRONZE:
        res_add(RES_CONF);
        break;
    case DRACONIAN_CRYSTAL:
        res_add(RES_SHARDS);
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        if (p_ptr->lev >= 40)
            p_ptr->reflect = TRUE;
        break;
    case DRACONIAN_GOLD:
        res_add(RES_SOUND);
        break;
    case DRACONIAN_SHADOW:
        res_add(RES_NETHER);
        break;
    }
    if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        int l = p_ptr->lev;
        int to_a = py_prorata_level(75);
        int ac = 15 + (l/10)*5;

        p_ptr->ac += ac;
        p_ptr->dis_ac += ac;

        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
    }
}
static void _draconian_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
        add_flag(flgs, OF_RES_FIRE);
        break;
    case DRACONIAN_WHITE:
        add_flag(flgs, OF_RES_COLD);
        break;
    case DRACONIAN_BLUE:
        add_flag(flgs, OF_RES_ELEC);
        break;
    case DRACONIAN_BLACK:
        add_flag(flgs, OF_RES_ACID);
        break;
    case DRACONIAN_GREEN:
        add_flag(flgs, OF_RES_POIS);
        break;
    case DRACONIAN_BRONZE:
        add_flag(flgs, OF_RES_CONF);
        break;
    case DRACONIAN_CRYSTAL:
        add_flag(flgs, OF_RES_SHARDS);
        if (p_ptr->lev >= 40)
            add_flag(flgs, OF_REFLECT);
        break;
    case DRACONIAN_GOLD:
        add_flag(flgs, OF_RES_SOUND);
        break;
    case DRACONIAN_SHADOW:
        add_flag(flgs, OF_RES_NETHER);
        break;
    }
}
/* cf design/dragons.ods */
static int _draconian_attack_level(void)
{
    int l = p_ptr->lev * 2;
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
    case DRACONIAN_WHITE:
        l = MAX(1, l * 105 / 100);
        break;

    case DRACONIAN_BLACK:
    case DRACONIAN_GREEN:
        break;

    case DRACONIAN_BLUE:
        l = MAX(1, l * 95 / 100);
        break;

    case DRACONIAN_CRYSTAL:
    case DRACONIAN_BRONZE:
    case DRACONIAN_GOLD:
        l = MAX(1, l * 90 / 100);
        break;

    case DRACONIAN_SHADOW:
        l = MAX(1, l * 85 / 100);
        break;
    }

    switch (p_ptr->pclass)
    {
    case CLASS_BERSERKER:
        l = MAX(1, l * 170 / 100);
        break;
    case CLASS_WARRIOR:
    case CLASS_MONK:
    case CLASS_BLOOD_KNIGHT:
        l = MAX(1, l * 120 / 100);
        break;
    case CLASS_PALADIN:
    case CLASS_CHAOS_WARRIOR:
        l = MAX(1, l * 110 / 100);
        break;
    case CLASS_RED_MAGE:
    case CLASS_WEAPONSMITH:
    case CLASS_ROGUE:
    case CLASS_ALCHEMIST:
        l = MAX(1, l * 105 / 100);
        break;
    case CLASS_PRIEST:
    case CLASS_MINDCRAFTER:
    case CLASS_MAGIC_EATER:
    case CLASS_ARCHAEOLOGIST:
    case CLASS_LAWYER:
    case CLASS_WILD_TALENT:
    case CLASS_PSION:
    case CLASS_SCOUT:
    case CLASS_DEVICEMASTER:
    case CLASS_FORCETRAINER:
    case CLASS_SKILLMASTER:
    case CLASS_DISCIPLE:
        /*l = MAX(1, l * 100 / 100);*/
        break;
    case CLASS_BARD:
    case CLASS_TIME_LORD:
    case CLASS_WARLOCK:
    case CLASS_POLITICIAN:
    case CLASS_RAGE_MAGE:
        l = MAX(1, l * 90 / 100);
        break;
    case CLASS_NINJA:
    case CLASS_NINJA_LAWYER:
    case CLASS_MAGE:
    case CLASS_HIGH_MAGE:
    case CLASS_TOURIST:
    case CLASS_MIRROR_MASTER:
    case CLASS_BLOOD_MAGE:
    case CLASS_YELLOW_MAGE:
    case CLASS_GRAY_MAGE:
        l = MAX(1, l * 80 / 100);
        break;
    case CLASS_SORCERER:
        l = MAX(1, l * 50 / 100);
        break;
    }

    return MAX(1, l);
}
static void _draconian_calc_innate_attacks(void)
{
    int l = _draconian_attack_level();
    int l2 = p_ptr->lev; /* Note: Using attack_level() for both dd and ds gives too much variation */
    int to_d = 0;
    int to_h = l2*3/5;

    if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
    {
        /* These monk postures get "1 try" when selecting a monk attack.
           Translate this into a lower attack rating. */
        if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
            l = (l + 2) / 3;
        /* Conversely, the White Tiger gets more "tries" for martial arts,
           so needs a better power rating */
        else if (p_ptr->special_defense & KAMAE_BYAKKO)
            l = l * 5 / 4;
    }

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

        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
        {
            a.effect[1] = GF_STUN;
            a.effect_chance[1] = 15 + l/4;

            if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
                a.blows = MAX(100, a.blows - 200);
        }

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

        if (l >= 175) /* White Berserker Only */
            calc_innate_blows(&a, 400);
        else if (l >= 160) /* Berserker Only */
            calc_innate_blows(&a, 300);
        else if (l >= 135) /* Berserker Only */
            calc_innate_blows(&a, 250);
        else if (l >= 85)  /* CL50 for a Shadow Priest */
            calc_innate_blows(&a, 200);
        else if (l >= 70) /* CL45 for a White Mage (Max Rating = 84) */
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite.";
        a.name = "Bite";

        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
        {
            if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
                a.blows = MAX(100, a.blows - 100);
        }

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}
static void _draconian_gain_power(void)
{
    if (p_ptr->draconian_power < 0)
    {
        int idx = mut_gain_choice(mut_draconian_pred);
        mut_lock(idx);
        p_ptr->draconian_power = idx;
        if (idx == MUT_DRACONIAN_METAMORPHOSIS)
        {
            msg_print("You are transformed into a dragon!");
            equip_on_change_race();
        }
    }
    else if (!mut_present(p_ptr->draconian_power))
    {
        mut_gain(p_ptr->draconian_power);
        mut_lock(p_ptr->draconian_power);
        if (p_ptr->draconian_power == MUT_DRACONIAN_METAMORPHOSIS)
            equip_on_change_race();
    }
}
static void _draconian_gain_level(int new_level)
{
    if (new_level >= 35)
        _draconian_gain_power();
}
race_t *draconian_get_race(int psubrace)
{
    static race_t me = {0};
    static bool init = FALSE;
    static int subrace_init = -1;

    if (!init)
    {
        me.name = "Draconian";
        me.desc = "Draconians are a humanoid race with dragon-like attributes. There are several "
                    "subtypes of draconians with different resistances, breaths and attributes. "
                    "For example, Red Draconians are resistant to fire which they may also breathe "
                    "at will, while White Draconians breathe and resist cold instead. All draconians "
                    "levitate. In addition, when they mature enough, they may choose a special "
                    "draconian power.";

        me.base_hp = 22;

        me.calc_bonuses = _draconian_calc_bonuses;
        me.get_powers = _draconian_get_powers;
        me.get_flags = _draconian_get_flags;
        me.gain_level = _draconian_gain_level;

        init = TRUE;
    }

    if (subrace_init != psubrace)
    {
        /* Reset to baseline */
        me.subname = NULL;
        me.subdesc = NULL;
        me.stats[A_STR] =  1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;

        me.skills.dis = -2;
        me.skills.dev = 1;
        me.skills.sav = 2;
        me.skills.stl = 1;
        me.skills.srh = 1;
        me.skills.fos = 10;
        me.skills.thn = 5;
        me.skills.thb = 1;

        me.infra = 2;

        me.exp = 190;
        me.life = 101;
        me.shop_adjust = 105;


        /* Override with New Type */
        switch (psubrace)
        {
        case DRACONIAN_RED:
            me.subname = "Red";
            me.subdesc = "Red Draconians have an affinity for fire, which they both breathe at will and resist. "
                         "Together with their White kin, they are the strongest in combat of the draconians. "
                         "But they are not so good with magic and their stealth is quite poor. Should they choose "
                         "the power of Dragon Skin, they will gain a fiery aura as well. Should they choose the "
                         "power of Dragon Strike, their blows will burn their enemies.";
            me.stats[A_STR] += 2;
            me.stats[A_INT] -= 1;
            me.stats[A_WIS] -= 1;
            me.skills.dev -= 3;
            me.skills.stl -= 2;
            me.skills.thn += 10;
            me.life += 3;
            me.shop_adjust = 115;
            break;
        case DRACONIAN_WHITE:
            me.subname = "White";
            me.subdesc = "White Draconians have an affinity for frost, which they both breathe at will and resist. "
                         "Together with their Red kin, they are the strongest in combat of the draconians. "
                         "But they are not so good with magic and their stealth is quite poor. Should they choose "
                         "the power of Dragon Skin, they will gain an aura of cold as well. Should they choose the "
                         "power of Dragon Strike, their blows will freeze their enemies.";
            me.stats[A_STR] += 2;
            me.stats[A_INT] -= 1;
            me.stats[A_WIS] -= 1;
            me.skills.dev -= 3;
            me.skills.stl -= 2;
            me.skills.thn += 9;
            me.life += 3;
            me.shop_adjust = 115;
            break;
        case DRACONIAN_BLUE:
            me.subname = "Blue";
            me.subdesc = "Blue Draconians have an affinity for lightning, which they both breathe at will "
                         "and resist. They are strong in combat but not so good with magic or stealth. "
                         "Should they choose the power of Dragon Skin, they will gain a shocking aura as well. "
                         "Should they choose the power of Dragon Strike, their blows will electrocute "
                         "their enemies.";
            me.stats[A_STR] += 1;
            me.skills.dev -= 2;
            me.skills.stl -= 1;
            me.skills.thn += 7;
            me.life += 2;
            me.shop_adjust = 110;
            break;
        case DRACONIAN_BLACK:
            me.subname = "Black";
            me.subdesc = "Black Draconians have an affinity for acid, which they both breathe at will "
                         "and resist. They are strong in combat but not so good with magic or stealth. "
                         "With the power of Dragon Strike, their blows will corrode their enemies.";
            me.stats[A_STR] += 1;
            me.skills.dev -= 2;
            me.skills.stl -= 1;
            me.skills.thn += 8;
            me.life += 2;
            me.shop_adjust = 110;
            break;
        case DRACONIAN_GREEN:
            me.subname = "Green";
            me.subdesc = "Green Draconians have an affinity for poison, which they both breathe at will "
                         "and resist. They are average in all respects among the draconians. With the "
                         "power of Dragon Strike, their blows will poison their enemies.";
            me.exp += 15;
            break;
        case DRACONIAN_BRONZE:
            me.subname = "Bronze";
            me.subdesc = "Bronze Draconians are the most intelligent of their kind, and the best with "
                         "magic as well. They are seldom confused, though the same may not be said of "
                         "their enemies. With the power of Dragon Strike, even the melee attacks of "
                         "the Bronze Draconian will baffle their enemies.";
            me.stats[A_INT] += 1;
            me.skills.sav += 1;
            me.skills.thn -= 2;
            me.skills.dev += 5;
            me.exp += 25;
            me.shop_adjust = 100;
            break;
        case DRACONIAN_CRYSTAL:
            me.subname = "Crystal";
            me.subdesc = "Hard of skin, the Crystal Draconian is difficult to hit in melee. But their agility "
                         "suffers and they are not the brightest of their kind. They resist shards, which they "
                         "may also breathe on command. With the power of Dragon Skin, they gain an aura of "
                         "shards as well. With the power of Dragon Strike, even their melee attacks will shred "
                         "their enemies.";
            me.stats[A_INT] -= 2;
            me.stats[A_DEX] -= 1;
            me.stats[A_CON] += 1;
            me.skills.dev -= 3;
            me.skills.stl -= 1;
            me.skills.thn += 7;
            me.life += 2;
            me.exp += 60;
            break;
        case DRACONIAN_GOLD:
            me.subname = "Gold";
            me.subdesc = "The wisest of their kind, Gold Draconians are resilient in the face of magical "
                         "attacks. They are resistant to sound which they may also breathe at will, stunning "
                         "their enemies. With the power of Dragon Strike, even their melee attacks will "
                         "stun their enemies.";
            me.stats[A_WIS] += 1;
            me.skills.dev += 3;
            me.skills.sav += 3;
            me.life += 1;
            me.exp += 30;
            me.shop_adjust = 95;
            break;
        case DRACONIAN_SHADOW:
            me.subname = "Shadow";
            me.subdesc = "Lithe, stealthy and nimble, the Shadow Draconian is seldom seen in this world. "
                         "They are resistant to the forces of nether which they may also breathe. They are the "
                         "weakest of the draconians, and the poorest in melee. But they are better than average "
                         "with magic. With the power of Dragon Strike, they may steal life from their enemies "
                         "in melee.";
            me.stats[A_STR] -= 1;
            me.stats[A_DEX] += 2;
            me.skills.dev += 2;
            me.skills.stl += 3;
            me.skills.thn -= 5;
            me.life -= 1;
            me.exp += 35;
            me.infra += 2;
            break;
        }
        subrace_init = psubrace;
    }
    me.equip_template = NULL;
    me.calc_innate_attacks = NULL;
    if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        me.equip_template = &b_info[20];
        me.calc_innate_attacks = _draconian_calc_innate_attacks;
    }
    return &me;
}

/****************************************************************
 * Dunadan
 ****************************************************************/
static void _dunadan_gain_level(int new_level)
{
	if ((new_level >= 30) && (p_ptr->prace != RACE_DOPPELGANGER))
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static void _dunadan_calc_bonuses(void)
{
    p_ptr->sustain_con = TRUE;
}
static void _dunadan_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_CON);
}
race_t *dunadan_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Dunadan";
        me.desc = "Dunedain are a race of hardy men from the West. This elder race surpasses human "
                    "abilities in every field, especially constitution. However, being men of the world, "
                    "very little is new to them, and levels are very hard for them to gain. Their "
                    "constitution cannot be reduced. ";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  4;
        me.skills.dev =  3;
        me.skills.sav =  3;
        me.skills.stl =  2;
        me.skills.srh =  3;
        me.skills.fos = 13;
        me.skills.thn = 15;
        me.skills.thb =  7;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 160;
        me.infra = 0;
        me.shop_adjust = 100;

        me.calc_bonuses = _dunadan_calc_bonuses;
        me.get_flags = _dunadan_get_flags;

		me.gain_level = _dunadan_gain_level;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Dwarf
 ****************************************************************/
static power_info _dwarf_powers[] =
{
    { A_WIS, {5, 5, 50, detect_doors_stairs_traps_spell}},
    { A_CHR, {10, 5, 50, detect_treasure_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _dwarf_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _dwarf_powers);
}
static void _dwarf_calc_bonuses(void)
{
    res_add(RES_BLIND);
}
static void _dwarf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_BLIND);
}
race_t *dwarf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Dwarf";
        me.desc = "Dwarves are the headstrong miners and fighters of legend. Dwarves tend to be stronger "
                    "and tougher but slower and less intelligent than humans. Because they are so headstrong "
                    "and are somewhat wise, they resist spells which are cast on them. They are very good "
                    "at searching, perception, fighting, and bows. Dwarves have miserable stealth. They "
                    "resist being blinded.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = 2;
        me.skills.dev = 5;
        me.skills.sav = 6;
        me.skills.stl = -1;
        me.skills.srh = 7;
        me.skills.fos = 10;
        me.skills.thn = 15;
        me.skills.thb = 0;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 135;
        me.infra = 5;
        me.shop_adjust = 115;

        me.calc_bonuses = _dwarf_calc_bonuses;
        me.get_powers = _dwarf_get_powers;
        me.get_flags = _dwarf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Einheri
 ****************************************************************/
static power_info _einheri_powers[] =
{
    { A_STR, {1, 10, 50, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _einheri_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _einheri_powers);
}

static void _einheri_gain_level(int new_level)
{
	if (new_level >= 30)
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}

static void _einheri_calc_bonuses(void)
{
    p_ptr->hold_life++;
    p_ptr->regen += 100;
}
static void _einheri_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_REGEN);
}
race_t *einheri_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Einheri";
        me.desc = "Dead heroes returning to the mortal world for one last fight, Einherjar are highly "
                    "skilled in hand-to-hand combat and start with the ability to enter a battle frenzy; "
                    "however, they are also passably good at using devices, and do not necessarily make "
                    "weak magic users. Already having one lifetime's adventures behind them, Einherjar are "
                    "very strong at the beginning but learn new things slowly. Like other undead races, they "
                    "are immune to life draining. Einherjar have one serious weakness: magical healing only "
                    "has a partial effect on them, healing them half as much as it would any other race.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;

        me.skills.dis = 5;
        me.skills.dev = 3;
        me.skills.sav = 8;
        me.skills.stl = -1;
        me.skills.srh = 7;
        me.skills.fos = 10;
        me.skills.thn = 22;
        me.skills.thb = 8;

        me.life = 113;
        me.base_hp = 44;
        me.exp = 160;
        me.infra = 3;
        me.shop_adjust = 100;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD;
        me.calc_bonuses = _einheri_calc_bonuses;
        me.get_powers = _einheri_get_powers;
        me.get_flags = _einheri_get_flags;
        me.gain_level = _einheri_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Ent
 ****************************************************************/
static power_info _ent_powers[] =
{
    { A_WIS, {10, 20, 70, summon_tree_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _ent_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _ent_powers);
}
static void _ent_calc_bonuses(void)
{
    /*res_add_vuln(RES_FIRE); cf resists.c res_pct_aux() for an alternative*/
    if (!equip_find_first(object_is_melee_weapon))
        p_ptr->skill_dig += p_ptr->lev * 10;
}
static void _ent_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    /*add_flag(flgs, TR_VULN_FIRE);*/
}
static void _ent_birth(void)
{
    py_birth_obj_aux(TV_POTION, SV_POTION_WATER, rand_range(15, 23));
    py_birth_light();
}
race_t *ent_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Ent";
        me.desc = "The Ents are a powerful race dating from the beginning of the world, oldest of all "
                    "animals or plants who inhabit Arda. Spirits of the land, they were summoned to "
                    "guard the forests of Middle-earth. Being much like trees they are very clumsy but "
                    "strong, and very susceptible to fire. They gain very little nutrition from the food "
                    "of mortals, but they can absorb water from potions as their nutrition. Finally, they "
                    "can summon the trees of the forest to their side.";

        me.skills.dis = -5;
        me.skills.dev =  1;
        me.skills.sav =  5;
        me.skills.stl = -1;
        me.skills.srh =  0;
        me.skills.fos =  9;
        me.skills.thn = 15;
        me.skills.thb = -3;

        me.life = 105;
        me.base_hp = 25;
        me.exp = 135;
        me.infra = 0;
        me.shop_adjust = 95;

        me.birth = _ent_birth;
        me.calc_bonuses = _ent_calc_bonuses;
        me.get_powers = _ent_get_powers;
        me.get_flags = _ent_get_flags;
        init = TRUE;
    }

    /* Since Ent racial stat bonuses are level dependent, we recalculate.
       Note, this prevents hackery in files.c for displaying racial stat bonuses correctly.
    */
    {
        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;

        if (!spoiler_hack) /* Otherwise, I need to be careful when generating automatic spoiler files! */
        {
            int amount = 0;
            if (p_ptr->lev >= 26) amount++;
            if (p_ptr->lev >= 41) amount++;
            if (p_ptr->lev >= 46) amount++;
            me.stats[A_STR] += amount;
            me.stats[A_DEX] -= amount;
            me.stats[A_CON] += amount;
        }
    }
    return &me;
}

/****************************************************************
 * Gnome
 ****************************************************************/
static power_info _gnome_powers[] =
{
    { A_INT, {5, 2, 50, phase_door_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _gnome_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _gnome_powers);
}
static void _gnome_calc_bonuses(void)
{
    p_ptr->free_act++;
}
static void _gnome_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_FREE_ACT);
}
race_t *gnome_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Gnome";
        me.desc = "Gnomes are smaller than dwarves but larger than Halflings. They, like the hobbits, "
                    "live in the earth in burrow-like homes. Gnomes make excellent mages, and have very "
                    "good saving throws. They are good at searching, disarming, perception, and stealth. "
                    "They have lower strength than humans so they are not very good at fighting with hand "
                    "weapons. Gnomes have fair infra-vision, so they can detect warm-blooded creatures "
                    "at a distance. Gnomes are intrinsically protected against paralysis.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -1;

        me.skills.dis = 10;
        me.skills.dev = 6;
        me.skills.sav = 7;
        me.skills.stl = 3;
        me.skills.srh = 6;
        me.skills.fos = 13;
        me.skills.thn = -8;
        me.skills.thb =  8;

        me.life = 95;
        me.base_hp = 16;
        me.exp = 115;
        me.infra = 4;
        me.shop_adjust = 115;

        me.calc_bonuses = _gnome_calc_bonuses;
        me.get_powers = _gnome_get_powers;
        me.get_flags = _gnome_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Golem
 ****************************************************************/
static power_info _golem_powers[] =
{
    { A_CON, {20, 20, 50, stone_skin_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _golem_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _golem_powers);
}
static void _golem_calc_bonuses(void)
{
    int ac = 10 + (p_ptr->lev * 2 / 5);
    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;
    p_ptr->no_stun = TRUE;

    p_ptr->slow_digest = TRUE;
    p_ptr->free_act++;
    p_ptr->see_inv++;
    res_add(RES_POIS);
    if (p_ptr->lev >= 35) p_ptr->hold_life++;

    p_ptr->pspeed -= p_ptr->lev/16;
}
static void _golem_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    if (p_ptr->lev >= 35)
        add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 16)
        add_flag(flgs, OF_DEC_SPEED);
}
static void _golem_birth(void)
{
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}
race_t *golem_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Golem";
        me.desc = "A Golem is an artificial creature, built from a lifeless raw material like clay, "
                    "and awakened to life. They are nearly mindless, making them useless for "
                    "professions which rely on magic, but as warriors they are very tough. They "
                    "are resistant to poison, they can see invisible things, and move freely. "
                    "At higher levels, they also become resistant to attacks which threaten to "
                    "drain away their life force. Golems gain very little nutrition from ordinary "
                    "food, but can absorb mana from staves and wands as their power source. Golems "
                    "also gain a natural armor class bonus from their tough body. Golems become "
                    "slower with age.";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -5;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] =  0;

        me.skills.dis = -5;
        me.skills.dev = -5;
        me.skills.sav = 6;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 8;
        me.skills.thn = 20;
        me.skills.thb = 0;

        me.life = 105;
        me.base_hp = 23;
        me.exp = 185;
        me.infra = 4;
        me.flags = RACE_IS_NONLIVING;
        me.shop_adjust = 120;

        me.birth = _golem_birth;
        me.get_powers = _golem_get_powers;
        me.calc_bonuses = _golem_calc_bonuses;
        me.get_flags = _golem_get_flags;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Half-Giant
 ****************************************************************/
static power_info _half_giant_powers[] =
{
    { A_STR, {20, 10, 70, stone_to_mud_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _half_giant_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _half_giant_powers);
}
static void _half_giant_calc_bonuses(void)
{
    p_ptr->sustain_str = TRUE;
    res_add(RES_SHARDS);
}
static void _half_giant_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_SUST_STR);
}
race_t *half_giant_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Half-Giant";
		me.desc = "Half-Giants' limited intelligence makes it difficult for them to become full spellcasters, "
					"but with their huge strength they make excellent warriors. Their thick skin makes "
					"them resistant to shards, and like Half-Trolls, they have their strength sustained.";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis = -6;
        me.skills.dev = -5;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 25;
        me.skills.thb =  0;

        me.life = 108;
        me.base_hp = 26;
        me.exp = 150;
        me.infra = 3;
        me.shop_adjust = 125;

        me.calc_bonuses = _half_giant_calc_bonuses;
        me.get_powers = _half_giant_get_powers;
        me.get_flags = _half_giant_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Half-Ogre
 ****************************************************************/
static power_info _half_ogre_powers[] =
{
    { A_INT, {25, 35, 70, explosive_rune_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _half_ogre_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _half_ogre_powers);
}
static void _half_ogre_calc_bonuses(void)
{
    p_ptr->sustain_int = TRUE;
}
static void _half_ogre_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_INT);
}
race_t *half_ogre_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Ogre";
        me.desc = "Ogres are big and ugly but possessed of a low cunning and great strength. "
                    "For warriors, they have all the necessary attributes, and they can even "
                    "become wizards: after all, they are related to Ogre Magi, from whom they "
                    "have learned the skill of setting trapped runes once their level is high "
                    "enough. Being simple minded they have their intelligence sustained";

        me.stats[A_STR] =  3;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis = -3;
        me.skills.dev = -3;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb =  -5;

        me.life = 106;
        me.base_hp = 23;
        me.exp = 140;
        me.infra = 0;
        me.shop_adjust = 125;

        me.calc_bonuses = _half_ogre_calc_bonuses;
        me.get_powers = _half_ogre_get_powers;
        me.get_flags = _half_ogre_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
* Half-Orc
****************************************************************/

static void _half_orc_gain_level(int new_level)
{
	if (new_level >= 30)
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static void _half_orc_calc_bonuses(void)
{
	res_add(RES_DARK);
}
static void _half_orc_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
	add_flag(flgs, OF_RES_DARK);
}
race_t *half_orc_get_race(void)
{
	static race_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{
		me.name = "Half-Orc";
		me.desc = "Half-orcs make excellent warriors, but are terrible at magic. "
			"They are as bad as dwarves at stealth, and horrible at searching, "
			"disarming, and perception. Half-orcs are quite ugly, and tend to "
			"pay more for goods in town. Because of their preference to living "
			"underground to on the surface, half-orcs resist darkness attacks. "
			"The human part of their heritage allows them to select a talent at "
			"Level 30.";

		me.stats[A_STR] = 2;
		me.stats[A_INT] = -1;
		me.stats[A_WIS] = 0;
		me.stats[A_DEX] = 0;
		me.stats[A_CON] = 1;
		me.stats[A_CHR] = -1;

		me.skills.dis = -3;
		me.skills.dev = -3;
		me.skills.sav = -1;
		me.skills.stl = -2;
		me.skills.srh = -1;
		me.skills.fos = 5;
		me.skills.thn = 20;
		me.skills.thb = -5;

		me.life = 103;
		me.base_hp = 20;
		me.exp = 110;
		me.infra = 3;
		me.shop_adjust = 120;

		me.calc_bonuses = _half_orc_calc_bonuses;
		me.get_flags = _half_orc_get_flags;

		me.gain_level = _half_orc_gain_level;
		init = TRUE;
	}

	return &me;
}

/****************************************************************
 * Half-Titan
 ****************************************************************/
static power_info _half_titan_powers[] =
{
    { A_INT, {15, 10, 60, probing_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _half_titan_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _half_titan_powers);
}
static void _half_titan_calc_bonuses(void)
{
    res_add(RES_CHAOS);
}
static void _half_titan_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_CHAOS);
}
race_t *half_titan_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Half-Titan";
        me.desc = "Half-mortal descendants of the mighty titans, these immensely powerful creatures "
                    "put almost any other race to shame. They may lack the fascinating special powers "
                    "of certain other races, but their enhanced attributes more than make up for that. "
                    "They learn to estimate the strengths of their foes, and their love for law and "
                    "order makes them resistant to the effects of Chaos.";

        me.stats[A_STR] =  5;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  3;

        me.skills.dis = -5;
        me.skills.dev =  3;
        me.skills.sav =  1;
        me.skills.stl = -2;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn = 25;
        me.skills.thb =  0;

        me.life = 110;
        me.base_hp = 28;
        me.exp = 200;
        me.infra = 0;
        me.shop_adjust = 90;

        me.calc_bonuses = _half_titan_calc_bonuses;
        me.get_powers = _half_titan_get_powers;
        me.get_flags = _half_titan_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Half-Troll
 ****************************************************************/
static power_info _half_troll_powers[] =
{
    { A_STR, {10, 12, 50, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _half_troll_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _half_troll_powers);
}
static void _half_troll_calc_bonuses(void)
{
    p_ptr->sustain_str = TRUE;
    p_ptr->regen += 100;
}
static void _half_troll_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_STR);
    add_flag(flgs, OF_REGEN);
}
race_t *half_troll_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Half-Troll";
        me.desc = "Half-Trolls are incredibly strong, and have more hit points than most other races. "
                    "They are also very stupid and slow. They are bad at searching, disarming, perception, "
                    "and stealth. They are so ugly that a Half-Orc grimaces in their presence. "
                    "They also happen to be fun to run... Half-trolls always have their strength sustained. "
                    "Half-Trolls regenerate wounds automatically.";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -4;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] = -2;

        me.skills.dis = -5;
        me.skills.dev = -6;
        me.skills.sav = -5;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb = -6;

        me.life = 107;
        me.base_hp = 25;
        me.exp = 150;
        me.infra = 3;
        me.shop_adjust = 135;

        me.calc_bonuses = _half_troll_calc_bonuses;
        me.get_powers = _half_troll_get_powers;
        me.get_flags = _half_troll_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * High-Elf
 ****************************************************************/
static void _high_elf_calc_bonuses(void)
{
    res_add(RES_LITE);
    p_ptr->see_inv++;
}
static void _high_elf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_SEE_INVIS);
}
race_t *high_elf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "High-Elf";
        me.desc = "High-elves are a race of immortal beings dating from the beginning of time. "
                    "They are masters of all skills, and are strong and intelligent, although "
                    "their wisdom is sometimes suspect. High-elves begin their lives able to "
                    "see the unseen, and resist light effects just like regular elves. However, "
                    "there are few things that they have not seen already, and experience is "
                    "very hard for them to gain.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;

        me.skills.dis =  4;
        me.skills.dev =  9;
        me.skills.sav = 12;
        me.skills.stl =  4;
        me.skills.srh =  3;
        me.skills.fos = 14;
        me.skills.thn = 10;
        me.skills.thb = 15;

        me.life = 99;
        me.base_hp = 19;
        me.exp = 190;
        me.infra = 4;
        me.shop_adjust = 90;

        me.calc_bonuses = _high_elf_calc_bonuses;
        me.get_flags = _high_elf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Hobbit
 ****************************************************************/
static power_info _hobbit_powers[] =
{
    { A_INT, {15, 10, 50, create_food_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _hobbit_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _hobbit_powers);
}
race_t *hobbit_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Hobbit";
        me.desc = "Hobbits, or Halflings, are very good at bows, throwing, and have good saving throws. "
                    "They also are very good at searching, disarming, perception, and stealth; so they "
                    "make excellent rogues, but prefer to be called burglars. They are much weaker than "
                    "humans, and no good at melee fighting. Halflings have fair infravision, so they can "
                    "detect warm creatures at a distance.";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = 15;
        me.skills.dev = 8;
        me.skills.sav = 10;
        me.skills.stl = 5;
        me.skills.srh = 12;
        me.skills.fos = 15;
        me.skills.thn = -10;
        me.skills.thb = 10;

        me.life = 92;
        me.base_hp = 14;
        me.exp = 120;
        me.infra = 4;
        me.shop_adjust = 100;

        me.get_powers = _hobbit_get_powers;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Human
 ****************************************************************/
static void _human_gain_power(int which)
{
	 if (p_ptr->demigod_power[which] < 0)
	 {
		 int idx = mut_gain_choice(mut_demigod_pred);
		 mut_lock(idx);
		 p_ptr->demigod_power[which] = idx;
	 }
	 else if (!mut_present(p_ptr->demigod_power[which]))
	 {
		 mut_gain(p_ptr->demigod_power[which]);
		 mut_lock(p_ptr->demigod_power[which]);
	 }
}
static void _human_gain_level(int new_level)
{
	 if (new_level >= 20)
		 _human_gain_power(0);
	 if (new_level >= 40)
		 _human_gain_power(1);
}

race_t *human_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Human";
        me.desc = "The human is the base character. All other races are compared to them. "
                    "Humans are average at everything and also tend to go up levels faster "
                    "than most other races because of their shorter life spans. No racial "
                    "adjustments or intrinsics occur to characters choosing human. However, "
                    "humans may choose special talents at L20 & 40 that more than make up "
                    "for their overall mediocrity.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  0;

        me.skills.dis = 0;
        me.skills.dev = 0;
        me.skills.sav = 0;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = 0;
        me.skills.thb = 0;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 100;
        me.infra = 0;
        me.shop_adjust = 100;

        me.gain_level = _human_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Imp
 ****************************************************************/
static power_info _imp_powers[] =
{
    { A_INT, {9, 8, 50, imp_fire_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _imp_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _imp_powers);
}
static void _imp_calc_bonuses(void)
{
    res_add(RES_FIRE);
    if (p_ptr->lev >= 10) p_ptr->see_inv++;
}
static void _imp_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
}
race_t *imp_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Imp";
        me.desc = "A demon-creature from the nether-world, naturally resistant to fire attacks, "
                    "and capable of learning fire bolt and fire ball attacks. They are little "
                    "loved by other races, but can perform fairly well in most professions. "
                    "As they advance levels, they gain the powers of See Invisible.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] = -1;

        me.skills.dis = -3;
        me.skills.dev = 1;
        me.skills.sav = -1;
        me.skills.stl = 1;
        me.skills.srh = -1;
        me.skills.fos = 10;
        me.skills.thn = 5;
        me.skills.thb = -3;

        me.life = 99;
        me.base_hp = 19;
        me.exp = 90;
        me.infra = 3;
        me.flags = RACE_IS_DEMON;
        me.shop_adjust = 120;

        me.calc_bonuses = _imp_calc_bonuses;
        me.get_powers = _imp_get_powers;
        me.get_flags = _imp_get_flags;
        init = TRUE;
    }

    return &me;
}

