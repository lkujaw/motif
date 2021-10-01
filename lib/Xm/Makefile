ROOT = ../..
include $(ROOT)/motif.mk

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

:ALL:

XmStrDefs.c XmStrDefs.h XmStrDefsI.h :STRING_TABLE: \
	xmstring.list XmStrDefs.ct XmStrDefs.ht XmStrDefsI.ht

Xm :MESSAGE_CATALOG:

BASE_SRC = ArrowB.c ArrowBG.c AtomMgr.c BBUtil.c BaseClass.c		\
	BulletinB.c Cache.c CallbackI.c CareVisualT.c CascadeB.c	\
	CascadeBG.c ChColor.c ClipWindow.c ColObjFunc.c Color.c		\
	ColorObj.c Command.c CutPaste.c CvtUnit.c Desktop.c Dest.c	\
	DialogS.c DialogSE.c Direction.c Display.c DrArrow.c		\
	DrHiDash.c DrPoly.c DrTog.c DragBS.c DragC.c DragICC.c		\
	DragIcon.c DragOverS.c DragUnder.c Draw.c DrawingA.c DrawnB.c	\
	DropSMgr.c DropSMgrI.c DropTrans.c ExtObject.c FileSB.c		\
	FocusAct.c Form.c Frame.c GMUtils.c Gadget.c GadgetUtil.c	\
	GeoUtils.c GetPixData.c GetRes.c Hash.c IconFile.c		\
	ImageCache.c IsMwmRun.c Label.c LabelG.c List.c MainW.c		\
	Manager.c MapEvents.c MenuProc.c MenuShell.c MenuState.c	\
	MenuUtil.c MessageB.c Messages.c PanedW.c PixConv.c		\
	Primitive.c PrintS.c Protocols.c PushB.c PushBG.c RCHook.c	\
	RCLayout.c RCMenu.c RCPopup.c ReadImage.c Region.c RepType.c	\
	ResConvert.c ResEncod.c ResInd.c Resolve.c RowColumn.c Sash.c	\
	ScalTics.c Scale.c Screen.c ScroVis.c ScrollBar.c		\
	ScrollFrameT.c ScrolledW.c SelectioB.c SeparatoG.c		\
	Separator.c ShellE.c Simple.c Synthetic.c TearOff.c		\
	TearOffB.c Text.c TextF.c TextFSel.c TextFind.c TextFunc.c	\
	TextIn.c TextOut.c TextSel.c TextStrSo.c ToggleB.c ToggleBG.c	\
	TrackLoc.c Trait.c Transfer.c Transltns.c TravAct.c		\
	Traversal.c TraversalI.c UniqueEvnt.c VaSimple.c ValTime.c	\
	VendorS.c VendorSE.c VirtKeys.c WidGet.c Xm.c XmFontList.c	\
	XmIm.c XmRenderT.c XmStrDefs.c XmString.c XmStringFunc.c	\
	XmStringGet.c XmStringObso.c XmTabList.c Xmos.c

OBSO_SRC = Obso1_2.c Obso2_0.c ObsoStr.c ObsoXme.c

XPM_SRC = XpmAttrib.c XpmCrBufFrI.c XpmCrBufFrP.c XpmCrDatFrI.c		\
	XpmCrDatFrP.c XpmCrIFrBuf.c XpmCrIFrDat.c XpmCrIFrP.c		\
	XpmCrPFrBuf.c XpmCrPFrDat.c XpmCrPFrI.c XpmImage.c XpmInfo.c	\
	XpmRdFToBuf.c XpmRdFToDat.c XpmRdFToI.c XpmRdFToP.c		\
	XpmWrFFrBuf.c XpmWrFFrDat.c XpmWrFFrI.c XpmWrFFrP.c		\
	Xpmcreate.c Xpmdata.c Xpmhashtab.c Xpmmisc.c Xpmparse.c		\
	Xpmrgb.c Xpmscan.c

NEW_WID_SRC = ComboBox.c Container.c GrabShell.c IconG.c IconH.c	\
	Notebook.c SSpinB.c SpinB.c TranslNew.c TxtPropCv.c		\
	XmStringSeg.c

Xm 2.1 :LIBRARY: $(BASE_SRC) $(OBSO_SRC) $(XPM_SRC) $(NEW_WID_SRC)
