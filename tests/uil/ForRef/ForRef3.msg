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
!   $XConsortium: ForRef3.msg /main/7 1995/07/13 20:29:49 drk $
!OSF/Motif: @(#)ForRef3.msg	3.2  92/01/31
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
	"This is a list widget with the following\ncharacteristics:" &
	"\n\n   - width is 150, height is 200\n   - background is navy\n   " &
	"- items are \n     item1, item2, item3, item4, item5\n   - itemCount" &
	" is 5\n   - visibleItemCount is 5\n   - automaticSelection is true\n" &
	"   - selectionPolicy is \n     XmBROWSE_SELECT\n" &
	"   - fontList is variable\n" &
	"   - listMarginWidth is 50\n   - listMarginHeight is 50\n" &
	"   - browseSelectionCallback is ON\n" &
	"   - defaultActionCallback is ON\n" &
	"\nAll other reources are set by default.\n" &
	"\nThe list forward references the list of\n"&
	"itmes used and the color navy.\n";
