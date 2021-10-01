# $TOG: Makefile /main/18 1999/10/12 09:31:01 mgreess $
# Motif
#
# Copyright (c) 1987-2012, The Open Group. All rights reserved.
#
# These libraries and programs are free software; you can
# redistribute them and/or modify them under the terms of the GNU
# Lesser General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# These libraries and programs are distributed in the hope that
# they will be useful, but WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with these librararies and programs; if not, write
# to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
# Floor, Boston, MA 02110-1301 USA
#
# HISTORY
#
# Makefile.ini
#
# This is a toplevel Makefile for bootstrapping an initial build of Motif
# Release 2.x.  It provides two targets: Makefile (for the cautious) and
# World (for the bold).  After the full Makefile has been produced, that
# should be used.  However, if it is accidentally corrupted, or a build
# is initiated on a new platform, Makefile.ini can be used to restart.
#
       RELEASE = "Release 2.1.32"
       PRODUCT = "Motif"
         SHELL = /bin/sh
            RM = rm -f
            MV = mv
         NMAKE = nmake
     WORLDOPTS = -k
WIN32WORLDOPTS = -i
           TOP = .
   CURRENT_DIR = .
     CONFIGSRC = $(TOP)/config
      IMAKESRC = $(CONFIGSRC)/imake
     DEPENDSRC = $(CONFIGSRC)/makedepend
     DEPENDTOP = ../..
      IMAKETOP = ../..
      IRULESRC = $(CONFIGSRC)/cf
         IMAKE = $(IMAKESRC)/imake
     IMAKE_CMD = $(IMAKE) -I$(IRULESRC) $(IMAKE_DEFINES)
     MAKE_OPTS = -f xmakefile
      MAKE_CMD = $(MAKE) $(MAKE_OPTS)
     NMAKE_CMD = $(NMAKE) $(MAKE_OPTS)
         FLAGS = $(MFLAGS) -f Makefile.ini BOOTSTRAPCFLAGS="$(BOOTSTRAPCFLAGS)"

all:
	@$(MAKE_CMD) xmakefile-exists || $(MAKE) all-initial
	@$(MAKE_CMD) $@

all-initial:
	@echo "Please begin by using make World or make Makefile.  You may"
	@echo "find it necessary to set the make variable BOOTSTRAPCFLAGS in"
	@echo "order to compile imake.  One common example is"
	@echo "\"BOOTSTRAPCFLAGS=-I<dir>\", where <dir> is the top of your"
	@echo "X11R6 source tree.  Remember to check the configuration"
	@echo "parameters in the config directory."
	@echo
	@echo "We recommend that you read the release notes carefully before"
	@echo "proceeding; they can be found under the doc directory.  If"
	@echo "the top-level Makefile gets corrupted, copy Makefile.ini to"
	@echo "Makefile and try again.  Do not name your log file make.log or"
	@echo "it will be deleted."

World:
	@echo ""
	@echo "Building $(RELEASE) of $(PRODUCT)"
	@echo ""
	@case "x$(BOOTSTRAPCFLAGS)" in x) \
	echo I hope you checked the configuration parameters in $(IRULESRC) ; \
	echo to see if you need to pass BOOTSTRAPCFLAGS. ; \
	echo "" ; \
	;; esac;
	@date
	@echo ""
	cd $(IMAKESRC) && $(MAKE) $(FLAGS) clean
	$(MAKE) $(MFLAGS) Makefile.boot
	$(MAKE_CMD) $(MFLAGS) VerifyOS
	$(MAKE_CMD) $(MFLAGS) Makefiles
	$(MAKE_CMD) $(MFLAGS) testsMakefile
	$(MAKE_CMD) $(MFLAGS) clean
	$(MAKE_CMD) $(MFLAGS) includes
	$(MAKE_CMD) $(MFLAGS) depend
	$(MAKE_CMD) $(MFLAGS) $(WORLDOPTS)
	@echo ""
	@date
	@echo ""
	@echo "Full build of $(RELEASE) of $(PRODUCT) complete."
	@echo ""

.PRECIOUS: Makefile

# This is just a sequence of bootstrapping steps we have to do.
# The steps are listed as separate targets so clearmake can wink in
# the Makefile.proto files.
Makefile.boot: imake.proto $(DEPENDSRC)/Makefile.proto depend.bootstrap $(IMAKESRC)/Makefile.proto imake.bootstrap

imake.proto:
	cd $(IMAKESRC) && $(MAKE) $(FLAGS)
	$(RM) $(DEPENDSRC)/Makefile.proto

$(DEPENDSRC)/Makefile.proto:
	$(IMAKE_CMD) -s $(DEPENDSRC)/Makefile.proto -f $(DEPENDSRC)/Imakefile -DTOPDIR=$(DEPENDTOP) -DCURDIR=$(DEPENDSRC)

depend.bootstrap:
	cd $(DEPENDSRC) && $(RM) -r Makefile Makefile.dep makedepend *.o bootstrap
	cd $(DEPENDSRC) && $(MAKE) -f Makefile.proto bootstrap

$(IMAKESRC)/Makefile.proto:
	$(IMAKE_CMD) -s $(IMAKESRC)/Makefile.proto -f $(IMAKESRC)/Imakefile -DTOPDIR=$(IMAKETOP) -DCURDIR=$(IMAKESRC)

imake.bootstrap:
	cd $(IMAKESRC) && $(MAKE) -f Makefile.proto bootstrapdepend
	cd $(IMAKESRC) && $(MAKE) $(FLAGS) bootstrap
	cd $(IMAKESRC) && $(MAKE) -f Makefile.proto all
	-@if [ -f xmakefile ]; then set -x; \
	  $(RM) xmakefile.bak; $(MV) xmakefile xmakefile.bak; \
	  else exit 0; fi
	$(MAKE) $(MFLAGS) xmakefile

Makefile::
	$(MAKE) $(MFLAGS) xmakefile

xmakefile: Imakefile
	$(IMAKE_CMD) -s xmakefile $(MOTIFBUILDEXTRAS) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)

World.Win32:
	@echo :
	@echo Building $(RELEASE) of $(PRODUCT).
	@echo :
	@echo :
	cd $(IMAKESRC)
	$(NMAKE) -f Makefile.ini clean.Win32
	$(NMAKE) -f Makefile.ini imake.exe
	cd ..\..
	-if exist xmakefile.bak del xmakefile.bak
	-if exist xmakefile ren xmakefile xmakefile.bak
	$(IMAKE:/=\) -s xmakefile -I$(IRULESRC) $(IMAKE_DEFINES) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)
	$(NMAKE_CMD) $(MFLAGS) VerifyOS
	$(NMAKE_CMD) $(MFLAGS) Makefiles
	$(NMAKE_CMD) $(MFLAGS) testsMakefile
	$(NMAKE_CMD) $(MFLAGS) clean
	$(NMAKE_CMD) $(MFLAGS) includes
	$(NMAKE_CMD) $(MFLAGS) depend
	$(NMAKE_CMD) $(MFLAGS) $(WIN32WORLDOPTS)
	@echo :
	@echo :
	@echo Full build of $(RELEASE) of $(PRODUCT) complete.
	@echo :


# don't allow any default rules in this Makefile
.SUFFIXES:

# a copy of every rule that might be invoked at top level

clean:
	$(MAKE_CMD) $@
dangerous_strip_clean:
	$(MAKE_CMD) $@
depend:
	$(MAKE_CMD) $@
Everything:
	$(MAKE_CMD) $@
external.ln:
	$(MAKE_CMD) $@
includes:
	$(MAKE_CMD) $@
install.ln:
	$(MAKE_CMD) $@
install.man:
	$(MAKE_CMD) $@
install:
	$(MAKE_CMD) $@
install.cde:
	$(MAKE_CMD) $@
Makefiles:
	$(MAKE_CMD) $@
man_keywords:
	$(MAKE_CMD) $@
tags:
	$(MAKE_CMD) $@
VerifyOS:
	$(MAKE_CMD) $@
