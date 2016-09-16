typedef void MSVC_WARNING_C4206; // "empty" file
#ifdef WIN2K_COMPAT
#include "win2k_compat.h"
#include "utl.h" // GetParentProcess
#include <errno.h>
#include <psapi.h> // GetProcessImageFileName

errno_t win2k_strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count){
	size_t len_src;
	if(!strDest || !numberOfElements)
		return EINVAL;
	strDest[0] = '\0';
	if(!strSource)
		return EINVAL;
	--numberOfElements;
	
	len_src = strnlen(strSource, count);
	if(len_src > numberOfElements)
		len_src = numberOfElements;
	memcpy(strDest, strSource, len_src);
	strDest[len_src] = '\0';
	return 0;
}
errno_t win2k_wcsncpy_s(wchar_t* strDest, size_t numberOfElements, const wchar_t* strSource, size_t count){
	size_t len_src;
	if(!strDest || !numberOfElements)
		return EINVAL;
	strDest[0] = '\0';
	if(!strSource)
		return EINVAL;
	--numberOfElements;
	
	len_src = wcsnlen(strSource, count);
	if(len_src > numberOfElements)
		len_src = numberOfElements;
	memcpy(strDest, strSource, len_src*sizeof(wchar_t));
	strDest[len_src] = '\0';
	return 0;
}

char* win2k_strtok_s(char* strToken, const char* strDelimit, char** context){
	char* ret;
	char* pos;
	const char* delim;
	if(!strDelimit || !context || (!*context&&!strToken)){
		_set_errno(EINVAL);
		return NULL;
	}
	_set_errno(0);
	if(strToken)
		*context = strToken;
	if(*context[0] == '\0')
		return NULL;
	// token start
	for(pos=*context; *pos; ++pos) {
		for(delim=strDelimit; *delim && *pos!=*delim; ++delim);
		if(!*delim)
			break;
	}
	*context = pos;
	// token end
	for(; *pos; ++pos) {
		for(delim=strDelimit; *delim && *pos!=*delim; ++delim);
		if(*delim)
			break;
	}
	ret = *context;
	*context = (*pos ? pos + 1 : pos);
	*pos = '\0';
	return ret;
}
wchar_t* win2k_wcstok_s(wchar_t* strToken, const wchar_t* strDelimit, wchar_t** context){
	wchar_t* ret;
	wchar_t* pos;
	const wchar_t* delim;
	if(!strDelimit || !context || (!*context&&!strToken)){
		_set_errno(EINVAL);
		return NULL;
	}
	_set_errno(0);
	if(strToken)
		*context = strToken;
	if(*context[0] == '\0')
		return NULL;
	// token start
	for(pos=*context; *pos; ++pos) {
		for(delim=strDelimit; *delim && *pos!=*delim; ++delim);
		if(!*delim)
			break;
	}
	*context = pos;
	// token end
	for(; *pos; ++pos) {
		for(delim=strDelimit; *delim && *pos!=*delim; ++delim);
		if(*delim)
			break;
	}
	ret = *context;
	*context = (*pos ? pos + 1 : pos);
	*pos = '\0';
	return ret;
}

static int subclass_init_ = 0;
typedef BOOL (WINAPI *SetWindowSubclass_ptr)(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static SetWindowSubclass_ptr pSetWindowSubclass_ = NULL;
typedef BOOL (WINAPI *GetWindowSubclass_ptr)(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR* pdwRefData);
static GetWindowSubclass_ptr pGetWindowSubclass_;
typedef BOOL (WINAPI *RemoveWindowSubclass_ptr)(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass);
static RemoveWindowSubclass_ptr pRemoveWindowSubclass_;

DefSubclassProc_ptr win2k_DefSubclassProc;

static BOOL SubclassInit() {
	if(!subclass_init_) {
		HMODULE comctl = GetModuleHandleA("comctl32");
		pSetWindowSubclass_ = (SetWindowSubclass_ptr)GetProcAddress(comctl, (const char*)(intptr_t)410);
		pGetWindowSubclass_ = (GetWindowSubclass_ptr)GetProcAddress(comctl, (const char*)(intptr_t)411);
		pRemoveWindowSubclass_ = (RemoveWindowSubclass_ptr)GetProcAddress(comctl, (const char*)(intptr_t)412);
		win2k_DefSubclassProc = (DefSubclassProc_ptr)GetProcAddress(comctl, (const char*)(intptr_t)413);
		if(!pSetWindowSubclass_ || !pGetWindowSubclass_ || !pRemoveWindowSubclass_ || !win2k_DefSubclassProc)
			return 0;
		subclass_init_ = 1;
	}
	return 1;
}

BOOL win2k_SetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	if(!SubclassInit())
		return 0;
	return pSetWindowSubclass_(hwnd, pfnSubclass, uIdSubclass, dwRefData);
}
BOOL win2k_GetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR* pdwRefData) {
	if(!SubclassInit())
		return 0;
	return pGetWindowSubclass_(hwnd, pfnSubclass, uIdSubclass, pdwRefData);
}
BOOL win2k_RemoveWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass) {
	if(!SubclassInit())
		return 0;
	return pRemoveWindowSubclass_(hwnd, pfnSubclass, uIdSubclass);
}

typedef BOOL (WINAPI *AttachConsole_t)(DWORD dwProcessId);
void OpportunisticConsole() {
	unsigned ppid;
	HMODULE kernel = GetModuleHandleA("kernel32");
	AttachConsole_t pAttachConsole = (AttachConsole_t)GetProcAddress(kernel, "AttachConsole");
	if(pAttachConsole) {
		FreeConsole();
		pAttachConsole(ATTACH_PARENT_PROCESS);
		return;
	}
	// w2k fallback (we could basically live without the code above but might be faster)
	ppid = GetParentProcess(GetCurrentProcessId());
	if(ppid) {
		typedef DWORD (WINAPI *GetProcessImageFileNameW_t)(HANDLE hProcess, wchar_t* lpImageFileName, DWORD  nSize);
		HMODULE psapi = GetModuleHandleA("psapi");
		GetProcessImageFileNameW_t pGetProcessImageFileNameW = (GetProcessImageFileNameW_t)GetProcAddress(psapi, "GetProcessImageFileNameW");
		wchar_t exe[100];
		HANDLE parent = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, 0, ppid);
		
		exe[0] = '\0';
		if(pGetProcessImageFileNameW) {
			pGetProcessImageFileNameW(parent, exe, _countof(exe));
		} else
			GetModuleFileNameExW(parent, NULL, exe, _countof(exe));
		if(wcsstr(exe, L"cmd.exe"))
			return;
	}
	FreeConsole();
}

typedef DWORD (WINAPI *WTSGetActiveConsoleSessionId_t)(void);
typedef BOOL (WINAPI *WTSRegisterSessionNotification_t)(HWND hWnd, DWORD dwFlags);
typedef BOOL (WINAPI *WTSUnRegisterSessionNotification_t)(HWND hWnd);

DWORD win2k_WTSGetActiveConsoleSessionId(void) {
	static WTSGetActiveConsoleSessionId_t WTSGetActiveConsoleSessionId = NULL;
	if(!WTSGetActiveConsoleSessionId) {
		WTSGetActiveConsoleSessionId = (WTSGetActiveConsoleSessionId_t)GetProcAddress(GetModuleHandleA("kernel32"), "WTSGetActiveConsoleSessionId");
		if(!WTSGetActiveConsoleSessionId)
			return 0xFFFFFFFF;
	}
	return WTSGetActiveConsoleSessionId();
}

BOOL win2k_WTSRegisterSessionNotification(HWND hWnd, DWORD dwFlags) {
	WTSRegisterSessionNotification_t WTSRegisterSessionNotification;
	WTSRegisterSessionNotification = (WTSRegisterSessionNotification_t)GetProcAddress(GetModuleHandleA("wtsapi32"), "WTSRegisterSessionNotification");
	if(!WTSRegisterSessionNotification)
		return 0;
	return WTSRegisterSessionNotification(hWnd, dwFlags);
}

BOOL win2k_WTSUnRegisterSessionNotification(HWND hWnd) {
	WTSUnRegisterSessionNotification_t WTSUnRegisterSessionNotification;
	WTSUnRegisterSessionNotification = (WTSUnRegisterSessionNotification_t)GetProcAddress(GetModuleHandleA("wtsapi32"), "WTSUnRegisterSessionNotification");
	if(!WTSUnRegisterSessionNotification)
		return 0;
	return WTSUnRegisterSessionNotification(hWnd);
}

#endif // WIN2K_COMPAT
