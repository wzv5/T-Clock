#include "tcdll.h"
#include "clock_internal.h"
#include <sys/stat.h>

static const char m_regkey[] = "Software\\Stoic Joker's\\T-Clock 2010"; /**< our registry key root */
static const size_t m_regkey_size = sizeof(m_regkey); /**< size of \c m_regkey incl. trailing null \sa m_regkey */

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
	mbp.hInstance = api.hInstance;
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

#ifndef S_ISDIR
#	define S_ISDIR(mode) (mode&S_IFDIR)
#endif // S_ISDIR
char Clock_PathExists(const char* path){
	struct stat st;
	if(stat(path,&st)==-1) return 0;
	return S_ISDIR(st.st_mode)?2:1;
}

void Clock_GetFileAndOption(const char* command, char* app, char* params) {
	const char* offset_params = NULL;
	const char* offset = command;
	char* out = app;
	
	for(; *offset; ) {
		if(*offset == ' ') {
			*out = '\0';
			if(Clock_PathExists(app) == 1){
				offset_params = offset;
			}else{
				if(offset-command <= MAX_PATH-5){
					memcpy(out, ".exe", 5);
					if(Clock_PathExists(app) == 1)
						offset_params = offset;
				}
			}
			for(; *offset == ' '; *out++ = *offset++);
			continue; // spaces skipped, check for null
		}
		*out++ = *offset++;
	}
	if(!offset_params){ // no valid file found
		offset_params = strchr(command, ' ');
		if(!offset_params) // entire command
			offset_params = offset;
	}
	out[offset_params-offset] = '\0';
	
	for(; *offset_params == ' ';  ++offset_params);
	
	strcpy(params, offset_params);
}

// registry

#ifdef RegDeleteKeyEx
#	undef RegDeleteKeyEx
#endif // RegDeleteKeyEx
#define RegDeleteKeyEx MyRegDeleteKeyEx
static LONG WINAPI MyRegDeleteKeyEx(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved){
	typedef LONG (WINAPI* RegDeleteKeyEx_t)(HKEY hKey,char* lpSubKey,REGSAM samDesired,DWORD Reserved);
	static RegDeleteKeyEx_t pRegDeleteKeyEx=NULL;
	if(gs_tos >= TOS_XP_64){
		if(!pRegDeleteKeyEx)
			pRegDeleteKeyEx=(RegDeleteKeyEx_t)GetProcAddress(GetModuleHandle("Advapi32"),"RegDeleteKeyExA");
		if(pRegDeleteKeyEx)
			return pRegDeleteKeyEx(hKey,lpSubKey,samDesired,Reserved);
	}
	return RegDeleteKey(hKey,lpSubKey);
}

/**
 * \brief prepares \a key string for registry functions by prefixing \a section with \c m_regkey
 * \param[out] key resulting key to query or manipulate
 * \param[in] section
 * \return boolean
 * \remarks call once for every Get/Set* function and before using \c m_inifile
 * \sa m_regkey, ms_bIniSetting, m_inifile */
static int PrepareMyRegKey_(char key[80], const char* section) {
	size_t section_len = (section ? strlen(section)+1 : 0);
	
	if(m_regkey_size+section_len > 80){
		#ifdef _DEBUG
		MessageBox(NULL,"settings key too huge","PrepareMyRegKey",0);
		#endif
		return 0;
	}
	
	if(ms_bIniSetting){
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
		if(ms_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, ms_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
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
		if(ms_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, ms_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
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
	
	if(RegOpenKeyEx(rootkey,section,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
		DWORD regtype,size=sizeof(LONG);
		LONG dw;
		if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
			defval = dw;
		RegCloseKey(hkey);
	}
	return defval;
}

int Clock_GetStr(const char* section, const char* entry, char* val, int len, const char* defval) {
	char key[80];
	HKEY hkey;
	DWORD regtype, size;
	int ret = -1;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val, len, ms_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
				size = len;
				if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
					ret = size;
					if(ret) --ret;
				}
				RegCloseKey(hkey);
			}
		}
	}
	if(ret == -1){
		if((ret=(int)strlen(defval)) <= len){
			strcpy(val, defval);
		}else ret = 0;
	}
	if(!ret) val[0] = '\0';
	return ret;
}

int Clock_GetStrEx(const char* section, const char* entry, char* val, int len, const char* defval) {
	char key[80];
	HKEY hkey;
	DWORD regtype, size;
	int ret = -1;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			ret = GetPrivateProfileString(key, entry, defval, val, len, ms_inifile);
			if(ret == len)
				Clock_SetStr(section, entry, defval);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
				size = len;
				if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
					ret = size;
					if(ret) --ret;
				}
				RegCloseKey(hkey);
			}
		}
	}
	if(ret == -1){
		if((ret=(int)strlen(defval)) <= len){
			Clock_SetStr(section, entry, defval);
			strcpy(val, defval);
		}else ret = 0;
	}
	if(!ret) val[0] = '\0';
	return ret;
}

int Clock_GetSystemStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval) {
	HKEY hkey;
	DWORD regtype, size;
	int ret = -1;
	
	if(RegOpenKeyEx(rootkey,section,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
		size = len;
		if(RegQueryValueEx(hkey, entry, 0, &regtype, (BYTE*)val, &size) == ERROR_SUCCESS) {
			ret = size;
			if(ret) --ret;
		}
		RegCloseKey(hkey);
	}
	if(ret == -1) {
		if((ret=(int)strlen(defval)) <= len){
			strcpy(val, defval);
		}else ret = 0;
	}
	if(!ret) val[0] = '\0';
	return ret;
}

int Clock_SetInt(const char* section, const char* entry, LONG val) {
	char key[80];
	HKEY hkey;
	int ret = 0;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			char s[20];
			wsprintf(s, "%d", val);
			return WritePrivateProfileString(key, entry, s, ms_inifile);
		} else {
			if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,ms_reg_sam,NULL,&hkey,NULL) == ERROR_SUCCESS) {
				if(RegSetValueEx(hkey,entry,0,REG_DWORD,(const BYTE*)&val,4) == ERROR_SUCCESS) {
					ret = 1;
				}
				RegCloseKey(hkey);
			}
		}
	}
	return ret;
}

int Clock_SetStr(const char* section, const char* entry, const char* val) {
	char key[80];
	HKEY hkey;
	int ret = 0;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting)
			return WritePrivateProfileString(key, entry, val, ms_inifile);
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,ms_reg_sam,NULL,&hkey,NULL) == ERROR_SUCCESS) {
			if(RegSetValueEx(hkey, entry, 0, REG_SZ, (const BYTE*)val, (DWORD)strlen(val)) == ERROR_SUCCESS) {
				ret = 1;
			}
			RegCloseKey(hkey);
		}
	}
	return ret;
}

int Clock_SetSystemStr(HKEY rootkey, const char* section, const char* entry, const char* val) {
	HKEY hkey;
	int ret = 0;
	
	if(RegCreateKeyEx(rootkey,section,0,NULL,0,ms_reg_sam,NULL,&hkey,NULL) == ERROR_SUCCESS) {
		if(RegSetValueEx(hkey, entry, 0, REG_SZ, (const BYTE*)val, (DWORD)strlen(val)) == ERROR_SUCCESS) {
			ret = 1;
		}
		RegCloseKey(hkey);
	}
	return ret;
}

int Clock_DelValue(const char* section, const char* entry) {
	char key[80];
	HKEY hkey;
	int ret = 0;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting)
			return WritePrivateProfileString(key, entry, NULL, ms_inifile);
		
		if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_sam,&hkey) == ERROR_SUCCESS) {
			if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS)
				ret = 1;
			RegCloseKey(hkey);
		}
	}
	return ret;
}

int Clock_DelKey(const char* section) {
	char key[80];
	
	if(!PrepareMyRegKey_(key,section))
		return 0;
	if(ms_bIniSetting)
		return WritePrivateProfileString(key, NULL, NULL, ms_inifile);
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
int Clock_ExecElevated(const char* app, const char* params, HWND parent)
{
	return Clock_ShellExecute("runas",app,params,parent,SW_SHOWNORMAL);
}
int Clock_ExecFile(const char* command, HWND parent) {
	char app[MAX_PATH], params[MAX_PATH];
	if(!command[0])
		return -1;
	// if(parent) SetForegroundWindow(parent);
	Clock_GetFileAndOption(command,app,params);
	return Clock_ShellExecute(NULL,app,(params[0]?params:NULL),parent,SW_SHOWNORMAL);
}
