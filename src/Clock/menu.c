//================================================================================
//--+++--> menu.c - pop-up menu on right button click - KAZUBON 1997-2001 =========
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->
#include <shlobj.h>//IShellDispatch4

void UpdateAlarmMenu(HMENU hMenu);
static char g_undo=0; // did we change windows and can undo it?

/*-----------------------------------------------------------------
 ----------------  when the clock is right-clicked show pop-up menu
-----------------------------------------------------------------*/
void OnContextMenu(HWND hwnd, HWND hwnd_from, int xPos, int yPos)
{
	BOOL g_bQMAudio;
	BOOL g_bQMNet;
	BOOL g_bQMLaunch;
	BOOL g_bQMExitWin;
	BOOL g_bQMDisplay;
	HMENU hPopupMenu;
	HMENU hMenu;
	int item;
	HBITMAP shield_bmp = NULL;
	
	g_bQMAudio   = api.GetInt(L"QuickyMenu", L"AudioProperties",   TRUE);
	g_bQMNet     = api.GetInt(L"QuickyMenu", L"NetworkDrives",     TRUE);
	g_bQMLaunch  = api.GetInt(L"QuickyMenu", L"QuickyMenu",        TRUE);
	g_bQMExitWin = api.GetInt(L"QuickyMenu", L"ExitWindows",       TRUE);
	g_bQMDisplay = api.GetInt(L"QuickyMenu", L"DisplayProperties", TRUE);
	
	hMenu = LoadMenu(g_instance, MAKEINTRESOURCE(IDR_MENU));
	hPopupMenu = GetSubMenu(hMenu, 0);
	
	if(!g_bQMAudio)		DeleteMenu(hPopupMenu, IDC_SOUNDAUDIO,	MF_BYCOMMAND);
	if(!g_bQMNet)		DeleteMenu(hPopupMenu, IDC_NETWORK,		MF_BYCOMMAND);
	if(!g_bQMLaunch)	DeleteMenu(hPopupMenu, IDC_QUICKYS,		MF_BYCOMMAND);
	if(!g_bQMExitWin)	DeleteMenu(hPopupMenu, IDC_EXITWIN,		MF_BYCOMMAND);
	if(!g_bQMDisplay)	DeleteMenu(hPopupMenu, IDM_DISPLAYPROP,	MF_BYCOMMAND);
	// simple implementation of "Undo ..." (eg. Undo Cascade windows)
	if(!g_undo)			DeleteMenu(hPopupMenu, IDM_FWD_UNDO,	MF_BYCOMMAND);
	// special menu items, only shown if SHIFT or CTRL was pressed
	if(!((GetAsyncKeyState(VK_SHIFT)|GetAsyncKeyState(VK_CONTROL))&0x8000)){
		DeleteMenu(hPopupMenu, IDS_NONE, MF_BYCOMMAND); // seperator
		DeleteMenu(hPopupMenu, IDM_FWD_RUNAPP, MF_BYCOMMAND);
		DeleteMenu(hPopupMenu, IDM_RESTART_EXPLORER, MF_BYCOMMAND);
		DeleteMenu(hPopupMenu, IDM_EXIT_EXPLORER, MF_BYCOMMAND);
	}
	// AlarmsTimer Menu Item y/n Goes HERE!!!
	UpdateAlarmMenu(hPopupMenu);
	UpdateTimerMenu(hPopupMenu); // Get the List of Active Timers.
	
	if(g_bQMLaunch) {
		wchar_t key[TNY_BUFF];
		int offset = 9;
		wchar_t name[TNY_BUFF];
		int idx;
		
		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.dwTypeData=name;
		memcpy(key, L"MenuItem-", offset*sizeof(wchar_t));
		for(idx=0; idx<12; ++idx) {
			offset = 9 + wsprintf(key+9, FMT("%i"), idx);
			if(api.GetInt(L"QuickyMenu\\MenuItems",key,0)){
				memcpy(key+offset, L"-Text", 6*sizeof(wchar_t));
				api.GetStr(L"QuickyMenu\\MenuItems", key, name, _countof(name), L"");
				mii.wID = IDM_I_MENU+idx;
				InsertMenuItem(hPopupMenu, IDM_SHOWCALENDER, FALSE, &mii);
			}
		}
	
		if(!HaveSetTimePermissions()) {
			HICON shield = GetStockIcon(SIID_SHIELD, SHGSI_SMALLICON);
			shield_bmp = GetBitmapFromIcon(shield, -2);
			DestroyIcon(shield);
			SetMenuItemBitmaps(hPopupMenu, IDM_SNTP_SYNC, MF_BYCOMMAND, shield_bmp, NULL);
		}
	}
	
	// TrackPopupMenu() quirks: http://support.microsoft.com/kb/135788
	SetForegroundWindow(hwnd);
	if(xPos == -1 && yPos == -1) {
		POINT pt; GetCursorPos(&pt);
		xPos = pt.x, yPos = pt.y;
	}
	item = TrackPopupMenu(hPopupMenu, (TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD), xPos, yPos, 0, hwnd, NULL);
	PostMessage(hwnd, WM_NULL, 0, 0);
	SetForegroundWindow(hwnd_from); // mainly for Keyboard use with `Win+B`; doesn't seem to bug below window creation
	if(item) {
		if(item >= IDM_I_BEGIN_) {
			if(item >= IDM_I_TIMER && item < (IDM_I_TIMER+1000)){
				TimerMenuItemClick(hMenu, item);
			}else if(item >= IDM_I_ALARM && item < (IDM_I_ALARM+1000)){
				AlarmEnable(item - IDM_I_ALARM, -1);
			}else if(item >= IDM_I_MENU && item < (IDM_I_MENU+1000)){
				wchar_t key[MAX_PATH];
				int offset = 9;
				wchar_t szQM_Target[MAX_PATH];
				wchar_t szQM_Switch[MAX_PATH];
				wcscpy(key, L"MenuItem-");
				offset += wsprintf(key+offset, FMT("%i"), (item-IDM_I_MENU));
				wcscpy(key+offset, L"-Target");
				api.GetStr(L"QuickyMenu\\MenuItems", key, szQM_Target, _countof(szQM_Target), L"");
				wcscpy(key+offset, L"-Switches");
				api.GetStr(L"QuickyMenu\\MenuItems", key, szQM_Switch, _countof(szQM_Switch), L"");
				api.Exec(szQM_Target, szQM_Switch, hwnd);
			}
		} else
			OnTClockCommand(hwnd, item);
	}
	DestroyMenu(hMenu); // Starting Over is Simpler & Recommended
	if(shield_bmp)
		DeleteBitmap(shield_bmp);
}
//================================================================================================
//--------------------------------------+++--> Show/Hide Desktop (e.g. Show/Hide all Open Windows):
void ToggleDesk()   //----------------------------------------------------------------------+++-->
{
	IShellDispatch4* pDisp;
	HRESULT hres;
	
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	
	hres = CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &IID_IShellDispatch4, (void**)&pDisp);
	
	if(SUCCEEDED(hres)) {
		pDisp->lpVtbl->ToggleDesktop(pDisp);
		pDisp->lpVtbl->Release(pDisp);
	}
	CoUninitialize();
}
//================================================================================================
//-----------------------------------------------------+++--> T-Clock Menu Command Message Handler:
LRESULT OnTClockCommand(HWND hwnd, WPARAM wParam)   //----------------------------------+++-->
{
	WORD wID = LOWORD(wParam);
	switch(wID) {
	case IDM_REFRESHTCLOCK:
		RefreshUs();
		break;
		
	case IDM_SHOWPROP:
		MyPropertySheet(-1);
		break;
	case IDM_PROP_ALARM:
		MyPropertySheet(1);
		break;
		
	case IDM_EXIT:
		SendMessage(hwnd,WM_CLOSE,0,0);
		break;
		
	case IDM_SHOWCALENDER:
		ToggleCalendar(1); // 1=own calendar
		break;
		
	case IDM_DISPLAYPROP: // https://msdn.microsoft.com/en-us/library/windows/desktop/cc144191(v=vs.85).aspx
		if(api.OS >= TOS_WIN7 && api.OS < TOS_WIN10_2)
			api.Exec(L"::{26EE0668-A00A-44D7-9371-BEB064C98683}\\1\\::{C555438B-3C23-4769-A71F-B6D3D9B6053A}", NULL, NULL);
		else
			api.Exec(L"control", L"desk.cpl,Settings,@Settings", NULL); // XP: @Themes,@Desktop,@ScreenSaver,@Appearance,@Settings (2k had more)
		break;
	case IDM_VOLUMECONTROL: //-------------------------------+++--> Volume Controls
		#ifndef _WIN64
		#	define OPEN_VOLUME L"SndVol32"
		#else
		#	define OPEN_VOLUME L"SndVol"
		#endif // _WIN64
		api.Exec(OPEN_VOLUME, NULL, NULL);
		break;
		
	case IDM_AUDIOPROP: //----------------------------------+++--> Audio settings / devices
		api.Exec(L"control", L"mmsys.cpl", NULL);
		break;
		
	case IDM_RECYCLEBIN:
		api.Exec(L"::{645FF040-5081-101B-9F08-00AA002F954E}", NULL, NULL);
		break;
		
	case IDM_RECYCLEBIN_PURGE:{
		SHQUERYRBINFO info = {sizeof(info)}; // Windows seriously asks :
		SHQueryRecycleBin(NULL, &info); // "are you sure to delete all items"
		if(info.i64NumItems > 0 || api.OS == TOS_2000) // when the recycle bin is actually empty...
			SHEmptyRecycleBin(g_hwndTClockMain, NULL, 0);
		break;}
		
	case IDM_MAPDRIVE: //----------------------------------+++--> Map Network Drive
		WNetConnectionDialog(hwnd, RESOURCETYPE_DISK);
		break;
		
	case IDM_DISCONNECT: //-------------------------+++--> Disconnect Network Drive
		WNetDisconnectDialog(hwnd, RESOURCETYPE_DISK);
		break;
		
	case IDM_TOGGLE_DT: //---------------------------+++--> Show / Hide the Desktop
		ToggleDesk();
		break;
		
	case IDM_QUICKY_WINEXP: { //-----------------//--+++--> Windows Explorer Opened
		api.Exec(L"explorer", L"/e, ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", hwnd);
		break;}
		
	case IDM_QUICKY_DOS: { // Command Prompt
		api.Exec(L"cmd", L"/f:on /t:0a", hwnd);
		break;}
		
	case IDM_QUICKY_EMPTYRB:
		SHEmptyRecycleBin(0, NULL, SHERB_NOCONFIRMATION);
		break;
		
	case IDM_LOCK:
		LockWorkStation();
		break;
	case IDM_LOGOFF:
		WindowsExit(EWX_LOGOFF);
		break;
	case IDM_SLEEP:
		if(!WindowsSleep(1))
			MessageBox(0, L"Sleep request failed!", L"Error", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
		break;
	case IDM_HIBERNATE:
		if(!WindowsSleep(0))
			MessageBox(0, L"Sleep request failed!", L"Error", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
		break;
	case IDM_SHUTDOWN:
		if(!WindowsExit(EWX_SHUTDOWN))
			MessageBox(0, L"Shutdown request failed!", L"Error", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
		break;
	case IDM_REBOOT:
		if(!WindowsExit(EWX_REBOOT))
			MessageBox(0, L"Shutdown request failed!", L"Error", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
		break;
	
	case IDM_EXIT_EXPLORER: // Windows 10 also supports IDM_FWD_EXITEXPLORER
	case IDM_RESTART_EXPLORER:{
		HANDLE process;
		DWORD processid;
		HWND hwndTray = FindWindowA("Shell_TrayWnd", NULL);
		DebugLog(1, "exiting explorer...");
		g_explorer_restarts = 0;
		if(hwndTray) {
			GetWindowThreadProcessId(hwndTray, &processid);
			if(api.OS >= TOS_VISTA) {
				process = OpenProcess(SYNCHRONIZE, FALSE, processid);
				PostMessage(hwndTray, WM_USER + 436, 0, 0);
				if(wID == IDM_RESTART_EXPLORER) {
					if(WaitForSingleObject(process, 15000) == WAIT_TIMEOUT) {
						CloseHandle(process);
						process = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processid);
						if(process) {
							TerminateProcess(process, 1); // exit code needs to be 1, otherwise "winlogon" restarts Explorer
							WaitForSingleObject(process, INFINITE);
						}
					}
				}
			} else {
				DWORD timeout;
				HWND current, last = hwndTray;
				char class[18]; // DimmedWindowClass / FakeDesktopWClass
				SetForegroundWindow(hwndTray); // the "Exit Windows" screen has to receive input focus for GetForegroundWindow() to work
				process = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processid);
				PostMessage(hwndTray, WM_COMMAND, 506, 0); // shutdown dlg (saves icon positions whatsoever)
				timeout = 5000 + GetTickCount();
				for(;;) {
					current = GetForegroundWindow();
					if(current != last) {
						last = current;
						class[0] = '\0';
						GetClassNameA(current, class, _countof(class));
						current = GetParent(current);
						if(!strcasecmp(class,"#32770") && current) {
							GetClassNameA(current, class, _countof(class));
							if(!strcasecmp(class,"DimmedWindowClass"/*XP*/) || !strcasecmp(class,"FakeDesktopWClass"/*2k*/)) {
								Sleep(50); // should be fine without, but better safe than sorry ;)
								break;
							}
						}
					}
					if(GetTickCount() >= timeout)
						break;
					Sleep(50);
				}
				TerminateProcess(process, 1); // exit code needs to be 1, otherwise "winlogon" restarts Explorer
				WaitForSingleObject(process, INFINITE);
			}
			CloseHandle(process);
		}
		if(wID == IDM_RESTART_EXPLORER) {
			DebugLog(0, "restarting explorer...");
			api.Exec(L"explorer", NULL, NULL);
		}
		DebugLog(-1, "done");
		break;}
	case IDM_FWD_CASCADE: case IDM_FWD_SIDEBYSIDE: case IDM_FWD_STACKED: case IDM_FWD_SHOWDESKTOP: case IDM_FWD_MINALL: case IDM_FWD_UNDO:
		g_undo=(wID!=IDM_FWD_UNDO);
		/* fall through */
	case IDM_FWD_DATETIME: case IDM_FWD_CUSTOMNOTIFYICONS:
	case IDM_FWD_TASKMAN:
	case IDM_FWD_LOCKTASKBAR: case IDM_FWD_LOCKALLTASKBAR:
	case IDM_FWD_TASKBARPROP: case IDM_FWD_RUNAPP: case IDM_FWD_EXITEXPLORER:{
		HWND hwndTray = FindWindowA("Shell_TrayWnd", NULL);
		if(hwndTray) PostMessage(hwndTray, WM_COMMAND, wID, 0);
		break;}
	case IDM_DATETIME_EX:{
		HWND hwnd1, hwnd2;
		int wait = 40;
		api.Exec(L"timedate.cpl", L"", 0);
		while((hwnd2=FindWindowA((char*)(uintptr_t)32770,"Date and Time"))==0 && wait--) Sleep(50);
		if(hwnd2){
			SetActiveWindow(hwnd2);
			wait = 10;
			while((hwnd1=FindWindowExA(hwnd2,NULL,(char*)(uintptr_t)32770,"Date and Time"))==0 && wait--) Sleep(50);
			if(hwnd1){
				hwnd2 = GetDlgItem(hwnd1,116);
				if(hwnd2) PostMessage(hwnd2,BM_CLICK,0,0);
			}
		}
		break;}
		
	case IDM_CHIME: /// Alarms
		AlarmChimeEnable(-1);
		break;
		
	case IDM_STOPWATCH: /// Timers
		DialogStopWatch();
		break;
	case IDM_STOPWATCH_START:
	case IDM_STOPWATCH_RESUME:
		if(!IsWindow(g_hDlgStopWatch))
			DialogStopWatch();
		StopWatch_Resume(g_hDlgStopWatch);
		break;
	case IDM_STOPWATCH_STOP:
	case IDM_STOPWATCH_PAUSE:
		if(IsWindow(g_hDlgStopWatch))
			StopWatch_Pause(g_hDlgStopWatch);
		break;
	case IDM_STOPWATCH_RESET:
		if(IsWindow(g_hDlgStopWatch))
			StopWatch_Reset(g_hDlgStopWatch);
		break;
	case IDM_STOPWATCH_LAP:
		if(IsWindow(g_hDlgStopWatch))
			StopWatch_Lap(g_hDlgStopWatch,0);
		break;
	case IDM_TIMER:
		DialogTimer(0);
		break;
	case IDM_TIMEWATCH:
		WatchTimer(0); // Shelter All the Homeless Timers.
		break;
	case IDM_TIMEWATCHRESET:
		WatchTimer(1); // Shelter All the Homeless Timers.
		break;
	case IDM_SNTP:{
		short just_elevated = HIWORD(wParam);
		if(!just_elevated || HaveSetTimePermissions()) {
			ReplyMessage(1);
			NetTimeConfigDialog(0);
			return 1; // handled
		} else {
			if(IsWindow(g_hDlgSNTP))
				SendMessage(g_hDlgSNTP, WM_CLOSE, 1, 0); // close window but safe changes
		}
		return 0;}
	case IDM_SYNCTIME:
	case IDM_SNTP_SYNC:{
		short just_elevated = HIWORD(wParam);
		int can_sync = HaveSetTimePermissions();
		if(!just_elevated || can_sync) {
			ReplyMessage(1);
			if(can_sync) {
				SyncTimeNow();
			} else {
				wchar_t exe[MAX_PATH];
				if(api.ExecElevated(GetClockExe(exe),L"/UAC /Sync",NULL) != 0) {
					MessageBox(0, L"T-Clock must be elevated to set your system time,\nbut elevation was canceled", L"Time Sync Failed", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
				}
			}
			return 1; // handled
		}
		return 0;}
	default:
		#ifdef _DEBUG
		DBGOUT("%s: unknown ID: %.5i(0x%.4x) (hwnd:%p)", __FUNCTION__, wID, wID, hwnd);
		#endif // _DEBUG
		break;
	}
	return 0;
}
//====================================================================
//----------+++--> Enumerate & display setup alarms incl. hourly chime:
void UpdateAlarmMenu(HMENU hMenu)   //--------------------------+++-->
{
	wchar_t buf[MAX_PATH];
	alarm_t pAS;
	int count;
	api.GetStr(L"", L"JihouFile", buf, _countof(buf), L"");
	if(buf[0])
		EnableMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_ENABLED);
	if(TimetableSearchID(SCHEDID_CHIME)) {
		CheckMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_CHECKED);
	}
	count = GetAlarmNum();
	if(count){
		int idx;
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
		for(idx=0; idx<count; ++idx) {
			ReadAlarmFromReg(&pAS,idx);
			wsprintf(buf, FMT("    %s	(%i"), pAS.dlgmsg.name,idx+1);
			InsertMenu(hMenu, IDM_PROP_ALARM, MF_BYCOMMAND|MF_STRING, IDM_I_ALARM+idx, buf);
			if(pAS.uFlags&ALRM_ENABLED)
				CheckMenuItem(hMenu,IDM_I_ALARM+idx,MF_BYCOMMAND|MF_CHECKED);
		}
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
	}
}
