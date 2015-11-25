/*	File:		SVAECopy.c	Contains:		Written by: Original version by Jon Lansdell and Nigel Humphreys.				3.1 updates by Greg Sutton.	Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "SVAECopy.h"#include "SVEditAEUtils.h"#include "SVEditWindow.h"		// for DPtrFromWindowPtr()#include "SVAESelect.h"#include <Scrap.h>#pragma segment AppleEvent// Handle a copy to scrap e.g 'copy last word of document 1'// Note that 'copy last word of document 1 to end of document 2' is a kAEClone event	 pascal OSErr	DoCopy(const AppleEvent *theAppleEvent, AppleEvent *reply, long refcon){#pragma unused (reply, refcon)	AEDesc		directObj = {typeNull, NULL};	TextToken	aTextToken;	short		ignore;	OSErr		err;	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeWildCard, &directObj);	// If we get an error here it just means that they haven't supplied a reference to	// an object to copy - so copy the current section instead.		if (directObj.descriptorType != typeNull)		err = CopyDesc(&directObj);	else	{			// Just copy the selection of the front window		err = GetWindowSelection(FrontWindow(), &aTextToken, &ignore);		if (noErr != err) goto done;				err = CopyTextToken(&aTextToken);	}done:		if (directObj.dataHandle)		AEDisposeDesc(&directObj);			return(err);} // DoCopyOSErr	CopyTextToken(TextToken* theToken){	WindowPtr		aWindow;	DPtr			docPtr;	OSErr			err;		aWindow = theToken->tokenWindow;	docPtr = DPtrFromWindowPtr(theToken->tokenWindow);		if (! aWindow || ! docPtr)		return(errAENoSuchObject);					// Set this tokens selection	err = SelectTextToken(theToken);	if (noErr != err) goto done;	err = (OSErr)ZeroScrap();	TECopy(docPtr->theText); 		done:	return(err);}OSErr	CopyTextDesc(AEDesc* textDesc){	TextToken		aTextToken;	Size			actualSize;	OSErr			err;	if (typeMyText != textDesc->descriptorType)		return(errAETypeError);			GetRawDataFromDescriptor(textDesc, (Ptr)&aTextToken, sizeof(aTextToken), &actualSize);	err = CopyTextToken(&aTextToken);		return(err);}OSErr	CopyDesc(AEDesc* aDesc){	AEDesc		copyDesc = {typeNull, NULL},				textDesc = {typeNull, NULL};	OSErr		err;		if (typeObjectSpecifier == aDesc->descriptorType)		err = AEResolve(aDesc, kAEIDoMinimum, &copyDesc);	else		err = AEDuplicateDesc(aDesc, &copyDesc);			if (noErr != err) goto done;		switch (copyDesc.descriptorType)	{		case typeAEList:			err = errAETypeError;			// We can't handle copying more than one item to the scrap			break;					default:			err = AECoerceDesc(&copyDesc, typeMyText, &textDesc);			if (noErr != err) goto done;			err = CopyTextDesc(&textDesc);	}	done:	if (copyDesc.dataHandle)		AEDisposeDesc(&copyDesc);	if (textDesc.dataHandle)		AEDisposeDesc(&textDesc);		return(err);}