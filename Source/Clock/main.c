//===============================================================================
//--+++--> main.c - KAZUBON 1997-2001 ============================================
//--+++--> WinMain, window procedure, and functions for initializing ==============
//==================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h" //---------------{ Stoic Joker 2006-2011 }---------------+++-->
#include <winver.h>
#include <wtsapi32.h>
#include <ShlObj.h>//SHGetFolderPath
#include "../common/version.h"
#include "../common/tcolor.h" // WM_DWMCOLORIZATIONCOLORCHANGED

// TCDLL.DLL‚API
IsCalendarOpen_t IsCalendarOpen;
HookStart_t HookStart;
HookEnd_t HookEnd;
HookEnd_t ClockExit;

char	g_mydir[MAX_PATH];

// Application Global Window Handles
HWND	g_hwndTClockMain = NULL;
HWND	g_hwndClock;
HWND	g_hwndSheet;
HWND	g_hDlgTimer;
HWND	g_hDlgStopWatch;
HWND	g_hDlgTimerWatch;

HICON	g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;

/** Make Background of Desktop Icon Text Labels Transparent:
 * (For Windows 2000 Only)
 * UnAvertized EasterEgg Function */
static BOOL m_bTrans2kIcons;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ATOM g_atomTClock = 0; /**< main window class atom */
const char g_szClassName[] = "TClockMainClass"; /**< our main window class name */

/** \brief processes our commandline parameters
 * \param hwndMain clock hwnd of master instance
 * \param cmdline commandline parameters
 * \remarks if hwndMain doesn' match \c g_hwndTClockMain, it'll create a message-only window for sound processing
 * \sa g_hwndTClockMain, WinMain() */
static void ProcessCommandLine(HWND hwndMain,const char* cmdline);
static void OnTimerMain(HWND hwnd);
//static void FindTrayServer(); // Redux: what ever it was supposed to be..
static void InitError(int n);
static int LoadTClockDLL(void);
static void SetDesktopIconTextBk(void);
static UINT s_uTaskbarRestart = 0;
static BOOL bStartTimer = FALSE;
static int nCountFindingClock = -1;
BOOL bMonOffOnLock = FALSE;

// alarm.c
extern char g_bPlayingNonstop;


//=================================================================
//---------------------------+++--> fixes lost-keyboard-control bug:
BOOL EnableDlgItemSafeFocus(HWND hDlg,int control,BOOL bEnable,int nextFocus)
{
	HWND hwnd=GetDlgItem(hDlg,control);
	if(!bEnable && GetFocus()==hwnd){
		if(nextFocus){
			HWND hwndnext=GetDlgItem(hDlg,nextFocus);
			SendMessage(hDlg,WM_NEXTDLGCTL,(WPARAM)hwndnext,TRUE);
		}else
			SendMessage(hDlg,WM_NEXTDLGCTL,0,FALSE);
	}
	return EnableWindow(hwnd,bEnable);
}
//=================================================================
//--------------------------+++--> toggle calendar (close or open):
void ToggleCalendar(int type)   //---------------------------+++-->
{
	if(IsCalendarOpen(1))
		return;
	if(g_tos>=TOS_VISTA && (!GetMyRegLong("Calendar","bCustom",0) && type!=1)){
		SendMessage(g_hwndClock,WM_USER+102,1,0);//1=open, 0=close
		// for multi-monitor support, 11px padding is Win8 default
		SetMyDialgPos(FindWindowEx(NULL,NULL,"ClockFlyoutWindow",NULL),11);
	}else{
		char cal[MAX_PATH];
		strcpy(cal,g_mydir); add_title(cal,"misc\\XPCalendar.exe");
		Exec(cal,NULL,g_hwndTClockMain);
	}
}
//================================================================================================
//------------------------------+++--> UnRegister the Clock For Login Session Change Notifications:
void UnregisterSession(HWND hwnd)   //--------{ Explicitly Linked for Windows 2000 }--------+++-->
{
	HINSTANCE handle = LoadLibrary("wtsapi32"); // Windows 2000 Does Not Have This .dll
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
	HINSTANCE handle = LoadLibrary("wtsapi32"); // Windows 2000 Does Not Have This .dll
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
//====================================================================================
//---------------------------+++--> Does our startup file exist? Also creates filename:
int GetStartupFile(HWND hDlg,char filename[MAX_PATH]){   //--------------------+++-->
	size_t offset;
	if(SHGetFolderPath(hDlg,CSIDL_STARTUP,NULL,SHGFP_TYPE_CURRENT,filename)!=S_OK){
		return 0;
	}
	offset=strlen(filename);
	filename[offset]='\\';
	filename[offset+1]='\0'; // old Stoic Joker link
	strcat(filename,CONF_START_OLD);
	strcat(filename,".lnk");
	if(PathFileExists(filename))
		return 1;
	filename[offset+1]='\0'; // new name
	strcat(filename,CONF_START);
	strcat(filename,".lnk");
	if(PathFileExists(filename))
		return 1;
	return 0;
}
//================================================================================================
//----------------------------------------+++--> Remove Launch T-Clock on Windows Startup ShortCut:
void RemoveStartup(HWND hDlg)   //----------------------------------------------------------+++-->
{
	char path[MAX_PATH];
	if(!GetStartupFile(hDlg,path))
		return;
	DeleteFile(path);
}
//===================================
void AddStartup(HWND hDlg) //--+++-->
{
	char path[MAX_PATH], myexe[MAX_PATH];
	if(GetStartupFile(hDlg,path) || !*path)
		return;
	*strrchr(path,'\\')='\0';
	GetModuleFileName(GetModuleHandle(NULL),myexe,MAX_PATH);
	CreateLink(myexe,path,CONF_START);
}
//==========================
//--+++--> Create Launch T-Clock on Windows Startup ShortCut:
int CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name)
{
	HRESULT hres;
	IShellLink* psl;
	
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	
	hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl);
	if(SUCCEEDED(hres)) {
		IPersistFile* ppf;
		char path[MAX_PATH];
		
		psl->lpVtbl->SetPath(psl, fname);
		psl->lpVtbl->SetDescription(psl, name);
		strncpy_s(path,MAX_PATH,fname,_TRUNCATE);
		del_title(path);
		psl->lpVtbl->SetWorkingDirectory(psl, path);
		
		hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
		if(SUCCEEDED(hres)) {
			WORD wsz[MAX_PATH];
			char lnkfile[MAX_PATH];
			strncpy_s(lnkfile, MAX_PATH, dstpath, _TRUNCATE);
			add_title(lnkfile, (char*)name);
			strcat(lnkfile, ".lnk");
			
			MultiByteToWideChar(CP_ACP, 0, lnkfile, -1, wsz, MAX_PATH);
			
			hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
			ppf->lpVtbl->Release(ppf);
		}
		psl->lpVtbl->Release(psl);
	}
	CoUninitialize();
	
	if(SUCCEEDED(hres))
		return 1;
	return 0;
}

//================================================================================================
//--------------------------------------------------==-+++--> Entry Point of Program Using WinMain:
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	BOOL SessionReged = FALSE; // IS Session Registered to Receive Session Change Notifications?
	WNDCLASS wndclass;
	HWND hwndMain;
	MSG msg;
	int updated;
	typedef DWORD (WINAPI* GetLongPathName_t)(char* lpszShortPath,char* lpszLongPath,DWORD cchBuffer);
	GetLongPathName_t pGetLongPathName=(GetLongPathName_t)GetProcAddress(GetModuleHandle("kernel32"),"GetLongPathNameA");
	
	(void)hPrevInstance;
	(void)nCmdShow;
	
	// get the path where .exe is positioned
	if(pGetLongPathName)
		GetLongPathName(GetClockExe(),g_mydir,MAX_PATH);
	else
		strncpy_s(g_mydir,MAX_PATH,GetClockExe(),_TRUNCATE);
	#ifdef _DEBUG
	OutputDebugString(g_mydir); OutputDebugString("\n");
	#endif
	del_title(g_mydir);
	
	// Make sure we're running Windows 2000 and above
	if(!CheckSystemVersion()) {
		MessageBox(NULL,"T-Clock requires Windows 2000 or newer","old OS",MB_OK|MB_ICONERROR);
		return 1;
	}
	
	// make sure ObjectBar isn't running -> From Original Code/Unclear if This is Still a Conflict. (test suggested not really.. no crash but no clock either :P)
	if(FindWindow("ObjectBar Main","ObjectBar")) {
		MessageBox(NULL,"ObjectBar and T-Clock can't be run together","ObjectBar detected!",MB_OK|MB_ICONERROR);
		return 1;
	}
	
//	FindTrayServer(hwndMain);
	
	// Make sure we're not running 32bit on 64bit OS / start the other one
	#ifndef __x86_64__
	if(IsWow64()){
		hwndMain = FindWindow(g_szClassName, NULL);
		if(hwndMain) { // send commands to existing instance
			ProcessCommandLine(hwndMain,lpCmdLine);
		}else{ // start new instance
			char clock64[MAX_PATH];
			strcpy(clock64,g_mydir); add_title(clock64,"Clock" ARCH_SUFFIX_64 ".exe");
			Exec(clock64,lpCmdLine,NULL);
		}
		return 0;
	}
	#endif // __x86_64__
	
	// Do Not Allow the Program to Execute Twice!
	updated = 25; /**< wait up to 5 sec in 1/5th seconds for other instance */
	do{
		HANDLE processlock=CreateMutex(NULL,FALSE,g_szClassName); // we leak handle here, but Windows closes on process exit anyway (so why do it manually?)
		if(processlock && GetLastError()==ERROR_ALREADY_EXISTS){
			CloseHandle(processlock);
			hwndMain = FindWindow(g_szClassName, NULL);
			if(hwndMain) { // This One Sends Commands to the Instance
				ProcessCommandLine(hwndMain,lpCmdLine); // That is Currently Running.
				return 0;
			}
			Sleep(200);
			continue;
		}
		break;
	}while(updated--);
	
	// Update settings if required and setup defaults
	if((updated=CheckSettings())<0){
		return 1;
	}
	//--------------+++--> This is For Windows 2000 Only - EasterEgg Function:
	m_bTrans2kIcons = GetMyRegLongEx("Desktop", "Transparent2kIconText", FALSE);
	CancelAllTimersOnStartUp();
	if(!LoadTClockDLL())
		return 2;
	
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
	wndclass.hbrBackground = (HBRUSH)(intptr_t)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = g_szClassName;
	g_atomTClock = RegisterClass(&wndclass);
	
	if(g_tos>=TOS_VISTA) { // allow non elevated processes to send control messages (eg, App with admin rights, explorer without)
		#define MSGFLT_ADD 1
		#define MSGFLT_REMOVE 2
		typedef BOOL (WINAPI* ChangeWindowMessageFilter_t)(UINT message,DWORD dwFlag);
		ChangeWindowMessageFilter_t ChangeWindowMessageFilter=(ChangeWindowMessageFilter_t)GetProcAddress(GetModuleHandle("User32"),"ChangeWindowMessageFilter");
		if(ChangeWindowMessageFilter){
			int msgid;
			ChangeWindowMessageFilter(WM_COMMAND,MSGFLT_ADD);
			for(msgid=WM_MOUSEFIRST; msgid<=WM_MOUSELAST; ++msgid)
				ChangeWindowMessageFilter(msgid,MSGFLT_ADD);
			for(msgid=MAINMFIRST; msgid<=MAINMLAST; ++msgid)
				ChangeWindowMessageFilter(msgid,MSGFLT_ADD);
		}
	}
	
	// create a hidden window
	g_hwndTClockMain = hwndMain = CreateWindowEx(WS_EX_NOACTIVATE, MAKEINTATOM(g_atomTClock),NULL, 0, 0,0,0,0, NULL,NULL,hInstance,NULL);
	// This Checks for First Instance Startup Options
	ProcessCommandLine(hwndMain,lpCmdLine);
	
	GetHotKeyInfo(hwndMain);
	
	if(g_tos>TOS_2000) {
		bMonOffOnLock = GetMyRegLongEx("Desktop", "MonOffOnLock", FALSE);
		if(bMonOffOnLock) {
			RegisterSession(hwndMain);
			SessionReged = TRUE;
		}
	}
	if(updated==1){
		PostMessage(hwndMain,WM_COMMAND,IDM_SHOWPROP,0);
	}
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!(g_hwndSheet && IsWindow(g_hwndSheet) && PropSheet_IsDialogMessage(g_hwndSheet,&msg))
		&& !(g_hDlgTimer && IsWindow(g_hDlgTimer) && IsDialogMessage(g_hDlgTimer,&msg))
		&& !(g_hDlgTimerWatch && IsWindow(g_hDlgTimerWatch) && IsDialogMessage(g_hDlgTimerWatch,&msg))
		&& !(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch) && IsDialogStopWatchMessage(g_hDlgStopWatch,&msg))){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	UnregisterHotKey(hwndMain, HOT_TIMER);
	UnregisterHotKey(hwndMain, HOT_WATCH);
	UnregisterHotKey(hwndMain, HOT_STOPW);
	UnregisterHotKey(hwndMain, HOT_PROPR);
	UnregisterHotKey(hwndMain, HOT_CALEN);
	UnregisterHotKey(hwndMain, HOT_TSYNC);
	
	if((SessionReged) || (bMonOffOnLock)) UnregisterSession(hwndMain);
	
	EndNewAPI(NULL);
	
	return (int)msg.wParam;
}

LRESULT CALLBACK MsgOnlyProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	(void)wParam; (void)lParam;
	
	switch(message) {
	case MM_MCINOTIFY: // stop playing or repeat mci file (all but .wav, .pcb)
		OnMCINotify(hwnd);
		return 0;
	case MM_WOM_DONE: // stop playing wave
	case MAINM_STOPSOUND:
		StopFile();
		return 0;
	}
	return 0;
}
//========================================================================================
//	/exit		exit T-Clock 2010
//	/prop		show T-Clock 2010 properties
//	/SyncOpt	SNTP options
//	/Sync		synchronize the system clock with an NTP server
//	/start		start the Stopwatch (open as needed)
//	/stop		stop (pause really) the Stopwatch
//	/reset		reset Stopwatch to 0 (stop as needed)
//	/lap		record a (the current) lap time
//================================================================================================
//---------------------------------------------//---------------+++--> T-Clock Command Line Option:
void ProcessCommandLine(HWND hwndMain,const char* cmdline)   //-----------------------------+++-->
{
	const char* p = cmdline;
	if(g_hwndTClockMain != hwndMain){
		g_hwndTClockMain = CreateWindow("STATIC",NULL,0,0,0,0,0,HWND_MESSAGE_nowarn,0,0,0);
		SubclassWindow(g_hwndTClockMain, MsgOnlyProc);
	}
	
	for(; *p; ++p) {
		if(*p == '/') {
			++p;
			if(_strnicmp(p, "prop", 4) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_SHOWPROP, 0);
				p += 4;
			} else if(_strnicmp(p, "exit", 4) == 0) {
				SendMessage(hwndMain, MAINM_EXIT, 0, 0);
				p += 4;
			} else if(_strnicmp(p, "start", 5) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_START, 0);
				p += 5;
			} else if(_strnicmp(p, "stop", 4) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_STOP, 0);
				p += 4;
			} else if(_strnicmp(p, "reset", 5) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_RESET, 0);
				p += 5;
			} else if(_strnicmp(p, "pause", 5) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_PAUSE, 0);
				p += 5;
			} else if(_strnicmp(p, "resume", 6) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_RESUME, 0);
				p += 6;
			} else if(_strnicmp(p, "lap", 3) == 0) {
				SendMessage(hwndMain, WM_COMMAND, IDM_STOPWATCH_LAP, 0);
				p += 3;
			} else if(_strnicmp(p, "SyncOpt", 7) == 0) {
				NetTimeConfigDialog();
				p += 7;
			} else if(_strnicmp(p, "Sync", 4) == 0) {
				if(g_hwndTClockMain == hwndMain) {
					MessageBox(0,
							   TEXT("T-Clock Must be Running for Time Synchronization to Succeed\n"
									"T-Clock Can Not be Started With the /Sync Switch"),
							   "ERROR: Time Sync Failure", MB_OK|MB_ICONERROR);
					SendMessage(hwndMain, WM_COMMAND, IDM_EXIT, 0);
				} else {
					SyncTimeNow();
					p += 4;
				}
			}
		}
	}
	
	if(g_hwndTClockMain != hwndMain){
		int timeout = 100; /**< timeout in 1/10th seconds to allow sounds up to 10 seconds before terminating */
		MSG msg;
		msg.message = 0;
		while(IsPlaying() && --timeout){
			while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
				if(msg.message==WM_QUIT)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if(msg.message==WM_QUIT)
				break;
			Sleep(100);
		}
		DestroyWindow(g_hwndTClockMain);
		g_hwndTClockMain = NULL;
	}
}
//================================================================================================
//--------------------------------------------------+++--> The Main Application "Window" Procedure:
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam)   //--------+++-->
{
	switch(message) {
	case WM_CREATE:
		InitAlarm();  // initialize alarms
		InitFormat(); // initialize/reset Date/Time format
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
		
	case WM_ENDSESSION:
		if(!wParam)
			break;
		/* fall through */
	case WM_DESTROY:
		if(g_hwndSheet && IsWindow(g_hwndSheet))
			SendMessage(g_hwndSheet, WM_CLOSE, 0, 0);
		if(g_hDlgTimer && IsWindow(g_hDlgTimer))
			SendMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
		if(g_hDlgStopWatch && IsWindow(g_hDlgStopWatch))
			SendMessage(g_hDlgStopWatch, WM_CLOSE, 0, 0);
		g_hwndSheet = g_hDlgTimer = g_hDlgStopWatch = NULL;
		EndAlarm();
		EndAllTimers();
		KillTimer(hwnd, IDTIMER_MAIN);
		if(bStartTimer) {
			KillTimer(hwnd, IDTIMER_START);
			bStartTimer = FALSE;
		}
		ClockExit(); // exit clock, remove injection
		if(message!=WM_ENDSESSION)
			PostQuitMessage(0);
		return 0;
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
		HookEnd(); // injected, now remove hook
		return 0;
		
	case MAINM_ERROR:    // error
		nCountFindingClock = -1;
		InitError((int)lParam);
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return 0;
		
	case MAINM_EXIT:    // exit
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return 0;
		
	case MAINM_BLINKOFF:    // clock no longer blinks
		if(!g_bPlayingNonstop) StopFile();
		return 0;
		
	case MM_MCINOTIFY: // stop playing or repeat mci file (all but .wav, .pcb)
		OnMCINotify(hwnd);
		return 0;
	case MM_WOM_DONE: // stop playing wave
	case MAINM_STOPSOUND:
		StopFile();
		return 0;
		
	case WM_WININICHANGE:
		RefreshUs();
		return 0;
	// inform clock about DWM color change
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		OnTColor_DWMCOLORIZATIONCOLORCHANGED((unsigned)wParam);
		PostMessage(g_hwndClock, WM_DWMCOLORIZATIONCOLORCHANGED, wParam, lParam);
		return 0;
		
	// context menu
	case WM_COMMAND:
		OnTClockCommand(hwnd, LOWORD(wParam)); // menu.c
		return 0;
		
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		if(!g_bPlayingNonstop) PostMessage(hwnd, MAINM_STOPSOUND, 0, 0);
		/* fall through */
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		OnMouseMsg(hwnd, message, wParam, lParam); // mouse.c
		return 0;
		
	case WM_WTSSESSION_CHANGE:
		switch(wParam) {
		case WTS_SESSION_LOCK:
			Sleep(500); // Eliminate user's interaction for 500 ms
			SendMessage(HWND_BROADCAST_nowarn, WM_SYSCOMMAND,SC_MONITORPOWER, 2);
			return 0;
		}
		break;
	default:
		if(message == s_uTaskbarRestart) { // IF the Explorer Shell Crashes,
			SetTimer(hwnd, IDTIMER_START, 1000, NULL); //  and the taskbar is recreated.
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
//=================================================================================================
//----------------------------//--------+++--> Verify the Correct Version of T-Clock.dll is Present:
int CheckDLL(char* fname)   //---------------------------------------------------------------+++-->
{
	DWORD size;
	char szVersion[32];
	int ret = 0;
	
	*szVersion='\0';
	size = GetFileVersionInfoSize(fname, 0);
	if(size > 0) {
		char* pBlock = malloc(size);
		if(GetFileVersionInfo(fname, 0, size, pBlock)) {
			VS_FIXEDFILEINFO* pffi;
			UINT uLen;
			if(VerQueryValue(pBlock, "\\\0", &pffi, &uLen)) {
				if(pffi->dwFileVersionMS == MAKELONG(VER_MINOR,VER_MAJOR)  &&
				   pffi->dwFileVersionLS == MAKELONG(VER_REVISION,VER_BUILD)){
					ret = 1; //--+++--> Correct T-Clock.dll File Version Found!
				} else {
					wsprintf(szVersion, "Version: %hu.%hu.%hu.%hu",
							 HIWORD(pffi->dwFileVersionMS),
							 LOWORD(pffi->dwFileVersionMS),
							 HIWORD(pffi->dwFileVersionLS),
							 LOWORD(pffi->dwFileVersionLS));
				}
			}
		}
		free(pBlock);
	}
	if(!ret) {
		char msg[MAX_PATH+30];
		if(*szVersion){
			strcpy(msg, "Invalid file version: ");
		}else{
			strcpy(msg, "Error loading: ");
		}
		get_title(msg + strlen(msg), fname);
		MyMessageBox(NULL, msg,
					 szVersion, MB_OK, MB_ICONEXCLAMATION);
	}
	return ret;
}
//================================================================================================
//-----------------------//---------------------------+++--> Check the File Version of T-Clock.dll:
int LoadTClockDLL(void)   //----------------------------------------------------------------+++-->
{
	char fname[MAX_PATH];
	strcpy(fname, g_mydir); add_title(fname, "misc\\T-Clock" ARCH_SUFFIX ".dll");
	if(CheckDLL(fname)){
		HMODULE dll=LoadLibrary(fname); // leak / auto-free on process exit
		IsCalendarOpen=(IsCalendarOpen_t)GetProcAddress(dll,"IsCalendarOpen");
		HookStart=(HookStart_t)GetProcAddress(dll,"HookStart");
		HookEnd=(HookEnd_t)GetProcAddress(dll,"HookEnd");
		ClockExit=(ClockExit_t)GetProcAddress(dll,"ClockExit");
		return 1;
	}
	return 0;
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
