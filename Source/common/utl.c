/*-------------------------------------------
  utl.c - KAZUBON 1997-1999
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "globals.h"
#include "utl.h"
//#include <sys/types.h>
#include <sys/stat.h>
//==================================================================================
//--------------------------------------------------+++--> finds the tray clock hwnd:
HWND FindClock()   //---------------------------------------------------------+++-->
{
	char classname[80];
	HWND hwndBar = FindWindow("Shell_TrayWnd",NULL);
	// find the clock window
	HWND hwndChild;
	for(hwndChild=GetWindow(hwndBar,GW_CHILD); hwndChild; hwndChild=GetWindow(hwndChild,GW_HWNDNEXT)) {
		GetClassName(hwndChild,classname,sizeof(classname));
		if(!lstrcmpi(classname,"TrayNotifyWnd")) {
			for(hwndChild=GetWindow(hwndChild,GW_CHILD); hwndChild; hwndChild=GetWindow(hwndChild,GW_HWNDNEXT)) {
				GetClassName(hwndChild,classname,sizeof(classname));
				if(!lstrcmpi(classname,"TrayClockWClass"))
					return hwndChild;
			}
			break;
		}
	}
	return NULL;
}
//===================================================================================
//-------------------------------------+++--> Force a ReDraw of T-Clock & the TaskBar:
void RefreshUs()   //----------------------------------------------------------+++-->
{
	HWND hclock=FindClock();
	if(hclock){
		SendMessage(hclock,CLOCKM_REFRESHCLOCK,0,0);
		SendMessage(hclock,CLOCKM_REFRESHTASKBAR,0,0);
	}
}
#ifndef S_ISDIR
#	define S_ISDIR(mode) (mode&S_IFDIR)
#endif // S_ISDIR
char PathExists(const char* path){
	struct stat st;
	if(stat(path,&st)==-1) return 0;
	return S_ISDIR(st.st_mode)?2:1;
}

int atox(const char* p)
{
	int r = 0;
	for(; *p; ++p) {
		if('0' <= *p && *p <= '9') r=(r<<4) + *p-'0';
		else if('A' <= *p && *p <= 'F') r=(r<<4) + *p-('A'-10);
		else if('a' <= *p && *p <= 'f') r=(r<<4) + *p-('a'-10);
		else break;
	}
	return r;
}

__inline int toupper(int c)
{
	if('a' <= c && c <= 'z') c -= 'a' - 'A';
	return c;
}

void add_title(char* path, const char* title)
{
	char* p=path;
	if(*path && (!*title || title[1]!=':')){ // not absolute device path
		if(*title == '\\') { // absolute path
			if(*p && p[1]==':') p+=2;
		}else{ // relative path
			for(; *p; p=CharNext(p)) {
				if((*p=='\\' || *p=='/') && !p[1]) {
					break;
				}
			}
			*p++='\\';
		}
	}
	while(*title) *p++=*title++;
	*p='\0';
}

void del_title(char* path)
{
	char* p,* ep;
	
	for(p=ep=path; *p; p=CharNext(p)) {
		if(*p=='\\' || *p=='/') {
			if(p>path && p[-1]==':') ep=p+1;
			else ep=p;
		}
	}
	*ep='\0';
}

void get_title(char* dst, const char* path)
{
	const char* p,* ep;
	
	for(p=ep=path; *p; p=CharNext(p)) {
		if(*p=='\\' || *p=='/') {
			if(!*CharNext(p)) break;
			if(p>path && p[-1]==':') ep=p+1;
			else ep=p;
		}
	}
	
	if(*ep == '\\' || *ep == '/') ++ep;
	
	while(*ep) *dst++=*ep++;
	if(dst[-1]=='\\' || dst[-1]=='/')
		--dst;
	*dst='\0';
}

int ext_cmp(const char* fname, const char* ext)
{
	const char* p, *sp;
	
	sp=NULL;
	for(p=fname; *p; p=CharNext(p)) {
		if(*p=='.') sp=p;
		else if(*p=='\\' || *p=='/') sp=NULL;
	}
	
	if(!sp) sp=p;
	if(*sp=='.') ++sp;
	
	for(;*sp||*ext; ++sp,++ext) {
		if(toupper(*sp)!=toupper(*ext))
			return (toupper(*sp)-toupper(*ext));
	}
	return 0;
}
/*
void parse(char* dst, char* src, int n)
{
	char* dp;
	int i;
	
	for(i = 0; i < n; i++) {
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0) {
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ',') *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp) {
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}// */
/*
void parsechar(char* dst, char* src, char ch, int n)
{
	char* dp;
	int i;
	
	for(i = 0; i < n; i++) {
		while(*src && *src != ch) src++;
		if(*src == ch) src++;
	}
	if(*src == 0) {
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ch) *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp) {
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}// */

void str0cat(char* list, const char* str)
{
	if(list[0]||list[1]){ // find last string pair
		for(; list[0]||list[1]; ++list);
		++list;
	}
	for(; *str; *list++=*str++); // append new string
	list[0]=list[1]='\0'; // end string & pair
}

/*---------------------------------------------
--------------------- returns a resource string
---------------------------------------------*/
char* MyString(UINT id)
{
	static char buf[80];
	
	*buf = '\0';
	LoadString(GetModuleHandle(NULL), id, buf, 80);
	return buf;
}

//================================================================================================
//----------------------------------------//--------------------------+++--> 32bit x 32bit = 64bit:
ULONGLONG M32x32to64(DWORD a, DWORD b)   //-------------------------------------------------+++-->
{
	ULARGE_INTEGER ret = {{0}};
	DWORD* p1, *p2, *p3;
	p1 = &ret.LowPart;
	p2 = (DWORD*)((char*)p1 + 2);
	p3 = (DWORD*)((char*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return ret.QuadPart;
}
/*
#include <stddef.h>
void ForceForegroundWindow(HWND hwnd)
{
	DWORD fgthread=GetWindowThreadProcessId(GetForegroundWindow(),0);
	if(fgthread && _threadid^fgthread && AttachThreadInput(_threadid,fgthread,1)){
//		AllowSetForegroundWindow(ASFW_ANY);//does nothing... we become foreground, but won't receive window messages
//		SetFocus(hwnd);// "
		SetForegroundWindow(hwnd);
		BringWindowToTop(hwnd);
		AttachThreadInput(_threadid,fgthread,0);
		return;
	}
	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
}// */
//===============================================================================


/*
void Pause(HWND hWnd, LPCTSTR pszArgs)
{
	LONG lInterval = atoi(pszArgs);
	LONG lTime = GetTickCount();
	MSG msg;
	
	if(lInterval > 0) {
		while((LONG)(GetTickCount() - lTime) < lInterval) {
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}// */
/// currently unused drawing stuff
/*
// tile an image vertically
void VerticalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest,
					 HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
					 BOOL ReverseBlt, BOOL useTrans)
{
	int y;
	
	if(ReverseBlt) {
		for(y = cyDest - cySrc; y > yDest - cySrc; y -= cySrc) {
			TC2DrawBlt(hdcDest,
					   xDest,
					   y,
					   cxDest,
					   cySrc,
					   hdcSrc,
					   xSrc,
					   ySrc,
					   cxSrc,
					   cySrc,
					   useTrans);
		}
	} else {
		for(y = 0; y < cyDest; y += cySrc) {
			TC2DrawBlt(hdcDest,
					   xDest,
					   yDest + y,
					   cxDest,
					   __min(cyDest - y, cySrc),
					   hdcSrc,
					   xSrc,
					   ySrc,
					   cxSrc,
					   __min(cyDest - y, cySrc),
					   useTrans);
		}
	}
}
// tile an image horizontally and vertically
void FillTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rasterOp)
{
	int x, y;
	
	for(y = 0; y < cyDest; y += cySrc) {
		for(x = 0; x < cxDest; x += cxSrc) {
			BitBlt(hdcDest,
				   xDest + x,
				   yDest + y,
				   cxSrc,
				   cySrc,
				   hdcSrc,
				   xSrc,
				   ySrc,
				   rasterOp);
		}
	}
}
void TileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc,
			 int xSrc, int ySrc, int cxSrc, int cySrc, BOOL useTrans)
{
	int y, x;
	
	for(y = yDest; y < cyDest; y = y + cySrc) {
		for(x = xDest; x < cxDest; x = x + cxSrc) {
			TC2DrawBlt(hdcDest, x, y, cxSrc, cySrc,
					   hdcSrc, xSrc, ySrc, cxSrc, cySrc, useTrans);
		}
	}
}// */
