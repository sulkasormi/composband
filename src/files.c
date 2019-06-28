/* File: files.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: code dealing with files (and death) */

#include "angband.h"
#include "equip.h"
#include "z-doc.h"

#ifdef SIGSTOP

/* OK, what header is this in? */
extern int kill(int, int);

#endif

/*
 * You may or may not want to use the following "#undef".
 */
/* #undef _POSIX_SAVED_IDS */


/*locks player name for server play
 *this is only placed here, since including it in the other
 *header files would interfere with the extern variable
 *--phantom
*/
bool arg_lock_name;


/*
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef SAFE_SETUID_POSIX

    if (setuid(getuid()) != 0)
    {
        quit("setuid(): cannot set permissions correctly!");

    }
    if (setgid(getgid()) != 0)
    {
        quit("setgid(): cannot set permissions correctly!");

    }

#  else

    if (setreuid(geteuid(), getuid()) != 0)
    {
        quit("setreuid(): cannot set permissions correctly!");

    }
    if (setregid(getegid(), getgid()) != 0)
    {
        quit("setregid(): cannot set permissions correctly!");

    }

#  endif

# endif

#endif

}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef SAFE_SETUID_POSIX

    if (setuid(player_euid) != 0)
    {
        quit("setuid(): cannot set permissions correctly!");

    }
    if (setgid(player_egid) != 0)
    {
        quit("setgid(): cannot set permissions correctly!");

    }

#  else

    if (setreuid(geteuid(), getuid()) != 0)
    {
        quit("setreuid(): cannot set permissions correctly!");

    }
    if (setregid(getegid(), getgid()) != 0)
    {
        quit("setregid(): cannot set permissions correctly!");

    }

#  endif /* SAFE_SETUID_POSIX */

# endif /* SAFE_SETUID */

#endif /* SET_UID */

}

/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens. The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens, int mode)
{
    int i = 0;

    char *s = buf;


    /* Process */
    while (i < num - 1)
    {
        char *t;

        /* Scan the string */
        for (t = s; *t; t++)
        {
            /* Found a delimiter */
            if ((*t == ':') || (*t == '/')) break;

            /* Handle single quotes */
            if ((mode & TOKENIZE_CHECKQUOTE) && (*t == '\''))
            {
                /* Advance */
                t++;

                /* Handle backslash */
                if (*t == '\\') t++;

                /* Require a character */
                if (!*t) break;

                /* Advance */
                t++;

                /* Hack -- Require a close quote */
                if (*t != '\'') *t = '\'';
            }

            /* Handle back-slash */
            if (*t == '\\') t++;
        }

        /* Nothing left */
        if (!*t) break;

        /* Nuke and advance */
        *t++ = '\0';

        /* Save the token */
        tokens[i++] = s;

        /* Advance */
        s = t;
    }

    /* Save the token */
    tokens[i++] = s;

    /* Number found */
    return (i);
}

/* tokenize, but with user supplied delimiters and no special backslash/quote handling
 * Added support for quoted delimiters, e.g. MON("fang, farmer maggot's dog", CLONE). */
int z_string_split(char *buf, char **tokens, int max, cptr delim)
{
    int i = 0;
    char *s = buf;
    bool quote = FALSE;

    /* inch-worm alogorithm: s marks the start of the current token
       while t scans ahead for the next delimiter. buf is destroyed. */
    while (i < max - 1)
    {
        char *t;

        for (t = s; *t; t++)
        {
            if (*t == '"') quote = !quote;
            if (!quote && strchr(delim, *t)) break;
        }

        if (!*t) break;
        *t++ = '\0';
        tokens[i++] = s;
        s = t;
    }

    tokens[i++] = s;
    trim_tokens(tokens, i);
    return i;
}

void trim_tokens(char **tokens, int ct)
{
    int i;
    for (i = 0; i < ct; i++)
    {
        char *s = tokens[i];
        char *t = s + strlen(s) - 1;

        while (*s && *s == ' ')
            s++;

        while (*t && *t == ' ' && t > s)
            t--;

        /* unquote */
        if (*s && *s == '"' && *t && *t == '"' && t > s)
        {
            s++;
            t--;
        }

        t++;
        *t = '\0';
        tokens[i] = s;
    }
}

/* Name(arg1, arg2, arg3) */
int parse_args(char *buf, char **name, char **args, int max)
{
    char *s = buf;
    char *t;
    int   ct = 0;

    for (t = s; *t; t++)
    {
        if (*t == '(') break;
    }

    if (!*t)
    {
        *name = s;
        return 0;
    }

    *t++ = '\0';
    *name = s;
    s = t;

    for (t = s; *t; t++)
    {
        if (*t == ')') break;
    }

    if (!*t)
    {
        /* Parse error ... */
        return -1;
    }

    *t++ = '\0';
    ct = z_string_split(s, args, max, ",");
    return ct;
}

/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Parse another file recursively, see below for details
 *   %:<filename>
 *
 * Specify the attr/char values for "monsters" by race index
 *   R:<num>:<a>:<c>
 *
 * Specify the attr/char values for "objects" by kind index
 *   K:<num>:<a>:<c>
 *
 * Specify the attr/char values for "features" by feature index
 *   F:<num>:<a>:<c>
 *
 * Specify the attr/char values for unaware "objects" by kind tval
 *   U:<tv>:<a>:<c>
 *
 * Specify the attr/char values for inventory "objects" by kind tval
 *   E:<tv>:<a>:<c>
 *
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a normal macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a command macro, given an encoded macro trigger
 *   C:<str>
 *
 * Create a keyset mapping
 *   S:<key>:<key>:<dir>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify the set of colors to use when drawing a zapped spell
 *   Z:<type>:<str>
 *
 * Specify a macro trigger template and macro trigger names.
 *   T:<template>:<modifier chr>:<modifier name1>:<modifier name2>:...
 *   T:<trigger>:<keycode>:<shift-keycode>
 *
 */

errr process_pref_file_command(char *buf)
{
    int i, j, n1, n2;

    char *zz[16];


    /* Require "?:*" format */
    if (buf[1] != ':') return 1;


    switch (buf[0])
    {
    /* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
    case 'R':
        if (tokenize(buf+2, 3, zz, TOKENIZE_CHECKQUOTE) == 3)
        {
            monster_race *r_ptr;
            i = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (i >= max_r_idx) return 1;
            r_ptr = &r_info[i];
            if (n1 || (!(n2 & 0x80) && n2)) r_ptr->x_attr = n1; /* Allow TERM_DARK text */
            if (n2) r_ptr->x_char = n2;
            return 0;
        }
        break;

    /* Process "K:<tval>:<sval>:<a>/<c>"  -- attr/char for object kinds
     * N.B. Stop using k_idx some day ... map (tv, sv)->obj_kind_ptr instead. */
    case 'K':
        if (tokenize(buf+2, 4, zz, TOKENIZE_CHECKQUOTE) == 4)
        {
            object_kind *k_ptr;
            int tval, sval, k_idx;
            tval = strtol(zz[0], NULL, 0);
            sval = strtol(zz[1], NULL, 0);
            n1 = strtol(zz[2], NULL, 0);
            n2 = strtol(zz[3], NULL, 0);
            k_idx = lookup_kind(tval, sval);
            if (!k_idx) return 1;
            k_ptr = &k_info[k_idx];
            if (n1 || (!(n2 & 0x80) && n2)) k_ptr->x_attr = n1; /* Allow TERM_DARK text */
            if (n2) k_ptr->x_char = n2;
            return 0;
        }
        break;

    /* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
    /* "F:<num>:<a>/<c>" */
    /* "F:<num>:<a>/<c>:LIT" */
    /* "F:<num>:<a>/<c>:<la>/<lc>:<da>/<dc>" */
    case 'F':
        {
            feature_type *f_ptr;
            int num = tokenize(buf + 2, F_LIT_MAX * 2 + 1, zz, TOKENIZE_CHECKQUOTE);

            if ((num != 3) && (num != 4) && (num != F_LIT_MAX * 2 + 1)) return 1;
            else if ((num == 4) && !streq(zz[3], "LIT")) return 1;

            i = (huge)strtol(zz[0], NULL, 0);
            if (i >= max_f_idx) return 1;
            f_ptr = &f_info[i];

            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (n1 || (!(n2 & 0x80) && n2)) f_ptr->x_attr[F_LIT_STANDARD] = n1; /* Allow TERM_DARK text */
            if (n2) f_ptr->x_char[F_LIT_STANDARD] = n2;

            /* Mega-hack -- feat supports lighting */
            switch (num)
            {
            /* No lighting support */
            case 3:
                n1 = f_ptr->x_attr[F_LIT_STANDARD];
                n2 = f_ptr->x_char[F_LIT_STANDARD];
                for (j = F_LIT_NS_BEGIN; j < F_LIT_MAX; j++)
                {
                    f_ptr->x_attr[j] = n1;
                    f_ptr->x_char[j] = n2;
                }
                break;

            /* Use default lighting */
            case 4:
                apply_default_feat_lighting(f_ptr->x_attr, f_ptr->x_char);
                break;

            /* Use desired lighting */
            case F_LIT_MAX * 2 + 1:
                for (j = F_LIT_NS_BEGIN; j < F_LIT_MAX; j++)
                {
                    n1 = strtol(zz[j * 2 + 1], NULL, 0);
                    n2 = strtol(zz[j * 2 + 2], NULL, 0);
                    if (n1 || (!(n2 & 0x80) && n2)) f_ptr->x_attr[j] = n1; /* Allow TERM_DARK text */
                    if (n2) f_ptr->x_char[j] = n2;
                }
                break;
            }
        }
        return 0;

    /* Process "S:<num>:<a>/<c>" -- attr/char for special things */
    case 'S':
        if (tokenize(buf+2, 3, zz, TOKENIZE_CHECKQUOTE) == 3)
        {
            j = (byte)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            misc_to_attr[j] = n1;
            misc_to_char[j] = n2;
            return 0;
        }
        break;

    /* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
    case 'U':
        if (tokenize(buf+2, 3, zz, TOKENIZE_CHECKQUOTE) == 3)
        {
            j = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            for (i = 1; i < max_k_idx; i++)
            {
                object_kind *k_ptr = &k_info[i];
                if (k_ptr->tval == j)
                {
                    if (n1) k_ptr->d_attr = n1;
                    if (n2) k_ptr->d_char = n2;
                }
            }
            return 0;
        }
        break;

    /* Process "E:<tv>:<a>" -- attribute for inventory objects */
    case 'E':
        if (tokenize(buf+2, 2, zz, TOKENIZE_CHECKQUOTE) == 2)
        {
            j = (byte)strtol(zz[0], NULL, 0) % 128;
            n1 = strtol(zz[1], NULL, 0);
            if (n1) tval_to_attr[j] = n1;
            return 0;
        }
        break;

    /* Process "A:<str>" -- save an "action" for later */
    case 'A':
        text_to_ascii(macro__buf, buf+2);
        return 0;

    /* Process "P:<str>" -- normal macro */
    case 'P':
    {
        char tmp[1024];

        text_to_ascii(tmp, buf+2);
        macro_add(tmp, macro__buf);
        return 0;
    }

    /* Process "C:<str>" -- create keymap */
    case 'C':
    {
        int mode;
        char tmp[1024];

        if (tokenize(buf+2, 2, zz, TOKENIZE_CHECKQUOTE) != 2) return 1;

        mode = strtol(zz[0], NULL, 0);
        if ((mode < 0) || (mode >= KEYMAP_MODES)) return 1;

        text_to_ascii(tmp, zz[1]);
        if (!tmp[0] || tmp[1]) return 1;
        i = (byte)(tmp[0]);

        z_string_free(keymap_act[mode][i]);

        keymap_act[mode][i] = z_string_make(macro__buf);

        return 0;
    }

    /* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
    case 'V':
        if (tokenize(buf+2, 5, zz, TOKENIZE_CHECKQUOTE) == 5)
        {
            i = (byte)strtol(zz[0], NULL, 0);
            angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
            angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
            angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
            angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
            return 0;
        }
        break;

    /* Process "X:<str>" -- turn option off */
    /* Process "Y:<str>" -- turn option on */
    case 'X':
    case 'Y':
        for (i = 0; option_info[i].o_desc; i++)
        {
            if (option_info[i].o_var &&
                option_info[i].o_text &&
                streq(option_info[i].o_text, buf + 2))
            {
                int os = option_info[i].o_set;
                int ob = option_info[i].o_bit;

                if ((p_ptr->playing || character_xtra) &&
                    (OPT_PAGE_BIRTH == option_info[i].o_page) && !p_ptr->wizard)
                {
                    msg_format("Birth options can not changed! '%s'", buf);
                    msg_print(NULL);
                    return 0;
                }

                if (buf[0] == 'X')
                {
                    /* Clear */
                    option_flag[os] &= ~(1L << ob);
                    (*option_info[i].o_var) = FALSE;
                }
                else
                {
                    /* Set */
                    option_flag[os] |= (1L << ob);
                    (*option_info[i].o_var) = TRUE;
                }
                return 0;
            }
        }

        /* don't know that option. ignore it.*/
        msg_format("Ignored invalid option: %s", buf);
        msg_print(NULL);
        return 0;

    /* Process "Z:<type>:<str>" -- set spell color */
    case 'Z':
    {
        /* Find the colon */
        char *t = my_strchr(buf + 2, ':');
        gf_info_ptr gf;

        /* Oops */
        if (!t) return 1;

        /* Nuke the colon */
        *(t++) = '\0';

        gf = gf_parse_name(buf+2);
        if (gf)
        {
            gf_color[gf->id] = quark_add(t);
            return 0;
        }
        break;
    }

    /* Initialize macro trigger names and a template */
    /* Process "T:<trigger>:<keycode>:<shift-keycode>" */
    /* Process "T:<template>:<modifier chr>:<modifier name>:..." */
    case 'T':
    {
        int tok = tokenize(buf+2, 2+MAX_MACRO_MOD, zz, 0);

        /* Process "T:<template>:<modifier chr>:<modifier name>:..." */
        if (tok >= 4)
        {
            int i;
            int num;

            if (macro_template != NULL)
            {
                num = strlen(macro_modifier_chr);

                /* Kill the template string */
                z_string_free(macro_template);
                macro_template = NULL;

                /* Kill flag characters of modifier keys */
                z_string_free(macro_modifier_chr);

                /* Kill corresponding modifier names */
                for (i = 0; i < num; i++)
                {
                    z_string_free(macro_modifier_name[i]);
                }

                /* Kill trigger name strings */
                for (i = 0; i < max_macrotrigger; i++)
                {
                    z_string_free(macro_trigger_name[i]);
                    z_string_free(macro_trigger_keycode[0][i]);
                    z_string_free(macro_trigger_keycode[1][i]);
                }

                max_macrotrigger = 0;
            }

            if (*zz[0] == '\0') return 0; /* clear template */

            /* Number of modifier flags */
            num = strlen(zz[1]);

            /* Limit the number */
            num = MIN(MAX_MACRO_MOD, num);

            /* Stop if number of modifier is not correct */
            if (2 + num != tok) return 1;

            /* Get a template string */
            macro_template = z_string_make(zz[0]);

            /* Get flag characters of modifier keys */
            macro_modifier_chr = z_string_make(zz[1]);

            /* Get corresponding modifier names */
            for (i = 0; i < num; i++)
            {
                macro_modifier_name[i] = z_string_make(zz[2+i]);
            }
        }

        /* Process "T:<trigger>:<keycode>:<shift-keycode>" */
        else if (tok >= 2)
        {
            char buf[1024];
            int m;
            char *t, *s;
            if (max_macrotrigger >= MAX_MACRO_TRIG)
            {
                msg_print("Too many macro triggers!");
                return 1;
            }
            m = max_macrotrigger;
            max_macrotrigger++;

            /* Take into account the escape character  */
            t = buf;
            s = zz[0];
            while (*s)
            {
                if ('\\' == *s) s++;
                *t++ = *s++;
            }
            *t = '\0';

            /* Get a trigger name */
            macro_trigger_name[m] = z_string_make(buf);

            /* Get the corresponding key code */
            macro_trigger_keycode[0][m] = z_string_make(zz[1]);

            if (tok == 3)
            {
                /* Key code of a combination of it with the shift key */
                macro_trigger_keycode[1][m] = z_string_make(zz[2]);
            }
            else
            {
                macro_trigger_keycode[1][m] = z_string_make(zz[1]);
            }
        }

        /* No error */
        return 0;
    }
    }

    /* Failure */
    return 1;
}


/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
cptr process_pref_file_expr(char **sp, char *fp)
{
    cptr v;

    char *b;
    char *s;

    char b1 = '[';
    char b2 = ']';

    char f = ' ';
    static char tmp[10];

    /* Initial */
    s = (*sp);

    /* Skip spaces */
    while (isspace(*s)) s++;

    /* Save start */
    b = s;

    /* Default */
    v = "?o?o?";

    /* Analyze */
    if (*s == b1)
    {
        const char *p;
        const char *t;

        /* Skip b1 */
        s++;

        /* First */
        t = process_pref_file_expr(&s, &f);

        /* Oops */
        if (!*t)
        {
            /* Nothing */
        }

        /* Function: IOR */
        else if (streq(t, "IOR") || streq(t, "OR"))
        {
            v = "0";
            while (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
                if (*t && !streq(t, "0")) v = "1";
            }
        }

        /* Function: AND */
        else if (streq(t, "AND"))
        {
            v = "1";
            while (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
                if (*t && streq(t, "0")) v = "0";
            }
        }

        /* Function: NOT */
        else if (streq(t, "NOT"))
        {
            v = "1";
            while (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
                if (*t && streq(t, "1")) v = "0";
            }
        }

        /* Function: EQU */
        else if (streq(t, "EQU"))
        {
            v = "0";
            if (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = process_pref_file_expr(&s, &f);
                if (streq(t, p)) v = "1";
            }
        }

        /* Function: LEQ */
        else if (streq(t, "LEQ"))
        {
            v = "1";
            if (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = t;
                t = process_pref_file_expr(&s, &f);
                if (*t && atoi(p) > atoi(t)) v = "0";
            }
        }

        /* Function: GEQ */
        else if (streq(t, "GEQ"))
        {
            v = "1";
            if (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
            }
            while (*s && (f != b2))
            {
                p = t;
                t = process_pref_file_expr(&s, &f);

                /* Compare two numbers instead of string */
                if (*t && atoi(p) < atoi(t)) v = "0";
            }
        }

        /* Oops */
        else
        {
            while (*s && (f != b2))
            {
                t = process_pref_file_expr(&s, &f);
            }
        }

        /* Verify ending */
        if (f != b2) v = "?x?x?";

        /* Extract final and Terminate */
        if ((f = *s) != '\0') *s++ = '\0';
    }

    /* Other */
    else
    {
        /* Accept all printables except spaces and brackets */
        while (isprint(*s) && !my_strchr(" []", *s)) ++s;

        /* Extract final and Terminate */
        if ((f = *s) != '\0') *s++ = '\0';

        /* Variable */
        if (*b == '$')
        {
            /* System */
            if (streq(b+1, "SYS"))
            {
                v = ANGBAND_SYS;
            }

            else if (streq(b+1, "KEYBOARD"))
            {
                v = ANGBAND_KEYBOARD;
            }

            /* Graphics */
            else if (streq(b+1, "GRAF"))
            {
                v = ANGBAND_GRAF;
            }

            /* Monochrome mode */
            else if (streq(b+1, "MONOCHROME"))
            {
                if (arg_monochrome)
                    v = "ON";
                else
                    v = "OFF";
            }

            /* Race */
            else if (streq(b+1, "RACE"))
            {
                if (p_ptr->prace == RACE_DOPPELGANGER) /* Use appropriate visuals for mimicked race */
                    v = get_race()->name;
                else
                    v = get_true_race()->name;
                while (1)
                {
                    unsigned int paikka = strpos(" ", v);
                    if (!paikka) break;
                    sprintf(tmp, v);
                    tmp[paikka - 1] = '-';
                    v = tmp;
                }
            }

            /* Class */
            else if (streq(b+1, "CLASS"))
            {
                v = get_class()->name;
            }

            /* Player */
            else if (streq(b+1, "PLAYER"))
            {
                static char tmp_player_name[32];
                char *pn, *tpn;
                for (pn = player_name, tpn = tmp_player_name; *pn; pn++, tpn++)
                {
                    *tpn = my_strchr(" []", *pn) ? '_' : *pn;
                }
                *tpn = '\0';
                v = tmp_player_name;
            }

            /* First realm */
            else if (streq(b+1, "REALM1"))
            {
                v = realm_names[p_ptr->realm1];
            }

            /* Second realm */
            else if (streq(b+1, "REALM2"))
            {
                v = realm_names[p_ptr->realm2];
            }

            /* Level */
            else if (streq(b+1, "LEVEL"))
            {
                sprintf(tmp, "%02d", p_ptr->lev);
                v = tmp;
            }

            /* Autopick auto-register is in-use or not? */
            else if (streq(b+1, "AUTOREGISTER"))
            {
                if (p_ptr->autopick_autoregister)
                    v = "1";
                else
                    v = "0";
            }

            /* Money */
            else if (streq(b+1, "MONEY"))
            {
                sprintf(tmp, "%09d", p_ptr->au);
                v = tmp;
            }
        }

        /* Constant */
        else
        {
            v = b;
        }
    }

    /* Save */
    (*fp) = f;

    /* Save */
    (*sp) = s;

    /* Result */
    return (v);
}


#define PREF_TYPE_NORMAL   0
#define PREF_TYPE_AUTOPICK 1

/*
 * Open the "user pref file" and parse it.
 */
static errr process_pref_file_aux(cptr name, int preftype)
{
    FILE *fp;

    char buf[1024];

    char old[1024];

    int line = -1;

    errr err = 0;

    bool bypass = FALSE;


    /* Open the file */
    fp = my_fopen(name, "r");

    /* No such file */
    if (!fp) return (-1);

    /* Process the file */
    while (0 == my_fgets(fp, buf, sizeof(buf)))
    {
        /* Count lines */
        line++;

        /* Skip "empty" lines */
        if (!buf[0]) continue;

        /* Skip "blank" lines */
        if (isspace(buf[0])) continue;

        /* Skip comments */
        if (buf[0] == '#') continue;


        /* Save a copy */
        strcpy(old, buf);


        /* Process "?:<expr>" */
        if ((buf[0] == '?') && (buf[1] == ':'))
        {
            char f;
            cptr v;
            char *s;

            /* Start */
            s = buf + 2;

            /* Parse the expr */
            v = process_pref_file_expr(&s, &f);

            /* Set flag */
            bypass = (streq(v, "0") ? TRUE : FALSE);

            /* Continue */
            continue;
        }

        /* Apply conditionals */
        if (bypass) continue;


        /* Process "%:<file>" */
        if (buf[0] == '%')
        {
            static int depth_count = 0;

            /* Ignore if deeper than 20 level */
            if (depth_count > 20) continue;

            /* Count depth level */
            depth_count++;

              /* Process that file if allowed */
            switch (preftype)
            {
            case PREF_TYPE_AUTOPICK:
                (void)process_autopick_file(buf + 2);
                break;
            default:
                (void)process_pref_file(buf + 2);
                break;
            }

            /* Set back depth level */
            depth_count--;

            /* Continue */
            continue;
        }


        /* Process the line */
        err = process_pref_file_command(buf);

        /* This is not original pref line... */
        if (err)
        {
            if (preftype != PREF_TYPE_AUTOPICK)
                  break;
            err = process_autopick_file_command(buf);
        }
    }


    /* Error */
    if (err)
    {
        /* Print error message */
        /* ToDo: Add better error messages */
        msg_format("Error %d in line %d of file '%s'.", err, line, name);
        msg_format("Parsing '%s'", old);
        msg_print(NULL);
    }

    /* Close the file */
    my_fclose(fp);

    /* Result */
    return (err);
}



/*
 * Process the "user pref file" with the given name
 *
 * See the functions above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
    char buf[1024];

    errr err1, err2;

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_PREF, name);

    /* Process the system pref file */
    err1 = process_pref_file_aux(buf, PREF_TYPE_NORMAL);

    /* Stop at parser errors, but not at non-existing file */
    if (err1 > 0) return err1;


    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

    /* Process the user pref file */
    err2 = process_pref_file_aux(buf, PREF_TYPE_NORMAL);


    /* User file does not exist, but read system pref file */
    if (err2 < 0 && !err1)
        return -2;

    /* Result of user file processing */
    return err2;
}



#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] =
{
    "SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
    "MON:XXXXXXXX.........XXXXXXX",
    "TUE:XXXXXXXX.........XXXXXXX",
    "WED:XXXXXXXX.........XXXXXXX",
    "THU:XXXXXXXX.........XXXXXXX",
    "FRI:XXXXXXXX.........XXXXXXX",
    "SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

    time_t      c;
    struct tm   *tp;

    /* No restrictions */
    if (!check_time_flag) return (0);

    /* Check for time violation */
    c = time((time_t *)0);
    tp = localtime(&c);

    /* Violation */
    if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);

#endif

    /* Success */
    return (0);
}



/*
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

    FILE        *fp;

    char    buf[1024];


    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "time.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Assume restrictions */
    check_time_flag = TRUE;

    /* Parse the file */
    while (0 == my_fgets(fp, buf, sizeof(buf)))
    {
        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Chop the buffer */
        buf[29] = '\0';

        /* Extract the info */
        if (prefix(buf, "SUN:")) strcpy(days[0], buf);
        if (prefix(buf, "MON:")) strcpy(days[1], buf);
        if (prefix(buf, "TUE:")) strcpy(days[2], buf);
        if (prefix(buf, "WED:")) strcpy(days[3], buf);
        if (prefix(buf, "THU:")) strcpy(days[4], buf);
        if (prefix(buf, "FRI:")) strcpy(days[5], buf);
        if (prefix(buf, "SAT:")) strcpy(days[6], buf);
    }

    /* Close it */
    my_fclose(fp);

#endif

    /* Success */
    return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN  64
#endif

typedef struct statstime statstime;

struct statstime
{
    int                 cp_time[4];
    int                 dk_xfer[4];
    unsigned int        v_pgpgin;
    unsigned int        v_pgpgout;
    unsigned int        v_pswpin;
    unsigned int        v_pswpout;
    unsigned int        v_intr;
    int                 if_ipackets;
    int                 if_ierrors;
    int                 if_opackets;
    int                 if_oerrors;
    int                 if_collisions;
    unsigned int        v_swtch;
    long                avenrun[3];
    struct timeval      boottime;
    struct timeval      curtime;
};

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

    struct statstime    st;

    /* Success if not checking */
    if (!check_load_value) return (0);

    /* Check the load */
    if (0 == rstat("localhost", &st))
    {
        long val1 = (long)(st.avenrun[2]);
        long val2 = (long)(check_load_value) * FSCALE;

        /* Check for violation */
        if (val1 >= val2) return (1);
    }

#endif

    /* Success */
    return (0);
}


/*
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

    FILE        *fp;

    char    buf[1024];

    char    temphost[MAXHOSTNAMELEN+1];
    char    thishost[MAXHOSTNAMELEN+1];


    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "load.txt");

    /* Open the "load" file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Default load */
    check_load_value = 100;

    /* Get the host name */
    (void)gethostname(thishost, (sizeof thishost) - 1);

    /* Parse it */
    while (0 == my_fgets(fp, buf, sizeof(buf)))
    {
        int value;

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Parse, or ignore */
        if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

        /* Skip other hosts */
        if (!streq(temphost, thishost) &&
            !streq(temphost, "localhost")) continue;

        /* Use that value */
        check_load_value = value;

        /* Done */
        break;
    }

    /* Close the file */
    my_fclose(fp);

#endif

    /* Success */
    return (0);
}

/*
 * Obtain the "flags" for the player as if he was an item
 */
void player_flags(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    class_t *class_ptr = get_class();
    race_t *race_ptr = get_race();
    personality_ptr pers_ptr = get_personality();

    /* Clear */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = 0L;

    if (class_ptr->get_flags)
        class_ptr->get_flags(flgs);

    if (race_ptr->get_flags)
        race_ptr->get_flags(flgs);

    if (pers_ptr->get_flags)
        pers_ptr->get_flags(flgs);

    if (race_ptr->infra)
        add_flag(flgs, OF_INFRA);

    mut_get_flags(flgs);
}

void tim_player_flags(u32b flgs[OF_ARRAY_SIZE])
{
    int i;

    /* Clear */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = 0L;

    if (IS_HERO())
        add_flag(flgs, OF_RES_FEAR);
    if (IS_SHERO())
        add_flag(flgs, OF_IM_FEAR);
    if (IS_INVULN())
        add_flag(flgs, OF_IM_FEAR);
    if (p_ptr->tim_invis)
        add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->tim_infra)
        add_flag(flgs, OF_INFRA);
    if (p_ptr->tim_regen)
        add_flag(flgs, OF_REGEN);
    if (IS_TIM_ESP())
        add_flag(flgs, OF_TELEPATHY);
    if (IS_FAST() || player_slow())
        add_flag(flgs, OF_SPEED);

    if (IS_OPPOSE_ACID() && !(p_ptr->special_defense & DEFENSE_ACID) && !(prace_is_(RACE_YEEK) && (p_ptr->lev > 19)))
        add_flag(flgs, OF_RES_ACID);
    if (IS_OPPOSE_ELEC() && !(p_ptr->special_defense & DEFENSE_ELEC))
        add_flag(flgs, OF_RES_ELEC);
    if (IS_OPPOSE_FIRE() && !(p_ptr->special_defense & DEFENSE_FIRE))
        add_flag(flgs, OF_RES_FIRE);
    if (IS_OPPOSE_COLD() && !(p_ptr->special_defense & DEFENSE_COLD))
        add_flag(flgs, OF_RES_COLD);
    if (IS_OPPOSE_POIS())
        add_flag(flgs, OF_RES_POIS);

    if (p_ptr->tim_sustain_str) add_flag(flgs, OF_SUST_STR);
    if (p_ptr->tim_sustain_int) add_flag(flgs, OF_SUST_INT);
    if (p_ptr->tim_sustain_wis) add_flag(flgs, OF_SUST_WIS);
    if (p_ptr->tim_sustain_dex) add_flag(flgs, OF_SUST_DEX);
    if (p_ptr->tim_sustain_con) add_flag(flgs, OF_SUST_CON);
    if (p_ptr->tim_sustain_chr) add_flag(flgs, OF_SUST_CHR);
    if (p_ptr->tim_hold_life) add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->tim_dark_stalker) add_flag(flgs, OF_STEALTH);
    if (p_ptr->tim_levitation) add_flag(flgs, OF_LEVITATION);

    if (p_ptr->special_attack & ATTACK_ACID)
        add_flag(flgs, OF_BRAND_ACID);
    if (p_ptr->special_attack & ATTACK_ELEC)
        add_flag(flgs, OF_BRAND_ELEC);
    if (p_ptr->special_attack & ATTACK_FIRE)
        add_flag(flgs, OF_BRAND_FIRE);
    if (p_ptr->special_attack & ATTACK_COLD)
        add_flag(flgs, OF_BRAND_COLD);
    if (p_ptr->special_attack & ATTACK_POIS)
        add_flag(flgs, OF_BRAND_POIS);
    if (p_ptr->special_defense & DEFENSE_ACID)
        add_flag(flgs, OF_IM_ACID);
    if (p_ptr->special_defense & DEFENSE_ELEC)
        add_flag(flgs, OF_IM_ELEC);
    if (p_ptr->special_defense & DEFENSE_FIRE)
        add_flag(flgs, OF_IM_FIRE);
    if (p_ptr->special_defense & DEFENSE_COLD)
        add_flag(flgs, OF_IM_COLD);

    if (IS_WRAITH())
    {
        add_flag(flgs, OF_REFLECT);
        add_flag(flgs, OF_IM_DARK);
        add_flag(flgs, OF_VULN_LITE);
    }
    /* by henkma */
    if (p_ptr->tim_reflect)
        add_flag(flgs, OF_REFLECT);

    if (p_ptr->tim_blood_shield)
    {
        int amt = 100 * (p_ptr->mhp - p_ptr->chp) / p_ptr->mhp;
        if (amt > 60)
            add_flag(flgs, OF_REFLECT);
    }

    if (p_ptr->tim_blood_seek)
        add_flag(flgs, OF_SLAY_LIVING);

    if (p_ptr->magicdef)
    {
        add_flag(flgs, OF_RES_BLIND);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_REFLECT);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_LEVITATION);
    }
    if ((p_ptr->tim_res_nether) || (IS_SPINNING()))
    {
        add_flag(flgs, OF_RES_NETHER);
    }
    if (p_ptr->tim_res_disenchantment)
    {
        add_flag(flgs, OF_RES_DISEN);
    }
    if (p_ptr->tim_sh_fire)
    {
        add_flag(flgs, OF_AURA_FIRE);
    }
    if (p_ptr->tim_sh_shards)
    {
        add_flag(flgs, OF_AURA_SHARDS);
    }
    if (p_ptr->tim_sh_elements)
    {
        add_flag(flgs, OF_AURA_FIRE);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_AURA_COLD);
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_AURA_ELEC);
    }
    if (p_ptr->ult_res)
    {
        add_flag(flgs, OF_RES_FEAR);
        add_flag(flgs, OF_RES_LITE);
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_RES_BLIND);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_SOUND);
        add_flag(flgs, OF_RES_SHARDS);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_CHAOS);
        add_flag(flgs, OF_RES_DISEN);
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_REFLECT);
        add_flag(flgs, OF_HOLD_LIFE);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_AURA_FIRE);
        add_flag(flgs, OF_AURA_ELEC);
        add_flag(flgs, OF_AURA_COLD);
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_LITE);
        add_flag(flgs, OF_SEE_INVIS);
        add_flag(flgs, OF_TELEPATHY);
        add_flag(flgs, OF_SLOW_DIGEST);
        add_flag(flgs, OF_REGEN);
        add_flag(flgs, OF_SUST_STR);
        add_flag(flgs, OF_SUST_INT);
        add_flag(flgs, OF_SUST_WIS);
        add_flag(flgs, OF_SUST_DEX);
        add_flag(flgs, OF_SUST_CON);
        add_flag(flgs, OF_SUST_CHR);
    }

    /* Hex bonuses */
    if (p_ptr->realm1 == REALM_HEX)
    {
        if (hex_spelling(HEX_DEMON_AURA))
        {
            add_flag(flgs, OF_AURA_FIRE);
            add_flag(flgs, OF_REGEN);
        }
        if (hex_spelling(HEX_ICE_ARMOR)) add_flag(flgs, OF_AURA_COLD);
        if (hex_spelling(HEX_SHOCK_CLOAK)) add_flag(flgs, OF_AURA_ELEC);
    }
}

void tim_player_stats(s16b stats[MAX_STATS])
{
    if (p_ptr->tsuyoshi)
    {
        stats[A_STR] += 4;
        stats[A_CON] += 4;
    }
    if (p_ptr->tim_building_up)
    {
        int amt = 4 * p_ptr->lev / 50; /* 13, 25, 38, 50 */
        stats[A_STR] += amt;
        stats[A_DEX] += amt;
        stats[A_CON] += amt;
    }
    if (p_ptr->realm1 == REALM_HEX)
    {
        if (hex_spelling(HEX_XTRA_MIGHT)) stats[A_STR] += 4;
        if (hex_spelling(HEX_BUILDING))
        {
            stats[A_STR] += 4;
            stats[A_DEX] += 4;
            stats[A_CON] += 4;
        }
    }
}

int ct_kills(void)
{
    int i;
    int result = 0;

    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            if (r_ptr->max_num == 0) result++;
        }
        else
        {
            result += r_ptr->r_pkills;
        }
    }

    return result;
}
int ct_kills_all(void)
{
    int i;
    int result = 0;

    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            if (r_ptr->max_num == 0) result++;
        }
        else
        {
            result += r_ptr->r_pkills;
        }
    }

    return result;
}

int ct_uniques(void)
{
    int i;
    int result = 0;

    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            if (r_ptr->max_num == 0) result++;
        }
    }

    return result;
}

int ct_artifacts(void)
{
    int i;
    int result = 0;

    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        if (!a_ptr->name) continue;
        if (!a_ptr->found) continue;
        result++;
    }

    return result;
}

/*
 *
 */
cptr map_name(void)
{
    quest_ptr q = quests_get_current();
    if (q && (q->flags & QF_GENERATE))
        return "Quest";
    else if (p_ptr->wild_mode)
        return "Surface";
    else if (p_ptr->inside_arena)
        return "Arena";
    else if (p_ptr->inside_battle)
        return "Monster Arena";
    else if (!dun_level && p_ptr->town_num)
        return town_name(p_ptr->town_num);
    else
        return d_name+d_info[dungeon_type].name;
}

/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
bool character_dump_hack = FALSE;
static errr file_character(cptr name, bool no_msgs)
{
    int        fd = -1;
    FILE        *fff = NULL;
    char        buf[1024];

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);

    /* Check for existing file */
    fd = fd_open(buf, O_RDONLY);

    /* Existing file */
    if (fd >= 0)
    {
        char out_val[160];

        /* Close the file */
        (void)fd_close(fd);

        /* Build query */
        (void)sprintf(out_val, "Replace existing file %s? ", buf);


        /* Ask */
        if (get_check_strict(out_val, CHECK_NO_HISTORY)) fd = -1;
    }

    /* Open the non-existing file */
    if (fd < 0) fff = my_fopen(buf, "w");

    /* Invalid file */
    if (!fff)
    {
        /* Message */
        prt("Character dump failed!", 0, 0);

        (void)inkey();

        /* Error */
        return (-1);
    }

    {
        doc_ptr doc = doc_alloc(80);
        int     format = DOC_FORMAT_TEXT;
        int     cb = strlen(name);

        if (cb > 5 && strcmp(name + cb - 5, ".html") == 0)
            format = DOC_FORMAT_HTML;
        else if (cb > 4 && strcmp(name + cb - 4, ".htm") == 0)
            format = DOC_FORMAT_HTML;

        py_display_character_sheet(doc);
        doc_write_file(doc, fff, format);
        doc_free(doc);
    }

    /* Close it */
    my_fclose(fff);


    if (!no_msgs)
    {
        /* Message */
        msg_print("Character dump successful.");

        if (!strpos("htm", name)) /* Assume we know better than the player */
        {
            char nuname[1024];
            int i, paikka = 0;
            strcpy(nuname, name);
            for (i = strlen(name) - 1; ((i > 0) && (i > (int)strlen(name) - 7)); i--)
            {
                unsigned char testi = name[i];
                if (testi == '/') break;
                if (testi == '\\') break;
                if (testi == '.')
                {
                    paikka = i + 1;
                    break;
                }
            }
            if ((!paikka) && (strlen(name))) paikka = strlen(name) + 1;
            if (paikka)
            {
                for (i = strlen(name) - 1; i > paikka - 2; i--)
                {
                    nuname[i] = '\0';
                }
            }
            strcat(nuname, ".html");
            file_character(nuname, TRUE);
            msg_format("Secondarily dumped as %s (angband.oook.cz ladder format)", nuname);
        }
        msg_print(NULL);
    }

    /* Success */
    return (0);
}


/*
 * Display single line of on-line help file
 *
 * You can insert some special color tag to change text color.
 * Such as...
 * WHITETEXT [[[[y|SOME TEXT WHICH IS DISPLAYED IN YELLOW| WHITETEXT
 *
 * A colored segment is between "[[[[y|" and the last "|".
 * You can use any single character in place of the "|".
 */
static void show_file_aux_line(cptr str, int cy, cptr shower)
{
    static const char tag_str[] = "[[[[";
    byte color = TERM_WHITE;
    char in_tag = '\0';
    int cx = 0;
    int i;
    char lcstr[1024];

    if (shower)
    {
        /* Make a lower case version of str for searching */
        strcpy(lcstr, str);
        str_tolower(lcstr);
    }

    /* Initial cursor position */
    Term_gotoxy(cx, cy);

    for (i = 0; str[i];)
    {
        int len = strlen(&str[i]);
        int showercol = len + 1;
        int bracketcol = len + 1;
        int endcol = len;
        cptr ptr;

        /* Search for a shower string in the line */
        if (shower)
        {
            ptr = my_strstr(&lcstr[i], shower);
            if (ptr) showercol = ptr - &lcstr[i];
        }

        /* Search for a color segment tag */
        ptr = in_tag ? my_strchr(&str[i], in_tag) : my_strstr(&str[i], tag_str);
        if (ptr) bracketcol = ptr - &str[i];

        /* A color tag is found */
        if (bracketcol < endcol) endcol = bracketcol;

        /* The shower string is found before the color tag */
        if (showercol < endcol) endcol = showercol;

        /* Print a segment of the line */
        Term_addstr(endcol, color, &str[i]);
        cx += endcol;
        i += endcol;

        /* Shower string? */
        if (endcol == showercol)
        {
            int showerlen = strlen(shower);

            /* Print the shower string in yellow */
            Term_addstr(showerlen, TERM_YELLOW, &str[i]);
            cx += showerlen;
            i += showerlen;
        }

        /* Colored segment? */
        else if (endcol == bracketcol)
        {
            if (in_tag)
            {
                /* Found the end of colored segment */
                i++;

                /* Now looking for an another tag_str */
                in_tag = '\0';

                /* Set back to the default color */
                color = TERM_WHITE;
            }
            else
            {
                /* Found a tag_str, and get a tag color */
                i += sizeof(tag_str)-1;

                /* Get tag color */
                color = color_char_to_attr(str[i]);

                /* Illegal color tag */
                if (color == 255 || str[i+1] == '\0')
                {
                    /* Illegal color tag */
                    color = TERM_WHITE;

                    /* Print the broken tag as a string */
                    Term_addstr(-1, TERM_WHITE, tag_str);
                    cx += sizeof(tag_str)-1;
                }
                else
                {
                    /* Skip the color tag */
                    i++;

                    /* Now looking for a close tag */
                    in_tag = str[i];

                    /* Skip the close-tag-indicator */
                    i++;
                }
            }
        }

    } /* for (i = 0; str[i];) */

    /* Clear rest of line */
    Term_erase(cx, cy, 255);
}


/*
 * Recursive file perusal.
 *
 * Process various special text in the input file, including
 * the "menu" structures used by the "help file" system.
 *
 * Return FALSE on 'q' to exit from a deep, otherwise TRUE.
 */
bool show_file(bool show_version, cptr name, cptr what, int line, int mode)
{
    int i, n, skey;

    /* Number of "real" lines passed by */
    int next = 0;

    /* Number of "real" lines in the file */
    int size = 0;

    /* Backup value for "line" */
    int back = 0;

    /* This screen has sub-screens */
    bool menu = FALSE;

    /* Current help file */
    FILE *fff = NULL;

    /* Find this string (if any) */
    cptr find = NULL;

    /* Jump to this tag */
    cptr tag = NULL;

    /* Hold strings to find/show */
    char finder_str[81];
    char shower_str[81];
    char back_str[81];

    /* String to show */
    cptr shower = NULL;

    /* Filename */
    char filename[1024];

    /* Describe this thing */
    char caption[128];

    /* Path buffer */
    char path[1024];

    /* General buffer */
    char buf[1024];

    /* Sub-menu information */
    char hook[68][32];

    bool reverse = (line < 0);

    int wid, hgt, rows;

    Term_get_size(&wid, &hgt);
    rows = hgt - 4;

    /* Wipe finder */
    strcpy(finder_str, "");

    /* Wipe shower */
    strcpy(shower_str, "");

    /* Wipe caption */
    strcpy(caption, "");

    /* Wipe the hooks */
    for (i = 0; i < 68; i++)
    {
        hook[i][0] = '\0';
    }

    /* Copy the filename */
    strcpy(filename, name);

    n = strlen(filename);

    /* Extract the tag from the filename */
    for (i = 0; i < n; i++)
    {
        if (filename[i] == '#')
        {
            filename[i] = '\0';
            tag = filename + i + 1;
            break;
        }
    }

    /* Redirect the name */
    name = filename;

    /* Hack XXX XXX XXX */
    if (what)
    {
        /* Caption */
        strcpy(caption, what);

        /* Access the "file" */
        strcpy(path, name);

        /* Open */
        fff = my_fopen(path, "r");
    }

    /* Look in "help" */
    if (!fff)
    {
        /* Caption */
        sprintf(caption, "Help file '%s'", name);


        /* Build the filename */
        path_build(path, sizeof(path), ANGBAND_DIR_HELP, name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Look in "info" */
    if (!fff)
    {
        /* Caption */
        sprintf(caption, "Info file '%s'", name);


        /* Build the filename */
        path_build(path, sizeof(path), ANGBAND_DIR_INFO, name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Look in "info" */
    if (!fff)
    {
        /* Build the filename */
        path_build(path, sizeof(path), ANGBAND_DIR, name);

        for (i = 0; path[i]; i++)
            if ('\\' == path[i])
                path[i] = PATH_SEP[0];

        /* Caption */
        sprintf(caption, "Info file '%s'", name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Oops */
    if (!fff)
    {
        /* Message */
        msg_format("Cannot open '%s'.", name);

        msg_print(NULL);

        /* Oops */
        return (TRUE);
    }


    /* Pre-Parse the file */
    while (TRUE)
    {
        char *str = buf;

        /* Read a line or stop */
        if (my_fgets(fff, buf, sizeof(buf))) break;

        /* XXX Parse "menu" items */
        if (prefix(str, "***** "))
        {
            /* Notice "menu" requests */
            if ((str[6] == '[') && isalpha(str[7]))
            {
                /* Extract the menu item */
                int k = str[7] - 'A';

                /* This is a menu file */
                menu = TRUE;

                if ((str[8] == ']') && (str[9] == ' '))
                {
                    /* Extract the menu item */
                    strncpy(hook[k], str + 10, 31);

                    /* Make sure it's null-terminated */
                    hook[k][31] = '\0';
                }
            }
            /* Notice "tag" requests */
            else if (str[6] == '<')
            {
                size_t len = strlen(str);

                if (str[len - 1] == '>')
                {
                    str[len - 1] = '\0';
                    if (tag && streq(str + 7, tag)) line = next;
                }
            }

            /* Skip this */
            continue;
        }

        /* Count the "real" lines */
        next++;
    }

    /* Save the number of "real" lines */
    size = next;

    /* start from bottom when reverse mode */
    if (line == -1) line = ((size-1)/rows)*rows;

    /* Clear screen */
    Term_clear();

    /* Display the file */
    while (TRUE)
    {
        /* Restart when necessary */
        if (line >= size - rows) line = size - rows;
        if (line < 0) line = 0;

        /* Re-open the file if needed */
        if (next > line)
        {
            /* Close it */
            my_fclose(fff);

            /* Hack -- Re-Open the file */
            fff = my_fopen(path, "r");

            /* Oops */
            if (!fff) return (FALSE);

            /* File has been restarted */
            next = 0;
        }

        /* Goto the selected line */
        while (next < line)
        {
            /* Get a line */
            if (my_fgets(fff, buf, sizeof(buf))) break;

            /* Skip tags/links */
            if (prefix(buf, "***** ")) continue;

            /* Count the lines */
            next++;
        }

        /* Dump the next 20, or rows, lines of the file */
        for (i = 0; i < rows; )
        {
            cptr str = buf;

            /* Hack -- track the "first" line */
            if (!i) line = next;

            /* Get a line of the file or stop */
            if (my_fgets(fff, buf, sizeof(buf))) break;

            /* Hack -- skip "special" lines */
            if (prefix(buf, "***** ")) continue;

            /* Count the "real" lines */
            next++;

            /* Hack -- keep searching */
            if (find && !i)
            {
                char lc_buf[1024];

                /* Make a lower case version of str for searching */
                strcpy(lc_buf, str);
                str_tolower(lc_buf);

                if (!my_strstr(lc_buf, find)) continue;
            }

            /* Hack -- stop searching */
            find = NULL;

            /* Dump the line */
            show_file_aux_line(str, i + 2, shower);

            /* Count the printed lines */
            i++;
        }

        while (i < rows)
        {
            /* Clear rest of line */
            Term_erase(0, i + 2, 255);

            i++;
        }

        /* Hack -- failed search */
        if (find)
        {
            bell();
            line = back;
            find = NULL;
            continue;
        }


        /* Show a general "title" */
        if (show_version)
        {
            prt(format(
                "[FrogComposband %d.%d.%s, %s, Line %d/%d]",
               VER_MAJOR, VER_MINOR, VER_PATCH,
               caption, line, size), 0, 0);
        }
        else
        {
            prt(format(
                "[%s, Line %d/%d]",
                caption, line, size), 0, 0);
        }

        /* Prompt -- small files */
        if (size <= rows)
        {
            /* Wait for it */
            prt("[Press ESC to exit.]", hgt - 1, 0);

        }

        /* Prompt -- large files */
        else
        {
            prt("[Press Return, Space, -, =, /, |, or ESC to exit.]", hgt - 1, 0);
        }

        /* Get a special key code */
        skey = inkey_special(TRUE);

        switch (skey)
        {
        /* Show the help for the help */
        case '?':
            /* Hack - prevent silly recursion
               TODO: I stole this file (helpinfo.txt) for the new help
               system (doc_display_help in z-doc.c). It is no longer accurate
               for show_file(), which I am in the process of replacing.
            if (strcmp(name, "helpinfo.txt") != 0)
                show_file(TRUE, "helpinfo.txt", NULL, 0, mode);*/
            break;

        /* Hack -- try showing */
        case '=':
            /* Get "shower" */
            prt("Show: ", hgt - 1, 0);

            strcpy(back_str, shower_str);
            if (askfor(shower_str, 80))
            {
                if (shower_str[0])
                {
                    /* Make it lowercase */
                    str_tolower(shower_str);

                    /* Show it */
                    shower = shower_str;
                }
                else shower = NULL; /* Stop showing */
            }
            else strcpy(shower_str, back_str);
            break;

        /* Hack -- try finding */
        case '/':
        case KTRL('s'):
            /* Get "finder" */
            prt("Find: ", hgt - 1, 0);

            strcpy(back_str, finder_str);
            if (askfor(finder_str, 80))
            {
                if (finder_str[0])
                {
                    /* Find it */
                    find = finder_str;
                    back = line;
                    line = line + 1;

                    /* Make finder lowercase */
                    str_tolower(finder_str);

                    /* Show it */
                    shower = finder_str;
                }
                else shower = NULL; /* Stop showing */
            }
            else strcpy(finder_str, back_str);
            break;

        /* Hack -- go to a specific line */
        case '#':
            {
                char tmp[81];
                prt("Goto Line: ", hgt - 1, 0);

                strcpy(tmp, "0");

                if (askfor(tmp, 80)) line = atoi(tmp);
            }
            break;

        /* Hack -- go to the top line */
        case SKEY_TOP:
        case '7': /* curses */
            line = 0;
            break;

        /* Hack -- go to the bottom line */
        case SKEY_BOTTOM:
        case '1': /* curses */
            line = ((size - 1) / rows) * rows;
            break;

        /* Hack -- go to a specific file */
        case '%':
            {
                char tmp[81];
                prt("Goto File: ", hgt - 1, 0);
                strcpy(tmp, "help.hlp");

                if (askfor(tmp, 80))
                {
                    if (!show_file(TRUE, tmp, NULL, 0, mode)) skey = 'q';
                }
            }
            break;

        /* Allow backing up */
        case '-':
            line = line + (reverse ? rows : -rows);
            if (line < 0) line = 0;
            break;

        /* One page up */
        case '9':
        case SKEY_PGUP:
            line = line - rows;
            if (line < 0) line = 0;
            break;

        /* Advance a single line */
        case '\n':
        case '\r':
            line = line + (reverse ? -1 : 1);
            if (line < 0) line = 0;
            break;

        /* Move up / down */
        case '8':
        case SKEY_UP:
            line--;
            if (line < 0) line = 0;
            break;

        case '2':
        case SKEY_DOWN:
            line++;
            break;

        /* Advance one page */
        case ' ':
            line = line + (reverse ? -rows : rows);
            if (line < 0) line = 0;
            break;

        /* One page down */
        case '3':
        case SKEY_PGDOWN:
            line = line + rows;
            break;
        }

        /* Recurse on numbers */
        if (menu)
        {
            int key = -1;

            if (!(skey & SKEY_MASK) && isalpha(skey))
                key = skey - 'A';

            if ((key > -1) && hook[key][0])
            {
                /* Recurse on that file */
                if (!show_file(TRUE, hook[key], NULL, 0, mode))
                    skey = 'q';
            }
        }

        /* Hack, dump to file */
        if (skey == '|')
        {
            FILE *ffp;
            char buff[1024];
            char xtmp[82];

            strcpy (xtmp, "");

            if (!get_string("File name: ", xtmp, 80)) continue;

            /* Close it */
            my_fclose(fff);

            /* Build the filename */
            path_build(buff, sizeof(buff), ANGBAND_DIR_USER, xtmp);

            /* Hack -- Re-Open the file */
            fff = my_fopen(path, "r");

            ffp = my_fopen(buff, "w");

            /* Oops */
            if (!(fff && ffp))
            {
                msg_print("Failed to open file.");
                skey = ESCAPE;
                break;
            }

            sprintf(xtmp, "%s: %s", player_name, what ? what : caption);
            my_fputs(ffp, xtmp, 80);
            my_fputs(ffp, "\n", 80);

            while (!my_fgets(fff, buff, sizeof(buff)))
                my_fputs(ffp, buff, 80);

            /* Close it */
            my_fclose(fff);
            my_fclose(ffp);

            /* Hack -- Re-Open the file */
            fff = my_fopen(path, "r");
        }

        /* Return to last screen */
        if (IS_ESCAPE(skey) || skey == '<') break;

        /* Exit on the ^q */
        if (skey == KTRL('q')) skey = 'q';

        /* Exit on the q key */
        if (skey == 'q') break;
    }

    /* Close the file */
    my_fclose(fff);

    /* Escape */
    if (skey == 'q') return (FALSE);

    /* Normal return */
    return (TRUE);
}


/*
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
    screen_save();
    doc_display_help("start.txt", NULL);
    screen_load();
}


/*
 * Process the player name.
 * Extract a clean "base name".
 * Build the savefile name if needed.
 */
void process_player_name(bool sf)
{
    int i, k = 0;
    char old_player_base[32] = "";

    if (character_generated) strcpy(old_player_base, player_base);

    /* Cannot be too long */
#if defined(MACINTOSH) || defined(MSDOS) || defined(USE_EMX) || defined(AMIGA) || defined(ACORN) || defined(VM)
#ifdef MSDOS
    if (strlen(player_name) > 8)
#else
    if (strlen(player_name) > 15)
#endif
    {
        /* Name too long */
        quit_fmt("The name '%s' is too long!", player_name);

    }
#endif

    /* Cannot contain "icky" characters */
    for (i = 0; player_name[i]; i++)
    {
        /* No control characters */
        if (iscntrl(player_name[i]))

        {
            /* Illegal characters */
            quit_fmt("The name '%s' contains control chars!", player_name);

        }
    }


#ifdef MACINTOSH

    /* Extract "useful" letters */
    for (i = 0; player_name[i]; i++)
    {
        char c = player_name[i];


        /* Convert "dot" to "underscore" */
        if (c == '.') c = '_';

        /* Accept all the letters */
        player_base[k++] = c;
    }

#else

    /* Extract "useful" letters */
    for (i = 0; player_name[i]; i++)
    {
        char c = player_name[i];

        /* Accept some letters */
        /* Convert path separator to underscore */
        if (!strncmp(PATH_SEP, player_name+i, strlen(PATH_SEP))){
            player_base[k++] = '_';
            i += strlen(PATH_SEP);
        }
        /* Convert some characters to underscore */
#ifdef MSDOS
        else if (my_strchr(" \"*+,./:;<=>?[\\]|", c)) player_base[k++] = '_';
#elif defined(WINDOWS)
        else if (my_strchr("\"*,/:;<>?\\|", c)) player_base[k++] = '_';
#endif
        else if (isprint(c)) player_base[k++] = c;
    }

#endif


#if defined(MSDOS)

    /* Hack -- max length */
    if (k > 8) k = 8;

#endif

    /* Terminate */
    player_base[k] = '\0';

    /* Require a "base" name */
    if (!player_base[0]) strcpy(player_base, "PLAYER");


#ifdef SAVEFILE_MUTABLE

    /* Accept */
    sf = TRUE;

#endif
    if (!savefile_base[0] && savefile[0])
    {
        cptr s;
        s = savefile;
        while (1)
        {
            cptr t;
            t = my_strstr(s, PATH_SEP);
            if (!t)
                break;
            s = t+1;
        }
        strcpy(savefile_base, s);
    }

    if (!savefile_base[0] || !savefile[0])
        sf = TRUE;

    /* Change the savefile name */
    if (sf)
    {
        char temp[128];

        strcpy(savefile_base, player_base);

#ifdef SAVEFILE_USE_UID
        /* Rename the savefile, using the player_uid and player_base */
        (void)sprintf(temp, "%d.%s", player_uid, player_base);
#else
        /* Rename the savefile, using the player_base */
        (void)sprintf(temp, "%s", player_base);
#endif

#ifdef VM
        /* Hack -- support "flat directory" usage on VM/ESA */
        (void)sprintf(temp, "%s.sv", player_base);
#endif /* VM */

        /* Build the filename */
        path_build(savefile, sizeof(savefile), ANGBAND_DIR_SAVE, temp);
    }

    /* Load an autopick preference file */
    if (character_generated)
    {
        if (!streq(old_player_base, player_base)) autopick_load_pref(FALSE);
    }
}


bool py_get_name(void)
{
    bool result = FALSE;
    char tmp[64];

    /* Save the player name */
    strcpy(tmp, player_name);

    /* Check to see if this is for server play. If so, lock the player name. (--phantom) */
    if(!arg_lock_name)
    {
        if (get_string("Enter a name for your character: ", tmp, PY_NAME_LEN))
        {
            /* Use the name */
            strcpy(player_name, tmp);
            result = TRUE;
        }

        if (0 == strlen(player_name))
        {
            /* Use default name */
            strcpy(player_name, "PLAYER");
            result = TRUE;
        }
    }
    return result;
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
{
    int i;

    /* Flush input */
    flush();

    /* Verify Retirement */
    if (p_ptr->total_winner)
    {
        /* Verify */
        if (!get_check_strict("Do you want to retire? ", CHECK_NO_HISTORY)) return;

    }

    /* Verify Suicide */
    else
    {
        /* Verify */
        if (!get_check("Do you really want to commit suicide? ")) return;
    }


    if (!p_ptr->noscore)
    {
        /* Special Verification for suicide */
        prt("Please verify SUICIDE by typing the '@' sign (or pressing Ctrl-E): ", 0, 0);

        flush();
        i = inkey();
        prt("", 0, 0);
        if ((i != '@') && (i != KTRL('E'))) return;
    }

    /* Initialize "last message" buffer */
    if (p_ptr->last_message) z_string_free(p_ptr->last_message);
    p_ptr->last_message = NULL;

    /* Hack -- Note *winning* message */
    if (p_ptr->total_winner && last_words)
    {
        char buf[1024] = "";

        do
        {
            while (!get_string("*Winning* message: ", buf, sizeof buf)) ;
        }
        while (!get_check_strict("Are you sure? ", CHECK_NO_HISTORY));

        if (buf[0])
        {
            p_ptr->last_message = z_string_make(buf);
            msg_print(p_ptr->last_message);
        }
    }

    /* Stop playing */
    p_ptr->playing = FALSE;

    /* Kill the player */
    p_ptr->is_dead = TRUE;

    /* Leaving */
    p_ptr->leaving = TRUE;

    /* Cause of death */
    (void)strcpy(p_ptr->died_from, p_ptr->total_winner ? "Ripe Old Age" : "Quitting");
}



/*
 * Save the game
 */
void do_cmd_save_game(int is_autosave)
{
    if (!is_autosave)
        disturb(1, 0);

    handle_stuff();

    if (!is_autosave)
        prt("Saving game...", 0, 0);

    Term_fresh();

    /* The player is not dead */
    (void)strcpy(p_ptr->died_from, "(saved)");

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Save the player */
    if (save_player())
    {
        if (!is_autosave)
            prt("Saving game... done.", 0, 0);
    }
    /* Save failed (oops) */
    else
    {
        prt("Saving game... failed!", 0, 0);
    }

    /* Allow suspend again */
    signals_handle_tstp();

    /* Refresh */
    Term_fresh();

    /* Note that the player is not dead */
    (void)strcpy(p_ptr->died_from, "(alive and well)");

    /* HACK -- don't get sanity blast on updating view */
    hack_mind = FALSE;

    /* Update stuff */
    update_stuff();

    /* HACK -- reset the hackish flag */
    hack_mind = TRUE;
}


/*
 * Save the game and exit
 */
void do_cmd_save_and_exit(void)
{
    p_ptr->playing = FALSE;

    /* Leaving */
    p_ptr->leaving = TRUE;
}


/*
 * Hack -- Calculates the total number of points earned        -JWT-
 */
long total_points(void)
{
    int i, mult = 100;
    s16b max_dl = 0;
    u32b point, point_h, point_l;
    int arena_win = MIN(p_ptr->arena_number, MAX_ARENA_MONS);

    if (!preserve_mode) mult += 10;
    if (!smart_learn) mult -= 20;
    if (smart_cheat) mult += 30;
    if (ironman_shops) mult += 50;
    if (ironman_empty_levels) mult += 20;
    if (ironman_nightmare) mult += 100;
    if (wacky_rooms) mult += 10;
    if (thrall_mode) mult += (p_ptr->personality == PERS_SEXY) ? 50 : 10;
    if (easy_damage) mult /= 2;
    if (coffee_break)
    {
        mult /= 2; /* This only cancels out the x2 from ironman_downward */
        mult -= (p_ptr->coffee_lv_revisits / 2);
    }

    if (mult < 5) mult = 5;

    for (i = 0; i < max_d_idx; i++)
        if(max_dlv[i] > max_dl)
            max_dl = max_dlv[i];

    point_l = (p_ptr->max_max_exp + (100 * max_dl));
    point_h = point_l / 0x10000L;
    point_l = point_l % 0x10000L;
    point_h *= mult;
    point_l *= mult;
    point_h += point_l / 0x10000L;
    point_l %= 0x10000L;

    point_l += ((point_h % 100) << 16);
    point_h /= 100;
    point_l /= 100;

    point = (point_h << 16) + (point_l);
    if (p_ptr->arena_number >= 0)
        point += (arena_win * arena_win * (arena_win > 29 ? 1000 : 100));

    if (ironman_downward) point *= 2;
    if (p_ptr->pclass == CLASS_BERSERKER)
    {
        if ( p_ptr->prace == RACE_SPECTRE )
            point = point / 5;
    }

    if ((p_ptr->personality == PERS_MUNCHKIN) && point)
    {
        if (p_ptr->total_winner) point = point / 10;
        else point = 1;
    }

    return point;
}


#define GRAVE_LINE_WIDTH 31

/*
 * Centers a string within a GRAVE_LINE_WIDTH character string        -JWT-
 */
static void center_string(char *buf, cptr str)
{
    int i, j;

    /* Total length */
    i = strlen(str);

    /* Necessary border */
    j = GRAVE_LINE_WIDTH / 2 - i / 2;

    /* Mega-Hack */
    (void)sprintf(buf, "%*s%s%*s", j, "", str, GRAVE_LINE_WIDTH - i - j, "");
}


#if 0
/*
 * Save a "bones" file for a dead character
 *
 * Note that we will not use these files until Angband 2.8.0, and
 * then we will only use the name and level on which death occured.
 *
 * Should probably attempt some form of locking...
 */
static void make_bones(void)
{
    FILE                *fp;

    char                str[1024];


    /* Ignore wizards and borgs */
    if (!(p_ptr->noscore & 0x00FF))
    {
        /* Ignore people who die in town */
        if (dun_level)
        {
            char tmp[128];

            /* XXX XXX XXX "Bones" name */
            sprintf(tmp, "bone.%03d", dun_level);

            /* Build the filename */
            path_build(str, sizeof(str), ANGBAND_DIR_BONE, tmp);

            /* Attempt to open the bones file */
            fp = my_fopen(str, "r");

            /* Close it right away */
            if (fp) my_fclose(fp);

            /* Do not over-write a previous ghost */
            if (fp) return;

            /* File type is "TEXT" */
            FILE_TYPE(FILE_TYPE_TEXT);

            /* Grab permissions */
            safe_setuid_grab();

            /* Try to write a new "Bones File" */
            fp = my_fopen(str, "w");

            /* Drop permissions */
            safe_setuid_drop();

            /* Not allowed to write it?  Weird. */
            if (!fp) return;

            /* Save the info */
            fprintf(fp, "%s\n", player_name);
            fprintf(fp, "%d\n", p_ptr->mhp);
            fprintf(fp, "%d\n", p_ptr->prace);
            fprintf(fp, "%d\n", p_ptr->pclass);

            /* Close and save the Bones file */
            my_fclose(fp);
        }
    }
}
#endif


/*
 * Redefinable "print_tombstone" action
 */
bool (*tombstone_aux)(void) = NULL;


/*
 * Display a "tomb-stone"
 */
static void print_tomb(void)
{
    bool done = FALSE;

    /* Do we use a special tombstone ? */
    if (tombstone_aux)
    {
        /* Use tombstone hook */
        done = (*tombstone_aux)();
    }

    /* Print the text-tombstone */
    if (!done)
    {
        cptr   p;
        char   tmp[160];
        char   buf[1024];
        char   dummy[80];
        char   *t;
        FILE   *fp;
        time_t ct = time(0);

        /* Clear screen */
        Term_clear();

        /* Build the filename */
        path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "dead.txt");

        /* Open the News file */
        fp = my_fopen(buf, "r");

        /* Dump */
        if (fp)
        {
            int i = 0;

            /* Dump the file to the screen */
            while (0 == my_fgets(fp, buf, sizeof(buf)))
            {
                /* Display and advance */
                put_str(buf, i++, 0);
            }

            /* Close */
            my_fclose(fp);
        }

        /* King or Queen */
        if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
        {
            p = "Magnificent";
        }

        /* Normal */
        else
        {
            p =  "Vanquished";
        }

        center_string(buf, player_name);
        put_str(buf, 6, 11);

        center_string(buf, "the");
        put_str(buf, 7, 11);

        center_string(buf, p);
        put_str(buf, 8, 11);

        center_string(buf, get_class()->name);
        put_str(buf, 10, 11);

        (void)sprintf(tmp, "Level: %d", (int)p_ptr->lev);
        center_string(buf, tmp);
        put_str(buf, 11, 11);

        (void)sprintf(tmp, "Exp: %d", p_ptr->exp);
        center_string(buf, tmp);
        put_str(buf, 12, 11);

        (void)sprintf(tmp, "AU: %d", p_ptr->au);
        center_string(buf, tmp);
        put_str(buf, 13, 11);

        (void)sprintf(tmp, "Killed on Level %d", dun_level);
        center_string(buf, tmp);
        put_str(buf, 14, 11);

        roff_to_buf(format("by %s.", p_ptr->died_from), GRAVE_LINE_WIDTH + 1, tmp, sizeof tmp);
        center_string(buf, tmp);
        put_str(buf, 15, 11);
        t = tmp + strlen(tmp) + 1;
        if (*t)
        {
            strcpy(dummy, t); /* 2nd line */
            if (*(t + strlen(t) + 1)) /* Does 3rd line exist? */
            {
                int dummy_len = strlen(dummy);
                strcpy(dummy + MIN(dummy_len, GRAVE_LINE_WIDTH - 3), "...");
            }
            center_string(buf, dummy);
            put_str(buf, 16, 11);
        }

        (void)sprintf(tmp, "%-.24s", ctime(&ct));
        center_string(buf, tmp);
        put_str(buf, 17, 11);

        msg_format("Goodbye, %s!", player_name);
    }
}


/*
 * Display some character info
 */
static void show_info(void)
{
    pack_for_each(obj_identify);
    equip_for_each(obj_identify);
    quiver_for_each(obj_identify);
    home_for_each(obj_identify);

    pack_optimize();
    quiver_optimize();
    home_optimize();

    /* Hack -- Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Flush all input keys */
    flush();

    /* Flush messages */
    msg_print(NULL);

    /* Describe options */
    prt("You may now dump a character record to one or more files.", 21, 0);
    prt("Then, hit RETURN to see the character, or ESC to abort.", 22, 0);


    /* Dump character records as requested */
    while (TRUE)
    {
        char out_val[160];

        /* Prompt */
        put_str("Filename: ", 23, 0);


        /* Default */
        strcpy(out_val, "");

        /* Ask for filename (or abort) */
        if (!askfor(out_val, 60)) return;

        /* Return means "show on screen" */
        if (!out_val[0]) break;

        /* Save screen */
        screen_save();

        /* Dump a character file */
        (void)file_character(out_val, FALSE);

        /* Load screen */
        screen_load();
    }

    update_playtime();

    /* Display player */
    py_display();
}


bool check_score(void)
{
    /* Clear screen */
    Term_clear();

#ifndef SCORE_WIZARDS
    /* Wizard-mode pre-empts scoring */
    if (p_ptr->noscore & 0x000F)
    {
        msg_print("Score not registered for wizards.");

        msg_print(NULL);
        return FALSE;
    }
#endif

#ifndef SCORE_BORGS
    /* Borg-mode pre-empts scoring */
    if (p_ptr->noscore & 0x00F0)
    {
        msg_print("Score not registered for borgs.");

        msg_print(NULL);
        return FALSE;
    }
#endif

#ifndef SCORE_CHEATERS
    /* Cheaters are not scored */
    if (p_ptr->noscore & 0xFF00)
    {
        msg_print("Score not registered for cheaters.");

        msg_print(NULL);
        return FALSE;
    }
#endif

    /* Interupted */
    if (!p_ptr->total_winner && streq(p_ptr->died_from, "Interrupting"))

    {
        msg_print("Score not registered due to interruption.");

        msg_print(NULL);
        return FALSE;
    }

    /* Quitter 
    if (!p_ptr->total_winner && streq(p_ptr->died_from, "Quitting"))

    {
        msg_print("Score not registered due to quitting.");

        msg_print(NULL);
        return FALSE;
    } */
    return TRUE;
}

/*
 * Change the player into a King!            -RAK-
 */
void kingly(void)
{
    int wid, hgt;
    int cx, cy;

    /* Hack -- retire in town */
    dun_level = 0;

    /* Restore the experience */
    p_ptr->exp = p_ptr->max_exp;

    /* Restore the level */
    p_ptr->lev = p_ptr->max_plv;

    Term_get_size(&wid, &hgt);
    cy = hgt / 2;
    cx = wid / 2;

    /* Hack -- Instant Gold */
    p_ptr->au += 10000000;
    stats_on_gold_winnings(10000000);

    /* Clear screen */
    Term_clear();

    /* Display a crown */
    if (!coffee_break) {
        put_str("#", cy - 11, cx - 1);
        put_str("#####", cy - 10, cx - 3);
        put_str("#", cy - 9, cx - 1);
        put_str(",,,  $$$  ,,,", cy - 8, cx - 7);
        put_str(",,=$   \"$$$$$\"   $=,,", cy - 7, cx - 11);
        put_str(",$$        $$$        $$,", cy - 6, cx - 13);
        put_str("*>         <*>         <*", cy - 5, cx - 13);
        put_str("$$         $$$         $$", cy - 4, cx - 13);
        put_str("\"$$        $$$        $$\"", cy - 3, cx - 13);
        put_str("\"$$       $$$       $$\"", cy - 2, cx - 12);
        put_str("*#########*#########*", cy - 1, cx - 11);
        put_str("*#########*#########*", cy, cx - 11); }
    else { /* Display a coffee cup */
        put_str("	 $$$$$$$", cy - 11, cx - 8);
        put_str(" .-''$$$$$$$''-.", cy - 10, cx - 10);
        put_str("((-''$$$$$$$$'-))", cy - 9, cx - 10);
        put_str("|'=::_______::='|", cy - 8, cx - 10);
        put_str("|             __|", cy - 7, cx - 10);
        put_str("|            (_ '-. ", cy - 6, cx - 10);
        put_str("|              '-. ) ", cy - 5, cx - 10);
        put_str("|               || |", cy - 4, cx - 10);
        put_str("|             _ || |", cy - 3, cx - 10);
        put_str("|            ( '-| |", cy - 2, cx - 10);
        put_str("|             '-.__)", cy - 1, cx - 10);
        put_str(" '-.._______..-'", cy, cx - 10);
    }

    /* Display a message */
    put_str("Veni, Vidi, Vici!", cy + 3, cx - 9);
    put_str("I came, I saw, I conquered!", cy + 4, cx - 14);
    put_str(format("All Hail the Mighty %s!", sex_info[p_ptr->psex].winner), cy + 5, cx - 13);


    /* Flush input */
    flush();

    /* Wait for response */
    pause_line(hgt - 1);
}
/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 */
void close_game(void)
{

    /* Handle stuff */
    handle_stuff();

    /* Flush the messages */
    msg_print(NULL);

    /* Flush the input */
    flush();


    /* No suspending now */
    signals_ignore_tstp();


    /* Hack -- Character is now "icky" */
    character_icky = TRUE;


    /* Handle death */
    if (p_ptr->is_dead)
    {
        /* Handle retirement */
        if ((p_ptr->total_winner) && ((strpos("Ripe Old Age", p_ptr->died_from)) || (strpos("Seppuku", p_ptr->died_from)))) kingly();

        /* Save memories */
        if (!cheat_save || get_check("Save death? "))
        {
            if (!save_player()) msg_print("death save failed!");
        }

        /* You are dead */
        print_tomb();

        flush();

        /* Show more info */
        show_info();

        /* Clear screen */
        Term_clear();

#if 0
        /* Dump bones file */
        make_bones();
#endif
    }

    /* Still alive */
    else
    {
        /* Save the game */
        do_cmd_save_game(FALSE);
    }

    if (check_score())
        scores_update();

    /* Kill all temporal files */
    clear_saved_floor_files();

    /* Allow suspending now */
    signals_handle_tstp();
}


/*
 * Handle abrupt death of the visual system
 *
 * This routine is called only in very rare situations, and only
 * by certain visual systems, when they experience fatal errors.
 *
 * XXX XXX Hack -- clear the death flag when creating a HANGUP
 * save file so that player can see tombstone when restart.
 */
void exit_game_panic(void)
{
    /* If nothing important has happened, just quit */
    if (!character_generated || character_saved) quit("panic");

    /* Clear the top line */
    msg_line_clear();

    /* Hack -- turn off some things */
    disturb(1, 0);

    /* Mega-Hack -- Delay death */
    if (p_ptr->chp < 0) p_ptr->is_dead = FALSE;

    /* Hardcode panic save */
    p_ptr->panic_save = 1;

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Indicate panic save */
    (void)strcpy(p_ptr->died_from, "(panic save)");


    /* Panic save, or get worried */
    if (!save_player()) quit("panic save failed!");


    /* Successful panic save */
    quit("panic save succeeded!");
}


/*
 * Get a random line from a file
 * Based on the monster speech patch by Matt Graham,
 */
errr get_rnd_line(cptr file_name, int entry, char *output)
{
    FILE    *fp;
    char    buf[1024];
    int     counter, test;
    int     line_num = 0;


    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, file_name);

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Failed */
    if (!fp) return -1;

    /* Find the entry of the monster */
    while (TRUE)
    {
        /* Get a line from the file */
        if (my_fgets(fp, buf, sizeof(buf)) == 0)
        {
            /* Count the lines */
            line_num++;

            /* Look for lines starting with 'N:' */
            if ((buf[0] == 'N') && (buf[1] == ':'))
            {
                /* Allow default lines */
                if (buf[2] == '*')
                {
                    /* Default lines */
                    break;
                }
                else if (buf[2] == 'M')
                {
                    if (r_info[entry].flags1 & RF1_MALE) break;
                }
                else if (buf[2] == 'F')
                {
                    if (r_info[entry].flags1 & RF1_FEMALE) break;
                }
                /* Get the monster number */
                else if (sscanf(&(buf[2]), "%d", &test) != EOF)
                {
                    /* Is it the right number? */
                    if (test == entry) break;
                }
                else
                {
                    /* Error while converting the number */
                    msg_format("Error in line %d of %s!", line_num, file_name);
                    my_fclose(fp);
                    return -1;
                }
            }
        }
        else
        {
            /* Reached end of file */
            my_fclose(fp);
            return -1;
        }
    }

    /* Get the random line */
    for (counter = 0; ; counter++)
    {
        while (TRUE)
        {
            test = my_fgets(fp, buf, sizeof(buf));

            /* Count the lines */
            /* line_num++; No more needed */

            if (!test)
            {
                /* Ignore lines starting with 'N:' */
                if ((buf[0] == 'N') && (buf[1] == ':')) continue;

                if (buf[0] != '#') break;
            }
            else break;
        }

        /* Abort */
        if (!buf[0]) break;

        /* Copy the line */
        if (one_in_(counter + 1))
            strcpy(output, buf);
    }

    /* Close the file */
    my_fclose(fp);

    /* Success */
    return counter ? 0 : -1;
}



/*
 * Process file for auto picker/destroyer.
 */
errr process_autopick_file(cptr name)
{
    char buf[1024];

    errr err = 0;

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

    err = process_pref_file_aux(buf, PREF_TYPE_AUTOPICK);

    /* Result */
    return (err);
}


static errr counts_seek(int fd, u32b where, bool flag)
{
    huge seekpoint;
    char temp1[128], temp2[128];
    u32b zero_header[3] = {0L, 0L, 0L};
    int i;

#ifdef SAVEFILE_USE_UID
    (void)sprintf(temp1, "%d.%s.%d%d%d", player_uid, savefile_base, p_ptr->pclass, p_ptr->personality, 0);
#else
    (void)sprintf(temp1, "%s.%d%d%d", savefile_base, p_ptr->pclass, p_ptr->personality, 0);
#endif
    for (i = 0; temp1[i]; i++)
        temp1[i] ^= (i+1) * 63;

    seekpoint = 0;
    while (1)
    {
        if (fd_seek(fd, seekpoint + 3 * sizeof(u32b)))
            return 1;
        if (fd_read(fd, (char*)(temp2), sizeof(temp2)))
        {
            if (!flag)
                return 1;
            /* add new name */
            fd_seek(fd, seekpoint);
            fd_write(fd, (char*)zero_header, 3*sizeof(u32b));
            fd_write(fd, (char*)(temp1), sizeof(temp1));
            break;
        }

        if (strcmp(temp1, temp2) == 0)
            break;

        seekpoint += 128 + 3 * sizeof(u32b);
    }

    return fd_seek(fd, seekpoint + where * sizeof(u32b));
}

u32b counts_read(int where)
{
    int fd;
    u32b count = 0;
    char buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, "z_info.raw");
    fd = fd_open(buf, O_RDONLY);

    if (counts_seek(fd, where, FALSE) ||
        fd_read(fd, (char*)(&count), sizeof(u32b)))
        count = 0;

    (void)fd_close(fd);

    return count;
}

errr counts_write(int where, u32b count)
{
    int fd;
    char buf[1024];
    errr err;

    path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, "z_info.raw");

    /* Grab permissions */
    safe_setuid_grab();

    fd = fd_open(buf, O_RDWR);

    /* Drop permissions */
    safe_setuid_drop();

    if (fd < 0)
    {
        /* File type is "DATA" */
        FILE_TYPE(FILE_TYPE_DATA);

        /* Grab permissions */
        safe_setuid_grab();

        /* Create a new high score file */
        fd = fd_make(buf, 0644);

        /* Drop permissions */
        safe_setuid_drop();
    }

    /* Grab permissions */
    safe_setuid_grab();

    err = fd_lock(fd, F_WRLCK);

    /* Drop permissions */
    safe_setuid_drop();

    if (err) return 1;

    counts_seek(fd, where, TRUE);
    fd_write(fd, (char*)(&count), sizeof(u32b));

    /* Grab permissions */
    safe_setuid_grab();

    err = fd_lock(fd, F_UNLCK);

    /* Drop permissions */
    safe_setuid_drop();

    if (err) return 1;

    (void)fd_close(fd);

    return 0;
}


#ifdef HANDLE_SIGNALS

#include <signal.h>


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);

#ifdef SIGSTOP

    /* Flush output */
    Term_fresh();

    /* Suspend the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 0);

    /* Suspend ourself */
    (void)kill(0, SIGSTOP);

    /* Resume the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 1);

    /* Redraw the term */
    Term_redraw();

    /* Flush the term */
    Term_fresh();

#endif

    /* Restore handler */
    (void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly. We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);


    /* Count the signals */
    signal_count++;


    /* Terminate dead characters */
    if (p_ptr->is_dead)
    {
        /* Mark the savefile */
        (void)strcpy(p_ptr->died_from, "Abortion");

        forget_lite();
        forget_view();
        clear_mon_lite();

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");

    }

    /* Allow suicide (after 5) */
    else if (signal_count >= 5)
    {
        /* Cause of "death" */
        (void)strcpy(p_ptr->died_from, "Interrupting");


        forget_lite();
        forget_view();
        clear_mon_lite();

        /* Stop playing */
        p_ptr->playing = FALSE;

        /* Suicide */
        p_ptr->is_dead = TRUE;

        /* Leaving */
        p_ptr->leaving = TRUE;

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");

    }

    /* Give warning (after 4) */
    else if (signal_count >= 4)
    {
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);

        /* Clear the top line */
        Term_erase(0, 0, 255);

        /* Display the cause */
        Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");


        /* Flush */
        Term_fresh();
    }

    /* Give warning (after 2) */
    else if (signal_count >= 2)
    {
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);
    }

    /* Restore handler */
    (void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
    int wid, hgt;

    Term_get_size(&wid, &hgt);

    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);


    forget_lite();
    forget_view();
    clear_mon_lite();

    /* Clear the bottom line */
    Term_erase(0, hgt - 1, 255);

    /* Give a warning */
    Term_putstr(0, hgt - 1, -1, TERM_RED,
            "A gruesome software bug LEAPS out at you!");


    /* Message */
    Term_putstr(45, hgt - 1, -1, TERM_RED, "Panic save...");



    /* Flush output */
    Term_fresh();

    /* Panic Save */
    p_ptr->panic_save = 1;

    /* Panic save */
    (void)strcpy(p_ptr->died_from, "(panic save)");

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Attempt to save */
    if (save_player())
    {
        Term_putstr(45, hgt - 1, -1, TERM_RED, "Panic save succeeded!");
    }

    /* Save failed */
    else
    {
        Term_putstr(45, hgt - 1, -1, TERM_RED, "Panic save failed!");
    }

    /* Flush output */
    Term_fresh();

    /* Quit */
    quit("software bug");
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init(void)
{

#ifdef SIGHUP
    (void)signal(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
    (void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
    (void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
    (void)signal(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
    (void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
    (void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
    (void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
    (void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
    (void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
    (void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
    (void)signal(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
    (void)signal(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
    (void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
    (void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
    (void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
    (void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
    (void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else    /* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}
#endif    /* HANDLE_SIGNALS */
