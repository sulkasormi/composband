/* File: hissatsu.c */

/* Purpose: Kendo code */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

#include "angband.h"

#define TECHNIC_HISSATSU (REALM_HISSATSU - MIN_TECHNIC)

static void _dump_description(doc_ptr doc, char *kuvaus)
{
    doc_printf(doc, "%s", kuvaus);
}

static void _display(rect_t r, char *kuvaus)
{
    doc_ptr doc = doc_alloc(r.cx);
    _dump_description(doc, kuvaus);
    doc_sync_term(doc, doc_range_all(doc), doc_pos_create(r.x, r.y));
    doc_free(doc);
}

static rect_t _menu_rect(int y)
{
    rect_t r = ui_menu_rect();
    r.y = y;
    r.x = 15;
    if (r.cx > 65)
        r.cx = 65;
    return r;
}

/*
 * Allow user to choose a hissatsu power.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray, FALSE for study
 *
 */
static int get_hissatsu_power(int *sn)
{
    int             i, j = 0;
    int             num = 0;
    int             y = 1;
    int             x = 15;
    int             plev = p_ptr->lev;
    int             ask = TRUE;
    char            choice;
    char            out_val[160];
    char sentaku[32];
    cptr            p = "technique";

    magic_type spell;
    bool            flag, redraw, skip;
    int menu_line = (use_menu ? 1 : 0);
    byte            is_browsing = 0;

    /* Assume cancelled */
    *sn = (-1);

#ifdef ALLOW_REPEAT /* TNB */

    /* Get the spell, if available */
    if (repeat_pull(sn))
    {
        /* Verify the spell */
        if (technic_info[TECHNIC_HISSATSU][*sn].slevel <= plev)
        {
            /* Success */
            return (TRUE);
        }
    }

#endif /* ALLOW_REPEAT -- TNB */

    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;

    /* Do not skip */
    skip = FALSE;

    for (i = 0; i < 32; i++)
    {
        if (technic_info[TECHNIC_HISSATSU][i].slevel <= PY_MAX_LEVEL)
        {
            sentaku[num] = i;
            num++;
        }
    }

    /* Build a prompt (accept all spells) */
    (void) strnfmt(out_val, 78, 
               "(%^ss %c-%c, ?=%s, !=Damage, ESC=exit) Use which %s? ",
               p, I2A(0), "abcdefghijklmnopqrstuvwxyz012345"[num-1], is_browsing ? "Use" : "Browse", p);

//    if (use_menu) screen_save();
    screen_save();

    /* Get a spell from the user */

    choice= ESCAPE;
    while (!flag)
    {
        if ((choice==ESCAPE) || (skip)) choice = ' ';
        else if( !get_com(out_val, &choice, FALSE) )break;

        screen_load();
        screen_save();

        if (use_menu && choice != ' ')
        {
            switch(choice)
            {
                case '0':
                {
                    screen_load();
                    return (FALSE);
                }

                case '8':
                case 'k':
                case 'K':
                {
                    do
                    {
                        menu_line += 31;
                        if (menu_line > 32) menu_line -= 32;
                    } while(!(p_ptr->spell_learned1 & (1L << (menu_line-1))));
                    break;
                }

                case '2':
                case 'j':
                case 'J':
                {
                    do
                    {
                        menu_line++;
                        if (menu_line > 32) menu_line -= 32;
                    } while(!(p_ptr->spell_learned1 & (1L << (menu_line-1))));
                    break;
                }

                case '4':
                case 'h':
                case 'H':
                case '6':
                case 'l':
                case 'L':
                {
                    bool reverse = FALSE;
                    if ((choice == '4') || (choice == 'h') || (choice == 'H')) reverse = TRUE;
                    if (menu_line > 16)
                    {
                        menu_line -= 16;
                        reverse = TRUE;
                    }
                    else menu_line+=16;
                    while(!(p_ptr->spell_learned1 & (1L << (menu_line-1))))
                    {
                        if (reverse)
                        {
                            menu_line--;
                            if (menu_line < 2) reverse = FALSE;
                        }
                        else
                        {
                            menu_line++;
                            if (menu_line > 31) reverse = TRUE;
                        }
                    }
                    break;
                }

                case 'x':
                case 'X':
                case '\r':
                case '\n':
                {
                    i = menu_line - 1;
                    ask = FALSE;
                    break;
                }
            }
        }

        skip = FALSE;

        if (choice == '?')
        {
            is_browsing = (is_browsing == 1) ? 0 : 1;
            (void) strnfmt(out_val, 78,
               "(%^ss %c-%c, ?=%s, !=Damage, ESC=exit) %s which %s? ",
               p, I2A(0), "abcdefghijklmnopqrstuvwxyz012345"[num-1], is_browsing ? "Use" : "Browse", is_browsing ? "Browse" : "Use", p);
        }

        if (choice == '!')
        {
            is_browsing = 2;
            (void) strnfmt(out_val, 78,
               "(%^ss %c-%c, ?=Browse, ESC=exit) Show damage for which %s? ",
               p, I2A(0), "abcdefghijklmnopqrstuvwxyz012345"[num-1], p);
        }

        if ((choice == '?') || (choice == '!') || (is_browsing >= 1))
        {
            redraw = FALSE;
        }

        /* Request redraw */
        {
            /* Show the list */
            if (!redraw || use_menu)
            {
                char psi_desc[80];
                int line;

                /* Show list */
                redraw = TRUE;

//                if ((use_menu) screen_save();

                /* Display a list of spells */
                prt("", y, x);
put_str("name              Lv  SP      name              Lv  SP ", y, x + 5);
                prt("", y+1, x);
                /* Dump the spells */
                for (i = 0, line = 0; i < 32; i++)
                {
                    spell = technic_info[TECHNIC_HISSATSU][i];

                    if (spell.slevel > PY_MAX_LEVEL) continue;
                    line++;
                    if (!(p_ptr->spell_learned1 >> i)) break;

                    /* Access the spell */
                    if (spell.slevel > plev)   continue;
                    if (!(p_ptr->spell_learned1 & (1L << i)))
                    {
                        if ((i < 16) && (p_ptr->spell_learned1 & (1L << (i + 16))))
                        {
                            Term_erase(x, y + (line%17) + (line >= 17), 30);
                            if (i == 15) Term_erase(x, y + 1 + (line%17) + (line >= 17), 30);
                        }
                        continue;
                    }
                    if (use_menu)
                    {
                        if (i == (menu_line-1))
                            strcpy(psi_desc, "  > ");
                        else strcpy(psi_desc, "    ");
                        
                    }
                    else
                    {
                        char letter;
                        if (line <= 26)
                            letter = I2A(line-1);
                        else
                            letter = '0' + line - 27;
                        sprintf(psi_desc, "  %c)",letter);
                    }

                    /* Dump the spell --(-- */
                    strcat(psi_desc, format(" %-18s%2d %3d",
                        do_spell(REALM_HISSATSU, i, SPELL_NAME),
                        spell.slevel, spell.smana));
                    prt(psi_desc, y + (line%17) + (line >= 17), x+(line/17)*30);
                    prt("", y + (line%17) + (line >= 17) + 1, x+(line/17)*30);
                }
            }

            /* Hide the list */
            else
            {
                /* Hide list */
                redraw = FALSE;

                /* Restore the screen */
//                screen_load();
            }

            /* Redo asking */
            if ((choice == ' ') || (choice == '*') || (choice == '?') || (choice == '!') || (use_menu && ask)) continue;
        }

        if (!use_menu)
        {
            if (isalpha(choice))
            {
                /* Note verify */
                ask = (isupper(choice));

                /* Lowercase */
                if (ask) choice = tolower(choice);

                /* Extract request */
                i = (islower(choice) ? A2I(choice) : -1);
            }
            else
            {
                ask = FALSE; /* Can't uppercase digits */

                i = choice - '0' + 26;
            }
        }

        /* Totally Illegal */
        if ((i < 0) || (i >= 32) || !(p_ptr->spell_learned1 & (1 << sentaku[i])))
        {
            bell();
            continue;
        }

        j = sentaku[i];

        if (is_browsing == 1)
        {
            char spell_desc[512];
            int desc_y = (num > 15) ? 19 : 3 + num;
            prt("", desc_y, 17);
            (void)strnfmt(spell_desc, sizeof(spell_desc), "  <style:indent>%s</style>\n", do_spell(REALM_HISSATSU, j, SPELL_DESC));
            ask = FALSE;
            _display(_menu_rect(desc_y), spell_desc);
            continue;
        }
        else if (is_browsing == 2)
        {
            (void)do_spell(REALM_HISSATSU, j, SPELL_ON_BROWSE);
            ask = FALSE;
            skip = TRUE;
            continue;
        }

        /* Verify it */
        if (ask)
        {
            char tmp_val[160];

            /* Prompt */
            (void)strnfmt(tmp_val, 78, "Use %s? ", do_spell(REALM_HISSATSU, j, SPELL_NAME));


            /* Belay that order */
            if (!get_check(tmp_val)) continue;
        }

        /* Stop the loop */
        flag = TRUE;
    }

    /* Restore the screen */
    screen_load();

    /* Show choices */
    p_ptr->window |= (PW_SPELL);

    /* Window stuff */
    window_stuff();


    /* Abort if needed */
    if (!flag) return (FALSE);

    /* Save the choice */
    (*sn) = j;

#ifdef ALLOW_REPEAT /* TNB */

    repeat_push(*sn);

#endif /* ALLOW_REPEAT -- TNB */

    /* Success */
    return (TRUE);
}

void do_cmd_hissatsu(void)
{
    int             n = 0;
    magic_type      spell;


    /* not if confused */
    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        return;
    }
    if (!equip_find_first(object_is_melee_weapon))
    {
        if (flush_failure) flush();
        msg_print("You need to wield a weapon!");
        return;
    }
    if (!p_ptr->spell_learned1)
    {
        msg_print("You don't know any special attacks.");
        return;
    }
    if (pelko()) return;

    if (p_ptr->special_defense & KATA_MASK)
        set_action(ACTION_NONE);

    /* get power */
    if (!get_hissatsu_power(&n)) return;

    spell = technic_info[TECHNIC_HISSATSU][n];

    /* Verify "dangerous" spells */
    if (spell.smana > p_ptr->csp)
    {
        if (flush_failure) flush();
        /* Warning */
        msg_print("You do not have enough mana to use this power.");
        msg_print(NULL);
        return;
    }

    sound(SOUND_ZAP);

    /* Cast the spell */
    if (!do_spell(REALM_HISSATSU, n, SPELL_CAST)) return;

    spell_stats_on_cast_old(REALM_HISSATSU, n);

    /* Take a turn */
    energy_use = 100;

    /* Use some mana */
    p_ptr->csp -= spell.smana;

    /* Limit */
    if (p_ptr->csp < 0) p_ptr->csp = 0;

    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);

    /* Window stuff */
    p_ptr->window |= (PW_SPELL);
}


static bool _is_book(obj_ptr obj)
{
    return obj->tval == TV_HISSATSU_BOOK;
}

void do_cmd_gain_hissatsu(void)
{
    obj_prompt_t prompt = {0};
    int          i, j;
    bool         gain = FALSE;

    if (p_ptr->special_defense & (KATA_MUSOU | KATA_KOUKIJIN))
        set_action(ACTION_NONE);

    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");
        return;
    }

    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        return;
    }

    if (!(p_ptr->new_spells))
    {
        msg_print("You cannot learn any new special attacks!");
        return;
    }

    msg_format("You can learn %d new special attack%s.", p_ptr->new_spells,
        (p_ptr->new_spells == 1?"":"s"));

    prompt.prompt = "Study which book?";
    prompt.error = "You have no books that you can read.";
    prompt.filter = _is_book;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    for (i = prompt.obj->sval * 8; i < prompt.obj->sval * 8 + 8; i++)
    {
        if (p_ptr->spell_learned1 & (1L << i)) continue;
        if (technic_info[TECHNIC_HISSATSU][i].slevel > p_ptr->lev) continue;

        p_ptr->spell_learned1 |= (1L << i);
        p_ptr->spell_worked1 |= (1L << i);
        msg_format("You have learned the special attack of %s.", do_spell(REALM_HISSATSU, i, SPELL_NAME));
        for (j = 0; j < 64; j++)
        {
            /* Stop at the first empty space */
            if (p_ptr->spell_order[j] == 99) break;
        }
        p_ptr->spell_order[j] = i;
        gain = TRUE;
    }

    /* No gain ... */
    if (!gain)
        msg_print("You were not able to learn any special attacks.");

    /* Take a turn */
    else
        energy_use = 100;

    p_ptr->update |= (PU_SPELLS);
}

