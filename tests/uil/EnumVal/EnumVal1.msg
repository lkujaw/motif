! 
! Motif
!
! Copyright (c) 1987-2012, The Open Group. All rights reserved.
!
! These libraries and programs are free software; you can
! redistribute them and/or modify them under the terms of the GNU
! Lesser General Public License as published by the Free Software
! Foundation; either version 2 of the License, or (at your option)
! any later version.
!
! These libraries and programs are distributed in the hope that
! they will be useful, but WITHOUT ANY WARRANTY; without even the
! implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
! PURPOSE. See the GNU Lesser General Public License for more
! details.
!
! You should have received a copy of the GNU Lesser General Public
! License along with these librararies and programs; if not, write
! to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
! Floor, Boston, MA 02110-1301 USA
! 
! 
! HISTORY
! 
!   $XConsortium: EnumVal1.msg /main/7 1995/07/13 20:26:43 drk $
!OSF/Motif: @(#)EnumVal1.msg	3.2  92/01/31
!
!  
!  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
!  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
!  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
!  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
!  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
!  SOFTWARE IS HEREBY TRANSFERRED.
!  
!  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
!  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
!  
!  
!  
!  
!  
!  
!  
!  
!  
! DEC is a registered trademark of Digital Equipment Corporation
! DIGITAL is a registered trademark of Digital Equipment Corporation
!
value
  message :
	"Enumerated Values Test 1:\n\n" &
	"The following is a Bulletin Board set up to test correct\n" &
	"usage of enumeration values. (Note: this explanation text\n" &
	"itself tests the following:\n" &
	"  - XmNeditMode = XmMULTI_LINE_EDIT\n" &
	"Bulletin Board characteristics:\n" &
	"  - XmNborderWidth = 1\n" &
	"  - XmNbackground =  sky_blue\n" &
	"  - XmNmarginWidth =  20\n" &
	"  - XmNmarginHeight =  20\n" &
	"  - XmNdefaultPosition =  false\n" &
	"  - XmNnoResize =  true\n" &
	"  - XmNResizePolicy = XmRESIZE_GROW\n" &
	"  - XmNshadowType = XmSHADOW_ETCHED_OUT\n" &
	"  - XmNshadowThickness = 10\n" &
	"ScrolledList characteristics:\n" &
	"  - XmNwidth = 200\n" &
	"  - XmNheight = 80\n" &
	"  - XmNshadowThickness = 4\n" &
	"  - XmNitems = string_table...\n" &
	"  - XmNvisibleItemCount = 7 (*)\n" &
	"  - XmNselectionPolicy = XmMULTIPLE_SELECT\n" &
	"  - XmNlistSizePolicy = XmCONSTANT\n" &
	"  - XmNscrollBarDisplayPolicy = XmAS_NEEDED\n" &
	"  - XmNscrollBarPlacement = XmTOP_LEFT\n" &
	"(*) visibleItemCount has lower priority than height";
