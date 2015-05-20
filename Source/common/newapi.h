#ifndef TCLOCK_NEWAPI_H
#define TCLOCK_NEWAPI_H
#include <windows.h>

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

/**
 * \brief get current theme color for our clock text
 * \param hwndClock handle to our clock
 * \return COLORREF with our text color
 * \sa GetXPClockColorBG(), ReloadXPClockTheme() */
COLORREF GetXPClockColor(HWND hwndClock);
/**
 * \brief get current theme color for our clock background
 * \param hwndClock handle to our clock
 * \return COLORREF with our background color
 * \remarks might not work properly on XP
 * \sa GetXPClockColor(), ReloadXPClockTheme() */
COLORREF GetXPClockColorBG(HWND hwndClock);
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

#endif // TCLOCK_NEWAPI_H
