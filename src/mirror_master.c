#include "angband.h"

static bool _on_mirror = FALSE;

typedef void (*pos_fn)(int y, int x);
static void _for_each_mirror(pos_fn f)
{
    int x, y;
    for (x = 0; x < cur_wid; x++)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            if (is_mirror_grid(&cave[y][x]))
                f(y, x);
        }
    }
}

static int _mirrors_ct(void)
{
    int ct = 0;
    int x, y;
    for (x = 0; x < cur_wid; x++)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            if (is_mirror_grid(&cave[y][x]))
                ct++;
        }
    }
    return ct;
}

static int _mirrors_max(void)
{
    return 4 + p_ptr->lev/10;
}

static bool _mirror_place(void)
{
    if (!cave_clean_bold(py, px))
    {
        msg_print("The object resists the spell.");
        return FALSE;
    }

    cave[py][px].info |= CAVE_OBJECT;
    cave[py][px].mimic = feat_mirror;
    cave[py][px].info |= CAVE_GLOW;
    note_spot(py, px);
    lite_spot(py, px);
    update_local_illumination(py, px);

    return TRUE;
}

static void _banishing_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Banishing Mirror");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "Quickly teleport away a nearby monster.");
        else
            var_set_string(res, "Teleport away a nearby monster.");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_AWAY_ALL, dir, spell_power(p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_ENERGY:
        if (_on_mirror)
        {
            var_set_int(res, 50);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _binding_field_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev*11 + 5 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Binding Field");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a magical triangle which damages all monsters in the area. The vertices of the triangle is you and two mirrors in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
        if (!binding_field(dam))
            msg_print("You were not able to choose suitable mirrors!");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _break_mirrors_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Break Mirrors");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys all mirrors on the current levels. Monsters close to a mirror take damage.");
        break;
    case SPELL_CAST:
        remove_all_mirrors(TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _drip_of_light_spell(int cmd, variant *res)
{
    int  dd = 3 + (p_ptr->lev-1)/5;
    int  ds = 4;
    bool beam = (p_ptr->lev >= 10 && _on_mirror) ? TRUE : FALSE;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Drip of Light");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 10)
            var_set_string(res, "Fires a beam of light or a bolt of light, depending on whether you are standing on a mirror.");
        else
            var_set_string(res, "Fires a bolt of light.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        if (beam)
            fire_beam(GF_LITE, dir,spell_power(damroll(dd, ds) + p_ptr->to_d_spell));
        else
            fire_bolt(GF_LITE, dir,spell_power(damroll(dd, ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _illusion_light_spell(int cmd, variant *res)
{
    int mult = _on_mirror ? 4 : 3;
    int power = spell_power(p_ptr->lev * mult);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Illusion Light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to slow, stun, confuse, scare, freeze all monsters in sight. This is more powerful if you are standing on a mirror.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(power));
        break;
    case SPELL_CAST:
        slow_monsters(power);
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(power);
        turn_monsters(power);
        stasis_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _make_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Make Mirror");
        break;
    case SPELL_DESC:
        var_set_string(res, "Makes a mirror under you.");
        break;
    case SPELL_CAST:
        if (_mirrors_ct() < _mirrors_max())
            _mirror_place();
        else
            msg_print("There are too many mirrors to control!");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_clashing_spell(int cmd, variant *res)
{
    int dd = 8 + (p_ptr->lev - 5)/4;
    int ds = 8;
    int rad = p_ptr->lev > 20 ? spell_power((p_ptr->lev - 20)/8 + 1) : 0;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror Clashing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of shards.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SHARDS, dir, spell_power(damroll(dd, ds) + p_ptr->to_d_spell), rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_concentration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror Concentration");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (total_friends)
        {
            msg_print("You need to focus on your pets.");
            return;
        }
        if (_on_mirror)
        {
            msg_print("You feel your head clear a little.");

            p_ptr->csp += (5 + p_ptr->lev * p_ptr->lev / 100);
            if (p_ptr->csp >= p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }

            p_ptr->redraw |= (PR_MANA);
            var_set_bool(res, TRUE);
        }
        else
        {
            msg_print("You need to stand on a mirror to use this spell!");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror of Light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Lights up nearby area and the inside of a room permanently.");
        break;
    case SPELL_CAST:
        lite_area(damroll(2, p_ptr->lev/2), p_ptr->lev/10 + 1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_recall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror of Recall");
        break;
    case SPELL_DESC:
        var_set_string(res, "Recalls player from dungeon to town, or from town to the deepest level of dungeon.");
        break;
    case SPELL_CAST:
        var_set_bool(res, word_of_recall(TRUE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_ruffnor_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror of Ruffnor");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks or duration time is exceeded.");
        break;
    case SPELL_CAST:
        set_invuln(spell_power(randint1(4) + 4), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


static void _mirror_of_seeing_spell(int cmd, variant *res)
{
    int lvl = p_ptr->lev;

    if (_on_mirror)
        lvl += 4;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror of Seeing");
        break;
    case SPELL_DESC:
        if (lvl >= 39)
            var_set_string(res, "Detects monsters in your vicinity. Grants temporary ESP. Maps nearby area.");
        else if (lvl >= 29)
            var_set_string(res, "Detects monsters in your vicinity. Grants temporary ESP");
        else if (lvl >= 19)
            var_set_string(res, "Detects monsters in your vicinity.");
        else if (lvl >= 5)
            var_set_string(res, "Detects visible monsters in your vicinity.");
        else
            var_set_string(res, "Does nothing since you are too weak. Try standing on a mirror.");
        break;
    case SPELL_CAST:
    {
        if (lvl < 5)
            msg_print("You need a mirror to concentrate!");

        if (lvl >= 5)
            detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (lvl >= 19)
            detect_monsters_invis(DETECT_RAD_DEFAULT);
        if (lvl >= 29)
            set_tim_esp(lvl + randint1(lvl),FALSE);
        if (lvl >= 39)
            map_area(DETECT_RAD_MAP);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_wandering_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror of Wandering");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "Quickly teleport a long distance.");
        else
            var_set_string(res, "Teleport a long distance.");
        break;
    case SPELL_CAST:
        teleport_player(p_ptr->lev*5, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
            break;
        } 
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_shifting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror Shifting");
        break;
    case SPELL_DESC:
        var_set_string(res, "Recreates current dungeon level. Can only be used on a mirror.");
        break;
    case SPELL_CAST:
        if (!_on_mirror)
            msg_print("You cannot find the World of the Mirror!");
        else
            alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_sleeping_fn(int y, int x)
{
    project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev, GF_OLD_SLEEP, 
        PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
}
static void _mirror_sleeping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror Sleeping");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generate balls which send monsters to sleep on all mirrors in the whole level.");
        break;
    case SPELL_CAST:
        _for_each_mirror(_mirror_sleeping_fn);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_tunnel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mirror Tunnel");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "Quickly teleport to a given location.");
        else
            var_set_string(res, "Teleport to a given location.");
        break;
    case SPELL_CAST:
    {
        int x = 0, y = 0;
        var_set_bool(res, FALSE);

        msg_print("You go through the mirror world ...");
        if (!tgt_pt(&x, &y, p_ptr->lev / 2 + 10)) return;
        if (!dimension_door_aux(x, y, p_ptr->lev / 2 + 10))
            msg_print("You fail to pass into the mirror plane correctly!");

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
            break;
        } 
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _multi_shadow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Multishadow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives you a 1 in 3 chance to completely evade an attack.");
        break;
    case SPELL_CAST:
        set_multishadow(6 + randint1(6), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _robe_of_dust_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Robe of Dust");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives aura of shards of mirror for a while, damaging any monster that attacks you in melee.");
        break;
    case SPELL_CAST:
        set_dustrobe(20+randint1(20),FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _seal_of_mirror_fn(int y, int x)
{
    int dam = spell_power(p_ptr->lev*4 + 100);
    if (project_m(0, 0, y, x, dam, GF_GENOCIDE, PROJECT_GRID|PROJECT_ITEM|PROJECT_KILL|PROJECT_JUMP,TRUE))
    {
        if(!cave[y][x].m_idx)
            remove_mirror(y,x);
    }
}
static void _seal_of_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Seal of Mirror");
        break;
    case SPELL_DESC:
        var_set_string(res, "Eliminates a monster on a mirror from current dungeon level.");
        break;
    case SPELL_CAST:
        _for_each_mirror(_seal_of_mirror_fn);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _seeker_ray_spell(int cmd, variant *res)
{
    int dd = 11 + (p_ptr->lev - 5)/4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Seeker Ray");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a beam of mana. If the beam hit a mirror, it breaks that mirror and reflects toward another mirror.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_SEEKER, dir, spell_power(damroll(dd,ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shield_of_water_spell(int cmd, variant *res)
{
    int lvl = p_ptr->lev; /* Boost if _on_mirror? */

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shield of Water");
        break;
    case SPELL_DESC:
        if (lvl >= 40)
            var_set_string(res, "Gives a bonus to AC, reflection and magic resistance.");
        else if (lvl >= 32)
            var_set_string(res, "Gives a bonus to AC and reflection.");
        else
            var_set_string(res, "Gives a bonus to AC.");
        break;
    case SPELL_CAST:
        set_shield(20 + randint1(20), FALSE);
        if (lvl >= 32) 
            set_tim_reflect(20 + randint1(20), FALSE);
        if (lvl >= 40) 
            set_resist_magic(20 + randint1(20),FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _super_ray_spell(int cmd, variant *res)
{
    int dd = 1;
    int ds = p_ptr->lev * 2;
    int b = 150 + p_ptr->to_d_spell;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Super Ray");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a powerful beam of mana. If the beam hit a mirror, it breaks that mirror and fires 8 beams of mana to 8 different directions from that point.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), b));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_SUPER_RAY, dir, spell_power(damroll(dd,ds) + b));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _warped_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Warped Mirror");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "Quickly teleport a short distance.");
        else
            var_set_string(res, "Teleport a short distance.");
        break;
    case SPELL_CAST:
        teleport_player(10, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
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
static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _mirror_of_seeing_spell},
    { 1,   2,  40, _make_mirror_spell},
    { 2,   2,  20, _drip_of_light_spell},
    { 3,   2,  20, _warped_mirror_spell},
    { 5,   3,  35, _mirror_of_light_spell},
    { 6,   5,  35, _mirror_of_wandering_spell},
    {10,   5,  30, _robe_of_dust_spell},
    {12,  12,  30, _banishing_mirror_spell},
    {15,  15,  30, _mirror_clashing_spell},
    {19,  13,  30, _mirror_sleeping_spell},
    {23,  18,  50, _seeker_ray_spell},
    {25,  20,  40, _seal_of_mirror_spell},
    {27,  30,  60, _shield_of_water_spell},
    {29,  30,  60, _super_ray_spell},
    {31,  35,  60, _illusion_light_spell},
    {33,  50,  80, _mirror_shifting_spell},
    {36,  30,  80, _mirror_tunnel_spell},
    {38,  40,  70, _mirror_of_recall_spell},
    {40,  50,  55, _multi_shadow_spell},
    {43,  55,  70, _binding_field_spell},
    {46,  70,  75, _mirror_of_ruffnor_spell},
    { -1, -1,  -1, NULL}
};

static power_info _powers[] =
{
    { A_NONE, { 1, 0,  0, _break_mirrors_spell}}, 
    { A_INT,  {30, 0, 50, _mirror_concentration_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};
static spell_info *_get_spells(void)
{
    _on_mirror = is_mirror_grid(&cave[py][px]);
    return _spells;
}

static power_info *_get_powers(void)
{    
    _on_mirror = is_mirror_grid(&cave[py][px]);
    return _powers;
}

static void _character_dump(doc_ptr doc)
{
    spellbook_character_dump(doc);
    doc_insert(doc, "<color:r>Realm:</color> <color:B>Mirror Magic</color>\n");
    py_dump_spells_aux(doc);
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 40) 
        p_ptr->reflect = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if(p_ptr->lev >= 40)
        add_flag(flgs, OF_REFLECT);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);

        if (b <= 50)
        {
        }
        else if (b <= 80)
        {
            msg_print("Weird visions seem to dance before your eyes...");
            teleport_player(10, TELEPORT_PASSIVE);
        }
        else if (b <= 95)
        {
            msg_print("Your brain is addled!");
            set_image(p_ptr->image + 5 + randint1(10), FALSE);
        }
        else
        {
            msg_print("Your mind unleashes its power in an uncontrollable storm!");

            project(PROJECT_WHO_UNCTRL_POWER, 2 + p_ptr->lev / 10, py, px, p_ptr->lev * 2,
                GF_MANA, PROJECT_JUMP | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM);
            p_ptr->csp = MAX(0, p_ptr->csp - p_ptr->lev * MAX(1, p_ptr->lev / 10));
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "mirror magic";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(2, 5));
}

class_t *mirror_master_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  33,  40,   3,  14,  16,  34,  30 };
    skills_t xs = { 10,  11,  12,   0,   0,   0,   6,  10 };

        me.name = "Mirror-Master";
        me.desc = "Mirror-Masters are spellcasters; like mages, they must live "
                    "by their wits. They can create magical mirrors, which they use to "
                    "support their special Mirror-Magic spells. Intelligence determines a "
                    "Mirror-Master's spellcasting ability.\n \n"
                    "Mirror-Masters gain more spells with experience, and their spells also "
                    "become more powerful over time. Much of their magic relies on the "
                    "careful placement of mirrors, which can be used for both offense "
                    "and defense. Mirror-Masters' abilities are also enhanced by standing on a mirror; "
                    "for example, they can perform quick teleports from there or rapidly regenerate their mana. "
                    "Nevertheless, not all Mirror-Magic requires a physical mirror; many effects "
                    "rely simply on light or mirror-shards, or on uncertainty, reflection and illusion. "
                    "The maximum number of Mirrors which can be controlled simultaneously depends "
                    "on the player's level; sometimes, unnecessary mirrors may need to be broken.";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.get_powers_fn = _get_powers;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}

bool is_mirror_grid(cave_type *c_ptr)
{
    if ((c_ptr->info & CAVE_OBJECT) && have_flag(f_info[c_ptr->mimic].flags, FF_MIRROR))
        return TRUE;
    else
        return FALSE;
}

void remove_mirror(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    c_ptr->info &= ~(CAVE_OBJECT);
    c_ptr->mimic = 0;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS)
    {
        c_ptr->info &= ~(CAVE_GLOW);
        if (!view_torch_grids) c_ptr->info &= ~(CAVE_MARK);
        if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);
        update_local_illumination(y, x);
    }

    note_spot(y, x);
    lite_spot(y, x);
}

static void _explode_fn(int y, int x)
{
    remove_mirror(y, x);
    project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS,
            PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
}
void remove_all_mirrors(bool explode)
{
    if (explode)
        _for_each_mirror(_explode_fn);
    else
        _for_each_mirror(remove_mirror);
}
