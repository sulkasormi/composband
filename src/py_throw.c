#include "angband.h"

#include <assert.h>

/************************************************************************
 * Throw Object
 ***********************************************************************/
static void    _animate(py_throw_ptr context);
static bool    _hit_mon(py_throw_ptr context, int m_idx);
static bool    _hit_wall(py_throw_ptr context);
static obj_ptr _get_obj(int type);
static bool    _get_target(py_throw_ptr context);
static bool    _init_context(py_throw_ptr context);
static void    _throw(py_throw_ptr context);
static void    _return(py_throw_ptr context);

bool py_throw(py_throw_ptr context)
{
    if (!_init_context(context)) return FALSE;
    _throw(context);
    _return(context);
    water_mana_action(FALSE, 5);
    if (energy_use > 0) p_inc_fatigue(MUT_EASY_TIRING, energy_use);
    return TRUE;
}

void _animate(py_throw_ptr context)
{
    int x, y;
    int msec = delay_time();

    y = GRID_Y(context->path[context->path_pos]);
    x = GRID_X(context->path[context->path_pos]);
    if (panel_contains(y, x) && player_can_see_bold(y, x))
    {
        char c = object_char(context->obj);
        byte a = object_attr(context->obj);

        /* Draw, Hilite, Fresh, Pause, Erase */
        print_rel(c, a, y, x);
        move_cursor_relative(y, x);
        Term_fresh();
        Term_xtra(TERM_XTRA_DELAY, msec);
        lite_spot(y, x);
        Term_fresh();
    }
    else
    {
        /* Pause anyway, for consistancy */
        Term_xtra(TERM_XTRA_DELAY, msec);
    }
}

static obj_ptr _get_obj(int type)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "Throw which item?";
    prompt.error = "You have nothing to throw.";
    if (type & THROW_BOOMERANG)
    {
        slot_t slot = equip_find_first(object_is_melee_weapon);
        if (slot && ((type & THROW_DISPLAY) || p_ptr->weapon_ct == 1))
            return equip_obj(slot);
        prompt.where[0] = INV_EQUIP;
        prompt.filter = object_is_melee_weapon;
    }
    else
    {
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_EQUIP;
        prompt.where[2] = INV_QUIVER;
        prompt.where[3] = INV_FLOOR;
    }
    obj_prompt(&prompt);
    return prompt.obj;
}

bool _get_target(py_throw_ptr context)
{
    int tx, ty;
    if (context->dir == 5 && !target_okay())
        context->dir = 0;
    if (!context->dir)
    {
        project_length = context->range;
        if (!get_fire_dir(&context->dir)) return FALSE;
        project_length = 0;
    }
    if (context->dir == DIR_RANDOM) /* Shuriken Spreading (ninja.c) */
    {
        tx = randint0(101)-50+px;
        ty = randint0(101)-50+py;
    }
    else if (context->dir == DIR_TARGET && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }
    else
    {
        tx = px + 99 * ddx[context->dir];
        ty = py + 99 * ddy[context->dir];
    }
    if (tx == px && ty == py) return FALSE;

    assert(context->range <= MAX_SIGHT);
    context->path_ct = project_path(context->path,
        context->range, py, px, ty, tx, PROJECT_PATH);
    context->path_pos = 0;
    if (!context->path_ct) return FALSE;
    return TRUE;
}

bool _init_context(py_throw_ptr context)
{
    /* object */
    if (!context->obj)
    {
        context->obj = _get_obj(context->type);
        if (!context->obj) return FALSE;
        if ( context->obj->loc.where == INV_EQUIP
          && context->obj->tval == TV_QUIVER
          && quiver_count(NULL) )
        {
            msg_print("Your quiver still holds ammo. Remove all the ammo from your quiver first.");
            return FALSE;
        }
    }
    obj_flags(context->obj, context->flags);
    object_desc(context->obj_name, context->obj, OD_NAME_ONLY | OD_OMIT_PREFIX | OD_NO_PLURAL | OD_OMIT_INSCRIPTION);

    /* checks before taking a turn */
    if (p_ptr->inside_arena && !(context->type & THROW_BOOMERANG))
    {
        if ((context->obj->tval != TV_SPIKE) || (!player_is_ninja))
        {
            msg_print("You're in the arena now. This is hand-to-hand!");
            return FALSE;
        }
    }

    /* energy and take a turn */
    if (!(context->type & THROW_DISPLAY))
    {
        if (!context->energy)
        {
            context->energy = 100;
            if (p_ptr->pclass == CLASS_ROGUE || p_ptr->pclass == CLASS_NINJA)
                context->energy -= p_ptr->lev;
            if (p_ptr->pclass == CLASS_NINJA_LAWYER)
                context->energy -= (p_ptr->lev * 2 / 3);
        }
        energy_use = context->energy;
        if (!fear_allow_shoot())
        {
            msg_print("You are too scared!");
            return FALSE;
        }
        if (context->obj->loc.where == INV_EQUIP && !equip_can_takeoff(context->obj))
            return FALSE;
    }

    /* multiplier (required for range calc) */
    if (!context->mult)
    {
        context->mult = 100;
        if (p_ptr->mighty_throw)
            context->mult += 100;
        if (have_flag(context->flags, OF_THROWING))
            context->mult += 100;
        context->mult = context->mult * (100 + adj_str_td[p_ptr->stat_ind[A_STR]] - 128) / 100;
    }

    /* range */
    if (!context->range)
    {
        int mul, div, rng;

        mul = 10 + 2 * (context->mult - 100) / 100;
        div = context->obj->weight > 10 ? context->obj->weight : 10;
        if (have_flag(context->flags, OF_THROWING) || (context->type & THROW_BOOMERANG))
            div /= 2;

        rng = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;
        if (rng > mul) rng = mul;
        if (rng < 5) rng = 5;

        context->range = rng;
    }
    context->range = MIN(MAX_RANGE, context->range);

    /* target */
    if (!(context->type & THROW_DISPLAY))
    {
        if (!_get_target(context)) { energy_use = 0; return FALSE; }
    }

    /* boomerang? */
    context->come_back = FALSE;
    context->fail_catch = FALSE;
    if (context->obj->name1 == ART_MJOLLNIR || context->obj->name1 == ART_AEGISFANG)
    {
        context->type |= THROW_BOOMERANG;
        context->back_chance += 20; /* upgrade the odds if already Boomerang */
    }
    if (context->type & THROW_BOOMERANG)
    {
        context->back_chance += adj_dex_th[p_ptr->stat_ind[A_DEX]] - 128;
        if (!(context->type & THROW_DISPLAY))
        {
            int oops = 100;
            context->back_chance += randint1(30);
            if (context->back_chance > 30 && !one_in_(oops))
            {
                context->come_back = TRUE;
                if (p_ptr->blind || p_ptr->image || p_ptr->confused || one_in_(oops))
                    context->fail_catch = TRUE;
                else
                {
                    oops = 37;
                    if (p_ptr->stun)
                        oops += 10;
                    if (context->back_chance <= oops)
                        context->fail_catch = TRUE;
                }
            }
        }
    }

    /* skill and some ninja hacks (hengband) */
    context->skill += p_ptr->skill_tht + (p_ptr->shooter_info.to_h + context->obj->to_h) * BTH_PLUS_ADJ;
    if (player_is_ninja)
    {
        if ( context->obj->tval == TV_SPIKE
          || (context->obj->tval == TV_SWORD && have_flag(context->flags, OF_THROWING)) )
        {
            context->skill *= 2; /* wow! */
            context->to_d += ((p_ptr->lev+30)*(p_ptr->lev+30)-900)/55; /* +100 at CL50 */
        }
    }
    if (p_ptr->stun)
        context->skill -= context->skill * MIN(100, p_ptr->stun) / 150;

    return TRUE;
}

bool _hit_mon(py_throw_ptr context, int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int           ac = mon_ac(m_ptr);
    bool          visible = m_ptr->ml;

    if (test_hit_fire(context->skill - (context->path_pos + 1), ac, m_ptr->ml))
    {
        bool fear = FALSE;
        bool ambush = MON_CSLEEP(m_ptr) && visible && p_ptr->ambush;
        int  tdam;

        if (!visible)
            msg_format("The %s finds a mark.", context->obj_name);
        else
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            if (ambush)
                cmsg_format(TERM_RED, "The %s cruelly hits %s.", context->obj_name, m_name);
            else
                msg_format("The %s hits %s.", context->obj_name, m_name);
            if (!p_ptr->image) mon_track(m_ptr);
            health_track(m_idx);
        }

        /***** The Damage Calculation!!! *****/
        tdam = damroll(context->obj->dd + context->to_dd, context->obj->ds);
        if (context->obj->tval == TV_CORPSE) tdam /= 10; /* Igor innate attack dice **/
        tdam = tot_dam_aux(context->obj, tdam, m_ptr, 0, 0, TRUE);
        if (have_flag(context->flags, OF_VORPAL) || have_flag(context->flags, OF_VORPAL2))
        {
            int  vorpal_chance = have_flag(context->flags, OF_VORPAL2) ? 2 : 4;
            if (one_in_(vorpal_chance * 3 / 2))
            {
                int mult = 2;
                char m_name[80];

                while (one_in_(vorpal_chance))
                    mult++;

                tdam *= mult;

                monster_desc(m_name, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
                switch (mult)
                {
                case 2: msg_format("Your weapon <color:U>gouges</color> %s!", m_name); break;
                case 3: msg_format("Your weapon <color:y>maims</color> %s!!", m_name); break;
                case 4: msg_format("Your weapon <color:R>carves</color> %s!!!", m_name); break;
                case 5: msg_format("Your weapon <color:r>cleaves</color> %s!!!!", m_name); break;
                case 6: msg_format("Your weapon <color:v>smites</color> %s!!!!!", m_name); break;
                case 7: msg_format("Your weapon <color:v>eviscerates</color> %s!!!!!!", m_name); break;
                default: msg_format("Your weapon <color:v>shreds</color> %s!!!!!!!", m_name); break;
                }

                if (have_flag(context->flags, OF_VORPAL2))
                    obj_learn_slay(context->obj, OF_VORPAL2, "is <color:v>*Sharp*</color>");
                else
                    obj_learn_slay(context->obj, OF_VORPAL, "is <color:R>Sharp</color>");
            }
        }
        if (!have_flag(context->flags, OF_BRAND_ORDER))
        {
            critical_t crit = critical_throw(context->obj->weight, context->obj->to_h);
            if (crit.desc)
            {
                tdam = tdam * crit.mul/100 + crit.to_d;
                msg_print(crit.desc);
            }
        }
        tdam += context->obj->to_d;
        tdam = tdam * context->mult / 100;
        if (ambush)
            tdam *= 2;
        tdam += context->to_d;

        if (context->mod_damage_f)
            tdam = context->mod_damage_f(context, tdam);

        if (p_ptr->stun)
            tdam -= tdam * MIN(100, p_ptr->stun) / 150;

        if (tdam < 0) tdam = 0;
        tdam = mon_damage_mod(m_ptr, tdam, FALSE);
        context->dam = tdam;
        if (mon_take_hit(m_idx, tdam, DAM_TYPE_ARCHERY, &fear, extract_note_dies(real_r_ptr(m_ptr))))
        {
            /* Dead monster */
        }
        else
        {
            if (have_flag(context->flags, OF_BRAND_VAMP))
            {
                char m_name[80];
                int  heal = MIN(30, damroll(3, tdam / 8));
                monster_desc(m_name, m_ptr, MD_PRON_VISIBLE | MD_OBJECTIVE);
                msg_format("Your weapon drains life from %s!", m_name);
                hp_player_aux(heal);
                obj_learn_slay(context->obj, OF_BRAND_VAMP, "is <color:D>Vampiric</color>");
            }
            message_pain(m_idx, tdam);
            if (tdam > 0)
                anger_monster(m_ptr);

            if (tdam > 0 && m_ptr->cdis > 1 && allow_ticked_off(r_ptr))
            {
                if (!mut_present(MUT_PEERLESS_SNIPER))
                    mon_anger_shoot(m_ptr, tdam);
            }

            if (fear && m_ptr->ml)
            {
                char m_name[80];
                sound(SOUND_FLEE);
                monster_desc(m_name, m_ptr, MD_PRON_VISIBLE);
                msg_format("%^s flees in terror!", m_name);
            }
            if (context->after_hit_f)
                context->after_hit_f(context, m_idx);
        }
        if (!(context->type & THROW_BOOMERANG))
            context->break_chance = breakage_chance(context->obj);
    }
    return TRUE;
}

bool _hit_wall(py_throw_ptr context)
{
    if (!(context->type & THROW_BOOMERANG))
        context->break_chance = breakage_chance(context->obj);
    return TRUE;
}

void _throw(py_throw_ptr context)
{
    assert(context->path_ct);
    for (context->path_pos = 0; ; context->path_pos++)
    {
        int x = GRID_X(context->path[context->path_pos]);
        int y = GRID_Y(context->path[context->path_pos]);

        _animate(context);

        if (cave[y][x].m_idx)
        {
            if (_hit_mon(context, cave[y][x].m_idx)) break;
        }

        if (!cave_have_flag_bold(y, x, FF_PROJECT))
        {
            if (_hit_wall(context)) 
            {
                /* Drop on the previous square to avoid objects flying through walls.
                 * It's possible the previous square isn't actually part of the path,
                 * in which case we need to improvise */
                if (!context->path_pos) context->path[0] = GRID(py, px);
                else context->path_pos--;
                break;
            }
        }

        /* careful ... leave path_pos on the last processed square
         * so we can drop the object if needed */
        if (context->path_pos == context->path_ct - 1) break;
    }
    if ((player_is_ninja) && (context->obj->tval == TV_SPIKE))
        stats_on_use(context->obj, 1);
}

void _return(py_throw_ptr context)
{
    /* animation for the return */
    if (context->come_back)
    {
        for (; context->path_pos >= 0; context->path_pos--)
            _animate(context);
        msg_format("Your %s comes back to you.", context->obj_name);
        if (object_is_(context->obj, TV_POLEARM, SV_DEATH_SCYTHE) && (one_in_(2) || context->fail_catch))
            death_scythe_miss(context->obj, HAND_NONE, 0);
    }

    /* handle fumbling, dropping and catching */
    if (!context->come_back || context->fail_catch)
    {
        /* only message/warn if the user expects a return */
        if (context->type & THROW_BOOMERANG)
        {
            if (!context->come_back)
                msg_format("Your %s fails to return!", context->obj_name);

            if (context->fail_catch)
                cmsg_print(TERM_VIOLET, "But you can't catch!");

            if (context->obj->loc.where == INV_EQUIP)
            {
                msg_print("Press <color:y>Space</color> to continue.");
                flush();
                for (;;)
                {
                    char ch = inkey();
                    if (ch == ' ') break;
                }
                msg_line_clear();
            }
        }
        if (!context->come_back)
        {
            int x,y;
            y = GRID_Y(context->path[context->path_pos]);
            x = GRID_X(context->path[context->path_pos]);
            /* Figurines transform into pets (enemies if cursed) */
            if (context->obj->tval == TV_FIGURINE && !p_ptr->inside_arena)
            {
                if (!summon_named_creature(0, y, x, context->obj->pval,
                                !object_is_cursed(context->obj) ? PM_FORCE_PET : 0))
                    msg_print("The Figurine writhes and then shatters.");

                else if (object_is_cursed(context->obj))
                    msg_print("You have a bad feeling about this.");

                context->obj->number--;
                obj_release(context->obj, OBJ_RELEASE_QUIET);
            }
            /* potions shatter with effects if they hit a wall or monster */
            else if (context->obj->tval == TV_POTION && randint0(100) < context->break_chance)
            {
                msg_format("The %s shatters!", context->obj_name);
                obj_dec_number(context->obj, 1, TRUE); /* Do this first - otherwise the effect could potentially re-shatter the same object again... */
                if (potion_smash_effect(0, y, x, context->obj->k_idx))
                {
                    /* I think this needs fixing in project_m ... */
                    if (cave[y][x].m_idx)
                    {
                        monster_type *m_ptr = &m_list[cave[y][x].m_idx];
                        if (is_friendly(m_ptr) && !MON_INVULNER(m_ptr))
                        {
                            char m_name[80];
                            monster_desc(m_name, m_ptr, 0);
                            msg_format("%^s gets angry!", m_name);
                            set_hostile(m_ptr);
                        }
                    }
                }
                obj_release(context->obj, OBJ_RELEASE_QUIET);
            }
            /* everything else drops (perhaps breaks) at the end of the path */
            else
                obj_drop_at(context->obj, 1, x, y, context->break_chance);
        }
        else
            obj_drop_at(context->obj, 1, px, py, context->break_chance);
        context->obj = NULL;
    }
    else
    {
        /* The thrown object returned and the player caught it.
         * This means it stays equipped (or, rarely, in the pack)
         * so no change is required. */
    }
}

/************************************************************************
 * Document Throw Weapon for Character Sheet
 ***********************************************************************/
static void _display_weapon_slay(int base_mult, int slay_mult, bool force, int throw_mult, int num_throw,
                                 int dd, int ds, int to_d, cptr name, int color, doc_ptr doc)
{
    int mult, dam;

    mult = slay_mult;
    if (force)
        mult = mult * 3/2 + 140;
    mult = mult * base_mult / 100;

    dam = mult * dd * (ds + 1)/200 + to_d;
    dam = throw_mult * dam / 100;
    if (p_ptr->stun)
        dam -= dam * MIN(100, p_ptr->stun) / 150;

    doc_printf(doc, "<color:%c> %-7.7s</color>", attr_to_attr_char(color), format("%^s", name));
    doc_printf(doc, ": %d/%d [%d.%02dx]\n",
                    dam, num_throw * dam / 100,
                    mult/100, mult%100);
}

void py_throw_doc(py_throw_ptr context, doc_ptr doc)
{
    int to_d = 0;
    int to_h = 0;
    int mult = 100;
    critical_t crit = {0};
    int crit_pct = 0;
    int num_throw = 100;
    bool force = FALSE;
    int i = 0;
    slay_type _slay;
    doc_ptr cols[2] = {0};

    _init_context(context);
    if (!context->obj) return;
    if (context->energy)
        num_throw = 10000 / context->energy;

    if (object_is_known(context->obj))
    {
        to_d = context->obj->to_d;
        to_h = context->obj->to_h;
    }

    obj_flags_known(context->obj, context->flags);
    if (have_flag(context->flags, OF_BRAND_MANA) || p_ptr->tim_force)
    {
        int cost = 1 + context->obj->dd * context->obj->ds / 7;
        if (p_ptr->csp >= cost)
            force = TRUE;
    }

    if (have_flag(context->flags, OF_VORPAL2))
        mult = mult * 5 / 3;  /* 1 + 1/3(1 + 1/2 + ...) = 1.667x */
    else if (have_flag(context->flags, OF_VORPAL))
        mult = mult * 11 / 9; /* 1 + 1/6(1 + 1/4 + ...) = 1.222x */

    if (!have_flag(context->flags, OF_BRAND_ORDER))
    {
        const int attempts = 10 * 1000;
        int i;
        int crits = 0;
        /* Compute Average Effects of Criticals by sampling */
        for (i = 0; i < attempts; i++)
        {
            critical_t tmp = critical_throw(context->obj->weight, context->obj->to_h);
            if (tmp.desc)
            {
                crit.mul += tmp.mul;
                crit.to_d += tmp.to_d;
                crits++;
            }
            else
                crit.mul += 100;
        }
        crit.mul = crit.mul / attempts;
        crit.to_d = crit.to_d * 100 / attempts;
        crit_pct = crits * 1000 / attempts;
    }
    else
        crit.mul = 100;


    /* Display in 2 columns, side by side */
    cols[0] = doc_alloc(60);
    cols[1] = doc_alloc(10);

    /* Column #1 */
    object_desc(context->obj_name, context->obj, OD_COLOR_CODED | OD_NAME_AND_ENCHANT | OD_THROWING);
    doc_printf(cols[0], "<color:y> Throwing:</color> <indent><style:indent>%s</style></indent>\n", context->obj_name);

    doc_printf(cols[0], " %-7.7s: %d.%d lbs\n", "Weight", context->obj->weight/10, context->obj->weight%10);
    doc_printf(cols[0], " %-7.7s: %d + %d = %d\n", "To Hit", to_h, p_ptr->shooter_info.dis_to_h, to_h + p_ptr->shooter_info.dis_to_h);

    doc_printf(cols[0], " %-7.7s: %d\n", "Range", context->range);
    if (p_ptr->wizard && 0)
        doc_printf(cols[0], " %-7.7s: %d (31 to return; 38 to catch)\n", "Back", context->back_chance);
    doc_printf(cols[0], " %-7.7s: %d%%\n", "Return", 99*(1000 - MAX(0, (30 - context->back_chance))*1000/30)/1000);
    doc_printf(cols[0], " %-7.7s: %d%%\n", "Catch", 99*(1000 - MAX(0, (37 - context->back_chance))*1000/30)/1000);
    doc_printf(cols[0], " %-7.7s: %d.%2.2dx\n", "Mult", context->mult/100, context->mult%100);
    doc_printf(cols[0], " %-7.7s: %d.%2.2d\n", "Throws", num_throw/100, num_throw%100);

    mult = mult * crit.mul / 100;
    to_d = to_d + crit.to_d/100;

    doc_printf(cols[0], "<color:G> %-7.7s</color>\n", "Damage");

    if (!have_flag(context->flags, OF_BRAND_ORDER))
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

    _display_weapon_slay(mult, 100, FALSE, context->mult, num_throw, context->obj->dd, context->obj->ds, to_d, "Normal", TERM_WHITE, cols[0]);
    if (force)
        _display_weapon_slay(mult, 100, force, context->mult, num_throw, context->obj->dd, context->obj->ds, to_d, "Force", TERM_L_BLUE, cols[0]);

    i = 0;
    
    for (_slay = slay_list[0];; _slay = slay_list[++i])
    {
        int _slay_mult = 0;
        if (!_slay.tier) break;
        if ((_slay.kill_flag) && (have_flag(context->flags, _slay.kill_flag)))
            _slay_mult = slay_tiers[_slay.tier - 1].kill * 10;
        else if (have_flag(context->flags, _slay.slay_flag))
            _slay_mult = slay_tiers[_slay.tier - 1].slay * 10;
        if (_slay_mult) _display_weapon_slay(mult, _slay_mult, force, context->mult, num_throw, context->obj->dd, context->obj->ds, to_d, (_slay.slay_flag == OF_BRAND_ELEC) ? "Elec" : _slay.kill_desc, _slay.is_slay ? TERM_YELLOW : TERM_RED, cols[0]);
    }

    /* Column #1 */
    doc_insert(cols[1], "<color:G>Accuracy</color>\n");
    doc_insert(cols[1], " AC Hit\n");

    doc_printf(cols[1], "%3d %2d%%\n", 25, throw_hit_chance(to_h, 25, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 50, throw_hit_chance(to_h, 50, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 75, throw_hit_chance(to_h, 75, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 100, throw_hit_chance(to_h, 100, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 125, throw_hit_chance(to_h, 125, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 150, throw_hit_chance(to_h, 150, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 175, throw_hit_chance(to_h, 175, 10));
    doc_printf(cols[1], "%3d %2d%%\n", 200, throw_hit_chance(to_h, 200, 10));

    /* Assemble the result */
    doc_insert_cols(doc, cols, 2, 1);
    doc_free(cols[0]);
    doc_free(cols[1]);
}

