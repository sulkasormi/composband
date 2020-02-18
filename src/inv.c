#include "angband.h"

#include <assert.h>

#define _FILTER 0x01
struct inv_s
{
    cptr    name;
    int     type;
    int     max;
    int     flags;
    vec_ptr objects; /* sparse ... grows as needed (up to max+1 if max is set) */
};

/* Slots: We are assuming slot <= 26 */
char slot_label(slot_t slot)
{
    assert(slot <= 26);
    if (slot) return slot - 1 + 'a';
    return ' ';
}

slot_t label_slot(char label)
{
    assert('a' <= label && label <= 'z');
    return label - 'a' + 1;
}

/* Helpers */
static void _grow(inv_ptr inv, slot_t slot)
{
    slot_t i;
    assert(slot);
    assert(!inv->max || slot <= inv->max);
    if (slot >= vec_length(inv->objects))
    {
        for (i = vec_length(inv->objects); i <= slot; i++)
            vec_add(inv->objects, NULL);
        assert(slot == vec_length(inv->objects) - 1);
    }
}

/* Creation */
inv_ptr inv_alloc(cptr name, int type, int max)
{
    inv_ptr result = malloc(sizeof(inv_t));
    result->name = name;
    result->type = type;
    result->max = max;
    result->flags = 0;
    result->objects = vec_alloc((vec_free_f)obj_free);
    return result;
}

inv_ptr inv_copy(inv_ptr src)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->name = src->name;
    result->type = src->type;
    result->max = src->max;
    result->flags = src->flags;
    result->objects = vec_alloc((vec_free_f)obj_free);

    for (i = 0; i < vec_length(src->objects); i++)
    {
        obj_ptr obj = vec_get(src->objects, i);
        if (obj)
            vec_add(result->objects, obj_copy(obj));
        else
            vec_add(result->objects, NULL);
    }
    return result;
}

/* Note, a null predicate accepts null objects. This is
 * for INV_EQUIP which is rigidly slot based. Other inventories
 * are lists occupied from [1..N] with valid objects (provided
 * you optimize them). */
static bool _filter(obj_ptr obj, obj_p p)
{
    if (!p) return TRUE;
    if (!obj) return FALSE;
    return p(obj);
}

/* Filtering preserves the slots, but nulls out objects
 * not meeting the predicate. You should never sort a
 * filtered inventory. (cf obj_prompt) */
inv_ptr inv_filter(inv_ptr src, obj_p p)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->name = src->name;
    result->type = src->type;
    result->max = src->max;
    result->flags = src->flags | _FILTER;
    result->objects = vec_alloc(NULL); /* src owns the objects! */

    for (i = 0; i < vec_length(src->objects); i++)
    {
        obj_ptr obj = vec_get(src->objects, i);

        if (_filter(obj, p))
            vec_add(result->objects, obj);
        else
            vec_add(result->objects, NULL);
    }
    return result;
}

/* floor objects form a linked list. There is no slot structure to
 * preserve. This 'fake inventory' is useful for obj_prompt */
inv_ptr inv_filter_floor(point_t loc, obj_p p)
{
    inv_ptr    result = malloc(sizeof(inv_t));
    cave_type *c_ptr = &cave[loc.y][loc.x];
    int        this_o_idx, next_o_idx = 0;

    result->name = "Floor";
    result->type = INV_FLOOR;
    result->max = 0;
    result->flags = _FILTER;
    result->objects = vec_alloc(NULL); /* o_list owns the objects! */

    vec_add(result->objects, NULL); /* slot 0 is invalid */

    if (0 && p_ptr->wizard) /* wizards have mighty magicks */
    {
        result->name = "*FLOOR*";
        for (this_o_idx = 0; this_o_idx < max_o_idx; this_o_idx++)
        {
            obj_ptr obj = &o_list[this_o_idx];
            assert(obj);
            if (!obj->k_idx) continue;
            if (_filter(obj, p))
                vec_add(result->objects, obj);
        }
    }
    else
    {
        for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            obj_ptr obj = &o_list[this_o_idx];

            assert(obj);
            assert(obj->k_idx);
            next_o_idx = obj->next_o_idx;
            if (_filter(obj, p))
                vec_add(result->objects, obj);
        }
    }
    return result;
}

void inv_free(inv_ptr inv)
{
    if (inv)
    {
        vec_free(inv->objects);
        inv->objects = NULL;
        inv->name = NULL;
        free(inv);
    }
}

/* Adding, Removing and Sorting
 * Note: We separate adding and combining in order to support
 * the autopicker, statistics gathering, marking objects as
 * 'touched' and other things that higher level clients need. */
static void _add_aux(inv_ptr inv, obj_ptr obj, slot_t slot)
{
    obj_ptr   copy;
    obj_loc_t loc = {0};
    int       ct = obj->number;

    if (inv->type == INV_EQUIP)
        ct = 1;

    if (inv->max) { assert(1 <= slot && slot <= inv->max); }
    assert(!(inv->flags & _FILTER));

    loc.where = inv->type;
    loc.slot = slot;

    copy = obj_copy(obj);
    copy->loc = loc;
    copy->number = ct;
    obj_clear_dun_info(copy);

    if (slot >= vec_length(inv->objects))
        _grow(inv, slot);
    vec_set(inv->objects, slot, copy);
    copy->marked |= OM_DELAYED_MSG;

    obj->number -= ct;
    if (obj->insured)
    {
        obj_dec_insured(obj, ct);
        if (obj->number) obj_dec_insured(copy, obj->insured % 100);
    }
}

slot_t inv_add(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr old = vec_get(inv->objects, slot);
        if (!old)
        {
            _add_aux(inv, obj, slot);
            return slot;
        }
    }
    if (!inv->max || slot <= inv->max)
    {
        _add_aux(inv, obj, slot);
        return slot;
    }
    return 0;
}

void inv_add_at(inv_ptr inv, obj_ptr obj, slot_t slot)
{
    _add_aux(inv, obj, slot);
}

/* This version of combine will place all of obj->number
 * into a single slot, combining only if there is room. */
slot_t inv_combine(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;

    assert(obj->number);
    assert(!(inv->flags & _FILTER));

    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if ( obj_can_combine(dest, obj, inv->type)
          && dest->number + obj->number <= OBJ_STACK_MAX )
        {
            obj_combine(dest, obj, inv->type);
            dest->marked |= OM_DELAYED_MSG;
            return slot;
        }
    }
    assert(obj->number);
    return 0;
}

bool inv_can_combine(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;

    assert(obj->number);
    assert(!(inv->flags & _FILTER));

    /* Assume that inv is optimized. This means that if obj
     * could be combined into an existing pile, then there
     * is only one possible candidate pile. In other words,
     * if there are multiple piles of the same object, all
     * but one contain 99 items. */
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if ( obj_can_combine(dest, obj, inv->type)
          && dest->number + obj->number <= OBJ_STACK_MAX )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* This version will combine the obj pile (if pile it be)
 * into as many slots as possible, splitting the pile in
 * the process. I think stores need this behaviour. We
 * cannot return a single slot, so we return the number
 * of items in the pile that we found room for. */
int inv_combine_ex(inv_ptr inv, obj_ptr obj)
{
    int    ct = 0;
    slot_t slot;

    assert(obj->number);
    assert(!(inv->flags & _FILTER));

    /* combine obj with as many existing slots as possible */
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, inv->type))
        {
            int n = obj_combine(dest, obj, inv->type);
            ct += n;
            if (n) dest->marked |= OM_DELAYED_MSG;
            if (!obj->number) break;
        }
    }
    return ct;
}

bool inv_optimize(inv_ptr inv)
{
    slot_t slot, seek;
    bool result = FALSE;
    assert(!(inv->flags & _FILTER));
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if (!dest->number)
        {
            vec_set(inv->objects, slot, NULL); /* free */
            dest = NULL;
            result = TRUE;
            continue;
        }
        for (seek = slot + 1; seek < vec_length(inv->objects); seek++)
        {
            obj_ptr src = vec_get(inv->objects, seek);
            if (!src) continue;
            if (obj_combine(dest, src, inv->type))
            {
                result = TRUE;
                if (!src->number)
                    vec_set(inv->objects, seek, NULL);
            }
        }
    }
    if (inv_sort(inv)) result = TRUE;
    return result;
}

void inv_remove(inv_ptr inv, slot_t slot)
{
    obj_ptr obj;

    assert(slot);
    obj = inv_obj(inv, slot);
    if (obj)
        vec_set(inv->objects, slot, NULL); /* free */
}

void inv_clear(inv_ptr inv)
{
    vec_clear(inv->objects);
}

bool inv_sort(inv_ptr inv)
{
    return inv_sort_aux(inv, obj_cmp);
}

bool inv_sort_aux(inv_ptr inv, obj_cmp_f f)
{
    int start = 1, stop = vec_length(inv->objects) - 1;
    if (start == stop) return FALSE;
    inv_for_each(inv, obj_clear_scratch);
    if (!vec_is_sorted_range(inv->objects, start, stop, (vec_cmp_f)f))
    {
        slot_t slot;
        vec_sort_range(inv->objects, start, stop, (vec_cmp_f)f);
        /* It is OK to sort a filtered inv, but we better not update
         * locations for objects we do not own! */
        if (!(inv->flags & _FILTER))
        {
            for (slot = start; slot <= stop; slot++)
            {
                obj_ptr obj = vec_get(inv->objects, slot);
                if (obj)
                {
                    obj->loc.slot = slot;
                    assert(obj->loc.where == inv->type);
                }
            }
        }
        return TRUE; /* So clients can notify the player ... */
    }
    return FALSE;
}

void inv_swap(inv_ptr inv, slot_t left, slot_t right)
{
    obj_ptr obj;

    assert(!(inv->flags & _FILTER));

    _grow(inv, MAX(left, right)); /* force allocation of slots */
    vec_swap(inv->objects, left, right);

    obj = vec_get(inv->objects, left);
    if (obj) obj->loc.slot = left;
    obj = vec_get(inv->objects, right);
    if (obj) obj->loc.slot = right;
}

/* Iterating, Searching and Accessing Objects (Predicates are always optional) */
obj_ptr inv_obj(inv_ptr inv, slot_t slot)
{
    assert(slot);
    if (slot >= vec_length(inv->objects))
    {
        assert(!inv->max || slot <= inv->max);
        return NULL;
    }
    return vec_get(inv->objects, slot);
}

slot_t inv_first(inv_ptr inv, obj_p p)
{
    return inv_next(inv, p, 0);
}

slot_t inv_next(inv_ptr inv, obj_p p, slot_t prev_match)
{
    int slot;
    for (slot = prev_match + 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (_filter(obj, p)) return slot;
    }
    return 0;
}

slot_t inv_last(inv_ptr inv, obj_p p)
{
    int slot;
    for (slot = vec_length(inv->objects) - 1; slot > 0; slot--)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (_filter(obj, p)) return slot;
    }
    return 0;
}

slot_t inv_find_art(inv_ptr inv, int which)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && obj->name1 == which) return slot;
    }
    return 0;
}

slot_t inv_find_ego(inv_ptr inv, int which)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && obj->name2 == which) return slot;
    }
    return 0;
}

slot_t inv_find_obj(inv_ptr inv, int tval, int sval)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (!obj) continue;
        if (obj->tval != tval) continue;
        if (sval != SV_ANY && obj->sval != sval) continue;
        return slot;
    }
    return 0;
}

void inv_for_each(inv_ptr inv, obj_f f)
{
    int slot;
    assert(f);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
            f(obj);
    }
}

void inv_for_each_that(inv_ptr inv, obj_f f, obj_p p)
{
    int slot;
    assert(f);
    assert(p);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (_filter(obj, p))
            f(obj);
    }
}

void inv_for_each_slot(inv_ptr inv, slot_f f)
{
    int slot;
    int max = inv->max ? inv->max : vec_length(inv->objects) - 1;
    assert(f);
    for (slot = 1; slot <= max; slot++)
        f(slot);
}

slot_t inv_random_slot(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;

    for (slot = 1; slot <= inv_max(inv); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (_filter(obj, p))
            ct++;
    }

    if (ct)
    {
        int which = randint0(ct);
        for (slot = 1; slot <= inv_max(inv); slot++)
        {
            obj_ptr obj = inv_obj(inv, slot);
            if (_filter(obj, p))
            {
                if (!which) return slot;
                which--;
            }
        }
    }
    return 0;
}

/* Properties of the Entire Inventory */
int inv_weight(inv_ptr inv, obj_p p)
{
    int wgt = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && _filter(obj, p))
            wgt += obj->weight * obj->number;
    }
    return wgt;
}

int inv_count(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && _filter(obj, p))
            ct += obj->number;
    }
    return ct;
}

int inv_count_slots(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (_filter(obj, p))
            ct++;
    }
    return ct;
}

int inv_loc(inv_ptr inv)
{
    return inv->type;
}

int inv_max(inv_ptr inv)
{
    return inv->max ? inv->max : vec_length(inv->objects) - 1;
}

cptr inv_name(inv_ptr inv)
{
    return inv->name;
}

/* Menus and Display
 * It turns out to be more convenient for inv_display() to know
 * about special equipment handling (cf describe_slots and two-handed
 * wielding). */
void inv_display(inv_ptr inv, slot_t start, slot_t stop, obj_p p, doc_ptr doc, int flags)
{
    slot_t slot;
    int    xtra = 0;
    int    max = inv_max(inv);

    if (!stop) stop = max;
    if (stop > max) stop = max;

    if (flags & INV_SHOW_FAIL_RATES)
        xtra = 6;  /* " 98.7%" */
    else if (flags & INV_SHOW_VALUE)
        xtra = 7;
    else if (show_weights)
        xtra = 9;  /* " 123.0 lbs" */

    if (!(flags & (INV_NO_LABELS | INV_SHOW_SLOT)))
        inv_calculate_labels(inv, start, stop, flags);

    doc_insert(doc, "<style:table>");
    for (slot = start; slot <= stop; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);

        if (!_filter(obj, p)) continue;
        if (inv->type == INV_EQUIP && equip_is_empty_two_handed_slot(slot)) continue;

        if (!obj)
        {
            if (flags & INV_SHOW_SLOT)
                doc_printf(doc, " %d)", slot);
            else if (!(flags & INV_NO_LABELS))
                doc_printf(doc, " %c)", slot_label(slot - start + 1));
            doc_insert(doc, " ");
            if (show_item_graph)
                doc_insert(doc, "  ");
            if (inv->type == INV_EQUIP && describe_slots)
                doc_printf(doc, "%-10.10s: ", equip_describe_slot(slot));
            /*doc_insert(doc, "<color:D>Empty</color>\n");*/
            doc_newline(doc);
        }
        else
        {
            char name[MAX_NLEN];
            doc_style_t style = *doc_current_style(doc);
            bool charging = FALSE;

            if ((flags & INV_SHOW_FAIL_RATES) && !obj_is_device(obj) && obj->timeout)
            {
                object_desc(name, obj, 0);
                charging = TRUE;
            }
            else
                object_desc(name, obj, OD_COLOR_CODED);
            if (flags & INV_SHOW_SLOT)
                doc_printf(doc, " %d)", slot);
            else if (!(flags & INV_NO_LABELS))
                doc_printf(doc, " %c)", inv_slot_label(inv, slot));
            doc_insert(doc, " ");
            if (show_item_graph)
            {
                doc_insert_char(doc, object_attr(obj), object_char(obj));
                doc_insert(doc, " ");
            }
            if (inv->type == INV_EQUIP && describe_slots)
                doc_printf(doc, "%-10.10s: ", equip_describe_slot(slot));
            if (xtra)
            {
                style.right = doc_width(doc) - xtra;
                doc_push_style(doc, &style);
            }
            if (charging)
                doc_printf(doc, "<color:D>%s</color>", name);
            else
                doc_printf(doc, "%s", name);
            if (xtra)
                doc_pop_style(doc);
            if (flags & INV_SHOW_FAIL_RATES)
            {
                if ( object_is_aware(obj)
                  && (obj_is_identified_fully(obj) || (obj->known_xtra & OFL_DEVICE_FAIL)) )
                {
                    int fail;
                    if (obj_is_device(obj) || obj->tval == TV_SCROLL)
                        fail = device_calc_fail_rate(obj);
                    else
                    {
                        effect_t effect = obj_get_effect(obj);
                        fail = effect_calc_fail_rate(&effect);
                    }
                    if (fail == 1000)
                        doc_printf(doc, "<tab:%d> %3d%%", doc_width(doc) - xtra, fail/10);
                    else
                        doc_printf(doc, "<tab:%d> %2d.%d%%", doc_width(doc) - xtra, fail/10, fail%10);
                }
            }
            else if (flags & INV_SHOW_VALUE)
            {
                int value = obj_value(obj);
                doc_printf(doc, "<tab:%d> %5d", doc_width(doc) - xtra, value);
            }
            else if (show_weights)
            {
                int wgt = obj->weight * obj->number;
                doc_printf(doc, "<tab:%d> %3d.%d lbs", doc_width(doc) - xtra, wgt/10, wgt%10);
            }
            doc_newline(doc);
        }
    }
    doc_insert(doc, "</style>");
}

char inv_slot_label(inv_ptr inv, slot_t slot)
{
    obj_ptr obj = inv_obj(inv, slot);
    if (obj && obj->scratch)
        return obj->scratch;
    return ' ';
}

slot_t inv_label_slot(inv_ptr inv, char label)
{
    slot_t slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = vec_get(inv->objects, slot);
        if (!obj) continue;
        if (obj->scratch != label) continue;
        return slot;
    }
    return 0;
}

void inv_calculate_labels(inv_ptr inv, slot_t start, slot_t stop, int flags)
{
    slot_t slot;
    int    max = vec_length(inv->objects) - 1;

    if (!stop) stop = max;
    if (stop > max) stop = max;

    /* Clear old labels or old sort data */
    inv_for_each(inv, obj_clear_scratch);
    
    /* Initialize by ordinal */
    for (slot = start; slot <= stop; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
            obj->scratch = slot_label(slot - start + 1);
    }

    /* Inscription overrides don't function is shops */
    if (inv->type == INV_SHOP || inv->type == INV_HOME || (flags & INV_IGNORE_INSCRIPTIONS))
        return;

    /* Override by inscription (e.g. @mf) */
    for (slot = start; slot <= stop; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
        {
            char label = obj_label(obj);
            if (label)
            {
                /* override this label if in use ... */
                slot_t slot2 = inv_label_slot(inv, label);
                if (slot2)
                    inv_obj(inv, slot2)->scratch = ' ';
                /* ... before marking this object */
                obj->scratch = label;
            }
        }
    }

    /* Add new labels to prevent unusable items */ 
    for (slot = start; slot <= stop; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if ((obj) && (obj->scratch == ' '))
        {
            int i;
            unsigned char lowercase[27] = "abcdefghijklmnopqrstuvwxyz";
            for (i = 0; i < 26; i++)
            {
                slot_t slot2 = inv_label_slot(inv, lowercase[i]);
                if (slot2) continue;
                obj->scratch = lowercase[i];     
                break;
            }
        }
    }
}

/* Savefiles */
void inv_load(inv_ptr inv, savefile_ptr file)
{
    int i, ct, slot;
    vec_clear(inv->objects);
    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        obj_ptr obj = malloc(sizeof(object_type));
        object_wipe(obj);

        slot = savefile_read_s32b(file);
        obj_load(obj, file);

        if (slot >= vec_length(inv->objects))
            _grow(inv, slot);
        vec_set(inv->objects, slot, obj);
    }
}

void inv_save(inv_ptr inv, savefile_ptr file)
{
    int ct = inv_count_slots(inv, obj_exists);
    int slot;

    savefile_write_s32b(file, ct);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
        {
            savefile_write_s32b(file, slot);
            obj_save(obj, file);
            ct--;
        }
    }
    assert(ct == 0);
}

