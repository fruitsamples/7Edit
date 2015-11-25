/*	File:		Offscreen.h	Contains:		Written by: Original version by Jon Lansdell and Nigel Humphreys.				3.1 updates by Greg Sutton.		Copyright:	Copyright � 1995-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				7/20/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#pragma once#ifndef __QUICKDRAW__	#include <Quickdraw.h>#endif#ifndef __QDOFFSCREEN__	#include <QDOffscreen.h>#endiftypedef struct WindowOffscreen{	CGrafPtr		windowPort;	GDHandle		windowDevice;	GWorldPtr		offscreenWorld;} tWindowOffscreen;tWindowOffscreen* DrawOffscreen ( WindowPtr theWindow );tWindowOffscreen* DrawOnscreen ( tWindowOffscreen* theOffscreen );tWindowOffscreen* DisposeOffscreen ( tWindowOffscreen* theOffscreen );