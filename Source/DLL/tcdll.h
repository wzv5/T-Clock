/*-------------------------------------------
  TCDLL.H - KAZUBON 1997-2001 // Modified by Stoic Joker: Thursday, 03/25/2010 @ 6:00:55pm
---------------------------------------------*/
#pragma once
#ifndef INC_TCDLL_H
#define INC_TCDLL_H
#include "../common/globals.h" // globals

#ifdef __GNUC__
#	define localtime_s _localtime64_s
#	define gmtime_s _gmtime64_s
#endif // __GNUC__

#define _CRT_SECURE_NO_DEPRECATE 1 // SHUT-UP About the New String Functions Already!!!
#include <windows.h>  // Required by the fact it runs on Windows.
#include <commctrl.h> // Required by most everything in TClock.c
#include <time.h>     // Required by time functions in Format.c
#include <math.h>     // Required by use of floor() in Format.c

#include "../common/resource.h" // common resource defines
#include "../common/newapi.h" // UxTheme stuff
#include "../common/utl.h" // utility functions

#define TZNAME_MAX		  256//10

#ifndef GWL_WNDPROC // Required for the x64 Edition
#define GWL_WNDPROC GWLP_WNDPROC
#endif

#define IDTIMER_DEKSTOPICONSTYLE 3

#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)
#define CLOCKM_BLINK          (WM_USER+3)
#define CLOCKM_COPY           (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP (WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)
/* defined in commctrl.h
#define TBCDRF_NOEDGES              0x00010000  // Don't draw button edges
#define TBCDRF_HILITEHOTTRACK       0x00020000  // Use color of the button bk when hottracked
#define TBCDRF_NOOFFSET             0x00040000  // Don't offset button if pressed
#define TBCDRF_NOMARK               0x00080000  // Don't draw default highlight of image/text for TBSTATE_MARKED
#define TBCDRF_NOETCHEDEFFECT       0x00100000  // Don't draw etched effect for disabled items
#define LPTBBUTTONINFO LPTBBUTTONINFOA
#define TBBUTTONINFO TBBUTTONINFOA
// */

#define DEFAULT_QUALITY				0
#define DRAFT_QUALITY				1
#define PROOF_QUALITY				2
#define NONANTIALIASED_QUALITY		3
#define ANTIALIASED_QUALITY			4
#define CLEARTYPE_QUALITY			5
#define CLEARTYPE_NATURAL_QUALITY	6

// tclock.c
extern char bNoClock;
void DrawClock(HWND hwnd, HDC hdc);
void GetDisplayTime(SYSTEMTIME* pt, int* beat100);
void FillClock(HWND hwnd, HDC hdc, RECT* prc, int nblink);

//#pragma once
//extern char szTZone[]; //---+++--> TimeZone String Buffer, Also Used (as External) in Format.c

// FORMAT.C
void InitFormat(SYSTEMTIME* lt);
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, const char* fmt);
#define FORMAT_SECOND    0x0001
#define FORMAT_SYSINFO   0x0002
#define FORMAT_BEAT1     0x0004
#define FORMAT_BEAT2     0x0008
#define FORMAT_BATTERY   0x0010
#define FORMAT_MEMORY    0x0020
#define FORMAT_MOTHERBRD 0x0040
#define FORMAT_PERMON    0x0080
#define FORMAT_NET       0x0100
DWORD FindFormat(const char* fmt);

#endif // INC_TCDLL_H
