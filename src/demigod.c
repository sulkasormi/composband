#include "angband.h"

/****************************************************************
 * Spells
 ****************************************************************/
static void _kiss_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Kiss");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to charm an adjacent monster.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev * 2);
        break;
    case SPELL_CAST:
    {
        int y, x, dir = 0, m_idx;
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        m_idx = cave[y][x].m_idx;
        if (m_idx)
        {
            monster_type *m_ptr = &m_list[m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];
            char desc[MAX_NLEN];
            monster_desc(desc, m_ptr, 0);
            if ((r_ptr->flags1 & RF1_UNIQUE) || mon_save_p(m_ptr->r_idx, A_CHR))
            {
                set_monster_csleep(m_idx, 0);
                if (is_hostile(m_ptr))
                {
                    switch (randint1(10))
                    {
                    case 1:
                        msg_format("%^s says 'Impudent Strumpet!'", desc);
                        break;
                    case 2:
                        msg_format("%^s says 'Ewwww! Gross!!'", desc);
                        break;
                    case 3:
                        msg_format("%^s says 'You ain't my type!'", desc);
                        break;
                    default:
                        msg_format("%^s resists your charms.", desc);
                    }

                    if (allow_ticked_off(r_ptr))
                        mon_anger(m_ptr);
                }
                else
                    msg_format("%^s ignores you.", desc);
            }
            else
            {
                if (is_pet(m_ptr))
                    msg_format("%^s slobbers on you affectionately.", desc);
                else if (is_friendly(m_ptr))
                {
                    set_pet(m_ptr);
                    msg_format("%^s is charmed!", desc);
                }
                else
                {
                    set_friendly_ingame(m_ptr);
                    msg_format("%^s suddenly becomes friendly.", desc);
                }
            }
            var_set_bool(res, TRUE);
        }
        else
        {
            msg_print("There is no monster.");
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _demeter_clw_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cure Wounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Heals cut and HP a little.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(p_ptr->lev/12 + 1, 10, 0));
        break;
    case SPELL_CAST:
        if (p_ptr->pclass == CLASS_BLOOD_MAGE)
            msg_print("There is no effect.");
        else
        {
            hp_player(damroll(p_ptr->lev/12 + 1, 10));
            set_cut(p_ptr->cut - 10, TRUE);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shine_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shine");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a large ball of sunlight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->lev * 3));
        break;
    case SPELL_CAST:
        fire_ball(GF_LITE, 0, p_ptr->lev * 3 * 2, 3);
        var_set_bool(res, TRUE);
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev - 20)/2);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Demigod
 ****************************************************************/
static void _gain_power(int which)
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
static void _gain_level(int new_level)
{
    if (new_level >= 20)
        _gain_power(0);
    if (new_level >= 40)
        _gain_power(1);
}

void demigod_rechoose_powers(void)
{
    int i, idx;
    for (i = 0; i < MAX_DEMIGOD_POWERS; i++)
    {
        idx = p_ptr->demigod_power[i];
        if (idx >= 0)
        {
            mut_unlock(idx);
            mut_lose(idx);
            p_ptr->demigod_power[i] = -1;
        }
    }
    _gain_level(p_ptr->lev);
}

/****************************************************************
 * Aphrodite
 ****************************************************************/
static power_info _aphrodite_powers[] =
{
    { A_CHR, {1, 10, 50, _kiss_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _aphrodite_calc_bonuses(void)
{
    p_ptr->sustain_chr = TRUE;
}
static void _aphrodite_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_CHR);
}

/****************************************************************
 * Apollo
 ****************************************************************/
static power_info _apollo_powers[] =
{
    { A_INT, { 5,  3, 50, light_area_spell}},
    { A_WIS, {12,  7, 60, ray_of_sunlight_spell}},
    { A_CHR, {20, 10, 40, _shine_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _apollo_calc_bonuses(void)
{
    res_add_immune(RES_LITE);
    res_add(RES_BLIND);
    /* cf calc_torch in xtra1.c for the 'extra light' */
}
static void _apollo_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_BLIND);
    add_flag(flgs, OF_IM_LITE);
}

/****************************************************************
 * Ares
 ****************************************************************/
static power_info _ares_powers[] =
{
    { A_STR, {10, 10, 30, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _ares_calc_bonuses(void)
{
    int dam = 1 + p_ptr->lev/7;
    int ac = 1 + p_ptr->lev/5;
    int hand;

    p_ptr->sustain_str = TRUE;

    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;

    p_ptr->to_d_m  += dam;
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
        {
            p_ptr->weapon_info[hand].to_d += dam / p_ptr->weapon_ct;
            p_ptr->weapon_info[hand].dis_to_d += dam / p_ptr->weapon_ct;
        }
    }
}
static void _ares_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_STR);
}

/****************************************************************
 * Artemis
 ****************************************************************/
static void _artemis_calc_bonuses(void)
{
    p_ptr->shooter_info.to_d += 1 + p_ptr->lev/7;
    p_ptr->shooter_info.dis_to_d += 1 + p_ptr->lev/7;
    p_ptr->sustain_dex = TRUE;
}
static void _artemis_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_DEX);
}
static void _artemis_birth(void)
{
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(15, 20));
    py_birth_food();
    py_birth_light();
}

/****************************************************************
 * Athena
 ****************************************************************/
static void _athena_calc_bonuses(void)
{
    p_ptr->sustain_int = TRUE;
}
static void _athena_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_INT);
}

/****************************************************************
 * Demeter
 ****************************************************************/
static power_info _demeter_powers[] =
{
    { A_WIS, {5, 0, 60, _demeter_clw_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _demeter_calc_bonuses(void)
{
    p_ptr->regen += 100;
    p_ptr->slow_digest = TRUE;
    if (p_ptr->lev >= 40)
        res_add(RES_TIME);
}
static void _demeter_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_REGEN);
    add_flag(flgs, OF_SLOW_DIGEST);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_TIME);
}

/****************************************************************
 * Hades
 ****************************************************************/
static void _hades_calc_bonuses(void)
{
    res_add(RES_NETHER);
    p_ptr->hold_life++;
    p_ptr->sustain_con = TRUE;
}
static void _hades_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_SUST_CON);
}

/****************************************************************
 * Hephaestus
 ****************************************************************/
static void _hephaestus_calc_bonuses(void)
{
    res_add(RES_DISEN);
}
static void _hephaestus_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_DISEN);
}

/****************************************************************
 * Hera
 ****************************************************************/
static power_info _hera_powers[] =
{
    { A_WIS, {15, 0, 30, clear_mind_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _hera_calc_bonuses(void)
{
    p_ptr->spell_cap += 2;
    if (p_ptr->lev >= 15)
        p_ptr->clear_mind = TRUE;
}
static void _hera_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPELL_CAP);
}

/****************************************************************
 * Hermes
 ****************************************************************/
static void _hermes_calc_bonuses(void)
{
    p_ptr->pspeed += 5 * p_ptr->lev/50;
    p_ptr->no_slow = TRUE;
}
static void _hermes_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SPEED);
}

/****************************************************************
 * Poseidon
 ****************************************************************/
static void _poseidon_birth(void)
{
    py_birth_obj_aux(TV_POLEARM, SV_TRIDENT, 1);
    skills_weapon_init(TV_POLEARM, SV_TRIDENT, WEAPON_EXP_BEGINNER);
    py_birth_food();
    py_birth_light();
}

static void _poseidon_calc_bonuses(void)
{
    p_ptr->melt_armor = TRUE;
    if (p_ptr->lev >= 5)
        res_add(RES_ACID);
    if (p_ptr->lev >= 10)
        res_add(RES_COLD);
    if (p_ptr->lev >= 20)
        res_add(RES_ELEC);
}
static void _poseidon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 5)
        add_flag(flgs, OF_RES_ACID);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_COLD);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_RES_ELEC);
}
/****************************************************************
 * Zeus
 ****************************************************************/
static void _zeus_calc_bonuses(void)
{
    res_add(RES_ELEC);
    p_ptr->sh_elec++;
}
static void _zeus_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_AURA_ELEC);
}

race_t *demigod_get_race(int psubrace)
{
    static race_t me = {0};
    static bool init = FALSE;
    static int subrace_init = -1;

    /* static info never changes */
    if (!init)
    {
        me.name = "Demigod";
        me.desc = "The term demigod is commonly used to describe mythological figures whose one "
                    "parent was a god and whose other parent was human; as such, demigods are "
                    "human-god hybrids and are quite powerful. Demigods receive special abilities "
                    "depending on their parentage.";
        
        me.infra = 0;

        me.gain_level = _gain_level;
        init = TRUE;
    }

    if (subrace_init != psubrace)
    {
        int i;
        
        /* Reset to Minor God */
        me.stats[A_STR] =  1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;
        
        me.skills.dis =  2;
        me.skills.dev =  2;
        me.skills.sav =  1;
        me.skills.stl = -1;
        me.skills.srh =  0;
        me.skills.fos =  7;
        me.skills.thn = 10;
        me.skills.thb =  3;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 180;
        me.shop_adjust = 100;

        me.birth = NULL;
        me.calc_bonuses = NULL;
        me.get_powers = NULL;
        me.get_flags = NULL;
        me.flags = RACE_DEMI_TALENT;

        me.subname = NULL;
        me.subdesc = NULL;

        /* Override with New Type */
        /* Some of the following descriptions were inspired by wikipedia ... */
        switch (psubrace)
        {
        case DEMIGOD_MINOR:
            me.subname = "Minor-God";
            me.subdesc = "Fathered by a minor god, you gain no starting powers.";
            break;
        case DEMIGOD_APHRODITE:
            me.subname = "Aphrodite";
            me.subdesc = "Aphrodite is the Greek goddess of love, beauty, pleasure, and procreation. "
                         "You inherit her sex appeal. As such, your pets are more obedient and shopkeepers "
                         "fawn over you in their efforts to please you. You may kiss monsters and they "
                         "might even decide to follow you with slavish devotion, though sometimes this "
                         "just angers them.";
            me.stats[A_CHR] += 2;
            me.exp += 40;
            me.skills.dev = 3;
            me.shop_adjust = 70;
            me.calc_bonuses = _aphrodite_calc_bonuses;
            me.get_powers = _aphrodite_powers;
            me.get_flags = _aphrodite_get_flags;
            break;
        case DEMIGOD_APOLLO:
            me.subname = "Apollo";
            me.subdesc = "Apollo has been variously recognized as a god of light and the sun, "
                         "truth and prophecy, medicine, healing, plague, music, poetry, arts, "
                         "archery, and more. You inherit powers of illumination and are completely "
                         "immune to light based attacks. You are seldom blinded.";
            me.exp += 50;
            me.skills.dev = 4;
            me.calc_bonuses = _apollo_calc_bonuses;
            me.get_powers = _apollo_powers;
            me.get_flags = _apollo_get_flags;
            break;
        case DEMIGOD_ARES:
            me.subname = "Ares";
            me.subdesc = "Ares is the bold son of Zeus and Hera, whose very name is feared and respected "
                         "by warriors and citizens alike. His legendary combat prowess exceeds that of "
                         "Zeus and Poseidon, but he is less skilled in wiles than the other Olympians. "
                         "You inherit exceptional bonuses to combat and have a firm grip on your strength. "
                         "Later in life, you will be able to fly into a berserk rage on command. However, "
                         "your lust for combat decreases your stealth as you call out challenges "
                         "to all that you meet. And your resistance to magic is suspect as well.";
            me.stats[A_STR] += 2;
            me.skills.dev  = 0;
            me.skills.sav -= 5;
            me.skills.stl -= 1;
            me.skills.thn += 15;
            me.exp += 60;
            me.calc_bonuses = _ares_calc_bonuses;
            me.get_powers = _ares_powers;
            me.get_flags = _ares_get_flags;
            break;
        case DEMIGOD_ARTEMIS:
            me.subname = "Artemis";
            me.subdesc = "Artemis was often described as the daughter of Zeus and Leto, and the "
                         "twin sister of Apollo. She was the Hellenic goddess of the hunt, wild "
                         "animals, wilderness, childbirth, virginity and young girls, bringing "
                         "and relieving disease in women; she often was depicted as a huntress "
                         "carrying a bow and arrows. You inherit powers of archery and are very "
                         "nimble. Your skills with the bow are unmatched and you shoot arrows "
                         "with increased range and deadliness.";
            me.stats[A_DEX] += 2;
            me.skills.thb += 12;
            me.exp += 50;
            me.birth = _artemis_birth;
            me.calc_bonuses = _artemis_calc_bonuses;
            me.get_flags = _artemis_get_flags;
            break;
        case DEMIGOD_ATHENA:
            me.subname = "Athena";
            me.subdesc = "Athena is the great goddess of wisdom and the protector of Athens. She was "
                         "born of Zeus and the Titan Metis, and her cunning far surpasses that of the "
                         "other deities. You inherit great clarity of thought and magic and will be "
                         "able to cast spells more reliably than other mortals. And should you fail to "
                         "cast a spell, you will pay a reduced casting cost rather than the full amount. "
                         "You also have a firm grip on your mental prowess.";
            me.stats[A_INT] += 2;
            me.exp += 60;
            me.skills.dev  = 9;
            me.skills.sav += 2;
            me.calc_bonuses = _athena_calc_bonuses;
            me.get_flags = _athena_get_flags;
            break;
        case DEMIGOD_DEMETER:
            me.subname = "Demeter";
            me.subdesc = "Demeter is the goddess of the harvest, who presided over grains, the "
                         "fertility of the earth, and the seasons. You gain powers of regeneration, "
                         "healing, and temperance. Eventually, you will become resistant to the "
                         "ravages of time.";
            me.exp += 40;
            me.skills.sav += 5;
            me.calc_bonuses = _demeter_calc_bonuses;
            me.get_powers = _demeter_powers;
            me.get_flags = _demeter_get_flags;
            break;
        case DEMIGOD_HADES:
            me.subname = "Hades";
            me.subdesc = "Hades is Ruler of the Underworld. You gain resistance to nether forces, "
                         "increased fortitude and have a firm grasp on your life and health.";
            me.stats[A_CON] += 2;
            me.skills.sav += 10;
            me.skills.thn += 5;
            me.skills.stl += 1;
            me.life += 7;
            me.exp += 60;
            me.calc_bonuses = _hades_calc_bonuses;
            me.get_flags = _hades_get_flags;
            break;
        case DEMIGOD_HEPHAESTUS:
            me.subname = "Hephaestus";
            me.subdesc = "Hephaestus was the god of technology, blacksmiths, craftsmen, artisans, "
                         "sculptors, metals, metallurgy, fire and volcanoes. Like other mythic smiths "
                         "but unlike most other gods, Hephaestus was lame, which gave him a "
                         "grotesque appearance in Greek eyes. He served as the blacksmith of the "
                         "gods. You inherit powers of enchantment and protection. Your equipment "
                         "will not suffer diminution by corrosion or magical attack.";
            me.exp += 50;
            me.skills.thn += 10;
            me.skills.sav += 2;
            me.calc_bonuses = _hephaestus_calc_bonuses;
            me.get_flags = _hephaestus_get_flags;
            break;
        case DEMIGOD_HERA:
            me.subname = "Hera";
            me.subdesc = "Hera was the wife and one of three sisters of Zeus. Her chief function "
                         "was as the goddess of women and marriage. You inherit great clarity of "
                         "mind and capacity for magic. You are very wise and have a firm grasp on "
                         "your magical energy, which is even augmented.";
            me.stats[A_WIS] += 2;
            me.exp += 40;
            me.skills.sav += 5;
            me.skills.dev  = 6;
            me.calc_bonuses = _hera_calc_bonuses;
            me.get_powers = _hera_powers;
            me.get_flags = _hera_get_flags;
            break;
        case DEMIGOD_HERMES:
            me.subname = "Hermes";
            me.subdesc = "Hermes, the Messenger, is the extremely cunning diplomat used by the Olympians "
                         "to negotiate truces. With his Winged Sandals and his powerful magic, there "
                         "is no place barred from him, and there is no way to detain him. You inherit "
                         "great powers of motion and will never become slowed. Your stealth is better "
                         "than your brethren and your speed will increase with experience.";
            me.skills.dis += 10;
            me.skills.stl += 3;
            me.skills.srh += 5;
            me.skills.fos += 5;
            me.exp += 60;
            me.calc_bonuses = _hermes_calc_bonuses;
            me.get_flags = _hermes_get_flags;
            break;
        case DEMIGOD_POSEIDON:
            me.subname = "Poseidon";
            me.subdesc = "Poseidon, Brother of Zeus, is Lord of the Seas and Storm. You inherit "
                         "elemental protection and corrosive attacks that melt the armor of your foes.";
            me.stats[A_STR] += 1;
            me.stats[A_DEX] += 1;
            me.skills.thn += 7;
            me.exp += 60;
            me.birth = _poseidon_birth;
            me.calc_bonuses = _poseidon_calc_bonuses;
            me.get_flags = _poseidon_get_flags;
            break;
        case DEMIGOD_ZEUS:
            me.subname = "Zeus";
            me.subdesc = "Zeus, King of the gods and ruler of Mount Olympus, is god of the Sky "
                         "and Thunder, and nominal husband of Hera. You inherit increased stature "
                         "and your divine birth will be marked by your aura of electricity. You also "
                         "resist lightning.";
            me.skills.dis += 2;
            me.skills.dev  = 5;
            me.skills.sav += 5;
            me.skills.thn += 10;
            me.skills.thb += 3;
            for (i = 0; i < 6; i++)
                me.stats[i]++;
            me.exp += 70;
            me.shop_adjust = 90;
            me.calc_bonuses = _zeus_calc_bonuses;
            me.get_flags = _zeus_get_flags;
            break;
        }
        
        subrace_init = psubrace;
    }

    return &me;
}


