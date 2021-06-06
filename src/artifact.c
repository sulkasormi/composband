/* Purpose: Artifact code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include <assert.h>

static void random_resistance(object_type * o_ptr);
static void random_slay(object_type *o_ptr);


/* Chance of using syllables to form the name instead of the "template" files */
#define SINDARIN_NAME   10
#define TABLE_NAME      20
#define A_CURSED        13
#define WEIRD_LUCK      12
#define BIAS_LUCK       20
#define IM_LUCK         7
#define IM_DEPTH        100
bool immunity_hack = FALSE;
int slaying_hack = 0;
static bool has_pval;

/*
 * Bias luck needs to be higher than weird luck,
 * since it is usually tested several times...
 */
#define ACTIVATION_CHANCE 5


/*
 * Use for biased artifact creation
 */
static int artifact_bias;

/*
 * Cursed arts now pick up bad flags at the end. To avoid idiocy like
 * arts that give resistance and vulnerability to fire simultaneously,
 * we implement the following check:
 */
static bool _add_bad_flag(object_type *o_ptr, int bad_flag, int good_flag)
{
    u32b flags[OF_ARRAY_SIZE];
    obj_flags(o_ptr, flags); /* Careful: Might be a cursed ego! */
    if (!have_flag(flags, good_flag))
    {
        add_flag(o_ptr->flags, bad_flag);
        return TRUE;
    }
    return FALSE;
}

int trim(int start_val, int hi_val, int very_hi_val, int item_lv)
{
    int vahennys = 0, i, koitto;
    if (very_hi_val < hi_val) hi_val = very_hi_val;
    if (start_val <= hi_val) return start_val;
    koitto = MIN(very_hi_val - hi_val, start_val - hi_val);
    for (i = 0; i < koitto; i++)
    {
        if (one_in_(2)) vahennys++;
        else if (randint0(100 + item_lv) < 80) vahennys++;
    }
    koitto = (start_val - vahennys) - very_hi_val;
    for (i = 0; i < koitto; i++)
    {
        if (!one_in_(4)) vahennys++;
        else if (randint0(100 + item_lv) < 120) vahennys++;
    }
    return start_val - vahennys;
}

static void _sanitise_flags(object_type *o_ptr)
{
    int i;
    if (have_flag(o_ptr->flags, OF_LITE) && have_flag(o_ptr->flags, OF_DARKNESS))
    {
        if (one_in_(2)) remove_flag(o_ptr->flags, OF_DARKNESS);
        else remove_flag(o_ptr->flags, OF_LITE);
    }
    for (i = OF_STR; i <= OF_CHR; i++)
    {
        if (have_flag(o_ptr->flags, i) && have_flag(o_ptr->flags, i + OF_DEC_STR - OF_STR))
        {
            remove_flag(o_ptr->flags, i);
            remove_flag(o_ptr->flags, i + OF_DEC_STR - OF_STR);
        }
    }

    /* Check for weapons with VORPAL2 but no VORPAL
     * They need VORPAL for consistent inscriptions, even though it has no
     * effect on their performance */
    if ((have_flag(o_ptr->flags, OF_VORPAL2)) && (!have_flag(o_ptr->flags, OF_VORPAL)))
        add_flag(o_ptr->flags, OF_VORPAL);
}

/*
 * Choose one random sustain
 */
void one_sustain(object_type *o_ptr)
{
    switch (randint0(6))
    {
        case 0: add_flag(o_ptr->flags, OF_SUST_STR); break;
        case 1: add_flag(o_ptr->flags, OF_SUST_INT); break;
        case 2: add_flag(o_ptr->flags, OF_SUST_WIS); break;
        case 3: add_flag(o_ptr->flags, OF_SUST_DEX); break;
        case 4: add_flag(o_ptr->flags, OF_SUST_CON); break;
        case 5: add_flag(o_ptr->flags, OF_SUST_CHR); break;
    }
}


/*
 * Choose one random high resistance
 */
void one_high_resistance(object_type *o_ptr)
{
    switch (randint0(12))
    {
        case  0: add_flag(o_ptr->flags, OF_RES_POIS);   break;
        case  1: add_flag(o_ptr->flags, OF_RES_LITE);   break;
        case  2: add_flag(o_ptr->flags, OF_RES_DARK);   break;
        case  3: add_flag(o_ptr->flags, OF_RES_SHARDS); break;
        case  4: add_flag(o_ptr->flags, OF_RES_BLIND);  break;
        case  5: add_flag(o_ptr->flags, OF_RES_CONF);   break;
        case  6: add_flag(o_ptr->flags, OF_RES_SOUND);  break;
        case  7: add_flag(o_ptr->flags, OF_RES_NETHER); break;
        case  8: add_flag(o_ptr->flags, OF_RES_NEXUS);  break;
        case  9: add_flag(o_ptr->flags, OF_RES_CHAOS);  break;
        case 10: add_flag(o_ptr->flags, OF_RES_DISEN);  break;
        case 11: add_flag(o_ptr->flags, OF_RES_FEAR);   break;
    }
}
bool one_high_vulnerability(object_type *o_ptr)
{
    int attempts = 100;
    int i;
    for (i = 0; i < attempts; i++)
    {
        int good_flag = 0, bad_flag = 0;
        switch (randint0(12))
        {
            case  0: bad_flag = OF_VULN_POIS; good_flag = OF_RES_POIS; break;
            case  1: bad_flag = OF_VULN_LITE; good_flag = OF_RES_LITE; break;
            case  2: bad_flag = OF_VULN_DARK; good_flag = OF_RES_DARK; break;
            case  3: bad_flag = OF_VULN_SHARDS; good_flag = OF_RES_SHARDS; break;
            case  4: bad_flag = OF_VULN_BLIND; good_flag = OF_RES_BLIND; break;
            case  5: bad_flag = OF_VULN_CONF; good_flag = OF_RES_CONF; break;
            case  6: bad_flag = OF_VULN_SOUND; good_flag = OF_RES_SOUND; break;
            case  7: bad_flag = OF_VULN_NETHER; good_flag = OF_RES_NETHER; break;
            case  8: bad_flag = OF_VULN_NEXUS; good_flag = OF_RES_NEXUS; break;
            case  9: bad_flag = OF_VULN_CHAOS; good_flag = OF_RES_CHAOS; break;
            case 10: bad_flag = OF_VULN_DISEN; good_flag = OF_RES_DISEN; break;
            case 11: bad_flag = OF_VULN_FEAR; good_flag = OF_RES_FEAR; break;
        }
        if (_add_bad_flag(o_ptr, bad_flag, good_flag))
            return TRUE;
    }
    return FALSE;
}
void one_undead_resistance(object_type *o_ptr)
{
    switch (randint1(6))
    {
        case 1: add_flag(o_ptr->flags, OF_RES_COLD);   break;
        case 2: add_flag(o_ptr->flags, OF_RES_POIS);   break;
        case 3: add_flag(o_ptr->flags, OF_RES_DARK);   break;
        case 4: add_flag(o_ptr->flags, OF_RES_NETHER); break;
        case 5: add_flag(o_ptr->flags, OF_RES_DISEN);  break;
        case 6: add_flag(o_ptr->flags, OF_RES_FEAR);   break;
    }
}
void one_demon_resistance(object_type *o_ptr)
{
    switch (randint1(6))
    {
        case 1: add_flag(o_ptr->flags, OF_RES_FIRE);   break;
        case 2: add_flag(o_ptr->flags, OF_RES_CONF);   break;
        case 3: add_flag(o_ptr->flags, OF_RES_NEXUS);  break;
        case 4: add_flag(o_ptr->flags, OF_RES_CHAOS);  break;
        case 5: add_flag(o_ptr->flags, OF_RES_DISEN);  break;
        case 6: add_flag(o_ptr->flags, OF_RES_FEAR);   break;
    }
}

void one_holy_resistance(object_type *o_ptr)
{
    switch (randint1(4))
    {
        case 1: add_flag(o_ptr->flags, OF_RES_LITE);   break;
        case 2: add_flag(o_ptr->flags, OF_RES_SOUND);  break;
        case 3: add_flag(o_ptr->flags, OF_RES_SHARDS); break;
        case 4: add_flag(o_ptr->flags, OF_RES_DISEN);  break;
    }
}

/*
 * Choose one random high resistance ( except poison and disenchantment )
 */
void one_lordly_high_resistance(object_type *o_ptr)
{
    switch (randint0(10))
    {
        case 0: add_flag(o_ptr->flags, OF_RES_LITE);   break;
        case 1: add_flag(o_ptr->flags, OF_RES_DARK);   break;
        case 2: add_flag(o_ptr->flags, OF_RES_SHARDS); break;
        case 3: add_flag(o_ptr->flags, OF_RES_BLIND);  break;
        case 4: add_flag(o_ptr->flags, OF_RES_CONF);   break;
        case 5: add_flag(o_ptr->flags, OF_RES_SOUND);  break;
        case 6: add_flag(o_ptr->flags, OF_RES_NETHER); break;
        case 7: add_flag(o_ptr->flags, OF_RES_NEXUS);  break;
        case 8: add_flag(o_ptr->flags, OF_RES_CHAOS);  break;
        case 9: add_flag(o_ptr->flags, OF_RES_FEAR);   break;
    }
}

/*
 * Choose one random element resistance
 */
void one_ele_resistance(object_type *o_ptr)
{
    switch (randint0(4))
    {
        case  0: add_flag(o_ptr->flags, OF_RES_ACID); break;
        case  1: add_flag(o_ptr->flags, OF_RES_ELEC); break;
        case  2: add_flag(o_ptr->flags, OF_RES_COLD); break;
        case  3: add_flag(o_ptr->flags, OF_RES_FIRE); break;
    }
}
bool one_ele_vulnerability(object_type *o_ptr)
{
    int attempts = 100;
    int i;
    for (i = 0; i < attempts; i++)
    {
        int good_flag = 0, bad_flag = 0;
        switch (randint0(4))
        {
            case  0: bad_flag = OF_VULN_ACID; good_flag = OF_RES_ACID; break;
            case  1: bad_flag = OF_VULN_ELEC; good_flag = OF_RES_ELEC; break;
            case  2: bad_flag = OF_VULN_COLD; good_flag = OF_RES_COLD; break;
            case  3: bad_flag = OF_VULN_FIRE; good_flag = OF_RES_FIRE; break;
        }
        if (_add_bad_flag(o_ptr, bad_flag, good_flag))
            return TRUE;
    }
    return FALSE;
}

void one_ele_slay(object_type *o_ptr)
{
    switch (randint0(5))
    {
        case  0: add_flag(o_ptr->flags, OF_BRAND_ACID); break;
        case  1: add_flag(o_ptr->flags, OF_BRAND_COLD); break;
        case  2: add_flag(o_ptr->flags, OF_BRAND_FIRE); break;
        case  3: add_flag(o_ptr->flags, OF_BRAND_ELEC); break;
        case  4: add_flag(o_ptr->flags, OF_BRAND_POIS); break;
    }
}


/*
 * Choose one random element or poison resistance
 */
void one_dragon_ele_resistance(object_type *o_ptr)
{
    if (one_in_(7))
    {
        add_flag(o_ptr->flags, OF_RES_POIS);
    }
    else
    {
        one_ele_resistance(o_ptr);
    }
}
bool one_dragon_ele_vulnerability(object_type *o_ptr)
{
    if (one_in_(7) && !have_flag(o_ptr->flags, OF_RES_POIS))
    {
        add_flag(o_ptr->flags, OF_VULN_POIS);
        return TRUE;
    }
    return one_ele_vulnerability(o_ptr);
}

/*
 * Choose one lower rank esp
 */
void one_low_esp(object_type *o_ptr)
{
    switch (randint1(9))
    {
    case 1: add_flag(o_ptr->flags, OF_ESP_ANIMAL);   break;
    case 2: add_flag(o_ptr->flags, OF_ESP_UNDEAD);   break;
    case 3: add_flag(o_ptr->flags, OF_ESP_DEMON);   break;
    case 4: add_flag(o_ptr->flags, OF_ESP_ORC);   break;
    case 5: add_flag(o_ptr->flags, OF_ESP_TROLL);   break;
    case 6: add_flag(o_ptr->flags, OF_ESP_GIANT);   break;
    case 7: add_flag(o_ptr->flags, OF_ESP_DRAGON);   break;
    case 8: add_flag(o_ptr->flags, OF_ESP_HUMAN);   break;
    case 9: add_flag(o_ptr->flags, OF_ESP_GOOD);   break;
    }
}



/*
 * Choose one random resistance
 */
void one_resistance(object_type *o_ptr)
{
    if (one_in_(3))
    {
        one_ele_resistance(o_ptr);
    }
    else
    {
        one_high_resistance(o_ptr);
    }
}
bool one_vulnerability(object_type *o_ptr)
{
    if (one_in_(3))
        return one_ele_vulnerability(o_ptr);

    return one_high_vulnerability(o_ptr);
}

bool one_stat_biff(object_type *o_ptr)
{
    int attempts = 100;
    int i;
    for (i = 0; i < attempts; i++)
    {
        int n = randint0(6);
        if (_add_bad_flag(o_ptr, OF_DEC_STR + n, OF_STR + n))
            return TRUE;
    }
    return FALSE;
}

bool one_biff(object_type *o_ptr)
{
    int attempts = 100;
    int i;
    for (i = 0; i < attempts; i++)
    {
        int n = randint0(100);
        if (n < 10)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_STEALTH, OF_STEALTH))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 13)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_SPEED, OF_SPEED))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 17)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_LIFE, OF_LIFE))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 22)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_MAGIC_MASTERY, OF_MAGIC_MASTERY))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 30)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_SPELL_CAP, OF_SPELL_CAP))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 35)
        {
            if (_add_bad_flag(o_ptr, OF_DEC_SPELL_POWER, OF_SPELL_POWER))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else if (n < 67)
        {
            if (one_stat_biff(o_ptr))
            {
                has_pval = TRUE;
                return TRUE;
            }
        }
        else
        {
            if (one_vulnerability(o_ptr))
                return TRUE;
        }
    }
    return FALSE;
}

/*
 * Choose one random ability
 */
void one_ability(object_type *o_ptr)
{
    switch (randint0(10))
    {
    case 0: add_flag(o_ptr->flags, OF_LEVITATION);     break;
    case 1: add_flag(o_ptr->flags, OF_LITE);        break;
    case 2: add_flag(o_ptr->flags, OF_SEE_INVIS);   break;
    case 3: add_flag(o_ptr->flags, OF_WARNING);     break;
    case 4: add_flag(o_ptr->flags, OF_SLOW_DIGEST); break;
    case 5: add_flag(o_ptr->flags, OF_REGEN);       break;
    case 6: add_flag(o_ptr->flags, OF_FREE_ACT);    break;
    case 7: add_flag(o_ptr->flags, OF_HOLD_LIFE);   break;
    case 8:
    case 9:
        one_low_esp(o_ptr);
        break;
    }
}

static void random_plus(object_type * o_ptr)
{
    int this_type = (object_is_weapon_ammo(o_ptr) ? 23 : 19);

    has_pval = TRUE;
    switch (artifact_bias)
    {
    case BIAS_WARRIOR:
        if (!(have_flag(o_ptr->flags, OF_STR)))
        {
            add_flag(o_ptr->flags, OF_STR);
            if (one_in_(2)) return;
        }

        if (!(have_flag(o_ptr->flags, OF_CON)))
        {
            add_flag(o_ptr->flags, OF_CON);
            if (one_in_(2)) return;
        }

        if (!(have_flag(o_ptr->flags, OF_DEX)))
        {
            add_flag(o_ptr->flags, OF_DEX);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_MAGE:
        if (!(have_flag(o_ptr->flags, OF_INT)))
        {
            add_flag(o_ptr->flags, OF_INT);
            if (one_in_(2)) return;
        }
        if ((o_ptr->tval == TV_GLOVES) && !(have_flag(o_ptr->flags, OF_MAGIC_MASTERY)))
        {
            add_flag(o_ptr->flags, OF_MAGIC_MASTERY);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_PRIESTLY:
        if (!(have_flag(o_ptr->flags, OF_WIS)))
        {
            add_flag(o_ptr->flags, OF_WIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_RANGER:
        if (!(have_flag(o_ptr->flags, OF_DEX)))
        {
            add_flag(o_ptr->flags, OF_DEX);
            if (one_in_(2)) return;
        }

        if (!(have_flag(o_ptr->flags, OF_CON)))
        {
            add_flag(o_ptr->flags, OF_CON);
            if (one_in_(2)) return;
        }

        if (!(have_flag(o_ptr->flags, OF_STR)))
        {
            add_flag(o_ptr->flags, OF_STR);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_ROGUE:
        if (!(have_flag(o_ptr->flags, OF_STEALTH)))
        {
            add_flag(o_ptr->flags, OF_STEALTH);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_SEARCH)))
        {
            add_flag(o_ptr->flags, OF_SEARCH);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_STR:
        if (!(have_flag(o_ptr->flags, OF_STR)))
        {
            add_flag(o_ptr->flags, OF_STR);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_WIS:
        if (!(have_flag(o_ptr->flags, OF_WIS)))
        {
            add_flag(o_ptr->flags, OF_WIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_INT:
        if (!(have_flag(o_ptr->flags, OF_INT)))
        {
            add_flag(o_ptr->flags, OF_INT);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_DEX:
        if (!(have_flag(o_ptr->flags, OF_DEX)))
        {
            add_flag(o_ptr->flags, OF_DEX);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CON:
        if (!(have_flag(o_ptr->flags, OF_CON)))
        {
            add_flag(o_ptr->flags, OF_CON);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CHR:
        if (!(have_flag(o_ptr->flags, OF_CHR)))
        {
            add_flag(o_ptr->flags, OF_CHR);
            if (one_in_(2)) return;
        }
        break;
    }

    if ((artifact_bias == BIAS_MAGE || artifact_bias == BIAS_PRIESTLY) && (o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->sval == SV_ROBE))
    {
        if (!(have_flag(o_ptr->flags, OF_DEC_MANA)) && one_in_(3))
        {
            add_flag(o_ptr->flags, OF_DEC_MANA);
            if (one_in_(2)) return;
        }
    }

    switch (randint1(this_type))
    {
    case 1: case 2:
        add_flag(o_ptr->flags, OF_STR);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_STR;
        else if (!artifact_bias && one_in_(7))
            artifact_bias = BIAS_WARRIOR;
        break;
    case 3: case 4:
        add_flag(o_ptr->flags, OF_INT);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_INT;
        else if (!artifact_bias && one_in_(7))
            artifact_bias = BIAS_MAGE;
        break;
    case 5: case 6:
        add_flag(o_ptr->flags, OF_WIS);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_WIS;
        else if (!artifact_bias && one_in_(7))
            artifact_bias = BIAS_PRIESTLY;
        break;
    case 7: case 8:
        add_flag(o_ptr->flags, OF_DEX);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_DEX;
        else if (!artifact_bias && one_in_(7))
            artifact_bias = BIAS_ROGUE;
        break;
    case 9: case 10:
        add_flag(o_ptr->flags, OF_CON);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_CON;
        else if (!artifact_bias && one_in_(9))
            artifact_bias = BIAS_RANGER;
        break;
    case 11: case 12:
        add_flag(o_ptr->flags, OF_CHR);
        if (!artifact_bias && !one_in_(13))
            artifact_bias = BIAS_CHR;
        break;
    case 13: case 14:
        add_flag(o_ptr->flags, OF_STEALTH);
        if (!artifact_bias && one_in_(3))
            artifact_bias = BIAS_ROGUE;
        break;
    case 15: case 16:
        add_flag(o_ptr->flags, OF_SEARCH);
        if (!artifact_bias && one_in_(9))
            artifact_bias = BIAS_RANGER;
        break;
    case 17: case 18:
        add_flag(o_ptr->flags, OF_INFRA);
        break;
    case 19:
        add_flag(o_ptr->flags, OF_SPEED);
        if (!artifact_bias && one_in_(11))
            artifact_bias = BIAS_ROGUE;
        break;
    case 20: case 21:
        if (one_in_(3))
            add_flag(o_ptr->flags, OF_SPEED);
        else
            add_flag(o_ptr->flags, OF_TUNNEL);
        break;
    case 22: case 23:
        if (o_ptr->tval == TV_BOW) random_plus(o_ptr);
        else
        {
            if (slaying_hack == 0)
            {
                if (one_in_(13))
                {
                    add_flag(o_ptr->flags, OF_BLOWS);
                    if (!artifact_bias && one_in_(11))
                        artifact_bias = BIAS_WARRIOR;
                }
                else
                    random_slay(o_ptr);
            }
            else
                random_plus(o_ptr);
        }
        break;
    }
}

static bool double_check_immunity(object_type * o_ptr)
{
    if (randint1(IM_DEPTH) < (object_level - 40))
        return TRUE;
    else if (immunity_hack)
    {
        return FALSE;
    }
    else
    {
        int i;
        int ct = randint1(3);

        immunity_hack = TRUE;

        if (object_is_melee_weapon(o_ptr))
        {
            for (i = 0; i < ct; i++)
                random_slay(o_ptr);

            switch (randint1(10))
            {
            case 1: case 2:
                if (!have_flag(o_ptr->flags, OF_SPEED))
                {
                    add_flag(o_ptr->flags, OF_SPEED);
                    has_pval = TRUE;
                    break;
                }
                /* vvvv FALL THRU vvvv */
            case 3: case 4: case 5: case 6:
                add_flag(o_ptr->flags, OF_SHOW_MODS);
                o_ptr->to_h += 5 + randint1(10);
                o_ptr->to_d += 5 + randint1(10);
                break;
            case 7: case 8:
                if (!have_flag(o_ptr->flags, OF_BLOWS))
                {
                    add_flag(o_ptr->flags, OF_BLOWS);
                    has_pval = TRUE;
                    break;
                }
                /* vvvv FALL THRU vvvv */
            case 9: case 10:
                add_flag(o_ptr->flags, OF_STR);
                add_flag(o_ptr->flags, OF_DEX);
                add_flag(o_ptr->flags, OF_CON);
                add_flag(o_ptr->flags, OF_FREE_ACT);
                add_flag(o_ptr->flags, OF_SEE_INVIS);
                has_pval = TRUE;
                break;
            }
        }
        else if (object_is_armour(o_ptr))
        {
            for (i = 0; i < ct; i++)
                random_resistance(o_ptr);

            switch (randint1(10))
            {
            case 1: case 2:
                if (!have_flag(o_ptr->flags, OF_SPEED))
                {
                    add_flag(o_ptr->flags, OF_SPEED);
                    has_pval = TRUE;
                    break;
                }
                /* vvvv FALL THRU vvvv */
            case 3: case 4: case 5: case 6:
                add_flag(o_ptr->flags, OF_SHOW_MODS);
                o_ptr->to_h += 5 + randint1(10);    /* These will get trimmed later ... */
                o_ptr->to_d += 5 + randint1(10);
                break;
            case 7: case 8:
                o_ptr->to_a += 5 + randint1(10);
                add_flag(o_ptr->flags, OF_FREE_ACT);
                add_flag(o_ptr->flags, OF_SEE_INVIS);
                add_flag(o_ptr->flags, OF_HOLD_LIFE);
                add_flag(o_ptr->flags, OF_LEVITATION);
                break;
            case 9: case 10:
                add_flag(o_ptr->flags, OF_STR);
                add_flag(o_ptr->flags, OF_DEX);
                add_flag(o_ptr->flags, OF_CON);
                has_pval = TRUE;
                break;
            }
        }
        else if (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET)
        {
            /*
            ct /= 2;
            for (i = 0; i < ct; i++)
                random_resistance(o_ptr);
            */

            switch (randint1(3))
            {
            case 1:
                if (!have_flag(o_ptr->flags, OF_SPEED))
                {
                    add_flag(o_ptr->flags, OF_SPEED);
                    has_pval = TRUE;
                    break;
                }
            case 2:
                add_flag(o_ptr->flags, OF_SHOW_MODS);
                o_ptr->to_h += 5 + randint1(10);    /* These will get trimmed later ... */
                o_ptr->to_d += 5 + randint1(10);
                break;

            case 3:
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_STR);
                    has_pval = TRUE;
                }
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_INT);
                    has_pval = TRUE;
                }
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_WIS);
                    has_pval = TRUE;
                }
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_DEX);
                    has_pval = TRUE;
                }
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_CON);
                    has_pval = TRUE;
                }
                if (one_in_(2))
                {
                    add_flag(o_ptr->flags, OF_CHR);
                    has_pval = TRUE;
                }
                break;
            }
        }
        return FALSE;
    }
}

static void random_resistance(object_type * o_ptr)
{
    switch (artifact_bias)
    {
    case BIAS_ACID:
        if (!(have_flag(o_ptr->flags, OF_RES_ACID)))
        {
            add_flag(o_ptr->flags, OF_RES_ACID);
            if (one_in_(2)) return;
        }
        if (one_in_(BIAS_LUCK) && !(have_flag(o_ptr->flags, OF_IM_ACID)) && double_check_immunity(o_ptr))
        {
            add_flag(o_ptr->flags, OF_IM_ACID);
            if (!one_in_(IM_LUCK))
            {
                remove_flag(o_ptr->flags, OF_IM_ELEC);
                remove_flag(o_ptr->flags, OF_IM_COLD);
                remove_flag(o_ptr->flags, OF_IM_FIRE);
            }
            if (one_in_(2)) return;
        }
        break;

    case BIAS_ELEC:
        if (!(have_flag(o_ptr->flags, OF_RES_ELEC)))
        {
            add_flag(o_ptr->flags, OF_RES_ELEC);
            if (one_in_(2)) return;
        }
        if ((o_ptr->tval >= TV_CLOAK) && (o_ptr->tval <= TV_HARD_ARMOR) &&
            !(have_flag(o_ptr->flags, OF_AURA_ELEC)))
        {
            add_flag(o_ptr->flags, OF_AURA_ELEC);
            if (one_in_(2)) return;
        }
        if (one_in_(BIAS_LUCK) && !(have_flag(o_ptr->flags, OF_IM_ELEC)) && double_check_immunity(o_ptr))
        {
            add_flag(o_ptr->flags, OF_IM_ELEC);
            if (!one_in_(IM_LUCK))
            {
                remove_flag(o_ptr->flags, OF_IM_ACID);
                remove_flag(o_ptr->flags, OF_IM_COLD);
                remove_flag(o_ptr->flags, OF_IM_FIRE);
            }

            if (one_in_(2)) return;
        }
        break;

    case BIAS_FIRE:
        if (!(have_flag(o_ptr->flags, OF_RES_FIRE)))
        {
            add_flag(o_ptr->flags, OF_RES_FIRE);
            if (one_in_(2)) return;
        }
        if ((o_ptr->tval >= TV_CLOAK) &&
            (o_ptr->tval <= TV_HARD_ARMOR) &&
            !(have_flag(o_ptr->flags, OF_AURA_FIRE)))
        {
            add_flag(o_ptr->flags, OF_AURA_FIRE);
            if (one_in_(2)) return;
        }
        if (one_in_(BIAS_LUCK)
            && !(have_flag(o_ptr->flags, OF_IM_FIRE))
            && double_check_immunity(o_ptr))
        {
            add_flag(o_ptr->flags, OF_IM_FIRE);
            if (!one_in_(IM_LUCK))
            {
                remove_flag(o_ptr->flags, OF_IM_ELEC);
                remove_flag(o_ptr->flags, OF_IM_COLD);
                remove_flag(o_ptr->flags, OF_IM_ACID);
            }
            if (one_in_(2)) return;
        }
        break;

    case BIAS_COLD:
        if (!(have_flag(o_ptr->flags, OF_RES_COLD)))
        {
            add_flag(o_ptr->flags, OF_RES_COLD);
            if (one_in_(2)) return;
        }
        if ((o_ptr->tval >= TV_CLOAK) &&
            (o_ptr->tval <= TV_HARD_ARMOR) &&
            !(have_flag(o_ptr->flags, OF_AURA_COLD)))
        {
            add_flag(o_ptr->flags, OF_AURA_COLD);
            if (one_in_(2)) return;
        }
        if (one_in_(BIAS_LUCK) && !(have_flag(o_ptr->flags, OF_IM_COLD)) && double_check_immunity(o_ptr))
        {
            add_flag(o_ptr->flags, OF_IM_COLD);
            if (!one_in_(IM_LUCK))
            {
                remove_flag(o_ptr->flags, OF_IM_ELEC);
                remove_flag(o_ptr->flags, OF_IM_ACID);
                remove_flag(o_ptr->flags, OF_IM_FIRE);
            }
            if (one_in_(2)) return;
        }
        break;

    case BIAS_POIS:
        if (!(have_flag(o_ptr->flags, OF_RES_POIS)))
        {
            add_flag(o_ptr->flags, OF_RES_POIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_WARRIOR:
        if (!one_in_(3) && (!(have_flag(o_ptr->flags, OF_RES_FEAR))))
        {
            add_flag(o_ptr->flags, OF_RES_FEAR);
            if (one_in_(2)) return;
        }
        if (one_in_(9) && (!(have_flag(o_ptr->flags, OF_NO_MAGIC))))
        {
            add_flag(o_ptr->flags, OF_NO_MAGIC);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_NECROMANTIC:
        if (!(have_flag(o_ptr->flags, OF_RES_NETHER)))
        {
            add_flag(o_ptr->flags, OF_RES_NETHER);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_RES_POIS)))
        {
            add_flag(o_ptr->flags, OF_RES_POIS);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_RES_DARK)))
        {
            add_flag(o_ptr->flags, OF_RES_DARK);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CHAOS:
        if (!(have_flag(o_ptr->flags, OF_RES_CHAOS)))
        {
            add_flag(o_ptr->flags, OF_RES_CHAOS);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_RES_CONF)))
        {
            add_flag(o_ptr->flags, OF_RES_CONF);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_RES_DISEN)))
        {
            add_flag(o_ptr->flags, OF_RES_DISEN);
            if (one_in_(2)) return;
        }
        break;
    }

    switch (randint1(42))
    {
        case 1:
            if (!one_in_(WEIRD_LUCK))
                random_resistance(o_ptr);
            else if (double_check_immunity(o_ptr))
            {
                add_flag(o_ptr->flags, OF_IM_ACID);
                if (!artifact_bias)
                    artifact_bias = BIAS_ACID;
            }
            break;
        case 2:
            if (!one_in_(WEIRD_LUCK))
                random_resistance(o_ptr);
            else if (double_check_immunity(o_ptr))
            {
                add_flag(o_ptr->flags, OF_IM_ELEC);
                if (!artifact_bias)
                    artifact_bias = BIAS_ELEC;
            }
            break;
        case 3:
            if (!one_in_(WEIRD_LUCK))
                random_resistance(o_ptr);
            else if (double_check_immunity(o_ptr))
            {
                add_flag(o_ptr->flags, OF_IM_COLD);
                if (!artifact_bias)
                    artifact_bias = BIAS_COLD;
            }
            break;
        case 4:
            if (!one_in_(WEIRD_LUCK))
                random_resistance(o_ptr);
            else if (double_check_immunity(o_ptr))
            {
                add_flag(o_ptr->flags, OF_IM_FIRE);
                if (!artifact_bias)
                    artifact_bias = BIAS_FIRE;
            }
            break;
        case 5:
        case 6:
        case 13:
            add_flag(o_ptr->flags, OF_RES_ACID);
            if (!artifact_bias)
                artifact_bias = BIAS_ACID;
            break;
        case 7:
        case 8:
        case 14:
            add_flag(o_ptr->flags, OF_RES_ELEC);
            if (!artifact_bias)
                artifact_bias = BIAS_ELEC;
            break;
        case 9:
        case 10:
        case 15:
            add_flag(o_ptr->flags, OF_RES_FIRE);
            if (!artifact_bias)
                artifact_bias = BIAS_FIRE;
            break;
        case 11:
        case 12:
        case 16:
            add_flag(o_ptr->flags, OF_RES_COLD);
            if (!artifact_bias)
                artifact_bias = BIAS_COLD;
            break;
        case 17:
        case 18:
            add_flag(o_ptr->flags, OF_RES_POIS);
            if (!artifact_bias && !one_in_(4))
                artifact_bias = BIAS_POIS;
            else if (!artifact_bias && one_in_(2))
                artifact_bias = BIAS_NECROMANTIC;
            else if (!artifact_bias && one_in_(2))
                artifact_bias = BIAS_ROGUE;
            break;
        case 19:
        case 20:
            add_flag(o_ptr->flags, OF_RES_FEAR);
            if (!artifact_bias && one_in_(3))
                artifact_bias = BIAS_WARRIOR;
            break;
        case 21:
            add_flag(o_ptr->flags, OF_RES_LITE);
            break;
        case 22:
            add_flag(o_ptr->flags, OF_RES_DARK);
            break;
        case 23:
        case 24:
            add_flag(o_ptr->flags, OF_RES_BLIND);
            break;
        case 25:
        case 26:
            add_flag(o_ptr->flags, OF_RES_CONF);
            if (!artifact_bias && one_in_(6))
                artifact_bias = BIAS_CHAOS;
            break;
        case 27:
        case 28:
            add_flag(o_ptr->flags, OF_RES_SOUND);
            break;
        case 29:
        case 30:
            add_flag(o_ptr->flags, OF_RES_SHARDS);
            break;
        case 31:
        case 32:
            add_flag(o_ptr->flags, OF_RES_NETHER);
            if (!artifact_bias && one_in_(3))
                artifact_bias = BIAS_NECROMANTIC;
            break;
        case 33:
        case 34:
            add_flag(o_ptr->flags, OF_RES_NEXUS);
            break;
        case 35:
        case 36:
            add_flag(o_ptr->flags, OF_RES_CHAOS);
            if (!artifact_bias && one_in_(2))
                artifact_bias = BIAS_CHAOS;
            break;
        case 37:
        case 38:
            add_flag(o_ptr->flags, OF_RES_DISEN);
            break;
        case 39:
            if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
                add_flag(o_ptr->flags, OF_AURA_ELEC);
            else
                random_resistance(o_ptr);
            if (!artifact_bias)
                artifact_bias = BIAS_ELEC;
            break;
        case 40:
            if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
                add_flag(o_ptr->flags, OF_AURA_FIRE);
            else
                random_resistance(o_ptr);
            if (!artifact_bias)
                artifact_bias = BIAS_FIRE;
            break;
        case 41:
            if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CLOAK ||
                o_ptr->tval == TV_HELM || o_ptr->tval == TV_HARD_ARMOR)
                add_flag(o_ptr->flags, OF_REFLECT);
            else
                random_resistance(o_ptr);
            break;
        case 42:
            if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
                add_flag(o_ptr->flags, OF_AURA_COLD);
            else
                random_resistance(o_ptr);
            if (!artifact_bias)
                artifact_bias = BIAS_COLD;
            break;
    }
}



static void random_misc(object_type * o_ptr)
{
    switch (artifact_bias)
    {
    case BIAS_RANGER:
        if (!(have_flag(o_ptr->flags, OF_SUST_CON)))
        {
            add_flag(o_ptr->flags, OF_SUST_CON);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_STR:
        if (!(have_flag(o_ptr->flags, OF_SUST_STR)))
        {
            add_flag(o_ptr->flags, OF_SUST_STR);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_WIS:
        if (!(have_flag(o_ptr->flags, OF_SUST_WIS)))
        {
            add_flag(o_ptr->flags, OF_SUST_WIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_INT:
        if (!(have_flag(o_ptr->flags, OF_SUST_INT)))
        {
            add_flag(o_ptr->flags, OF_SUST_INT);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_DEX:
        if (!(have_flag(o_ptr->flags, OF_SUST_DEX)))
        {
            add_flag(o_ptr->flags, OF_SUST_DEX);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CON:
        if (!(have_flag(o_ptr->flags, OF_SUST_CON)))
        {
            add_flag(o_ptr->flags, OF_SUST_CON);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CHR:
        if (!(have_flag(o_ptr->flags, OF_SUST_CHR)))
        {
            add_flag(o_ptr->flags, OF_SUST_CHR);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_CHAOS:
        if (!(have_flag(o_ptr->flags, OF_TELEPORT)))
        {
            add_flag(o_ptr->flags, OF_TELEPORT);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_FIRE:
        if (!(have_flag(o_ptr->flags, OF_LITE)))
        {
            add_flag(o_ptr->flags, OF_LITE); /* Freebie */
        }
        break;
    }

    switch (randint1(33))
    {
        case 1:
            add_flag(o_ptr->flags, OF_SUST_STR);
            if (!artifact_bias)
                artifact_bias = BIAS_STR;
            break;
        case 2:
            add_flag(o_ptr->flags, OF_SUST_INT);
            if (!artifact_bias)
                artifact_bias = BIAS_INT;
            break;
        case 3:
            add_flag(o_ptr->flags, OF_SUST_WIS);
            if (!artifact_bias)
                artifact_bias = BIAS_WIS;
            break;
        case 4:
            add_flag(o_ptr->flags, OF_SUST_DEX);
            if (!artifact_bias)
                artifact_bias = BIAS_DEX;
            break;
        case 5:
            add_flag(o_ptr->flags, OF_SUST_CON);
            if (!artifact_bias)
                artifact_bias = BIAS_CON;
            break;
        case 6:
            add_flag(o_ptr->flags, OF_SUST_CHR);
            if (!artifact_bias)
                artifact_bias = BIAS_CHR;
            break;
        case 7:
        case 8:
        case 14:
            add_flag(o_ptr->flags, OF_FREE_ACT);
            break;
        case 9:
            add_flag(o_ptr->flags, OF_HOLD_LIFE);
            if (!artifact_bias && one_in_(5))
                artifact_bias = BIAS_PRIESTLY;
            else if (!artifact_bias && one_in_(6))
                artifact_bias = BIAS_NECROMANTIC;
            break;
        case 10:
        case 11:
            add_flag(o_ptr->flags, OF_LITE);
            break;
        case 12:
        case 13:
            add_flag(o_ptr->flags, OF_LEVITATION);
            break;
        case 15:
        case 16:
        case 17:
            add_flag(o_ptr->flags, OF_SEE_INVIS);
            break;
        case 19:
        case 20:
            add_flag(o_ptr->flags, OF_SLOW_DIGEST);
            break;
        case 21:
        case 22:
            add_flag(o_ptr->flags, OF_REGEN);
            break;
        case 23:
            add_flag(o_ptr->flags, OF_TELEPORT);
            break;
        case 24:
        case 25:
        case 26:
            if (object_is_armour(o_ptr))
                random_misc(o_ptr);
            else
            {
                o_ptr->to_a = 4 + randint1(11);
            }
            break;
        case 27:
        case 28:
        case 29:
        {
            int bonus_h, bonus_d;
            add_flag(o_ptr->flags, OF_SHOW_MODS);
            bonus_h = 4 + (randint1(11));
            bonus_d = 4 + (randint1(11));
            if ((o_ptr->tval != TV_SWORD) && (o_ptr->tval != TV_POLEARM) && (o_ptr->tval != TV_HAFTED) && (o_ptr->tval != TV_DIGGING) && (o_ptr->tval != TV_GLOVES) && (o_ptr->tval != TV_RING))
            {
                bonus_h /= 2;
                bonus_d /= 2;
            }
            o_ptr->to_h += bonus_h;
            o_ptr->to_d += bonus_d;
            break;
        }
        case 30:
            add_flag(o_ptr->flags, OF_NO_MAGIC);
            break;
        case 31:
        {
            bool tele_proof = ((p_ptr->pclass == CLASS_BERSERKER) ? magik(10) : magik(90));
            if (tele_proof) add_flag(o_ptr->flags, OF_WARNING);
            else add_flag(o_ptr->flags, OF_NO_TELE);
            break;
        }
        case 32:
            add_flag(o_ptr->flags, OF_WARNING);
            break;

        case 18:
            switch (randint1(3))
            {
            case 1:
                add_flag(o_ptr->flags, OF_ESP_EVIL);
                if (!artifact_bias && one_in_(3))
                    artifact_bias = BIAS_LAW;
                break;
            case 2:
                add_flag(o_ptr->flags, OF_ESP_NONLIVING);
                if (!artifact_bias && one_in_(3))
                    artifact_bias = BIAS_MAGE;
                break;
            case 3:
                add_flag(o_ptr->flags, OF_TELEPATHY);
                if (!artifact_bias && one_in_(9))
                    artifact_bias = BIAS_MAGE;
                break;
            }
            break;

        case 33:
        {
            int idx[3];
            int n = randint1(3);

            idx[0] = randint1(8);

            idx[1] = randint1(7);
            if (idx[1] >= idx[0]) idx[1]++;

            idx[2] = randint1(6);
            if (idx[2] >= idx[0]) idx[2]++;
            if (idx[2] >= idx[1]) idx[2]++;

            while (n--) switch (idx[n])
            {
            case 1:
                add_flag(o_ptr->flags, OF_ESP_ANIMAL);
                if (!artifact_bias && one_in_(4))
                    artifact_bias = BIAS_RANGER;
                break;
            case 2:
                add_flag(o_ptr->flags, OF_ESP_UNDEAD);
                if (!artifact_bias && one_in_(3))
                    artifact_bias = BIAS_PRIESTLY;
                else if (!artifact_bias && one_in_(6))
                    artifact_bias = BIAS_NECROMANTIC;
                break;
            case 3:
                add_flag(o_ptr->flags, OF_ESP_DEMON);
                break;
            case 4:
                add_flag(o_ptr->flags, OF_ESP_ORC);
                break;
            case 5:
                add_flag(o_ptr->flags, OF_ESP_TROLL);
                break;
            case 6:
                add_flag(o_ptr->flags, OF_ESP_GIANT);
                break;
            case 7:
                add_flag(o_ptr->flags, OF_ESP_HUMAN);
                if (!artifact_bias && one_in_(6))
                    artifact_bias = BIAS_ROGUE;
                break;
            case 8:
                add_flag(o_ptr->flags, OF_ESP_GOOD);
                if (!artifact_bias && one_in_(3))
                    artifact_bias = BIAS_LAW;
                break;
            }
            break;
        }
    }
}

static void random_slay_aux(object_type *o_ptr)
{
    switch (randint1((o_ptr->tval == TV_BOW) ? 25 : (29 + object_level/15)))
    {
        case 1:
        case 2:
            add_flag(o_ptr->flags, OF_SLAY_ANIMAL);
            break;
        case 3:
        case 4:
            add_flag(o_ptr->flags, OF_SLAY_HUMAN);
            break;
        case 5:
            if (one_in_(5)) /* low level slay evil is rare ... */
            {
                add_flag(o_ptr->flags, OF_SLAY_EVIL);
                break;
            }
        case 6:
            add_flag(o_ptr->flags, OF_SLAY_ORC);
            break;
        case 7:
        case 8:
            add_flag(o_ptr->flags, OF_SLAY_TROLL);
            break;
        case 9:
        case 10:
            add_flag(o_ptr->flags, OF_SLAY_GIANT);
            break;
        case 11:
        case 12:
            if (randint1(150) <= object_level)
                add_flag(o_ptr->flags, OF_KILL_DRAGON);
            else
                add_flag(o_ptr->flags, OF_SLAY_DRAGON);
            break;
        case 13:
        case 14:
            add_flag(o_ptr->flags, OF_SLAY_DEMON);
            if (!artifact_bias && one_in_(9))
                artifact_bias = BIAS_PRIESTLY;
            break;
        case 15:
        case 16:
            add_flag(o_ptr->flags, OF_SLAY_UNDEAD);
            if (!artifact_bias && one_in_(9))
                artifact_bias = BIAS_PRIESTLY;
            break;
        case 17:
        case 18:
        case 19:
            if (o_ptr->tval == TV_SWORD)
            {
                add_flag(o_ptr->flags, OF_VORPAL);
                if (!artifact_bias && one_in_(9))
                    artifact_bias = BIAS_WARRIOR;
            }
            else
                random_slay_aux(o_ptr);
            break;
        case 20:
            if (o_ptr->tval == TV_HAFTED || o_ptr->tval == TV_DIGGING)
                add_flag(o_ptr->flags, OF_IMPACT);
            else
                random_slay_aux(o_ptr);
            break;
        case 21:
            add_flag(o_ptr->flags, OF_BRAND_FIRE);
            if (!artifact_bias)
                artifact_bias = BIAS_FIRE;
            break;
        case 22:
            add_flag(o_ptr->flags, OF_BRAND_COLD);
            if (!artifact_bias)
                artifact_bias = BIAS_COLD;
            break;
        case 23:
            add_flag(o_ptr->flags, OF_BRAND_ELEC);
            if (!artifact_bias)
                artifact_bias = BIAS_ELEC;
            break;
        case 24:
            add_flag(o_ptr->flags, OF_BRAND_ACID);
            if (!artifact_bias)
                artifact_bias = BIAS_ACID;
            break;
        case 25:
            add_flag(o_ptr->flags, OF_BRAND_POIS);
            if (!artifact_bias && !one_in_(3))
                artifact_bias = BIAS_POIS;
            else if (!artifact_bias && one_in_(6))
                artifact_bias = BIAS_NECROMANTIC;
            else if (!artifact_bias)
                artifact_bias = BIAS_ROGUE;
            break;
        case 26:
            add_flag(o_ptr->flags, OF_BRAND_VAMP);
            if (!artifact_bias)
                artifact_bias = BIAS_NECROMANTIC;
            break;
        case 27:
            if (have_flag(o_ptr->flags, OF_KILL_EVIL) && !one_in_(100))
            {
                random_slay_aux(o_ptr);
            }
            else if (have_flag(o_ptr->flags, OF_SLAY_EVIL) && !one_in_(20))
            {
                random_slay_aux(o_ptr);
            }
            else
            {
                add_flag(o_ptr->flags, OF_BRAND_MANA);
                if (!artifact_bias)
                    artifact_bias = (one_in_(2) ? BIAS_MAGE : BIAS_PRIESTLY);
            }
            break;
        case 28:
            add_flag(o_ptr->flags, OF_BRAND_CHAOS);
            if (!artifact_bias)
                artifact_bias = BIAS_CHAOS;
            break;
        case 29:
            add_flag(o_ptr->flags, OF_BRAND_VAMP);
            if (!artifact_bias)
                artifact_bias = BIAS_NECROMANTIC;
            break;
        case 30:
            add_flag(o_ptr->flags, OF_KILL_DEMON);
            if (!artifact_bias && one_in_(3))
                artifact_bias = BIAS_PRIESTLY;
            break;
        case 31:
            add_flag(o_ptr->flags, OF_KILL_UNDEAD);
            if (!artifact_bias && one_in_(3))
                artifact_bias = BIAS_PRIESTLY;
            break;
        default:
            if (!have_flag(o_ptr->flags, OF_BRAND_ORDER) && !have_flag(o_ptr->flags, OF_BRAND_WILD) && one_in_(100))
                add_flag(o_ptr->flags, OF_BRAND_ORDER);
            else if (!have_flag(o_ptr->flags, OF_BRAND_WILD) && !have_flag(o_ptr->flags, OF_BRAND_ORDER) &&  one_in_(100))
                add_flag(o_ptr->flags, OF_BRAND_WILD);
            else
            {
                if (randint1(1250) <= object_level - 50)
                    add_flag(o_ptr->flags, OF_KILL_EVIL);
                else if (o_ptr->tval == TV_SWORD && randint1(625) <= object_level - 50)
                {
                    add_flag(o_ptr->flags, OF_VORPAL2);
                    break;
                }
                else if (o_ptr->tval == TV_HAFTED && randint1(625) <= object_level - 50)
                {
                    add_flag(o_ptr->flags, OF_STUN);
                    break;
                }
                else
                    add_flag(o_ptr->flags, OF_SLAY_EVIL);
                if (!artifact_bias && one_in_(2))
                    artifact_bias = BIAS_LAW;
                else if (!artifact_bias && one_in_(9))
                    artifact_bias = BIAS_PRIESTLY;
            }
            break;
    }
}
static void random_slay(object_type *o_ptr)
{
    if (o_ptr->tval == TV_BOW)
    {
        if (o_ptr->sval == SV_CRIMSON || o_ptr->sval == SV_RAILGUN)
        {
            if (one_in_(2))
            {
                random_plus(o_ptr);
                has_pval = TRUE;
            }
            else
                one_high_resistance(o_ptr);
            return;
        }
        else if (obj_is_harp(o_ptr))
        {
            if (one_in_(2))
                random_resistance(o_ptr);
            else
            {
                random_plus(o_ptr);
                has_pval = TRUE;
            }
            return;
        }
        switch (randint1(6))
        {
            case 1:
                if (!one_in_(6))
                {
                    random_slay_aux(o_ptr);
                    break;
                }
            case 2:
            case 3:
                add_flag(o_ptr->flags, OF_XTRA_MIGHT);
                if (!one_in_(2)) remove_flag(o_ptr->flags, OF_XTRA_SHOTS);
                if (!artifact_bias && one_in_(9))
                    artifact_bias = BIAS_RANGER;
                break;
            default:
                add_flag(o_ptr->flags, OF_XTRA_SHOTS);
                if (!one_in_(2)) remove_flag(o_ptr->flags, OF_XTRA_MIGHT);
                has_pval = TRUE;
                if (!artifact_bias && one_in_(9))
                    artifact_bias = BIAS_RANGER;
                break;
        }
        return;
    }

    switch (artifact_bias)
    {
    case BIAS_CHAOS:
        if (!(have_flag(o_ptr->flags, OF_BRAND_CHAOS)))
        {
            add_flag(o_ptr->flags, OF_BRAND_CHAOS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_PRIESTLY:
        if((o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM) &&
           !(have_flag(o_ptr->flags, OF_BLESSED)))
        {
            /* A free power for "priestly" random artifacts */
            add_flag(o_ptr->flags, OF_BLESSED);
        }
        break;

    case BIAS_NECROMANTIC:
        if (!(have_flag(o_ptr->flags, OF_BRAND_VAMP)))
        {
            add_flag(o_ptr->flags, OF_BRAND_VAMP);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_BRAND_POIS)) && one_in_(2))
        {
            add_flag(o_ptr->flags, OF_BRAND_POIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_RANGER:
        if (!(have_flag(o_ptr->flags, OF_SLAY_ANIMAL)))
        {
            add_flag(o_ptr->flags, OF_SLAY_ANIMAL);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_ROGUE:
        if ((((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_DAGGER)) ||
             ((o_ptr->tval == TV_POLEARM) && (o_ptr->sval == SV_SPEAR))) &&
             !(have_flag(o_ptr->flags, OF_THROWING)))
        {
            /* Free power for rogues... */
            add_flag(o_ptr->flags, OF_THROWING);
        }
        if (!(have_flag(o_ptr->flags, OF_BRAND_POIS)))
        {
            add_flag(o_ptr->flags, OF_BRAND_POIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_POIS:
        if (!(have_flag(o_ptr->flags, OF_BRAND_POIS)))
        {
            add_flag(o_ptr->flags, OF_BRAND_POIS);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_FIRE:
        if (!(have_flag(o_ptr->flags, OF_BRAND_FIRE)))
        {
            add_flag(o_ptr->flags, OF_BRAND_FIRE);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_COLD:
        if (!(have_flag(o_ptr->flags, OF_BRAND_COLD)))
        {
            add_flag(o_ptr->flags, OF_BRAND_COLD);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_ELEC:
        if (!(have_flag(o_ptr->flags, OF_BRAND_ELEC)))
        {
            add_flag(o_ptr->flags, OF_BRAND_ELEC);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_ACID:
        if (!(have_flag(o_ptr->flags, OF_BRAND_ACID)))
        {
            add_flag(o_ptr->flags, OF_BRAND_ACID);
            if (one_in_(2)) return;
        }
        break;

    case BIAS_LAW:
        if (!(have_flag(o_ptr->flags, OF_SLAY_EVIL)))
        {
            add_flag(o_ptr->flags, OF_SLAY_EVIL);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_SLAY_UNDEAD)))
        {
            add_flag(o_ptr->flags, OF_SLAY_UNDEAD);
            if (one_in_(2)) return;
        }
        if (!(have_flag(o_ptr->flags, OF_SLAY_DEMON)))
        {
            add_flag(o_ptr->flags, OF_SLAY_DEMON);
            if (one_in_(2)) return;
        }
        break;
    }

    random_slay_aux(o_ptr);
}

static void get_random_name_aux_aux(cptr file_name, int entry, int yrkka, char *output)
{
    int i;
    if (yrkka < 10) yrkka = 10;
    if (yrkka > 1000) yrkka = 1000;
    while (1)
    {
        bool sopii = TRUE;
        get_rnd_line(file_name, entry, output);
        if (quark__num == QUARK_MAX) return;

        /* Try to avoid quark duplication */
        for (i = 1; i < quark__num; i++)
        {
            /* Check for equality */
            if (streq(quark__str[i], output))
            {
                sopii = FALSE;
                break;
            }
        }
        if (sopii) return;
        yrkka--;
        if (yrkka < 1) return;
    }
}

static void get_random_name_aux(char *return_name, object_type *o_ptr, int power)
{
    /* Hack: BIAS_* got converted to bits but the artifact name files still use
       old values. Fortunately, the bit position in the new constants will
       recover the old values.
       TODO: Change the parser (e.g. N:18:BIAS_WARRIOR -> N:BIAS_WARRIOR)
       so this doesn't happen again.
    */
    int bias_hack = 0;
    int temp = artifact_bias;

    while (temp)
    {
        temp >>= 1;
        bias_hack++;
    }

    if (o_ptr->tval == TV_LITE)
    {
        cptr filename;
        if (have_flag(o_ptr->flags, OF_DARKNESS)) filename = "lite_drk.txt";
        else
        {
            switch (power)
            {
            case 0:
                filename = "lite_cursed.txt";
                break;
            case 1:
                filename = "lite_low.txt";
                break;
            case 2:
                filename = "lite_med.txt";
                break;
            default:
                filename = "lite_high.txt";
            }
        }
        get_random_name_aux_aux(filename, bias_hack, 40, return_name);
    }
    else if (o_ptr->tval == TV_RING)
    {
        cptr filename;
        switch (power)
        {
        case 0:
            filename = "ring_cursed.txt";
            break;
        case 1:
            filename = "ring_low.txt";
            break;
        case 2:
            filename = "ring_med.txt";
            break;
        default:
            filename = "ring_high.txt";
        }
        get_random_name_aux_aux(filename, bias_hack, 60, return_name);
    }
    else if (o_ptr->tval == TV_AMULET)
    {
        cptr filename;
        switch (power)
        {
        case 0:
            filename = "amu_cursed.txt";
            break;
        case 1:
            filename = "amu_low.txt";
            break;
        case 2:
            filename = "amu_med.txt";
            break;
        default:
            filename = "amu_high.txt";
        }
        get_random_name_aux_aux(filename, bias_hack, 80, return_name);
    }
    else if (object_is_bow(o_ptr))
    {
        cptr filename = "ranged.txt";
        get_random_name_aux_aux(filename, one_in_(2) ? o_ptr->sval : 0, 60, return_name);
    }
    else
    {
        cptr filename;

        if (object_is_armour(o_ptr))
        {
            if (magik(40))
            {
                switch (o_ptr->tval)
                {
                    case TV_SOFT_ARMOR:
                    case TV_HARD_ARMOR:
                    case TV_DRAG_ARMOR:
                        filename = "aa_med.txt";
                        break;
                    case TV_BOOTS:
                        filename = "ab_med.txt";
                        break;
                    case TV_CLOAK:
                        filename = "ac_med.txt";
                        break;
                    case TV_GLOVES:
                        filename = "ag_med.txt";
                        break;
                    case TV_HELM:
                    case TV_CROWN:
                        filename = "ah_med.txt";
                        break;
                    default:
                        filename = "as_med.txt";
                        break;
                }
            }
            else
            {
                switch (power)
                {
                    case 0:
                        filename = "a_cursed.txt";
                        break;
                    case 1:
                    case 2:
                        filename = "a_med.txt";
                        break;
                    default:
                        filename = "a_high.txt";
                        break;
                }
            }
        }
        else /* weapon/digger */
        {
            int _odds = (bias_hack <= 0) ? 6 : 17;
            if (o_ptr->tval == TV_DIGGING) _odds = 2;
            if (one_in_(_odds))
            {
                filename = "w_types.txt";
                bias_hack = (100 * o_ptr->tval) + o_ptr->sval;
            }
            else if ((o_ptr->tval != TV_DIGGING) && (magik((power == 2) ? 33 : 55)))
            {
                switch (o_ptr->tval)
                {
                    case TV_SWORD:
                        filename = "w_sword.txt";
                        break;
                    case TV_HAFTED:
                        filename = "w_hafted.txt";
                        break;
                    default:
                        filename = "w_pole.txt";
                        break;
                }
            }
            else
            {
                switch (power)
                {
                case 0:
                    filename = "w_cursed.txt";
                    break;
                case 1:
                case 2:
                    filename = "w_med.txt";
                    break;
                default:
                    filename = "w_high.txt";
                }
            }
        }

        get_random_name_aux_aux(filename, bias_hack, 25, return_name);
    }
}

void get_random_name(char *return_name, object_type *o_ptr, int power)
{
    int attempt;
    return_name[0] = '\0';
    for (attempt = 0;; ++attempt)
    {
        get_random_name_aux(return_name, o_ptr, power);
        if (return_name[0] != '\0') break;
        if (attempt > 100) break;
    }
}

void curse_object(object_type *o_ptr)
{
    int ct = randint1(2);
    int v = obj_value_real(o_ptr) / 10000;
    int i;

    o_ptr->curse_flags |= OFC_CURSED;

    one_biff(o_ptr);
    ct--;

    ct += randint1(randint1(v)); /* TODO */

    for (i = 0; i < ct; i++)
    {
        int n = randint0(70 + v*v);
        if (n < 25)
        {
            do { o_ptr->curse_flags |= get_curse(0, o_ptr); } while (one_in_(2));
        }
        else if (n < 35)
            o_ptr->curse_flags |= get_curse(1, o_ptr);
        else if (n < 40)
            o_ptr->curse_flags |= OFC_HEAVY_CURSE;
        else if (n < 45)
            o_ptr->curse_flags |= get_curse(2, o_ptr);
        else if (n < 100)
            one_biff(o_ptr);
        else if (n < 105)
            add_flag(o_ptr->flags, OF_AGGRAVATE);
        else if (n < 107)
            add_flag(o_ptr->flags, OF_TY_CURSE);
        else if (n < 108)
            o_ptr->curse_flags |= OFC_PERMA_CURSE;
        else if (n < 250)
            one_biff(o_ptr);
        else
        {
            if (one_in_(2))
            {
                o_ptr->curse_flags |= OFC_PERMA_CURSE;
                if (one_in_(2))
                    add_flag(o_ptr->flags, OF_TY_CURSE);
                if (one_in_(2))
                    add_flag(o_ptr->flags, OF_NO_TELE);
                if (one_in_(2))
                    add_flag(o_ptr->flags, OF_AGGRAVATE);
            }
            else
                one_biff(o_ptr);
        }
    }
}

typedef struct {
    cptr  name;
    obj_p pred;
    int   weight;
} _slot_weight_t, *_slot_weight_ptr;
static _slot_weight_t _slot_weight_tbl[] = {
    {"Weapons", object_is_melee_weapon, 80},
    {"Shields", object_is_shield, 52},
    {"Bows", object_is_bow, 60},
    {"Rings", object_is_ring, 55},
    {"Amulets", object_is_amulet, 40},
    {"Lights", object_is_lite, 36},
    {"Body Armor", object_is_body_armour, 80},
    {"Cloaks", object_is_cloak, 43},
    {"Helmets", object_is_helmet, 50},
    {"Gloves", object_is_gloves, 45},
    {"Boots", object_is_boots, 50},
    {NULL}
};
int get_slot_weight(obj_ptr obj)
{
    int i;

    /* Hack - harps are too strong
     * Also, non-bards shouldn't get full benefits from harps
     * Since this would be hard to code, we just generate weaker harps
     * This code also prevents reforges onto mundanified guns from getting
     * completely ridiculous */
    if ((obj->tval == TV_BOW) && (obj->sval > SV_RANGED_MAX_NORMAL) && (obj->sval != SV_NAMAKE_BOW))
    {
        return ((obj_is_harp(obj)) && (p_ptr->pclass == CLASS_BARD)) ? 50 : 40;
    }

    for (i = 0; ; i++)
    {
        _slot_weight_ptr row = &_slot_weight_tbl[i];
        if (!row->name) break;
        if (row->pred(obj)) return row->weight;
    }
    return 80;
}

int get_dest_weight(obj_ptr dest)
{
    int dest_weight = get_slot_weight(dest);
    /* Penalize reforging a powerful slot into a weak slot (e.g. weapons into lites)
     * Also penalize moving power into ninja weapons */
    if (object_is_melee_weapon(dest))
    {
        int dice = k_info[dest->k_idx].dd * k_info[dest->k_idx].ds;
        if (dice == 2) /* hack for wizardstaves */
        {
            dest_weight = 55;
        }
        else if (dice < 12)
            dest_weight = dest_weight * dice / 12;
    }
    return dest_weight;
}

/* This might also benefit the rand-art power channels in ego.c */
int get_slot_power(obj_ptr obj)
{
    int w = get_slot_weight(obj);
    if (object_is_melee_weapon(obj))
    {
        int d = k_info[obj->k_idx].dd * k_info[obj->k_idx].ds;
        if (d < 12)
            w = w * d / 12;
    }
    else if (object_is_body_armour(obj))
    {
        int ac = k_info[obj->k_idx].ac;
        if (ac < 16)
            w = w * (ac + 20) / 36;
    }
    return 100 * w / 80;
}

static int _slot_normalize(int amt, int pct)
{
    int n = amt * 10 * pct / 100;
    amt = n / 10;
    if (n%10 && randint0(10) < n%10) amt++;
    return amt;
}

static int _roll_powers(obj_ptr obj, int lev)
{
    int powers;
    int pct = get_slot_weight(obj) * 100 / 80;
    int spread = _slot_normalize(6, pct);
    int max_powers = _slot_normalize(12, pct);

    powers = randint1(spread) + 1;

    while (one_in_(powers) || one_in_(7 * 90/lev) || one_in_(10 * 70/lev))
        powers++;

    if (one_in_(WEIRD_LUCK))
        powers *= 2;

    if (powers > max_powers)
        powers = max_powers;

    return powers;
}

s32b create_artifact(object_type *o_ptr, u32b mode)
{
    char    new_name[1024];
    int     powers = 0;
    int     power_level;
    s32b    total_flags;
    bool    a_cursed = FALSE;
    int     warrior_artifact_bias = 0;
    int     i;
    bool    has_resistance = FALSE, boosted_ac = FALSE, boosted_dam = FALSE, boosted_hit = FALSE;
    int     esp_count = 0;
    int     lev = object_level;
    bool    is_falcon_sword = FALSE;

    if (no_artifacts) return 0;
    if (have_flag(o_ptr->flags, OF_NO_REMOVE)) return 0;
    if (o_ptr->tval == TV_QUIVER) return 0;

    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_HAYABUSA)
        is_falcon_sword = TRUE;

    immunity_hack = FALSE;
    slaying_hack = 0;
    has_pval = FALSE;

    if (obj_is_harp(o_ptr))
        has_pval = TRUE;

    /* Hack for Demeter. Torches start with a pval of 4000! Not sure about lanterns ... */
    if (o_ptr->tval == TV_LITE /*&& o_ptr->sval == SV_LITE_TORCH*/)
        o_ptr->pval = 0;

    if (lev > 127)
        lev = 127; /* no going to heaven or hell for uber nutso craziness */
    if (lev < 1)
        lev = 1;   /* town? */

    /* Reset artifact bias */
    artifact_bias = 0;

    /* Nuke enchantments */
    o_ptr->name1 = 0;
    o_ptr->name2 = 0;
    o_ptr->name3 = 0;

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        o_ptr->flags[i] |= k_info[o_ptr->k_idx].flags[i];

    if (o_ptr->pval) has_pval = TRUE;

    if (obj_drop_theme)
    {
        switch (obj_drop_theme)
        {
        case R_DROP_WARRIOR:
        case R_DROP_WARRIOR_SHOOT:
        case R_DROP_SAMURAI:
        case R_DROP_DWARF:
            artifact_bias = BIAS_WARRIOR;
            break;
        case R_DROP_ARCHER:
            artifact_bias = BIAS_ARCHER;
            break;
        case R_DROP_MAGE:
            artifact_bias = BIAS_MAGE;
            break;
        case R_DROP_PRIEST:
            artifact_bias = BIAS_PRIESTLY;
            break;
        case R_DROP_PRIEST_EVIL:
            artifact_bias = BIAS_NECROMANTIC;
            break;
        case R_DROP_PALADIN:
            artifact_bias = BIAS_LAW;
            warrior_artifact_bias = 60;
            break;
        case R_DROP_PALADIN_EVIL:
            artifact_bias = BIAS_NECROMANTIC;
            warrior_artifact_bias = 60;
            break;
        case R_DROP_NINJA:
        case R_DROP_HOBBIT:
            artifact_bias = BIAS_ROGUE;
            break;
        case R_DROP_ROGUE:
            artifact_bias = BIAS_ROGUE;
            warrior_artifact_bias = 50;
            break;
        }
    }

    if (!artifact_bias && (mode & (CREATE_ART_SCROLL/* | CREATE_ART_GOOD*/)) && one_in_(4))
    {
        switch (p_ptr->pclass)
        {
            case CLASS_WARRIOR:
            case CLASS_BERSERKER:
            case CLASS_ARCHER:
            case CLASS_SAMURAI:
            case CLASS_CAVALRY:
            case CLASS_WEAPONSMITH:
            case CLASS_DUELIST:
            case CLASS_BLOOD_KNIGHT:
            case CLASS_WEAPONMASTER:
            case CLASS_RUNE_KNIGHT:
            case CLASS_MAULER:
                artifact_bias = BIAS_WARRIOR;
                break;
            case CLASS_MAGE:
            case CLASS_HIGH_MAGE:
            case CLASS_SORCERER:
            case CLASS_MAGIC_EATER:
            case CLASS_BLUE_MAGE:
            case CLASS_WARLOCK:
            case CLASS_BLOOD_MAGE:
            case CLASS_NECROMANCER:
            case CLASS_DEVICEMASTER:
            case CLASS_YELLOW_MAGE:
            case CLASS_GRAY_MAGE:
                artifact_bias = BIAS_MAGE;
                warrior_artifact_bias = 20;
                break;
            case CLASS_PRIEST:
                artifact_bias = BIAS_PRIESTLY;
                warrior_artifact_bias = 30;
                break;
            case CLASS_ROGUE:
            case CLASS_NINJA:
            case CLASS_SCOUT:
            case CLASS_NINJA_LAWYER:
            case CLASS_ALCHEMIST:
                artifact_bias = BIAS_ROGUE;
                warrior_artifact_bias = 30;
                break;
            case CLASS_RANGER:
            case CLASS_SNIPER:
                artifact_bias = BIAS_RANGER;
                warrior_artifact_bias = 30;
                break;
            case CLASS_PALADIN:
                artifact_bias = BIAS_PRIESTLY;
                warrior_artifact_bias = 60;
                break;
            case CLASS_WARRIOR_MAGE:
            case CLASS_RED_MAGE:
            case CLASS_PSION:
                artifact_bias = BIAS_MAGE;
                warrior_artifact_bias = 20;
                break;
            case CLASS_CHAOS_WARRIOR:
                artifact_bias = BIAS_CHAOS;
                warrior_artifact_bias = 60;
                break;
            case CLASS_LAWYER:
            case CLASS_POLITICIAN:
            case CLASS_MONK:
            case CLASS_FORCETRAINER:
                artifact_bias = BIAS_PRIESTLY;
                warrior_artifact_bias = 40;
                break;
            case CLASS_MINDCRAFTER:
                artifact_bias = BIAS_PRIESTLY;
                warrior_artifact_bias = 20;
                break;
            case CLASS_BARD:
                artifact_bias = BIAS_PRIESTLY;
                warrior_artifact_bias = 20;
                break;
            case CLASS_TOURIST:
            case CLASS_ARCHAEOLOGIST:
            case CLASS_TIME_LORD:
                warrior_artifact_bias = 20;
                break;
            case CLASS_BEASTMASTER:
                artifact_bias = BIAS_CHR;
                warrior_artifact_bias = 30;
                break;
            case CLASS_MIRROR_MASTER:
                if (randint1(4) > 1)
                {
                    artifact_bias = BIAS_MAGE;
                }
                else
                {
                    artifact_bias = BIAS_ROGUE;
                }
                warrior_artifact_bias = 20;
                break;
            case CLASS_DISCIPLE:
                if (disciple_is_(DISCIPLE_KARROT))
                {
                    artifact_bias = BIAS_CHAOS;
                    warrior_artifact_bias = 40;
                }
                else if (disciple_is_(DISCIPLE_TROIKA))
                {
                    artifact_bias = BIAS_WARRIOR;
                }
                else
                {
                    artifact_bias = BIAS_INT;
                    warrior_artifact_bias = 30;
                }
                break;
            default:
                artifact_bias = BIAS_WARRIOR;
                break;
        }
    }

    if ((mode & (CREATE_ART_SCROLL | CREATE_ART_GOOD)) && (randint1(100) <= warrior_artifact_bias))
        artifact_bias = BIAS_WARRIOR;

    strcpy(new_name, "");

    if (!(mode & CREATE_ART_GOOD) && one_in_(A_CURSED))
        a_cursed = TRUE;

    if (mode & CREATE_ART_CURSED)
        a_cursed = TRUE;

    if ( (o_ptr->tval == TV_AMULET || o_ptr->tval == TV_RING)
      && object_is_cursed(o_ptr) )
    {
        a_cursed = TRUE;
    }

    powers = _roll_powers(o_ptr, lev);

    /* Playtesting shows that FA, SI and HL are too rare ... let's boost these a bit */
    switch (o_ptr->tval)
    {
    case TV_BOLT:
    case TV_SHOT:
    case TV_ARROW:
        o_ptr->number = 1;    /* Where's the best place for this? */
        break;

    case TV_RING:
        if (one_in_(2))
            add_flag(o_ptr->flags, OF_FREE_ACT);
        if (one_in_(2))
            add_flag(o_ptr->flags, OF_SEE_INVIS);
        if ((randint1(150) < powers))
        {
            add_flag(o_ptr->flags, OF_LIFE);
            has_pval = TRUE;
            powers--;
        }
        break;

    case TV_AMULET:
        if (one_in_(3))
            add_flag(o_ptr->flags, OF_FREE_ACT);
        if (one_in_(3))
            add_flag(o_ptr->flags, OF_HOLD_LIFE);
        break;


    case TV_LITE:
        if (one_in_(3))
            add_flag(o_ptr->flags, OF_HOLD_LIFE);
        if (one_in_(10))
            add_flag(o_ptr->flags, OF_DARKNESS);
        break;

    case TV_BOOTS:
        if (one_in_(4))
            add_flag(o_ptr->flags, OF_FREE_ACT);
        if (one_in_(7))
            add_flag(o_ptr->flags, OF_LEVITATION);
        break;

    case TV_GLOVES:
        if (one_in_(4))
            add_flag(o_ptr->flags, OF_FREE_ACT);
        break;

    case TV_HELM:
        if (one_in_(4))
            add_flag(o_ptr->flags, OF_SEE_INVIS);
        break;
    }

    /* Main loop */
    while (powers > 0)
    {
        powers--;
        /* Attempt to craft artifacts by appropriate to type. For example, arrows should
           not grant extra blows or +3 strength :)  More realistically, weapons should
           preferentially get slays while armor should favor resists */
        switch (o_ptr->tval)
        {
        case TV_BOLT:
        case TV_SHOT:
        case TV_ARROW:
            /* Ammo either boosts up damage dice, or gains slays */
            if ( randint1(225) < lev
              && slaying_hack == 0 ) /* OK: Slaying can now only happen once! */
            {
                if (slaying_hack == 0 && one_in_(3)) /* double damage */
                {
                    powers -= o_ptr->dd - 1;
                    o_ptr->dd *= 2;
                    slaying_hack += o_ptr->dd;
                }
                else
                {
                    powers++;
                    powers++;
                    do
                    {
                        o_ptr->dd++;
                        powers--;
                        slaying_hack++;
                    }
                    while (one_in_(o_ptr->dd));

                    do
                    {
                        o_ptr->ds++;
                        powers--;
                        slaying_hack++;
                    }
                    while (one_in_(o_ptr->ds));
                }
            }
            else
                random_slay(o_ptr);
            break;

        case TV_LITE:
            switch (randint1(7))
            {
            case 1: case 2:
                random_plus(o_ptr);
                has_pval = TRUE;
                break;
            case 3:
                if (one_in_(45))
                {
                    add_flag(o_ptr->flags, OF_TELEPATHY);
                }
                else if (one_in_(15))
                {
                    effect_add(o_ptr, EFFECT_CLAIRVOYANCE);
                }
                else if (one_in_(77))
                {
                    add_flag(o_ptr->flags, OF_SPELL_POWER);
                    add_flag(o_ptr->flags, OF_DEC_STR);
                    add_flag(o_ptr->flags, OF_DEC_DEX);
                    add_flag(o_ptr->flags, OF_DEC_CON);
                    has_pval = TRUE;
                }
                else
                {
                    random_plus(o_ptr);
                    has_pval = TRUE;
                }
                break;
            case 4: case 5: case 6:
                if (one_in_(3))
                    one_high_resistance(o_ptr);
                else
                    random_resistance(o_ptr);
                break;
            case 7:
                random_misc(o_ptr);
                break;
            }
            break;

        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
            switch (randint1(7))
            {
            case 1: case 2:
                random_plus(o_ptr);
                has_pval = TRUE;
                break;
            case 3:
                if ( slaying_hack == 0 /* OK: Slaying can now only happen once! */
                    && (!object_is_(o_ptr, TV_POLEARM, SV_DEATH_SCYTHE))
                    && (is_falcon_sword || !have_flag(o_ptr->flags, OF_BLOWS)))
                {
                    object_kind *k_ptr = &k_info[o_ptr->k_idx];
                    int          max = MIN(81, k_ptr->dd * k_ptr->ds * 4); /* 9d9 */
                    int          dd = o_ptr->dd;
                    int          ds = o_ptr->ds;
                    slaying_hack++;
                    dd++;
                    while (randint0(1000) < 4000 / (dd * ds)) /* <--> 1 in (dd*ds/4) */
                    {
                        if (one_in_(2))
                        {
                            dd++;
                            if (dd * ds > max)
                            {
                                dd--;
                                break;
                            }
                        }
                        else
                        {
                            ds++;
                            if (dd * ds > max)
                            {
                                ds--;
                                break;
                            }
                        }
                        powers--;
                        slaying_hack++;
                    }
                    o_ptr->dd = dd;
                    o_ptr->ds = ds;
                    assert(dd * ds <= max);
                }
                else if (!boosted_dam && !boosted_hit)
                {
                    if (one_in_(7))
                    {
                        o_ptr->to_h += randint1(8);
                        o_ptr->to_d += randint1(8);
                        boosted_dam = TRUE;
                        boosted_hit = TRUE;
                    }
                    else if (one_in_(2))
                    {
                        o_ptr->to_h += randint1(8);
                        boosted_hit = TRUE;
                    }
                    else
                    {
                        o_ptr->to_d += randint1(8);
                        boosted_dam = TRUE;
                    }
                }
                else
                    random_resistance(o_ptr);
                break;
            case 4:
                random_misc(o_ptr);
                break;
            case 5: case 6: case 7:
                random_slay(o_ptr);
                break;
            }
            break;

        case TV_BOW:
            switch (randint1(9))
            {
            case 1: case 2:
                random_plus(o_ptr);
                has_pval = TRUE;
                break;
            case 3: case 4: case 5:
                random_slay(o_ptr);
                break;
            case 6: case 7:
                random_misc(o_ptr);
                break;
            case 8: case 9:
                random_resistance(o_ptr);
                break;
            }
            break;

        case TV_RING:
            switch (randint1(7))
            {
            case 1: case 2:
                random_plus(o_ptr);
                has_pval = TRUE;
                break;
            case 3:
                if (one_in_(10))
                {
                    add_flag(o_ptr->flags, OF_SPEED);
                    has_pval = TRUE;
                }
                else if ( (artifact_bias == BIAS_MAGE && one_in_(10))
                       || one_in_(50) )
                {
                    add_flag(o_ptr->flags, OF_DEC_MANA);
                }
                else if (one_in_(20))
                {
                    add_flag(o_ptr->flags, OF_WEAPONMASTERY);
                    has_pval = TRUE;
                }
                else if (one_in_(10) && randint1(150) < lev - 50)
                {
                    add_flag(o_ptr->flags, OF_BLOWS);
                    has_pval = TRUE;
                }
                else if (one_in_(50))
                {
                    add_flag(o_ptr->flags, OF_XTRA_MIGHT);
                    if (!one_in_(7)) remove_flag(o_ptr->flags, OF_XTRA_SHOTS);
                    has_pval = TRUE;
                }
                else if (one_in_(50))
                {
                    add_flag(o_ptr->flags, OF_XTRA_SHOTS);
                    if (!one_in_(7)) remove_flag(o_ptr->flags, OF_XTRA_MIGHT);
                    has_pval = TRUE;
                }
                else if (one_in_(15))
                {
                    add_flag(o_ptr->flags, OF_SUST_STR);
                    add_flag(o_ptr->flags, OF_SUST_INT);
                    add_flag(o_ptr->flags, OF_SUST_WIS);
                    add_flag(o_ptr->flags, OF_SUST_DEX);
                    add_flag(o_ptr->flags, OF_SUST_CON);
                    add_flag(o_ptr->flags, OF_SUST_CHR);
                    add_flag(o_ptr->flags, OF_HOLD_LIFE);
                    powers--;
                }
                else if (one_in_(77))
                {
                    add_flag(o_ptr->flags, OF_SPELL_POWER);
                    add_flag(o_ptr->flags, OF_DEC_STR);
                    add_flag(o_ptr->flags, OF_DEC_DEX);
                    add_flag(o_ptr->flags, OF_DEC_CON);
                    has_pval = TRUE;
                }
                else if (one_in_(3))
                {
                    add_flag(o_ptr->flags, OF_SHOW_MODS);
                    o_ptr->to_h += randint1(5) + m_bonus(5, lev);
                    o_ptr->to_d += randint1(5) + m_bonus(5, lev);
                }
                else if (one_in_(3))
                {
                    o_ptr->to_a += randint1(5) + m_bonus(5, lev);
                }
                else if (one_in_(20))
                {
                    switch (randint1(5))
                    {
                    case 1: add_flag(o_ptr->flags, OF_BRAND_ACID); break;
                    case 2: add_flag(o_ptr->flags, OF_BRAND_ELEC); break;
                    case 3: add_flag(o_ptr->flags, OF_BRAND_FIRE); break;
                    case 4: add_flag(o_ptr->flags, OF_BRAND_COLD); break;
                    case 5: add_flag(o_ptr->flags, OF_BRAND_VAMP); break;
                    }
                }
                else
                {
                    one_high_resistance(o_ptr);
                }
                break;
            case 4:
                if (one_in_(3))
                {
                    add_flag(o_ptr->flags, OF_SHOW_MODS);
                    o_ptr->to_h += randint1(5) + m_bonus(5, lev);
                    o_ptr->to_d += randint1(5) + m_bonus(5, lev);
                    break;
                }
                else if (one_in_(2))
                {
                    random_plus(o_ptr);
                    has_pval = TRUE;
                    break;
                }
                else if (one_in_(5))
                {
                    one_high_resistance(o_ptr);
                    break;
                }
            case 5:
                if (one_in_(2))
                {
                    random_plus(o_ptr);
                    has_pval = TRUE;
                }
                else
                    random_resistance(o_ptr);
                break;
            case 6:
                random_resistance(o_ptr);
                break;
            case 7:
                random_misc(o_ptr);
                break;
            }
            break;

        case TV_AMULET:
            switch (randint1(7))
            {
            case 1: case 2:
                random_plus(o_ptr);
                has_pval = TRUE;
                break;
            case 3:
                if (one_in_(45))
                {
                    add_flag(o_ptr->flags, OF_TELEPATHY);
                }
                else if ( (artifact_bias == BIAS_MAGE && one_in_(10))
                       || one_in_(50) )
                {
                    add_flag(o_ptr->flags, OF_DEC_MANA);
                }
                else if (one_in_(7))
                {
                    add_flag(o_ptr->flags, OF_REFLECT);
                }
                else if (one_in_(30))
                {
                    add_flag(o_ptr->flags, OF_MAGIC_MASTERY);
                    has_pval = TRUE;
                }
                else if (one_in_(77))
                {
                    add_flag(o_ptr->flags, OF_SPELL_POWER);
                    add_flag(o_ptr->flags, OF_DEC_STR);
                    add_flag(o_ptr->flags, OF_DEC_DEX);
                    add_flag(o_ptr->flags, OF_DEC_CON);
                    has_pval = TRUE;
                }
                else if (one_in_(3))
                {
                    add_flag(o_ptr->flags, OF_SHOW_MODS);
                    o_ptr->to_h += randint1(5) + m_bonus(5, lev);
                    o_ptr->to_d += randint1(5) + m_bonus(5, lev);
                }
                else
                {
                    one_high_resistance(o_ptr);
                }
                break;
            case 4:
                if (one_in_(3))
                {
                    add_flag(o_ptr->flags, OF_SHOW_MODS);
                    o_ptr->to_h += randint1(5) + m_bonus(5, lev);
                    o_ptr->to_d += randint1(5) + m_bonus(5, lev);
                    break;
                }
                else if (one_in_(2))
                {
                    random_plus(o_ptr);
                    has_pval = TRUE;
                    break;
                }
                else if (one_in_(5))
                {
                    one_high_resistance(o_ptr);
                    break;
                }
            case 5: case 6:
                random_resistance(o_ptr);
                break;
            case 7:
                random_misc(o_ptr);
                break;
            }
            break;

        default: /* Assume Armor */
            switch (randint1(7))
            {
                case 1: case 2:
                    random_plus(o_ptr);
                    has_pval = TRUE;
                    break;
                case 3:
                    if (!has_resistance
                      && (object_is_body_armour(o_ptr) || object_is_shield(o_ptr))
                      && one_in_(4) )
                    {
                        add_flag(o_ptr->flags, OF_RES_ACID);
                        add_flag(o_ptr->flags, OF_RES_COLD);
                        add_flag(o_ptr->flags, OF_RES_FIRE);
                        add_flag(o_ptr->flags, OF_RES_ELEC);
                        if (one_in_(3))
                            add_flag(o_ptr->flags, OF_RES_POIS);
                        has_resistance = TRUE;
                        powers -= 3;
                    }
                    else if (o_ptr->tval == TV_BOOTS && one_in_(3))
                    {
                        add_flag(o_ptr->flags, OF_SPEED);
                        has_pval = TRUE;
                    }
                    else if (o_ptr->tval == TV_BOOTS && one_in_(2))
                    {
                        add_flag(o_ptr->flags, OF_LEVITATION);
                    }
                    else if ( (o_ptr->tval == TV_CROWN && one_in_(2 + esp_count*10))
                           || (o_ptr->tval == TV_HELM && one_in_(7 + esp_count*10)) )
                    {
                        if (add_esp_strong(o_ptr))
                            add_esp_weak(o_ptr, TRUE);
                        else
                            add_esp_weak(o_ptr, FALSE);
                        ++esp_count;
                    }
                    else if (!boosted_ac)
                    {
                        o_ptr->to_a += randint1(8);
                        if (object_is_body_armour(o_ptr) && one_in_(7))
                            o_ptr->ac += 5;

                        boosted_ac = TRUE;
                    }
                    else if (o_ptr->tval == TV_GLOVES && one_in_(2))
                    {
                        add_flag(o_ptr->flags, OF_SHOW_MODS);
                        o_ptr->to_h = 4 + (randint1(11));
                        o_ptr->to_d = 4 + (randint1(11));
                    }
                    else if (o_ptr->tval == TV_GLOVES && one_in_(2))
                    {
                        add_flag(o_ptr->flags, OF_DEX);
                        has_pval = TRUE;
                    }
                    else if (o_ptr->tval == TV_GLOVES && one_in_(77))
                    {
                        add_flag(o_ptr->flags, OF_BLOWS);
                        has_pval = TRUE;
                    }
                    else
                        one_high_resistance(o_ptr);
                    break;
                case 4:
                    if (!(object_is_body_armour(o_ptr) || object_is_shield(o_ptr))
                      || one_in_(3) )
                    {
                        random_misc(o_ptr);
                        break;
                    }
                case 5: case 6: case 7:
                    random_resistance(o_ptr);
                    break;
            }
        }
    };

    /* Sometimes create rings as mixed blessings */
    if ((o_ptr->tval == TV_RING) && (one_in_(15)))
    {
        bool jatka = TRUE;
        if (one_in_(2))
        {
            if (o_ptr->to_h > 0) o_ptr->to_h = 0 - o_ptr->to_h;
            else if (one_in_(4))
            {
                o_ptr->to_h = 0 - randint1(10);
                o_ptr->to_d = (one_in_(2)) ? 0 - randint1(10) : randint1(7);
                jatka = FALSE;
            }
        }
        if ((jatka) && (one_in_(2)))
        {
            if (o_ptr->to_d > 0) o_ptr->to_d = 0 - o_ptr->to_d;
            else if (one_in_(6))
            {
                o_ptr->to_d = 0 - randint1(10);
                o_ptr->to_h = (one_in_(2)) ? 0 - randint1(10) : randint1(7);
            }
        }
    }
    if ((o_ptr->tval == TV_RING) && (one_in_(30)))
    {
        if (o_ptr->to_a > 0) o_ptr->to_a = 0 - o_ptr->to_a;
        else if (one_in_(2)) o_ptr->to_a = 0 - randint1(12);
    }

    if (has_pval)
    {
        if (have_flag(o_ptr->flags, OF_BLOWS))
        {
            if (o_ptr->tval == TV_RING || o_ptr->tval == TV_GLOVES)
            {
                o_ptr->pval = randint1(2);
                if (one_in_(6)) o_ptr->pval++;
            }
            else
            {
                o_ptr->pval = randint1(3);
                if (one_in_(15)) o_ptr->pval++;
                if (is_falcon_sword)
                    o_ptr->pval += randint1(2);
            }
        }
        else
        {
            /* Hengband:  1: 0.0%  2:20.0%  3:32.0%  4+:48.0%
               Chengband: 1:16.7%  2:28.6%  3:28.6%  4+:26.0%
               4+ usually becomes a 4. The only tweak I made was changing
               "|| one_in_(pval)" to "|| one_in_(pval+5)."
            */
            if (mode & CREATE_ART_SCROLL)
                o_ptr->pval = 0; /* Surprise! Always re-roll the pval ... */
            do
            {
                o_ptr->pval++;
            }
            while (o_ptr->pval < randint1(5) || one_in_(o_ptr->pval+5));
        }
    }

    if (o_ptr->pval > 4 && !one_in_(WEIRD_LUCK))
        o_ptr->pval = 4;

    if (o_ptr->tval == TV_DIGGING && !have_flag(o_ptr->flags, OF_BLOWS))
    {
        o_ptr->pval += randint1(2);
        if (o_ptr->pval > 6)
            o_ptr->pval = 6;
    }

    if (have_flag(o_ptr->flags, OF_LIFE) && o_ptr->pval > 2)
    {
        o_ptr->pval = randint1(2);
        if (one_in_(WEIRD_LUCK)) o_ptr->pval++;
    }

    if (have_flag(o_ptr->flags, OF_WEAPONMASTERY) && o_ptr->pval > 2)
    {
        if (one_in_(6))
            o_ptr->pval = 3;
        else
            o_ptr->pval = 2;
    }

    if (have_flag(o_ptr->flags, OF_MAGIC_MASTERY) && o_ptr->pval > 2)
    {
        if (one_in_(30))
            o_ptr->pval = 3;
        else
            o_ptr->pval = 2;
    }

    /* give it some plusses... */
    if (object_is_armour(o_ptr))
    {
        int a = randint1(5) + m_bonus(5, lev) + m_bonus(10, lev);
        int max = 20;
        if (object_is_body_armour(o_ptr)) max += 5;
        o_ptr->to_a += a;
        if (o_ptr->to_a > max - 5) o_ptr->to_a = trim(o_ptr->to_a, max - 5, max, lev);
    }
    else if (object_is_weapon_ammo(o_ptr))
    {
        int h = randint1(5) + m_bonus(5, lev) + m_bonus(10, lev);
        int d = randint1(5) + m_bonus(5, lev) + m_bonus(10, lev);
        o_ptr->to_h += h;
        o_ptr->to_d += d;
        if (o_ptr->to_h > 22) o_ptr->to_h = trim(o_ptr->to_h, 20, 25, lev);
        if (o_ptr->to_d > 20) o_ptr->to_d = trim(o_ptr->to_h, 20, 25, lev);

        if ((have_flag(o_ptr->flags, OF_WIS)) && (o_ptr->pval > 0)) add_flag(o_ptr->flags, OF_BLESSED);
    }

    /* Fix up Damage Dice for (Wild) and (Order) weapons */
    if (have_flag(o_ptr->flags, OF_BRAND_ORDER))
    {
        remove_flag(o_ptr->flags, OF_BRAND_WILD);
        o_ptr->dd = o_ptr->dd * o_ptr->ds;
        o_ptr->ds = 1;
    }
    else if (have_flag(o_ptr->flags, OF_BRAND_WILD))
    {
        o_ptr->ds = o_ptr->dd * o_ptr->ds;
        o_ptr->dd = 1;
    }

    /* Just to be sure */
    add_flag(o_ptr->flags, OF_IGNORE_ACID);
    add_flag(o_ptr->flags, OF_IGNORE_ELEC);
    add_flag(o_ptr->flags, OF_IGNORE_FIRE);
    add_flag(o_ptr->flags, OF_IGNORE_COLD);

    if (have_flag(o_ptr->flags, OF_BRAND_FIRE))
        add_flag(o_ptr->flags, OF_LITE);

    if ( !obj_has_effect(o_ptr)
      && !object_is_ammo(o_ptr) )
    {
        int odds = object_is_armour(o_ptr) ? ACTIVATION_CHANCE * 2 : ACTIVATION_CHANCE;
        if (one_in_(odds))
        {
            effect_add_random(o_ptr, artifact_bias);
        }
    }

    if (object_is_armour(o_ptr) || o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET)
    {
        int lower1 = 20, lower2 = 10;

        if (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET)
        {
            lower1 = 25;
            lower2 = 20;
        }

        while ((o_ptr->to_d+o_ptr->to_h) > lower1)
        {
            if (one_in_(o_ptr->to_d) && one_in_(o_ptr->to_h)) break;
            o_ptr->to_d -= (s16b)randint0(3);
            o_ptr->to_h -= (s16b)randint0(3);
        }
        while ((o_ptr->to_d+o_ptr->to_h) > lower2 && !immunity_hack)
        {
            if (one_in_(o_ptr->to_d) || one_in_(o_ptr->to_h)) break;
            o_ptr->to_d -= (s16b)randint0(3);
            o_ptr->to_h -= (s16b)randint0(3);
        }
    }

    if ((artifact_bias == BIAS_MAGE || artifact_bias == BIAS_INT)
      && (o_ptr->tval == TV_GLOVES))
    {
        add_flag(o_ptr->flags, OF_FREE_ACT);
    }

    if (obj_is_harp(o_ptr))
    {
        o_ptr->to_h = 0;
        o_ptr->to_d = 0;
        remove_flag(o_ptr->flags, OF_SHOW_MODS);
    }

    /* Hack: Apply extra might at the end (so only once) */
    if (o_ptr->tval == TV_BOW && have_flag(o_ptr->flags, OF_XTRA_MIGHT) && o_ptr->mult)
    {
        o_ptr->mult += (25 + m_bonus(75, object_level)) * bow_energy(o_ptr->sval) / 10000;
        remove_flag(o_ptr->flags, OF_XTRA_MIGHT);
    }
    if (o_ptr->tval == TV_BOW)
    {
        /* rescale damage ... heavy crossbows shoot 0.75 while slings shoot 1.40x
         * damage on the bow should reflect this! */
        o_ptr->to_d = o_ptr->to_d * bow_energy(o_ptr->sval) / 7150;
    }

    if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_DOKUBARI))
    {
        o_ptr->to_h = 0;
        o_ptr->to_d = 0;
        remove_flag(o_ptr->flags, OF_BLOWS);
        remove_flag(o_ptr->flags, OF_BRAND_MANA);
        remove_flag(o_ptr->flags, OF_SLAY_ANIMAL);
        remove_flag(o_ptr->flags, OF_SLAY_EVIL);
        remove_flag(o_ptr->flags, OF_SLAY_UNDEAD);
        remove_flag(o_ptr->flags, OF_SLAY_DEMON);
        remove_flag(o_ptr->flags, OF_SLAY_ORC);
        remove_flag(o_ptr->flags, OF_SLAY_TROLL);
        remove_flag(o_ptr->flags, OF_SLAY_GIANT);
        remove_flag(o_ptr->flags, OF_SLAY_DRAGON);
        remove_flag(o_ptr->flags, OF_KILL_DRAGON);
        remove_flag(o_ptr->flags, OF_SLAY_HUMAN);
        remove_flag(o_ptr->flags, OF_VORPAL);
        remove_flag(o_ptr->flags, OF_BRAND_POIS);
        remove_flag(o_ptr->flags, OF_BRAND_ACID);
        remove_flag(o_ptr->flags, OF_BRAND_ELEC);
        remove_flag(o_ptr->flags, OF_BRAND_FIRE);
        remove_flag(o_ptr->flags, OF_BRAND_COLD);
    }

    /* Hack: Lights won't value at all unless they are artifacts and the
       only way an object is marked as a rand-art is via the art_name field. */
    if (o_ptr->tval == TV_LITE)
        o_ptr->art_name = quark_add("Temp");

    if (object_is_melee_weapon(o_ptr))
        ego_weapon_adjust_weight(o_ptr);

    _sanitise_flags(o_ptr);

    total_flags = new_object_cost(o_ptr, COST_REAL);
    if (cheat_peek) msg_format("Score: %d", total_flags);

    if (o_ptr->tval == TV_RING)
    {
        if (total_flags <= 0) power_level = 0;
        else if (total_flags < 15000) power_level = 1;
        else if (total_flags < 60000) power_level = 2;
        else power_level = 3;
    }
    else if (o_ptr->tval == TV_AMULET)
    {
        if (total_flags <= 0) power_level = 0;
        else if (total_flags < 8000) power_level = 1;
        else if (total_flags < 25000) power_level = 2;
        else power_level = 3;
    }
    else if (!object_is_weapon_ammo(o_ptr))
    {
        /* For armors */
        if (total_flags <= 0) power_level = 0;
        else if (total_flags < 30000) power_level = 1;
        else if (total_flags < 70000) power_level = 2;
        else
        {
            power_level = 3;
            if (one_in_(17)) add_flag(o_ptr->flags, OF_AGGRAVATE);
            if (one_in_(5)) add_flag(o_ptr->flags, OF_HOLD_LIFE);
        }
    }
    else
    {
        /* For weapons */
        if (total_flags <= 0) power_level = 0;
        else if (total_flags <  50000) power_level = 1;
        else if (total_flags < 100000) power_level = 2;
        else
        {
            power_level = 3;
            if (one_in_(17)) add_flag(o_ptr->flags, OF_AGGRAVATE);
            if (one_in_(5)) add_flag(o_ptr->flags, OF_FREE_ACT);
            if (one_in_(5)) add_flag(o_ptr->flags, OF_SEE_INVIS);
        }
    }

    if (mode & CREATE_ART_SCROLL)
    {
        char dummy_name[80] = "";
        cptr ask_msg = "What do you want to call the artifact? ";

        no_karrot_hack = TRUE;
        obj_identify_fully(o_ptr);
        obj_display(o_ptr);
        no_karrot_hack = FALSE;

        if (!get_string(ask_msg, dummy_name, sizeof dummy_name)
            || !dummy_name[0])
        {
            get_random_name(new_name, o_ptr, power_level);
        }
        else
            sprintf(new_name, "'%s'", dummy_name);

        virtue_add(VIRTUE_INDIVIDUALISM, 2);
        virtue_add(VIRTUE_ENCHANTMENT, 5);
    }
    else
    {
        get_random_name(new_name, o_ptr, power_level);
        if (a_cursed) /* Curse after naming for flavor. ?Art will never curse */
        {
            curse_object(o_ptr);
            if (has_pval && !o_ptr->pval)
                o_ptr->pval = randint1(5);
            total_flags = new_object_cost(o_ptr, COST_REAL);
        }
    }

    if (cheat_xtra)
    {
        if (artifact_bias) msg_format("Biased artifact: %d.", artifact_bias);
        else msg_print("No bias in artifact.");
    }

    /* Save the inscription */
    o_ptr->art_name = quark_add(new_name);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP);

    return total_flags;
}

void get_bloody_moon_flags(object_type *o_ptr)
{
    int dummy, i;

    for (i = 0; i < OF_ARRAY_SIZE; i++)
    {
        o_ptr->flags[i] = 0;
        o_ptr->known_flags[i] = 0;
    }

    dummy = randint1(2) + randint1(2);
    for (i = 0; i < dummy; i++)
    {
        switch (randint1(26))
        {
        case  1: add_flag(o_ptr->flags, OF_SLAY_EVIL); break;
        case  2: add_flag(o_ptr->flags, OF_SLAY_GOOD); break;
        case  3: add_flag(o_ptr->flags, OF_SLAY_LIVING); break;
        case  4: add_flag(o_ptr->flags, OF_SLAY_DRAGON); break;
        case  5: add_flag(o_ptr->flags, OF_SLAY_DEMON); break;
        case  6: add_flag(o_ptr->flags, OF_SLAY_UNDEAD); break;
        case  7: add_flag(o_ptr->flags, OF_SLAY_ANIMAL); break;
        case  8: add_flag(o_ptr->flags, OF_SLAY_HUMAN); break;
        case  9: add_flag(o_ptr->flags, OF_SLAY_TROLL); break;
        case 10: add_flag(o_ptr->flags, OF_SLAY_GIANT); break;

        case 11: add_flag(o_ptr->flags, OF_KILL_DRAGON); break;
        case 12: add_flag(o_ptr->flags, OF_KILL_DEMON); break;
        case 13: add_flag(o_ptr->flags, OF_KILL_UNDEAD); break;
        case 14: add_flag(o_ptr->flags, OF_KILL_ANIMAL); break;
        case 15: add_flag(o_ptr->flags, OF_KILL_HUMAN); break;
        case 16: add_flag(o_ptr->flags, OF_KILL_TROLL); break;
        case 17: add_flag(o_ptr->flags, OF_KILL_GIANT); break;

        case 18: add_flag(o_ptr->flags, OF_BRAND_ACID); break;
        case 19: add_flag(o_ptr->flags, OF_BRAND_ELEC); break;
        case 20: add_flag(o_ptr->flags, OF_BRAND_FIRE); break;
        case 21: add_flag(o_ptr->flags, OF_BRAND_COLD); break;
        case 22: add_flag(o_ptr->flags, OF_BRAND_POIS); break;
        case 23: add_flag(o_ptr->flags, OF_BRAND_CHAOS); break;
        case 24: add_flag(o_ptr->flags, OF_BRAND_VAMP); break;
        case 25: add_flag(o_ptr->flags, OF_BRAND_MANA); break;
        case 26: add_flag(o_ptr->flags, OF_VORPAL); break;
        }
    }

    dummy = randint1(2);
    for (i = 0; i < dummy; i++)
        one_resistance(o_ptr);

    for (i = 0; i < 2; i++)
    {
        int tmp = randint0(11);
        if (tmp < 6) add_flag(o_ptr->flags, OF_STR + tmp);
        else one_ability(o_ptr);
    }
}


void random_artifact_resistance(object_type * o_ptr, artifact_type *a_ptr)
{
    bool give_resistance = FALSE, give_power = FALSE;

    if (o_ptr->name1 == ART_GOTHMOG)
    {
        if (prace_is_(RACE_MON_DEMON))
        {
            o_ptr->dd = 6;
            o_ptr->ds = 6;
            o_ptr->to_h = 22;
            o_ptr->to_d = 25;
        }
        else
        {
            o_ptr->curse_flags |= (OFC_CURSED | OFC_HEAVY_CURSE);
        }
    }

    if (o_ptr->name1 == ART_TWILIGHT)
    {
        if (giant_is_(GIANT_FIRE)) /* Boss reward for Fire Giants */
        {
            o_ptr->to_h = 10;
            o_ptr->to_d = 10;
            o_ptr->to_a = 0;
            add_flag(o_ptr->flags, OF_SLAY_EVIL);
            give_resistance = TRUE;
        }
        else if (warlock_is_(WARLOCK_GIANTS)) /* Pseudo-boss reward for Giant Warlocks */
        {
            o_ptr->to_h = 5;
            o_ptr->to_d = 5;
            o_ptr->to_a = 0;
        }
        else
        {
            add_flag(o_ptr->flags, OF_AGGRAVATE);
            add_flag(o_ptr->flags, OF_TY_CURSE);
            o_ptr->curse_flags |=
                (OFC_CURSED | OFC_HEAVY_CURSE);
            o_ptr->curse_flags |= get_curse(2, o_ptr);
        }
    }

    if (o_ptr->name1 == ART_STORMBRINGER)
    {
        if (prace_is_(RACE_MON_SWORD))
        {
            add_flag(o_ptr->flags, OF_BLOWS); /* Just like the good 'ol days :) */
        }
        else
        {
            add_flag(o_ptr->flags, OF_AGGRAVATE);
            add_flag(o_ptr->flags, OF_DRAIN_EXP);
            o_ptr->curse_flags |=
                (OFC_CURSED | OFC_HEAVY_CURSE);
        }
    }

    if (o_ptr->name1 == ART_DESTROYER)
    {
        if (prace_is_(RACE_MON_GOLEM))
        {
        }
        else
        {
            add_flag(o_ptr->flags, OF_DEC_BLOWS);
            add_flag(o_ptr->flags, OF_AGGRAVATE);
            add_flag(o_ptr->flags, OF_DRAIN_EXP);
            add_flag(o_ptr->flags, OF_TY_CURSE);
            o_ptr->curse_flags |=
                (OFC_CURSED | OFC_HEAVY_CURSE | OFC_PERMA_CURSE);
        }
    }

    if (o_ptr->name1 == ART_TERROR) /* Terror Mask is for warriors... */
    {
        if (p_ptr->pclass == CLASS_WARRIOR ||
            p_ptr->pclass == CLASS_CAVALRY ||
            p_ptr->pclass == CLASS_BERSERKER ||
            p_ptr->pclass == CLASS_MAULER ||
            p_ptr->prace == RACE_MON_HYDRA ||
            p_ptr->prace == RACE_MON_TROLL ||
            p_ptr->prace == RACE_MON_JELLY )
        {
            give_power = TRUE;
            give_resistance = TRUE;
        }
        else
        {
            add_flag(o_ptr->flags, OF_AGGRAVATE);
            add_flag(o_ptr->flags, OF_TY_CURSE);
            o_ptr->curse_flags |=
                (OFC_CURSED | OFC_HEAVY_CURSE);
            o_ptr->curse_flags |= get_curse(2, o_ptr);
            return;
        }
    }
    if (o_ptr->name1 == ART_STONEMASK)
    {
        if (p_ptr->prace == RACE_MON_VAMPIRE)
        {
            add_flag(o_ptr->flags, OF_CHR);
            o_ptr->to_a = 20;
        }
        else
        {
            add_flag(o_ptr->flags, OF_VULN_LITE);
            if ((comp_mode) && (p_ptr->prace != RACE_VAMPIRE) && (!player_obviously_poly_immune(FALSE)))
            {
                add_flag(o_ptr->flags, OF_TY_CURSE);
            }
        }
    }

    if (o_ptr->name1 == ART_MURAMASA)
    {
        if (p_ptr->pclass != CLASS_SAMURAI && p_ptr->pclass != CLASS_BLOOD_KNIGHT)
        {
            add_flag(o_ptr->flags, OF_NO_MAGIC);
            o_ptr->curse_flags |= (OFC_HEAVY_CURSE);
        }
    }

    if (o_ptr->name1 == ART_DR_JONES)
    {
        if (p_ptr->pclass == CLASS_ARCHAEOLOGIST)
        {
            give_power = TRUE;
            give_resistance = TRUE;
            o_ptr->pval += 2;
            add_flag(o_ptr->flags, OF_SEARCH);
            add_flag(o_ptr->flags, OF_FREE_ACT);
        }
    }

    if (o_ptr->name1 == ART_XIAOLONG)
    {
        if (p_ptr->pclass == CLASS_MONK)
            add_flag(o_ptr->flags, OF_BLOWS);
    }

    if (o_ptr->name1 == ART_DRAGONLANCE)
    {
        if (warlock_is_(WARLOCK_DRAGONS))
        {
            add_flag(o_ptr->flags, OF_SLAY_EVIL);
            add_flag(o_ptr->flags, OF_SLAY_DEMON);
            add_flag(o_ptr->flags, OF_SLAY_UNDEAD);
        }
    }

    if (o_ptr->name1 == ART_BLOOD)
    {
        get_bloody_moon_flags(o_ptr);
    }

    if (a_ptr->gen_flags & (OFG_XTRA_POWER)) give_power = TRUE;
    if (a_ptr->gen_flags & (OFG_XTRA_H_RES)) give_resistance = TRUE;
    if (a_ptr->gen_flags & (OFG_XTRA_RES_OR_POWER))
    {
        /* Give a resistance OR a power */
        if (one_in_(2)) give_resistance = TRUE;
        else give_power = TRUE;
    }

    if (give_power)
    {
        one_ability(o_ptr);
    }

    if (give_resistance)
    {
        one_high_resistance(o_ptr);
    }
}

void get_reforge_powers(bool do_minmax, object_type *src, object_type *dest, int *swgt, int *dwgt, int *min_p, int *max_p, int *avg_bp, int fame)
{
    int base_power = obj_value_real(src);
    int min_bp, max_bp;
    *swgt = get_slot_weight(src);
    *dwgt = get_dest_weight(dest);

    if ((*swgt) > (*dwgt))
        base_power = base_power * (*dwgt) / (*swgt);

    /* Power sanity check */
    base_power = MIN(base_power, 1125L * (*dwgt));

    *avg_bp = base_power * 3/4 + ((base_power*MIN(255, fame) + 1) / 3000);

    /* Pay a Power Tax! */
    if (do_minmax)
    {
        min_bp = base_power*3/4 + 1;
        max_bp = base_power*3/4 + (base_power*MIN(255, fame) / 1500);
        if (min_bp < 1000) *min_p = 1;
        else *min_p = 3 * min_bp / 4;
        if (max_bp < 1000) *max_p = max_bp + 5000;
        else *max_p = MAX(6000, 5 * max_bp / 4);
    }
    else
    {
        base_power = base_power*3/4 + randint1(base_power*MIN(255, fame)/1500);
        /* Setup thresholds. For weak objects, it's better to use a generous range ... */
        if (base_power < 1000)
        {
            *min_p = 1;
            *max_p = base_power + 5000;
        }
        else
        {
            *min_p = 3 * base_power / 4;
            *max_p = MAX(6000, 5 * base_power / 4);
        }
    }

    if (mut_present(MUT_INSPIRED_SMITHING))
    {
        *min_p += ((*min_p) / 8);
        *max_p += ((*max_p) / 12);
    }
}

bool reforge_artifact(object_type *src, object_type *dest, int fame)
{
    bool        result = FALSE;
    object_type forge = {0};
    object_type best = {0}, worst = {0};
    int         best_power = -10000000, power = 0, worst_power = 10000000;
    int         min_power, max_power;
    int         old_level, i;
    int         src_weight, dest_weight, avg_bp;

    get_reforge_powers(FALSE, src, dest, &src_weight, &dest_weight, &min_power, &max_power, &avg_bp, fame);

    /* Better Fame means better results! */
    old_level = object_level;
    object_level = MIN(fame/2, 75);

    for (i = 0; i < 1000 && !result; i++)
    {
        object_copy(&forge, dest);
        create_artifact(&forge, CREATE_ART_GOOD);
        power = obj_value_real(&forge);

        if (power > best_power)
        {
            object_copy(&best, &forge);
            best_power = power;
        }
        if (power < worst_power && min_power < power)
        {
            object_copy(&worst, &forge);
            worst_power = power;
        }

        if (power >= min_power && power <= max_power)
        {
            object_copy(dest, &forge);
            result = TRUE;
        }
    }

    object_level = old_level;

    if (!result)
    {
        /* Failed! Return best or worst */
        if (worst_power > min_power && worst.k_idx)
            object_copy(dest, &worst);
        else
            object_copy(dest, &best);

        result = TRUE;
    }

    /* Flavor: Keep name of source artifact if possible */
    if (src->name1)
    {
        dest->name3 = src->name1;
        if (have_flag(a_info[src->name1].flags, OF_FULL_NAME))
        {
            char buf[255], minibuf[2] = "\'";
            cptr oldname = a_name + a_info[src->name1].name;
            if (strpos(" '", oldname)) /* The Pendant 'Efki', etc. */
            {
                sprintf(buf, oldname + strpos(" '", oldname));
            }
            else
            {
                sprintf(buf, minibuf);
                strcat(buf, ((oldname[0] == '&') && (strlen(oldname) > 2)) ? oldname + 2 : oldname);
                strcat(buf, minibuf);
            }
            (void)clip_and_locate("~", buf);
            dest->art_name = quark_add(buf);
        }
        else dest->art_name = quark_add(a_name + a_info[src->name1].name);
    }
    else
        dest->art_name = src->art_name;

    object_origins(dest, ORIGIN_REFORGE);

    return result;
}

int original_score = 0;
int replacement_score = 0;

bool create_replacement_art(int a_idx, object_type *o_ptr, byte origin)
{
    object_type    forge1 = {0};
    object_type    forge2 = {0};
    object_type    best = {0}, worst = {0};
    int            base_power, best_power, power = 0, worst_power = 10000000;
    int            min_power, max_power;
    int            old_level;
    artifact_type *a_ptr = &a_info[a_idx];
    int            i;

    if (!a_ptr->name) return FALSE;
    if (no_artifacts) return FALSE;

    /* Score the Original */
    if (!create_named_art_aux(a_idx, &forge1)) return FALSE;
    if (object_is_weapon_ammo(&forge1))
    {
        forge1.to_h = MAX(10, forge1.to_h);
        forge1.to_d = MAX(10, forge1.to_d);
    }
    if (object_is_armour(&forge1))
    {
        forge1.to_a = MAX(10, forge1.to_a);
    }
    base_power = obj_value_real(&forge1);
    if (!obj_is_ammo(&forge1) && base_power < 7500)
        base_power = 7500;
    min_power = MAX(base_power * 4 / 5, base_power - 10000);
    max_power = MIN(base_power * 6 / 5, base_power + 10000);

    original_score += base_power;
    best_power = -10000000;
    power = 0;
    old_level = object_level;
    object_level = a_ptr->level;

    for (i = 0; i < 10000; i++)
    {
        object_prep(&forge2, forge1.k_idx);
        create_artifact(&forge2, CREATE_ART_GOOD);
        if (a_ptr->gen_flags & OFG_FIXED_ACT)
        {
            forge2.activation = a_ptr->activation;
            add_flag(forge2.flags, OF_ACTIVATE); /* for object lore */
        }
        power = obj_value_real(&forge2);

        if (power > best_power)
        {
            object_copy(&best, &forge2);
            best_power = power;
        }
        if (power < worst_power)
        {
            object_copy(&worst, &forge2);
            worst_power = power;
        }

        if (min_power <= power && power <= max_power)
        {
            replacement_score += power;
            object_level = old_level;
            object_copy(o_ptr, &forge2);
            object_origins(o_ptr, origin);
            o_ptr->name3 = a_idx;
            o_ptr->weight = forge1.weight;
            o_ptr->level = a_ptr->level;
            return TRUE;
        }
    }

    /* Failed! Return best or worst */
    object_level = old_level;
    if (worst_power > base_power)
    {
        replacement_score += worst_power;
        object_copy(o_ptr, &worst);
    }
    else
    {
        replacement_score += best_power;
        object_copy(o_ptr, &best);
    }

    o_ptr->name3 = a_idx;
    o_ptr->weight = forge1.weight;
    o_ptr->level = a_ptr->level;

    return TRUE;
}

/*
 * Create the artifact of the specified number
 */
bool create_named_art_aux_aux(int a_idx, object_type *o_ptr)
{
    int k_idx;
    artifact_type *a_ptr = &a_info[a_idx];

    if (!a_ptr->name) return FALSE;
    k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
    if (!k_idx) return FALSE;

    object_prep(o_ptr, k_idx);

    o_ptr->name1 = a_idx;
    o_ptr->pval = a_ptr->pval;
    if ((obj_is_harp(o_ptr)) && (p_ptr->pclass != CLASS_BARD))
        o_ptr->pval -= (o_ptr->pval / 2);
    o_ptr->ac = a_ptr->ac;
    o_ptr->dd = a_ptr->dd;
    o_ptr->ds = a_ptr->ds;
    o_ptr->mult = a_ptr->mult;
    o_ptr->to_a = a_ptr->to_a;
    o_ptr->to_h = a_ptr->to_h;
    o_ptr->to_d = a_ptr->to_d;
    o_ptr->weight = a_ptr->weight;

    return TRUE;
}
bool create_named_art_aux(int a_idx, object_type *o_ptr)
{
    artifact_type *a_ptr = &a_info[a_idx];

    if (!create_named_art_aux_aux(a_idx, o_ptr))
        return FALSE;

    if (a_ptr->gen_flags & OFG_CURSED) o_ptr->curse_flags |= (OFC_CURSED);
    if (a_ptr->gen_flags & OFG_HEAVY_CURSE) o_ptr->curse_flags |= (OFC_HEAVY_CURSE);
    if (a_ptr->gen_flags & OFG_PERMA_CURSE) o_ptr->curse_flags |= (OFC_PERMA_CURSE);
    if (a_ptr->gen_flags & (OFG_RANDOM_CURSE0)) o_ptr->curse_flags |= get_curse(0, o_ptr);
    if (a_ptr->gen_flags & (OFG_RANDOM_CURSE1)) o_ptr->curse_flags |= get_curse(1, o_ptr);
    if (a_ptr->gen_flags & (OFG_RANDOM_CURSE2)) o_ptr->curse_flags |= get_curse(2, o_ptr);

    random_artifact_resistance(o_ptr, a_ptr);

    return TRUE;
}

bool create_named_art(int a_idx, int y, int x, byte origin, int xtra)
{
    assert(0 <= a_idx && a_idx < max_a_idx);
    if (no_artifacts) return FALSE;

    if ( random_artifacts
      && !(a_info[a_idx].gen_flags & OFG_FIXED_ART)
      && randint0(100) < random_artifact_pct )
    {
        object_type forge;
        if (create_replacement_art(a_idx, &forge, origin))
        {
            if (origin == ORIGIN_DROP) forge.origin_xtra = xtra;
            return drop_near(&forge, -1, y, x) ? TRUE : FALSE;
        }
    }
    else
    {
        object_type forge;
        if (create_named_art_aux(a_idx, &forge))
        {
            object_origins(&forge, origin);
            if (origin == ORIGIN_DROP) forge.origin_xtra = xtra;
            return drop_near(&forge, -1, y, x) ? TRUE : FALSE;
        }
    }
    return FALSE;
}
