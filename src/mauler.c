#include "angband.h"

/****************************************************************
 * Helpers
 ****************************************************************/

static bool _weapon_check(void)
{
    /* Fail if any weapon is not wielded with 2 hands */
    int hand;
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if ( p_ptr->weapon_info[hand].wield_how != WIELD_NONE
          && p_ptr->weapon_info[hand].wield_how != WIELD_TWO_HANDS )
        {
            return FALSE;
        }
    }
    /* Fail if there is no wielded weapon */
    return equip_find_first(object_is_melee_weapon);
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    return result;
}

int mauler_get_toggle(void)
{
    /* exposed for prtstatus() in xtra1.c 
       this is easier than rewriting the status code so that classes can maintain it!
    */
    int result = TOGGLE_NONE;
    if (p_ptr->pclass == CLASS_MAULER && _weapon_check())
        result = _get_toggle();
    return result;
}

static void _blast_obj(obj_ptr obj)
{
    char o_name[MAX_NLEN];
    object_desc(o_name, obj, OD_OMIT_PREFIX | OD_COLOR_CODED);
    msg_format("A terrible black aura blasts your %s!", o_name);
    blast_object(obj);
    disturb(1, 0);
}

void process_maul_of_vice(void)
{
    int amt;
    if (!p_ptr->maul_of_vice) return;
    
    amt = randint1(p_ptr->lev);
    p_ptr->au -= amt;
    stats_on_gold_stolen(amt);
    p_ptr->redraw |= PR_GOLD;

    if (p_ptr->au < 0)
    {
        slot_t slot = pack_find_art(ART_MAUL_OF_VICE);

        if (slot) _blast_obj(pack_obj(slot));
        else
        {
            slot = equip_find_art(ART_MAUL_OF_VICE);
            if (slot) _blast_obj(equip_obj(slot));
        }
        p_ptr->au = 0;
        p_ptr->update |= PU_BONUS;
    }
    else if (p_ptr->au < 1000)
    {
        msg_print("***LOW GOLD WARNING!!!!***");
        disturb(1, 0);
    }
    else if (one_in_(111))
    {
        msg_print("You feel your wealth draining away!");
    }
}

bool do_blow(int type)
{
    int x = 0, y = 0;
    int dir;
    int m_idx = 0;

    if ((!p_ptr->weapon_ct) && (!attack_mode_allows_innate(type)))
    {
        msg_print("You need to wield a weapon!");
        return FALSE;
    }

    /* For ergonomics sake, use currently targeted monster. This allows
       a macro of \e*tmaa or similar to pick an adjacent foe, while
       \emaa*t won't work, since get_rep_dir2() won't allow a target. */
    if (old_target_okay())
    {
        y = target_row;
        x = target_col;
        m_idx = cave[y][x].m_idx;
        if (m_idx)
        {
            if (m_list[m_idx].cdis > 1)
                m_idx = 0;
            else
                dir = 5;
        }
    }

    if (!m_idx)
    {
        if (!get_rep_dir2(&dir)) return FALSE;
        if (dir == 5) return FALSE;

        y = py + ddy[dir];
        x = px + ddx[dir];
        m_idx = cave[y][x].m_idx;

        if (!m_idx)
        {
            msg_print("There is no monster there.");

            /* Consume energy if we couldn't know there was no monster there */
            return (p_ptr->blind > 0);
        }

    }

    if (m_idx)
        py_attack(y, x, type);

    return TRUE;
}

/****************************************************************
 * Spells
 ****************************************************************/
static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


static void _block_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Block");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you will gain an AC bonus based on the weight of your current weapon.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_BLOCK, cmd, res);
        break;
    }
}

static void _close_in_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Close In");
        break;
    case SPELL_DESC:
        var_set_string(res, "Close in for the kill on a nearby monster.");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, rush_attack(2, NULL));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _critical_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Critical Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a single devastating blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MAULER_CRITICAL_BLOW));
        break;
    case SPELL_ON_BROWSE:
    {
        display_weapon_info_aux(MAULER_CRITICAL_BLOW);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _crushing_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Crushing Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a single powerful blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MAULER_CRUSHING_BLOW));
        break;
    case SPELL_ON_BROWSE:
    {
        display_weapon_info_aux(MAULER_CRUSHING_BLOW);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_ferocity_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Ferocity");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects all monsters except mindless in your vicinity.");
        break;
    case SPELL_CAST:
        detect_monsters_mind(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
    
static void _drain_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Drain");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique you will drain life from your foes. In addition, enemies will never fully recover from the wounds you inflict.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_DRAIN, cmd, res);
        break;
    }
}

static void _killing_spree_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Killing Spree");
        break;
    case SPELL_DESC:
        var_set_string(res, "Engage in wanton destruction! During this time, any foe you slay will haste you, so seek to kill as many enemies as possible!");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->tim_killing_spree)
        {
            msg_print("You are already on a Killing Spree. Show some mercy!");
            return;
        }
        set_tim_killing_spree(44, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _knockback_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Knockback");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a single blow. If landed, your foe will be knocked back away from you.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MAULER_KNOCKBACK));
        break;
    case SPELL_ON_BROWSE:
    {
        display_weapon_info_aux(MAULER_KNOCKBACK);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _maul_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Maul");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique you will maul your opponents.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_MAUL, cmd, res);
        break;
    }
}

void _scatter_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Scatter");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack all adjacent monsters with a single strike. If landed, your enemies will be scattered away from you.");
        break;
    case SPELL_CAST:
    {
        int              dir, x, y;
        cave_type       *c_ptr;
        monster_type    *m_ptr;

        for (dir = 0; dir < 8; dir++)
        {
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            m_ptr = &m_list[c_ptr->m_idx];

            if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                py_attack(y, x, MAULER_SCATTER);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shatter_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shatter");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, your weapon will cause earthquakes.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_SHATTER, cmd, res);
        break;
    }
}

static void _smash_wall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smash");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys adjacent targeted wall, door, tree, or trap.");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        
        if (!in_bounds(y, x)) return;

        if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            p_ptr->update |= PU_FLOW;
        }
        else if ((cave_have_flag_bold(y, x, FF_TUNNEL)) && (cave_have_flag_bold(y, x, FF_HURT_FIRE)))
        {
            cave_alter_feat(y, x, FF_HURT_FIRE);
            p_ptr->update |= PU_FLOW;
        }
        else if (cave_have_flag_bold(y, x, FF_TREE))
        {
            if (cave_have_flag_bold(y, x, FF_SNOW))
                cave_set_feat(y, x, feat_snow_floor);
            else cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);
        }
        else
        {
            int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
            project(0, 0, y, x, 0, GF_KILL_DOOR, flg);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _splatter_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Splatter");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, monsters will explode as you kill them.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_SPLATTER, cmd, res);
        break;
    }
}

void stunning_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stunning Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a single blow aimed to stun.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MAULER_STUNNING_BLOW));
        break;
    case SPELL_ON_BROWSE:
    {
        display_weapon_info_aux(MAULER_STUNNING_BLOW);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _tunnel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Tunnel");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique you may move through walls.");
        break;
    default:
        _toggle_spell(MAULER_TOGGLE_TUNNEL, cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    {  5,  5, 30, _smash_wall_spell},
    {  8,  5, 30, _detect_ferocity_spell},
    { 12, 10,  0, _critical_blow_spell},
    { 15,  0,  0, _splatter_spell},
    { 17,  0,  0, _block_spell},
    { 21, 15, 40, _close_in_spell},
    { 23, 20,  0, _knockback_spell},
    { 30,  0,  0, _shatter_spell},
    { 32, 30, 50, _killing_spree_spell},
    { 35, 30, 50, _scatter_spell},
    { 37,  0,  0, _tunnel_spell},
    { 40, 50,  0, _crushing_blow_spell},
    { 42,  0,  0, _drain_spell},
    { 45,  0,  0, _maul_spell},
    { -1, -1, -1, NULL}
};

static spell_info *_get_spells(void)
{
    if (!_weapon_check())
    {
        msg_print("Rargh! You need to wield a single weapon with both hands to properly maul stuff!");
        return NULL;
    }

    return _spells;
}

static void _calc_bonuses(void)
{
    int w = equip_weight(object_is_melee_weapon);
    if (_weapon_check())
    {
        if (_get_toggle() == MAULER_TOGGLE_BLOCK)
        {
            int a = w/20 + (w/100)*(w/100);
            p_ptr->to_a += a;
            p_ptr->dis_to_a += a;
        }

        /* TODO: This should cost more energy too ... */
        if (_get_toggle() == MAULER_TOGGLE_TUNNEL)
            p_ptr->kill_wall = TRUE;

        if (_get_toggle() == MAULER_TOGGLE_MAUL)
        {
            p_ptr->to_a -= 20;
            p_ptr->dis_to_a -= 20;
        }
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (_weapon_check())
    {
        /* CL1: Mighty */
        if (p_ptr->lev >= 1)
        {
            int w = o_ptr->weight;
            int h = (w - 150)/20;
            int d = (w - 150)/10;

            if (_get_toggle() != MAULER_TOGGLE_BLOCK)
            {
                info_ptr->to_h += h;
                info_ptr->dis_to_h += h;

                info_ptr->to_d += d;
                info_ptr->dis_to_d += d;
            }
        }
        /* CL25: Impact 
            20lb +1d0
            25lb +1d1
            30lb +2d1
            40lb +2d2
            50lb +3d2
            60lb +3d3

            Stagger in bonuses with level. Also, I'm debating > rather than >= for cutoffs.
        */
        if (p_ptr->lev >= 25 )
        {
            int w = o_ptr->weight;

            if (w >= 200)
                info_ptr->to_dd++;
            if (p_ptr->lev >= 30 && w >= 250)
                info_ptr->to_ds++;
            if (p_ptr->lev >= 35 && w >= 300)
                info_ptr->to_dd++;
            if (p_ptr->lev >= 40 && w >= 400)
                info_ptr->to_ds++;
            if (p_ptr->lev >= 45 && w >= 500)
                info_ptr->to_dd++;
            if (p_ptr->lev >= 50 && w >= 600)
                info_ptr->to_ds++;
        }

        /* Destroyer is handled as a hack in cmd1.c critical_norm() and now scales with level */

        if (_get_toggle() == MAULER_TOGGLE_MAUL)
        {
            info_ptr->to_dd += 1;
            info_ptr->to_ds += 1;
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "technique";
        me.options = CASTER_USE_HP;
        me.which_stat = A_STR;
        init = TRUE;
    }
    return &me;
}

static void _character_dump(doc_ptr doc)
{
    if (_weapon_check() && p_ptr->lev >= 5)
    {
        py_dump_spells(doc);
    }
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_TWO_HANDED_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_BOOTS, SV_PAIR_OF_METAL_SHOD_BOOTS, 1);
}

class_t *mauler_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  25,  35,   0,  14,   2,  70,  16 };
    skills_t xs = { 12,   9,  12,   0,   0,   0,  30,   6 };

        me.name = "Mauler";
        me.desc = 
        "The Mauler favors extremely heavy weapons, and possesses powerful abilities whose "
            "effectiveness depends on the weight of the weapon. While they only gain a limited "
            "number of blows which can never be magically increased, they are capable of "
            "hitting opponents exceptionally hard to make the most of each strike. Maulers "
            "must wield a single weapon with two hands for their talents to "
            "function properly.",

        me.stats[A_STR] =  5;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 110;
        me.base_hp = 18;
        me.exp = 120;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
