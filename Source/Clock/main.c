//===============================================================================
//--+++--> main.c - KAZUBON 1997-2001 ============================================
//--+++--> WinMain, window procedure, and functions for initializing ==============
//==================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h" //---------------{ Stoic Joker 2006-2011 }---------------+++-->
#include <winver.h>
#include <wtsapi32.h>
#include "../common/version.h"
#include "../common/tcolor.h" // WM_DWMCOLORIZATIONCOLORCHANGED

// Application Global Window Handles
HWND	g_hwndTClockMain;	// Main Window Anchor for HotKeys Only!
HWND	g_hwndClock;		// Main Clock Window Handle
HWND	g_hwndSheet;		// property sheet window
HWND	g_hDlgTimer;		// Timer Dialog Handle
HWND	g_hDlgStopWatch;	// Stopwatch Dialog Handle
HWND	g_hDlgTimerWatch;	// Timer Watch Dialog Handle

// icons to use frequently
HICON	g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;
char	g_mydir[MAX_PATH]; // path to tclock.exe

// Make Background of Desktop Icon Text Labels Transparent:
BOOL m_bTrans2kIcons; //-------+++--> (For Windows 2000 Only)
//-----------------//+++--> UnAvertized EasterEgg Function:

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

const char g_szClassName[] = "TClockMainClass"; // window class name
const char g_szWindowText[] = "TClock";        // caption of the window

static void CheckCommandLine(HWND hwnd,const char* cmdline,int other);
static void OnTimerMain(HWND hwnd);
//static void FindTrayServer(); // Redux: what ever it was supposed to be..
static void InitError(int n);
static BOOL CheckTCDLL(void);
static BOOL CheckDLL(char* fname);
static void SetDesktopIconTextBk(void);
static UINT s_uTaskbarRestart = 0;
static BOOL bStartTimer = FALSE;
static int nCountFindingClock = -1;
BOOL bMonOffOnLock = FALSE;

// alarm.c
extern char g_bPlayingNonstop;

//=========================================================
//-------------------+++--> toggle calendar (close or open):
void ToggleCalendar()   //---------------------------+++-->
{
	if(IsCalendarOpen())
		return;
	if(g_tos>=TOS_VISTA && !GetMyRegLong("Calendar","bCustom",0)){
		PostMessage(g_hwndClock,WM_USER+102,1,0);//1=open, 0=close
	}else{
		char cal[MAX_PATH];
		strcpy(cal,g_mydir); add_title(cal,"XPCalendar.exe");
		ExecFile(g_hwndTClockMain,cal);
	}
}
//================================================================================================
//------------------------------+++--> UnRegister the Clock For Login Session Change Notifications:
void UnregisterSession(HWND hwnd)   //--------{ Explicitly Linked for Windows 2000 }--------+++-->
{
	HINSTANCE handle = LoadLibrary("Wtsapi32.dll"); // Windows 2000 Does Not Have This .dll
	// ...Or Support This Feature.
	if(handle){
		typedef BOOL (WINAPI *WTSUnRegisterSessionNotification_t)(HWND);
		WTSUnRegisterSessionNotification_t WTSUnRegisterSessionNotification=(WTSUnRegisterSessionNotification_t)GetProcAddress(handle,"WTSUnRegisterSessionNotification");
		if(WTSUnRegisterSessionNotification){
			WTSUnRegisterSessionNotification(hwnd);
			bMonOffOnLock=FALSE;
		}
		FreeLibrary(handle);
	}
}
//================================================================================================
//--------------------------------+++--> Register the Clock For Login Session Change Notifications:
void RegisterSession(HWND hwnd)   //---------{ Explicitly Linked for Windows 2000 }---------+++-->
{
	HINSTANCE handle = LoadLibrary("Wtsapi32.dll"); // Windows 2000 Does Not Have This .dll
	// ...Or Support This Feature.
	if(handle){
		typedef BOOL (WINAPI *WTSRegisterSessionNotification_t)(HWND,DWORD);
		WTSRegisterSessionNotification_t WTSRegisterSessionNotification=(WTSRegisterSessionNotification_t)GetProcAddress(handle,"WTSRegisterSessionNotification");
		if(WTSRegisterSessionNotification) {
			WTSRegisterSessionNotification(hwnd,NOTIFY_FOR_THIS_SESSION);
			bMonOffOnLock=TRUE;
		}
		FreeLibrary(handle);
	}
}

//================================================================================================
//--------------------------------------------------==-+++--> Entry Point of Program Using WinMain:
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	BOOL SessionReged = FALSE; // IS Session Registered to Receive Session Change Notifications?
	WNDCLASS wndclass;
	HWND hwnd;
	MSG msg;
	int updated;
	
	(void)hPrevInstance;
	(void)nCmdShow;
	
	// Make Sure We're Running Windows 2000 or Newer!
	if(!CheckSystemVersion()) {
		MessageBox(NULL,"T-Clock requires Windows 2000 or newer","old OS",MB_OK|MB_ICONERROR);
		ExitProcess(1);
	}
	
	// make sure ObjectBar isn't running -> From Original Code/Unclear if This is Still a Conflict. (test suggested not really.. no crash but no clock either :P)
	if(FindWindow("ObjectBar Main","ObjectBar")) {
		MessageBox(NULL,"ObjectBar and T-Clock can't be run together","ObjectBar detected!",MB_OK|MB_ICONERROR);
		ExitProcess(1);
	}
	
//	FindTrayServer(hwnd);
	// Do Not Allow the Program to Execute Twice!
	for(updated=0; updated<25; ++updated){ // up to 5 sec
		HANDLE processlock=CreateMutex(NULL,FALSE,g_szClassName);
		if(processlock && GetLastError()==ERROR_ALREADY_EXISTS){
			CloseHandle(processlock);
			hwnd=FindWindow(g_szClassName, g_szWindowText);
			if(hwnd) { // This One Sends Commands to the Instance
				CheckCommandLine(hwnd,lpCmdLine,1); // That is Currently Running.
				ExitProcess(0);
			}
			Sleep(200);
			continue;
		}
		break;
	}
	
	// get the path where .exe is positioned
	GetModuleFileName(hInstance, g_mydir, MAX_PATH);
	del_title(g_mydir);
	
	// Update settings if required and setup defaults
	if((updated=CheckSettings())<0){
		ExitProcess(1);
	}
	//--------------+++--> This is For Windows 2000 Only - EasterEgg Function:
	m_bTrans2kIcons = GetMyRegLongEx("Desktop", "Transparent2kIconText", FALSE);
	CancelAllTimersOnStartUp();
	if(!CheckTCDLL()) { ExitProcess(2);}
	
	// Message of the taskbar recreating - Special thanks to Mr.Inuya
	s_uTaskbarRestart = RegisterWindowMessage("TaskbarCreated");
	// Load ALL of the Global Resources
	g_hIconTClock = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	g_hIconPlay = LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	g_hIconStop = LoadImage(hInstance, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	g_hIconDel  = LoadImage(hInstance, MAKEINTRESOURCE(IDI_DEL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	g_hwndSheet = g_hDlgTimer = NULL;
	
	// register a window class
	wndclass.style         = 0;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = g_hIconTClock;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = g_szClassName;
	RegisterClass(&wndclass);
	
	// create a hidden window
	hwnd = CreateWindowEx(WS_EX_NOACTIVATE, g_szClassName, g_szWindowText,
						  0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
						  
	CheckCommandLine(hwnd,lpCmdLine,0); // This Checks for First Instance Startup Options
	g_hwndTClockMain = hwnd; // Main Window Anchor for HotKeys Only!
	
	GetHotKeyInfo(hwnd);
	
	if(g_tos>TOS_2000) {
		bMonOffOnLock = GetMyRegLongEx("Desktop", "MonOffOnLock", FALSE);
		if(bMonOffOnLock) {
			RegisterSession(hwnd);
			SessionReged = TRUE;
		}
	}
	if(updated==1){
		PostMessage(hwnd,WM_COMMAND,IDM_SHOWPROP,0);
	}
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(g_hwndSheet && IsWindow(g_hwndSheet) && PropSheet_IsDialogMessage(g_hwndSheet,&msg)){
			if(g_hwndSheet && !PropSheet_GetCurrentPageHwnd(g_hwndSheet))
				DestroyWindow(g_hwndSheet);
		}else if(!(g_hDlgTimer && IsWindow(g_hDlgTimer) && IsDialogMessage(g_hDlgTimer,&msg)) &&
			!(g_hDlgTimerWatch && IsWindow(g_hDlgTimerWatch) && IsDialogMessage(g_hDlgTimerWatch,&msg)) &&
			!(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch) && IsDialogStopWatchMessage(g_hDlgStopWatch,&msg))
			){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	UnregisterHotKey(hwnd, HOT_TIMER);
	UnregisterHotKey(hwnd, HOT_WATCH);
	UnregisterHotKey(hwnd, HOT_STOPW);
	UnregisterHotKey(hwnd, HOT_PROPR);
	UnregisterHotKey(hwnd, HOT_CALEN);
	UnregisterHotKey(hwnd, HOT_TSYNC);
	
	if((SessionReged) || (bMonOffOnLock)) UnregisterSession(hwnd);
	
	EndNewAPI(NULL);
	
	ExitProcess((UINT)msg.wParam);
}
//========================================================================================
//   /exit	: Exit T-Clock 2010
//   /prop	: Show T-Clock 2010 Properties
//   /Sync    : Synchronize the System Clock With an NTP Server
//   /start	: Start the Stopwatch Counter (open as needed)
//   /stop	: Stop (pause really) the Stopwatch Counter
//   /reset	: Reset Stopwatch to 0 (stop as needed)
//   /lap		: Record a (the current) Lap Time
//================================================================================================
//---------------------------------------------//---------------+++--> T-Clock Command Line Option:
void CheckCommandLine(HWND hwnd,const char* cmdline,int other)   //--------------------------------------------+++-->
{
	const char* p=cmdline;
	for(; *p; ++p) {
		if(*p == '/') {
			++p;
			if(_strnicmp(p, "prop", 4) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_SHOWPROP, 0);
				p += 4;
			} else if(_strnicmp(p, "exit", 4) == 0) {
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				p += 4;
			} else if(_strnicmp(p, "start", 5) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_START, 0);
				p += 5;
			} else if(_strnicmp(p, "stop", 4) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_STOP, 0);
				p += 4;
			} else if(_strnicmp(p, "reset", 5) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_RESET, 0);
				p += 5;
			} else if(_strnicmp(p, "pause", 5) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_PAUSE, 0);
				p += 5;
			} else if(_strnicmp(p, "resume", 6) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_RESUME, 0);
				p += 6;
			} else if(_strnicmp(p, "lap", 3) == 0) {
				SendMessage(hwnd, WM_COMMAND, IDM_STOPWATCH_LAP, 0);
				p += 3;
			} else if(_strnicmp(p, "SyncOpt", 7) == 0) {
				NetTimeConfigDialog();
				p += 7;
			} else if(_strnicmp(p, "Sync", 4) == 0) {
				if(!other) {
					MessageBox(0,
							   TEXT("T-Clock Must be Running for Time Synchronization to Succeed\n"
									"T-Clock Can Not be Started With the /Sync Switch"),
							   "ERROR: Time Sync Failure", MB_OK|MB_ICONERROR);
					SendMessage(hwnd, WM_COMMAND, IDM_EXIT, 0);
				} else {
					SyncTimeNow();
					p += 4;
				}
			}
		}
	}
}
//================================================================================================
//--------------------------------------------------+++--> The Main Application "Window" Procedure:
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam)   //--------+++-->
{
	switch(message) {
	case WM_CREATE:
		InitAlarm();  // initialize alarms
		InitFormat(); // initialize a Date/Time format
		SendMessage(hwnd, WM_TIMER, IDTIMER_START, 0);
		SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);
		return 0;
		
	case WM_TIMER:
		if(wParam == IDTIMER_START) {
			if(bStartTimer) KillTimer(hwnd, wParam);
			bStartTimer = FALSE;
			HookStart(hwnd); // install a hook
			nCountFindingClock = 0;
			#ifndef _DEBUG
			EmptyWorkingSet(GetCurrentProcess());
			#endif
		} else if(wParam == IDTIMER_MAIN) OnTimerMain(hwnd);
		else if(wParam == IDTIMER_MOUSE) OnTimerMouse(hwnd);
		return 0;
		
	case WM_DESTROY:
		EndAlarm();
		EndAllTimers();
		KillTimer(hwnd, IDTIMER_MAIN);
		if(bStartTimer) {
			KillTimer(hwnd, IDTIMER_START);
			bStartTimer = FALSE;
		} else {
			HookEnd();  // uninstall a hook
		}
		PostQuitMessage(0);
		return 0;
		
	case WM_ENDSESSION:
		if(wParam) {
			EndAlarm();
			EndAllTimers();
			if(bStartTimer) {
				KillTimer(hwnd, IDTIMER_START);
				bStartTimer = FALSE;
			} else {
				HookEnd();  // uninstall a hook
			}
		} break;
		
	case WM_PAINT: {
//			HDC hdc;
//			PAINTSTRUCT ps;
//			hdc = BeginPaint(hwnd, &ps);
//			EndPaint(hwnd, &ps);
			return 0;}
	case WM_HOTKEY: // Feature Requested From eweoma at DonationCoder.com
		switch(wParam) { // And a Damn Fine Request it Was... :-)
		case HOT_WATCH:
			PostMessage(hwnd, WM_COMMAND, IDM_TIMEWATCH, 0);
			return 0;
			
		case HOT_TIMER:
			PostMessage(hwnd, WM_COMMAND, IDM_TIMER, 0);
			return 0;
			
		case HOT_STOPW:
			PostMessage(hwnd, WM_COMMAND, IDM_STOPWATCH, 0);
			return 0;
			
		case HOT_PROPR:
			PostMessage(hwnd, WM_COMMAND, IDM_SHOWPROP, 0);
			return 0;
			
		case HOT_CALEN:
			PostMessage(hwnd, WM_COMMAND, IDM_SHOWCALENDER, 0);
			return 0;
			
		case HOT_TSYNC:
			SyncTimeNow();
			return 0;
			
		} return 0;
		
		//==================================================
	case MAINM_CLOCKINIT: // Messages sent/posted from TCDLL.dll
		nCountFindingClock = -1;
		g_hwndClock = (HWND)lParam;
		return 0;
		
	case MAINM_ERROR:    // error
		nCountFindingClock = -1;
		InitError((int)lParam);
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return 0;
		
	case MAINM_EXIT:    // exit
		if(g_hwndSheet && IsWindow(g_hwndSheet))
			PostMessage(g_hwndSheet, WM_CLOSE, 0, 0);
		if(g_hDlgTimer && IsWindow(g_hDlgTimer))
			PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
		if(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch))
			PostMessage(g_hDlgStopWatch, WM_CLOSE, 0, 0);
		g_hwndSheet = g_hDlgTimer = g_hDlgStopWatch = NULL;
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return 0;
		
	case MAINM_BLINKOFF:    // clock no longer blinks
		if(!g_bPlayingNonstop) StopFile();
		return 0;
		
	case MM_MCINOTIFY:
		OnMCINotify(hwnd);
		return 0;
		
	case MM_WOM_DONE: // stop playing wave
	case MAINM_STOPSOUND:
		StopFile();
		return 0;
		
	case WM_WININICHANGE: {
			HWND hwndBar;
			HWND hwndChild;
			char classname[80];
			
			hwndBar = FindWindow("Shell_TrayWnd", NULL);
			
			// find the clock window
			hwndChild = GetWindow(hwndBar, GW_CHILD);
			while(hwndChild) {
				GetClassName(hwndChild, classname, 80);
				if(lstrcmpi(classname, "TrayNotifyWnd") == 0) {
					hwndChild = GetWindow(hwndChild, GW_CHILD);
					while(hwndChild) {
						GetClassName(hwndChild, classname, 80);
						if(lstrcmpi(classname, "TrayClockWClass") == 0) {
							SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
							break;
						}
					}
					break;
				}
				hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
			}
			return 0;
		}
		// inform clock about DWM color change
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		OnTColor_DWMCOLORIZATIONCOLORCHANGED((unsigned)wParam);
		PostMessage(g_hwndClock, WM_DWMCOLORIZATIONCOLORCHANGED, wParam, lParam);
		return 0;
		
		// context menu
	case WM_COMMAND:
		OnTClockCommand(hwnd, LOWORD(wParam)); // menu.c
		return 0;
		
		// messages transfered from the dll
	case WM_CONTEXTMENU: //-------------------------------------- menu.c
		OnContextMenu(hwnd, LOWORD(lParam), HIWORD(lParam));
		return 0;
		
	case WM_DROPFILES: //------ mouse.c
		OnDropFiles(hwnd, (HDROP)wParam);
		return 0;
		
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		if(!g_bPlayingNonstop) PostMessage(hwnd, MAINM_STOPSOUND, 0, 0);
	case WM_LBUTTONUP: // <^ Code is Designed to "Fall Through" Here.
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		OnMouseMsg(hwnd, message, wParam, lParam); // mouse.c
		return 0;
		
	case WM_WTSSESSION_CHANGE:
		switch(wParam) {
		case WTS_SESSION_LOCK:
			Sleep(500); // Eliminate user's interaction for 500 ms
			SendMessage(HWND_BROADCAST, WM_SYSCOMMAND,SC_MONITORPOWER, (LPARAM)2);
			return 0;
		}
		break;
	default:
		if(message == s_uTaskbarRestart) { // IF the Explorer Shell Crashes,
			HookEnd();                     //  and the taskbar is recreated.
			SetTimer(hwnd, IDTIMER_START, 1000, NULL);
			bStartTimer = TRUE;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*---------------------------------------------------------
-- show a message when TClock failed to customize the clock
---------------------------------------------------------*/
void InitError(int n)
{
	char s[160];
	
	wsprintf(s, "%s: %d", MyString(IDS_NOTFOUNDCLOCK), n);
	MyMessageBox(NULL, s, "Error", MB_OK, MB_ICONEXCLAMATION);
}
/*---------------------------------------------------------
---- Main Timer -------------------------------------------
---- synchronize, alarm, timer, execute Desktop Calendar...
---------------------------------------------------------*/
static int hourLast = -1, minuteLast = -1;
static int daySaved = -1;
//================================================================================================
//-----------------------------------+++--> Values Above Are Required by Main Timer Function Below:
void OnTimerMain(HWND hwnd)   //------------------------------------------------------------+++-->
{
	SYSTEMTIME st;
	BOOL b = TRUE;
	
	GetLocalTime(&st); // Allow OnTimerAlarm(...) to Fire once Every 60 Seconds
	if(hourLast == (int)st.wHour && minuteLast == (int)st.wMinute) b = FALSE;
	
	hourLast = st.wHour;
	minuteLast = st.wMinute;
	if(daySaved >= 0 && st.wDay != daySaved) ;
	else daySaved = st.wDay;
	
	if(b) OnTimerAlarm(hwnd, &st); // alarm.c
	OnTimerTimer(hwnd); // timer.c
	if(b) SetDesktopIconTextBk();
	
	// the clock window exists ?
	if(0 <= nCountFindingClock && nCountFindingClock < 20) nCountFindingClock++;
	else if(nCountFindingClock == 20) nCountFindingClock++;
}
//================================================================================================
//-----------------------//----------------------------+++--> Check the File Version of tClock.dll:
BOOL CheckTCDLL(void)   //------------------------------------------------------------------+++-->
{
	char fname[MAX_PATH];
	strcpy(fname, g_mydir); add_title(fname, "tClock.dll");
	return CheckDLL(fname);
}
//================================================================================================
//----------------------------//--------+++--> Verify the Correct Version of tClock.dll is Present:
BOOL CheckDLL(char* fname)   //-------------------------------------------------------------+++-->
{
	DWORD size;
	char szVersion[32] = {0};
	BOOL br = FALSE;
	
	size = GetFileVersionInfoSize(fname, 0);
	if(size > 0) {
		char* pBlock = malloc(size);
		if(GetFileVersionInfo(fname, 0, size, pBlock)) {
			VS_FIXEDFILEINFO* pffi;
			UINT uLen;
			if(VerQueryValue(pBlock, "\\\0", (LPVOID*)&pffi, &uLen)) {
				if(HIWORD(pffi->dwFileVersionMS) == VER_MAJOR &&
				   LOWORD(pffi->dwFileVersionMS) == VER_MINOR &&
				   HIWORD(pffi->dwFileVersionLS) == VER_BUILD &&
				   LOWORD(pffi->dwFileVersionLS) == VER_REVISION) {
					br = TRUE; //--+++--> Correct tClock.dll File Version Found!
				} else {
					wsprintf(szVersion, "Version: %d.%d.%d.%d",
							 HIWORD(pffi->dwFileVersionMS),
							 LOWORD(pffi->dwFileVersionMS),
							 HIWORD(pffi->dwFileVersionLS),
							 LOWORD(pffi->dwFileVersionLS));
				}
			}
		}
		free(pBlock);
	}
	if(!br) {
		char msg[MAX_PATH+30];
		strcpy(msg, "Invalid file version: ");
		get_title(msg + strlen(msg), fname);
		MyMessageBox(NULL, msg,
					 szVersion, MB_OK, MB_ICONEXCLAMATION);
	}
	return br;
}
//================================================================================================
//----------+++--> Make Background of Desktop Icon Text Labels Transparent (For Windows 2000 Only):
void SetDesktopIconTextBk(void)   //--------------------------------------------------------+++-->
{
	COLORREF col;
	HWND hwnd;
	
	if(m_bTrans2kIcons) {
		hwnd = FindWindow("Progman", "Program Manager");
		if(!hwnd) return;
		hwnd = GetWindow(hwnd, GW_CHILD);
		hwnd = GetWindow(hwnd, GW_CHILD);
		while(hwnd) {
			char s[80];
			GetClassName(hwnd, s, 80);
			if(lstrcmpi(s, "SysListView32") == 0) break;
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
		if(!hwnd) return;
	} else return;
	
	if(m_bTrans2kIcons) {
		if(ListView_GetTextBkColor(hwnd) == CLR_NONE) return;
		col = CLR_NONE;
	} else {
		if(ListView_GetTextBkColor(hwnd) != CLR_NONE) return;
		col = GetSysColor(COLOR_DESKTOP);
	}
	
	ListView_SetTextBkColor(hwnd, col);
	ListView_RedrawItems(hwnd, 0, ListView_GetItemCount(hwnd));
	
	hwnd = GetParent(hwnd);
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd) {
		InvalidateRect(hwnd, NULL, TRUE);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}
//================================================================================================
//-----------------------+++--> Go Find the Default Windows Clock Window - So We Can Assimilate it: (was this at anypoint from Windows itself?)
//void FindTrayServer()   //---------------------------------------------------------+++-->
//{ // Redux: dunno what this is.. not needed
//	HWND hwndTrayServer = FindWindow("Shell_TrayWnd", "CTrayServer");
//	if(hwndTrayServer > 0) SendMessage(hwndTrayServer, WM_CLOSE, 0, 0);
//}
