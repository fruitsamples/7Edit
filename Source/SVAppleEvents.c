/*	File:		SVAppleEvents.c	Contains:		Written by: 	Original version by Jon Lansdell and Nigel Humphreys.								3.1 updates by Greg Sutton.	Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.							You may incorporate this Apple sample source code into your program(s) without							restriction. This Apple sample source code has been provided "AS IS" and the							responsibility for its operation is yours. You are not permitted to redistribute							this Apple sample source code as "Apple sample source code" after having made							changes. If you're going to re-distribute the source, we require that you make							it clear in the source that the code was descended from Apple sample source							code, but that you've made changes.	Change History (most recent first):							7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "SVAppleEvents.h"#include <Resources.h>#include "SVEditGlobals.h"#include "SVEditUtils.h"#include "SVEditAEUtils.h"#include "SVEditWindow.h"#include "SVEditFile.h"#include "SVAERecording.h"#include "SVAECompare.h"#include "SVAECountElements.h"#include "SVAEAccessors.h"#include "SVAECoercions.h"#include "SVAEObjectsExist.h"#include "SVAECreate.h"#include "SVAEClone.h"#include "SVAEMove.h"#include "SVAEDelete.h"#include "SVAECopy.h"#include "SVAECut.h"#include "SVAEPaste.h"#include "SVAESelect.h"#include "SVAEClose.h"#include "SVAESave.h"#include "SVAERevert.h"#include "SVAEGetData.h"#include "SVAESetData.h"#include "SVAEGetDataSize.h"short		gRefNum;#pragma segment Main/*-----------------------------------------------------------------------*//**----------						 APPLE EVENT HANDLING 					---------------**//*-----------------------------------------------------------------------*/// -----------------------------------------------------------------------//		Name: 			DoAppleEvent//		Purpose:		Process and despatch the AppleEvent// -----------------------------------------------------------------------void DoAppleEvent(EventRecord theEvent){	OSErr err;  // should check for your own event message types here - if you have any		err = AEProcessAppleEvent(&theEvent);}// -----------------------------------------------------------------------//		Name: 			DoOpenApp//		Purpose:		Called on startup, creates a new document.// -----------------------------------------------------------------------pascal OSErr DoOpenApp(const AppleEvent *message,const AppleEvent *reply,long refcon)  {#pragma unused (reply,refcon, message)		  DPtr ourDoc;			/*just create a new document*/		ourDoc = NewDocument(false, (WindowPtr)-1L);				if (ourDoc)			{				ShowWindow(ourDoc->theWindow);				return(noErr);			}		else			return(-108);	}// -----------------------------------------------------------------------//		Name: 			DoOpenDocument//		Purpose:		Open all the documents passed in the Open AppleEvent.// -----------------------------------------------------------------------pascal OSErr DoOpenDocument(const AppleEvent *message, const AppleEvent *reply, long refcon)			  {#pragma unused (reply, refcon)		long        index;		long        itemsInList;		AEKeyword   keywd;		OSErr       err;		OSErr       ignoreErr;		AEDescList  docList;		long        actSize;		DescType    typeCode;		FSSpec      theFSSpec;				/*open the specified documents*/				docList.dataHandle = nil;				err = AEGetParamDesc(message, keyDirectObject, typeAEList, &docList);				if (err==noErr)			err = AECountItems( &docList, &itemsInList) ;		else		  itemsInList = 0;					for (index = 1; index <= itemsInList; index++)			if (err==noErr)				{					err = AEGetNthPtr( &docList, index, typeFSS, &keywd, &typeCode,														 (Ptr)&theFSSpec, sizeof(theFSSpec), &actSize ) ;					if (err==noErr)					  err = OpenOld(theFSSpec);				}		  if (docList.dataHandle)			ignoreErr = AEDisposeDesc(&docList);					return(err);	}// -----------------------------------------------------------------------//		Name: 			MyQuit//		Purpose:		Quit event received- exit the program.// -----------------------------------------------------------------------pascal OSErr MyQuit(const AppleEvent *message,const AppleEvent *reply,long refcon)				{#pragma unused (reply,refcon)			DescType saveOpt;		OSErr    tempErr;		OSErr    myErr;		DescType returnedType;		long     actSize;				saveOpt = kAEAsk; /* the default */		tempErr = AEGetParamPtr(message,														keyAESaveOptions,														typeEnumerated,														&returnedType,														(Ptr)&saveOpt,														sizeof(saveOpt),														&actSize);				if (saveOpt != kAENo)			myErr = AEInteractWithUser(kAEDefaultTimeout, nil, nil);	  if (myErr == noErr)			DoQuit(saveOpt);				return(myErr);	}// -----------------------------------------------------------------------//	Name: 			DoPrintDocuments//	Purpose:		Print a list of documents (or windows).// -----------------------------------------------------------------------pascal OSErr DoPrintDocuments(const AppleEvent *message, AppleEvent *reply, long refcon){#pragma unused (reply, refcon)	long          index;	long          itemsInList;	AEKeyword     keywd;	OSErr         err;	AEDescList    docList;	Size          actSize;	DescType      typeCode;	FSSpec        theFSSpec;	WindowToken   theWindowToken;	OSErr         forgetErr;	Boolean       talkToUser;			err = AEGetParamDesc(message,											 keyDirectObject,											 typeAEList,											 &docList);											 	err = AECountItems(&docList, &itemsInList);			for (index = 1; index<=itemsInList; index++)		if (err == noErr) 			{				forgetErr = AEGetNthPtr( &docList, index, typeFSS, &keywd,																 &typeCode, (Ptr)&theFSSpec, sizeof(theFSSpec), &actSize);																					talkToUser = false;		// (AEInteractWithUser(kAEDefaultTimeout, nil, nil) == noErr);					if (forgetErr == noErr) 					{						if (err == noErr) 							err = IssueAEOpenDoc(theFSSpec);													if (err == noErr) 							IssuePrintWindow(FrontWindow(), talkToUser);													if (err == noErr) 							IssueCloseCommand(FrontWindow());					}				else					{ /* wasn't a file - was it a window ? */						err = AEGetNthPtr(&docList,						                  index,						                  typeMyWndw,						                  &keywd,															&typeCode,															(Ptr)&theWindowToken,															sizeof(WindowToken),															&actSize);																																													if (err == noErr)								PrintWindow(DPtrFromWindowPtr(theWindowToken.tokenWindow), talkToUser);					}			}		if (docList.dataHandle)		forgetErr = AEDisposeDesc(&docList);			return(err);} /* DoPrintDocuments */// -----------------------------------------------------------------------//	Name: 			InitAppleEvents//	Purpose:		Initialise the AppleEvent despatch table// -----------------------------------------------------------------------void	InitAppleEvents(void){	OSErr aevtErr;		gRefNum = CurResFile();		// Needed for getting application property			// set up the dispatch table for the four standard AppleEvents		aevtErr = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(DoOpenApp), noRefCon, false) ;	aevtErr = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,   NewAEEventHandlerProc(DoOpenDocument), noRefCon, false) ;	aevtErr = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,  NewAEEventHandlerProc(DoPrintDocuments), noRefCon, false) ;	aevtErr = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(MyQuit), noRefCon, false) ;			// set up the dispatch table for the core AppleEvents	aevtErr = AEInstallEventHandler( kAECoreSuite, kAESetData,				 NewAEEventHandlerProc(DoSetData),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEGetData,         NewAEEventHandlerProc(DoGetData),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEGetDataSize,     NewAEEventHandlerProc(DoGetDataSize),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAECountElements,   NewAEEventHandlerProc(DoCountElements),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEDoObjectsExist,  NewAEEventHandlerProc(DoObjectsExist),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAECreateElement,   NewAEEventHandlerProc(DoNewElement),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEClone,           NewAEEventHandlerProc(DoClone),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEDelete,          NewAEEventHandlerProc(DoDelete),noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEMove,  					 NewAEEventHandlerProc(DoMove),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAEClose,           NewAEEventHandlerProc(DoCloseWindow),noRefCon, false);	aevtErr = AEInstallEventHandler( kAECoreSuite, kAESave,            NewAEEventHandlerProc(DoSaveWindow),noRefCon, false);		// set up the dispatch table for the miscellaneous AppleEvents		aevtErr = AEInstallEventHandler( kAEMiscStandards, kAECut,    NewAEEventHandlerProc(DoCut),   noRefCon, false);	aevtErr = AEInstallEventHandler( kAEMiscStandards, kAECopy,   NewAEEventHandlerProc(DoCopy),  noRefCon, false);	aevtErr = AEInstallEventHandler( kAEMiscStandards, kAEPaste,  NewAEEventHandlerProc(DoPaste), noRefCon, false);	aevtErr = AEInstallEventHandler( kAEMiscStandards, kAERevert, NewAEEventHandlerProc(DoRevert),noRefCon, false);	aevtErr = AEInstallEventHandler( kAEMiscStandards, kAESelect, NewAEEventHandlerProc(DoSelect),noRefCon, false);		// Install recording handlers	aevtErr = InstallRecordingHandlers();			// Install callbacks for count and compare procedures	aevtErr = InstallObjectCallbacks();			// Now install our object accessors	aevtErr = InstallAccessors();		// Now the coercion handlers	aevtErr = InstallCoercions();} // InitAppleEvents