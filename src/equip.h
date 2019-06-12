#ifndef EQUIP_H
#define EQUIP_H

#include "inv.h"

/* Equipment Module.

   The goal is to support different kinds of equipment "templates".
   For example, a "hydra" player race would have multiple heads and
   should allow one amulet and helmet per head. An "insect" player
   race might allow multiple pairs of boots, but body armor is 
   probably out. Dragons get multiple ring slots but not much else
   (cf Drangband which was always good fun, back in the day).

   Slots are [1..equip_max()]

   NULL predicates are always OK and mean that no restriction should be applied.
*/

/* This is the maximum possible value of equip_max() ... It is needed
 * by code that creates fixed size arrays at compile time, such as 
 * _flagzilla_t in py_display.c or b_info's equip_template_t. *YOU* should
 * not be using this, however. */
#define EQUIP_MAX 15

enum {
    EQUIP_SLOT_NONE,
    EQUIP_SLOT_GLOVES,
    EQUIP_SLOT_WEAPON_SHIELD,
    EQUIP_SLOT_RING,
    EQUIP_SLOT_BOW,
    EQUIP_SLOT_AMULET,
    EQUIP_SLOT_LITE,
    EQUIP_SLOT_BODY_ARMOR,
    EQUIP_SLOT_CLOAK,
    EQUIP_SLOT_BOOTS,
    EQUIP_SLOT_HELMET,
    EQUIP_SLOT_ANY,
    EQUIP_SLOT_WEAPON,
    EQUIP_SLOT_CAPTURE_BALL,
    EQUIP_SLOT_QUIVER,
    EQUIP_SLOT_MAX
};

extern void    equip_ui(void);
extern void    equip_display(doc_ptr doc, obj_p p, int flags);

/* Adding and Removing Equipment */
extern void equip_wield_ui(void); 
    extern void equip_wield(obj_ptr obj, slot_t slot);

extern bool equip_can_takeoff(obj_ptr obj);
extern void equip_takeoff_ui(void);
    /* Taking off an item implies moving it to the pack */

extern void    equip_drop(obj_ptr obj);
    /* helper for do_cmd_drop */

extern void    equip_remove(slot_t slot);
    /* helper fn ... try equip_takeoff instead */

extern void    equip_calc_bonuses(void);
extern void    equip_xtra_might(int pval);
extern inv_ptr equip_filter(obj_p p);
extern void    equip_for_each(obj_f f);
extern void    equip_for_each_that(obj_f f, obj_p p);
extern void    equip_for_each_slot(slot_f f);
extern int     equip_max(void);
extern int     equip_count_used(void);
extern bool    equip_is_valid_slot(slot_t slot);
extern bool    equip_verify_slot(slot_t slot, obj_ptr obj);
extern slot_t  equip_first_slot(obj_ptr obj);
extern slot_t  equip_first_empty_slot(obj_ptr obj);
extern slot_t  equip_next_slot(obj_ptr obj, slot_t last);
extern slot_t  equip_find_first(obj_p p);
extern slot_t  equip_find_next(obj_p p, slot_t prev_match);
extern slot_t  equip_find_art(int which);
extern slot_t  equip_find_ego(int which);
extern slot_t  equip_find_empty_hand(void);
extern slot_t  equip_find_obj(int tval, int sval);
extern slot_t  equip_random_slot(obj_p p);
extern int     equip_slot_type(slot_t slot);
extern cptr    equip_describe_slot(slot_t slot);
extern slot_t  equip_is_worn(obj_ptr obj); /* Hack for sloppy code ... */
extern int     equip_which_hand(obj_ptr obj); /* Hack for sloppy code ... */
extern obj_ptr equip_obj(slot_t slot);
extern int     equip_weight(obj_p p);
extern void    equip_init(void);
extern void    equip_on_load(void);
extern void    equip_on_change_race(void);
extern bool    equip_can_wield_kind(int tval, int sval);
extern bool    equip_has_slot_type(int type);

extern bool    equip_is_empty_hand(int hand);
extern bool    equip_is_valid_hand(int hand);
extern bool    equip_is_empty_two_handed_slot(slot_t slot);

extern void    equip_learn_curse(int flag);
extern void    equip_learn_resist(int obj_flag); /* pass OF_RES_FIRE rather than RES_FIRE */
extern void    equip_learn_vuln(int obj_flag);
extern void    equip_learn_flag(int obj_flag);
extern void    equip_learn_slay(int slay_flag, cptr msg);

extern void    equip_load(savefile_ptr file);
extern void    equip_save(savefile_ptr file);
extern inv_ptr get_equipment(void);

extern void    ring_finger_swap_ui(slot_t f1, slot_t f2);

#endif
