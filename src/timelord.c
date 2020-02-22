#include "angband.h"

#include <assert.h>

/****************************************************************
 * Private Helpers
 ****************************************************************/

/* Finding what monster to evolve into is trivial, since the monster_race type
   keeps a pointer in that direction. However, we would like to reverse evolution
   turning harder monsters into easier ones. This fn will scan the monster race
   table looking for a monster that evolves into this one. Of course, we assume
   there is at most one such race to be found (Not True!)
   Returns 0 if no such race can be found.
*/
static int _find_evolution_idx(int r_idx)
{
    monster_race *r_ptr;

    if (r_idx <= 0) return 0;
    r_ptr = &r_info[r_idx];
    return r_ptr->next_r_idx;
}

static int _find_devolution_idx(int r_idx)
{
    int i;

    if (r_idx <= 0) return 0;

    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (r_ptr->next_r_idx == r_idx)
            return i;
    }

    return 0;
}

/*  Evolve or Devolve a Monster. I spiked this from monster_gain_exp() in melee2.c without
    any great understanding on my part.
    UPDATE: Use this for polymorph monster as well (spells3.c).
*/
void mon_change_race(mon_ptr mon, int new_r_idx, cptr verb)
{
    char m_name[80], new_name[80];
    int old_hp, old_maxhp, old_r_idx;
    byte old_sub_align;
    monster_race *race;

    assert(mon);
    if (new_r_idx <= 0) return;
    if (!quest_allow_poly(mon)) return;
    if (p_ptr->inside_arena) return;

    old_hp = mon->hp;
    old_maxhp = mon->max_maxhp;
    old_r_idx = mon->r_idx;
    old_sub_align = mon->sub_align;

    inc_cur_num(mon, -1);

    monster_desc(m_name, mon, 0);
    mon->r_idx = new_r_idx;
    mon->drop_ct = get_monster_drop_ct(mon);

    inc_cur_num(mon, 1);

    mon->ap_r_idx = mon->r_idx;
    race = mon_race(mon);

    if (race->flags1 & RF1_FORCE_MAXHP)
    {
        mon->max_maxhp = maxroll(race->hdice, race->hside);
    }
    else
    {
        mon->max_maxhp = damroll(race->hdice, race->hside);
    }
    if (ironman_nightmare)
    {
        u32b hp = mon->max_maxhp * 2L;

        mon->max_maxhp = (s16b)MIN(30000, hp);
    }
    mon->maxhp = mon->max_maxhp;
    mon->hp = old_hp * mon->maxhp / old_maxhp;

    mon->mspeed = get_mspeed(race);

    if (!is_pet(mon) && !(race->flags3 & (RF3_EVIL | RF3_GOOD)))
        mon->sub_align = old_sub_align;
    else
    {
        mon->sub_align = SUB_ALIGN_NEUTRAL;
        if (race->flags3 & RF3_EVIL) mon->sub_align |= SUB_ALIGN_EVIL;
        if (race->flags3 & RF3_GOOD) mon->sub_align |= SUB_ALIGN_GOOD;
    }

    mon->exp = 0;

    if (is_pet(mon) || mon->ml)
    {
        if (!ignore_unview || player_can_see_bold(mon->fy, mon->fx))
        {
            if (p_ptr->image)
            {
                monster_race *hallu_race;
                cptr hallu_name;
                do
                {
                    hallu_race = &r_info[randint1(max_r_idx - 1)];
                }
                while (!hallu_race->name || (hallu_race->flags1 & RF1_UNIQUE));
                hallu_name = r_name + hallu_race->name;
                cmsg_format(TERM_L_BLUE, "%^s %s into %s %s.", m_name, verb, is_a_vowel(hallu_name[0]) ? "an" : "a", r_name + hallu_race->name);
            }
            else if (mon->nickname)
            {
                cptr my_desc = r_name + race->name; /* hack - no monster_desc() flags fully suppress the nickname */
                cmsg_format(TERM_L_BLUE, "%^s %s into %s %s.", m_name, verb, is_a_vowel(my_desc[0]) ? "an" : "a", my_desc);
            }
            else
            {
                monster_desc(new_name, mon, MD_INDEF_VISIBLE);
                cmsg_format(TERM_L_BLUE, "%^s %s into %s.", m_name, verb, new_name);
                if (race->r_sights < MAX_SHORT) race->r_sights++;
            }
        }
        if (!p_ptr->image) r_info[old_r_idx].r_xtra1 |= MR1_SINKA;

        /* Now you feel very close to this pet. */
        mon_set_parent(mon, 0);
    }

    update_mon(mon->id, FALSE);
    lite_spot(mon->fy, mon->fx);

    p_ptr->window |= (PW_MONSTER_LIST);
}

static bool _monster_save(monster_race* r_ptr, int power)
{
    if (r_ptr->flagsr & RFR_RES_ALL)
        return TRUE;
    else if (r_ptr->flags1 & RF1_UNIQUE)
        return r_ptr->level > randint1(2*power/3);
    else
        return r_ptr->level > randint1(power);
}

bool devolve_monster(int m_idx, bool msg)
{
    monster_type* m_ptr = &m_list[m_idx];
    monster_race *r_ptr;
    int r_idx = real_r_idx(m_ptr);
    char m_name[MAX_NLEN];

    if (r_idx <= 0) return FALSE;

    r_ptr = &r_info[r_idx];    /* We'll use the current race for a saving throw */
    r_idx = _find_devolution_idx(r_idx);
    monster_desc(m_name, m_ptr, 0);

    if (r_idx <= 0)
    {
        if (msg)
            msg_format("%^s is too primitive for further devolution.", m_name);
        return FALSE;
    }

    if (_monster_save(r_ptr, 2*p_ptr->lev))
    {
        if (msg)
            msg_format("%^s resists.", m_name);
        return FALSE;
    }

    set_monster_csleep(m_idx, 0);
    mon_change_race(m_ptr, r_idx, "devolved");
    return TRUE;
}

bool evolve_monster(int m_idx, bool msg)
{
    monster_type* m_ptr = &m_list[m_idx];
    monster_race *r_ptr;
    int r_idx = real_r_idx(m_ptr);
    char m_name[MAX_NLEN];

    if (r_idx <= 0) return FALSE;
    monster_desc(m_name, m_ptr, 0);
    r_idx = _find_evolution_idx(r_idx);

    if (r_idx <= 0)
    {
        if (msg)
            msg_format("%^s has reached evolutionary perfection.", m_name);
        return FALSE;
    }
    r_ptr = &r_info[r_idx];    /* We'll use the target race for a saving throw */
    set_monster_csleep(m_idx, 0);
    if (_monster_save(r_ptr, 2*p_ptr->lev))
    {
        if (msg)
            msg_format("%^s resists.", m_name);
        return FALSE;
    }
    mon_change_race(m_ptr, r_idx, "evolved");
    return TRUE;
}

bool check_foresight(void)
{
    if (psion_check_foresight())
        return TRUE;

    if (p_ptr->tim_foresight && randint1(100) <= 25)
    {
        msg_print("You saw that one coming!");
        return TRUE;
    }

    return FALSE;
}

/****************************************************************
 * Private Spells
 ****************************************************************/
static void _bolt_spell(int cmd, variant *res)
{
    int dd = 3 + p_ptr->lev/4;
    int ds = 4;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a temporal bolt at chosen foe. Time based attacks may produce various "
                            "effects on a monster including slowing, stasis, and many others.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_bolt_or_beam(
            beam_chance() - 10,
            GF_TIME,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _regeneration_spell(int cmd, variant *res)
{
    int b = spell_power(80);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Regeneration");
        break;
    case SPELL_DESC:
        var_set_string(res, "Speeds your recovery from physical damage.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(b, b));
        break;
    case SPELL_CAST:
        set_tim_regen(b + randint1(b), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _foretell_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Foretell");
        break;
    case SPELL_DESC:
        var_set_string(res, "Detects nearby monsters.");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _quicken_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Quicken");
        break;
    case SPELL_DESC:
        var_set_string(res, "Gives a small speed boost for a short while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(7, 7));
        break;
    case SPELL_CAST:
        set_tim_spurt(spell_power(7 + randint1(7)), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _withering_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wither");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroy an adjacent wall, tree or door.");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        var_set_bool(res, TRUE);

        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (!in_bounds(y, x)) return;
        if (cave_have_flag_bold(y, x, FF_DOOR))
        {
            cave_alter_feat(y, x, FF_TUNNEL);
            if (!cave_have_flag_bold(y, x, FF_DOOR)) /* Hack: Permanent Door in Arena! */
            {
                msg_print("The door withers away.");
                p_ptr->update |= (PU_FLOW);
            }
        }
        else if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            msg_print("The wall turns to dust.");

            p_ptr->update |= (PU_FLOW);
        }
        else if (cave_have_flag_bold(y, x, FF_TREE))
        {
            cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);
            msg_print("The tree shrivels and dies.");
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _blast_spell(int cmd, variant *res)
{
    int dam = spell_power(3*p_ptr->lev/2 + 15 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Blast");
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a temporal blast at chosen foe. Time based attacks may produce various "
                            "effects on a monster including slowing, stasis, and many others.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_TIME, dir, dam, 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _back_to_origins_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Revert");
        break;
    case SPELL_DESC:
        var_set_string(res, "Eliminate monster offspring.");
        break;
    case SPELL_CAST:
    {
        int i, ct;

        ct = 0;
        for (i = 1; i < max_m_idx; i++)
        {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;

            if (!m_ptr->r_idx) continue;
            r_ptr = real_r_ptr(m_ptr);
            if ( (r_ptr->flags2 & RF2_MULTIPLY)
                && r_ptr->cur_num > 1  /* shouldn't this be 2 ... well, breeding in *band has never been biologically accurate */
                && !_monster_save(r_ptr, 3*p_ptr->lev) )
            {
                delete_monster_idx(i);
                ct++;
            }
        }

        if (ct > 0)
            msg_print("You feel the local population has reverted to an earlier state.");
        else
            msg_print("You feel the local population is stable.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _haste_spell(int cmd, variant *res)
{
    int base = spell_power(p_ptr->lev);
    int sides = spell_power(20 + p_ptr->lev);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "You gain a temporary speed boost.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, sides));
        break;
    case SPELL_CAST:
        set_fast(base + randint1(sides), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wave_spell(int cmd, variant *res)
{
    int ds = 3*p_ptr->lev/2;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Wave");
        break;
    case SPELL_DESC:
        var_set_string(res, "Produce a wave of time, affecting all monsters in sight.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        project_hack(GF_TIME, spell_power(randint1(ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shield_spell(int cmd, variant *res)
{
    int b = spell_power(15);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shield");
        break;
    case SPELL_DESC:
        var_set_string(res, "Grants an Aura of Time for a short while.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(b, b));
        break;
    case SPELL_CAST:
        set_tim_sh_time(b + randint1(b), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rewind_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Rewind");
        break;
    case SPELL_DESC:
        var_set_string(res, "Temporal escape:  You flee to safety, but forget some of your recent experiences.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!get_check("You will irreversibly alter the time line. Are you sure?")) return;
        var_set_bool(res, TRUE);

        if (p_ptr->inside_arena || ironman_downward || !dun_level)
        {
            msg_print("Nothing happens.");
            return;
        }

        recall_player(1, FALSE);
        p_ptr->leaving_method = LEAVING_REWIND_TIME; /* Set after recall_player() to override LEAVING_RECALL */
        process_world_aux_movement(); /* Hack! Recall Now, Now, Now!!! */

        if (p_ptr->prace == RACE_ANDROID)
        {
            dec_stat(A_CON, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_INT, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_DEX, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_WIS, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_STR, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_CHR, 10, TRUE);
        }
        else
        {
            int amount = 0;

            if (p_ptr->lev < 3) return;
            amount = exp_requirement(p_ptr->lev-1);
            amount -= exp_requirement(p_ptr->lev-2);
            if (amount > 100000) amount = 100000;
            if (amount > p_ptr->max_exp) amount = p_ptr->max_exp;
            if (amount > p_ptr->exp) p_ptr->exp = 0;
            else p_ptr->exp -= amount;
            p_ptr->max_exp -= amount;
            check_experience();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int  _breath_dam(void) {
    int l = (p_ptr->lev - 30);
    return spell_power(11*p_ptr->lev/3 + l*l/4 + p_ptr->to_d_spell); /* 283 max damage ... */
}
static void _breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathe time at chosen foe. Time based attacks may produce various "
                            "effects on a monster including slowing, stasis, and many others.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_dam()));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_TIME, dir, _breath_dam(), -3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _remember_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Remembrance");
        break;
    case SPELL_DESC:
        var_set_string(res, "Restores life and stats.");
        break;
    case SPELL_CAST:
        do_res_stat(A_STR);
        do_res_stat(A_INT);
        do_res_stat(A_WIS);
        do_res_stat(A_DEX);
        do_res_stat(A_CON);
        do_res_stat(A_CHR);
        restore_level();
        lp_player(1000);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _stasis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stasis");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to suspend all monsters in view.");
        break;
    case SPELL_CAST:
        stasis_monsters(spell_power(7 * p_ptr->lev / 3));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _travel_spell(int cmd, variant *res)
{
    int r = spell_power(p_ptr->lev / 2 + 10);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Travel");
        break;
    case SPELL_DESC:
        var_set_string(res, "Travel instantaneously to given location. Be careful you don't accidentally get lost!");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(r));
        break;
    case SPELL_CAST:
        var_set_bool(res, dimension_door(r));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _double_move_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Double Move");
        break;
    case SPELL_DESC:
        var_set_string(res, "After casting this spell, you may take two additional free moves. Make the most of them!");
        break;
    case SPELL_CAST:
        if (p_ptr->free_turns)
        {
            msg_print("You're wasting your free turns!");
        }
        else
        {
            /* 3 is a bit of a hack to prevent chain casting this spell.
               See process_player in dungeon.c for details */
            p_ptr->free_turns = 3;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _foresee_spell(int cmd, variant *res)
{
    int b = spell_power(7);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Foresee");
        break;
    case SPELL_DESC:
        var_set_string(res, "For a very short time, you will be able to look into the future.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("dur %d", b));
        break;
    case SPELL_CAST:
        set_tim_foresight(b, FALSE);
        var_set_bool(res, TRUE);
        break;
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
    {  1,  2, 30, _bolt_spell},
    {  3,  3, 40, _regeneration_spell},
    {  6,  4, 40, _foretell_spell},
    {  8,  8, 50, _quicken_spell},
    { 10,  9, 50, _withering_spell},
    { 13, 10, 50, _blast_spell},
    { 17, 12, 50, _back_to_origins_spell},
    { 23, 15, 60, _haste_spell},
    { 27, 20, 60, _wave_spell},
    { 30, 10, 60, _shield_spell},
    { 33, 50, 70, _rewind_time_spell},
    { 35, 35, 70, _breath_spell},
    { 37, 50, 70, _remember_spell},
    { 39, 30, 70, _stasis_spell},
    { 41, 20, 80, _travel_spell},
    { 45, 80, 80, _double_move_spell},
    { 49,100, 80, _foresee_spell},
    { -1, -1, -1, NULL}
};

static int _get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, max, _spells);
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 30) res_add(RES_TIME);
    p_ptr->pspeed += (p_ptr->lev) / 7;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_TIME);
    if (p_ptr->lev >= 7) add_flag(flgs, OF_SPEED);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);
        if (b <= 90)
        {
        }
        else if (b <= 95)
        {
            set_fast(0, TRUE);
            set_slow(randint1(5) + 5, FALSE);
            msg_print("You feel caught in a temporal inversion!");
        }
        else if (b <= 99)
        {
            lose_exp(p_ptr->exp / 4);
            msg_print("You feel life's experiences fade away!");
        }
        else
        {
            dec_stat(A_DEX, 10, FALSE);
            dec_stat(A_WIS, 10, FALSE);
            dec_stat(A_CON, 10, FALSE);
            dec_stat(A_STR, 10, FALSE);
            dec_stat(A_CHR, 10, FALSE);
            dec_stat(A_INT, 10, FALSE);
            msg_print("You feel as weak as a newborn kitten!");
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "timecraft";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 80;
        me.encumbrance.enc_wgt = 600;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    py_display_spells(doc, spells, ct);
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(4, 7));
}

class_t *time_lord_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  35,  35,   2,  16,   8,  48,  20 };
    skills_t xs = {  7,  11,  10,   0,   0,   0,  13,  13 };

        me.name = "Time-Lord";
        me.desc = "Time-Lords are masters of temporal magic, altering the flow of time "
                  "to their advantage. They gain new powers as their experience grows, "
                  "and are unique in their ability to use time-based spells against monsters. "
                  "Not only do these attacks damage a foe, they also inflict a wide variety of possible effects, "
                  "from slowing to amnesia, from evolution to devolution, from weakening to stasis. "
                  "In addition to temporal attacks, Time-Lords gain great powers of speed, and "
                  "become naturally faster with experience. Legend has it that masters of time can even see "
                  "into the future, avoiding attacks that would otherwise prove fatal!\n \n"
                  "Time-Lords are fairly proficient with magical devices, but are mediocre fighters and not "
                  "good at archery at all. At high levels, they become resistant to time. "
                  "Time-Lords' primary stat is Wisdom.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  0;

        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 96;
        me.base_hp = 0;
        me.exp = 125;
        me.pets = 20;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
