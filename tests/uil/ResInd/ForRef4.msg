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
!   $XConsortium: ForRef4.msg /main/4 1995/07/13 20:37:31 drk $
!OSF/Motif: @(#)ForRef4.msg	3.2  92/01/31
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
	"This is a label widget with the \nfollowing charcteristics:\n\n" &
	"   - unitType is Xm1000TH_INCHES\n   - width is 2000\n" &
	"   - borderWidth is 10\n   - background color is blue\n   - traversalOn is true\n   -" &
	" highlightThickness is 60\n   - highlightColor is Red\n   - labelString is \n     This is " &
	"a Label\n   - recomputeSize is true\n   - helpCallback is ON via a\n     translation to <Btn3Down>\n" &
	"\nAll other resources are set by default.\n" &
	"\nThe Label forward references the translation to Help().\n";
