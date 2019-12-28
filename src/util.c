/* File: util.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Angband utilities -BEN- */

#include "angband.h"
#include "z-doc.h"

#include <assert.h>

/* Stop using auto_more and use the new improved handling instead! */
#define AUTO_MORE_PROMPT     0
#define AUTO_MORE_SKIP_ONE   1   /* Skip to next message */
#define AUTO_MORE_SKIP_BLOCK 2   /* Skip to next message boundary */
#define AUTO_MORE_SKIP_ALL   3   /* Skip to next player action */

int auto_more_state = AUTO_MORE_PROMPT;

/*
 * Hack -- prevent "accidents" in "screen_save()" or "screen_load()"
 */
static int screen_depth = 0;


/* Save macro trigger string for use in inkey_special() */
static char inkey_macro_trigger_string[1024];

#if 0
#ifndef HAS_STRICMP

/*
 * For those systems that don't have "stricmp()"
 *
 * Compare the two strings "a" and "b" ala "strcmp()" ignoring case.
 */
int stricmp(cptr a, cptr b)
{
    cptr s1, s2;
    char z1, z2;

    /* Scan the strings */
    for (s1 = a, s2 = b; TRUE; s1++, s2++)
    {
        z1 = FORCEUPPER(*s1);
        z2 = FORCEUPPER(*s2);
        if (z1 < z2) return (-1);
        if (z1 > z2) return (1);
        if (!z1) return (0);
    }
}

#endif /* HAS_STRICMP */
#endif /* 0 */

#ifdef SET_UID

# ifndef HAVE_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
int usleep(huge usecs)
{
    struct timeval      Timer;

    int                 nfds = 0;

#ifdef FD_SET
    fd_set        *no_fds = NULL;
#else
    int            *no_fds = NULL;
#endif


    /* Was: int readfds, writefds, exceptfds; */
    /* Was: readfds = writefds = exceptfds = 0; */


    /* Paranoia -- No excessive sleeping */
    if (usecs > 4000000L) core("Illegal usleep() call");



    /* Wait for it */
    Timer.tv_sec = (usecs / 1000000L);
    Timer.tv_usec = (usecs % 1000000L);

    /* Wait for it */
    if (select(nfds, no_fds, no_fds, no_fds, &Timer) < 0)
    {
        /* Hack -- ignore interrupts */
        if (errno != EINTR) return -1;
    }

    /* Success */
    return 0;
}

# endif


/*
 * Hack -- External functions
 */
#ifdef SET_UID
extern struct passwd *getpwuid(uid_t uid);
extern struct passwd *getpwnam(const char *name);
#endif

/*
 * Find a default user name from the system.
 */
void user_name(char *buf, int id)
{
    struct passwd *pw;

    /* Look up the user name */
    if ((pw = getpwuid(id)))
    {
        (void)strcpy(buf, pw->pw_name);
        buf[16] = '\0';

#ifdef CAPITALIZE_USER_NAME
        /* Hack -- capitalize the user name */
            if (islower(buf[0]))
                buf[0] = toupper(buf[0]);
#endif /* CAPITALIZE_USER_NAME */

        return;
    }

    /* Oops. Hack -- default to "PLAYER" */
    strcpy(buf, "PLAYER");
}

#endif /* SET_UID */




/*
 * The concept of the "file" routines below (and elsewhere) is that all
 * file handling should be done using as few routines as possible, since
 * every machine is slightly different, but these routines always have the
 * same semantics.
 *
 * In fact, perhaps we should use the "path_parse()" routine below to convert
 * from "canonical" filenames (optional leading tilde's, internal wildcards,
 * slash as the path seperator, etc) to "system" filenames (no special symbols,
 * system-specific path seperator, etc). This would allow the program itself
 * to assume that all filenames are "Unix" filenames, and explicitly "extract"
 * such filenames if needed (by "path_parse()", or perhaps "path_canon()").
 *
 * Note that "path_temp" should probably return a "canonical" filename.
 *
 * Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
 * and "my_move()" and "my_copy()" should all take "canonical" filenames.
 *
 * Note that "canonical" filenames use a leading "slash" to indicate an absolute
 * path, and a leading "tilde" to indicate a special directory, and default to a
 * relative path, but MSDOS uses a leading "drivename plus colon" to indicate the
 * use of a "special drive", and then the rest of the path is parsed "normally",
 * and MACINTOSH uses a leading colon to indicate a relative path, and an embedded
 * colon to indicate a "drive plus absolute path", and finally defaults to a file
 * in the current working directory, which may or may not be defined.
 *
 * We should probably parse a leading "~~/" as referring to "ANGBAND_DIR". (?)
 */


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 */


#else /* ACORN */


#ifdef SET_UID

/*
 * Extract a "parsed" path from an initial filename
 * Normally, we simply copy the filename into the buffer
 * But leading tilde symbols must be handled in a special way
 * Replace "~user/" by the home directory of the user named "user"
 * Replace "~/" by the home directory of the current user
 */
errr path_parse(char *buf, int max, cptr file)
{
    cptr        u, s;
    struct passwd    *pw;
    char        user[128];


    /* Assume no result */
    buf[0] = '\0';

    /* No file? */
    if (!file) return (-1);

    /* File needs no parsing */
    if (file[0] != '~')
    {
        (void)strnfmt(buf, max, "%s", file);
        return (0);
    }

    /* Point at the user */
    u = file+1;

    /* Look for non-user portion of the file */
    s = my_strstr(u, PATH_SEP);

    /* Hack -- no long user names */
    if (s && (s >= u + sizeof(user))) return (1);

    /* Extract a user name */
    if (s)
    {
        int i;
        for (i = 0; u < s; ++i) user[i] = *u++;
        user[i] = '\0';
        u = user;
    }

    /* Look up the "current" user */
    if (u[0] == '\0') u = getlogin();

    /* Look up a user (or "current" user) */
    if (u) pw = getpwnam(u);
    else pw = getpwuid(getuid());

    /* Nothing found? */
    if (!pw) return (1);

    /* Make use of the info */
    if (s) strnfmt(buf, max, "%s%s", pw->pw_dir, s);
    else strnfmt(buf, max, "%s", pw->pw_dir);

    /* Success */
    return (0);
}


#else /* SET_UID */


/*
 * Extract a "parsed" path from an initial filename
 *
 * This requires no special processing on simple machines,
 * except for verifying the size of the filename.
 */
errr path_parse(char *buf, int max, cptr file)
{
    /* Accept the filename */
    (void)strnfmt(buf, max, "%s", file);

#if defined(MAC_MPW) && defined(CARBON)
     /* Fix it according to the current operating system */
    convert_pathname(buf);
#endif /* MAC_MPW && CARBON */

    /* Success */
    return (0);
}


#endif /* SET_UID */

#if !defined(HAVE_MKSTEMP) || defined(WINDOWS) || defined(WIN32)

/*
 * Hack -- acquire a "temporary" file name if possible
 *
 * This filename is always in "system-specific" form.
 */
static errr path_temp(char *buf, int max)
{
    cptr s;

    /* Temp file */
    s = tmpnam(NULL);

    /* Oops */
    if (!s) return (-1);

    /* Format to length */
#ifndef WIN32
    (void)strnfmt(buf, max, "%s", s);
#else
    (void)strnfmt(buf, max, ".%s", s);
#endif

    /* Success */
    return (0);
}

#endif

/*
 * Add a formatted string to the end of a string
 */
void strnfcat(char *str, size_t max, size_t *end, const char *fmt, ...)
{
	size_t len;

	va_list vp;

	/* Paranoia */
	if (*end >= max) return;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Build the string */
	len = vstrnfmt(&str[*end], max - *end, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Change the end value */
	*end += len;
}

static void path_process(char *buf, size_t len, size_t *cur_len, const char *path)
{
#if defined(SET_UID)

	/* Home directory on Unixes */
	if (path[0] == '~')
	{
		const char *s;
		const char *username = path + 1;

		struct passwd *pw;
		char user[128];

		/* Look for non-user portion of the file */
		s = strstr(username, PATH_SEP);
		if (s)
		{
			int i;

			/* Keep username a decent length */
			if (s >= username + sizeof(user)) return;

			for (i = 0; username < s; ++i) user[i] = *username++;
			user[i] = '\0';
			username = user;
		}

		/* Look up a user (or "current" user) */
		pw = username[0] ? getpwnam(username) : getpwuid(getuid());
		if (!pw) return;

		/* Copy across */
		strnfcat(buf, len, cur_len, "%s%s", pw->pw_dir, PATH_SEP);
		if (s) strnfcat(buf, len, cur_len, "%s", s);
	}
	else

#endif /* defined(SET_UID) */

	strnfcat(buf, len, cur_len, "%s", path);
}

/*
 * Create a new path string by appending a 'leaf' to 'base'.
 *
 * On Unixes, we convert a tidle at the beginning of a basename to mean the
 * directory, complicating things a little, but better now than later.
 *
 * Remember to free the return value.
 */
size_t path_build(char *buf, size_t len, const char *base, const char *leaf)
{
	size_t cur_len = 0;
	buf[0] = '\0';

	if (!leaf || !leaf[0])
	{
		if (base && base[0])
			path_process(buf, len, &cur_len, base);

		return cur_len;
	}


	/*
	 * If the leafname starts with the seperator,
	 *   or with the tilde (on Unix),
	 *   or there's no base path,
	 * We use the leafname only.
	 */
#if defined(UNIX)
	if ((!base || !base[0]) || prefix(leaf, PATH_SEP) || leaf[0] == '~')
#else
	if ((!base || !base[0]) || prefix(leaf, PATH_SEP))
#endif
	{
		path_process(buf, len, &cur_len, leaf);
		return cur_len;
	}


	/* There is both a relative leafname and a base path from which it is relative */
	path_process(buf, len, &cur_len, base);
	strnfcat(buf, len, &cur_len, "%s", PATH_SEP);
	path_process(buf, len, &cur_len, leaf);

	return cur_len;
}
#if 0
/*
 * Create a new path by appending a file (or directory) to a path.
 *
 * This requires no special processing on simple machines, except
 * for verifying the size of the filename, but note the ability to
 * bypass the given "path" with certain special file-names.
 *
 * Note that the "file" may actually be a "sub-path", including
 * a path and a file.
 *
 * Note that this function yields a path which must be "parsed"
 * using the "parse" function above.
 */
errr path_build(char *buf, int max, cptr path, cptr file)
{
    /* Special file */
    if (file[0] == '~')
    {
        /* Use the file itself */
        (void)strnfmt(buf, max, "%s", file);
    }

    /* Absolute file, on "normal" systems */
    else if (prefix(file, PATH_SEP) && !streq(PATH_SEP, ""))
    {
        /* Use the file itself */
        (void)strnfmt(buf, max, "%s", file);
    }

    /* No path given */
    else if (!path[0])
    {
        /* Use the file itself */
        (void)strnfmt(buf, max, "%s", file);
    }

    /* Path and File */
    else
    {
        /* Build the new path */
        (void)strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
    }

    /* Success */
    return (0);
}
#endif

/*
 * Hack -- replacement for "fopen()"
 */
FILE *my_fopen(cptr file, cptr mode)
{
    char buf[1024];

#if defined(MAC_MPW) || defined(MACH_O_CARBON)
    FILE *tempfff;
#endif

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (NULL);

    //#if defined(MAC_MPW) || defined(MACH_O_CARBON)
#if 0
    if (my_strchr(mode, 'w'))
    {
        /* setting file type/creator */
        tempfff = fopen(buf, mode);
        fsetfileinfo(buf, _fcreator, _ftype);
        fclose(tempfff);
    }
#endif

    /* Attempt to fopen the file anyway */
    return (fopen(buf, mode));
}


/*
 * Hack -- replacement for "fclose()"
 */
errr my_fclose(FILE *fff)
{
    /* Require a file */
    if (!fff) return (-1);

    /* Close, check for error */
    if (fclose(fff) == EOF) return (1);

    /* Success */
    return (0);
}


#endif /* ACORN */


#if defined(HAVE_MKSTEMP) && !defined(WINDOWS) && !defined(WIN32)

FILE *my_fopen_temp(char *buf, int max)
{
    int fd;

    /* Prepare the buffer for mkstemp */
    strncpy(buf, "/tmp/anXXXXXX", max);

    /* Secure creation of a temporary file */
    fd = mkstemp(buf);

    /* Check the file-descriptor */
    if (fd < 0) return (NULL);

    /* Return a file stream */
    return (fdopen(fd, "w"));
}

#else /* HAVE_MKSTEMP */

FILE *my_fopen_temp(char *buf, int max)
{
    /* Generate a temporary filename */
    if (path_temp(buf, max)) return (NULL);

    /* Open the file */
    return (my_fopen(buf, "w"));
}

#endif /* HAVE_MKSTEMP */


/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, strip internal non-printables
 */
errr my_fgets(FILE *fff, char *buf, huge n)
{
    huge i = 0;

    char *s;

    char tmp[1024];

    /* Read a line */
    if (fgets(tmp, 1024, fff))
    {
        /* Convert weirdness */
        for (s = tmp; *s; s++)
        {
#if defined(MACINTOSH) || defined(MACH_O_CARBON)

            /*
             * Be nice to the Macintosh, where a file can have Mac or Unix
             * end of line, especially since the introduction of OS X.
             * MPW tools were also very tolerant to the Unix EOL.
             */
            if (*s == '\r') *s = '\n';

#endif /* MACINTOSH || MACH_O_CARBON */

            /* Handle newline */
            if (*s == '\n')
            {
                /* Terminate */
                buf[i] = '\0';

                /* Success */
                return (0);
            }

            /* Handle tabs */
            else if (*s == '\t')
            {
                /* Hack -- require room */
                if (i + 8 >= n) break;

                /* Append a space */
                buf[i++] = ' ';

                /* Append some more spaces */
                while (0 != (i % 8)) buf[i++] = ' ';
            }

            /* Handle printables HACK: msvcr100d will assert the character belongs to the ASCII code set*/
            else if ((unsigned)(*s + 1) <= 256 && isprint(*s))
            {
                /* Copy */
                buf[i++] = *s;

                /* Check length */
                if (i >= n) break;
            }
        }
        /* No newline character, but terminate */
        buf[i] = '\0';

        /* Success */
        return (0);
    }

    /* Nothing */
    buf[0] = '\0';

    /* Failure */
    return (1);
}


/*
 * Hack -- replacement for "fputs()"
 *
 * Dump a string, plus a newline, to a file
 *
 * XXX XXX XXX Process internal weirdness?
 */
errr my_fputs(FILE *fff, cptr buf, huge n)
{
    /* XXX XXX */
    n = n ? n : 0;

    /* Dump, ignore errors */
    (void)fprintf(fff, "%s\n", buf);

    /* Success */
    return (0);
}


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 *
 * Many of them can be rewritten now that only "fd_open()" and "fd_make()"
 * and "my_fopen()" should ever create files.
 */


#else /* ACORN */


/*
 * Code Warrior is a little weird about some functions
 */
#ifdef BEN_HACK
extern int open(const char *, int, ...);
extern int close(int);
extern int read(int, void *, unsigned int);
extern int write(int, const void *, unsigned int);
extern long lseek(int, long, int);
#endif /* BEN_HACK */


/*
 * The Macintosh is a little bit brain-dead sometimes
 */
#ifdef MACINTOSH
# define open(N,F,M) \
((M), open((char*)(N),F))
# define write(F,B,S) \
write(F,(char*)(B),S)
#endif /* MACINTOSH */


/*
 * Several systems have no "O_BINARY" flag
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */


/*
 * Hack -- attempt to delete a file
 */
errr fd_kill(cptr file)
{
    char buf[1024];

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (-1);

    /* Remove */
    (void)remove(buf);

    /* XXX XXX XXX */
    return (0);
}


/*
 * Hack -- attempt to move a file
 */
errr fd_move(cptr file, cptr what)
{
    char buf[1024];
    char aux[1024];

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (-1);

    /* Hack -- Try to parse the path */
    if (path_parse(aux, 1024, what)) return (-1);

    /* Rename */
    (void)rename(buf, aux);

    /* XXX XXX XXX */
    return (0);
}


/*
 * Hack -- attempt to copy a file
 */
errr fd_copy(cptr file, cptr what)
{
    errr rc = ERROR_SUCCESS;
    char buf[1024];
    char aux[1024];
    int read_num;
    int src_fd, dst_fd;

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (-1);

    /* Hack -- Try to parse the path */
    if (path_parse(aux, 1024, what)) return (-1);

    /* Open source file */
    src_fd = fd_open(buf, O_RDONLY);
    if (src_fd < 0) return (-1);

    /* Open destination file */
    dst_fd = fd_open(aux, O_WRONLY|O_TRUNC|O_CREAT);
    if (dst_fd < 0) return (-1);

    /* Copy */
    while ((read_num = read(src_fd, buf, 1024)) > 0)
    {
        int write_num = write(dst_fd, buf, read_num);
        /* "For writing, the return value is the number of bytes written; an error
            has occurred if this isn't equal to the number requested." */
        if (write_num != read_num)
        {
            rc = ERROR_UNKOWN_FAILURE;
            break;
        }
    }

    /* Close files */
    fd_close(src_fd);
    fd_close(dst_fd);

    /* XXX XXX XXX */
    return rc;
}


/*
 * Hack -- attempt to open a file descriptor (create file)
 *
 * This function should fail if the file already exists
 *
 * Note that we assume that the file should be "binary"
 *
 * XXX XXX XXX The horrible "BEN_HACK" code is for compiling under
 * the CodeWarrior compiler, in which case, for some reason, none
 * of the "O_*" flags are defined, and we must fake the definition
 * of "O_RDONLY", "O_WRONLY", and "O_RDWR" in "A-win-h", and then
 * we must simulate the effect of the proper "open()" call below.
 */
int fd_make(cptr file, int mode)
{
    char buf[1024];

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (-1);

#ifdef BEN_HACK

    /* Check for existance */
    /* if (fd_close(fd_open(file, O_RDONLY | O_BINARY))) return (1); */

    /* Mega-Hack -- Create the file */
    (void)my_fclose(my_fopen(file, "wb"));

    /* Re-open the file for writing */
    return (open(buf, O_WRONLY | O_BINARY, mode));

#else /* BEN_HACK */

#if 0
    //#if defined(MAC_MPW) || defined(MACH_O_CARBON)
    {
        int fdes;
        /* Create the file, fail if exists, write-only, binary */
        fdes = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode);
        /* Set creator and type if the file is successfully opened */
        if (fdes >= 0) fsetfileinfo(buf, _fcreator, _ftype);
        /* Return the descriptor */
        return (fdes);
    }
# else
    /* Create the file, fail if exists, write-only, binary */
    return (open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode));
# endif

#endif /* BEN_HACK */

}


/*
 * Hack -- attempt to open a file descriptor (existing file)
 *
 * Note that we assume that the file should be "binary"
 */
int fd_open(cptr file, int flags)
{
    char buf[1024];

    /* Hack -- Try to parse the path */
    if (path_parse(buf, 1024, file)) return (-1);

    /* Attempt to open the file */
    return open(buf, flags | O_BINARY, 0);
}


/*
 * Hack -- attempt to lock a file descriptor
 *
 * Legal lock types -- F_UNLCK, F_RDLCK, F_WRLCK
 */
errr fd_lock(int fd, int what)
{
    /* XXX XXX */
    what = what ? what : 0;

    /* Verify the fd */
    if (fd < 0) return (-1);

#ifdef SET_UID

# ifdef USG

#  if defined(F_ULOCK) && defined(F_LOCK)

    /* Un-Lock */
    if (what == F_UNLCK)
    {
        /* Unlock it, Ignore errors */
        lockf(fd, F_ULOCK, 0);
    }

    /* Lock */
    else
    {
        /* Lock the score file */
        if (lockf(fd, F_LOCK, 0) != 0) return (1);
    }

#  endif

# else

#  if defined(LOCK_UN) && defined(LOCK_EX)

    /* Un-Lock */
    if (what == F_UNLCK)
    {
        /* Unlock it, Ignore errors */
        (void)flock(fd, LOCK_UN);
    }

    /* Lock */
    else
    {
        /* Lock the score file */
        if (flock(fd, LOCK_EX) != 0) return (1);
    }

#  endif

# endif

#endif

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to seek on a file descriptor
 */
errr fd_seek(int fd, huge n)
{
    huge p;

    /* Verify fd */
    if (fd < 0) return (-1);

    /* Seek to the given position */
    p = lseek(fd, n, SEEK_SET);

    /* Failure */
    if (p != n) return (1);

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to truncate a file descriptor
 */
errr fd_chop(int fd, huge n)
{
    /* XXX XXX */
    n = n ? n : 0;

    /* Verify the fd */
    if (fd < 0) return (-1);

#if defined(SUNOS) || defined(ULTRIX) || defined(NeXT)
    /* Truncate */
    ftruncate(fd, n);
#endif

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to read data from a file descriptor
 */
errr fd_read(int fd, char *buf, huge n)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#ifndef SET_UID

    /* Read pieces */
    while (n >= 16384)
    {
        /* Read a piece */
        if (_read(fd, buf, 16384) != 16384) return (1);

        /* Shorten the task */
        buf += 16384;

        /* Shorten the task */
        n -= 16384;
    }

#endif

    /* Read the final piece */
    if (read(fd, buf, n) != (int)n) return (1);

    /* Success */
    return (0);
}


/*
 * Hack -- Attempt to write data to a file descriptor
 */
errr fd_write(int fd, cptr buf, huge n)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#ifndef SET_UID

    /* Write pieces */
    while (n >= 16384)
    {
        /* Write a piece */
        if (_write(fd, buf, 16384) != 16384) return (1);

        /* Shorten the task */
        buf += 16384;

        /* Shorten the task */
        n -= 16384;
    }

#endif

    /* Write the final piece */
    if (write(fd, buf, n) != (int)n) return (1);

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to close a file descriptor
 */
errr fd_close(int fd)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

    /* Close */
    close(fd);

    /* XXX XXX XXX */
    return (0);
}


#endif /* ACORN */




/*
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "TERM_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, TERM_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * The following info is from "Torbjorn Lindgren" (see "main-xaw.c").
 *
 * These values are NOT gamma-corrected. On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7. The converted value for each of the five
 * different "quarter" values is given below:
 *
 *  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
 *  -----       ----            ----            ----          ---
 *   0/4        0.00            0.00            0.00          #00
 *   1/4        0.25            0.27            0.28          #47
 *   2/4        0.50            0.55            0.56          #8f
 *   3/4        0.75            0.82            0.84          #d7
 *   4/4        1.00            1.00            1.00          #ff
 *
 * Note that some machines (i.e. most IBM machines) are limited to a
 * hard-coded set of colors, and so the information above is useless.
 *
 * Also, some machines are limited to a pre-determined set of colors,
 * for example, the IBM can only display 16 colors, and only 14 of
 * those colors resemble colors used by Angband, and then only when
 * you ignore the fact that "Slate" and "cyan" are not really matches,
 * so on the IBM, we use "orange" for both "Umber", and "Light Umber"
 * in addition to the obvious "Orange", since by combining all of the
 * "indeterminate" colors into a single color, the rest of the colors
 * are left with "meaningful" values.
 */


/*
 * Move the cursor
 */
void move_cursor(int row, int col)
{
    Term_gotoxy(col, row);
}



/*
 * Convert a decimal to a single digit octal number
 */
static char octify(uint i)
{
    return (hexsym[i%8]);
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(uint i)
{
    return (hexsym[i%16]);
}


/*
 * Convert a octal-digit into a decimal
 */
static int deoct(char c)
{
    if (isdigit(c)) return (D2I(c));
    return (0);
}

/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
    if (isdigit(c)) return (D2I(c));
    if (islower(c)) return (A2I(c) + 10);
    if (isupper(c)) return (A2I(tolower(c)) + 10);
    return (0);
}


int my_stricmp(cptr a, cptr b)
{
    cptr s1, s2;
    char z1, z2;

    /* Scan the strings */
    for (s1 = a, s2 = b; TRUE; s1++, s2++)
    {
        z1 = FORCEUPPER(*s1);
        z2 = FORCEUPPER(*s2);
        if (z1 < z2) return (-1);
        if (z1 > z2) return (1);
        if (!z1) return (0);
    }
}

static int my_strnicmp(cptr a, cptr b, int n)
{
    cptr s1, s2;
    char z1, z2;

    /* Scan the strings */
    for (s1 = a, s2 = b; n > 0; s1++, s2++, n--)
    {
        z1 = FORCEUPPER(*s1);
        z2 = FORCEUPPER(*s2);
        if (z1 < z2) return (-1);
        if (z1 > z2) return (1);
        if (!z1) return (0);
    }
    return 0;
}


static void trigger_text_to_ascii(char **bufptr, cptr *strptr)
{
    char *s = *bufptr;
    cptr str = *strptr;
    bool mod_status[MAX_MACRO_MOD];

    int i, len = 0;
    int shiftstatus = 0;
    cptr key_code;

    if (macro_template == NULL)
        return;
    
    for (i = 0; macro_modifier_chr[i]; i++)
        mod_status[i] = FALSE;
    str++;

    /* Examine modifier keys */
    while (1)
    {
        for (i=0; macro_modifier_chr[i]; i++)
        {
            len = strlen(macro_modifier_name[i]);
            
            if(!my_strnicmp(str, macro_modifier_name[i], len))
                break;
        }
        if (!macro_modifier_chr[i]) break;
        str += len;
        mod_status[i] = TRUE;
        if ('S' == macro_modifier_chr[i])
            shiftstatus = 1;
    }
    for (i = 0; i < max_macrotrigger; i++)
    {
        len = strlen(macro_trigger_name[i]);
        if (!my_strnicmp(str, macro_trigger_name[i], len) && ']' == str[len])
        {
            /* a trigger name found */
            break;
        }
    }

    /* Invalid trigger name? */
    if (i == max_macrotrigger)
    {
        str = my_strchr(str, ']');
        if (str)
        {
            *s++ = (char)31;
            *s++ = '\r';
            *bufptr = s;
            *strptr = str; /* where **strptr == ']' */
        }
        return;
    }
    key_code = macro_trigger_keycode[shiftstatus][i];
    str += len;

    *s++ = (char)31;
    for (i = 0; macro_template[i]; i++)
    {
        char ch = macro_template[i];
        int j;

        switch(ch)
        {
        case '&':
            for (j = 0; macro_modifier_chr[j]; j++) {
                if (mod_status[j])
                    *s++ = macro_modifier_chr[j];
            }
            break;
        case '#':
            strcpy(s, key_code);
            s += strlen(key_code);
            break;
        default:
            *s++ = ch;
            break;
        }
    }
    *s++ = '\r';

    *bufptr = s;
    *strptr = str; /* where **strptr == ']' */
    return;
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * I have no clue if this function correctly handles, for example,
 * parsing "\xFF" into a (signed) char. Whoever thought of making
 * the "sign" of a "char" undefined is a complete moron. Oh well.
 */
void text_to_ascii(char *buf, cptr str)
{
    char *s = buf;

    /* Analyze the "ascii" string */
    while (*str)
    {
        /* Backslash codes */
        if (*str == '\\')
        {
            /* Skip the backslash */
            str++;

            /* Paranoia */
            if (!(*str)) break;

            /* Macro Trigger */
            if (*str == '[')
            {
                trigger_text_to_ascii(&s, &str);
            }
            else

            /* Hex-mode XXX */
            if (*str == 'x')
            {
                *s = 16 * dehex(*++str);
                *s++ += dehex(*++str);
            }

            /* Hack -- simple way to specify "backslash" */
            else if (*str == '\\')
            {
                *s++ = '\\';
            }

            /* Hack -- simple way to specify "caret" */
            else if (*str == '^')
            {
                *s++ = '^';
            }

            /* Hack -- simple way to specify "space" */
            else if (*str == 's')
            {
                *s++ = ' ';
            }

            /* Hack -- simple way to specify Escape */
            else if (*str == 'e')
            {
                *s++ = ESCAPE;
            }

            /* Backspace */
            else if (*str == 'b')
            {
                *s++ = '\b';
            }

            /* Newline */
            else if (*str == 'n')
            {
                *s++ = '\n';
            }

            /* Return */
            else if (*str == 'r')
            {
                *s++ = '\r';
            }

            /* Tab */
            else if (*str == 't')
            {
                *s++ = '\t';
            }

            /* Octal-mode */
            else if (*str == '0')
            {
                *s = 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '1')
            {
                *s = 64 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '2')
            {
                *s = 64 * 2 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '3')
            {
                *s = 64 * 3 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Skip the final char */
            str++;
        }

        /* Normal Control codes */
        else if (*str == '^')
        {
            str++;
            *s++ = (*str++ & 037);
        }

        /* Normal chars */
        else
        {
            *s++ = *str++;
        }
    }

    /* Terminate */
    *s = '\0';
}


static bool trigger_ascii_to_text(char **bufptr, cptr *strptr)
{
    char *s = *bufptr;
    cptr str = *strptr;
    char key_code[100];
    int i;
    cptr tmp;

    if (macro_template == NULL)
        return FALSE;

    *s++ = '\\';
    *s++ = '[';

    for (i = 0; macro_template[i]; i++)
    {
        int j;
        char ch = macro_template[i];

        switch(ch)
        {
        case '&':
            while ((tmp = my_strchr(macro_modifier_chr, *str)))
            {
                j = (int)(tmp - macro_modifier_chr);
                tmp = macro_modifier_name[j];
                while(*tmp) *s++ = *tmp++;
                str++;
            }
            break;
        case '#':
            for (j = 0; *str && *str != '\r'; j++)
                key_code[j] = *str++;
            key_code[j] = '\0';
            break;
        default:
            if (ch != *str) return FALSE;
            str++;
        }
    }
    if (*str++ != '\r') return FALSE;

    for (i = 0; i < max_macrotrigger; i++)
    {
        if (!my_stricmp(key_code, macro_trigger_keycode[0][i])
            || !my_stricmp(key_code, macro_trigger_keycode[1][i]))
            break;
    }
    if (i == max_macrotrigger)
        return FALSE;

    tmp = macro_trigger_name[i];
    while (*tmp) *s++ = *tmp++;

    *s++ = ']';
    
    *bufptr = s;
    *strptr = str;
    return TRUE;
}


/*
 * Hack -- convert a string into a printable form
 */
void ascii_to_text(char *buf, cptr str)
{
    char *s = buf;

    /* Analyze the "ascii" string */
    while (*str)
    {
        byte i = (byte)(*str++);

        /* Macro Trigger */
        if (i == 31)
        {
            if(!trigger_ascii_to_text(&s, &str))
            {
                *s++ = '^';
                *s++ = '_';
            }
        }
        else

        if (i == ESCAPE)
        {
            *s++ = '\\';
            *s++ = 'e';
        }
        else if (i == ' ')
        {
            *s++ = '\\';
            *s++ = 's';
        }
        else if (i == '\b')
        {
            *s++ = '\\';
            *s++ = 'b';
        }
        else if (i == '\t')
        {
            *s++ = '\\';
            *s++ = 't';
        }
        else if (i == '\n')
        {
            *s++ = '\\';
            *s++ = 'n';
        }
        else if (i == '\r')
        {
            *s++ = '\\';
            *s++ = 'r';
        }
        else if (i == '^')
        {
            *s++ = '\\';
            *s++ = '^';
        }
        else if (i == '\\')
        {
            *s++ = '\\';
            *s++ = '\\';
        }
        else if (i < 32)
        {
            *s++ = '^';
            *s++ = i + 64;
        }
        else if (i < 127)
        {
            *s++ = i;
        }
        else if (i < 64)
        {
            *s++ = '\\';
            *s++ = '0';
            *s++ = octify(i / 8);
            *s++ = octify(i % 8);
        }
        else
        {
            *s++ = '\\';
            *s++ = 'x';
            *s++ = hexify(i / 16);
            *s++ = hexify(i % 16);
        }
    }

    /* Terminate */
    *s = '\0';
}



/*
 * The "macro" package
 *
 * Functions are provided to manipulate a collection of macros, each
 * of which has a trigger pattern string and a resulting action string
 * and a small set of flags.
 */



/*
 * Determine if any macros have ever started with a given character.
 */
static bool macro__use[256];


/*
 * Find the macro (if any) which exactly matches the given pattern
 */
sint macro_find_exact(cptr pat)
{
    int i;

    /* Nothing possible */
    if (!macro__use[(byte)(pat[0])])
    {
        return (-1);
    }

    /* Scan the macros */
    for (i = 0; i < macro__num; ++i)
    {
        /* Skip macros which do not match the pattern */
        if (!streq(macro__pat[i], pat)) continue;

        /* Found one */
        return (i);
    }

    /* No matches */
    return (-1);
}


/*
 * Find the first macro (if any) which contains the given pattern
 */
static sint macro_find_check(cptr pat)
{
    int i;

    /* Nothing possible */
    if (!macro__use[(byte)(pat[0])])
    {
        return (-1);
    }

    /* Scan the macros */
    for (i = 0; i < macro__num; ++i)
    {
        /* Skip macros which do not contain the pattern */
        if (!prefix(macro__pat[i], pat)) continue;

        /* Found one */
        return (i);
    }

    /* Nothing */
    return (-1);
}


/*
 * Find the first macro (if any) which contains the given pattern and more
 */
static sint macro_find_maybe(cptr pat)
{
    int i;

    /* Nothing possible */
    if (!macro__use[(byte)(pat[0])])
    {
        return (-1);
    }

    /* Scan the macros */
    for (i = 0; i < macro__num; ++i)
    {
        /* Skip macros which do not contain the pattern */
        if (!prefix(macro__pat[i], pat)) continue;

        /* Skip macros which exactly match the pattern XXX XXX */
        if (streq(macro__pat[i], pat)) continue;

        /* Found one */
        return (i);
    }

    /* Nothing */
    return (-1);
}


/*
 * Find the longest macro (if any) which starts with the given pattern
 */
static sint macro_find_ready(cptr pat)
{
    int i, t, n = -1, s = -1;

    /* Nothing possible */
    if (!macro__use[(byte)(pat[0])])
    {
        return (-1);
    }

    /* Scan the macros */
    for (i = 0; i < macro__num; ++i)
    {
        /* Skip macros which are not contained by the pattern */
        if (!prefix(pat, macro__pat[i])) continue;

        /* Obtain the length of this macro */
        t = strlen(macro__pat[i]);

        /* Only track the "longest" pattern */
        if ((n >= 0) && (s > t)) continue;

        /* Track the entry */
        n = i;
        s = t;
    }

    /* Result */
    return (n);
}


/*
 * Add a macro definition (or redefinition).
 *
 * We should use "act == NULL" to "remove" a macro, but this might make it
 * impossible to save the "removal" of a macro definition. XXX XXX XXX
 *
 * We should consider refusing to allow macros which contain existing macros,
 * or which are contained in existing macros, because this would simplify the
 * macro analysis code. XXX XXX XXX
 *
 * We should consider removing the "command macro" crap, and replacing it
 * with some kind of "powerful keymap" ability, but this might make it hard
 * to change the "roguelike" option from inside the game. XXX XXX XXX
 */
errr macro_add(cptr pat, cptr act)
{
    int n;


    /* Paranoia -- require data */
    if (!pat || !act) return (-1);


    /* Look for any existing macro */
    n = macro_find_exact(pat);

    /* Replace existing macro */
    if (n >= 0)
    {
        /* Free the old macro action */
        z_string_free(macro__act[n]);
    }
    else if (macro__num >= MACRO_MAX - 1)
    {
        return -1;
    }
    /* Create a new macro */
    else
    {
        /* Acquire a new index */
        n = macro__num++;

        /* Save the pattern */
        macro__pat[n] = z_string_make(pat);
    }

    /* Save the action */
    macro__act[n] = z_string_make(act);

    /* Efficiency */
    macro__use[(byte)(pat[0])] = TRUE;

    /* Success */
    return (0);
}



/*
 * Local variable -- we are inside a "macro action"
 *
 * Do not match any macros until "ascii 30" is found.
 */
static bool parse_macro = FALSE;

/*
 * Local variable -- we are inside a "macro trigger"
 *
 * Strip all keypresses until a low ascii value is found.
 */
static bool parse_under = FALSE;


/*
 * Flush all input chars. Actually, remember the flush,
 * and do a "special flush" before the next "inkey()".
 *
 * This is not only more efficient, but also necessary to make sure
 * that various "inkey()" codes are not "lost" along the way.
 */
void flush(void)
{
    /* Do it later */
    inkey_xtra = TRUE;
}


/*
 * Flush the screen, make a noise
 */
void bell(void)
{
    /* Mega-Hack -- Flush the output */
    Term_fresh();

    /* Make a bell noise (if allowed) */
    if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

    /* Flush the input (later!) */
    flush();
}


/*
 * Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
    /* No sound */
    if (!use_sound) return;

    /* Make a sound (if allowed) */
    Term_xtra(TERM_XTRA_SOUND, val);
}

/*
 * Cancel macro action on the queue
 */
static void forget_macro_action(void)
{
    if (!parse_macro) return;

    /* Drop following macro action string */
    while (TRUE)
    {
        char ch;

        /* End loop if no key ready */
        if (Term_inkey(&ch, FALSE, TRUE)) break;

        /* End loop if no key ready */
        if (ch == 0) break;

        /* End of "macro action" */
        if (ch == 30) break;
    }

    /* No longer inside "macro action" */
    parse_macro = FALSE;
}

/*
 * Helper function called only from "inkey()"
 *
 * This function does almost all of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros, as well
 * as "extra" keys read in while choosing the proper macro, and also to hold
 * the action for the macro, plus a special "ascii 30" character indicating
 * that any macro action in progress is complete. Embedded macros are thus
 * illegal, unless a macro action includes an explicit "ascii 30" character,
 * which would probably be a massive hack, and might break things.
 *
 * Only 500 (0+1+2+...+29+30) milliseconds may elapse between each key in
 * the macro trigger sequence. If a key sequence forms the "prefix" of a
 * macro trigger, 500 milliseconds must pass before the key sequence is
 * known not to be that macro trigger. XXX XXX XXX
 */
static char inkey_aux(void)
{
    int k = 0, n, p = 0, w = 0, max_delay = 100;

    char ch;

    cptr pat, act;

    char *buf = inkey_macro_trigger_string;

    auto_more_state = AUTO_MORE_PROMPT;

    if (parse_macro)
    {
        /* Scan next keypress from macro action */
        if (Term_inkey(&ch, FALSE, TRUE))
        {
            /* Over-flowed? Cancel macro action */
            parse_macro = FALSE;
        }
    }
    else
    {
        /* Wait for a keypress */
        (void) (Term_inkey(&ch, TRUE, TRUE));
    }


    /* End "macro action" */
    if (ch == 30) parse_macro = FALSE;

    /* Inside "macro action" */
    if (ch == 30) return (ch);

    /* Inside "macro action" */
    if (parse_macro) return (ch);

    /* Inside "macro trigger" */
    if (parse_under) return (ch);

    /* Save the first key, advance */
    buf[p++] = ch;
    buf[p] = '\0';


    /* Check for possible macro */
    k = macro_find_check(buf);

    /* No macro pending */
    if (k < 0) return (ch);

    if (strcmp(ANGBAND_SYS, "gcu") == 0)
    {
        /* curses sends many single keystrokes as escape sequences, and we
         * use macros to handle these (see pref-gcu.prf). For example, pressing '2'
         * on the number pad comes as the three character sequence \e[B. If we
         * aren't careful, then the game will pause when the user really just types
         * escape. For the longest time, I was convinced this had to do with ESCDELAY
         * in ncurses. Alas, the culprit is our macro processing system! */
        if (ch == 27)
            max_delay = 10;
        else /* if (ch == SHIFT?) */
        {
            /* More Curses Problems: I'm finding these pauses unacceptable. By
             * default, the curses port defines many macros that also begin with
             * shift, among other things. This means that every time I run I
             * get what seems like a 2s delay. I know the delay is only 550ms, but
             * it feels like it lasts forever and makes the game unplayable! */
            max_delay = 30; /* I can detect it at 40 which should be only a 100ms
                             * delay. This is physiologically impossible, so there
                             * must be a delay bug someplace in the curses port */
        }
    }
    
    /* Wait for a macro, or a timeout */
    while (TRUE)
    {
        /* Check for pending macro */
        k = macro_find_maybe(buf);

        /* No macro pending */
        if (k < 0) break;

        /* Check for (and remove) a pending key */
        if (0 == Term_inkey(&ch, FALSE, TRUE))
        {
            /* Append the key */
            buf[p++] = ch;
            buf[p] = '\0';

            /* Restart wait */
            w = 0;
        }

        /* No key ready */
        else
        {
            /* Increase "wait" */
            w += 10;

            /* Excessive delay */
            if (w >= max_delay) break;

            /* Delay */
            Term_xtra(TERM_XTRA_DELAY, w);
        }
    }


    /* Check for available macro */
    k = macro_find_ready(buf);

    /* No macro available */
    if (k < 0)
    {
        /* Push all the keys back on the queue */
        while (p > 0)
        {
            /* Push the key, notice over-flow */
            if (Term_key_push(buf[--p])) return (0);
        }

        /* Wait for (and remove) a pending key */
        (void)Term_inkey(&ch, TRUE, TRUE);

        /* Return the key */
        return (ch);
    }


    /* Get the pattern */
    pat = macro__pat[k];

    /* Get the length of the pattern */
    n = strlen(pat);

    /* Push the "extra" keys back on the queue */
    while (p > n)
    {
        /* Push the key, notice over-flow */
        if (Term_key_push(buf[--p])) return (0);
    }


    /* Begin "macro action" */
    parse_macro = TRUE;

    /* Push the "end of macro action" key */
    if (Term_key_push(30)) return (0);


    /* Access the macro action */
    act = macro__act[k];

    /* Get the length of the action */
    n = strlen(act);

    if ((autopick_inkey_hack == 1) && (n == 2) && ((act[0] == '.') || (act[0] == ';')))
    {
        int skey = 0;
        switch (act[1])
        {
            case '8': skey = (SKEY_UP & ~SKEY_MASK); break;
            case '2': skey = (SKEY_DOWN & ~SKEY_MASK); break;
            case '4': skey = (SKEY_LEFT & ~SKEY_MASK); break;
            case '6': skey = (SKEY_RIGHT & ~SKEY_MASK); break;
            default: break;
        }
        if (skey)
        {
            autopick_inkey_hack = 2;
            forget_macro_action();
            return ((char)skey);
        }
    }

    autopick_inkey_hack = 0;

    /* Push the macro "action" onto the key queue */
    while (n > 0)
    {
        /* Mega-Hack - allow numlockless players to play on angband.live */
        char my_key = act[--n];
        if ((online_macros) && (my_key == '.')) my_key = ';';
        
        /* Push the key, notice over-flow */
        if (Term_key_push(my_key)) return (0);
    }


    /* Hack -- Force "inkey()" to call us again */
    return (0);
}

/*
 * Mega-Hack -- special "inkey_next" pointer. XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()". This key sequence will not
 * trigger any macros, and cannot be bypassed by the Borg. It is used
 * in Angband to handle "keymaps".
 */
static cptr inkey_next = NULL;


/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters". These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns. Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is TRUE, then all pending keypresses will be flushed,
 * and any macro processing in progress will be aborted. This flag is
 * set by the "flush()" function, which does not actually flush anything
 * itself, but rather, triggers delayed input flushing via "inkey_xtra".
 *
 * If "inkey_scan" is TRUE, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_base" is TRUE, then all macro processing will be bypassed.
 * If "inkey_base" and "inkey_scan" are both TRUE, then this function will
 * not return immediately, but will wait for a keypress for as long as the
 * normal macro matching code would, allowing the direct entry of macro
 * triggers. The "inkey_base" flag is extremely dangerous!
 *
 * If "inkey_flag" is TRUE, then we will assume that we are waiting for a
 * normal command, and we will only show the cursor if "hilite_player" is
 * TRUE (or if the player is in a store), instead of always showing the
 * cursor. The various "main-xxx.c" files should avoid saving the game
 * in response to a "menu item" request unless "inkey_flag" is TRUE, to
 * prevent savefile corruption.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key. This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of "ascii 30" (ctrl-caret, aka ctrl-shift-six)
 * and "ascii 31" (ctrl-underscore, aka ctrl-shift-minus), which are used to
 * provide support for simple keyboard "macros". These keys are so strange
 * that their loss as normal keys will probably be noticed by nobody. The
 * "ascii 30" key is used to indicate the "end" of a macro action, which
 * allows recursive macros to be avoided. The "ascii 31" key is used by
 * some of the "main-xxx.c" files to introduce macro trigger sequences.
 *
 * Hack -- we use "ascii 29" (ctrl-right-bracket) as a special "magic" key,
 * which can be used to give a variety of "sub-commands" which can be used
 * any time. These sub-commands could include commands to take a picture of
 * the current screen, to start/stop recording a macro action, etc.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
char inkey(void)
{
    int v;
    char kk;
    char ch = 0;
    bool done = FALSE;
    term *old = Term;

    /* Hack -- Use the "inkey_next" pointer */
    if (inkey_next && *inkey_next && !inkey_xtra)
    {
        /* Get next character, and advance */
        ch = *inkey_next++;

        /* Cancel the various "global parameters" */
        inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;

        /* Accept result */
        return (ch);
    }

    /* Forget pointer */
    inkey_next = NULL;


    /* Hack -- handle delayed "flush()" */
    if (inkey_xtra)
    {
        /* End "macro action" */
        parse_macro = FALSE;

        /* End "macro trigger" */
        parse_under = FALSE;

        /* Forget old keypresses */
        Term_flush();
    }


    /* Access cursor state */
    (void)Term_get_cursor(&v);

    /* Show the cursor if waiting, except sometimes in "command" mode */
    if (!inkey_scan && (!inkey_flag || hilite_player || character_icky))
    {
        /* Show the cursor */
        (void)Term_set_cursor(1);
    }


    /* Hack -- Activate main screen */
    Term_activate(angband_term[0]);


    /* Get a key */
    while (!ch)
    {
        /* Hack -- Handle "inkey_scan" */
        if (!inkey_base && inkey_scan &&
            (0 != Term_inkey(&kk, FALSE, FALSE)))
        {
            break;
        }


        /* Hack -- Flush output once when no key ready */
        if (!done && (0 != Term_inkey(&kk, FALSE, FALSE)))
        {
            /* Hack -- activate proper term */
            Term_activate(old);

            /* Flush output */
            Term_fresh();

            /* Hack -- activate main screen */
            Term_activate(angband_term[0]);

            /* Mega-Hack -- reset saved flag */
            character_saved = FALSE;

            /* Mega-Hack -- reset signal counter */
            signal_count = 0;

            /* Only once */
            done = TRUE;
        }


        /* Hack -- Handle "inkey_base" */
        if (inkey_base)
        {
            int w = 0;

            /* Wait forever */
            if (!inkey_scan)
            {
                /* Wait for (and remove) a pending key */
                if (0 == Term_inkey(&ch, TRUE, TRUE))
                {
                    /* Done */
                    break;
                }

                /* Oops */
                break;
            }

            /* Wait */
            while (TRUE)
            {
                /* Check for (and remove) a pending key */
                if (0 == Term_inkey(&ch, FALSE, TRUE))
                {
                    /* Done */
                    break;
                }

                /* No key ready */
                else
                {
                    /* Increase "wait" */
                    w += 10;

                    /* Excessive delay */
                    if (w >= 100) break;

                    /* Delay */
                    Term_xtra(TERM_XTRA_DELAY, w);
                }
            }

            /* Done */
            break;
        }


        /* Get a key (see above) */
        ch = inkey_aux();


        /* Handle "control-right-bracket" */
        if (ch == 29)
        {
            /* Strip this key */
            ch = 0;

            /* Grab a screen dump to embed in documentation.
               On curses, you can only use the ')' command at
               certain points while I would ambitiously like
               to document much much more :) This little hook
               is awesome, btw! */
            do_cmd_save_screen_doc();
            do_cmd_save_screen_txt();
            do_cmd_save_screen_html();

            /* Continue */
            continue;
        }


        /* Treat back-quote as escape */
/*        if (ch == '`') ch = ESCAPE; */


        /* End "macro trigger" */
        if (parse_under && (ch <= 32))
        {
            /* Strip this key */
            ch = 0;

            /* End "macro trigger" */
            parse_under = FALSE;
        }


        /* Handle "control-caret" */
        if (ch == 30)
        {
            /* Strip this key */
            ch = 0;
        }

        /* Handle "control-underscore" */
        else if (ch == 31)
        {
            /* Strip this key */
            ch = 0;

            /* Begin "macro trigger" */
            parse_under = TRUE;
        }

        /* Inside "macro trigger" */
        else if (parse_under)
        {
            /* Strip this key */
            ch = 0;
        }
    }


    /* Hack -- restore the term */
    Term_activate(old);


    /* Restore the cursor */
    Term_set_cursor(v);


    /* Cancel the various "global parameters" */
    inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;

    update_playtime(); /* ... which now limits to 30s max */

    /* Return the keypress */
    return (ch);
}




/*
 * We use a global array for all inscriptions to reduce the memory
 * spent maintaining inscriptions. Of course, it is still possible
 * to run out of inscription memory, especially if too many different
 * inscriptions are used, but hopefully this will be rare.
 *
 * We use dynamic string allocation because otherwise it is necessary
 * to pre-guess the amount of quark activity. We limit the total
 * number of quarks, but this is much easier to "expand" as needed.
 *
 * Any two items with the same inscription will have the same "quark"
 * index, which should greatly reduce the need for inscription space.
 *
 * Note that "quark zero" is NULL and should not be "dereferenced".
 */

/*
 * Initialize the quark array
 */
void quark_init(void)
{
    /* Quark variables */
    C_MAKE(quark__str, QUARK_MAX, cptr);

    /* Prepare first quark, which is used when quark_add() is failed */
    quark__str[1] = z_string_make("");

    /* There is one quark (+ NULL) */
    quark__num = 2;
}


/*
 * Add a new "quark" to the set of quarks.
 */
s16b quark_add(cptr str)
{
    int i;

    /* Look for an existing quark */
    for (i = 1; i < quark__num; i++)
    {
        /* Check for equality */
        if (streq(quark__str[i], str)) return (i);
    }

    /* Return "" when no room is available */
    if (quark__num == QUARK_MAX) return 1;

    /* New maximal quark */
    quark__num = i + 1;

    /* Add a new quark */
    quark__str[i] = z_string_make(str);

    /* Return the index */
    return (i);
}


/*
 * This function looks up a quark
 */
cptr quark_str(s16b i)
{
    cptr q;

    /* Return NULL for an invalid index */
    if ((i < 1) || (i >= quark__num)) return NULL;

    /* Access the quark */
    q = quark__str[i];

    /* Return the quark */
    return (q);
}



/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save_aux(void)
{
    /* Save the screen (if legal) */
    if (screen_depth++ == 0) Term_save();

    /* Increase "icky" depth */
    character_icky++;
}

void screen_save(void)
{
    /* Hack -- Flush messages */
    msg_print(NULL);
    screen_save_aux();
}


/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load_aux(void)
{
    /* Load the screen (if legal) */
    if (--screen_depth == 0) Term_load();

    /* Decrease "icky" depth */
    character_icky--;
}

void screen_load(void)
{
    screen_load_aux();
    msg_line_redraw();
}

bool screen_is_saved(void)
{
    if (screen_depth > 0)
        return TRUE;
    return FALSE;
}

/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string. Do not clear the line.
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
    /* Position cursor, Dump the attr/text */
    Term_putstr(col, row, -1, attr, str);
}

/*
 * As above, but in "white"
 */
void put_str(cptr str, int row, int col)
{
    /* Spawn */
    Term_putstr(col, row, -1, TERM_WHITE, str);
}



/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
    /* Clear line, position cursor */
    Term_erase(col, row, 255);

    /* Dump the attr/text */
    Term_addstr(-1, attr, str);
}

/*
 * As above, but in "white"
 */
void prt(cptr str, int row, int col)
{
    /* Spawn */
    c_prt(TERM_WHITE, str, row, col);
}




/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line. Also, allow "newline" to force
 * a "wrap" to the next line. Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "c_roff()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void c_roff(byte a, cptr str)
{
    int x, y;

    int w, h;

    cptr s;

    /* Obtain the size */
    (void)Term_get_size(&w, &h);

    /* Obtain the cursor */
    (void)Term_locate(&x, &y);

    /* Hack -- No more space */
    if( y == h - 1 && x > w - 3) return;

    /* Process the string */
    for (s = str; *s; s++)
    {
        char ch;

        /* Force wrap */
        if (*s == '\n')
        {
            /* Wrap */
            x = 0;
            y++;

            /* No more space */
            if( y == h ) break;

            /* Clear line, move cursor */
            Term_erase(x, y, 255);

            break;
        }

        /* Clean up the char */
        ch = (isprint(*s) ? *s : ' ');


        /* Wrap words as needed */
        if ((x >= w - 1) && (ch != ' '))

        {
            int i, n = 0;

            byte av[256];
            char cv[256];

            /* Wrap word */
            if (x < w)
            {
                /* Scan existing text */
                for (i = w - 2; i >= 0; i--)
                {
                    /* Grab existing attr/char */
                    Term_what(i, y, &av[i], &cv[i]);

                    /* Break on space */
                    if (cv[i] == ' ') break;

                    /* Track current word */
                    n = i;
                }
            }

            /* Special case */
            if (n == 0) n = w;

            /* Clear line */
            Term_erase(n, y, 255);

            /* Wrap */
            x = 0;
            y++;

            /* No more space */
            if( y == h ) break;

            /* Clear line, move cursor */
            Term_erase(x, y, 255);

            /* Wrap the word (if any) */
            for (i = n; i < w - 1; i++)
            {
                /* Dump */
                Term_addch(av[i], cv[i]);

                /* Advance (no wrap) */
                if (++x > w) x = w;
            }
        }

        /* Dump */
        Term_addch(a, ch);


        /* Advance */
        if (++x > w) x = w;
    }
}

/*
 * As above, but in "white"
 */
void roff(cptr str)
{
    /* Spawn */
    c_roff(TERM_WHITE, str);
}




/*
 * Clear part of the screen
 */
void clear_from(int row)
{
    int y;

    /* Erase requested rows */
    for (y = row; y < Term->hgt; y++)
    {
        /* Erase part of the screen */
        Term_erase(0, y, 255);
    }
}




/*
 * Get some string input at the cursor location.
 * Assume the buffer is initialized to a default string.
 *
 * The default buffer is in Overwrite mode and displayed in yellow at
 * first. Normal chars clear the yellow text and append the char in
 * white text.
 *
 * LEFT (^B) and RIGHT (^F) movement keys move the cursor position.
 * If the text is still displayed in yellow (Overwite mode), it will
 * turns into white (Insert mode) when cursor moves.
 *
 * DELETE (^D) deletes a char at the cursor position.
 * BACKSPACE (^H) deletes a char at the left of cursor position.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 *
 * N.B. len is not the length of buf, but the length of input.
 * buf s/b char[len+1] ... at least!
 */
bool askfor_aux(char *buf, int len, bool numpad_cursor)
{
    int y, x;
    int pos = 0;

    /*
     * Text color
     * TERM_YELLOW : Overwrite mode
     * TERM_WHITE : Insert mode
     */
    byte color = TERM_YELLOW;

    /* Locate the cursor position */
    Term_locate(&x, &y);

    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Paranoia -- check column */
    if ((x < 0) || (x >= 80)) x = 0;

    /* Restrict the length */
    if (x + len > 80) len = 80 - x;

    /* Paranoia -- Clip the default entry */
    buf[len] = '\0';


    /* Process input */
    while (TRUE)
    {
        int skey;

        /* Display the string */
        Term_erase(x, y, len);
        Term_putstr(x, y, -1, color, buf);

        /* Place cursor */
        Term_gotoxy(x + pos, y);

        /* Get a special key code */
        skey = inkey_special(numpad_cursor);

        /* Analyze the key */
        switch (skey)
        {
        case SKEY_LEFT:
        case KTRL('b'):
        {
            int i = 0;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at beginning of line */
            if (0 == pos) break;

            while (TRUE)
            {
                int next_pos = i + 1;


                /* Is there the cursor at next position? */ 
                if (next_pos >= pos) break;

                /* Move to next */
                i = next_pos;
            }

            /* Get previous position */
            pos = i;

            break;
        }

        case SKEY_RIGHT:
        case KTRL('f'):
            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at end of line */
            if ('\0' == buf[pos]) break;

            pos++;

            break;

        case ESCAPE:
            /* Cancel input */
            buf[0] = '\0';
            return FALSE;

        case '\n':
        case '\r':
            /* Success */
            return TRUE;

        case '\010':
            /* Backspace */
        {
            int i = 0;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at beginning of line */
            if (0 == pos) break;

            while (TRUE)
            {
                int next_pos = i + 1;


                /* Is there the cursor at next position? */ 
                if (next_pos >= pos) break;

                /* Move to next */
                i = next_pos;
            }

            /* Get previous position */
            pos = i;

            /* Fall through to 'Delete key' */
        }

        case 0x7F:
        case KTRL('d'):
            /* Delete key */
        {
            int dst, src;

            /* Now on insert mode */
            color = TERM_WHITE;

            /* No move at end of line */
            if ('\0' == buf[pos]) break;

            /* Position of next character */
            src = pos + 1;


            dst = pos;

            /* Move characters at src to dst */
            while ('\0' != (buf[dst++] = buf[src++]))
                /* loop */;

            break;
        }

        default:
        {
            /* Insert a character */

            char tmp[100];
            char c;

            /* Ignore special keys */
            if (skey & SKEY_MASK) break;

            /* Get a character code */
            c = (char)skey;

            if (color == TERM_YELLOW)
            {
                /* Overwrite default string */
                buf[0] = '\0';

                /* Go to insert mode */
                color = TERM_WHITE;
            }

            /* Save right part of string */
            strcpy(tmp, buf + pos);
            if (pos < len && isprint(c))
            {
                buf[pos++] = c;
            }
            else
            {
                bell();
            }

            /* Terminate */
            buf[pos] = '\0';

            /* Write back the left part of string */
            my_strcat(buf, tmp, len + 1);

            break;
        } /* default: */

        }

    } /* while (TRUE) */
}


/*
 * Get some string input at the cursor location.
 *
 * Allow to use numpad keys as cursor keys.
 *
 * N.B. len is not the length of buf, but the length of input.
 * buf s/b char[len+1] ... at least!
 */
bool askfor(char *buf, int len)
{
    return askfor_aux(buf, len, TRUE);
}


/*
 * Get a string from the user
 *
 * The "prompt" should take the form "Prompt: "
 *
 * Note that the initial contents of the string is used as
 * the default response, so be sure to "clear" it if needed.
 *
 * We clear the input, and return FALSE, on "ESCAPE".
 *
 * N.B. len is not the length of buf, but the length of input.
 * buf s/b char[len+1] ... at least!
 */
bool get_string(cptr prompt, char *buf, int len)
{
    bool res;

    /* Paranoia XXX XXX XXX */
    msg_print(NULL);

    /* Display prompt */
    prt(prompt, 0, 0);

    /* Ask the user for a string */
    res = askfor(buf, len);

    /* Clear prompt */
    prt("", 0, 0);

    /* Result */
    return (res);
}


/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(cptr prompt)
{
/*  return get_check_strict(prompt, 0);*/
    char buf[255];
    sprintf(buf, "%s<color:y>[y/n]</color>", prompt);
    if (msg_prompt(buf, "ny", PROMPT_DEFAULT) == 'y')
        return TRUE;
    return FALSE;
}

/*
 * Verify something with the user strictly
 *
 * mode & CHECK_OKAY_CANCEL : force user to answer 'O'kay or 'C'ancel
 * mode & CHECK_NO_ESCAPE   : don't allow ESCAPE key
 * mode & CHECK_NO_HISTORY  : no message_add
 * mode & CHECK_DEFAULT_Y   : accept any key as y, except n and Esc.
 */
bool get_check_strict(cptr prompt, int mode)
{
    int i;
    char buf[80];
    bool flag = FALSE;

    auto_more_state = AUTO_MORE_PROMPT;

    /* Paranoia XXX XXX XXX */
    msg_print(NULL);

    if (!rogue_like_commands)
        mode &= ~CHECK_OKAY_CANCEL;


    /* Hack -- Build a "useful" prompt */
    if (mode & CHECK_OKAY_CANCEL)
    {
        my_strcpy(buf, prompt, sizeof(buf)-15);
        strcat(buf, "[(O)k/(C)ancel]");
    }
    else if (mode & CHECK_DEFAULT_Y)
    {
        my_strcpy(buf, prompt, sizeof(buf)-5);
        strcat(buf, "[Y/n]");
    }
    else
    {
        my_strcpy(buf, prompt, sizeof(buf)-5);
        strcat(buf, "[y/n]");
    }

    /* Prompt for it */
    prt(buf, 0, 0);

    if (!(mode & CHECK_NO_HISTORY) && p_ptr->playing)
    {
        /* HACK : Add the line to message buffer */
        cmsg_add(TERM_YELLOW, buf);
        p_ptr->window |= (PW_MESSAGE);
        window_stuff();
    }

    /* Get an acceptable answer */
    while (TRUE)
    {
        i = inkey();

        if (!(mode & CHECK_NO_ESCAPE))
        {
            if (i == ESCAPE)
            {
                flag = FALSE;
                break;
            }
        }

        if (mode & CHECK_OKAY_CANCEL)
        {
            if (i == 'o' || i == 'O')
            {
                flag = TRUE;
                break;
            }
            else if (i == 'c' || i == 'C')
            {
                flag = FALSE;
                break;
            }
        }
        else
        {
            if (i == 'y' || i == 'Y')
            {
                flag = TRUE;
                break;
            }
            else if (i == 'n' || i == 'N')
            {
                flag = FALSE;
                break;
            }
        }

        if (mode & CHECK_DEFAULT_Y)
        {
            flag = TRUE;
            break;
        }

        bell();
    }

    /* Erase the prompt */
    prt("", 0, 0);

    /* Return the flag */
    return flag;
}


/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command, bool z_escape)
{
    /* Paranoia XXX XXX XXX */

    /* Wednesday 2016-07-27 20:14:30 lumiera: removing this to avoid annoying -more- prompt in capture ball activation
    msg_print(NULL);
    */
    msg_line_clear();

    /* Display a prompt */
    prt(prompt, 0, 0);

    /* Get a key */
    if (get_com_no_macros)
        *command = inkey_special(FALSE);
    else
        *command = inkey();

    /* Clear the prompt */
    prt("", 0, 0);

    /* Handle "cancel" */
    if (*command == ESCAPE) return (FALSE);
    if (z_escape && ((*command == 'z') || (*command == 'Z'))) return (FALSE);

    /* Success */
    return (TRUE);
}


/*
 * Request a "quantity" from the user
 *
 * Hack -- allow "command_arg" to specify a quantity
 */
s16b get_quantity_aux(cptr prompt, int max, int default_amt)
{
    bool res;
    int amt;
    char tmp[80];
    char buf[80];


    /* Use "command_arg" */
    if (command_arg)
    {
        /* Extract a number */
        amt = command_arg;

        /* Clear "command_arg" */
        command_arg = 0;

        /* Enforce the maximum */
        if (amt > max) amt = max;

        /* Use it */
        return (amt);
    }

#ifdef ALLOW_REPEAT /* TNB */

    /* Get the item index */
    if ((max != 1) && repeat_pull(&amt))
    {
        /* Enforce the maximum */
        if (amt > max) amt = max;

        /* Enforce the minimum */
        if (amt < 0) amt = 0;

        /* Use it */
        return (amt);
    }

#endif /* ALLOW_REPEAT -- TNB */

    /* Build a prompt if needed */
    if (!prompt)
    {
        /* Build a prompt */
        sprintf(tmp, "Quantity (1-%d): ", max);


        /* Use that prompt */
        prompt = tmp;
    }

    /* Paranoia XXX XXX XXX */
    msg_print(NULL);

    /* Display prompt */
    prt(prompt, 0, 0);

    /* Default to one */
    amt = MAX(1, MIN(default_amt, max));

    /* Build the default */
    sprintf(buf, "%d", amt);

    /*
     * Ask for a quantity
     * Don't allow to use numpad as cursor key.
     */
    res = askfor_aux(buf, 6, FALSE);

    /* Clear prompt */
    prt("", 0, 0);

    /* Cancelled */
    if (!res) return 0;

    /* Extract a number */
    amt = atoi(buf);

    /* A letter means "all" */
    if (isalpha(buf[0])) amt = max;

    /* Enforce the maximum */
    if (amt > max) amt = max;

    /* Enforce the minimum */
    if (amt < 0) amt = 0;

#ifdef ALLOW_REPEAT /* TNB */

    if (amt) repeat_push(amt);

#endif /* ALLOW_REPEAT -- TNB */

    /* Return the result */
    return (amt);
}

/*
 * Request a "quantity" from the user
 */
s16b get_quantity(cptr prompt, int max)
{
    return get_quantity_aux(prompt, max, 1);
}


/*
 * Pause for user response XXX XXX XXX
 */
void pause_line_aux(cptr prompt, int row, int col)
{
    prt("", row, 0);
    put_str(prompt, row, col);

    (void)inkey();
    prt("", row, 0);
}

void pause_line(int row)
{
    if (row < 0)
    {
        int w, h;
        Term_get_size(&w, &h);
        row = h - 1;
    }

    pause_line_aux("[Press any key to continue]", row, 23);
}


/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];



typedef struct
{
    cptr name;
    byte cmd;
    bool fin;
} menu_naiyou;

menu_naiyou menu_info[10][10] =
{
    {
        {"Magic/Special", 1, FALSE},
        {"Action", 2, FALSE},
        {"Items(use)", 3, FALSE},
        {"Items(other)", 4, FALSE},
        {"Equip", 5, FALSE},
        {"Door", 6, FALSE},
        {"Information", 7, FALSE},
        {"Options", 8, FALSE},
        {"Save/Exit/Help", 9, FALSE},
        {"", 0, FALSE},
    },

    {
        {"Use(m)", 'm', TRUE},
        {"See tips(b/P)", 'b', TRUE},
        {"Study(G)", 'G', TRUE},
        {"Special abilities(U/O)", 'U', TRUE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE}
    },

    {
        {"Rest(R)", 'R', TRUE},
        {"Disarm a trap(D)", 'D', TRUE},
        {"Search(s)", 's', TRUE},
        {"Look(l/x)", 'l', TRUE},
        {"Target(*)", '*', TRUE},
        {"Dig(T/^t)", 'T', TRUE},
        {"Go up stairs(<)", '<', TRUE},
        {"Go down stairs(>)", '>', TRUE},
        {"Command pets(p)", 'p', TRUE},
        {"Toggle searching(S/#)", 'S', TRUE}
    },

    {
        {"Read a scroll(r)", 'r', TRUE},
        {"Drink a potion(q)", 'q', TRUE},
        {"Use a staff(u/Z)", 'u', TRUE},
        {"Aim a wand(a/z)", 'a', TRUE},
        {"Zap a rod(z/a)", 'z', TRUE},
        {"Activate an equipment(A)", 'A', TRUE},
        {"Eat(E)", 'E', TRUE},
        {"Fire missile weapon(f/t)", 'f', TRUE},
        {"Throw an item(v)", 'v', TRUE},
        {"", 0, FALSE}
    },

    {
        {"Get items(g)", 'g', TRUE},
        {"Drop an item(d)", 'd', TRUE},
        {"Destroy an item(k/^d)", 'k', TRUE},
        {"Inscribe an item(Z/{)", 'Z', TRUE},
        {"Uninscribe an item(})", '}', TRUE},
        {"Inspect an item(I)", 'I', TRUE},
        {"Inventory list(i)", 'i', TRUE},
        {"Travel to item(H/^E)", 'H', TRUE},
        {"Resume travel(J/())", 'J', TRUE},
        {"", 0, FALSE}
    },

    {
        {"Wear(w)", 'w', TRUE},
        {"Take off(t/T)", 't', TRUE},
        {"Refuel(F)", 'F', TRUE},
        {"Equipment list(e)", 'e', TRUE},
        {"Switch ring fingers", 'W', TRUE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE}
    },

    {
        {"Open(o)", 'o', TRUE},
        {"Close(c)", 'c', TRUE},
        {"Bash a door(B/f)", 'B', TRUE},
        {"Jam a door(j/S)", 'j', TRUE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE}
    },

    {
        {"List monsters(Y/[)", 'Y', TRUE},
        {"List objects(O/])", ']', TRUE},
        {"Character sheet(C)", 'C', TRUE},
        {"Full map(M)", 'M', TRUE},
        {"Map(L/W)", 'L', TRUE},
        {"Level feeling(^f)", KTRL('F'), TRUE},
        {"Identify symbol(/)", '/', TRUE},
        {"Show prev messages(^p)", KTRL('P'), TRUE},
        {"Current time(^t/')", KTRL('T'), TRUE},
        {"Knowledge menu(~)", '~', TRUE}
    },

    {
        {"Set options(=)", '=', TRUE},
        {"Interact with macros(@)", '@', TRUE},
        {"Interact w/ visuals(%)", '%', TRUE},
        {"Interact with colors(&)", '&', TRUE},
        {"Enter a user pref(\")", '\"', TRUE},
        {"Edit Mogaminator(_)", '_', TRUE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE},
        {"", 0, FALSE}
    },

    {
        {"Save and quit(^x)", KTRL('X'), TRUE},
        {"Save(^s)", KTRL('S'), TRUE},
        {"Help(?)", '?', TRUE},
        {"Redraw(^r)", KTRL('R'), TRUE},
        {"Take note(:)", ':', TRUE},
        {"Dump screen dump(()", ')', TRUE},
        {"Load screen dump())", '(', TRUE},
        {"Version info(V)", 'V', TRUE},
        {"Commit suicide(Q)", 'Q', TRUE},
        {"", 0, FALSE}
    },
};

typedef struct
{
    cptr name;
    byte window;
    byte number;
    byte jouken;
    byte jouken_naiyou;
} special_menu_naiyou;

#define MENU_CLASS 1
#define MENU_WILD 2

special_menu_naiyou special_menu_info[] =
{
    {"MindCraft/Special", 0, 0, MENU_CLASS, CLASS_MINDCRAFTER},
    {"Song/Special", 0, 0, MENU_CLASS, CLASS_BARD},
    {"Technique/Special", 0, 0, MENU_CLASS, CLASS_SAMURAI},
    {"Mind/Magic/Special", 0, 0, MENU_CLASS, CLASS_FORCETRAINER},
    {"BrutalPower/Special", 0, 0, MENU_CLASS, CLASS_BERSERKER},
    {"MirrorMagic/Special", 0, 0, MENU_CLASS, CLASS_MIRROR_MASTER},
    {"Ninjutsu/Special", 0, 0, MENU_CLASS, CLASS_NINJA},
    {"Ninjutsu/Special", 0, 0, MENU_CLASS, CLASS_NINJA_LAWYER},
    {"Enter global map(<)", 2, 6, MENU_WILD, FALSE},
    {"Enter local map(>)", 2, 7, MENU_WILD, TRUE},
    {"", 0, 0, 0, 0},
};

static char inkey_from_menu(void)
{
    char cmd;
    int basey, basex;
    int num = 0, max_num, old_num = 0;
    int menu = 0;
    bool kisuu;

    /*if (py - panel_row_min > 10) basey = 2;
    else*/ basey = 13;
    basex = 15;

    /* Clear top line */
    prt("", 0, 0);

    screen_save();

    while(1)
    {
        int i;
        char sub_cmd;
        cptr menu_name;
        if (!menu) old_num = num;
        put_str("+----------------------------------------------------+", basey, basex);
        put_str("|                                                    |", basey+1, basex);
        put_str("|                                                    |", basey+2, basex);
        put_str("|                                                    |", basey+3, basex);
        put_str("|                                                    |", basey+4, basex);
        put_str("|                                                    |", basey+5, basex);
        put_str("+----------------------------------------------------+", basey+6, basex);

        for(i = 0; i < 10; i++)
        {
            int hoge;
            if (!menu_info[menu][i].cmd) break;
            menu_name = menu_info[menu][i].name;
            for(hoge = 0; ; hoge++)
            {
                if (!special_menu_info[hoge].name[0]) break;
                if ((menu != special_menu_info[hoge].window) || (i != special_menu_info[hoge].number)) continue;
                switch(special_menu_info[hoge].jouken)
                {
                case MENU_CLASS:
                    if (p_ptr->pclass == special_menu_info[hoge].jouken_naiyou) menu_name = special_menu_info[hoge].name;
                    break;
                case MENU_WILD:
                    if (py_on_surface())
                    {
                        if ((byte)p_ptr->wild_mode == special_menu_info[hoge].jouken_naiyou) menu_name = special_menu_info[hoge].name;
                    }
                    break;
                default:
                    break;
                }
            }
            put_str(menu_name, basey + 1 + i / 2, basex + 4 + (i % 2) * 24);
        }
        max_num = i;
        kisuu = max_num % 2;
        put_str("> ",basey + 1 + num / 2, basex + 2 + (num % 2) * 24);

        /* Place the cursor on the player */
        move_cursor_relative(py, px);

        /* Get a command */
        sub_cmd = inkey();
        if ((sub_cmd == ' ') || (sub_cmd == 'x') || (sub_cmd == 'X') || (sub_cmd == '\r') || (sub_cmd == '\n'))
        {
            if (menu_info[menu][num].fin)
            {
                cmd = menu_info[menu][num].cmd;
                use_menu = TRUE;
                break;
            }
            else
            {
                menu = menu_info[menu][num].cmd;
                num = 0;
                basey += 2;
                basex += 8;
            }
        }
        else if ((sub_cmd == ESCAPE) || (sub_cmd == 'z') || (sub_cmd == 'Z') || (sub_cmd == '0'))
        {
            if (!menu)
            {
                cmd = ESCAPE;
                break;
            }
            else
            {
                menu = 0;
                num = old_num;
                basey -= 2;
                basex -= 8;
                screen_load();
                screen_save();
            }
        }
        else if ((sub_cmd == '2') || (sub_cmd == 'j') || (sub_cmd == 'J'))
        {
            if (kisuu)
            {
                if (num % 2)
                    num = (num + 2) % (max_num - 1);
                else
                    num = (num + 2) % (max_num + 1);
            }
            else num = (num + 2) % max_num;
        }
        else if ((sub_cmd == '8') || (sub_cmd == 'k') || (sub_cmd == 'K'))
        {
            if (kisuu)
            {
                if (num % 2)
                    num = (num + max_num - 3) % (max_num - 1);
                else
                    num = (num + max_num - 1) % (max_num + 1);
            }
            else num = (num + max_num - 2) % max_num;
        }
        else if ((sub_cmd == '4') || (sub_cmd == '6') || (sub_cmd == 'h') || (sub_cmd == 'H') || (sub_cmd == 'l') || (sub_cmd == 'L'))
        {
            if ((num % 2) || (num == max_num - 1))
            {
                num--;
            }
            else if (num < max_num - 1)
            {
                num++;
            }
        }
    }

    screen_load();
    if (!inkey_next) inkey_next = "";

    return (cmd);
}

/*
 * Request a command from the user.
 *
 * Sets p_ptr->command_cmd, p_ptr->command_dir, p_ptr->command_rep,
 * p_ptr->command_arg. May modify p_ptr->command_new.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters. This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character. This is useful for macros.
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 *
 * Note that "p_ptr->command_new" may not work any more. XXX XXX XXX
 */
void request_command(int shopping)
{
    int i,ct;

    char cmd;
    int mode;

    cptr act;

    /* Roguelike */
    if (rogue_like_commands)
    {
        mode = KEYMAP_MODE_ROGUE;
    }

    /* Original */
    else
    {
        mode = KEYMAP_MODE_ORIG;
    }


    /* No command yet */
    command_cmd = 0;

    /* No "argument" yet */
    command_arg = 0;

    /* No "direction" yet */
    command_dir = 0;

    use_menu = FALSE;


    /* Get command */
    for (ct = 0;;ct++)
    {
        /* Hack -- auto-commands */
        if (command_new)
        {
            /* Flush messages */
            msg_print(NULL);

            /* Use auto-command */
            cmd = (char)command_new;  /* safe?? */

            /* Forget it */
            command_new = 0;
        }

        /* Get a keypress in "command" mode */
        else
        {
            auto_more_state = AUTO_MORE_PROMPT;

            /* Activate "command mode" */
            inkey_flag = TRUE;

            /* Get a command */
            cmd = inkey();

            if (!shopping && command_menu && ((cmd == '\r') || (cmd == '\n') || (cmd == 'x') || (cmd == 'X'))
                && !keymap_act[mode][(byte)(cmd)])
                cmd = inkey_from_menu();
        }

        /* Clear top line
        prt("", 0, 0);*/
        if (cmd != KTRL('R') && cmd != ')')
            msg_line_clear();

        /* Command Count */
        if (cmd == '0')
        {
            int old_arg = command_arg;

            /* Reset */
            command_arg = 0;

            /* Begin the input */
            prt("Count: ", 0, 0);


            /* Get a command count */
            while (1)
            {
                /* Get a new keypress */
                cmd = inkey();

                /* Simple editing (delete or backspace) */
                if ((cmd == 0x7F) || (cmd == KTRL('H')))
                {
                    /* Delete a digit */
                    command_arg = command_arg / 10;

                    /* Show current count */
                    prt(format("Count: %d", command_arg), 0, 0);

                }

                /* Actual numeric data */
                else if (cmd >= '0' && cmd <= '9')
                {
                    /* Stop count at 9999 */
                    if (command_arg >= 1000)
                    {
                        /* Warn */
                        bell();

                        /* Limit */
                        command_arg = 9999;
                    }

                    /* Increase count */
                    else
                    {
                        /* Incorporate that digit */
                        command_arg = command_arg * 10 + D2I(cmd);
                    }

                    /* Show current count */
                    prt(format("Count: %d", command_arg), 0, 0);

                }

                /* Exit on "unusable" input */
                else
                {
                    break;
                }
            }

            /* Hack -- Handle "zero" */
            if (command_arg == 0)
            {
                /* Default to 99 */
                command_arg = 99;

                /* Show current count */
                prt(format("Count: %d", command_arg), 0, 0);

            }

            /* Hack -- Handle "old_arg" */
            if (old_arg != 0)
            {
                /* Restore old_arg */
                command_arg = old_arg;

                /* Show current count */
                prt(format("Count: %d", command_arg), 0, 0);

            }

            /* Hack -- white-space means "enter command now" */
            if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r'))
            {
                /* Get a real command */
                if (!get_com("Command: ", (char *)&cmd, FALSE))

                {
                    /* Clear count */
                    command_arg = 0;

                    /* Continue */
                    continue;
                }
            }
        }


        /* Allow "keymaps" to be bypassed */
        if (cmd == '\\')
        {
            /* Get a real command */
            (void)get_com("Command: ", (char *)&cmd, FALSE);


            /* Hack -- bypass keymaps */
            if (!inkey_next) inkey_next = "";
        }


        /* Allow "control chars" to be entered */
        if (cmd == '^')
        {
            /* Get a new command and controlify it */
            if (get_com("Control: ", (char *)&cmd, FALSE)) cmd = KTRL(cmd);

        }


        /* Look up applicable keymap */
        act = keymap_act[mode][(byte)(cmd)];

        /* Apply keymap if not inside a keymap already */
        if (act && !inkey_next)
        {
            /* Install the keymap (limited buffer size) */
            (void)strnfmt(request_command_buffer, 256, "%s", act);

            /* Start using the buffer */
            inkey_next = request_command_buffer;

            /* Continue */
            continue;
        }


        /* Paranoia */
        if (!cmd) continue;


        /* Use command */
        command_cmd = (byte)cmd;

        /* Done */
        break;
    }

    /* Hack -- Auto-repeat certain commands */
    if (always_repeat && (command_arg <= 0))
    {
        /* Hack -- auto repeat certain commands */
        if (my_strchr("TBDoc+", (char)command_cmd)) /* safe?? */
        {
            /* Repeat 99 times */
            command_arg = 99;
        }
    }

    /* Shopping */
    if (shopping == 1)
    {
        /* Convert */
        switch (command_cmd)
        {
            /* Command "p" -> "purchase" (get) */
        case 'p': command_cmd = 'g'; break;

            /* Command "m" -> "purchase" (get) */
        case 'm': command_cmd = 'g'; break;

            /* Command "s" -> "sell" (drop) */
        case 's': command_cmd = 'd'; break;
        }
    }


    /* Hack -- Scan equipment */
    for (i = 1; i <= equip_max(); i++)
    {
        cptr s;
        object_type *o_ptr = equip_obj(i);

        if (!o_ptr) continue;
        if (!o_ptr->inscription) continue;

        s = quark_str(o_ptr->inscription);
        s = my_strchr(s, '^');
        while (s)
        {
            if ((s[1] == command_cmd) || (s[1] == '*'))
            {
                if (!get_check("Are you sure? "))
                {
                    /* Hack -- Use space */
                    command_cmd = ' ';
                }
            }
            s = my_strchr(s + 1, '^');
        }
    }


    /* Hack -- erase the message line. */
    prt("", 0, 0);
}



/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
    switch (ch)
    {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
    case 'A':
    case 'E':
    case 'I':
    case 'O':
    case 'U':
        return (TRUE);
    }

    return (FALSE);
}



#if 0

/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * XXX Could be made more efficient, especially in the
 * case where "insert" is smaller than "target".
 */
static bool insert_str(char *buf, cptr target, cptr insert)
{
    int   i, len;
    int           b_len, t_len, i_len;

    /* Attempt to find the target (modify "buf") */
    buf = my_strstr(buf, target);

    /* No target found */
    if (!buf) return (FALSE);

    /* Be sure we have an insertion string */
    if (!insert) insert = "";

    /* Extract some lengths */
    t_len = strlen(target);
    i_len = strlen(insert);
    b_len = strlen(buf);

    /* How much "movement" do we need? */
    len = i_len - t_len;

    /* We need less space (for insert) */
    if (len < 0)
    {
        for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
    }

    /* We need more space (for insert) */
    else if (len > 0)
    {
        for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
    }

    /* If movement occured, we need a new terminator */
    if (len) buf[b_len+len] = '\0';

    /* Now copy the insertion string */
    for (i = 0; i < i_len; ++i) buf[i] = insert[i];

    /* Successful operation */
    return (TRUE);
}


#endif


/*
 * GH
 * Called from cmd4.c and a few other places. Just extracts
 * a direction from the keymap for ch (the last direction,
 * in fact) byte or char here? I'm thinking that keymaps should
 * generally only apply to single keys, which makes it no more
 * than 128, so a char should suffice... but keymap_act is 256...
 */
int get_keymap_dir(char ch, bool under)
{
    int d = 0;

    /* Already a direction? */
    if (isdigit(ch))
    {
        d = D2I(ch);
    }
    else
    {
        int mode;
        cptr act, s;

        /* Roguelike */
        if (rogue_like_commands)
        {
            mode = KEYMAP_MODE_ROGUE;
        }

        /* Original */
        else
        {
            mode = KEYMAP_MODE_ORIG;
        }

        /* Extract the action (if any) */
        act = keymap_act[mode][(byte)(ch)];

        /* Analyze */
        if (act)
        {
            /* Convert to a direction */
            for (s = act; *s; ++s)
            {
                /* Use any digits in keymap */
                if (isdigit(*s)) d = D2I(*s);
            }
        }
    }

    /* Paranoia */
    if ((d == 5) && (!under)) d = 0;

    /* Return direction */
    return (d);
}


#ifdef ALLOW_REPEAT /* TNB */

#define REPEAT_MAX 20

typedef struct {
    int keys[REPEAT_MAX];
    int ct;
    int pos;
} _repeat_buffer_t, *_repeat_buffer_ptr;

static _repeat_buffer_t _repeat_buffers[255];

enum { _UNKNOWN, _RECORDING, _PLAYING };
static char _repeat_reg;
static int _repeat_state;

static void _repeat_push(_repeat_buffer_ptr buf, int what)
{
    if (buf->ct == REPEAT_MAX) return;
    buf->keys[buf->ct++] = what;
}

void repeat_push(int what)
{
    if (_repeat_state == _RECORDING)
    {
        _repeat_push(&_repeat_buffers['.'], what);
        if (_repeat_reg && _repeat_reg != '.')
            _repeat_push(&_repeat_buffers[(int)_repeat_reg], what);
    }
}

static void _repeat_pop(_repeat_buffer_ptr buf)
{
    if (buf->ct > 0)
        buf->ct--;
}

void repeat_pop(void)
{
    if (_repeat_state == _RECORDING)
    {
        _repeat_pop(&_repeat_buffers['.']);
        if (_repeat_reg && _repeat_reg != '.')
            _repeat_pop(&_repeat_buffers[(int)_repeat_reg]);
    }
}

static bool _repeat_pull(_repeat_buffer_ptr buf, int *what)
{
    if (buf->pos >= buf->ct) return FALSE;
    *what = buf->keys[buf->pos++];
    return TRUE;
}

bool repeat_pull(int *what)
{
    if (_repeat_state == _PLAYING)
        return _repeat_pull(&_repeat_buffers[(int)_repeat_reg], what);
    return FALSE;
}

static void _repeat_list_aux(doc_ptr doc, int i)
{
    _repeat_buffer_ptr buf = &_repeat_buffers[i];
    if (buf->ct)
    {
        int j, c;
        doc_printf(doc, "<color:y>%c</color>:", (char)i);
        for (j = 0; j < buf->ct; j++)
        {
            c = buf->keys[j];
            /* Range checking is required before calling isprint on Windows */
            if (0 < c && c < 256 && isprint(c))
                doc_printf(doc, "'%c' ", (char)c);
            else
                doc_printf(doc, "%d ", c);
        }
        doc_newline(doc);
    }
}

static void _repeat_list(void)
{
    int     i;
    rect_t  r = ui_map_rect();
    doc_ptr doc = doc_alloc(r.cx < 80 ? r.cx : 80);

    for (i = 0; i < 255; i++)
        _repeat_list_aux(doc, i);
    doc_sync_term(doc, doc_range_all(doc), doc_pos_create(r.x, r.y));
    doc_free(doc);
}

void repeat_check(int shopping)
{
    int what;

    /* Ignore some commands */
    if (command_cmd == ESCAPE) return;
    if (command_cmd == ' ') return;
    if (command_cmd == '\r') return;
    if (command_cmd == '\n') return;

    /* Playback Command */
    if (command_cmd == '\'')
    {
        _repeat_state = _PLAYING;
        prt("Playback: ", 0, 0);
        _repeat_reg = inkey_special(FALSE);
        if (_repeat_reg == '\'')
        {
            Term_save();
            _repeat_list();
            _repeat_reg = inkey_special(FALSE);
            Term_load();
        }
        prt("", 0, 0);
        if (_repeat_reg == ESCAPE)
        {
            _repeat_reg = 0;
            command_cmd = ESCAPE;
            _repeat_state = _UNKNOWN;
            return;
        }
        _repeat_buffers[(int)_repeat_reg].pos = 0;
        if (repeat_pull(&what))
            command_cmd = what;
        else
        {
            msg_format("There is no recorded command in <color:y>%c</color>. Type <color:keypress>'</color> to view recordings.", _repeat_reg);
            _repeat_reg = 0;
            _repeat_state = _UNKNOWN;
            command_cmd = ESCAPE;
            return;
        }
    }
    /* Repeat Last Command: Backwards Compatibility. Same as '. */
    else if (command_cmd == 'n')
    {
        _repeat_state = _PLAYING;
        _repeat_reg = '.';
        _repeat_buffers[(int)_repeat_reg].pos = 0;
        if (repeat_pull(&what))
            command_cmd = what;
        else
            request_command(shopping);
    }
    /* Record Command */
    else 
    {
        _repeat_state = _RECORDING;
        _repeat_reg = 0;
        if (command_cmd == '"')
        {
            prt("Record: ", 0, 0);
            _repeat_reg = inkey_special(FALSE);
            prt("", 0, 0);
            if (_repeat_reg == ESCAPE)
            {
                _repeat_reg = 0;
                command_cmd = ESCAPE;
                _repeat_state = _UNKNOWN;
                return;
            }
            _repeat_buffers[(int)_repeat_reg].ct = 0;
            _repeat_buffers[(int)_repeat_reg].pos = 0;
            prt("Command: ", 0, 0);
            request_command(shopping);
            prt("", 0, 0);
        }
        _repeat_buffers['.'].ct = 0;
        _repeat_buffers['.'].pos = 0;

        what = command_cmd;
        repeat_push(what);
    }
}

#endif /* ALLOW_REPEAT -- TNB */


#ifdef SORT_R_INFO

/*
 * Array size for which InsertionSort
 * is used instead of QuickSort
 */
#define CUTOFF 4


/*
 * Exchange two sort-entries
 * (should probably be coded inline
 * for speed increase)
 */
static void swap(tag_type *a, tag_type *b)
{
    tag_type temp;

    temp.tag = a->tag;
    temp.value = a->value;

    a->tag = b->tag;
    a->value = b->value;

    b->tag = temp.tag;
    b->value = temp.value;
}


/*
 * Insertion-Sort algorithm
 * (used by the Quicksort algorithm)
 */
static void InsertionSort(tag_type elements[], int number)
{
    int j, P;

    tag_type tmp;

    for (P = 1; P < number; P++)
    {
        tmp = elements[P];
        for (j = P; (j > 0) && (elements[j - 1].tag > tmp.tag); j--)
            elements[j] = elements[j - 1];
        elements[j] = tmp;
    }
}


/*
 * Helper function for Quicksort
 */
static tag_type median3(tag_type elements[], int left, int right)
{
    int center = (left + right) / 2;

    if (elements[left].tag > elements[center].tag)
        swap(&elements[left], &elements[center]);
    if (elements[left].tag > elements[right].tag)
        swap(&elements[left], &elements[right]);
    if (elements[center].tag > elements[right].tag)
        swap(&elements[center], &elements[right]);

    swap(&elements[center], &elements[right - 1]);
    return (elements[right - 1]);
}


/*
 * Quicksort algorithm
 *
 * The "median of three" pivot selection eliminates
 * the bad case of already sorted input.
 *
 * We use InsertionSort for smaller sub-arrays,
 * because it is faster in this case.
 *
 * For details see: "Data Structures and Algorithm
 * Analysis in C" by Mark Allen Weiss.
 */
static void quicksort(tag_type elements[], int left, int right)
{
    int i, j;
    tag_type pivot;

    if (left + CUTOFF <= right)
    {
        pivot = median3(elements, left, right);

        i = left; j = right -1;

        while (TRUE)
        {
            while (elements[++i].tag < pivot.tag);
            while (elements[--j].tag > pivot.tag);

            if (i < j)
                swap(&elements[i], &elements[j]);
            else
                break;
        }

        /* Restore pivot */
        swap(&elements[i], &elements[right - 1]);

        quicksort(elements, left, i - 1);
        quicksort(elements, i + 1, right);
    }
    else
    {
        /* Use InsertionSort on small arrays */
        InsertionSort(elements + left, right - left + 1);
    }
}


/*
 * Frontend for the sorting algorithm
 *
 * Sorts an array of tagged pointers
 * with <number> elements.
 */
void tag_sort(tag_type elements[], int number)
{
    quicksort(elements, 0, number - 1);
}

#endif /* SORT_R_INFO */

#ifdef SUPPORT_GAMMA

/* Table of gamma values */
byte gamma_table[256];

/* Table of ln(x/256) * 256 for x going from 0 -> 255 */
static s16b gamma_helper[256] =
{
0,-1420,-1242,-1138,-1065,-1007,-961,-921,-887,-857,-830,-806,-783,-762,-744,-726,
-710,-694,-679,-666,-652,-640,-628,-617,-606,-596,-586,-576,-567,-577,-549,-541,
-532,-525,-517,-509,-502,-495,-488,-482,-475,-469,-463,-457,-451,-455,-439,-434,
-429,-423,-418,-413,-408,-403,-398,-394,-389,-385,-380,-376,-371,-367,-363,-359,
-355,-351,-347,-343,-339,-336,-332,-328,-325,-321,-318,-314,-311,-308,-304,-301,
-298,-295,-291,-288,-285,-282,-279,-276,-273,-271,-268,-265,-262,-259,-257,-254,
-251,-248,-246,-243,-241,-238,-236,-233,-231,-228,-226,-223,-221,-219,-216,-214,
-212,-209,-207,-205,-203,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,
-178,-176,-174,-172,-170,-168,-166,-164,-162,-160,-158,-156,-155,-153,-151,-149,
-147,-146,-144,-142,-140,-139,-137,-135,-134,-132,-130,-128,-127,-125,-124,-122,
-120,-119,-117,-116,-114,-112,-111,-109,-108,-106,-105,-103,-102,-100,-99,-97,
-96,-95,-93,-92,-90,-89,-87,-86,-85,-83,-82,-80,-79,-78,-76,-75,
-74,-72,-71,-70,-68,-67,-66,-65,-63,-62,-61,-59,-58,-57,-56,-54,
-53,-52,-51,-50,-48,-47,-46,-45,-44,-42,-41,-40,-39,-38,-37,-35,
-34,-33,-32,-31,-30,-29,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,
-17,-16,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1
};


/* 
 * Build the gamma table so that floating point isn't needed.
 * 
 * Note gamma goes from 0->256. The old value of 100 is now 128.
 */
void build_gamma_table(int gamma)
{
    int i, n;
    
    /*
     * value is the current sum.
     * diff is the new term to add to the series.
     */
    long value, diff;
    
    /* Hack - convergence is bad in these cases. */
    gamma_table[0] = 0;
    gamma_table[255] = 255;
    
    for (i = 1; i < 255; i++)
    {
        /* 
         * Initialise the Taylor series
         *
         * value and diff have been scaled by 256
         */
        
        n = 1;
        value = 256 * 256;
        diff = ((long)gamma_helper[i]) * (gamma - 256);
        
        while (diff)
        {
            value += diff;
            n++;
            
            
            /*
             * Use the following identiy to calculate the gamma table.
             * exp(x) = 1 + x + x^2/2 + x^3/(2*3) + x^4/(2*3*4) +...
             *
             * n is the current term number.
             * 
             * The gamma_helper array contains a table of
             * ln(x/256) * 256
             * This is used because a^b = exp(b*ln(a))
             *
             * In this case:
             * a is i / 256
             * b is gamma.
             *
             * Note that everything is scaled by 256 for accuracy,
             * plus another factor of 256 for the final result to
             * be from 0-255. Thus gamma_helper[] * gamma must be
             * divided by 256*256 each itteration, to get back to
             * the original power series.
             */
            diff = (((diff / 256) * gamma_helper[i]) * (gamma - 256)) / (256 * n);
        }
        
        /* 
         * Store the value in the table so that the
         * floating point pow function isn't needed .
         */
        gamma_table[i] = (byte)(((long)(value / 256) * i) / 256);
    }
}

#endif /* SUPPORT_GAMMA */


/*
 * Add a series of keypresses to the "queue".
 *
 * Return any errors generated by Term_keypress() in doing so, or SUCCESS
 * if there are none.
 *
 * Catch the "out of space" error before anything is printed.
 *
 * NB: The keys added here will be interpreted by any macros or keymaps.
 */
errr type_string(cptr str, uint len)
{
    errr err = 0;
    cptr s;

    term *old = Term;

    /* Paranoia - no string. */
    if (!str) return -1;

    /* Hack - calculate the string length here if none given. */
    if (!len) len = strlen(str);

    /* Activate the main window, as all pastes go there. */
    Term_activate(term_screen);

    for (s = str; s < str+len; s++)
    {
        /* Catch end of string */
        if (*s == '\0') break;

        err = Term_keypress(*s);

        /* Catch errors */
        if (err) break;
    }

    /* Activate the original window. */
    Term_activate(old);

    return err;
}



void roff_to_buf(cptr str, int maxlen, char *tbuf, size_t bufsize)
{
    int read_pt = 0;
    int write_pt = 0;
    int line_len = 0;
    int word_punct = 0;
    char ch[3];
    ch[2] = '\0';

    while (str[read_pt])
    {
        int ch_len = 1;

        /* Prepare one character */
        ch[0] = str[read_pt];
        ch[1] = '\0';
        if (!isprint(ch[0]))
            ch[0] = ' ';

        if (line_len + ch_len > maxlen - 1 || str[read_pt] == '\n')
        {
            int word_len;

            /* return to better wrapping point. */
            /* Space character at the end of the line need not to be printed. */
            word_len = read_pt - word_punct;
            if (ch[0] == ' ' || word_len >= line_len/2)
                read_pt++;
            else
            {
                read_pt = word_punct;
                if (str[word_punct] == ' ')
                    read_pt++;
                write_pt -= word_len;
            }

            tbuf[write_pt++] = '\0';
            line_len = 0;
            word_punct = read_pt;
            continue;
        }
        if (ch[0] == ' ')
            word_punct = read_pt;

        /* Not enough buffer size */
        if ((size_t)(write_pt + 3) >= bufsize) break;

        tbuf[write_pt++] = ch[0];
        line_len++;
        read_pt++;
    }
    tbuf[write_pt] = '\0';
    tbuf[write_pt+1] = '\0';

    return;
}


/*
 * The my_strcpy() function copies up to 'bufsize'-1 characters from 'src'
 * to 'buf' and NUL-terminates the result. The 'buf' and 'src' strings may
 * not overlap.
 *
 * my_strcpy() returns strlen(src). This makes checking for truncation
 * easy. Example: if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
size_t my_strcpy(char *buf, const char *src, size_t bufsize)
{

    size_t len = strlen(src);
    size_t ret = len;

    /* Paranoia */
    if (bufsize == 0) return ret;

    /* Truncate */
    if (len >= bufsize) len = bufsize - 1;

    /* Copy the string and terminate it */
    (void)memcpy(buf, src, len);
    buf[len] = '\0';

    /* Return strlen(src) */
    return ret;

}


/*
 * The my_strcat() tries to append a string to an existing NUL-terminated string.
 * It never writes more characters into the buffer than indicated by 'bufsize' and
 * NUL-terminates the buffer. The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src). This makes checking for
 * truncation easy. Example:
 * if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
size_t my_strcat(char *buf, const char *src, size_t bufsize)
{
    size_t dlen = strlen(buf);

    /* Is there room left in the buffer? */
    if (dlen < bufsize - 1)
    {
        /* Append as much as possible  */
        return (dlen + my_strcpy(buf + dlen, src, bufsize - dlen));
    }
    else
    {
        /* Return without appending */
        return (dlen + strlen(src));
    }
}


/*
 * A copy of ANSI strstr()
 *
 * my_strstr() can handle Kanji strings correctly.
 */
char *my_strstr(const char *haystack, const char *needle)
{
    int i;
    int l1 = strlen(haystack);
    int l2 = strlen(needle);

    if (l1 >= l2)
    {
        for(i = 0; i <= l1 - l2; i++)
        {
            if(!strncmp(haystack + i, needle, l2))
                return (char *)haystack + i;

        }
    }

    return NULL;
}


/*
 * A copy of ANSI strchr()
 *
 * my_strchr() can handle Kanji strings correctly.
 */
char *my_strchr(const char *ptr, char ch)
{
    for ( ; *ptr != '\0'; ptr++)
    {
        if (*ptr == ch) return (char *)ptr;

    }

    return NULL;
}


/*
 * Convert string to lower case
 */
void str_tolower(char *str)
{
    /* Force to be lower case string */
    for (; *str; str++)
    {
        *str = tolower(*str);
    }
}

/**
 * Screw this, I need my Pascal tools...
 * Returns the position of mika in missa, plus 1, so that 0 can mean
 * a negative result.
 */
unsigned int strpos(const char *mika, const char *missa)
{
	char *loppu = strstr(missa, mika);
	if (loppu) return ((loppu - missa) + 1); else return 0;
}

unsigned int chrpos(const char mika, const char *missa)
{
    char ch2[2] = "\0\0";
    ch2[0] = mika;
    return strpos(ch2, missa);
}

/*
 * Clips away everything from paikka to paikka + pituus - 1
 * (this considers the array as starting at 1; it actually starts at 0,
 * so make sure paikka accounts for that)
 */
void string_clip(char *apu, unsigned int paikka, int pituus)
{
	unsigned int i;
	if ((paikka < 1) || (pituus < 1)) return; /* Abort */
	if ((paikka + pituus) >= (strlen(apu) + 1)) /* Speedy treatment */
	{
		apu[paikka - 1] = '\0';
		return;
	}
	for (i = paikka - 1; i < strlen(apu) - pituus; i++)
	{
		apu[i] = apu[i + pituus];
	}
	apu[i++] = '\0';
	return;
}

/*
 * Locates and clips a substring from a string. Checks if consecutive spaces
 * or spaces at the end of the string are created as a result, and removes
 * them if necessary
 */
bool clip_and_locate(char *poista, char *mista)
{
	unsigned int sijainti = strpos(poista, mista);
	unsigned int pituus = strlen(poista);
	unsigned int pituus2 = strlen(mista);
	int loppu, erotus;
	byte tyhjat = 0;
	if ((!sijainti) || (!pituus)) return FALSE;
	loppu = pituus + sijainti - 1;
	erotus = pituus2 - loppu;
	if ((sijainti > 1) && (mista[sijainti - 2] == ' ')) tyhjat++;
	if (((tyhjat) || (sijainti == 1)) && (erotus) && (mista[loppu] == ' ')) tyhjat += (sijainti > 1) ? 1 : 2;
	if ((tyhjat == 1) && (!erotus)) { sijainti--; pituus++; }
	string_clip(mista, sijainti, (tyhjat >= 2) ? pituus + 1 : pituus);
	return TRUE;
}


/*
 * Get a keypress from the user.
 * And interpret special keys as internal code.
 *
 * This function is a Mega-Hack and depend on pref-xxx.prf's.
 * Currently works on Linux(UNIX), Windows, and Macintosh only.
 *
 * CTK: I don't fully understand this routine, but the codes
 * parsed were those in X11. Perhaps Windows uses the same?
 * At any rate, they definitely don't work under curses and
 * sdl which make things like the Mogaminator editor painful
 * to use. I'm trying a fix for sdl ...
 */
typedef struct {
    cptr keyname;
    int keyflag;
} _modifier_t, *_modifier_ptr; 

static _modifier_t _x11_modifiers[] = {
    {"shift-", SKEY_MOD_SHIFT},
    {"control-", SKEY_MOD_CONTROL},
    {NULL, 0},
};

/* sdl: ^_S[[7]] for SHIFT+NUM_PAD7
 *      ^_C[up] for SHIFT+UP (not on the numpad) */
static _modifier_t _sdl_modifiers[] = {
    {"S", SKEY_MOD_SHIFT},
    {"C", SKEY_MOD_CONTROL},
    {NULL, 0},
};

typedef struct {
    bool numpad;
    cptr keyname;
    int keycode;
} _special_key_t, *_special_key_ptr;

static _special_key_t _x11_special_keys[] = {
    {FALSE, "Down]", SKEY_DOWN},
    {FALSE, "Left]", SKEY_LEFT},
    {FALSE, "Right]", SKEY_RIGHT},
    {FALSE, "Up]", SKEY_UP},
    {FALSE, "Page_Up]", SKEY_PGUP},
    {FALSE, "Page_Down]", SKEY_PGDOWN},
    {FALSE, "Home]", SKEY_TOP},
    {FALSE, "End]", SKEY_BOTTOM},
    {TRUE, "KP_Down]", SKEY_DOWN},
    {TRUE, "KP_Left]", SKEY_LEFT},
    {TRUE, "KP_Right]", SKEY_RIGHT},
    {TRUE, "KP_Up]", SKEY_UP},
    {TRUE, "KP_Page_Up]", SKEY_PGUP},
    {TRUE, "KP_Page_Down]", SKEY_PGDOWN},
    {TRUE, "KP_Home]", SKEY_TOP},
    {TRUE, "KP_End]", SKEY_BOTTOM},
    {TRUE, "KP_2]", SKEY_DOWN},
    {TRUE, "KP_4]", SKEY_LEFT},
    {TRUE, "KP_6]", SKEY_RIGHT},
    {TRUE, "KP_8]", SKEY_UP},
    {TRUE, "KP_9]", SKEY_PGUP},
    {TRUE, "KP_3]", SKEY_PGDOWN},
    {TRUE, "KP_7]", SKEY_TOP},
    {TRUE, "KP_1]", SKEY_BOTTOM},
    {FALSE, NULL, 0},
};
static _special_key_t _sdl_special_keys[] = {
    {FALSE, "[down]", SKEY_DOWN},
    {FALSE, "[left]", SKEY_LEFT},
    {FALSE, "[right]", SKEY_RIGHT},
    {FALSE, "[up]", SKEY_UP},
    {FALSE, "[page_up]", SKEY_PGUP},
    {FALSE, "[page_down]", SKEY_PGDOWN},
    {FALSE, "[home]", SKEY_TOP},
    {FALSE, "[end]", SKEY_BOTTOM},
    {TRUE, "[[2]]", SKEY_DOWN},
    {TRUE, "[[4]]", SKEY_LEFT},
    {TRUE, "[[6]]", SKEY_RIGHT},
    {TRUE, "[[8]]", SKEY_UP},
    {TRUE, "[[9]]", SKEY_PGUP},
    {TRUE, "[[3]]", SKEY_PGDOWN},
    {TRUE, "[[7]]", SKEY_TOP},
    {TRUE, "[[1]]", SKEY_BOTTOM},
    {FALSE, NULL, 0},
};
int inkey_special(bool numpad_cursor)
{
    _modifier_ptr    modifiers = _x11_modifiers;
    _special_key_ptr keys = _x11_special_keys;
    cptr             start = "\\[";
    char             buf[1024];
    cptr             str = buf;
    char             key;
    int              skey = 0;
    int              modifier = 0;
    int              i;
    size_t           trig_len;

    if (strcmp(ANGBAND_SYS, "sdl") == 0)
    {
        modifiers = _sdl_modifiers;
        keys = _sdl_special_keys;
        start = "^_";
    }

    /*
     * Forget macro trigger ----
     * It's important if we are already expanding macro action
     */
    inkey_macro_trigger_string[0] = '\0';

    /* Get a keypress */
    key = inkey();

    if (autopick_inkey_hack == 2)
    {
        autopick_inkey_hack = 0;
        return (((int)key) | SKEY_MASK);
    }
    else autopick_inkey_hack = 0;

    /* Examine trigger string */
    trig_len = strlen(inkey_macro_trigger_string);

    /* Already known that no special key */
    if (!trig_len) return (int)((unsigned char)key);

    /*
     * Hack -- Ignore macro defined on ASCII characters.
     */
    if (trig_len == 1 && parse_macro)
    {
        char c = inkey_macro_trigger_string[0];

        /* Cancel macro action on the queue */
        forget_macro_action();

        /* Return the originaly pressed key */
        return (int)((unsigned char)c);
    }

    /* Convert the trigger */
    ascii_to_text(buf, inkey_macro_trigger_string);

    /* Check the prefix "\[" */
    if (prefix(str, start))
    {
        /* Skip "\[" */
        str += strlen(start);

        /* Examine modifier keys */
        while (TRUE)
        {
            for (i = 0; modifiers[i].keyname; i++)
            {
                if (prefix(str, modifiers[i].keyname))
                {
                    /* Get modifier key flag */
                    str += strlen(modifiers[i].keyname);
                    modifier |= modifiers[i].keyflag;
                }
            }

            /* No more modifier key found */
            if (!modifiers[i].keyname) break;
        }

        /* numpad_as_cursorkey option force numpad keys to input numbers */
        if (!numpad_as_cursorkey) numpad_cursor = FALSE;

        /* Get a special key code */
        for (i = 0; keys[i].keyname; i++)
        {
            if ((!keys[i].numpad || numpad_cursor) &&
                streq(str, keys[i].keyname))
            {
                skey = keys[i].keycode;
                break;
            }
        }

        /* A special key found */
        if (skey)
        {
            /* Cancel macro action on the queue */
            forget_macro_action();

            /* Return special key code and modifier flags */
            return (skey | modifier);
        }
    }

    /* No special key found? */

    /* Don't bother with this trigger no more */
    inkey_macro_trigger_string[0] = '\0';

    /* Return normal keycode */
    return (int)((unsigned char)key);
}

/* For use with randints and other varying input
 * MIN and MAX cannot handle unpredictable input properly */
int pienempi(int a, int b)
{
    return MIN(a, b);
}
int isompi(int a, int b)
{
    return MAX(a, b);
}

int count_bits(u32b x)
{
    int n = 0;

    if (x) do
    {
        n++;
    }
    while (0 != (x = x&(x-1)));

    return (n);
}

