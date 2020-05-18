#include "angband.h"
#include "equip.h"

void absconding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Absconding");
        break;
    default:
        teleport_spell(cmd, res);
        break;
    }
}

static void _ancient_knowledge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ancient Knowledge");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

static void _bind_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bind Monster");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to freeze a monster.");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stasis_monster(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void bunshin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bunshin");
        break;
    case SPELL_DESC:
        var_set_string(res, "Creates shadows of yourself, giving you a 1 in 3 chance to completely evade any attacks.");
        break;
    case SPELL_CAST:
        set_multishadow(6+randint1(6), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _chain_hook_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Chain Hook");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleport a monster to a place adjacent to you.");
        break;
    case SPELL_CAST:
    {
        monster_type *m_ptr;
        int m_idx;
        char m_name[80];
        int i;
        int path_n;
        u16b path_g[512];
        int ty,tx;

        var_set_bool(res, FALSE);
        if (!target_set(TARGET_KILL)) return;
        m_idx = cave[target_row][target_col].m_idx;
        if (!m_idx) return;
        if (m_idx == p_ptr->riding) return;
        if (!player_has_los_bold(target_row, target_col)) return;
        if (!projectable(py, px, target_row, target_col)) return;
        m_ptr = &m_list[m_idx];
        monster_desc(m_name, m_ptr, 0);
        msg_format("You pull back %s.", m_name);

        path_n = project_path(path_g, MAX_RANGE, target_row, target_col, py, px, 0);
        ty = target_row, tx = target_col;
        for (i = 1; i < path_n; i++)
        {
            int ny = GRID_Y(path_g[i]);
            int nx = GRID_X(path_g[i]);
            cave_type *c_ptr = &cave[ny][nx];

            if (in_bounds(ny, nx) && cave_empty_bold(ny, nx) &&
                !(c_ptr->info & CAVE_OBJECT) &&
                !pattern_tile(ny, nx))
            {
                ty = ny;
                tx = nx;
            }
        }
        cave[target_row][target_col].m_idx = 0;
        cave[ty][tx].m_idx = m_idx;
        m_ptr->fy = ty;
        m_ptr->fx = tx;
        (void)set_monster_csleep(m_idx, 0);
        update_mon(m_idx, TRUE);
        lite_spot(target_row, target_col);
        lite_spot(ty, tx);

        if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
            p_ptr->update |= (PU_MON_LITE);

        if (m_ptr->ml)
        {
            if (!p_ptr->image) mon_track(m_ptr);
            health_track(m_idx);
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_near_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Near");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "Detects nearby monsters, traps, doors, stairs and objects. Maps the entire level.");
        else if (p_ptr->lev >= 15)
            var_set_string(res, "Detects nearby monsters, traps, doors, stairs and objects.");
        else if (p_ptr->lev >= 5)
            var_set_string(res, "Detects nearby monsters, traps, doors and stairs.");
        else 
            var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 45)
        {
            wiz_lite(TRUE);
        }
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (p_ptr->lev >= 5)
        {
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
        }
        if (p_ptr->lev >= 15)
        {
            detect_objects_normal(DETECT_RAD_DEFAULT);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void floating_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Floating");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives levitation for a while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
        set_tim_levitation(randint1(20) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _glyph_of_explosion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Glyph of Explosion");
        break;
    default:
        explosive_rune_spell(cmd, res);
        break;
    }
}

void hide_in_flame_spell(int cmd, variant *res)
{
    int dam = 50 + p_ptr->lev;
    int rad = 2 + p_ptr->lev/10;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hide in Flame");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate a fire ball and teleport in a time. Gives resistance to fire for a while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam/2));
        break;
    case SPELL_CAST:
        fire_ball(GF_FIRE, 0, dam, rad);
        teleport_player(30, 0);
        set_oppose_fire(p_ptr->lev, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hide_in_leaves_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hide in Leaves");
        break;
    default:
        phase_door_spell(cmd, res);
        break;
    }
}

void hide_in_mist_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hide in Mist");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates huge balls of poison, drain life and confusion, then teleports 30 squares.");
        break;
    case SPELL_CAST:
        fire_ball(GF_POIS, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        fire_ball(GF_OLD_DRAIN, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        fire_ball(GF_CONFUSION, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        teleport_player(30, 0L);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hit_and_away_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Hit and Away");
        break;
    default:
        panic_hit_spell(cmd, res);
        break;
    }
}

void kawarimi_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Kawarimi");
        break;
    case SPELL_DESC:
        var_set_string(res, "Teleport as you receive an attack. Might be able to teleport just before receiving damages at higher level.");
        break;
    case SPELL_CAST:
        if (!(p_ptr->special_defense & NINJA_KAWARIMI))
        {
            msg_print("You are now prepared to evade any attacks.");
            p_ptr->special_defense |= NINJA_KAWARIMI;
            p_ptr->redraw |= PR_STATUS;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nyusin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nyusin");
        break;
    case SPELL_DESC:
        var_set_string(res, "Steps close to a monster and attacks at a time.");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void quick_walk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Quick Walk");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        if (p_ptr->action == ACTION_QUICK_WALK) set_action(ACTION_NONE);
        else set_action(ACTION_QUICK_WALK);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rengoku_kaen_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rengoku-Kaen");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires some number of beams of fire, nether or plasma in random directions.");
        break;
    case SPELL_CAST:
    {
        int k, x = 0, y = 0;
        int num = damroll(3, 9);

        for (k = 0; k < num; k++)
        {
            int typ = one_in_(2) ? GF_FIRE : one_in_(3) ? GF_NETHER : GF_PLASMA;
            int attempts = 1000;

            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);

                if (!player_bold(y, x)) break;
            }
            project(0, 0, y, x, damroll(6 + p_ptr->lev / 8, 10), typ,
                (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL));
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smoke_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smoke Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Releases a confusion ball which doesn't inflict any damage.");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_OLD_CONF, dir, p_ptr->lev*3, 3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _obj_is_shuriken(obj_ptr obj) { return obj->tval == TV_SPIKE; }

void syuriken_spreading_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Syuriken Spreading");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoots 8 iron Spikes in 8 random directions.");
        break;
    case SPELL_CAST:
    {
        int i;
        for (i = 0; i < 8; i++)
        {
            int        slot = pack_find_first(_obj_is_shuriken);
            py_throw_t context = {0}; /* better reset for each shot! */
            if (!slot)
            {
                if (!i) msg_print("You have no Iron Spikes.");
                else msg_print("You have no more Iron Spikes.");
                break;
            }
            context.dir = DIR_RANDOM;
            context.obj = pack_obj(slot);
            py_throw(&context);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _get_spells[] =
{
    /*lvl cst fail spell */
    { 1,   1,  20, create_darkness_spell},
    { 2,   2,  25, _detect_near_spell},
    { 3,   3,  25, hide_in_leaves_spell},
    { 5,   3,  30, kawarimi_spell},
    { 7,   8,  35, absconding_spell},
    { 8,  10,  35, hit_and_away_spell},
    {10,  10,  40, _bind_monster_spell},
    {12,  12,  70, _ancient_knowledge_spell},
    {15,  10,  50, floating_spell},
    {17,  12,  45, hide_in_flame_spell},
    {18,  20,  40, nyusin_spell},
    {20,   5,  50, syuriken_spreading_spell},
    {22,   5,  55, _chain_hook_spell},
    {25,  32,  60, _smoke_ball_spell},
    {28,  32,  60, swap_pos_spell},
    {30,  30,  70, _glyph_of_explosion_spell},
    {32,  40,  40, hide_in_mud_spell},
    {34,  35,  50, hide_in_mist_spell},
    {38,  40,  60, _rengoku_kaen_spell},
    {41,  50,  55, bunshin_spell},
    { -1, -1,  -1, NULL}
};

static power_info _get_powers[] =
{
    { A_NONE, { 20, 0,  0, quick_walk_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    if (heavy_armor())
    {
        p_ptr->pspeed -= p_ptr->lev/10;
        p_ptr->skills.stl -= p_ptr->lev/10;
    }
    else if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->pspeed += 3;
        p_ptr->pspeed += p_ptr->lev/10;
        p_ptr->skills.stl += p_ptr->lev/10;
        if (p_ptr->lev >= 25)
            p_ptr->free_act++;
        /* Ninjas are not archers, and have relatively poor thb skills.
         * However, they excel at throwing (tht)! */
        p_ptr->skill_tht += 30 + p_ptr->lev;
    }
    if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->to_a += p_ptr->lev/2 + 5;
        p_ptr->dis_to_a += p_ptr->lev/2 + 5;
    }
    p_ptr->slow_digest = TRUE;
    res_add(RES_FEAR);
    if (p_ptr->lev >= 20) res_add(RES_POIS);
    if (p_ptr->lev >= 25) p_ptr->sustain_dex = TRUE;
    if (p_ptr->lev >= 30) p_ptr->see_inv++;
    if (p_ptr->lev >= 45) res_add(RES_POIS);
    p_ptr->see_nocto = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (heavy_armor())
        add_flag(flgs, OF_SPEED);
    else
    {
        if (!equip_find_obj(TV_SHIELD, SV_ANY))
        {
            add_flag(flgs, OF_SPEED);
        }
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_FREE_ACT);
    }
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_NIGHT_VISION);
    if (p_ptr->lev >= 20) add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 25) add_flag(flgs, OF_SUST_DEX);
    if (p_ptr->lev >= 30) add_flag(flgs, OF_SEE_INVIS);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if ( skills_weapon_is_icky(o_ptr->tval, o_ptr->sval) 
      || equip_find_obj(TV_SHIELD, SV_ANY) )
    {
        info_ptr->to_h -= 40;
        info_ptr->dis_to_h -= 40;
        info_ptr->icky_wield = TRUE;
        info_ptr->base_blow /= 2;
        info_ptr->xtra_blow /= 2;
        if (info_ptr->base_blow < 100) info_ptr->base_blow = 100;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "ninjutsu";
        me.options = CASTER_USE_HP;
        me.which_stat = A_DEX;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
    py_birth_obj_aux(TV_SPIKE, 0, rand_range(15, 20));
}

class_t *ninja_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  24,  36,   8,  48,  32,  70,  35 };
    skills_t xs = { 15,  10,  10,   0,   0,   0,  25,  11 };

        me.name = "Ninja";
        me.desc = "A Ninja is a fearful assassin lurking in darkness. He or she can "
                    "navigate effectively with no light source, catch enemies by surprise, "
                    "and kill with a single blow. Ninjas can use Ninjutsu, and are "
                    "good at locating hidden traps and doors, disarming traps and "
                    "picking locks. Since heavy armors, heavy weapons, or shields will "
                    "restrict their motion greatly, they prefer light clothes, and "
                    "become faster and more stealthy as they gain levels. A Ninja "
                    "knows no fear and, at high level, becomes almost immune to poison "
                    "and able to see invisible things. Dexterity determines a Ninja's "
                    "ability to use Ninjutsu.\n \n"
                    "A Ninja can use Ninjutsu for lurking and surprise attacks. They "
                    "gain more Ninjutsu techniques as they gain levels. They have a "
                    "class power - 'Quick Walk' - which makes their walking speed "
                    "extremely fast.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 120;
        me.pets = 40;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = py_dump_spells;
        me.known_icky_object = skills_obj_is_icky_weapon;
        init = TRUE;
    }

    return &me;
}
