//================================================================================
//--+++--> menu.c - pop-up menu on right button click - KAZUBON 1997-2001 =========
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->

#define QM_COMMAND 3800
void UpdateTimerMenu(HMENU hMenu);
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
	/// Timers Menu Item y/n Goes HERE!!!
	
	UpdateTimerMenu(hPopupMenu); // Get the List of Active Timers.
	
	if(g_bQMLaunch) {
		char szmItem[TNY_BUFF] = {0};
		UINT uItemID = QM_COMMAND;
		char s[TNY_BUFF] = {0};
		int iMenu;
		
		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.fMask = MIIM_STRING | MIIM_ID;
		
		for(iMenu = 0; iMenu <= 11; iMenu++) {
			wsprintf(szmItem, "MenuItem-%d", iMenu);
			
			if(GetMyRegLong("QuickyMenu\\MenuItems", szmItem, FALSE)) {
				wsprintf(szmItem, "MenuItem-%d-Text", iMenu);
				GetMyRegStr("QuickyMenu\\MenuItems", szmItem, s, TNY_BUFF, "");
				mii.dwTypeData = s;
				mii.wID = uItemID;
				InsertMenuItem(hPopupMenu, IDM_SHOWCALENDER, FALSE, &mii);
			}
			uItemID++;
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
		return;
		
	case IDM_SHOWPROP: //---------------------+++--> Show T-Clock Properties Dialog
		MyPropertySheet();
		return;
		
	case IDM_SYNCTIME:
		SyncTimeNow();
		return;
		
	case JRMSG_BOING:
		ReleaseTheHound(hwnd, TRUE);
		return;
		
	case IDM_EXIT: //--------------------------------------+++--> Exit T-Clock 2010
		PostMessage(g_hwndClock, WM_COMMAND, IDM_EXIT, 0);
		return;
		
	case IDM_SHOWCALENDER: //-------------------------------+++--> Display Calender
		ToggleCalendar();
		return;
		
	case IDM_DISPLAYPROP: //------------------------------+++--> Display Properties
		WinExec(("control.exe desk.cpl, display,1"),SW_SHOW);
		return;
//========================================================================
#if defined _M_IX86 //-----+++--> IF Compiling This as a 32-bit Clock Use:
#define OPEN_VOLUME "sndvol32.exe"

#else //---------+++--> ELSE Assume: _M_X64 - IT's a 64-bit Clock and Use:
#define OPEN_VOLUME "sndvol.exe"

#endif
//========================================================================
	case IDM_VOLUMECONTROL: //-------------------------------+++--> Volume Controls
		WinExec((OPEN_VOLUME),SW_SHOW);
		return;
		
	case IDM_AUDIOPROP: //----------------------------------+++--> Audio Properties
		WinExec(("control.exe mmsys.cpl"),SW_SHOW);
		return;
		
	case IDM_MAPDRIVE: //----------------------------------+++--> Map Network Drive
		WNetConnectionDialog(hwnd, RESOURCETYPE_DISK);
		return;
		
	case IDM_DISCONNECT: //-------------------------+++--> Disconnect Network Drive
		WNetDisconnectDialog(hwnd, RESOURCETYPE_DISK);
		return;
		
	case IDM_TOGGLE_DT: //---------------------------+++--> Show / Hide the Desktop
		ToggleDesk();
		return;
		
	case IDM_QUICKY_WINEXP: { //-----------------//--+++--> Windows Explorer Opened
			ShellExecute(hwnd, "open","Explorer.exe", //-> Correctly at My Computer Level
						 "/e, ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}",NULL,SW_SHOWNORMAL);
			return;
		}
		
	case IDM_QUICKY_DOS: { // Command Prompt
			char RooT[MAX_PATH];
			GetWindowsDirectory(RooT,MAX_PATH);
			ShellExecute(hwnd, "open","cmd.exe", "/f:on /t:0a", RooT, SW_SHOWNORMAL);
			return;
		}
		
	case QM_COMMAND:
	case QM_COMMAND+1:
	case QM_COMMAND+2:
	case QM_COMMAND+3:
	case QM_COMMAND+4:
	case QM_COMMAND+5:
	case QM_COMMAND+6:
	case QM_COMMAND+7:
	case QM_COMMAND+8:
	case QM_COMMAND+9:
	case QM_COMMAND+10:
	case QM_COMMAND+11: {
			char szQM_Temp[260]="";
			char szQM_Target[260]="";
			char szQM_Switch[260]="";
			UINT uQM_cID = (wID - QM_COMMAND);
			
			wsprintf(szQM_Temp, "MenuItem-%d-Target", uQM_cID);
			GetMyRegStr("QuickyMenu\\MenuItems", szQM_Temp, szQM_Target, 260, "");
			
			wsprintf(szQM_Temp, "MenuItem-%d-Switches", uQM_cID);
			GetMyRegStr("QuickyMenu\\MenuItems", szQM_Temp, szQM_Switch, 260, "");
			
			ShellExecute(hwnd, "open", szQM_Target, szQM_Switch, NULL, SW_SHOWNORMAL);
			return;
		}
		
	case IDM_QUICKY_EMPTYRB:
		SHEmptyRecycleBin(0, NULL, SHERB_NOCONFIRMATION);
		return;
//-----------------------//--------------------------------------------+++-->
	case IDCB_SW_START: //-> These Messages are Bounced From the Command Line
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_START, 0); //-+> Through Here
		return; //-+-> Then to the StopWatch Window - IF/When it is/Gets Opened
	case IDCB_SW_STOP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_STOP, 0);
		return;
	case IDCB_SW_RESET:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_RESET, 0);
		return;
	case IDCB_SW_LAP:
		SendMessage(g_hDlgStopWatch, WM_COMMAND, IDCB_SW_LAP, 0);
		return; //------------------------+++--> End of Bounce Through Messages
//-----------//--------------------------------------------------------+++-->
	case IDM_SHUTDOWN:
		if(!ShutDown())
			MessageBox(0, "Shutdown Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		return;
		
	case IDM_REBOOT:
		if(!ReBoot())
			MessageBox(0, "Reboot Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		return;
		
	case IDM_LOGOFF:
		if(!LogOff())
			MessageBox(0, "Logoff Request Failed!", "ERROR:", MB_OK|MB_ICONERROR);
		return;
		
	case IDM_TIMER: // Timer
		DialogTimer();
		return;
		
	case IDM_STOPWATCH:
		DialogStopWatch();
		
	case IDM_FWD_CASCADE: case IDM_FWD_SIDEBYSIDE: case IDM_FWD_STACKED: case IDM_FWD_SHOWDESKTOP: case IDM_FWD_MINALL: case IDM_FWD_UNDO:
		g_undo=(wID!=IDM_FWD_UNDO);
	case IDM_FWD_DATETIME: case IDM_FWD_CUSTOMNOTIFYICONS:
	case IDM_FWD_TASKMAN:
	case IDM_FWD_LOCKTASKBAR: case IDM_FWD_LOCKALLTASKBAR:
	case IDM_FWD_TASKBARPROP: case IDM_FWD_RUNAPP: case IDM_FWD_EXITEXPLORER:{
		HWND hwndTray = FindWindow("Shell_TrayWnd", NULL);
		if(hwndTray) PostMessage(hwndTray, WM_COMMAND, (WPARAM)wID, 0);
		return;}
		
	case ID_T_TIMER1:
	case ID_T_TIMER2:
	case ID_T_TIMER3:
	case ID_T_TIMER4:
	case ID_T_TIMER5:
	case ID_T_TIMER6:
	case ID_T_TIMER7:
		{char szTime[GEN_BUFF];
		GetTimerInfo(szTime, wID-ID_T_TIMER1, FALSE);}
	case IDM_TIMEWATCH:
		WatchTimer(); // Shelter All the Homeless Timers.
		return;
	}
	
	if((IDM_STOPTIMER <= wID && wID < IDM_STOPTIMER + MAX_TIMER)) {
		StopTimer(wID-IDM_STOPTIMER); //-+-> Stop Timer X!
	}
	return;
}
//================================================================================================
//----------+++--> Enumerate & Display ALL Currently Active Timers on The Running Timers Menu List:
void UpdateTimerMenu(HMENU hMenu)   //------------------------------------------------------+++-->
{
	int tid; char buf[GEN_BUFF];
	for(tid=0; tid<7; ++tid) { //---------+++--> the Running Timers List Menu Item.
		if(!GetTimerInfo(buf,tid,TRUE)) break; // Get the Timer's Name Only
		InsertMenu(hMenu, IDM_TIMEWATCH, MF_BYCOMMAND|MF_STRING, ID_T_TIMER1+tid, buf);
		EnableMenuItem(hMenu,IDM_TIMEWATCH,MF_BYCOMMAND|MF_ENABLED);
	}
}
