#include "angband.h"

#include <assert.h>

static void _context_make(obj_prompt_context_ptr context);
static void _context_unmake(obj_prompt_context_ptr context);
static int  _find_tab(vec_ptr tabs, int loc);
static int  _count_lines(cptr s);

static void _display(obj_prompt_context_ptr context);
static void _sync_doc(doc_ptr doc);

static int  _basic_cmd(obj_prompt_context_ptr context, int cmd);

void _obj_prompt_add_race_packs(obj_prompt_ptr prompt)
{
    int i;
    race_t *race_ptr = get_race();
    if (!(race_ptr->bonus_pack)) return;
    if (prompt->where[MAX_LOC - 1]) return; /* No room for another pack */
    if ((allow_special3_hack) && (race_ptr->bonus_pack2))
    {
        for (i = 0; i < MAX_LOC; i++)
        {
            if (!prompt->where[i])
            {
                prompt->where[i] = INV_SPECIAL3;
                break;
            }
        }
    }
    for (i = 0; i < MAX_LOC; i++)
    {
        if (!prompt->where[i]) 
        {
            prompt->where[i] = INV_SPECIAL1;
            return;
        }
    }
}

void _obj_prompt_add_class_packs(obj_prompt_ptr prompt)
{
    int i;
    class_t *class_ptr = get_class();
    if (!(class_ptr->bonus_pack)) return;
    if (prompt->where[MAX_LOC - 1]) return; /* No room for another pack */
    for (i = 0; i < MAX_LOC; i++)
    {
        if (!prompt->where[i]) 
        {
            prompt->where[i] = INV_SPECIAL2;
            return;
        }
    }
}

inv_ptr _racial_pack_filter(obj_p p)
{
    inv_ptr race_inv = get_race()->bonus_pack;
    if (!race_inv) return NULL;
    return inv_filter(race_inv, p);
}

inv_ptr _racial2_pack_filter(obj_p p)
{
    inv_ptr race_inv = get_race()->bonus_pack2;
    if (!race_inv) return NULL;
    return inv_filter(race_inv, p);
}

inv_ptr _class_pack_filter(obj_p p)
{
    inv_ptr class_inv = get_class()->bonus_pack;
    if (!class_inv) return NULL;
    return inv_filter(class_inv, p);
}

void obj_prompt_add_special_packs(obj_prompt_ptr prompt)
{
    _obj_prompt_add_race_packs(prompt);
    _obj_prompt_add_class_packs(prompt);
}

int obj_prompt(obj_prompt_ptr prompt)
{
    obj_prompt_context_t context = {0};
    int                  tmp;
    int                  result = 0;

    assert(!prompt->obj);

    context.prompt = prompt;
    context.page_size = MIN(26, ui_doc_menu_rect().cy - 3 - _count_lines(prompt->prompt));
    _context_make(&context);

    if (!vec_length(context.tabs))
    {
        if (prompt->error)
            msg_format("<color:R>%s</color>", prompt->error);
        _context_unmake(&context);
        return OP_NO_OBJECTS;
    }

    if (REPEAT_PULL(&tmp))
    {
        int repeat_tab = _find_tab(context.tabs, tmp);
        if (repeat_tab < 0)
        {
            /* XXX This might need further thought ... tested for force-trainers */
            if ( prompt->cmd_handler
              && prompt->cmd_handler(&context, tmp) == OP_CMD_DISMISS )
            {
                _context_unmake(&context);
                return OP_CUSTOM;
            }
        }
        else if (repeat_tab >= 0 && REPEAT_PULL(&tmp))
        {
            obj_prompt_tab_ptr tab = vec_get(context.tabs, repeat_tab);
            slot_t             slot;

            inv_calculate_labels(tab->inv, 1, context.page_size, prompt->flags);
            slot = inv_label_slot(tab->inv, tmp);
            if (slot)
            {
                prompt->obj = inv_obj(tab->inv, slot);
                _context_unmake(&context);
                return OP_SUCCESS;
            }
        }
    }

    context.doc = doc_alloc(MIN(80, ui_map_rect().cx));
    Term_save();
    for (;;)
    {
        obj_prompt_tab_ptr tab = vec_get(context.tabs, context.tab);
        int                cmd;
        slot_t             slot;

        _display(&context);

        cmd = inkey_special(TRUE);        
        if (prompt->cmd_handler)
        {
            tmp = prompt->cmd_handler(&context, cmd);
            if (tmp == OP_CMD_HANDLED) continue;
            if (tmp == OP_CMD_DISMISS)
            {
                result = OP_CUSTOM;
                REPEAT_PUSH(cmd);
                break;
            }
          /*  if ((isupper(cmd)) && (inv_label_slot(tab->inv, tolower(cmd))))
            {
                cmd = tolower(cmd);
                tmp = prompt->cmd_handler(&context, cmd);
                if (tmp == OP_CMD_HANDLED) continue;
                if (tmp == OP_CMD_DISMISS)
                {
                    result = OP_CUSTOM;
                    REPEAT_PUSH(cmd);
                    break;
                }
                cmd = toupper(cmd);
            } */
        }
        slot = inv_label_slot(tab->inv, cmd);
        if (slot)
        {
            obj_ptr obj = inv_obj(tab->inv, slot);
            if (!obj_confirm_choice(obj)) continue;
            prompt->obj = obj; 
            result = OP_SUCCESS;
            REPEAT_PUSH(inv_loc(tab->inv));
            /* repeat can be dangerous if the pack shuffles */
            if (tab->page == 0 && object_is_aware(obj))
                REPEAT_PUSH(cmd);
            break;
        }
        else if (isupper(cmd))
        {
            slot = inv_label_slot(tab->inv, tolower(cmd));
            if (slot)
            {
                obj_ptr obj = inv_obj(tab->inv, slot);
                doc_clear(context.doc);
                if (object_is_flavor(obj) && !object_is_known(obj))
                {
                    char name[MAX_NLEN];
                    object_desc(name, obj, OD_COLOR_CODED);
                    doc_insert(context.doc, name);
                    doc_insert(context.doc, "\n\nYou have no special knowledge about this item.\n");
                    doc_insert(context.doc, "<color:B>[Press <color:y>Any Key</color> to Continue]</color>\n\n");
                }
                else obj_display_doc(obj, context.doc);
                _sync_doc(context.doc);
                cmd = inkey();
                continue;
            }
        }
        else if (cmd == ESCAPE || cmd == '\r')
        {
            result = OP_CANCELED;
            break;
        }
        else /* Note: We do basic commands last in case the user inscribed @3. */
        {    /* 3 is Page Down in curses ... Similarly with 9 and Page Up in curses. */
            tmp = _basic_cmd(&context, cmd);
            if (tmp == OP_CMD_HANDLED) continue;
            if (tmp == OP_CMD_DISMISS)
            {
                result = OP_SUCCESS;
                break;
            }
        }
    }
    Term_load();
    _context_unmake(&context);
    return result;
}

static obj_prompt_tab_ptr _tab_alloc(inv_ptr inv, int page_size)
{
    obj_prompt_tab_ptr tab = malloc(sizeof(obj_prompt_tab_t));
    tab->inv = inv;
    tab->ct = inv_count_slots(inv, obj_exists);
    if (tab->ct)
        tab->page_ct = (tab->ct - 1) / page_size + 1;
    else
        tab->page_ct = 0;
    tab->page = 0;
    return tab;
}

static void _tab_free(obj_prompt_tab_ptr tab)
{
    if (tab)
    {
        inv_free(tab->inv);
        tab->inv = NULL;
        free(tab);
    }
}

static void _context_make(obj_prompt_context_ptr context)
{
    int i;
    obj_p filter = context->prompt->filter;

    if (!filter)
        filter = obj_exists;

    context->tabs = vec_alloc((vec_free_f)_tab_free);

    for (i = 0; i < MAX_LOC; i++)
    {
        inv_ptr inv = NULL;
        switch (context->prompt->where[i])
        {
        case INV_FLOOR:
            inv = inv_filter_floor(point(px, py), filter);
            inv_sort(inv);
            break;
        case INV_EQUIP:
            inv = equip_filter(context->prompt->filter);
            /* don't sort: always keep same slot labels!
             * always use the supplied filter (NULL->show
             * empty slots) */
            break;
        case INV_PACK:
            inv = pack_filter(filter);
            if (!use_pack_slots)
                inv_sort(inv);
            break;
        case INV_QUIVER:
            inv = quiver_filter(filter);
            if (!use_pack_slots) /* quiver might contain non-matching ammo */
                inv_sort(inv);
            break;
        case INV_SPECIAL1:
            inv = _racial_pack_filter(filter);
            break;
        case INV_SPECIAL2:
            inv = _class_pack_filter(filter);
            break;
        case INV_SPECIAL3:
            inv = _racial2_pack_filter(filter);
            break;
        }
        if (inv)
        {
            int ct = inv_count(inv, obj_exists);
            if (!ct)
                inv_free(inv);
            else
            {
                if (context->prompt->top_loc == context->prompt->where[i])
                    context->tab = vec_length(context->tabs);
                vec_add(context->tabs, _tab_alloc(inv, context->page_size));
            }
        }
    }
}

static void _context_unmake(obj_prompt_context_ptr context)
{
    assert(context);
    vec_free(context->tabs);
    doc_free(context->doc);
}

static void _sync_doc(doc_ptr doc)
{
    Term_load();
    doc_sync_menu(doc);
}

static void _display(obj_prompt_context_ptr context)
{
    obj_prompt_tab_ptr tab;
    int                i, start, stop;
    obj_p              filter = context->prompt->filter;

    doc_clear(context->doc);
    doc_insert(context->doc, "<style:table>");
    /* Tab Headers */
    for (i = 0; i < vec_length(context->tabs); i++)
    {
        tab = vec_get(context->tabs, i);
        if (i)
            doc_insert(context->doc, " <color:b>|</color> ");
        doc_printf(context->doc, "<color:%c>%s</color>",
            i == context->tab ? 'G' : 'D',
            inv_name(tab->inv));
    }
    if (context->prompt->flags & INV_SHOW_FAIL_RATES)
        doc_printf(context->doc, "<tab:%d><color:r>Fail</color>", doc_width(context->doc) - 5);
    doc_newline(context->doc);

    /* Active Tab */
    tab = vec_get(context->tabs, context->tab);
    start = tab->page * context->page_size + 1;
    stop = (tab->page + 1) * context->page_size;
    if (inv_loc(tab->inv) == INV_EQUIP)
        stop = equip_max(); /* Hack: only show valid slots for this body type */
    else if (!filter)
        filter = obj_exists; /* Hack: null filter only shows empty slots for INV_EQUIP */

    inv_display(tab->inv, start, stop, filter, context->doc, context->prompt->flags);

    if (tab->page_ct > 1)
    {
        doc_printf(context->doc,
            "<color:B>-%s-</color> <color:G>Page %d of %d</color>\n",
            tab->page == tab->page_ct - 1 ? "less" : "more",
            tab->page + 1, tab->page_ct);
    }
    if (!(context->prompt->flags & (INV_SHOW_FAIL_RATES | INV_SHOW_VALUE)))
    {
        if (show_weights)
        {
            int wgt = inv_weight(tab->inv, NULL);
            doc_printf(context->doc, "<tab:%d><color:R> %3d.%d lbs</color>",
                doc_width(context->doc) - 9, wgt/10, wgt%10);
        }
    }
    doc_newline(context->doc);
    if (context->prompt->prompt)
        doc_printf(context->doc, "<color:y>%s</color> ", context->prompt->prompt);
    else
        doc_insert(context->doc, "<color:y>Choice</color>: ");

    doc_insert(context->doc, "</style>");
    _sync_doc(context->doc);
}

static int _find_tab(vec_ptr tabs, int loc)
{
    int i;
    for (i = 0; i < vec_length(tabs); i++)
    {
        obj_prompt_tab_ptr tab = vec_get(tabs, i);
        if (inv_loc(tab->inv) == loc) return i;
    }
    return -1;
}

static int _basic_cmd(obj_prompt_context_ptr context, int cmd)
{
    switch (cmd)
    {
    case '-': {
        /* Legacy: In the olden days, - was used to autopick
         * the floor item. Of course, you had to do it blind
         * since it was never displayed. */
        int floor_tab = _find_tab(context->tabs, INV_FLOOR);
        if (floor_tab >= 0)
        {
            obj_prompt_tab_ptr tab = vec_get(context->tabs, floor_tab);
            context->tab = floor_tab;
            if ((inv_count_slots(tab->inv, obj_exists) == 1) && (!strpos("nscribe", context->prompt->prompt)))
            {
                slot_t slot = inv_first(tab->inv, obj_exists);
                obj_ptr obj; 
                assert(slot);
                obj = inv_obj(tab->inv, slot);
                if (obj_confirm_choice(obj))
                {
                    context->prompt->obj = obj; 
                    REPEAT_PUSH(inv_loc(tab->inv));
                    REPEAT_PUSH('a');
                    return OP_CMD_DISMISS;
                }
            }
        }
        return OP_CMD_HANDLED; }
    case '/':
        context->tab++;
        if (context->tab == vec_length(context->tabs))
            context->tab = 0;
        return OP_CMD_HANDLED;
    case '\\':
        context->tab--;
        if (context->tab < 0)
            context->tab = vec_length(context->tabs) - 1;
        return OP_CMD_HANDLED;
    case '@':
        if (context->prompt->flags & INV_IGNORE_INSCRIPTIONS)
            context->prompt->flags &= ~INV_IGNORE_INSCRIPTIONS;
        else
            context->prompt->flags |= INV_IGNORE_INSCRIPTIONS;
        break;
    case KTRL('I'): case KTRL('P'): { /* fyi, TAB is ^I in current encoding scheme ... */
        int tab = _find_tab(context->tabs, INV_PACK);
        if (tab >= 0)
            context->tab = tab;
        return OP_CMD_HANDLED; }
    case KTRL('E'): {
        int tab = _find_tab(context->tabs, INV_EQUIP);
        if (tab >= 0)
            context->tab = tab;
        return OP_CMD_HANDLED; }
    case KTRL('Q'): {
        int tab = _find_tab(context->tabs, INV_QUIVER);
        if (tab >= 0)
            context->tab = tab;
        return OP_CMD_HANDLED; }
    case KTRL('F'): {
        int tab = _find_tab(context->tabs, INV_FLOOR);
        if (tab >= 0)
            context->tab = tab;
        return OP_CMD_HANDLED; }
    case KTRL('W'):
        if (!(context->prompt->flags & (INV_SHOW_FAIL_RATES | INV_SHOW_VALUE)))
            show_weights = !show_weights;
        return OP_CMD_HANDLED;
    case KTRL('G'):
        show_item_graph = !show_item_graph;
        return OP_CMD_HANDLED;
    case KTRL('L'):
        show_labels = !show_labels;
        return OP_CMD_HANDLED;
    case SKEY_PGDOWN: case '3': case ' ': {
        obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
        if (tab->page < tab->page_ct - 1)
            tab->page++;
        else if (tab->page_ct > 1) /* wrap */
            tab->page = 0;
        return OP_CMD_HANDLED; }
    case SKEY_PGUP: case '9': {
        obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
        if (tab->page > 0)
            tab->page--;
        else if (tab->page_ct > 1) /* wrap */
            tab->page = tab->page_ct - 1;
        return OP_CMD_HANDLED; }
    case '?':
        if (context->prompt->help)
            doc_display_help(context->prompt->help, NULL);
        else
            doc_display_help("context_obj_prompt.txt", "QuickRef");
        return OP_CMD_HANDLED;
    }
    return OP_CMD_SKIPPED;
}

int _count_lines(cptr s)
{
    int ct = 1;
    cptr pos = strchr(s, '\n');
    while (pos)
    {
        ct++;
        pos = strchr(pos + 1, '\n');
    }
    return ct;
}


