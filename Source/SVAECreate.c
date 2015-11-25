/*	File:		SVAECreate.c	Contains:		Written by: Original version by Jon Lansdell and Nigel Humphreys.				3.1 updates by Greg Sutton.	Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "SVAECreate.h"#include "SVEditGlobals.h"#include "SVEditWindow.h"#include "SVEditAEUtils.h"#include "SVAETextUtils.h"#include "SVAppleEvents.h"#include "SVAEAccessors.h"#include "SVAERecording.h"#include "SVAESelect.h"#include "SVAEWindowUtils.h"#include "SVAESetData.h"#pragma segment AppleEvent// ------------------------------------------------------------------------//	Name: 		DoNewElement//	Purpose:	Handles the NewElement AppleEvent. Creates windows and//				text.// ------------------------------------------------------------------------pascal OSErr	DoNewElement(const AppleEvent	*theAppleEvent,									AppleEvent	*reply, 									long		handlerRefCon){#pragma unused (handlerRefCon)	DescType	returnedType,				newElemClass;	Size        actualSize;	AEDesc    	dataDesc = {typeNull, NULL},				insertHereDesc = {typeNull, NULL},				propertyDesc = {typeNull, NULL},				insertData = {typeNull, NULL},				resultDesc = {typeNull, NULL};	OSErr       ignoreErr,				err;	    	err = AEGetParamPtr(theAppleEvent, keyAEObjectClass, typeType,							&returnedType, (Ptr)&newElemClass, sizeof(newElemClass), &actualSize);	if (noErr != err) goto done;		// We have to know what object to create			// Get optional parameters	ignoreErr = AEGetParamDesc(theAppleEvent, keyAEData, typeWildCard, &dataDesc);	ignoreErr = AEGetParamDesc(theAppleEvent, keyAEInsertHere, typeWildCard, &insertHereDesc);	ignoreErr = AEGetParamDesc(theAppleEvent, keyAEPropData, typeWildCard, &propertyDesc);	  	  		// check for missing required parameters	err = GotRequiredParams(theAppleEvent);	if (noErr != err) goto done; 	switch (newElemClass)	{		case cWindow:		case cDocument:			err = CreateWindow(&dataDesc, &insertHereDesc, &propertyDesc, &resultDesc);			break;					case cChar:		case cText:		case cWord:		case cParagraph:			err = CreateText(newElemClass, &dataDesc, &insertHereDesc, &propertyDesc, &resultDesc);			break;					default:			err = errAEWrongDataType;	}		err = AddResultToReply(&resultDesc, reply, err);done:				if (dataDesc.dataHandle) 		AEDisposeDesc(&dataDesc);	if (insertHereDesc.dataHandle) 		AEDisposeDesc(&insertHereDesc);	if (propertyDesc.dataHandle) 		AEDisposeDesc(&propertyDesc);	if (resultDesc.dataHandle) 		AEDisposeDesc(&resultDesc);			return(err);} // DoNewElementOSErr	CreateWindow(AEDesc* dataDesc, AEDesc* insertHereDesc,												AEDesc* propertyDesc, AEDesc* result){	AEDesc		insertDesc = {typeNull, NULL};	DescType 	insertType;	DPtr		docPtr;	WindowPtr	behindWindow;	OSErr		err; 	err = GetInsertDescFromInsertHere(insertHereDesc, &insertDesc, &insertType);	if (noErr != err) goto done;		err = GetBehindWindow(&insertDesc, insertType, &behindWindow);	if (noErr != err) goto done;		docPtr = NewDocument(false, behindWindow);	if (! docPtr)	{		err = errAENoSuchObject;		goto done;	}	if (propertyDesc->dataHandle)		err = SetWindowPropertyRecord(docPtr->theWindow, propertyDesc);	if (noErr != err) goto done;	if (dataDesc->dataHandle)	{		err = SetWindowData(docPtr->theWindow, dataDesc);		docPtr->dirty = true;	}	else		docPtr->dirty = false;	if (noErr != err) goto done;	ShowWindow(docPtr->theWindow);	err = MakeWindowObj(docPtr->theWindow, result);done:	if (insertDesc.dataHandle)		AEDisposeDesc(&insertDesc);	return(err);}OSErr	GetBehindWindow(AEDesc* insertDesc, DescType insertType, WindowPtr* behindWindow){	AEDesc			windowDesc = {typeNull, NULL};	WindowToken		aWindowToken;	Size			actualSize;	short			index;	OSErr			err;	if (typeNull == insertDesc->descriptorType)	{		*behindWindow = (WindowPtr) -1L;		return(noErr);	}		err = AECoerceDesc(insertDesc, typeMyWndw, &windowDesc);	if (noErr != err) goto done;	GetRawDataFromDescriptor(&windowDesc, (Ptr)&aWindowToken,									sizeof(aWindowToken), &actualSize);	switch (insertType)	{		case kAEBeginning:			*behindWindow = (WindowPtr) -1L;			break;			case kAEEnd:			*behindWindow = NULL;			break;				case kAEBefore:			index = GetNthWindowOfWindowPtr(aWindowToken.tokenWindow);			if (index > 1)				*behindWindow = GetWindowPtrOfNthWindow(index - 1);			else				*behindWindow = (WindowPtr) -1L;	// Stick at front because no			break;									// windows before that.				case kAEAfter:			*behindWindow = aWindowToken.tokenWindow;			break;				case kAEReplace:		default:			err = errAEEventFailed;		// We won't allow a new window to replace an existing one	}	done:	if (windowDesc.dataHandle)		(void)AEDisposeDesc(&windowDesc);	return(err);}OSErr	SetWindowPropertyRecord(WindowPtr theWindow, AEDesc* propertyRecord){	WindowPropToken		aWindowPropToken;	AEDesc				dataDesc = {typeNull, NULL},						propertyDesc = {typeNull, NULL};	AEKeyword			theAEKeyword;	long				index;	OSErr				err;		aWindowPropToken.tokenWindowToken.tokenWindow = theWindow;										err = AECountItems(propertyRecord, &index);	if (noErr != err) goto done;				// Step through each property - creating a window property token AEDesc			// and letting SetWindowProperty() do the work.	for (; index > 0; index--)	{		err = AEGetNthDesc(propertyRecord, index, typeWildCard, &theAEKeyword, &dataDesc);		if (noErr != err) goto done;		aWindowPropToken.tokenProperty = theAEKeyword;		err = AECreateDesc(typeMyWindowProp, (Ptr)&aWindowPropToken, 										sizeof(aWindowPropToken), &propertyDesc);		if (noErr != err) goto done;				err = DoSetWindowProperty(&propertyDesc, &dataDesc);		if (noErr != err) goto done;				if (dataDesc.dataHandle)			AEDisposeDesc(&dataDesc);		if (propertyDesc.dataHandle)			AEDisposeDesc(&propertyDesc);	}	done:	if (dataDesc.dataHandle)		AEDisposeDesc(&dataDesc);	if (propertyDesc.dataHandle)		AEDisposeDesc(&propertyDesc);		return(err);}// We'll just assume it's text and put it through as the selection.OSErr	SetWindowData(WindowPtr theWindow, AEDesc* dataDesc){	WindowPropToken		aWindowPropToken;	AEDesc				propertyDesc = {typeNull, NULL};	OSErr				err;		aWindowPropToken.tokenWindowToken.tokenWindow = theWindow;	aWindowPropToken.tokenProperty = pSelection;										err = AECreateDesc(typeMyWindowProp, (Ptr)&aWindowPropToken, 									sizeof(aWindowPropToken), &propertyDesc);	if (noErr != err) goto done;			err = DoSetWindowProperty(&propertyDesc, dataDesc);done:	if (propertyDesc.dataHandle)		AEDisposeDesc(&propertyDesc);	return(err);}OSErr	CreateText(DescType textType, AEDesc* dataDesc, AEDesc* insertHereDesc,												AEDesc* propertyDesc, AEDesc* result){	AEDesc		insertDesc = {typeNull, NULL};	DescType 	insertType;	TextToken	anInsertToken;	short		ignore;	OSErr		err = noErr; 	err = GetInsertDescFromInsertHere(insertHereDesc, &insertDesc, &insertType);	if (noErr != err) goto done;		if (typeNull == insertType)		// Default to setting the selection in the front window		err = GetWindowSelection(FrontWindow(), &anInsertToken, &ignore);	else							// Otherwise get a selection from the insertDesc		err = GetInsertToken(&insertDesc, insertType, &anInsertToken);			if (noErr != err) goto done;		err = CreateAtTextToken(textType, dataDesc, &anInsertToken, propertyDesc, result);done:	if (insertDesc.dataHandle)		AEDisposeDesc(&insertDesc);	return(err);}// Get a TextToken for the location of where to insert the text. If the insertType is// a relative position then work this out. Otherwise just use the insertDesc.OSErr	GetInsertToken(AEDesc* insertDesc, DescType insertType, TextToken* resultToken){	AEDesc		textDesc = {typeNull, NULL};	TextToken	aTextToken;	Size		actualSize;	OSErr		err;	err = AECoerceDesc(insertDesc, typeMyText, &textDesc);	if (noErr != err) goto done;	GetRawDataFromDescriptor(&textDesc, (Ptr)&aTextToken,									sizeof(aTextToken), &actualSize);	resultToken->tokenWindow = aTextToken.tokenWindow;	switch (insertType)	{		case kAEBeginning:		case kAEBefore:			resultToken->tokenOffset = aTextToken.tokenOffset;			resultToken->tokenLength = 0;			break;			case kAEEnd:		case kAEAfter:			resultToken->tokenOffset = aTextToken.tokenOffset + aTextToken.tokenLength;			resultToken->tokenLength = 0;			break;				case kAEReplace:		default:			// default is probably some text token to replace							// e.g make new word at middle word of document 1 with data "Iris"							// It has been coerced to text so it's okay			resultToken->tokenOffset = aTextToken.tokenOffset;			resultToken->tokenLength = aTextToken.tokenLength;			break;	}done:	if (textDesc.dataHandle)		(void)AEDisposeDesc(&textDesc);		return(err);}// Create text using the dataDesc (which could be text, styled text, etc�) at the// the location held in the TextToken.// Properties in the propertyDesc will be applied to the new text.// Returns an object specifier to the text.OSErr	   CreateAtTextToken(DescType textType, const AEDesc* dataDesc, TextToken* theToken,													AEDesc* propertyDesc, AEDesc* result){#pragma unused(textType)	DPtr		docPtr;	TextToken	aSelectionToken;	short		oldLength;	OSErr		err;	docPtr = DPtrFromWindowPtr(theToken->tokenWindow);		if (! docPtr)		return(errAENoSuchObject);			// Copy the current selection - so we can restore it after	err = GetWindowSelection(theToken->tokenWindow, &aSelectionToken, &oldLength);	if (noErr != err) goto done;			// Set the selection we want to insert the new text into	err = SelectTextToken(theToken);	if (noErr != err) goto done;	err = PutStyledTextFromDescIntoTEHandle(dataDesc, docPtr->theText);	if (noErr != err) goto done;				// Update the selection and get the length of the insertion	err = UpdateSelectionToken(theToken, &aSelectionToken, oldLength,														&theToken->tokenLength);	if (noErr != err) goto done;		// Need to check on token type in here and make what user wanted	// e.g. make sure a word is a word.	// Would have to remember to balance token for any chages to TEHandle.	if (propertyDesc->dataHandle)	{									// I doubt we'll handle setting the text property										// or anything that changes the token length� oh well		err = SetTextPropertyRecord(theToken, propertyDesc);		if (noErr != err) goto done;	}			// Make the returned object	err = MakeTextObjFromToken(theToken, result);	done:	return(err);}// Take a TextToken and apply the properties in the propertyRecord descriptor// to the text.// e.g. make new word at beginning of document 1 with data "Bert" �//								with properties {size:32, font:"Courier"}// The with properties part is the property record.OSErr	SetTextPropertyRecord(TextToken* aTextToken, AEDesc* propertyRecord){	TextPropToken		aTextPropToken;	AEDesc				dataDesc = {typeNull, NULL},						propertyDesc = {typeNull, NULL};	AEKeyword			theAEKeyword;	long				index;	OSErr				err;		aTextPropToken.tokenTextToken = *aTextToken;										err = AECountItems(propertyRecord, &index);	if (noErr != err) goto done;				// Step through each property - creating a window property token AEDesc			// and letting SetWindowProperty() do the work.	for (; index > 0; index--)	{		err = AEGetNthDesc(propertyRecord, index, typeWildCard, &theAEKeyword, &dataDesc);		if (noErr != err) goto done;		aTextPropToken.tokenProperty = theAEKeyword;		err = AECreateDesc(typeMyTextProp, (Ptr)&aTextPropToken, 										sizeof(aTextPropToken), &propertyDesc);		if (noErr != err) goto done;				err = SetTextProperty(&propertyDesc, &dataDesc);		if (noErr != err) goto done;				if (dataDesc.dataHandle)			AEDisposeDesc(&dataDesc);		if (propertyDesc.dataHandle)			AEDisposeDesc(&propertyDesc);	}	done:	if (dataDesc.dataHandle)		AEDisposeDesc(&dataDesc);	if (propertyDesc.dataHandle)		AEDisposeDesc(&propertyDesc);		return(err);}