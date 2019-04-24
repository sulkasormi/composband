#include "angband.h"

int _get_level(int amount)
{
    if (amount >= FEAR_PETRIFIED)
        return FEAR_PETRIFIED;
    if (amount >= FEAR_TERRIFIED)
        return FEAR_TERRIFIED;
    if (amount >= FEAR_SCARED)
        return FEAR_SCARED;
    if (amount >= FEAR_NERVOUS)
        return FEAR_NERVOUS;
    if (amount >= FEAR_UNEASY)
        return FEAR_UNEASY;

    return FEAR_BOLD;
}

int _decrement_level(int lvl)
{
    switch (lvl)
    {
    case FEAR_UNEASY: return FEAR_BOLD;
    case FEAR_NERVOUS: return FEAR_UNEASY;
    case FEAR_SCARED: return FEAR_NERVOUS;
    case FEAR_TERRIFIED: return FEAR_SCARED;
    case FEAR_PETRIFIED: return FEAR_TERRIFIED;
    }
    return FEAR_BOLD;
}

static void _decrease_p(int lvls)
{
    int lvl = fear_level_p();

    for (;lvls; lvls--)
        lvl = _decrement_level(lvl);

    fear_set_p(lvl);
}

int  fear_level_p(void)
{
    return _get_level(p_ptr->afraid);
}

void fear_clear_p(void)
{
    fear_set_p(0);
}

bool fear_add_p(int amount)
{
    return fear_set_p(p_ptr->afraid + amount);
}

static char _get_level_color(int v)
{
    int lvl = _get_level(v);
    switch (lvl)
    {
    case FEAR_UNEASY: return 'U';
    case FEAR_NERVOUS: return 'y';
    case FEAR_SCARED: return 'R';
    case FEAR_TERRIFIED: return 'r';
    case FEAR_PETRIFIED: return 'v';
    }

    return 'W';
}

static cptr _get_level_name(int v)
{
    int lvl = _get_level(v);
    switch (lvl)
    {
    case FEAR_UNEASY: return "uneasy";
    case FEAR_NERVOUS: return "nervous";
    case FEAR_SCARED: return "scared";
    case FEAR_TERRIFIED: return "terrified";
    case FEAR_PETRIFIED: return "petrified";
    }

    return "bold";
}

bool fear_set_p(int v)
{
    int old_lvl, new_lvl;
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->is_dead) return FALSE;

    old_lvl = _get_level(p_ptr->afraid);
    new_lvl = _get_level(v);

    if (new_lvl > old_lvl)
    {
        msg_format("You feel <color:%c>%s</color>.", _get_level_color(new_lvl), _get_level_name(new_lvl));
        if (old_lvl <= FEAR_SCARED && one_in_(6) && !fear_save_p(v/5))
            do_dec_stat(A_CHR);
        if (p_ptr->special_defense & KATA_MASK)
        {
            msg_print("Your posture gets loose.");
            p_ptr->special_defense &= ~KATA_MASK;
            p_ptr->update |= PU_BONUS;
            p_ptr->update |= PU_MONSTERS;
            p_ptr->redraw |= PR_STATE;
            p_ptr->redraw |= PR_STATUS;
            p_ptr->action = ACTION_NONE;
        }
        notice = TRUE;
        p_ptr->counter = FALSE;
        virtue_add(VIRTUE_VALOUR, -1);
    }
    else if (new_lvl < old_lvl)
    {
        if (new_lvl == FEAR_BOLD)
            msg_print("Your fears finally subside.");
        else
        {
            msg_format("You are no longer %s, but you still feel %s.",
                        _get_level_name(old_lvl), _get_level_name(new_lvl));
        }
        notice = TRUE;
    }

    p_ptr->afraid = v;
    p_ptr->redraw |= PR_EFFECTS;
    if (!notice) return FALSE;
    if (disturb_state) disturb(0, 0);
    handle_stuff();
    return TRUE;
}

/* Effective monster level for fear calculations */
static int _r_level(monster_race *r_ptr)
{
int ml = r_ptr->level;

    if (r_ptr->flags1 & RF1_UNIQUE)
        ml += 3;

    return ml;
}

/* Effective threat level for player recovery */
int fear_threat_level(void)
{
    int dl = MIN(dun_level + 2, 127);
    int ml = 0;
    int i;

    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        if (!m_ptr->r_idx) continue;
        if (!m_ptr->ml) continue;
        if (!(r_ptr->flags2 & RF2_AURA_FEAR)) continue;
        if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;

        ml = MAX(ml, _r_level(r_ptr)/MAX(1, m_ptr->cdis - 2));
    }

    return MAX(dl, ml);
}

/* Permissable Player Actions */
bool fear_allow_device(void)
{
    if (p_ptr->afraid && !fear_save_p(p_ptr->afraid)) return FALSE;
    return TRUE;
}

bool fear_allow_magic(void)
{
    static s32b last_passed = 0;
    if (last_passed == game_turn) return TRUE; /* don't punish cancelling the prompt */
    if (p_ptr->afraid && !fear_save_p(p_ptr->afraid))
    {
        if (p_ptr->pclass == CLASS_ALCHEMIST) return !one_in_(3);
        return FALSE;
    }
    last_passed = game_turn;
    return TRUE;
}

bool pelko(void)
{
    if (!fear_allow_magic())
    {
        msg_print("You are too scared!");
        if (energy_use < 100) energy_use = 100;
        if (flush_failure) flush();
        return TRUE;
    }
    return FALSE;
}

bool fear_allow_melee(int m_idx)
{
    if (p_ptr->afraid)
    {
        if ( p_ptr->pclass == CLASS_DUELIST
            && p_ptr->lev >= 5
            && p_ptr->duelist_target_idx == m_idx )
        {
            /* Duelist: Fearless Duel */
            if (!fear_save_p(p_ptr->afraid)) return FALSE;
        }
        else if (!fear_save_p(3*p_ptr->afraid))
            return FALSE;
    }
    return TRUE;
}

bool fear_allow_shoot(void)
{
    if (p_ptr->afraid && !fear_save_p(3*p_ptr->afraid)) return FALSE;
    return TRUE;
}

static int _plev(void)
{
    int l = p_ptr->lev;
    if (p_ptr->personality == PERS_CRAVEN)
        l = MAX(1, l - 5);
    if (l <= 40)
        return 5 + l;

    return 45 + (l - 40)*2;
}

/* Fear Saving Throws
   Odds that 1dM <= 1dN. There are 2 cases:
   [1] M<=N: p = (2N-M+1)/2N
   [2] M>N : p = (N+1)/2M
 */
bool fear_save_p(int ml)
{
    int pl;
    int rolls;
    int i;

    if (!hack_mind) return TRUE;
    if (ml <= 1) return TRUE;

    /* Immunity to Fear?*/
    if (res_pct(RES_FEAR) >= 100) return TRUE;

    pl = _plev() + adj_stat_save_fear[p_ptr->stat_ind[A_CHR]];
    if (pl < 1) pl = 1;

    rolls = 1 + p_ptr->resist[RES_FEAR];

    /* Vulnerability to Fear? At least give the player a chance! */
    if (rolls < 1)
    {
        rolls = 1;
        pl = (pl + 1)/2;
    }

    for (i = 0; i < rolls; i++)
    {
        if (randint1(ml) <= randint1(pl))
        {
            equip_learn_resist(OF_RES_FEAR);
            return TRUE;
        }
    }
    return FALSE;
}

bool life_save_p(int ml)
{
    int pl;
    int rolls;
    int i;

    pl = _plev() + adj_stat_save[p_ptr->stat_ind[A_CHR]];
    if (pl < 1) pl = 1;

    rolls = p_ptr->hold_life;

    if (p_ptr->wizard) msg_format("<color:D>life_save_p: 1d%d <= 1d%d</color>", ml, pl);
    for (i = 0; i < rolls; i++)
    {
        if (randint1(ml) <= randint1(pl))
        {
            equip_learn_resist(OF_HOLD_LIFE);
            return TRUE;
        }
    }
    return FALSE;
}

bool fear_save_m(monster_type *m_ptr)
{
    int           pl = _plev();
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int           ml = _r_level(r_ptr);
    bool          result = FALSE;

    /* Player may not exert their force of will out of sight! */
    if (projectable(py, px, m_ptr->fy, m_ptr->fx))
        pl += adj_stat_save[p_ptr->stat_ind[A_CHR]];

    if (pl <= 1) return TRUE;

    if (randint1(pl) <= randint1(ml)) result = TRUE;

    return result;
}

/* Recovery from Fear */
void fear_recover_p(void)
{
    if (p_ptr->afraid)
    {
        int threat = fear_threat_level();
        if (fear_save_p(threat + p_ptr->afraid / 10))
        {
            int lvls = 1;

            while (lvls < 3 && fear_save_p(threat + p_ptr->afraid))
                lvls++;

            _decrease_p(lvls);
        }
        else if (p_ptr->afraid >= FEAR_SCARED && !fear_save_p((threat + p_ptr->afraid)/6))
        {
            if (p_ptr->afraid >= FEAR_PETRIFIED)
            {
                p_ptr->energy_need += 100 * TURNS_PER_TICK / 10;
                msg_print("You are scared stiff!");
                disturb(1, 0);
            }
            else if (p_ptr->afraid >= FEAR_TERRIFIED)
            {
                p_ptr->energy_need += 60 * TURNS_PER_TICK / 10;
                msg_print("You shudder uncontrollably!");
                disturb(1, 0);
            }
            else
            {
                p_ptr->energy_need += 30 * TURNS_PER_TICK / 10;
                msg_print("You tremble in terror!");
                disturb(1, 0);
            }
        }
    }
}

/* Handle the Terrifying Aura of Fear! */
void fear_process_p(void)
{
    int i, r_level;
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;

        if (!m_ptr->r_idx) continue;
        if (!m_ptr->ml) continue;

        r_ptr = mon_apparent_race(m_ptr);

        if (!(r_ptr->flags2 & RF2_AURA_FEAR)) continue;
        if (is_pet(m_ptr) || is_friendly(m_ptr)) continue;
        if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;

        r_level = _r_level(r_ptr);
        if ((m_ptr->mflag2 & MFLAG2_FUZZY) && (randint1(200) > r_level)) continue;
        if (!fear_save_p(r_level/MAX(1, m_ptr->cdis-2)))
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            msg_format("You behold the terrifying visage of %s!", m_name);
            mon_lore_2(m_ptr, RF2_AURA_FEAR);
            fear_add_p(r_level/MAX(1, m_ptr->cdis-2));
        }
    }
}

void fear_update_m(monster_type *m_ptr)
{
    monster_race *r_ptr = mon_apparent_race(m_ptr);
    if ((r_ptr->flags2 & RF2_AURA_FEAR) && m_ptr->ml && !is_pet(m_ptr) && !is_friendly(m_ptr))
    {
        int r_level = _r_level(r_ptr);
        if (!fear_save_p(r_level))
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            msg_format("You behold the terrifying visage of %s!", m_name);
            mon_lore_2(m_ptr, RF2_AURA_FEAR);
            fear_add_p(r_level);
        }
    }
}

void fear_p_touch_m(monster_type *m_ptr)
{
    monster_race *r_ptr = mon_apparent_race(m_ptr);

    if (r_ptr->flags2 & RF2_AURA_FEAR)
    {
        int r_level = _r_level(r_ptr);
        if (!fear_save_p(r_level))
        {
            mon_lore_2(m_ptr, RF2_AURA_FEAR);
            fear_add_p(r_level);
        }
    }
}

bool fear_p_hurt_m(int m_idx, int dam)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = mon_apparent_race(m_ptr);
    bool          result = FALSE;

    /* Apply Aura of Fear to the Player for non-melee damage */
    if (!melee_hack && (r_ptr->flags2 & RF2_AURA_FEAR))
    {
        int r_level = _r_level(r_ptr);
        if (!fear_save_p(r_level))
        {
            mon_lore_2(m_ptr, RF2_AURA_FEAR);
            fear_add_p(r_level/MAX(1, m_ptr->cdis - 2));
        }
    }

    if (!MON_MONFEAR(m_ptr) && !(r_ptr->flags3 & (RF3_NO_FEAR)))
    {
        int percentage = (100 * m_ptr->hp) / m_ptr->maxhp;
        int n = 10;

        n = n * adj_fear_m[p_ptr->stat_ind[A_CHR]] / 100;

        if ((n >= percentage) || (dam >= m_ptr->hp && randint0(100) < 80))
        {
            if (!fear_save_m(m_ptr))
            {
                result = TRUE;
                set_monster_monfear(m_idx, randint1(10) + 20);
            }
        }
    }
    return result;
}

void fear_terrify_p(monster_type *m_ptr)
{
    monster_race *r_ptr = mon_apparent_race(m_ptr);
    int           r_level = _r_level(r_ptr);

    if (fear_save_p(r_level))
    {
        if (disturb_minor)
            msg_print("You stand your ground!");
    }
    else
        fear_add_p(r_level);
}

void fear_scare_p(monster_type *m_ptr)
{
    monster_race *r_ptr = mon_apparent_race(m_ptr);
    int           r_level = _r_level(r_ptr);

    if (fear_save_p(r_level))
        msg_print("You refuse to be frightened.");
    else
        fear_add_p(r_level);
}

/* Monster Fear */
bool fear_process_m(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    if (MON_MONFEAR(m_ptr) && !MON_CSLEEP(m_ptr))
    {
        if (fear_save_m(m_ptr))
        {
            bool recovered = FALSE;
            if (fear_save_m(m_ptr))
            {
                set_monster_monfear(m_idx, 0);
                recovered = TRUE;
            }
            else
            {
                monster_race *r_ptr = mon_apparent_race(m_ptr);
                recovered = set_monster_monfear(m_idx,
                    MON_MONFEAR(m_ptr) - randint1(r_ptr->level / 20 + 1));
            }

            if (recovered && mon_show_msg(m_ptr))
            {
                char m_name[80];
                char m_poss[80];

                monster_desc(m_poss, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);
                monster_desc(m_name, m_ptr, 0);

                msg_format("%^s recovers %s courage.", m_name, m_poss);
            }
        }
        else if (one_in_(3) && !fear_save_m(m_ptr))
        {
            if (mon_show_msg(m_ptr))
            {
                char m_name[80];
                monster_desc(m_name, m_ptr, 0);
                msg_format("%^s is scared stiff!", m_name);
            }
            return FALSE;
        }
    }
    return TRUE;
}

#define HURT_0   0
#define HURT_25 25
#define HURT_50 50
#define HURT_65 65
#define HURT_80 80
#define HURT_90 90
#define HURT_95 95

static int _get_hurt_level(int chp)
{
    int pct = (p_ptr->mhp - MAX(chp, 0)) * 100 / p_ptr->mhp;

    if (p_ptr->mhp < 100)
    {
        if (pct >= HURT_65)
            return HURT_65;
    }
    else if (p_ptr->mhp < 250)
    {
        if (pct >= HURT_80)
            return HURT_80;
        if (pct >= HURT_50)
            return HURT_50;
    }
    else if (p_ptr->mhp < 500)
    {
        if (pct >= HURT_90)
            return HURT_90;
        if (pct >= HURT_65)
            return HURT_65;
        if (pct >= HURT_50)
            return HURT_50;
    }
    else
    {
        if (pct >= HURT_95)
            return HURT_95;
        if (pct >= HURT_90)
            return HURT_90;
        if (pct >= HURT_80)
            return HURT_80;
        if (pct >= HURT_65)
            return HURT_65;
        if (pct >= HURT_50)
            return HURT_50;
        if (pct >= HURT_25)
            return HURT_25;
    }
    return HURT_0;
}

void fear_heal_p(int old_hp, int new_hp)
{
    if (p_ptr->pclass != CLASS_BLOOD_KNIGHT && p_ptr->pclass != CLASS_BLOOD_MAGE && p_ptr->afraid)
    {
        int old_hurt = _get_hurt_level(old_hp);
        int new_hurt = _get_hurt_level(new_hp);

        if (new_hurt < old_hurt && fear_save_p(fear_threat_level()))
            _decrease_p(1);
    }
}

void fear_hurt_p(int old_hp, int new_hp)
{
    if (p_ptr->pclass != CLASS_BLOOD_KNIGHT && p_ptr->pclass != CLASS_BLOOD_MAGE)
    {
        int old_hurt = _get_hurt_level(old_hp);
        int new_hurt = _get_hurt_level(new_hp);
        if (new_hurt > old_hurt)
        {
            if ( !fear_save_p(fear_threat_level())
              || (new_hurt > HURT_50 && !fear_save_p(fear_threat_level())) )
            {
                fear_add_p(new_hurt);
            }
            else
                msg_format("You stand your ground!");
        }
    }
}

