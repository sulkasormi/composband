#ifndef INCLUDED_OBJ_PROMPT_H
#define INCLUDED_OBJ_PROMPT_H

#include "obj.h"

/* Customization */
struct obj_prompt_context_s;
typedef struct obj_prompt_context_s obj_prompt_context_t, *obj_prompt_context_ptr;
typedef int (*obj_prompt_cmd_f)(obj_prompt_context_ptr context, int cmd);
enum {
    OP_CMD_SKIPPED = 0,
      /* prompt->cmd_handler did not process the cmd */

    OP_CMD_HANDLED,
      /* prompt->cmd_handler processed the cmd. The dialog
       * should continue so the user can select an object. */

    OP_CMD_DISMISS
      /* prompt->cmd_handler not only processed the cmd, but
       * obj_prompt should now dismiss the dialog with OP_CUSTOM.
       * In this case the handler should fill in context->prompt->custom
       * and perhaps context->prompt->obj as well. Its really up to you. */
};

/* Parameters to obj_prompt */
#define MAX_LOC 7
struct obj_prompt_s
{
    /* input */
    cptr    prompt;         /* "Wear/Wield which item?" */
    cptr    error;          /* "You have nothing you can wear or wield." */
    cptr    help;           /* generic help topic if not set */
    obj_p   filter; 
    int     where[MAX_LOC]; /* INV_EQUIP, INV_FLOOR, etc. order matters */
    int     flags;          /* INV_SHOW_FAIL_RATES, etc. */
    int     top_loc;

    /* customize */
    obj_prompt_cmd_f
            cmd_handler;

    /* output */
    obj_ptr obj;
    vptr    custom;
};
typedef struct obj_prompt_s obj_prompt_t, *obj_prompt_ptr;

/* Context for Customization */
struct obj_prompt_tab_s
{
    inv_ptr inv;
    int     ct;
    int     page_ct;
    int     page;
};
typedef struct obj_prompt_tab_s obj_prompt_tab_t, *obj_prompt_tab_ptr;

struct obj_prompt_context_s
{
    obj_prompt_ptr prompt;
    vec_ptr        tabs;
    int            tab;
    doc_ptr        doc;
    int            page_size;
};

/* Prompt for an object, but we are very customizable.
 * Usually, the return is OP_SUCCESS and prompt->obj holds 
 * the selected object. Most clients can just check
 * if (prompt.obj) to see if the user selected an object
 * and just ignore the return code. */
extern int obj_prompt(obj_prompt_ptr prompt);
extern void obj_prompt_add_special_packs(obj_prompt_ptr prompt);
enum {
    OP_NO_OBJECTS = 1,
      /* No objects meet prompt->filter in the various prompt->where locations.
       * prompt->obj is set to NULL */

    OP_CANCELED,
      /* User canceled the prompt. prompt->obj is NULL */

    OP_SUCCESS,
      /* User chose a single object which is now in prompt->obj.
       * prompt->obj->loc describes where it came from. Be sure
       * to obj_release(prompt->obj) if you move it or use it. */

    OP_CUSTOM
      /* prompt->cmd_handler handled a cmd with OP_CMD_DISMISS.
       * prompt->obj is probably NULL and prompt->custom is probably not.
       * The cmd_handler fills in these fields as needed prior to
       * dismissing the dialog.
       *
       * Why have this?
       * [1] Option to select all (*) for Identify, Get, etc. All
       *     may apply to a single location (context->tab) or to
       *     all valid locations (context->tabs). You decide.
       * [2] Force-trainers select the Force (F) when choosing spellbooks.
       * [3] Bowmasters select Unlimited Quiver (U) when choosing ammo.
       * etc. */
};

#endif
