/* 
 * Motif
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
*/ 
/*
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */
/* 
 * HISTORY
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: ImageCache.c /main/44 1998/10/06 17:26:25 samborn $"
#endif
#endif
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include "XmI.h"
#include "XmosI.h"              /* for mask name API and stuff */
#include "BitmapsI.h"		/* for built-in images */
#include "ColorI.h"		/* for _XmSearchColorCache() */
#include "HashI.h"		/* for hash table routines. */
#include "ImageCachI.h"		/* for DIRECT_PIXMAP_CACHED */
#include "ReadImageI.h"		/* for read xbm stuff */
#include <Xm/AccColorT.h>       /* for new _XmGetColoredPixmap API */
#include <Xm/ColorObjP.h>       /* for Xme Color Obj access API */
#include <Xm/IconFile.h>        /* XmGetIconFileName */
#include <Xm/PrintSP.h>         /* for pixmap resolution */
#include <Xm/XpmP.h>
#include <X11/Xresource.h>

/* additional value for GetImage return, FALSE, TRUE and */
#define NOT_CACHED 2

/* index and max size of the override_colorsarray */
#define NUM_SYMBOLIC_COLORS       7
#define SYMB_BACKGROUND           0
#define SYMB_FOREGROUND           1
#define TRANSPARENT_COLOR         2
#define SYMB_TOP_SHADOW_COLOR     3
#define SYMB_BOTTOM_SHADOW_COLOR  4
#define SYMB_SELECT_COLOR         5
#define SYMB_HIGHLIGHT_COLOR      6

/*  Image set handling defines, structure, and global statics  */

typedef struct _ImageData
{
   int		 hot_x, hot_y;
   XImage	 *image;
   char		 *image_name;
   unsigned char *builtin_data;
} ImageData;

static XmHashTable image_set = NULL;


/*  Pixmap caching structure and global statics  */

typedef struct _PixmapData
{
   Screen * screen;
   char   * image_name;
   XmAccessColorData acc_color ;
   Pixmap   pixmap;
   int	    depth;
   Dimension width ;
   Dimension height ;
   int      reference_count;
   unsigned short print_resolution ;
   Widget   print_shell ;
   double   scaling_ratio ;
   Pixel  * pixels;
   int      npixels;
} PixmapData;

static XmHashTable pixmap_set = NULL;
static XmHashTable pixmap_data_set = NULL;


/*  Color caching structure and global statics  */

typedef struct _CachedColorStruct {
    Display	*display;
    Colormap 	 colormap;
    XrmQuark 	 colorname;
    unsigned short red, green, blue;
    Pixel    	 pixel;
    unsigned int num_cached;	/* reference count */
} CachedColor;

typedef struct _CachedColorListStruct {
    int          numEntries;
    int          maxEntries;
    CachedColor  *cache;
} CachedColorList;

static CachedColorList colorCacheList;


/*  GC caching structure and global statics  */

typedef struct _GCData
{
    GC		gc;
    Screen     *screen;
    Widget	print_shell;
    int		depth;
    int		image_depth;
    Pixel	foreground;
    Pixel	background;
} GCData;

static XmHashTable gc_set = NULL;


typedef struct _CleanKey {
    Screen * screen ;
    Widget shell ;
} CleanKey ;

/********    Static Function Declarations    ********/

static Boolean ComparePixmaps (XmHashKey key_1, XmHashKey key_2);
static Boolean ComparePixmapDatas (XmHashKey key_1, XmHashKey key_2);
static Boolean CompareStrings (XmHashKey key_1, XmHashKey key_2);
static Boolean CompareGCDatas (XmHashKey key_1, XmHashKey key_2);
static XmHashValue HashPixmap (XmHashKey key);
static XmHashValue HashPixmapData (XmHashKey key);
static XmHashValue HashString (XmHashKey key);
static XmHashValue HashGCData (XmHashKey key);
static void InitializeImageSet( void ) ;
static void InitializePixmapSets( void ) ;
static Boolean UninstallImageMapProc (XmHashKey key, XtPointer value, 
			       XtPointer image);  
static Boolean SymbolicColorUsed(String color_name,
				 XpmColor * xpm_colors,
				 unsigned int ncolors);
static void CompleteUnspecColors(
			       Screen *screen,
			       XpmColorSymbol *override_colors);
static int GetOverrideColors(
			    Screen *screen,
			    XmAccessColorData acc_color,
			    XpmColorSymbol *override_colors);
static XtEnum GetImage(Screen *screen, char *image_name,
		       XmAccessColorData acc_color, XImage **image,
		       unsigned short * pixmap_resolution,
		       Pixel **pixels,	/* allocated pixels */
		       int *npixels);
static GC GetGCForPutImage(Screen *screen,
			   Widget print_shell,
			   XImage * image, 
			   Pixmap pixmap, 
			   int depth,
			   Pixel foreground,
			   Pixel background);

static int GetCacheColor(Display *display, Colormap colormap,
			 char *colorname, XColor *color, void *data);
static int FreeCacheColors(Display *display, Colormap colormap,
			   Pixel *pixels, int npixels, void *data);

/********    End Static Function Declarations    ********/




/*** IMAGE CACHE PART FIRST ***/




/* Compare two strings. */
static Boolean 
CompareStrings (XmHashKey key_1, 
		XmHashKey key_2)
{
  char *data_1 = (char *) key_1;
  char *data_2 = (char *) key_2;

  return ((data_1 == data_2) || (strcmp(data_1, data_2) == 0));
}


/* Hash a string. */
static XmHashValue 
HashString (XmHashKey key)
{
  char *data = (char *) key;
  unsigned int len = strlen(data);

  return (((len << 8) | data[0]) << 8) | data[len];
}

/************************************************************************
 *
 *  InitializeImageSet
 *	Initialize the image set.
 *
 ************************************************************************/
static void 
InitializeImageSet( void )
{
  register int i;

  /* Allocate the hash table. */
  assert (image_set == NULL);

  _XmProcessLock();
  image_set = 
    _XmAllocHashTable (MAX_BUILTIN_IMAGES + 100, CompareStrings, HashString);

  /* Load the built-in image data.
     Builtins have a non NULL builtin data.
     Their image field is setdynamically at GetImage time */
  for (i = 0; i < MAX_BUILTIN_IMAGES ; i++)
    {
      ImageData *entry = XtNew(ImageData);

      entry->hot_x = 0;
      entry->hot_y = 0;
      entry->image = NULL;
      entry->image_name = (char*) bitmap_name_set[i];
      entry->builtin_data = (unsigned char *) bitmaps[i];

      _XmAddHashEntry(image_set, entry->image_name, entry);
    }
  _XmProcessUnlock();
}

/************************************************************************
 *
 *  _XmInstallImage 
 * 
 *		Allow a hot_spot to be specified
 *
 ************************************************************************/
Boolean 
_XmInstallImage(
        XImage *image,
        char *image_name,
	int hot_x,
	int hot_y)
{
   ImageData *entry;


   /*  Error checking  */

   if (image == NULL || image_name == NULL) return (False);


   /*  Check for the initial allocation of the image set array  */

   if (image_set == NULL) InitializeImageSet();


   /*  Verify that the image_name is not already in the image set.  */
   _XmProcessLock();

   if (_XmGetHashEntry(image_set, image_name) != NULL) {
     _XmProcessUnlock();
     return (False);
   }


   /*  Initialize the image element for the new image and return True.  */

   entry = XtNew(ImageData);
   entry->hot_x = hot_x;
   entry->hot_y = hot_y;
   entry->image = image;
   entry->image_name = XtNewString(image_name);
   entry->builtin_data = NULL;

   _XmAddHashEntry(image_set, entry->image_name, entry);

   _XmProcessUnlock();
   return (True);
}

/************************************************************************
 *
 *  XmInstallImage
 *	Add the provided image for the image set and return an
 *	id to be used for further referencing.  
 *
 ************************************************************************/
Boolean 
XmInstallImage(
        XImage *image,
        char *image_name )
{
    Boolean	ret_val;
    
    _XmProcessLock();
    ret_val = _XmInstallImage(image, image_name, 0, 0);
    _XmProcessUnlock();

    return ret_val;
}
/* A hash table map procedure to uninstall an image. */

/*ARGSUSED*/
static Boolean 
UninstallImageMapProc (XmHashKey key, /* unused */
		XtPointer value,
		XtPointer image)
{
  ImageData *entry = (ImageData *) value;

  if (entry->image == (XImage*) image)
    {
      /* Can't free built-in data. */
      if (entry->builtin_data == NULL)
	{
	  _XmProcessLock();
	  _XmRemoveHashEntry (image_set, entry->image_name);
	  _XmProcessUnlock();
	  XtFree(entry->image_name);
	  /* uninstall do not free the XImage itself */
	  XtFree((char*) entry);
	}

      return True;
    }

  return False;
}

/************************************************************************
 *
 *  XmUninstallImage
 *	Remove an image from the image set.
 *	Return a boolean (True) if the uninstall succeeded.  Return
 *	a boolean (False) if an error condition occurs.
 *
 ************************************************************************/
Boolean 
XmUninstallImage(
        XImage *image )
{
   Cardinal old_count;
   Boolean	ret_val;

   /*  Check for invalid conditions  */

   if ((image == NULL) || (image_set == NULL)) return (False);

   _XmProcessLock();
   /*  Since we can't index based on the image, search the hash table */
   /*  until the desired entry is found and then remove it.	      */

   old_count = _XmHashTableCount(image_set);

   _XmMapHashTable(image_set, UninstallImageMapProc, image);
   ret_val = (old_count > _XmHashTableCount(image_set));
   _XmProcessUnlock();

   return ret_val;
}



/************************************************************************
 *
 * SymbolicColorUsed
 *  Used to determine if a given color string is defined as a symbolic
 *   color in an Xpm image. This is used for caching policy where we
 *   don't want to cache based on irrelevant colors.
 *  Note that we don't look if the color is actually used because it
 *   is too expensive.
 *
 ************************************************************************/
static Boolean 
SymbolicColorUsed(String color_name,
		  XpmColor * xpm_colors,
		  unsigned int ncolors)
{
    Cardinal i, j, data_size ;

    /* first look if the color is present as a symbolic in the 
       colorTable */
    for (i = 0; i < ncolors; i++, xpm_colors++) {
	if (xpm_colors->symbolic &&
	    !strcmp(xpm_colors->symbolic, color_name))
	    break ;
    }

    if (i == ncolors) return False ;	

    return False ;
}


/************************************************************************
 *
 *  CompleteUnspecColors
 *     Complete the unspecified_pixel values of an override_colors array 
 *      based on the values coming from the ColorObj or the color cache.
 *     Does not handle highlight.
 *     The logic here comes directly from CDE.
 *
 *  The above description does not fit with the actual logic of CDE where
 *  not only the unspecified colors are set here but the whole set of
 *  symbols! See defect CDExc17529.
 *
 ************************************************************************/
static void
CompleteUnspecColors(
		Screen *screen,
		XpmColorSymbol *override_colors)
{
    XmPixelSet 	pixelSets[XmCO_NUM_COLORS];
    int 	colorUse ;
    Boolean	bgFound = False, fgFound = False, result = False;
    Pixel	*pixelPtr;
    Cardinal i, j ;
    XmAccessColorDataRec loc_acc_color;


    /* Ask the color obj */
    if (!(result = XmeGetColorObjData(screen, 
				      &colorUse, pixelSets, XmCO_NUM_COLORS,
				      NULL, NULL, NULL, NULL, NULL)))
      i = XmCO_NUM_COLORS;
    else
      i = 0;

    /* Look for a pixels set containing the given fg and bg,
     * no matter whether they are actually the fg and bg of the set...
     */
    for ( ; i < XmCO_NUM_COLORS; i++) {
	bgFound = False;
	fgFound = False;
	pixelPtr = (Pixel *)&(pixelSets[i].fg);
	for (j = 0; j < 5; j++, pixelPtr++) {
	    if (*pixelPtr == override_colors[SYMB_BACKGROUND].pixel)
	      bgFound = True;
	    else if (*pixelPtr == override_colors[SYMB_FOREGROUND].pixel)
	      fgFound = True;
	}
	if (bgFound && fgFound)
	  break;
    }

    if (i == XmCO_NUM_COLORS) {
	/*
	 * We didn't find the bg/fg tuple in any of the Dt colorsets
	 * or if the color server is not running, we will now try
	 * the color cache. 
	 */

      XmColorData *old_colors;
      XmColorData new_colors;

      loc_acc_color.background = override_colors[SYMB_BACKGROUND].pixel ;

      new_colors.screen = screen;
      new_colors.color_map = DefaultColormapOfScreen(screen);
      new_colors.background.pixel = loc_acc_color.background ;

      /* Use motif color set only if already allocated.
       */
      if (!result && _XmSearchColorCache(
		 (XmLOOK_AT_SCREEN | XmLOOK_AT_CMAP | XmLOOK_AT_BACKGROUND),
					 &new_colors, &old_colors)) {
	  XmGetColors(screen, 
		      DefaultColormapOfScreen(screen),
		      loc_acc_color.background,
		      &loc_acc_color.foreground,
		      &loc_acc_color.top_shadow_color,
		      &loc_acc_color.bottom_shadow_color,
		      &loc_acc_color.select_color);
      } else {
	  /* Cannot generate missing colors */
	  return ;
       }
    }
    else {
	loc_acc_color.background = pixelSets[i].bg;
	loc_acc_color.foreground = pixelSets[i].fg;
	loc_acc_color.top_shadow_color = pixelSets[i].ts;
	loc_acc_color.bottom_shadow_color = pixelSets[i].bs;
	loc_acc_color.select_color = pixelSets[i].sc;
    }

    /* Now process the setting for unspecified pixel values.
       If we're here, all the colors but highlight are available */
    
    override_colors[SYMB_BACKGROUND].pixel = loc_acc_color.background ;

    override_colors[SYMB_FOREGROUND].pixel = loc_acc_color.foreground ;

    override_colors[SYMB_TOP_SHADOW_COLOR].pixel = 
	loc_acc_color.top_shadow_color ;

    override_colors[SYMB_BOTTOM_SHADOW_COLOR].pixel = 
	loc_acc_color.bottom_shadow_color ;

    override_colors[SYMB_SELECT_COLOR].pixel = loc_acc_color.select_color ;
}


/************************************************************************
 *
 *  GetOverrideColors
 *   Given a set of colors in acc_color with values specified or not
 *   this function fills up an XpmColorSymbol override_colors array to
 *   be used by the Xpm reader.
 *   It then calls the ColorObject aware filling proc to find values
 *   for the still unspecified ones.
 *   It returns the number of entries to consider in override_colors,
 *   which can be 2 (background and foreground are always valid), 
 *   NUM_SYMBOLIC_COLORS - 1 (if everything but the highlight could
 *   be generated), or NUM_SYMBOLIC_COLOR.
 *
 ************************************************************************/

static int
GetOverrideColors(
		Screen *screen,
		XmAccessColorData acc_color,
		XpmColorSymbol *override_colors)
{
    Cardinal i, n ;

    /* init the value fields */
    for (i=0; i<NUM_SYMBOLIC_COLORS; i++) override_colors[i].value = NULL ;
	    
    /* proceed each color in turn: acc_color might point to
       XmUNSPECIFIED_PIXEL, but background/foreground should be valid */

    override_colors[SYMB_BACKGROUND].name = XmNbackground ;
    override_colors[SYMB_BACKGROUND].pixel = acc_color->background ;

    override_colors[SYMB_FOREGROUND].name = XmNforeground ;
    override_colors[SYMB_FOREGROUND].pixel = acc_color->foreground ;

    /* Set "none" color to background in case the mask is not used. */
    override_colors[TRANSPARENT_COLOR].name = NULL ;
    override_colors[TRANSPARENT_COLOR].value = "None" ;
    override_colors[TRANSPARENT_COLOR].pixel = acc_color->background ;

    override_colors[SYMB_TOP_SHADOW_COLOR].name = XmNtopShadowColor ;
    override_colors[SYMB_TOP_SHADOW_COLOR].pixel = 
	acc_color->top_shadow_color ;

    override_colors[SYMB_BOTTOM_SHADOW_COLOR].name = XmNbottomShadowColor ;
    override_colors[SYMB_BOTTOM_SHADOW_COLOR].pixel = 
	acc_color->bottom_shadow_color ;

    override_colors[SYMB_SELECT_COLOR].name = XmNselectColor ;
    override_colors[SYMB_SELECT_COLOR].pixel = acc_color->select_color ;
    /* need to hack around selectColor which might be still
       unspecified while highlight is not (and that woudl break our 
       ordering rule - see below). Use top_shadow color so that select 
       follows its set/unset semantics regarding overriding */
    if (override_colors[SYMB_SELECT_COLOR].pixel == XmUNSPECIFIED_PIXEL)
	override_colors[SYMB_SELECT_COLOR].pixel = 
	    override_colors[SYMB_TOP_SHADOW_COLOR].pixel ;

    override_colors[SYMB_HIGHLIGHT_COLOR].name = XmNhighlightColor ;
    override_colors[SYMB_HIGHLIGHT_COLOR].pixel = acc_color->highlight_color ;



    /* now call ColorObject aware routine to give it a chance to
       fill the remaining unspecified pixel field in override_colors */

    CompleteUnspecColors (screen, override_colors) ;

    /* Just count the number of valid pixel. 
       This makes a strong assumption on the ordering: if we find 
       5 valid pixels, the 5 first one will be used.
       This is ok since _XmGetColoredPixmap is only used internally
       in such a way that this happens to be true: background/foreground
       are always here, and highlight color is the only one
       that might be missing after CompleteUnspecColors call.
       The day XmGetColoredPixmap is promoted public, this simple 
       logic will have to be reviewed */

    n = 0 ;
    for (i=0; i<NUM_SYMBOLIC_COLORS; i++) 
	if (override_colors[i].pixel != XmUNSPECIFIED_PIXEL) n++;
	    
    return n ;
}



#ifdef _ORIG_GET_ICON_FILE

/******************************************************************
 * 
 *   GetIconFileName. get a file name using XBMLANGPATH
 *
 *****************************************************************/

static String
GetIconFileName(
    Screen *screen,
    char *icon_name)
{
    char *bmPath;
    SubstitutionRec	subs[1] ;
    Boolean user_path ;
    Display		*display = DisplayOfScreen(screen);
    char *file_name;
 
    subs[0].substitution = icon_name;
	  
    bmPath = _XmOSInitPath(icon_name, "XBMLANGPATH", &user_path);
	  
    if (user_path) subs[0].match = 'B';
    else           subs[0].match = MATCH_CHAR ;
	  
    file_name = XtResolvePathname(display, "bitmaps", NULL,
				  NULL, bmPath, subs, XtNumber(subs), NULL);
    XtFree (bmPath);

    return file_name ;
      
}

#endif

/************************************************************************
 *
 *  GetImage
 *	Main routine of the image cache part.
 *
 ************************************************************************/
static XtEnum
GetImage(
	 Screen *screen,
	 char *image_name,
         XmAccessColorData acc_color,
	 XImage **image,
	 unsigned short * pixmap_resolution,
	 Pixel **pixels,	/* allocated pixels */
	 int *npixels)
{
    static XImage  * built_in_image = NULL;
    register Display *display = DisplayOfScreen(screen);
    int hot_x = 0 , hot_y = 0 ;
    ImageData *entry;
    char *file_name;
    XImage * mask_image = NULL ;
    XpmAttributes attrib ;
    XpmColorSymbol override_colors[NUM_SYMBOLIC_COLORS]; /* max */
    int num_override_colors;
    Boolean useIconFileCache;
    Boolean useMask;
    Boolean useColor;
    int xpmStatus;

    /* init so that we can call safely XpmFreeAttributes. */
    attrib.valuemask = 0;

    /* init for when we go thru the image cache first */
    if (pixmap_resolution) *pixmap_resolution = 0 ;

    if (pixels) *pixels = NULL;
    if (npixels) *npixels = 0;

    /***  Check for the initial allocation of the image set array  */
    
    if (image_set == NULL) InitializeImageSet();

    if (!image_name) return FALSE ;


    /*** look in the XImage cache first */
    _XmProcessLock();
    entry = (ImageData*) _XmGetHashEntry(image_set, image_name);
    _XmProcessUnlock();
    

    if (entry) {

	/*  If the image is a builtin image then get it.  */

	if (entry->builtin_data) {
	    /* there is one builtin XImage shared by all builtins,
	       only the data part change every time a query is made */
	    _XmProcessLock();
	    if (!built_in_image) {
		/* update that with new R6 init image stuff when out */
		_XmCreateImage(built_in_image, display, NULL, 
			       16, 16, MSBFirst);
	    }
	
	    built_in_image->data = (char *) entry->builtin_data;
	    _XmProcessUnlock();
	    *image = built_in_image;
	    
	} else {   

	    /* other entry found are just fine, set the image and return  */
    
	    *image = entry->image ;
	}	

	return TRUE ;
    }

    /*** if no entry, try to read a new file and cache it
         only if it is a bitmap */

#ifdef _ORIG_GET_ICON_FILE
    file_name = GetIconFileName(screen, image_name);
#else
    file_name = XmGetIconFileName(screen, NULL, image_name, 
				  NULL, XmUNSPECIFIED_ICON_SIZE) ;
#endif

    if (!file_name) {	
	return FALSE ;
    }

    /* Init the Xpm attributes to be passed to the reader */
    attrib.closeness = 40000;
    attrib.bitmap_format = XYBitmap;
    attrib.alloc_color = GetCacheColor;
    attrib.free_colors = FreeCacheColors;
    attrib.valuemask = XpmCloseness | XpmBitmapFormat | XpmReturnColorTable
	| XpmAllocColor | XpmFreeColors | XpmReturnAllocPixels;

    /* if any symbolic color are defined, used them */
    if (acc_color && 
	(num_override_colors = GetOverrideColors(screen, 
						 acc_color,
						 override_colors)) != 0) {
	attrib.colorsymbols = override_colors;
	attrib.numsymbols = num_override_colors;
	attrib.valuemask |= XpmColorSymbols;
    }

    /* ask the color object for information about the
       use of mask or color */
    (void)XmeGetIconControlInfo(screen, 
				&useMask,
				&useColor,
				&useIconFileCache); /* unused */

    if (!useColor) {
	attrib.depth = 1;
	attrib.valuemask |= XpmDepth;
    }

    *image = NULL ;

    xpmStatus = XmeXpmReadFileToImage(display,
			      file_name,
			      image,
			      &mask_image,
			      &attrib);
    if (xpmStatus < 0)
	*image = NULL ;
    else {
	/* store allocated colors */
	if (pixels) *pixels = attrib.alloc_pixels;
	if (npixels) *npixels = attrib.nalloc_pixels;
	/* and make sure they won't be free'd */
	attrib.alloc_pixels = NULL;
	attrib.nalloc_pixels = 0;
    }
    

    if (!(*image))
	*image = (XImage *) _XmReadImageAndHotSpotFromFile (display, 
							    file_name,
							    &hot_x, &hot_y);
    XtFree(file_name);

    /* get the image "design" resolution */
    /* in the future: look in the file */
    if (pixmap_resolution) *pixmap_resolution = 0 ;


    if (*image) {

	/* if the XPM file contained a embedded mask,
	   install it using our mask name scheme if the color object
	   tell us to do so */
	if (mask_image && useMask) {
	    char mask_name[255] ;
	    
	    _XmOSGenerateMaskName(image_name, mask_name);
	    /* if an image already exist under that
	       name, nothing will be done */
	    _XmInstallImage (mask_image, mask_name, hot_x, hot_y);
	}

	/* now we have to adjust the passed acc_color */
	if (acc_color) {
	    if ((*image)->depth == 1) {
		/* we've loaded an xbm file, just forget about
		   the 'other' symbolic colors, for they
		   are not going to be used during pixmap 
		   generation for depth 1 => pixmap putimage */
		acc_color->top_shadow_color = XmUNSPECIFIED_PIXEL;
		acc_color->bottom_shadow_color= XmUNSPECIFIED_PIXEL;
		acc_color->select_color = XmUNSPECIFIED_PIXEL;
		acc_color->highlight_color = XmUNSPECIFIED_PIXEL;
		if (acc_color->foreground == XmUNSPECIFIED_PIXEL
		    && acc_color->background == XmUNSPECIFIED_PIXEL) {
		    acc_color->foreground = 1;
		    acc_color->background = 0;
		}
	    } else {

		/* we've loaded a xpm, check which symbolics
		   colors were actually used during the read,
		   we don't want to remember the unused one
		   for the pixmap caching */
	
		if (!SymbolicColorUsed(XmNbackground,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->background = XmUNSPECIFIED_PIXEL;
		
		if (!SymbolicColorUsed(XmNforeground,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->foreground = XmUNSPECIFIED_PIXEL;
		
		if (!SymbolicColorUsed(XmNtopShadowColor,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->top_shadow_color = XmUNSPECIFIED_PIXEL;	
		
		if (!SymbolicColorUsed(XmNbottomShadowColor,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->bottom_shadow_color = XmUNSPECIFIED_PIXEL;
		
		if (!SymbolicColorUsed(XmNselectColor,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->select_color = XmUNSPECIFIED_PIXEL;
		
		if (!SymbolicColorUsed(XmNhighlightColor,
				       attrib.colorTable,
				       attrib.ncolors))
		    acc_color->highlight_color = XmUNSPECIFIED_PIXEL;
		
	    }	
	}	
	

	/* install the XImage with that name.
	   We do not cache the non-depth 1 ones
	   for we don't want to keep them in the image
	   cache, since they need color lookup,
	   which is done one level up in the pixmap cache */
	if (((*image)->depth == 1)
	    && (acc_color->foreground == 1)
	    && (acc_color->background == 0)
	    ) {
	    _XmInstallImage (*image, image_name, hot_x, hot_y);
	    return TRUE ;
	} else {
	    if (xpmStatus >= 0)
		XmeXpmFreeAttributes(&attrib);
	    return NOT_CACHED ; /* mean the image can be destroyed
				   after it is used */
	}
    }
    
    if (xpmStatus >= 0)
	XmeXpmFreeAttributes(&attrib);

    return FALSE ;
} 

/*** Keep this one in here, this is the only entry point to
     the Image cache. It can be used to duplicate them, etc */
Boolean
_XmGetImage(
	Screen *screen,
	char *image_name,
	XImage **image )
{
    return GetImage(screen, image_name, NULL, image, NULL, NULL, NULL) ;
}


/************************************************************************
*
*  _XmInImageCache   
*       Used by IconFile.c
*
************************************************************************/

Boolean
_XmInImageCache(
	String image_name)
{
    XtPointer	ret_val;

    if (!image_set) return False ;
    _XmProcessLock();
    ret_val = _XmGetHashEntry(image_set, image_name);
    _XmProcessUnlock();

    return (ret_val != NULL) ;
}









/*** PIXMAP CACHE NOW ***/

/** static dependencies with Image part: 
  * GetImage(), ImageData&image_set, HashString 
  **/
/** static dependencies with Color part: 
  * FreeCacheColors()
  **/




static Boolean
ComparePixmaps (XmHashKey key_1, 
		XmHashKey key_2)
{
  PixmapData *data_1 = (PixmapData *) key_1;
  PixmapData *data_2 = (PixmapData *) key_2;

    /* Check for a matching pixmap using screen and pixmap */

    return 
	(data_1->screen == data_2->screen &&
	 data_1->pixmap == data_2->pixmap) ;
}

static Boolean
ComparePixmapDatas (XmHashKey key_1, 
		    XmHashKey key_2)
{
  PixmapData *data_1 = (PixmapData *) key_1;
  PixmapData *data_2 = (PixmapData *) key_2;

    /* Check for a matching pixmap using depth, screen, name and
       colors info + print_resolution/scaling_ratio and print_shell */
    /* if a negative depth is given, we must look for a matching
       pixmap of -depth or 1 */

    return 
	((data_2->pixmap == XmUNSPECIFIED_PIXMAP ||
	  data_1->pixmap == data_2->pixmap) &&
	 data_1->image_name != NULL &&
	 data_2->image_name != NULL &&
	 strcmp (data_1->image_name, data_2->image_name) == 0  &&

	 (data_1->screen == data_2->screen) &&

	 ( /* ratio == 0 means print resolution is used */
	  (data_1->scaling_ratio == data_2->scaling_ratio &&
	   data_1->scaling_ratio) ||
	  (data_1->print_resolution == data_2->print_resolution &&
	   !data_1->scaling_ratio)) &&

	 (data_1->print_shell == data_2->print_shell) &&

	 ((data_1->depth == data_2->depth)  ||
	  (data_2->depth < 0 && data_1->depth == -data_2->depth) ||
	  (data_2->depth < 0 && data_1->depth == 1)) &&

	 ((data_1->acc_color->foreground == data_2->acc_color->foreground) ||
	  (data_2->depth < 0 && data_1->acc_color->foreground == 1) ||
	  (data_1->acc_color->foreground == XmUNSPECIFIED_PIXEL) ||
	  (data_2->acc_color->foreground == XmUNSPECIFIED_PIXEL))          &&
	 ((data_1->acc_color->background == data_2->acc_color->background) ||
	  (data_2->depth < 0 && data_1->acc_color->background == 0) ||
	  (data_1->acc_color->background == XmUNSPECIFIED_PIXEL)||
	  (data_2->acc_color->background == XmUNSPECIFIED_PIXEL))          &&

	 ((data_1->acc_color->top_shadow_color == 
	   data_2->acc_color->top_shadow_color) ||
	  (data_1->acc_color->top_shadow_color == XmUNSPECIFIED_PIXEL)||
	  (data_2->acc_color->top_shadow_color == XmUNSPECIFIED_PIXEL))     &&
	 ((data_1->acc_color->bottom_shadow_color 
	   == data_2->acc_color->bottom_shadow_color)||
	  (data_1->acc_color->bottom_shadow_color == XmUNSPECIFIED_PIXEL)||
	  (data_2->acc_color->bottom_shadow_color == XmUNSPECIFIED_PIXEL))  &&
	 ((data_1->acc_color->select_color == 
	   data_2->acc_color->select_color) ||
	  (data_1->acc_color->select_color == XmUNSPECIFIED_PIXEL)||
	  (data_2->acc_color->select_color == XmUNSPECIFIED_PIXEL))       &&
	 ((data_1->acc_color->highlight_color == 
	   data_2->acc_color->highlight_color) ||
	  (data_1->acc_color->highlight_color == XmUNSPECIFIED_PIXEL) ||
	  (data_2->acc_color->highlight_color == XmUNSPECIFIED_PIXEL))) ;
}

/* Hash a Pixmap entry. */
static XmHashValue 
HashPixmap (XmHashKey key)
{
  PixmapData *data = (PixmapData *) key;

  return ((long)data->screen + data->pixmap);
}

/* Hash a PixmapData entry. Reuse the string hash function */
static XmHashValue 
HashPixmapData (XmHashKey key)
{
  PixmapData *data = (PixmapData *) key;

  return ((long)data->screen + HashString ((XmHashKey)data->image_name));
}

/************************************************************************
 *
 *  InitializePixmapSets
 *	Initialize the pixmap sets.
 *
 ************************************************************************/
static void 
InitializePixmapSets( void )
{
    /* Allocate the pixmap hash tables.
     One for the regular XmGetPixmap lookup, one for the
     XmeGetPixmapData lookup */
    assert (pixmap_data_set == NULL && pixmap_set == NULL);

    _XmProcessLock();
    pixmap_data_set = _XmAllocHashTable (100, ComparePixmapDatas, 
					 HashPixmapData);

    pixmap_set = _XmAllocHashTable (100, ComparePixmaps, HashPixmap);
    _XmProcessUnlock();

}

/************************************************************************
 *
 *  _XmCachePixmap
 *	Install a pixmap into the pixmap cache.  This is used to add
 *	cached pixmaps which have no XImage associated with them.
 *      This one you can pass depth and size info, if you don't,
 *      the X server will be queried: very expensive.
 *
 ************************************************************************/
Boolean 
_XmCachePixmap(
        Pixmap pixmap,
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background,
        int depth,
        Dimension width,
        Dimension height)
{
   PixmapData * pix_entry;

   Window root;
   int x,y;
   unsigned int loc_width, loc_height, border_width, loc_depth;
   
   /*  Error checking  */
   if (image_name == NULL) return (False);

   if (!pixmap_data_set) InitializePixmapSets() ;

   /* if no information was given, get it from the server */
   if (!(width && height && depth))
       XGetGeometry(DisplayOfScreen(screen), pixmap, &root, &x, &y,
		    &loc_width, &loc_height, &border_width, &loc_depth);
   else {
	   loc_width = width ;
	   loc_height = height ;
	   loc_depth = depth ;
       }
   
   /*  Allocate the cache structure and put it into the list  */

   pix_entry = XtNew (PixmapData);
   
   pix_entry->screen = screen;
   pix_entry->acc_color = XtNew(XmAccessColorDataRec);
   pix_entry->acc_color->foreground = foreground;
   pix_entry->acc_color->background = background;
   pix_entry->acc_color->top_shadow_color = XmUNSPECIFIED_PIXEL;
   pix_entry->acc_color->bottom_shadow_color = XmUNSPECIFIED_PIXEL;
   pix_entry->acc_color->select_color = XmUNSPECIFIED_PIXEL;
   pix_entry->acc_color->highlight_color = XmUNSPECIFIED_PIXEL;
   pix_entry->depth = loc_depth;
   pix_entry->width = loc_width;
   pix_entry->height = loc_height;
   pix_entry->image_name = XtNewString(image_name);
   pix_entry->pixmap = pixmap;
   pix_entry->reference_count = 1;
   pix_entry->print_resolution = 100 ;
   pix_entry->print_shell = NULL ;
   pix_entry->scaling_ratio = 1 ;
   pix_entry->pixels = NULL;
   pix_entry->npixels = 0;

   _XmProcessLock();
   _XmAddHashEntry(pixmap_set, (XmHashKey)pix_entry, (XtPointer)pix_entry);

   /* Only add pixmaps not issued from XmeGetPixmapData to the "real" table */
   if (0 != strcmp(image_name, DIRECT_PIXMAP_CACHED))
       _XmAddHashEntry(pixmap_data_set, (XmHashKey)pix_entry, 
		       (XtPointer)pix_entry);
   _XmProcessUnlock();

   return (True);
}



/********************************************************************/

Boolean 
_XmGetPixmapData(
		   Screen *screen,
		   Pixmap pixmap,
		   char **image_name,
		   int *depth,
		   Pixel *foreground,
		   Pixel *background,
		   int *hot_x,
		   int *hot_y,
		   unsigned int *width,
		   unsigned int *height)
/* no pixmap_resolution returned... */
{
    PixmapData pix_data, *pix_entry;
    ImageData * entry;

    if (!pixmap_data_set) InitializePixmapSets() ;

    /*  checks for a matching screen and pixmap.            */
    
    pix_data.screen = screen ;
    pix_data.pixmap = pixmap ;
   
    _XmProcessLock();
    if ((pix_entry = (PixmapData*) 
	 _XmGetHashEntry(pixmap_set, (XmHashKey)&pix_data)) != NULL) {

	*foreground = pix_entry->acc_color->foreground;
	*background = pix_entry->acc_color->background;
	*depth = pix_entry->depth;
	*image_name = pix_entry->image_name;
	*width = pix_entry->width;
	*height = pix_entry->height;
	/* try to get the hot spot data from the image cache */
	if (image_name && image_set)  {
	    entry = (ImageData *) _XmGetHashEntry(image_set, *image_name);
	    if (entry) {
		*hot_x = entry->hot_x;
		*hot_y = entry->hot_y;
	    }
	}
	_XmProcessUnlock();
	return True;
    }
    _XmProcessUnlock();
    return False ;    
}



/*******************************************************************/
/* the real one, used by PixConv and locally too */
Pixmap 
_XmGetScaledPixmap(
    Screen *screen,
    Widget widget,
    char *image_name,
    XmAccessColorData acc_color,
    int depth,
#if NeedWidePrototypes
    int only_if_exists,
#else
    Boolean only_if_exists,
#endif /* NeedWidePrototypes */
    double scaling_ratio)
{    
    Display * display = DisplayOfScreen(screen);
    XImage * image;
    Pixmap   pixmap;
    GC gc ;
    XtEnum ret ;
    PixmapData pix_data, *pix_entry ;
    unsigned short pixmap_resolution ;
    Pixel *pixels;		/* allocated colors */
    int npixels;
    int old_image_format;
    char *old_image_data = NULL;

    /*  Error checking  */    
    if (image_name == NULL) return (XmUNSPECIFIED_PIXMAP);

    if (!pixmap_data_set) InitializePixmapSets() ;

    /* if screen not provided, widget has to be! */
    if (!screen) screen = XtScreen(widget);

    /* since more than one printers can attach to one screen,
       we need to use the print shell in the caching */
    /* since resolution can be dynamic for a printer, we need
       to use the resolution too, when scaling is 0 */
    pix_data.screen = screen ;
    pix_data.image_name = image_name ;
    pix_data.depth = depth ;
    pix_data.acc_color = acc_color ;
    pix_data.print_resolution = 100 ; /* default */
    pix_data.scaling_ratio = scaling_ratio ;

    pix_data.pixmap = XmUNSPECIFIED_PIXMAP;

    /* find out the print shell ancestor */
    pix_data.print_shell = widget ;
    while(pix_data.print_shell && !XmIsPrintShell(pix_data.print_shell)) 
	pix_data.print_shell = XtParent(pix_data.print_shell);
    /* pix_data.print_shell might be NULL here */


    /* scaling_ratio == 0 means use print_resolution and pixmap_resolution 
       in scaling - so first find out the print_resolution, since it
       is used in caching */
    if (!scaling_ratio && pix_data.print_shell) 
	pix_data.print_resolution = 
	  ((XmPrintShellWidget)pix_data.print_shell)->print.print_resolution ;

    /* if scaling_ratio a real number, like 1 or 1.2 
       print_resolution still 100 and will not be used */

    /* look if we have a match and return it */
    _XmProcessLock();
    if ((pix_entry = (PixmapData*) 
	 _XmGetHashEntry(pixmap_data_set, (XmHashKey)&pix_data)) != NULL) {
	pix_entry->reference_count++;
	_XmProcessUnlock();
	return pix_entry->pixmap ;
    }
    _XmProcessUnlock();
	
    /* The desired pixmap is not already cached.  Quit now or create it. */
    if (only_if_exists)
	return (XmUNSPECIFIED_PIXMAP);

    /* no pixmap found, look for an XImage. 
       generate a new one if needed.
       GetImage can return a bitmap or a pixmap XImage */
    /* GetImage modify the acc_color struct to update the
       colors that are not used during reading:
         - for xbm, everything but foreground and background.
	 - for xpm, on a case by case basis, depending
	 if the corresponding symbolic color was used. */
       
    if (!(ret = GetImage(screen, image_name, acc_color, &image, 
			 &pixmap_resolution, &pixels, &npixels)))
	return (XmUNSPECIFIED_PIXMAP);

    /* now find how much we need to scale this baby */

    /* compute the real ratio to be used in PutImage */
    if (!pix_data.scaling_ratio) {
	/* if scaling ratio is real, like 1.2, the pixmap resolution is
	   not used, neither is the print_resolution */

	if (!pixmap_resolution) {
	    /* GetImage returns the resolution for which the image 
	       was designed. if none was provided in the file, it's 0, 
	       in which case, the print shell default resolution 
	       for pixmap is used */

	    if (pix_data.print_shell) 
		pixmap_resolution = 
		    ((XmPrintShellWidget)pix_data.print_shell)
			->print.default_pixmap_resolution ;
	    else pixmap_resolution = 100 ;
	}

	pix_data.scaling_ratio = (double)pix_data.print_resolution / 
	    (double)pixmap_resolution ;
    } 
    

    /* now we treat the -depth case: if a negative depth was given
       then xbm file should map into bitmap and xpm into pixmap */
   
    if (depth < 0) {
	if (image->depth == 1) 
	  depth = 1 ;
	else 
	  depth = -depth ;
    }

    /* now check the validity of the image depth.  XPutImage can
       only handle unequal depths for bitmaps. */
    if ((image->depth != depth) && (image->depth != 1))
	return (XmUNSPECIFIED_PIXMAP);

    /* force the foreground/background if a Bitmap
       is returned. These values are going to go in the cache
       too, so we will have to remember this forcing for the
       case -depth: you ask for -depth and some
       foreground/background, that should match a depth 1
       with 1/0. Look at ComparePixmapDatas above for details */

    if (depth == 1) {
	acc_color->foreground = 1;
	acc_color->background = 0;
    }

    /* XPutImage will only deepen bitmaps -- fake it with pixmaps. */
    old_image_format = image->format;
    if (image->depth == 1) { 
      switch(image->format)
	{
	case XYBitmap:
	  /* XPutImage will do the right thing. */
	  break;

	case XYPixmap:
	  /* We're going to muck with the shared image... */
	  if (ret != NOT_CACHED)
	    _XmProcessLock();

	  /* Assume black == fg in the image. */
	  if ((BlackPixelOfScreen(screen) == 0) ||
	      (WhitePixelOfScreen(screen) == 1))
	    {
	      /* Flip the bits so fg == 1 in the image. */
	      register int nbytes = image->height * image->bytes_per_line;
	      register int byte;

	      /* Image data may be constant, so we must copy it. */
	      old_image_data = image->data;
	      image->data = XtMalloc(nbytes);

	      for (byte = 0; byte < nbytes; byte++)
		image->data[byte] = ~old_image_data[byte];
	    }

	  /* In this depth-1 case the image formats are equivalent. */
	  image->format = XYBitmap;
	  break;

	case ZPixmap:
	  /* Assume the application really wants BadMatch if (depth != 1). */
	  break;

	default:
	  assert(FALSE);
	}
    }

    
    /*
     * Create a pixmap to hold the image, allocate a new pixmap
     * cache entry, put the cache entry in the tables.
     */
    
    pix_entry = XtNew (PixmapData);

    pix_entry->screen = screen;
    pix_entry->acc_color = XtNew(XmAccessColorDataRec);
    memcpy((void*)pix_entry->acc_color, (void*)acc_color,
	   sizeof(XmAccessColorDataRec));
    pix_entry->depth = depth;
    pix_entry->image_name = XtNewString(image_name);
    pix_entry->print_shell = pix_data.print_shell;
    pix_entry->print_resolution = pix_data.print_resolution ;
    pix_entry->pixels = pixels;
    pix_entry->npixels = npixels;

    /* use scaling_ratio for caching: as 0, in which case the
       print resolution will be used, or 1, no scaling, or a real ratio. 
       That's because when we enter this function, we don't have 
       pixmap_resolution to cache against, and also because 
       print_resolution might change for the same print shell */
    pix_entry->scaling_ratio = scaling_ratio ;

    /* use the pixmap scaling ratio for printing  */
    pix_entry->width = image->width * pix_data.scaling_ratio ;
    pix_entry->height = image->height * pix_data.scaling_ratio ;

    pixmap = XCreatePixmap (display, RootWindowOfScreen(screen), 
			    pix_entry->width,
			    pix_entry->height,
			    depth);
    
    pix_entry->pixmap = pixmap;
    pix_entry->reference_count = 1;
    
    /* put the new entry in both tables: the table used for
       this routine and the table used in XmeGetPixmapData */
    _XmProcessLock();
    _XmAddHashEntry(pixmap_set, (XmHashKey)pix_entry, (XtPointer)pix_entry);
    _XmAddHashEntry(pixmap_data_set, (XmHashKey)pix_entry, 
		    (XtPointer)pix_entry);
    _XmProcessUnlock();

    /*  Set up a gc for the image to pixmap copy, store the image  */
    /*  into the pixmap and return the pixmap.                     */
    
    gc = GetGCForPutImage(screen, pix_entry->print_shell,
			  image, pixmap, depth,
			  acc_color->foreground,
			  acc_color->background);

    /* transfer and scale the image */
    _XmPutScaledImage (display, pixmap, gc, image, 
		       0, 0, 0, 0, image->width, image->height,
		       pix_entry->width, pix_entry->height);

    /* Destroy non-cached XImage now that we've cached the pixmap. */
    if (ret == NOT_CACHED)
      {
	XDestroyImage(image) ;
	if (old_image_data)
	  XtFree(image->data);
      }
    else if (image->format != old_image_format)
      {
	/* Undo the XYPixmap to XYBitmap conversion done earlier. */
	image->format = old_image_format;
	assert(old_image_format == XYPixmap);

	if (old_image_data)
	  {
	    XtFree(image->data);
	    image->data = old_image_data;
	  }

	/* We're done mangling the image. */
	_XmProcessUnlock();
      }

    return (pixmap);
}


/*******************************************************************
 *
 * _XmGetColoredPixmap
 *   a improved version of XmGetPixmapByDepth that lets the caller
 *   specify more color information usable during the loading
 *   of the pixmap file to override symbolic colors.
 *
 *   It also supports the negative depth convention where if a -depth
 *   is given, xbm file give Bitmap and xpm file give Pixmap, while
 *   with positive depth, all files give Pixmap result.
 *
 *   Now a wrapper, but still used by IconG, MessageB, and Text directly.
 *
 *******************************************************************/
Pixmap 
_XmGetColoredPixmap(Screen *screen,
		    char *image_name,
		    XmAccessColorData acc_color,
		    int depth,
#if NeedWidePrototypes
		   int only_if_exists)
#else
                   Boolean only_if_exists)
#endif /* NeedWidePrototypes */
{  
    return _XmGetScaledPixmap (screen, NULL,
			       image_name, acc_color, depth,
			       only_if_exists, 1); /* no scaling */
}

Pixmap 
XmGetScaledPixmap(
    Widget widget,
    char *image_name,
    Pixel foreground,
    Pixel background,
    int depth,
    double scaling_ratio)
{  
    XmAccessColorDataRec acc_color_rec;
    Pixmap ret_val;
    XtAppContext app = XtWidgetToApplicationContext(widget);

    _XmAppLock(app);
    _XmProcessLock();
    acc_color_rec.foreground = foreground ;
    acc_color_rec.background = background ;
    acc_color_rec.top_shadow_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.bottom_shadow_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.select_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.highlight_color = XmUNSPECIFIED_PIXEL ;
    ret_val = _XmGetScaledPixmap(XtScreen(widget), widget, image_name, 
				 &acc_color_rec, depth, False,
				 scaling_ratio);
    _XmProcessUnlock();
    _XmAppUnlock(app);

    return  ret_val;

}


/*******************************************************************
 *
 * XmGetPixmapByDepth.
 *  create a pixmap from the image_name.  foreground and background
 *  must be valid values. For depth 1 they should be 1 and 0
 *  respectively. 
 *
 *******************************************************************/
Pixmap 
XmGetPixmapByDepth(
    Screen *screen,
    char *image_name,
    Pixel foreground,
    Pixel background,
    int depth)
{    
    XmAccessColorDataRec acc_color_rec;
    Pixmap ret_val;
    XtAppContext app = XtDisplayToApplicationContext(
				DisplayOfScreen(screen));

    _XmAppLock(app);
    _XmProcessLock();
    acc_color_rec.foreground = foreground ;
    acc_color_rec.background = background ;
    acc_color_rec.top_shadow_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.bottom_shadow_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.select_color = XmUNSPECIFIED_PIXEL ;
    acc_color_rec.highlight_color = XmUNSPECIFIED_PIXEL ;
    ret_val = _XmGetScaledPixmap(screen, NULL, image_name, 
				 &acc_color_rec, depth, False,
				 1);
    _XmProcessUnlock();
    _XmAppUnlock(app);

    return  ret_val;
}


/************************************************************************
 *
 *  This one is deprecated, but keep it in here, it was public.
 *
 ************************************************************************/
			 
Pixmap 
XmGetPixmap(
        Screen *screen,
        char *image_name,
        Pixel foreground,
        Pixel background )
{
    Pixmap	ret_val;

    XtAppContext app = XtDisplayToApplicationContext(
				DisplayOfScreen(screen));
    _XmAppLock(app);
    ret_val = XmGetPixmapByDepth(screen, image_name, foreground, background,
				 DefaultDepthOfScreen(screen));
    _XmAppUnlock(app);

    return (ret_val);
}



/************************************************************************
*
*  XmeGetMask    
*
************************************************************************/
Pixmap 
XmeGetMask(
        Screen *screen,
        char *image_name)
{
    char 		mask_name[256];
    Pixmap		ret_val;
    XtAppContext app = XtDisplayToApplicationContext(
				DisplayOfScreen(screen));

    _XmAppLock(app);
    
    _XmProcessLock();
    _XmOSGenerateMaskName(image_name, mask_name);
    _XmProcessUnlock();

    ret_val = XmGetPixmapByDepth(screen, mask_name, 1, 0, 1);
    
    _XmAppUnlock(app);
    return ret_val;
}


/************************************************************************
 *
 *  XmDestroyPixmap
 *	Use the pixmap table (pixmap/screen) to find the pixmap,
 *      then remove it from both pixmap and pixmap_data tables.
 *
 ************************************************************************/
Boolean 
XmDestroyPixmap(
        Screen *screen,
        Pixmap pixmap )
{
   PixmapData pix_data, *pix_entry ;
   XtAppContext app;

   /*  Check for invalid conditions  */

   if (screen == NULL || pixmap == 0 || pixmap_set == NULL)
     return (False);

   app = XtDisplayToApplicationContext(
				DisplayOfScreen(screen));

   _XmAppLock(app);
   _XmProcessLock();

   pix_data.screen = screen ;
   pix_data.pixmap = pixmap ;

   if ((pix_entry = (PixmapData*) 
	_XmGetHashEntry(pixmap_set, (XmHashKey)&pix_data)) != NULL) {
      pix_entry->reference_count--;
      if (pix_entry->reference_count == 0) {
	  _XmRemoveHashEntry (pixmap_data_set, pix_entry);
	  _XmRemoveHashEntry (pixmap_set, pix_entry);
   	  if (0 != strcmp(pix_entry->image_name, DIRECT_PIXMAP_CACHED))
	    XFreePixmap(DisplayOfScreen(pix_entry->screen), pix_entry->pixmap);
	  XtFree(pix_entry->image_name);
	  XtFree((char*)pix_entry->acc_color);
	  if (pix_entry->pixels) {
	      FreeCacheColors(DisplayOfScreen(pix_entry->screen),
			      DefaultColormapOfScreen(pix_entry->screen),
			      pix_entry->pixels, pix_entry->npixels, NULL);
	      XmeXpmFree(pix_entry->pixels);
	  }
	  XtFree((char*)pix_entry);
      }

      _XmProcessUnlock();
      _XmAppUnlock(app);
      return True;
  }

  _XmProcessUnlock(); 
  _XmAppUnlock(app);
  return False;
}


/* Compare two gc entry. Only care about the colors for depth 1 */
static Boolean 
CompareGCDatas (XmHashKey key_1, 
		XmHashKey key_2)
{
  GCData *data_1 = (GCData *) key_1;
  GCData *data_2 = (GCData *) key_2;

  return ((data_1->screen == data_2->screen) &&
	  (data_1->print_shell == data_2->print_shell) &&
	  (data_1->depth == data_2->depth) &&
	  ((data_1->image_depth != 1) ||
	   ((data_1->image_depth == 1) &&
	   (data_1->foreground == data_2->foreground) &&
	   (data_1->background == data_2->background)))) ;
}

/* Hash a GC entry. Only care about the colors for depth 1 */
static XmHashValue 
HashGCData (XmHashKey key)
{
  GCData *data = (GCData *) key;
  XmHashValue hv = 0 ;
 
  if (data->image_depth == 1)
    hv = data->foreground + data->background ;

  return (hv + (long)data->screen + (long)data->print_shell +
	  data->depth + data->image_depth);
}

/***
 * GetGCForPutImage
 * Maintain a cache of GC to use for transfering the XImage to
 * a Pixmap depending on the screen, depth, colors.
 ***/
static GC 
GetGCForPutImage(Screen *screen,
		 Widget print_shell,
		 XImage * image, 
		 Pixmap pixmap, 
		 int depth,
		 Pixel foreground,
		 Pixel background)
{
    XGCValues gcValues;
    GCData gc_data, *gc_entry ;

    /* There is a problem here: we cannot simply use the xmScreen object
     * to store the GCs, for the pixmap conversion can be called
     * at a time the xmScreen is not yet available: a conversion
     * for the first VendorShell on this screen, or a conversion for
     * the xmScreen or xmDisplay.
     * We can either arrange for delaying the conversion for those cases,
     * or not use the xmScreen here. Since we need to cache per depth
     * in addition to per screen, and since the Xt routine cannot be used
     * since there is no widget available, let's forget about using the
     * xmScreen object, do our own specialized caching.
     */

    /* There shouldn't be a lot of different GCs, one for
     * depth 1 to depth 1, foreground 1, back 0, one for 
     * depth N to N, fore/back don't matter and some for depth 1 to
     * depth N, where colors matter, for backgroundPixmap resources 
     * and regular use
     * of XmGetPixmapByDepth (with no -depth) issued from XBM file.
     */

    _XmProcessLock();
    if (gc_set == NULL) 
	gc_set = _XmAllocHashTable (20, CompareGCDatas, HashGCData);
    _XmProcessUnlock();
    
    /* set up an entry for search. We only care about the colors
       for image_depth 1 to pixmap depth (N or 1), since otherwise, N to N,
       the transfer is a plain copy */
    gc_data.screen = screen ;    
    gc_data.print_shell = print_shell ;
    gc_data.depth = depth ;    
    gc_data.image_depth = image->depth ;    
    gc_data.foreground = foreground ;
    gc_data.background = background ;

    /* look if we have a match and return it */
    _XmProcessLock();
    if ((gc_entry = (GCData*) 
	 _XmGetHashEntry(gc_set, (XmHashKey)&gc_data)) != NULL) {
	_XmProcessUnlock();
	return gc_entry->gc ;
    }
    _XmProcessUnlock();
	
    /* create a new GC, cache it and return it */
    gc_entry = XtNew(GCData);
    gc_entry->screen = screen ;    
    gc_entry->print_shell = print_shell ;
    gc_entry->depth = depth ;    
    gc_entry->image_depth = image->depth ;    
    gc_entry->foreground = foreground ;
    gc_entry->background = background ;
    _XmProcessLock();
    _XmAddHashEntry(gc_set, (XmHashKey)gc_entry, (XtPointer)gc_entry);
    _XmProcessUnlock();

    gcValues.foreground = foreground;
    gcValues.background = background;
    gc_entry->gc = XCreateGC (DisplayOfScreen(screen), pixmap,
			     GCForeground | GCBackground, &gcValues);

    return gc_entry->gc ;
}
 

/************
 When a Screen is closed, or a PrintShell, we need to clean up the
 respective cache .
*****/


static Boolean 
CleanGCMapProc (XmHashKey key, /* unused */
		 XtPointer value,
		 XtPointer data)
{
  GCData *entry = (GCData *) value;
  CleanKey * ck = (CleanKey*) data ;

  /* shell should be NULL for non printing screen */
  if (entry->print_shell == ck->shell &&
      entry->screen == ck->screen)
    {
	  _XmProcessLock();
	  _XmRemoveHashEntry (gc_set, entry);
	  _XmProcessUnlock();
	  XFreeGC(DisplayOfScreen(entry->screen), entry->gc);
	  XtFree((char*) entry);
    }

  /* never return True: remove all gc keyed on this print shell */
  return False;
}

static Boolean 
CleanPixmapMapProc (XmHashKey key, /* unused */
		     XtPointer value,
		     XtPointer data)
{
  PixmapData *entry = (PixmapData *) value;
  CleanKey * ck = (CleanKey*) data ;

  /* shell should be NULL for non printing screen */
  if (entry->print_shell == ck->shell &&
      entry->screen == ck->screen)
    {
	/* this should take care of everything */
	XmDestroyPixmap(entry->screen, entry->pixmap);
    }

  /* never return True: remove all pixmap keyed on this print shell */
  return False;
}

#ifdef XP_DEBUG
static Boolean 
PrintPixmapMapProc (XmHashKey key, /* unused */
		    XtPointer value,
		    XtPointer data)
{
  PixmapData *entry = (PixmapData *) value;
  CleanKey * ck = (CleanKey*) data ;

  printf("entry->screen %d\n", entry->screen);
  printf("entry->print_shell %d\n", entry->print_shell);
  printf("entry->image_name %s\n", entry->image_name);
  printf("entry->pixmap %d\n\n", entry->pixmap);

  return False;
}

static Boolean 
PrintGCMapProc (XmHashKey key, /* unused */
		XtPointer value,
		XtPointer data)
{
  GCData *entry = (GCData *) value;
  CleanKey * ck = (CleanKey*) data ;

  printf("entry->screen %d\n", entry->screen);
  printf("entry->print_shell %d\n", entry->print_shell);
  printf("entry->gc %d\n\n", entry->gc);

  return False;
}
#endif
/******
 This is called from PrintShell.Destroy to invalidate the
 GCs and Pixmap used for a given print shell and from Screen.destroy,
 with a NULL shell, to remove the GCs and Pixmap used for a 
 given screen. So in the case of PrintShell, this is call twice
 in a row, where the second time (from Screen) takes care of
 the Screen (non print shell) specific pixmaps abd GCs.
*******/
void 
_XmCleanPixmapCache(Screen * screen, Widget shell)
{
    CleanKey ck ;

    ck.screen = screen;
    ck.shell = shell ;

    _XmMapHashTable(gc_set, CleanGCMapProc, &ck);
    _XmMapHashTable(pixmap_set, CleanPixmapMapProc, &ck);
}




/************************************************************************
*******************************************************************------
 
  Design notes:

  There is really two different caches here, an XImage cache and
  a Pixmap cache (+ a GC cache).

  The XImage cache is name based (hash key is image_name). No colors are
  supported, and therefore, it is only useful with depth 1 XImage.
  
  Program can directly install non-depth 1 XImage in this cache using 
  the public API (XmInstallImage), but that's not advised, for the
  non-depth 1 XImage resulting from XPM file have color variations
  for the same image_name. If you ask for image_name Foo, foreground 
  red and background blue, and a Foo has been installed in the image
  cache using different foreground & background, you don't want
  to get back this Foo, but a new instance of Foo, one that uses
  the red and blue colors.

  Also, the only access to the XImage reader (GetImage) is via the pixmap 
  API, so if a XPM with no colors sensitivity (no pertinent Motif
  symbolic colors) is read once, it will be found by the
  pixmap cache first, no need to cache it in the image cache.

  The Pixmap cache is name/depth/colors or pixmap based, depending
  which API is looking up.
  The old XmGetPixmapByDepth() is equivalent to asking for
  valid foreground/background and unspecified others, which
  means these additional colors (shadows, select) will not be
  used during Xpm reading for overriding symbolic colors.

  Motifnext adds printing support and clean cache support.

***/


#define roundint(x)                   MAX(1, (int)((x) + 0.5))

typedef struct {
  Position *x, *y;
  Dimension *width, *height;
} Table;

void _XmPutScaledImage (    
    Display*		 display,
    Drawable		 d,
    GC			 gc,
    XImage*		 src_image,
    int			 src_x,
    int			 src_y,
    int			 dest_x,
    int			 dest_y,
    unsigned int	 src_width,
    unsigned int	 src_height, 
    unsigned int	 dest_width,  
    unsigned int	 dest_height) 
{
    XImage *dest_image;
    Position x, y, min_y, max_y, src_max_x;
    Dimension w, h, strip_height;
    Table table;    
    Pixel pixel;
    double ratio_x, ratio_y;
    Bool fast8;
    int xp_event, xp_error;

    if (dest_width == src_width && dest_height == src_height) {
	/* same for x and y, just send it out */
	XPutImage(display, d, gc, src_image, src_x, src_y, 
		  dest_x, dest_y, dest_width, dest_height); 
	return;
    }

    ratio_x = (double)dest_width / (double)src_width;
    ratio_y = (double)dest_height / (double)src_height;
 
    /*
     * Check that we have uniform scaling, and that the print extension
     * exists.  We can't call XpGetContext first, because if the print
     * extension doesn't exist we'll get a warning to stderr.
     * It would be better if the print context was passed in directly, so
     * that we didn't spend a round trip when scaling for the video screen,
     * but it's just one round trip on just the first call per display, so
     * it's not that bad.
     */
    h = (double)src_height * ratio_x + 0.5;
    if (dest_height <= h + 1 && h <= dest_height + 1 &&
	XpQueryExtension(display, &xp_event, &xp_error)) {
	/*
	 * We could be clever and try to get the print context first from
	 * the _XmPrintScreenToShellContext and then get the resolution from
	 * the print shell, but we don't have a Screen* to do the lookup.
	 */
	XPContext pcontext = XpGetContext(display);
	if (pcontext) {
	    int media_res;
	    media_res = atoi(XpGetOneAttribute(display, pcontext, XPDocAttr,
					       "default-printer-resolution"));
	    if (media_res) {
		int image_res = (double)media_res / ratio_x + 0.5;
		int prev_res;
		if (XpSetImageResolution(display, pcontext,
					 image_res, &prev_res)) {
		    XPutImage(display, d, gc, src_image, src_x, src_y,
			      dest_x, dest_y, src_width, src_height);
		    XpSetImageResolution(display, pcontext, prev_res, NULL);
		    return;
		}
	    }
	}
    }

    src_max_x = src_x + src_width;

    strip_height = 65536 / roundint(ratio_x * src_image->bytes_per_line);
    if (strip_height == 0)
	strip_height = 1;
    if (strip_height > dest_height)
	strip_height = dest_height;

    h = strip_height + roundint(ratio_y);
    dest_image = XCreateImage(display,
			      DefaultVisualOfScreen(
					     DefaultScreenOfDisplay(display)),
			      src_image->depth, src_image->format, 
			      0, NULL,
			      dest_width, h,
			      src_image->bitmap_pad, 0);
    dest_image->data = XtMalloc(dest_image->bytes_per_line * h);
    fast8 = (src_image->depth == 8 && src_image->bits_per_pixel == 8 &&
	     dest_image->bits_per_pixel == 8 && src_image->format == ZPixmap);

    table.x = (Position *) XtMalloc(sizeof(Position) * (src_image->width + 1));
    table.y = (Position *) XtMalloc(sizeof(Position) * (src_image->height + 1));
    table.width = (Dimension *) XtMalloc(sizeof(Dimension) * src_image->width);
    table.height = (Dimension *) XtMalloc(sizeof(Dimension)*src_image->height);
    
    table.x[0] = 0;
    for (x = 1; x <= src_image->width; x++) {
	table.x[x] = roundint(ratio_x * x);
	table.width[x - 1] = table.x[x] - table.x[x - 1];
    }

    table.y[0] = 0;
    for (y = 1; y <= src_image->height; y++) {
	table.y[y] = roundint(ratio_y * y);
	table.height[y - 1] = table.y[y] - table.y[y - 1];
    }

    for (min_y = table.y[src_y]; min_y < dest_height; min_y = table.y[y]) {
	max_y = min_y + strip_height;
	if (max_y > dest_height) {
	    strip_height = dest_height - min_y;
	    max_y = dest_height;
	}
	for (y = src_y; table.y[y] < max_y; y++) {
	    if (table.y[y] < min_y)
		continue;
	    if (fast8) {
		for (x = src_x; x < src_max_x; x++) {
		    pixel = ((unsigned char *)src_image->data)
			[y * src_image->bytes_per_line + x];
		    for (h = 0; h < table.height[y]; h++) {
			memset(dest_image->data +
			       (table.y[y] + h - min_y) *
			       dest_image->bytes_per_line + table.x[x],
			       pixel, table.width[x]);
		    }
		}
	    } else {
		for (x = src_x; x < src_max_x; x++) {
		    pixel = XGetPixel(src_image, x, y);
		    for (h = 0; h < table.height[y]; h++) {
			for (w = 0; w < table.width[x]; w++)
			    XPutPixel(dest_image,
				      table.x[x] + w,
				      table.y[y] + h - min_y,
				      pixel);
		    }
		}
	    }
	}
	XPutImage(display, d, gc, dest_image, dest_x, 0,
		  dest_x, dest_y + min_y, dest_width, table.y[y] - min_y);
	if (y >= src_image->height)
	    break;
    }
    
    XtFree((char *)table.x);
    XtFree((char *)table.y);
    XtFree((char *)table.width);
    XtFree((char *)table.height);

    XDestroyImage(dest_image);
}



/*** COLOR CACHE NOW ***/


static Boolean
GetCacheColorByName( Display *display, Colormap colormap,
		     char *colorname, XColor *xcolor)
{
    static Boolean firstTime = True;
    XrmQuark colorname_q;
    int i;
    CachedColor *color;

    if (!colorname)
	return False;

    if ( firstTime )
    {
       colorCacheList.numEntries = colorCacheList.maxEntries = 0;
       colorCacheList.cache = NULL;
       firstTime = False; 
       return False;
    }

    colorname_q = XrmStringToQuark(colorname);
	
    for (i = 0, color = colorCacheList.cache; i < colorCacheList.numEntries;
	 i++, color++)
    {
	if (color->colorname == colorname_q &&
            color->colormap == colormap &&
            color->display == display)
        {
	   xcolor->pixel = color->pixel;
	   color->num_cached++;
           return True;
        }
    }
    return False;
}

static Boolean
GetCacheColorByRGB( Display *display, Colormap colormap, XColor *xcolor)
{
    int i;
    CachedColor *color;

    for (i = 0, color = colorCacheList.cache; i < colorCacheList.numEntries;
	 i++, color++)
    {
	if (color->colormap == colormap && color->display == display &&
	    color->red == xcolor->red &&
	    color->green == xcolor->green &&
	    color->blue == xcolor->blue)
        {
	   xcolor->pixel = color->pixel;
	   color->num_cached++;
           return True;
        }
    }
    return False;
}

static Boolean
FreeCacheColor( Display *display, Colormap colormap, Pixel pixel)
{
    int i;
    CachedColor *color;

    for (i = 0, color = colorCacheList.cache; i < colorCacheList.numEntries;
	 i++, color++)
    {
	if (color->colormap == colormap && color->display == display &&
	    color->pixel == pixel)
        {
	    color->num_cached--;
	    if (color->num_cached == 0) {
		int j;
		for (j = i + 1; j < colorCacheList.numEntries; i++, j++)
		    colorCacheList.cache[i] = colorCacheList.cache[j];
		colorCacheList.numEntries--;
		XFreeColors(display, colormap, &pixel, 1, 0);
		return True;
	    }
	}
    }
    return False;
}

static void
CacheColorPixel( Display *display, Colormap colormap,
                 char *colorname, XColor *xcolor)
{
    int numEntries = colorCacheList.numEntries;

    if (numEntries == colorCacheList.maxEntries)
    {
        colorCacheList.maxEntries += 25;
	colorCacheList.cache = (CachedColor *)
				  XtRealloc((char *)colorCacheList.cache,
					    colorCacheList.maxEntries *
				            sizeof(CachedColor));
    }

    colorCacheList.cache[numEntries].display = display;
    colorCacheList.cache[numEntries].colormap = colormap;
    colorCacheList.cache[numEntries].colorname =
	colorname ? XrmStringToQuark(colorname) : NULLQUARK;
    
    colorCacheList.cache[numEntries].red = xcolor->red;
    colorCacheList.cache[numEntries].green = xcolor->green;
    colorCacheList.cache[numEntries].blue = xcolor->blue;
    colorCacheList.cache[numEntries].pixel = xcolor->pixel;
    colorCacheList.cache[numEntries].num_cached = 1;
                                                      
    colorCacheList.numEntries++;
}


static int
GetCacheColor(Display *display, Colormap colormap,
	      char *colorname, XColor *xcolor, void *closure)
{
    int status;
    if (colorname) {
	/* look for the colorname in the cache */
	if (!GetCacheColorByName(display, colormap, colorname, xcolor)) {
	    /* when not found parse and alloc color */
	    if (!XParseColor(display, colormap, colorname, xcolor))
		return -1;
	    if (!GetCacheColorByRGB(display, colormap, xcolor)) {
		status = XAllocColor(display, colormap, xcolor);
		if (status)	/* if allocation succeeded, store color */
		    CacheColorPixel(display, colormap, colorname, xcolor);
	    } else
		status = 1;
	} else
	    status = 1;
    } else {
	if (!GetCacheColorByRGB(display, colormap, xcolor)) {
	    status = XAllocColor(display, colormap, xcolor);
	    if (status)		/* if allocation succeeded, store color */
		CacheColorPixel(display, colormap, colorname, xcolor);
	} else
	    status = 1;
    }

    return status != 0 ? 1 : 0;
}

static int
FreeCacheColors(Display *display, Colormap colormap,
		Pixel *pixels, int n, void *closure)
{
    int i, status;
    for (i = 0; i < n; i++, pixels++)
	status = FreeCacheColor(display, colormap, *pixels);
    return 0;
}
