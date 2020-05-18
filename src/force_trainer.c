#include "angband.h"

static int _force_boost(void) { return p_ptr->magic_num1[0]; }

static void _small_force_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Small Force Ball");
        break;
    case SPELL_DESC:
    {        
        var_set_string(res, "Fires a very small energy ball.");
        break;
    }
    case SPELL_INFO:
    {
        int dice = 3 + ((p_ptr->lev - 1) / 5) + _force_boost()/ 12;
        int sides = 4;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 3 + ((p_ptr->lev - 1) / 5) + _force_boost()/ 12;
            int sides = 4;
            fire_ball(
                GF_MISSILE,
                dir,
                spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                0
            );
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _flying_technique_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Flying Technique");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives levitation a while.");
        break;
    case SPELL_CAST:
    {
        set_tim_levitation(spell_power(randint1(30) + 30 + _force_boost() / 5), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _kamehameha_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Kamehameha");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a short energy beam.");
        break;
    case SPELL_INFO:
    {
        int dice = 5 + ((p_ptr->lev - 1) / 5) + _force_boost() / 10;
        int sides = 5;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        project_length = p_ptr->lev / 8 + 3;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 5 + ((p_ptr->lev - 1) / 5) + _force_boost() / 10;
            int sides = 5;
            fire_beam(
                GF_MISSILE,
                dir,
                spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
            );
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _magic_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Magic Resistance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives magic resistance for a while.");
        break;
    case SPELL_CAST:
    {
        int dur = randint1(20) + 20 + _force_boost() / 5;
        set_resist_magic(spell_power(dur), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _improve_force_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Improve Force");
        break;
    case SPELL_DESC:
        var_set_string(res, "Improves spirit energy power temporarily. Improved spirit energy will be more and more powerful or have longer duration. Too many improving results in uncontrollable explosion of spirit energy.");
        break;
    case SPELL_CAST:
    {
        msg_print("You improved the Force.");
        p_ptr->magic_num1[0] += (70 + p_ptr->lev);
        p_ptr->update |= (PU_BONUS);
        if (randint1(p_ptr->magic_num1[0]) > (p_ptr->lev * 4 + 120))
        {
            msg_print("The Force exploded!");
            fire_ball(GF_MANA, 0, p_ptr->magic_num1[0] / 2, 10);
            take_hit(DAMAGE_LOSELIFE, p_ptr->magic_num1[0] / 2, "Explosion of the Force");
            p_ptr->magic_num1[0] = 0;
            p_ptr->update |= (PU_BONUS);
            var_set_bool(res, FALSE); /* no energy consumed?? */
        }
        else var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        int j;
        for (j = 0; j < p_ptr->magic_num1[0] / 50; j++)
            n += (j+1) * 3 / 2;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _aura_of_force_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Aura of Force");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives aura which damages all monsters which attacked you for a while.");
        break;
    case SPELL_CAST:
    {
        set_tim_sh_touki(spell_power(randint1(p_ptr->lev / 2) + 15 + _force_boost() / 7), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shock_power_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shock Power");
        break;
    case SPELL_DESC:
        var_set_string(res, "Damages an adjacent monster, and blow it away.");
        break;
    case SPELL_INFO:
    {
        int dice = 8 + ((p_ptr->lev - 5) / 4) + _force_boost() / 12;
        int sides = 8;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int y, x, dam, dir;
        project_length = 1;
        /*if (!get_fire_dir(&dir)) XXX blow away won't work if a target is chosen ... dir == 5 */
        if (!get_rep_dir2(&dir))
        { 
            var_set_bool(res, FALSE);
            return;
        }

        y = py + ddy[dir];
        x = px + ddx[dir];
        dam = spell_power(damroll(8 + ((p_ptr->lev - 5) / 4) + _force_boost() / 12, 8) + p_ptr->to_d_spell);
        fire_beam(GF_MISSILE, dir, dam);
        if (cave[y][x].m_idx)
        {
            int i;
            int ty = y, tx = x;
            int oy = y, ox = x;
            int m_idx = cave[y][x].m_idx;
            monster_type *m_ptr = &m_list[m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];
            char m_name[80];

            monster_desc(m_name, m_ptr, 0);

            if (randint1(r_ptr->level * 3 / 2) > randint0(dam / 2) + dam/2)
            {
                msg_format("%^s was not blown away.", m_name);
            }
            else
            {
                for (i = 0; i < 5; i++)
                {
                    y += ddy[dir];
                    x += ddx[dir];
                    if (cave_empty_bold(y, x))
                    {
                        ty = y;
                        tx = x;
                    }
                    else break;
                }
                if ((ty != oy) || (tx != ox))
                {
                    msg_format("You blow %s away!", m_name);

                    cave[oy][ox].m_idx = 0;
                    cave[ty][tx].m_idx = m_idx;
                    m_ptr->fy = ty;
                    m_ptr->fx = tx;

                    update_mon(m_idx, TRUE);
                    lite_spot(oy, ox);
                    lite_spot(ty, tx);

                    if (r_ptr->flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                        p_ptr->update |= (PU_MON_LITE);
                }
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

static void _large_force_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Large Force Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a large energy ball.");
        break;
    case SPELL_INFO:
    {
        int dice = spell_power(10);
        int sides = 6;
        int base = spell_power(p_ptr->lev * 3 / 2 + _force_boost() * 3 / 5 + p_ptr->to_d_spell);
        var_set_string(res, info_damage(dice, sides, base));
        break;
    }
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 10;
            int sides = 6;
            int base = p_ptr->lev * 3 / 2 + _force_boost() * 3 / 5;
            int radius = spell_power((p_ptr->lev < 30) ? 2 : 3);
            int dam = spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell);
            fire_ball(GF_MISSILE, dir, dam, radius);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_ghost_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Ghost");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summons ghosts.");
        break;
    case SPELL_CAST:
    {
        int i;
        bool success = FALSE;

        for (i = 0; i < 1 + _force_boost()/100; i++)
            if (summon_specific(-1, py, px, p_ptr->lev, SUMMON_PHANTOM, PM_FORCE_PET))
                success = TRUE;
        if (success)
            msg_print("'Your wish, master?'");
        else
            msg_print("Nothing happen.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _exploding_flame_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Exploding Flame");
        break;
    case SPELL_DESC:
        var_set_string(res, "Generates a huge ball of flame centered on you.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(100 + p_ptr->lev + _force_boost() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        fire_ball(
            GF_FIRE,
            0,
            spell_power((100 +  p_ptr->lev + _force_boost() + p_ptr->to_d_spell) * 2),
            10
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _super_kamehameha_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Super Kamehameha");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a long, powerful energy beam.");
        break;
    case SPELL_INFO:
    {
        int dice = spell_power(10 + p_ptr->lev/2 + _force_boost()*3/10);
        int sides = 15;
        var_set_string(res, info_damage(dice, sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir;
        int dice = 10 + p_ptr->lev/2 + _force_boost()*3/10;
        int sides = 15;
        
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_beam(
            GF_MANA,
            dir,
            spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _light_speed_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Light Speed");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives extremely fast speed.");
        break;
    case SPELL_CAST:
    {
        set_lightspeed(spell_power(randint1(16) + 16 + _force_boost() / 20), FALSE);
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

#define MAX_FORCETRAINER_SPELLS    15

static spell_info _spells[MAX_FORCETRAINER_SPELLS] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _small_force_ball_spell},
    { 3,   3,  30, light_area_spell},
    { 5,   6,  35, _flying_technique_spell},
    { 8,   5,  40, _kamehameha_spell},
    { 10,  7,  45, _magic_resistance_spell},
    { 13,  5,  60, _improve_force_spell},
    { 17, 17,  50, _aura_of_force_spell},
    { 20, 20,  50, _shock_power_spell},
    { 23, 18,  55, _large_force_ball_spell},
    { 25, 30,  70, dispel_magic_spell},
    { 28, 26,  50, _summon_ghost_spell},
    { 32, 35,  65, _exploding_flame_spell},
    { 38, 42,  75, _super_kamehameha_spell},
    { 44, 50,  80, _light_speed_spell},
    { -1, -1,  -1, NULL},
};

static spell_info *_get_spells(void)
{
    int i, hand;
//    int ct = 0;
    int stat_idx = p_ptr->stat_ind[A_WIS];
    int penalty1 = 0;
    int penalty2 = 0;
    static spell_info spells[MAX_FORCETRAINER_SPELLS];

    /* These penalties should only apply to Force spells ... at the moment, choice
       of a conventional spellbook and realm is handled elsewhere, but should some
       day be moved here. */
    if (heavy_armor()) 
    {
        penalty1 += 20;
        penalty2 += 5;
    }
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].icky_wield) 
        {
            penalty1 += 20;
            penalty2 += 5;
        }
        else if ( p_ptr->weapon_info[hand].wield_how != WIELD_NONE
              && !p_ptr->weapon_info[hand].bare_hands )
        {
            penalty1 += 10;
        }
    }
    for (i = 0; i < MAX_FORCETRAINER_SPELLS; i++)
    {
        spell_info *base = &_spells[i];
        spells[i].fn = base->fn;
        spells[i].level = base->level;
        spells[i].cost = base->cost;
        if (!base->fn) 
        {
            spells[i].fail = 0;
            continue;
        }
        /* The first penalty can be overcome... */ 
        spells[i].fail = calculate_fail_rate(base->level, base->fail + penalty1, stat_idx);            
        /* ...but the second penalty is just added */
        spells[i].fail += penalty2;
        if (spells[i].fail > 95) spells[i].fail = 95;
    }
    return spells;
}

static power_info _get_powers[] =
{
    { A_WIS, {15, 0, 30, clear_mind_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    p_ptr->monk_lvl = (p_ptr->lev * 94 + 50) / 100;
    if (p_ptr->lev >= 15) 
        p_ptr->clear_mind = TRUE;

    if (!(heavy_armor()))
    {
        p_ptr->pspeed += (p_ptr->lev) / 10;
        p_ptr->sh_retaliation = TRUE;
        if  (p_ptr->lev >= 25)
            p_ptr->free_act++;

    }
    monk_ac_bonus();
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (!heavy_armor())
    {
        add_flag(flgs, OF_AURA_REVENGE);
        if (p_ptr->lev >= 10)
            add_flag(flgs, OF_SPEED);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_FREE_ACT);
    }
}

static void _on_fail(const spell_info *spell)
{
    /* reset force counter for all spells *except* Improve Force */
    if (spell->fn != _improve_force_spell && p_ptr->magic_num1[0])
    {
        msg_print("Your improved Force has gone away...");
        p_ptr->magic_num1[0] = 0;
        p_ptr->update |= (PU_BONUS);
    }
}

static void _on_cast(const spell_info *spell)
{
    /* reset force counter for all spells *except* Improve Force */
    if (spell->fn != _improve_force_spell && p_ptr->magic_num1[0])
    {
        p_ptr->magic_num1[0] = 0;
        p_ptr->update |= (PU_BONUS);
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "force";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 350;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        me.on_cast = _on_cast;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CLARITY, rand_range(5, 10));
    py_birth_spellbooks();
}

static void _character_dump(doc_ptr doc)
{
    spellbook_character_dump(doc);

    doc_insert(doc, "<color:r>Realm:</color> <color:B>Force</color>\n");
    py_dump_spells_aux(doc);
}

class_t *force_trainer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  34,  38,   4,  32,  24,  50,  40 };
    skills_t xs = { 10,  11,  11,   0,   0,   0,  14,  15 };

        me.name = "Force-Trainer";
        me.desc = "A Force-Trainer is a master of the spiritual Force. They prefer "
                    "fighting with neither weapon nor armor. They are not as good "
                    "fighters as are Monks, but they can use both magic and the "
                    "spiritual Force. Wielding weapons or wearing heavy armor disturbs "
                    "use of the Force. Wisdom is a Force-Trainer's primary stat.\n \n"
                    "Force-Trainers use both spellbook magic and the special spiritual "
                    "power called the Force. They can select a realm from Life, "
                    "Nature, Craft, Death, and Crusade. To use The Force, you select "
                    "it just as if it were spellbook 'F'; which means you need to press "
                    "'m' and then 'F' to select the Force. The most important spell of "
                    "the Force is 'Improve Force'; each time a Force-Trainer activates "
                    "it, their Force power becomes more powerful, and their attack "
                    "power in bare-handed melee fighting is increased temporarily. The "
                    "strengthened Force can be released at one stroke when a "
                    "Force-Trainer activates some other Force spell, typically an attack "
                    "spell. They have a class power - 'Clear Mind' - which allows them "
                    "to rapidly regenerate their mana.";
        
        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 135;
        me.pets = 40;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        me.known_icky_object = skills_obj_is_icky_weapon;
        init = TRUE;
    }

    return &me;
}
