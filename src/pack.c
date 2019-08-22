#include "angband.h"

#include <assert.h>

static inv_ptr _inv = NULL;
static vec_ptr _overflow = NULL;
static int     _lock = 0;

void pack_init(void)
{
    inv_free(_inv);
    vec_free(_overflow);

    _inv = inv_alloc("Inventory", INV_PACK, pack_max());
    _overflow = vec_alloc(free);
}

void pack_lock(void)
{
    _lock++;
}

void pack_unlock(void)
{
    assert(_lock > 0);
    _lock--;
}

void pack_ui(void)
{
    gear_ui(INV_PACK);
}

void pack_display(doc_ptr doc, obj_p p, int flags)
{
    inv_display(_inv, 1, pack_max(), p, doc, flags);
}

/* Adding and removing */
static void pack_push_overflow(obj_ptr obj);
void pack_carry(obj_ptr obj)
{
    /* Carrying an object is rather complex, and the pile,
     * if pile it be, may distribute between various quiver
     * and pack slots. It may consume new slots. We must handle
     * statistics and the autopicker. It's easiest if all this
     * logic is here, in one place. And it simplifies things
     * to omit checking for a full pack in client code. Instead
     * the pack will just overflow as needed. Perhaps this is
     * a bit comical if the player gets a large floor pile? */
    object_mitze(obj, MITZE_PICKUP);
    if ((!obj) || (!obj->k_idx)) return;
    stats_on_pickup(obj);
    if (quiver_likes(obj))
        quiver_carry(obj);
    if (obj->number)
        pack_carry_aux(obj);
}
void pack_carry_aux(obj_ptr obj)
{
    if (obj->number)
        inv_combine_ex(_inv, obj);
    if (obj->number)
    {
        slot_t slot = inv_add(_inv, obj);
        if (slot)
        {
            obj_ptr new_obj = inv_obj(_inv, slot);
            new_obj->marked |= OM_TOUCHED;
            autopick_alter_obj(new_obj, FALSE);
            p_ptr->notice |= PN_OPTIMIZE_PACK;
        }
    }
    if (obj->number)
        pack_push_overflow(obj);
    p_ptr->update |= PU_BONUS; /* Weight changed */
    p_ptr->window |= PW_INVEN;
    p_ptr->notice |= PN_CARRY;
}

/* Helper for pack_get_floor ... probably s/b private but the autopicker needs it */
void pack_get(obj_ptr obj)
{
    char     name[MAX_NLEN];
    class_t *class_ptr = get_class();

    object_desc(name, obj, OD_COLOR_CODED);

    if (obj->tval == TV_GOLD)
    {
        int value = obj->pval;

        if ((p_ptr->prace == RACE_WEREWOLF) && (strpos("silver", name)))
        {
            msg_print("You carefully avoid touching a pile of silver coins.");
        }
        else
        {
            msg_format("You collect %d gold pieces' worth of %s.",
               value, name);

            sound(SOUND_SELL);

            p_ptr->au += value;
            stats_on_gold_find(value);

            p_ptr->redraw |= PR_GOLD;

            if (prace_is_(RACE_MON_LEPRECHAUN))
                p_ptr->update |= PU_BONUS | PU_HP | PU_MANA;
        }
        obj->number = 0;
    }
    else
    {
        if (class_ptr->get_object)
            class_ptr->get_object(obj);

        /*msg_format("You get %s.", name);*/

        quests_on_get_obj(obj);
        pack_carry(obj);
    }
    obj_release(obj, OBJ_RELEASE_QUIET);
}

static int _get_cmd_handler(obj_prompt_context_ptr context, int cmd)
{
    obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
    if (cmd == '*')
    {
        inv_for_each(tab->inv, pack_get);
        return OP_CMD_DISMISS;
    }
    else
    {
        slot_t slot = inv_label_slot(tab->inv, cmd);
        if (slot)
        {
            obj_ptr obj = inv_obj(tab->inv, slot);
            pack_get(obj);
            inv_remove(tab->inv, slot);
            if (!inv_count_slots(tab->inv, obj_exists))
                return OP_CMD_DISMISS;
            return OP_CMD_HANDLED;
        }
    }
    return OP_CMD_SKIPPED;
}

static bool _get_floor(inv_ptr floor)
{
    int          ct = inv_count_slots(floor, obj_exists);
    obj_prompt_t prompt = {0};

    /* Autopicker cleared 'em all? */
    if (!ct) return TRUE;

    /* Autoget a single floor object */
    if (ct == 1) 
    {
        pack_get(inv_obj(floor, 1));
        return TRUE;
    }

    /* Prompt user for multiple floor objects */
    msg_print(NULL); /* Clear Autopicker Msg Spam */
    prompt.prompt = "Get which item (<color:keypress>*</color> for all)? ";
    prompt.where[0] = INV_FLOOR;
    prompt.cmd_handler = _get_cmd_handler;

    switch (obj_prompt(&prompt))
    {
    case OP_CUSTOM:
    case OP_SUCCESS:
        return TRUE;
    }
    return FALSE;
}

static bool _obj_not_autoleave(object_type *o_ptr)
{
    int idx;
    if ((!o_ptr) || (!o_ptr->k_idx)) return FALSE;
    idx = is_autopick(o_ptr);
    if ((idx >= 0) && (autopick_list[idx].action & DO_AUTODESTROY))
    {
        obj_release(o_ptr, 0);
        return FALSE;
    }
    return TRUE;
}

bool pack_get_floor(void)
{
    bool    result = FALSE;
    inv_ptr floor;

    if ((delay_autopick) && (!delay_autopick_hack)) delay_autopick_hack = 1;

    autopick_get_floor(); /* no energy charge */

    floor = inv_filter_floor(point(px, py), ((leave_mogaminator) && (delay_autopick_hack < 2)) ? _obj_not_autoleave : NULL);
    result = _get_floor(floor);

    if (delay_autopick_hack)
    {
        delay_autopick_hack = 0;
        autopick_get_floor();
    }
    
    inv_free(floor);

    return result;
}

void pack_drop(obj_ptr obj)
{
    int  amt = obj->number;

    assert(obj);
    assert(obj->loc.where == INV_PACK);
    assert(obj->number > 0);

    if (obj->number > 1)
    {
        amt = get_quantity(NULL, obj->number);
        if (amt <= 0)
        {
            energy_use = 0;
            return;
        }
    }

    obj_drop(obj, amt);
}

void pack_describe(obj_ptr obj)
{
    char name[MAX_NLEN];

    assert(obj);
    assert(obj->loc.where == INV_PACK);
    assert(1 <= obj->loc.slot && obj->loc.slot <= 26);

    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You have %s (%c).", name, slot_label(obj->loc.slot));
}

void pack_remove(slot_t slot)
{
    inv_remove(_inv, slot);
    p_ptr->notice |= PN_OPTIMIZE_PACK;
    p_ptr->update |= PU_BONUS;
    p_ptr->window |= PW_INVEN;
}

/* Accessing, Iterating, Searching */
obj_ptr pack_obj(slot_t slot)
{
    return inv_obj(_inv, slot);
}

int pack_max(void)
{
    int vahennys = 0;
    if (p_ptr->pclass == CLASS_ALCHEMIST) vahennys += 3; /* infusion space */
    if (p_ptr->prace == RACE_WEREWOLF) vahennys += 1; /* werewolf pack */
    if (p_ptr->prace == RACE_IGOR) vahennys += 1; /* cold storage */
    return PACK_MAX - vahennys;
}

inv_ptr pack_filter(obj_p p)
{
    return inv_filter(_inv, p);
}

void pack_for_each(obj_f f)
{
    inv_for_each(_inv, f);
}

void pack_for_each_that(obj_f f, obj_p p)
{
    inv_for_each_that(_inv, f, p);
}

slot_t pack_find_first(obj_p p)
{
    return inv_first(_inv, p);
}

slot_t pack_find_next(obj_p p, slot_t prev_match)
{
    return inv_next(_inv, p, prev_match);
}

slot_t pack_find_art(int which)
{
    return inv_find_art(_inv, which);
}

slot_t pack_find_ego(int which)
{
    return inv_find_ego(_inv, which);
}

slot_t pack_find_obj(int tval, int sval)
{
    return inv_find_obj(_inv, tval, sval);
}

slot_t pack_find_device(int effect)
{
    int slot;
    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (!obj) continue;
        if (!object_is_device(obj)) continue;
        if (!object_is_known(obj)) continue;
        if (obj->activation.type != effect) continue;
        if (device_sp(obj) < obj->activation.cost) continue;
        return slot;
    }
    return 0;
}

slot_t pack_random_slot(obj_p p)
{
    return inv_random_slot(_inv, p);
}

/* Bonuses: A few rare items grant bonuses from the pack. */
void pack_calc_bonuses(void)
{
    slot_t slot;
    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (!obj) continue;
        if (obj->name1 == ART_MAUL_OF_VICE)
            p_ptr->maul_of_vice = TRUE;
        if (obj->rune == RUNE_ELEMENTAL_PROTECTION)
            p_ptr->rune_elem_prot = TRUE;
        if (obj->rune == RUNE_GOOD_FORTUNE)
            p_ptr->good_luck = TRUE;
    }
}

/* Overflow: We run obj thru the autopicker to inscribe
 * and perhaps auto-id. We also clear dun-info before
 * redropping, just to be safe. */
static void pack_push_overflow(obj_ptr obj)
{
    obj_ptr new_obj = obj_copy(obj);
    obj_clear_dun_info(new_obj);
    new_obj->marked |= OM_TOUCHED;
    autopick_alter_obj(new_obj, FALSE);
    vec_push(_overflow, new_obj);
    obj->number = 0;
}

int pack_overflow_count(void)
{
    return vec_length(_overflow);
}

bool pack_overflow(void)
{
    bool result = FALSE;
    char name[MAX_NLEN];

    /* quest reward on a full pack ... the reward forces
     * reinit_wilderness, but we better wait to drop the reward! */
    if (p_ptr->leaving) return FALSE;

    while (vec_length(_overflow))
    {
        obj_ptr obj = vec_pop(_overflow);
        /* Weird case: Wield an item from your full pack. This first
         * removes the item from your equipment, placing it in your
         * full pack (hence, into _overflow). Then it wears the selected
         * object, freeing up an equipment slot. Your pack shouldn't
         * overflow after all! */
        if (!pack_is_full())
        {
            pack_carry_aux(obj);
            free(obj);
            continue;
        }
        if (!result)
        {
            disturb(0, 0);
            msg_boundary();
            cmsg_print(TERM_VIOLET, "Your pack overflows:");
            result = TRUE;
        }
        object_desc(name, obj, OD_COLOR_CODED);
        msg_format("You drop %s.", name);
        drop_near(obj, 0, py, px);
        free(obj);
    }
    if (result)
    {
        notice_stuff();
        handle_stuff();
    }
    return result;
}

/* The pack will 'optimize' upon request, combining objects via
 * stacking and resorting. See PN_REORDER and PN_COMBINE, which
 * I've combined into a single method since it is unclear why 
 * they need to be separate. */
bool pack_optimize(void)
{
    if (_lock)
    {
        /* Try again later ... */
        p_ptr->notice |= PN_OPTIMIZE_PACK;
    }
    else if (inv_optimize(_inv))
    {
        p_ptr->window |= PW_INVEN;
        return TRUE;
    }
    return FALSE;
}
void pack_delayed_describe(void)
{
    if (_lock)
        p_ptr->notice |= PN_CARRY;
    else
    {
        msg_boundary();
        pack_for_each(obj_delayed_describe);
    }
}

/* Properties of the Entire Inventory */
int pack_weight(obj_p p)
{
    return inv_weight(_inv, p);
}

int pack_count(obj_p p)
{
    return inv_count(_inv, p);
}

int pack_count_slots(obj_p p)
{
    return inv_count_slots(_inv, p);
}

bool pack_is_full(void)
{
    return pack_count_slots(obj_exists) == pack_max();
}

/* Savefiles */
void pack_load(savefile_ptr file)
{
    int i, ct;
    inv_load(_inv, file);
    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        obj_ptr obj = obj_alloc();
        obj_load(obj, file);
        vec_add(_overflow, obj);
    }
}

void pack_save(savefile_ptr file)
{
    int i;
    inv_save(_inv, file);
    savefile_write_s16b(file, vec_length(_overflow));
    for (i = 0; i < vec_length(_overflow); i++)
    {
        obj_ptr obj = vec_get(_overflow, i);
        obj_save(obj, file);
    }
}

