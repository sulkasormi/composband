#include "angband.h"

static void _cavern_creation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cavern Creation");
        break;
    case SPELL_DESC:
        var_set_string(res, "Stone to Mud all surrounding walls.");
        break;
    case SPELL_CAST:
    {
        int dir, x, y, ct = 0;
        for (dir = 0; dir < 8; dir++)
        {
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];

            if (!in_bounds(y, x)) continue;
            if (!cave_have_flag_bold(y, x, FF_HURT_ROCK))  continue;
            cave_alter_feat(y, x, FF_HURT_ROCK);
            ct++;
        }
        if (ct)
            p_ptr->update |= (PU_FLOW | PU_BONUS);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _dark_stalker_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Dark Stalker");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporarily grants enhanced stealth.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(50, 50));
        break;
    case SPELL_CAST:
        set_tim_dark_stalker(50 + randint1(50), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _greater_mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Greater Mapping");
        break;
    default:
        clairvoyance_spell(cmd, res);
        break;
    }
}

static void _greater_whirlwind_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Greater Ambush");
        break;
    case SPELL_DESC:
        var_set_string(res, "Perform a massive ambush on nearby monsters.");
        break;
    case SPELL_CAST:
    {
        int              i, x, y;
        cave_type       *c_ptr;
        monster_type    *m_ptr;

/*       cba
        d218l
        e3@7k
        f456j
         ghi  */

        typedef struct _offset_t { int dx; int dy; } _offset;
        static _offset offsets[] = {
            { 0, -1},
            {-1, -1},
            {-1,  0},
            {-1,  1},
            { 0,  1},
            { 1,  1},
            { 1,  0},
            { 1, -1},
            { 1, -2},
            { 0, -2},
            {-1, -2},
            {-2, -1},
            {-2,  0},
            {-2,  1},
            {-1,  2},
            { 0,  2},
            { 1,  2},
            { 2,  1},
            { 2,  0},
            { 2, -1},
            { 0,  0}, /* sentinel */
        };

        for (i = 0;; i++)
        {
            _offset offset = offsets[i];
            if (offset.dx == 0 && offset.dy == 0) break;

            y = py + offset.dy;
            x = px + offset.dx;

            if (!in_bounds(y, x)) continue;
            if (!projectable(py, px, y, x)) continue;

            c_ptr = &cave[y][x];

            if (!c_ptr->m_idx) continue;

            m_ptr = &m_list[c_ptr->m_idx];

            if (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT))
            {
                int msec = delay_time();

                if (panel_contains(y, x) && player_can_see_bold(y, x))
                {
                    char c = 0x30;
                    byte a = TERM_WHITE;

                    print_rel(c, a, y, x);
                    move_cursor_relative(y, x);
                    Term_fresh();
                    Term_xtra(TERM_XTRA_DELAY, msec);
                    lite_spot(y, x);
                    Term_fresh();
                }
                else
                    Term_xtra(TERM_XTRA_DELAY, msec);

                py_attack(y, x, 0);
            }
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _lookout_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lookout");
        break;
    default:
        detect_monsters_spell(cmd, res);
        break;
    }
}

static void _mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mapping");
        break;
    default:
        magic_mapping_spell(cmd, res);
        break;
    }
}

static void _nimble_dodge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Nimble Dodge");
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short time, you will have a chance of dodging enemy breath attacks.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
        set_tim_nimble_dodge(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _reconnaissance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reconnaissance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Quickly scout nearby terrain for enemies, traps and loot.");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_MAP);
        detect_all(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _retreat_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Retreat");
        break;
    default:
        teleport_spell(cmd, res);
        break;
    }
}

static void _sniping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sniping");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot a sleeping enemy sentry with great precision.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!equip_find_obj(TV_BOW, SV_ANY))
        {
            msg_print("You need a bow to use this talent.");
            break;
        }
        shoot_hack = SHOOT_SNIPING;
        command_cmd = 'f'; /* Hack for inscriptions (e.g. '@f1') */
        var_set_bool(res, do_cmd_fire());
        shoot_hack = SHOOT_NONE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spying_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Spying");
        break;
    default:
        telepathy_spell(cmd, res);
        break;
    }
}

static void _stealthy_snipe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stealthy Snipe");
        break;
    case SPELL_DESC:
        var_set_string(res, "For a short while, your missile attacks will not anger distant monsters.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(6, 6));
        break;
    case SPELL_CAST:
        set_tim_stealthy_snipe(6 + randint1(6), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _whirlwind_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Ambush");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack all adjacent monsters in a single ambush.");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/

static spell_info _spells[] =
{
    /*lvl cst fail spell */
    {  1,  1, 30, _lookout_spell},
    {  5,  2, 30, strafing_spell},
    {  9, 10, 35, _mapping_spell},
    { 13,  8, 40, stone_to_mud_spell},
    { 17, 12, 50, _spying_spell},
    { 21, 30, 50, _dark_stalker_spell},
    { 23, 15, 50, _reconnaissance_spell},
    { 25, 18, 50, _whirlwind_attack_spell},
    { 29, 25, 50, _retreat_spell},
    { 33, 25,  0, _sniping_spell},
    { 35, 40, 55, _nimble_dodge_spell},
    { 37, 24, 45, _cavern_creation_spell},
    { 41, 70, 50, _stealthy_snipe_spell},
    { 45, 60, 70, _greater_mapping_spell},
    { 49, 42, 65, _greater_whirlwind_attack_spell},
    { -1, -1, -1, NULL}
};

static spell_info *_get_spells(void)
{
    if (heavy_armor())
    {
        msg_print("Your talents are disrupted!");
        return NULL;
    }
    return _spells;
}

static bool _cave_is_open(int y, int x)
{
    if (cave_have_flag_bold(y, x, FF_HURT_ROCK)) return FALSE;
    if (cave[y][x].feat == feat_permanent) return FALSE;
    if (cave[y][x].feat == feat_permanent_glass_wall) return FALSE;
    if (cave[y][x].feat == feat_mountain) return FALSE;
    return TRUE;
}

static int _count_open_terrain(void)
{
    int dir, x, y;
    int count = 0;
    for (dir = 0; dir < 8; dir++)
    {
        y = py + ddy_ddd[dir];
        x = px + ddx_ddd[dir];

        if (!in_bounds(y, x))
        {
            /* Count the edge of wilderness maps as open.
               Count the edge of dungeon maps as permanent walls. */
            if (dun_level == 0)
                count++;

            continue;
        }

        if (_cave_is_open(y, x))
            count++;
    }
    return count;
}

static int _prorate_effect(int amt)
{
    int base = (amt + 3) / 4;
    int xtra = amt - base;
    xtra = xtra * (p_ptr->lev/2) / 25;

    return base + xtra;
}

static int _unfettered_body(int ct)
{
    int amt = (ct + 1) * (ct + 1) - 41;
    return _prorate_effect(amt);
}

static int _unfettered_mind(int ct)
{
    int amt = (ct + 1) * (ct + 1)/2 - 20;
    return _prorate_effect(amt);
}

static void _calc_bonuses(void)
{
    int ct = _count_open_terrain();
    bool disrupt = heavy_armor();

    /* Hack: Heavy Armor negates advantages of being in the open, and
       actually incurs penalties for being entrenched! */
    if (disrupt)
        ct = 0;

    p_ptr->open_terrain_ct = ct; /* Nimble Dodge needs this information! */

    /* Unfettered Body */
    if (p_ptr->lev >= 1)
    {
        int amt = _unfettered_body(ct);
        p_ptr->to_a += amt;
        p_ptr->dis_to_a += amt;
    }

    /* Unfettered Mind */
    if (p_ptr->lev >= 1)
    {
        p_ptr->skills.sav += _unfettered_mind(ct);
    }

    if (!disrupt && p_ptr->lev >= 20)
        p_ptr->ambush = TRUE;

    if (!disrupt && p_ptr->lev >= 50)
        p_ptr->peerless_stealth = TRUE;
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
}

static void _character_dump(doc_ptr doc)
{
    int ct = _count_open_terrain();
    bool disrupt = heavy_armor();

    if (!disrupt && p_ptr->lev >= 5)
    {
        py_dump_spells(doc);
    }

    doc_printf(doc, "<topic:Abilities>================================== <color:keypress>A</color>bilities ==================================\n\n");

    /* Hack: Heavy Armor negates advantages of being in the open, and
       actually incurs penalties for being entrenched! */
    if (disrupt)
    {
        doc_printf(doc, "  * Your talents are disrupted by the weight of your armor.\n");
        ct = 0;
    }
    else
    {
        if (ct >= 6)
            doc_printf(doc, "  * You are out in the open (%d adjacent open squares).\n", ct);
        else if (ct >= 3)
            doc_printf(doc, "  * You are somewhat confined (%d adjacent open squares).\n", ct);
        else
            doc_printf(doc, "  * You are very confined (%d adjacent open squares).\n", ct);
    }

    /* Unfettered Body */
    if (p_ptr->lev >= 1)
    {
        int amt = _unfettered_body(ct);
        if (amt > 0)
            doc_printf(doc, "  * You gain %+d to your AC being out in the open.\n", amt);
        else if (amt < 0)
            doc_printf(doc, "  * You lose %+d to your AC being so confined.\n", amt);
    }

    /* Unfettered Mind */
    if (p_ptr->lev >= 1)
    {
        int amt = _unfettered_mind(ct);
        if (amt > 0)
            doc_printf(doc, "  * You gain %+d to your Saving Throws being out in the open.\n", amt);
        else if (amt < 0)
            doc_printf(doc, "  * You lose %+d to your Saving Throws being so confined.\n", amt);
    }

    if (!disrupt && p_ptr->lev >= 20)
        doc_printf(doc, "  * You ambush sleeping monsters for extra damage.\n");

    if (!disrupt && p_ptr->lev >= 50)
        doc_printf(doc, "  * You have Peerless Stealth and will never aggravate monsters.\n");

    doc_newline(doc);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "technique";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 350;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

static void _move_player(void)
{
    p_ptr->update |= PU_BONUS;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(20, 30));
}

class_t *scout_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  33,  34,   6,  50,  24,  50,  65 };
    skills_t xs = { 15,  11,  10,   0,   0,   0,  20,  25 };

        me.name = "Scout";
        me.desc = "The scout is the vanguard of any attack, and excels at stealth and observation "
                    "skills. Scouts are not the best at one-on-one combat, but are unparalleled at "
                    "ambush techniques for destroying groups of weak sentries. Scouts are lightly "
                    "armored, and heavy armor disrupts their abilities. Unlike most characters, "
                    "scouts are not at home in cramped corridors; in open areas they can "
                    "dodge attacks effectively, but being confined severely hampers their "
                    "defensive abilities.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 104;
        me.base_hp = 8;
        me.exp = 130;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.move_player = _move_player;
        me.character_dump = _character_dump;

        init = TRUE;
    }

    return &me;
}
