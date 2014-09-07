#ifndef TCLOCK_NEWAPI_H
#define TCLOCK_NEWAPI_H

void EndNewAPI(HWND hwnd);
void SetLayeredTaskbar(HWND hwndClock,BOOL refresh);
void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans);

HRESULT SetXPWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
void InitDrawThemeParentBackground(void);

#endif // TCLOCK_NEWAPI_H
