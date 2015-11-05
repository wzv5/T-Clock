#include "tcdll.h"
#include "clock_internal.h"
#include <sys/stat.h>

static const char m_regkey[] = "Software\\Stoic Joker's\\T-Clock 2010"; /**< our registry key root */
static const size_t m_regkey_size = sizeof(m_regkey); /**< size of \c m_regkey incl. trailing null \sa m_regkey */
static const char kInvalidKey[] = "\1\b";

// misc

HWND Clock_GetCalendar()
{
	HWND hwnd = FindWindowEx(NULL,NULL,"ClockFlyoutWindow",NULL);
	if(hwnd)
		return hwnd;
	hwnd = FindWindowEx(NULL, NULL, "Windows.UI.Core.CoreWindow", "Date and Time Information");
	if(hwnd){ // starts "invisible" full-size and becomes re-sized/moved and "visible" later
		union{
			RECT rc;
			POINT pt;
		} u;
		GetWindowRect(hwnd, &u.rc);
		// IsWindowVisible()/IsWindowEnabled() are always true
		if(WindowFromPoint(u.pt) == hwnd)
			return hwnd;
	}
	return gs_hwndCalendar;
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
		// center small windows within clock dimension when possible
		GetClientRect(gs_hwndClock, &moni.rcWork);
		if(wProp < moni.rcWork.right)
			moni.rcMonitor.left -= ((moni.rcWork.right - wProp)>>1) + api.desktop_button_size;
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

int Clock_GetFileAndOption(const char* command, char* app, char* params) {
	int invalid_path = 0;
	const char* offset_params = NULL;
	const char* offset = command;
	char* out = app;
	
	for(;;) {
		if(*offset <= ' ') { // any non-printable char, incl. null
			*out = '\0';
			if(Clock_PathExists(app) == 1){
				offset_params = offset;
			}else{
				if(offset-command <= MAX_PATH-5){
					memcpy(out, ".exe", 5);
					if(Clock_PathExists(app) == 1)
						offset_params = offset;
					*out = '\0';
				}
			}
			for(; *offset == ' '; *out++ = *offset++);
			if(!*offset)
				break;
		}
		if(*offset == '/'){ // Windows still fails to work with "/" and relative paths
			++offset;
			*out++ = '\\';
			continue;
		}
		*out++ = *offset++;
	}
	if(!offset_params){ // no valid file found
		invalid_path = 1;
		offset_params = strchr(command, ' ');
		if(!offset_params) // entire command
			offset_params = offset;
	}
	out[offset_params-offset] = '\0';
	
	if(params){
		for(; *offset_params == ' ';  ++offset_params);
		strcpy(params, offset_params);
	}
	return invalid_path;
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

int Clock_GetInt(const char* section, const char* entry, int defval) {
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			return GetPrivateProfileInt(key, entry, defval, ms_inifile);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
				int val;
				DWORD regtype, size = sizeof(val);
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&val,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
					defval = val;
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

int64_t Clock_GetInt64(const char* section, const char* entry, int64_t defval) {
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			char s[16+1]; // max: 19+1+1(decimal), 16+1(hexadecimal)
			int ret = GetPrivateProfileString(key, entry, "", s, sizeof(s), ms_inifile);
			if(ret)
				sscanf(s, "%" SCNx64, &defval);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
				int64_t val;
				DWORD regtype, size = sizeof(val);
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&val,&size)==ERROR_SUCCESS && regtype==REG_QWORD)
					defval = val;
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

int Clock_GetIntEx(const char* section, const char* entry, int defval) {
	char key[80];
	HKEY hkey;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			char val[20];
			GetPrivateProfileString(key, entry, kInvalidKey, val, sizeof(val), ms_inifile);
			if(val[0] != kInvalidKey[0] || val[1] != kInvalidKey[1])
				defval = atoi(val);
			else
				Clock_SetInt(section, entry, defval);
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
				int val;
				DWORD regtype, size = sizeof(val);
				if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&val,&size)==ERROR_SUCCESS && regtype==REG_DWORD){
					defval = val;
				}else{
					Clock_SetInt(section,entry,defval);
				}
				RegCloseKey(hkey);
			}
		}
	}
	return defval;
}

int Clock_GetSystemInt(HKEY rootkey, const char* section, const char* entry, int defval) {
	HKEY hkey;
	
	if(RegOpenKeyEx(rootkey,section,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
		int val;
		DWORD regtype, size = sizeof(val);
		if(RegQueryValueEx(hkey,entry,0,&regtype,(BYTE*)&val,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
			defval = val;
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
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
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
			ret = GetPrivateProfileString(key, entry, kInvalidKey, val, len, ms_inifile);
			if(val[0] == kInvalidKey[0] && val[1] == kInvalidKey[1])
				ret = -1;
		} else {
			if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
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
	
	if(RegOpenKeyEx(rootkey,section,0,ms_reg_read,&hkey) == ERROR_SUCCESS) {
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

int Clock_SetInt(const char* section, const char* entry, int val) {
	char key[80];
	HKEY hkey;
	int ret = 0;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			char s[20];
			wsprintf(s, "%d", val);
			return WritePrivateProfileString(key, entry, s, ms_inifile);
		} else {
			if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,ms_reg_fullaccess,NULL,&hkey,NULL) == ERROR_SUCCESS) {
				if(RegSetValueEx(hkey, entry, 0, REG_DWORD, (const BYTE*)&val, sizeof(val)) == ERROR_SUCCESS) {
					ret = 1;
				}
				RegCloseKey(hkey);
			}
		}
	}
	return ret;
}

int Clock_SetInt64(const char* section, const char* entry, int64_t val) {
	char key[80];
	HKEY hkey;
	int ret = 0;
	
	if(PrepareMyRegKey_(key,section)){
		if(ms_bIniSetting) {
			char s[16+1]; // max: 19+1+1(decimal), 16+1(hexadecimal)
			wsprintf(s, "%" PRIx64, val);
			return WritePrivateProfileString(key, entry, s, ms_inifile);
		} else {
			if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,ms_reg_fullaccess,NULL,&hkey,NULL) == ERROR_SUCCESS) {
				if(RegSetValueEx(hkey, entry, 0, REG_DWORD, (const BYTE*)&val, sizeof(val)) == ERROR_SUCCESS) {
					ret = 1;
				}
				RegCloseKey(hkey);
			}
		}
	}
	return ret;
}

int Clock_SetSystemInt(HKEY rootkey, const char* section, const char* entry, int val) {
	HKEY hkey;
	int ret = 0;
	
	if(RegCreateKeyEx(rootkey,section,0,NULL,0,ms_reg_fullaccess,NULL,&hkey,NULL) == ERROR_SUCCESS) {
		if(RegSetValueEx(hkey, entry, 0, REG_DWORD, (const BYTE*)&val, sizeof(val)) == ERROR_SUCCESS) {
			ret = 1;
		}
		RegCloseKey(hkey);
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
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER,key,0,NULL,0,ms_reg_fullaccess,NULL,&hkey,NULL) == ERROR_SUCCESS) {
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
	
	if(RegCreateKeyEx(rootkey,section,0,NULL,0,ms_reg_fullaccess,NULL,&hkey,NULL) == ERROR_SUCCESS) {
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
		
		if(RegOpenKeyEx(HKEY_CURRENT_USER,key,0,ms_reg_fullaccess,&hkey) == ERROR_SUCCESS) {
			if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS)
				ret = 1;
			RegCloseKey(hkey);
		}
	}
	return ret;
}

int Clock_DelSystemValue(HKEY rootkey, const char* section, const char* entry) {
	HKEY hkey;
	int ret = 0;
	if(RegOpenKeyEx(rootkey,section,0,ms_reg_fullaccess,&hkey) == ERROR_SUCCESS) {
		if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS)
			ret = 1;
		RegCloseKey(hkey);
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

// format stuff

char Clock_GetFormat(const char** offset, int* minimum, int* padding) {
	char specifier;
	const char* pos = *offset;
	int pad = 0;
	int min = 0;
	// padding
	for(; *pos=='_'; ++pad,++pos);
	specifier = *pos;
	if(specifier){
		// min / zeros
		do{
			++min; ++pos;
		}while(*pos == specifier);
	}
	//
	*padding = pad;
	*offset = pos;
	*minimum = min;
	return specifier;
}
int Clock_WriteFormatNum(char* buffer, int number, int minimum, int padding) {
	char* out = buffer;
	char negative = '\0';
	int num, nums = 1;
	if(number < 0){
		number = -number;
		negative = '-';
	}
	// count chars && init
	for(num=number; num>=10; ++nums,num/=10);
	minimum -= nums;
	padding += (minimum<0 ? minimum : 0);
	// write padding
	for(; padding>0; --padding)
		*out++ = ' ';
	// minus sign
	if(negative)
		*out++ = negative;
	// write zero padding
	for(; minimum>0; --minimum)
		*out++ = '0';
	// write number
	ltoa(number, out, 10);
	out += nums;
	return (int)(out - buffer);
}
