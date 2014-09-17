#ifndef TCLOCK_GLOBAL_H
#define TCLOCK_GLOBAL_H

#define _CRT_SECURE_NO_DEPRECATE 1 // SHUT-UP About the New String Functions Already!!!

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include <Windows.h>  // Required by the fact it runs on Windows.

#ifdef __GNUC__
#	define localtime_s _localtime64_s
#	define gmtime_s _gmtime64_s
#else
#	define strdup _strdup
#endif // __GNUC__

#include "resource.h"

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
extern HWND		g_hwndTClockMain;	// Main Window Anchor for HotKeys Only!
extern HWND		g_hwndClock;		// Main Clock Window Handle
extern HHOOK	g_hhook;

#ifndef GWL_WNDPROC // Required for the x64 Edition
#	define GWL_WNDPROC GWLP_WNDPROC
#endif

// messages to send the main/helper app
#define MAINM_CLOCKINIT				(WM_USER)
#define MAINM_ERROR					(WM_USER+1)
#define MAINM_EXIT					(WM_USER+2)
#define MAINM_STOPSOUND				(WM_USER+3)
#define MAINM_BLINKOFF				(WM_USER+4)
// messages to send the clock
#define CLOCKM_REFRESHCLOCK			(WM_USER+1)
#define CLOCKM_REFRESHTASKBAR		(WM_USER+2)
#define CLOCKM_BLINK				(WM_USER+3)
#define CLOCKM_COPY					(WM_USER+4)
#define CLOCKM_REFRESHDESKTOP		(WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)
#define CLOCKM_REFRESHCLOCKPREVIEW	(WM_USER+7)
#define CLOCKM_REFRESHCLOCKSUBS		(WM_USER+8)

// Global Buffer Size Labels
#define TNY_BUFF	32
#define MIN_BUFF	64
#define GEN_BUFF	128
#define LRG_BUFF	256
#define MAX_BUFF	1024

// font quality / smoothing
#define DEFAULT_QUALITY				0
#define DRAFT_QUALITY				1
#define PROOF_QUALITY				2
#define NONANTIALIASED_QUALITY		3
#define ANTIALIASED_QUALITY			4
#define CLEARTYPE_QUALITY			5
#define CLEARTYPE_NATURAL_QUALITY	6

#endif // TCLOCK_GLOBAL_H
