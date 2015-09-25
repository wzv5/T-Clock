/*-------------------------------------------
  newapi.c - Kazubon 1999
  GradientFill and Layerd Window
---------------------------------------------*/
#include "../common/globals.h"
#include <uxtheme.h>
#include "../common/utl.h"

int IsWow64(){
	int ret=0;
	#ifndef _WIN64
	typedef BOOL (WINAPI *IsWow64Process_t)(HANDLE hProcess,BOOL* iswow64);
	IsWow64Process_t pIsWow64Process=(IsWow64Process_t)GetProcAddress(GetModuleHandleA("kernel32"),"IsWow64Process");
	if(pIsWow64Process){
		pIsWow64Process(GetCurrentProcess(),&ret);
	}
	#endif // _WIN64
	return ret;
}

static HMODULE hmodUSER32 = NULL;
static HMODULE hmodUxTheme = NULL;

typedef BOOL (WINAPI* pSetLayeredWindowAttributes_t)(HWND,COLORREF,BYTE,DWORD);
pSetLayeredWindowAttributes_t pSetLayeredWindowAttributes=NULL;

/// UxTheme macros
#define THEME_FUNC_RETRIEVE(name) \
	p##name = (p##name##_t)GetProcAddress(hmodUxTheme, #name);\
	if(!p##name){\
		FreeLibrary(hmodUxTheme); hmodUxTheme = NULL;\
		return;\
	}
#define THEME_FUNC_RELEASE(name) \
	p##name = NULL
#define THEME_FUNC_CHECK(name,ret) \
	if(!p##name){\
		InitDrawTheme();\
		if(!p##name) return ret;\
	}
#define THEME_FUNC_CHECK_THEME(name,hwnd,ret) \
	THEME_FUNC_CHECK(name,ret)\
	if(!hClockTheme){\
		hClockTheme=pOpenThemeData(hwnd,CLOCK);/*TrayClockWClass*/\
		if(!hClockTheme) return ret;\
	}
#define THEME_FUNC_DEFINE(name,ret,params) \
	typedef ret (WINAPI* p##name##_t)params;\
	p##name##_t p##name=NULL
/// UxTheme defines
static HTHEME hClockTheme=NULL;
THEME_FUNC_DEFINE(CloseThemeData,HRESULT,(HTHEME hTheme));
THEME_FUNC_DEFINE(OpenThemeData,HTHEME,(HWND hwnd, LPCWSTR pszClassList));

THEME_FUNC_DEFINE(IsThemeActive,BOOL,());
THEME_FUNC_DEFINE(SetWindowTheme,HRESULT,(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList));
THEME_FUNC_DEFINE(GetThemeColor,HRESULT,(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor));
THEME_FUNC_DEFINE(DrawThemeBackground,HRESULT,(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect));
THEME_FUNC_DEFINE(DrawThemeParentBackground,HRESULT,(HWND hwnd, HDC hdc, RECT* prc));

static BOOL bInitLayeredWindow = FALSE;
static BOOL bInitDrawTheme = FALSE;

static void RefreshRebar(HWND hwndBar);

void InitLayeredWindow(void)
{
	if(bInitLayeredWindow) return;
	
	hmodUSER32 = LoadLibraryA("user32");
	if(hmodUSER32) {
		pSetLayeredWindowAttributes = (pSetLayeredWindowAttributes_t)GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
		if(!pSetLayeredWindowAttributes) {
			FreeLibrary(hmodUSER32); hmodUSER32 = NULL;
		}
	}
	bInitLayeredWindow = TRUE;
}

void EndNewAPI(HWND hwndClock)
{
	if(pSetLayeredWindowAttributes && hwndClock) {
		HWND hwnd;
		LONG_PTR exstyle;
		
		hwnd = GetParent(GetParent(hwndClock));
		exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if(exstyle & WS_EX_LAYERED) {
			exstyle &= ~WS_EX_LAYERED;
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);
			RefreshRebar(hwnd);
		}
	}
	
	if(hmodUSER32) FreeLibrary(hmodUSER32);
	hmodUSER32 = NULL;
	pSetLayeredWindowAttributes = NULL;
	/// DrawTheme
	THEME_FUNC_RELEASE(DrawThemeParentBackground);
	THEME_FUNC_RELEASE(DrawThemeBackground);
	THEME_FUNC_RELEASE(GetThemeColor);
	THEME_FUNC_RELEASE(OpenThemeData);
	THEME_FUNC_RELEASE(SetWindowTheme);
	THEME_FUNC_RELEASE(IsThemeActive);
	if(hClockTheme){
		pCloseThemeData(hClockTheme); hClockTheme=NULL;
	}
	THEME_FUNC_RELEASE(CloseThemeData);
	if(hmodUxTheme) FreeLibrary(hmodUxTheme);
	hmodUxTheme = NULL;
}
/*
void GradientFillClock(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2) {
	TRIVERTEX vert[2];
	GRADIENT_RECT gRect;

	vert[0].x      = prc->left;
	vert[0].y      = prc->top;
	vert[0].Red    = (COLOR16)GetRValue(col1) * 256;
	vert[0].Green  = (COLOR16)GetGValue(col1) * 256;
	vert[0].Blue   = (COLOR16)GetBValue(col1) * 256;
	vert[0].Alpha  = 0x0000;
	vert[1].x      = prc->right;
	vert[1].y      = prc->bottom;
	vert[1].Red    = (COLOR16)GetRValue(col2) * 256;
	vert[1].Green  = (COLOR16)GetGValue(col2) * 256;
	vert[1].Blue   = (COLOR16)GetBValue(col2) * 256;
	vert[1].Alpha  = 0x0000;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;

	GdiGradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}
*/
void SetLayeredTaskbar(HWND hwndClock, int alpha, int clear_taskbar, int refresh)
{
	LONG_PTR exstyle;
	HWND hwnd;
	
	alpha = 255 - (alpha * 255 / 100);
	if(alpha < 8) alpha = 8; else if(alpha > 255) alpha = 255;
	
	if(!pSetLayeredWindowAttributes && (alpha < 255 || clear_taskbar)) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;
	
	hwnd = GetParent(GetParent(hwndClock));
	
	exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if(alpha < 255 || clear_taskbar) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);
	if(refresh) {
		RefreshUs();
	}
	RefreshRebar(hwnd);
	
	if(alpha < 255 && !clear_taskbar)
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);
	else if(clear_taskbar)
		pSetLayeredWindowAttributes(hwnd, GetSysColor(COLOR_3DFACE), (BYTE)alpha, LWA_COLORKEY|LWA_ALPHA);
}

/*--------------------------------------------------
    redraw ReBarWindow32 forcely
----------------------------------------------------*/
void RefreshRebar(HWND hwndBar)
{
	HWND hwnd;
	char classname[80];
	
	hwnd = GetWindow(hwndBar, GW_CHILD);
	while(hwnd) {
		GetClassNameA(hwnd, classname, 80);
		if(strcasecmp(classname, "ReBarWindow32") == 0) {
			InvalidateRect(hwnd, NULL, TRUE);
			hwnd = GetWindow(hwnd, GW_CHILD);
			while(hwnd) {
				InvalidateRect(hwnd, NULL, TRUE);
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
			break;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

//void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans)
//{
//	if(useTrans) GdiTransparentBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, RGB(255, 0, 255));
//	else StretchBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, SRCCOPY);
//}

/// DrawTheme

void InitDrawTheme()
{
	if(bInitDrawTheme) return;
	bInitDrawTheme=1;
	hmodUxTheme=LoadLibraryA("uxtheme");
	if(hmodUxTheme){
		THEME_FUNC_RETRIEVE(CloseThemeData);
		THEME_FUNC_RETRIEVE(OpenThemeData);
		THEME_FUNC_RETRIEVE(IsThemeActive);
		THEME_FUNC_RETRIEVE(SetWindowTheme);
		THEME_FUNC_RETRIEVE(GetThemeColor);
		THEME_FUNC_RETRIEVE(DrawThemeBackground);
		THEME_FUNC_RETRIEVE(DrawThemeParentBackground);
	}
}

/// Win7+ clock defines
#define TASKBAND L"TaskBand2"
#define CLOCK L"Clock"
#ifndef CLP_TIME
#	define CLP_TIME 1
#	define CLS_NORMAL 1
#	define CLS_HOT 2
#	define CLS_PRESSED 3
//#	define TMT_COLOR 204
#	define TMT_BACKGROUND 1602
#	define TMT_TEXTCOLOR 3803

#	define TMT_TRANSPARENTCOLOR 3809
#	define TMT_WINDOW 1606
#	define TMT_WINDOWFRAME 1607
#	define TMT_FILLCOLOR 3802
#endif
void ReloadXPClockTheme()
{
	THEME_FUNC_CHECK(OpenThemeData,)
	if(hClockTheme){
		pCloseThemeData(hClockTheme);
		hClockTheme=NULL;
	}
}
COLORREF GetXPClockColor(HWND hwndClock)
{
	COLORREF ret;
	THEME_FUNC_CHECK_THEME(GetThemeColor,hwndClock,0x00FFFFFF)
	pGetThemeColor(hClockTheme,CLP_TIME,CLS_NORMAL,TMT_TEXTCOLOR,&ret);
	return ret;
}
COLORREF GetXPClockColorBG(HWND hwndClock)
{
	COLORREF ret;
	THEME_FUNC_CHECK_THEME(GetThemeColor,hwndClock,0x00000000)
	pGetThemeColor(hClockTheme,CLP_TIME,CLS_NORMAL,TMT_BACKGROUND,&ret);
	return ret;
}
void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT* prc)
{
	THEME_FUNC_CHECK(DrawThemeParentBackground,)
	pDrawThemeParentBackground(hwnd, hdc, prc);
}
void DrawXPClockHover(HWND hwnd, HDC hdc, RECT* prc)
{
	THEME_FUNC_CHECK_THEME(DrawThemeBackground,hwnd,)
	pDrawThemeBackground(hClockTheme,hdc,CLP_TIME,CLS_HOT,prc,NULL);
}

BOOL IsXPThemeActive(){
	HIGHCONTRAST highcontContrast={sizeof(HIGHCONTRAST)};
	THEME_FUNC_CHECK(IsThemeActive,0)
	SystemParametersInfo(SPI_GETHIGHCONTRAST,sizeof(HIGHCONTRAST),&highcontContrast,0);
	if(highcontContrast.dwFlags&HCF_HIGHCONTRASTON)
		return 0;
	return pIsThemeActive();
}
HRESULT SetXPWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList){
	THEME_FUNC_CHECK(SetWindowTheme,S_FALSE)
	return pSetWindowTheme(hwnd,pszSubAppName,pszSubIdList);
}
