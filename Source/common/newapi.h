#ifndef TCLOCK_NEWAPI_H
#define TCLOCK_NEWAPI_H

void EndNewAPI(HWND hwnd);

int IsWow64();

void SetLayeredTaskbar(HWND hwndClock,BOOL refresh);
void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans);

COLORREF GetXPClockColor();
COLORREF GetXPClockColorBG();

void ReloadXPClockTheme();
BOOL IsXPThemeActive();
HRESULT SetXPWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT* prc);
void DrawXPClockHover(HWND hwnd, HDC hdc, RECT* prc);
void InitDrawThemeParentBackground(void);

#endif // TCLOCK_NEWAPI_H
