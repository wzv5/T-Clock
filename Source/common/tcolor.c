#include "globals.h"
#include "newapi.h"
#include "tcolor.h"

unsigned themecolor=0x00000000;
void OnTColor_DWMCOLORIZATIONCOLORCHANGED(unsigned argb) /// there's a bug with "high" or "low" color intensity...
{
	union{
		unsigned ref;
		RGBQUAD quad;
	} col;
	BYTE whitepart;
	unsigned tmp;
	col.ref=(argb&0xFF00FF00)|((argb&0xFF)<<16)|((argb>>16)&0xFF);
	whitepart=255-col.quad.rgbReserved;
	tmp=col.quad.rgbBlue*col.quad.rgbReserved/0xFF + whitepart;
	col.quad.rgbBlue=(tmp>255?255:(BYTE)tmp);
	tmp=col.quad.rgbGreen*col.quad.rgbReserved/0xFF + whitepart;
	col.quad.rgbGreen=(tmp>255?255:(BYTE)tmp);
	tmp=col.quad.rgbRed*col.quad.rgbReserved/0xFF + whitepart;
	col.quad.rgbRed=(tmp>255?255:(BYTE)tmp);
	col.quad.rgbReserved=0x00;
	themecolor=col.ref;
}
unsigned GetTColor(unsigned ogbr,int useraw)
{
	// useraw = 1 == want some raw values, mainly "default"
	// useraw = 2 == always want raw values (that is, everytime a TCOLOR_* isn't save to be right)
	unsigned sub;
	if((ogbr&TCOLOR_MASK)!=TCOLOR_MAGIC)
		return ogbr;
	sub=ogbr^TCOLOR_MAGIC;
	if(sub<TCOLOR_BEGIN_)
		return GetSysColor(sub);
	switch(sub){
	case TCOLOR_DEFAULT:
		if(useraw)
			return ogbr;
		if(IsXPThemeActive())
			return GetXPClockColor();
		else
			return GetSysColor(COLOR_WINDOWTEXT);
	case TCOLOR_TRANSPARENT:
		if(useraw)
			return ogbr;
		return 0xFF000000;
	case TCOLOR_THEME:{
		HKEY hkey;
		if(!themecolor && RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\DWM", &hkey) == 0) {
			DWORD regtype,size=sizeof(sub);
			if(RegQueryValueEx(hkey,"ColorizationColor",0,&regtype,(LPBYTE)&sub,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
				OnTColor_DWMCOLORIZATIONCOLORCHANGED(sub);
			RegCloseKey(hkey);
			return themecolor;
		}
		return themecolor;}
	case TCOLOR_THEME2:
		if(useraw==2)
			return ogbr;
		if(IsXPThemeActive())
			return GetXPClockColorBG();
		else
			return GetSysColor(COLOR_3DFACE);
	}
	return ogbr;
}
