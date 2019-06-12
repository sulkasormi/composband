#ifndef INCLUDED_SHOP_H
#define INCLUDED_SHOP_H

#include "inv.h"

extern bool store_hack;

/************************************************************************
 * Shops
 ***********************************************************************/
struct shop_s;
typedef struct shop_s shop_t, *shop_ptr;

enum
{
    SHOP_NONE = -1, /* TODO: f_info.txt */
    SHOP_GENERAL = 0,
    SHOP_ARMORY,
    SHOP_WEAPON,
    SHOP_TEMPLE,
    SHOP_ALCHEMIST,
    SHOP_MAGIC,
    SHOP_BLACK_MARKET,
    SHOP_HOME,
    SHOP_BOOK,
    SHOP_MUSEUM,
    SHOP_JEWELER,
	SHOP_SHROOMERY,
	SHOP_DRAGON
};

extern shop_ptr shop_alloc(int which);
extern shop_ptr shop_load(savefile_ptr file);
extern void     shop_free(shop_ptr shop);

extern void     shop_ui(shop_ptr shop);

extern void     shop_reset(shop_ptr shop); /* for dungeon shops */
extern bool     shop_common_cmd_handler(int cmd); /* shared with home_ui */
extern void     shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size);
extern void     shop_save(shop_ptr shop, savefile_ptr file);


/************************************************************************
 * Towns
 ***********************************************************************/
struct town_s;
typedef struct town_s town_t, *town_ptr;


/* Note: If you add towns, also update NO_TOWN and SECRET_TOWN in defines.h */
enum
{
    TOWN_OUTPOST = 1,
    TOWN_TELMORA,
    TOWN_MORIVANT,
    TOWN_ANGWIL,
	TOWN_ANAMBAR,
	TOWN_THALOS,
    TOWN_ZUL,
    TOWN_RANDOM,
    TOWN_MIN = TOWN_OUTPOST,
    TOWN_MAX = TOWN_RANDOM,
    TOWN_MAX_STD = TOWN_ZUL,
    /* XXX This may get redone at some point. See the q_info.txt changes
     * for my thoughts ... */
};

#define TOWN_BIRTH TOWN_OUTPOST

extern void     towns_init(void);

extern town_ptr towns_current_town(void);
extern town_ptr towns_get_town(int which);
extern void     towns_save(savefile_ptr file);
extern void     towns_load(savefile_ptr file);
extern void     towns_on_turn_overflow(int rollback_turns);

extern void     towns_init_buildings(void);
extern room_ptr towns_get_map(void);
                /* Comment: atm, t_info.txt uses $TOWN to load the correct
                 * file (p_ptr->town_num). This should change to town_get_map(town_ptr)
                 * in my opinion. Also, atm, buildings are global and stored outside
                 * of the town but are initialized when the appropriate t_*.txt file
                 * is parsed. I think town_t should own buildings as well as shops (later) */

extern shop_ptr town_get_shop(town_ptr town, int which);

extern void     town_on_visit(int which);
extern bool     town_visited(int which);
extern cptr     town_name(int which);

extern int      town_service_price(int price);

extern void     birth_shop_items(void);
#endif
