#include "angband.h"

#include <assert.h>

void res_add(int which)
{
    assert(0 <= which && which < RES_MAX);
    p_ptr->resist[which]++;
}

void res_add_amt(int which, int amt)
{
    p_ptr->resist[which] += amt;
}

void res_add_all(void)
{
    int i;
    for (i = 0; i < RES_MAX; i++)
        res_add(i);
}

void res_add_immune(int which)
{
    p_ptr->resist[which] += 100;
}

void res_add_vuln(int which)
{
    p_ptr->resist[which]--;
}

bool res_is_high(int which)
{
    if (res_is_low(which))
        return FALSE;
    return TRUE;
}

bool res_is_low(int which)
{
    switch (which)
    {
    case RES_ACID:
    case RES_ELEC:
    case RES_FIRE:
    case RES_COLD:
    case RES_POIS:
        return TRUE;
    }
    return FALSE;
}

typedef struct {
    cptr name;
    byte color;
    int flg;
    int vuln_flg;
    int im_flg;
} _res_info_t;

static _res_info_t _resist_map[RES_MAX] = {
    { "Acid",           TERM_GREEN,   OF_RES_ACID,    OF_VULN_ACID,   OF_IM_ACID },
    { "Electricity",    TERM_BLUE,    OF_RES_ELEC,    OF_VULN_ELEC,   OF_IM_ELEC },
    { "Fire",           TERM_RED,     OF_RES_FIRE,    OF_VULN_FIRE,   OF_IM_FIRE },
    { "Cold",           TERM_L_WHITE, OF_RES_COLD,    OF_VULN_COLD,   OF_IM_COLD },
    { "Poison",         TERM_L_GREEN, OF_RES_POIS,    OF_VULN_POIS,   OF_IM_POIS },
    { "Light",          TERM_YELLOW,  OF_RES_LITE,    OF_VULN_LITE,   OF_IM_LITE },
    { "Dark",           TERM_L_DARK,  OF_RES_DARK,    OF_VULN_DARK,   OF_IM_DARK },
    { "Confusion",      TERM_L_RED,   OF_RES_CONF,    OF_VULN_CONF,   OF_INVALID },
    { "Nether",         TERM_L_DARK,  OF_RES_NETHER,  OF_VULN_NETHER, OF_IM_NETHER },
    { "Nexus",          TERM_VIOLET,  OF_RES_NEXUS,   OF_VULN_NEXUS,  OF_INVALID },
    { "Sound",          TERM_ORANGE,  OF_RES_SOUND,   OF_VULN_SOUND,  OF_INVALID },
    { "Shards",         TERM_L_UMBER, OF_RES_SHARDS,  OF_VULN_SHARDS, OF_INVALID },
    { "Chaos",          TERM_VIOLET,  OF_RES_CHAOS,   OF_VULN_CHAOS,  OF_INVALID },
    { "Disenchantment", TERM_VIOLET,  OF_RES_DISEN,   OF_VULN_DISEN,  OF_INVALID },
    { "Time",           TERM_L_BLUE,  OF_RES_TIME,    OF_INVALID,     OF_INVALID },
    { "Blindness",      TERM_L_DARK,  OF_RES_BLIND,   OF_VULN_BLIND,  OF_IM_BLIND },
    { "Fear",           TERM_L_RED,   OF_RES_FEAR,    OF_VULN_FEAR,   OF_IM_FEAR },
    { "Teleportation",  TERM_ORANGE,  OF_NO_TELE,     OF_INVALID,     OF_INVALID }
};

/* For rag use only */
void res_calc_rag_bonuses(object_type *o_ptr)
{
    int i;
    for (i = RES_BEGIN; i < RES_END; i++)
    {
        _res_info_t m = _resist_map[i];
        if (m.flg != OF_INVALID && have_flag(o_ptr->flags, m.flg))
            res_add_amt(i, rag_effect_pval(o_ptr, -1, m.flg, TRUE));
        if (m.im_flg != OF_INVALID && have_flag(o_ptr->flags, m.im_flg))
            res_add_immune(i);
    }
}

void res_calc_rag_flags(object_type *o_ptr)
{
    int i;
    for (i = RES_BEGIN; i < RES_END; i++)
    {
        _res_info_t m = _resist_map[i];
        if (m.flg != OF_INVALID && rag_effect_pval(o_ptr, -1, m.flg, TRUE))
            add_flag(o_ptr->flags, m.flg);
        if (m.im_flg != OF_INVALID && rag_effect_pval(o_ptr, -1, m.im_flg, TRUE))
            add_flag(o_ptr->flags, m.im_flg);
    }
}

void res_calc_bonuses(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = RES_BEGIN; i < RES_END; i++)
    {
        _res_info_t m = _resist_map[i];
        if (m.flg != OF_INVALID && have_flag(flgs, m.flg))
            res_add(i);
        if (m.vuln_flg != OF_INVALID && have_flag(flgs, m.vuln_flg))
            res_add_vuln(i);
        if (m.im_flg != OF_INVALID && have_flag(flgs, m.im_flg))
            res_add_immune(i);
    }
}

bool res_has_bonus(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = RES_BEGIN; i < RES_END; i++)
    {
        _res_info_t m = _resist_map[i];
        int    net = 0;
        if (m.im_flg != OF_INVALID && have_flag(flgs, m.im_flg))
            return TRUE;
        if (m.flg != OF_INVALID && have_flag(flgs, m.flg))
            net++;
        if (m.vuln_flg != OF_INVALID && have_flag(flgs, m.vuln_flg))
            net--;
        if (net)
            return TRUE;
    }
    return FALSE;
}

int  res_get_object_flag(int which)
{
    return _resist_map[which].flg;
}

int  res_get_object_vuln_flag(int which)
{
    return _resist_map[which].vuln_flg;
}

int  res_get_object_immune_flag(int which)
{
    return _resist_map[which].im_flg;
}

static int _randomize(int pct, int modi)
{
    if (pct != 100)
    {
        if (pct > 0)
            pct = randnor(pct, modi);
        else if (pct < 0)
            pct = -randnor(-pct, modi);
    }
    return pct;
}
int res_calc_dam(int which, int dam)
{
    int pct1 = res_pct(which);
    int pct2 = _randomize(pct1, res_is_low(which) ? 3 : 2);
    int result = dam;

    if ((p_ptr->resist[which] > 0) && (pct2 != 100))
    {
        int _max_rand = MAX(1, 6 - p_ptr->resist[which]);
        if (res_is_low(which)) _max_rand *= 2;
        if (pct2 < pct1 - _max_rand) pct2 = pct1 - _max_rand;
        else if (pct2 > pct1 + _max_rand) pct2 = pct1 + _max_rand;
    }

    result -= pct2 * dam / 100;
    if (result < 0)
        result = 0;

    if (result < dam)
    {
        int flag = result ? res_get_object_flag(which) : res_get_object_immune_flag(which);
        equip_learn_resist(flag);
    }
    else if (result > dam)
    {
        equip_learn_vuln(res_get_object_vuln_flag(which));
    }

    return result;
}

void res_clear(void)
{
    int i;
    for (i = 0; i < RES_MAX; i++)
        p_ptr->resist[i] = 0;
}

cptr res_name(int which)
{
    return _resist_map[which].name;
}

byte res_color(int which)
{
    return _resist_map[which].color;
}


#define _MAX_PCTS 29
static int _lo_pcts[_MAX_PCTS] = {
   0, 50, 65, 72, 75,
  77, 78, 79, 80, 81,
  82, 83, 84, 85, 86,
  87, 88, 89, 90, 91,
  92, 93, 94, 95, 96,
  97, 98, 99, 100
};

static int _hi_pcts[_MAX_PCTS] = {
   0, 30, 40, 45, 47,
  48, 49, 50, 51, 52,
  53, 54, 55, 56, 57,
  58, 59, 60, 61, 62,
  63, 64, 65, 66, 67,
  68, 69, 70, 100
};
/* Note: I've decided to move back to a two-tiered resistance system.
 * While I like the simplicity of a single system, the fact is that it
 * forced the player to cover all resists, especially rather early on when
 * it is practically impossible to do so. This redesign is closer to the
 * original Hengband system. Also, you can no longer "shut-down" end game
 * high damage with multiple resists like before:
 * Attack  Hengband(1x) Old(3x) New(3x) New(1x)
 * ======= ============ ======= ======= =======
 * ROCKET           400     224     330     420
 * BR_CHAO          439     196     330     420
 * BR_NETH          356     182     302     385

 * These are the Serpent's big three attacks. As you can see, he could be
 * made quite tame with enough resists. But no longer ;)
 */


int res_pct_aux(int which, int count)
{
    int result = 0;
    int idx = count;

    if (count < 0)
        idx *= -1;

    if (idx >= _MAX_PCTS)
        idx = _MAX_PCTS-1;

    if (res_is_low(which))
        result = _lo_pcts[idx];
    else
        result = _hi_pcts[idx];

    if (p_ptr->prace == RACE_MON_ARMOR)
    {
        if (!res_is_low(which))
        {
            switch (result)
            {
                case 30: result = 16; break;
                case 40: result = 25; break;
                case 45: result = 32; break;
                case 47: result = 37; break;
                case 48: result = 41; break;
                case 49: result = 44; break;
                case 50: result = 46; break;
                case 51: result = 48; break;
                case 52: result = 50; break;
/*              case 30: result = 22; break;
                case 40: result = 33; break;
                case 45: result = 40; break;
                case 47: result = 45; break;
                case 50: result = 40; break;
                case 65: result = 55; break;
                case 72: result = 65; break;
                case 75: result = 72; break;             */
                default: break;
            }
        }
        else
        {
            switch (result)
            {
                case 50: result = 32; break;
                case 65: result = 47; break;
                case 72: result = 56; break;
                case 75: result = 63; break;
                case 77: result = 68; break;
                case 78: result = 72; break;
                case 79: result = 75; break;
                case 80: result = 78; break;
                case 81: result = 80; break;
                default: break;
            }
        }
    }

    if (count < 0)
        result *= -1;
    else if (result < 100)
    {
        if (which == RES_CONF)
        {
            if (prace_is_(RACE_TONBERRY) || demon_is_(DEMON_CYBERDEMON))
                result = (result + 1) / 2;
        }

        if (which == RES_LITE)
        {
            if (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || prace_is_(MIMIC_VAMPIRE))
                result = (result + 1) / 2;
        }

        if (which == RES_FIRE)
        {
            if (prace_is_(RACE_ENT))
                result = result * 7 / 10;
            else if (prace_is_(RACE_WEREWOLF))
                result = result * 4 / 5;
        }

        if (which == RES_ELEC)
        {
            if (prace_is_(RACE_ANDROID))
                result = result * 7 / 10;
        }
    }
    return result;
}

int res_pct(int which)
{
    int ct = p_ptr->resist[which];
    return res_pct_aux(which, ct);
}

int res_ct_known(int which)
{
    int ct = p_ptr->resist[which];
    int hidden = 0;
    int flg = res_get_object_flag(which);
    int vuln_flg = res_get_object_vuln_flag(which);
    int imm_flg = res_get_object_immune_flag(which);
    int i;

    /* Life is a bit hard at the moment since "player flags"
       may account for multiple resistances. Really, the entire
       flag based approach to resistance is just wrong, but I'm
       too lazy to fix ...
    */
    for (i = 1; i <= equip_max(); i++)
    {
        object_type *o_ptr = equip_obj(i);
        u32b         flgs[OF_ARRAY_SIZE];
        u32b         flgs_known[OF_ARRAY_SIZE];

        if (!o_ptr) continue;
        if (o_ptr->marked & OM_SLIPPING) continue;
        obj_flags(o_ptr, flgs);
        obj_flags_known(o_ptr, flgs_known);

        if (have_flag(flgs, flg) && !have_flag(flgs_known, flg))
            hidden++;
        if (vuln_flg != OF_INVALID && have_flag(flgs, vuln_flg) && !have_flag(flgs_known, vuln_flg))
            hidden--;
        if (imm_flg != OF_INVALID && have_flag(flgs, imm_flg) && !have_flag(flgs_known, imm_flg))
            hidden += 100;
    }

    ct -= hidden;
    return ct;
}

int res_pct_known(int which)
{
    return res_pct_aux(which, res_ct_known(which));
}

bool res_save(int which, int power)
{
    int pct = res_pct(which);
    int roll = randint0(power);
    if (roll < pct)
    {
        int flag = res_get_object_flag(which);
        equip_learn_resist(flag);
        return TRUE;
    }
    return FALSE;
}

bool res_save_default(int which)
{
    int power = res_is_low(which) ? 55 : 33;
    return res_save(which, power);
}
bool res_can_ignore(int which)
{
    int power = res_is_low(which) ? 55 : 33;
    if (res_pct(which) >= power)
        return TRUE;
    return FALSE;
}

bool res_save_inventory(int which)
{
    int power = res_is_low(which) ? 66 : 41;

    /* Mercy for racial vulnerabilities:
        ResCt  Old% New%
        =====  ==== ====
          1    24.2 35.2
          2     1.5 16.7
          3     0.0  7.4
          4          3.7
          5          1.9
          6          0.0
    */
    if (prace_is_(RACE_ENT) && which == RES_FIRE)
        power = 54;
    if (prace_is_(RACE_ANDROID) && which == RES_ELEC)
        power = 54;
    if (prace_is_(RACE_WEREWOLF) && which == RES_FIRE)
        power = 53;

    return res_save(which, power);
}
