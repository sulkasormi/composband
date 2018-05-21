/*
 * File: randname.c
 * Purpose: Random name generation
 *
 * Copyright (c) 2007 Antony Sidwell, Sheldon Simms
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "randname.h"

#include <assert.h>

/* Markers for the start and end of words. */
#define S_WORD 26
#define E_WORD S_WORD
#define TOTAL  27

typedef unsigned short name_probs[S_WORD+1][S_WORD+1][TOTAL+1];

/*
 * This function builds probability tables from a list of purely alphabetical
 * lower-case words, and puts them into the supplied name_probs object.
 * The array of names should have a NULL entry at the end of the list.
 * It relies on the ASCII character set (through use of A2I).
 */
static void _build_prob(name_probs probs, const char **learn)
{
	int c_prev, c_cur, c_next;
	const char *ch;
	int i;

	/* Build raw frequencies */
	for (i = 0; learn[i] != NULL; i++)
	{
		c_prev = c_cur = S_WORD;
		ch = learn[i];

		/* Iterate over the next word */
		while (*ch != '\0')
		{
			c_next = A2I(tolower((unsigned char)*ch));

			probs[c_prev][c_cur][c_next]++;
			probs[c_prev][c_cur][TOTAL]++;
                        
			/* Step on */
			c_prev = c_cur;
			c_cur = c_next;
			ch++;
		}

		probs[c_prev][c_cur][E_WORD]++;
		probs[c_prev][c_cur][TOTAL]++;
	}
}

/*
 * Use W. Sheldon Simms' random name generator algorithm (Markov Chain stylee).
 * 
 * Generate a random word using the probability tables we built earlier.  
 * Relies on the A2I and I2A macros (and so the ASCII character set) and 
 * is_a_vowel (so the basic 5 English vowels).
 */
size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***sections)
{
	size_t lnum = 0;
	bool found_word = FALSE;

	static name_probs lprobs;
	static randname_type cached_type = RANDNAME_NUM_TYPES;

	assert(name_type > 0 && name_type < RANDNAME_NUM_TYPES);

	/* To allow for a terminating character */
	assert(buflen > max);

	/* We cache one set of probabilities, only regenerate when
	   the type changes.  It's as good a way as any for now.
	   Frankly, we could probably regenerate every time. */
	if (cached_type != name_type)
	{
		const char **wordlist = NULL;

		wordlist = sections[name_type];

		(void)WIPE(lprobs, name_probs);
		_build_prob(lprobs, wordlist);

		cached_type = name_type;
	}
        
	/* Generate the actual word wanted. */
	while (!found_word)
	{
		char *cp = word_buf;
		int c_prev = S_WORD;
		int c_cur = S_WORD;
		int tries = 0;
		bool contains_vowel = FALSE;
		lnum = 0;

		/* We start the word again if we run out of space or have
		   had to have 10 goes to find a word that satisfies the
		   minimal conditions. */
		while (tries < 10 && lnum <= max && !found_word)
		{
			/* Pick the next letter based on a simple weighting
			  of which letters can follow the previous two */
			int r;
			int c_next = 0;

			assert(c_prev >= 0 && c_prev <= S_WORD);
			assert(c_cur >= 0 && c_cur <= S_WORD);

			r = randint0(lprobs[c_prev][c_cur][TOTAL]);

			while (r >= lprobs[c_prev][c_cur][c_next])
			{
				r -= lprobs[c_prev][c_cur][c_next];
				c_next++;
			}

			assert(c_next <= E_WORD);
			assert(c_next >= 0);
            
			if (c_next == E_WORD)
			{
				/* If we've reached the end, we check if we've
				   met the simple conditions, otherwise have
				   another go at choosing a letter for this
				   position. */
				if (lnum >= min && contains_vowel)
				{
					*cp = '\0';
					found_word = TRUE;
				}
				else
				{
					tries++;
				}
			}
			else
			{
				/* Add the letter to the word and move on. */
				*cp = I2A(c_next);

				if (is_a_vowel(*cp))
					contains_vowel = TRUE;

				cp++;
				lnum++;
				assert(c_next <= S_WORD);
				assert(c_next >= 0);
				c_prev = c_cur;
				c_cur = c_next;
			}
		}
	}

	return lnum;
}

/* Parsing functions for namebase.txt (random name fragments) */
struct name {
	struct name *next;
	const char *str;
};

struct names_parse {
	unsigned int section;
	unsigned int nnames[RANDNAME_NUM_TYPES];
	struct name *names[RANDNAME_NUM_TYPES];
};

/*
 * Initialize the "name_sections" array, by parsing an ascii "template" file
 */
errr _parse_namebase(char *buf, struct names_parse *n)
{
    if (buf[0] == 'N')
    {
        int section;
        
        if (n->section >= RANDNAME_NUM_TYPES)
        return PARSE_ERROR_GENERIC;
        if (sscanf(buf+2, "%d", &section) != 1) return (1);
        n->section = section;
    }
    else if (buf[0] == 'D')
    {
        struct name *ns = malloc(sizeof *ns);
        const char *nimi = buf+2;

        if (!nimi || !strlen(nimi)) return PARSE_ERROR_TOO_FEW_ARGUMENTS;
        memset(ns, 0, sizeof(*ns));
        n->nnames[n->section]++;
        ns->next = n->names[n->section];
        ns->str = z_string_make(nimi);
        n->names[n->section] = ns;
    }
        /* Oops */
    else return PARSE_ERROR_UNDEFINED_DIRECTIVE;

    /* Success */
    return (0);
}

static void _finish_name_parse(struct names_parse *n) {
	int i;
	unsigned int j;
	int name_sec_size = (sizeof(char**)) * RANDNAME_NUM_TYPES;
	struct name *nm;
//	C_MAKE(name_sections, RANDNAME_NUM_TYPES, char**);
	name_sections = malloc(name_sec_size);
	memset(name_sections, 0, name_sec_size);
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		name_sections[i] = malloc(sizeof(char*) * (n->nnames[i] + 1));
		for (nm = n->names[i], j = 0; nm && j < n->nnames[i]; nm = nm->next, j++) {
			name_sections[i][j] = nm->str;
		}
		name_sections[i][n->nnames[i]] = NULL;
		while (n->names[i]) {
			nm = n->names[i]->next;
			FREE(n->names[i], struct name);
			n->names[i] = nm;
		}
	}
	FREE(n, struct names_parse);
}

/* Can be used to free name memory. Currently we just seem to rely
 * on all memory being freed on game exit anyhow
 * Note that mem_free() is a Vanilla thing and not available here */
/*
static void cleanup_names(void)
{
	int i, j;
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		for (j = 0; name_sections[i][j]; j++) {
			z_string_free((char *)name_sections[i][j]);
		}
		mem_free(name_sections[i]);
	}
	mem_free(name_sections);
}
*/

/* Ugly parser. Mashup of code from Vanilla 3.5.1 and PosChengband */
errr name_parser(void)
{
    FILE *fp;
    char buf[1024];
    errr err;
    struct names_parse *n;
    int err_line = 0;

    /* Build path */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "namebase.txt");
    fp = my_fopen(buf, "r");

    /* Failed */
    if (!fp) return -1;

    /* Initialize */
    n = malloc(sizeof(*n));
    memset(n, 0, sizeof(*n));
    n->section = 0; /* Paranoia */

    while (0 == my_fgets(fp, buf, 1024))
    {
        /* Advance the line number */
        err_line++;

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (PARSE_ERROR_GENERIC);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V')
        {
            /* ignore */
            continue;
        }

        /* Parse the line */
        if ((err = _parse_namebase(buf, n)) != 0)
        break;
    }

    /* Errors */
    if (err)
    {
        cptr oops;

        /* Error string */
        oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

        /* Oops */
        msg_boundary();
        msg_format("<color:v>Error</color> %d (%s) at line %d of 'namebase.txt'.", err, oops, err_line);
        msg_format("Parsing '%s'.", buf);

        msg_print(NULL);
    }

    /* Close the file */
    my_fclose(fp);

    /* Finish parse */
    _finish_name_parse(n);

    /* Result */
    return (err);
}
