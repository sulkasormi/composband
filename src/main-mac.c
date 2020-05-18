/* File: main-mac.c */

/* Purpose: Simple support for MACINTOSH Angband */

/*
 * This file should only be compiled with the "Macintosh" version
 *
 * This file written by "Ben Harrison (benh@phial.com)".
 *
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * Maarten Hazewinkel (mmhazewi@cs.ruu.nl) provided some initial
 * suggestions for the PowerMac port.
 *
 * Steve Linberg (slinberg@crocker.com) provided the code surrounded
 * by "USE_SFL_CODE".
 *
 * The graphics code is adapted from an extremely minimal subset of
 * the code from "Sprite World II", an amazing animation package.
 *
 * See "z-term.c" for info on the concept of the "generic terminal"
 *
 * The preference file is now a text file named "Angband preferences".
 *
 * Note that the "preference" file is now a simple text file called
 * "Angband preferences", which contains the versions information, so
 * that obsolete preference files can be ignored (this may be bad).
 *
 * Note that "init1.c", "init2.c", "load1.c", "load2.c", and "birth.c"
 * should probably be "unloaded" as soon as they are no longer needed,
 * to save space, but I do not know how to do this.
 *
 * Stange bug -- The first "ClipRect()" call crashes if the user closes
 * all the windows, switches to another application, switches back, and
 * then re-opens the main window, for example, using "command-a".
 *
 * By default, this file assumes that you will be using a 68020 or better
 * machine, running System 7 and Color Quickdraw.  In fact, the game will
 * refuse to run unless these features are available.  This allows the use
 * of a variety of interesting features such as graphics and sound.
 *
 * To create a version which can be used on 68000 machines, or on machines
 * which are not running System 7 or Color Quickdraw, simply activate the
 * "ANGBAND_LITE_MAC" compilation flag in the proper header file.  This
 * will disable all "modern" features used in this file, including support
 * for multiple sub-windows, color, graphics, and sound.
 *
 * When compiling with the "ANGBAND_LITE_MAC" flag, the "ANGBAND_LITE"
 * flag will be automatically defined, which will disable many of the
 * advanced features of the game itself, reducing the total memory usage.
 *
 * If you are never going to use "graphics" (especially if you are not
 * compiling support for graphics anyway) then you can delete the "pict"
 * resource with id "1001" with no dangerous side effects.
 */


/*
 * Important Resources in the resource file:
 *
 *   FREF 130 = 'A271' / 'APPL' (application)
 *   FREF 129 = 'A271' / 'SAVE' (save file)
 *   FREF 130 = 'A271' / 'TEXT' (bone file, generic text file)
 *   FREF 131 = 'A271' / 'DATA' (binary image file, score file)
 *
 *   DLOG 128 = "About Angband..."
 *
 *   ALRT 128 = unused (?)
 *   ALRT 129 = "Warning..."
 *   ALRT 130 = "Are you sure you want to quit without saving?"
 *
 *   DITL 128 = body for DLOG 128
 *   DITL 129 = body for ALRT 129
 *   DITL 130 = body for ALRT 130
 *
 *   ICON 128 = "warning" icon
 *
 *   MENU 128 = apple (about, -, ...)
 *   MENU 129 = File (new, open, close, save, -, exit, quit)
 *   MENU 130 = Edit (undo, -, cut, copy, paste, clear)
 *
 *   PICT 1001 = Graphics tile set
 */


/*
 * File name patterns:
 *   all 'APEX' files have a filename of the form "*:apex:*" (?)
 *   all 'BONE' files have a filename of the form "*:bone:*" (?)
 *   all 'DATA' files have a filename of the form "*:data:*"
 *   all 'SAVE' files have a filename of the form "*:save:*"
 *   all 'USER' files have a filename of the form "*:user:*" (?)
 *
 * Perhaps we should attempt to set the "_ftype" flag inside this file,
 * to avoid nasty file type information being spread all through the
 * rest of the code.  (?)  This might require adding hooks into the
 * "fd_open()" and "my_fopen()" functions in "util.c".  XXX XXX XXX
 */


/*
 * Reasons for each header file:
 *
 *   angband.h = Angband header file
 *
 *   Types.h = (included anyway)
 *   Gestalt.h = gestalt code
 *   QuickDraw.h = (included anyway)
 *   OSUtils.h = (included anyway)
 *   Files.h = file code
 *   Fonts.h = font code
 *   Menus.h = menu code
 *   Dialogs.h = dialog code
 *   Windows.h = (included anyway)
 *   Palettes.h = palette code
 *   StandardFile.h = file dialog box
 *   DiskInit.h = disk initialization
 *   ToolUtils.h = HiWord() / LoWord()
 *   Desk.h = OpenDeskAcc()
 *   Devices.h = OpenDeskAcc()
 *   Events.h = event code
 *   Resources.h = resource code
 *   Controls.h = button code
 *   SegLoad.h = ExitToShell(), AppFile, etc
 *   Memory.h = SetApplLimit(), NewPtr(), etc
 *   QDOffscreen.h = GWorld code
 *   Sound.h = Sound code
 *
 * For backwards compatibility:
 *   Use GestaltEqu.h instead of Gestalt.h
 *   Add Desk.h to include simply includes Menus.h, Devices.h, Events.h
 */


#include "angband.h"

#ifdef MACH_O_CARBON

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>

#define TARGET_API_MAC_CARBON 1

#else /* MACH_O_CARBON */

#include <Types.h>
#include <Gestalt.h>
#include <QuickDraw.h>
#include <Files.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Windows.h>
#include <Palettes.h>
#include <StandardFile.h>
#include <DiskInit.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Events.h>
#include <Resources.h>
#include <Controls.h>
#include <SegLoad.h>
#include <Memory.h>
#include <QDOffscreen.h>
#include <Sound.h>
#if TARGET_API_MAC_CARBON
#include <Navigation.h>
#include <CFPreferences.h>
#include <CFNumber.h>
# ifdef MAC_MPW
# include <CarbonStdCLib.h>
# endif
#endif


#endif /* MACH_O_CARBON */

/*
 * Cleaning up a couple of things to make these easier to change --AR
 */
#define PREF_FILE_NAME "Hengband-E Preferences"

/*
 * Use "malloc()" instead of "NewPtr()"
 */
/* #define USE_MALLOC */


/* Default creator signature */
#ifndef ANGBAND_CREATOR
# define ANGBAND_CREATOR 'Heng'
#endif


#if defined(powerc) || defined(__powerc)

/*
 * Disable "LITE" version
 */
# undef ANGBAND_LITE_MAC

#endif


#ifdef ANGBAND_LITE_MAC

/*
 * Maximum number of windows
 */
# define MAX_TERM_DATA 1

#else /* ANGBAND_LITE_MAC */

/*
 * Maximum number of windows
 */
# define MAX_TERM_DATA 8

/*
 * Activate some special code
 */
# define USE_SFL_CODE

/*
 * Use rewritten asynchronous sound player
 */
#define USE_ASYNC_SOUND


#ifdef MACH_O_CARBON

#define USE_QT_SOUND

#endif /* MACH_O_CARBON */
#endif /* ANGBAND_LITE_MAC */



#ifndef MACH_O_CARBON
#ifdef USE_SFL_CODE

/*
 * Include the necessary header files
 */
#include <AppleEvents.h>
#include <EPPC.h>
#include <Folders.h>

#endif
#endif /* !MACH_O_CARBON */

/*
 * Globals for MPW compilation
 */
#if defined(MACH_O_CARBON) || defined(MAC_MPW)
       /* Globals needed */
#if !TARGET_API_MAC_CARBON
       QDGlobals qd;
#endif
       OSType _ftype;
       OSType _fcreator;
#endif



#if 0

/*
 * The Angband Color Set (0 to 15):
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 *
 * On the Macintosh, we use color quickdraw, and we use actual "RGB"
 * values below to choose the 16 colors.
 *
 * If we are compiled for ancient machines, we bypass color and simply
 * draw everything in white (letting "z-term.c" automatically convert
 * "black" into "wipe" calls).
 */
static RGBColor foo[16] =
{
	{0x0000, 0x0000, 0x0000},	/* TERM_DARK */
	{0xFFFF, 0xFFFF, 0xFFFF},	/* TERM_WHITE */
	{0x8080, 0x8080, 0x8080},	/* TERM_SLATE */
	{0xFFFF, 0x8080, 0x0000},	/* TERM_ORANGE */
	{0xC0C0, 0x0000, 0x0000},	/* TERM_RED */
	{0x0000, 0x8080, 0x4040},	/* TERM_GREEN */
	{0x0000, 0x0000, 0xFFFF},	/* TERM_BLUE */
	{0x8080, 0x4040, 0x0000},	/* TERM_UMBER */
	{0x4040, 0x4040, 0x4040},	/* TERM_L_DARK */
	{0xC0C0, 0xC0C0, 0xC0C0},	/* TERM_L_WHITE */
	{0xFFFF, 0x0000, 0xFFFF},	/* TERM_VIOLET */
	{0xFFFF, 0xFFFF, 0x0000},	/* TERM_YELLOW */
	{0xFFFF, 0x0000, 0x0000},	/* TERM_L_RED */
	{0x0000, 0xFFFF, 0x0000},	/* TERM_L_GREEN */
	{0x0000, 0xFFFF, 0xFFFF},	/* TERM_L_BLUE */
	{0xC0C0, 0x8080, 0x4040}	/* TERM_L_UMBER */
};

#endif


/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term		*t;

	Rect		r;

	WindowPtr	w;

#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

	short padding;

	short pixelDepth;

	GWorldPtr theGWorld;

	GDHandle theGDH;

	GDHandle mainSWGDH;

#endif /* ANGBAND_LITE_MAC */

	Str15		title;

	s16b		oops;

	s16b		keys;

	s16b		last;

	s16b		mapped;

	s16b		rows;
	s16b		cols;

	s16b		font_id;
	s16b		font_size;
	s16b		font_face;
	s16b		font_mono;

	s16b		font_o_x;
	s16b		font_o_y;
	s16b		font_wid;
	s16b		font_hgt;

	s16b		tile_o_x;
	s16b		tile_o_y;
	s16b		tile_wid;
	s16b		tile_hgt;

	s16b		size_wid;
	s16b		size_hgt;

	s16b		size_ow1;
	s16b		size_oh1;
	s16b		size_ow2;
	s16b		size_oh2;
};




/*
 * Forward declare -- see below
 */
static bool CheckEvents(bool wait);

#ifndef MACH_O_CARBON

/*
 * Hack -- location of the main directory
 */
static short app_vol;
static long  app_dir;

#endif /* !MACH_O_CARBON */

/*
 * Delay handling of double-clicked savefiles
 */
Boolean open_when_ready = FALSE;

/*
 * Delay handling of pre-emptive "quit" event
 */
Boolean quit_when_ready = FALSE;


/*
 * Hack -- game in progress
 */
static int game_in_progress = 0;


/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;



/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];



/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;

/*
 * Version of Mac OS - for version specific bug workarounds (; ;)
 */
static long mac_os_version;


#if defined(__MWERKS__)
/*
 * CodeWarrior uses Universal Procedure Pointers
 */
static ModalFilterUPP ynfilterUPP;

#endif /* __MWERKS__ */

#ifdef USE_SFL_CODE

/*
 * Apple Event Hooks
 */
AEEventHandlerUPP AEH_Start_UPP;
AEEventHandlerUPP AEH_Quit_UPP;
AEEventHandlerUPP AEH_Print_UPP;
AEEventHandlerUPP AEH_Open_UPP;

#endif

/*
 * Convert refnum+vrefnum+fname into a full file name
 * Store this filename in 'buf' (make sure it is long enough)
 * Note that 'fname' looks to be a "pascal" string
 */
#if TARGET_API_MAC_CARBON
static void refnum_to_name(char *buf, long refnum, short vrefnum, char *fname)
{
	CInfoPBRec pb;
	int err;
	int i, j;

	char res[1000];
	
	FSSpec spec;
	short	vref;
    long	dirID;
    
	i=999;

	res[i]=0; i--;
	for (j=1; j<=fname[0]; j++)
	{
		res[i-fname[0]+j] = fname[j];
	}
	i-=fname[0];

	vref = vrefnum;
	dirID = refnum;

	while (1)
	{
		pb.dirInfo.ioDrDirID=pb.dirInfo.ioDrParID;
		err = FSMakeFSSpec( vref, dirID, "\p", &spec );
		
		if( err != noErr )
		    break;
		
		res[i] = ':'; i--;
		for (j=1; j<=spec.name[0]; j++)
		{
			res[i-spec.name[0]+j] = spec.name[j];
		}
		i -= spec.name[0];
		
		dirID = spec.parID;
	}

	/* Extract the result */
	for (j = 0, i++; res[i]; j++, i++) buf[j] = res[i];
	buf[j] = 0;
}
#else
static void refnum_to_name(char *buf, long refnum, short vrefnum, char *fname)
{
	DirInfo pb;
	Str255 name;
	int err;
	int i, j;

	char res[1000];

	i=999;

	res[i]=0; i--;
	for (j=1; j<=fname[0]; j++)
	{
		res[i-fname[0]+j] = fname[j];
	}
	i-=fname[0];

	pb.ioCompletion=NULL;
	pb.ioNamePtr=name;
	pb.ioVRefNum=vrefnum;
	pb.ioDrParID=refnum;
	pb.ioFDirIndex=-1;

	while (1)
	{
		pb.ioDrDirID=pb.ioDrParID;
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);
		res[i] = ':'; i--;
		for (j=1; j<=name[0]; j++)
		{
			res[i-name[0]+j] = name[j];
		}
		i -= name[0];

		if (pb.ioDrDirID == fsRtDirID) break;
	}

	/* Extract the result */
	for (j = 0, i++; res[i]; j++, i++) buf[j] = res[i];
	buf[j] = 0;
}
#endif

#if TARGET_API_MAC_CARBON
pascal OSErr FSpLocationFromFullPath(short fullPathLength,
									 const void *fullPath,
									 FSSpec *spec)
{
	AliasHandle	alias;
	OSErr		result;
	Boolean		wasChanged;
	Str32		nullString;
	
	/* Create a minimal alias from the full pathname */
	nullString[0] = 0;	/* null string to indicate no zone or server name */
	result = NewAliasMinimalFromFullPath(fullPathLength, fullPath, nullString, nullString, &alias);
	if ( result == noErr )
	{
		/* Let the Alias Manager resolve the alias. */
		result = ResolveAlias(NULL, alias, spec, &wasChanged);
		
		/* work around Alias Mgr sloppy volume matching bug */
		if ( spec->vRefNum == 0 )
		{
			/* invalidate wrong FSSpec */
			spec->parID = 0;
			spec->name[0] =  0;
			result = nsvErr;
		}
		DisposeHandle((Handle)alias);	/* Free up memory used */
	}
	return ( result );
}
#endif

#if 0

/*
 * XXX XXX XXX Allow the system to ask us for a filename
 */
static bool askfor_file(char *buf, int len)
{
	SFReply reply;
	Str255 dflt;
	Point topleft;
	short vrefnum;
	long drefnum, junk;

	/* Default file name */
	sprintf((char*)dflt + 1, "%s's description", buf);
	dflt[0] = strlen((char*)dflt + 1);

	/* Ask for a file name */
	topleft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
	topleft.v=(2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;
	SFPutFile(topleft, "\pSelect a filename:", dflt, NULL, &reply);
	/* StandardPutFile("\pSelect a filename:", dflt, &reply); */

	/* Process */
	if (reply.good)
	{
		int fc;

		/* Get info */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);

		/* Extract the name */
		refnum_to_name(buf, drefnum, vrefnum, (char*)reply.fName);

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
}

#endif

#if !TARGET_API_MAC_CARBON
static void local_to_global( Rect *r )
{
	Point		temp;
	
	temp.h = r->left;
	temp.v = r->top;
	
	LocalToGlobal( &temp );
	
	r->left = temp.h;
	r->top = temp.v;
	
	temp.h = r->right;
	temp.v = r->bottom;
	
	LocalToGlobal( &temp );
	
	r->right = temp.h;
	r->bottom = temp.v;
}
#endif /* !TARGET_API_MAC_CARBON */

static void global_to_local( Rect *r )
{
	Point		temp;
	
	temp.h = r->left;
	temp.v = r->top;
	
	GlobalToLocal( &temp );
	
	r->left = temp.h;
	r->top = temp.v;
	
	temp.h = r->right;
	temp.v = r->bottom;
	
	GlobalToLocal( &temp );
	
	r->right = temp.h;
	r->bottom = temp.v;
}


#ifdef MAC_MPW

/*
 * Convert pathname to an appropriate format, because MPW's
 * CarbonStdCLib chose to use system's native path format,
 * making our lives harder to create binaries that run on
 * OS 8/9 and OS X :( -- pelpel
 */
void convert_pathname(char* path)
{
	char buf[1024];

	/* Nothing has to be done for CarbonLib on Classic */
	if (mac_os_version >= 0x1000)
	{
		/* Convert to POSIX style */
		ConvertHFSPathToUnixPath(path, buf);

		/* Copy the result back */
		strcpy(path, buf);
	}

	/* Done. */
	return;
}

# ifdef CHECK_MODIFICATION_TIME

/*
 * Although there is no easy way to emulate fstat in the old interface,
 * we still can do stat-like things, because Mac OS is an OS.
 */
static int get_modification_time(cptr path, u32b *mod_time)
{
	CInfoPBRec pb;
	Str255 pathname;
	int i;

	/* Paranoia - make sure the pathname fits in Str255 */
	i = strlen(path);
	if (i > 255) return (-1);

	/* Convert pathname to a Pascal string */
	strncpy((char *)pathname + 1, path, 255);
	pathname[0] = i;

	/* Set up parameter block */
	pb.hFileInfo.ioNamePtr = pathname;
	pb.hFileInfo.ioFDirIndex = 0;
	pb.hFileInfo.ioVRefNum = app_vol;
	pb.hFileInfo.ioDirID = 0;

	/* Get catalog information of the file */
	if (PBGetCatInfoSync(&pb) != noErr) return (-1);

	/* Set modification date and time */
	*mod_time = pb.hFileInfo.ioFlMdDat;

	/* Success */
	return (0);
}


/*
 * A (non-Mach-O) Mac OS version of check_modification_time, for those
 * compilers without good enough POSIX-compatibility libraries XXX XXX
 */
errr check_modification_date(int fd, cptr template_file)
{
#pragma unused(fd)
	u32b txt_stat, raw_stat;
	char *p;
	char fname[32];
	char buf[1024];

	/* Build the file name */
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, template_file);

	/* XXX XXX XXX */
	convert_pathname(buf);

	/* Obtain modification time */
	if (get_modification_time(buf, &txt_stat)) return (-1);

	/* XXX Build filename of the corresponding *.raw file */
	strnfmt(fname, sizeof(fname), "%s", template_file);

	/* Find last '.' */
	p = strrchr(fname, '.');

	/* Can't happen */
	if (p == NULL) return (-1);

	/* Substitute ".raw" for ".txt" */
	strcpy(p, ".raw");

	/* Build the file name of the raw file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, fname);

	/* XXX XXX XXX */
	convert_pathname(buf);

	/* Obtain modification time */
	if (get_modification_time(buf, &raw_stat)) return (-1);

	/* Ensure the text file is not newer than the raw file */
	if (txt_stat > raw_stat) return (-1);

	/* Keep using the current .raw file */
	return (0);
}

# endif /* CHECK_MODIFICATION_TIME */

#endif /* MAC_MPW */

/*
 * Center a rectangle inside another rectangle
 */
static void center_rect(Rect *r, Rect *s)
{
	int centerx = (s->left + s->right)/2;
	int centery = (2*s->top + s->bottom)/3;
	int dx = centerx - (r->right - r->left)/2 - r->left;
	int dy = centery - (r->bottom - r->top)/2 - r->top;
	r->left += dx;
	r->right += dx;
	r->top += dy;
	r->bottom += dy;
}


#ifdef MACH_O_CARBON

/* Carbon File Manager utilities by pelpel */

/*
 * (Carbon)
 * Convert a pathname to a corresponding FSSpec.
 * Returns noErr on success.
 */
static OSErr path_to_spec(const char *path, FSSpec *spec)
{
	OSErr err;
	FSRef ref;

	/* Convert pathname to FSRef ... */
	err = FSPathMakeRef(path, &ref, NULL);
	if (err != noErr) return (err);

	/* ... then FSRef to FSSpec */
	err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);
	
	/* Inform caller of success or failure */
	return (err);
}


/*
 * (Carbon)
 * Convert a FSSpec to a corresponding pathname.
 * Returns noErr on success.
 */
static OSErr spec_to_path(const FSSpec *spec, char *buf, size_t size)
{
	OSErr err;
	FSRef ref;

	/* Convert FSSpec to FSRef ... */
	err = FSpMakeFSRef(spec, &ref);
	if (err != noErr) return (err);

	/* ... then FSRef to pathname */
	err = FSRefMakePath(&ref, buf, size);

	/* Inform caller of success or failure */
	return (err);
}


/*
 * (Carbon) [via path_to_spec]
 * Set creator and filetype of a file specified by POSIX-style pathname.
 * Returns 0 on success, -1 in case of errors.
 */
void fsetfileinfo(cptr pathname, OSType fcreator, OSType ftype)
{
	OSErr err;
	FSSpec spec;
	FInfo info;

	/* Convert pathname to FSSpec */
	if (path_to_spec(pathname, &spec) != noErr) return;

	/* Obtain current finder info of the file */
	if (FSpGetFInfo(&spec, &info) != noErr) return;

	/* Overwrite creator and type */
	info.fdCreator = fcreator;
	info.fdType = ftype;
	err = FSpSetFInfo(&spec, &info);

	/* Done */
	return;
}


#else /* MACH_O_CARBON */


/*
 * Convert a pascal string in place
 *
 * This function may be defined elsewhere, but since it is so
 * small, it is not worth finding the proper function name for
 * all the different platforms.
 */
static void ptocstr(StringPtr src)
{
	int i;

	/* Hack -- pointer */
	char *s = (char*)(src);

	/* Hack -- convert the string */
	for (i = s[0]; i; i--, s++) s[0] = s[1];

	/* Hack -- terminate the string */
	s[0] = '\0';
}


#if defined(USE_SFL_CODE)


/*
 * The following three routines (pstrcat, pstrinsert, and PathNameFromDirID)
 * were taken from the Think Reference section called "Getting a Full Pathname"
 * (under the File Manager section).  We need PathNameFromDirID to get the
 * full pathname of the opened savefile, making no assumptions about where it
 * is.
 *
 * I had to hack PathNameFromDirID a little for MetroWerks, but it's awfully
 * nice.
 */
static void pstrcat(StringPtr dst, StringPtr src)
{
	/* copy string in */
	BlockMove(src + 1, dst + *dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

/*
 * pstrinsert - insert string 'src' at beginning of string 'dst'
 */
static void pstrinsert(StringPtr dst, StringPtr src)
{
	/* make room for new string */
	BlockMove(dst + 1, dst + *src + 1, *dst);

	/* copy new string in */
	BlockMove(src + 1, dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

static void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName)
{
	CInfoPBRec	block;
	Str255	directoryName;
	OSErr	err;

	fullPathName[0] = '\0';

	block.dirInfo.ioDrParID = dirID;
	block.dirInfo.ioNamePtr = directoryName;

	while (1)
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		err = PBGetCatInfo(&block, FALSE);
		pstrcat(directoryName, (StringPtr)"\p:");
		pstrinsert(fullPathName, directoryName);
		if (block.dirInfo.ioDrDirID == 2) break;
	}
}

#endif
#endif /* MACH_O_CARBON */

/*
 * Activate a given window, if necessary
 */
static void activate(WindowPtr w)
{
	/* Activate */
	if (active != w)
	{
		/* Activate */
#if TARGET_API_MAC_CARBON
		if (w) SetPortWindowPort(w);
#else
		if (w) SetPort(w);
#endif

		/* Remember */
		active = w;
	}
}


/*
 * Display a warning message
 */
static void mac_warning(cptr warning)
{
	Str255 text;
	int len, i;

	/* Limit of 250 chars */
	len = strlen(warning);
	if (len > 250) len = 250;

	/* Make a "Pascal" string */
	text[0] = len;
	for (i=0; i<len; i++) text[i+1] = warning[i];

	/* Prepare the dialog box values */
	ParamText(text, "\p", "\p", "\p");

	/* Display the Alert, wait for Okay */
	Alert(129, 0L);
}



/*** Some generic functions ***/


#ifdef ANGBAND_LITE_MAC

/*
 * Hack -- activate a color (0 to 255)
 */
#define term_data_color(TD,A) /* Nothing */

#else /* ANGBAND_LITE_MAC */

/*
 * Hack -- activate a color (0 to 255)
 */
static void term_data_color(term_data *td, int a)
{
	u16b rv, gv, bv;

	RGBColor color;

	/* Extract the R,G,B data */
	rv = angband_color_table[a][1];
	gv = angband_color_table[a][2];
	bv = angband_color_table[a][3];

	/* Set the color */
	color.red = (rv | (rv << 8));
	color.green = (gv | (gv << 8));
	color.blue = (bv | (bv << 8));

	/* Activate the color */
	RGBForeColor(&color);

	/* Memorize color */
	td->last = a;
}

#endif /* ANGBAND_LITE_MAC */


/*
 * Hack -- Apply and Verify the "font" info
 *
 * This should usually be followed by "term_data_check_size()"
 */
static void term_data_check_font(term_data *td)
{
	int i;

	FontInfo info;

	WindowPtr old = active;


	/* Activate */
	activate(td->w);

	/* Instantiate font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);

	/* Extract the font info */
	GetFontInfo(&info);

	/* Assume monospaced */
	td->font_mono = TRUE;

	/* Extract the font sizing values XXX XXX XXX */
	td->font_wid = CharWidth('@'); /* info.widMax; */
	td->font_hgt = info.ascent + info.descent;
	td->font_o_x = 0;
	td->font_o_y = info.ascent;

	/* Check important characters */
	for (i = 33; i < 127; i++)
	{
		/* Hack -- notice non-mono-space */
		if (td->font_wid != CharWidth(i)) td->font_mono = FALSE;

		/* Hack -- collect largest width */
		if (td->font_wid < CharWidth(i)) td->font_wid = CharWidth(i);
	}

	/* Set default offsets */
	td->tile_o_x = td->font_o_x;
	td->tile_o_y = td->font_o_y;

	/* Set default tile size */
	if( td->tile_wid == 0 && td->tile_hgt == 0 ){
		td->tile_wid = td->font_wid;
		td->tile_hgt = td->font_hgt;
	}

	/* Re-activate the old window */
	activate(old);
}


/*
 * Hack -- Apply and Verify the "size" info
 */
static void term_data_check_size(term_data *td)
{
	BitMap		screen;
	
#if TARGET_API_MAC_CARBON
	GetQDGlobalsScreenBits( &screen );
#else
	screen = qd.screenBits;
#endif
	/* Minimal window size */
	if (td == &data[0])
	{
		/* Enforce minimal size */
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;
	}

	/* Allow small windows for the rest */
	else
	{
		if (td->cols < 1) td->cols = 1;
		if (td->rows < 1) td->rows = 1;
	}

	/* Minimal tile size */
	if (td->tile_wid < 4) td->tile_wid = 4;
	if (td->tile_hgt < 4) td->tile_hgt = 4;

	/* Default tile offsets */
	td->tile_o_x = (td->tile_wid - td->font_wid) / 2;
	td->tile_o_y = (td->tile_hgt - td->font_hgt) / 2;

	/* Minimal tile offsets */
	if (td->tile_o_x < 0) td->tile_o_x = 0;
	if (td->tile_o_y < 0) td->tile_o_y = 0;

	/* Apply font offsets */
	td->tile_o_x += td->font_o_x;
	td->tile_o_y += td->font_o_y;

	/* Calculate full window size */
	td->size_wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	td->size_hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	/* Verify the top */
	if (td->r.top > screen.bounds.bottom - td->size_hgt)
	{
		td->r.top = screen.bounds.bottom - td->size_hgt;
	}

	/* Verify the top */
	if (td->r.top < screen.bounds.top + 30)
	{
		td->r.top = screen.bounds.top + 30;
	}

	/* Verify the left */
	if (td->r.left > screen.bounds.right - td->size_wid)
	{
		td->r.left = screen.bounds.right - td->size_wid;
	}

	/* Verify the left */
	if (td->r.left < screen.bounds.left)
	{
		td->r.left = screen.bounds.left;
	}

	/* Calculate bottom right corner */
	td->r.right = td->r.left + td->size_wid;
	td->r.bottom = td->r.top + td->size_hgt;

	/* Assume no graphics */
	td->t->higher_pict = FALSE;
	td->t->always_pict = FALSE;

#ifdef ANGBAND_LITE_MAC

	/* No graphics */

#else /* ANGBAND_LITE_MAC */

	/* Handle graphics */
	if (use_graphics)
	{
		/* Use higher_pict whenever possible */
		if (td->font_mono) td->t->higher_pict = TRUE;

		/* Use always_pict only when necessary */
		else td->t->always_pict = TRUE;
	}

#endif /* ANGBAND_LITE_MAC */

	/* Fake mono-space */
	if (!td->font_mono ||
	    (td->font_wid != td->tile_wid) ||
		(td->font_hgt != td->tile_hgt))
	{
		/* Handle fake monospace -- this is SLOW */
		if (td->t->higher_pict) td->t->higher_pict = FALSE;
		td->t->always_pict = TRUE;
	}
}

/*
 * Hack -- resize a term_data
 *
 * This should normally be followed by "term_data_resize()"
 */
static void term_data_resize(term_data *td)
{
	/* Actually resize the window */
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);
}



/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;

	/* Activate the term */
	Term_activate(td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();

	/* Restore the old term */
	Term_activate(old);
	
	/* No need to redraw */
#if TARGET_API_MAC_CARBON
	{
		RgnHandle		theRgn = NewRgn();
		GetWindowRegion( td->w, kWindowContentRgn, theRgn );
		ValidWindowRgn( (WindowRef)(td->w), theRgn );
		DisposeRgn( theRgn );
	}
#else
	ValidRect(&td->w->portRect);
#endif

}




#ifdef ANGBAND_LITE_MAC

/* No graphics */

#else /* ANGBAND_LITE_MAC */


/*
 * Graphics support
 */

/*
 * PICT id of image tiles, set by Term_xtra_mac_react
 */
#ifdef MACH_O_CARBON
static CFStringRef pictID = NULL;
#else
static int pictID = 0;
#endif /* MACH_O_CARBON */

/*
 * Width and height of a tile in pixels
 */
static int grafWidth = 0;
static int grafHeight = 0;

/*
 * Numbers of rows and columns in tiles, calculated by
 * the PICT loading code
 */
static int pictCols = 0;
static int pictRows = 0;

/*
 * Available graphics modes - 32x32 tiles don't work on Classic
 */
#define GRAF_MODE_NONE	0
#define GRAF_MODE_8X8	1
#define GRAF_MODE_16X16	2
#define GRAF_MODE_32X32	3

/*
 * Current and requested graphics modes
 */
static int graf_mode = GRAF_MODE_NONE;
static int graf_mode_req = GRAF_MODE_NONE;


/*
 * Forward Declare
 */
typedef struct FrameRec FrameRec;

/*
 * Frame
 *
 *	- GWorld for the frame image
 *	- Handle to pix map (saved for unlocking/locking)
 *	- Pointer to color pix map (valid only while locked)
 */
struct FrameRec
{
	GWorldPtr 		framePort;
	PixMapHandle 	framePixHndl;
	PixMapPtr 		framePix;
	
};


/*
 * The global picture data
 */
static FrameRec *frameP = NULL;


/*
 * Lock a frame
 */
static void BenSWLockFrame(FrameRec *srcFrameP)
{
	PixMapHandle 		pixMapH;

	pixMapH = GetGWorldPixMap(srcFrameP->framePort);
	(void)LockPixels(pixMapH);
	HLockHi((Handle)pixMapH);
	srcFrameP->framePixHndl = pixMapH;
#if TARGET_API_MAC_CARBON
	srcFrameP->framePix = (PixMapPtr)*(Handle)pixMapH;
#else
	srcFrameP->framePix = (PixMapPtr)StripAddress(*(Handle)pixMapH);
#endif
	
}


/*
 * Unlock a frame
 */
static void BenSWUnlockFrame(FrameRec *srcFrameP)
{
	if (srcFrameP->framePort != NULL)
	{
		HUnlock((Handle)srcFrameP->framePixHndl);
		UnlockPixels(srcFrameP->framePixHndl);
	}

	srcFrameP->framePix = NULL;
	
}

#ifdef MACH_O_CARBON

/*
 * Moving graphics resources into data fork -- pelpel
 *
 * (Carbon, Bundle)
 * Given base and type names of a resource, find a file in the
 * current application bundle and return its FSSpec in the third argument.
 * Returns true on success, false otherwise.
 * e.g. get_resource_spec(CFSTR("8x8"), CFSTR("png"), &spec);
 */
static Boolean get_resource_spec(
	CFStringRef base_name, CFStringRef type_name, FSSpec *spec)
{
	CFURLRef res_url;
	FSRef ref;

	/* Find resource (=file) in the current bundle */
	res_url = CFBundleCopyResourceURL(
		CFBundleGetMainBundle(), base_name, type_name, NULL);

	/* Oops */
	if (res_url == NULL) return (false);

	/* Convert CFURL to FSRef */
	(void)CFURLGetFSRef(res_url, &ref);

	/* Convert FSRef to FSSpec */
	(void)FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);

	/* Free allocated CF data */
	CFRelease(res_url);

	/* Success */
	return (true);
}


/*
 * (QuickTime)
 * Create a off-screen GWorld from contents of a file specified by a FSSpec.
 * Based on BenSWCreateGWorldFromPict.
 *
 * Globals referenced: data[0], graf_height, graf_width
 * Globals updated: pict_rows, pict_cols.
 */
static OSErr create_gworld_from_spec(
	GWorldPtr *tile_gw, FSSpec *tile_spec)
{
	OSErr err;
	GraphicsImportComponent gi;
	GWorldPtr gw, tmp_gw;
	GDHandle gdh, tmp_gdh;
	Rect r;
	SInt16 depth;

	/* See if QuickTime understands the file format */
	err = GetGraphicsImporterForFile(tile_spec, &gi);

	/* Oops */
	if (err != noErr) return (err);

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	gdh = data[0].theGDH;

	/* Retrieve the rect of the image */
	err = GraphicsImportGetNaturalBounds(gi, &r);

	/* Adjust it, so that the upper left corner becomes (0, 0) */
	OffsetRect(&r, -r.left, -r.top);

	/* Calculate and set numbers of rows and columns */
	pictRows = r.bottom / grafHeight;
	pictCols = r.right / grafWidth;

	/* Create a GWorld */
	err = NewGWorld(&gw, depth, &r, NULL, gdh, noNewDevice);

	/* Oops */
	if (err != noErr) return (err);

	/* Save the pointer to the GWorld */
	*tile_gw = gw;

	/* Save the current GWorld */
	GetGWorld(&tmp_gw, &tmp_gdh);

	/* Activate the newly created GWorld */
	(void)GraphicsImportSetGWorld(gi, gw, NULL);

	/* Prevent pixmap from moving while drawing */
	(void)LockPixels(GetGWorldPixMap(gw));

	/* Clear the pixels */
	EraseRect(&r);

	/* Draw the image into it */
	(void)GraphicsImportDraw(gi);

	/* Release the lock*/
	UnlockPixels(GetGWorldPixMap(gw));

	/* Restore GWorld */
	SetGWorld(tmp_gw, tmp_gdh);

	/* Close the image importer */
	CloseComponent(gi);

	/* Success */
	return (noErr);
}

#else /* MACH_O_CARBON */

static OSErr BenSWCreateGWorldFromPict(
	GWorldPtr *pictGWorld,
	PicHandle pictH)
{
	OSErr err;
	GWorldPtr saveGWorld;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	Rect pictRect;
	short depth;
	GDHandle theGDH;

	/* Reset */
	*pictGWorld = NULL;

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	theGDH = data[0].theGDH;

	/* Obtain size rectangle */
	pictRect = (**pictH).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);

	/* Create a GWorld */
	err = NewGWorld(&tempGWorld, depth, &pictRect, nil, 
					theGDH, noNewDevice);

	/* Success */
	if (err != noErr)
	{
		return (err);
	}

	/* Save pointer */
	*pictGWorld = tempGWorld;

	/* Save GWorld */
	GetGWorld(&saveGWorld, &saveGDevice);

	/* Activate */
	SetGWorld(tempGWorld, nil);

	/* Dump the pict into the GWorld */
	(void)LockPixels(GetGWorldPixMap(tempGWorld));
	EraseRect(&pictRect);
	DrawPicture(pictH, &pictRect);
	UnlockPixels(GetGWorldPixMap(tempGWorld));

	/* Restore GWorld */
	SetGWorld(saveGWorld, saveGDevice);
	
	/* Success */
	return (0);
}

#endif /* MACH_O_CARBON */


/*
 * Init the global "frameP"
 */

static errr globe_init(void)
{
	OSErr err;
	
	GWorldPtr tempPictGWorldP;

#ifdef MACH_O_CARBON
	FSSpec pict_spec;
#else
	PicHandle newPictH;
#endif /* MACH_O_CARBON */

	/* Use window XXX XXX XXX */
#if TARGET_API_MAC_CARBON
	SetPortWindowPort(data[0].w);
#else
	SetPort(data[0].w);
#endif


#ifdef MACH_O_CARBON

	/* Get the tile resources */
	if (!get_resource_spec(pictID, CFSTR("png"), &pict_spec)) return (-1);

	/* Create GWorld */
	err = create_gworld_from_spec(&tempPictGWorldP, &pict_spec);

	/* Error */
	if (err != noErr) return (err);

	/* Create the frame */
	frameP = (FrameRec*)NewPtrClear((Size)sizeof(FrameRec));

	/* Analyze result */
	if (frameP == NULL)
	{
		/* Dispose of image GWorld */
		DisposeGWorld(tempPictGWorldP);

		/* Fake error code */
		return (-1);
	}

	/* Save GWorld */
	frameP->framePort = tempPictGWorldP;

	/* Lock it */
	BenSWLockFrame(frameP);

#else /* MACH_O_CARBON */

	/* Get the pict resource */
	newPictH = GetPicture(pictID);

	/* Analyze result */
	err = (newPictH ? 0 : -1);

	/* Oops */
	if (err == noErr)
	{

		/* Create GWorld */
		err = BenSWCreateGWorldFromPict(&tempPictGWorldP, newPictH);
		
		/* Release resource */
		ReleaseResource((Handle)newPictH);

		/* Error */
		if (err == noErr)
		{
			/* Create the frame */
			frameP = (FrameRec*)NewPtrClear((Size)sizeof(FrameRec));

			/* Analyze result */
			err = (frameP ? 0 : -1);

			/* Oops */
			if (err == noErr)
			{
				/* Save GWorld */
				frameP->framePort = tempPictGWorldP;

				/* Lock it */
				BenSWLockFrame(frameP);
			}
		}
	}
#endif /* MACH_O_CARBON */


	/* Result */
	return (err);
}


/*
 * Nuke the global "frameP"
 */
static errr globe_nuke(void)
{
	/* Dispose */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);

		/* Forget */
		frameP = NULL;
	}

	/* Flush events */	
	FlushEvents(everyEvent, 0);

	/* Success */
	return (0);
}


# ifdef USE_ASYNC_SOUND

/*
 * Asynchronous sound player revised
 */
#if defined(USE_QT_SOUND) && !defined(MACH_O_CARBON)
# undef USE_QT_SOUND
#endif /* USE_QT_SOUND && !MACH_O_CARBON */


/*
 * Number of channels in the channel pool
 */
#if TARGET_API_MAC_CARBON
#define MAX_CHANNELS		8
#else
#define MAX_CHANNELS		4
#endif

/*
 * A pool of sound channels
 */
static SndChannelPtr channels[MAX_CHANNELS];

/*
 * Status of the channel pool
 */
static Boolean channel_initialised = FALSE;

/*
 * Data handles containing sound samples
 */
static SndListHandle samples[SOUND_MAX];

/*
 * Reference counts of sound samples
 */
static SInt16 sample_refs[SOUND_MAX];


/*
 * Sound effects
 *
 * These constants aren't used by the program at the moment.
 */
#define SOUND_VOLUME_MIN	0	/* Default minimum sound volume */
#define SOUND_VOLUME_MAX	255	/* Default maximum sound volume */
#define VOLUME_MIN			0	/* Minimum sound volume in % */
#define VOLUME_MAX			100	/* Maximum sound volume in % */
#define VOLUME_INC			5	/* Increment sound volume in % */

/* I'm just too lazy to write a panel for this XXX XXX */
static SInt16 sound_volume = SOUND_VOLUME_MAX;

#ifdef USE_QT_SOUND

/*
 * QuickTime sound, by Ron Anderson
 *
 * I didn't choose to use Windows-style .ini files (Ron wrote a parser
 * for it, but...), nor did I use lib/xtra directory, hoping someone
 * would code plist-based configuration code in the future -- pelpel
 */

/*
 * (QuickTime)
 * Load sound effects from data-fork resources.  They are wav files
 * with the same names as angband_sound_name[] (variable.c)
 *
 * Globals referenced: angband_sound_name[]
 * Globals updated: samples[] (they can be *huge*)
 */
static void load_sounds(void)
{
	OSErr err;
	int i;

	/* Start QuickTime */
	err = EnterMovies();

	/* Error */
	if (err != noErr) return;

	/*
	 * This loop may take a while depending on the count and size of samples
	 * to load.
	 *
	 * We should use a progress dialog for this.
	 */
	for (i = 1; i < SOUND_MAX; i++)
	{
		/* Apple APIs always give me headacke :( */
		CFStringRef name;
		FSSpec spec;
		SInt16 file_id;
		SInt16 res_id;
		Str255 movie_name;
		Movie movie;
		Track track;
		Handle h;
		Boolean res;

		/* Allocate CFString with the name of sound event to be processed */
		name = CFStringCreateWithCString(NULL, angband_sound_name[i],
			kTextEncodingUS_ASCII);

		/* Error */
		if (name == NULL) continue;

		/* Find sound sample resource with the same name */
		res = get_resource_spec(name, CFSTR("wav"), &spec);

		/* Free the reference to CFString */
		CFRelease(name);

		/* Error */
		if (!res) continue;

		/* Open the sound file */
		err = OpenMovieFile(&spec, &file_id, fsRdPerm);

		/* Error */
		if (err != noErr) continue;

		/* Create Movie from the file */
		err = NewMovieFromFile(&movie, file_id, &res_id, movie_name,
			newMovieActive, NULL);

		/* Error */
		if (err != noErr) goto close_file;

		/* Get the first track of the movie */
		track = GetMovieIndTrackType(movie, 1, AudioMediaCharacteristic,
			movieTrackCharacteristic | movieTrackEnabledOnly );

		/* Error */
		if (track == NULL) goto close_movie;

		/* Allocate a handle to store sample */
		h = NewHandle(0);

		/* Error */
		if (h == NULL) goto close_track;

		/* Dump the sample into the handle */
		err = PutMovieIntoTypedHandle(movie, track, soundListRsrc, h, 0,
			GetTrackDuration(track), 0L, NULL);

		/* Success */
		if (err == noErr)
		{
			/* Store the handle in the sample list */
			samples[i] = (SndListHandle)h;
		}

		/* Failure */
		else
		{
			/* Free unused handle */
			DisposeHandle(h);
		}

		/* Free the track */
		close_track: DisposeMovieTrack(track);

		/* Free the movie */
		close_movie: DisposeMovie(movie);

		/* Close the movie file */
		close_file: CloseMovieFile(file_id);
	}

	/* Stop QuickTime */
	ExitMovies();
}

#else /* USE_QT_SOUND */

/*
 * Return a handle of 'snd ' resource given Angband sound event number,
 * or NULL if it isn't found.
 *
 * Globals referenced: angband_sound_name[] (variable.c)
 */
static SndListHandle find_sound(int num)
{
	Str255 sound;

	/* Get the proper sound name */
	strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[num]);
	sound[0] = strlen((char*)sound + 1);

	/* Obtain resource XXX XXX XXX */
	return ((SndListHandle)GetNamedResource('snd ', sound));
}

#endif /* USE_QT_SOUND */


/*
 * Clean up sound support - to be called when the game exits.
 *
 * Globals referenced: channels[], samples[], sample_refs[].
 */
static void cleanup_sound(void)
{
	int i;

	/* No need to clean it up */
	if (!channel_initialised) return;

	/* Dispose channels */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		/* Drain sound commands and free the channel */
		SndDisposeChannel(channels[i], TRUE);
	}

	/* Free sound data */
	for (i = 1; i < SOUND_MAX; i++)
	{
		/* Still locked */
		if ((sample_refs[i] > 0) && (samples[i] != NULL))
		{
			/* Unlock it */
			HUnlock((Handle)samples[i]);
		}

#ifndef USE_QT_SOUND

		/* Release it */
		if (samples[i]) ReleaseResource((Handle)samples[i]);
#else

		/* Free handle */
		if (samples[i]) DisposeHandle((Handle)samples[i]);
#endif /* !USE_QT_SOUND */
	}
}


/*
 * Play sound effects asynchronously -- pelpel
 *
 * I don't believe those who first started using the previous implementations
 * imagined this is *much* more complicated as it may seem.  Anyway, 
 * introduced round-robin scheduling of channels and made it much more
 * paranoid about HLock/HUnlock.
 *
 * XXX XXX de-refcounting, HUnlock and ReleaseResource should be done
 * using channel's callback procedures, which set global flags, and
 * a procedure hooked into CheckEvents does housekeeping.  On the other
 * hand, this lazy reclaiming strategy keeps things simple (no interrupt
 * time code) and provides a sort of cache for sound data.
 *
 * Globals referenced: channel_initialised, channels[], samples[],
 *   sample_refs[].
 * Globals updated: ditto.
 */
static void play_sound(int num, SInt16 vol)
{
	OSErr err;
	int i;
	int prev_num;
	SndListHandle h;
	SndChannelPtr chan;
	SCStatus status;

	static int next_chan;
	static SInt16 channel_occupants[MAX_CHANNELS];
	static SndCommand volume_cmd, quiet_cmd;


	/* Initialise sound channels */
	if (!channel_initialised)
	{
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			/* Paranoia - Clear occupant table */
			/* channel_occupants[i] = 0; */

			/* Create sound channel for all sounds to play from */
			err = SndNewChannel(&channels[i], sampledSynth, initMono, 0L);

			/* Error */
			if (err != noErr)
			{
				/* Free channels */
				while (--i >= 0)
				{
					SndDisposeChannel(channels[i], TRUE);
				}

				/* Notify error */
				plog("Cannot initialise sound channels!");

				/* Cancel request */
				use_sound = arg_sound = FALSE;

				/* Failure */
				return;
			}
		}

		/* First channel to use */
		next_chan = 0;

		/* Prepare volume command */
		volume_cmd.cmd = volumeCmd;
		volume_cmd.param1 = 0;
		volume_cmd.param2 = 0;

		/* Prepare quiet command */
		quiet_cmd.cmd = quietCmd;
		quiet_cmd.param1 = 0;
		quiet_cmd.param2 = 0;

		/* Initialisation complete */
		channel_initialised = TRUE;
	}

	/* Paranoia */
	if ((num <= 0) || (num >= SOUND_MAX)) return;

	/* Prepare volume command */
	volume_cmd.param2 = ((SInt32)vol << 16) | vol;

	/* Channel to use (round robin) */
	chan = channels[next_chan];

	/* See if the resource is already in use */
	if (sample_refs[num] > 0)
	{
		/* Resource in use */
		h = samples[num];

		/* Increase the refcount */
		sample_refs[num]++;
	}

	/* Sound is not currently in use */
	else
	{
		/* Get handle for the sound */
#ifdef USE_QT_SOUND
		h = samples[num];
#else
		h = find_sound(num);
#endif /* USE_QT_SOUND */

		/* Sample not available */
		if (h == NULL) return;

#ifndef USE_QT_SOUND

		/* Load resource */
		LoadResource((Handle)h);

		/* Remember it */
		samples[num] = h;

#endif /* !USE_QT_SOUND */

		/* Lock the handle */
		HLockHi((Handle)h);

		/* Initialise refcount */
		sample_refs[num] = 1;
	}

	/* Poll the channel */
	err = SndChannelStatus(chan, sizeof(SCStatus), &status);

	/* It isn't available */
	if ((err != noErr) || status.scChannelBusy)
	{
		/* Shut it down */
		SndDoImmediate(chan, &quiet_cmd);
	}

	/* Previously played sound on this channel */
	prev_num = channel_occupants[next_chan];

	/* Process previously played sound */
	if (prev_num != 0)
	{
		/* Decrease refcount */
		sample_refs[prev_num]--;

		/* We can free it now */
		if (sample_refs[prev_num] <= 0)
		{
			/* Unlock */
			HUnlock((Handle)samples[prev_num]);

#ifndef USE_QT_SOUND

			/* Release */
			ReleaseResource((Handle)samples[prev_num]);

			/* Forget handle */
			samples[prev_num] = NULL;

#endif /* !USE_QT_SOUND */

			/* Paranoia */
			sample_refs[prev_num] = 0;
		}
	}

	/* Remember this sound as the current occupant of the channel */
	channel_occupants[next_chan] = num;

	/* Set up volume for channel */
	SndDoImmediate(chan, &volume_cmd);

	/* Play new sound asynchronously */
	SndPlay(chan, h, TRUE);

	/* Schedule next channel (round robin) */
	next_chan++;
	if (next_chan >= MAX_CHANNELS) next_chan = 0;
}

# else /* USE_ASYNC_SOUND */

/*
 * Play sound synchronously
 *
 * This may not be your choice, but much safer and much less resource hungry.
 */
static void play_sound(int num, SInt16 vol)
{
	Handle handle;
	Str255 sound;

	/* Get the proper sound name */
	strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[num]);
	sound[0] = strlen((char*)sound + 1);

	/* Obtain resource XXX XXX XXX */
	handle = GetNamedResource('snd ', sound);

	/* Oops */
	if (handle == NULL) return;

	/* Load and Lock */
	LoadResource(handle);
	HLockHi(handle);

	/* Play sound (wait for completion) */
	SndPlay(NULL, (SndListHandle)handle, FALSE);

	/* Unlock and release */
	HUnlock(handle);
	ReleaseResource(handle);
}

# endif /* USE_ASYNC_SOUND */

/*
	Extra Sound Mode
*/


static short soundmode[8];

#define		SND_NON		0
#define		SND_ATTACK	1
#define		SND_MOVE		2
#define		SND_TRAP		3
#define		SND_SHOP		4
#define		SND_ME		5
#define		SND_CMD_ERROR	6

#ifndef MACH_O_CARBON

static int soundchoice[] = {
	SND_NON,
	SND_ATTACK,
	SND_ATTACK,
	SND_ATTACK,
	SND_TRAP,
	SND_ATTACK,
	SND_ME,
	SND_ME,
	SND_ME,
	SND_MOVE,
	SND_ATTACK,
	SND_ME,
	SND_ATTACK,
	SND_NON,
	SND_MOVE,
	SND_MOVE,
	SND_ME,
	SND_SHOP,
	SND_SHOP,
	SND_SHOP,
	SND_SHOP,
	SND_MOVE,
	SND_MOVE,
	SND_MOVE,
	SND_MOVE,
	SND_ATTACK,
	SND_SHOP,
	SND_SHOP,
	SND_ME,
	SND_NON,
	SND_ATTACK,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_ATTACK,
	SND_ATTACK,
	SND_NON,
	SND_NON,
	SND_ATTACK,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_TRAP,
	SND_ATTACK,
	SND_ATTACK,
	SND_ATTACK,
	SND_ATTACK,
	SND_ATTACK,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_NON,
	SND_CMD_ERROR,
	SND_TRAP,
	SND_NON,
	SND_NON,
	SND_TRAP,
	SND_ATTACK,
	SND_TRAP,
	SND_ATTACK,
	SND_ATTACK,
	SND_NON,
	SND_TRAP,
};

static int ext_sound = 0;
static int ext_graf = 0;

#endif /* !MACH_O_CARBON */

#endif /* ANGBAND_LITE_MAC */



/*** Support for the "z-term.c" package ***/


/*
 * Initialize a new Term
 *
 * Note also the "window type" called "noGrowDocProc", which might be more
 * appropriate for the main "screen" window.
 *
 * Note the use of "srcCopy" mode for optimized screen writes.
 */
static void Term_init_mac(term *t)
{
	term_data *td = (term_data*)(t->data);

	static RGBColor black = {0x0000,0x0000,0x0000};
	static RGBColor white = {0xFFFF,0xFFFF,0xFFFF};

#ifdef ANGBAND_LITE_MAC

	/* Make the window */
	td->w = NewWindow(0, &td->r, td->title, 0, noGrowDocProc, (WindowPtr)-1, 1, 0L);

#else /* ANGBAND_LITE_MAC */

	/* Make the window */
	td->w = NewCWindow(0, &td->r, td->title, 0, documentProc, (WindowPtr)-1, 1, 0L);

#endif /* ANGBAND_LITE_MAC */

	/* Activate the window */
	activate(td->w);

	/* Erase behind words */
	TextMode(srcCopy);

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);

	/* Resize the window */
	term_data_resize(td);

#ifdef ANGBAND_LITE_MAC

	/* Prepare the colors (base colors) */
	BackColor(blackColor);
	ForeColor(whiteColor);

#else /* ANGBAND_LITE_MAC */

	/* Prepare the colors (real colors) */
	RGBBackColor(&black);
	RGBForeColor(&white);

	/* Block */
	{
		Rect globalRect;
		GDHandle mainGDH;
		GDHandle currentGDH;
		GWorldPtr windowGWorld;
		PixMapHandle basePixMap;

		/* Obtain the rect */
#if TARGET_API_MAC_CARBON
		GetWindowBounds( (WindowRef)td->w, kWindowContentRgn, &globalRect );
#else
		globalRect = td->w->portRect;
		LocalToGlobal((Point*)&globalRect.top);
		LocalToGlobal((Point*)&globalRect.bottom);
#endif

		/* Obtain the proper GDH */
		mainGDH = GetMaxDevice(&globalRect);

		/* Extract GWorld and GDH */
		GetGWorld(&windowGWorld, &currentGDH);

		/* Obtain base pixmap */
		basePixMap = (**mainGDH).gdPMap;

		/* Save pixel depth */
		td->pixelDepth = (**basePixMap).pixelSize;

		/* Save Window GWorld */
		td->theGWorld = windowGWorld;

		/* Save Window GDH */
		td->theGDH = currentGDH;

		/* Save main GDH */
		td->mainSWGDH = mainGDH;
	}

#endif /* ANGBAND_LITE_MAC */

	{
		Rect		portRect;

#if TARGET_API_MAC_CARBON
		GetWindowBounds( (WindowRef)td->w, kWindowContentRgn, &portRect );
		global_to_local( &portRect );
#else
		portRect = td->w->portRect;
#endif
		/* Clip to the window */
		ClipRect(&portRect);

		/* Erase the window */
		EraseRect(&portRect);

		/* Invalidate the window */
#if TARGET_API_MAC_CARBON
		InvalWindowRect((WindowRef)(td->w), (const Rect *)(&portRect));
#else
		InvalRect(&portRect);
#endif

		/* Display the window if needed */
		if (td->mapped) ShowWindow(td->w);

		/* Hack -- set "mapped" flag */
		t->mapped_flag = td->mapped;

		/* Forget color */
		td->last = -1;
	}
	/* Oops */
/*	if (err == noErr)
	{
		
	}*/
}



/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{

#pragma unused (t)

	/* XXX */
}



/*
 * Unused
 */
static errr Term_user_mac(int n)
{

#pragma unused (n)

	/* Success */
	return (0);
}



/*
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	term_data *td = (term_data*)(Term->data);


	/* Reset color */
	td->last = -1;

#ifdef ANGBAND_LITE_MAC

	/* Nothing */
	
#else /* ANGBAND_LITE_MAC */

	/* Handle sound */
	if (use_sound != arg_sound)
	{
		/* Apply request */
		use_sound = arg_sound;
	}

	
	/* Handle graphics */
	if (graf_mode_req != graf_mode)
	{
		/* dispose old GWorld's if present */
		globe_nuke();

		/* Setup parameters according to request */
		switch (graf_mode_req)
		{
			/* ASCII - no graphics whatsoever */
			case GRAF_MODE_NONE:
			{
				use_graphics = arg_graphics = GRAPHICS_NONE;
				break;
			}

			/*
			 * 8x8 tiles (PICT id 1001)
			 * no transparency effect
			 * "old" graphics definitions
			 */
			case GRAF_MODE_8X8:
			{
				use_graphics = arg_graphics = GRAPHICS_ORIGINAL;
				ANGBAND_GRAF = "old";
#ifdef MACH_O_CARBON
				pictID = CFSTR("8x8");
#else
				pictID = 1001;
#endif /* MACH_O_CARBON */
				grafWidth = grafHeight = 8;
				break;
			}

			/*
			 * 16x16 tiles (images: PICT id 1002)
			 * with transparency effect
			 * "new" graphics definitions
			 */
			case GRAF_MODE_16X16:
			{
				use_graphics = arg_graphics = GRAPHICS_ADAM_BOLT;
				ANGBAND_GRAF = "new";
#ifdef MACH_O_CARBON
				pictID = CFSTR("16x16");
#else
				pictID = 1002;
#endif /* MACH_O_CARBON */
				grafWidth = grafHeight = 16;
				break;
			}
		}

		if ((graf_mode_req != GRAF_MODE_NONE) && !frameP && (globe_init() != 0))
		{
			plog("Cannot initialize graphics!");

			/* reject request */
			graf_mode_req = GRAF_MODE_NONE;

			/* reset graphics flags */
			use_graphics = arg_graphics = FALSE;

		}

		/* update current graphics mode */
		graf_mode = graf_mode_req;

		/* Apply and Verify */
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		/* Reset visuals */
		reset_visuals();
	}

#endif /* ANGBAND_LITE_MAC */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	Rect r;

	/* Analyze */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		{
			/* Make a noise */
			SysBeep(1);

			/* Success */
			return (0);
		}

#ifdef ANGBAND_LITE_MAC

		/* Nothing */

#else /* ANGBAND_LITE_MAC */

		/* Make a sound */
		case TERM_XTRA_SOUND:
		{
			/* Play sound */
			play_sound(v, sound_volume);

			/* Success */
			return (0);
		}

#endif /* ANGBAND_LITE_MAC */

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			/* Process an event */
			(void)CheckEvents(FALSE);

			/* Success */
			return (0);
		}

		/* Process pending events */
		case TERM_XTRA_EVENT:
		{
			/* Process an event */
			(void)CheckEvents(v);

			/* Success */
			return (0);
		}

		/* Flush all pending events (if any) */
		case TERM_XTRA_FLUSH:
		{
			/* Hack -- flush all events */
			while (CheckEvents(TRUE)) /* loop */;

			/* Success */
			return (0);
		}

		/* Hack -- Change the "soft level" */
		case TERM_XTRA_LEVEL:
		{
			/* Activate if requested */
			if (v) activate(td->w);

			/* Success */
			return (0);
		}

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			Rect		portRect;
			
#if TARGET_API_MAC_CARBON
			GetWindowBounds( (WindowRef)td->w, kWindowContentRgn, &portRect );
			global_to_local( &portRect );
#else
			portRect = td->w->portRect;
#endif

			/* No clipping XXX XXX XXX */
			ClipRect(&portRect);

			/* Erase the window */
			EraseRect(&portRect);

			/* Set the color */
			term_data_color(td, TERM_WHITE);

			/* Frame the window in white */
			MoveTo(0, 0);
			LineTo(0, td->size_hgt-1);
			LineTo(td->size_wid-1, td->size_hgt-1);
			LineTo(td->size_wid-1, 0);

			/* Clip to the new size */
			r.left = portRect.left + td->size_ow1;
			r.top = portRect.top + td->size_oh1;
			r.right = portRect.right - td->size_ow2;
			r.bottom = portRect.bottom - td->size_oh2;
			ClipRect(&r);

			/* Success */
			return (0);
		}

		/* React to changes */
		case TERM_XTRA_REACT:
		{
			/* React to changes */
			return (Term_xtra_mac_react());
		}

		/* Delay (milliseconds) */
		case TERM_XTRA_DELAY:
		{
			/* If needed */
			if (v > 0)
			{
#if TARGET_API_MAC_CARBON
				EventRecord tmp;
				UInt32 ticks;

				/* Convert millisecs to ticks */
				ticks = (v * 60L) / 1000;

				/*
				 * Hack? - Put the programme into sleep.
				 * No events match ~everyEvent, so nothing
				 * should be lost in Angband's event queue.
				 * Even if ticks are 0, it's worth calling for
				 * the above mentioned reasons.
				 */
				WaitNextEvent((EventMask)~everyEvent, &tmp, ticks, nil);
#else
				long m = TickCount() + (v * 60L) / 1000;

				/* Wait for it */
				while (TickCount() < m) /* loop */;
#endif
			}

			/* Success */
			return (0);
		}
	}

	/* Oops */
	return (1);
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_grab()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_curs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;

	FrameRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input).
 * Draw a "big cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_grab()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_bigcurs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + 2 * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;

	FrameRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Erase the block of characters */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + n * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	EraseRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_mac(int x, int y, int n, byte a, const char *cp)
{
	int xp, yp;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, (a & COLOR_MASK));

	/* Starting pixel */
	xp = x * td->tile_wid + td->tile_o_x + td->size_ow1;
	yp = y * td->tile_hgt + td->tile_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*cp);

	/* Draw the string */
	else DrawText(cp, 0, n);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp,
			  const byte *tap, const char *tcp)
{
	int i;
	Rect r2;
	term_data *td = (term_data*)(Term->data);
	GDHandle saveGDevice;
	GWorldPtr saveGWorld;
	
	/* Save GWorld */
	GetGWorld(&saveGWorld, &saveGDevice);
	
	r2.left = x * td->tile_wid + td->size_ow1;
	r2.right = r2.left + td->tile_wid;
	r2.top = y * td->tile_hgt + td->size_oh1;
	r2.bottom = r2.top + td->tile_hgt;
	
	if( n > 1 )
	{
		/* Instantiate font */
		TextFont(td->font_id);
		TextSize(td->font_size);
		TextFace(td->font_face);
		
		/* Restore colors */
		BackColor(blackColor);
		ForeColor(whiteColor);
	}
	else
	{
		/* Destination rectangle */
/*		r2.left = x * td->tile_wid + td->size_ow1;
		r2.top = y * td->tile_hgt + td->size_oh1;
		r2.bottom = r2.top + td->tile_hgt;*/
		
	}
		
	/* Scan the input */
	for (i = 0; i < n; i++)
	{
		bool done = FALSE;

		byte a = ap[i];
		char c = cp[i];

		/* Second byte of bigtile */
		if (use_bigtile && a == 255)
		{
			/* Advance */
			r2.left += td->tile_wid;

			continue;
		}

		/* Prepare right of rectangle now */
		r2.right = r2.left + td->tile_wid;

#ifdef ANGBAND_LITE_MAC

		/* No graphics */

#else /* ANGBAND_LITE_MAC */

		/* Graphics -- if Available and Needed */
		if (use_graphics && ((byte)a & 0x80) && ((byte)c & 0x80))
		{
#if TARGET_API_MAC_CARBON
			PixMapHandle	srcBitMap = GetGWorldPixMap(frameP->framePort);
			PixMapHandle	destBitMap;
#else
			BitMapPtr srcBitMap = (BitMapPtr)(frameP->framePix);
			BitMapPtr destBitMap;
#endif
				
			int col, row;
			Rect r1;

			Rect terrain_r;
			bool terrain_flag = FALSE;
			byte ta = tap[i];
			char tc = tcp[i];

			if ((a != ta || c != tc) &&
			    ((byte)ta & 0x80) && ((byte)tc & 0x80))
			{
				/* Row and Col */
				row = ((byte)ta & 0x7F);
				col = ((byte)tc & 0x7F);

				/* Terrain Source rectangle */
				terrain_r.left = col * grafWidth;
				terrain_r.top = row * grafHeight;
				terrain_r.right = terrain_r.left + grafWidth;
				terrain_r.bottom = terrain_r.top + grafHeight;

				terrain_flag = TRUE;
			}

			/* Row and Col */
			row = ((byte)a & 0x7F);
			col = ((byte)c & 0x7F);
			
			/* Source rectangle */
			r1.left = col * grafWidth;
			r1.top = row * grafHeight;
			r1.right = r1.left + grafWidth;
			r1.bottom = r1.top + grafHeight;

			/* Hardwire CopyBits */
			BackColor(whiteColor);
			ForeColor(blackColor);

			/* Draw the picture */
#if TARGET_API_MAC_CARBON
			destBitMap = GetPortPixMap(GetWindowPort( td->w ));
#else
			destBitMap = (BitMapPtr)&(td->w->portBits);
#endif
			if (use_bigtile) r2.right += td->tile_wid;

			if (terrain_flag)
			{
				/*
				 * Source mode const = srcCopy:
				 *
				 * determine how close the color of the source
				 * pixel is to black, and assign this relative
				 * amount of foreground color to the
				 * destination pixel; determine how close the
				 * color of the source pixel is to white, and
				 * assign this relative amount of background
				 * color to the destination pixel
				 */
#if TARGET_API_MAC_CARBON
				CopyBits( (BitMap *) *srcBitMap, (BitMap *) *destBitMap, &terrain_r, &r2, srcCopy, NULL);
#else
				CopyBits( srcBitMap, destBitMap, &terrain_r, &r2, srcCopy, NULL );
#endif
				/*
				 * Draw transparent tile
				 * BackColor is ignored and the destination is
				 * left untouched
				 */
				BackColor(blackColor);
#if TARGET_API_MAC_CARBON
				CopyBits( (BitMap *) *srcBitMap, (BitMap *) *destBitMap, &r1, &r2, transparent, NULL);
#else
				CopyBits( srcBitMap, destBitMap, &r1, &r2, transparent, NULL );
#endif
			}
			else
			{
#if TARGET_API_MAC_CARBON
				CopyBits( (BitMap *) *srcBitMap, (BitMap *) *destBitMap, &r1, &r2, srcCopy, NULL);
#else
				CopyBits( srcBitMap, destBitMap, &r1, &r2, srcCopy, NULL );
#endif
			}

			/* Restore colors */
			BackColor(blackColor);
			ForeColor(whiteColor);

			/* Forget color */
			td->last = -1;

			/* Done */
			done = TRUE;
		}

#endif /* ANGBAND_LITE_MAC */

		/* Normal */
		if (!done)
		{
			int xp, yp;

			/* Set the color */
			term_data_color(td, (a & COLOR_MASK));
			
			/* Starting pixel */
			xp = r2.left + td->tile_o_x;
			yp = r2.top + td->tile_o_y;
			
			/* Move to the correct location */
			MoveTo(xp, yp);

			{
				/* Erase */
				EraseRect(&r2);

				/* Draw the character */
				DrawChar(c);
			}
		}

		/* Advance */
		r2.left += td->tile_wid;
	}
		
	/* Success */
	return (0);
}


/*
 * Create and initialize window number "i"
 */
static void term_data_link(int i)
{
	term *old = Term;

	term_data *td = &data[i];

	/* Only once */
	if (td->t) return;

	/* Require mapped */
	if (!td->mapped) return;

	/* Allocate */
	MAKE(td->t, term);

	/* Initialize the term */
	term_init(td->t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	td->t->soft_cursor = TRUE;

	/* Erase with "white space" */
	td->t->attr_blank = TERM_WHITE;
	td->t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	td->t->init_hook = Term_init_mac;
	td->t->nuke_hook = Term_nuke_mac;

	/* Prepare the function hooks */
	td->t->user_hook = Term_user_mac;
	td->t->xtra_hook = Term_xtra_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->curs_hook = Term_curs_mac;
	td->t->bigcurs_hook = Term_bigcurs_mac;
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;

	/* Link the local structure */
	td->t->data = (vptr)(td);

	/* Activate it */
	Term_activate(td->t);

	/* Global pointer */
	angband_term[i] = td->t;

	/* Activate old */
	Term_activate(old);
}



#ifdef MACH_O_CARBON

/*
 * (Carbon, Bundle)
 * Return a POSIX pathname of the lib directory, or NULL if it can't be
 * located.  Caller must supply a buffer along with its size in bytes,
 * where returned pathname will be stored.
 * I prefer use of goto's to several nested if's, if they involve error
 * handling.  Sorry if you are offended by their presence.  Modern
 * languages have neater constructs for this kind of jobs -- pelpel
 */
static char *locate_lib(char *buf, size_t size)
{
	CFURLRef main_url = NULL;
	CFStringRef main_str = NULL;
	char *p;
	char *res = NULL;

	/* Obtain the URL of the main bundle */
	main_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());

	/* Oops */
	if (main_url == NULL) goto ret;

	/* Convert it to POSIX pathname */
	main_str = CFURLCopyFileSystemPath(main_url, kCFURLPOSIXPathStyle);

	/* Oops */
	if (main_str == NULL) goto ret;

	/* Convert it again from darn unisomething encoding to ASCII */
	if (CFStringGetCString(main_str, buf, size, kTextEncodingUS_ASCII) == FALSE)
		goto ret;

	/* Find the last '/' in the pathname */
	p = strrchr(buf, '/');

	/* Paranoia - cannot happen */
	if (p == NULL) goto ret;

	/* Remove the trailing path */
	*p = '\0';

	/*
	 * Paranoia - bounds check, with 5 being the length of "/lib/"
	 * and 1 for terminating '\0'.
	 */
	if (strlen(buf) + 5 + 1 > size) goto ret;

	/* Append "/lib/" */
	strcat(buf, "/lib/");

	/* Set result */
	res = buf;

ret:

	/* Release objects allocated and implicitly retained by the program */
	if (main_str) CFRelease(main_str);
	if (main_url) CFRelease(main_url);

	/* pathname of the lib folder or NULL */
	return (res);
}


#else /* MACH_O_CARBON */


/*
 * Set the "current working directory" (also known as the "default"
 * volume/directory) to the location of the current application.
 *
 * Code by: Maarten Hazewinkel (mmhazewi@cs.ruu.nl)
 *
 * This function does not appear to work correctly with System 6.
 */
static void SetupAppDir(void)
{
	FCBPBRec fcbBlock;
	OSErr err = noErr;
	char errString[100];

	/* Get the location of the Angband executable */
	fcbBlock.ioCompletion = NULL;
	fcbBlock.ioNamePtr = NULL;
	fcbBlock.ioVRefNum = 0;
	fcbBlock.ioRefNum = CurResFile();
	fcbBlock.ioFCBIndx = 0;
	err = PBGetFCBInfo(&fcbBlock, FALSE);
	if (err != noErr)
	{
		sprintf(errString, "Fatal PBGetFCBInfo Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}

	/* Extract the Vol and Dir */
	app_vol = fcbBlock.ioFCBVRefNum;
	app_dir = fcbBlock.ioFCBParID;

	/* Set the current working directory to that location */
	err = HSetVol(NULL, app_vol, app_dir);
	if (err != noErr)
	{
		sprintf(errString, "Fatal HSetVol Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}
}

#endif



#if TARGET_API_MAC_CARBON
/*
 * Using Core Foundation's Preferences services -- pelpel
 *
 * Requires OS 8.6 or greater with CarbonLib 1.1 or greater. Or OS X,
 * of course.
 *
 * Without this, we can support older versions of OS 8 as well
 * (with CarbonLib 1.0.4).
 *
 * Frequent allocation/deallocation of small chunks of data is
 * far from my liking, but since this is only called at the
 * beginning and the end of a session, I hope this hardly matters.
 */


/*
 * Store "value" as the value for preferences item name
 * pointed by key
 */
static void save_pref_short(const char *key, short value)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* allocate and initialise the value */
	cf_value = CFNumberCreate(NULL, kCFNumberShortType, &value);

	if ((cf_key != NULL) && (cf_value != NULL))
	{
		/* Store the key-value pair in the applications preferences */
		CFPreferencesSetAppValue(
			cf_key,
			cf_value,
			kCFPreferencesCurrentApplication);
	}

	/*
	 * Free CF data - the reverse order is a vain attempt to
	 * minimise memory fragmentation.
	 */
	if (cf_value) CFRelease(cf_value);
	if (cf_key) CFRelease(cf_key);
}


/*
 * Load preference value for key, returns TRUE if it succeeds with
 * vptr updated appropriately, FALSE otherwise.
 */
static bool query_load_pref_short(const char *key, short *vptr)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* Oops */
	if (cf_key == NULL) return (FALSE);

	/* Retrieve value for the key */
	cf_value = CFPreferencesCopyAppValue(
		cf_key,
		kCFPreferencesCurrentApplication);

	/* Value not found */
	if (cf_value == NULL)
	{
		CFRelease(cf_key);
		return (FALSE);
	}

	/* Convert the value to short */
	CFNumberGetValue(
		cf_value,
		kCFNumberShortType,
		vptr);

	/* Free CF data */
	CFRelease(cf_value);
	CFRelease(cf_key);

	/* Success */
	return (TRUE);
}


/*
 * Update short data pointed by vptr only if preferences
 * value for key is located.
 */
static void load_pref_short(const char *key, short *vptr)
{
	short tmp;

	if (query_load_pref_short(key, &tmp)) *vptr = tmp;
	return;
}


/*
 * Save preferences to preferences file for current host+current user+
 * current application.
 */
static void cf_save_prefs()
{
	int i;

	/* Version stamp */
	save_pref_short("version.major", FAKE_VERSION);
	save_pref_short("version.minor", FAKE_VER_MAJOR);
	save_pref_short("version.patch", FAKE_VER_MINOR);
	save_pref_short("version.extra", FAKE_VER_PATCH);

	/* Gfx settings */
	save_pref_short("arg.arg_sound", arg_sound);
	save_pref_short("arg.graf_mode", graf_mode);
	save_pref_short("arg.arg_bigtile", arg_bigtile);

#ifndef MACH_O_CARBON
	/* SoundMode */
	for( i = 0 ; i < 7 ; i++ )
		save_pref_short(format("sound%d.on", i), soundmode[i]);
#endif /* MACH_O_CARBON */

	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		save_pref_short(format("term%d.mapped", i), td->mapped);

		save_pref_short(format("term%d.font_id", i), td->font_id);
		save_pref_short(format("term%d.font_size", i), td->font_size);
		save_pref_short(format("term%d.font_face", i), td->font_face);

		save_pref_short(format("term%d.tile_wid", i), td->tile_wid);
		save_pref_short(format("term%d.tile_hgt", i), td->tile_hgt);

		save_pref_short(format("term%d.cols", i), td->cols);
		save_pref_short(format("term%d.rows", i), td->rows);
		save_pref_short(format("term%d.left", i), td->r.left);
		save_pref_short(format("term%d.top", i), td->r.top);
	}

	/*
	 * Make sure preferences are persistent
	 */
	CFPreferencesAppSynchronize(
		kCFPreferencesCurrentApplication);
}


/*
 * Load preferences from preferences file for current host+current user+
 * current application.
 */
static void cf_load_prefs()
{
	bool ok;
	short pref_major, pref_minor, pref_patch, pref_extra;
	int i;

	/* Assume nothing is wrong, yet */
	ok = TRUE;

	/* Load version information */
	ok &= query_load_pref_short("version.major", &pref_major);
	ok &= query_load_pref_short("version.minor", &pref_minor);
	ok &= query_load_pref_short("version.patch", &pref_patch);
	ok &= query_load_pref_short("version.extra", &pref_extra);

	/* Any of the above failed */
	if (!ok)
	{
		/* This may be the first run */
		mac_warning("Preferences are not found.");

		/* Ignore the rest */
		return;
	}

	/* Gfx settings */
	{
		short pref_tmp;

		/* sound */
		if (query_load_pref_short("arg.arg_sound", &pref_tmp))
			arg_sound = pref_tmp;

		/* graphics */
		if (query_load_pref_short("arg.graf_mode", &pref_tmp))
			graf_mode_req = pref_tmp;

		/* double-width tiles */
		if (query_load_pref_short("arg.arg_bigtile", &pref_tmp))
		{
			arg_bigtile = use_bigtile = pref_tmp;
		}

	}

#ifndef MACH_O_CARBON
	/* SoundMode */
	for( i = 0 ; i < 7 ; i++ )
	{
		query_load_pref_short(format("sound%d.on", i), &soundmode[i]);
	}
#endif /* MACH_O_CARBON */

	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		load_pref_short(format("term%d.mapped", i), &td->mapped);

		load_pref_short(format("term%d.font_id", i), &td->font_id);
		load_pref_short(format("term%d.font_size", i), &td->font_size);
		load_pref_short(format("term%d.font_face", i), &td->font_face);

		load_pref_short(format("term%d.tile_wid", i), &td->tile_wid);
		load_pref_short(format("term%d.tile_hgt", i), &td->tile_hgt);

		load_pref_short(format("term%d.cols", i), &td->cols);
		load_pref_short(format("term%d.rows", i), &td->rows);
		load_pref_short(format("term%d.left", i), &td->r.left);
		load_pref_short(format("term%d.top", i), &td->r.top);
	}
}

#else
/*
 * Global "preference" file pointer
 */
static FILE *fff;

/*
 * Read a "short" from the file
 */
static int getshort(void)
{
	int x = 0;
	char buf[256];
	if (0 == my_fgets(fff, buf, sizeof(buf))) x = atoi(buf);
	return (x);
}

/*
 * Dump a "short" to the file
 */
static void putshort(int x)
{
	fprintf(fff, "%d\n", x);
}



/*
 * Write the "preference" data to the current "file"
 */
static void save_prefs(void)
{
	int i;

	term_data *td;


	/*** The current version ***/

	putshort(FAKE_VERSION);
	putshort(FAKE_VER_MAJOR);
	putshort(FAKE_VER_MINOR);
	putshort(FAKE_VER_PATCH);

	putshort(arg_sound);
	putshort(graf_mode);
	putshort(arg_bigtile);
	
	/* SoundMode */
	for( i = 0 ; i < 7 ; i++ )
		putshort(soundmode[i]);
	
	/* Dump */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		putshort(td->mapped);

		putshort(td->font_id);
		putshort(td->font_size);
		putshort(td->font_face);

		putshort(td->tile_wid);
		putshort(td->tile_hgt);

		putshort(td->cols);
		putshort(td->rows);

		putshort(td->r.left);
		putshort(td->r.top);
	}
}


/*
 * Load the preferences from the current "file"
 *
 * XXX XXX XXX Being able to undefine various windows is
 * slightly bizarre, and may cause problems.
 */
static void load_prefs(void)
{
	int i;

	int old_version, old_major, old_minor, old_patch;

	term_data *td;
	MenuHandle m;

	/*** Version information ***/

	/* Preferences version */
	old_version = getshort();
	old_major = getshort();
	old_minor = getshort();
	old_patch = getshort();

	/* Hack -- Verify or ignore */
	if ((old_version != FAKE_VERSION) ||
	    (old_major != FAKE_VER_MAJOR) ||
	    (old_minor != FAKE_VER_MINOR) ||
	    (old_patch != FAKE_VER_PATCH))
	{
		/* Message */
		mac_warning("Ignoring old preferences.");
		/* Ignore */
		return;
	}

	arg_sound = getshort();
	graf_mode_req = getshort();
	arg_bigtile = getshort();
	use_bigtile = arg_bigtile;
	
	/* SoundMode */
	for( i = 0 ; i < 7 ; i++ )
		soundmode[i] = getshort();
	
	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		td->mapped = getshort();

		td->font_id = getshort();
		td->font_size = getshort();
		td->font_face = getshort();

		td->tile_wid = getshort();
		td->tile_hgt = getshort();

		td->cols = getshort();
		td->rows = getshort();

		td->r.left = getshort();
		td->r.top = getshort();

		/* Done */
		if (feof(fff)) break;
	}
}
#endif /* TARGET_API_MAC_CARBON */



/*
 * Hack -- default data for a window
 */
static void term_data_hack(term_data *td)
{
	short fid;

#if TARGET_API_MAC_CARBON
	/* Default to Monaco font */
	fid = FMGetFontFamilyFromName("\pmonaco");
#else
	/* Default to Monaco font */
	GetFNum("\pmonaco", &fid);
#endif

	/* Wipe it */
	WIPE(td, term_data);

	/* No color */
	td->last = -1;

	/* Default borders */
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh2 = 2;

	/* Start hidden */
	td->mapped = FALSE;

	/* Default font */
	td->font_id = fid;

	/* Default font size */
	td->font_size = 12;

	/* Default font face */
	td->font_face = 0;

	/* Default size */
	td->rows = 24;
	td->cols = 80;

	/* Default position */
	td->r.left = 10;
	td->r.top = 40;

	/* Minimal keys */
	td->keys = 16;
}


/*
 * Read the preference file, Create the windows.
 *
 * We attempt to use "FindFolder()" to track down the preference file,
 * but if this fails, for any reason, we will try the "SysEnvirons()"
 * method, which may work better with System 6.
 */
static void init_windows(void)
{
	int i, b = 0;

	term_data *td;

#if !TARGET_API_MAC_CARBON

	SysEnvRec env;
	short savev;
	long saved;

	bool oops;

#endif /* !TARGET_API_MAC_CARBON */


	/*** Default values ***/

	/* Initialize (backwards) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		int n;

		cptr s;

		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Obtain title */
		s = angband_term_name[i];

		/* Get length */
		n = strlen(s);

		/* Maximal length */
		if (n > 15) n = 15;

		/* Copy the title */
		strncpy((char*)(td->title) + 1, s, n);

		/* Save the length */
		td->title[0] = n;

		/* Tile the windows */
		td->r.left += (b * 30);
		td->r.top += (b * 30);

		/* Tile */
		b++;
	}


	/*** Load preferences ***/
	
#if TARGET_API_MAC_CARBON
	cf_load_prefs();
#else
	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short	vref;
		long	dirID;
		char	foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
				 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, PREF_FILE_NAME);

			/* Open the preference file */
			fff = fopen(foo, "r");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

	/* Oops */
	if (oops)
	{
		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to the "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the file */
		fff = fopen(PREF_FILE_NAME, "r");

		/* Restore */
		HSetVol(0, savev, saved);
	}

	/* Load preferences */
	if (fff)
	{
		/* Load a real preference file */
		load_prefs();

		/* Close the file */
		my_fclose(fff);
	}
#endif /* TARGET_API_MAC_CARBON */


	/*** Instantiate ***/

	/* Main window */
	td = &data[0];

	/* Many keys */
	td->keys = 1024;

	/* Start visible */
	td->mapped = TRUE;

	/* Link (backwards, for stacking order) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		term_data_link(i);
	}

	/* Main window */
	td = &data[0];

	/* Main window */
	Term_activate(td->t);
}

#ifndef MACH_O_CARBON

static void init_sound( void )
{
	int err, i;
	CInfoPBRec pb;
	SignedByte		permission = fsRdPerm;
	pascal short	ret;
	
	Handle handle;
	Str255 sound;

	/* Descend into "lib" folder */
	pb.dirInfo.ioCompletion = NULL;
	pb.dirInfo.ioNamePtr = "\plib";
	pb.dirInfo.ioVRefNum = app_vol;
	pb.dirInfo.ioDrDirID = app_dir;
	pb.dirInfo.ioFDirIndex = 0;

	/* Check for errors */
	err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

	/* Success */
	if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
	{
		/* Descend into "lib/save" folder */
		pb.dirInfo.ioCompletion = NULL;
		pb.dirInfo.ioNamePtr = "\pxtra";
		pb.dirInfo.ioVRefNum = app_vol;
		pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrDirID;
		pb.dirInfo.ioFDirIndex = 0;

		/* Check for errors */
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);
			
			/* Success */
		if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
		{
			/* Descend into "lib/save" folder */
			pb.dirInfo.ioCompletion = NULL;
			pb.dirInfo.ioNamePtr = "\psound";
			pb.dirInfo.ioVRefNum = app_vol;
			pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrDirID;
			pb.dirInfo.ioFDirIndex = 0;

			/* Check for errors */
			err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

			/* Success */
			if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
			{
				ret = HOpenResFile( app_vol , pb.dirInfo.ioDrDirID , "\psound.rsrc" , permission );
				if( ret != -1 ){
					ext_sound = 1;
					
					for( i = 0 ; i < 7 ; i++ )
							soundmode[i] = false;
					
					for( i = 1 ; i < SOUND_MAX ; i++ ){
						/* Get the proper sound name */
						sprintf((char*)sound + 1, "%.16s.wav", angband_sound_name[i]);
						sound[0] = strlen((char*)sound + 1);

						/* Obtain resource XXX XXX XXX */
						handle = Get1NamedResource('snd ', sound);
						if( handle == NULL || ext_sound )
							handle = GetNamedResource('snd ', sound);
						
						if( handle )
							soundmode[soundchoice[i]] = true;
						
					}
				}
			}
		}
	}
}

static void init_graf( void )
{
	int err, i;
	CInfoPBRec pb;
	SignedByte		permission = fsRdPerm;
	pascal short	ret;
	
	Handle handle;
	Str255 graf;

	/* Descend into "lib" folder */
	pb.dirInfo.ioCompletion = NULL;
	pb.dirInfo.ioNamePtr = "\plib";
	pb.dirInfo.ioVRefNum = app_vol;
	pb.dirInfo.ioDrDirID = app_dir;
	pb.dirInfo.ioFDirIndex = 0;

	/* Check for errors */
	err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

	/* Success */
	if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
	{
		/* Descend into "lib/xtra" folder */
		pb.dirInfo.ioCompletion = NULL;
		pb.dirInfo.ioNamePtr = "\pxtra";
		pb.dirInfo.ioVRefNum = app_vol;
		pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrDirID;
		pb.dirInfo.ioFDirIndex = 0;

		/* Check for errors */
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);
			
		/* Success */
		if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
		{
			/* Descend into "lib/xtra/graf" folder */
			pb.dirInfo.ioCompletion = NULL;
			pb.dirInfo.ioNamePtr = "\pgraf";
			pb.dirInfo.ioVRefNum = app_vol;
			pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrDirID;
			pb.dirInfo.ioFDirIndex = 0;

			/* Check for errors */
			err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

			/* Success */
			if ((err == noErr) && (pb.dirInfo.ioFlAttrib & 0x10))
			{
				ret = HOpenResFile( app_vol , pb.dirInfo.ioDrDirID , "\pgraf.rsrc" , permission );
				if (ret != -1)
				{
					ext_graf = 1;

					/* Obtain resource XXX XXX XXX */
					handle = Get1NamedResource('PICT', graf);
					if ( handle == NULL || ext_graf )
						handle = GetNamedResource('PICT', "\pgraf.rsrc");
				}
			}
		}
	}
}

#endif /* MACH_O_CARBON */


/*

*/
short InevrtCheck( DialogPtr targetDlg, short check )
{
	Handle 	checkH;
	short 	itemType;
	long	result;
	Rect	box;
	
	GetDialogItem( targetDlg, check, &itemType, &checkH, &box );
	result = (GetControlValue( (ControlHandle)checkH ) + 1 ) % 2;
	SetControlValue( (ControlHandle)checkH , result );
	return result ;

}

/*

*/
short SetCheck( DialogPtr targetDlg, short check, long result )
{
	Handle 	checkH;
	short 	itemType;

	Rect	box;
	
	GetDialogItem( targetDlg, check, &itemType, &checkH, &box );
	SetControlValue( (ControlHandle)checkH , result );
	return result ;

}

/*

*/
short GetCheck( DialogPtr targetDlg, short check )
{
	Handle 	checkH;
	short 	itemType;
	long	result;
	Rect	box;
	
	GetDialogItem( targetDlg, check, &itemType, &checkH, &box );
	result = GetControlValue( (ControlHandle)checkH );
	return result ;

}
void SoundConfigDLog(void)
{
	DialogPtr dialog;
	short item_hit;
	int	i;

	dialog=GetNewDialog(131, 0, (WindowPtr)-1);
	SetDialogDefaultItem( dialog, ok );
	SetDialogCancelItem( dialog, cancel );
	for( i = 1 ; i < 7 ; i++ )
		SetCheck( dialog, i+2 , soundmode[i] );
	
	/* ShowWindow(dialog); */ 
	for( item_hit = 100 ; cancel < item_hit ; ){
		ModalDialog(0, &item_hit);
		
		switch(item_hit){
			case ok:
				for( i = 1 ; i < 7 ; i++ )
					soundmode[i] = GetCheck( dialog, i+2 );
				break;
			case cancel:
				break;
			default:
				InevrtCheck( dialog, item_hit );
		}
	}
	DisposeDialog(dialog);

}


/*
 * Exit the program
 */
#if TARGET_API_MAC_CARBON
static void save_pref_file(void)
{
	cf_save_prefs();
}
#else
static void save_pref_file(void)
{
	bool oops;

	SysEnvRec env;
	short savev;
	long saved;


	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

	/* Text file */
	_ftype = 'TEXT';


#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short	vref;
		long	dirID;
		char	foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
				 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, PREF_FILE_NAME);

			/* Open the preference file */
			/* my_fopen set file type and file creator for MPW */
			fff = my_fopen(foo, "w");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

	/* Oops */
	if (oops)
	{
		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the preference file */
		/* my_fopen set file type and file creator for MPW */
		fff = fopen(PREF_FILE_NAME, "w");

		/* Restore */
		HSetVol(0, savev, saved);
	}

	/* Save preferences */
	if (fff)
	{
		/* Write the preferences */
		save_prefs();

		/* Close it */
		my_fclose(fff);
	}
}
#endif



#if defined(__MWERKS__)

/*
 * A simple "Yes/No" filter to parse "key press" events in dialog windows
 */
static pascal Boolean ynfilter(DialogPtr dialog, EventRecord *event, short *ip)
{
	/* Parse key press events */
	if (event->what == keyDown)
	{
		int i = 0;
		char c;

		/* Extract the pressed key */
		c = (event->message & charCodeMask);

		/* Accept "no" and <return> and <enter> */
		if ((c=='n') || (c=='N') || (c==13) || (c==3)) i = 1;

		/* Accept "yes" */
		else if ((c=='y') || (c=='Y')) i = 2;

		/* Handle "yes" or "no" */
		if (i)
		{
			short type;
			ControlHandle control;
			Rect r;

			/* Get the button */
			GetDialogItem(dialog, i, &type, (Handle*)&control, &r);

			/* Blink button for 1/10 second */
			HiliteControl(control, 1);
			Term_xtra_mac(TERM_XTRA_DELAY, 100);
			HiliteControl(control, 0);

			/* Result */
			*ip = i;
			return (1);
		}
	}

	/* Ignore */
	return (0);
}

#endif /* __MWERKS__ */


#if TARGET_API_MAC_CARBON

/*
 * Prepare savefile dialogue and set the variable
 * savefile accordingly. Returns true if it succeeds, false (or
 * aborts) otherwise. If all is false, only allow files whose type
 * is 'SAVE'.
 * Originally written by Peter Ammon
 */
static bool select_savefile(bool all)
{
	OSErr err;
	FSSpec theFolderSpec;
	FSSpec savedGameSpec;
	NavDialogOptions dialogOptions;
	NavReplyRecord reply;
	/* Used only when 'all' is true */
	NavTypeList types = {ANGBAND_CREATOR, 1, 1, {'SAVE'}};
	NavTypeListHandle myTypeList;
	AEDesc defaultLocation;

#ifdef MACH_O_CARBON

	/* Find the save folder */
	err = path_to_spec(ANGBAND_DIR_SAVE, &theFolderSpec);

#else

	/* Find :lib:save: folder */
	err = FSMakeFSSpec(app_vol, app_dir, "\p:lib:save:", &theFolderSpec);

#endif

	/* Oops */
	if (err != noErr) quit("Unable to find the folder :lib:save:");

	/* Get default Navigator dialog options */
	err = NavGetDefaultDialogOptions(&dialogOptions);

	/* Clear preview option */
	dialogOptions.dialogOptionFlags &= ~kNavAllowPreviews;

	/* Disable multiple file selection */
	dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;

	/* Make descriptor for default location */
	err = AECreateDesc(typeFSS, &theFolderSpec, sizeof(FSSpec),
		&defaultLocation);

	/* Oops */
	if (err != noErr) quit("Unable to allocate descriptor");

	/* We are indifferent to signature and file types */
	if (all)
	{
		myTypeList = (NavTypeListHandle)nil;
	}

	/* Set up type handle */
	else
	{
		err = PtrToHand(&types, (Handle *)&myTypeList, sizeof(NavTypeList));

		/* Oops */
		if (err != noErr) quit("Error in PtrToHand. Try enlarging heap");

	}

	/* Call NavGetFile() with the types list */
	err = NavChooseFile(&defaultLocation, &reply, &dialogOptions, NULL,
		NULL, NULL, myTypeList, NULL);

	/* Free type list */
	if (!all) DisposeHandle((Handle)myTypeList);

	/* Error */
	if (err != noErr)
	{
		/* Nothing */
	}

	/* Invalid response -- allow the user to cancel */
	else if (!reply.validRecord)
	{
		/* Hack -- Fake error */
		err = -1;
	}

	/* Retrieve FSSpec from the reply */
	else
	{
		AEKeyword theKeyword;
		DescType actualType;
		Size actualSize;

		/* Get a pointer to selected file */
		(void)AEGetNthPtr(&reply.selection, 1, typeFSS, &theKeyword,
			&actualType, &savedGameSpec, sizeof(FSSpec), &actualSize);

		/* Dispose NavReplyRecord, resources and descriptors */
		(void)NavDisposeReply(&reply);
	}

	/* Dispose location info */
	AEDisposeDesc(&defaultLocation);

	/* Error */
	if (err != noErr) return (FALSE);

#ifdef MACH_O_CARBON

	/* Convert FSSpec to pathname and store it in variable savefile */
	(void)spec_to_path(&savedGameSpec, savefile, sizeof(savefile));

#else

	/* Convert FSSpec to pathname and store it in variable savefile */
	refnum_to_name(
		savefile,
		savedGameSpec.parID,
		savedGameSpec.vRefNum,
		(char *)savedGameSpec.name);

#endif

	/* Success */
	return (TRUE);
}
#endif


/*
 * Handle menu: "File" + "New"
 */
static void do_menu_file_new(void)
{
	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(TRUE);

	/* Hack -- quit */
	quit(NULL);
}


/*
 * Handle menu: "File" + "Open"
 */
#if TARGET_API_MAC_CARBON
static void do_menu_file_open(bool all)
{
	/* Let the player to choose savefile */
	if (!select_savefile(all)) return;

	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = TRUE;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(FALSE);

	/* Hack -- quit */
	quit(NULL);
}
#else
static void do_menu_file_open(bool all)
{
	int err;
	short vrefnum;
	long drefnum;
	long junk;
	DirInfo pb;
	SFTypeList types;
	SFReply reply;
	Point topleft;


	/* XXX XXX XXX */

	/* vrefnum = GetSFCurVol(); */
	vrefnum = -*((short*)0x214);

	/* drefnum = GetSFCurDir(); */
	drefnum = *((long*)0x398);

	/* Descend into "lib" folder */
	pb.ioCompletion = NULL;
	pb.ioNamePtr = "\plib";
	pb.ioVRefNum = vrefnum;
	pb.ioDrDirID = drefnum;
	pb.ioFDirIndex = 0;

	/* Check for errors */
	err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

	/* Success */
	if ((err == noErr) && (pb.ioFlAttrib & 0x10))
	{
		/* Descend into "lib/save" folder */
		pb.ioCompletion = NULL;
		pb.ioNamePtr = "\psave";
		pb.ioVRefNum = vrefnum;
		pb.ioDrDirID = pb.ioDrDirID;
		pb.ioFDirIndex = 0;

		/* Check for errors */
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

		/* Success */
		if ((err == noErr) && (pb.ioFlAttrib & 0x10))
		{
			/* SetSFCurDir(pb.ioDrDirID); */
			*((long*)0x398) = pb.ioDrDirID;
		}
	}

	/* Window location */
	topleft.h = (qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
	topleft.v = (2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;

	/* Allow "all" files */
	if (all)
	{
		/* Get any file */
		SFGetFile(topleft, "\p", NULL, -1, types, NULL, &reply);
	}

	/* Allow "save" files */
	else
	{
		/* Legal types */
		types[0] = 'SAVE';

		/* Get a file */
		SFGetFile(topleft, "\p", NULL, 1, types, NULL, &reply);
	}

	/* Allow cancel */
	if (!reply.good) return;

	/* Extract textual file name for save file */
	GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
	refnum_to_name(savefile, drefnum, vrefnum, (char*)reply.fName);

	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(FALSE);

	/* Hack -- quit */
	quit(NULL);
}
#endif


/*
 * Handle the "open_when_ready" flag
 */
static void handle_open_when_ready(void)
{
	/* Check the flag XXX XXX XXX make a function for this */
	if (open_when_ready && initialized && !game_in_progress)
	{
		/* Forget */
		open_when_ready = FALSE;

		/* Game is in progress */
		game_in_progress = 1;

		/* Wait for it */
		pause_line(23);

		/* Flush input */
		flush();

		/* Play a game */
		play_game(FALSE);

		/* Quit */
		quit(NULL);
	}
}



/*
 * Initialize the menus
 *
 *   Apple (128) =   { About, -, ... }
 *   File (129) =    { New,Open,Import,Close,Save,-,Exit,Quit }
 *   Edit (130) =    { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) =    { Bold, Extend, -, Monaco, ..., -, ... }
 *   Size (132) =    { ... }
 *   Window (133) =  { Angband, Mirror, Recall, Choice,
 *                     Term-4, Term-5, Term-6, Term-7 }
 *   Special (134) = { Sound, Graphics, TileWidth, TileHeight, -,
 *                     Fiddle, Wizard }
 */

/* Apple menu */
#define MENU_APPLE	128
#define ITEM_ABOUT	1

/* File menu */
#define MENU_FILE	129
# define ITEM_NEW	1
# define ITEM_OPEN	2
# define ITEM_IMPORT	3
# define ITEM_CLOSE	4
# define ITEM_SAVE	5
#  define ITEM_QUIT	7

/* Edit menu */
#define MENU_EDIT	130
# define ITEM_UNDO	1
# define ITEM_CUT	3
# define ITEM_COPY	4
# define ITEM_PASTE	5
# define ITEM_CLEAR	6

/* Font menu */
#define MENU_FONT	131
# define ITEM_BOLD	1
# define ITEM_WIDE	2

/* Size menu */
#define MENU_SIZE	132

/* Windows menu */
#define MENU_WINDOWS	133

/* Special menu */
#define MENU_SPECIAL	134
# define ITEM_SOUND	1
# define ITEM_GRAPH	2
# define ITEM_TILEWIDTH 3
# define ITEM_TILEHEIGHT 4
# define ITEM_FIDDLE	6
# define ITEM_WIZARD	7

/* Sounds submenu */
#define SUBMENU_SOUND	143
# define ITEM_USE_SOUND	1
# define ITEM_SOUND_SETTING	2

/* Graphics submenu */
#define SUBMENU_GRAPH	144
# define ITEM_NONE	1
# define ITEM_8X8	2
# define ITEM_16X16	3
# define ITEM_BIGTILE 5	

/* TileWidth submenu */
#define SUBMENU_TILEWIDTH	145

/* TileHeight submenu */
#define SUBMENU_TILEHEIGHT	146


static void init_menubar(void)
{
	int i, n;

	Rect r;

	WindowPtr tmpw;

	MenuHandle m;

	Handle mbar;

#if TARGET_API_MAC_CARBON
	OSErr err;
	long response;
#endif

	/* Get the "apple" menu */
	mbar = GetNewMBar(128);

	/* Whoops! */
	if (mbar == nil) quit("Cannot find menubar('MBAR') id 128!");

	/* Insert them into the current menu list */
	SetMenuBar(mbar);

	/* Free handle */
	DisposeHandle(mbar);

#if !TARGET_API_MAC_CARBON
	/* Apple menu (id 128) */
	m = GetMenuHandle(MENU_APPLE);

	/* Add the DA's to the "apple" menu */
	AppendResMenu	(m, 'DRVR');
#endif

	/* Get the "File" menu */
#if TARGET_API_MAC_CARBON
	m = GetMenuHandle(MENU_FILE);
	err = Gestalt( gestaltSystemVersion, &response );
	if ( (err == noErr) && (response >= 0x00000A00) )
	{
		DeleteMenuItem(m, ITEM_QUIT);
	}
#endif

	/* Edit menu (id 130) - we don't have to do anything */

	/*
	 * Font menu (id 131) - append names of mono-spaced fonts
	 * followed by all available ones
	 */
	m = GetMenuHandle(MENU_FONT);

	/* Fake window */
	r.left = r.right = r.top = r.bottom = 0;

	/* Make the fake window */
	tmpw = NewWindow(0, &r, "\p", false, documentProc, 0, 0, 0);

	/* Activate the "fake" window */
#if TARGET_API_MAC_CARBON
	SetPortWindowPort(tmpw);
#else
	SetPort(tmpw);
#endif

	/* Default mode */
	TextMode(0);

	/* Default size */
	TextSize(12);

	/* Add the fonts to the menu */
	AppendResMenu(m, 'FONT');

	/* Size of menu */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Scan the menu */
	for (i = n; i >= 4; i--)
	{
		Str255 tmpName;
		short fontNum;

		/* Acquire the font name */
		GetMenuItemText(m, i, tmpName);

		/* Acquire the font index */
#if TARGET_API_MAC_CARBON
		fontNum = FMGetFontFamilyFromName( tmpName );
#else
		GetFNum(tmpName, &fontNum);
#endif

		/* Apply the font index */
		TextFont(fontNum);

		/* Remove non-mono-spaced fonts */
		if ((CharWidth('i') != CharWidth('W')) || (CharWidth('W') == 0))
		{
			/* Delete the menu item XXX XXX XXX */
			DeleteMenuItem	(m, i);
		}
	}

	/* Destroy the old window */
	DisposeWindow(tmpw);

	/* Add a separator */
	AppendMenu(m, "\p-");

	/* Add the fonts to the menu */
	AppendResMenu	(m, 'FONT');


	/* Size menu (id 132) */
	m = GetMenuHandle(MENU_SIZE);

	/* Add some sizes (stagger choices) */
	for (i = 8; i <= 32; i += ((i / 16) + 1))
	{
		Str15 buf;
		
		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);
	}


	/* Windows menu (id 133) */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Default choices */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		Str15 buf;
		
		/* Describe the item */
		sprintf((char*)buf + 1, "%.15s", angband_term_name[i]);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);

		/* Command-Key shortcuts */
		if (i < 8) SetItemCmd(m, i + 1, '0' + i);
	}


#if TARGET_API_MAC_CARBON && !defined(MAC_MPW)

	/* CW or gcc -- Use recommended interface for hierarchical menus */

	/* Special menu (id 134) */
	m = GetMenuHandle(MENU_SPECIAL);

	/* Insert Graphics submenu (id 143) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_SOUND);

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_SOUND, submenu);
	}

	/* Insert Graphics submenu (id 144) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_GRAPH);

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_GRAPH, submenu);
	}

	/* Insert TileWidth submenu (id 145) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_TILEWIDTH);

		/* Add some sizes */
		for (i = 4, n = 1; i <= 32; i++, n++)
		{
			Str15 buf;

			/* Textual size */
			strnfmt((char*)buf + 1, 15, "%d", i);
			buf[0] = strlen((char*)buf + 1);

			/* Append item */
			AppendMenu(submenu, buf);
		}

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_TILEWIDTH, submenu);
	}

	/* Insert TileHeight submenu (id 146) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_TILEHEIGHT);


		/* Add some sizes */
		for (i = 4, n = 1; i <= 32; i++, n++)
		{
			Str15 buf;

			/* Textual size */
			strnfmt((char*)buf + 1, 15, "%d", i);
			buf[0] = strlen((char*)buf + 1);

			/* Append item */
			AppendMenu(submenu, buf);
		}

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_TILEHEIGHT, submenu);
	}

#else

	/* Special menu (id 134) */

	/* Get graphics (sub)menu (id 143) */
	m = GetMenu(SUBMENU_SOUND);

	/* Insert it as a submenu */		
	InsertMenu(m, hierMenu);


	/* Get graphics (sub)menu (id 144) */
	m = GetMenu(SUBMENU_GRAPH);

	/* Insert it as a submenu */		
	InsertMenu(m, hierMenu);


	/* Get TileWidth (sub)menu (id 145) */
	m = GetMenu(SUBMENU_TILEWIDTH);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;
		
		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}


	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);

	/* Get TileHeight (sub)menu (id 146) */
	m = GetMenu(SUBMENU_TILEHEIGHT);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;

		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}


	/* Insert the menu */
	InsertMenu(m, hierMenu);

#endif
	/* Update the menu bar */
	DrawMenuBar();
}


/*
 * Prepare the menus
 */
static void setup_menus(void)
{
	int i, n;

	short value;

	Str255 s;

	MenuHandle m;

	term_data *td = NULL;


	/* Relevant "term_data" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Unused */
		if (!data[i].t) continue;

		/* Notice the matching window */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* File menu */
	m = GetMenuHandle(MENU_FILE);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
#if TARGET_API_MAC_CARBON
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
#else
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
#endif
	}

	/* Enable "new"/"open..."/"import..." */
	if (initialized && !game_in_progress)
	{
#if TARGET_API_MAC_CARBON
		EnableMenuItem(m, ITEM_NEW);
		EnableMenuItem(m, ITEM_OPEN);
		EnableMenuItem(m, ITEM_IMPORT);
#else
		EnableItem(m, ITEM_NEW);
		EnableItem(m, ITEM_OPEN);
		EnableItem(m, ITEM_IMPORT);
#endif
	}

	/* Enable "close" */
	if (initialized)
	{
#if TARGET_API_MAC_CARBON
		EnableMenuItem(m, ITEM_CLOSE);
#else
		EnableItem(m, ITEM_CLOSE);
#endif
	}

	/* Enable "save" */
	if (initialized && character_generated)
	{
#if TARGET_API_MAC_CARBON
		EnableMenuItem(m, ITEM_SAVE);
#else
		EnableItem(m, ITEM_SAVE);
#endif
	}

	/* Enable "quit" */
	if (TRUE)
	{
#if TARGET_API_MAC_CARBON
		EnableMenuItem(m, ITEM_QUIT);
#else
		EnableItem(m, ITEM_QUIT);
#endif
	}


	/* Edit menu */
	m = GetMenuHandle(MENU_EDIT);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
#if TARGET_API_MAC_CARBON
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
#else
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
#endif
	}

	/* Enable "edit" options if "needed" */
	if (!td)
	{
#if TARGET_API_MAC_CARBON
		EnableMenuItem(m, ITEM_UNDO);
		EnableMenuItem(m, ITEM_CUT);
		EnableMenuItem(m, ITEM_COPY);
		EnableMenuItem(m, ITEM_PASTE);
		EnableMenuItem(m, ITEM_CLEAR);
#else
		EnableItem(m, ITEM_UNDO);
		EnableItem(m, ITEM_CUT);
		EnableItem(m, ITEM_COPY);
		EnableItem(m, ITEM_PASTE);
		EnableItem(m, ITEM_CLEAR);
#endif
	}


	/* Font menu */
	m = GetMenuHandle(MENU_FONT);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
#if TARGET_API_MAC_CARBON
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
#else
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
#endif
	}

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, 1, bold); */

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, 2, extend); */

	/* Active window */
	if (td)
	{
#if TARGET_API_MAC_CARBON
		/* Enable "bold" */
		EnableMenuItem(m, ITEM_BOLD);

		/* Enable "extend" */
		EnableMenuItem(m, ITEM_WIDE);

		/* Check the appropriate "bold-ness" */
		if (td->font_face & bold) CheckMenuItem(m, ITEM_BOLD, TRUE);

		/* Check the appropriate "wide-ness" */
		if (td->font_face & extend) CheckMenuItem(m, ITEM_WIDE, TRUE);

		/* Analyze fonts */
		for (i = 4; i <= n; i++)
		{
			/* Enable it */
			EnableMenuItem(m, i);

			/* Analyze font */
			GetMenuItemText(m, i, s);
			GetFNum(s, &value);

			/* Check active font */
			if (td->font_id == value) CheckMenuItem(m, i, TRUE);
		}
#else
		/* Enable "bold" */
		EnableItem(m, ITEM_BOLD);

		/* Enable "extend" */
		EnableItem(m, ITEM_WIDE);

		/* Check the appropriate "bold-ness" */
		if (td->font_face & bold) CheckItem(m, ITEM_BOLD, TRUE);

		/* Check the appropriate "wide-ness" */
		if (td->font_face & extend) CheckItem(m, ITEM_WIDE, TRUE);

		/* Analyze fonts */
		for (i = 4; i <= n; i++)
		{
			/* Enable it */
			EnableItem(m, i);

			/* Analyze font */
			GetMenuItemText(m, i, s);
			GetFNum(s, &value);

			/* Check active font */
			if (td->font_id == value) CheckItem(m, i, TRUE);
		}
#endif
	}


	/* Size menu */
	m = GetMenuHandle(MENU_SIZE);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
#if TARGET_API_MAC_CARBON
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
#else
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
#endif
	}
	
	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			GetMenuItemText(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

#if TARGET_API_MAC_CARBON
			/* Enable the "real" sizes */
			if (RealFont(td->font_id, value)) EnableMenuItem(m, i);

			/* Check the current size */
			if (td->font_size == value) CheckMenuItem(m, i, TRUE);
#else
			/* Enable the "real" sizes */
			if (RealFont(td->font_id, value)) EnableItem(m, i);

			/* Check the current size */
			if (td->font_size == value) CheckItem(m, i, TRUE);
#endif
		}
	}


	/* Windows menu */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Check active windows */
	for (i = 1; i <= n; i++)
	{
		/* Check if needed */
#if TARGET_API_MAC_CARBON
		CheckMenuItem(m, i, data[i-1].mapped);
#else
		CheckItem(m, i, data[i-1].mapped);
#endif
	}


	/* Special menu */
	m = GetMenuHandle(MENU_SPECIAL);

	/* Get menu size */
#if TARGET_API_MAC_CARBON
	n = CountMenuItems(m);
#else
	n = CountMItems(m);
#endif

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
#if TARGET_API_MAC_CARBON
		DisableMenuItem(m, i);

#ifdef MAC_MPW
		/* XXX Oh no, this removes submenu... */
		if ((i != ITEM_SOUND) &&
		    (i != ITEM_GRAPH) &&
		    (i != ITEM_TILEWIDTH) &&
		    (i != ITEM_TILEHEIGHT)) CheckMenuItem(m, i, FALSE);
#else

		CheckMenuItem(m, i, FALSE);
#endif

#else
		DisableItem(m, i);

		/* XXX Oh no, this removes submenu... */
		if ((i != ITEM_SOUND) &&
		    (i != ITEM_GRAPH) &&
		    (i != ITEM_TILEWIDTH) &&
		    (i != ITEM_TILEHEIGHT)) CheckItem(m, i, FALSE);
#endif
	}

#if TARGET_API_MAC_CARBON
	/* Item "arg_sound" */
	EnableMenuItem(m, ITEM_SOUND);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* Graphics submenu */
		submenu = GetMenuHandle(SUBMENU_SOUND);

#else

		/* Graphics submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_SOUND, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Item "Sound On/Off" */
		EnableMenuItem(submenu, ITEM_USE_SOUND);
		CheckMenuItem(submenu, ITEM_USE_SOUND, arg_sound);

		/* Item "Sounf Config" */
#ifndef MACH_O_CARBON
		EnableMenuItem(submenu, ITEM_SOUND_SETTING);
#endif
	}

	/* Item "Graphics" */
	EnableMenuItem(m, ITEM_GRAPH);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* Graphics submenu */
		submenu = GetMenuHandle(SUBMENU_GRAPH);

#else

		/* Graphics submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_GRAPH, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Item "None" */
		EnableMenuItem(submenu, ITEM_NONE);
		CheckMenuItem(submenu, ITEM_NONE, (graf_mode == GRAF_MODE_NONE));

		/* Item "8x8" */
		EnableMenuItem(submenu, ITEM_8X8);
		CheckMenuItem(submenu, ITEM_8X8, (graf_mode == GRAF_MODE_8X8));

		/* Item "16x16" */
		EnableMenuItem(submenu, ITEM_16X16);
		CheckMenuItem(submenu, ITEM_16X16, (graf_mode == GRAF_MODE_16X16));

		/* Item "Big tiles" */
		EnableMenuItem(submenu, ITEM_BIGTILE);
		CheckMenuItem(submenu, ITEM_BIGTILE, arg_bigtile);
	}

	/* Item "TileWidth" */
	EnableMenuItem(m, ITEM_TILEWIDTH);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* TIleWidth submenu */
		submenu = GetMenuHandle(SUBMENU_TILEWIDTH);

#else

		/* TileWidth submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_TILEWIDTH, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Active window */
		if (td)
		{
			/* Analyze sizes */
			for (i = 1; i <= n; i++)
			{
				/* Analyze size */
				/* GetMenuItemText(m,i,s); */
				GetMenuItemText(submenu, i, s);
				s[s[0]+1] = '\0';
				value = atoi((char*)(s+1));

				/* Enable */
				if (value >= td->font_wid) EnableMenuItem(submenu, i);

				/* Check the current size */
				if (td->tile_wid == value) CheckMenuItem(submenu, i, TRUE);
			}
		}
	}

	/* Item "TileHeight" */
	EnableMenuItem(m, ITEM_TILEHEIGHT);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* TileHeight submenu */
		submenu = GetMenuHandle(SUBMENU_TILEHEIGHT);

#else

		/* TileWidth submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_TILEHEIGHT, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Active window */
		if (td)
		{
			/* Analyze sizes */
			for (i = 1; i <= n; i++)
			{
				/* Analyze size */
				/* GetMenuItemText(m,i,s); */
				GetMenuItemText(submenu, i, s);
				s[s[0]+1] = '\0';
				value = atoi((char*)(s+1));

				/* Enable */
				if (value >= td->font_hgt) EnableMenuItem(submenu, i);

				/* Check the current size */
				if (td->tile_hgt == value) CheckMenuItem(submenu, i, TRUE);
			}
		}
	}

	/* Item "arg_fiddle" */
	EnableMenuItem(m, ITEM_FIDDLE);
	CheckMenuItem(m, ITEM_FIDDLE, arg_fiddle);

	/* Item "arg_wizard" */
	EnableMenuItem(m, ITEM_WIZARD);
	CheckMenuItem(m, ITEM_WIZARD, arg_wizard);

#else
	/* Item "arg_sound" */
	EnableItem(m, ITEM_SOUND);

	/* Item "arg_graphics" */
	EnableItem(m, ITEM_GRAPH);

	/* Item "TileWidth" */
	EnableItem(m, ITEM_TILEWIDTH);

	/* Item "TileHeight" */
	EnableItem(m, ITEM_TILEHEIGHT);

	/* Item "arg_fiddle" */
	EnableItem(m, ITEM_FIDDLE);
	CheckItem(m, ITEM_FIDDLE, arg_fiddle);

	/* Item "arg_wizard" */
	EnableItem(m, ITEM_WIZARD);
	CheckItem(m, ITEM_WIZARD, arg_wizard);

	/* Sounds submenu */
	m = GetMenuHandle(SUBMENU_SOUND);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}
	
	/* Item "Sound On/Off" */
	EnableItem(m, ITEM_USE_SOUND);
	CheckItem(m, ITEM_USE_SOUND, arg_sound);

	/* Item "Sound Config" */
	EnableItem(m, ITEM_SOUND_SETTING);

	/* Graphics submenu */
	m = GetMenuHandle(SUBMENU_GRAPH);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}
	
	/* Item "None" */
	EnableItem(m, ITEM_NONE);
	CheckItem(m, ITEM_NONE, (graf_mode == GRAF_MODE_NONE));

	/* Item "8x8" */
	EnableItem(m, ITEM_8X8);
	CheckItem(m, ITEM_8X8, (graf_mode == GRAF_MODE_8X8));

	/* Item "16x16" */
	EnableItem(m, ITEM_16X16);
	CheckItem(m, ITEM_16X16, (graf_mode == GRAF_MODE_16X16));

	/* Item "Bigtile" */
	EnableItem(m, ITEM_BIGTILE);
	CheckItem(m, ITEM_BIGTILE, arg_bigtile);


	/* TIleWidth submenu */
	m = GetMenuHandle(SUBMENU_TILEWIDTH);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			/* GetMenuItemText(m,i,s); */
			GetMenuItemText(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

			/* Enable */
			if (value >= td->font_wid) EnableItem(m, i);

			/* Check the current size */
			if (td->tile_wid == value) CheckItem(m, i, TRUE);
		}
	}


	/* TileHeight submenu */
	m = GetMenuHandle(SUBMENU_TILEHEIGHT);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			GetMenuItemText(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

			/* Enable */
			if (value >= td->font_hgt) EnableItem(m, i);

			/* Check the current size */
			if (td->tile_hgt == value) CheckItem(m, i, TRUE);
		}
	}
#endif

}


/*
 * Process a menu selection (see above)
 *
 * Hack -- assume that invalid menu selections are disabled above,
 * which I have been informed may not be reliable.  XXX XXX XXX
 */
static void menu(long mc)
{
	int i;

	int menuid, selection;

	static unsigned char s[1000];

	short fid;

	term_data *td = NULL;

	WindowPtr old_win;


	/* Analyze the menu command */
	menuid = HiWord(mc);
	selection = LoWord(mc);


	/* Find the window */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Skip dead windows */
		if (!data[i].t) continue;

		/* Notice matches */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* Branch on the menu */
	switch (menuid)
	{
		/* Apple Menu */
		case MENU_APPLE:
		{
			/* About Angband... */
#if TARGET_API_MAC_CARBON
			if (selection == ITEM_ABOUT)
			{
				DialogPtr dialog;
				short item_hit;

				/* Get the about dialogue */
				dialog=GetNewDialog(128, 0, (WindowPtr)-1);

				/* Move it to the middle of the screen */
				RepositionWindow(
					GetDialogWindow(dialog),
					NULL,
					kWindowCenterOnMainScreen);

				/* Show the dialog */
				TransitionWindow(GetDialogWindow(dialog),
					kWindowZoomTransitionEffect,
					kWindowShowTransitionAction,
					NULL);

				/* Wait for user to click on it */
				ModalDialog(0, &item_hit);

				/* Free the dialogue */
				DisposeDialog(dialog);
				break;
			}
#else
			if (selection == ITEM_ABOUT)
			{
				DialogPtr dialog;
				Rect r;
				short item_hit;

				dialog=GetNewDialog(128, 0, (WindowPtr)-1);

				r=dialog->portRect;
				center_rect(&r, &qd.screenBits.bounds);
				MoveWindow(dialog, r.left, r.top, 1);
				ShowWindow(dialog);
				ModalDialog(0, &item_hit);
				DisposeDialog(dialog);
				break;
			}

			/* Desk accessory */
			GetMenuItemText(GetMenuHandle(MENU_APPLE), selection, s);
			OpenDeskAcc(s);
			break;
#endif
		}

		/* File Menu */
		case MENU_FILE:
		{
			switch (selection)
			{
				/* New */
				case ITEM_NEW:
				{
					do_menu_file_new();
					break;
				}

				/* Open... */
				case ITEM_OPEN:
				{
					do_menu_file_open(FALSE);
					break;
				}

				/* Import... */
				case ITEM_IMPORT:
				{
					do_menu_file_open(TRUE);
					break;
				}

				/* Close */
				case ITEM_CLOSE:
				{
					/* No window */
					if (!td) break;

					/* Not Mapped */
					td->mapped = FALSE;

					/* Not Mapped */
					td->t->mapped_flag = FALSE;

					/* Hide the window */
					HideWindow(td->w);

					break;
				}

				/* Save */
				case ITEM_SAVE:
				{
					if (!can_save){
						plog("You may not do that right now.");
						break;
					}
					
					/* Hack -- Save the game */
					do_cmd_save_game(FALSE);

					break;
				}

				/* Quit (with save) */
				case ITEM_QUIT:
				{
					/* Save the game (if necessary) */
					if (game_in_progress && character_generated)
					{
						if (!can_save){
							plog("You may not do that right now.");
							break;
						}
						/* Save the game */
#if 0
						do_cmd_save_game(FALSE);
#endif
						Term_key_push(SPECIAL_KEY_QUIT);
						break;
					}

					/* Quit */
					quit(NULL);
					break;
				}
			}
			break;
		}

		/* Edit menu */
		case MENU_EDIT:
		{
			/* Unused */
			break;
		}

		/* Font menu */
		case MENU_FONT:
		{
			/* Require a window */
			if (!td) break;

			/* Memorize old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Toggle the "bold" setting */
			if (selection == ITEM_BOLD)
			{
				/* Toggle the setting */
				if (td->font_face & bold)
				{
					td->font_face &= ~bold;
				}
				else
				{
					td->font_face |= bold;
				}

				/* Tile Width Hight Init */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Toggle the "wide" setting */
			if (selection == ITEM_WIDE)
			{
				/* Toggle the setting */
				if (td->font_face & extend)
				{
					td->font_face &= ~extend;
				}
				else
				{
					td->font_face |= extend;
				}

				/* Tile Width Hight Init */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Get a new font name */
			GetMenuItemText(GetMenuHandle(MENU_FONT), selection, s);
			GetFNum(s, &fid);

			/* Save the new font id */
			td->font_id = fid;

			/* Current size is bad for new font */
			if (!RealFont(td->font_id, td->font_size))
			{
				/* Find similar size */
				for (i = 1; i <= 32; i++)
				{
					/* Adjust smaller */
					if (td->font_size - i >= 8)
					{
						if (RealFont(td->font_id, td->font_size - i))
						{
							td->font_size -= i;
							break;
						}
					}

					/* Adjust larger */
					if (td->font_size + i <= 128)
					{
						if (RealFont(td->font_id, td->font_size + i))
						{
							td->font_size += i;
							break;
						}
					}
				}
			}

			/* Tile Width Hight Init */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore the window */
			activate(old_win);

			break;
		}

		/* Size menu */
		case MENU_SIZE:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			GetMenuItemText(GetMenuHandle(MENU_SIZE), selection, s);
			s[s[0]+1]=0;
			td->font_size = atoi((char*)(s+1));

			/* Tile Width Hight Init */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* Window menu */
		case MENU_WINDOWS:
		{
			/* Parse */
			i = selection - 1;

			/* Check legality of choice */
			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			/* Obtain the window */
			td = &data[i];

			/* Mapped */
			td->mapped = TRUE;

			/* Link */	
			term_data_link(i);

			/* Mapped (?) */
			td->t->mapped_flag = TRUE;

			/* Show the window */
			ShowWindow(td->w);

			/* Bring to the front */
			SelectWindow(td->w);

			break;
		}

		/* Special menu */
		case MENU_SPECIAL:
		{
			switch (selection)
			{
				case ITEM_FIDDLE:
				{
					arg_fiddle = !arg_fiddle;
					break;
				}

				case ITEM_WIZARD:
				{
					arg_wizard = !arg_wizard;
					break;
				}
			}

			break;
		}

		/* Sounds submenu */
		case SUBMENU_SOUND:
		{
			switch (selection)
			{
				case ITEM_USE_SOUND:
				{
					/* Toggle arg_sound */
					arg_sound = !arg_sound;

					/* React to changes */
					Term_xtra(TERM_XTRA_REACT, 0);

					break;
				}

				case ITEM_SOUND_SETTING:
				{
					SoundConfigDLog();

					break;
				}
			}

			break;
		}

		/* Graphics submenu */
		case SUBMENU_GRAPH:
		{
			switch (selection)
			{
				case ITEM_NONE:
				{
					graf_mode_req = GRAF_MODE_NONE;

					break;
				}

				case ITEM_8X8:
				{
					graf_mode_req = GRAF_MODE_8X8;

					break;
				}

				case ITEM_16X16:
				{
					graf_mode_req = GRAF_MODE_16X16;

					break;
				}

				case ITEM_BIGTILE:
				{
					term *old = Term;
					term_data *td = &data[0];

					/* Toggle "arg_bigtile" */
					arg_bigtile = !arg_bigtile;

					/* Activate */
					Term_activate(td->t);

					/* Resize the term */
					Term_resize(td->cols, td->rows);

					/* Activate old */
					Term_activate(old);

					break;
				}
			}

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

		/* TileWidth submenu */
		case SUBMENU_TILEWIDTH:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Analyse value */
			GetMenuItemText(GetMenuHandle(SUBMENU_TILEWIDTH), selection, s);
			s[s[0]+1]=0;
			td->tile_wid = atoi((char*)(s+1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* TileHeight submenu */
		case SUBMENU_TILEHEIGHT:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Analyse value */
			GetMenuItemText(GetMenuHandle(SUBMENU_TILEHEIGHT), selection, s);
			s[s[0]+1]=0;
			td->tile_hgt = atoi((char*)(s+1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}
	}


	/* Clean the menu */
	HiliteMenu(0);
}


#ifdef USE_SFL_CODE


/*
 * Check for extra required parameters -- From "Maarten Hazewinkel"
 */
static OSErr CheckRequiredAEParams(const AppleEvent *theAppleEvent)
{
	OSErr	aeError;
	DescType	returnedType;
	Size	actualSize;

	aeError = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
				    &returnedType, NULL, 0, &actualSize);

	if (aeError == errAEDescNotFound) return (noErr);

	if (aeError == noErr) return (errAEParamMissed);

	return (aeError);
}


/*
 * Apple Event Handler -- Open Application
 */
static pascal OSErr AEH_Start(const AppleEvent *theAppleEvent,
			      AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Quit Application
 */
static pascal OSErr AEH_Quit(const AppleEvent *theAppleEvent,
			     AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)
#if TARGET_API_MAC_CARBON
#pragma unused(theAppleEvent)

	/* Save the game (if necessary) */
	if (game_in_progress && character_generated)
	{
			if (!can_save){
				plog("You may not do that right now.");
				return;
			}
			/* Hack -- Forget messages */
			msg_flag = FALSE;

			/* Save the game */
#if 0
			do_cmd_save_game(FALSE);
#endif
			Term_key_push(SPECIAL_KEY_QUIT);
			return;
		}

		/* Quit */
		quit(NULL);
#else
	/* Quit later */
	quit_when_ready = TRUE;

	/* Check arguments */
	return (CheckRequiredAEParams(theAppleEvent));
#endif
}


/*
 * Apple Event Handler -- Print Documents
 */
static pascal OSErr AEH_Print(const AppleEvent *theAppleEvent,
			      AppleEvent *reply, long handlerRefCon)
{
#pragma unused(theAppleEvent, reply, handlerRefCon)

	return (errAEEventNotHandled);
}


/*
 * Apple Event Handler by Steve Linberg (slinberg@crocker.com).
 *
 * The old method of opening savefiles from the finder does not work
 * on the Power Macintosh, because CountAppFiles and GetAppFiles,
 * used to return information about the selected document files when
 * an application is launched, are part of the Segment Loader, which
 * is not present in the RISC OS due to the new memory architecture.
 *
 * The "correct" way to do this is with AppleEvents.  The following
 * code is modeled on the "Getting Files Selected from the Finder"
 * snippet from Think Reference 2.0.  (The prior sentence could read
 * "shamelessly swiped & hacked")
 */
static pascal OSErr AEH_Open(const AppleEvent *theAppleEvent,
			     AppleEvent* reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	char		foo[128];
	FInfo		myFileInfo;

	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;

	/*
	 * We ignore the validity check, because we trust the FInder, and we only
	 * allow one savefile to be opened, so we ignore the depth of the list.
	 */

	err = AEGetNthPtr(&docList, 1L, typeFSS, &keywd,
			  &returnedType, (Ptr) &myFSS, sizeof(myFSS), &actualSize);
	if (err) return err;

	/* Only needed to check savefile type below */
	err = FSpGetFInfo(&myFSS, &myFileInfo);
	if (err)
	{
		sprintf(foo, "Arg!  FSpGetFInfo failed with code %d", err);
		mac_warning (foo);
		return err;
	}

	/* Ignore non 'SAVE' files */
	if (myFileInfo.fdType != 'SAVE') return noErr;

#ifdef MACH_O_CARBON

	/* Extract a file name */
	(void)spec_to_path(&myFSS, savefile, sizeof(savefile));

#else

	/* XXX XXX XXX Extract a file name */
	PathNameFromDirID(myFSS.parID, myFSS.vRefNum, (StringPtr)savefile);
	pstrcat((StringPtr)savefile, (StringPtr)&myFSS.name);

	/* Convert the string */
	ptocstr((StringPtr)savefile);

#endif /* MACH_O_CARBON */

	/* Delay actual open */
	open_when_ready = TRUE;

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}


#endif



/*
 * Macintosh modifiers (event.modifier & ccc):
 *   cmdKey, optionKey, shiftKey, alphaLock, controlKey
 *
 *
 * Macintosh Keycodes (0-63 normal, 64-95 keypad, 96-127 extra):
 *
 * Return:36
 * Delete:51
 *
 * Period:65
 * Star:67
 * Plus:69
 * Clear:71
 * Slash:75
 * Enter:76
 * Minus:78
 * Equal:81
 * 0-7:82-89
 * 8-9:91-92
 *
 * F5: 96
 * F6: 97
 * F7: 98
 * F3:99
 * F8:100
 * F10:109
 * F11:103
 * F13:105
 * F14:107
 * F9:101
 * F12:111
 * F15:113
 * Help:114
 * Home:115
 * PgUp:116
 * Del:117
 * F4: 118
 * End:119
 * F2:120
 * PgDn:121
 * F1:122
 * Lt:123
 * Rt:124
 * Dn:125
 * Up:126
 */


/*
 * Optimize non-blocking calls to "CheckEvents()"
 * Idea from "Maarten Hazewinkel <mmhazewi@cs.ruu.nl>"
 */
#define EVENT_TICKS 6


/*
 * Check for Events, return TRUE if we process any
 *
 * Hack -- Handle AppleEvents if appropriate (ignore result code).
 */
static bool CheckEvents(bool wait)
{
	EventRecord event;

	WindowPtr w;

	Rect r;

	long newsize;

	int ch, ck;

	int mc, ms, mo, mx;

	int i;

	term_data *td = NULL;

	huge curTicks;

	static huge lastTicks = 0L;


	/* Access the clock */
	curTicks = TickCount();

	/* Hack -- Allow efficient checking for non-pending events */
	if (!wait && (curTicks < lastTicks + EVENT_TICKS)) return (FALSE);

	/* Timestamp last check */
	lastTicks = curTicks;

#if TARGET_API_MAC_CARBON
	WaitNextEvent( everyEvent, &event, 1L, nil );
#else
	/* Let the "system" run */
	SystemTask();

	/* Get an event (or null) */
	GetNextEvent(everyEvent, &event);
#endif

	/* Hack -- Nothing is ready yet */
	if (event.what == nullEvent) return (FALSE);

	/* Analyze the event */
	switch (event.what)
	{

#if 0

		case activateEvt:
		{
			w = (WindowPtr)event.message;

			activate(w);

			break;
		}

#endif

		case updateEvt:
		{
			/* Extract the window */
			w = (WindowPtr)event.message;

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Hack XXX XXX XXX */
			BeginUpdate(w);
			EndUpdate(w);

			/* Redraw the window */
			if (td) term_data_redraw(td);

			break;
		}

		case keyDown:
		case autoKey:
		{
			/* Extract some modifiers */
			mc = (event.modifiers & controlKey) ? TRUE : FALSE;
			ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
			mo = (event.modifiers & optionKey) ? TRUE : FALSE;
			mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

			/* Keypress: (only "valid" if ck < 96) */
			ch = (event.message & charCodeMask) & 255;

			/* Keycode: see table above */
			ck = ((event.message & keyCodeMask) >> 8) & 255;

			/* Command + "normal key" -> menu action */
			if (mx && (ck < 64))
			{
				/* Hack -- Prepare the menus */
				setup_menus();

				/* Run the Menu-Handler */
				menu(MenuKey(ch));

				/* Turn off the menus */
				HiliteMenu(0);

				/* Done */
				break;
			}


			/* Hide the mouse pointer */
			ObscureCursor();

			/* Normal key -> simple keypress */
			if (ck < 64)
			{
				/* Enqueue the keypress */
				Term_keypress(ch);
			}

			/* Bizarre key -> encoded keypress */
			else if (ck <= 127)
			{
				/* Hack -- introduce with control-underscore */
				Term_keypress(31);

				/* Send some modifier keys */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (mo) Term_keypress('O');
				if (mx) Term_keypress('X');

				/* Hack -- Downshift and encode the keycode */
				Term_keypress('0' + (ck - 64) / 10);
				Term_keypress('0' + (ck - 64) % 10);

				/* Hack -- Terminate the sequence */
				/* MPW can generate 10 or 13 for keycode of '\r' */
				/* -noMapCR option swaps '\r' and '\n' */
				Term_keypress('\r');
			}

			break;
		}

		case mouseDown:
		{
			int code;

			/* Analyze click location */
			code = FindWindow(event.where, &w);

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Analyze */
			switch (code)
			{
				case inMenuBar:
				{
					setup_menus();
					menu(MenuSelect(event.where));
					HiliteMenu(0);
					break;
				}
#if !TARGET_API_MAC_CARBON
				case inSysWindow:
				{
					SystemClick(&event, w);
					break;
				}
#endif

				case inDrag:
				{
					Point p;

					WindowPtr old_win;
					BitMap screen;
					Rect portRect;
#if TARGET_API_MAC_CARBON						
					GetQDGlobalsScreenBits( &screen );
#else
					screen = qd.screenBits;
#endif	
					r = screen.bounds;
					r.top += 20; /* GetMBarHeight() XXX XXX XXX */
					InsetRect(&r, 4, 4);
					DragWindow(w, event.where, &r);

					/* Oops */
					if (!td) break;

					/* Save */
					old_win = active;

					/* Activate */
					activate(td->w);

					/* Analyze */
#if TARGET_API_MAC_CARBON
					GetWindowBounds( (WindowRef)td->w, kWindowContentRgn, &portRect );
#else
					portRect = td->w->portRect;
					local_to_global( &portRect );
#endif
					p.h = portRect.left;
					p.v = portRect.top;
#if !TARGET_API_MAC_CARBON
					LocalToGlobal(&p);
#endif
					td->r.left = p.h;
					td->r.top = p.v;

					/* Restore */
					activate(old_win);

					/* Apply and Verify */
					term_data_check_size(td);

					break;
				}

				case inGoAway:
				{
					/* Oops */
					if (!td) break;

					/* Track the go-away box */
					if (TrackGoAway(w, event.where))
					{
						/* Not Mapped */
						td->mapped = FALSE;

						/* Not Mapped */
						td->t->mapped_flag = FALSE;

						/* Hide the window */
						HideWindow(td->w);
					}

					break;
				}

				case inGrow:
				{
					s16b x, y;

					term *old = Term;
					BitMap		screen;
	
#if TARGET_API_MAC_CARBON
					GetQDGlobalsScreenBits( &screen );
#else
					screen = qd.screenBits;
#endif
					/* Oops */
					if (!td) break;

					/* Fake rectangle */
					r.left = 20 * td->tile_wid + td->size_ow1;
					r.right = screen.bounds.right;
					r.top = 1 * td->tile_hgt + td->size_oh1;
					r.bottom = screen.bounds.bottom;

					/* Grow the rectangle */
					newsize = GrowWindow(w, event.where, &r);

					/* Handle abort */
					if (!newsize) break;

					/* Extract the new size in pixels */
					y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
					x = LoWord(newsize) - td->size_ow1 - td->size_ow2;

					/* Extract a "close" approximation */
					td->rows = y / td->tile_hgt;
					td->cols = x / td->tile_wid;

					/* Apply and Verify */
					term_data_check_size(td);
					/* Activate */
					Term_activate(td->t);

					/* Hack -- Resize the term */
					Term_resize(td->cols, td->rows);

					/* Resize and Redraw */
					term_data_resize(td);
					term_data_redraw(td);

					/* Restore */
					Term_activate(old);

					break;
				}

				case inContent:
				{
					SelectWindow(w);

					break;
				}
			}

			break;
		}

		/* Disk Event -- From "Maarten Hazewinkel" */
		case diskEvt:
		{

#if TARGET_API_MAC_CARBON
#else
			/* check for error when mounting the disk */
			if (HiWord(event.message) != noErr)
			{
				Point p =
				{120, 120};

				DILoad();
				DIBadMount(p, event.message);
				DIUnload();
			}
#endif
			break;
		}

		/* OS Event -- From "Maarten Hazewinkel" */
		case osEvt:
		{
			switch ((event.message >> 24) & 0x000000FF)
			{
				case suspendResumeMessage:

				/* Resuming: activate the front window */
				if (event.message & resumeFlag)
				{
#if TARGET_API_MAC_CARBON
					Cursor	arrow;
						
					SetPortWindowPort( FrontWindow() );
					
					GetQDGlobalsArrow( &arrow );
					SetCursor(&arrow);
#else
					SetPort(FrontWindow());
					SetCursor(&qd.arrow);
#endif
				}

				/* Suspend: deactivate the front window */
				else
				{
					/* Nothing */
				}

				break;
			}

			break;
		}

#ifdef USE_SFL_CODE

		/* From "Steve Linberg" and "Maarten Hazewinkel" */
		case kHighLevelEvent:
		{
#if TARGET_API_MAC_CARBON
			AEProcessAppleEvent(&event);
#else
			/* Process apple events */
			if (AEProcessAppleEvent(&event) != noErr)
			{
				plog("Error in Apple Event Handler!");
			}

			/* Handle "quit_when_ready" */
			if (quit_when_ready)
			{
				/* Forget */
				quit_when_ready = FALSE;

				/* Do the menu key */
				menu(MenuKey('q'));

				/* Turn off the menus */
				HiliteMenu(0);
			}

			/* Handle "open_when_ready" */
			handle_open_when_ready();
#endif

			break;
		}

#endif

	}


	/* Something happened */
	return (TRUE);
}




/*** Some Hooks for various routines ***/


/*
 * Mega-Hack -- emergency lifeboat
 */
static vptr lifeboat = NULL;


/*
 * Hook to "release" memory
 */
static vptr hook_rnfree(vptr v, huge size)
{

#pragma unused (size)

#ifdef USE_MALLOC

	/* Alternative method */
	free(v);

#else

	/* Dispose */
	DisposePtr(v);

#endif

	/* Success */
	return (NULL);
}

/*
 * Hook to "allocate" memory
 */
static vptr hook_ralloc(huge size)
{

#ifdef USE_MALLOC

	/* Make a new pointer */
	return (malloc(size));

#else

	/* Make a new pointer */
	return (NewPtr(size));

#endif

}

/*
 * Hook to handle "out of memory" errors
 */
static vptr hook_rpanic(huge size)
{

#pragma unused (size)

	/* vptr mem = NULL; */

	/* Free the lifeboat */
	if (lifeboat)
	{
		/* Free the lifeboat */
		DisposePtr(lifeboat);

		/* Forget the lifeboat */
		lifeboat = NULL;

		/* Mega-Hack -- Warning */
		mac_warning("Running out of Memory!\rAbort this process now!");

		/* Mega-Hack -- Never leave this function */
		while (TRUE) CheckEvents(TRUE);
	}

	/* Mega-Hack -- Crash */
	return (NULL);
}


/*
 * Hook to tell the user something important
 */
static void hook_plog(cptr str)
{
	/* Warning message */
	mac_warning(str);
}

/*
 * Hook to tell the user something, and then quit
 */
static void hook_quit(cptr str)
{
	/* Warning if needed */
	if (str) mac_warning(str);

#ifdef USE_ASYNC_SOUND

	/* Clean up sound support */
	cleanup_sound();

#endif /* USE_ASYNC_SOUND */

	/* Write a preference file */
	save_pref_file();

	/* All done */
	ExitToShell();
}

/*
 * Hook to tell the user something, and then crash
 */
static void hook_core(cptr str)
{
	/* XXX Use the debugger */
	/* DebugStr(str); */

	/* Warning */
	if (str) mac_warning(str);

	/* Warn, then save player */
	mac_warning("Fatal error.\rI will now attempt to save and quit.");

	/* Attempt to save */
	if (!save_player()) mac_warning("Warning -- save failed!");
	
	/* Quit */
	quit(NULL);
}



/*** Main program ***/


/*
 * Init some stuff
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "Macintosh Save Bug" by using "absolute" path names, since on
 * System 7 machines anyway, the "current working directory" often
 * "changes" due to background processes, invalidating any "relative"
 * path names.  Note that the Macintosh is limited to 255 character
 * path names, so be careful about deeply embedded directories...
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "missing lib folder bug" by allowing the user to help find the
 * "lib" folder by hand if the "application folder" code fails...
 */
static void init_stuff(void)
{
	int i;

#if !TARGET_API_MAC_CARBON
	short vrefnum;
	long drefnum;
	long junk;

	SFTypeList types;
	SFReply reply;
#endif

	Rect r;
	Point topleft;

	char path[1024];

	BitMap screen;
	Rect screenRect;

#if TARGET_API_MAC_CARBON
	OSErr err = noErr;
	NavDialogOptions dialogOptions;
	FSSpec theFolderSpec;
	NavReplyRecord theReply;
#endif
	/* Fake rectangle */
	r.left = 0;
	r.top = 0;
	r.right = 344;
	r.bottom = 188;

	/* Center it */
#if TARGET_API_MAC_CARBON
	screenRect = GetQDGlobalsScreenBits(&screen)->bounds;
#else
	screenRect = qd.screenBits.bounds;
#endif
	center_rect(&r, &screenRect);

	/* Extract corner */
	topleft.v = r.top;
	topleft.h = r.left;


	/* Default to the "lib" folder with the application */
#ifdef MACH_O_CARBON
	if (locate_lib(path, sizeof(path)) == NULL) quit(NULL);
#else
	refnum_to_name(path, app_dir, app_vol, (char*)("\plib:"));
#endif


	/* Check until done */
	while (1)
	{
		/* Prepare the paths */
		init_file_paths(path);

		/* Build the filename */
		path_build(path, sizeof(path), ANGBAND_DIR_FILE, "news.txt");

		/* Attempt to open and close that file */
		if (0 == fd_close(fd_open(path, O_RDONLY))) break;

		/* Warning */
		plog_fmt("Unable to open the '%s' file.", path);

		/* Warning */
		plog("The Angband 'lib' folder is probably missing or misplaced.");

		/* Warning */
		plog("Please 'open' any file in any sub-folder of the 'lib' folder.");
		
#if TARGET_API_MAC_CARBON
		/* Ask the user to choose the lib folder */
		err = NavGetDefaultDialogOptions(&dialogOptions);

		/* Paranoia */
		if (err != noErr) quit(NULL);

		/* Set default location option */
		dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;

		/* Clear preview option */
		dialogOptions.dialogOptionFlags &= ~(kNavAllowPreviews);

		/* Forbit selection of multiple files */
		dialogOptions.dialogOptionFlags &= ~(kNavAllowMultipleFiles);

		/* Display location */
		dialogOptions.location = topleft;

		/* Load the message for the missing folder from the resource fork */
		GetIndString(dialogOptions.message, 128, 1);

		/* Wait for the user to choose a folder */
		err = NavChooseFolder(
			nil,
			&theReply,
			&dialogOptions,
			nil,
			nil,
			nil);

		/* Assume the player doesn't want to go on */
		if ((err != noErr) || !theReply.validRecord) quit(NULL);

		/* Retrieve FSSpec from the reply */
		{
			AEKeyword theKeyword;
			DescType actualType;
			Size actualSize;

			/* Get a pointer to selected folder */
			err = AEGetNthPtr(
				&(theReply.selection),
				1,
				typeFSS,
				&theKeyword,
				&actualType,
				&theFolderSpec,
				sizeof(FSSpec),
				&actualSize);

			/* Paranoia */
			if (err != noErr) quit(NULL);
		}

		/* Free navitagor reply */
		err = NavDisposeReply(&theReply);

		/* Paranoia */
		if (err != noErr) quit(NULL);

		/* Extract textual file name for given file */
		refnum_to_name(
			path,
			theFolderSpec.parID,
			theFolderSpec.vRefNum,
			(char *)theFolderSpec.name);
#else
		/* Allow "text" files */
		types[0] = 'TEXT';

		/* Allow "save" files */
		types[1] = 'SAVE';

		/* Allow "data" files */
		types[2] = 'DATA';

		/* Get any file */
		SFGetFile(topleft, "\p", NULL, 3, types, NULL, &reply);

		/* Allow cancel */
		if (!reply.good) quit(NULL);

		/* Extract textual file name for given file */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
		refnum_to_name(path, drefnum, vrefnum, (char*)reply.fName);
#endif

		/* Hack -- Remove the "filename" */
		i = strlen(path) - 1;
		while ((i > 0) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i+1] = '\0';

		/* Hack -- allow "lib" folders */
		if (suffix(path, "lib:")) continue;

		/* Hack -- Remove the "sub-folder" */
		i = i - 1;
		while ((i > 1) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i+1] = '\0';
	}
}


/*
 * Macintosh Main loop
 */
int main(void)
{
	EventRecord tempEvent;
	int numberOfMasters = 10;

#if !TARGET_API_MAC_CARBON
	/* Increase stack space by 64K */
	SetApplLimit(GetApplLimit() - 131072L);//65536L);

	/* Stretch out the heap to full size */
	MaxApplZone();
#endif

	/* Get more Masters */
	while (numberOfMasters--) MoreMasters();

#if !TARGET_API_MAC_CARBON
	/* Set up the Macintosh */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	/* TEInit(); */
	InitDialogs(NULL);
#endif
	InitCursor();


	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Flush events some more (?) */
	(void)EventAvail(everyEvent, &tempEvent);
	(void)EventAvail(everyEvent, &tempEvent);
	(void)EventAvail(everyEvent, &tempEvent);


#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

# if defined(powerc) || defined(__powerc)

	/* Assume System 7 */
	
	/* Assume Color Quickdraw */

# else

#if TARGET_API_MAC_CARBON

	{
		OSErr err;
		long response;

		/* Check for existence of Carbon */
		err = Gestalt(gestaltCarbonVersion, &response);

		if (err != noErr) quit("This program requires Carbon API");
	}

#else
	/* Block */
	if (TRUE)
	{
		OSErr err;
		long versionNumber;

		/* Check the Gestalt */
		err = Gestalt(gestaltSystemVersion, &versionNumber);

		/* Check the version */
		if ((err != noErr) || (versionNumber < 0x0700))
		{
			quit("You must have System 7 to use this program.");
		}
	}

	/* Block */
	if (TRUE)
	{
		SysEnvRec env;

		/* Check the environs */
		if (SysEnvirons(1, &env) != noErr)
		{
			quit("The SysEnvirons call failed!");
		}

		/* Check for System Seven Stuff */
		if (env.systemVersion < 0x0700)
		{
			quit("You must have System 7 to use this program.");
		}

		/* Check for Color Quickdraw */
		if (!env.hasColorQD)
		{
			quit("You must have Color Quickdraw to use this program.");
		}
	}

#endif /* CARBON */
#endif

#endif /* ANGBAND_LITE_MAC */

	/* 
	 * Remember Mac OS version, in case we have to cope with version-specific
	 * problems
	 */
	(void)Gestalt(gestaltSystemVersion, &mac_os_version);

#ifdef USE_SFL_CODE
	/* Obtain a "Universal Procedure Pointer" */
	AEH_Start_UPP = NewAEEventHandlerUPP(AEH_Start);
	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, AEH_Start_UPP,
			      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Quit_UPP = NewAEEventHandlerUPP(AEH_Quit);
	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, AEH_Quit_UPP,
			      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Print_UPP = NewAEEventHandlerUPP(AEH_Print);
	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, AEH_Print_UPP,
			      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Open_UPP = NewAEEventHandlerUPP(AEH_Open);
	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, AEH_Open_UPP,
			      0L, FALSE);
#endif

#ifndef MACH_O_CARBON
	/* Find the current application */
	SetupAppDir();
#endif

	/* Mark ourself as the file creator */
	_fcreator = ANGBAND_CREATOR;

	/* Default to saving a "text" file */
	_ftype = 'TEXT';


#if defined(__MWERKS__)

	/* Obtian a "Universal Procedure Pointer" */
#if TARGET_API_MAC_CARBON
	ynfilterUPP = NewModalFilterUPP(ynfilter);
#else
	ynfilterUPP = NewModalFilterProc(ynfilter);
#endif

#endif


	/* Hook in some "z-virt.c" hooks */
	rnfree_aux = hook_rnfree;
	ralloc_aux = hook_ralloc;
	rpanic_aux = hook_rpanic;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;

	BackColor(blackColor);
	ForeColor(whiteColor);

	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));

	/* Prepare the menubar */
	init_menubar();

	/* Prepare the windows */
	init_windows();

#ifndef MACH_O_CARBON

	init_sound();

	init_graf();

#endif
	
	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;

	/* Reset the cursor */
#if TARGET_API_MAC_CARBON
	{
		Cursor	arrow;
		GetQDGlobalsArrow( &arrow );
		SetCursor(&arrow);
	}
#else
	SetCursor( &qd.arrow );
#endif


	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = NewPtr(16384);

#ifdef USE_QT_SOUND

	/* Load sound effect resources */
	load_sounds();

#endif /* USE_QT_SOUND */

	/* Note the "system" */
	ANGBAND_SYS = "mac";

	/* Initialize */
	init_stuff();

	/* Catch nasty signals */
	signals_init();

	/* Initialize */
	init_angband();


	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;


	/* We are now initialized */
	initialized = TRUE;


	/* Handle "open_when_ready" */
	handle_open_when_ready();


	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);

	/* Flush the prompt */
	Term_fresh();


	/* Hack -- Process Events Forever */
	while (TRUE) CheckEvents(TRUE);
}

