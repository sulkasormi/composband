#include "angband.h"

#define WEREWOLF_FORM_HUMAN 0
#define WEREWOLF_FORM_WOLF 1

#define _MAX_PACK_SLOTS 12

static int _werewolf_form = 0;
static inv_ptr _werewolf_pack = NULL;
static bool _pack_initialized = FALSE;
static bool _do_init_pack = FALSE;

static bool _moon_is_full(void);

void _werewolf_pack_init(void)
{
    if ((_pack_initialized) && (!_do_init_pack)) return;
    inv_free(_werewolf_pack);
    _werewolf_pack = inv_alloc("Satchel", INV_SPECIAL1, _MAX_PACK_SLOTS);
    _pack_initialized = TRUE;
    _do_init_pack = FALSE;
}

equip_template_ptr _werewolf_equip_template(int hahmo)
{
    if (hahmo == WEREWOLF_FORM_WOLF) return &b_info[107];
    else return &b_info[0];
}

void _equip_on_change_form(void)
{
    equip_template_ptr old_template = _werewolf_equip_template(1 - _werewolf_form);
    equip_template_ptr new_template = _werewolf_equip_template(_werewolf_form);

    slot_t  slot;
    inv_ptr _equipment = get_equipment();
    inv_ptr temp = inv_copy(_equipment);
    inv_ptr temp2 = inv_copy(_werewolf_pack);

//    inv_clear(_equipment);
    inv_clear(_werewolf_pack);
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
//            equip_wield(src, new_slot);
            inv_add_at(_equipment, src, new_slot);
        else
            inv_add(_werewolf_pack, src);
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

        /* Assume lone shields are in the left hand */
        if ((_werewolf_form == WEREWOLF_FORM_HUMAN) && (slot == 1) && (new_slot == 1) && (src->tval == TV_SHIELD) && (!(weaponmaster_is_(WEAPONMASTER_SHIELDS))) && (!equip_obj(2)))
        {
            obj_ptr tmp_obj = inv_obj(temp2, 2);
            if ((!tmp_obj) || ((tmp_obj->tval != TV_SHIELD) && (!object_is_melee_weapon(tmp_obj)))) new_slot = 2;
        }

        if (new_slot)
            equip_wield(src, new_slot);
        else
            inv_add(_werewolf_pack, src);
        obj_release(src, OBJ_RELEASE_QUIET);
        src->marked &= ~OM_BEING_SHUFFLED;
    }

    inv_free(temp);
    inv_free(temp2);

    p_ptr->update |= PU_BONUS | PU_TORCH | PU_MANA | PU_HP;
    p_ptr->redraw |= PR_EQUIPPY | PR_EFFECTS | PR_STATUS;
    p_ptr->window |= PW_INVEN | PW_EQUIP;
}

/**********************************************************************
 * Werewolf Start
 **********************************************************************/
static void _birth(void)
{
    _do_init_pack = TRUE;
    _werewolf_pack_init();

    _werewolf_form = 0;
    skills_innate_init("Claw", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("Bite", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    py_birth_food();
    py_birth_light();
}

void _werewolf_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_d = py_prorata_level(15);
    int to_h = l/2;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 17;
        a.ds = 3 + l / 21;
        a.to_d += to_d; /* (MIN(to_d, to_d * 5 / 6)); */
        a.to_h += to_h;
        a.to_d += 3;

        a.weight = 100;
        calc_innate_blows(&a, 300);
        a.msg = "You claw.";
        a.name = "Claw";

        if (psion_combat()) psion_combat_innate_blows(&a);
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 23;
        a.ds = 4 + l / 5;
        a.to_d += to_d; /* (MIN(to_d, to_d * 5 / 6)); */
        a.to_h += to_h;
        a.to_d += 3;

        a.weight = 200;
        a.effect[0] = GF_MISSILE;

        calc_innate_blows(&a, 300);
        a.msg = "You bite.";
        a.name = "Bite";
        if (psion_combat()) psion_combat_innate_blows(&a);
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void werewolf_change_shape_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Change Shape");
        break;
    case SPELL_DESC:
        if (_werewolf_form == WEREWOLF_FORM_HUMAN) var_set_string(res, "Assume the shape of a wolf. Changing shape costs one turn.");
        else var_set_string(res, "Assume the shape of a human. Changing shape costs one turn.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->cursed & 0x0000000F)
        {
            msg_print("Your cursed equipment prevents you from changing shape!");
            break;
        }
        if (psion_weapon_graft())
        {
            msg_print("You cannot change shape with a weapon fused to your arm!");
            break;
        }
        _werewolf_form = 1 - _werewolf_form;
        _equip_on_change_form();
        if (_werewolf_form == WEREWOLF_FORM_WOLF) msg_format("You turn into %s wolf!", ((one_in_(8)) && (p_ptr->food < PY_FOOD_ALERT)) ? "a ravenous" : "a");
        else msg_format("You turn into %s human!", ((one_in_(8)) && (p_ptr->food < PY_FOOD_ALERT)) ? "a ravenous" : "a");
        if ((_werewolf_form == WEREWOLF_FORM_HUMAN) && (p_ptr->action == ACTION_STALK)) set_action(ACTION_NONE);
        var_set_bool(res, TRUE);
        handle_stuff();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Hounds: Evolution is tier based with a random choice from each tier.
 **********************************************************************/
static power_info _wolfpowers[] = {
    { A_DEX, {  1,  1, 30, hound_sniff_spell } },
    { A_DEX, { 10,  0,  0, hound_stalk_spell}},
    { A_DEX, { 25, 18, 30, hound_leap_spell}},
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _default_power[] = {
    { A_DEX, {  1,  0,  0, werewolf_change_shape_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static int _get_powers(spell_info* spells, int max) {
    int ct = get_powers_aux(spells, max, _default_power);
    if (_werewolf_form == WEREWOLF_FORM_WOLF)
        ct += get_powers_aux(spells + ct, max - ct, _wolfpowers);
    return ct;
}
static void _calc_bonuses(void) {
    int to_a = py_prorata_level_aux(25, 1, 2, 2);

    p_ptr->regen += 100;
    if (p_ptr->lev >= 30) p_ptr->free_act++;
    if (p_ptr->lev >= 40) p_ptr->hold_life++;
    if (p_ptr->lev >= 45) res_add(RES_POIS);

    if (_werewolf_form == WEREWOLF_FORM_WOLF)
    {
        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
        if (_moon_is_full())
        {
            p_ptr->to_h_m += 10;
        }
        p_ptr->pspeed += 4;
        p_ptr->pspeed += p_ptr->lev / 14;
        p_ptr->skills.stl += p_ptr->lev / 10;
        if (p_ptr->lev >= 5) res_add(RES_DARK);
        if (p_ptr->lev >= 10) p_ptr->esp_orc = TRUE;
        if (p_ptr->lev >= 15) res_add(RES_ACID);
        if (p_ptr->lev >= 20) p_ptr->esp_animal = TRUE;
        if (p_ptr->lev >= 25) res_add(RES_COLD);
        if (p_ptr->lev >= 35) p_ptr->esp_dragon = TRUE;
        if (p_ptr->lev >= 50) p_ptr->esp_living = TRUE;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_REGEN);

    if (p_ptr->lev >= 30) add_flag(flgs, OF_FREE_ACT);
    if (p_ptr->lev >= 40) add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 45) add_flag(flgs, OF_RES_POIS);
        
    if (_werewolf_form == WEREWOLF_FORM_WOLF)
    {
        if (p_ptr->lev >= 5) add_flag(flgs, OF_RES_DARK);
        if (p_ptr->lev >= 10) add_flag(flgs, OF_ESP_ORC);
        if (p_ptr->lev >= 15) add_flag(flgs, OF_RES_ACID);
        if (p_ptr->lev >= 20) add_flag(flgs, OF_ESP_ANIMAL);
        if (p_ptr->lev >= 25) add_flag(flgs, OF_RES_COLD);
        if (p_ptr->lev >= 35) add_flag(flgs, OF_ESP_DRAGON);
        if (p_ptr->lev >= 50) add_flag(flgs, OF_ESP_LIVING);
    }
}

static void _werewolf_save(savefile_ptr file)
{
    inv_save(_werewolf_pack, file);
    savefile_write_byte(file, _werewolf_form);
}

static void _werewolf_load(savefile_ptr file)
{
    _werewolf_pack_init();
    inv_load(_werewolf_pack, file);
    _werewolf_form = savefile_read_byte(file);
}

void werewolf_init(void)
{
    _werewolf_form = 0;
}

#define MOON_NEW 0
#define MOON_CRESCENT_WAXING 1
#define MOON_HALF_WAXING 2
#define MOON_GIBBOUS_WAXING 3
#define MOON_RLYBIG_WAXING 4
#define MOON_FULL 5
#define MOON_WANING 6

byte get_moon_phase(void)
{
    const s32b pituus = TURNS_PER_TICK * TOWN_DAWN;
    int paiva = (game_turn + (pituus / 4)) / pituus + 1;
    switch (paiva % 29)
    {
        case 27:
        case 28:
           return MOON_NEW;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
           return MOON_CRESCENT_WAXING;
        case 5:
        case 6:
        case 7:
           return MOON_HALF_WAXING;
        case 8:
        case 9:
        case 10:
        case 11:
           return MOON_GIBBOUS_WAXING;
        case 12:
           return MOON_RLYBIG_WAXING;
        case 13:
           return MOON_FULL;
        default:
           return MOON_WANING;
    }
}

static bool _moon_is_full(void)
{
    if (get_moon_phase() == MOON_FULL) return TRUE;
    return FALSE;
}

bool werewolf_in_human_form(void)
{
    if (_werewolf_form == WEREWOLF_FORM_WOLF) return FALSE;
    return TRUE;
}

char *werewolf_moon_message(void)
{
    switch (get_moon_phase())
    {
        case MOON_NEW:
            return " The moon is completely dark.";
        case MOON_CRESCENT_WAXING:
            return " The moon is a waxing crescent.";
        case MOON_HALF_WAXING:
            return " There's a waxing half moon.";
        case MOON_GIBBOUS_WAXING:
            return " The moon is gibbous, and waxing. You feel a bit apprehensive...";
        case MOON_RLYBIG_WAXING:
            return " The moon is almost full.";
        case MOON_FULL:
            return " There's a full moon tonight.";
        default:
            return " The moon is waning.";
    }
}

void werewolf_check_midnight(void)
{
    if ((_moon_is_full()) && (_werewolf_form == WEREWOLF_FORM_HUMAN))
    {
        msg_print("You feel the full moon pulling you!");
        cast_spell(werewolf_change_shape_spell);
        energy_use = 100;
    }
}

void werewolf_silver_effect(int power, bool allow_mitigation)
{
    byte noppa;

    /* We should allow mitigation against attacks by silver monsters
     * (our AC protects us from the silver) but not when we voluntarily
     * claw or bite a silver monster */
    if ((allow_mitigation) && (p_ptr->ac > 0))
    {
        power -= (power * p_ptr->ac / 300);
    }
    if (power < 1) return;

    noppa = randint0(10);
    switch (noppa)
    {
        case 0:
        {
            msg_print("The silver burns you!");
            take_hit(DAMAGE_NOESCAPE, MIN(power, 32), "contact with silver");
            break;
        }
        case 1:
        {
            msg_print("The silver hurts you!");
            take_hit(DAMAGE_NOESCAPE, pienempi(randint1(power), 32), "contact with silver");
            break;
        }
        case 2:
        {
            msg_print("The silver saps your energy away!");
            if (p_ptr->csp > 0)
            {
                p_ptr->csp = 0;
                p_ptr->redraw |= (PR_MANA);
                break;
            }
        } /* Fall through */
        case 3:
        {
            int dam = randint1(MIN(40, power));
            if (noppa == 3) msg_print("The silver saps your energy away!");
            p_inc_minislow(MAX(1, dam / 8));
            break;
        }
        case 4:
        {
            if ((!p_ptr->unwell) || (p_ptr->unwell > UNWELL_EFFECTIVE_MAX))
            {
                msg_print("The silver makes you feel ill!");
                set_unwell(UNWELL_EFFECTIVE_MAX, TRUE);
                break;
            }
        } /* Fall through */
        case 5:
        {
            if (!p_ptr->cut) msg_print("The silver tears your scars open!");
            else msg_print("The silver burns your wounds!");
            set_cut(p_ptr->cut + power, FALSE);
            break;
        }
        case 6:
        {
            msg_print("You are poisoned by the silver!");
            set_poisoned(p_ptr->poisoned + power, FALSE);
            break;
        }
        default:
        {
            if (!fear_save_p(power))
                fear_add_p(power);
            break;
        }
    }
}

static void _dump_satchel(doc_ptr doc)
{
    int laskuri = 0;
    slot_t slot;

    /* We need to not just clear the equipment, but also make sure the items
     * aren't tracked as equipped */
    for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
    {
        obj_ptr src = inv_obj(_werewolf_pack, slot);
        if (!src) continue;
        laskuri++;
    }

    if (!laskuri) return;

    else
    {
        char o_name[MAX_NLEN];

        doc_insert(doc, "<topic:Satchel>============================= <color:keypress>S</color>hape-Shift Satchel =============================\n\n");
        for (slot = 1; slot <= _MAX_PACK_SLOTS; slot++)
        {
            object_type *o_ptr = inv_obj(_werewolf_pack, slot);
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

/* Account for the weight of the items in the werewolf pack */
int _werewolf_pack_weight(obj_p p)
{
    /* For new characters, _werewolf_pack_weight() is called before
     * _werewolf_pack_init()... */
    if (!_werewolf_pack) return 0;

    return inv_weight(_werewolf_pack, p);
}

race_t *werewolf_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static byte   init_form = 0;
    static bool   init_moon = FALSE;
    bool          full_moon = _moon_is_full();
    if (init_moon != full_moon) init_form = 1 - _werewolf_form; /* hack */

    if ((!init) || (init_form != _werewolf_form))
    {
        int sign = (_werewolf_form == WEREWOLF_FORM_HUMAN) ? -1 : 1;
        if (!init)
        {
            me.name = "Werewolf";
            me.desc = "Werewolves are fearsome hunters who can assume either human or wolf shape. "
                    "They are exceptionally durable in both forms, and have great powers of "
                    "regeneration, though they are infamously vulnerable to fire and silver, "
                    "and are not particularly good at using magical spells or devices in either "
                    "shape. While in wolf form, werewolves wield no weapons; instead, they "
                    "attack with their teeth and claws. As werewolves wear different equipment "
                    "in human form and wolf form, they use one inventory slot for a special "
                    "satchel in which they keep the items not needed in their current shape.";

            me.infra = 5;
            me.exp = 140;
            me.base_hp = 27;
            me.calc_bonuses = _calc_bonuses;
            me.get_powers = _get_powers;
            me.get_flags = _get_flags;
            me.birth = _birth;
            me.boss_r_idx = MON_CARCHAROTH;
            me.load_player = _werewolf_load;
            me.save_player = _werewolf_save;
            me.character_dump = _dump_satchel;
            me.calc_extra_weight = _werewolf_pack_weight;
            me.flags = RACE_NO_POLY;
        }
        if (_werewolf_form == WEREWOLF_FORM_HUMAN)
        {
            me.stats[A_STR] =  1;
            me.stats[A_INT] =  0;
            me.stats[A_WIS] =  0;
            me.stats[A_DEX] =  0;
            me.stats[A_CON] =  1;
            me.stats[A_CHR] =  0;
            me.life = 103;
            me.shop_adjust = 100;

            me.skills.dis = 0;
            me.skills.dev = -8;
            me.skills.sav = 2;
            me.skills.stl = 1;
            me.skills.srh = 5;
            me.skills.fos = 10;
            me.skills.thn = 2;
            me.skills.thb = 0;
            me.calc_innate_attacks = NULL;
        }
        else
        {
            me.stats[A_STR] =  2;
            me.stats[A_INT] =  -1;
            me.stats[A_WIS] =  -1;
            me.stats[A_DEX] =  0;
            me.stats[A_CON] =  2;
            me.stats[A_CHR] =  -1;
            me.shop_adjust = 120;

            me.life = 105;
            me.skills.dis = -8;
            me.skills.dev = -24;
            me.skills.sav = 2;
            me.skills.stl = 1;
            me.skills.srh = 5;
            me.skills.fos = 10;
            me.skills.thn = 6;
            me.skills.thb = 0;
            me.calc_innate_attacks = _werewolf_calc_innate_attacks;
        }

        if (full_moon)
        {
            me.stats[A_STR] += 1 * sign;
            me.stats[A_INT] += 1 * sign;
            me.stats[A_WIS] += 1 * sign;
            me.stats[A_DEX] += 1 * sign;
            me.stats[A_CON] += 1 * sign;
            me.stats[A_CHR] += 1 * sign;
        }

        init_moon = full_moon;
        init_form = _werewolf_form;
        init = TRUE;
    }

    me.equip_template = _werewolf_equip_template(_werewolf_form);
    me.bonus_pack = _werewolf_pack;

    return &me;
}
