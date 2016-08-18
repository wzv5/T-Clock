#include "tcdll.h"
#include "clock_internal.h"

static unsigned m_themecolor = 0x00000000;
static unsigned m_themecolor_dark;
static unsigned m_themecolor_alpha;

void Clock_On_DWMCOLORIZATIONCOLORCHANGED(unsigned argb, BOOL blend) { /// there's a bug with "high" or "low" color intensity...
	union{
		unsigned ref;
		RGBQUAD quad;
	} col;
	BYTE whitepart;
	unsigned tmp;
	// currently, DwmGetColorizationColor() returns wrong values in some cases,
	// but we can't use WM_DWMCOLORIZATIONCOLORCHANGED on startup...
	// so always rely on the bugged DwmGetColorizationColor()
	/* slightly wrong color but always "works". DwmGetColorizationColor fails for some colors
	HKEY hkey;
	if(RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\DWM", &hkey) == 0){
		DWORD regtype,size=sizeof(sub);
		if(RegQueryValueEx(hkey,"ColorizationColor",0,&regtype,(LPBYTE)&sub,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
			Clock_On_DWMCOLORIZATIONCOLORCHANGED(sub, 1);
		RegCloseKey(hkey);
	}// */
	typedef HRESULT (WINAPI *DwmGetColorizationColorPtr)(DWORD* pcrColorization, BOOL* pfOpaqueBlend);
	HMODULE hdwm = LoadLibraryA("dwmapi");
	if(hdwm) {
		DwmGetColorizationColorPtr pDwmGetColorizationColor; /**< \sa DwmGetColorizationColor() */
		pDwmGetColorizationColor = (DwmGetColorizationColorPtr)GetProcAddress(hdwm, "DwmGetColorizationColor");
		pDwmGetColorizationColor((DWORD*)&argb, &blend);
		FreeLibrary(hdwm);
	} else {
		argb = 0x77010101; // fallback for <Vista
	}
	col.ref=(argb&0xFF00FF00)|((argb&0xFF)<<16)|((argb>>16)&0xFF);
	if(!col.quad.rgbReserved && col.ref) // fix for high intensity colors and DwmGetColorizationColor()
		col.quad.rgbReserved = 0xFF;
//	if(!blend) DBGMSG("!blend"); // so far, not seen on Win8 but 7
	
	tmp = col.quad.rgbReserved;
	col.quad.rgbReserved = 0xFF - col.quad.rgbReserved;
	m_themecolor_alpha = col.ref;
	
	col.quad.rgbReserved = 0x00;
	m_themecolor_dark = col.ref;
	col.quad.rgbReserved = (BYTE)tmp;
	
	whitepart = (255-col.quad.rgbReserved);
	tmp = (col.quad.rgbBlue*col.quad.rgbReserved/0xFF) + whitepart;
	col.quad.rgbBlue = (BYTE)tmp;
	tmp = (col.quad.rgbGreen*col.quad.rgbReserved/0xFF) + whitepart;
	col.quad.rgbGreen = (BYTE)tmp;
	tmp = (col.quad.rgbRed*col.quad.rgbReserved/0xFF) + whitepart;
	col.quad.rgbRed = (BYTE)tmp;
	col.quad.rgbReserved = 0x00;
	m_themecolor = col.ref;
}
unsigned Clock_GetColor(unsigned color, int flags)
{
	// useraw = 1 == want some raw values, mainly "default"
	// useraw = 2 == always want raw values (that is, everytime a TCOLOR_* isn't save to be right)
	unsigned sub;
	int state = CLS_NORMAL;
	if((color&TCOLOR_MASK)!=TCOLOR_MAGIC)
		return color;
	if(flags & TCOLORFLAG_HOVER)
		state = CLS_HOT;
//	if(flags & TCOLORFLAG_PRESSED)
//		state = CLS_PRESSED;
	sub = color^TCOLOR_MAGIC;
	if(sub < TCOLOR_BEGIN_)
		return GetSysColor(sub);
	switch(sub){
	case TCOLOR_DEFAULT:
		if(flags & (TCOLORFLAG_RAW1|TCOLORFLAG_RAW2))
			return color;
		if(IsXPThemeActive() && gs_hwndClock)
			return GetXPClockColor(gs_hwndClock, state);
		else
			return GetSysColor(COLOR_WINDOWTEXT);
	case TCOLOR_TRANSPARENT:
		if(flags & (TCOLORFLAG_RAW1|TCOLORFLAG_RAW2))
			return color;
		return 0xFF000000;
	case TCOLOR_THEME:
		if(!m_themecolor)
			Clock_On_DWMCOLORIZATIONCOLORCHANGED(0, 1);
		return m_themecolor;
	case TCOLOR_THEME_DARK:
		if(!m_themecolor)
			Clock_On_DWMCOLORIZATIONCOLORCHANGED(0, 1);
		return m_themecolor_dark;
	case TCOLOR_THEME_ALPHA:
		if(!m_themecolor)
			Clock_On_DWMCOLORIZATIONCOLORCHANGED(0, 1);
		return m_themecolor_alpha;
	case TCOLOR_THEME_BG:
		if(flags & TCOLORFLAG_RAW2)
			return color;
		if(IsXPThemeActive() && gs_hwndClock)
			return GetXPClockColorBG(gs_hwndClock, state);
		else
			return GetSysColor(COLOR_3DFACE);
	}
	return color;
}
