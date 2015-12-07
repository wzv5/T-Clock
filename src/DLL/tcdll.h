/*-------------------------------------------
  TCDLL.H - KAZUBON 1997-2001 // Modified by Stoic Joker: Thursday, 03/25/2010 @ 6:00:55pm
---------------------------------------------*/
#pragma once
#ifndef INC_TCDLL_H
#define INC_TCDLL_H
#include "../common/globals.h" // globals

#include <commctrl.h> // Required by most everything in TClock.c
#include <time.h>     // Required by time functions in Format.c
#include <math.h>     // Required by use of floor() in Format.c

#include "../common/resource.h"
#include "../common/newapi.h" // UxTheme stuff
#include "../common/utl.h" // utility functions
#include "../common/clock.h" // common clock api

extern HWND gs_hwndTClockMain; /**< our main window for hotkeys, menus and sounds \b [shared] */
extern HWND gs_hwndClock;      /**< the clock hwnd \b [shared] */
extern HWND gs_hwndCalendar;   /**< calendar state \b [shared] \sa TClockAPI::GetCalendar() */

#define TZNAME_MAX		  256//10

// tclock.c
void DrawClock(HDC hdc);
void GetDisplayTime(SYSTEMTIME* pt, int* beat100);
void FillClockBG();
void FillClockBGHover();

// font.c
HFONT CreateMyFont(const wchar_t* fontname, int fontsize, LONG weight, LONG italic, int angle, BYTE guality);

//#pragma once
//extern char szTZone[]; //---+++--> TimeZone String Buffer, Also Used (as External) in Format.c

// FORMAT.C
void InitFormat(const wchar_t* section, SYSTEMTIME* lt);
#define FORMAT_MAX_SIZE 1024
unsigned MakeFormat(wchar_t buf[FORMAT_MAX_SIZE], const wchar_t* fmt, SYSTEMTIME* pt, int beat100);
#define FORMAT_SECOND    0x0001
#define FORMAT_SYSINFO   0x0002
#define FORMAT_BEAT1     0x0004
#define FORMAT_BEAT2     0x0008
#define FORMAT_BATTERY   0x0010
#define FORMAT_MEMORY    0x0020
#define FORMAT_MOTHERBRD 0x0040
#define FORMAT_PERMON    0x0080
#define FORMAT_NET       0x0100
DWORD FindFormat(const wchar_t* fmt);

#endif // INC_TCDLL_H
