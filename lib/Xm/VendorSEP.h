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
 * HISTORY
*/ 
/* $XConsortium: VendorSEP.h /main/14 1996/05/21 12:11:50 pascale $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmVendorSEP_h
#define _XmVendorSEP_h

#include <Xm/ShellEP.h>
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsVendorShellExt
#define XmIsVendorShellExt(w)	XtIsSubclass(w, xmVendorShellExtObjectClass)
#endif /* XmIsVendorShellExt */

typedef struct _XmVendorShellExtRec *XmVendorShellExtObject;
typedef struct _XmVendorShellExtClassRec *XmVendorShellExtObjectClass;
externalref WidgetClass xmVendorShellExtObjectClass;


#define XmInheritProtocolHandler	((XtCallbackProc)_XtInherit)

typedef struct _XmVendorShellExtClassPart{
    XtCallbackProc	delete_window_handler;
    XtCallbackProc	offset_handler;
    XtPointer		extension;
}XmVendorShellExtClassPart, *XmVendorShellExtClassPartPtr;

typedef struct _XmVendorShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
}XmVendorShellExtClassRec;

typedef struct {
 XmFontList		default_font_list;
 unsigned char		focus_policy;
 XmFocusData		focus_data;
 unsigned char		delete_response;
 unsigned char		unit_type;
 MwmHints		mwm_hints;
 MwmInfo		mwm_info;
 String			mwm_menu;
 XtCallbackList		focus_moved_callback;
 /*
  * internal fields
  */
 Widget			old_managed;
 Position		xAtMap, yAtMap, xOffset, yOffset;
 unsigned long		lastOffsetSerial;
 unsigned long		lastMapRequest;
 Boolean		externalReposition;

 /* mapStyle is an unused field. I'm using this field to keep
  * track of the *font_list resource values. Refer 
  * CheckSetRenderTable in VendorSE.c
  */
 unsigned char		mapStyle;

 XtCallbackList		realize_callback;
 XtGrabKind		grab_kind;
 Boolean		audible_warning;
 XmFontList             button_font_list;
 XmFontList             label_font_list;
 XmFontList             text_font_list;
 String			input_method_string;
 String			preedit_type_string;
 unsigned int           light_threshold;
 unsigned int           dark_threshold;
 unsigned int           foreground_threshold;
 unsigned int		im_height;
 XtPointer		im_info;
 Boolean		im_vs_height_set;

 /* New public resources for Motif 2.0 */
 XmDirection            layout_direction;
 XmInputPolicy		input_policy;

 Boolean 		verify_preedit;
#ifdef M222i
 Widget         pad_label;
 int            pad_post_delay;
 int            pad_post_duration;
 XtIntervalId   pad_timer;
 XtIntervalId   pad_duration_timer;
 Time           pad_leave_time;
 Widget         pad_slider;
 Boolean        pad_enable;
#endif /* M222i */
} XmVendorShellExtPart, *XmVendorShellExtPartPtr;

externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;

typedef struct _XmVendorShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
} XmVendorShellExtRec;


/******** Xme Functions ********/

void XmeAddFocusChangeCallback(Widget, XtCallbackProc, XtPointer);
void XmeRemoveFocusChangeCallback(Widget, XtCallbackProc, XtPointer);

/******** End Xme Functions ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmVendorSEP_h */
