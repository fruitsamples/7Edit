/*	File:		Offscreen.c	Contains:		Written by: Original version by Jon Lansdell and Nigel Humphreys.				3.1 updates by Greg Sutton.		Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "Offscreen.h"#ifndef __QUICKDRAW__	#include <Quickdraw.h>#endif#ifndef __QDOFFSCREEN__	#include <QDOffscreen.h>#endif#ifndef __MEMORY__	#include <Memory.h>#endiftWindowOffscreen* DrawOffscreen ( WindowRef theWindow ){	tWindowOffscreen*	theOffscreen;	GWorldPtr			theWorld;	Rect				globalRect;			theOffscreen = (tWindowOffscreen*) NewPtr ( sizeof ( tWindowOffscreen ) );	if ( theOffscreen == 0L )		return 0L;		SetPort ( theWindow );	GetGWorld ( &theOffscreen->windowPort, &theOffscreen->windowDevice );		globalRect = theWindow->portRect;	LocalToGlobal ( (Point*) &globalRect.top );	LocalToGlobal ( (Point*) &globalRect.bottom );		if ( NewGWorld ( &theWorld, 0, &globalRect, 0L, 0L, 0) == noErr )	{		SetGWorld ( theWorld, 0L );		if ( !LockPixels ( theWorld->portPixMap ) )		{			DisposeOffscreen ( theOffscreen );			return 0L;		}				CopyBits ( &theWindow->portBits, &((GrafPtr) theWorld)->portBits,				 	&theWindow->portRect, &theWorld->portRect, srcCopy, 0L );				theOffscreen->offscreenWorld = theWorld;		return theOffscreen;	}		DisposePtr ( (Ptr) theOffscreen );	return 0L;}tWindowOffscreen* DrawOnscreen ( tWindowOffscreen* theOffscreen ){	if ( theOffscreen )	{		SetGWorld ( theOffscreen->windowPort, theOffscreen->windowDevice );		CopyBits ( &((GrafPtr) theOffscreen->offscreenWorld)->portBits,				 		(BitMap*) &(theOffscreen->windowPort)->portPixMap,				 		&theOffscreen->offscreenWorld->portRect,				 		&theOffscreen->windowPort->portRect,						srcCopy, 0L );		UnlockPixels ( theOffscreen->offscreenWorld->portPixMap );		DisposeOffscreen ( theOffscreen );	}		return nil;}tWindowOffscreen* DisposeOffscreen ( tWindowOffscreen* theOffscreen ){		DisposeGWorld ( theOffscreen->offscreenWorld );	DisposePtr ( (Ptr) theOffscreen );		return nil;}