//================================================================================
//--+++--> menu.c - pop-up menu on right button click - KAZUBON 1997-2001 =========
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->
#include <ShlObj.h>//IShellDispatch4

void UpdateAlarmMenu(HMENU hMenu);
static char g_undo=0; // did we change windows and can undo it?

/*-----------------------------------------------------------------
 ----------------  when the clock is right-clicked show pop-up menu
-----------------------------------------------------------------*/
void OnContextMenu(HWND hWnd, int xPos, int yPos)
{
	BOOL g_bQMAudio;
	BOOL g_bQMNet;
	BOOL g_bQMLaunch;
	BOOL g_bQMExitWin;
	BOOL g_bQMDisplay;
	HMENU hPopupMenu;
	HMENU hMenu;
	
	g_bQMAudio   = api.GetInt("QuickyMenu", "AudioProperties",   TRUE);
	g_bQMNet     = api.GetInt("QuickyMenu", "NetworkDrives",     TRUE);
	g_bQMLaunch  = api.GetInt("QuickyMenu", "QuickyMenu",        TRUE);
	g_bQMExitWin = api.GetInt("QuickyMenu", "ExitWindows",       TRUE);
	g_bQMDisplay = api.GetInt("QuickyMenu", "DisplayProperties", TRUE);
	
	hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU));
	hPopupMenu = GetSubMenu(hMenu, 0);
	
	if(!g_bQMAudio)		DeleteMenu(hPopupMenu, IDC_SOUNDAUDIO,	MF_BYCOMMAND);
	if(!g_bQMNet)		DeleteMenu(hPopupMenu, IDC_NETWORK,		MF_BYCOMMAND);
	if(!g_bQMLaunch)	DeleteMenu(hPopupMenu, IDC_QUICKYS,		MF_BYCOMMAND);
	if(!g_bQMExitWin)	DeleteMenu(hPopupMenu, IDC_EXITWIN,		MF_BYCOMMAND);
	if(!g_bQMDisplay)	DeleteMenu(hPopupMenu, IDM_DISPLAYPROP,	MF_BYCOMMAND);
	/// simple implementation of "Undo ..." (eg. Undo Cascade windows)
	if(!g_undo)			DeleteMenu(hPopupMenu, IDM_FWD_UNDO,	MF_BYCOMMAND);
	/// special menu items, only shown if SHIFT or CTRL was pressed
	if(!((GetAsyncKeyState(VK_SHIFT)|GetAsyncKeyState(VK_CONTROL))&0x8000)){
		DeleteMenu(hPopupMenu, IDS_NONE, MF_BYCOMMAND); // seperator
		DeleteMenu(hPopupMenu, IDM_FWD_RUNAPP, MF_BYCOMMAND);
		DeleteMenu(hPopupMenu, IDM_FWD_EXITEXPLORER, MF_BYCOMMAND);
	}
	/// AlarmsTimer Menu Item y/n Goes HERE!!!
	UpdateAlarmMenu(hPopupMenu);
	UpdateTimerMenu(hPopupMenu); // Get the List of Active Timers.
	
	if(g_bQMLaunch) {
		char key[TNY_BUFF];
		int offset=9;
		char name[TNY_BUFF];
		int idx;
		
		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.dwTypeData=name;
		memcpy(key,"MenuItem-",offset);
		for(idx=0; idx<12; ++idx) {
			offset=9+wsprintf(key+9,"%i",idx);
			if(api.GetInt("QuickyMenu\\MenuItems",key,0)){
				memcpy(key+offset,"-Text",6);
				api.GetStr("QuickyMenu\\MenuItems",key,name,sizeof(name),"");
				mii.wID=IDM_I_MENU+idx;
				InsertMenuItem(hPopupMenu, IDM_SHOWCALENDER, FALSE, &mii);
			}
		}
	}
	
	/// http://support.microsoft.com/kb/135788
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hPopupMenu, TPM_LEFTBUTTON, xPos, yPos, 0, hWnd, NULL);
	PostMessage(hWnd,WM_NULL,0,0);
	DestroyMenu(hMenu); // Starting Over is Simpler & Recommended
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
void OnTClockCommand(HWND hwnd, WORD wID)   //----------------------------------+++-->
{
	switch(wID) {
	case IDM_REFRESHTCLOCK: //-----+++--> RePaint & Size the T-Clock Display Window
		RefreshUs();
		break;
		
	case IDM_SHOWPROP: //---------------------+++--> Show T-Clock Properties Dialog
		MyPropertySheet(-1);
		break;
	case IDM_PROP_ALARM:
		MyPropertySheet(1);
		break;
		
	case IDM_SYNCTIME:
		SyncTimeNow();
		break;
		
	case IDM_EXIT: //--------------------------------------+++--> Exit T-Clock 2010
		SendMessage(hwnd,WM_CLOSE,0,0);
		break;
		
	case IDM_SHOWCALENDER: //-------------------------------+++--> Display Calender
		ToggleCalendar(1); // 1=own calendar
		break;
		
	case IDM_DISPLAYPROP: //------------------------------+++--> Display Properties
		if(api.OS >= TOS_VISTA)
			api.Exec("::{26EE0668-A00A-44D7-9371-BEB064C98683}\\1\\::{C555438B-3C23-4769-A71F-B6D3D9B6053A}",NULL,NULL);
		else
			api.Exec("control", "desk.cpl, display,1", NULL);
		break;
	case IDM_VOLUMECONTROL: //-------------------------------+++--> Volume Controls
		#ifndef __x86_64__
		#	define OPEN_VOLUME "SndVol32"
		#else
		#	define OPEN_VOLUME "SndVol"
		#endif
		api.Exec(OPEN_VOLUME, NULL, NULL);
		break;
		
	case IDM_AUDIOPROP: //----------------------------------+++--> Audio Properties
		api.Exec("control", "mmsys.cpl", NULL);
		break;
		
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
		ShellExecute(hwnd, "open","Explorer.exe", //-> Correctly at My Computer Level
					 "/e, ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}",NULL,SW_SHOWNORMAL);
		break;}
		
	case IDM_QUICKY_DOS: { // Command Prompt
		char RooT[MAX_PATH];
		GetWindowsDirectory(RooT,MAX_PATH);
		ShellExecute(hwnd, "open","cmd.exe", "/f:on /t:0a", RooT, SW_SHOWNORMAL);
		break;}
		
	case IDM_QUICKY_EMPTYRB:
		SHEmptyRecycleBin(0, NULL, SHERB_NOCONFIRMATION);
		break;
		
	case IDM_SHUTDOWN:
		if(!ShutDown())
			MessageBox(0, "Shutdown Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		break;
		
	case IDM_REBOOT:
		if(!ReBoot())
			MessageBox(0, "Reboot Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		break;
		
	case IDM_LOGOFF:
		if(!LogOff())
			MessageBox(0, "Logoff Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		break;
		
	case IDM_FWD_CASCADE: case IDM_FWD_SIDEBYSIDE: case IDM_FWD_STACKED: case IDM_FWD_SHOWDESKTOP: case IDM_FWD_MINALL: case IDM_FWD_UNDO:
		g_undo=(wID!=IDM_FWD_UNDO);
		/* fall through */
	case IDM_FWD_DATETIME: case IDM_FWD_CUSTOMNOTIFYICONS:
	case IDM_FWD_TASKMAN:
	case IDM_FWD_LOCKTASKBAR: case IDM_FWD_LOCKALLTASKBAR:
	case IDM_FWD_TASKBARPROP: case IDM_FWD_RUNAPP: case IDM_FWD_EXITEXPLORER:{
		HWND hwndTray = FindWindow("Shell_TrayWnd", NULL);
		if(hwndTray) PostMessage(hwndTray, WM_COMMAND, wID, 0);
		break;}
	case IDM_DATETIME_EX:{
		HWND hwnd1=FindWindow("Shell_TrayWnd",NULL);
		if(hwnd1){
			HWND hwnd2;
			int wait=40;
			SendMessage(hwnd1,WM_COMMAND,IDM_FWD_DATETIME,0);
			while((hwnd2=FindWindow(MAKEINTATOM(32770),"Date and Time"))==0 && wait--) Sleep(50);
			if(hwnd2){
				SetActiveWindow(hwnd2);
				wait=10; while((hwnd1=FindWindowEx(hwnd2,NULL,MAKEINTATOM(32770),"Date and Time"))==0 && wait--) Sleep(50);
				if(hwnd1){
					hwnd2=GetDlgItem(hwnd1,116);
					if(hwnd2) PostMessage(hwnd2,BM_CLICK,0,0);
				}
			}
		}
		break;}
		
	case IDM_CHIME:{ /// Alarms
		SetHourlyChime(!GetHourlyChime());
		break;}
		
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
		DialogTimer();
		break;
	case IDM_TIMEWATCH:
		WatchTimer(0); // Shelter All the Homeless Timers.
		break;
	case IDM_TIMEWATCHRESET:
		WatchTimer(1); // Shelter All the Homeless Timers.
		break;
	default:
		if(wID>=IDM_I_TIMER && wID<IDM_I_TIMER+1000){
			ToggleTimer(wID-IDM_I_TIMER);
		}else if(wID>=IDM_I_ALARM && wID<IDM_I_ALARM+1000){
			SetAlarmEnabled(wID-IDM_I_ALARM,!GetAlarmEnabled(wID-IDM_I_ALARM));
		}else if(wID>=IDM_I_MENU && wID<IDM_I_MENU+1000){
			char key[MAX_PATH];
			int offset=9;
			char szQM_Target[MAX_PATH];
			char szQM_Switch[MAX_PATH];
			memcpy(key,"MenuItem-",offset);
			offset+=wsprintf(key+offset,"%i",wID-IDM_I_MENU);
			memcpy(key+offset,"-Target",8);
			api.GetStr("QuickyMenu\\MenuItems",key,szQM_Target,sizeof(szQM_Target),"");
			memcpy(key+offset,"-Switches",10);
			api.GetStr("QuickyMenu\\MenuItems",key,szQM_Switch,sizeof(szQM_Switch),"");
			ShellExecute(hwnd, "open", szQM_Target, szQM_Switch, NULL, SW_SHOWNORMAL);
		}
		#ifdef _DEBUG
		else
			{char buf[1024]; wsprintf(buf,"%s: unknown ID: %.5i(0x%.4x) (hwnd:%p)\n",__FUNCTION__,wID,wID,hwnd);
			OutputDebugString(buf);}
		#endif // _DEBUG
	}
	return;
}
//====================================================================
//----------+++--> Enumerate & display setup alarms incl. hourly chime:
void UpdateAlarmMenu(HMENU hMenu)   //--------------------------+++-->
{
	char buf[MAX_PATH];
	alarm_t pAS;
	int count;
	api.GetStr("","JihouFile",buf,MAX_PATH,"");
	if(PathExists(buf)==1)
		EnableMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_ENABLED);
	if(GetHourlyChime()){
		CheckMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_CHECKED);
	}
	count=api.GetInt("","AlarmNum",0);
	if(count<1) count=0;
	if(count){
		int idx;
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
		for(idx=0; idx<count; ++idx) {
			ReadAlarmFromReg(&pAS,idx);
			wsprintf(buf,"    %s	(%i",pAS.dlgmsg.name,idx+1);
			InsertMenu(hMenu, IDM_PROP_ALARM, MF_BYCOMMAND|MF_STRING, IDM_I_ALARM+idx, buf);
			if(pAS.uFlags&ALRM_ENABLED)
				CheckMenuItem(hMenu,IDM_I_ALARM+idx,MF_BYCOMMAND|MF_CHECKED);
		}
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
	}
}
