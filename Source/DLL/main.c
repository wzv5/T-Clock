/*-----------------------------------------------------
// main.c : API, hook procedure -> KAZUBON 1997-2001
//-------------------------------------------------------*/
// Modified by Stoic Joker: Tuesday, March 2 2010 - 10:42:42
#include "tcdll.h"

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
void InitClock(HWND hwnd);

/*------------------------------------------------
  globals
--------------------------------------------------*/
extern WNDPROC oldWndProc;
extern HINSTANCE hInstance;

//================================================================================================
//-------------------------------------+++--> Entry Point of This (SystemTray Clock ShellHook) DLL:
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)   //---------------+++-->
{
	(void)lpvReserved;
	hInstance = hinstDLL;
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	default: break;
	}
	return TRUE;
}
//================================================================================================
//-------------------------------------------------------+++--> Install SystemTray Clock ShellHook:
void WINAPI HookStart(HWND hwnd)   //-------------------------------------------------------+++-->
{
	char classname[80] = {0};
	HWND hwndBar, hwndChild;
	DWORD dwThreadId;
	
	g_hwndTClockMain = hwnd;
	
	// find the taskbar
	hwndBar = FindWindow("Shell_TrayWnd", NULL);
	if(!hwndBar) {
		SendMessage(hwnd, WM_USER+1, 0, 1);
		return;
	}
	
	// get thread ID of taskbar (explorer) - Specal thanks to T.Iwata.
	dwThreadId = GetWindowThreadProcessId(hwndBar, NULL);
	
	if(!dwThreadId) {
		SendMessage(hwnd, WM_USER+1, 0, 2);
		return;
	}
	
	// install an hook to thread of taskbar
	g_hhook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hInstance, dwThreadId);
	if(!g_hhook) {
		SendMessage(hwnd, WM_USER+1, 0, 3);
		return;
	}
	
	// refresh the taskbar
	PostMessage(hwndBar, WM_SIZE, SIZE_RESTORED, 0);
	
	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild) {
		GetClassName(hwndChild, classname, 80);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0) {
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild) {
				GetClassName(hwndChild, classname, 80);
				if(lstrcmpi(classname, "TrayClockWClass") == 0) {
					SendMessage(hwndChild, WM_NULL, 0, 0);
					break;
				}
			} break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}
//================================================================================================
//--------------------------------------------------------+++--> Remove SystemTray Clock ShellHook:
void WINAPI HookEnd(void)   //--------------------------------------------------------------+++-->
{
	HWND hwnd;
	
	// force the clock to end customizing
	if(g_hwndClock && IsWindow(g_hwndClock))
		SendMessage(g_hwndClock, WM_COMMAND, IDM_EXIT, 0);
	// uninstall my hook
	if(g_hhook){
		UnhookWindowsHookEx(g_hhook);
		g_hhook = NULL;
	}
	// refresh the clock
	if(g_hwndClock && IsWindow(g_hwndClock)){
		PostMessage(g_hwndClock, WM_TIMER, 0, 0);
	}
	g_hwndClock = NULL;
	// refresh the taskbar
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd) {
		PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(hwnd, NULL, TRUE);
	}
}
//================================================================================================
//-----------------------------------------------------+++--> SystemTray Clock ShellHook Procedure:
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)   //------------------+++-->
{/// @todo : rewrite hooking code (use wrapper exe to hook it)
	LPCWPSTRUCT pcwps = (LPCWPSTRUCT)lParam;
	if(nCode >= 0 && pcwps && pcwps->hwnd) { // if this message is sent to the clock
		char classname[80];
		if(!g_hwndClock
		   && GetClassName(pcwps->hwnd, classname, 80) > 0
		   && lstrcmpi(classname, "TrayClockWClass") == 0) {
			InitClock(pcwps->hwnd); // initialize  cf. wndproc.c
		}
	}
	return CallNextHookEx(g_hhook, nCode, wParam, lParam);
}
