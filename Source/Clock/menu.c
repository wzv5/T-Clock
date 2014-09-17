//================================================================================
//--+++--> menu.c - pop-up menu on right button click - KAZUBON 1997-2001 =========
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->

void UpdateAlarmMenu(HMENU hMenu);
static char g_undo=0; // did we change windows and can undo it?

/*-----------------------------------------------------------------
 ----------------  when the clock is right-clicked show pop-up menu
-----------------------------------------------------------------*/
void OnContextMenu(HWND hWnd, HWND hwndClicked, int xPos, int yPos)
{
	BOOL g_bQMAudio;
	BOOL g_bQMNet;
	BOOL g_bQMLaunch;
	BOOL g_bQMExitWin;
	BOOL g_bQMDisplay;
	HMENU hPopupMenu;
	HMENU hMenu;
	
	(void)hwndClicked;
	
	g_bQMAudio   = GetMyRegLong("QuickyMenu", "AudioProperties",   TRUE);
	g_bQMNet     = GetMyRegLong("QuickyMenu", "NetworkDrives",     TRUE);
	g_bQMLaunch  = GetMyRegLong("QuickyMenu", "QuickyMenu",        TRUE);
	g_bQMExitWin = GetMyRegLong("QuickyMenu", "ExitWindows",       TRUE);
	g_bQMDisplay = GetMyRegLong("QuickyMenu", "DisplayProperties", TRUE);
	
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
			if(GetMyRegLong("QuickyMenu\\MenuItems",key,0)){
				memcpy(key+offset,"-Text",6);
				GetMyRegStr("QuickyMenu\\MenuItems",key,name,sizeof(name),"");
				mii.wID=IDM_I_MENU+idx;
				InsertMenuItem(hPopupMenu, IDM_SHOWCALENDER, FALSE, &mii);
			}
		}
	}
	
	ForceForegroundWindow(hWnd);
	TrackPopupMenu(hPopupMenu, TPM_NONOTIFY|TPM_LEFTBUTTON, xPos, yPos, 0, hWnd, NULL);
	DestroyMenu(hMenu); // Starting Over is Simpler & Recommended
}
#ifdef __GNUC__
const GUID CLSID_Shell={0x13709620,0xc279,0x11ce,{0xa4,0x9e,0x44,0x45,0x53,0x54,0,1}};
const GUID IID_IShellDispatch4={0xefd84b2d,0x4bcf,0x4298,{0xbe,0x25,0xeb,0x54,0x2a,0x59,0xfb,0xda}};
#endif
//================================================================================================
//--------------------------------------+++--> Show/Hide Desktop (e.g. Show/Hide all Open Windows):
void ToggleDesk()   //----------------------------------------------------------------------+++-->
{
	IShellDispatch4* pDisp;
	HRESULT hres;
	
	CoInitialize(NULL);
	
	hres = CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &IID_IShellDispatch4, (LPVOID*)&pDisp);
	
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
		PostMessage(g_hwndClock, WM_COMMAND, IDM_EXIT, 0);
		break;
		
	case IDM_SHOWCALENDER: //-------------------------------+++--> Display Calender
		ToggleCalendar();
		break;
		
	case IDM_DISPLAYPROP: //------------------------------+++--> Display Properties
		WinExec(("control.exe desk.cpl, display,1"),SW_SHOW);
		break;
	case IDM_VOLUMECONTROL: //-------------------------------+++--> Volume Controls
		#ifndef __x86_64__
		#	define OPEN_VOLUME "SndVol32.exe"
		#else
		#	define OPEN_VOLUME "SndVol.exe"
		#endif
		WinExec((OPEN_VOLUME),SW_SHOW);
		break;
		
	case IDM_AUDIOPROP: //----------------------------------+++--> Audio Properties
		WinExec(("control.exe mmsys.cpl"),SW_SHOW);
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
//-----------------------//--------------------------------------------+++-->
	case IDCB_SW_START: //-> These Messages are Bounced From the Command Line
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_START, 0); //-+> Through Here
		break; //-+-> Then to the StopWatch Window - IF/When it is/Gets Opened
	case IDCB_SW_STOP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_STOP, 0);
		break;
	case IDCB_SW_RESET:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_RESET, 0);
		break;
	case IDCB_SW_LAP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_LAP, 0);
		break; //------------------------+++--> End of Bounce Through Messages
//-----------//--------------------------------------------------------+++-->
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
	case IDM_FWD_DATETIME: case IDM_FWD_CUSTOMNOTIFYICONS:
	case IDM_FWD_TASKMAN:
	case IDM_FWD_LOCKTASKBAR: case IDM_FWD_LOCKALLTASKBAR:
	case IDM_FWD_TASKBARPROP: case IDM_FWD_RUNAPP: case IDM_FWD_EXITEXPLORER:{
		HWND hwndTray = FindWindow("Shell_TrayWnd", NULL);
		if(hwndTray) PostMessage(hwndTray, WM_COMMAND, (WPARAM)wID, 0);
		break;}
	case IDM_DATETIME_EX:{
		int wait=40;
		HWND hwnd1=FindWindow("Shell_TrayWnd",NULL);
		if(hwnd1){
			HWND hwnd2;
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
			GetMyRegStr("QuickyMenu\\MenuItems",key,szQM_Target,sizeof(szQM_Target),"");
			memcpy(key+offset,"-Switches",10);
			GetMyRegStr("QuickyMenu\\MenuItems",key,szQM_Switch,sizeof(szQM_Switch),"");
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
	int count,idx;
	GetMyRegStr("","JihouFile",buf,MAX_PATH,"");
	if(PathExists(buf)==1)
		EnableMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_ENABLED);
	if(GetHourlyChime()){
		CheckMenuItem(hMenu,IDM_CHIME,MF_BYCOMMAND|MF_CHECKED);
	}
	count=GetMyRegLong("","AlarmNum",0);
	if(count<1) count=0;
	if(count){
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
		for(idx=0; idx<count; ++idx) {
			ReadAlarmFromReg(&pAS,idx);
			wsprintf(buf,"    %s	(%i",pAS.dlgmsg.name,idx+1);
			InsertMenu(hMenu, IDM_PROP_ALARM, MF_BYCOMMAND|MF_STRING, IDM_I_ALARM+idx, buf);
			if(pAS.bAlarm)
				CheckMenuItem(hMenu,IDM_I_ALARM+idx,MF_BYCOMMAND|MF_CHECKED);
		}
		InsertMenu(hMenu,IDM_PROP_ALARM,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
	}
}
