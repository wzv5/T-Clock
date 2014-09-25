// Written by Stoic Joker: Tuesday, 03/16/2010 @ 10:18:59pm
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"
#define DEFAULT_TIMETEXT "00 h 00 m 00 s 000 ms"
static LARGE_INTEGER m_frequency={{0}};
static LARGE_INTEGER m_start;// start time
static LARGE_INTEGER m_lap;// latest lap time
static LARGE_INTEGER m_stop;// latest lap time
static char m_paused; // Global Pause/Resume Displayed Counter.

void OnTimer(HWND hDlg);

BOOL IsDialogStopWatchMessage(HWND hwnd, MSG* msg){ // handles hotkeys
	int msgid=LOWORD(msg->message);
	if(msgid==WM_KEYDOWN || msgid==WM_KEYUP){
		if(msg->hwnd==hwnd || GetParent(msg->hwnd)==hwnd){
			int control;
			switch(msg->wParam){
			case 'S':
				control=IDC_SW_START;
				break;
			case 'W':
				control=IDC_SW_PAUSE;
				break;
			case 'A':
				control=IDC_SW_LAP;
				break;
			default:
				return IsDialogMessage(hwnd,msg);
			}
			if(msgid==WM_KEYDOWN && !(msg->lParam&0x40000000)){ // was up before, not repeated
				SendMessage(hwnd,WM_COMMAND,control,0);
			}
			return 1;
		}
	}
	return IsDialogMessage(hwnd,msg);
}
void StopWatch_Start(HWND hDlg){
	if(m_start.QuadPart){
		KillTimer(hDlg,1);
	}
	ListView_DeleteAllItems(GetDlgItem(hDlg,IDC_SW_LAPS));
	QueryPerformanceCounter(&m_start);
	m_lap=m_start;
	m_paused = 0;
	SetTimer(hDlg,1,7,NULL);
	SetDlgItemText(hDlg,IDC_SW_PAUSE,"Pause (w)");
	EnableDlgItem(hDlg,IDC_SW_PAUSE,1);
}
void StopWatch_Stop(HWND hDlg){
	if(!m_start.QuadPart) return;
	KillTimer(hDlg,1);
	m_paused = 1;
	OnTimer(hDlg); // update time text
	m_start.QuadPart=0;
	EnableDlgItem(hDlg,IDC_SW_PAUSE,0);
}
void StopWatch_Reset(HWND hDlg){
	if(!m_start.QuadPart) return;
	SetDlgItemText(hDlg,IDC_SW_ELAPSED,DEFAULT_TIMETEXT);
	ListView_DeleteAllItems(GetDlgItem(hDlg,IDC_SW_LAPS));
	if(m_paused){ // paused
		m_start.QuadPart=0;
		EnableDlgItem(hDlg,IDC_SW_PAUSE,0);
	}else{ // running
		QueryPerformanceCounter(&m_start);
		m_lap=m_start;
	}
}
void StopWatch_Pause(HWND hDlg){
	if(m_paused) return;
	KillTimer(hDlg,1);
	QueryPerformanceCounter(&m_stop);
	m_paused = 1;
	OnTimer(hDlg); // update time text
	SetDlgItemText(hDlg,IDC_SW_PAUSE,"Resume (w)");
}
void StopWatch_Resume(HWND hDlg){
	LARGE_INTEGER end,diff;
	if(!m_paused) return;
	if(!m_start.QuadPart){
		StopWatch_Start(hDlg);
		return;
	}
	QueryPerformanceCounter(&end);
	diff.QuadPart=end.QuadPart-m_stop.QuadPart;
	m_start.QuadPart+=diff.QuadPart;
	if(m_lap.QuadPart)
		m_lap.QuadPart+=diff.QuadPart;
	else
		m_lap=end;
	m_paused = 0;
	SetTimer(hDlg,1,7,NULL);
	SetDlgItemText(hDlg,IDC_SW_PAUSE,"Pause (w)");
}
void StopWatch_TogglePause(HWND hDlg){
	if(m_paused){
		StopWatch_Resume(hDlg);
	}else{
		StopWatch_Pause(hDlg);
	}
}
void StopWatch_Lap(HWND hDlg){ // Get Current Time as Lap Time and Add it to the ListView Control
	char buf[TNY_BUFF];
	int hrs, min;
	LVITEM lvItem; // ListView Control Row Identifier
	LARGE_INTEGER end;
	unsigned long elapsed;
	HWND hList=GetDlgItem(hDlg,IDC_SW_LAPS);
	
	if(!m_start.QuadPart || !m_lap.QuadPart)
		return;
	QueryPerformanceCounter(&end);
	if(m_paused)
		end=m_stop;
	elapsed=(unsigned long)((end.QuadPart-m_lap.QuadPart)*1000/m_frequency.QuadPart);
	if(m_paused)
		m_lap.QuadPart=0;
	else
		m_lap=end;
	
	hrs=elapsed/3600000; elapsed%=3600000;
	min=elapsed/60000; elapsed%=60000;
	
	wsprintf(buf,"Lap %d",ListView_GetItemCount(hList)+1);
	lvItem.mask=LVIF_TEXT;
	lvItem.iSubItem=0;
	lvItem.iItem=0;
	lvItem.pszText=buf;
	ListView_InsertItem(hList,&lvItem);
	
	wsprintf(buf,"%02d:%02d:%02lu.%03lu",hrs,min,elapsed/1000,elapsed%1000);
	lvItem.iSubItem=1;
	ListView_SetItem(hList,&lvItem);
}
//================================================================================================
// ----------------------------------------------------+++--> Initialize Stopwatch Dialog Controls:
void OnInit(HWND hDlg)   //-----------------------------------------------------------------+++-->
{
	LVCOLUMN lvCol;
	HWND hList=GetDlgItem(hDlg,IDC_SW_LAPS);
	m_paused=1;
	m_start.QuadPart=0;
	QueryPerformanceFrequency(&m_frequency);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL,(LPARAM)g_hIconTClock);
	SendMessage(hDlg, WM_SETICON, ICON_BIG,(LPARAM)g_hIconTClock);
	
	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
	SetXPWindowTheme(hList,L"Explorer",NULL);
	
	lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvCol.cx = 52; // Column Width
	lvCol.iSubItem = 0;
	lvCol.fmt = LVCFMT_CENTER;
	lvCol.pszText = TEXT("Lap");
	ListView_InsertColumn(hList,0,&lvCol);
	
	lvCol.cx = 164;
	lvCol.iSubItem = 1;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = TEXT("Times");
	ListView_InsertColumn(hList,1,&lvCol);
	
	{
		LOGFONT logfont;
		HFONT hfont;
		hfont=(HFONT)SendMessage(hDlg,WM_GETFONT,0,0);
		GetObject(hfont,sizeof(LOGFONT),&logfont);
		logfont.lfHeight=logfont.lfHeight*120/100;
		logfont.lfWeight=FW_BOLD;
		hfont=CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg,IDC_SW_ELAPSED,WM_SETFONT,(WPARAM)hfont,0);
	}
	SetDlgItemText(hDlg, IDC_SW_ELAPSED, DEFAULT_TIMETEXT);
}
//================================================================================================
//-------------------------//------------------+++--> Updates the Stopwatch's Elapsed Time Display:
void OnTimer(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char szElapsed[TNY_BUFF];
	int hrs, min, sec;
	union{
		unsigned long elapsed;
		LARGE_INTEGER end;// latest lap time
	} un;
	
	QueryPerformanceCounter(&un.end);
	un.end.QuadPart-=m_start.QuadPart;
	un.elapsed=(unsigned long)(un.end.QuadPart*1000/m_frequency.QuadPart);
	
	hrs=un.elapsed/3600000; un.elapsed%=3600000;
	min=un.elapsed/60000; un.elapsed%=60000;
	sec=un.elapsed/1000; un.elapsed%=1000;
	wsprintf(szElapsed,"%02d h %02d m %02d s %03lu ms",hrs,min,sec,un.elapsed);
	SetDlgItemText(hDlg, IDC_SW_ELAPSED, szElapsed);
}
//================================================================================================
// --------------------------------------------------+++--> Message Processor for Stopwatch Dialog:
INT_PTR CALLBACK DlgProcStopwatch(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)   //------+++-->
{
	switch(msg) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		SetMyDialgPos(hDlg,21);
		return TRUE;
	case WM_DESTROY:{
		HFONT hfont=(HFONT)SendDlgItemMessage(hDlg,IDC_SW_ELAPSED,WM_GETFONT,0,0);
		SendDlgItemMessage(hDlg,IDC_SW_ELAPSED,WM_SETFONT,0,0);
		DeleteObject(hfont);
		break;}
	case WM_ACTIVATE:
		if(LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE){
			SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}else{
			SetWindowPos(hDlg,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SetWindowPos((HWND)wParam,hDlg,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		break;
	case WM_CTLCOLORSTATIC:
		if((HWND)lParam!=GetDlgItem(hDlg,IDC_SW_ELAPSED))
			break;
		SetTextColor((HDC)wParam,0x00000000);
		SetBkColor((HDC)wParam,0x00FFFFFF);
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	case WM_TIMER:
		if(!m_paused)
			OnTimer(hDlg);
		return TRUE;
	case WM_COMMAND: {
			WORD id = LOWORD(wParam);
			switch(id) {
			case IDC_SW_START: // Start/Stop
				StopWatch_Start(hDlg);
				break;
			case IDC_SW_PAUSE:
				StopWatch_TogglePause(hDlg);
				break;
			case IDC_SW_LAP:
				StopWatch_Lap(hDlg);
				break;
			case IDCANCEL:
				KillTimer(hDlg, 1);
				g_hDlgStopWatch = NULL;
				EndDialog(hDlg, TRUE);
			}
			return TRUE;
		}
	}
	return FALSE;
}
//================================================================================================
// -------------------------------------------------------------------+++--> Open Stopwatch Dialog:
void DialogStopWatch()   //--------------------------------------------------------+++-->
{
	if(!g_hDlgStopWatch || !IsWindow(g_hDlgStopWatch))
		g_hDlgStopWatch=CreateDialog(0,MAKEINTRESOURCE(IDD_STOPWATCH),NULL,DlgProcStopwatch);
}
