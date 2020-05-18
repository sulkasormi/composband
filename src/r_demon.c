#include "angband.h"

static cptr _desc =
    "Demons are powerful servants of evil and come in many forms. They rely on their "
    "devilish powers or their brutish strength to survive.\n \n"
    "The various demonic races include the Balrogs, powerful demons of fire; the Servants "
    "of Khorne, mighty warriors of destruction; the Tanar'ri, weaker demons whose ultimate "
    "form has three sets of arms, but prefers to fight naked; and Cyberdemons, whose firepower "
    "is unsurpassable.\n \n"
    "Demons cannot eat normal food, but must feast upon the remains of their human "
    "enemies. They are unaffected by the Eldritch Horror.";

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "devilish power";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 750;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

static void _demon_birth(void)
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

/******************************************************************************
 *                                20            30            40
 * Servant of Khorne: Bloodletter -> Fleshhound -> Juggernaut -> Bloodthirster
 ******************************************************************************/
static void _khorne_birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_BLOODLETTER_KHORNE;
    equip_on_change_race();

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 6;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));
    forge.name2 = EGO_WEAPON_SHARPNESS;
    forge.pval = 1;
    forge.to_h = 0;
    forge.to_d = 0;
    forge.ds = 6;
    forge.weight = 220;
    add_flag(forge.flags, OF_VORPAL);
    py_birth_obj(&forge);

    _demon_birth();
}

static void _khorne_calc_innate_attacks(void)
{
    if (p_ptr->current_r_idx == MON_FLESHHOUND_KHORNE)
    {
        int to_d = py_prorata_level(15);
        int to_h = py_prorata_level(30);

        /* Claws */
        {
            innate_attack_t    a = {0};

            a.dd = 5;
            a.ds = 2;
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

            a.dd = 10;
            a.ds = 2;
            a.to_d += to_d;
            a.to_h += to_h;

            a.weight = 200;

            calc_innate_blows(&a, 300);
            a.msg = "You bite.";
            a.name = "Bite";
            p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        }
    }
    else if (p_ptr->current_r_idx == MON_JUGGERNAUT_KHORNE)
    {
        int to_d = py_prorata_level(15);
        int to_h = py_prorata_level(30);

        /* Claws */
        {
            innate_attack_t    a = {0};

            a.dd = 8;
            a.ds = 6;
            a.to_d += to_d;
            a.to_h += to_h;

            a.weight = 200;
            calc_innate_blows(&a, 400);
            a.msg = "You crush.";
            a.name = "Claw";

            p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        }
        /* Bite */
        {
            innate_attack_t    a = {0};

            a.dd = 10;
            a.ds = 6;
            a.to_d += to_d;
            a.to_h += to_h;

            a.weight = 500;

            calc_innate_blows(&a, 200);
            a.msg = "You butt.";
            a.name = "Head";
            p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        }
    }
}

static void _khorne_calc_bonuses(void)
{
    p_ptr->align -= 200;

    res_add(RES_FIRE);
    res_add(RES_NETHER);

    p_ptr->slow_digest = TRUE;
    p_ptr->hold_life++;
    p_ptr->no_eldritch = TRUE;

    switch (p_ptr->current_r_idx)
    {
    case MON_BLOODLETTER_KHORNE:
        p_ptr->regen += 100;
        res_add(RES_COLD);
        res_add(RES_POIS);
        res_add(RES_CHAOS);
        break;
    case MON_FLESHHOUND_KHORNE:
        p_ptr->pspeed += 2;
        res_add(RES_CONF);
        res_add(RES_NEXUS);
        res_add(RES_DISEN);
        break;
    case MON_JUGGERNAUT_KHORNE:
        p_ptr->pspeed += 4;
        p_ptr->to_a += 100;
        p_ptr->dis_to_a += 100;
        p_ptr->reflect = TRUE;
        p_ptr->free_act++;
        p_ptr->see_inv++;

        res_add(RES_COLD);
        res_add(RES_ELEC);
        res_add(RES_POIS);
        res_add(RES_CONF);
        res_add(RES_FEAR);
        break;
    case MON_BLOODTHIRSTER:
        p_ptr->pspeed += 6;
        p_ptr->to_a += 50;
        p_ptr->dis_to_a += 50;
        p_ptr->regen += 150;
        p_ptr->levitation = TRUE;
        p_ptr->free_act++;
        p_ptr->see_inv++;
        res_add(RES_ACID);
        res_add(RES_COLD);
        res_add(RES_POIS);
        res_add(RES_CONF);
        res_add(RES_NEXUS);
        res_add(RES_TELEPORT);
        break;
    }
}

static void _khorne_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);

    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_SLOW_DIGEST);

    switch (p_ptr->current_r_idx)
    {
    case MON_BLOODLETTER_KHORNE:
        add_flag(flgs, OF_REGEN);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CHAOS);
        break;
    case MON_FLESHHOUND_KHORNE:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_DISEN);
        break;
    case MON_JUGGERNAUT_KHORNE:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_REFLECT);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);

        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_FEAR);
        break;
    case MON_BLOODTHIRSTER:
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_REGEN);
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_NEXUS);
        break;
    }
}

static void _khorne_gain_level(int new_level)
{
    if (p_ptr->current_r_idx == MON_BLOODLETTER_KHORNE && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_FLESHHOUND_KHORNE;
        msg_print("You have evolved into a Fleshhound of Khorne.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FLESHHOUND_KHORNE && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_JUGGERNAUT_KHORNE;
        msg_print("You have evolved into a Juggernaut of Khorne.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_JUGGERNAUT_KHORNE && new_level >= 40)
    {
        object_type forge;
        int slot;
        p_ptr->current_r_idx = MON_BLOODTHIRSTER;
        msg_print("You have evolved into a Bloodthirster.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP;
        object_prep(&forge, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));
        forge.name2 = EGO_WEAPON_DEATH; /* Prevent ?Artifact or ?WeaponBranding */
        forge.dd = 50;
        forge.ds = 1;
        forge.weight = 500;
        forge.to_h = 15;
        forge.to_d = 15;

        /* Poor man's py_birth_obj() - we can't call the actual py_birth_obj()
         * because we need a different origin, which we cannot specify later */
        obj_identify_fully(&forge);
        object_origins(&forge, ORIGIN_PATRON);
        forge.origin_xtra = 11; /* Khorne */

        slot = equip_first_empty_slot(&forge);
        if (slot)
            equip_wield(&forge, slot);
        else
            pack_carry(&forge);

        p_ptr->update |= PU_BONUS;
    }
}

static race_t *_khorne_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Bloodletter of Khorne", "Fleshhound of Khorne", "Juggernaut of Khorne", "Bloodthirster"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  20,  40,  -1,  13,   7,  70,  30};
    skills_t xs = { 12,   8,  10,   0,   0,   0,  32,   7};

        me.subdesc = "Khorne's servants come in many forms and are powerful forces of melee. They know nothing "
        "save melee, and strike at all that dare oppose the will of their master. As they gain "
        "experience, Khorne rewards his servants with new and more powerful forms.";

        me.skills = bs;
        me.extra_skills = xs;

        me.exp = 250;
        me.infra = 5;
        me.base_hp = 42;

        me.birth = _khorne_birth;
        me.calc_innate_attacks = _khorne_calc_innate_attacks;
        me.calc_bonuses = _khorne_calc_bonuses;
        me.get_flags = _khorne_get_flags;
        me.gain_level = _khorne_gain_level;
        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Servant of Khorne";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  3 + rank;
    me.stats[A_INT] = -5;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  0 + rank/3;
    me.stats[A_CON] =  2 + rank;
    me.stats[A_CHR] =  rank/3;
    me.life = 100 + 5*rank;

    me.equip_template = mon_get_equip_template();
    me.boss_r_idx = MON_MEPHISTOPHELES;

    return &me;
}

/******************************************************************************
 *                 10        20       30              40
 * Marilith: Manes -> Quasit -> Bodak -> Death Quasit -> Marilith
 ******************************************************************************/
static spell_info _marilith_get_spells[] = {
    { 10,  3, 30, phase_door_spell},
    { 12,  5, 35, scare_spell},
    { 15,  7, 35, teleport_spell},
    { 17, 10, 35, slow_spell},
    { 20,  7, 40, fire_bolt_spell},
    { 22,  9, 40, summon_manes_spell},
    { 25, 16, 40, fire_ball_spell},
    { 30, 18, 45, cause_wounds_III_spell},
//    { 32, 20, 50, amnesia_spell},
    { 36, 70, 85, summon_demon_spell},
    { 40, 10, 50, enchantment_spell}, /* Note: Mariliths need corpses to eat, so they cannot spam
                                         this spell in the town the way other characters may */
    { -1, -1, -1, NULL}
};

static void _marilith_birth(void) {
    object_type    forge;

    p_ptr->current_r_idx = MON_MANES;
    equip_on_change_race();

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 3;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_BROAD_SWORD));
    forge.to_h = 1;
    forge.to_d = 2;
    py_birth_obj(&forge);

    _demon_birth();
}

static void _marilith_calc_innate_attacks(void) {
    if (p_ptr->lev >= 40)
    {
        innate_attack_t    a = {0};

        a.dd = 3;
        a.ds = 7;
        a.weight = 250;
        a.to_h = p_ptr->lev/2;

        a.effect[0] = GF_MISSILE;
        a.blows = 100;

        a.msg = "You sting.";
        a.name = "Tail";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _marilith_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int slot;
    if (p_ptr->current_r_idx != MON_MARILITH) return;
    if ((!o_ptr) || (!o_ptr->k_idx)) return; /* paranoia */

    /* Penalize shields */
    for (slot = equip_find_first(object_is_shield); slot; slot = equip_find_next(object_is_shield, slot))
    {
        int kasi = equip_which_hand(o_ptr);
        info_ptr->icky_wield = TRUE;
        info_ptr->to_h -= 20;
        info_ptr->to_d -= 10;

        /* Extra-penalize the weapon in the same pair of arms */
        if ((kasi != HAND_NONE) && ((kasi / 2) == (equip_which_hand(equip_obj(slot)) / 2)))
        {
            info_ptr->to_h -= 24;
        }
    }
}

static void _marilith_calc_bonuses(void) {
    p_ptr->align -= 200;

    res_add(RES_FIRE);
    res_add(RES_NETHER);

    p_ptr->slow_digest = TRUE;
    p_ptr->hold_life++;
    p_ptr->no_eldritch = TRUE;

    switch (p_ptr->current_r_idx)
    {
    case MON_QUASIT:
        p_ptr->levitation = TRUE;
        p_ptr->see_inv++;
        break;
    case MON_BODAK:
        res_add(RES_CONF);
        res_add(RES_POIS);
        p_ptr->sh_fire++;
        p_ptr->free_act++;
        p_ptr->see_inv++;
        break;
    case MON_DEATH_QUASIT:
        res_add(RES_CONF);
        res_add(RES_POIS);
        res_add(RES_TELEPORT);
        p_ptr->pspeed += 5;
        p_ptr->levitation = TRUE;
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
        p_ptr->free_act++;
        p_ptr->see_inv++;
        break;
    case MON_MARILITH:
        res_add(RES_POIS);
        res_add(RES_CONF);
        res_add(RES_CHAOS);
        p_ptr->pspeed += 5;
        p_ptr->free_act++;
        p_ptr->see_inv++;
        p_ptr->easy_2weapon = TRUE;
        break;
    }
}

static void _marilith_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);

    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_SLOW_DIGEST);

    switch (p_ptr->current_r_idx)
    {
    case MON_QUASIT:
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_SEE_INVIS);
        break;
    case MON_BODAK:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_AURA_FIRE);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
        break;
    case MON_DEATH_QUASIT:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
        break;
    case MON_MARILITH:
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
        break;
    }
}

static void _marilith_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_MANES && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_QUASIT;
        msg_print("You have evolved into a Quasit.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_QUASIT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_BODAK;
        msg_print("You have evolved into a Bodak.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_BODAK && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_DEATH_QUASIT;
        msg_print("You have evolved into a Death Quasit.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEATH_QUASIT && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_MARILITH;
        p_ptr->psex = SEX_FEMALE;
        msg_print("You have evolved into a Marilith.");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP;
    }
}

static race_t *_marilith_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[5] =  {"Manes", "Quasit", "Bodak", "Death Quasit", "Marilith"};
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  33,  36,   3,  16,  10,  56,  35};
    skills_t xs = { 12,  11,  10,   0,   0,   0,  20,  11};

        me.subdesc = "Tanar'ri were originally slave demons, but rose up to overthrow their masters. "
        "They generally take on a humanoid form and are classic demons full of malice and "
        "cruelty. The ultimate form for this demon is the Marilith, a female demon with "
        "three sets of arms and a serpent body capable of attacking with six melee weapons!";

        me.skills = bs;
        me.extra_skills = xs;

        me.exp = 200;
        me.infra = 5;
        me.base_hp = 30;

        me.birth = _marilith_birth;
        me.calc_innate_attacks = _marilith_calc_innate_attacks;
        me.get_spells = _marilith_get_spells;
        me.calc_bonuses = _marilith_calc_bonuses;
        me.calc_weapon_bonuses = _marilith_calc_weapon_bonuses;
        me.get_flags = _marilith_get_flags;
        me.gain_level = _marilith_gain_level;
        me.caster_info = _caster_info;
        me.pseudo_class_idx = CLASS_CHAOS_WARRIOR;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Tanar'ri";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  (rank == 3) ? 0 : rank;
    me.stats[A_INT] =  rank/2;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  rank;
    me.stats[A_CON] =  (rank == 3) ? 1 : MIN(3, rank);
    me.stats[A_CHR] =  rank/2;
    me.life = (rank == 4) ? 105 : 95;

    me.equip_template = mon_get_equip_template();
    me.boss_r_idx = MON_MEPHISTOPHELES;

    return &me;
}

/******************************************************************************
 *                       40
 * Balrog: Lesser Balrog -> Greater Balrog
 ******************************************************************************/
static spell_info _balrog_get_spells[] = {
    {  2,  1, 20, detect_unlife_spell},
    {  3,  2, 25, evil_bless_spell},
    {  4,  5, 30, resist_fire_spell},
    {  7,  5, 45, scare_spell},
    {  9,  5, 40, fire_bolt_spell},
    { 10,  6, 40, nether_bolt_spell},
    { 11,  9, 35, summon_manes_spell},
    { 20, 12, 40, plasma_bolt_spell},
    { 25, 12, 40, fire_ball_spell},
    { 27, 17, 50, flow_of_lava_spell},
    { 30, 20, 50, recharging_spell},
    { 32, 22, 50, nether_ball_spell},
    { 34, 24, 50, plasma_ball_spell},
    { 36, 70, 55, summon_demon_spell},
    { 37, 32, 60, kiss_of_succubus_spell},
    { 40, 35, 50, brain_smash_spell},
    { 43, 90, 90, summon_greater_demon_spell},
    { 45, 80, 85, hellfire_spell},
    { -1, -1, -1, NULL}
};
static void _balrog_birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_LESSER_BALROG;

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 5;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HAFTED, SV_WHIP));
    forge.name2 = EGO_WEAPON_SLAYING;
    add_flag(forge.flags, OF_BRAND_FIRE);
    forge.dd = 2;
    forge.ds = 6;
    forge.to_h = 5;
    forge.to_d = 5;
    py_birth_obj(&forge);

    _demon_birth();
}
static void _balrog_calc_bonuses(void) {
    p_ptr->align -= 200;

    res_add(RES_FIRE);
    res_add(RES_NETHER);

    p_ptr->hold_life++;
    p_ptr->no_eldritch = TRUE;
    p_ptr->pspeed += p_ptr->lev/8; /* Angels get +7 speed. Demons get +6 speed. */
    p_ptr->sh_fire++;

    if (p_ptr->lev >= 10)
        p_ptr->see_inv++;

    if (p_ptr->lev >= 30)
    {
        res_add(RES_FIRE);
        res_add(RES_CHAOS);
    }

    if (p_ptr->lev >= 40)
    {
        res_add_immune(RES_FIRE);
        res_add(RES_NETHER);
        p_ptr->kill_wall = TRUE;
        p_ptr->no_charge_drain = TRUE;
    }
}
static void _balrog_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);

    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_AURA_FIRE);

    if (p_ptr->lev >= 8)
        add_flag(flgs, OF_SPEED);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_CHAOS);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_IM_FIRE);
}
static void _balrog_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_LESSER_BALROG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_BALROG;
        msg_print("You have evolved into a Greater Balrog.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_balrog_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Lesser Balrog", "Greater Balrog"};
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  35,  40,  -2,  10,   7,  75,  30};
    skills_t xs = { 12,  11,  15,   0,   0,   0,  35,   7};


        me.subdesc = "Balrogs are demons of shadow and flame. Their evil knows no bounds. Their spells are "
        "the most powerful of all demonkind, and at very high levels they may even call forth "
        "fires directly from hell.";

        me.skills = bs;
        me.extra_skills = xs;

        me.exp = 300;
        me.base_hp = 45;

        me.birth = _balrog_birth;
        me.get_spells = _balrog_get_spells;
        me.calc_bonuses = _balrog_calc_bonuses;
        me.get_flags = _balrog_get_flags;
        me.gain_level = _balrog_gain_level;
        me.caster_info = _caster_info;
        me.pseudo_class_idx = CLASS_CHAOS_WARRIOR;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "Balrog";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  4 + 3*rank;
    me.stats[A_INT] =  1 + 2*rank;
    me.stats[A_WIS] = -10;
    me.stats[A_DEX] =  1 + 2*rank;
    me.stats[A_CON] =  4 + 2*rank;
    me.stats[A_CHR] =  2 + rank;
    me.infra = 5 + 10*rank;
    me.life = 105 + 10*rank;

    me.boss_r_idx = MON_GOTHMOG;

    return &me;
}

/******************************************************************************
 * Cyberdemon
 ******************************************************************************/
static int _rocket_amount(void)
{
    int pct = 15 + py_prorata_level(30);
    return 25 + p_ptr->chp * pct / 100;
}

void _cyber_rocket_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rocket");
        break;
    case SPELL_DESC:
        var_set_string(res, "Launches a powerful rocket at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _rocket_amount()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("You launch a rocket.");
        fire_rocket(GF_ROCKET, dir, _rocket_amount(), 2);

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev*19/50 + p_ptr->lev*p_ptr->lev*19/2500);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _cyber_get_powers[] = {
    { A_CON, {  1,  2, 30, _cyber_rocket_spell} },
    {    -1, { -1, -1, -1, NULL}}
};

static void _cyber_birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_CYBER;

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 10;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_HAFTED, SV_GREAT_HAMMER));
    py_birth_obj(&forge);

    _demon_birth();
}

static void _cyber_calc_bonuses(void)
{
    int to_a = py_prorata_level(75);

    p_ptr->move_random = TRUE;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;
    p_ptr->pspeed -= 1 + p_ptr->lev/23;

    res_add(RES_FIRE);
    res_add(RES_POIS);
/*  Cyberdemons are vulnerable to confusion. See res_pct_aux() in resist.c
    res_add_vuln(RES_CONF); */

    p_ptr->hold_life++;
    p_ptr->no_eldritch = TRUE;
    p_ptr->free_act++;
}

static void _cyber_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_DEC_SPEED);

    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_FREE_ACT);
}

static void _cyber_move_player(void)
{
    /* Cyberdemons move erratically (cf get_rep_dir()) and make a lot of noise */
    if ((one_in_(66)) &&
        (!cave_have_flag_bold(py, px, FF_WATER)) &&
        (!cave_have_flag_bold(py, px, FF_LAVA)) &&
        (!cave_have_flag_bold(py, px, FF_ACID)))
    {
        int i;

        cmsg_print(TERM_RED, py_in_dungeon() ? "The dungeon trembles!" : "The ground trembles!");
        if (disturb_minor)
            disturb(0, 0);

        for (i = 1; i < m_max; i++)
        {
            monster_type *m_ptr = &m_list[i];

            if (!m_ptr->r_idx) continue;
            if (m_ptr->cdis < MAX_SIGHT * 2 && MON_CSLEEP(m_ptr))
                (void)set_monster_csleep(i, 0);
        }
    }
}

static race_t *_cyber_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  18,  31,  -1,  13,   7,  75,  30};
    skills_t xs = { 12,   6,   9,   0,   0,   0,  35,   7};

        me.subname = "Cyberdemon";
        me.subdesc = "Cyberdemons are giant humanoid forms, half demon and half machine. They are a bit "
        "slow and move erratically, but their immense bodies and unsurpassable firepower "
        "more than make up for this. The walls of the dungeon reverberate with their heavy steps!\n\n"
        "Cyberdemons are one of the very few monster races not to undergo evolution, and so almost all "
        "their might is available from the beginning, although they do gain even more "
        "strength later on and their offense becomes stronger with experience.";
        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.life = 120;

        me.exp = 275;
        me.base_hp = 50;

        me.birth = _cyber_birth;
        me.get_powers = _cyber_get_powers;
        me.calc_bonuses = _cyber_calc_bonuses;
        me.get_flags = _cyber_get_flags;
        me.move_player = _cyber_move_player;
        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    me.stats[A_STR] =  5 + p_ptr->lev/10;
    me.stats[A_INT] = -10;
    me.stats[A_WIS] = -10;
    me.stats[A_DEX] = -3;
    me.stats[A_CON] =  5 + p_ptr->lev/10;
    me.stats[A_CHR] =  0;

    me.boss_r_idx = MON_OREMORJ;

    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_demon_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case DEMON_BALROG:
        result = _balrog_get_race_t();
        break;
    case DEMON_MARILITH:
        result = _marilith_get_race_t();
        break;
    case DEMON_CYBERDEMON:
        result = _cyber_get_race_t();
        break;
    case DEMON_KHORNE:
        result = _khorne_get_race_t();
        break;
    default: /* Birth Menus */
        result = _balrog_get_race_t();
    }

    result->name = "Demon";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER | RACE_IS_DEMON | RACE_IS_NONLIVING;
    result->shop_adjust = 140;

    return result;
}
