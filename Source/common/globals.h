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

#include <Ws2tcpip.h> // getaddrinfo, Winsock2.h
#include <Wspiapi.h> // for Windows 2000 mainly
#include <Windows.h>  // Required by the fact it runs on Windows.

#ifdef __GNUC__
#	define localtime_s _localtime64_s
#	define gmtime_s _gmtime64_s
#else
#	define strdup _strdup
#	define wcsdup _wcsdup
#endif // __GNUC__

#define ARCH_SUFFIX_32 ""
#define ARCH_SUFFIX_64 "64"
#ifndef __x86_64__
#	define ARCH_SUFFIX ARCH_SUFFIX_32
#	define TCLOCK_SUFFIX ""
#else
#	define ARCH_SUFFIX ARCH_SUFFIX_64
#	define TCLOCK_SUFFIX " x64"
#endif // __x86_64__

#define ABT_TITLE "T-Clock Redux" TCLOCK_SUFFIX " - " VER_SHORT_DOTS " build " STR(VER_REVISION)
#define ABT_TCLOCK "T-Clock 2010 is Stoic Joker's rewrite of their code which allows it to run on Windows XP and up. While he removed some of T-Clock's previous functionality. He felt this makes it a more \"Administrator Friendly\" application as it no longer required elevated privileges to run.\n\nT-Clock Redux tries to continue Stoic Joker's efforts."
#define CONF_START "T-Clock Redux" TCLOCK_SUFFIX
#define CONF_START_OLD "Stoic Joker's T-Clock 2010" TCLOCK_SUFFIX

#define TC_TOOLTIP "\"T-Clock\"\\nLDATE"

#include "resource.h"

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
extern HWND		g_hwndTClockMain;	// Main Window Anchor for HotKeys Only!
extern HWND		g_hwndClock;		// Main Clock Window Handle
extern char		g_bCalOpen;

#define REG_MOUSE "Mouse"
enum{ // Drop&File enum / registry settings
	DF_NONE=0,
	DF_RECYCLE, // default
	DF_OPEN,
	DF_COPY,
	DF_MOVE,
};

// messages to send the main/helper app
#define MAINMFIRST					MAINM_CLOCKINIT
#define MAINM_CLOCKINIT				(WM_USER)
#define MAINM_ERROR					(WM_USER+1)
#define MAINM_EXIT					(WM_USER+2)
#define MAINM_STOPSOUND				(WM_USER+3)
#define MAINM_BLINKOFF				(WM_USER+4)
#define MAINMLAST					MAINM_BLINKOFF
// messages to send the clock
#define CLOCKM_REFRESHCLOCK					(WM_USER+1)
#define CLOCKM_REFRESHTASKBAR				(WM_USER+2)
#define CLOCKM_BLINK						(WM_USER+3)
#define CLOCKM_COPY							(WM_USER+4)
#define CLOCKM_REFRESHDESKTOP				(WM_USER+5)
//#define CLOCKM_REFRESHCLEARTASKBAR			(WM_USER+6)
#define CLOCKM_REFRESHCLOCKPREVIEW			(WM_USER+7)
#define CLOCKM_REFRESHCLOCKPREVIEWFORMAT	(WM_USER+8)
#define CLOCKM_BLINKOFF						(WM_USER+9)

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

// own defines to suppress warning C4306 in Windows' API
#define LPSTR_TEXTCALLBACK_nowarn ((LPSTR)(intptr_t)-1L)
#define HWND_BROADCAST_nowarn ((HWND)(intptr_t)0xffff)
#define HWND_TOPMOST_nowarn ((HWND)(intptr_t)-1)
#define HWND_NOTOPMOST_nowarn ((HWND)(intptr_t)-2)
#define HWND_MESSAGE_nowarn ((HWND)(intptr_t)-3)

// suppress warning C4127 about const expression in do{}while(0);
#define FD_SET_nowarn __pragma(warning(suppress:4127)) FD_SET(sock, &fds);

// socket defines currently missing in Windows
#define s6_addr16 u.Word
#define EAI_AGAIN		WSATRY_AGAIN			/**< A temporary failure in name resolution occurred */
#define EAI_BADFLAGS	WSAEINVAL				/**< An invalid value was provided for the ai_flags member of the pHints parameter */
#define EAI_FAIL		WSANO_RECOVERY			/**< A nonrecoverable failure in name resolution occurred */
#define EAI_FAMILY		WSAEAFNOSUPPORT			/**< The ai_family member of the pHints parameter is not supported */
#define EAI_MEMORY		WSA_NOT_ENOUGH_MEMORY	/**< A memory allocation failure occurred */
#define EAI_NONAME		WSAHOST_NOT_FOUND		/**< The name does not resolve for the supplied parameters or the pNodeName and pServiceName parameters were not provided */
#define EAI_SERVICE		WSATYPE_NOT_FOUND		/**< The pServiceName parameter is not supported for the specified ai_socktype member of the pHints parameter */
#define EAI_SOCKTYPE	WSAESOCKTNOSUPPORT		/**< The ai_socktype member of the pHints parameter is not supported */

#endif // TCLOCK_GLOBAL_H
