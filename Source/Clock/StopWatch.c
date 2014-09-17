// Written by Stoic Joker: Tuesday, 03/16/2010 @ 10:18:59pm
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"

LARGE_INTEGER g_frequency={{0}};
LARGE_INTEGER g_start;// start time
LARGE_INTEGER g_lap={{0}};// latest lap time
LARGE_INTEGER g_stop;// latest lap time
BOOL g_paused; // Global Pause/Resume Displayed Counter.
//================================================================================================
// ----------------------------------------------------+++--> Initialize Stopwatch Dialog Controls:
void OnInit(HWND hDlg, HWND* hList)   //-----------------------------------------------------+++-->
{
	LVCOLUMN lvCol;
	SendMessage(hDlg, WM_SETICON, ICON_SMALL,(LPARAM)g_hIconTClock);
	SendMessage(hDlg, WM_SETICON, ICON_BIG,(LPARAM)g_hIconTClock);
	
	*hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|LVS_REPORT|
						 LVS_NOSORTHEADER|LVS_SINGLESEL, 9, 55, 236, 104, hDlg, 0, 0, NULL);
	ListView_SetExtendedListViewStyle(*hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
	SetXPWindowTheme(*hList,L"Explorer",NULL);
	
	lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvCol.cx = 52; // Column Width
	lvCol.iSubItem = 0;
	lvCol.fmt = LVCFMT_CENTER;
	lvCol.pszText = TEXT("Lap");
	ListView_InsertColumn(*hList,0,&lvCol);
	
	lvCol.cx = 164;
	lvCol.iSubItem = 1;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = TEXT("Times");
	ListView_InsertColumn(*hList,1,&lvCol);
	
	ShowWindow(*hList, SW_SHOW);
	
	SetDlgItemText(hDlg, IDCE_SW_ELAPSED, "00H: 00M: 00S: 000ms");
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
	un.end.QuadPart-=g_start.QuadPart;
	un.elapsed=(unsigned long)(un.end.QuadPart*1000/g_frequency.QuadPart);
	
	hrs=un.elapsed/3600000; un.elapsed%=3600000;
	min=un.elapsed/60000; un.elapsed%=60000;
	sec=un.elapsed/1000; un.elapsed%=1000;
	wsprintf(szElapsed,"%02dH: %02dM: %02dS: %03lums",hrs,min,sec,un.elapsed);
	SetDlgItemText(hDlg, IDCE_SW_ELAPSED, szElapsed);
}
//================================================================================================
//--------------------------+++--> Get Current Time as Lap Time and Add it to the ListView Control:
void InsertLapTime(HWND hList)   //---------------------------------------------------------+++-->
{
	char buf[TNY_BUFF];
	int hrs, min;
	LVITEM lvItem; // ListView Control Row Identifier
	LARGE_INTEGER end;
	unsigned long elapsed;
	
	if(!g_lap.QuadPart)
		return;
	QueryPerformanceCounter(&end);
	if(g_paused)
		end=g_stop;
	elapsed=(unsigned long)((end.QuadPart-g_lap.QuadPart)*1000/g_frequency.QuadPart);
	g_lap=end;
	if(g_paused)
		g_lap.QuadPart=0;
	
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
// --------------------------------------------------+++--> Message Processor for Stopwatch Dialog:
INT_PTR CALLBACK DlgProcStopwatch(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)   //------+++-->
{
	static HWND hList=NULL;
	
	(void)lParam;
	
	switch(msg) {
	case WM_INITDIALOG:
		OnInit(hDlg,&hList);
		SetMyDialgPos(hDlg,21);
		return TRUE;
	case WM_ACTIVATE:
		if(LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE){
			SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}else{
			SetWindowPos(hDlg,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SetWindowPos((HWND)wParam,hDlg,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		break;
	case WM_TIMER:
		if(!g_paused)
			OnTimer(hDlg);
		return TRUE;
	case WM_COMMAND: {
			WORD id = LOWORD(wParam);
			switch(id) {
			case IDCB_SW_START: // Start
				if(!g_paused) {
					ListView_DeleteAllItems(hList);
					if(!g_frequency.QuadPart)
						QueryPerformanceFrequency(&g_frequency);
					QueryPerformanceCounter(&g_start);
					g_lap=g_start;
				}else{
					LARGE_INTEGER end;
					QueryPerformanceCounter(&end);
					end.QuadPart-=g_stop.QuadPart;
					if(g_start.QuadPart)
						g_start.QuadPart+=end.QuadPart;
					else
						QueryPerformanceCounter(&g_start);
					if(g_lap.QuadPart)
						g_lap.QuadPart+=end.QuadPart;
					else
						QueryPerformanceCounter(&g_lap);
					g_paused = FALSE;
				}
				SetTimer(hDlg,1,7,NULL);
				ShowWindow(GetDlgItem(hDlg,IDCB_SW_STOP),1);
				ShowWindow(GetDlgItem(hDlg,IDCB_SW_START),0);
				EnableWindow(GetDlgItem(hDlg,IDCB_SW_RESET),1);
				break;
			case IDCB_SW_STOP:{
				KillTimer(hDlg,1);
				ShowWindow(GetDlgItem(hDlg,IDCB_SW_START),1);
				ShowWindow(GetDlgItem(hDlg,IDCB_SW_STOP),0);
				g_paused = TRUE;
				QueryPerformanceCounter(&g_stop);
				break;}
			case IDCB_SW_RESET:
				SetDlgItemText(hDlg,IDCE_SW_ELAPSED,"00H: 00M: 00S: 000ms");
				ListView_DeleteAllItems(hList);
				if(g_paused){
					g_start.QuadPart=g_lap.QuadPart=0;
					EnableWindow(GetDlgItem(hDlg,IDCB_SW_RESET),0);
				}else{
					QueryPerformanceCounter(&g_start);
					g_lap=g_start;
				}
				break;
			case IDCB_SW_LAP:
				InsertLapTime(hList);
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
	ForceForegroundWindow(g_hDlgStopWatch);
}
