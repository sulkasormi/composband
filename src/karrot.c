#include "angband.h"

typedef struct
{
    u16b goal_idx;
    u16b dungeon;
    u16b level;
    u16b danger_level;
    byte goal_ct;
    byte killed;
    byte start_lev;
    byte completed_lev;
    u32b completed_turn;
} dpl_quest;

#define MAX_KT_QUEST 30
#define _MAX_PACK_SLOTS 12

static byte _q_idx = 0;
static dpl_quest _kt_quests[MAX_KT_QUEST];
static inv_ptr _dragon_pack = NULL;
static bool _pack_initialized = FALSE;

void _dragon_pack_init(void)
{
    if (_pack_initialized) return;
    inv_free(_dragon_pack);
    _dragon_pack = inv_alloc("Extra", INV_SPECIAL2, _MAX_PACK_SLOTS);
    _pack_initialized = TRUE;
}

inv_ptr _get_pack(void)
{
    if (_pack_initialized) return NULL;
    return _dragon_pack;
}

inv_ptr _copy_dragon_pack(void)
{
    _dragon_pack_init();
    return inv_copy(_dragon_pack);
}

equip_template_ptr _karrot_equip_template(bool dragon)
{
    if (dragon) return &b_info[109];
    else return &b_info[0];
}

void karrot_equip_on_poly(void)
{
    bool to_dragon = (p_ptr->mimic_form == MIMIC_DRAGON);
    equip_template_ptr old_template = _karrot_equip_template(!to_dragon);
    equip_template_ptr new_template = _karrot_equip_template(to_dragon);

    slot_t  slot;
    inv_ptr _equipment = get_equipment();
    inv_ptr temp = inv_copy(_equipment);
    inv_ptr temp2 = _copy_dragon_pack();

    inv_clear(_dragon_pack);
    set_equip_template(new_template);

    /* We need to not just clear the equipment, but also make sure the items
     * aren't tracked as equipped */
    for (slot = 1; slot <= old_template->max; slot++)
    {
        obj_ptr src = inv_obj(temp, slot);
        if (!src) continue;
        equip_remove(slot);
        src->loc.where = 0;
    }

    for (slot = 1; slot <= old_template->max; slot++)
    {
        obj_ptr src = inv_obj(temp, slot);
        slot_t  new_slot;

        if (!src) continue;
        src->marked |= OM_BEING_SHUFFLED;
        new_slot = equip_first_empty_slot(src);
        if (new_slot)
            inv_add_at(_equipment, src, new_slot);
        else
            inv_add(_dragon_pack, src);
        obj_release(src, OBJ_RELEASE_QUIET);
        src->marked &= ~OM_BEING_SHUFFLED;
    }

    for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
    {
        obj_ptr src = inv_obj(temp2, slot);
        slot_t  new_slot;

        if (!src) continue;
        src->marked |= OM_BEING_SHUFFLED;
        new_slot = equip_first_empty_slot(src);

        if (new_slot)
            equip_wield(src, new_slot);
        else
            inv_add(_dragon_pack, src);
        obj_release(src, OBJ_RELEASE_QUIET);
        src->marked &= ~OM_BEING_SHUFFLED;
    }

    inv_free(temp);
    inv_free(temp2);

    p_ptr->update |= PU_BONUS | PU_TORCH | PU_MANA | PU_HP;
    p_ptr->redraw |= PR_EQUIPPY;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
}

static void _generate_storm_spell(int cmd, variant *res)
{
    int dam = spell_power(13*p_ptr->lev/2 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Storm Winds");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a ball of storm winds at your chosen target.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_STORM, dir, dam, 3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _confuse_monster_spell(int cmd, variant *res)
{
    int pow = spell_power(p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confuse Monster");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to confuse a monster.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        confuse_monster(dir, pow);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _sleep_monster_spell(int cmd, variant *res)
{
    int pow = spell_power(p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sleep Monster");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to put a single monster to sleep.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        sleep_monster(dir, pow);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 8);
    int sides = 10;
    switch (cmd)
    {
        case SPELL_NAME:
        var_set_string(res, "Mana Bolt");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell + p_ptr->lev)));
        break;
    case SPELL_DESC:
        var_set_string(res, "Fires a bolt of mana.");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_MANA, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell + p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _polymorph_dragon_spell(int cmd, variant *res)
{
    bool no_poly = ((get_race()->flags & RACE_NO_POLY) || (mut_present(MUT_DRACONIAN_METAMORPHOSIS)));
    switch (cmd)
    {
    case SPELL_NAME:
        if (no_poly) var_set_string(res, "Word of Destruction");
        else var_set_string(res, "Dragon Polymorph");
        break;
    case SPELL_DESC:
        if (no_poly) var_set_string(res, "Destroys everything nearby.");
        else if (p_ptr->mimic_form == MIMIC_DRAGON) var_set_string(res, "Reverts you back into your normal shape.");
        else var_set_string(res, "Transforms you into a fire-breathing dragon.");
        break;
    case SPELL_INFO:
        if (!no_poly) var_set_string(res, info_duration(500, 500));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (no_poly)
        {
            destroy_area(py, px, 12 + randint1(4), spell_power(4 * p_ptr->lev));
            var_set_bool(res, TRUE);
            break;
        }
        else if (p_ptr->mimic_form == MIMIC_DRAGON) set_mimic(500 + randint1(500), MIMIC_NONE, FALSE);
        else if (p_ptr->mimic_form == MIMIC_NONE) set_mimic(500 + randint1(500), MIMIC_DRAGON, FALSE);
        else
        {
            msg_print("You are already polymorphed!");
            break;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _device_focus_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Device Focus");
        break;
    case SPELL_DESC:
        var_set_string(res, "Channels your magic through a device, generating a powerful mana attack but sometimes destroying the device and hurting you.");
        break;
    case SPELL_INFO:
        var_set_string(res, "dam 7*power");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        int dir = 0;

        var_set_bool(res, FALSE);

        prompt.prompt = "Focus on which device?";
        prompt.error = "You have nothing to focus on.";
        prompt.filter = object_is_device;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;
        if (!get_fire_dir(&dir)) return;

        var_set_bool(res, TRUE);

        fire_ball_hide(GF_MANA, dir, spell_power(7*device_level(prompt.obj)), 0);
        if (randint1(106) >= p_ptr->lev + adj_mag_mana[p_ptr->stat_ind[A_INT]])
        {
            char o_name[MAX_NLEN];
            object_desc(o_name, prompt.obj, OD_OMIT_PREFIX | OD_COLOR_CODED);
            msg_format("Your %s explodes!", o_name);
            obj_zero(prompt.obj);
            obj_release(prompt.obj, OBJ_RELEASE_QUIET);
            take_hit(DAMAGE_NOESCAPE, 20, "an exploding device");
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _force_field_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Force Field");
        break;
    case SPELL_DESC:
        var_set_string(res, "Installs a temporary force field around your weapon, adding 1 damage die and causing the weapon's attacks to stun. The force field does not work if you are dual-wielding, and disappears if you unwield your weapon.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(35, 35));
        break;
    case SPELL_CAST:
        set_tim_field(35 + randint1(35), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _thundershadow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shadow Projection");
        break;
    case SPELL_INFO:
        var_set_string(res, format("dam %dd6/zap", 8 + p_ptr->lev / 3));
        break;
    case SPELL_DESC:
        var_set_string(res, "Projects your shadow to a nearby location. Your shadow will conduct lightning and zap any monster who touches it; however, you will also take damage if a monster successfully eliminates the shadow (15 if the shadow explodes, 30 if it is destroyed and 60 if it is dispelled).");
        break;
    case SPELL_CAST:
    {
        int dir = 0, y, x, tx = 0, ty = 0, etaisyys = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        if ((dir == 5) && (target_okay()))
        {
            tx = target_col;
            ty = target_row;
        }
        else
        {
            ty = py + ddy[dir];
            tx = px + ddx[dir];
        }
        if (!cave_naked_bold(ty, tx))
        {
            msg_print("You can only project your shadow onto an empty square!");
            return;
        }
        etaisyys = distance(ty, tx, py, px);
        if (etaisyys > MAX_RANGE)
        {
            msg_print("You cannot project your shadow that far!");
            return;
        }

        var_set_bool(res, TRUE);

        /* This costs time and HP whether successful or not */
        if ((!los(ty, tx, py, px)) && (!one_in_(etaisyys)))
        {
            msg_print("You try to project your shadow without line of sight, but your attempt fails.");
            return;
        }

        /* Only project one shadow */
        for (y = 1; y < cur_hgt - 1; y++)
        {
            for (x = 1; x < cur_wid - 1; x++)
            {
                cave_type *c_ptr;
                if (!in_bounds(y, x)) continue; /* Paranoia - should never happen */
                c_ptr = &cave[y][x];
                if (c_ptr->mimic == feat_shadow_zap)
                {
                    c_ptr->info &= ~(CAVE_OBJECT);
                    c_ptr->mimic = 0;
                    note_spot(y, x);
                    lite_spot(y, x);
                }
            }
        }

        /* Place the shadow */
        set_trap(ty, tx, feat_shadow_zap);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _karrot_spells[] =
{
    /* lvl cst fail spell */
    {  1,  1, 22, detect_monsters_spell},
    {  4,  3, 27, scare_spell},
    {  7,  4, 32, lightning_bolt_spell},
    { 10,  5, 32, heroism_spell},
    { 13,  3, 37, _sleep_monster_spell},
    { 16,  8, 42, _confuse_monster_spell},
    { 19, 10, 47, mind_blast_spell},
    { 22, 12, 47, floating_spell},
    { 25, 30, 52, _thundershadow_spell},
    { 28, 20, 52, fire_ball_spell},
    { 31, 10, 47, phase_door_spell},
    { 34, 25, 52, _mana_bolt_spell},
    { 37, 40, 57, dominate_living_I_spell},
    { 40, 50, 57, _force_field_spell},
    { 43, 50, 62, _device_focus_spell},
    { 46, 80, 67, _generate_storm_spell},
    { 49, 100, 72, _polymorph_dragon_spell},
    { -1, -1, -1, NULL},
};

static int _karrot_get_spells(spell_info* spells, int max)
{
    return get_spells_aux(spells, max, _karrot_spells);
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 30) res_add(RES_FEAR);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_FEAR);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "spell";
        me.which_stat = A_INT;
        me.min_fail = 0;
        me.min_level = 1;
        me.options = CASTER_USE_HP;
        init = TRUE;
    }
    return &me;
}

static void _karrot_ini_quests(void)
{
    int i;
    for (i = 0; i < MAX_KT_QUEST; i++)
    {
        _kt_quests[i].goal_idx = 0;
        _kt_quests[i].dungeon = 0;
        _kt_quests[i].level = 0;
        _kt_quests[i].danger_level = 0;
        _kt_quests[i].goal_ct = 0;
        _kt_quests[i].killed = 0;
        _kt_quests[i].start_lev = 0;
        _kt_quests[i].completed_lev = 0;
        _kt_quests[i].completed_turn = 0;
    }
    _q_idx = 0;
}

static void _give_reward(int monesko)
{
    static arena_type _karrot_rewards[MAX_KT_QUEST] = {
     { 0, TV_POTION, SV_POTION_SPEED},
     { 1, TV_POTION, SV_POTION_SPEED},
     { 2, TV_POTION, SV_POTION_SPEED},
     { 3, TV_POTION, SV_POTION_SPEED},
     { 4, TV_WAND, EFFECT_TELEPORT_AWAY},
     { 5, TV_STAFF, EFFECT_TELEPORT},
     { 6, TV_WAND, EFFECT_BALL_FIRE},
     { 7, TV_ROD, EFFECT_BALL_FIRE},
     { 8, TV_SCROLL, SV_SCROLL_ACQUIREMENT},
     { 9, TV_SCROLL, SV_SCROLL_ACQUIREMENT},
     { 10, TV_WAND, EFFECT_BALL_NEXUS},
     { 11, TV_SCROLL, SV_SCROLL_ICE},
     { 12, TV_SCROLL, SV_SCROLL_CHAOS},
     { 13, TV_POTION, SV_POTION_NEW_LIFE},
     { 14, TV_POTION, SV_POTION_RESTORE_MANA},
     { 15, TV_ROD, EFFECT_BALL_NETHER},
     { 16, TV_STAFF, EFFECT_CONFUSING_LITE},
     { 17, TV_POTION, SV_POTION_POLYMORPH},
     { 18, TV_ROD, EFFECT_SPEED_HERO},
     { 19, TV_STAFF, EFFECT_DESTRUCTION},
     { 20, TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION},
     { 21, TV_SCROLL, SV_SCROLL_GENOCIDE},
     { 22, TV_STAFF, EFFECT_BANISH_ALL},
     { 23, TV_WAND, EFFECT_BALL_DISINTEGRATE},
     { 24, TV_ROD, EFFECT_BALL_CHAOS},
     { 25, TV_STAFF, EFFECT_DARKNESS_STORM},
     { 26, TV_WAND, EFFECT_ROCKET},
     { 27, TV_STAFF, EFFECT_RESTORE_MANA},
     { 28, TV_SCROLL, SV_SCROLL_CRAFTING},
     { 29, TV_SCROLL, SV_SCROLL_ARTIFACT},
    };
    if (no_wilderness) /* accelerated rewards */
    {
        monesko *= 2;
        if (one_in_(2)) monesko++;
    }
    if (monesko >= MAX_KT_QUEST) return;
    else {
        int tval, sval, taso = (p_ptr->max_plv * 8 / 5);
        object_type *q_ptr, forge;
        tval = _karrot_rewards[monesko].tval;
        sval = _karrot_rewards[monesko].sval;

        q_ptr = &forge;
        switch (tval)
        {
            case TV_WAND: case TV_ROD: case TV_STAFF:
            {
                device_effect_info_ptr e_ptr = device_get_effect_info(tval, sval);
                if (!e_ptr) return; /* paranoia */
                object_prep(q_ptr, lookup_kind(tval, SV_ANY));
                if (taso > e_ptr->level) q_ptr->level = e_ptr->level - taso;
                device_init_fixed(q_ptr, sval);
                break;
            }
            default:
                object_prep(q_ptr, lookup_kind(tval, sval));
                apply_magic(q_ptr, object_level, AM_NO_FIXED_ART);
                obj_make_pile(q_ptr);
        }
        object_origins(q_ptr, ORIGIN_PATRON);
        obj_identify_fully(q_ptr);
        msg_format("The voice of Karrot echoes through the dungeon: <color:v>Behold, my %s, how generously I reward thee!</color>", p_ptr->psex == SEX_FEMALE ? "daughter" : "son");
        (void)drop_near(q_ptr, -1, py, px);
    }    
}

quest_ptr karrot_get_quest(int dungeon, int level)
{
    point_t tbl[3] = { {30, 130}, {34, 134}, {50, 166} };
    int mahis = (no_wilderness ? 5 : 9);
    int klev = karrot_level();
    int verrokki = isompi(klev + 5, (klev * interpolate(klev, tbl, 3) / 100) + randint1(mahis) - 3);
    quest_ptr q;

    /* No quest if level too low */
    if (level < verrokki) return NULL;

    /* Not too early */
    if (p_ptr->max_plv < 3) return NULL;

    /* Not too many quests */
    if (_q_idx >= MAX_KT_QUEST) return NULL;
    if ((no_wilderness) && ((_q_idx * 2) >= MAX_KT_QUEST)) return NULL;

    /* Paranoia */
    if (!dungeon) return NULL;
    if ((level == d_info[dungeon].maxdepth) && (!dungeon_conquered(dungeon))) return NULL;

    /* Not in Chameleon Cave */
    if (dungeon == DUNGEON_CHAMELEON) return NULL;

    /* Not too many quests in one dungeon */
    if (!no_wilderness)
    {
        int i, osumat = 0, limit = (dungeon == DUNGEON_ANGBAND) ? 6 : 3;
        for (i = 0; i < _q_idx; i++)
        {
            if (_kt_quests[i].dungeon == dungeon) osumat++;
            if (osumat >= limit) break;
        }
        if (osumat >= limit) return NULL;
        /* Never guarantee a quest */
        if (magik(60))
        {
            return NULL;
        }
    }

    /* Not too many quests at this clvl, especially if the danger level is decreasing */
    {
        int i, osumat = 0, huippu = 0;
        for (i = 0; i < _q_idx; i++)
        {
            if (_kt_quests[i].start_lev == p_ptr->max_plv)
            {
                osumat++;
                if (_kt_quests[i].level > huippu) huippu = _kt_quests[i].level;
            }
        }
        if (osumat >= 6) return NULL;
        if ((osumat >= 2) && (level < huippu + osumat)) return NULL;
    }

    /* Roll a new quest */
    q = quests_get(PURPLE_QUEST);
    q->level = MIN(88, (MAX(klev + 5, (klev * interpolate(klev, tbl, 3) / 100)) + level) / 2);
    q->dungeon = dungeon;
    get_purple_questor(q);
    q->goal = QG_KILL_MON;
    q->status = QS_IN_PROGRESS;
    q->level = level;
    q->danger_level = level;

    /* Store the details permanently */
    _kt_quests[_q_idx].goal_idx = q->goal_idx;
    _kt_quests[_q_idx].goal_ct = q->goal_count;
    _kt_quests[_q_idx].dungeon = q->dungeon;
    _kt_quests[_q_idx].start_lev = p_ptr->max_plv;
    _kt_quests[_q_idx].killed = 0;
    _kt_quests[_q_idx].completed_lev = 0;
    _kt_quests[_q_idx].completed_turn = 0;
    _kt_quests[_q_idx].level = level;
    _kt_quests[_q_idx].danger_level = r_info[q->goal_idx].level;

    _q_idx++;

    return q;
}

static void _karrot_quest_cleanup(quest_ptr q)
{
    q->status = QS_UNTAKEN;
    q->level = 0;
    q->completed_lev = 0;
    q->completed_turn = 0;
    q->goal_current = 0;
    q->goal_idx = 0;
    q->goal_count = 0;
}

void karrot_quest_finished(quest_ptr q, bool success)
{
    int _tq = _q_idx - 1;
    _kt_quests[_tq].killed = q->goal_current;
    if (success) _kt_quests[_tq].killed = q->goal_count; /* paranoia */
    _kt_quests[_tq].completed_lev = (prace_is_(RACE_ANDROID)) ? p_ptr->lev : p_ptr->max_plv;
    _kt_quests[_tq].completed_turn = game_turn;
    _karrot_quest_cleanup(q);
    if (!success)
    {
        int i, monesko = 0;
        msg_print("The voice of Karrot roars: <color:v>I am disappointed in thee, mortal.</color>");
        for (i = 0; i < _tq; i++)
        {
            if ((_kt_quests[i].completed_turn) && (_kt_quests[i].killed != _kt_quests[i].goal_ct)) monesko++;
        }
        if ((randint0(50) < monesko) && (_kt_quests[_tq].killed < _kt_quests[_tq].goal_ct / 2))
        {
            bool punished = FALSE;
            msg_print("You feel a sudden surge of power as Karrot gathers his will to punish you...");
            while (!punished) /* This isn't going to be pretty... */
            {
                if (one_in_(3))
                {
                    for (i = 0; i < 6; i++)
                        dec_stat(i, 10 + randint1(15), TRUE);
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    msg_print("You are punched by an invisible fist!");
                    take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), "the wrath of Karrot");
                    set_stun(p_ptr->stun + 10, FALSE);
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    int slot = equip_random_slot(object_is_armour);
                    if (slot) curse_armor(slot);
                    else
                    {
                        msg_print("You are punched by an invisible fist!");
                        take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 5, p_ptr->mhp / 2), "the wrath of Karrot");
                        set_stun(p_ptr->stun + 10, FALSE);
                    }
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    int slot = equip_random_slot(object_is_melee_weapon);
                    if (slot)
                    {
                        object_type *o_ptr = equip_obj(slot);
                        if ((o_ptr) && (o_ptr->name1 != ART_UROG)) curse_weapon(FALSE, slot);
                    }
                    else
                    {
                        msg_print("You are punched by an invisible fist!");
                        take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), "the wrath of Karrot");
                        set_stun(p_ptr->stun + 10, FALSE);
                    }
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    nonlethal_ty_substitute(TRUE);
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    inven_damage(1, set_acid_destroy, 3, RES_NEXUS);
                    inven_damage(1, set_cold_destroy, 3, RES_SHARDS);
                    inven_damage(1, set_elec_destroy, 3, RES_CHAOS);
                    inven_damage(1, set_fire_destroy, 3, RES_CHAOS);
                    punished = TRUE;
                }
                while (one_in_(3))
                {
                    curse_equipment(100, 50);
                    punished = TRUE;
                }
                if (one_in_(3))
                {
                    fame_on_failure();
                    punished = TRUE;
                }
            }
        }
        return;
    }
    else
    {
        int i, monesko = 0;
        /* No need to create stairs because purple quests allow normal stair generation
         * (indeed, generating stairs would be risky because purple quests can appear
         * at the bottom of a dungeon) */
        cmsg_print(TERM_L_BLUE, "You just completed your quest!");
        msg_add_tiny_screenshot(50, 24);
        p_ptr->redraw |= PR_DEPTH;

        for (i = 0; i < _tq; i++)
        {
            if ((_kt_quests[i].completed_turn) && (_kt_quests[i].killed == _kt_quests[i].goal_ct)) monesko++;
        }
        _give_reward(monesko);
    }
}

static bool _kind_is_art_okay(int k_idx)
{
    obj_kind_ptr kind = &k_info[k_idx];
    if (!kind) return FALSE;
    if ((kind->tval < TV_WEARABLE_BEGIN) || (kind->tval > TV_WEARABLE_END)) return FALSE;
    if ((kind->tval == TV_LITE) && (kind->sval != SV_LITE_FEANOR)) return FALSE;
    if (kind->gen_flags & (OFG_INSTA_ART)) return FALSE;
    return TRUE;
}

static int _uusi_tyyppi(void)
{
    int tyyppi;
    get_obj_num_hook = _kind_is_art_okay;
    get_obj_num_prep();
    tyyppi = get_obj_num(object_level);
    get_obj_num_hook = NULL;
    get_obj_num_prep();
    return tyyppi;
}

static int _karrot_happy_score(void)
{
    int i, rspct = 0;
    s32b ptaso = 0, pisteet = 0, osumat = 0, hudit = 0;
    int mult = (no_wilderness ? 6 : 3);
    if (!_q_idx) return 125;
    for (i = 0; i < _q_idx; i++)
    {
        if (!_kt_quests[i].completed_lev) continue;
        if (_kt_quests[i].killed >= _kt_quests[i].goal_ct)
        {
            ptaso += ((_kt_quests[i].start_lev + _kt_quests[i].completed_lev) / 2 + 15);
            pisteet += (_kt_quests[i].level + 25);
            osumat++;
        }
        else
        {
            ptaso += ((_kt_quests[i].start_lev + _kt_quests[i].completed_lev) / 2 + 15);
            pisteet += ((_kt_quests[i].level + 25) * _kt_quests[i].killed / _kt_quests[i].goal_ct);
            hudit++;
            pisteet -= (20 * hudit);
        }
    }
    rspct = MIN(MIN(120 + (10 * osumat), 180 + osumat), (pisteet * 100 / ptaso)) + (osumat * mult) - (hudit * MIN(4, hudit)) + (p_ptr->max_plv * 2);
    return MAX(1, rspct);
}

static int _my_get_slot_weight(object_type *o_ptr)
{
    int paino = get_slot_weight(o_ptr);
    if ((o_ptr->tval == TV_RING) || (o_ptr->tval == TV_AMULET) || (o_ptr->tval == TV_LITE)) return (paino - (paino / 4));
    return paino;
}

bool karrot_replace_art(object_type *o_ptr)
{
    s32b arvo;
    if (!o_ptr) return FALSE; /* paranoia */
    if (!disciple_is_(DISCIPLE_KARROT)) return FALSE;
    if (!object_is_artifact(o_ptr)) return FALSE;
    switch (o_ptr->origin_type)
    {
        case ORIGIN_PATRON:
        case ORIGIN_ART_CREATION:
        case ORIGIN_ACQUIRE:
        case ORIGIN_BIRTH: /* ?? */
        case ORIGIN_STORE:
        case ORIGIN_QUEST_REWARD:
        case ORIGIN_REFORGE:
            return FALSE;
        default: break;
    }
    /* This is a very ugly hack, but also a very simple one */
    if ((o_ptr->name1 == ART_STING) || (o_ptr->name1 == ART_MOKOMAGI) || (o_ptr->name1 == ART_JONES) ||
        (o_ptr->name3 == ART_STING) || (o_ptr->name3 == ART_MOKOMAGI) || (o_ptr->name3 == ART_JONES)) return FALSE;
    arvo = obj_value_real(o_ptr);
    if (randint1(1000) > isompi(arvo / get_slot_weight(o_ptr), 250L)) return FALSE;
    else /* Try to generate a replacement artifact */
    {
        int i;
        object_type forge;
        bool onnistui = FALSE;
        int tyyppi = o_ptr->k_idx;
        int old_object_level = object_level;
        int mode = CREATE_ART_GOOD;
        int paino = -1;
        int happy = MAX(86, _karrot_happy_score()) + 300;
        s32b minimi = pienempi(50000, arvo * happy / 20 / get_slot_weight(o_ptr));
        s32b maksimi = isompi(minimi + 1000, arvo * happy / 15 / get_slot_weight(o_ptr));
        s32b power;

        object_level = MIN(100, MAX(o_ptr->level, minimi / 200));
        if (((minimi < 1000) && (one_in_(2))) || (one_in_(13))) mode = CREATE_ART_CURSED;
        else if ((minimi < 10000) && (one_in_(2))) mode = CREATE_ART_NORMAL;

        if (one_in_(2)) /* Replacement artifact of a different type */
        {
            int uusi_tyyppi = _uusi_tyyppi();
            if (uusi_tyyppi) tyyppi = uusi_tyyppi;
        }

        if (randint1(MIN(324, happy - 300)) > 308 + randint0(16))
        {
            if (!a_info[ART_UROG].generated)
            {
                onnistui = create_named_art_aux(ART_UROG, &forge);
                if (onnistui)
                {
                    a_info[ART_UROG].generated = TRUE;
                    a_info[ART_UROG].found = TRUE;
                }
            }
        }

        for (i = 0; ((i < 16000) && (!onnistui)); i++)
        {
            object_prep(&forge, tyyppi);
            create_artifact(&forge, mode);
            if (paino < 0) paino = _my_get_slot_weight(&forge);
            if (paino < 1) paino = 1;
            power = obj_value_real(&forge) * 40 / paino;

            if ((power >= minimi) && (power <= maksimi))
            {
                onnistui = TRUE;
                break;
            }

            if ((i >= 14641) && (power < minimi) && (power > minimi / 2) && (minimi > 30000))
            {
                if ((forge.pval) && (one_in_(2))) forge.pval += randint1((i - 13441) / 400);
                else do { one_high_resistance(&forge); } while (one_in_(2));
                power = obj_value_real(&forge) * 40 / paino;

                if ((power >= minimi) && (power <= maksimi))
                {
                    onnistui = TRUE;
                    break;
                }
            }
            
            if ((i % 500) == 499)
            {
                if (one_in_(2))
                {
                    int uusi_tyyppi = _uusi_tyyppi();
                    if (uusi_tyyppi) tyyppi = uusi_tyyppi;
                    paino = -1;
                }
                else
                {
                    if (mode == CREATE_ART_GOOD) mode = CREATE_ART_CURSED;
                    else if (mode == CREATE_ART_CURSED) mode = CREATE_ART_NORMAL;
                    else mode = CREATE_ART_GOOD;
                }
            }
        }
        object_level = old_object_level;
        if (onnistui)
        {
            char o_name[MAX_NLEN];
            char *kuvaus;
            strip_name(o_name, o_ptr->k_idx);
            switch (randint0(5))
            {
                case 0: kuvaus = "long-lost"; break;
                case 1: kuvaus = "lost"; break;
                case 2: kuvaus = "stolen"; break;
                case 3: kuvaus = "beloved"; break;
                default: kuvaus = "precious"; break;
            }
            msg_format("The voice of Karrot booms out: <color:v>I am pleased with thee, my %s; thou hast done well to recover my %s %s!</color>", p_ptr->psex == SEX_FEMALE ? "daughter" : "son", kuvaus, o_name);
            no_karrot_hack = TRUE;
            obj_identify_fully(o_ptr);
            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            if (forge.name1 == ART_UROG)
            {
                cmsg_print(TERM_VIOLET, "Thou art my most favored servant, first among my Disciples; take now upon thyself the sword of Destiny, for none are more worthy to wield it.");
            }
            msg_format("Karrot claims %s, and gives you a reward for your good work.", o_name);
            forge.level = o_ptr->level;
            object_origins(&forge, ORIGIN_PATRON);
            obj_identify_fully(&forge);
            obj_zero(o_ptr);
            obj_release(o_ptr, OBJ_RELEASE_QUIET);
            pack_carry(&forge);
            no_karrot_hack = FALSE;
        }
        //obj_free(&forge);
        return (onnistui);
    }
}

static void _birth(void)
{
    disciple_birth();
    py_birth_obj_aux(TV_SWORD, SV_CUTLASS, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    _karrot_ini_quests();
}

static void _karrot_load(savefile_ptr file)
{
    int i;
    u16b check_pack;
    _karrot_ini_quests();
    _q_idx = savefile_read_byte(file);
    for (i = 0; i < _q_idx; i++)
    {
        int j = MIN(i, MAX_KT_QUEST - 1); /* paranoia */
        _kt_quests[j].goal_idx = savefile_read_u16b(file);
        _kt_quests[j].dungeon = savefile_read_u16b(file);
        _kt_quests[j].level = savefile_read_u16b(file);
        _kt_quests[j].danger_level = savefile_read_u16b(file);
        _kt_quests[j].goal_ct = savefile_read_byte(file);
        _kt_quests[j].killed = savefile_read_byte(file);
        _kt_quests[j].start_lev = savefile_read_byte(file);
        _kt_quests[j].completed_lev = savefile_read_byte(file);
        _kt_quests[j].completed_turn = savefile_read_u32b(file);
    }
    check_pack = savefile_read_u16b(file);
    if (check_pack == 0x3FFF)
    {
        _dragon_pack_init();
        inv_load(_dragon_pack, file);
    }
}

static void _karrot_save(savefile_ptr file)
{
    int i;
    savefile_write_byte(file, _q_idx);
    for (i = 0; i < _q_idx; i++)
    {
        savefile_write_u16b(file, _kt_quests[i].goal_idx);
        savefile_write_u16b(file, _kt_quests[i].dungeon);
        savefile_write_u16b(file, _kt_quests[i].level);
        savefile_write_u16b(file, _kt_quests[i].danger_level);
        savefile_write_byte(file, _kt_quests[i].goal_ct);
        savefile_write_byte(file, _kt_quests[i].killed);
        savefile_write_byte(file, _kt_quests[i].start_lev);
        savefile_write_byte(file, _kt_quests[i].completed_lev);
        savefile_write_u32b(file, _kt_quests[i].completed_turn);
    }
    if (_pack_initialized)
    {
        savefile_write_u16b(file, 0x3FFF);
        inv_save(_dragon_pack, file);
    }
    else savefile_write_u16b(file, 0x3FFE);
}

static int _satchel_weight(obj_p p)
{
    if ((!_pack_initialized) || (!_get_pack())) return 0;

    return inv_weight(_dragon_pack, p);
}

static void _dump_satchel(doc_ptr doc)
{
    int laskuri = 0;
    slot_t slot;

    if (!_pack_initialized) return;

    for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
    {
        obj_ptr src = inv_obj(_dragon_pack, slot);
        if (!src) continue;
        laskuri++;
    }

    if (!laskuri) return;

    else
    {
        char o_name[MAX_NLEN];

        doc_insert(doc, "<topic:Satchel>============================ <color:keypress>S</color>hape-Shift Equipment ============================\n\n");
        for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
        {
            object_type *o_ptr = inv_obj(_dragon_pack, slot);
            if (!o_ptr) continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", slot - 1 + 'a', o_name);
            if (((always_dump_origins) || ((final_dump_origins) && ((p_ptr->total_winner) || (p_ptr->is_dead))))
              && (o_ptr->origin_type != ORIGIN_NONE) && (o_ptr->origin_type != ORIGIN_MIXED))
            {
                doc_printf(doc, "    <indent><style:indent><color:W>");
                (void)display_origin(o_ptr, doc);
                doc_printf(doc, "</color></style></indent>\n");
            }
        }
        doc_newline(doc);
    }
}

static void _dump_quests(doc_ptr doc)
{
    int i;
    doc_insert(doc, "<topic:Purple Quests>================================ <color:keypress>P</color>urple Quests ================================\n\n");
    for (i = 0; i < _q_idx; i++)
    {
        int vari = TERM_L_GREEN;
        int day = 0, hour = 0, min = 0;
        monster_race *r_ptr = &r_info[_kt_quests[i].goal_idx];
        if (!r_ptr) continue; /* paranoia */
        if ((_kt_quests[i].start_lev) && (!_kt_quests[i].completed_lev)) vari = TERM_YELLOW;
        else if (_kt_quests[i].killed < _kt_quests[i].goal_ct) vari = TERM_RED;
        if (_kt_quests[i].goal_ct > 1)
        {
            char name[MAX_NLEN];
            strcpy(name, r_name + r_ptr->name);
            plural_aux(name);
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s, Level %d - Kill %d %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_kt_quests[i].dungeon].name, _kt_quests[i].level, _kt_quests[i].goal_ct, name);
        }
        else
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s, Level %d - Kill %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_kt_quests[i].dungeon].name, _kt_quests[i].level, r_name + r_ptr->name);
        switch (vari)
        {
            case TERM_YELLOW:
                doc_printf(doc, "In Progress\n");
                break;
            case TERM_RED:
                extract_day_hour_min_imp(_kt_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "Failed: Day %d, %d:%02d, at CL %d", day, hour, min, _kt_quests[i].completed_lev);
                if (_kt_quests[i].goal_ct > 1) doc_printf(doc, " (%d kills)", _kt_quests[i].killed);
                doc_printf(doc, "\n");
                break;
            default:
                extract_day_hour_min_imp(_kt_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "Completed: Day %d, %d:%02d, at CL %d\n", day, hour, min, _kt_quests[i].completed_lev);
                break;
        }
        doc_printf(doc, "</color></style></indent>\n");
    }
    if (p_ptr->wizard)
    {
        doc_printf(doc, "\n Score: %d\n", _karrot_happy_score());
    }
}

static void _karrot_dump(doc_ptr doc)
{
    if (_pack_initialized) _dump_satchel(doc);
    if (_q_idx) _dump_quests(doc);
    {
        spell_info spells[MAX_SPELLS];
        int        ct = _karrot_get_spells(spells, MAX_SPELLS);

        py_display_spells(doc, spells, ct);
    }
}

static void _dragon_breathe_spell(int cmd, variant *res)
{
    int l = p_ptr->lev;
    int puhallus = MIN(600, p_ptr->chp * (25 + l*l*l/2500) / 100);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes fire at your opponent.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, puhallus));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(1, l/2 + l*l*15/2500));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_print("You breathe fire!");
            fire_ball(GF_FIRE, dir, puhallus, -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _dragon_powers[] = {
    { A_CON, {  1,  0, 30, _dragon_breathe_spell}},
    { A_DEX, { 20,  7,  0, dragon_reach_spell}},
    { A_DEX, { 25, 15,  0, dragon_tail_sweep_spell}},
    { A_DEX, { 30, 30,  0, dragon_wing_storm_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _dragon_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _dragon_powers);
}

static void _dragon_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_d = 0;
    int to_h = l*3/5;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 15;
        a.ds = 3 + l / 16;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 100 + l;
        calc_innate_blows(&a, 400);
        a.msg = "You claw.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 10;
        a.ds = 4 + l / 6;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 200 + 2 * l;

        if (p_ptr->lev >= 40)
            calc_innate_blows(&a, 200);
        else if (p_ptr->lev >= 35)
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite.";
        a.name = "Bite";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _dragon_calc_bonuses(void)
{
    int to_a = py_prorata_level(50);
    int ac = 15 + (p_ptr->lev/10)*5;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add_immune(RES_FIRE);
    p_ptr->pspeed += 3;
    p_ptr->skill_dig += 100;
    p_ptr->levitation = TRUE;
    p_ptr->free_act++;
    p_ptr->see_inv++;
}

static void _dragon_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_IM_FIRE);
}

race_t *karrot_dragon_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {
        me.skills.dis = 0;
        me.skills.dev = 5;
        me.skills.sav = 25;
        me.skills.stl = -2;
        me.skills.srh = 8;
        me.skills.fos = 5;
        me.skills.thn = 15;
        me.skills.thb = 0;

        me.infra = 5;
        me.exp = 250;
        me.name = "Dragon";
        me.desc = "";
            
        me.calc_bonuses = _dragon_calc_bonuses;
        me.get_powers = _dragon_get_powers;
        me.get_flags = _dragon_get_flags;
        me.calc_innate_attacks = _dragon_calc_innate_attacks;
        me.stats[A_STR] = 4;
        me.stats[A_INT] = 2;
        me.stats[A_WIS] = 1;
        me.stats[A_DEX] = 0;
        me.stats[A_CON] = 4;
        me.stats[A_CHR] = 3;
        me.equip_template = &b_info[109];
    }
    me.life = 71 + p_ptr->lev;
    
    return &me;
}

class_t *karrot_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 23,  33,  39,   3,  32,  16,  56,  46};
    skills_t xs = {  9,  12,  11,   0,   0,   0,  15,  14};

        me.name = "Karrot";
        me.subdesc = "Karrot is a proud and ambitious Purple, convinced that it is his destiny "
                    "to one day rule the universe; but rather than getting personally involved, "
                    "he has assigned to his Disciples the task of eliminating any potential "
                    "rivals (like the Serpent of Chaos). Being so sure of his greatness, Karrot "
                    "sees little reason to vary his habits and is the only one of the Purples to "
                    "predictably teach all of his Disciples the same spells. Karrot occasionally "
                    "assigns side-quests to his Disciples, with the dual aim of testing their "
                    "mettle and removing lesser threats to his supremacy. These side-quests only "
                    "appear on fairly challenging levels; they can be avoided by sticking to "
                    "safer areas, but this will not make Karrot happy...\n\n"
                    "Karrot considers many artifacts to be his rightful personal property, and "
                    "will claim them immediately if you should find them. He will, however, give "
                    "you another artifact as a reward for your discovery. These replacement "
                    "artifacts tend to start out as fairly weak, but eventually become very "
                    "strong as you gain Karrot's respect by leveling up and by successfully "
                    "completing side-quests. If you're really lucky and do a particularly good job, "
                    "you might even receive the ultimate gift - Urog, Karrot's legendary blade of "
                    "shadows!\n\n"
                    "Disciples of Karrot tend to play as fighter-mages, relying on melee, spells "
                    "and devices alike. They are unique among Disciples in not having a mana pool; "
                    "casting a spell takes a toll on their hit points instead. They are also the only "
                    "Disciples to have a charming spell - but the greatest of Karrot's followers "
                    "are rumoured to eventually learn even rarer abilities...\n"
                    "Karrot's disciples depend on Intelligence to make their spells work.";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 95;
        me.base_hp = 14;
        me.exp = 125;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells = _karrot_get_spells;
        me.load_player = _karrot_load;
        me.save_player = _karrot_save;
        me.calc_extra_weight = _satchel_weight;
        me.character_dump = _karrot_dump;
        init = TRUE;
    }
    me.bonus_pack = _get_pack();

    return &me;
}

