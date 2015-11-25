/*	File:		SVAEGetDataSize.c	Contains:		Written by: Original version by Jon Lansdell and Nigel Humphreys.				3.1 updates by Greg Sutton.	Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "SVAEGetDataSize.h"#include "SVEditAEUtils.h"// -----------------------------------------------------------------------//		Name: 			DoGetDataSize//		Purpose:		Handles the GetDataSize AppleEvent.// -----------------------------------------------------------------------pascal OSErr	DoGetDataSize(const AppleEvent	*theAppleEvent,									AppleEvent	*reply,									long		handlerRefCon){ #pragma unused (handlerRefCon)	AEDesc    	directObj = {typeNull, NULL},				result = {typeNull, NULL};	Size      	actualSize;	DescType  	returnedType;	DescType  	reqType;	OSErr     	err;			// pick up the direct object, which is the object whose data is to be sized	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeWildCard, &directObj);				// now the get the type wanted - optional	reqType = typeChar;	// Default to char	(void)AEGetParamPtr(theAppleEvent, keyAERequestedType, typeType, &returnedType,												(Ptr)&reqType, sizeof(reqType), &actualSize);			err = GotRequiredParams(theAppleEvent);	if (noErr != err) goto done;		if (typeNull != directObj.descriptorType)		err = GetDataSizeDesc(&directObj, reqType, &result);	else		err = errAENoSuchObject;			err = AddResultToReply(&result, reply, err);done:		if (directObj.dataHandle) 		AEDisposeDesc(&directObj);	if (result.dataHandle) 		AEDisposeDesc(&result);				return(err);} // DoGetDataSizeOSErr	GetDataSizeTextToken(TextToken* theToken, AEDesc* result){	long	aSize;	OSErr	err;		aSize = theToken->tokenLength;		err = AECreateDesc(typeLongInteger, (Ptr)&aSize, sizeof(aSize), result);	return(err);}OSErr	GetDataSizeTextDesc(AEDesc* textDesc, AEDesc* result){	TextToken		aTextToken;	Size			actualSize;	OSErr			err;	if (typeMyText != textDesc->descriptorType)		return(errAETypeError);			GetRawDataFromDescriptor(textDesc, (Ptr)&aTextToken, sizeof(aTextToken), &actualSize);	err = GetDataSizeTextToken(&aTextToken, result);		return(err);}OSErr	GetDataSizeDesc(AEDesc* aDesc, DescType reqType, AEDesc* result){	AEDesc		dataSizeDesc = {typeNull, NULL},				textDesc = {typeNull, NULL},				itemDesc = {typeNull, NULL},				itemResult = {typeNull, NULL};	long		itemCount,				index;	DescType	theAEKeyword;	OSErr		err;		if (typeObjectSpecifier == aDesc->descriptorType)		err = AEResolve(aDesc, kAEIDoMinimum, &dataSizeDesc);	else		err = AEDuplicateDesc(aDesc, &dataSizeDesc);			if (noErr != err) goto done;		if (typeAEList == dataSizeDesc.descriptorType)	{		err = AECreateList(NULL, 0 , false, result);	// Result will also be a list of items		if (noErr != err) goto done;		err = AECountItems(&dataSizeDesc, &itemCount);		if (noErr != err) goto done;				for (index = 1; index <= itemCount; index++)		{			err = AEGetNthDesc(&dataSizeDesc, index, typeWildCard, &theAEKeyword, &itemDesc);			if (noErr != err) goto done;						err = GetDataSizeDesc(&itemDesc, reqType, &itemResult);	// Call recursively			if (noErr != err) goto done;						err = AEPutDesc(result, 0, &itemResult);	// Add to our result list			if (noErr != err) goto done;			if (itemDesc.dataHandle)				AEDisposeDesc(&itemDesc);			if (itemResult.dataHandle)				AEDisposeDesc(&itemResult);		}	}	else	{		switch (reqType)		{			case typeChar:				err = AECoerceDesc(&dataSizeDesc, typeMyText, &textDesc);				if (noErr != err) goto done;				err = GetDataSizeTextDesc(&textDesc, result);				break;							default:				err = errAECantSupplyType;		}	}	done:	if (dataSizeDesc.dataHandle)		AEDisposeDesc(&dataSizeDesc);	if (textDesc.dataHandle)		AEDisposeDesc(&textDesc);	if (itemDesc.dataHandle)		AEDisposeDesc(&itemDesc);	if (itemResult.dataHandle)		AEDisposeDesc(&itemResult);		return(err);}