#ifndef INCLUDED_OBJ_H
#define INCLUDED_OBJ_H

/* Module foo should name its types foo_t or foo_info_t.
 * Since I am not up to disentangling types.h, we'll just
 * pretend object_type is obj_t and obj_ptr. */
struct object_type;
typedef struct object_type obj_t, *obj_ptr;

struct object_kind;
typedef struct object_kind obj_kind_t, *obj_kind_ptr;

/* An object function (obj_f) operates/modifies an object.
 * It should never be called with NULL. */
typedef void (*obj_f)(obj_ptr obj);
/* An object predicate (obj_p) tests an object against certain
 * criterion, returning TRUE to accept the object, FALSE otherwise.
 * It too should never be called with NULL. */
typedef bool (*obj_p)(obj_ptr obj);
typedef int  (*obj_cmp_f)(obj_ptr left, obj_ptr right);
typedef bool (*obj_create_f)(obj_ptr obj, u32b mode);

/* Creation */
extern obj_ptr obj_alloc(void);
extern obj_ptr obj_split(obj_ptr obj, int amt);
extern obj_ptr obj_copy(obj_ptr obj);
extern void    obj_free(obj_ptr obj);

#define OBJ_RELEASE_QUIET       0x0001
#define OBJ_RELEASE_ENCHANT     0x0002
#define OBJ_RELEASE_ID          0x0004
#define OBJ_RELEASE_DELAYED_MSG 0x0008
extern void    obj_release(obj_ptr obj, int options);
extern void    gear_notice_id(obj_ptr obj);
extern void    gear_notice_enchant(obj_ptr obj);
extern void    obj_make_pile(obj_ptr obj);

/* Commands (Top Level User Interface) */
extern void obj_destroy_ui(void);
extern void obj_inscribe_ui(void);
extern void obj_uninscribe_ui(void);
extern void obj_inspect_ui(void);
extern void gear_ui(int which);

extern void obj_destroy(obj_ptr obj, int amt);
extern void obj_drop(obj_ptr obj, int amt);
extern void obj_drop_at(obj_ptr obj, int amt, int x, int y, int break_chance);
extern void obj_describe_charges(obj_ptr obj);

/* Predicates */
extern bool obj_can_sense1(obj_ptr obj);
extern bool obj_can_sense2(obj_ptr obj);
extern bool obj_can_shoot(obj_ptr obj);
extern bool obj_exists(obj_ptr obj);
extern bool obj_is_ammo(obj_ptr obj);
extern bool obj_is_armor(obj_ptr obj);
extern bool obj_is_art(obj_ptr obj);
extern bool obj_is_blessed(obj_ptr obj);
extern bool obj_is_book(obj_ptr obj);
extern bool obj_is_device(obj_ptr obj);
extern bool obj_is_ego(obj_ptr obj);
extern bool obj_is_found(obj_ptr obj);
extern bool obj_is_inscribed(obj_ptr obj);
extern bool obj_is_quiver(obj_ptr obj);
extern bool obj_is_readable_book(obj_ptr obj);
extern bool obj_is_rod(obj_ptr obj);
extern bool obj_is_staff(obj_ptr obj);
extern bool obj_is_wand(obj_ptr obj);

extern bool obj_is_shooter(obj_ptr obj);
extern bool obj_is_bow(obj_ptr obj);
extern bool obj_is_sling(obj_ptr obj);
extern bool obj_is_crossbow(obj_ptr obj);
extern bool obj_is_harp(obj_ptr obj);
extern bool obj_is_gun(obj_ptr obj);
extern bool obj_is_fake_bow(obj_ptr obj);

extern bool obj_is_known(obj_ptr obj);
extern bool obj_is_unknown(obj_ptr obj);

/* Sorting */
extern void obj_clear_scratch(obj_ptr obj); /* Call before sorting ... scratch is used to memoize obj_value */
extern int  obj_cmp(obj_ptr left, obj_ptr right);

/* Menus: These helpers handle the inscription hacks (@mw !* !q) */
extern char obj_label(obj_ptr obj);
extern bool obj_confirm_choice(obj_ptr obj);

/* Stacking */
#define OBJ_STACK_MAX 99
extern bool obj_can_combine(obj_ptr dest, obj_ptr obj, int options);
extern int  obj_combine(obj_ptr dest, obj_ptr obj, int options);
extern void obj_delayed_describe(obj_ptr obj);
extern void obj_dec_number(obj_ptr obj, int amount, bool mark);
extern void obj_dec_insured(obj_ptr obj, int amount);
extern void obj_zero(obj_ptr obj);

/* Helpers */
extern void obj_clear_dun_info(obj_ptr obj);

/* Savefiles */
extern void obj_load(obj_ptr obj, savefile_ptr file);
extern void obj_save(obj_ptr obj, savefile_ptr file);

/* Handle special inventories */
extern void special1_drop(obj_ptr obj);
extern void special1_remove(int slot);
extern void special2_drop(obj_ptr obj);
extern void special2_remove(int slot);
#endif
