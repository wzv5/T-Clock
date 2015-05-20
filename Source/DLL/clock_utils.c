//#include "tcdll.h"
// test only!!!
#include "../common/globals.h"
#include "../common/resource.h"
#include "../common/clock.h"
#include "clock_internal.h"
// \test only!!!

static const char m_regkey[] = "Software\\Stoic Joker's\\T-Clock 2010"; /**< our registry key root */
static const size_t m_regkey_size = sizeof(m_regkey); /**< size of \c m_regkey incl. trailing null \sa m_regkey */

char m_bIniSetting = 0;
char m_inifile[MAX_PATH] = {0};

// misc

int Clock_IsCalendarOpen(int set_focus)
{
	HWND hwnd = FindWindowEx(NULL,NULL,"ClockFlyoutWindow",NULL);
	if(hwnd){
		if(set_focus) SetForegroundWindow(hwnd);
		return 1;
	}
	return gs_bCalOpen;
}

int Clock_Message(HWND parent, const char* msg, const char* title, UINT uType, UINT uBeep)
{
	MSGBOXPARAMS mbp = {sizeof(MSGBOXPARAMS)};
	mbp.hwndOwner = parent;
	mbp.hInstance = hInstance;
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_MAIN);
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if(uBeep != 0xFFFFFFFF) MessageBeep(uBeep);
	return MessageBoxIndirect(&mbp);
}

void Clock_PositionWindow(HWND hwnd, int padding) {
	POINT cursor_pos;
	MONITORINFO moni = {sizeof(moni)};
	int wProp, hProp;
	
	GetWindowRect(hwnd,&moni.rcWork); // Properties Dialog Dimensions
	wProp = moni.rcWork.right-moni.rcWork.left;  //----------+++--> Width
	hProp = moni.rcWork.bottom-moni.rcWork.top; //----------+++--> Height
	
	GetCursorPos(&cursor_pos);
	GetMonitorInfo(MonitorFromPoint(cursor_pos,MONITOR_DEFAULTTONEAREST),&moni);
	
	if(moni.rcWork.top!=moni.rcMonitor.top || moni.rcWork.bottom!=moni.rcMonitor.bottom) { // taskbar is horizontal
		moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
		if(moni.rcWork.top!=moni.rcMonitor.top) // top
			moni.rcMonitor.top=moni.rcWork.top+padding;
		else // bottom
			moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
	}else if(moni.rcWork.left!=moni.rcMonitor.left || moni.rcWork.right!=moni.rcMonitor.right){ // vertical
		moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
		if(moni.rcWork.left!=moni.rcMonitor.left) // left
			moni.rcMonitor.left=moni.rcWork.left+padding;
		else // right
			moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
	}else{ // autohide taskbar
		MONITORINFO taskbarMoni;
		HWND taskbar=FindWindow("Shell_TrayWnd",NULL);
		RECT taskbarRC;
		int tbW,tbH;
		if(!taskbar)
			return;
		taskbarMoni.cbSize=sizeof(moni);
		GetMonitorInfo(MonitorFromWindow(taskbar,MONITOR_DEFAULTTONEAREST),&taskbarMoni); // correct monitor for single monitor setup, probably wrong for multimon
		GetWindowRect(taskbar,&taskbarRC);
		tbW=taskbarRC.right-taskbarRC.left;
		tbH=taskbarRC.bottom-taskbarRC.top;
		if(tbW > tbH){ // horizontal
			int diff=taskbarMoni.rcMonitor.top-taskbarRC.top;
			int visiblesize=taskbarMoni.rcMonitor.bottom-taskbarMoni.rcMonitor.top+diff;
			moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
			if((diff>0 && diff>tbH/10) || !diff || (diff<0 && (visiblesize!=tbH && visiblesize>tbH/10))) // top
				moni.rcMonitor.top=moni.rcWork.top+padding+tbH;
			else // bottom
				moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding-tbH;
		}else{
			int diff=taskbarMoni.rcMonitor.left-taskbarRC.left;
			int visiblesize=taskbarMoni.rcMonitor.right-taskbarMoni.rcMonitor.left+diff;
			moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
			if((diff>0 && diff>tbW/10) || !diff || (diff<0 && (visiblesize!=tbW && visiblesize>tbW/10))) // left
				moni.rcMonitor.left=moni.rcWork.left+padding+tbW;
			else // right
				moni.rcMonitor.left=moni.rcWork.right-wProp-padding-tbW;
		}
	}
	SetWindowPos(hwnd,HWND_TOP,moni.rcMonitor.left,moni.rcMonitor.top,0,0,SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
}

void Clock_GetFileAndOption(const char* command, char* fname, char* opt) {
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

// registry

#ifdef RegDeleteKeyEx
#	undef RegDeleteKeyEx
#endif // RegDeleteKeyEx
#define RegDeleteKeyEx MyRegDeleteKeyEx
static LONG WINAPI MyRegDeleteKeyEx(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved){
	typedef LONG (WINAPI* RegDeleteKeyEx_t)(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved);
	static RegDeleteKeyEx_t pRegDeleteKeyEx=NULL;
	if(gs_tos > TOS_XP){ // actually XP 64bit supports it...
		if(!pRegDeleteKeyEx)
			pRegDeleteKeyEx=(RegDeleteKeyEx_t)GetProcAddress(GetModuleHandle("Advapi32"),"RegDeleteKeyExA");
		if(pRegDeleteKeyEx)
			return pRegDeleteKeyEx(hKey,lpSubKey,samDesired,Reserved);
	}
	return RegDeleteKey(hKey,lpSubKey);
}

/**
 * \brief prepares \a key string for registry functions by prefixing \a section with \c m_regkey .
 * Furthermore, initializes ini path \c m_inifile if enabled by \c m_bIniSetting
 * \param[out] key resulting key to query or manipulate
 * \param[in] section
 * \return boolean
 * \remarks call once for every Get/Set* function and before using \c m_inifile
 * \sa m_regkey, m_bIniSetting, m_inifile */
static int PrepareMyRegKey_(char key[80], const char* section) {
	size_t section_len = (section ? strlen(section)+1 : 0);
	
	if(m_regkey_size+section_len > 80){
		#ifdef _DEBUG
		MessageBox(NULL,"settings key too huge","PrepareMyRegKey",0);
		#endif
		return 0;
	}
	
	if(m_bIniSetting){
		if(!m_inifile[0]){
			strcpy(m_inifile, api.root);
			strcat(m_inifile, "\\Clock.ini");
		}
		if(section_len > 1)
			memcpy(key, section, section_len);
		else
			memcpy(key, "Main", 5);
		return 1;
	}

	memcpy(key, m_regkey, m_regkey_size);
	if(section_len > 1){
		key[m_regkey_size-1] = '\\';
		memcpy(key+m_regkey_size, section, section_len);
	}
	return 1;
}

int Clock_GetInt(const char* section, const char* entry, LONG defval) {
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, m_inifile);
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

int Clock_GetIntEx(const char* section, const char* entry, LONG defval) {
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, m_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
				DWORD regtype,size=sizeof(LONG);
				LONG dw;
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD){
					defval = dw;
				}else{
					Clock_SetInt(section,entry,defval);
				}
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

int Clock_GetSystemInt(HKEY rootkey, const char* section, const char* entry, LONG defval) {
	HKEY hkey;
	
	if(RegOpenKeyEx(rootkey,section,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
		DWORD regtype,size=sizeof(LONG);
		LONG dw;
		if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
			defval = dw;
		RegCloseKey(hkey);
	}
	return defval;
}

int Clock_GetStr(const char* section, const char* entry, char* val, int len, const char* defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;
	int ret=-1;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val, len, m_inifile);
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

int Clock_GetStrEx(const char* section, const char* entry, char* val, int len, const char* defval) {
	HKEY hkey;	char key[80];	DWORD regtype, size;
	int ret=-1;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val, len, m_inifile);
			if(ret == len)
				Clock_SetStr(section, entry, defval);
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
			Clock_SetStr(section, entry, defval);
			strcpy(val,defval);
		}else ret=0;
	}
	if(!ret) *val='\0';
	return ret;
}

int Clock_GetSystemStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval) {
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

BOOL Clock_SetInt(const char* section, const char* entry, LONG val) {
	HKEY hkey;
	BOOL r = FALSE;
	char key[80];
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting) {
			char s[20];
			wsprintf(s, "%d", val);
			if(WritePrivateProfileString(key, entry, s, m_inifile))
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

BOOL Clock_SetStr(const char* section, const char* entry, const char* val) {
	HKEY hkey;	char key[80];	BOOL r=FALSE;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting)
			return WritePrivateProfileString(key, entry, val, m_inifile);
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,NULL,&hkey,NULL) == ERROR_SUCCESS) {
			if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)strlen(val)) == ERROR_SUCCESS) {
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

BOOL Clock_DelValue(const char* section, const char* entry) {
	HKEY hkey;	char key[80];	BOOL r = FALSE;
	
	if(PrepareMyRegKey_(key,section)){
		if(m_bIniSetting)
			return WritePrivateProfileString(key, entry, NULL, m_inifile);
		
		if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&hkey) == ERROR_SUCCESS) {
			if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS) r = TRUE;
			RegCloseKey(hkey);
		}
	}
	return r;
}

BOOL Clock_DelKey(const char* section) {
	char key[80];
	
	if(!PrepareMyRegKey_(key,section))
		return 0;
	if(m_bIniSetting)
		return WritePrivateProfileString(key,NULL,NULL,m_inifile);
	return RegDeleteKeyEx(HKEY_CURRENT_USER,key,KEY_WOW64_64KEY,0) == ERROR_SUCCESS;
}

// exec

int Clock_ShellExecute(const char* method, const char* app, const char* params, HWND parent, int show) {
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
int Clock_Exec(const char* app, const char* params, HWND parent) {
	return Clock_ShellExecute(NULL,app,params,parent,SW_SHOWNORMAL);
}
int Clock_ExecFile(const char* command, HWND parent) {
	char app[MAX_PATH], params[MAX_PATH];
	if(!command[0])
		return -1;
	// if(parent) SetForegroundWindow(parent);
	Clock_GetFileAndOption(command,app,params);
	return Clock_ShellExecute(NULL,app,(params[0]?params:NULL),parent,SW_SHOWNORMAL);
}
