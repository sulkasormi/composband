/* File: main-win.c */

/*
 * Copyright (c) 1997 Ben Harrison, Skirmantas Kligys, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/*
 * This file helps Angband work with Windows computers.
 *
 * To use this file, use an appropriate "Makefile" or "Project File",
 * make sure that "WINDOWS" and/or "WIN32" are defined somewhere, and
 * make sure to obtain various extra files as described below.
 *
 * The official compilation uses the CodeWarrior Pro compiler, which
 * includes a special project file and precompilable header file.
 *
 *
 * See also "main-dos.c" and "main-ibm.c".
 *
 *
 * The "lib/user/pref-win.prf" file contains keymaps, macro definitions,
 * and/or color redefinitions.
 *
 * The "lib/user/font-win.prf" contains attr/char mappings for use with the
 * normal "lib/xtra/font/*.fon" font files.
 *
 * The "lib/user/graf-win.prf" contains attr/char mappings for use with the
 * special "lib/xtra/graf/*.bmp" bitmap files, which are activated by a menu
 * item.
 *
 *
 * Compiling this file, and using the resulting executable, requires
 * several extra files not distributed with the standard Angband code.
 * If "USE_GRAPHICS" is defined, then "readdib.h" and "readdib.c" must
 * be placed into "src/", and the "8X8.BMP" bitmap file must be placed
 * into "lib/xtra/graf".  In any case, some "*.fon" files (including
 * "8X13.FON" if nothing else) must be placed into "lib/xtra/font/".
 * If "USE_SOUND" is defined, then some special library (for example,
 * "winmm.lib") may need to be linked in, and desired "*.WAV" sound
 * files must be placed into "lib/xtra/sound/".  All of these extra
 * files can be found in the "ext-win" archive.
 *
 *
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things,
 * if only for efficiency.  XXX XXX XXX
 *
 * A simpler method is needed for selecting the "tile size" for windows.
 * XXX XXX XXX
 *
 * The various "warning" messages assume the existance of the "screen.w"
 * window, I think, and only a few calls actually check for its existance,
 * this may be okay since "NULL" means "on top of all windows". (?)  The
 * user must never be allowed to "hide" the main window, or the "menubar"
 * will disappear.  XXX XXX XXX
 *
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.  These files *may* be available
 * at the ftp site somewhere, but I have not seen them.  XXX XXX XXX
 *
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Original code by Skirmantas Kligys (kligys@scf.usc.edu).
 *
 * Additional code by Ross E Becker (beckerr@cis.ohio-state.edu),
 * and Chris R. Martin (crm7479@tam2000.tamu.edu).
 */

#include "angband.h"
#undef BOOL /* <== this really messes up windows.h 
               h-type.h:107 #define BOOL(E) !!(E) 
               windef.h:153 typedef int BOOL; */

#ifdef WINDOWS

#ifdef MSVC
/*1>C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\windef.h(230): warning C4255: 'FARPROC' : no function prototype given: converting '()' to '(void)'*/
#pragma warning (disable:4255) 
#endif 

#include <windows.h>
#include <direct.h>

/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif


/*
 * Hack -- allow use of "screen saver" mode
 */
#define USE_SAVER

/*
 * Menu constants -- see "ANGBAND.RC"
 */

#define IDM_FILE_NEW            100
#define IDM_FILE_OPEN            101
#define IDM_FILE_SAVE            110
#define IDM_FILE_MOVIE            121
#define IDM_FILE_EXIT            130

#define IDM_WINDOW_VIS_0        200
#define IDM_WINDOW_VIS_1        201
#define IDM_WINDOW_VIS_2        202
#define IDM_WINDOW_VIS_3        203
#define IDM_WINDOW_VIS_4        204
#define IDM_WINDOW_VIS_5        205
#define IDM_WINDOW_VIS_6        206
#define IDM_WINDOW_VIS_7        207

#define IDM_WINDOW_FONT_0        210
#define IDM_WINDOW_FONT_1        211
#define IDM_WINDOW_FONT_2        212
#define IDM_WINDOW_FONT_3        213
#define IDM_WINDOW_FONT_4        214
#define IDM_WINDOW_FONT_5        215
#define IDM_WINDOW_FONT_6        216
#define IDM_WINDOW_FONT_7        217

#define IDM_WINDOW_POS_0        220
#define IDM_WINDOW_POS_1        221
#define IDM_WINDOW_POS_2        222
#define IDM_WINDOW_POS_3        223
#define IDM_WINDOW_POS_4        224
#define IDM_WINDOW_POS_5        225
#define IDM_WINDOW_POS_6        226
#define IDM_WINDOW_POS_7        227

#define IDM_WINDOW_BIZ_0        230
#define IDM_WINDOW_BIZ_1        231
#define IDM_WINDOW_BIZ_2        232
#define IDM_WINDOW_BIZ_3        233
#define IDM_WINDOW_BIZ_4        234
#define IDM_WINDOW_BIZ_5        235
#define IDM_WINDOW_BIZ_6        236
#define IDM_WINDOW_BIZ_7        237

#define IDM_WINDOW_I_WID_0        240
#define IDM_WINDOW_I_WID_1        241
#define IDM_WINDOW_I_WID_2        242
#define IDM_WINDOW_I_WID_3        243
#define IDM_WINDOW_I_WID_4        244
#define IDM_WINDOW_I_WID_5        245
#define IDM_WINDOW_I_WID_6        246
#define IDM_WINDOW_I_WID_7        247

#define IDM_WINDOW_D_WID_0        250
#define IDM_WINDOW_D_WID_1        251
#define IDM_WINDOW_D_WID_2        252
#define IDM_WINDOW_D_WID_3        253
#define IDM_WINDOW_D_WID_4        254
#define IDM_WINDOW_D_WID_5        255
#define IDM_WINDOW_D_WID_6        256
#define IDM_WINDOW_D_WID_7        257

#define IDM_WINDOW_I_HGT_0        260
#define IDM_WINDOW_I_HGT_1        261
#define IDM_WINDOW_I_HGT_2        262
#define IDM_WINDOW_I_HGT_3        263
#define IDM_WINDOW_I_HGT_4        264
#define IDM_WINDOW_I_HGT_5        265
#define IDM_WINDOW_I_HGT_6        266
#define IDM_WINDOW_I_HGT_7        267

#define IDM_WINDOW_D_HGT_0        270
#define IDM_WINDOW_D_HGT_1        271
#define IDM_WINDOW_D_HGT_2        272
#define IDM_WINDOW_D_HGT_3        273
#define IDM_WINDOW_D_HGT_4        274
#define IDM_WINDOW_D_HGT_5        275
#define IDM_WINDOW_D_HGT_6        276
#define IDM_WINDOW_D_HGT_7        277

#define IDM_OPTIONS_NO_GRAPHICS     400
#define IDM_OPTIONS_OLD_GRAPHICS 401
#define IDM_OPTIONS_NEW_GRAPHICS 402
#define IDM_OPTIONS_BIGTILE        409
#define IDM_OPTIONS_SOUND        410
#define IDM_OPTIONS_SAVER        420
#define IDM_OPTIONS_MAP            430

#define IDM_DUMP_SCREEN_HTML    450

#define IDM_HELP_CONTENTS       901



/*
 * Exclude parts of WINDOWS.H that are not needed
 */
#define NOCOMM            /* Comm driver APIs and definitions */
#define NOLOGERROR        /* LogError() and related definitions */
#define NOPROFILER        /* Profiler APIs */
#define NOLFILEIO         /* _l* file I/O routines */
#define NOOPENFILE        /* OpenFile and related definitions */
#define NORESOURCE        /* Resource management */
#define NOATOM            /* Atom management */
#define NOLANGUAGE        /* Character test routines */
#define NOLSTRING         /* lstr* string management routines */
#define NODBCS            /* Double-byte character set routines */
#define NOKEYBOARDINFO    /* Keyboard driver routines */
#define NOCOLOR           /* COLOR_* color values */
#define NODRAWTEXT        /* DrawText() and related definitions */
#define NOSCALABLEFONT    /* Truetype scalable font support */
#define NOMETAFILE        /* Metafile support */
#define NOSYSTEMPARAMSINFO /* SystemParametersInfo() and SPI_* definitions */
#define NODEFERWINDOWPOS  /* DeferWindowPos and related definitions */
#define NOKEYSTATES       /* MK_* message key state flags */
#define NOWH              /* SetWindowsHook and related WH_* definitions */
#define NOCLIPBOARD       /* Clipboard APIs and definitions */
#define NOICONS           /* IDI_* icon IDs */
#define NOMDI             /* MDI support */
#define NOHELP            /* Help support */

/* Not defined since it breaks Borland C++ 5.5 */
/* #define NOCTLMGR */    /* Control management and controls */

/*
 * Exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */

/*
 * Include the "windows" support file
 */
#include <windows.h>

/*
 * Exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOTIMER        /* Timer support */
#define MMNOJOY          /* Joystick support */
#define MMNOMCI          /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

/*
 * Include some more files. Note: the Cygnus Cygwin compiler
 * doesn't use mmsystem.h instead it includes the winmm library
 * which performs a similar function.
 */
#include <mmsystem.h>
#include <commdlg.h>

/*
 * HTML-Help requires htmlhelp.h and htmlhelp.lib from Microsoft's
 * HTML Workshop < http://msdn.microsoft.com/workshop/author/htmlhelp/ >.
 */
/* #define HTML_HELP */

#ifdef HTML_HELP
#include <htmlhelp.h>
#endif /* HTML_HELP */

/*
 * Include the support for loading bitmaps
 */
#ifdef USE_GRAPHICS
# include "readdib.h"
#endif

/*
 * Hack -- Fake declarations from "dos.h" XXX XXX XXX
 */
#ifdef WIN32
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#else /* WIN32 */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#endif /* WIN32 */

/*
 * Silliness in WIN32 drawing routine
 */
#ifdef WIN32
# define MoveTo(H,X,Y) MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

/*
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif

/*
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK    0x00
#define VID_BLUE    0x01
#define VID_GREEN    0x02
#define VID_CYAN    0x03
#define VID_RED        0x04
#define VID_MAGENTA    0x05
#define VID_YELLOW    0x06
#define VID_WHITE    0x07

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT    0x08

/*
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK    0x00
#define VUD_BLUE    0x10
#define VUD_GREEN    0x20
#define VUD_CYAN    0x30
#define VUD_RED        0x40
#define VUD_MAGENTA    0x50
#define VUD_YELLOW    0x60
#define VUD_WHITE    0x70

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT    0x80


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 *
 * Note the use of "font_want" for the names of the font file requested by
 * the user, and the use of "font_file" for the currently active font file.
 *
 * The "font_file" is uppercased, and takes the form "8X13.FON", while
 * "font_want" can be in almost any form as long as it could be construed
 * as attempting to represent the name of a font.
 */
struct _term_data
{
    term t;

    cptr s;

    HWND w;
    HDC  hDC;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    RECT updateRect;

    DWORD dwStyle;
    DWORD dwExStyle;

    uint keys;

    uint rows;    /* int -> uint */
    uint cols;

    uint pos_x;
    uint pos_y;
    uint size_wid;
    uint size_hgt;
    uint size_ow1;
    uint size_oh1;
    uint size_ow2;
    uint size_oh2;

    bool size_hack;

    bool xtra_hack;

    bool visible;

    bool bizarre;

    cptr font_want;

    cptr font_file;

    HFONT font_id;

    uint font_wid;
    uint font_hgt;

    uint tile_wid;
    uint tile_hgt;

    uint map_tile_wid;
    uint map_tile_hgt;

    bool map_active;
    LOGFONT lf;

    bool posfix;
};


/*
 * Maximum number of windows XXX XXX XXX
 */
#define MAX_TERM_DATA 8

/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];

/*
 * Hack -- global "window creation" pointer
 */
static term_data *my_td;

/*
 * Remember normal size of main window when maxmized
 */
POINT normsize;

/*
 * was main window maximized on previous playing
 */
bool win_maximized = FALSE;

/*
 * game in progress
 */
bool game_in_progress = FALSE;

/*
 * screen paletted, i.e. 256 colors
 */
bool paletted = FALSE;

/*
 * 16 colors screen, don't use RGB()
 */
bool colors16 = FALSE;

/*
 * Saved instance handle
 */
static HINSTANCE hInstance;

/*
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/*
 * An icon
 */
static HICON hIcon;

/*
 * A palette
 */
static HPALETTE hPal;


#ifdef USE_SAVER

/*
 * The screen saver window
 */
static HWND hwndSaver;

#endif /* USE_SAVER */


#ifdef USE_GRAPHICS

/*
 * Flag set once "graphics" has been initialized
 */
static bool can_use_graphics = FALSE;

/*
 * The global bitmap
 */
typedef struct {
    DIBINIT tiles;
    HDC     hdcTiles;
    DIBINIT mask;
    HDC     hdcMask;
    int        width;
    int        height;
} graphics_t;

static graphics_t _graphics = {0};

#endif /* USE_GRAPHICS */


#ifdef USE_SOUND

/*
 * Flag set once "sound" has been initialized
 */
static bool can_use_sound = FALSE;

#define SAMPLE_MAX 8

/*
 * An array of sound file names
 */
static cptr sound_file[SOUND_MAX][SAMPLE_MAX];

#endif /* USE_SOUND */


/*
 * Full path to ANGBAND.INI
 */
static cptr ini_file = NULL;

/*
 * Name of application
 */
static cptr AppName = "ANGBAND";

/*
 * Name of sub-window type
 */
static cptr AngList = "AngList";

/*
 * Directory names
 */
static cptr ANGBAND_DIR_XTRA_GRAF;
static cptr ANGBAND_DIR_XTRA_SOUND;
static cptr ANGBAND_DIR_XTRA_HELP;
#ifdef USE_MUSIC
static cptr ANGBAND_DIR_XTRA_MUSIC;
#endif


/*
 * The "complex" color values
 */
static COLORREF win_clr[256];


/*
 * Flag for macro trigger with dump ASCII
 */
static bool Term_no_press = FALSE;

/*
 * Copy and paste
 */
static bool mouse_down = FALSE;
static bool paint_rect = FALSE;
static int mousex = 0, mousey = 0;
static int oldx, oldy;


/*
 * The "simple" color values
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static BYTE win_pal[256] =
{
    VID_BLACK,                    /* Dark */
    VID_WHITE,                    /* White */
    VID_CYAN,                    /* Slate XXX */
    VID_RED | VID_BRIGHT,        /* Orange XXX */
    VID_RED,                    /* Red */
    VID_GREEN,                    /* Green */
    VID_BLUE,                    /* Blue */
    VID_YELLOW,                    /* Umber XXX */
    VID_BLACK | VID_BRIGHT,        /* Light Dark */
    VID_CYAN | VID_BRIGHT,        /* Light Slate XXX */
    VID_MAGENTA,                /* Violet XXX */
    VID_YELLOW | VID_BRIGHT,    /* Yellow */
    VID_MAGENTA | VID_BRIGHT,    /* Light Red XXX */
    VID_GREEN | VID_BRIGHT,        /* Light Green */
    VID_BLUE | VID_BRIGHT,        /* Light Blue */
    VID_YELLOW,                    /* Light Umber XXX */
    VID_GREEN,                  /* Int. Green */
    VID_MAGENTA | VID_BRIGHT,   /* Pink */
    VID_BLUE | VID_BRIGHT,      /* Int. Blue */
    VID_MAGENTA,                /* Purple */
    VID_GREEN | VID_BRIGHT,     /* Blue-Green */
    VID_BLUE | VID_BRIGHT,      /* Sky-Blue */
    VID_YELLOW,                 /* Mud */
    VID_YELLOW,                 /* Dark Yellow */
    VID_CYAN,                   /* Turquoise */
    VID_RED | VID_BRIGHT,       /* Light Orange */
    VID_MAGENTA | VID_BRIGHT,   /* Lilac */
    VID_BLUE,                   /* Dark Purple */
    VID_BLUE,                   /* Dark Sky-Blue */
    VID_WHITE,                  /* Pale Blue */
    VID_RED,                    /* Dark Pink */
    VID_RED,                    /* Chestnut */
};


/*
 * Hack -- define which keys are "special"
 */
static bool special_key[256];
static bool ignore_key[256];

/*
 * Hack -- initialization list for "special_key"
 */
static byte special_key_list[] = {
    VK_CLEAR, VK_PAUSE, VK_CAPITAL,
    VK_KANA, VK_JUNJA, VK_FINAL, VK_KANJI,
    VK_CONVERT, VK_NONCONVERT, VK_ACCEPT, VK_MODECHANGE,
    VK_PRIOR, VK_NEXT, VK_END, VK_HOME,
    VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,
    VK_SELECT, VK_PRINT, VK_EXECUTE, VK_SNAPSHOT,
    VK_INSERT, VK_DELETE, VK_HELP, VK_APPS,
    VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3,
    VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7,
    VK_NUMPAD8, VK_NUMPAD9, VK_MULTIPLY, VK_ADD,
    VK_SEPARATOR, VK_SUBTRACT, VK_DECIMAL, VK_DIVIDE,
    VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
    VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
    VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18,
    VK_F19,VK_F20, VK_F21, VK_F22, VK_F23, VK_F24,
    VK_NUMLOCK, VK_SCROLL, VK_ATTN, VK_CRSEL,
    VK_EXSEL, VK_EREOF, VK_PLAY, VK_ZOOM,
    VK_NONAME, VK_PA1,
    0    /* End of List */
};

static byte ignore_key_list[] = {
    VK_ESCAPE, VK_TAB, VK_SPACE,
    'F', 'W', 'O', /*'H',*/ /* these are menu characters.*/
    VK_SHIFT, VK_CONTROL, VK_MENU, VK_LWIN, VK_RWIN,
    VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL,
    VK_LMENU, VK_RMENU,
    0    /* End of List */
};



/*
 * Check for existance of a file
 */
static bool check_file(cptr s)
{
    char path[1024];

#ifdef WIN32

    DWORD attrib;

#else /* WIN32 */

    unsigned int attrib;

#endif /* WIN32 */

    /* Copy it */
    strcpy(path, s);

#ifdef WIN32

    /* Examine */
    attrib = GetFileAttributes(path);

    /* Require valid filename */
    if (attrib == INVALID_FILE_NAME) return (FALSE);

    /* Prohibit directory */
    if (attrib & FILE_ATTRIBUTE_DIRECTORY) return (FALSE);

#else /* WIN32 */

    /* Examine and verify */
    if (_dos_getfileattr(path, &attrib)) return (FALSE);

    /* Prohibit something */
    if (attrib & FA_LABEL) return (FALSE);

    /* Prohibit directory */
    if (attrib & FA_DIREC) return (FALSE);

#endif /* WIN32 */

    /* Success */
    return (TRUE);
}


/*
 * Check for existance of a directory
 */
static bool check_dir(cptr s)
{
    int i;

    char path[1024];

#ifdef WIN32

    DWORD attrib;

#else /* WIN32 */

    unsigned int attrib;

#endif /* WIN32 */

    /* Copy it */
    strcpy(path, s);

    /* Check length */
    i = strlen(path);

    /* Remove trailing backslash */
    if (i && (path[i-1] == '\\')) path[--i] = '\0';

#ifdef WIN32

    /* Examine */
    attrib = GetFileAttributes(path);

    /* Require valid filename */
    if (attrib == INVALID_FILE_NAME) return (FALSE);

    /* Require directory */
    if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return (FALSE);

#else /* WIN32 */

    /* Examine and verify */
    if (_dos_getfileattr(path, &attrib)) return (FALSE);

    /* Prohibit something */
    if (attrib & FA_LABEL) return (FALSE);

    /* Require directory */
    if (!(attrib & FA_DIREC)) return (FALSE);

#endif /* WIN32 */

    /* Success */
    return (TRUE);
}


/*
 * Validate a file
 */
static void validate_file(cptr s)
{
    /* Verify or fail */
    if (!check_file(s))
    {
        char buf[1024];
        path_build(buf, sizeof(buf), ANGBAND_DIR_SAVE, s);
        if (!check_file(buf))
            quit_fmt("Cannot find required file:\n%s", buf);
    }
}


/*
 * Validate a directory
 */
static void validate_dir(cptr s, bool vital)
{
    /* Verify or fail */
    if (!check_dir(s))
    {
        /* This directory contains needed data */
        if (vital)
        {
            quit_fmt("Cannot find required directory:\n%s", s);

        }
        /* Attempt to create this directory */
        else if (_mkdir(s))
        {
            quit_fmt("Unable to create directory:\n%s", s);
        }
    }
}

static void term_init_double_buffer(term_data *td)
{
    HDC hdc;

    if (!td->w) return;

    if (td->hDC)
    {
        SelectObject(td->hDC, td->hOldBitmap);
        DeleteObject(td->hBitmap);
        DeleteDC(td->hDC);
    }

    hdc = GetDC(td->w);

    td->hDC = CreateCompatibleDC(hdc);        
    td->hBitmap = CreateCompatibleBitmap(
        hdc, 
        td->size_wid,
        td->size_hgt
    );
    td->hOldBitmap = SelectObject(td->hDC, td->hBitmap);

    ReleaseDC(td->w, hdc);
}

/*
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
    RECT rc;

    int wid, hgt;

    /* Paranoia */
    if (td->cols < 1) td->cols = 1;
    if (td->rows < 1) td->rows = 1;

    /* Window sizes */
    wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
    hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

    /* Fake window size */
    rc.left = 0;
    rc.right = rc.left + wid;
    rc.top = 0;
    rc.bottom = rc.top + hgt;

    /* XXX XXX XXX */
    /* rc.right += 1; */
    /* rc.bottom += 1; */

    /* Adjust */
    AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

    /* Total size */
    td->size_wid = rc.right - rc.left;
    td->size_hgt = rc.bottom - rc.top;

    /* See CreateWindowEx */
    if (!td->w) return;
    term_init_double_buffer(td);

    /* Extract actual location */
    GetWindowRect(td->w, &rc);

    /* Save the location */
    td->pos_x = rc.left;
    td->pos_y = rc.top;
}


/*
 * Write the "prefs" for a single term
 */
static void save_prefs_aux(int i)
{
    term_data *td = &data[i];
    char sec_name[128];
    char buf[1024];

    RECT rc;
    WINDOWPLACEMENT lpwndpl;

    /* Paranoia */
    if (!td->w) return;

    /* Make section name */
    sprintf(sec_name, "Term-%d", i);

    /* Visible */
    if (i > 0)
    {
        strcpy(buf, td->visible ? "1" : "0");
        WritePrivateProfileString(sec_name, "Visible", buf, ini_file);
    }

    /* Font */
    strcpy(buf, td->lf.lfFaceName[0]!='\0' ? td->lf.lfFaceName : "Courier");

    WritePrivateProfileString(sec_name, "Font", buf, ini_file);

    wsprintf(buf, "%d", td->lf.lfWidth);
    WritePrivateProfileString(sec_name, "FontWid", buf, ini_file);
    wsprintf(buf, "%d", td->lf.lfHeight);
    WritePrivateProfileString(sec_name, "FontHgt", buf, ini_file);
    wsprintf(buf, "%d", td->lf.lfWeight);
    WritePrivateProfileString(sec_name, "FontWgt", buf, ini_file);

    /* Bizarre */
    strcpy(buf, td->bizarre ? "1" : "0");
    WritePrivateProfileString(sec_name, "Bizarre", buf, ini_file);

    /* Tile size (x) */
    wsprintf(buf, "%d", td->tile_wid);
    WritePrivateProfileString(sec_name, "TileWid", buf, ini_file);

    /* Tile size (y) */
    wsprintf(buf, "%d", td->tile_hgt);
    WritePrivateProfileString(sec_name, "TileHgt", buf, ini_file);

    /* Get window placement and dimensions */
    lpwndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(td->w, &lpwndpl);

    /* Acquire position in *normal* mode (not minimized) */
    rc = lpwndpl.rcNormalPosition;

    /* Window size (x) */
    if (i == 0) wsprintf(buf, "%d", normsize.x);
    else wsprintf(buf, "%d", td->cols);
    WritePrivateProfileString(sec_name, "NumCols", buf, ini_file);

    /* Window size (y) */
    if (i == 0) wsprintf(buf, "%d", normsize.y);
    else wsprintf(buf, "%d", td->rows);
    WritePrivateProfileString(sec_name, "NumRows", buf, ini_file);

    /* Maxmized (only main window) */
    if (i == 0)
    {
        strcpy(buf, IsZoomed(td->w) ? "1" : "0");
        WritePrivateProfileString(sec_name, "Maximized", buf, ini_file);
    }

    /* Acquire position */
    GetWindowRect(td->w, &rc);

    /* Window position (x) */
    wsprintf(buf, "%d", rc.left);
    WritePrivateProfileString(sec_name, "PositionX", buf, ini_file);

    /* Window position (y) */
    wsprintf(buf, "%d", rc.top);
    WritePrivateProfileString(sec_name, "PositionY", buf, ini_file);

    /* Window Z position */
    if (i > 0)
    {
        strcpy(buf, td->posfix ? "1" : "0");
        WritePrivateProfileString(sec_name, "PositionFix", buf, ini_file);
    }
}


/*
 * Write the "prefs"
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
    int i;

    char buf[128];

    /* Save the "arg_graphics" flag */
    sprintf(buf, "%d", arg_graphics);
    WritePrivateProfileString("Angband", "Graphics", buf, ini_file);

    /* Save the "arg_bigtile" flag */
    strcpy(buf, arg_bigtile ? "1" : "0");
    WritePrivateProfileString("Angband", "Bigtile", buf, ini_file);

    /* Save the "arg_sound" flag */
    strcpy(buf, arg_sound ? "1" : "0");
    WritePrivateProfileString("Angband", "Sound", buf, ini_file);

    wsprintf(buf, "%d", object_list_width);
    WritePrivateProfileString("Angband", "ObjListWidth", buf, ini_file);
    wsprintf(buf, "%d", monster_list_width);
    WritePrivateProfileString("Angband", "MonListWidth", buf, ini_file);

    /* Save window prefs */
    for (i = 0; i < MAX_TERM_DATA; ++i)
    {
        save_prefs_aux(i);
    }
}


/*
 * Load the "prefs" for a single term
 */
static void load_prefs_aux(int i)
{
    term_data *td = &data[i];
    char sec_name[128];
    char tmp[1024];

    int wid, hgt;

    /* Make section name */
    sprintf(sec_name, "Term-%d", i);

    /* Make section name */
    sprintf(sec_name, "Term-%d", i);

    /* Visible */
    if (i > 0)
    {
        td->visible = (GetPrivateProfileInt(sec_name, "Visible", td->visible, ini_file) != 0);
    }

    /* Desired font, with default */
    GetPrivateProfileString(sec_name, "Font", "Courier", tmp, 127, ini_file);


    /* Bizarre */
    td->bizarre = (GetPrivateProfileInt(sec_name, "Bizarre", td->bizarre, ini_file) != 0);

    /* Analyze font, save desired font name */
    td->font_want = z_string_make(tmp);
    hgt = 15; wid = 0;
    td->lf.lfWidth  = GetPrivateProfileInt(sec_name, "FontWid", wid, ini_file);
    td->lf.lfHeight = GetPrivateProfileInt(sec_name, "FontHgt", hgt, ini_file);
    td->lf.lfWeight = GetPrivateProfileInt(sec_name, "FontWgt", 0, ini_file);


    /* Tile size */
    td->tile_wid = GetPrivateProfileInt(sec_name, "TileWid", td->lf.lfWidth, ini_file);
    td->tile_hgt = GetPrivateProfileInt(sec_name, "TileHgt", td->lf.lfHeight, ini_file);


    /* Window size */
    td->cols = GetPrivateProfileInt(sec_name, "NumCols", td->cols, ini_file);
    td->rows = GetPrivateProfileInt(sec_name, "NumRows", td->rows, ini_file);
    normsize.x = td->cols; normsize.y = td->rows;

    /* Window size */
    if (i == 0)
    {
        win_maximized = GetPrivateProfileInt(sec_name, "Maximized", win_maximized, ini_file);
    }

    /* Window position */
    td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", td->pos_x, ini_file);
    td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", td->pos_y, ini_file);

    if (i == 0)
    {
    }

    /* Window Z position */
    if (i > 0)
    {
        td->posfix = GetPrivateProfileInt(sec_name, "PositionFix", td->posfix, ini_file);
    }
}


/*
 * Load the "prefs"
 */
static void load_prefs(void)
{
    int i;

    /* Extract the "arg_graphics" flag */
    arg_graphics = GetPrivateProfileInt("Angband", "Graphics", GRAPHICS_NONE, ini_file);

    /* Extract the "arg_bigtile" flag */
    arg_bigtile = GetPrivateProfileInt("Angband", "Bigtile", FALSE, ini_file);
    use_bigtile = arg_bigtile;

    /* Extract the "arg_sound" flag */
    arg_sound = (GetPrivateProfileInt("Angband", "Sound", 0, ini_file) != 0);

    object_list_width = MAX(24, GetPrivateProfileInt("Angband", "ObjListWidth", object_list_width, ini_file));
    monster_list_width = MAX(24, GetPrivateProfileInt("Angband", "MonListWidth", monster_list_width, ini_file));

    /* Load window prefs */
    for (i = 0; i < MAX_TERM_DATA; ++i)
    {
        load_prefs_aux(i);
    }
}

#ifdef USE_SOUND

/*
 * XXX XXX XXX - Taken from files.c.
 *
 * Extract "tokens" from a buffer
 *
 * This function uses "whitespace" as delimiters, and treats any amount of
 * whitespace as a single delimiter.  We will never return any empty tokens.
 * When given an empty buffer, or a buffer containing only "whitespace", we
 * will return no tokens.  We will never extract more than "num" tokens.
 *
 * By running a token through the "text_to_ascii()" function, you can allow
 * that token to include (encoded) whitespace, using "\s" to encode spaces.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 */
static s16b tokenize_whitespace(char *buf, s16b num, char **tokens)
{
    int k = 0;

    char *s = buf;


    /* Process */
    while (k < num)
    {
        char *t;

        /* Skip leading whitespace */
        for ( ; *s && isspace(*s); ++s) /* loop */;

        /* All done */
        if (!*s) break;

        /* Find next whitespace, if any */
        for (t = s; *t && !isspace(*t); ++t) /* loop */;

        /* Nuke and advance (if necessary) */
        if (*t) *t++ = '\0';

        /* Save the token */
        tokens[k++] = s;

        /* Advance */
        s = t;
    }

    /* Count */
    return (k);
}

static void load_sound_prefs(void)
{
    int i, j, num;
    char tmp[1024];
    char ini_path[1024];
    char wav_path[1024];
    char *zz[SAMPLE_MAX];

    /* Access the sound.cfg */
    path_build(ini_path, 1024, ANGBAND_DIR_XTRA_SOUND, "sound.cfg");

    for (i = 0; i < SOUND_MAX; i++)
    {
        GetPrivateProfileString("Sound", angband_sound_name[i], "", tmp, 1024, ini_path);

        num = tokenize_whitespace(tmp, SAMPLE_MAX, zz);

        for (j = 0; j < num; j++)
        {
            /* Access the sound */
            path_build(wav_path, 1024, ANGBAND_DIR_XTRA_SOUND, zz[j]);

            /* Save the sound filename, if it exists */
            if (check_file(wav_path))
                sound_file[i][j] = z_string_make(zz[j]);
        }
    }
}

#endif /* USE_SOUND */

/*
 * Create the new global palette based on the bitmap palette
 * (if any), and the standard 16 entry palette derived from
 * "win_clr[]" which is used for the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 *
 * This function returns FALSE if the new palette could not be
 * prepared, which should normally be a fatal error.  XXX XXX
 *
 * Note that only some machines actually use a "palette".
 */
static int new_palette(void)
{
    HPALETTE hBmPal;
    HPALETTE hNewPal;
    HDC hdc;
    int i, nEntries;
    int pLogPalSize;
    int lppeSize;
    LPLOGPALETTE pLogPal;
    LPPALETTEENTRY lppe;

    term_data *td;


    /* This makes no sense */
    if (!paletted) return (TRUE);


    /* No bitmap */
    lppeSize = 0;
    lppe = NULL;
    nEntries = 0;

#ifdef USE_GRAPHICS

    /* Check the bitmap palette */
    hBmPal = _graphics.tiles.hPalette;

    /* Use the bitmap */
    if (hBmPal)
    {
        lppeSize = 256 * sizeof(PALETTEENTRY);
        lppe = (LPPALETTEENTRY)ralloc(lppeSize);
        nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
        if ((nEntries == 0) || (nEntries > 220))
        {
            /* Warn the user */
            plog("Please switch to high- or true-color mode.");


            /* Cleanup */
            rnfree(lppe, lppeSize);

            /* Fail */
            return (FALSE);
        }
    }

#endif /* USE_GRAPHICS */

    /* Size of palette */
    pLogPalSize = sizeof(LOGPALETTE) + (nEntries + MAX_COLOR) * sizeof(PALETTEENTRY);

    /* Allocate palette */
    pLogPal = (LPLOGPALETTE)ralloc(pLogPalSize);

    /* Version */
    pLogPal->palVersion = 0x300;

    /* Make room for bitmap and normal data */
    pLogPal->palNumEntries = nEntries + MAX_COLOR;

    /* Save the bitmap data */
    for (i = 0; i < nEntries; i++)
    {
        pLogPal->palPalEntry[i] = lppe[i];
    }

    /* Save the normal data */
    for (i = 0; i < MAX_COLOR; i++)
    {
        LPPALETTEENTRY p;

        /* Access the entry */
        p = &(pLogPal->palPalEntry[i+nEntries]);

        /* Save the colors */
        p->peRed = GetRValue(win_clr[i]);
        p->peGreen = GetGValue(win_clr[i]);
        p->peBlue = GetBValue(win_clr[i]);

        /* Save the flags */
        p->peFlags = PC_NOCOLLAPSE;
    }

    /* Free something */
    if (lppe) rnfree(lppe, lppeSize);

    /* Create a new palette, or fail */
    hNewPal = CreatePalette(pLogPal);
    if (!hNewPal) quit("Cannot create palette!");


    /* Free the palette */
    rnfree(pLogPal, pLogPalSize);

    /* Main window */
    td = &data[0];

    /* Realize the palette */
    hdc = GetDC(td->w);
    SelectPalette(hdc, hNewPal, 0);
    i = RealizePalette(hdc);
    ReleaseDC(td->w, hdc);
    if (i == 0) quit("Cannot realize palette!");


    /* Sub-windows */
    for (i = 1; i < MAX_TERM_DATA; i++)
    {
        td = &data[i];

        hdc = GetDC(td->w);
        SelectPalette(hdc, hNewPal, 0);
        ReleaseDC(td->w, hdc);
    }

    /* Delete old palette */
    if (hPal) DeleteObject(hPal);

    /* Save new palette */
    hPal = hNewPal;

    /* Success */
    return (TRUE);
}


#ifdef USE_GRAPHICS
/*
 * Initialize graphics
 */
static bool init_graphics(void)
{
    /* Initialize once */
    /* if (can_use_graphics != arg_graphics) */
    {
        char buf[1024];
        int wid, hgt;
        cptr name;

        if (arg_graphics == GRAPHICS_ADAM_BOLT)
        {
            wid = 16;
            hgt = 16;

            name = "16X16.BMP";

            ANGBAND_GRAF = "new";
        }
        else
        {
            wid = 8;
            hgt = 8;

            name = "8X8.BMP";
            ANGBAND_GRAF = "old";
        }

        /* Access the bitmap file */
        path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, name);

        /* Load the bitmap or quit */
        if (!ReadDIB(data[0].w, buf, &_graphics.tiles))
        {
            plog_fmt("Cannot read bitmap file '%s'", name);

            return (FALSE);
        }

        _graphics.hdcTiles = CreateCompatibleDC(NULL);
        SelectObject(_graphics.hdcTiles, _graphics.tiles.hBitmap);

        /* Save the new sizes */
        _graphics.width = wid;
        _graphics.height = hgt;

        if (arg_graphics == GRAPHICS_ADAM_BOLT)
        {
            /* Access the mask file */
            path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "mask.bmp");

            /* Load the bitmap or quit */
            if (!ReadDIB(data[0].w, buf, &_graphics.mask))
            {
                plog_fmt("Cannot read bitmap file '%s'", buf);
                return (FALSE);
            }

            _graphics.hdcMask = CreateCompatibleDC(NULL);
            SelectObject(_graphics.hdcMask, _graphics.mask.hBitmap);
        }

        /* Activate a palette */
        if (!new_palette())
        {
            /* Free bitmap XXX XXX XXX */

            /* Oops */
            plog("Cannot activate palette!");

            return (FALSE);
        }

        /* Graphics available */
        can_use_graphics = arg_graphics;
    }

    /* Result */
    return (can_use_graphics);
}
#endif /* USE_GRAPHICS */


#ifdef USE_SOUND
/*
 * Initialize sound
 */
static bool init_sound(void)
{
    /* Initialize once */
    if (!can_use_sound)
    {
        /* Load the prefs */
        load_sound_prefs();

        /* Sound available */
        can_use_sound = TRUE;
    }

    /* Result */
    return (can_use_sound);
}
#endif /* USE_SOUND */


/*
 * Resize a window
 */
static void term_window_resize(term_data *td)
{
    /* Require window */
    if (!td->w) return;

    /* Resize the window */
    SetWindowPos(td->w, 0, 0, 0,
             td->size_wid, td->size_hgt,
             SWP_NOMOVE | SWP_NOZORDER);

    /* Redraw later */
    InvalidateRect(td->w, NULL, TRUE);
}


/*
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 *
 * Note that the "font name" must be capitalized!!!
 */
static errr term_force_font(term_data *td, cptr path)
{
    int wid, hgt;

    /* Forget the old font (if needed) */
    if (td->font_id) DeleteObject(td->font_id);

    /* Unused */
    (void)path;

    /* Create the font (using the 'base' of the font file name!) */
    td->font_id = CreateFontIndirect(&(td->lf));
    wid = td->lf.lfWidth;
    hgt = td->lf.lfHeight;
    if (!td->font_id) return (1);

    /* Hack -- Unknown size */
    if (!wid || !hgt)
    {
        HDC hdcDesktop;
        HFONT hfOld;
        TEXTMETRIC tm;

        /* all this trouble to get the cell size */
        hdcDesktop = GetDC(HWND_DESKTOP);
        hfOld = SelectObject(hdcDesktop, td->font_id);
        GetTextMetrics(hdcDesktop, &tm);
        SelectObject(hdcDesktop, hfOld);
        ReleaseDC(HWND_DESKTOP, hdcDesktop);

        /* Font size info */
        wid = tm.tmAveCharWidth;
        hgt = tm.tmHeight;
    }

    /* Save the size info */
    td->font_wid = wid;
    td->font_hgt = hgt;

    /* Success */
    return (0);
}



/*
 * Allow the user to change the font for this window.
 */
static void term_change_font(term_data *td)
{
    CHOOSEFONT cf;

    memset(&cf, 0, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.Flags = CF_SCREENFONTS | CF_FIXEDPITCHONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT;
    cf.lpLogFont = &(td->lf);

    if (ChooseFont(&cf))
    {
        /* Force the font */
        term_force_font(td, NULL);

        /* Assume not bizarre */
        td->bizarre = TRUE;

        /* Reset the tile info */
        td->tile_wid = td->font_wid;
        td->tile_hgt = td->font_hgt;

        /* Analyze the font */
        term_getsize(td);

        /* Resize the window */
        term_window_resize(td);
    }
}

/*
 * Allow the user to lock this window.
 */
static void term_window_pos(term_data *td, HWND hWnd)
{
    SetWindowPos(td->w, hWnd, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

static void windows_map(void);

/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
    if (td->map_active)
    {
        /* Redraw the map */
        windows_map();
    }
    else
    {
        /* Activate the term */
        Term_activate(&td->t);

        /* Redraw the contents */
        Term_redraw();

        /* Restore the term */
        Term_activate(term_screen);
    }
}


void Term_inversed_area(HWND hWnd, int x, int y, int w, int h)
{
    HDC hdc;
    HPEN oldPen;
    HBRUSH myBrush, oldBrush;

    term_data *td = (term_data *)GetWindowLong(hWnd, 0);
    int tx = td->size_ow1 + x * td->tile_wid;
    int ty = td->size_oh1 + y * td->tile_hgt;
    int tw = w * td->tile_wid - 1;
    int th = h * td->tile_hgt - 1;

    hdc = GetDC(hWnd);
    myBrush = CreateSolidBrush(RGB(255, 255, 255));
    oldBrush = SelectObject(hdc, myBrush);
    oldPen = SelectObject(hdc, GetStockObject(NULL_PEN) );

    PatBlt(hdc, tx, ty, tw, th, PATINVERT);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
}


/*
 * Interact with the User
 */
static errr Term_user_win(int n)
{
    /* Unused */
    (void)n;

    /* Success */
    return (0);
}


/*
 * React to global changes
 */
static errr Term_xtra_win_react(void)
{
    int i;


    /* Simple color */
    if (colors16)
    {
        /* Save the default colors */
        for (i = 0; i < 256; i++)
        {
            /* Simply accept the desired colors */
            win_pal[i] = angband_color_table[i][0];
        }
    }

    /* Complex color */
    else
    {
        COLORREF code;

        byte rv, gv, bv;

        bool change = FALSE;

        /* Save the default colors */
        for (i = 0; i < 256; i++)
        {
            /* Extract desired values */
            rv = angband_color_table[i][1];
            gv = angband_color_table[i][2];
            bv = angband_color_table[i][3];

            /* Extract a full color code */
            code = PALETTERGB(rv, gv, bv);

            /* Activate changes */
            if (win_clr[i] != code)
            {
                /* Note the change */
                change = TRUE;

                /* Apply the desired color */
                win_clr[i] = code;
            }
        }

        /* Activate the palette if needed */
        if (change) (void)new_palette();
    }


#ifdef USE_SOUND

    /* Handle "arg_sound" */
    if (use_sound != arg_sound)
    {
        /* Initialize (if needed) */
        if (arg_sound && !init_sound())
        {
            /* Warning */
            plog("Cannot initialize sound!");


            /* Cannot enable */
            arg_sound = FALSE;
        }

        /* Change setting */
        use_sound = arg_sound;
    }

#endif


#ifdef USE_GRAPHICS

    /* Handle "arg_graphics" */
    if (use_graphics != arg_graphics)
    {
        /* Initialize (if needed) */
        if (arg_graphics && !init_graphics())
        {
            /* Warning */
            plog("Cannot initialize graphics!");


            /* Cannot enable */
            arg_graphics = GRAPHICS_NONE;
        }

        /* Change setting */
        use_graphics = arg_graphics;

        /* Reset visuals */
        reset_visuals();
    }

#endif /* USE_GRAPHICS */


    /* Clean up windows */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        term *old = Term;

        term_data *td = &data[i];

        /* Update resized windows */
        if ((td->cols != (uint)td->t.wid) || (td->rows != (uint)td->t.hgt))
        {
            /* Activate */
            Term_activate(&td->t);

            /* Hack -- Resize the term */
            Term_resize(td->cols, td->rows);

            /* Redraw the contents */
            Term_redraw();

            /* Restore */
            Term_activate(old);
        }
    }


    /* Success */
    return (0);
}


/*
 * Process at least one event
 */
static errr Term_xtra_win_event(int v)
{
    MSG msg;

    /* Wait for an event */
    if (v)
    {
        /* Block */
        if (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /* Check for an event */
    else
    {
        /* Check */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /* Success */
    return 0;
}


/*
 * Process all pending events
 */
static errr Term_xtra_win_flush(void)
{
    MSG msg;

    /* Process all pending events */
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Success */
    return (0);
}

/* Hack: Remember how much we need to blt later on ... */
static bool _update_rect_is_valid(term_data *td)
{
    return td->updateRect.left >= 0;
}

static void _update_rect_reset(term_data *td)
{
    td->updateRect.left = -1;
}

static void _update_rect_enlarge(term_data *td, RECT *r)
{
    if (!_update_rect_is_valid(td))
    {
        td->updateRect.left = r->left;
        td->updateRect.top = r->top;
        td->updateRect.right = r->right;
        td->updateRect.bottom = r->bottom;
    }
    else
    {
        td->updateRect.left = MIN(td->updateRect.left, r->left);
        td->updateRect.top = MIN(td->updateRect.top, r->top);
        td->updateRect.right = MAX(td->updateRect.right, r->right);
        td->updateRect.bottom = MAX(td->updateRect.bottom, r->bottom);
    }
}

static errr Term_xtra_win_fresh(void)
{
    term_data *td = (term_data*)(Term->data);

    if (_update_rect_is_valid(td))
    {
        HDC dc;
        int x = td->updateRect.left;
        int y = td->updateRect.top;
        int cx = td->updateRect.right - td->updateRect.left;
        int cy = td->updateRect.bottom - td->updateRect.top;

        dc = GetDC(td->w);
        BitBlt(dc, x, y, cx, cy, td->hDC, x, y, SRCCOPY);
        /*BitBlt(dc, 0, 0, td->size_wid, td->size_hgt, td->hDC, 0, 0, SRCCOPY);*/
        ReleaseDC(td->w, dc);

        _update_rect_reset(td);
    }
    return 0;
}

/*
 * Hack -- clear the screen
 *
 * Make this more efficient XXX XXX XXX
 */
static errr Term_xtra_win_clear(void)
{
    term_data *td = (term_data*)(Term->data);

    HDC hdc = td->hDC;
    RECT rc;

    /* Rectangle to erase */
    rc.left = td->size_ow1;
    rc.right = rc.left + td->cols * td->tile_wid;
    rc.top = td->size_oh1;
    rc.bottom = rc.top + td->rows * td->tile_hgt;

    /* Erase it */
    SetBkColor(hdc, RGB(0, 0, 0));
    SelectObject(hdc, td->font_id);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    _update_rect_enlarge(td, &rc);

    /* Success */
    return 0;
}


/*
 * Hack -- make a noise
 */
static errr Term_xtra_win_noise(void)
{
    MessageBeep(MB_ICONASTERISK);
    return (0);
}


/*
 * Hack -- make a sound
 */
static errr Term_xtra_win_sound(int v)
{
#ifdef USE_SOUND
    int i;
    char buf[1024];
#endif /* USE_SOUND */

    /* Sound disabled */
    if (!use_sound) return (1);

    /* Illegal sound */
    if ((v < 0) || (v >= SOUND_MAX)) return (1);

#ifdef USE_SOUND

    /* Count the samples */
    for (i = 0; i < SAMPLE_MAX; i++)
    {
        if (!sound_file[v][i])
            break;
    }

    /* No sample */
    if (i == 0) return (1);

    /* Build the path */
    path_build(buf, 1024, ANGBAND_DIR_XTRA_SOUND, sound_file[v][Rand_simple(i)]);

#ifdef WIN32

    /* Play the sound, catch errors */
    return (PlaySound(buf, 0, SND_FILENAME | SND_ASYNC));

#else /* WIN32 */

    /* Play the sound, catch errors */
    return (sndPlaySound(buf, SND_ASYNC));

#endif /* WIN32 */

#else /* USE_SOUND */

    /* Oops */
    return (1);

#endif /* USE_SOUND */
}


/*
 * Delay for "x" milliseconds
 */
static int Term_xtra_win_delay(int v)
{

#ifdef WIN32

    /* Sleep */
    Sleep(v);

#else /* WIN32 */

    DWORD t;
    MSG msg;

    /* Final count */
    t = GetTickCount() + v;

    /* Wait for it */
    while (GetTickCount() < t)
    {
        /* Handle messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

#endif /* WIN32 */

    /* Success */
    return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
    /* Handle a subset of the legal requests */
    switch (n)
    {
        /* Make a bell sound */
        case TERM_XTRA_NOISE:
        {
            return (Term_xtra_win_noise());
        }

        /* Make a special sound */
        case TERM_XTRA_SOUND:
        {
            return (Term_xtra_win_sound(v));
        }

        /* Process random events */
        case TERM_XTRA_BORED:
        {
            return (Term_xtra_win_event(0));
        }

        /* Process an event */
        case TERM_XTRA_EVENT:
        {
            return (Term_xtra_win_event(v));
        }

        /* Flush all events */
        case TERM_XTRA_FLUSH:
        {
            return (Term_xtra_win_flush());
        }

        case TERM_XTRA_FRESH:
        {
            return (Term_xtra_win_fresh());
        }

        /* Clear the screen */
        case TERM_XTRA_CLEAR:
        {
            return (Term_xtra_win_clear());
        }

        /* React to global changes */
        case TERM_XTRA_REACT:
        {
            return (Term_xtra_win_react());
        }

        /* Delay for some milliseconds */
        case TERM_XTRA_DELAY:
        {
            return (Term_xtra_win_delay(v));
        }
    }

    /* Oops */
    return 1;
}



/*
 * Low level graphics (Assumes valid input).
 *
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_win(int x, int y)
{
    term_data *td = (term_data*)(Term->data);

    RECT rc;
    HDC hdc;

    int tile_wid, tile_hgt;

    if (td->map_active)
    {
        tile_wid = td->map_tile_wid;
        tile_hgt = td->map_tile_hgt;
    }
    else
    {
        tile_wid = td->tile_wid;
        tile_hgt = td->tile_hgt;
    }

    /* Frame the grid */
    rc.left = x * tile_wid + td->size_ow1;
    rc.right = rc.left + tile_wid;
    rc.top = y * tile_hgt + td->size_oh1;
    rc.bottom = rc.top + tile_hgt;

    /* Cursor is done as a yellow "box" */
    hdc = td->hDC;
    FrameRect(hdc, &rc, hbrYellow);
    _update_rect_enlarge(td, &rc);

    /* Success */
    return 0;
}


/*
 * Low level graphics (Assumes valid input).
 *
 * Draw a "big cursor" at (x,y), using a "yellow box".
 */
static errr Term_bigcurs_win(int x, int y)
{
    term_data *td = (term_data*)(Term->data);

    RECT rc;
    HDC hdc;

    int tile_wid, tile_hgt;

    if (td->map_active)
    {
        /* Normal cursor in map window */
        Term_curs_win(x, y);
        return 0;
    }
    else
    {
        tile_wid = td->tile_wid;
        tile_hgt = td->tile_hgt;
    }

    /* Frame the grid */
    rc.left = x * tile_wid + td->size_ow1;
    rc.right = rc.left + 2 * tile_wid;
    rc.top = y * tile_hgt + td->size_oh1;
    rc.bottom = rc.top + tile_hgt;

    /* Cursor is done as a yellow "box" */
    hdc = td->hDC;
    FrameRect(hdc, &rc, hbrYellow);
    _update_rect_enlarge(td, &rc);

    /* Success */
    return 0;
}


/*
 * Low level graphics (Assumes valid input).
 *
 * Erase a "block" of "n" characters starting at (x,y).
 */
static errr Term_wipe_win(int x, int y, int n)
{
    term_data *td = (term_data*)(Term->data);

    HDC hdc;
    RECT rc;

    /* Rectangle to erase in client coords */
    rc.left = x * td->tile_wid + td->size_ow1;
    rc.right = rc.left + n * td->tile_wid;
    rc.top = y * td->tile_hgt + td->size_oh1;
    rc.bottom = rc.top + td->tile_hgt;

    hdc = td->hDC;
    SetBkColor(hdc, RGB(0, 0, 0));
    SelectObject(hdc, td->font_id);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    _update_rect_enlarge(td, &rc);

    /* Success */
    return 0;
}

/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", below.
 *
 * One would think there is a more efficient method for telling a window
 * what color it should be using to draw with, but perhaps simply changing
 * it every time is not too inefficient.  XXX XXX XXX
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
    term_data *td = (term_data*)(Term->data);
    RECT rc;
    HDC hdc;

    static HBITMAP  WALL;
    static HBRUSH   myBrush, oldBrush;
    static HPEN     oldPen;
    static bool init_done = FALSE;

    if (!init_done){
        WALL = LoadBitmap(hInstance, AppName);
        myBrush = CreatePatternBrush(WALL);
        init_done = TRUE;
    }

    /* Total rectangle */
    rc.left = x * td->tile_wid + td->size_ow1;
    rc.right = rc.left + n * td->tile_wid;
    rc.top = y * td->tile_hgt + td->size_oh1;
    rc.bottom = rc.top + td->tile_hgt;

    _update_rect_enlarge(td, &rc);

    /* Acquire DC */
    hdc = td->hDC;

    /* Background color */
    SetBkColor(hdc, RGB(0, 0, 0));

    /* Foreground color */
    if (colors16)
    {
        SetTextColor(hdc, PALETTEINDEX(win_pal[a]));
    }
    else if (paletted)
    {
        SetTextColor(hdc, win_clr[a&COLOR_MASK]);
    }
    else
    {
        SetTextColor(hdc, win_clr[a]);
    }

    /* Use the font */
    SelectObject(hdc, td->font_id);
    
    /* Bizarre size */
    if (td->bizarre ||
        (td->tile_hgt != td->font_hgt) ||
        (td->tile_wid != td->font_wid))
    {
        int i;

        /* Erase complete rectangle */
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
        
        /* New rectangle */
        rc.left += ((td->tile_wid - td->font_wid) / 2);
        rc.right = rc.left + td->font_wid;
        rc.top += ((td->tile_hgt - td->font_hgt) / 2);
        rc.bottom = rc.top + td->font_hgt;

        /* Dump each character */
        for (i = 0; i < n; i++)
        {
            if (*(s+i)==127){
                oldBrush = SelectObject(hdc, myBrush);
                oldPen = SelectObject(hdc, GetStockObject(NULL_PEN) );

                /* Dump the wall */
                Rectangle(hdc, rc.left, rc.top, rc.right+1, rc.bottom+1);
                _update_rect_enlarge(td, &rc);

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);

                /* Advance */
                rc.left += td->tile_wid;
                rc.right += td->tile_wid;
            } else {
                /* Dump the text */
                ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED, &rc,
                       s+i, 1, NULL);
                _update_rect_enlarge(td, &rc);
                /* Advance */
                rc.left += td->tile_wid;
                rc.right += td->tile_wid;
            }

        }
    }

    /* Normal size */
    else
    {
        /* Dump the text */
        ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
               s, n, NULL);
    }

    /* Success */
    return 0;
}

/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw an array of "special" attr/char pairs at the given location.
 *
 * We use the "Term_pict_win()" function for "graphic" data, which are
 * encoded by setting the "high-bits" of both the "attr" and the "char"
 * data.  We use the "attr" to represent the "row" of the main bitmap,
 * and the "char" to represent the "col" of the main bitmap.  The use
 * of this function is induced by the "higher_pict" flag.
 *
 * If "graphics" is not available, we simply "wipe" the given grids.
 */
static errr Term_pict_win(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
    term_data *td = (term_data*)(Term->data);

#ifdef USE_GRAPHICS

    int i;
    int x1, y1, w1, h1;
    int x2, y2, w2, h2, tw2 = 0;
    int x3, y3;
    HDC hdc;
    
    /* Paranoia */
    if (!use_graphics)
    {
        /* Erase the grids */
        return (Term_wipe_win(x, y, n));
    }

    /* Size of bitmap cell */
    w1 = _graphics.width;
    h1 = _graphics.height;

    /* Size of window cell */
    if (td->map_active)
    {
        w2 = td->map_tile_wid;
        h2 = td->map_tile_hgt;
    }
    else
    {
        w2 = td->tile_wid;
        h2 = td->tile_hgt;
        tw2 = w2;

        /* big tile mode */
        if (use_bigtile) tw2 *= 2;
    }

    /* Location of window cell */
    x2 = x * w2 + td->size_ow1;
    y2 = y * h2 + td->size_oh1;

    /* Info */
    hdc = td->hDC;

    /* Draw attr/char pairs */
    for (i = 0; i < n; i++, x2 += w2)
    {
        byte a = ap[i];
        char c = cp[i];

        /* Extract picture */
        int row = (a & 0x7F);
        int col = (c & 0x7F);

        /* Location of bitmap cell */
        x1 = col * w1;
        y1 = row * h1;

        if (arg_graphics == GRAPHICS_ADAM_BOLT)
        {
            x3 = (tcp[i] & 0x7F) * w1;
            y3 = (tap[i] & 0x7F) * h1;

            /* Perfect size */
            if ((w1 == tw2) && (h1 == h2))
            {
                /* Copy the terrain picture from the bitmap to the window */
                BitBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x3, y3, SRCCOPY);

                /* Mask out the tile */
                BitBlt(hdc, x2, y2, tw2, h2, _graphics.hdcMask, x1, y1, SRCAND);

                /* Draw the tile */
                BitBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x1, y1, SRCPAINT);
            }

            /* Need to stretch */
            else
            {
                /* Set the correct mode for stretching the tiles */
                SetStretchBltMode(hdc, COLORONCOLOR);

                /* Copy the terrain picture from the bitmap to the window */
                StretchBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x3, y3, w1, h1, SRCCOPY);

                /* Only draw if terrain and overlay are different */
                if ((x1 != x3) || (y1 != y3))
                {
                    /* Mask out the tile */
                    StretchBlt(hdc, x2, y2, tw2, h2, _graphics.hdcMask, x1, y1, w1, h1, SRCAND);

                    /* Draw the tile */
                    StretchBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x1, y1, w1, h1, SRCPAINT);
                }
            }
        }
        else
        {
            /* Perfect size */
            if ((w1 == tw2) && (h1 == h2))
            {
                /* Copy the picture from the bitmap to the window */
                BitBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x1, y1, SRCCOPY);
            }

            /* Need to stretch */
            else
            {
                /* Set the correct mode for stretching the tiles */
                SetStretchBltMode(hdc, COLORONCOLOR);

                /* Copy the picture from the bitmap to the window */
                StretchBlt(hdc, x2, y2, tw2, h2, _graphics.hdcTiles, x1, y1, w1, h1, SRCCOPY);
            }
        }

        {
            RECT rc;
            rc.left = x2;
            rc.top = y2;
            rc.right = x2 + tw2;
            rc.bottom = y2 + h2;
            _update_rect_enlarge(td, &rc);
        }
    }

#else /* USE_GRAPHICS */

    /* Just erase this grid */
    return (Term_wipe_win(x, y, n));

#endif /* USE_GRAPHICS */

    /* Success */
    return 0;
}


static void windows_map(void)
{
    term_data *td = &data[0];
    byte a;
    char c;
    int x, min_x, max_x;
    int y, min_y, max_y;

    byte ta;
    char tc;

    /* Only in graphics mode */
    if (!use_graphics) return;

    /* Clear screen */
    Term_xtra_win_clear();

    td->map_tile_wid = (td->tile_wid * td->cols) / MAX_WID;
    td->map_tile_hgt = (td->tile_hgt * td->rows) / MAX_HGT;
    td->map_active = TRUE;

    {
        min_x = 0;
        min_y = 0;
        max_x = cur_wid;
        max_y = cur_hgt;
    }

    /* Draw the map */
    for (x = min_x; x < max_x; x++)
    {
        for (y = min_y; y < max_y; y++)
        {
            map_info(y, x, &a, (char*)&c, &ta, (char*)&tc);

            /* Ignore non-graphics */
            if ((a & 0x80) && (c & 0x80))
            {
                Term_pict_win(x - min_x, y - min_y, 1, &a, &c, &ta, &tc);
            }
        }
    }

    /* Hilite the player */
    Term_curs_win(px - min_x, py - min_y);

    /* Wait for a keypress, flush key buffer */
    Term_inkey(&c, TRUE, TRUE);
    Term_flush();

    /* Switch off the map display */
    td->map_active = FALSE;

    /* Restore screen */
    Term_xtra_win_clear();
    Term_redraw();
}


/*** Other routines ***/


/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
    term *t = &td->t;

    /* Initialize the term */
    term_init(t, td->cols, td->rows, td->keys);

    /* Use a "software" cursor */
    t->soft_cursor = TRUE;

    /* Use "Term_pict" for "graphic" data */
    t->higher_pict = TRUE;

    /* Erase with "white space" */
    t->attr_blank = TERM_WHITE;
    t->char_blank = ' ';

    /* Prepare the template hooks */
    t->user_hook = Term_user_win;
    t->xtra_hook = Term_xtra_win;
    t->curs_hook = Term_curs_win;
    t->bigcurs_hook = Term_bigcurs_win;
    t->wipe_hook = Term_wipe_win;
    t->text_hook = Term_text_win;
    t->pict_hook = Term_pict_win;

    /* Remember where we came from */
    t->data = (vptr)(td);
}


/*
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette.  XXX XXX XXX
 */
static void init_windows(void)
{
    int i;

    term_data *td;

    /* Main window */
    td = &data[0];
    WIPE(td, term_data);
    td->s = angband_term_name[0];

    td->keys = 1024;
    td->rows = 27;
    td->cols = 80;
    td->visible = TRUE;
    td->size_ow1 = 2;
    td->size_ow2 = 2;
    td->size_oh1 = 2;
    td->size_oh2 = 2;
    td->pos_x = 7 * 30;
    td->pos_y = 7 * 20;
    td->posfix = FALSE;
    td->bizarre = TRUE;
    /* Sub windows */
    for (i = 1; i < MAX_TERM_DATA; i++)
    {
        td = &data[i];
        WIPE(td, term_data);
        td->s = angband_term_name[i];
        td->keys = 16;
        td->rows = 27;
        td->cols = 80;
        td->visible = FALSE;
        td->size_ow1 = 1;
        td->size_ow2 = 1;
        td->size_oh1 = 1;
        td->size_oh2 = 1;
        td->pos_x = (7 - i) * 30;
        td->pos_y = (7 - i) * 20;
        td->posfix = FALSE;
        td->bizarre = TRUE;
    }


    /* Load prefs */
    load_prefs();


    /* Main window (need these before term_getsize gets called) */
    td = &data[0];
    td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU |
               WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION |
               WS_VISIBLE);
    td->dwExStyle = 0;
    td->visible = TRUE;

    /* Sub windows (need these before term_getsize gets called) */
    for (i = 1; i < MAX_TERM_DATA; i++)
    {
        td = &data[i];
        td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU);
        td->dwExStyle = (WS_EX_TOOLWINDOW);
    }


    /* All windows */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        td = &data[i];

        strncpy(td->lf.lfFaceName, td->font_want, LF_FACESIZE);
        td->lf.lfCharSet = DEFAULT_CHARSET;
        td->lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
        /* Activate the chosen font */
        term_force_font(td, NULL);
        td->tile_wid = td->font_wid;
        td->tile_hgt = td->font_hgt;


        /* Analyze the font */
        term_getsize(td);

        /* Resize the window */
        term_window_resize(td);
    }


    /* Sub windows (reverse order) */
    for (i = MAX_TERM_DATA - 1; i >= 1; --i)
    {
        td = &data[i];

        my_td = td;
        td->w = CreateWindowEx(td->dwExStyle, AngList,
                       td->s, td->dwStyle,
                       td->pos_x, td->pos_y,
                       td->size_wid, td->size_hgt,
                       HWND_DESKTOP, NULL, hInstance, NULL);
        my_td = NULL;
        if (!td->w) quit("Failed to create sub-window");
        term_init_double_buffer(td);

        if (td->visible)
        {
            td->size_hack = TRUE;
            ShowWindow(td->w, SW_SHOW);
            td->size_hack = FALSE;
        }

        term_data_link(td);
        angband_term[i] = &td->t;

        if (td->visible)
        {
            /* Activate the window */
            SetActiveWindow(td->w);
        }

        if (data[i].posfix)
        {
            term_window_pos(&data[i], HWND_TOPMOST);
        }
        else
        {
            term_window_pos(&data[i], td->w);
        }
    }


    /* Main window */
    td = &data[0];

    /* Main window */
    my_td = td;
    td->w = CreateWindowEx(td->dwExStyle, AppName,
                   td->s, td->dwStyle,
                   td->pos_x, td->pos_y,
                   td->size_wid, td->size_hgt,
                   HWND_DESKTOP, NULL, hInstance, NULL);
    my_td = NULL;
    if (!td->w) quit("Failed to create Angband window");

    term_data_link(td);
    angband_term[0] = &td->t;
    normsize.x = td->cols;
    normsize.y = td->rows;

    /* Activate the main window */
    if (win_maximized) ShowWindow(td->w, SW_SHOWMAXIMIZED);
    else ShowWindow(td->w, SW_SHOW);

    /* Bring main window back to top */
    SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    term_init_double_buffer(td);

    /* New palette XXX XXX XXX */
    (void)new_palette();


    /* Create a "brush" for drawing the "cursor" */
    hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);


    /* Process pending messages */
    (void)Term_xtra_win_flush();
}



/*
 * Prepare the menus
 */
static void setup_menus(void)
{
    int i;

    HMENU hm = GetMenu(data[0].w);


    /* Menu "File", Disable all */
    EnableMenuItem(hm, IDM_FILE_NEW,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_FILE_OPEN,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_FILE_SAVE,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_FILE_EXIT,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);


    /* No character available */
    if (!character_generated)
    {
        /* Menu "File", Item "New" */
        EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_ENABLED);

        /* Menu "File", Item "Open" */
        EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_ENABLED);
    }

    /* A character available */
    if (character_generated)
    {
        /* Menu "File", Item "Save" */
        EnableMenuItem(hm, IDM_FILE_SAVE,
               MF_BYCOMMAND | MF_ENABLED);
    }

    /* Menu "File", Item "Exit" */
    EnableMenuItem(hm, IDM_FILE_EXIT,
               MF_BYCOMMAND | MF_ENABLED);


    /* Menu "Window::Visibility" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        CheckMenuItem(hm, IDM_WINDOW_VIS_0 + i,
                  (data[i].visible ? MF_CHECKED : MF_UNCHECKED));

        EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);
    }

    /* Menu "Window::Font" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
                       MF_BYCOMMAND | MF_ENABLED);
        }
    }

    /* Menu "Window::Window Position Fix" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_POS_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        CheckMenuItem(hm, IDM_WINDOW_POS_0 + i,
                  (data[i].posfix ? MF_CHECKED : MF_UNCHECKED));

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_POS_0 + i,
                       MF_BYCOMMAND | MF_ENABLED);
        }
    }

    /* Menu "Window::Bizarre Display" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        CheckMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
                  (data[i].bizarre ? MF_CHECKED : MF_UNCHECKED));

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);

        }
    }

    /* Menu "Window::Increase Tile Width" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);

        }
    }

    /* Menu "Window::Decrease Tile Width" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);

        }
    }

    /* Menu "Window::Increase Tile Height" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);

        }
    }

    /* Menu "Window::Decrease Tile Height" */
    for (i = 0; i < MAX_TERM_DATA; i++)
    {
        EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (data[i].visible)
        {
            EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
                   MF_BYCOMMAND | MF_ENABLED);

        }
    }

    /* Menu "Options", disable all */
    EnableMenuItem(hm, IDM_OPTIONS_NO_GRAPHICS,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_OPTIONS_OLD_GRAPHICS,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_OPTIONS_NEW_GRAPHICS,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_OPTIONS_BIGTILE,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_OPTIONS_SOUND,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_OPTIONS_SAVER,
               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

    /* Menu "Options", Item "Map" */
    if (use_graphics != GRAPHICS_NONE)
        EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP, MF_BYCOMMAND | MF_ENABLED);
    else
        EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP,
                   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

    /* Menu "Options", update all */
    CheckMenuItem(hm, IDM_OPTIONS_NO_GRAPHICS,
              (arg_graphics == GRAPHICS_NONE ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hm, IDM_OPTIONS_OLD_GRAPHICS,
              (arg_graphics == GRAPHICS_ORIGINAL ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hm, IDM_OPTIONS_NEW_GRAPHICS,
              (arg_graphics == GRAPHICS_ADAM_BOLT ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hm, IDM_OPTIONS_BIGTILE,
              (arg_bigtile ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hm, IDM_OPTIONS_SOUND,
              (arg_sound ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hm, IDM_OPTIONS_SAVER,
              (hwndSaver ? MF_CHECKED : MF_UNCHECKED));

#ifdef USE_GRAPHICS
    /* Menu "Options", Item "Graphics" */
    EnableMenuItem(hm, IDM_OPTIONS_NO_GRAPHICS, MF_ENABLED);
    /* Menu "Options", Item "Graphics" */
    EnableMenuItem(hm, IDM_OPTIONS_OLD_GRAPHICS, MF_ENABLED);
    /* Menu "Options", Item "Graphics" */
    EnableMenuItem(hm, IDM_OPTIONS_NEW_GRAPHICS, MF_ENABLED);
    /* Menu "Options", Item "Graphics" */
    EnableMenuItem(hm, IDM_OPTIONS_BIGTILE, MF_ENABLED);
#endif /* USE_GRAPHICS */

#ifdef USE_SOUND
    /* Menu "Options", Item "Sound" */
    EnableMenuItem(hm, IDM_OPTIONS_SOUND, MF_ENABLED);
#endif /* USE_SOUND */

#ifdef USE_SAVER
    /* Menu "Options", Item "ScreenSaver" */
    EnableMenuItem(hm, IDM_OPTIONS_SAVER,
               MF_BYCOMMAND | MF_ENABLED);
#endif /* USE_SAVER */
}


/*
 * Check for double clicked (or dragged) savefile
 *
 * Apparently, Windows copies the entire filename into the first
 * piece of the "command line string".  Perhaps we should extract
 * the "basename" of that filename and append it to the "save" dir.
 */
static void check_for_save_file(LPSTR cmd_line)
{
    char *s;

    /* First arg */
    s = cmd_line;

    /* No args */
    if (!*s) return;

    /* Extract filename */
    strcat(savefile, s);
    if (!check_file(savefile))
    {
        path_build(savefile, sizeof(savefile), ANGBAND_DIR_SAVE, s);
        if (!check_file(savefile))
            quit_fmt("Cannot find required file:\n%s", savefile);
    }

    /* Game in progress */
    game_in_progress = TRUE;

    /* Play game */
    play_game(FALSE);
}


/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
    int i;

    term_data *td;

    OPENFILENAME ofn;

    /* Analyze */
    switch (wCmd)
    {
        /* New game */
        case IDM_FILE_NEW:
        {
            if (!initialized)
            {
                plog("You cannot do that yet...");

            }
            else if (game_in_progress)
            {
                plog("You can't start a new game while you're still playing!");

            }
            else
            {
                game_in_progress = TRUE;
                Term_flush();
                play_game(TRUE);
                quit(NULL);
            }
            break;
        }

        /* Open game */
        case IDM_FILE_OPEN:
        {
            if (!initialized)
            {
                plog("You cannot do that yet...");

            }
            else if (game_in_progress)
            {
                plog("You can't open a new game while you're still playing!");

            }
            else
            {
                memset(&ofn, 0, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = data[0].w;
                ofn.lpstrFilter = "Save Files (*.)\0*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFile = savefile;
                ofn.nMaxFile = 1024;
                ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

                if (GetOpenFileName(&ofn))
                {
                    /* Load 'savefile' */
                    validate_file(savefile);
                    game_in_progress = TRUE;
                    Term_flush();
                    play_game(FALSE);
                    quit(NULL);
                }
            }
            break;
        }

        /* Save game */
        case IDM_FILE_SAVE:
        {
            if (game_in_progress && character_generated)
            {
                /* Paranoia */
                if (!can_save)
                {
                    plog("You may not do that right now.");

                    break;
                }

                /* Save the game */
                do_cmd_save_game(FALSE);
            }
            else
            {
                plog("You may not do that right now.");

            }
            break;
        }

        /* Exit */
        case IDM_FILE_EXIT:
        {
            if (game_in_progress && character_generated)
            {
                /* Paranoia */
                if (!can_save)
                {
                    plog("You may not do that right now.");

                    break;
                }

                forget_lite();
                forget_view();
                clear_mon_lite();

                Term_key_push(SPECIAL_KEY_QUIT);
                break;
            }
            quit(NULL);
            break;
        }

        /* Show scores */
        case IDM_WINDOW_VIS_0:
        {
            plog("You are not allowed to do that!");


            break;
        }

        /* Window visibility */
        case IDM_WINDOW_VIS_1:
        case IDM_WINDOW_VIS_2:
        case IDM_WINDOW_VIS_3:
        case IDM_WINDOW_VIS_4:
        case IDM_WINDOW_VIS_5:
        case IDM_WINDOW_VIS_6:
        case IDM_WINDOW_VIS_7:
        {
            i = wCmd - IDM_WINDOW_VIS_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            if (!td->visible)
            {
                td->visible = TRUE;
                ShowWindow(td->w, SW_SHOW);
                term_data_redraw(td);
            }
            else
            {
                td->visible = FALSE;
                td->posfix = FALSE;
                ShowWindow(td->w, SW_HIDE);
            }

            break;
        }

        /* Window fonts */
        case IDM_WINDOW_FONT_0:
        case IDM_WINDOW_FONT_1:
        case IDM_WINDOW_FONT_2:
        case IDM_WINDOW_FONT_3:
        case IDM_WINDOW_FONT_4:
        case IDM_WINDOW_FONT_5:
        case IDM_WINDOW_FONT_6:
        case IDM_WINDOW_FONT_7:
        {
            i = wCmd - IDM_WINDOW_FONT_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            term_change_font(td);

            break;
        }

        /* Window Z Position */
        case IDM_WINDOW_POS_1:
        case IDM_WINDOW_POS_2:
        case IDM_WINDOW_POS_3:
        case IDM_WINDOW_POS_4:
        case IDM_WINDOW_POS_5:
        case IDM_WINDOW_POS_6:
        case IDM_WINDOW_POS_7:
        {
            i = wCmd - IDM_WINDOW_POS_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            if (!td->posfix && td->visible)
            {
                td->posfix = TRUE;
                term_window_pos(td, HWND_TOPMOST);
            }
            else
            {
                td->posfix = FALSE;
                term_window_pos(td, data[0].w);
            }

            break;
        }

        /* Bizarre Display */
        case IDM_WINDOW_BIZ_0:
        case IDM_WINDOW_BIZ_1:
        case IDM_WINDOW_BIZ_2:
        case IDM_WINDOW_BIZ_3:
        case IDM_WINDOW_BIZ_4:
        case IDM_WINDOW_BIZ_5:
        case IDM_WINDOW_BIZ_6:
        case IDM_WINDOW_BIZ_7:
        {
            i = wCmd - IDM_WINDOW_BIZ_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            td->bizarre = !td->bizarre;

            term_getsize(td);

            term_window_resize(td);

            break;
        }

        /* Increase Tile Width */
        case IDM_WINDOW_I_WID_0:
        case IDM_WINDOW_I_WID_1:
        case IDM_WINDOW_I_WID_2:
        case IDM_WINDOW_I_WID_3:
        case IDM_WINDOW_I_WID_4:
        case IDM_WINDOW_I_WID_5:
        case IDM_WINDOW_I_WID_6:
        case IDM_WINDOW_I_WID_7:
        {
            i = wCmd - IDM_WINDOW_I_WID_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            td->tile_wid += 1;

            term_getsize(td);

            term_window_resize(td);

            break;
        }

        /* Decrease Tile Height */
        case IDM_WINDOW_D_WID_0:
        case IDM_WINDOW_D_WID_1:
        case IDM_WINDOW_D_WID_2:
        case IDM_WINDOW_D_WID_3:
        case IDM_WINDOW_D_WID_4:
        case IDM_WINDOW_D_WID_5:
        case IDM_WINDOW_D_WID_6:
        case IDM_WINDOW_D_WID_7:
        {
            i = wCmd - IDM_WINDOW_D_WID_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            td->tile_wid -= 1;

            term_getsize(td);

            term_window_resize(td);

            break;
        }

        /* Increase Tile Height */
        case IDM_WINDOW_I_HGT_0:
        case IDM_WINDOW_I_HGT_1:
        case IDM_WINDOW_I_HGT_2:
        case IDM_WINDOW_I_HGT_3:
        case IDM_WINDOW_I_HGT_4:
        case IDM_WINDOW_I_HGT_5:
        case IDM_WINDOW_I_HGT_6:
        case IDM_WINDOW_I_HGT_7:
        {
            i = wCmd - IDM_WINDOW_I_HGT_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            td->tile_hgt += 1;

            term_getsize(td);

            term_window_resize(td);

            break;
        }

        /* Decrease Tile Height */
        case IDM_WINDOW_D_HGT_0:
        case IDM_WINDOW_D_HGT_1:
        case IDM_WINDOW_D_HGT_2:
        case IDM_WINDOW_D_HGT_3:
        case IDM_WINDOW_D_HGT_4:
        case IDM_WINDOW_D_HGT_5:
        case IDM_WINDOW_D_HGT_6:
        case IDM_WINDOW_D_HGT_7:
        {
            i = wCmd - IDM_WINDOW_D_HGT_0;

            if ((i < 0) || (i >= MAX_TERM_DATA)) break;

            td = &data[i];

            td->tile_hgt -= 1;

            term_getsize(td);

            term_window_resize(td);

            break;
        }

        case IDM_OPTIONS_NO_GRAPHICS:
        {
            /* Paranoia */
            if (!inkey_flag)
            {
                plog("You may not do that right now.");
                break;
            }

            /* Toggle "arg_graphics" */
            if (arg_graphics != GRAPHICS_NONE)
            {
                arg_graphics = GRAPHICS_NONE;

                /* React to changes */
                Term_xtra_win_react();

                /* Hack -- Force redraw */
                Term_key_push(KTRL('R'));
            }

            break;
        }

        case IDM_OPTIONS_OLD_GRAPHICS:
        {
            /* Paranoia */
            if (!inkey_flag)
            {
                plog("You may not do that right now.");
                break;
            }

            /* Toggle "arg_graphics" */
            if (arg_graphics != GRAPHICS_ORIGINAL)
            {
                arg_graphics = GRAPHICS_ORIGINAL;

                /* React to changes */
                Term_xtra_win_react();

                /* Hack -- Force redraw */
                Term_key_push(KTRL('R'));
            }

            break;
        }

        case IDM_OPTIONS_NEW_GRAPHICS:
        {
            /* Paranoia */
            if (!inkey_flag)
            {
                plog("You may not do that right now.");
                break;
            }

            /* Toggle "arg_graphics" */
            if (arg_graphics != GRAPHICS_ADAM_BOLT)
            {
                arg_graphics = GRAPHICS_ADAM_BOLT;

                /* React to changes */
                Term_xtra_win_react();

                /* Hack -- Force redraw */
                Term_key_push(KTRL('R'));
            }

            break;
        }

        case IDM_OPTIONS_BIGTILE:
        {
            term_data *td = &data[0];

            /* Paranoia */
            if (!inkey_flag)
            {
                plog("You may not do that right now.");
                break;
            }

            /* Toggle "arg_sound" */
            arg_bigtile = !arg_bigtile;

            /* Activate */
            Term_activate(&td->t);

            /* Resize the term */
            Term_resize(td->cols, td->rows);

            /* Redraw later */
            InvalidateRect(td->w, NULL, TRUE);

            break;
        }

        case IDM_OPTIONS_SOUND:
        {
            /* Paranoia */
            if (!inkey_flag)
            {
                plog("You may not do that right now.");
                break;
            }

            /* Toggle "arg_sound" */
            arg_sound = !arg_sound;

            /* React to changes */
            Term_xtra_win_react();

            /* Hack -- Force redraw */
            Term_key_push(KTRL('R'));

            break;
        }

        case IDM_DUMP_SCREEN_HTML:
        {
            static char buf[1024] = "";
            memset(&ofn, 0, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = data[0].w;
            ofn.lpstrFilter = "HTML Files (*.html)\0*.html\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = 1023;
            ofn.lpstrDefExt = "html";
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrTitle = "Save screen dump as HTML.";
            ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn))
            {
                save_screen_aux(buf, DOC_FORMAT_HTML);
            }
            break;
        }

#ifdef USE_SAVER

        case IDM_OPTIONS_SAVER:
        {
            if (hwndSaver)
            {
                DestroyWindow(hwndSaver);
                hwndSaver = NULL;
            }
            else
            {
                /* Create a screen scaver window */
                hwndSaver = CreateWindowEx(WS_EX_TOPMOST, "WindowsScreenSaverClass",
                               "Angband Screensaver",
                               WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
                               0, 0, GetSystemMetrics(SM_CXSCREEN),
                               GetSystemMetrics(SM_CYSCREEN),
                               NULL, NULL, hInstance, NULL);

                if (hwndSaver)
                {
                    /* Push the window to the bottom XXX XXX XXX */
                    SetWindowPos(hwndSaver, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                else
                {
                    plog("Failed to create saver window");

                }
            }
            break;
        }

#endif

        case IDM_OPTIONS_MAP:
        {
            windows_map();
            break;
        }

        case IDM_HELP_CONTENTS:
        {
#ifdef HTML_HELP
            char tmp[1024];
            path_build(tmp, sizeof(tmp), ANGBAND_DIR_XTRA_HELP, "zangband.chm");
            if (check_file(tmp))
            {
                HtmlHelp(data[0].w, tmp, HH_DISPLAY_TOPIC, 0);
            }
            else
            {
                plog_fmt("Cannot find help file: %s", tmp);
                plog("Use the online help files instead.");

            }
            break;
#else /* HTML_HELP */
            char buf[1024];
            char tmp[1024];
            path_build(tmp, sizeof(tmp), ANGBAND_DIR_XTRA_HELP, "zangband.hlp");
            if (check_file(tmp))
            {
                sprintf(buf, "winhelp.exe %s", tmp);
                WinExec(buf, SW_NORMAL);
            }
            else
            {
                plog_fmt("Cannot find help file: %s", tmp);
                plog("Use the online help files instead.");

            }
            break;
#endif /* HTML_HELP */
        }
    }
}


static bool process_keydown(WPARAM wParam, LPARAM lParam)
{
    int i;
    bool mc = FALSE;
    bool ms = FALSE;
    bool ma = FALSE;

    /* Extract the modifiers */
    if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
    if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
    if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

    /* Handle "special" keys */
    Term_no_press = FALSE;
    if (special_key[(byte)(wParam)])
    {
        bool ext_key = (lParam & 0x1000000L) ? TRUE : FALSE;
        bool numpad = FALSE;

        /* Begin the macro trigger */
        Term_keypress(31);

        /* Send the modifiers */
        if (mc) Term_keypress('C');
        if (ms) Term_keypress('S');
        if (ma) Term_keypress('A');

        /* Extract "scan code" */
        i = LOBYTE(HIWORD(lParam));

        /* Introduce the scan code */
        Term_keypress('x');

        /* Extended key bit */
        switch (wParam)
        {
            /* Numpad Enter and '/' are extended key */
        case VK_DIVIDE:
            Term_no_press = TRUE;
        case VK_RETURN:    /* Enter */
            numpad = ext_key;
            break;
            /* Other extended keys are on full keyboard */
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_ADD:
        case VK_MULTIPLY:
        case VK_SUBTRACT:
        case VK_SEPARATOR:
        case VK_DECIMAL:
            Term_no_press = TRUE;
        case VK_CLEAR:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:    /* Page Up */
        case VK_NEXT:    /* Page Down */
        case VK_INSERT:
        case VK_DELETE:
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            numpad = !ext_key;
        }

        /* Special modifiers for keypad keys */
        if (numpad) Term_keypress('K');

        /* Encode the hexidecimal scan code */
        Term_keypress(hexsym[i/16]);
        Term_keypress(hexsym[i%16]);

        /* End the macro trigger */
        Term_keypress(13);

        return 1;
    }

    return 0;
}


#ifdef __MWERKS__
LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                  WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                  WPARAM wParam, LPARAM lParam)
#else /* __MWERKS__ */
LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                      WPARAM wParam, LPARAM lParam)
#endif /* __MWERKS__ */
{
    PAINTSTRUCT ps;
    HDC hdc;
    term_data *td;
    int i;


    /* Acquire proper "term_data" info */
    td = (term_data *)GetWindowLong(hWnd, 0);

    /* Handle message */
    switch (uMsg)
    {
        /* XXX XXX XXX */
        case WM_NCCREATE:
        {
            SetWindowLong(hWnd, 0, (LONG)(my_td));
            break;
        }

        /* XXX XXX XXX */
        case WM_CREATE:
        {
            return 0;
        }

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO FAR *lpmmi;
            RECT rc;

            lpmmi = (MINMAXINFO FAR *)lParam;

            /* this message was sent before WM_NCCREATE */
            if (!td) return 1;

            /* Minimum window size is 80x27 */
            rc.left = rc.top = 0;
            rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 + td->size_ow2;
            rc.bottom = rc.top + 27 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

            /* Adjust */
            AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

            /* Save minimum size */
            lpmmi->ptMinTrackSize.x = rc.right - rc.left;
            lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

            return 0;
        }

        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            if (td) term_data_redraw(td);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            if (process_keydown(wParam, lParam))
                return 0;
            break;
        }

        case WM_CHAR:
        {
            if (Term_no_press) Term_no_press = FALSE;
            else Term_keypress(wParam);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            mousex = MIN(LOWORD(lParam) / td->tile_wid, td->cols - 1);
            mousey = MIN(HIWORD(lParam) / td->tile_hgt, td->rows - 1);
            mouse_down = TRUE;
            oldx = mousex;
            oldy = mousey;
            return 0;
        }

        case WM_LBUTTONUP:
        {
            HGLOBAL hGlobal;
            LPSTR lpStr;
            int i, j, sz;
            int dx = abs(oldx - mousex) + 1;
            int dy = abs(oldy - mousey) + 1;
            int ox = (oldx > mousex) ? mousex : oldx;
            int oy = (oldy > mousey) ? mousey : oldy;

            mouse_down = FALSE;
            paint_rect = FALSE;

            sz = (dx + 2) * dy;
            hGlobal = GlobalAlloc(GHND, sz + 1);
            if (hGlobal == NULL) return 0;
            lpStr = (LPSTR)GlobalLock(hGlobal);

            for (i = 0; i < dy; i++)
            {
                for (j = 0; j < dx; j++)
                {
                    *lpStr++ = data[0].t.scr->c[oy + i][ox + j];
                }
                if (dy > 1)
                {
                    *lpStr++ = '\r';
                    *lpStr++ = '\n';
                }
            }

            GlobalUnlock(hGlobal);
            if (OpenClipboard(hWnd) == 0)
            {
                GlobalFree(hGlobal);
                return 0;
            }
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hGlobal);
            CloseClipboard();

            Term_redraw();

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (mouse_down)
            {
                int dx, dy;
                int cx = MIN(LOWORD(lParam) / td->tile_wid, td->cols - 1);
                int cy = MIN(HIWORD(lParam) / td->tile_hgt, td->rows - 1);
                int ox, oy;

                if (paint_rect)
                {
                    dx = abs(oldx - mousex) + 1;
                    dy = abs(oldy - mousey) + 1;
                    ox = (oldx > mousex) ? mousex : oldx;
                    oy = (oldy > mousey) ? mousey : oldy;
                    Term_inversed_area(hWnd, ox, oy, dx, dy);
                }
                else
                {
                    paint_rect = TRUE;
                }

                dx = abs(cx - mousex) + 1;
                dy = abs(cy - mousey) + 1;
                ox = (cx > mousex) ? mousex : cx;
                oy = (cy > mousey) ? mousey : cy;
                Term_inversed_area(hWnd, ox, oy, dx, dy);

                oldx = cx;
                oldy = cy;
            }
            return 0;
        }

        case WM_INITMENU:
        {
            setup_menus();
            return 0;
        }

        case WM_CLOSE:
        {
            if (game_in_progress && character_generated)
            {
                if (!can_save)
                {
                    plog("You may not do that right now.");
                    return 0;
                }

                forget_lite();
                forget_view();
                clear_mon_lite();

                Term_key_push(SPECIAL_KEY_QUIT);
                return 0;
            }
            quit(NULL);
            return 0;
        }

        case WM_QUERYENDSESSION:
        {
            if (game_in_progress && character_generated)
            {
                /* Mega-Hack -- Delay death */
                if (p_ptr->chp < 0) p_ptr->is_dead = FALSE;


                /* Hardcode panic save */
                p_ptr->panic_save = 1;

                /* Forbid suspend */
                signals_ignore_tstp();

                /* Indicate panic save */
                (void)strcpy(p_ptr->died_from, "(panic save)");

                /* Panic save */
                (void)save_player();
            }
            quit(NULL);
            return 0;
        }

        case WM_QUIT:
        {
            quit(NULL);
            return 0;
        }

        /*case WM_ERASEBKGND:
            return 1;*/

        case WM_COMMAND:
        {
            process_menus(LOWORD(wParam));
            return 0;
        }

        case WM_SIZE:
        {
            /* this message was sent before WM_NCCREATE */
            if (!td) return 1;

            /* it was sent from inside CreateWindowEx */
            if (!td->w) return 1;

            /* was sent from WM_SIZE */
            if (td->size_hack) return 1;

            switch (wParam)
            {
                case SIZE_MINIMIZED:
                {
                    /* Hide sub-windows */
                    for (i = 1; i < MAX_TERM_DATA; i++)
                    {
                        if (data[i].visible) ShowWindow(data[i].w, SW_HIDE);
                    }
                    return 0;
                }

                case SIZE_MAXIMIZED:
                {
                    /* fall through XXX XXX XXX */
                }

                case SIZE_RESTORED:
                {
                    uint cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
                    uint rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

                    /* New size */
                    if ((td->cols != cols) || (td->rows != rows))
                    {
                        /* Save the new size */
                        td->cols = cols;
                        td->rows = rows;

                        if (!IsZoomed(td->w) && !IsIconic(td->w))
                        {
                            normsize.x = td->cols;
                            normsize.y = td->rows;
                        }

                        /* Activate */
                        Term_activate(&td->t);

                        /* Resize the term */
                        Term_resize(td->cols, td->rows);

                        /* I'm going nuts here! Was td size data just always
                           wrong before? Crazy Nuts :P */
                        term_getsize(td);
                        
                        /* Redraw later */
                        InvalidateRect(td->w, NULL, TRUE);
                    }

                    td->size_hack = TRUE;

                    /* Show sub-windows */
                    for (i = 1; i < MAX_TERM_DATA; i++)
                    {
                        if (data[i].visible) ShowWindow(data[i].w, SW_SHOW);
                    }

                    td->size_hack = FALSE;

                    return 0;
                }
            }
            break;
        }

        case WM_PALETTECHANGED:
        {
            /* Ignore if palette change caused by itself */
            if ((HWND)wParam == hWnd) return 0;

            /* Fall through... */
        }

        case WM_QUERYNEWPALETTE:
        {
            if (!paletted) return 0;

            hdc = GetDC(hWnd);

            SelectPalette(hdc, hPal, FALSE);

            i = RealizePalette(hdc);

            /* if any palette entries changed, repaint the window. */
            if (i) InvalidateRect(hWnd, NULL, TRUE);

            ReleaseDC(hWnd, hdc);

            return 0;
        }

        case WM_ACTIVATE:
        {
            if (wParam && !HIWORD(lParam))
            {
                /* Do something to sub-windows */
                for (i = 1; i < MAX_TERM_DATA; i++)
                {
                    if (!data[i].posfix) term_window_pos(&data[i], hWnd);
                }

                /* Focus on main window */
                SetFocus(hWnd);

                return 0;
            }

            break;
        }

        case WM_ACTIVATEAPP:
        {
            if (IsIconic(td->w)) break;

            for (i = 1; i < MAX_TERM_DATA; i++)
            {
                if(data[i].visible)
                {
                    if (wParam == TRUE)
                    {
                        ShowWindow(data[i].w, SW_SHOW);
                    }
                    else
                    {
                        ShowWindow(data[i].w, SW_HIDE);
                    }
                }
            }
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef __MWERKS__
LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                       WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                       WPARAM wParam, LPARAM lParam)
#else /* __MWERKS__ */
LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                       WPARAM wParam, LPARAM lParam)
#endif /* __MWERKS__ */
{
    term_data *td;
    PAINTSTRUCT ps;
    HDC hdc;
    int i;


    /* Acquire proper "term_data" info */
    td = (term_data *)GetWindowLong(hWnd, 0);

    /* Process message */
    switch (uMsg)
    {
        /* XXX XXX XXX */
        case WM_NCCREATE:
        {
            SetWindowLong(hWnd, 0, (LONG)(my_td));
            break;
        }

        /* XXX XXX XXX */
        case WM_CREATE:
        {
            return 0;
        }

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO FAR *lpmmi;
            RECT rc;

            lpmmi = (MINMAXINFO FAR *)lParam;

            /* this message was sent before WM_NCCREATE */
            if (!td) return 1;

            rc.left = rc.top = 0;
            rc.right = rc.left + 20 * td->tile_wid + td->size_ow1 + td->size_ow2;
            rc.bottom = rc.top + 3 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

            /* Adjust */
            AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

            /* Save minimum size */
            lpmmi->ptMinTrackSize.x = rc.right - rc.left;
            lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

            return 0;
        }

        case WM_SIZE:
        {
            uint cols;
            uint rows;
            
            /* this message was sent before WM_NCCREATE */
            if (!td) return 1;

            /* it was sent from inside CreateWindowEx */
            if (!td->w) return 1;

            /* was sent from inside WM_SIZE */
            if (td->size_hack) return 1;

            td->size_hack = TRUE;

            cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
            rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

            /* New size */
            if ((td->cols != cols) || (td->rows != rows))
            {
                /* Save old term */
                term *old_term = Term;

                /* Save the new size */
                td->cols = cols;
                td->rows = rows;

                /* Activate */
                Term_activate(&td->t);

                /* Resize the term */
                Term_resize(td->cols, td->rows);

                /* I'm going nuts here! Was td size data just always
                    wrong before? Crazy Nuts :P */
                term_getsize(td);

                /* Activate */
                Term_activate(old_term);

                /* Redraw later */
                InvalidateRect(td->w, NULL, TRUE);

                /* HACK - Redraw all windows */
                p_ptr->window = 0xFFFFFFFF;
                window_stuff();
            }

            td->size_hack = FALSE;

            return 0;
        }

        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            if (td) term_data_redraw(td);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            if (process_keydown(wParam, lParam))
                return 0;
            break;
        }

        case WM_CHAR:
        {
            if (Term_no_press) Term_no_press = FALSE;
            else Term_keypress(wParam);
            return 0;
        }

        case WM_PALETTECHANGED:
        {
            /* ignore if palette change caused by itself */
            if ((HWND)wParam == hWnd) return FALSE;
            /* otherwise, fall through!!! */
        }

        case WM_QUERYNEWPALETTE:
        {
            if (!paletted) return 0;
            hdc = GetDC(hWnd);
            SelectPalette(hdc, hPal, FALSE);
            i = RealizePalette(hdc);
            /* if any palette entries changed, repaint the window. */
            if (i) InvalidateRect(hWnd, NULL, TRUE);
            ReleaseDC(hWnd, hdc);
            return 0;
        }

        case WM_NCLBUTTONDOWN:
        {

#ifdef HTCLOSE
            if (wParam == HTCLOSE) wParam = HTSYSMENU;
#endif /* HTCLOSE */

            if (wParam == HTSYSMENU)
            {
                if (td->visible)
                {
                    td->visible = FALSE;
                    ShowWindow(td->w, SW_HIDE);
                }

                return 0;
            }

            break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef USE_SAVER

#define MOUSE_SENS 40

#ifdef __MWERKS__
LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam)
#else /* __MWERKS__ */
LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
                        WPARAM wParam, LPARAM lParam)
#endif /* __MWERKS__ */
{
    static int iMouse = 0;
    static WORD xMouse = 0;
    static WORD yMouse = 0;

    int dx, dy;


    /* Process */
    switch (uMsg)
    {
        /* XXX XXX XXX */
        case WM_NCCREATE:
        {
            break;
        }

        case WM_SETCURSOR:
        {
            SetCursor(NULL);
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_KEYDOWN:
        {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (iMouse)
            {
                dx = LOWORD(lParam) - xMouse;
                dy = HIWORD(lParam) - yMouse;

                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;

                if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
                {
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                }
            }

            /* Save last location */
            iMouse = 1;
            xMouse = LOWORD(lParam);
            yMouse = HIWORD(lParam);

            return 0;
        }

        case WM_CLOSE:
        {
            DestroyWindow(hwndSaver);
            hwndSaver = NULL;
            return 0;
        }
    }

    /* Oops */
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif /* USE_SAVER */





/*** Temporary Hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hack_plog(cptr str)
{
    /* Give a warning */
    if (str)
    {
        MessageBox(NULL, str, "Warning",
               MB_ICONEXCLAMATION | MB_OK);

    }
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hack_quit(cptr str)
{
    /* Give a warning */
    if (str)
    {
        MessageBox(NULL, str, "Error",
               MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);

    }

    /* Unregister the classes */
    UnregisterClass(AppName, hInstance);

    /* Destroy the icon */
    if (hIcon) DestroyIcon(hIcon);

    /* Exit */
    exit(0);
}



/*** Various hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(cptr str)
{
    /* Warning */
    if (str)
    {
        MessageBox(data[0].w, str, "Warning",
               MB_ICONEXCLAMATION | MB_OK);

    }
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(cptr str)
{
    int i;


    /* Give a warning */
    if (str)
    {
        MessageBox(data[0].w, str, "Error",
               MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);

    }


    /* Save the preferences */
    save_prefs();


    /*** Could use 'Term_nuke_win()' XXX XXX XXX */

    /* Destroy all windows */
    for (i = MAX_TERM_DATA - 1; i >= 0; --i)
    {
        term_force_font(&data[i], NULL);
        if (data[i].font_want) z_string_free(data[i].font_want);
        if (data[i].w) DestroyWindow(data[i].w);
        if (data[i].hDC) DeleteDC(data[i].hDC);
        if (data[i].hBitmap) DeleteObject(data[i].hBitmap);
        data[i].w = 0;
        data[i].hDC = 0;
        data[i].hBitmap = 0;
    }

    /* Free the bitmap stuff */
#ifdef USE_GRAPHICS

    if (_graphics.tiles.hPalette) DeleteObject(_graphics.tiles.hPalette);
    if (_graphics.tiles.hBitmap) DeleteObject(_graphics.tiles.hBitmap);
    if (_graphics.hdcTiles) DeleteDC(_graphics.hdcTiles);

    if (_graphics.mask.hPalette) DeleteObject(_graphics.mask.hPalette);
    if (_graphics.mask.hBitmap) DeleteObject(_graphics.mask.hBitmap);
    if (_graphics.hdcMask) DeleteDC(_graphics.hdcMask);

#endif /* USE_GRAPHICS */

    /*** Free some other stuff ***/

    DeleteObject(hbrYellow);

    if (hPal) DeleteObject(hPal);

    UnregisterClass(AppName, hInstance);

    if (hIcon) DestroyIcon(hIcon);

    exit(0);
}



/*** Initialize ***/


/*
 * Init some stuff
 */
static void init_stuff(void)
{
    int i;

    char path[1024];


    /* Get program name with full path */
    GetModuleFileName(hInstance, path, 512);

    /* Save the "program name" XXX XXX XXX */
    argv0 = path;

    /* Get the name of the "*.ini" file */
    strcpy(path + strlen(path) - 4, ".INI");

    /* Save the the name of the ini-file */
    ini_file = z_string_make(path);

    /* Analyze the path */
    i = strlen(path);

    /* Get the path */
    for (; i > 0; i--)
    {
        if (path[i] == '\\')
        {
            /* End of path */
            break;
        }
    }

    /* Add "lib" to the path */
    strcpy(path + i + 1, "lib\\");

    /* Validate the path */
    validate_dir(path, TRUE);

    /* Init the file paths */
    init_file_paths(path, path, path);

    /* Hack -- Validate the paths */
    validate_dir(ANGBAND_DIR_APEX, FALSE);
    validate_dir(ANGBAND_DIR_BONE, FALSE);

    /* Allow missing 'edit' directory */
    if (!check_dir(ANGBAND_DIR_EDIT))
    {
        /* Must have 'data'! */
        validate_dir(ANGBAND_DIR_DATA, TRUE);
    }
    else
    {
        /* Don't need 'data' */
        validate_dir(ANGBAND_DIR_DATA, FALSE);
    }

    validate_dir(ANGBAND_DIR_FILE, TRUE);
    validate_dir(ANGBAND_DIR_HELP, FALSE);
    validate_dir(ANGBAND_DIR_INFO, FALSE);
    validate_dir(ANGBAND_DIR_PREF, TRUE);
    validate_dir(ANGBAND_DIR_SAVE, FALSE);
    validate_dir(ANGBAND_DIR_USER, TRUE);
    validate_dir(ANGBAND_DIR_XTRA, TRUE);

    /* Build the filename */
    path_build(path, sizeof(path), ANGBAND_DIR_FILE, "news.txt");


    /* Hack -- Validate the "news.txt" file */
    validate_file(path);

#ifdef USE_GRAPHICS

    /* Build the "graf" path */
    path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "graf");

    /* Allocate the path */
    ANGBAND_DIR_XTRA_GRAF = z_string_make(path);

    /* Validate the "graf" directory */
    validate_dir(ANGBAND_DIR_XTRA_GRAF, TRUE);

#endif /* USE_GRAPHICS */


#ifdef USE_SOUND

    /* Build the "sound" path */
    path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "sound");

    /* Allocate the path */
    ANGBAND_DIR_XTRA_SOUND = z_string_make(path);

    /* Validate the "sound" directory */
    validate_dir(ANGBAND_DIR_XTRA_SOUND, FALSE);

#endif /* USE_SOUND */

#ifdef USE_MUSIC

    /* Build the "music" path */
    path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "music");

    /* Allocate the path */
    ANGBAND_DIR_XTRA_MUSIC = z_string_make(path);

    /* Validate the "music" directory */
    validate_dir(ANGBAND_DIR_XTRA_MUSIC, FALSE);

#endif /* USE_MUSIC */

    /* Build the "help" path */
    path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "help");

    /* Allocate the path */
    ANGBAND_DIR_XTRA_HELP = z_string_make(path);

    /* Validate the "help" directory */
    /* validate_dir(ANGBAND_DIR_XTRA_HELP); */
}


int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
               LPSTR lpCmdLine, int nCmdShow)
{
    int i;

    WNDCLASS wc;
    HDC hdc;
    MSG msg;

    /* Unused */
    (void)nCmdShow;

    /* Save globally */
    hInstance = hInst;

    /* Initialize */
    if (hPrevInst == NULL)
    {
        wc.style         = CS_CLASSDC;
        wc.lpfnWndProc   = AngbandWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 4; /* one long pointer to term_data */
        wc.hInstance     = hInst;
        wc.hIcon         = hIcon = LoadIcon(hInst, AppName);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName  = AppName;
        wc.lpszClassName = AppName;

        if (!RegisterClass(&wc)) exit(1);

        wc.lpfnWndProc   = AngbandListProc;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = AngList;

        if (!RegisterClass(&wc)) exit(2);

#ifdef USE_SAVER

        wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
        wc.lpfnWndProc    = AngbandSaverProc;
        wc.hCursor        = NULL;
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = "WindowsScreenSaverClass";

        if (!RegisterClass(&wc)) exit(3);

#endif

    }

    /* Temporary hooks */
    plog_aux = hack_plog;
    quit_aux = hack_quit;
    core_aux = hack_quit;

    /* Prepare the filepaths */
    init_stuff();

    /* Initialize the keypress analyzer */
    for (i = 0; special_key_list[i]; ++i)
    {
        special_key[special_key_list[i]] = TRUE;
    }
    /* Initialize the keypress analyzer */
    for (i = 0; ignore_key_list[i]; ++i)
    {
        ignore_key[ignore_key_list[i]] = TRUE;
    }

    /* Determine if display is 16/256/true color */
    hdc = GetDC(NULL);
    colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
    paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
    ReleaseDC(NULL, hdc);

    /* Initialize the colors */
    for (i = 0; i < 256; i++)
    {
        byte rv, gv, bv;

        /* Extract desired values */
        rv = angband_color_table[i][1];
        gv = angband_color_table[i][2];
        bv = angband_color_table[i][3];

        /* Extract the "complex" code */
        win_clr[i] = PALETTERGB(rv, gv, bv);

        /* Save the "simple" code */
        angband_color_table[i][0] = win_pal[i];
    }

    /* Prepare the windows */
    init_windows();

    /* Activate hooks */
    plog_aux = hook_plog;
    quit_aux = hook_quit;
    core_aux = hook_quit;

    /* Set the system suffix */
    ANGBAND_SYS = "win";

    /* Set the keyboard suffix */
    if (7 != GetKeyboardType(0))
        ANGBAND_KEYBOARD = "0";
    else
    {
        /* Japanese keyboard */
        switch (GetKeyboardType(1))
        {
        case 0x0D01: case 0x0D02:
        case 0x0D03: case 0x0D04:
        case 0x0D05: case 0x0D06:
            /* NEC PC-98x1 */
            ANGBAND_KEYBOARD = "NEC98";
            break;
        default:
            /* PC/AT */
            ANGBAND_KEYBOARD = "JAPAN";
        }
    }

    /* Catch nasty signals */
    signals_init();

    /* Initialize */
    init_angband();

    /* Did the user double click on a save file? */
    check_for_save_file(lpCmdLine);

    Term_flush();
    display_news();
    c_prt(TERM_YELLOW, "                 [Choose 'New' or 'Open' from the 'File' menu]", Term->hgt - 1, 0);
    Term_fresh();

    /* Process messages forever */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Paranoia */
    quit(NULL);

    /* Paranoia */
    return (0);
}


#endif /* WINDOWS */

