#include "angband.h"
#include <assert.h>

int hit_chance_innate(int to_h, int ac)
{
    int chance = p_ptr->skills.thn + to_h * BTH_PLUS_ADJ;
    int odds;

    if (p_ptr->stun)
        chance -= chance * MIN(100, p_ptr->stun) / 150;
    if (chance <= 0) return 0;

    if (prace_is_(RACE_MON_GOLEM))
        ac = ac * (100 - p_ptr->lev) / 100;

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (odds > 50)
    {
        if (personality_is_(PERS_LAZY)) odds = (19 * odds + 10) / 20;
        if (mut_present(MUT_HUMAN_CHR)) odds = (19 * odds + 10) / 20;
    }
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

int hit_chance(int hand, int to_h, int ac)
{
    int chance = p_ptr->skills.thn + (p_ptr->weapon_info[hand].to_h + to_h) * BTH_PLUS_ADJ;
    int odds;

    chance = chance * p_ptr->weapon_info[hand].dual_wield_pct / 1000;
    if (p_ptr->special_defense & KATA_KOUKIJIN) chance += 150;
//    if (p_ptr->sutemi) chance = MAX(chance * 3 / 2, chance + 60);
    chance += virtue_current(VIRTUE_VALOUR) / 10;
    if (p_ptr->stun)
        chance -= chance * MIN(100, p_ptr->stun) / 150;
    if (chance <= 0) return 0;

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (odds > 50)
    {
        if (personality_is_(PERS_LAZY)) odds = (19 * odds + 10) / 20;
        if (mut_present(MUT_HUMAN_CHR)) odds = (19 * odds + 10) / 20;
    }
    if (odds < 50) odds = 50;
    if (display_weapon_mode == HISSATSU_MAJIN) odds /= 2;
    return (odds+5)/10;
}

int throw_hit_chance(int to_h, int ac, int range)
{
    int chance = p_ptr->skill_tht + (p_ptr->shooter_info.to_h + to_h) * BTH_PLUS_ADJ - range;
    int odds;

    if (p_ptr->stun)
        chance -= chance * MIN(100, p_ptr->stun) / 150;
    if (chance <= 0) return 0;
    if (melee_challenge) return 0;

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (odds > 50)
    {
        if (personality_is_(PERS_LAZY)) odds = (19 * odds + 10) / 20;
        if (mut_present(MUT_HUMAN_CHR)) odds = (19 * odds + 10) / 20;
    }
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

int bow_hit_chance(int to_h, int ac)
{
    int chance;
    int odds;

    chance = p_ptr->skills.thb + to_h * BTH_PLUS_ADJ;
    if (p_ptr->stun)
        chance -= chance * MIN(100, p_ptr->stun) / 150;
    if (chance <= 0) return 0;
    if (melee_challenge) return 0;

    if (p_ptr->concent)
    {
        ac *= (10 - p_ptr->concent);
        ac /= 10;
    }

    odds = 95*(chance - ac*3/4)*1000/(chance*100);
    if (odds > 50)
    {
        if (personality_is_(PERS_LAZY)) odds = (19 * odds + 10) / 20;
        if (mut_present(MUT_HUMAN_CHR)) odds = (19 * odds + 10) / 20;
    }
    if (odds < 50) odds = 50;
    return (odds+5)/10;
}

/* Class-dependent melee multiplier */
int class_melee_mult(void)
{
    switch (p_ptr->pclass)
    {
        case CLASS_NINJA_LAWYER: return 80;
        case CLASS_LAWYER: return 95;
        case CLASS_MAULER: return 91;
        case CLASS_ALCHEMIST: return 88;
        case CLASS_POLITICIAN: return 86;
        case CLASS_PRIEST: return 94;
        case CLASS_MONK: return (94 - (p_ptr->lev / 11));
        default: return 100;
    }
}

/* Race-dependent melee multiplier */
int race_melee_mult(bool attack_is_innate)
{
    switch ((p_ptr->mimic_form != MIMIC_NONE) ? p_ptr->mimic_form : p_ptr->prace)
    {
        case RACE_WEREWOLF:
        {
            if (werewolf_in_human_form()) return 100;
            else if (!attack_is_innate) return 100;
            else return MIN(115, MAX(66, 61 + (get_class()->base_skills.thn * 3 / 5)));
        }
        case RACE_BEORNING:
        {
            if (beorning_is_(BEORNING_FORM_HUMAN)) return 100;
            else if (!attack_is_innate) return 100;
            else return MIN(140, MAX(55, 50 + (get_class()->base_skills.thn * 3 / 4)));
        }
        case RACE_MON_ELEMENTAL:
        {
            if (elemental_is_(ELEMENTAL_WATER)) return 75 + (water_flow_rate() / 2);
            return 100;
        }
        case RACE_IGOR:
        {
            if (!attack_is_innate) return 100;
            return MIN(115, MAX(66, 61 + (get_class()->base_skills.thn * 3 / 5)));
        }
        case RACE_MON_ORC:
        case RACE_BOIT: return 95;
        case RACE_TOMTE: return 82;
        case RACE_NIBELUNG: return 96;
        default: return 100;
    }
}

/**********************************************************************
 * Number of Blows
 **********************************************************************/

/* TODO: This should be moved to class_t.calc_weapon_bonuses */
void init_blows_calc(object_type *o_ptr, weapon_info_t *info_ptr)
{
    switch (p_ptr->pclass)
    {
    case CLASS_WARRIOR:
        info_ptr->blows_calc.max = 600;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 50 + p_ptr->lev/2;
        break;

    case CLASS_MAULER:
        info_ptr->blows_calc.max = 300;
        info_ptr->blows_calc.wgt = 280;
        info_ptr->blows_calc.mult = 125;
        break;

    case CLASS_BERSERKER:
        info_ptr->blows_calc.max = 600;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 75;
        break;

    case CLASS_RAGE_MAGE:
        info_ptr->blows_calc.max = 300;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 30;
        break;

    case CLASS_MAGE:
    case CLASS_NECROMANCER:
    case CLASS_BLOOD_MAGE:
    case CLASS_HIGH_MAGE:
    case CLASS_YELLOW_MAGE:
    case CLASS_GRAY_MAGE:
    case CLASS_BLUE_MAGE:
        info_ptr->blows_calc.max = 400;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 20;
        break;

    case CLASS_WARLOCK:
        info_ptr->blows_calc.max = 400;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 35;
        switch (p_ptr->psubclass)
        {
        case WARLOCK_DRAGONS:
            if ( p_ptr->riding
              && (object_is_(o_ptr, TV_POLEARM, SV_LANCE) || object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE)) )
            {
                info_ptr->blows_calc.mult = 65;
            }
            break;
        case WARLOCK_ANGELS:
        case WARLOCK_DEMONS:
            info_ptr->blows_calc.max = 450;
            break;
        case WARLOCK_HOUNDS:
            info_ptr->blows_calc.max = 475;
            break;
        case WARLOCK_GIANTS:
            info_ptr->blows_calc.wgt = 200;
            info_ptr->blows_calc.mult = 50 + p_ptr->lev/5;
            info_ptr->blows_calc.max = 500;
            break;
        }
        break;

    case CLASS_DISCIPLE:
        info_ptr->blows_calc.max = 475;
        info_ptr->blows_calc.wgt = 75;
        info_ptr->blows_calc.mult = 40;
        if (p_ptr->psubclass == DISCIPLE_TROIKA)
        {
            info_ptr->blows_calc.wgt = 100;
            info_ptr->blows_calc.mult = 50;
        }
        break;

    case CLASS_PSION:
        info_ptr->blows_calc.max = 400;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 30;
        break;

    case CLASS_PRIEST:
    case CLASS_MAGIC_EATER:
    case CLASS_MINDCRAFTER:
        info_ptr->blows_calc.max = 500;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 35;
        break;

    case CLASS_DEVICEMASTER:
        info_ptr->blows_calc.max = 400;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 35;
        if (p_ptr->psubclass == DEVICEMASTER_POTIONS || p_ptr->psubclass == DEVICEMASTER_SCROLLS)
            info_ptr->blows_calc.max = 500;
        break;

    case CLASS_ROGUE:
        info_ptr->blows_calc.max = MAX(400, 500 + (150 - o_ptr->weight));
        info_ptr->blows_calc.wgt = 40;
        info_ptr->blows_calc.mult = 30;
        break;

    case CLASS_SCOUT:
        info_ptr->blows_calc.max = 400;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 25;
        break;

    case CLASS_RANGER:
        info_ptr->blows_calc.max = 500;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 40;
        break;

    case CLASS_PALADIN:
    case CLASS_SAMURAI:
        info_ptr->blows_calc.max = 550;
        info_ptr->blows_calc.wgt = 70;
        info_ptr->blows_calc.mult = 45;
        break;

    case CLASS_MYSTIC:
        info_ptr->blows_calc.max = 100;
        info_ptr->blows_calc.wgt = 100;
        info_ptr->blows_calc.mult = 10;
        break;

    case CLASS_WEAPONSMITH:
    case CLASS_RUNE_KNIGHT:
        info_ptr->blows_calc.max = 525;
        info_ptr->blows_calc.wgt = 150;
        info_ptr->blows_calc.mult = 55;
        break;

    case CLASS_WARRIOR_MAGE:
    case CLASS_RED_MAGE:
        info_ptr->blows_calc.max = 525; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 30; break;

    case CLASS_CHAOS_WARRIOR:
        info_ptr->blows_calc.max = 550; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 45; break;

    case CLASS_MONK:
        info_ptr->blows_calc.max = 500; info_ptr->blows_calc.wgt = 60; info_ptr->blows_calc.mult = 30; break;

    case CLASS_TOURIST:
    case CLASS_TIME_LORD:
        info_ptr->blows_calc.max = 400; info_ptr->blows_calc.wgt = 100; info_ptr->blows_calc.mult = 30; break;

    case CLASS_ARCHAEOLOGIST:
        info_ptr->blows_calc.max = 400; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 30;
        if (archaeologist_is_favored_weapon(o_ptr))
        {
            info_ptr->blows_calc.max = 500;
            info_ptr->blows_calc.mult = 40;
        }
        break;

    case CLASS_BLOOD_KNIGHT:
        info_ptr->blows_calc.max = 300; info_ptr->blows_calc.wgt = 150; info_ptr->blows_calc.mult = 30; break;

    case CLASS_DUELIST:
        info_ptr->blows_calc.max = 100; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 40; break;

    case CLASS_WILD_TALENT:
        info_ptr->blows_calc.max = 450; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 40; break;

    case CLASS_LAWYER:
        info_ptr->blows_calc.max = 450; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 30; break;

    case CLASS_POLITICIAN:
        info_ptr->blows_calc.max = 475; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 25; break;

    case CLASS_BEASTMASTER:
        info_ptr->blows_calc.max = 500; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 35; break;

    case CLASS_CAVALRY:
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        if (p_ptr->riding && have_flag(flgs, OF_RIDING)) {info_ptr->blows_calc.max = 550; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 65;}
        else {info_ptr->blows_calc.max = 500; info_ptr->blows_calc.wgt = 100; info_ptr->blows_calc.mult = 35;}
        break;
    }
    case CLASS_SORCERER:
        info_ptr->blows_calc.max = 100; info_ptr->blows_calc.wgt = 1; info_ptr->blows_calc.mult = 10; break;

    case CLASS_ARCHER:
    case CLASS_BARD:
        info_ptr->blows_calc.max = 450; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 20; break;

    case CLASS_FORCETRAINER:
        info_ptr->blows_calc.max = 400; info_ptr->blows_calc.wgt = 60; info_ptr->blows_calc.mult = 20; break;

    case CLASS_MIRROR_MASTER:
    case CLASS_SNIPER:
        info_ptr->blows_calc.max = 400; info_ptr->blows_calc.wgt = 100; info_ptr->blows_calc.mult = 30; break;

    case CLASS_NINJA:
    case CLASS_NINJA_LAWYER:
        info_ptr->blows_calc.max = 425; info_ptr->blows_calc.wgt = 20; info_ptr->blows_calc.mult = 10; break;

    case CLASS_ALCHEMIST:
        info_ptr->blows_calc.max = 525; info_ptr->blows_calc.wgt = 100; info_ptr->blows_calc.mult = 25; break;

    case CLASS_MONSTER:
        info_ptr->blows_calc.max = 500; info_ptr->blows_calc.wgt = 70; info_ptr->blows_calc.mult = 50;
        if (prace_is_(RACE_MON_ARMOR))
        {
            info_ptr->blows_calc.max = 666;
            info_ptr->blows_calc.mult = 40;
        }
        else if (prace_is_(RACE_MON_LICH))
        {
            info_ptr->blows_calc.max = 400;
            info_ptr->blows_calc.mult = 30;
        }
        else if (prace_is_(RACE_MON_POSSESSOR))
        {
            info_ptr->blows_calc.max = 400;
        }
        else if (prace_is_(RACE_MON_MIMIC))
        {
            info_ptr->blows_calc.max = 400;
        }
        else if (prace_is_(RACE_MON_TROLL))
        {
            info_ptr->blows_calc.max = 550;
        }
        else if (prace_is_(RACE_MON_PUMPKIN))
        {
            info_ptr->blows_calc.max = 470;
        }
        else if (prace_is_(RACE_MON_GIANT))
        {
            info_ptr->blows_calc.max = 550;
            info_ptr->blows_calc.mult = 50 + p_ptr->lev/5;
            info_ptr->blows_calc.wgt = 200;
            if (giant_is_(GIANT_HRU) && p_ptr->lev >= 40)
                info_ptr->blows_calc.mult = 80;
        }
        else if ( prace_is_(RACE_MON_JELLY)
               || demon_is_(DEMON_KHORNE) )
        {
            info_ptr->blows_calc.max = 600;
            info_ptr->blows_calc.mult = 50 + p_ptr->lev/5;
        }
        else if (prace_is_(RACE_MON_LEPRECHAUN))
        {
            info_ptr->blows_calc.max = 300; /* but gold gives extra blows ... */
            info_ptr->blows_calc.mult = 20;
        }
        else if (prace_is_(RACE_MON_SWORD))
        {
            info_ptr->blows_calc.max = 525;
            if (p_ptr->lev >= 45) /* Death Scythes retaliate! */
                info_ptr->blows_calc.max = 300;
        }
        else if ((prace_is_(RACE_MON_GOLEM)) || (prace_is_(RACE_MON_MUMMY)))
        {
            info_ptr->blows_calc.max = 100;
        }
        break;
    }
}

/* New Blows Calc
   Dex (old dex_index) determines the min/max range of the number of blows (scaled by 100).
   Str*Mul/Div (old str_index) interpolates this range linearly (110 steps)
   We now support fractional blows (e.g. 2.75 is stored as 275 and gives 3 blows 75% of the time)

   This is not exactly the same as before, but I think the differences are minor enough.
*/
typedef struct {
    int min;
    int max;
} _range_t;

static _range_t _blows_range[] =
{
    {   0, 200}       /* 3 */,
    {   0, 200}       /* 4 */,
    {  10, 200}       /* 5 */,
    {  20, 210}       /* 6 */,
    {  30, 220}       /* 7 */,
    {  40, 230}       /* 8 */,
    {  50, 240}       /* 9 */,
    {  60, 250}       /* 10 */,
    {  70, 260}       /* 11 */,
    {  80, 270}       /* 12 */,
    {  90, 280}       /* 13 */,
    { 100, 290}       /* 14 */,
    { 110, 300}       /* 15 */,
    { 120, 350}       /* 16 */,
    { 130, 400}       /* 17 */,
    { 140, 450}       /* 18/00-18/09 */,
    { 150, 460}       /* 18/10-18/19 */,
    { 160, 470}       /* 18/20-18/29 */,
    { 170, 480}       /* 18/30-18/39 */,
    { 180, 490}       /* 18/40-18/49 */,
    { 190, 500}       /* 18/50-18/59 */,
    { 200, 520}       /* 18/60-18/69 */,
    { 210, 540}       /* 18/70-18/79 */,
    { 220, 560}       /* 18/80-18/89 */,
    { 230, 580}       /* 18/90-18/99 */,
    { 240, 600}       /* 18/100-18/109 */,
    { 250, 610}       /* 18/110-18/119 */,
    { 260, 620}       /* 18/120-18/129 */,
    { 280, 630}       /* 18/130-18/139 */,
    { 300, 640}       /* 18/140-18/149 */,
    { 320, 650}       /* 18/150-18/159 */,
    { 340, 660}       /* 18/160-18/169 */,
    { 350, 670}       /* 18/170-18/179 */,
    { 360, 680}       /* 18/180-18/189 */,
    { 370, 690}       /* 18/190-18/199 */,
    { 380, 700}       /* 18/200-18/209 */,
    { 390, 725}       /* 18/210-18/219 */,
    { 400, 750}       /* 18/220+ */
};

int calculate_base_blows(int hand, int str_idx, int dex_idx)
{
    int            result = 0;
    weapon_info_t *info_ptr = &p_ptr->weapon_info[hand];
    object_type   *o_ptr = NULL;
    int            blow_str_idx;
    int            div = 0;
    int            mul = 0;
    int            wgt = 0;
    _range_t       rng = _blows_range[dex_idx];

    if (info_ptr->wield_how == WIELD_NONE) return 0;

    o_ptr = equip_obj(info_ptr->slot);
    if (!o_ptr) return 0;
    if (info_ptr->heavy_wield) return 100;

    wgt = o_ptr->weight;
    div = wgt < info_ptr->blows_calc.wgt ? info_ptr->blows_calc.wgt : wgt;
    mul = info_ptr->blows_calc.mult + info_ptr->giant_wield * 10;

    blow_str_idx = adj_str_blow[str_idx] * mul / div; /* Scaled by 10 */
    if (info_ptr->wield_how == WIELD_TWO_HANDS)
    {
        if (!info_ptr->omoi)
            blow_str_idx += 10;
        if (prace_is_(RACE_MON_GIANT) && giant_is_favorite(o_ptr))
            blow_str_idx += 10;
    }
    if (player_is_ninja) blow_str_idx = MAX(0, blow_str_idx-10);
    if (blow_str_idx > 110) blow_str_idx = 110;

    result = rng.min + (rng.max - rng.min) * blow_str_idx / 110;

    if (p_ptr->pclass == CLASS_MAULER)
        result = 100 + (result - 100)*2/5;
    else if (p_ptr->pclass == CLASS_WEAPONMASTER)
    {
        if ((weaponmaster_is_(WEAPONMASTER_POLEARMS)) ||
            (weaponmaster_is_(WEAPONMASTER_STAVES)) ||
            (weaponmaster_is_(WEAPONMASTER_CLUBS)) ||
            (weaponmaster_is_(WEAPONMASTER_AXES)))
        result = 100 + (result - 100)*4/5;
    }

    if (result > info_ptr->blows_calc.max)
        result = info_ptr->blows_calc.max;

    if (result < 100)
        result = 100;

    return result;
}

static int _calc_innate_blows_aux(innate_attack_ptr a, int max, int str_idx, int dex_idx)
{
    int      result = 0;
    int      blow_str_idx;
    int      mul = 55, div = MAX(70, a->weight);
    _range_t rng = _blows_range[dex_idx];

    blow_str_idx = (adj_str_blow[str_idx] * mul / div);
    if (blow_str_idx > 110) blow_str_idx = 110;

    result = rng.min + (rng.max - rng.min) * blow_str_idx / 110;

    if (prace_is_(RACE_MON_LEPRECHAUN))
        result /= 2;

    if (prace_is_(RACE_MON_HYDRA))
        result *= 2;

    if (result < 100)
        result = 100;

    if (prace_is_(RACE_WEREWOLF))
        result -= ((result - 100) * 3 / 5);

    if (result > max)
        result = max;

    return result;
}

void calc_innate_blows(innate_attack_ptr a, int max)
{
    a->blows = _calc_innate_blows_aux(a, max, p_ptr->stat_ind[A_STR], p_ptr->stat_ind[A_DEX]);
}

#define _MAX_CHAOS_SLAYS 15

int _chaos_slays[_MAX_CHAOS_SLAYS] = {
    OF_SLAY_ANIMAL,
    OF_SLAY_EVIL,
    OF_SLAY_GOOD,
    OF_SLAY_UNDEAD,
    OF_SLAY_DEMON,
    OF_SLAY_ORC,
    OF_SLAY_TROLL,
    OF_SLAY_GIANT,
    OF_SLAY_DRAGON,
    OF_SLAY_HUMAN,
    OF_BRAND_POIS,
    OF_BRAND_ACID,
    OF_BRAND_ELEC,
    OF_BRAND_FIRE,
    OF_BRAND_COLD,
};

#define _SLAY_TIER_BASIC 1
#define _SLAY_TIER_MID 2
#define _SLAY_TIER_HIGH SLAY_TIER_MAX

static int _hissatsu_mult(int mult, int mode, int min)
{
    if (mode == HISSATSU_ELEC)
    {
        mult += 35; /* !! */
    }
    else
    {
        mult += ((mult - 10) / 2); /* 32 -> 43, 24 -> 31 */
        if (mult < min) mult = min;
    }
    return mult;
}

static bool _zammaken_mult(int *mult)
{
    if (*mult < 15)
    {
        *mult = 25;
        return TRUE;
    }
    else if (*mult < KILL_MULT_HIGH / 10)
    {
        *mult = MIN(KILL_MULT_HIGH / 10, *mult+(SLAY_MULT_BASIC / 10));
        return TRUE;
    }
    return FALSE;
}

static int _hissatsu_undead_mult(int mult, bool is_undead)
{
    if (is_undead)
    {
        if (mult == 10) mult = KILL_MULT_HIGH * 2 / 15;
        else if (mult < KILL_MULT_HIGH * 4 / 15) mult = MIN(KILL_MULT_HIGH * 4 / 15, mult + (KILL_MULT_HIGH / 9));
    }
    else
    {
        if (mult == 10) mult = KILL_MULT_HIGH * 2 / 25;
        else if (mult < KILL_MULT_HIGH * 4 / 15) mult = MIN(KILL_MULT_HIGH * 4 / 15, mult + (KILL_MULT_HIGH / 18));
    }
    return mult;
}

static int _sekiryuka_mult(int mult)
{
    int tmp = MIN(SLAY_MULT_BASIC / 2, MAX((p_ptr->cut + 300) / 10, p_ptr->cut / 5));
    return MAX(tmp, mult);
}

static int _hagan_mult(int mult)
{
    if (mult == 10) mult = KILL_MULT_MID / 10;
    else if (mult < KILL_MULT_HIGH / 8) mult = KILL_MULT_HIGH / 8;
    return mult;
}

static bool _can_slay_animal(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_ANIMAL);
    }
    mon_lore_3(m_ptr, RF3_ANIMAL);
    return TRUE;
}

static bool _can_slay_evil(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_EVIL);
    }
    mon_lore_3(m_ptr, RF3_EVIL);
    return TRUE;
}

static bool _can_slay_good(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_GOOD);
    }
    mon_lore_3(m_ptr, RF3_GOOD);
    return TRUE;
}

static bool _can_slay_living(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore) return monster_living(r_ptr);
    return TRUE;
}

static bool _can_slay_humans(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags2 & RF2_HUMAN);
    }
    mon_lore_2(m_ptr, RF2_HUMAN);
    return TRUE;
}

static bool _can_slay_undead(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_UNDEAD);
    }
    mon_lore_3(m_ptr, RF3_UNDEAD);
    return TRUE;
}

static bool _can_slay_demons(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_DEMON);
    }
    mon_lore_3(m_ptr, RF3_DEMON);
    return TRUE;
}

static bool _can_slay_orcs(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_ORC);
    }
    mon_lore_3(m_ptr, RF3_ORC);
    return TRUE;
}

static bool _can_slay_trolls(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_TROLL);
    }
    mon_lore_3(m_ptr, RF3_TROLL);
    return TRUE;
}

static bool _can_slay_giants(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_GIANT);
    }
    mon_lore_3(m_ptr, RF3_GIANT);
    return TRUE;
}

static bool _can_slay_dragons(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return BOOL(r_ptr->flags3 & RF3_DRAGON);
    }
    mon_lore_3(m_ptr, RF3_DRAGON);
    return TRUE;
}

static bool _can_slay_acid(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_IM_ACID_MASK) mon_lore_r(m_ptr, RFR_EFF_IM_ACID_MASK);
    return TRUE;
}

static bool _can_slay_elec(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK) mon_lore_r(m_ptr, RFR_EFF_IM_ACID_MASK);
    return TRUE;
}

static bool _can_slay_fire(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK) mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
    return TRUE;
}

static bool _can_slay_cold(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK) mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
    return TRUE;
}

static bool _can_slay_pois(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_IM_POIS_MASK) mon_lore_r(m_ptr, RFR_EFF_IM_POIS_MASK);
    return TRUE;
}

static bool _can_slay_dark(monster_race *r_ptr, monster_type *m_ptr, bool update_lore)
{
    if (!update_lore)
    {
        return (r_ptr->flagsr & RFR_EFF_RES_DARK_MASK) ? FALSE : TRUE;
    }
    if (r_ptr->flagsr & RFR_EFF_RES_DARK_MASK) mon_lore_r(m_ptr, RFR_EFF_RES_DARK_MASK);
    return TRUE;
}

slay_type slay_list[] =
/* The ordering is historical and definitely not in any way optimized, it has
 * so far been retained for consistent ordering on the character sheet */
{
    { _SLAY_TIER_MID, OF_KILL_ANIMAL, OF_SLAY_ANIMAL, 0, TRUE, _can_slay_animal, 'g', "animals", "" },
    { _SLAY_TIER_BASIC, OF_KILL_EVIL, OF_SLAY_EVIL, 0, TRUE, _can_slay_evil, 'y', "evil", "" },
    { _SLAY_TIER_BASIC, OF_KILL_GOOD, OF_SLAY_GOOD, 0, TRUE, _can_slay_good, 'W', "good", "" },
    { _SLAY_TIER_BASIC, OF_KILL_LIVING, OF_SLAY_LIVING, 0, TRUE, _can_slay_living, 'o', "living", "" },
    { _SLAY_TIER_MID, OF_KILL_HUMAN, OF_SLAY_HUMAN, 0, TRUE, _can_slay_humans, 's', "humans", "" },
    { _SLAY_TIER_HIGH, OF_KILL_UNDEAD, OF_SLAY_UNDEAD, 0, TRUE, _can_slay_undead, 'D', "undead", "" },
    { _SLAY_TIER_HIGH, OF_KILL_DEMON, OF_SLAY_DEMON, 0, TRUE, _can_slay_demons, 'R', "demons", "" },
    { _SLAY_TIER_HIGH, OF_KILL_ORC, OF_SLAY_ORC, 0, TRUE, _can_slay_orcs, 'U', "orcs", "" },
    { _SLAY_TIER_HIGH, OF_KILL_TROLL, OF_SLAY_TROLL, 0, TRUE, _can_slay_trolls, 'g', "trolls", "" },
    { _SLAY_TIER_HIGH, OF_KILL_GIANT, OF_SLAY_GIANT, 0, TRUE, _can_slay_giants, 'u', "giants", "" },
    { _SLAY_TIER_HIGH, OF_KILL_DRAGON, OF_SLAY_DRAGON, 0, TRUE, _can_slay_dragons, 'r', "dragons", "" },
    { _SLAY_TIER_MID, 0, OF_BRAND_ACID, 0, FALSE, _can_slay_acid, 'g', "acid", "is <color:g>Acid Branded</color>" },
    { _SLAY_TIER_MID, 0, OF_BRAND_ELEC, HISSATSU_ELEC, FALSE, _can_slay_elec, 'b', "electricity", "is <color:b>Lightning Branded</color>" },
    { _SLAY_TIER_MID, 0, OF_BRAND_FIRE, HISSATSU_FIRE, FALSE, _can_slay_fire, 'r', "fire", "has <color:r>Flame Tongue</color>" },
    { _SLAY_TIER_MID, 0, OF_BRAND_COLD, HISSATSU_COLD, FALSE, _can_slay_cold, 'W', "frost", "is <color:W>Frost Branded</color>" },
    { _SLAY_TIER_MID, 0, OF_BRAND_POIS, HISSATSU_POISON, FALSE, _can_slay_pois, 'G', "poison", "has <color:G>Viper's Fang</color>" },
    { _SLAY_TIER_MID, 0, OF_BRAND_DARK, 0, FALSE, _can_slay_dark, 'G', "dark", "has <color:G>Shadow Sweep</color>" },
    { 0, 0, 0, 0, FALSE, NULL, 0, "", "" },
};

slay_tier slay_tiers[SLAY_TIER_MAX] =
{
    { KILL_MULT_BASIC / 8, KILL_MULT_BASIC / 10, SLAY_MULT_BASIC * 2 / 15, SLAY_MULT_BASIC / 10, 186, 143 },
    { KILL_MULT_MID / 8, KILL_MULT_MID / 10, SLAY_MULT_MID * 2 / 15, SLAY_MULT_MID / 10, 234, 162 },
    { KILL_MULT_HIGH / 8, KILL_MULT_HIGH / 10, SLAY_MULT_HIGH * 2 / 15, SLAY_MULT_HIGH / 10, 280, 190 },
};

s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s16b hand, int mode, bool thrown)
{
    int mult = 10;

    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int chaos_slay = 0;

    u32b flgs[OF_ARRAY_SIZE] = {0};
    char o_name[MAX_NLEN];

    /* Extract the flags */
    if (thrown)
        obj_flags(o_ptr, flgs);
    else
    {
        weapon_flags(hand, flgs);
        switch (mode)
        {
        case DRACONIAN_STRIKE_ACID:
            add_flag(flgs, OF_BRAND_ACID);
            break;
        case DRACONIAN_STRIKE_ELEC:
            add_flag(flgs, OF_BRAND_ELEC);
            break;
        case DRACONIAN_STRIKE_FIRE:
            add_flag(flgs, OF_BRAND_FIRE);
            break;
        case DRACONIAN_STRIKE_COLD:
            add_flag(flgs, OF_BRAND_COLD);
            break;
        case DRACONIAN_STRIKE_POIS:
            add_flag(flgs, OF_BRAND_POIS);
            break;
        case PY_ATTACK_MANA:
            add_flag(flgs, OF_BRAND_MANA);
            break;
        }
    }
    /* Chaos Weapons now have random slay effects, and the slay so
       chosen will augment any existing slay of the same type. */
    if (have_flag(flgs, OF_BRAND_CHAOS))
    {
        chaos_slay = _chaos_slays[randint0(_MAX_CHAOS_SLAYS)];
        object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_OMIT_PREFIX | OD_COLOR_CODED);
    }

    /* Hex swords slay good (make this not be weirdly separate from normal slay good) */
    if (hex_spelling(HEX_RUNESWORD))
    {
        add_flag(flgs, OF_SLAY_GOOD);
    }

    if (p_ptr->tim_blood_seek)
    {
        add_flag(flgs, OF_SLAY_LIVING);
    }

    if (weaponmaster_get_toggle() == TOGGLE_HOLY_BLADE)
    {
        add_flag(flgs, OF_SLAY_EVIL);
    }

    if (mode == PY_ATTACK_ACID)
    {
        add_flag(flgs, OF_BRAND_ACID);
    }

    /* Some "weapons" and "ammo" do extra damage */
    switch (o_ptr->tval)
    {
        case TV_SHOT:   /* FYI for the curious: You may throw (v) a shot by hand! */
        case TV_ARROW:  /* But, for normal shooting, see tot_dam_aux_shot() in cmd2.c */
        case TV_BOLT:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:
        case TV_GLOVES:
        {
            int hissatsu_brand = 0;
            int i;
            char m_name_subject[MAX_NLEN];
            slay_type _slay = slay_list[0];
            monster_desc(m_name_subject, m_ptr, MD_PRON_VISIBLE);

            for (i = 0;; i++)
            {
                int my_mult = 10;
                bool _chaos = FALSE;
                int tmp_hb = 0;
                _slay = slay_list[i];
                if (!_slay.tier) break; /* the only exit from this loop! */
                if (!_slay.tester(r_ptr, m_ptr, FALSE)) continue;
                if (chaos_slay == _slay.slay_flag)
                {
                    msg_format("Your %s %s %s.", o_name, _slay.is_slay ? "slays" : "is covered in", _slay.kill_desc);
                    obj_learn_slay(o_ptr, OF_BRAND_CHAOS, "has the <color:v>Mark of Chaos</color>");
                    _chaos = TRUE;
                }
                if ((_slay.kill_flag > 0) && (have_flag(flgs, _slay.kill_flag)))
                {
                    char oppi[80];
                    my_mult = _chaos ? slay_tiers[_slay.tier - 1].kill_with_chaos : slay_tiers[_slay.tier - 1].kill;
                    if ((o_ptr->name1 == ART_NOTHUNG) && (m_ptr->r_idx == MON_FAFNER) && (_slay.kill_flag == OF_KILL_DRAGON))
                        my_mult *= 3;
                    strcpy(oppi, format("slays <color:%c>*%^s*</color>", _slay.attr, _slay.kill_desc)); /* Assume no brand */
                    obj_learn_slay(o_ptr, _slay.kill_flag, oppi);
                }
                else if (have_flag(flgs, _slay.slay_flag))
                {
                    my_mult = _chaos ? slay_tiers[_slay.tier - 1].slay_with_chaos : slay_tiers[_slay.tier - 1].slay;
                    if (!_slay.is_slay) obj_learn_slay(o_ptr, _slay.slay_flag, _slay.brand_learn);
                    else
                    {
                        char oppi[80];
                        strcpy(oppi, format("slays <color:%c>%^s</color>", _slay.attr, _slay.kill_desc));
                        obj_learn_slay(o_ptr, _slay.slay_flag, oppi);
                    }
                }
                else if (_chaos)
                {
                    my_mult = slay_tiers[_slay.tier - 1].slay;
                }
                if ((_slay.hissatsu) && (mode == _slay.hissatsu))
                {
                    my_mult = _hissatsu_mult(my_mult, mode, slay_tiers[_slay.tier - 1].slay);
                    tmp_hb = mode;
                }
                if (my_mult > 10) /* Hack - monster vulnerabilities */
                {
                    if (_slay.slay_flag == OF_BRAND_FIRE)
                    {
                        if (r_ptr->flags3 & RF3_HURT_FIRE)
                        {
                            my_mult *= 2;
                            mon_lore_3(m_ptr, RF3_HURT_FIRE);
                        }
                    }
                    else if (_slay.slay_flag == OF_BRAND_COLD)
                    {
                        if (r_ptr->flags3 & RF3_HURT_COLD)
                        {
                            my_mult *= 2;
                            mon_lore_3(m_ptr, RF3_HURT_COLD);
                        }
                    }
                }
                if (my_mult > mult)
                {
                    mult = my_mult;
                    if (tmp_hb) hissatsu_brand = tmp_hb;
                    else hissatsu_brand = 0;
                }
                if (my_mult > 10) _slay.tester(r_ptr, m_ptr, TRUE);
            }

            /* 'light brand' */
            if (have_flag(flgs, OF_LITE) && (r_ptr->flags3 & RF3_HURT_LITE))
            {
            	if (mult < KILL_MULT_HIGH / 8) msg_format("%^s cringes.", m_name_subject);
            	if (mult == 10) mult = SLAY_MULT_BASIC / 10;
            	else if (mult < KILL_MULT_HIGH / 8) mult = MIN(KILL_MULT_HIGH / 8, mult + 10);
            }

            if ((mode == HISSATSU_ZANMA) && !monster_living(r_ptr) && (r_ptr->flags3 & RF3_EVIL))
            {
                if (_zammaken_mult(&mult))
                {
                    hissatsu_brand = OF_SLAY_EVIL;
                    mon_lore_3(m_ptr, RF3_EVIL);
                }
            }
            if (mode == HISSATSU_UNDEAD) /* Intentionally hurts non-undead */
            {
                if (r_ptr->flags3 & RF3_UNDEAD)
                {
                    mon_lore_3(m_ptr, RF3_UNDEAD);
                    mult = _hissatsu_undead_mult(mult, TRUE);
                }
                else mult = _hissatsu_undead_mult(mult, FALSE);
                hissatsu_brand = OF_SLAY_UNDEAD; /* Assume this brand always improves damage */
            }
            if ((mode == HISSATSU_SEKIRYUKA) && p_ptr->cut && monster_living(r_ptr))
            {
                mult = _sekiryuka_mult(mult);
            }
            if ((mode == HISSATSU_HAGAN) && (r_ptr->flags3 & RF3_HURT_ROCK))
            {
                mon_lore_3(m_ptr, RF3_HURT_ROCK);
                mult = _hagan_mult(mult);
                hissatsu_brand = 0;
            }
            if (p_ptr->tim_slay_sentient && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
            {
                if (r_ptr->flags3 & RF3_NO_STUN)
                {
                    mon_lore_3(m_ptr, RF3_NO_STUN);
                }
                else
                {
                    if (mult < SLAY_MULT_BASIC / 10) mult = SLAY_MULT_BASIC / 10;
                }
            }

            if (hissatsu_brand)
            {
                switch (hissatsu_brand)
                {
                case OF_BRAND_ELEC:
                        msg_format("%^s is <color:b>shocked</color>!", m_name_subject);
                        break;
                case OF_BRAND_ACID:
                        msg_format("%^s is <color:g>dissolved</color>!", m_name_subject);
                        break;
                case OF_BRAND_FIRE:
                        msg_format("%^s is <color:r>burned</color>!", m_name_subject);
                        break;
                case OF_BRAND_COLD:
                        msg_format("%^s is <color:W>frozen</color>!", m_name_subject);
                        break;
                case OF_BRAND_POIS:
                        msg_format("%^s is <color:G>poisoned</color>!", m_name_subject);
                        break;
                default: break;
                /* Messages from Composband - possible future use
                        msg_format("It howls!");
                        msg_format("It wails!");
                        msg_format("It screeches!");
                        msg_format("It convulses!");
                        msg_format("It shrieks!");
                        msg_format("It cowers!");
                        msg_format("It cringes.");
                        msg_format("It winces.");
                        msg_format("It recoils.");
                        msg_format("It staggers.");
                        msg_format("It groans.");
                        msg_format("It shudders."); */
                }
            }

            if ((have_flag(flgs, OF_BRAND_MANA) || p_ptr->tim_force) && (!elemental_is_(ELEMENTAL_WATER)))
            {
                int          cost = 0;
                int          dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
                int          ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;
                caster_info *caster = get_caster_info();

                if (p_ptr->pclass == CLASS_SAMURAI)
                    cost = (1 + (dd * ds * 2 / 7));
                else
                    cost = (1 + (dd * ds / 7));

                if (thrown)
                    cost *= 3;

                if (caster && (caster->options & CASTER_USE_AU))
                {
                    cost *= 10;
                    if (p_ptr->au >= cost)
                    {
                        p_ptr->au -= cost;
                        stats_on_gold_services(cost); /* ? */
                        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
                        p_ptr->redraw |= (PR_GOLD);

                        mult = mult * 3 / 2 + 14;
                        obj_learn_slay(o_ptr, OF_BRAND_MANA, "is <color:B>Mana Branded</color>");
                    }
                }
                else if (p_ptr->csp >= cost)
                {
                    p_ptr->csp -= cost;
                    p_ptr->redraw |= (PR_MANA);
                    mult = mult * 3 / 2 + 14;
                    obj_learn_slay(o_ptr, OF_BRAND_MANA, "is <color:B>Mana Branded</color>");
                }
            }
            if (p_ptr->tim_blood_feast)
            {
                take_hit(DAMAGE_ATTACK, 15, "blood feast");
            }
            break;
        }
    }
    if (mult > 150) mult = 150;

//    msg_format("Mult: %d", mult);

    /* Return the total damage */
    return (tdam * mult / 10);
}

static int _critical_loop = 0;
static int _critical_attempts = 0;
static int _critical_roll = 0;
static int _critical_comp = 0;

/*
 * Critical hits (from bows/crossbows/slings)
 * Factor in item weight, total plusses, and player level.
 */
critical_t critical_shot(int weight, int plus)
{
    critical_t result = {0};
    int i, k;

    /* Extract "shot" power */
    i = (p_ptr->shooter_info.to_h + plus) * 3 + p_ptr->skills.thb * 2;

    /* Snipers and Crossbowmasters get more crits */
    if (p_ptr->concent) i += i * p_ptr->concent / 10;
    if (p_ptr->pclass == CLASS_SNIPER &&
        ((p_ptr->shooter_info.tval_ammo == TV_BOLT) ||
        (p_ptr->shooter_info.tval_ammo == TV_ANY_AMMO))) i = i * 3 / 2;
    if (weaponmaster_get_toggle() == TOGGLE_CAREFUL_AIM)
        i *= 3;
    if (p_ptr->pclass == CLASS_ARCHER) i += i * p_ptr->lev / 100;

    /* Critical hit */
    if (randint1(5000) <= i)
    {
        k = weight * randint1(500);
        result.mul = 150 + k * 200 / 2000;

        if (result.mul < 200)
            result.desc = "It was a <color:y>decent</color> shot!";
        else if (result.mul < 240)
            result.desc = "It was a <color:R>good</color> shot!";
        else if (result.mul < 270)
            result.desc = "It was a <color:r>great</color> shot!";
        else if (result.mul < 300)
            result.desc = "It was a <color:v>superb</color> shot!";
        else
            result.desc = "It was a <color:v>*GREAT*</color> shot!";
    }

    return result;
}

/*
 * Critical hits (from bows/crossbows/slings)
 * Factor in item weight, total plusses, and player level.
 */
critical_t critical_throw(int weight, int plus)
{
    critical_t result = {0};
    int i, k;

    /* Extract "shot" power */
    i = (p_ptr->shooter_info.to_h + plus)*4 + p_ptr->lev*3;

    /* Critical hit */
    if (randint1(5000) <= i)
    {
        k = weight + randint1(650);

        if (k < 400)
        {
            result.desc = "It was a <color:y>good</color> hit!";
            result.mul = 150;
        }
        else if (k < 700)
        {
            result.desc = "It was a <color:R>great</color> hit!";
            result.mul = 200;
        }
        else
        {
            result.desc = "It was a <color:r>superb</color> hit!";
            result.mul = 250;
        }
    }

    return result;
}

static bool _always_crit(int mode)
{
    if ( mode == HISSATSU_MAJIN
      || mode == HISSATSU_3DAN
      || mode == MAULER_CRITICAL_BLOW
      || mode == GOLEM_BIG_PUNCH
      || mode == MYSTIC_CRITICAL)
        return TRUE;
    else return FALSE;
}

static void _initialize_crit_loop(int mode)
{
    if ( mode == HISSATSU_MAJIN
      || mode == HISSATSU_3DAN
      || mode == MAULER_CRITICAL_BLOW) /* Extra randomness */
    {
        _critical_loop = -30;
    }
    else _critical_loop = -2; /* No randomness, why waste time */
    _critical_attempts = 0;
    _critical_roll = 0;
    _critical_comp = 0;
}

/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
critical_t critical_norm(int weight, int plus, s16b meichuu, int mode, int hand)
{
    critical_t result = {0};
    int i;
    int roll = (player_is_ninja) ? 4444 : 5000;
    int quality = 650;
    static int next_k = 0;

    if (p_ptr->enhanced_crit)
    {
        weight = weight * 3 / 2;
        weight += 300;
    }

    if ( equip_is_valid_hand(hand)
      && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS
      && p_ptr->pclass != CLASS_DUELIST
      && !p_ptr->weapon_info[hand].omoi )
    {
        roll = roll * 4 / 5;
    }

    /* Extract "blow" power */
    i = (weight + (meichuu * 3 + plus * 5) + (p_ptr->lev * 3));

    /* Mauler: Destroyer now scales with level */
    if ( p_ptr->pclass == CLASS_MAULER
      && equip_is_valid_hand(hand)
      && p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS )
    {
        int pct = MIN((weight - 200)/20, 20);
        if (pct > 0)
            pct = pct * p_ptr->lev / 50;
        i += roll * pct / 100;
        quality += quality * pct / 100;
    }

    if (_critical_loop < 0) /* Hack */
    {
        _critical_loop = 0 - _critical_loop;
        _critical_comp = i;
        _critical_roll = _always_crit(mode) ? i : MAX(i, roll);
        next_k = quality;
    }

    /* Chance */
    if ( _always_crit(mode)
      || _critical_loop > 0
      || randint1(roll) <= i )
    {
        int k;
        if (_critical_loop)
        {
            k = weight + next_k;
            next_k--;
            if (next_k <= 0)
            {
                next_k = quality;
                _critical_loop--;
                if (!_critical_loop) return result;
            }
            _critical_attempts++;
        }
        else k = weight + randint1(quality);

        if ( mode == HISSATSU_MAJIN
          || mode == HISSATSU_3DAN )
        {
            k += randint1(650);
        }
        if (mode == MAULER_CRITICAL_BLOW)
        {
            k += randint1(250*p_ptr->lev/50);
        }

        if (k < 400)
        {
            result.desc = "It was a <color:y>good</color> hit!";
            result.mul = 200;
        }
        else if (k < 700)
        {
            result.desc = "It was a <color:R>great</color> hit!";
            result.mul = 250;
        }
        else if (k < 900)
        {
            result.desc = "It was a <color:r>superb</color> hit!";
            result.mul = 300;
        }
        else if (k < 1300)
        {
            result.desc = "It was a <color:v>*GREAT*</color> hit!";
            result.mul = 350;
        }
        else
        {
            result.desc = "It was a <color:v>*SUPERB*</color> hit!";
            result.mul = 400;
        }
    }

    /* Golem criticals are too strong */
    if (prace_is_(RACE_MON_GOLEM) && (result.mul > 100))
    {
        result.mul -= ((result.mul - 100) / 3);
    }

    return result;
}

/**********************************************************************
 * Display Weapon Information to the Player
 **********************************************************************/
int display_weapon_mode = 0;

static void _display_weapon_slay(int base_mult, int slay_mult, bool force, int blows,
                                 int dd, int ds, int to_d, cptr name, int color, doc_ptr doc)
{
    int mult, min, max, div = 100;

    mult = MIN(slay_mult, 1500);
    if (force)
        mult = mult * 3/2 + 140;
    mult = mult * base_mult / 100;

    if (display_weapon_mode == HISSATSU_SUTEMI || display_weapon_mode == HISSATSU_3DAN) div /= 2;
    else if (display_weapon_mode == HISSATSU_SEKIRYUKA && !p_ptr->cut) div *= 2;

    min = blows * (mult*dd/100 + to_d) / div;
    max = blows * (mult*dd*ds/100 + to_d) / div;

    if ((p_ptr->pclass == CLASS_DUELIST) && (!duelist_equip_error()))
    {
        int l1 = min, l2 = max, mahis = MAX(0, p_ptr->lev + adj_stat_save[p_ptr->stat_ind[A_DEX]]);
        if (p_ptr->lev >= 10)
        {
            min += l1;
            max += l2;
        }
        if ((p_ptr->lev >= 20) && (mahis))
        {
            min += (2L * l1 * mahis / (mahis + 125)); /* Worst-case scenario - Serpent */
            max += (2L * l2 * mahis / (mahis + 125));
        }
        if ((p_ptr->lev >= 40) && (mahis))
        {
            min += (8L * l1 * mahis / (mahis + 125) / 2); /* Worst-case scenario - Serpent */
            max += (8L * l2 * mahis / (mahis + 125) / 2);
        }
    }

    min = ((min * (class_melee_mult() * race_melee_mult(FALSE) / 100)) + 50) / 100;
    max = ((max * (class_melee_mult() * race_melee_mult(FALSE) / 100)) + 50) / 100;

    if (p_ptr->stun)
    {
        min -= min * MIN(100, p_ptr->stun) / 150;
        max -= max * MIN(100, p_ptr->stun) / 150;
    }
    if (weaponmaster_get_toggle() == TOGGLE_ORDER_BLADE)
        min = max;

    doc_printf(doc, "<color:%c> %-7.7s</color>", attr_to_attr_char(color), format("%^s", name));
    doc_printf(doc, ": %d [%d.%02dx]\n",
                    (min + max)/2,
                    mult/100, mult%100);
}

/* Certified 100% fake math
 * Do not take any mathematics lessons here
 * This is not remotely even the most accurate way to do fake fractional
 * exponentiation with integers, but it combines speed, simplicity
 * and reasonable accuracy, and limits the maximum error rather than the
 * average error since small errors here are drowned out by rounding
 * elsewhere
 * -- base is probability in parts per 10000, exponent is scaled by 100
 * -- returns probability (base/10000)^exponent, in parts per 10000 */
static s32b _fake_fractional_power(int base, s32b exponent)
{
    s32b t = base * 10;
    s32b c = 10000 - base;
    s32b u = 0;
    s32b m = 0;
    s32b div = 0;
    if (t >= 100000) return 10000;
    if (exponent <= 100) return base;
    exponent -= 100;
    while (exponent >= 100)
    {
        t = (t * base) + 5000L;
        t /= 10000L;
        if (!t) return 0;
        exponent -= 100;
    }

    if (exponent != 0)
    {
        div = 450000L / (450L - (base / 33)); /* Do not ask */
        u = exponent * 1000L + ((c * exponent * (100 - exponent) + (div / 2)) / div);
    }
    m = (c * u + 50000L) / 100000L;
    if (m != 0)
    {
        t *= (10000L - m);
        t += 5000L;
        t /= 10000L;
    }
    t = (t + 5) / 10;
/*    msg_format("Base: %d Blows: %d Tulos: %d Div: %d", base, exponent, t, div);
    (void)inkey();*/
    return t;
}

void display_weapon_info(doc_ptr doc, int hand)
{
    object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
    char o_name[MAX_NLEN];
    u32b flgs[OF_ARRAY_SIZE];
    int dd;
    int ds;
    int to_d = 0;
    int to_h = 0;
    int mult;
    critical_t crit = {0};
    int crit_pct = 0;
    int num_blow = NUM_BLOWS(hand);
    bool force = FALSE;
    doc_ptr cols[2] = {0};
    int i;
    int norm_mult = 100;
    bool loytyi = FALSE;
    slay_type _slay;

    if (p_ptr->weapon_info[hand].wield_how == WIELD_NONE) return;
    if (!o_ptr) return;
    if (no_melee_challenge) return;

    dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
    ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;
    if (object_is_known(o_ptr))
    {
        to_d = o_ptr->to_d;
        to_h = o_ptr->to_h;
    }

    switch (display_weapon_mode)
    {
    case MAULER_CRITICAL_BLOW:
        if (num_blow > 100)
            num_blow = 100 + (num_blow - 100) / 2;
        break;
    case MAULER_STUNNING_BLOW:
    case MAULER_CRUSHING_BLOW:
    case MAULER_KNOCKBACK:
        num_blow = 100;
        break;
    case PY_POWER_ATTACK:
        to_h += 10;
        to_d += p_ptr->lev / 2;
        break;
    case HISSATSU_UNDEAD:
        norm_mult = 360;
        break;
    case HISSATSU_ZANMA:
        norm_mult = 250;
        break;
    case HISSATSU_COLD:
        num_blow += 200;
        break;
    case HISSATSU_3DAN:
        num_blow = 300;
        break;
    }

    if ((p_ptr->pclass == CLASS_DUELIST) && (!duelist_equip_error())) to_h += p_ptr->lev;

    weapon_flags_known(hand, flgs);
    if ((have_flag(flgs, OF_BRAND_MANA) || p_ptr->tim_force) && (!elemental_is_(ELEMENTAL_WATER)))
    {
        caster_info *caster = get_caster_info();
        int          cost = 0;

        if (p_ptr->pclass == CLASS_SAMURAI)
            cost = (1 + (dd * ds * 2 / 7));
        else
            cost = (1 + (dd * ds / 7));

        if (caster && (caster->options & CASTER_USE_AU))
        {
            cost *= 10;
            if (p_ptr->au >= cost)
                force = TRUE;
        }
        else if (p_ptr->csp >= cost)
            force = TRUE;
    }

    if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH && object_is_shield(o_ptr))
    {
        dd = 3 + p_ptr->weapon_info[hand].to_dd;
        ds = o_ptr->ac + p_ptr->weapon_info[hand].to_ds;
        if (object_is_known(o_ptr))
        {
            to_h = o_ptr->to_a;
            to_d = o_ptr->to_a;
            to_h += 2*o_ptr->to_h;
            to_d += 2*o_ptr->to_d;
        }
    }

    mult = 100;
    if (have_flag(flgs, OF_VORPAL2))
        mult = mult * 5 / 3;  /* 1 + 1/3(1 + 1/2 + ...) = 1.667x */
    else if (have_flag(flgs, OF_VORPAL) && p_ptr->vorpal)
        mult = mult * 11 / 8; /* 1 + 1/4(1 + 1/3 + ...) = 1.375x */
    else if (have_flag(flgs, OF_VORPAL) || p_ptr->vorpal)
        mult = mult * 11 / 9; /* 1 + 1/6(1 + 1/4 + ...) = 1.222x */

    if (display_weapon_mode == MAULER_CRUSHING_BLOW)
        mult = mult * NUM_BLOWS(hand) / 50;

    if (!have_flag(flgs, OF_BRAND_ORDER)
        && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE)
    {
/*        const int attempts = 10 * 1000;*/
        int crits = 0;
        /* Compute average effects of criticals by sampling
         * Try to get full sample if possible */
        _initialize_crit_loop(display_weapon_mode);
        while (1)
        {
            critical_t tmp = critical_norm(o_ptr->weight, to_h, p_ptr->weapon_info[hand].to_h, display_weapon_mode, hand);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
                crits++;
            }
            else if (!_critical_loop) break;
            else crit.mul += 100;
        }
        if ((!_critical_attempts) || (((_critical_roll < 1) || (_critical_comp < 1)) && (!_always_crit(display_weapon_mode))))
            /* Something has gone horribly wrong, or the numbers are negative/zero... */
        {
            crit.mul = 100;
            crit.to_d = 0;
            crit_pct = 0;
        }
        else /* Evil voodoo */
        {
            /* Fake math for the human crit-limiting mut. (All the other
             * math here is real, just incredibly ugly...) We could use
             * real math if we were willing to use non-integers, but we can
             * get close enough using integers that rounding errors already
             * present in any case are larger than the math errors */
            if ((mut_present(MUT_HUMAN_STR)) && (num_blow > 100) && (crits))
            {
                /* Store probabilities in parts per 10000 */
                s32b orig_crit_prob = (_critical_comp * 10000 + (_critical_roll / 2)) / _critical_roll;
                s32b crit_round_prob = 10000 - _fake_fractional_power(10000 - orig_crit_prob, num_blow);
                s32b real_crit_prob = (crit_round_prob * 100 + (num_blow / 2)) / num_blow;
                if (real_crit_prob < orig_crit_prob)
                {
                    _critical_comp = (_critical_comp * real_crit_prob * 2) / orig_crit_prob + 1;
                    _critical_roll *= 2;
                }
            }
            if (_critical_roll != _critical_comp)
            {
                crit.mul -= (_critical_attempts * 100);
                if (((0x7FFFFFFF - (_critical_roll / 2)) / _critical_comp) < crit.mul)
                {
                    crit.mul = (crit.mul * _critical_comp + (_critical_roll / 2)) / _critical_roll;
                }
                else crit.mul = (crit.mul + (_critical_roll / 2)) / _critical_roll * _critical_comp;
                crit.mul += (_critical_attempts * 201) / 2;
                crit.mul /= _critical_attempts;
                crit.to_d = (crit.to_d * 100 + (_critical_roll / 2)) / _critical_roll * _critical_comp / _critical_attempts;
                crit_pct = (_critical_comp * 1000 + (_critical_roll / 2)) / _critical_roll;
            }
            else
            {
                crit.mul = (crit.mul + (_critical_attempts / 2)) / _critical_attempts;
                crit.to_d = (crit.to_d * 100 + (_critical_attempts / 2)) / _critical_attempts;
                crit_pct = 1000;
            }
        }
    }
    else
        crit.mul = 100;


    /* Display in 2 columns, side by side */
    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    /* Column #1 */
    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_AND_ENCHANT);
    if (prace_is_(RACE_MON_SWORD))
        doc_printf(cols[0], "<color:y> You    :</color> <indent><style:indent>%s</style></indent>\n", o_name);
    else
        doc_printf(cols[0], "<color:y> Hand #%d:</color> <indent><style:indent>%s</style></indent>\n", hand+1, o_name);

    doc_printf(cols[0], " %-7.7s: %d.%d lbs\n", "Weight", o_ptr->weight/10, o_ptr->weight%10);

    if (object_is_(o_ptr, TV_SWORD, SV_POISON_NEEDLE)) /* special case */
    {
        doc_insert(cols[0], " Blows  : 1.00\n");
        doc_insert(cols[0], " Damage : 1\n");
        doc_insert(cols[1], "<color:G>Accuracy</color>\n");
        doc_printf(cols[1], "  %d%%", (1000 / MAX(1, p_ptr->weapon_ct) + 5) / 10);
        doc_insert_cols(doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);
        return;
    }

    if (weaponmaster_get_toggle() == TOGGLE_SHIELD_BASH)
    {
        assert(o_ptr->tval == TV_SHIELD);
        doc_printf(cols[0], " %-7.7s: %dd%d (%+d,%+d)\n", "Bash", dd, ds, to_h, to_d);
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_shield_describe_current(o_ptr->sval),
                    skills_shield_calc_bonus(o_ptr->sval));
    }
    else
    {
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_weapon_describe_current(o_ptr->tval, o_ptr->sval),
                    skills_weapon_calc_bonus(o_ptr->tval, o_ptr->sval));
    }
    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Hit", to_h, p_ptr->weapon_info[hand].to_h, to_h + p_ptr->weapon_info[hand].to_h);
    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Dam", to_d, p_ptr->weapon_info[hand].to_d, to_d + p_ptr->weapon_info[hand].to_d);
    doc_printf(cols[0], " %-7.7s: %d.%2.2d\n", "Blows", num_blow/100, num_blow%100);

    if (p_ptr->weapon_info[hand].dual_wield_pct < 1000)
    {
        doc_printf(cols[0], " %-7.7s: %d.%d%%\n", "Skill",
            p_ptr->weapon_info[hand].dual_wield_pct/ 10,
            p_ptr->weapon_info[hand].dual_wield_pct % 10);
    }

    mult = mult * crit.mul / 100;
    to_d = to_d + crit.to_d/100 + p_ptr->weapon_info[hand].to_d;

    doc_printf(cols[0], "<color:G> %-7.7s</color>\n", "Damage");

    if (!have_flag(flgs, OF_BRAND_ORDER)
        && weaponmaster_get_toggle() != TOGGLE_ORDER_BLADE)
    {
        if (crit.to_d)
        {
            doc_printf(cols[0], " %-7.7s: %d.%02dx + %d.%02d\n", "Crits",
                            crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
        }
        else
        {
            doc_printf(cols[0], " %-7.7s: %d.%02dx (%d.%d%%)\n", "Crits",
                            crit.mul/100, crit.mul%100, crit_pct / 10, crit_pct % 10);
        }
    }

    _display_weapon_slay(mult, norm_mult, FALSE, num_blow, dd, ds, to_d, "Normal", TERM_WHITE, cols[0]);
    if (force)
        _display_weapon_slay(mult, norm_mult, force, num_blow, dd, ds, to_d, "Force", TERM_L_BLUE, cols[0]);
    if (p_ptr->tim_slay_sentient)
        _display_weapon_slay(mult, SLAY_MULT_BASIC, force, num_blow, dd, ds, to_d, "Sent.", TERM_YELLOW, cols[0]);

    i = 0;
    
    for (_slay = slay_list[0];; _slay = slay_list[++i])
    {
        int _slay_mult = 10;
        if (!_slay.tier) break;
        if ((display_weapon_mode == HISSATSU_ZANMA) && (_slay.slay_flag == OF_SLAY_LIVING)) continue;
        if ((display_weapon_mode == HISSATSU_SEKIRYUKA) && ((_slay.slay_flag == OF_SLAY_DEMON) || (_slay.slay_flag == OF_SLAY_UNDEAD))) continue;
        if ((_slay.kill_flag) && (have_flag(flgs, _slay.kill_flag)))
            _slay_mult = slay_tiers[_slay.tier - 1].kill;
        else if (have_flag(flgs, _slay.slay_flag))
            _slay_mult = slay_tiers[_slay.tier - 1].slay;
        if ((_slay.hissatsu) && (_slay.hissatsu == display_weapon_mode))
        {
            _slay_mult = _hissatsu_mult(_slay_mult, display_weapon_mode, slay_tiers[_slay.tier - 1].slay);
        }
        if ((display_weapon_mode == HISSATSU_SEKIRYUKA) && (p_ptr->cut) && (_slay.slay_flag == OF_SLAY_LIVING))
            _slay_mult = _sekiryuka_mult(_slay_mult);
        if (_slay_mult != 10)
        {
            loytyi = TRUE;
            if (display_weapon_mode == HISSATSU_ZANMA) (void)_zammaken_mult(&_slay_mult);
            else if (display_weapon_mode == HISSATSU_UNDEAD) _slay_mult = _hissatsu_undead_mult(_slay_mult, (_slay.slay_flag == OF_SLAY_UNDEAD) ? TRUE : FALSE);
            _display_weapon_slay(mult, _slay_mult * 10, force, num_blow, dd, ds, to_d, (_slay.slay_flag == OF_BRAND_ELEC) ? "Elec" : _slay.kill_desc, _slay.is_slay ? TERM_YELLOW : TERM_RED, cols[0]);
        }
    }

    if (display_weapon_mode == HISSATSU_HAGAN)
    {
        _display_weapon_slay(mult, _hagan_mult(10) * 10, force, num_blow, dd, ds, to_d, "Rock", TERM_UMBER, cols[0]);
        if (loytyi) _display_weapon_slay(mult, _hagan_mult(11) * 10, force, num_blow, dd, ds, to_d, "Combine", TERM_UMBER, cols[0]);
    }

    if (display_weapon_mode == HISSATSU_ZANMA)
    {
        doc_insert(cols[0], " Only affects evil demons, evil undead and evil nonliving.\n");
    }

    if (p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
    {
        if (p_ptr->weapon_info[hand].omoi)
            doc_insert(cols[0], " Your weapon requires two hands to wield properly.\n");
    }

    if (p_ptr->weapon_info[hand].info)
    {
        byte a = p_ptr->weapon_info[hand].info_attr;
        if (!a) a = TERM_WHITE; /* uninitialized is TERM_DARK???! */
        doc_printf(cols[0], " <color:%c>%s</color>\n", attr_to_attr_char(a), p_ptr->weapon_info[hand].info);
    }

    /* Column #1 */
    doc_insert(cols[1], "<color:G>Accuracy</color>\n");
    doc_insert(cols[1], " AC Hit\n");

    doc_printf(cols[1], "%3d %2d%%\n", 25, hit_chance(hand, to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, hit_chance(hand, to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 75, hit_chance(hand, to_h, 75));
    doc_printf(cols[1], "%3d %2d%%\n", 100, hit_chance(hand, to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 125, hit_chance(hand, to_h, 125));
    doc_printf(cols[1], "%3d %2d%%\n", 150, hit_chance(hand, to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, hit_chance(hand, to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, hit_chance(hand, to_h, 200));

    /* Assemble the result */
    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

/**********************************************************************
 * Innate Attacks
 **********************************************************************/
static cptr _effect_name(int which)
{
    gf_info_ptr gf;
    if (p_ptr->current_r_idx == MON_AETHER_VORTEX)
        return "Random";

    switch (which)
    {
    case 0: case GF_MISSILE: return "Normal";
    case GF_ACID: return "Acid";
    case GF_ELEC: return "Elec";
    case GF_FIRE: return "Fire";
    case GF_COLD: return "Cold";
    case GF_POIS: return "Poison";
    case GF_NETHER: return "Nether";
    case GF_NEXUS: return "Nexus";
    case GF_LITE: return "Light";
    case GF_CHAOS: return "Chaos";
    case GF_SHARDS: return "Shards";
    case GF_DISINTEGRATE: return "Disint";
    case GF_DISENCHANT: return "Disench";
    case GF_TIME: return "Time";
    case GF_OLD_DRAIN: return "Drain";
    case GF_OLD_CONF: return "Confuse";
    case GF_CONFUSION: return "Confuse";
    case GF_STUN: return "Stun";
    case GF_DRAIN_MANA: return "Drain Mana";
    case GF_TURN_ALL: return "Terrify";
    }
    gf = gf_lookup(which);
    if (gf) return gf->name;
    return "Unknown";
}

void display_innate_attack_info(doc_ptr doc, int which)
{
    innate_attack_ptr a = &p_ptr->innate_attacks[which];
    int blows, min, max, min_base, max_base, min2, max2;
    int i;
    int to_h = p_ptr->to_h_m + a->to_h;
    int to_d = p_ptr->to_d_m + a->to_d;
    int dd = a->dd + p_ptr->innate_attack_info.to_dd;
    int mult;
    int strt = 1;
    doc_ptr cols[2] = {0};

    blows = a->blows;
    if (which == 0)
        blows += p_ptr->innate_attack_info.xtra_blow;

    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    /* First Column */
    if (a->flags & INNATE_NO_DAM)
        doc_printf(cols[0], "<color:y> %-7.7s</color>: Your %s\n", "Attack", a->name);
    else
        doc_printf(cols[0], "<color:y> %-7.7s</color>: Your %s (%dd%d)\n", "Attack", a->name, dd, a->ds);

    if (a->weight && !(a->flags & INNATE_NO_DAM) && p_ptr->prace != RACE_MON_BEHOLDER)
        doc_printf(cols[0], " %-7.7s: %d.%d lbs\n", "Weight", a->weight/10, a->weight%10);

    {
        cptr name = skills_innate_calc_name(a);
        doc_printf(cols[0], " %-7.7s: %s (%+d To Hit)\n",
                    "Profic",
                    skills_innate_describe_current(name),
                    skills_innate_calc_bonus(name));
    }

    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Hit", a->to_h, p_ptr->to_h_m, to_h);

    if (!(a->flags & INNATE_NO_DAM))
        doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Dam", a->to_d, p_ptr->to_d_m, to_d);

    doc_printf(cols[0], " %-7.7s: %d.%2.2d\n", "Blows", blows/100, blows%100);

    mult = 100;

    if (!(a->flags & INNATE_NO_DAM))
    {
        doc_printf(cols[0], "<color:G> %-7.7s</color>\n", "Damage");
        if (a->flags & INNATE_VORPAL)
        {
            mult = mult * 11 / 9;
            doc_printf(cols[0], " %-7.7s: %d.%02dx\n", "Vorpal", mult/100, mult%100);
        }
    }

    if (!(a->flags & (INNATE_NO_DAM | INNATE_NO_CRIT)))
    {
        critical_t crit = {0};
        _initialize_crit_loop(0);
        while (1)
        {
            critical_t tmp = critical_norm(a->weight, to_h, 0, 0, HAND_NONE);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
            }
            else if (!_critical_loop) break;
            else crit.mul += 100;
        }
        if ((!_critical_attempts) || (_critical_roll < 1) || (_critical_comp < 1)) /* Something has gone horribly wrong... */
        {
            crit.mul = 100;
            crit.to_d = 0;
        }
        else /* Evil voodoo */
        {
            if (_critical_roll != _critical_comp)
            {
                crit.mul -= (_critical_attempts * 100);
                if (((0x7FFFFFFF - (_critical_roll / 2)) / _critical_comp) < crit.mul)
                {
                    crit.mul = (crit.mul * _critical_comp + (_critical_roll / 2)) / _critical_roll;
                }
                else crit.mul = (crit.mul + (_critical_roll / 2)) / _critical_roll * _critical_comp;
                crit.mul += (_critical_attempts * 201) / 2;
                crit.mul /= _critical_attempts;
                crit.to_d = (crit.to_d * 100 + (_critical_roll / 2)) / _critical_roll * _critical_comp / _critical_attempts;
            }
            else
            {
                crit.mul = (crit.mul + (_critical_attempts / 2)) / _critical_attempts;
                crit.to_d = (crit.to_d * 100 + (_critical_attempts / 2)) / _critical_attempts;
            }
        }
        if (crit.to_d)
            doc_printf(cols[0], " %-7.7s: %d.%02dx + %d.%02d\n", "Crits", crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
        else
            doc_printf(cols[0], " %-7.7s: %d.%02dx\n", "Crits", crit.mul/100, crit.mul%100);
        crit.to_d /= 100;
        mult = mult * crit.mul / 100;
        to_d = to_d + crit.to_d;
    }

    min_base = mult * dd / 100;
    min = min_base + to_d;
    min2 = min_base + a->to_d;
    max_base = mult * dd * a->ds / 100;
    max = max_base + to_d;
    max2 = max_base + a->to_d;
    if (p_ptr->stun)
    {
        min_base -= min_base * MIN(100, p_ptr->stun) / 150;
        max_base -= max_base * MIN(100, p_ptr->stun) / 150;
        min -= min * MIN(100, p_ptr->stun) / 150;
        max -= max * MIN(100, p_ptr->stun) / 150;
        min2 -= min2 * MIN(100, p_ptr->stun) / 150;
        max2 -= max2 * MIN(100, p_ptr->stun) / 150;
    }
    min_base = (min_base * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;
    max_base = (max_base * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;
    min = (min * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;
    min2 = (min2 * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;
    max = (max * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;
    max2 = (max2 * (class_melee_mult() * race_melee_mult(TRUE) / 100) + 50) / 100;

    if (a->effect[0] == GF_OLD_CONF) /* Hack for Umber Hulk ... */
    {
        doc_insert(cols[0], "<tab:10><color:B>Confuses</color>\n");
    }
    else if (!(a->flags & INNATE_NO_DAM))
    {
        doc_printf(cols[0], " %-7.7s: %d\n",_effect_name(a->effect[0]), blows * (min + max)/200);
    }
    else strt = 0;

    if (p_ptr->current_r_idx == MON_AETHER_VORTEX) /* Hack ... cf race_vortex.c:_calc_innate_attacks() */
    {
        int min3 = 9*(min_base + a->to_d)/4 + p_ptr->to_d_m; /* 1 + .75 + .5 = 2.25 = 9/4 */
        int max3 = 9*(max_base + a->to_d)/4 + p_ptr->to_d_m;
        doc_printf(cols[0], "<color:r> %-7.7s</color>: %d\n",
                _effect_name(a->effect[0]),
                blows * (min3 + max3)/200);
    }
    else
    {
        for (i = strt; i < MAX_INNATE_EFFECTS; i++)
        {
            int p = a->effect_chance[i];
            char xtra[255];
            if (!a->effect[i]) continue;
            if ((!p) || (p == 100))
                sprintf(xtra, "%s", "");
            else
                sprintf(xtra, " (%d%%)", p);

            switch (a->effect[i])
            {
            case GF_STEAL:
                doc_printf(cols[0], "<tab:10><color:B>Steals%s</color>\n", xtra);
                break;
            case GF_OLD_SLOW:
                doc_printf(cols[0], "<tab:10><color:U>Slows%s</color>\n", xtra);
                break;
            case GF_BABY_SLOW:
                doc_printf(cols[0], "<tab:10><color:W>Slows%s</color>\n", xtra);
                break;
            case GF_OLD_CONF:
            case GF_BLIND:
                doc_printf(cols[0], "<tab:10><color:u>Confuses%s</color>\n", xtra);
                break;
            case GF_OLD_SLEEP:
                doc_printf(cols[0], "<tab:10><color:b>Sleeps%s</color>\n", xtra);
                break;
            case GF_STASIS:
            case GF_PARALYSIS:
                doc_printf(cols[0], "<tab:10><color:r>Paralyzes%s</color>\n", xtra);
                break;
            case GF_DRAIN_MANA:
                doc_printf(cols[0], "<tab:10><color:B>Drains Mana%s</color>\n", xtra);
                break;
            case GF_STUN:
                doc_printf(cols[0], "<tab:10><color:B>Stuns%s</color>\n", xtra);
                break;
            case GF_AMNESIA:
                doc_printf(cols[0], "<tab:10><color:R>Causes Amnesia%s</color>\n", xtra);
                break;
            case GF_TURN_ALL:
                doc_printf(cols[0], "<tab:10><color:v>Terrifies%s</color>\n", xtra);
                break;
            case GF_QUAKE:
                doc_printf(cols[0], "<tab:10><color:B>Shatters%s</color>\n", xtra);
                break;
            case GF_MISSILE: /* Full damage */
                if ((!p) || (p == 100))
                    doc_printf(cols[0], "<color:r>+%-7.7s</color>: %d\n", "Hurt", blows * (min + max)/200);
                else
                    doc_printf(cols[0], "<color:r>+%-7.7s</color>: %d/%d%s\n", "Hurt", blows * (min + max)/200, (s32b)blows * (min + max) * p / 20000L, xtra);
                break;
            case GF_OLD_DRAIN:
                if (i > 0)
                {
                    doc_printf(cols[0], "<tab:10><color:B>Drains%s</color>\n", xtra);
                    break;
                } /* Fall through */
            default:
                if ((!p) || (p == 100))
                {
                    doc_printf(cols[0], "<color:r>+%-7.7s</color>: %d\n",
                        _effect_name(a->effect[i]),
                        blows * (min2 + max2)/200);
                }
                else
                {
                    doc_printf(cols[0], "<color:r>+%-7.7s</color>: %d/%d%s\n",
                        _effect_name(a->effect[i]),
                        blows * (min2 + max2)/200, (s32b)blows * (min2 + max2) * p / 20000L, xtra);
                }
            }
        }
    }
    /* Second Column */
    doc_insert(cols[1], "<color:G>Accuracy</color>\n");
    doc_insert(cols[1], " AC Hit\n");

    doc_printf(cols[1], "%3d %2d%%\n", 25, hit_chance_innate(to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, hit_chance_innate(to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 75, hit_chance_innate(to_h, 75));
    doc_printf(cols[1], "%3d %2d%%\n", 100, hit_chance_innate(to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 125, hit_chance_innate(to_h, 125));
    doc_printf(cols[1], "%3d %2d%%\n", 150, hit_chance_innate(to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, hit_chance_innate(to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, hit_chance_innate(to_h, 200));

    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

/**********************************************************************
 * Ranged Attacks
 **********************************************************************/
int display_shooter_mode = 0;

/* Multiplier scaled by 100 */
int bow_mult(object_type *o_ptr)
{
    int mult = o_ptr->mult;

    mult += p_ptr->shooter_info.to_mult;

    if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS))
    {
        int idx = p_ptr->stat_ind[A_STR] + 4;
        if (idx > 40-3)
            idx = 40-3;
        mult = mult * (100 + (int)(adj_str_td[idx]) - 128) / 100;
    }
    else
        mult = mult * (100 + (int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128) / 100;

    return mult;
}

int bow_range(object_type *o_ptr)
{
    int range, mult;

    mult = bow_mult(o_ptr);
    range = 13 + mult/80;

    switch (o_ptr->sval)
    {
    case SV_LIGHT_XBOW:
    case SV_HEAVY_XBOW:
        if (p_ptr->concent)
            range += (p_ptr->concent + 1) / 2;
        break;
    }

    return MIN(MAX_RANGE, range);
}

static void _display_missile_slay(int bow_mult, int slay_mult, int crit_mult,
                                  bool force, int shots,
                                  int dd, int ds, int to_d, int to_d_xtra,
                                  cptr name, int color, doc_ptr doc)
{
    int dam = dd*(ds + 1)/2 + to_d;

    if (force) slay_mult += 50;
    dam = dam * slay_mult / 100;
    dam = dam * crit_mult / 100;
    if (p_ptr->concent) dam = boost_concentration_damage(dam);
    dam = dam * bow_mult / 100;

    dam += to_d_xtra;
    if (p_ptr->stun)
        dam -= dam * MIN(100, p_ptr->stun) / 150;

    doc_printf(doc, " <color:%c>%-8.8s</color>", attr_to_attr_char(color), format("%^s", name));
    doc_printf(doc, ": %d/%d\n", dam, shots * dam / 100);
}


static void _shooter_info_aux(doc_ptr doc, object_type *bow, object_type *arrow, int ct)
{
    char         o_name[MAX_NLEN];
    u32b         flgs[OF_ARRAY_SIZE];
    int          mult;
    int          to_h = 0;
    int          to_d = 0;
    int          to_h_bow = 0;
    int          to_d_bow = 0;
    int          to_h_xtra = p_ptr->shooter_info.dis_to_h;
    int          to_d_xtra = p_ptr->shooter_info.dis_to_d;
    int          dd = arrow->dd;
    int          ds = arrow->ds;
    critical_t   crit = {0};
    int          crit_pct = 0;
    int          num_fire = 0;
    int          real_snipe = 0;
    doc_ptr      cols[2] = {0};
    bool         force = FALSE;
    int          i;
    slay_type    _slay;

    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    missile_flags_known(arrow, flgs);
    mult = bow_mult(bow);

    if (object_is_artifact(arrow))
        num_fire = 100;
    else if (p_ptr->shooter_info.base_shot)
        num_fire = NUM_SHOTS * 100 * 100 / bow_energy(bow->sval);

    if (object_is_known(bow))
    {
        to_h_bow = bow->to_h;
        to_d_bow = bow->to_d;
    }
    to_h_bow += skills_bow_calc_bonus(bow->sval);

    if (object_is_known(arrow))
    {
        to_h = arrow->to_h;
        to_d = arrow->to_d;
    }

    if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS) && p_ptr->lev >= 15)
        to_d += 1 + p_ptr->lev/10;

    if (p_ptr->big_shot)
        ds += 2;

    {
        const int ct = 10 * 1000;
        int i, crits = 0;
        /* Compute Average Effects of Criticals by sampling */
        for (i = 0; i < ct; i++)
        {
            critical_t tmp = critical_shot(arrow->weight, arrow->to_h);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
                crits++;
            }
            else
                crit.mul += 100;
        }
        crit.mul = crit.mul / ct;
        crit.to_d = crit.to_d * 100 / ct;
        crit_pct = crits * 1000 / ct;
    }

    /* First Column */
    object_desc(o_name, arrow, OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
    doc_printf(cols[0], "<color:u> Ammo #%-2d</color>: <indent><style:indent>%s</style></indent>\n", ct, o_name);

    real_snipe = shoot_hack;
    if (display_shooter_mode) shoot_hack = display_shooter_mode;
    doc_printf(cols[0], " %-8.8s: %d%%\n", "Breakage", breakage_chance(arrow));
    shoot_hack = real_snipe;
    doc_printf(cols[0], " %-8.8s: %d.%d lbs\n", "Weight", arrow->weight/10, arrow->weight%10);
    doc_printf(cols[0], " %-8.8s: %d + %d = %d\n", "To Hit", to_h, to_h_bow + to_h_xtra, to_h + to_h_bow + to_h_xtra);
    doc_printf(cols[0], " %-8.8s: %d (%s)\n", "To Dam", to_d, "Multiplier Applies");
    doc_printf(cols[0], " %-8.8s: %d (%s)\n", "To Dam", to_d_bow + to_d_xtra, "Multiplier Does Not Apply");
    doc_printf(cols[0], " <color:G>%-8.8s</color>\n", "Damage");

    if (crit.to_d)
    {
        doc_printf(cols[0], " %-8.8s: %d.%02dx + %d.%02d\n", "Crits",
                        crit.mul/100, crit.mul%100, crit.to_d/100, crit.to_d%100);
    }
    else
    {
        doc_printf(cols[0], " %-8.8s: %d.%02dx (%d.%d%%)\n", "Crits",
                        crit.mul/100, crit.mul%100, crit_pct / 10, crit_pct % 10);
    }

    to_d_xtra = to_d_bow + to_d_xtra + crit.to_d/100;

    _display_missile_slay(mult, 100, crit.mul, FALSE, num_fire, dd, ds, to_d, to_d_xtra, "Normal", TERM_WHITE, cols[0]);

    if (p_ptr->tim_force && p_ptr->csp >= 1 + arrow->dd * arrow->ds / 2)
    {
        force = TRUE;
        _display_missile_slay(mult, 100, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, "Force", TERM_L_BLUE, cols[0]);
    }

    if (display_shooter_mode == SP_FINAL)
    {
        int snipe = sniper_multiplier(display_shooter_mode, arrow, NULL) * 10;
        _display_missile_slay(mult, snipe, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, "All", TERM_VIOLET, cols[0]);
    }

    i = 0;
    
    for (_slay = slay_list[0];; _slay = slay_list[++i])
    {
        int _slay_mult = 100;
        if (!_slay.tier) break;
        if ((display_shooter_mode) &&
            (((display_shooter_mode == SP_HOLYNESS) && (_slay.kill_flag == OF_KILL_EVIL)) ||
            ((display_shooter_mode == SP_EVILNESS) && (_slay.kill_flag == OF_KILL_GOOD)) ||
            ((display_shooter_mode == SP_ELEC) && (_slay.slay_flag == OF_BRAND_ELEC)) ||
            ((display_shooter_mode == SP_FIRE) && (_slay.slay_flag == OF_BRAND_FIRE)) ||
            ((display_shooter_mode == SP_COLD) && (_slay.slay_flag == OF_BRAND_COLD))))
            _slay_mult = sniper_multiplier(display_shooter_mode, arrow, NULL) * 10;
        else if ((_slay.kill_flag) && (have_flag(flgs, _slay.kill_flag)))
            _slay_mult = slay_tiers[_slay.tier - 1].archery_kill;
        else if (have_flag(flgs, _slay.slay_flag))
            _slay_mult = slay_tiers[_slay.tier - 1].archery_slay;
        if (_slay_mult != 100)
        {
            _display_missile_slay(mult, _slay_mult, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, (_slay.slay_flag == OF_BRAND_ELEC) ? "Elec" : _slay.kill_desc, _slay.is_slay ? TERM_YELLOW : TERM_RED, cols[0]);
        }
    }

    if (display_shooter_mode == SP_KILL_WALL)
    {
        int snipe = sniper_multiplier(display_shooter_mode, arrow, NULL) * 10;
        _display_missile_slay(mult, snipe, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, "HurtRock", TERM_VIOLET, cols[0]);
        _display_missile_slay(mult, snipe, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, "Nonliving", TERM_VIOLET, cols[0]);
    }
    if (display_shooter_mode == SP_LITE)
    {
        int snipe = sniper_multiplier(display_shooter_mode, arrow, NULL) * 10;
        _display_missile_slay(mult, snipe, crit.mul, force, num_fire, dd, ds, to_d, to_d_xtra, "HurtLite", TERM_VIOLET, cols[0]);
    }
    /* Second Column */
    to_h = to_h + to_h_bow + to_h_xtra;

    doc_insert(cols[1], " <color:G>AC Hit</color>\n");
    doc_printf(cols[1], "%3d %2d%%\n", 25, bow_hit_chance(to_h, 25));
    doc_printf(cols[1], "%3d %2d%%\n", 50, bow_hit_chance(to_h, 50));
    doc_printf(cols[1], "%3d %2d%%\n", 100, bow_hit_chance(to_h, 100));
    doc_printf(cols[1], "%3d %2d%%\n", 150, bow_hit_chance(to_h, 150));
    doc_printf(cols[1], "%3d %2d%%\n", 175, bow_hit_chance(to_h, 175));
    doc_printf(cols[1], "%3d %2d%%\n", 200, bow_hit_chance(to_h, 200));

    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

void display_shooter_info(doc_ptr doc)
{
    object_type *bow_ptr = NULL;
    int          slot = equip_find_obj(TV_BOW, SV_ANY);
    char         o_name[MAX_NLEN];
    int          mult;
    int          num_fire = 0;
    int          to_h = 0;
    int          to_d = 0;
    int          i, j;

    if (!slot || prace_is_(RACE_MON_JELLY) || p_ptr->shooter_info.tval_ammo == TV_NO_AMMO)
        return;

    bow_ptr = equip_obj(slot);
    assert(bow_ptr);

    mult = bow_mult(bow_ptr);

    if (p_ptr->shooter_info.base_shot)
        num_fire = NUM_SHOTS * 100 * 100 / bow_energy(bow_ptr->sval);

    if (object_is_known(bow_ptr))
    {
        to_h = bow_ptr->to_h;
        to_d = bow_ptr->to_d;
    }
    to_h += skills_bow_calc_bonus(bow_ptr->sval);

    /* Shooter */
    object_desc(o_name, bow_ptr, OD_OMIT_INSCRIPTION | OD_COLOR_CODED);
    doc_printf(doc, " <color:y>Shooting</color>: <indent><style:indent>%s</style></indent>\n", o_name);

    doc_printf(doc, " %-8.8s: %d'\n", "Range", (bow_range(bow_ptr) + 1) * 10);
    doc_printf(doc, " %-8.8s: %d.%02d\n", "Shots", num_fire/100, num_fire%100);
    doc_printf(doc, " %-8.8s: %d.%02dx\n", "Mult", mult/100, mult%100);
    doc_printf(doc, " %-8.8s: %s (%+d To Hit)\n", "Profic", skills_bow_describe_current(bow_ptr->sval), skills_bow_calc_bonus(bow_ptr->sval));
    doc_printf(doc, " %-8.8s: %d + %d = %d\n", "To Hit", to_h, p_ptr->shooter_info.dis_to_h, to_h + p_ptr->shooter_info.dis_to_h);
    if (weaponmaster_is_(WEAPONMASTER_CROSSBOWS) && p_ptr->lev >= 15)
        doc_printf(doc, " %-8.8s: %d (%s)\n", "To Dam", 1 + p_ptr->lev/10, "Multiplier Applies");
    doc_printf(doc, " %-8.8s: %d (%s)\n", "Xtra Dam", p_ptr->shooter_info.dis_to_d + to_d, "Multiplier Does Not Apply");
    doc_newline(doc);

    /* Ammo */
    j = 0;
    for (i = quiver_find_first(obj_can_shoot); i; i = quiver_find_next(obj_can_shoot, i))
    {
        obj_ptr ammo = quiver_obj(i);
        _shooter_info_aux(doc, bow_ptr, ammo, ++j);
    }
    for (i = pack_find_first(obj_can_shoot); i; i = pack_find_next(obj_can_shoot, i))
    {
        obj_ptr ammo = pack_obj(i);
        _shooter_info_aux(doc, bow_ptr, ammo, ++j);
    }
}
