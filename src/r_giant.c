#include "angband.h"

static cptr _desc =
    "Giants are humanoids of immense stature. There are several types of giants. "
    "Fire, Frost and Storm giants are elemental giants, and gain elemental resistances, "
    "slays and even attacks appropriate for their type. Titans are powerful immortal beings of legend; "
    "their attacks often confuse their foes, and they rarely fight alone. Finally, there are the mighty "
    "Hrus; they are so strong they can eventually crush the walls in their path, but their magical "
    "powers are fairly limited.\n \n"
    "Giants rely on their superior physical stature to pummel their opponents with mighty blows. Against a distant "
    "foe, giants are capable of hurling large boulders with devastating effect. "
    "Giants use the same equipment slots as most normal player races, and have no innate attacks.";

static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_HILL_GIANT;

    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_EXECUTIONERS_SWORD));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/**********************************************************************
 * Giant Weapon Restrictions
 * N.B. We could use weapon weight for this, but weight and size aren't
 *      always the same thing.
 **********************************************************************/
static bool _weapon_is_small(int tval, int sval)
{
    if (tval == TV_SWORD)
    {
        switch (sval)
        {
        case SV_BROKEN_DAGGER:
        case SV_BROKEN_SWORD:
        case SV_DAGGER:
        case SV_MAIN_GAUCHE:
        case SV_TANTO:
        case SV_RAPIER:
        case SV_SMALL_SWORD:
        case SV_BASILLARD:
        case SV_SHORT_SWORD:
        case SV_SABRE:
        case SV_DOKUBARI:
        case SV_HAYABUSA:
        case SV_DRAGON_FANG:
            return TRUE;
        }
    }
    if (tval == TV_POLEARM)
    {
        switch (sval)
        {
        case SV_HATCHET:
        case SV_SICKLE:
        case SV_TSURIZAO:
            return TRUE;
        }
    }
    if (tval == TV_HAFTED)
    {
        switch (sval)
        {
        case SV_WHIP:
        case SV_NUNCHAKU:
        case SV_JO_STAFF:
        case SV_THREE_PIECE_ROD:
            return TRUE;
        }
    }
    if (tval == TV_DIGGING)
    {
        switch (sval)
        {
        case SV_SHOVEL:
        case SV_GNOMISH_SHOVEL:
            return TRUE;
        }
    }
    return FALSE;
}
static bool _weapon_is_giant(int tval, int sval)
{
    if (tval == TV_SWORD)
    {
        switch (sval)
        {
        case SV_GREAT_SCIMITAR:
        case SV_FLAMBERGE:
        case SV_TWO_HANDED_SWORD:
        case SV_NO_DACHI:
        case SV_EXECUTIONERS_SWORD:
        case SV_ZWEIHANDER:
            return TRUE;
        }
    }
    if (tval == TV_POLEARM)
    {
        switch (sval)
        {
        case SV_LANCE:
        case SV_GREAT_AXE:
        case SV_TRIFURCATE_SPEAR:
        case SV_LOCHABER_AXE:
        case SV_HEAVY_LANCE:
        case SV_SCYTHE_OF_SLICING:
        case SV_DEATH_SCYTHE:
            return TRUE;
        }
    }
    if (tval == TV_HAFTED)
    {
        switch (sval)
        {
        case SV_TWO_HANDED_FLAIL:
        case SV_GREAT_HAMMER:
        case SV_MACE_OF_DISRUPTION:
        case SV_GROND:
            return TRUE;
        }
    }
    if (tval == TV_DIGGING)
    {
        switch (sval)
        {
        case SV_MATTOCK:
            return TRUE;
        }
    }
    return FALSE;
}
static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (_weapon_is_small(o_ptr->tval, o_ptr->sval))
    {
        info_ptr->to_h -= 20;
        info_ptr->dis_to_h -= 20;
        info_ptr->to_d -= 5;
        info_ptr->dis_to_d -= 5;
        info_ptr->info_attr = TERM_RED;
        info_ptr->info = "Your weapon is too small.";
    }
    if (_weapon_is_giant(o_ptr->tval, o_ptr->sval))
    {
        info_ptr->to_h += 5 + p_ptr->lev/5;
        info_ptr->dis_to_h += 5 + p_ptr->lev/5;
        info_ptr->to_d += 3 + p_ptr->lev/7;
        info_ptr->dis_to_d += 3 + p_ptr->lev/7;
        info_ptr->info_attr = TERM_L_GREEN;
        info_ptr->info = "You love your weapon.";
    }
}

/**********************************************************************
 * Throw Monster :)
 **********************************************************************/
typedef struct {
    int m_idx;
    int wgt;
    int mult;
    int tdis;
    int tx;
    int ty;
} _monster_toss_info;
static void _monster_toss_imp(_monster_toss_info *info);

bool monster_toss(int m_idx)
{
    int dir, chance;
    _monster_toss_info info = {0};
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    char m_name[MAX_NLEN];

    monster_desc(m_name, m_ptr, 0);

    if (r_ptr->flags2 & RF2_PASS_WALL)
    {
        msg_format("Failed! %^s is incorporeal!", m_name);
        return TRUE;
    }
    if ((r_ptr->flags1 & RF1_UNIQUE) && r_ptr->level > p_ptr->lev)
    {
        msg_format("Failed! %^s is too powerful!", m_name);
        return TRUE;
    }
    chance = p_ptr->skills.thn + ((p_ptr->lev + p_ptr->to_h_m) * BTH_PLUS_ADJ);
    if (!test_hit_norm(chance, mon_ac(m_ptr), TRUE))
    {
        msg_format("You lose hold of %s.", m_name);
        return TRUE;
    }

    info.m_idx = m_idx;
    info.wgt = r_ptr->weight;

    /* Pick a target */
    info.mult = 1 + p_ptr->lev / 10;
    if (p_ptr->mighty_throw) info.mult++;
    {
        int mul, div;
        int wgt = info.wgt * 10;
        mul = 10 + 2 * (info.mult - 1);
        div = (wgt > 10) ? wgt : 10;
        div /= 2;
        info.tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 250) * mul / div;

        if (info.tdis <= 1) /* Can the giant lift the monster? */
        {
            msg_format("Failed! %^s is too heavy!", m_name);
            return TRUE;
        }

        info.tdis += info.mult; /* Tossing only 2 squares is pretty lame! */
        if (info.tdis > mul) info.tdis = mul;

        project_length = info.tdis;
        command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
        target_who = 0;  /* TODO: Repeat command is busted ... */
        if (!get_aim_dir(&dir)) return FALSE;

        info.tx = px + 99 * ddx[dir];
        info.ty = py + 99 * ddy[dir];

        if ((dir == 5) && target_okay())
        {
            info.tx = target_col;
            info.ty = target_row;
        }

        project_length = 0;

        if (info.tx == px && info.ty == py) return FALSE;
    }

    /* Toss */
    _monster_toss_imp(&info);

    return TRUE;
}

static void _monster_toss_imp(_monster_toss_info *info)
{
    char m_name[MAX_NLEN];
    u16b path[512];
    int msec = delay_time();
    int y, x, ny, nx, dam = 0;
    int cur_dis, ct;
    bool do_stun = FALSE;
    int chance;
    monster_type *m_ptr = &m_list[info->m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    chance = p_ptr->skill_tht + (p_ptr->lev * BTH_PLUS_ADJ);
    chance *= 2;

    monster_desc(m_name, m_ptr, 0);
    msg_format("You toss %s.", m_name);

    cave[m_ptr->fy][m_ptr->fx].m_idx = 0;
    lite_spot(m_ptr->fy, m_ptr->fx);

    ct = project_path(path, info->tdis, py, px, info->ty, info->tx, PROJECT_PATH);
    y = py;
    x = px;

    for (cur_dis = 0; cur_dis < ct; )
    {
        /* Peek ahead at the next square in the path */
        ny = GRID_Y(path[cur_dis]);
        nx = GRID_X(path[cur_dis]);

        /* Always draw the visual effect ... Its nice to see
           monsters bouncing off nearby walls :) */
        if (panel_contains(ny, nx) && player_can_see_bold(ny, nx))
        {
            char c = r_ptr->x_char;
            byte a = r_ptr->x_attr;

            print_rel(c, a, ny, nx);
            move_cursor_relative(ny, nx);
            Term_fresh();
            Term_xtra(TERM_XTRA_DELAY, msec);
            lite_spot(ny, nx);
            Term_fresh();
        }
        else
        {
            Term_xtra(TERM_XTRA_DELAY, msec);
        }

        /* Stopped by walls/doors/forest ... but allow hitting your target, please! */
        if (!cave_have_flag_bold(ny, nx, FF_PROJECT)
         && !cave[ny][nx].m_idx)
        {
            if (cave_have_flag_bold(ny, nx, FF_WALL))
            {
                dam = p_ptr->lev * info->mult;
                do_stun = one_in_(2);
            }
            else if (!(r_ptr->flags7 & RF7_CAN_FLY))
                dam = p_ptr->lev / 5 * info->mult;

            break;
        }

        /* Advance the distance */
        cur_dis++;

        /* Monster here, Try to hit it */
        if (cave[ny][nx].m_idx)
        {
            cave_type *c_ptr = &cave[ny][nx];
            monster_type *m_ptr2 = &m_list[c_ptr->m_idx];
            char m_name2[80];
            bool visible = m_ptr2->ml;

            monster_desc(m_name2, m_ptr2, 0);

            if (test_hit_fire(chance - cur_dis, mon_ac(m_ptr2), visible))
            {
                bool fear = FALSE;
                critical_t crit;

                if (!visible)
                    msg_format("%^s finds a mark.", m_name);
                else
                {
                    msg_format("%^s hits %s.", m_name, m_name2);
                    if (visible)
                    {
                        if (!p_ptr->image) mon_track(m_ptr2);
                        health_track(c_ptr->m_idx);
                    }
                }

                /***** The Damage Calculation!!! *****/
                dam = damroll(1 + MIN(10 + p_ptr->lev/3, info->wgt / 25), 5);
                crit = critical_throw(info->wgt * 10, p_ptr->lev);
                if (crit.desc)
                {
                    dam = dam * crit.mul/100 + crit.to_d;
                    msg_print(crit.desc);
                }
                dam *= info->mult;
                if (dam < 0) dam = 0;
                dam = mon_damage_mod(m_ptr, dam, FALSE);

                if (mon_take_hit(c_ptr->m_idx, dam, DAM_TYPE_ARCHERY, &fear, extract_note_dies(real_r_ptr(m_ptr2))))
                {
                    /* Dead monster */
                    x = nx;
                    y = ny;
                }
                else
                {
                    message_pain(c_ptr->m_idx, dam);
                    if (dam > 0)
                        anger_monster(m_ptr);

                    if (dam > 0 && m_ptr2->cdis > 1)
                        mon_anger_spell(m_ptr2, dam);

                    if (fear && visible)
                    {
                        sound(SOUND_FLEE);
                        msg_format("%^s flees in terror!", m_name2);
                    }
                }
            }
            else
            {
                msg_format("%^s misses %s.", m_name, m_name2);
            }

            /* Stop looking */
            break;
        }

        /* Save the new location */
        x = nx;
        y = ny;

        if (cur_dis >= ct)
        {
            if (!(r_ptr->flags7 & RF7_CAN_FLY))
                dam = p_ptr->lev / 5 * info->mult;
            break;
        }
    }

    if (cave_empty_bold(y, x))
    {
        cave[y][x].m_idx = info->m_idx;
        m_ptr->fy = y;
        m_ptr->fx = x;
        lite_spot(y, x);
    }
    else /* oops ... put the monster back where it started! */
    {
        cave[m_ptr->fy][m_ptr->fx].m_idx = info->m_idx;
        lite_spot(m_ptr->fy, m_ptr->fx);
    }
    if ((dam) && (!no_melee_challenge))
    {
        bool fear = FALSE;
        if (mon_take_hit(info->m_idx, dam, DAM_TYPE_MELEE, &fear, extract_note_dies(real_r_ptr(m_ptr))))
        {
            /* Dead monster */
        }
        else
        {
            if ( !do_stun
              || (r_ptr->flagsr & RFR_RES_ALL)
              || (r_ptr->flags3 & RF3_NO_STUN)
              || ((r_ptr->flags1 & RF1_UNIQUE) && mon_stun_save(r_ptr->level, dam)) )
            {
            }
            else
            {
                msg_format("%^s is stunned.", m_name);
                mon_stun(m_ptr, mon_stun_amount(dam));
            }

            message_pain(info->m_idx, dam);
            if (dam > 0)
                anger_monster(m_ptr);

            if (fear)
                msg_format("%^s flees in terror!", m_name);
        }
    }
}

void monster_toss_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Throw Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Throw an adjacent monster at chosen target, provided it is light enough, of course!");
        break;
    case SPELL_CAST:
    {
        int x, y;
        int dir;
        int m_idx = 0;
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        m_idx = cave[y][x].m_idx;

        if (!m_idx)
        {
            msg_print("There is no monster there.");
            if (p_ptr->blind > 0) var_set_bool(res, TRUE);
            return;
        }

        var_set_bool(res, monster_toss(m_idx));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}


/******************************************************************************
 *                 20             30            40
 * Hru: Hill Giant -> Stone Giant -> Rock Giant -> Hru
 ******************************************************************************/
static power_info _hru_get_powers[] = {
    { A_STR, {  5,  0, 50, throw_boulder_spell} },
    { A_STR, {  7,  0,  0, monster_toss_spell} },
    { A_STR, { 30,  5, 25, stone_to_mud_spell} },
    { A_STR, { 35, 10, 45, earthquake_spell} },
    { A_STR, { 40, 15, 50, stone_skin_spell} },
    {    -1, { -1, -1, -1, NULL}}
};

static void _hru_calc_bonuses(void) {
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev >= 30)
    {
        p_ptr->kill_wall = TRUE;
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SHARDS);
        p_ptr->to_a += 15;
        p_ptr->dis_to_a += 15;
    }
    p_ptr->skill_tht += 2*p_ptr->lev;
}
static void _hru_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SUST_STR);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_SHARDS);
}
static void _hru_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_HILL_GIANT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_STONE_GIANT;
        msg_print("You have evolved into a Stone Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GIANT && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ROCK_GIANT;
        msg_print("You have evolved into a Rock Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ROCK_GIANT && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_HRU;
        msg_print("You have evolved into a Hru.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_hru_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Hill Giant", "Stone Giant", "Rock Giant", "Hru"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  30,   0,  13,   7,  75,  30};
    skills_t xs = { 10,   7,  10,   0,   0,   0,  34,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 225;

        me.birth = _birth;
        /* This defeats the purpose of their monster king reward:
        me.calc_innate_attacks = _hru_calc_innate_attacks;*/
        me.get_powers = _hru_get_powers;
        me.calc_bonuses = _hru_calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_flags = _hru_get_flags;
        me.gain_level = _hru_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  7 + rank;
    me.stats[A_INT] = -5;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] = -3;
    me.stats[A_CON] =  4 + rank;
    me.stats[A_CHR] =  0 + rank/2;
    me.life = 110 + 5*rank;
    me.boss_r_idx = MON_ATLAS;

    return &me;
}

/******************************************************************************
 *                        20             30            40
 * Fire Giant: Hill Giant -> Stone Giant -> Fire Giant -> Elder Fire Giant
 ******************************************************************************/
static void _breathe_plasma_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Plasma");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes Plasma at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp*3/10));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_print("You breathe plasma...");
            fire_ball(GF_PLASMA, dir, p_ptr->chp*3/10, -3);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _fire_get_powers[] = {
    { A_STR, {  5,  0, 50, throw_boulder_spell} },
    { A_STR, {  7,  0,  0, monster_toss_spell} },
    { A_STR, { 30,  5, 25, fire_bolt_spell} },
    { A_STR, { 32, 10, 50, fire_ball_spell} },
    { A_STR, { 35, 15, 60, plasma_bolt_spell} },
    { A_CON, { 40,  0, 65, breathe_fire_II_spell} },
    { A_CON, { 42, 10, 70, _breathe_plasma_spell} },
    {    -1, { -1, -1, -1, NULL}}
};
static void _fire_calc_bonuses(void) {
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev >= 30)
    {
        res_add(RES_FIRE);
        p_ptr->sh_fire++;
    }
    if (p_ptr->lev >= 40)
        res_add(RES_FIRE);
    p_ptr->skill_tht += 2*p_ptr->lev;
}
static void _fire_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SUST_STR);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_AURA_FIRE);
    }
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_BRAND_FIRE);
}
static void _fire_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    _calc_weapon_bonuses(o_ptr, info_ptr);
    if (p_ptr->lev >= 40)
        add_flag(info_ptr->flags, OF_BRAND_FIRE);
}
static void _fire_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_HILL_GIANT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_STONE_GIANT;
        msg_print("You have evolved into a Stone Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GIANT && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_FIRE_GIANT;
        msg_print("You have evolved into a Fire Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FIRE_GIANT && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ELDER_FIRE_GIANT;
        msg_print("You have evolved into an Elder Fire Giant.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_fire_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Hill Giant", "Stone Giant", "Fire Giant", "Elder Fire Giant"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  30,   0,  13,   7,  75,  30};
    skills_t xs = { 10,   7,  10,   0,   0,   0,  34,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 200;

        me.birth = _birth;
        me.get_powers = _fire_get_powers;
        me.calc_bonuses = _fire_calc_bonuses;
        me.calc_weapon_bonuses = _fire_calc_weapon_bonuses;
        me.get_flags = _fire_get_flags;
        me.gain_level = _fire_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  6 + rank;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -3;
    me.stats[A_DEX] = -2;
    me.stats[A_CON] =  3 + rank;
    me.stats[A_CHR] =  0 + rank/2;
    me.life = 107 + 5*rank;
    me.boss_r_idx = MON_SURTUR;

    return &me;
}

/******************************************************************************
 *                         20             30             40
 * Frost Giant: Hill Giant -> Stone Giant -> Frost Giant -> Ice Giant
 ******************************************************************************/
static void _ice_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ice Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate a huge ball of ice on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 6*p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ICE, dir, 6*p_ptr->lev, 5);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _frost_get_powers[] = {
    { A_STR, {  5,  0, 50, throw_boulder_spell} },
    { A_STR, {  7,  0,  0, monster_toss_spell} },
    { A_STR, { 30,  3, 25, frost_bolt_spell} },
    { A_STR, { 32,  9, 50, frost_ball_spell} },
    { A_STR, { 35, 15, 60, ice_bolt_spell} },
    { A_STR, { 40, 40, 60, _ice_storm_spell} },
    {    -1, { -1, -1, -1, NULL}}
};
static void _frost_calc_bonuses(void) {
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev >= 30)
    {
        res_add(RES_COLD);
        p_ptr->sh_cold++;
    }
    if (p_ptr->lev >= 40)
        res_add(RES_COLD);
    p_ptr->skill_tht += 2*p_ptr->lev;
}
static void _frost_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SUST_STR);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_AURA_COLD);
    }
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_BRAND_COLD);
}
static void _frost_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    _calc_weapon_bonuses(o_ptr, info_ptr);
    if (p_ptr->lev >= 40)
        add_flag(info_ptr->flags, OF_BRAND_COLD);
}
static void _frost_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_HILL_GIANT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_STONE_GIANT;
        msg_print("You have evolved into a Stone Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GIANT && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_FROST_GIANT;
        msg_print("You have evolved into a Frost Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_FROST_GIANT && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ICE_GIANT;
        msg_print("You have evolved into an Ice Giant.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_frost_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Hill Giant", "Stone Giant", "Frost Giant", "Ice Giant"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  30,   0,  13,   7,  75,  30};
    skills_t xs = { 10,   7,  10,   0,   0,   0,  34,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 200;

        me.birth = _birth;
        me.get_powers = _frost_get_powers;
        me.calc_bonuses = _frost_calc_bonuses;
        me.calc_weapon_bonuses = _frost_calc_weapon_bonuses;
        me.get_flags = _frost_get_flags;
        me.gain_level = _frost_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  6 + rank;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -3;
    me.stats[A_DEX] = -2;
    me.stats[A_CON] =  3 + rank;
    me.stats[A_CHR] =  0 + rank/2;
    me.life = 107 + 5*rank;
    me.boss_r_idx = MON_YMIR;

    return &me;
}

/*******************************************************************************************
 *                         20             30             40             45
 * Storm Giant: Hill Giant -> Stone Giant -> Cloud Giant -> Storm Giant -> Elder Storm Giant
 *******************************************************************************************/
static void _breathe_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes storm winds at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp*3/10));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_print("You breathe storm winds...");
            fire_ball(GF_STORM, dir, p_ptr->chp*3/10, -3);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _lightning_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Storm");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate a huge ball of lightning on chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 7*p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ELEC, dir, 7*p_ptr->lev, 5);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _storm_get_powers[] = {
    { A_STR, {  5,  0, 50, throw_boulder_spell} },
    { A_STR, {  7,  0,  0, monster_toss_spell} },
    { A_STR, { 30,  3, 25, lightning_bolt_spell} },
    { A_STR, { 35, 10, 50, lightning_ball_spell} },
    { A_DEX, { 40,  2, 10, phase_door_spell} },
    { A_DEX, { 40, 10, 50, teleport_to_spell} },
    { A_STR, { 40, 40, 60, _lightning_storm_spell} },
    { A_CON, { 45, 10, 70, _breathe_storm_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static void _storm_calc_bonuses(void) {
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev >= 30)
        res_add(RES_ELEC);

    if (p_ptr->lev >= 40)
    {
        p_ptr->sh_elec++;
        res_add(RES_ELEC);
    }
    p_ptr->skill_tht += 2*p_ptr->lev;
}
static void _storm_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SUST_STR);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_ELEC);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_AURA_ELEC);
        add_flag(flgs, OF_BRAND_ELEC);
    }
}
static void _storm_calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    _calc_weapon_bonuses(o_ptr, info_ptr);
    if (p_ptr->lev >= 40)
        add_flag(info_ptr->flags, OF_BRAND_ELEC);
}
static void _storm_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_HILL_GIANT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_STONE_GIANT;
        msg_print("You have evolved into a Stone Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GIANT && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_CLOUD_GIANT;
        msg_print("You have evolved into a Cloud Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_CLOUD_GIANT && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_STORM_GIANT;
        msg_print("You have evolved into a Storm Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STORM_GIANT && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_ELDER_STORM_GIANT;
        msg_print("You have evolved into an Elder Storm Giant.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_storm_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[5] =  {"Hill Giant", "Stone Giant", "Cloud Giant", "Storm Giant", "Elder Storm Giant"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  30,   0,  13,   7,  70,  30};
    skills_t xs = { 10,   7,  10,   0,   0,   0,  30,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _birth;
        me.get_powers = _storm_get_powers;
        me.calc_bonuses = _storm_calc_bonuses;
        me.calc_weapon_bonuses = _storm_calc_weapon_bonuses;
        me.get_flags = _storm_get_flags;
        me.gain_level = _storm_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  5 + (rank + 1)/2;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -3;
    me.stats[A_DEX] = -2;
    me.stats[A_CON] =  3 + (rank + 1)/2;
    me.stats[A_CHR] =  0 + rank/2;
    me.life = 105 + 3*rank;
    me.boss_r_idx = MON_TYPHOEUS;

    return &me;
}

/******************************************************************************
 *                   20             30              40
 * Titan: Hill Giant -> Stone Giant -> Lesser Titan -> Greater Titan
 ******************************************************************************/
static power_info _titan_get_powers[] = {
    { A_STR, {  5,   0, 50, throw_boulder_spell} },
    { A_STR, {  7,   0,  0, monster_toss_spell} },
    { A_CHR, { 30,  30, 25, summon_monsters_spell} },
    { A_DEX, { 35,  10, 50, teleport_to_spell} },
    { A_CHR, { 40,  30, 50, summon_kin_spell} },
    { A_CON, { 45, 100, 95, healing_I_spell} }, /* N.B. Casting costs are paid with hp! */
    {    -1, { -1,  -1, -1, NULL}}
};
static void _titan_calc_bonuses(void) {
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev >= 30)
    {
        res_add(RES_CHAOS);
        p_ptr->pspeed += 2;
    }
    if (p_ptr->lev >= 40)
        p_ptr->pspeed += 3;
    p_ptr->skill_tht += 2*p_ptr->lev;
}
static void _titan_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_SUST_STR);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_SPEED);
    }
}
static void _titan_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_HILL_GIANT && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_STONE_GIANT;
        msg_print("You have evolved into a Stone Giant.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GIANT && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_LESSER_TITAN;
        msg_print("You have evolved into a Lesser Titan.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_LESSER_TITAN && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_TITAN;
        msg_print("You have evolved into a Greater Titan.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_titan_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[4] =  {"Hill Giant", "Stone Giant", "Lesser Titan", "Greater Titan"};
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  20,  32,   0,  15,  10,  75,  30};
    skills_t xs = { 11,   8,  10,   0,   0,   0,  35,  15};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _birth;
        me.get_powers = _titan_get_powers;
        me.calc_bonuses = _titan_calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_flags = _titan_get_flags;
        me.gain_level = _titan_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  5 + rank;
    me.stats[A_INT] =  0 + rank;
    me.stats[A_WIS] =  0 + rank;
    me.stats[A_DEX] = -2;
    me.stats[A_CON] =  3 + rank;
    me.stats[A_CHR] =  3 + rank;
    me.life = 102 + 5*rank;
    me.boss_r_idx = MON_KRONOS;

    return &me;
}

static name_desc_t _info[GIANT_MAX] = {
    { "Fire Giant", "Fire Giants are massive giants of flame. At high levels they become "
                        "wreathed in flames, and even their weapons will burn their foes. Like "
                        "all giants, they may toss loose rubble at their foes. In addition, "
                        "they have a few fire-based distance attacks up their sleeves." },
    { "Frost Giant", "Frost Giants are massive giants of ice. At high levels they become "
                        "wreathed in cold, and even their weapons will freeze their foes. Like "
                        "all giants, they may toss loose rubble at their foes. In addition, "
                        "they have a few cold-based distance attacks up their sleeves." },
    { "Storm Giant", "Storm Giants are massive giants of lightning. At high levels they become "
                        "wreathed in electricity, and even their weapons will shock their foes. Like "
                        "all giants, they may toss loose rubble at their foes. In addition, "
                        "they have a few lightning-based distance attacks up their sleeves." },
    { "Titan", "Titans are huge immortal beings of incredible strength and awesome power. "
                "Descended from Gaia and Uranus, they ruled during the legendary Golden Age, "
                "but were overthrown by the Olympians during the War of the Titans." },
    { "Hru", "Hrus are rock giants, made of stone. Their hides are tough and they are able "
                "to break through walls effortlessly. Hrus are incredibly strong, but lack "
                "much in the way of magical powers." },
};
/**********************************************************************
 * Public
 **********************************************************************/
bool giant_is_favorite(object_type *o_ptr)
{
    if (prace_is_(RACE_MON_GIANT))
        return _weapon_is_giant(o_ptr->tval, o_ptr->sval);
    return FALSE;
}

race_t *mon_giant_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case GIANT_FIRE:
        result = _fire_get_race_t();
        break;
    case GIANT_FROST:
        result = _frost_get_race_t();
        break;
    case GIANT_STORM:
        result = _storm_get_race_t();
        break;
    case GIANT_TITAN:
        result = _titan_get_race_t();
        break;
    case GIANT_HRU:
        result = _hru_get_race_t();
        break;
    default: /* Birth Menus */
        result = _fire_get_race_t();
    }

    result->name = "Giant";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->base_hp = 46;
    result->pseudo_class_idx = CLASS_WARRIOR;
    result->shop_adjust = 130;

    if (birth_hack || spoiler_hack)
    {
        result->subname = _info[psubrace].name;
        result->subdesc = _info[psubrace].desc;
    }

    return result;
}

