/*-------------------------------------------
  utl.c - KAZUBON 1997-1999
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "globals.h"
#include "utl.h"
//#include <sys/types.h>
#include <sys/stat.h>
unsigned short g_tos=0;
static const char m_mykey[] = "Software\\Stoic Joker's\\T-Clock 2010";
static const size_t m_mykey_size = sizeof(m_mykey);

char g_bIniSetting = 0;
char g_inifile[MAX_PATH];

#ifdef RegDeleteKeyEx
#	undef RegDeleteKeyEx
#endif // RegDeleteKeyEx
#define RegDeleteKeyEx MyRegDeleteKeyEx
static LONG WINAPI MyRegDeleteKeyEx(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved){
	typedef LONG (WINAPI* RegDeleteKeyEx_t)(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved);
	static RegDeleteKeyEx_t pRegDeleteKeyEx=NULL;
	if(g_tos>TOS_XP){ // actually XP 64bit supports it...
		if(!pRegDeleteKeyEx)
			pRegDeleteKeyEx=(RegDeleteKeyEx_t)GetProcAddress(GetModuleHandle("Advapi32"),"RegDeleteKeyExA");
		if(pRegDeleteKeyEx)
			return pRegDeleteKeyEx(hKey,lpSubKey,samDesired,Reserved);
	}
	return RegDeleteKey(hKey,lpSubKey);
}
//================================================================================================
//---------------------------//-----+++--> Find Out If it's Older Then Windows 2000 If it is, Die!:
BOOL CheckSystemVersion()   //--------------------------------------------------------------+++-->
{
	OSVERSIONINFO osvi={sizeof(OSVERSIONINFO)};
	if(!GetVersionEx(&osvi))
		return FALSE;
	switch(osvi.dwMajorVersion){
	case 5: // 2000-Vista
		switch(osvi.dwMinorVersion ){
		case 0:
			g_tos=TOS_2000; break;
		default:
			g_tos=TOS_XP;
		}
		break;
	case 6: // Vista+
		switch(osvi.dwMinorVersion ){
		case 0:
			g_tos=TOS_VISTA; break;
		case 1:
			g_tos=TOS_WIN7; break;
		case 2:
			g_tos=TOS_WIN8; break;
		case 3:
			g_tos=TOS_WIN8_1; break;
		case 4:
			g_tos=TOS_WIN10; break;
		default:
			g_tos=TOS_NEWER;
		}
		break;
	default:
		if(osvi.dwMajorVersion<5)
			return FALSE;
		g_tos=TOS_NEWER;
	}
	if(g_tos>=TOS_VISTA){
		GetTickCount64_t ptr=(GetTickCount64_t)GetProcAddress(GetModuleHandle("kernel32"),"GetTickCount64");
		if(ptr) pGetTickCount64=ptr;
	}
	return TRUE;
}
ULONGLONG WINAPI GetTickCount64_Wrapper(){
	return GetTickCount();
}
GetTickCount64_t pGetTickCount64=GetTickCount64_Wrapper;
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
/*--------------------------------------------------------
  Retreive a file name and option from a command string
----------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* opt)
{
	const char* p, *pe;
	char* pd;
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	p = command; pd = fname;
	pe = NULL;
	for(; ;) {
		if(*p == ' ' || *p == 0) {
			*pd = 0;
			hfind = FindFirstFile(fname, &fd);
			if(hfind != INVALID_HANDLE_VALUE) {
				FindClose(hfind);
				pe = p;
			}
			if(*p == 0) break;
		}
		*pd++ = *p++;
	}
	if(pe == NULL) pe = p;
	
	p = command; pd = fname;
	for(; p != pe;) {
		*pd++ = *p++;
	}
	*pd = 0;
	if(*p == ' ') p++;
	
	pd = opt;
	for(; *p;) *pd++ = *p++;
	*pd = 0;
}
/*------------------------------------------------
  Open a file
--------------------------------------------------*/
int MyShellExecute(const char* method, const char* app, const char* params, HWND parent, int show)
{
	if(*app){
		SHELLEXECUTEINFO sei = {sizeof(sei)};
		sei.hwnd = parent;
		sei.lpVerb = method;
		sei.lpFile = app;
		sei.lpParameters = params;
		sei.nShow = show;
		if(ShellExecuteEx(&sei))
			return 0;
		if(GetLastError()==ERROR_CANCELLED){ // UAC dialog user cancled
			return 1;
		}
	}
	return -1;
}
int Exec(const char* app, const char* params, HWND parent)
{
	return MyShellExecute(NULL,app,params,parent,SW_SHOWNORMAL);
}
int ExecFile(const char* command, HWND parent)
{
	char app[MAX_PATH], params[MAX_PATH];
	if(!command[0])
		return -1;
	// if(parent) SetForegroundWindow(parent);
	GetFileAndOption(command,app,params);
	return MyShellExecute(NULL,app,(params[0]?params:NULL),parent,SW_SHOWNORMAL);
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

int MyMessageBox(HWND hwnd, const char* msg, const char* title, UINT uType, UINT uBeep)
{
	MSGBOXPARAMS mbp={sizeof(MSGBOXPARAMS)};
	mbp.hwndOwner = hwnd;
	mbp.hInstance = GetModuleHandle(NULL);
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_MAIN);
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if(uBeep != 0xFFFFFFFF) MessageBeep(uBeep);
	return MessageBoxIndirect(&mbp);
}
//================================================================================================
//----------------------------------------//--------------------------+++--> 32bit x 32bit = 64bit:
ULONGLONG M32x32to64(DWORD a, DWORD b)   //-------------------------------------------------+++-->
{
	ULARGE_INTEGER ret = {0};
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

static int PrepareMyRegKey_(char key[80], const char* section)
{
	size_t section_len = (section ? strlen(section)+1 : 0);
	
	if(m_mykey_size+section_len > 80){
		#ifdef _DEBUG
		MessageBox(NULL,"settings key too huge","PrepareMyRegKey",0);
		#endif
		return 0;
	}
	
	if(g_bIniSetting){
		if(section_len > 1)
			memcpy(key, section, section_len);
		else
			memcpy(key, "Main", 5);
		return 1;
	}

	memcpy(key, m_mykey, m_mykey_size);
	if(section_len > 1){
		key[m_mykey_size-1] = '\\';
		memcpy(key+m_mykey_size, section, section_len);
	}
	return 1;
}

int GetMyRegStr(const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	char key[80];	DWORD regtype, size;
	int ret=-1;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val,
										len, g_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey)==ERROR_SUCCESS) {
				size = len;
				if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
					ret=size;
					if(ret) --ret;
				}
				RegCloseKey(hkey);
			}
		}
	}
	if(ret==-1){
		if((ret=(int)strlen(defval))<=len){
			strcpy(val,defval);
		}else ret=0;
	}
	if(!ret) *val='\0';
	return ret;
}

int GetMyRegStrEx(const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	char key[80];	DWORD regtype, size;
	int ret=-1;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val,
										len, g_inifile);
			if(ret == len)
				SetMyRegStr(section, entry, defval);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey)==ERROR_SUCCESS) {
				size = len;
				if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
					ret=size;
					if(ret) --ret;
				}
				RegCloseKey(hkey);
			}
		}
	}
	if(ret==-1){
		if((ret=(int)strlen(defval))<=len){
			SetMyRegStr(section, entry, defval);
			strcpy(val,defval);
		}else ret=0;
	}
	if(!ret) *val='\0';
	return ret;
}

LONG GetMyRegLong(const char* section, const char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, g_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
				DWORD regtype,size=sizeof(LONG);
				LONG dw=0;
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
					defval=dw;
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

LONG GetMyRegLongEx(const char* section, const char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting) {
			return GetPrivateProfileInt(key,entry,defval,g_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
				DWORD regtype,size=sizeof(LONG);
				LONG dw=0;
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD){
					defval=dw;
				}else{
					SetMyRegLong(section,entry,defval);
				}
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

int GetRegStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	DWORD regtype, size;	BOOL b = FALSE;	int r=0;
	
	if(RegOpenKeyEx(rootkey,section,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
		size = len;
		if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
			if(size == 0) *val = 0;
			b = TRUE;
		}
		RegCloseKey(hkey);
	}
	
	if(b == FALSE) {
		strcpy(val, defval);
		r = (int)strlen(defval);
	}
	return r;
}

BOOL SetMyRegStr(const char* section, const char* entry, const char* val)
{
	HKEY hkey;	char key[80];	BOOL r=FALSE;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting)
			return WritePrivateProfileString(key, entry, val, g_inifile);
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,NULL,&hkey,NULL) == ERROR_SUCCESS) {
			if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)strlen(val)) == ERROR_SUCCESS) {
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

BOOL SetMyRegLong(const char* section, const char* entry, LONG val)
{
	HKEY hkey;
	BOOL r = FALSE;
	char key[80];
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting) {
			char s[20];
			wsprintf(s, "%d", val);
			if(WritePrivateProfileString(key, entry, s, g_inifile))
				r = TRUE;
		} else {
			if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,NULL,&hkey,NULL) == ERROR_SUCCESS) {
				if(RegSetValueEx(hkey,entry,0,REG_DWORD,(CONST BYTE*)&val,4)==ERROR_SUCCESS) {
					r = TRUE;
				}
				RegCloseKey(hkey);
			}
		}
	}
	return r;
}

BOOL DelMyReg(const char* section, const char* entry)
{
	HKEY hkey;	char key[80];	BOOL r = FALSE;
	
	if(PrepareMyRegKey_(key,section)){
		if(g_bIniSetting)
			return WritePrivateProfileString(key,entry,NULL,g_inifile);
		
		if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
			if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS) r = TRUE;
			RegCloseKey(hkey);
		}
	}
	return r;
}

BOOL DelMyRegKey(const char* section)
{
	char key[80];
	
	if(!PrepareMyRegKey_(key,section))
		return 0;
	if(g_bIniSetting)
		return WritePrivateProfileString(key,NULL,NULL,g_inifile);
	return RegDeleteKeyEx(HKEY_CURRENT_USER,key,KEY_WOW64_64KEY,0) == ERROR_SUCCESS;
}

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
