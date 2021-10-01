:PACKAGE: X11

GENCAT    = gencat
MAKESTRS  = $(ROOT)/config/util/makestrs
MKCATDEFS = $(ROOT)/localized/util/mkcatdefs

.DO.MAKESTRS : .USE (MAKESTRS)
	$(MAKESTRS) < $(>) > $(<)

":STRING_TABLE:" : .OPERATOR .MAKE
	.CLOBBER. += $(<:N=*.c)
	$(<) : .JOINT .DO.MAKESTRS $(*)

.DO.MKCATDEFS : .USE (MKCATDEFS)
	$(MKCATDEFS) $(<:O=1) $(>) > $(<:O=2)

%.cat : %.msg (GENCAT)
	$(GENCAT) $(<) $(>)

":MESSAGE_CATALOG:" : .OPERATOR .MAKE
	$(<)MsgCatI.h $(ROOT)/localized/C/msg/$(<).msg : .JOINT .DO.MKCATDEFS $(<).msg
	$(<).cat : $(ROOT)/localized/C/msg/$(<).msg
