#include "tcdll.h"
#include "clock_internal.h" // internal clock api functions

#ifdef __GNUC__
#	define SHARED __attribute__((section(".shared"),shared))
#else
#	define SHARED
#	pragma data_seg(".shared")
#endif
// shared variables must be initialized
SHARED char ms_root[MAX_PATH] = {0}; /**< \sa TClockAPI::root */
SHARED size_t ms_root_len = 0; /**< \sa TClockAPI::root_len */
SHARED REGSAM ms_reg_sam = KEY_ALL_ACCESS | KEY_WOW64_64KEY;
SHARED char ms_bIniSetting = 0;
SHARED char ms_inifile[MAX_PATH] = {0};

SHARED HWND gs_hwndTClockMain = NULL;
SHARED HWND gs_hwndClock = NULL;
SHARED char gs_bCalOpen = 0;
SHARED unsigned short gs_tos = 0;
#ifndef __GNUC__
#	pragma data_seg()
#endif

HHOOK m_hhook = NULL;

static ULONGLONG WINAPI GetTickCount64_Wrapper(){
	return GetTickCount();
}
// main.c
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);

#define ClockAPI(func) Clock_##func,
TClockAPI api = {
	NULL, // hInstance
	0, // OS
	ms_root, // root
	0, // root_len
	// base
	ClockAPI(Inject)
	ClockAPI(InjectFinalize)
	ClockAPI(Exit)
	// misc
	ClockAPI(IsCalendarOpen)
	ClockAPI(Message)
	ClockAPI(PositionWindow)
	NULL, // ClockAPI(GetTickCount)
	ClockAPI(GetFileAndOption)
	ClockAPI(GetColor)
	ClockAPI(On_DWMCOLORIZATIONCOLORCHANGED)
	// registry
	ClockAPI(GetInt)
	ClockAPI(GetIntEx)
	ClockAPI(GetSystemInt)
	ClockAPI(GetStr)
	ClockAPI(GetStrEx)
	ClockAPI(GetSystemStr)
	ClockAPI(SetInt)
	ClockAPI(SetStr)
	ClockAPI(SetSystemStr)
	ClockAPI(DelValue)
	ClockAPI(DelKey)
	// exec
	ClockAPI(ShellExecute)
	ClockAPI(Exec)
	ClockAPI(ExecElevated)
	ClockAPI(ExecFile)
	// translation
//	ClockAPI(T)
//	ClockAPI(Translate)
//	ClockAPI(TranslateWindow)
};


DLL_EXPORT int SetupClockAPI(int version, TClockAPI* _api){
	char own_path[sizeof(ms_root)];
	OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO)};
//	typedef DWORD (WINAPI* GetLongPathName_t)(char* lpszShortPath,char* lpszLongPath,DWORD cchBuffer);
//	GetLongPathName_t pGetLongPathName=(GetLongPathName_t)GetProcAddress(GetModuleHandle("kernel32"),"GetLongPathNameA");
	
	if(version != CLOCK_API)
		return -1;
	
	if(!ms_root_len){ // initialize once. (only use ms_/gs_ variables!!!)
		GetModuleFileName(api.hInstance, own_path, sizeof(own_path));
//		if(pGetLongPathName)
//			pGetLongPathName(own_path,ms_root,MAX_PATH);
//		else
//			strncpy_s(ms_root,MAX_PATH,own_path,_TRUNCATE);
		GetLongPathName(own_path, ms_root, sizeof(ms_root));
		del_title(ms_root); del_title(ms_root);
		ms_root_len = strlen(ms_root);
		DBGOUT("root: %s\n",ms_root);
		
		memcpy(ms_inifile, ms_root, ms_root_len+1);
		strcat(ms_inifile, "\\T-Clock.ini");
		if(PathExists(ms_inifile)){
			ms_bIniSetting = 1;
		}
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724832%28v=vs.85%29.aspx
		if(GetVersionEx(&osvi)){
			switch(osvi.dwMajorVersion){
			case 0: case 1: case 2: case 3: case 4:
				break;
			case 5: // 2000-Vista
				switch(osvi.dwMinorVersion ){
				case 0:
					gs_tos=TOS_2000; break;
				case 1:
					gs_tos=TOS_XP; break;
				default: // 2+
					gs_tos=TOS_XP_64;
				}
				break;
			case 6: // Vista+
				switch(osvi.dwMinorVersion ){
				case 0:
					gs_tos=TOS_VISTA; break;
				case 1:
					gs_tos=TOS_WIN7; break;
				case 2:
					gs_tos=TOS_WIN8; break;
				case 3:
					gs_tos=TOS_WIN8_1; break;
				case 4:
					gs_tos=TOS_WIN10; break;
				default:
					gs_tos=TOS_NEWER;
				}
				break;
			default:
				gs_tos=TOS_NEWER;
			}
		}
		if(gs_tos < TOS_XP_64) // Win2000 fix
			ms_reg_sam &= ~KEY_WOW64_64KEY;
	}
	api.OS = gs_tos;
	api.root_len = ms_root_len;
	if(gs_tos >= TOS_VISTA){
		api.GetTickCount64 = (GetTickCount64_t)GetProcAddress(GetModuleHandle("kernel32"),"GetTickCount64");
		if(!api.GetTickCount64)
			api.GetTickCount64 = GetTickCount64_Wrapper;
	}
	if(_api) // NULL if internally called
		memcpy(_api, &api, sizeof(TClockAPI));
	return 0;
}

void Clock_Inject(HWND hwnd)
{
	HWND hwndBar, hwndClock;
	DWORD dwThreadId;
	
	hwndClock = FindClock();
	gs_hwndTClockMain = hwnd;
	if(gs_hwndClock && IsWindow(gs_hwndClock) && gs_hwndClock==hwndClock){
		PostMessage(gs_hwndTClockMain,MAINM_CLOCKINIT,0,(LPARAM)gs_hwndClock);
		return; // already hooked / old instance
	}
	gs_hwndClock = NULL;
	
	// find the taskbar
	hwndBar = FindWindow("Shell_TrayWnd", NULL);
	if(!hwndBar) {
		SendMessage(hwnd, MAINM_ERROR, 0, 1);
		return;
	}
	
	// get thread ID of taskbar (explorer) - Specal thanks to T.Iwata.
	dwThreadId = GetWindowThreadProcessId(hwndBar, NULL);
	
	if(!dwThreadId) {
		SendMessage(hwnd, MAINM_ERROR, 0, 2);
		return;
	}
	
	// install an hook to thread of taskbar
	m_hhook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, api.hInstance, dwThreadId);
	if(!m_hhook) {
		SendMessage(hwnd, MAINM_ERROR, 0, 3);
		return;
	}
	
	// refresh the taskbar
	PostMessage(hwndBar, WM_SIZE, SIZE_RESTORED, 0);
	
	SendMessage(hwndClock, WM_NULL, 0, 0);
}

void Clock_InjectFinalize()
{
	// uninstall my hook
	if(m_hhook){
		UnhookWindowsHookEx(m_hhook);
		m_hhook=NULL;
	}
}

void Clock_Exit()
{
	Clock_InjectFinalize(); // uninstall hook helper if any
	if(gs_hwndClock && IsWindow(gs_hwndClock)){
		HWND hwnd = FindWindow("Shell_TrayWnd",NULL);
		SendMessage(gs_hwndClock,WM_COMMAND,IDM_EXIT,0); // kill our clock
		PostMessage(gs_hwndClock,WM_TIMER,0,0); // refresh Windows' clock
		gs_hwndClock = NULL;
		// refresh the taskbar
		if(hwnd) {
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}
}
