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
extern HINSTANCE hInstance;

DLL_EXPORT int WINAPI IsCalendarOpen(int focus)
{
	HWND hwnd = FindWindowEx(NULL,NULL,"ClockFlyoutWindow",NULL);
	if(hwnd){
		if(focus) SetForegroundWindow(hwnd);
		return 1;
	}
	return g_bCalOpen;
}
//========================================================================================
//-----------------------------+++--> Entry Point of This (SystemTray Clock ShellHook) DLL:
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)   //---+++-->
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
HHOOK m_hhook = NULL;
//========================================================================================
//-----------------------------------------------+++--> Install SystemTray Clock ShellHook:
DLL_EXPORT void WINAPI HookStart(HWND hwnd)   //-------------------------------------------+++-->
{
	HWND hwndBar, hwndClock;
	DWORD dwThreadId;
	
	g_hwndTClockMain = hwnd;
	if(g_hwndClock && IsWindow(g_hwndClock) && g_hwndClock==FindClock()){
		PostMessage(g_hwndTClockMain,MAINM_CLOCKINIT,0,(LPARAM)g_hwndClock);
		return; // already hooked / old instance
	}
	g_hwndClock=NULL;
	
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
	m_hhook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hInstance, dwThreadId);
	if(!m_hhook) {
		SendMessage(hwnd, MAINM_ERROR, 0, 3);
		return;
	}
	
	// refresh the taskbar
	PostMessage(hwndBar, WM_SIZE, SIZE_RESTORED, 0);
	
	// send message to trigger our hook
	hwndClock=FindClock();
	SendMessage(hwndClock, WM_NULL, 0, 0);
}
//========================================================================================
//------------------------------------------------+++--> Remove SystemTray Clock ShellHook:
DLL_EXPORT void WINAPI HookEnd(void)   //--------------------------------------------------+++-->
{
	// uninstall my hook
	if(m_hhook){
		UnhookWindowsHookEx(m_hhook);
		m_hhook=NULL;
	}
}
//========================================================================================
//---------------------------------------------+++--> SystemTray Clock ShellHook Procedure:
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)   //----------+++-->
{/// @todo : rewrite hooking code (use wrapper exe to hook it)
	CWPSTRUCT* pcwps = (CWPSTRUCT*)lParam;
	if(nCode >= 0 && pcwps && pcwps->hwnd) { // if this message is sent to the clock
		char classname[80];
		if(!g_hwndClock && GetClassName(pcwps->hwnd, classname, 80) && !lstrcmpi(classname, "TrayClockWClass")) {
			InitClock(pcwps->hwnd); // initialize  cf. wndproc.c
			PostMessage(g_hwndTClockMain,MAINM_CLOCKINIT,0,(LPARAM)pcwps->hwnd);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
//========================================================================================
//-----------------------------------------------------------+++--> Remove Clock injection:
DLL_EXPORT void WINAPI ClockExit()   //----------------------------------------------------+++-->
{
	HookEnd(); // uninstall hook helper if any
	if(g_hwndClock && IsWindow(g_hwndClock)){
		HWND hwnd=FindWindow("Shell_TrayWnd",NULL);
		SendMessage(g_hwndClock,WM_COMMAND,IDM_EXIT,0); // kill our clock
		PostMessage(g_hwndClock,WM_TIMER,0,0); // refresh Windows' clock
		g_hwndClock=NULL;
		// refresh the taskbar
		if(hwnd) {
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}
}
