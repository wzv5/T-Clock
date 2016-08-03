#ifndef TCLOCK_NEWAPI_H
#define TCLOCK_NEWAPI_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief unloads all new API functions and removes \c WS_EX_LAYERED from the taskbar of our clock \a hwndClock
 * \param hwndClock HWND to our clock (can be \c NULL)
 * \sa SetLayeredTaskbar() */
void EndNewAPI(HWND hwndClock);
/**
 * \brief checks if we're running under WoW64 (32bit app on 64bit OS)
 * \return boolean */
int IsWow64();
/**
 * \brief sets the taskbar \c WS_EX_LAYERED to enable transparency (when user chose to)
 * \param hwndClock HWND to our clock
 * \param refresh forces a full taskbar refresh if set
 * \sa EndNewAPI() */
void SetLayeredTaskbar(HWND hwndClock, int alpha, int clear_taskbar, int refresh);
//void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans);

// DrawTheme
#define VSCLASS_CLOCK L"Clock"
#define VSCLASS_TASKBAND2 L"TaskBand2" /**< Win10+ */

#ifndef CLP_TIME
#	define CLP_TIME 1
#	define CLS_NORMAL 1
#	define CLS_HOT 2
#	define CLS_PRESSED 3
//#	define TMT_COLOR 204
#	define TMT_BACKGROUND 1602
#	define TMT_WINDOWTEXT 1609
#	define TMT_CAPTIONTEXT 1610
#	define TMT_BTNTEXT 1619
#	define TMT_INFOTEXT 1624
#	define TMT_TEXTCOLOR 3803

#	define TMT_TRANSPARENTCOLOR 3809
#	define TMT_WINDOW 1606
#	define TMT_WINDOWFRAME 1607
#	define TMT_FILLCOLOR 3802
#endif

/**
 * \brief get current theme color for our clock text
 * \param hwndClock handle to our clock
 * \param state one of \c CLS_NORMAL, \c CLS_HOT or \c CLS_PRESSED
 * \return COLORREF with our text color
 * \sa GetXPClockColorBG(), ReloadXPClockTheme(), CLS_NORMAL, CLS_HOT, CLS_PRESSED */
COLORREF GetXPClockColor(HWND hwndClock, int state);
/**
 * \brief get current theme color for our clock background
 * \param hwndClock handle to our clock
 * \param state one of \c CLS_NORMAL, \c CLS_HOT or \c CLS_PRESSED
 * \return COLORREF with our background color
 * \remarks might not work properly on XP
 * \sa GetXPClockColor(), ReloadXPClockTheme(), CLS_NORMAL, CLS_HOT, CLS_PRESSED */
COLORREF GetXPClockColorBG(HWND hwndClock, int state);
/**
 * \brief reloads current theme information used by \c GetXPClockColor() and \c GetXPClockColorBG()
 * \sa GetXPClockColor(), GetXPClockColorBG() */
void ReloadXPClockTheme();
/**
 * \brief is theme engine active (always true for Win8+? or does it indicate that we're not running a high contrast theme?)
 * \return boolean
 * \sa SetXPWindowTheme(), GetXPClockColor(), GetXPClockColorBG() */
BOOL IsXPThemeActive();
/**
 * \brief enable 3rd party theme for window \a hwnd
 * \param hwnd target window
 * \param pszSubAppName other application's theme to use. Eg. "Explorer"
 * \param pszSubIdList theme ID to use. Default: \c NULL
 * \return \c S_OK on success, otherwise returns an \c HRESULT error code.
 * \sa SetWindowTheme() */
HRESULT SetXPWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
/**
 * \brief draw parent background for \a hwnd
 * \param hwnd window to paint
 * \param hdc \c DC of window to paint
 * \param prc \c RECT structure with region to paint
 * \sa DrawXPClockHover(), DrawThemeParentBackground() */
void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT* prc);
/**
 * \brief draw parent background hover for \a hwnd using \c CLP_TIME and \c CLS_HOT
 * \param hwnd window to paint hover background
 * \param hdc \c DC of window to paint
 * \param prc \c RECT structure with region to paint
 * \sa DrawXPClockBackground(), DrawThemeParentBackground() */
void DrawXPClockHover(HWND hwnd, HDC hdc, RECT* prc);

// UAC and stuff

#ifndef BCM_SETSHIELD /* _WIN32_WINNT >= 0x0600 */
#	define BCM_SETSHIELD            (BCM_FIRST + 0x000C)
/** \brief add or remove a UAC shield icon for a button \remark this macro is only useful on Windows Vista+ */
#	define Button_SetElevationRequiredState(hwnd, fRequired) \
		(LRESULT)SNDMSG((hwnd), BCM_SETSHIELD, 0, (LPARAM)fRequired)
#endif

#ifdef __cplusplus
}
#endif
#endif // TCLOCK_NEWAPI_H
