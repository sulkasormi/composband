#ifndef RANDNAME_H
#define RANDNAME_H

/*
 * The different types of name randname.c can generate
 * which is also the number of sections in namebase.txt
 */
typedef enum
{
	RANDNAME_TOLKIEN = 1,
	RANDNAME_SCROLL,
 
	/* End of type marker - not a valid name type */
	RANDNAME_NUM_TYPES 
} randname_type;

/*
 * Make a random name.
 */
extern size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***wordlist);
extern errr name_parser(void);

#endif /* RANDNAME_H */
