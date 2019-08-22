#include "angband.h"

#include <assert.h>

static inv_ptr _home = NULL;
static inv_ptr _museum = NULL;

void home_init(void)
{
    inv_free(_home);
    inv_free(_museum);

    _home = inv_alloc("Home", INV_HOME, 0);
    _museum = inv_alloc("Museum", INV_MUSEUM, 0);
}

inv_ptr home_filter(obj_p p)
{
    return inv_filter(_home, p);
}

obj_ptr home_obj(slot_t slot)
{
    return inv_obj(_home, slot);
}

int home_max(void)
{
    return inv_max(_home);
}

void home_for_each(obj_f f)
{
    inv_for_each(_home, f);
}

void home_optimize(void)
{
    inv_optimize(_home);
}

void home_carry(obj_ptr obj)
{
    if (obj->number)
        inv_combine_ex(_home, obj);
    if (obj->number)
        inv_add(_home, obj);
}

static void museum_carry(obj_ptr obj)
{
    if (obj->number)
        inv_combine_ex(_museum, obj);
    if (obj->number)
        inv_add(_museum, obj);
}

/************************************************************************
 * Character Sheet (py_display)
 ***********************************************************************/

void home_display(doc_ptr doc, obj_p p, int flags)
{
    inv_ptr inv = inv_filter(_home, obj_exists);
    char    name[MAX_NLEN];
    slot_t  slot;
    slot_t  max = inv_count_slots(inv, obj_exists);

    inv_sort(inv);

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (!obj) continue; /* bug */
        object_desc(name, obj, OD_COLOR_CODED);
        if (!obj->scratch) obj->scratch = obj_value(obj);
        doc_printf(doc, "<color:R>%6d</color> <indent><style:indent>%s</style></indent>\n", obj->scratch, name);
    }
    inv_free(inv);
}

int home_count(obj_p p)
{
    return inv_count(_home, p);
}

int museum_count(obj_p p)
{
    return inv_count(_museum, p);
}

void museum_display(doc_ptr doc, obj_p p, int flags)
{
    slot_t slot;
    slot_t max = inv_last(_museum, obj_exists);
    char   name[MAX_NLEN];

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(_museum, slot);
        if (!obj) continue; /* bug */
        object_desc(name, obj, OD_COLOR_CODED);
        doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", name);
    }
}

/************************************************************************
 * Savefiles
 ***********************************************************************/
void home_load(savefile_ptr file)
{
    inv_load(_home, file);
    inv_load(_museum, file);
}

void home_save(savefile_ptr file)
{
    inv_save(_home, file);
    inv_save(_museum, file);
}

/************************************************************************
 * User Interface
 ***********************************************************************/
struct _ui_context_s
{
    inv_ptr inv;
    slot_t  top;
    int     page_size;
    doc_ptr doc;
};
typedef struct _ui_context_s _ui_context_t, *_ui_context_ptr;

static void _display(_ui_context_ptr context);
static void _drop(_ui_context_ptr context);
static void _remove(_ui_context_ptr context);
static void _examine(_ui_context_ptr context);
static void _get(_ui_context_ptr context);
static void _ui(_ui_context_ptr context);

void home_ui(void)
{
    _ui_context_t context = {0};

    context.inv = _home;
    context.top = 1;

    _ui(&context);
}

void museum_ui(void)
{
    _ui_context_t context = {0};

    context.inv = _museum;
    context.top = 1;

    _ui(&context);
}

static void _ui(_ui_context_ptr context)
{
    forget_lite(); /* resizing the term would redraw the map ... sigh */
    forget_view();
    character_icky = TRUE;

    msg_line_clear();
    msg_line_init(ui_shop_msg_rect());

    Term_clear();
    context->doc = doc_alloc(MIN(80, ui_shop_rect().cx));
    for (;;)
    {
        int    max = inv_last(context->inv, obj_exists);
        rect_t r = ui_shop_rect(); /* recalculate in case resize */
        int    cmd, ct;

        context->page_size = MIN(26, r.cy - 3 - 4);
        if ((context->top - 1) % context->page_size != 0) /* resize?? */
            context->top = 1;

        _display(context);

        cmd = inkey_special(TRUE);
        msg_line_clear();
        msg_boundary(); /* turn_count is unchanging while in home/museum */
        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q') break;
        pack_lock();
        if (!shop_common_cmd_handler(cmd))
        {
            switch (cmd)
            {
            case 'g': case 'b': case 'p':  _get(context); break;
            case 'd': case 's':  _drop(context); break;
            case 'x': _examine(context); break;
            case 'r': _remove(context); break;
            case '?':
                doc_display_help("context_home.txt", inv_loc(context->inv) == INV_MUSEUM ? "Museum" : NULL);
                Term_clear_rect(ui_shop_msg_rect());
                break;
            case SKEY_PGDOWN: case '3': case ' ':
                if (context->top + context->page_size - 1 < max)
                    context->top += context->page_size;
                break;
            case SKEY_PGUP: case '9': case '-':
                if (context->top > context->page_size)
                    context->top -= context->page_size;
                break;
            default:
                if (cmd < 256 && isprint(cmd))
                {
                    msg_format("Unrecognized command: <color:R>%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
                else if (KTRL('A') <= cmd && cmd <= KTRL('Z'))
                {
                    cmd |= 0x40;
                    msg_format("Unrecognized command: <color:R>^%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
            }
            ct = inv_count_slots(context->inv, obj_exists);
            if (ct)
            {
                max = inv_last(context->inv, obj_exists);
                while (context->top > max)
                    context->top -= context->page_size;
                if (context->top < 1) context->top = 1;
            }
        }
        pack_unlock();
        notice_stuff(); /* PW_INVEN and PW_PACK ... */
        handle_stuff(); /* Plus 'C' to view character sheet */
        if ((shop_exit_hack) || (pack_overflow_count() > ((pack_is_full()) ? 0 : 1)))
        {
            if (shop_exit_hack) msg_print("It's time for you to leave!");
            else msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
            msg_print(NULL);
            shop_exit_hack = FALSE;
            break;
        }
    }
    character_icky = FALSE;
    energy_use = 100;
    msg_line_clear();
    msg_line_init(ui_msg_rect());

    Term_clear();
    do_cmd_redraw();

    doc_free(context->doc);
}

static void _display(_ui_context_ptr context)
{
    rect_t  r = ui_shop_rect();
    doc_ptr doc = context->doc;

    doc_clear(doc);
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "%*s<color:G>%s</color>\n\n",
        (doc_width(doc) - 10)/2, "", inv_name(context->inv));

    shop_display_inv(doc, context->inv, context->top, context->page_size);
    
    {
        slot_t max = inv_last(context->inv, obj_exists);
        slot_t bottom = context->top + context->page_size - 1;

        if (context->top > 1 || bottom < max)
        {
            int page_count = (max - 1) / context->page_size + 1;
            int page_current = (context->top - 1) / context->page_size + 1;

            doc_printf(doc, "<color:B>(Page %d of %d)</color>\n", page_current, page_count);
        }
        else
            doc_newline(doc);
    }
    if (inv_loc(context->inv) == INV_HOME)
    {
        doc_insert(doc,
            "<color:keypress>g</color> to get an item. "
            "<color:keypress>d</color> to drop an item. ");
    }
    else
        doc_insert(doc, "<color:keypress>d</color> to donate an item. ");
    
    doc_insert(doc,
        "<color:keypress>x</color> to begin examining items.\n"
        "<color:keypress>r</color> to begin removing (destroying) items.\n"
        "<color:keypress>Esc</color> to exit. "
        "<color:keypress>?</color> for help.");
    doc_insert(doc, "</style>");

    Term_clear_rect(r);
    doc_sync_term(doc,
        doc_range_top_lines(context->doc, r.cy),
        doc_pos_create(r.x, r.y));
}

static void _get_aux(obj_ptr obj)
{
    /*char name[MAX_NLEN];
    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You get %s.", name);*/
    pack_carry(obj);
}

static void _get(_ui_context_ptr context)
{
    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        msg_print("All donations are <color:v>FINAL</color>!");
        return;
    }
    assert(context->inv == _home);
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        int     amt = 1;

        if (!msg_command("<color:y>Get which item <color:w>(<color:keypress>Esc</color> "
                         "to cancel)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        if (obj->number > 1)
        {
            if (!msg_input_num("Quantity", &amt, 1, obj->number)) continue;
        }
        if (amt < obj->number)
        {
            obj_t copy = *obj;
            copy.number = amt;
            obj->number -= amt;
            if (obj->insured)
            {
                int vahennys = MIN(amt, (obj->insured % 100));
                copy.insured = (obj->insured / 100) * 100 + vahennys;
                obj_dec_insured(obj, vahennys);
            }
            _get_aux(&copy);
        }
        else
        {
            _get_aux(obj);
            if (!obj->number)
            {
                inv_remove(context->inv, slot);
                inv_sort(context->inv);
            }
        }
        break;
    }
}

static void _drop_aux(obj_ptr obj, _ui_context_ptr context)
{
    char name[MAX_NLEN];
    if (object_is_(obj, TV_POTION, SV_POTION_BLOOD))
    {
        msg_print("The potion goes sour.");
        obj->sval = SV_POTION_SALT_WATER;
        obj->k_idx = lookup_kind(TV_POTION, SV_POTION_SALT_WATER);
        object_origins(obj, ORIGIN_BLOOD);
        obj->mitze_type = 0;
    }
    object_desc(name, obj, OD_COLOR_CODED);
    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        msg_format("You donate %s.", name);
        museum_carry(obj);
        inv_sort(_museum);
        virtue_add(VIRTUE_SACRIFICE, 1); /* TODO: should depend on obj_value() */
    }
    else
    {
        msg_format("You drop %s.", name);
        home_carry(obj);
        inv_sort(_home);
    }
}

static void _drop(_ui_context_ptr context)
{
    obj_prompt_t prompt = {0};
    int          amt = 1;

    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        prompt.prompt = "Donate which item?";
        prompt.error = "You have nothing to donate.";
    }
    else
    {
        prompt.prompt = "Drop which item?";
        prompt.error = "You have nothing to drop.";
    }
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    obj_prompt_add_special_packs(&prompt);

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    if (prompt.obj->loc.where == INV_EQUIP)
    {
        if (prompt.obj->tval == TV_QUIVER && quiver_count(NULL))
        {
            msg_print("Your quiver still holds ammo. Remove all the ammo from your quiver first.");
            return;
        }
        if (!equip_can_takeoff(prompt.obj)) return;
    }

    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        char       name[MAX_NLEN];
        string_ptr s = string_copy_s("<color:v>Warning:</color> All donations are final! ");
        char       c;

        object_desc(name, prompt.obj, OD_COLOR_CODED);
        string_printf(s, "Really donate %s to the museum? <color:y>[y/n]</color>", name);
        c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
        string_free(s);
        if (c == 'n') return;
    }
    else
        amt = prompt.obj->number;

    if (prompt.obj->number > 1)
    {
        if (!msg_input_num("Quantity", &amt, 1, prompt.obj->number)) return;
    }

    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        /* *identify* here rather than in _drop_aux in case the user splits a pile. */
        no_karrot_hack = TRUE;
        obj_identify_fully(prompt.obj);
        no_karrot_hack = FALSE;
    }

    if (prompt.obj->loc.where == INV_EQUIP)
    {
        char name[MAX_NLEN];
        object_desc(name, prompt.obj, OD_COLOR_CODED);
        msg_format("You are no longer wearing %s.", name);
        p_ptr->update |= PU_BONUS | PU_TORCH | PU_MANA;
        p_ptr->redraw |= PR_EQUIPPY;
        p_ptr->window |= PW_EQUIP;        
    }

    if (amt < prompt.obj->number)
    {
        obj_t copy = *prompt.obj;
        copy.number = amt;
        prompt.obj->number -= amt;
        if (prompt.obj->insured)
        {
            copy.insured = 0;
            if ((prompt.obj->insured % 100) > prompt.obj->number)
            {
                int vahennys = (prompt.obj->insured % 100) - prompt.obj->number;
                copy.insured = prompt.obj->insured / 100 * 100 + vahennys;
                obj_dec_insured(prompt.obj, vahennys);
            }
        }
        _drop_aux(&copy, context);
    }
    else
        _drop_aux(prompt.obj, context);

    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

static void _examine(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>Examine which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        obj_display(obj);
    }
}

static void _remove(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        char    name[MAX_NLEN];

        if (!msg_command("<color:y>Remove which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        object_desc(name, obj, OD_COLOR_CODED);
        cmd = msg_prompt(format("<color:y>Really remove %s?</color> <color:v>It will "
            "be permanently destroyed!</color> <color:y>[Y,n]</color>", name), "ny", PROMPT_YES_NO);
        if (cmd == 'n') continue;
        if (!can_player_destroy_object(obj))
        {
            object_desc(name, obj, OD_COLOR_CODED);
            msg_format("You cannot destroy %s.", name);
            continue;
        }
        inv_remove(context->inv, slot);
        inv_sort(context->inv);
        _display(context);
    }
}

