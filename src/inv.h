#ifndef INCLUDED_INV_H
#define INCLUDED_INV_H

#include "obj.h"

/* Helper Module for Managing Object Inventories (inv)
 * Used by equip, pack and quiver modules. Could also
 * be used for shop inventories.
 *
 * Object Ownership: Any objects added to the inventory
 * are copied. Clients own the original while this module
 * owns the copies. Filtered inventories must outlive
 * the parent (source) inventory.
 *
 * You should know that a NULL predicate accepts a NULL
 * object. If you don't like this, then pass obj_exists.
 * For inv_filter*, we always return an inventory with
 * the same slot structure as the source no matter the
 * predicate (but non-matching slots will be NULL in the
 * filtered inventory).
 */

typedef int slot_t; /* Slots are 1..max ('if (slot) ...' is a valid idiom) */
                    /* Slots may be empty (unused) */
typedef void (*slot_f)(slot_t slot);

extern char    slot_label(slot_t slot);
extern slot_t  label_slot(char label);

typedef struct inv_s inv_t, *inv_ptr; /* Hidden/Abstract */

#define INV_EQUIP     1
#define INV_PACK      2
#define INV_QUIVER    3
#define INV_SHOP      4
#define INV_FLOOR     5
#define INV_HOME      6
#define INV_MUSEUM    7
#define INV_TMP_ALLOC 8  /* Hack so obj_release knows to free() temp objs */
#define INV_SPECIAL1  9
#define INV_SPECIAL2  10
#define INV_SPECIAL3  11

/* Creation ... be sure to inv_free() any returned inv_ptr */
extern inv_ptr inv_alloc(cptr name, int type, int max);
extern inv_ptr inv_copy(inv_ptr src);
extern inv_ptr inv_filter(inv_ptr src, obj_p p);
extern inv_ptr inv_filter_floor(point_t loc, obj_p p);     /* player's current tile */
extern void    inv_free(inv_ptr inv);

/* Adding, Removing and Sorting */
extern slot_t  inv_add(inv_ptr inv, obj_ptr obj);
extern void    inv_add_at(inv_ptr inv, obj_ptr obj, slot_t slot);
extern slot_t  inv_combine(inv_ptr inv, obj_ptr obj);
extern int     inv_combine_ex(inv_ptr inv, obj_ptr obj);
extern bool    inv_can_combine(inv_ptr inv, obj_ptr obj);
extern void    inv_remove(inv_ptr inv, slot_t slot);
extern void    inv_clear(inv_ptr inv);
extern bool    inv_optimize(inv_ptr inv);
extern bool    inv_sort(inv_ptr inv);
extern bool    inv_sort_aux(inv_ptr inv, obj_cmp_f f);
extern void    inv_swap(inv_ptr inv, slot_t left, slot_t right);

/* Iterating, Searching and Accessing Objects (Predicates are always optional) */
extern obj_ptr inv_obj(inv_ptr inv, slot_t slot); /* NULL if slot is not occupied */
extern slot_t  inv_first(inv_ptr inv, obj_p p);
extern slot_t  inv_next(inv_ptr inv, obj_p p, slot_t prev_match); /* Begins new search at prev_match + 1 */
extern slot_t  inv_last(inv_ptr inv, obj_p p);
extern slot_t  inv_find_art(inv_ptr inv, int which); /* equip module wants to know if a certain artifact is being worn */
extern slot_t  inv_find_ego(inv_ptr inv, int which);
extern slot_t  inv_find_obj(inv_ptr inv, int tval, int sval);
extern void    inv_for_each(inv_ptr inv, obj_f f); /* apply f to each non-null object */
extern void    inv_for_each_that(inv_ptr inv, obj_f f, obj_p p); /* apply f to each object that p accepts */
extern void    inv_for_each_slot(inv_ptr inv, slot_f f); /* apply f to all slots, empty or not */
extern slot_t  inv_random_slot(inv_ptr inv, obj_p p); /* used for disenchantment, cursing, rusting, inventory damage, etc */

/* Properties of the Entire Inventory */
extern int     inv_weight(inv_ptr inv, obj_p p); /* Pass NULL for total weight */
extern int     inv_count(inv_ptr inv, obj_p p); /* Sum(obj->number) for all non-null objects p accepts */
extern int     inv_count_slots(inv_ptr inv, obj_p p); /* Sum(1) for all (possibly null) objects p accepts */
extern int     inv_loc(inv_ptr inv);
extern int     inv_max(inv_ptr inv);
extern cptr    inv_name(inv_ptr inv);

/* Menus and Display */
/* inv_display is a bit overwhelming, but it is a low level helper for pack_display,
 * etc. Basically, the requested range in inv is displayed to doc, but the range
 * might be filtered by a predicate.
 * Labels are calculated by inv_display which properly handles all the inscription
 * coding (e.g. @mh to label an object 'h' for the 'm' command, etc.).
 * We respect show_weights (provided flags don't override) as well as show_item_graph */
#define INV_SHOW_FAIL_RATES     0x0001
#define INV_SHOW_VALUE          0x0002
#define INV_IGNORE_INSCRIPTIONS 0x0004
#define INV_NO_LABELS           0x0008
#define INV_SHOW_SLOT           0x0010
extern void inv_display(
    /* What we display */
    inv_ptr inv,
    slot_t start, slot_t stop, /* Range to display: stores diplay one page at a time.*/
    obj_p p,                   /* NULL shows all slots, even empty ones. Otherwise, p must match an existing object */
    /* Where and how we display it */
    doc_ptr doc,
    int flags                  /* TODO: Display Fail Rates or Object Values ... */
);
extern char    inv_slot_label(inv_ptr inv, slot_t slot);
extern slot_t  inv_label_slot(inv_ptr inv, char label);
/* Normally, you don't need to call this since you will usually display()
 * before inspecting labels. Except for REPEAT_PULL() ... sigh */
extern void    inv_calculate_labels(inv_ptr inv, slot_t start, slot_t stop, int flags);

/* Savefiles */
extern void    inv_load(inv_ptr inv, savefile_ptr file);
extern void    inv_save(inv_ptr inv, savefile_ptr file);

#endif
