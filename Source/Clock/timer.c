/*-------------------------------------------
  timer.c - Kazubon 1998-1999
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

char g_szTimersSubKey[]="Timers";
static int GetTimerInfo(char* dst, int num);
//==================================================================
//----------+++--> enumerate & display all times to our context menu:
void UpdateTimerMenu(HMENU hMenu)   //------------------------+++-->
{
	unsigned count=GetMyRegLong(g_szTimersSubKey,"NumberOfTimers",0);
	if(count){
		unsigned idx;
		char buf[GEN_BUFF+16];
		char subkey[TNY_BUFF];
		size_t offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
		memset(buf,' ',4);
		EnableMenuItem(hMenu,IDM_TIMEWATCH,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hMenu,IDM_TIMEWATCHRESET,MF_BYCOMMAND|MF_ENABLED);
		InsertMenu(hMenu,IDM_TIMER,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
		for(idx=0; idx<count; ++idx){
			char* pos=buf+4;
			wsprintf(subkey+offset, "%d", idx+1);
			pos+=GetMyRegStr(subkey,"Name",pos,GEN_BUFF,"");
			wsprintf(pos,"	(%i",idx+1);
			InsertMenu(hMenu,IDM_TIMER,MF_BYCOMMAND|MF_STRING,IDM_I_TIMER+idx,buf);
			if(GetMyRegLong(subkey,"Active",0))
				CheckMenuItem(hMenu,IDM_I_TIMER+idx,MF_BYCOMMAND|MF_CHECKED);
		}
		InsertMenu(hMenu,IDM_TIMER,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);
	}
}

// Structure for Timer Setting
typedef struct{
	int second;
	int minute;
	int hour;
	int day;
	char name[GEN_BUFF];
	char fname[MAX_BUFF];
	char bActive;
	char bRepeat;
	char bBlink;
} timeropt_t;

// Structure for Active Timers
typedef struct{
	int id;
	DWORD seconds;		// Second  = 1 Second
	DWORD tickonstart;	// Minute = 60 Seconds
	char name[GEN_BUFF];// Hour = 3600 Seconds
	char bHomeless;		// Day = 86400 Seconds
} timer_t;

static int m_timers = 0;
static timer_t* m_timer = NULL; // Array of Currently Active Timers

//========================================================================
//---------------------//---------------------------+++--> Clear All Timer:
void EndAllTimers()   //--------------------------------------------+++-->
{
	m_timers=0;
	free(m_timer),m_timer=NULL;
}
//=================================================================================
//----------------------------------------------+++--> Free Memory to Clear a Timer:
void StopTimer(int id)   //--------------------------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	int idx;
	
	for(idx=0; idx<m_timers; ++idx) {
		if(id==m_timer[idx].id){
			timer_t* told=m_timer;
			offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
			wsprintf(subkey+offset, "%d", m_timer[idx].id+1);
			SetMyRegLong(subkey, "Active", 0);
			if(--m_timers){
				int i;
				timer_t* tnew=(timer_t*)malloc(sizeof(timer_t)*m_timers);
				if(!tnew){
					++m_timers; return;
				}
				for(i=0; i<idx; ++i){
					tnew[i]=told[i];
				}
				for(i=idx; i<m_timers; ++i){
					tnew[i]=told[i+1];
				}
				m_timer=tnew;
			}else
				m_timer=NULL;
			Sleep(0);
			free(told);
			return;
		}
	}
}
//=================================================================================
//----------------------------------------------+++--> Free Memory to Clear a Timer:
void StartTimer(int id)   //-------------------------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	int idx;
	timer_t* told=m_timer;
	timer_t* tnew;
	for(idx=0; idx<m_timers; ++idx) {
		if(id==m_timer[idx].id) return;
	}
	tnew=(timer_t*)malloc(sizeof(timer_t)*(m_timers+1));
	if(!tnew) return;
	for(idx=0; idx<m_timers; ++idx){
		tnew[idx]=told[idx];
	}
	offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
	wsprintf(subkey+offset, "%d", id+1);
	GetMyRegStr(subkey,"Name",tnew[m_timers].name,sizeof(tnew[m_timers].name),"");
	if(!*tnew[m_timers].name){
		free(tnew);
		return;
	}
	tnew[m_timers].id=id;
	tnew[m_timers].seconds =GetMyRegLong(subkey,"Seconds",0);
	tnew[m_timers].seconds+=GetMyRegLong(subkey,"Minutes",0)*60;
	tnew[m_timers].seconds+=GetMyRegLong(subkey,"Hours",0)*3600;
	tnew[m_timers].seconds+=GetMyRegLong(subkey,"Days",0)*86400;
	tnew[m_timers].bHomeless=1;
	tnew[m_timers].tickonstart=GetTickCount()/1000;
	m_timer=tnew;
	++m_timers;
	free(told);
	SetMyRegLong(subkey, "Active", 1);
}
//=================================================================
//----------------------------------------------+++--> Toggle timer:
void ToggleTimer(int id)   //--------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
	wsprintf(subkey+offset, "%d", id+1);
	if(GetMyRegLong(subkey,"Active",0)){
		StopTimer(id);
	}else{
		StartTimer(id);
	}
}

static void OnOK(HWND hDlg);
static void OnDel(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnStopTimer(HWND hDlg);
static void OnTimerName(HWND hDlg);
static void Ring(HWND hwnd, int id);
static void OnTest(HWND hDlg, WORD id);
static void OnSanshoAlarm(HWND hDlg, WORD id);
static void UpdateNextCtrl(HWND hWnd, int iSpin, int iEdit, char bGoUp);

INT_PTR CALLBACK DlgProcTimer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//=========================================================================================*
// ------------------------------------------------------------- Open Add/Edit Timers Dialog
//===========================================================================================*
void DialogTimer()
{
	if(!g_hDlgTimer || !IsWindow(g_hDlgTimer))
		g_hDlgTimer=CreateDialog(0,MAKEINTRESOURCE(IDD_TIMER),NULL,DlgProcTimer);
}
//==============================================================================*
// ---------------------------------- Dialog Procedure for Add/Edit Timers Dialog
//================================================================================*
INT_PTR CALLBACK DlgProcTimer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:
		OnDestroy(hDlg);
		break;
		
	case WM_COMMAND: {
			WORD id = LOWORD(wParam);
			WORD code = HIWORD(wParam);
			switch(id) {
			case IDC_TIMERDEL:
				OnDel(hDlg);
				break;
				
			case IDOK:
				OnOK(hDlg);
				/* fall through */
			case IDCANCEL:
				DestroyWindow(hDlg);
				break;
				
			case IDC_TIMERNAME:
				if(code==CBN_EDITCHANGE) OnTimerName(hDlg);
				else if(code==CBN_SELCHANGE) PostMessage(hDlg, WM_COMMAND, MAKELONG(id, CBN_EDITCHANGE), 0);
				break;
				
			case IDCB_STOPTIMER:
				OnStopTimer(hDlg);
				break;
				
			case IDC_TIMERSANSHO:
				OnSanshoAlarm(hDlg, id);
				break;
				
			case IDC_TIMERTEST:
				OnTest(hDlg, id);
				break;
			}
			return TRUE;
		}
		//--------------------------------------------------------------------------+++-->
	case WM_NOTIFY: { //========================================== BEGIN WM_NOTIFY:
//----------------------------------------------------------------------------+++-->
			if(((NMHDR*)lParam)->code == UDN_DELTAPOS) {
				NMUPDOWN* lpnmud;
				int i;
				
				lpnmud = (NMUPDOWN*)lParam;
				if(lpnmud->iDelta > 0) { // User Selected the Up Arrow
					switch(LOWORD(wParam)) { //--+++--> on One of the Timer Controls.
					case IDC_TIMERSECSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERSECOND, NULL, TRUE);
						if(i == 59)
							UpdateNextCtrl(hDlg, IDC_TIMERMINSPIN, IDC_TIMERMINUTE, TRUE);
						break;
						
					case IDC_TIMERMINSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, NULL, TRUE);
						if(lpnmud->iDelta == 4) {
							if(i < 59)
								SetDlgItemInt(hDlg, IDC_TIMERMINUTE, i+1, TRUE);
						}
						if(i == 59)
							UpdateNextCtrl(hDlg, IDC_TIMERHORSPIN, IDC_TIMERHOUR, TRUE);
						break;
						
					case IDC_TIMERHORSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERHOUR, NULL, TRUE);
						if(lpnmud->iDelta == 4) {
							if(i < 23)
								SetDlgItemInt(hDlg, IDC_TIMERHOUR, i+1, TRUE);
						}
						if(i == 23)
							UpdateNextCtrl(hDlg, IDC_TIMERDAYSPIN, IDC_TIMERDAYS, TRUE);
						break;
						
					case IDC_TIMERDAYSPIN:
						if(lpnmud->iDelta == 4) {
							i = GetDlgItemInt(hDlg, IDC_TIMERDAYS, NULL, TRUE);
							if(i < 7)
								SetDlgItemInt(hDlg, IDC_TIMERDAYS, i+1, TRUE);
						} break;
					}
				} else { //--+++--> User Selected the Down Arrow
					switch(LOWORD(wParam)) { // on One of the Timer Controls.
					case IDC_TIMERSECSPIN:
						if(lpnmud->iDelta == -4) {
							i = GetDlgItemInt(hDlg, IDC_TIMERSECOND, NULL, TRUE);
							if(i > 0)
								SetDlgItemInt(hDlg, IDC_TIMERSECOND, i -1, TRUE);
						} break;
						
					case IDC_TIMERMINSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, NULL, TRUE);
						if(lpnmud->iDelta == -4) {
							if(i > 0)
								SetDlgItemInt(hDlg, IDC_TIMERMINUTE, i -1, TRUE);
						}
						if(i == 0)
							UpdateNextCtrl(hDlg, IDC_TIMERSECSPIN, IDC_TIMERSECOND, FALSE);
						break;
						
					case IDC_TIMERHORSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERHOUR, NULL, TRUE);
						if(lpnmud->iDelta == -4) {
							if(i > 0)
								SetDlgItemInt(hDlg, IDC_TIMERHOUR, i -1, TRUE);
						}
						if(i == 0)
							UpdateNextCtrl(hDlg, IDC_TIMERMINSPIN, IDC_TIMERMINUTE, FALSE);
						break;
						
					case IDC_TIMERDAYSPIN:
						i = GetDlgItemInt(hDlg, IDC_TIMERDAYS, NULL, TRUE);
						if(i == 0)
							UpdateNextCtrl(hDlg, IDC_TIMERHORSPIN, IDC_TIMERHOUR, FALSE);
						break;
					}
				}
			}
//----------------------------------------------------------------------------+++-->
			return TRUE; //=============================================== END WM_NOTIFY:
		} //----------------------------------------------------------------------+++-->
		
	case MM_MCINOTIFY:
	case MM_WOM_DONE:
		StopFile();
		SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
		return TRUE;
	}
	return FALSE;
}
//================================================================================================
//------------------------//------------------------+++--> free memories associated with combo box:
void OnDestroy(HWND hDlg)   //--------------------------------------------------------------+++-->
{
	int idx;
	int count=(int)CBGetCount(hDlg, IDC_TIMERNAME);
	StopFile();
	for(idx=0; idx<count; ++idx) {
		free((void*)CBGetItemData(hDlg,IDC_TIMERNAME,idx));
	}
	g_hDlgTimer = NULL;
}
//================================================================================================
//------------------------//----------------------------------+++--> Initialize the "Timer" Dialog:
void OnInit(HWND hDlg)   //-----------------------------------------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	int idx, count;
	timeropt_t* pts;
	
	SendMessage(hDlg, WM_SETICON, ICON_SMALL,(LPARAM)g_hIconTClock);
	SendMessage(hDlg, WM_SETICON, ICON_BIG,(LPARAM)g_hIconTClock);
	// init dialog items
	SendDlgItemMessage(hDlg, IDC_TIMERSECSPIN, UDM_SETRANGE32, 0,59); // 60 Seconds Max
	SendDlgItemMessage(hDlg, IDC_TIMERMINSPIN, UDM_SETRANGE32, 0,59); // 60 Minutes Max
	SendDlgItemMessage(hDlg, IDC_TIMERHORSPIN, UDM_SETRANGE32, 0,23); // 24 Hours Max
	SendDlgItemMessage(hDlg, IDC_TIMERDAYSPIN, UDM_SETRANGE32, 0,7); //  7 Days Max
	/// add default sound files to file dropdown
	if(g_tos>TOS_2000) {
		char tmp[MAX_PATH];
		HANDLE hFind;
		WIN32_FIND_DATA FindFileData;
		strcpy(tmp,g_mydir); add_title(tmp,"waves/*");
		if((hFind=FindFirstFile(tmp,&FindFileData)) != INVALID_HANDLE_VALUE) {
			do{
				if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) { // only files (also ignores . and ..)
					CBAddString(hDlg,IDC_TIMERFILE,FindFileData.cFileName);
				}
			}while(FindNextFile(hFind,&FindFileData));
			FindClose(hFind);
		}
	}
	// add timer to combobox
	offset=wsprintf(subkey,"%s\\Timer",g_szTimersSubKey);
	count=GetMyRegLong(g_szTimersSubKey, "NumberOfTimers", 0);
	for(idx=0; idx<count; ++idx) {
		pts = (timeropt_t*)malloc(sizeof(timeropt_t));
		wsprintf(subkey+offset,"%d",idx+1);
		pts->second = GetMyRegLong(subkey, "Seconds",  0);
		pts->minute = GetMyRegLong(subkey, "Minutes", 10);
		pts->hour   = GetMyRegLong(subkey, "Hours",    0);
		pts->day    = GetMyRegLong(subkey, "Days",     0);
		GetMyRegStr(subkey, "Name", pts->name, sizeof(pts->name), "");
		GetMyRegStr(subkey, "File", pts->fname, sizeof(pts->fname), "");
		pts->bBlink = (char)GetMyRegLong(subkey, "Blink", FALSE);
		pts->bRepeat = (char)GetMyRegLong(subkey, "Repeat", FALSE);
		pts->bActive = (char)GetMyRegLong(subkey, "Active", FALSE);
		CBAddString(hDlg, IDC_TIMERNAME, pts->name);
		CBSetItemData(hDlg, IDC_TIMERNAME, idx, pts);
	}
	// add "new timer" item
	pts = (timeropt_t*)calloc(1,sizeof(timeropt_t));
	memcpy(pts->name,"   <new timer>",15);
	CBAddString(hDlg, IDC_TIMERNAME, pts->name);
	CBSetItemData(hDlg, IDC_TIMERNAME, count, pts);
	CBSetCurSel(hDlg, IDC_TIMERNAME, 0);
	OnTimerName(hDlg);
	SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
	SendDlgItemMessage(hDlg, IDC_TIMERDEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconDel);
	
	SetMyDialgPos(hDlg,21);
}
//================================================================================================
//--{ START TIMER }-----//-----------------+++--> Called When "OK" Button is Clicked (Start Timer):
void OnOK(HWND hDlg)   //-------------------------------------------------------------------+++-->
{
	int idx, count, seconds, minutes, hours, days;
	char subkey[TNY_BUFF];
	size_t offset;
	char name[GEN_BUFF];
	char fname[MAX_PATH];
	
	offset=wsprintf(subkey,"%s\\Timer",g_szTimersSubKey);
	GetDlgItemText(hDlg, IDC_TIMERNAME, name, sizeof(name));
	
	count = (int)CBGetCount(hDlg, IDC_TIMERNAME);
	count -=1; // Skip the Last One Because It's the New Timer Dummy Item
	
	for(idx=0; idx<count; ++idx) {
		timeropt_t* pts;
		pts = (timeropt_t*)CBGetItemData(hDlg, IDC_TIMERNAME, idx);
		if(!strcmp(pts->name, name)) {
			pts->bActive = TRUE;
			break;
		}
	}
	wsprintf(subkey+offset,"%d",idx+1);
	SetMyRegStr(subkey, "Name", name);
	seconds = GetDlgItemInt(hDlg, IDC_TIMERSECOND, 0, 0);
	minutes = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, 0, 0);
	hours   = GetDlgItemInt(hDlg, IDC_TIMERHOUR,   0, 0);
	days    = GetDlgItemInt(hDlg, IDC_TIMERDAYS,   0, 0);
	if(seconds>59) for(; seconds>59; seconds-=60,++minutes);
	if(minutes>59) for(; minutes>59; minutes-=60,++hours);
	if(hours>23) for(; hours>23; hours-=24,++days);
	if(days>42) days=7;
	SetMyRegLong(subkey, "Seconds", seconds);
	SetMyRegLong(subkey, "Minutes", minutes);
	SetMyRegLong(subkey, "Hours",   hours);
	SetMyRegLong(subkey, "Days",    days);
	
	GetDlgItemText(hDlg, IDC_TIMERFILE, fname, sizeof(fname));
	SetMyRegStr(subkey, "File", fname);
	
	SetMyRegLong(subkey, "Repeat", IsDlgButtonChecked(hDlg, IDC_TIMERREPEAT));
	SetMyRegLong(subkey, "Blink",  IsDlgButtonChecked(hDlg, IDC_TIMERBLINK));
	SetMyRegLong(subkey, "Active",  TRUE);
	if(idx==count)
		SetMyRegLong(g_szTimersSubKey, "NumberOfTimers", idx+1);
	
	StartTimer(idx);
}
//================================================================================================
//-----------------------------//-----+++--> Load the Data Set For Timer X When Its Name is Called:
void OnTimerName(HWND hDlg)   //------------------------------------------------------------+++-->
{
	char name[TNY_BUFF];
	int idx, count;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, name, sizeof(name));
	count=(int)CBGetCount(hDlg,IDC_TIMERNAME);
	for(idx=0; idx<count; ++idx){
		timeropt_t* pts;
		pts = (timeropt_t*)CBGetItemData(hDlg, IDC_TIMERNAME, idx);
		if(!strcmp(name, pts->name)){
			SetDlgItemInt(hDlg, IDC_TIMERSECOND,	pts->second, 0);
			SetDlgItemInt(hDlg, IDC_TIMERMINUTE,	pts->minute, 0);
			SetDlgItemInt(hDlg, IDC_TIMERHOUR,		pts->hour,   0);
			SetDlgItemInt(hDlg, IDC_TIMERDAYS,		pts->day,    0);
			SetDlgItemText(hDlg, IDC_TIMERFILE,		pts->fname);
			CheckDlgButton(hDlg, IDC_TIMERREPEAT,	pts->bRepeat);
			CheckDlgButton(hDlg, IDC_TIMERBLINK,	pts->bBlink);
			if(pts->bActive){
				EnableDlgItem(hDlg, IDCB_STOPTIMER, TRUE);
				EnableDlgItem(hDlg, IDOK, FALSE);
			}else{
				EnableDlgItem(hDlg, IDCB_STOPTIMER, FALSE);
				EnableDlgItem(hDlg, IDOK, TRUE);
			}
			break;
		}
	}
	if(idx<count-1){
		SetDlgItemText(hDlg,IDOK,"Start");
	}else{
		SetDlgItemText(hDlg,IDOK,"Create");
	}
	EnableDlgItem(hDlg, IDC_TIMERDEL, idx<count-1);
}
/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnSanshoAlarm(HWND hDlg, WORD id)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	if(!BrowseSoundFile(hDlg, deffile, fname)) // soundselect.c
		return;
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

//================================================================================================
//-----------------------//-----------------------------+++--> Delete One of the Configured Timers:
void OnDel(HWND hDlg)   //------------------------------------------------------------------+++-->
{
	char name[TNY_BUFF];
	char subkey[TNY_BUFF];
	size_t offset;
	int idx, idx2, count;
	
	offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
	GetDlgItemText(hDlg, IDC_TIMERNAME, name, sizeof(name));
	count = (int)CBGetCount(hDlg, IDC_TIMERNAME) -1;
	for(idx=0; idx<count; ++idx) {
		timeropt_t* pts = (timeropt_t*)CBGetItemData(hDlg, IDC_TIMERNAME, idx);
		if(!strcmp(name,pts->name)){
			break;
		}
	}
	if(idx==count) return;
	StopTimer(idx);
	
	for(idx2=idx+1; idx2<count; ++idx2) {
		timeropt_t* pts;
		pts = (timeropt_t*)CBGetItemData(hDlg, IDC_TIMERNAME, idx2);
		wsprintf(subkey+offset, "%d", idx2); // we're 1 behind, as needed
		SetMyRegStr(subkey, "Name",		pts->name);
		SetMyRegLong(subkey, "Seconds",	pts->second);
		SetMyRegLong(subkey, "Minutes",	pts->minute);
		SetMyRegLong(subkey, "Hours",	pts->hour);
		SetMyRegLong(subkey, "Days",	pts->day);
		SetMyRegStr(subkey, "File",		pts->fname);
		SetMyRegLong(subkey, "Repeat",	pts->bRepeat);
		SetMyRegLong(subkey, "Blink",	pts->bBlink);
		SetMyRegLong(subkey, "Active",	pts->bActive);
	}
	wsprintf(subkey+offset, "%d", count);
	DelMyRegKey(subkey);
	SetMyRegLong(g_szTimersSubKey, "NumberOfTimers", --count);
	free((void*)CBGetItemData(hDlg,IDC_TIMERNAME,idx));
	CBDeleteString(hDlg,IDC_TIMERNAME,idx);
	
	CBSetCurSel(hDlg, IDC_TIMERNAME, (idx>0)?(idx-1):idx);
	OnTimerName(hDlg);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}
//================================================================================================
//---------------------------------//--------------------+++--> Test -> Play/Stop Alarm Sound File:
void OnTest(HWND hDlg, WORD id)   //--------------------------------------------------------+++-->
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id - 2, fname, MAX_PATH);
	if(!*fname) return;
	
	if((HICON)SendDlgItemMessage(hDlg, id, BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay) {
		if(PlayFile(hDlg, fname, 0)) {
			SendDlgItemMessage(hDlg, id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, id), NULL, FALSE);
		}
	} else StopFile();
}
//================================================================================================
//------+++--> Called When Main Window Receives WM_TIMER - Sound the Alarm if Clock has Run Out...:
void OnTimerTimer(HWND hwnd)   //-------------------------------------------+++-->
{
	DWORD tick;
	int idx;
	if(!m_timers) return;
	
	tick=GetTickCount()/1000;
	for(idx=0; idx<m_timers; ) {
		DWORD elapsed = tick-m_timer[idx].tickonstart;
		if(elapsed >= m_timer[idx].seconds){
			int id=m_timer[idx].id;
			StopTimer(id);
			Ring(hwnd,id);
		}else
			++idx;
	}
}
//================================================================================================
//------------------------------//---------------------------+++--> Sound Alarm or Open Timer File:
void Ring(HWND hwnd, int id)   //-----------------------------------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	char fname[MAX_BUFF];
	offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
	wsprintf(subkey+offset, "%d", id+1);
	GetMyRegStr(subkey, "File", fname, sizeof(fname), "");
	PlayFile(hwnd, fname, GetMyRegLong(subkey, "Repeat", 0)?-1:0);
	if(GetMyRegLong(subkey, "Blink", 0))
		PostMessage(g_hwndClock, CLOCKM_BLINK, 0, 0);
}
//================================================================================================
//---------+++--> Get Active Timer Name(s) to Populate Menu -or- Mark Selected Timer as "Homeless":
int GetTimerInfo(char* dst, int num)   //-----------------------------------+++-->
{
	if(num < m_timers) {
		char* out=dst;
		DWORD tick = GetTickCount()/1000;
		int iTCount = tick - m_timer[num].tickonstart;
		int days,hours,minutes;
		iTCount = m_timer[num].seconds - iTCount;
		if(iTCount <= 0) {
			return wsprintf(dst, " <- Time Expired!");
		}
		days = iTCount/86400; iTCount%=86400;
		hours = iTCount/3600; iTCount%=3600;
		minutes = iTCount/60; iTCount%=60;
		if(days) out+=wsprintf(out,"%d day%s ",days,(days==1?"":"s"));
		out+=wsprintf(out,"%02d:%02d:%02d",hours,minutes,iTCount);
		return (int)(out-dst);
	}
	*dst='\0';
	return 0;
}
//================================================================================================
//-------------------------------------+++--> Spoof Control Message to Force Update of Nexe Window:
void UpdateNextCtrl(HWND hWnd, int iSpin, int iEdit, char bGoUp)   //-----------------------+++-->
{
	NMUPDOWN nmud;
	
	nmud.hdr.hwndFrom = GetDlgItem(hWnd, iSpin);
	nmud.hdr.idFrom = iSpin;
	nmud.hdr.code = UDN_DELTAPOS;
	if(bGoUp)nmud.iDelta = 4; // Fake Message Forces Update of Next Control!
	else nmud.iDelta = -4;   // Fake Message Forces Update of Next Control!
	nmud.iPos = GetDlgItemInt(hWnd, iEdit, NULL, 1);
	
	SendMessage(hWnd, WM_NOTIFY, iSpin, (LPARAM)&nmud);
}
//================================================================================================
//-----------------------------//-------------------+++--> Stop & Cancel a Currently Running Timer:
void OnStopTimer(HWND hWnd)   //------------------------------------------------------------+++-->
{
	char name[GEN_BUFF];
	int idx;
	
	GetDlgItemText(hWnd, IDC_TIMERNAME, name, sizeof(name));
	
	for(idx=0; idx<m_timers; ++idx) {
		if(!strcmp(name, m_timer[idx].name)) {
			int id=m_timer[idx].id;
			timeropt_t* pts=(timeropt_t*)CBGetItemData(hWnd, IDC_TIMERNAME, id);;
			
			StopTimer(id);
			pts->bActive = 0;
			
			EnableDlgItem(hWnd, IDOK, 1);
			EnableDlgItemSafeFocus(hWnd, IDCB_STOPTIMER, 0, IDOK);
			break;
		}
	}
}
//================================================================================================
//-----------------------------+++--> When T-Clock Starts, Make Sure ALL Timer Are Set as INActive:
void CancelAllTimersOnStartUp()   //--------------------------------------------------------+++-->
{
	char subkey[TNY_BUFF];
	size_t offset;
	int idx, count;
	
	offset=wsprintf(subkey, "%s\\Timer", g_szTimersSubKey);
	count = GetMyRegLong(g_szTimersSubKey, "NumberOfTimers", 0);
	for(idx=0; idx<count; ) {
		wsprintf(subkey+offset, "%d", ++idx);
		SetMyRegLong(subkey, "Active", FALSE);
	}
}
//================================================================================================
// -------------------------------------------//+++--> Initialize Timer View/Watch Dialog Controls:
void OnInitTimeView(HWND hDlg)   //---------------------------------------------+++-->
{
	LVCOLUMN lvCol;
	HWND hList=GetDlgItem(hDlg,IDC_LIST);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL,(LPARAM)g_hIconTClock);
	SendMessage(hDlg, WM_SETICON, ICON_BIG,(LPARAM)g_hIconTClock);
	
	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
	SetXPWindowTheme(hList,L"Explorer",NULL);
	
	lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvCol.cx = 125;		 // Column Width
	lvCol.iSubItem = 0;      // Column Number
	lvCol.fmt = LVCFMT_CENTER; // Column Alignment
	lvCol.pszText = TEXT("Timer"); // Column Header Text
	ListView_InsertColumn(hList, 0, &lvCol);
	
	lvCol.cx = 150;
	lvCol.iSubItem = 1;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = TEXT("Remaining");
	ListView_InsertColumn(hList, 1, &lvCol);
}
//================================================================================================
//-------------------------------------+++--> Gather Status Info About the Timers User is Watching:
BOOL OnWatchTimer(HWND hDlg)   //-----------------------------------------------+++-->
{
	HWND hList=GetDlgItem(hDlg,IDC_LIST);
	char szStatus[MIN_BUFF];
	BOOL bNeeded = FALSE;
	LVFINDINFO lvFind;
	LVITEM lvItem;
	int id;
	for(id=m_timers; id--; ) {
		if(m_timer[id].bHomeless) {
			int iF;
			GetTimerInfo(szStatus,id);
			
			lvFind.flags = LVFI_STRING;
			lvFind.psz = m_timer[id].name;
			if((iF = ListView_FindItem(hList, -1, &lvFind)) != -1) {
				ListView_SetItemText(hList, iF, 1, szStatus); // IF Timer Pre-Exists,
				bNeeded = TRUE; //------------+++--> Update the Existing Timer Entry.
			} else {
				//---------------------+++--> ELSE Add the New Timer Entry to Watch List.
				lvItem.mask = LVIF_TEXT;
				lvItem.iSubItem = 0;
				lvItem.iItem = 0;
				
				lvItem.pszText = m_timer[id].name;
				ListView_InsertItem(hList, &lvItem);
				
				lvItem.iSubItem = 1;
				lvItem.pszText = szStatus;
				ListView_SetItem(hList, &lvItem);
				bNeeded = TRUE;
			}
		}
	}
	return bNeeded;
}
//================================================================================================
//-----------------------------+++--> Remove Timer X From Timer Watch List (Set Homeless to FALSE):
void RemoveFromWatch(HWND hWnd, HWND hList, char* szTimer, int iLx)
{
	const char szMessage[] = TEXT("Yes will cancel the timer & remove it from the Watch List\n"
							"No will remove timer from Watch List only (timer continues)\n"
							"Cancel will assume you hit delete accidentally (and do nothing)");
	char szCaption[GEN_BUFF];
	int idx;
	int id=-1;
							
	for(idx=0; idx<m_timers; ++idx) {
		if(!strcmp(szTimer, m_timer[idx].name)) {
			id=m_timer[idx].id;
			break;
		}
	}
	
	if(id==-1) { //----+++--> IF the Timer Has Expired...
		ListView_DeleteItem(hList,iLx); // Just Delete it.
		return;
	}
	
	wsprintf(szCaption, "Cancel Timer (%s) Also?", szTimer);
	
	switch(MessageBox(hWnd, szMessage, szCaption, MB_YESNOCANCEL|MB_ICONQUESTION)) {
	case IDYES:
		ListView_DeleteItem(hList, iLx);
		StopTimer(id); // Does Not Reset Active Flag!
		break;
	case IDNO:
		for(idx=0; idx<m_timers; ++idx) {
			if(!strcmp(szTimer, m_timer[idx].name)) {
				m_timer[idx].bHomeless = 0;
				ListView_DeleteItem(hList,iLx);
				break;
			}
		}
		break;
	}
}
//================================================================================================
// -----------------------+++--> Message Processor for the Selected Running Timers Watching Dialog:
INT_PTR CALLBACK DlgTimerViewProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)   //------+++-->
{
	switch(msg) {
	case WM_INITDIALOG:
		OnInitTimeView(hDlg);
		SetTimer(hDlg, 3, 285, NULL); // Timer Refresh Times Above 400ms Make
		SetMyDialgPos(hDlg,21); //-----+++--> Timer Watch Dialog Appear Sluggish.
		return TRUE; //-------------------------------+++--> END of Case WM_INITDOALOG
//================//================================================================
	case WM_TIMER:
		if(!OnWatchTimer(hDlg)) { // When the Last Monitored Timer
			KillTimer(hDlg, 3);			 // Expires, Close the Now UnNeeded
			g_hDlgTimerWatch = NULL;
			DestroyWindow(hDlg);		 // Timer Watch/View Dialog Window.
		} return TRUE; //--------------------------------+++--> END of Case WM_TIMER
//====================//============================================================
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			KillTimer(hDlg, 3);
			g_hDlgTimerWatch = NULL;
			DestroyWindow(hDlg);
			return TRUE;
		} return FALSE;//------------------------------+++--> END of Case WM_COMMAND
//===//=============================================================================
	case WM_NOTIFY:
		//--------------------------------------------------------------------+++-->
		if(((NMHDR*)lParam)->code == LVN_KEYDOWN) { //-+> Capture Key Strokes Here.
			LPNMLVKEYDOWN nmkey = (NMLVKEYDOWN*)lParam;
			HWND hList=GetDlgItem(hDlg,IDC_LIST);
			switch(nmkey->wVKey) {
			case VK_DELETE:{
				int i;
				if((i = ListView_GetNextItem(hList,-1,LVNI_SELECTED)) != -1) {
					char szTimer[GEN_BUFF];
					ListView_GetItemText(hList, i, 0, szTimer, GEN_BUFF);
					RemoveFromWatch(hDlg, hList, szTimer, i);
				}
				return TRUE;}// Delete Key Handled
			}
		} break; //-------------------------------------+++--> END of Case WM_NOTIFY
//===//=============================================================================
	}
	return FALSE;
}
//================================================================================================
// ------------------//---------------------------------------------+++--> Open Timer Watch Dialog:
void WatchTimer(int reset)   //----------------------------------------------------------------------+++-->
{
	if(reset){
		int idx; for(idx=0; idx<m_timers; ++idx) {
			m_timer[idx].bHomeless=1;
		}
	}
	if(!g_hDlgTimerWatch || !IsWindow(g_hDlgTimerWatch)) {
		g_hDlgTimerWatch=CreateDialog(0,MAKEINTRESOURCE(IDD_TIMERVIEW),NULL,DlgTimerViewProc);
	}
}
