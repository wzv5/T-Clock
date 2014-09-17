/*-------------------------------------------
  pagealarm.c
  "Alarm" page of properties
  KAZUBON 1997-2001
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnDeinit(HWND hDlg);
static void OnApply(HWND hDlg);
static void GetAlarmFromDlg(HWND hDlg, alarm_t* pAS);
static void SetAlarmToDlg(HWND hDlg, alarm_t* pAS);
static void OnChangeAlarm(HWND hDlg);
static void OnDropDownAlarm(HWND hDlg);
static void OnDay(HWND hDlg);
static void OnAlermJihou(HWND hDlg, WORD id);
static void OnSanshoAlarm(HWND hDlg, WORD id);
static void On12Hour(HWND hDlg);
static void OnDelAlarm(HWND hDlg);
static void OnFileChange(HWND hDlg, WORD id);
static void OnTest(HWND hDlg, WORD id);
static void OnMsgAlarm(HWND hDlg, WORD id);
static void FormatTimeText(HWND hDlg, WORD id);
static void UpdateAMPMDisplay(HWND hDlg);

static char m_bPlaying = 0;
static char m_bTransition=0;
static int m_curAlarm;
static unsigned m_days;

static void SendPSChanged(HWND hDlg){
	if(!m_bTransition)
		SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageAlarmProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
		
	case WM_DESTROY:
		OnDeinit(hDlg);
		break;
		
	case WM_COMMAND: {
		WORD id = LOWORD(wParam);
		WORD code = HIWORD(wParam);
		switch(id){
		/// global hourly chime and "chime hour"
		case IDC_ALARM:
		case IDC_JIHOU:
			OnAlermJihou(hDlg, id);
			SendPSChanged(hDlg);
			break;
		// file name changed
		case IDC_HOURALARM:
		case IDC_MINUTEALARM:
			if(code==EN_CHANGE){
				if(!m_bTransition){
					FormatTimeText(hDlg,id);
					SendPSChanged(hDlg);
				}
			}
			break;
		// browse file
		case IDC_SANSHOALARM:
		case IDC_SANSHOJIHOU:
			OnSanshoAlarm(hDlg, id);
			OnFileChange(hDlg, (WORD)(id - 1));
			SendPSChanged(hDlg);
			break;
		// test sound
		case IDC_TESTALARM:
		case IDC_TESTJIHOU:
			OnTest(hDlg,id);
			break;
		/// alarm chooser
		case IDC_COMBOALARM:
			if(code==CBN_SELCHANGE){
				OnChangeAlarm(hDlg); // update currently selected alarm
			}else if(code==CBN_DROPDOWN){
				OnDropDownAlarm(hDlg); // update name if changed
			}else if(code==CBN_EDITCHANGE){
				SendPSChanged(hDlg);
			}
			break;
		// delete an alarm
		case IDC_DELALARM:
			OnDelAlarm(hDlg);
			break;
		// file name changed
		case IDC_FILEALARM:
		case IDC_FILEJIHOU:
			if(code==EN_CHANGE){
				OnFileChange(hDlg, id);
				SendPSChanged(hDlg);
			}
			break;
		// day selector
		case IDC_ALARMDAY:
			OnDay(hDlg);
			break;
		// Message Window Options
		case IDCB_MSG_ALARM:{
			dlgmsg_t dlg;
			GetDlgItemText(hDlg, IDC_COMBOALARM, dlg.name, sizeof(dlg.name));
			GetDlgItemText(hDlg, IDC_JRMSG_TEXT, dlg.message, sizeof(dlg.message));
			GetDlgItemText(hDlg, IDC_JR_SETTINGS, dlg.settings, sizeof(dlg.settings));
			if(BounceWindOptions(hDlg,&dlg)){
				SetDlgItemText(hDlg, IDC_JR_SETTINGS, dlg.settings);
				SetDlgItemText(hDlg, IDC_JRMSG_TEXT, dlg.message);
				SendPSChanged(hDlg);
			}
			break;}
		// checked "12 hour"
		case IDC_12HOURALARM:
			On12Hour(hDlg);
			SendPSChanged(hDlg);
			break;
		// checked PM - Toggle Display of AM or PM
		case IDC_AMPM_CHECK:
			UpdateAMPMDisplay(hDlg);
			SendPSChanged(hDlg);
			break;
		// checked "display message window"
		case IDC_MSG_ALARM:
			OnMsgAlarm(hDlg,id);
			SendPSChanged(hDlg);
			break;
		// checked repeat alarm or chime
		case IDC_CHIMEALARM:
			CheckDlgButton(hDlg, IDC_REPEATALARM, FALSE);
			EnableDlgItem(hDlg, IDC_REPEATIMES, FALSE);
			EnableDlgItem(hDlg, IDC_SPINTIMES, FALSE);
		case IDC_REPEATALARM:
			if(id==IDC_REPEATALARM) {
				CheckDlgButton(hDlg, IDC_CHIMEALARM, FALSE);
				EnableDlgItem(hDlg, IDC_REPEATIMES, TRUE);
				EnableDlgItem(hDlg, IDC_SPINTIMES, TRUE);
			}
		// checked other checkboxes
		case IDC_REPEATJIHOU:
		case IDC_BLINKALARM:
		case IDC_BLINKJIHOU:
			SendPSChanged(hDlg);
			break;
		}
		return TRUE;}
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			OnApply(hDlg);
			break;
			
		case UDN_DELTAPOS:
			break;
			
		} return TRUE; //--+++--> End Of Case WM_NOTIFY:
		
		// playing sound ended
	case MM_MCINOTIFY:
	case MM_WOM_DONE:
		StopFile(); m_bPlaying = FALSE;
		SendDlgItemMessage(hDlg, IDC_TESTALARM, BM_SETIMAGE, IMAGE_ICON,
						   (LPARAM)g_hIconPlay);
		SendDlgItemMessage(hDlg, IDC_TESTJIHOU, BM_SETIMAGE, IMAGE_ICON,
						   (LPARAM)g_hIconPlay);
		return TRUE;
	}
	return FALSE;
}
/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[1024] = "";
	int i, count;
	HBITMAP hBMPJRPic;
	
	hBMPJRPic = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 64, 80, LR_DEFAULTCOLOR);
	SendDlgItemMessage(hDlg, IDC_BMPJACK, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBMPJRPic);
	
	CBSetItemData(hDlg, IDC_COMBOALARM, CBAddString(hDlg, IDC_COMBOALARM, MyString(IDS_ADDALARM)), 0);
	
	count = GetMyRegLong("", "AlarmNum", 0);
	if(count < 1) count = 0;
	for(i=0; i<count; ++i) {
		alarm_t* pAS = malloc(sizeof(alarm_t));
		ReadAlarmFromReg(pAS, i);
		CBSetItemData(hDlg, IDC_COMBOALARM, CBAddString(hDlg, IDC_COMBOALARM, pAS->dlgmsg.name), pAS);
		if(!i) SetAlarmToDlg(hDlg, pAS);
	}
	if(count > 0) {
		CBSetCurSel(hDlg, IDC_COMBOALARM, 1);
		m_curAlarm = 1;
	} else {
		alarm_t as;
		CBSetCurSel(hDlg, IDC_COMBOALARM, 0);
		m_curAlarm = -1;
		memset(&as, 0, sizeof(as));
		as.hour = 12;
		as.days = 0x7f;
		SetAlarmToDlg(hDlg, &as);
	}
	
	CheckDlgButton(hDlg, IDC_JIHOU,
				   GetMyRegLong("", "Jihou", FALSE));
				   
	GetMyRegStr("", "JihouFile", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FILEJIHOU, s);
	
	CheckDlgButton(hDlg, IDC_REPEATJIHOU,
				   GetMyRegLong("", "JihouRepeat", FALSE));
				   
	CheckDlgButton(hDlg, IDC_BLINKJIHOU,
				   GetMyRegLong("", "JihouBlink", FALSE));
				   
	OnAlermJihou(hDlg, IDC_JIHOU);
	
	SendDlgItemMessage(hDlg, IDC_TESTALARM, BM_SETIMAGE, IMAGE_ICON,
					   (LPARAM)g_hIconPlay);
	OnFileChange(hDlg, IDC_FILEALARM);
	SendDlgItemMessage(hDlg, IDC_TESTJIHOU, BM_SETIMAGE, IMAGE_ICON,
					   (LPARAM)g_hIconPlay);
	OnFileChange(hDlg, IDC_FILEJIHOU);
	
	SendDlgItemMessage(hDlg, IDC_DELALARM, BM_SETIMAGE, IMAGE_ICON,
					   (LPARAM)g_hIconDel);
					   
	m_bPlaying = 0;
}
/*------------------------------------------------
  deinitialize
--------------------------------------------------*/
void OnDeinit(HWND hDlg)
{
	HWND combo=GetDlgItem(hDlg,IDC_COMBOALARM);
	int count=ComboBox_GetCount(hDlg);
	if(m_bPlaying)
		StopFile();
	m_bPlaying=0;
	for(; count>0; ){ // free memory
		free((void*)ComboBox_GetItemData(combo,--count));
		ComboBox_DeleteString(combo,count);
	}
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char file[1024];
	int i, count, n_alarm;
	alarm_t* pAS;
	
	n_alarm = 0;
	
	if(m_curAlarm < 0) {
		char name[sizeof(pAS->dlgmsg.name)];
		GetDlgItemText(hDlg, IDC_COMBOALARM, name, sizeof(pAS->dlgmsg.name));
		if(name[0] && IsDlgButtonChecked(hDlg, IDC_ALARM)) {
			pAS = malloc(sizeof(alarm_t));
			if(pAS) {
				GetAlarmFromDlg(hDlg, pAS);
				i = (int)CBAddString(hDlg, IDC_COMBOALARM, pAS->dlgmsg.name);
				CBSetItemData(hDlg, IDC_COMBOALARM, i, pAS);
				m_curAlarm = i;
				CBSetCurSel(hDlg, IDC_COMBOALARM, i);
				EnableDlgItem(hDlg, IDC_DELALARM, TRUE);
			}
		}
	} else {
		pAS=(alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, m_curAlarm);
		if(pAS) GetAlarmFromDlg(hDlg, pAS);
	}
	
	count = (int)CBGetCount(hDlg, IDC_COMBOALARM);
	for(i=0; i<count; ++i) {
		pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, i);
		if(pAS) {
			SaveAlarmToReg(pAS, n_alarm);
			n_alarm++;
		}
	}
	for(i=n_alarm; ; ++i) {
		char subkey[20];
		wsprintf(subkey, "Alarm%d", i + 1);
		if(GetMyRegLong(subkey, "Hour", -1) >= 0)
			DelMyRegKey(subkey);
		else break;
	}
	
	SetMyRegLong("", "AlarmNum", n_alarm);
	
	SetMyRegLong("", "Jihou",
				 IsDlgButtonChecked(hDlg, IDC_JIHOU));
				 
	GetDlgItemText(hDlg, IDC_FILEJIHOU, file, sizeof(file));
	SetMyRegStr("", "JihouFile", file);
	
	SetMyRegLong("", "JihouRepeat",
				 IsDlgButtonChecked(hDlg, IDC_REPEATJIHOU));
	SetMyRegLong("", "JihouBlink",
				 IsDlgButtonChecked(hDlg, IDC_BLINKJIHOU));
				 
	InitAlarm(); // alarm.c
}
//================================================================================================
//------------------------------------+++--> Load Current Alarm Setting From Dialog into Structure:
void GetAlarmFromDlg(HWND hDlg, alarm_t* pAS)   //--------------------------------------+++-->
{
	if(m_curAlarm!=-1){ // update alarm name in combobox
		size_t oldlen=strlen(pAS->dlgmsg.name), newlen;
		char name[sizeof(pAS->dlgmsg.name)];
		newlen=GetDlgItemText(hDlg, IDC_COMBOALARM, name, sizeof(name));
		if(newlen!=oldlen || memcmp(name,pAS->dlgmsg.name,newlen)){
			HWND combo=GetDlgItem(hDlg,IDC_COMBOALARM);
			OutputDebugString("GetAlarmFromDlg new name");
			ComboBox_DeleteString(combo,m_curAlarm);
			ComboBox_InsertString(combo,m_curAlarm,name);
			ComboBox_SetItemData(combo,m_curAlarm,pAS);
			memcpy(pAS->dlgmsg.name,name,sizeof(pAS->dlgmsg.name));
		}
	}else
		GetDlgItemText(hDlg, IDC_COMBOALARM, pAS->dlgmsg.name, sizeof(pAS->dlgmsg.name));
	pAS->bAlarm = (char)IsDlgButtonChecked(hDlg, IDC_ALARM);
	
	pAS->hour = (int)SendDlgItemMessage(hDlg,IDC_SPINHOUR,UDM_GETPOS32,0,0);
	pAS->minute = (int)SendDlgItemMessage(hDlg,IDC_SPINMINUTE,UDM_GETPOS32,0,0);
	pAS->days = m_days;
	
	pAS->iTimes = GetDlgItemInt(hDlg, IDC_REPEATIMES, NULL, FALSE);
	pAS->bHour12 = (char)IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
	pAS->bChimeHr = (char)IsDlgButtonChecked(hDlg, IDC_CHIMEALARM);
	pAS->bRepeat = (char)IsDlgButtonChecked(hDlg, IDC_REPEATALARM);
	pAS->bBlink = (char)IsDlgButtonChecked(hDlg, IDC_BLINKALARM);
	pAS->bPM = (char)IsDlgButtonChecked(hDlg, IDC_AMPM_CHECK);
	
	GetDlgItemText(hDlg, IDC_FILEALARM, pAS->fname, MAX_PATH);
	
	pAS->bDlg = (char)IsDlgButtonChecked(hDlg, IDC_MSG_ALARM);
	GetDlgItemText(hDlg, IDC_JRMSG_TEXT, pAS->dlgmsg.message, sizeof(pAS->dlgmsg.message));
	GetDlgItemText(hDlg, IDC_JR_SETTINGS, pAS->dlgmsg.settings, sizeof(pAS->dlgmsg.settings));
}
//================================================================================================
//-------------------------------------------------+++--> Load Dialog With Settings for This Alarm:
void SetAlarmToDlg(HWND hDlg, alarm_t* pAS)   //--------------------------------------------+++-->
{
	SetDlgItemText(hDlg, IDC_COMBOALARM, pAS->dlgmsg.name);
	CheckDlgButton(hDlg, IDC_ALARM, pAS->bAlarm);
	
	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETRANGE32, 0,23);
	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETPOS32, 0, pAS->hour);
	SendDlgItemMessage(hDlg, IDC_SPINMINUTE, UDM_SETRANGE32, 0,59);
	SendDlgItemMessage(hDlg, IDC_SPINMINUTE, UDM_SETPOS32, 0, pAS->minute);
	SendDlgItemMessage(hDlg, IDC_SPINTIMES, UDM_SETRANGE32, 1,42);
	SendDlgItemMessage(hDlg, IDC_SPINTIMES, UDM_SETPOS32, 0, pAS->iTimes);
	SetDlgItemText(hDlg, IDC_FILEALARM, pAS->fname);
	
	CheckDlgButton(hDlg, IDC_MSG_ALARM, pAS->bDlg);
	SetDlgItemText(hDlg, IDC_JRMSG_TEXT, pAS->dlgmsg.message);
	SetDlgItemText(hDlg, IDC_JR_SETTINGS, pAS->dlgmsg.settings);
	
	CheckDlgButton(hDlg, IDC_12HOURALARM, pAS->bHour12);
	CheckDlgButton(hDlg, IDC_AMPM_CHECK, pAS->bPM);
	CheckDlgButton(hDlg, IDC_CHIMEALARM, pAS->bChimeHr);
	CheckDlgButton(hDlg, IDC_REPEATALARM, pAS->bRepeat);
	CheckDlgButton(hDlg, IDC_BLINKALARM, pAS->bBlink);
	m_days=pAS->days;
	
	On12Hour(hDlg);
	OnFileChange(hDlg, IDC_FILEALARM);
	OnMsgAlarm(hDlg, IDC_MSG_ALARM);
	OnAlermJihou(hDlg, IDC_ALARM);
	UpdateAMPMDisplay(hDlg);
	
	EnableDlgItem(hDlg, IDC_REPEATIMES, pAS->bRepeat);
	EnableDlgItem(hDlg, IDC_SPINTIMES, pAS->bRepeat);
}
/*------------------------------------------------
   selected an alarm name by combobox
--------------------------------------------------*/
void OnChangeAlarm(HWND hDlg)
{
	alarm_t* pAS;
	int index;
	
	index = (int)CBGetCurSel(hDlg, IDC_COMBOALARM);
	if(m_curAlarm >= 0 && index == m_curAlarm) return;
	m_bTransition=1; // start transition
	
	if(m_curAlarm < 0) {
		char name[sizeof(pAS->dlgmsg.name)];
		GetDlgItemText(hDlg, IDC_COMBOALARM, name, sizeof(pAS->dlgmsg.name));
		if(name[0] && IsDlgButtonChecked(hDlg, IDC_ALARM)) {
			pAS = malloc(sizeof(alarm_t));
			if(pAS) {
				int index;
				GetAlarmFromDlg(hDlg, pAS);
				index = (int)CBAddString(hDlg, IDC_COMBOALARM, pAS->dlgmsg.name);
				CBSetItemData(hDlg, IDC_COMBOALARM, index, pAS);
				m_curAlarm = index;
			}
		}
	} else {
		pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, m_curAlarm);
		if(pAS) GetAlarmFromDlg(hDlg, pAS);
	}
	
	pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, index);
	if(pAS) {
		SetAlarmToDlg(hDlg, pAS);
		EnableDlgItem(hDlg, IDC_DELALARM, TRUE);
		m_curAlarm = index;
	} else {
		alarm_t as;
		memset(&as, 0, sizeof(as));
		as.hour = 12;
		as.days = 0x7f;
		SetAlarmToDlg(hDlg, &as);
		EnableDlgItem(hDlg, IDC_DELALARM, FALSE);
		m_curAlarm = -1;
	}
	
	m_bTransition=0; // end transition
}
/*------------------------------------------------
  combo box is about to be made visible
--------------------------------------------------*/
void OnDropDownAlarm(HWND hDlg)
{
	alarm_t* pAS;
	char name[sizeof(pAS->dlgmsg.name)];
	
	if(m_curAlarm < 0) return;
	pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, m_curAlarm);
	if(!pAS) return;
	GetDlgItemText(hDlg, IDC_COMBOALARM, name, sizeof(pAS->dlgmsg.name));
	if(strcmp(name, pAS->dlgmsg.name)) {
		strcpy(pAS->dlgmsg.name, name);
		CBDeleteString(hDlg, IDC_COMBOALARM, m_curAlarm);
		(int)CBInsertString(hDlg, IDC_COMBOALARM, m_curAlarm, name);
		CBSetItemData(hDlg, IDC_COMBOALARM, m_curAlarm, pAS);
		CBSetCurSel(hDlg, IDC_COMBOALARM, m_curAlarm);
	}
}

/*------------------------------------------------
  "Day..."
--------------------------------------------------*/
void OnDay(HWND hDlg)
{
	int days=ChooseAlarmDay(hDlg,m_days);
	if(days&ALARMDAY_OKFLAG && (days^ALARMDAY_OKFLAG)!=m_days){
		m_days=days^ALARMDAY_OKFLAG;
		SendPSChanged(hDlg);
	}
}
/*------------------------------------------------
  checked "Enable" or "Play sound every hour"
--------------------------------------------------*/
void OnAlermJihou(HWND hDlg, WORD id)
{
	int cstart, cend, i;
	BOOL enabled;
	
	enabled = IsDlgButtonChecked(hDlg, id);
	
	if(id == IDC_ALARM) {
		cstart = IDC_LABTIMEALARM; cend = IDC_BLINKALARM;
		EnableDlgItem(hDlg, IDC_CHIMEALARM, enabled); // Lazy, I know...
		EnableDlgItem(hDlg, IDC_REPEATIMES, enabled); // Lazy, I know...
		EnableDlgItem(hDlg, IDC_SPINTIMES, enabled);  // Lazy, I know...
		EnableDlgItem(hDlg, IDC_MSG_ALARM, enabled);
		if(enabled) {
			OnMsgAlarm(hDlg, IDC_MSG_ALARM);
		} else {
			ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_HIDE);
			EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), FALSE);
		}
	} else {
		cstart = IDC_LABSOUNDJIHOU; cend = IDC_BLINKJIHOU;
	}
	
	for(i=cstart; i<=cend; ++i)
		EnableDlgItem(hDlg, i, enabled);
	
	if(id == IDC_ALARM){
		OnFileChange(hDlg, IDC_FILEALARM);
		if(enabled) {
			char name[40];
			GetDlgItemText(hDlg, IDC_COMBOALARM, name, sizeof(name));
			if(strcmp(name, MyString(IDS_ADDALARM)) == 0)
				SetDlgItemText(hDlg, IDC_COMBOALARM, "");
			EnableDlgItem(hDlg,IDC_AMPM_CHECK, IsDlgButtonChecked(hDlg,IDC_12HOURALARM));
		}
	}else
		OnFileChange(hDlg, IDC_FILEJIHOU);
}
//-------------------------------------//------------------------------------+++-->
//-----------------------------+++--> Show/Hide the Bouncing Message Window Options:
void OnMsgAlarm(HWND hDlg, WORD id)   //-------------------------------------+++-->
{
	BOOL b; //--+++--> Here Begins the Jack Russel Terrier Window Feature!
	
	b = IsDlgButtonChecked(hDlg, id);
	if(b) {
		ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), TRUE);
	} else {
		ShowWindow(GetDlgItem(hDlg, IDC_BMPJACK), SW_HIDE);
		EnableWindow(GetDlgItem(hDlg, IDCB_MSG_ALARM), FALSE);
	}
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

/*------------------------------------------------
  checked "12 hour"
--------------------------------------------------*/
void On12Hour(HWND hDlg)
{
	int b12h = IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
	int h, u, l;
	char hour[5];
	Edit_GetText(GetDlgItem(hDlg,IDC_HOURALARM),hour,5);
	h=atoi(hour);
	if(h>24) h=24;
	
	u = 23; l = 0;
	if(b12h){ // convert to 12h
		u=12; l=1;
		if(h>=12 && h<24){
			h-=12;
			if(!h) h=12;
		}else{
			if(!h || h==24) h=12;
		}
	}else{ // convert to 24h
		if(IsDlgButtonChecked(hDlg,IDC_AMPM_CHECK)){
			h+=12;
			if(h>=24) h=12;
		}else if(h>=12)
			h=0;
	}
	EnableDlgItem(hDlg,IDC_AMPM_CHECK,b12h);
	
	// set limits to spin controls
	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETRANGE32, l,u);
	SendDlgItemMessage(hDlg, IDC_SPINHOUR, UDM_SETPOS32, 0, h);
}
/*------------------------------------------------
  delete an alarm
--------------------------------------------------*/
void OnDelAlarm(HWND hDlg)
{
	alarm_t* pAS;
	
	if(m_curAlarm < 0) return;
	
	pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, m_curAlarm);
	if(pAS) {
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		CBDeleteString(hDlg, IDC_COMBOALARM, m_curAlarm);
		free(pAS);
		if(m_curAlarm > 0) --m_curAlarm;
		CBSetCurSel(hDlg, IDC_COMBOALARM, m_curAlarm);
		pAS = (alarm_t*)CBGetItemData(hDlg, IDC_COMBOALARM, m_curAlarm);
		if(pAS) SetAlarmToDlg(hDlg, pAS);
		else {
			alarm_t as;
			memset(&as, 0, sizeof(as));
			as.hour = 14;
			as.days = 0x7f;
			SetAlarmToDlg(hDlg, &as);
			EnableDlgItem(hDlg, IDC_DELALARM, FALSE);
			m_curAlarm = -1;
		}
	}
}

/*------------------------------------------------
   file name changed - enable/disable controls
--------------------------------------------------*/
void OnFileChange(HWND hDlg, WORD id)
{
	char fname[MAX_PATH];
	BOOL b = FALSE;
	
	GetDlgItemText(hDlg, id, fname, MAX_PATH);
	if(IsWindowEnabled(GetDlgItem(hDlg, id)))
		b = TRUE;
	EnableDlgItem(hDlg, id + 3, b);
	
	EnableDlgItem(hDlg,
				  (id==IDC_FILEALARM)?IDC_REPEATALARM:IDC_REPEATJIHOU,
				  b?IsMMFile(fname):FALSE);
				  
	if(id == IDC_FILEALARM)
		EnableDlgItem(hDlg, IDC_CHIMEALARM, b?IsMMFile(fname):FALSE);
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg, WORD id)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id - 3, fname, MAX_PATH);
	if(fname[0] == 0) return;
	
	if((HICON)SendDlgItemMessage(hDlg, id, BM_GETIMAGE, IMAGE_ICON, 0)
	   == g_hIconPlay) {
		if(PlayFile(hDlg, fname, 0)) {
			SendDlgItemMessage(hDlg, id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, id), NULL, FALSE);
			m_bPlaying = 1;
		}
	} else {
		StopFile();
		m_bPlaying = 0;
	}
}
//======================================*
// Format 12/24hr Time Text 02:01 Properly
//========================================*
void FormatTimeText(HWND hDlg, WORD idc)
{
	char txt[5];
	char txt2[5];
	int iTxt;
	
	m_bTransition=1; // start transition
	
	if(idc==IDC_MINUTEALARM){
		HWND edit=GetDlgItem(hDlg,IDC_MINUTEALARM);
		Edit_GetText(edit,txt,5);
		iTxt=atoi(txt);
		if(iTxt>59) iTxt=0;
		
		wsprintf(txt2, "%02d", iTxt);
		if(strcmp(txt2,txt)){
			Edit_SetText(edit,txt2);
			Edit_SetSel(edit,2,2);
		}
	}else{
		int b12h=IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
		HWND edit=GetDlgItem(hDlg,IDC_HOURALARM);
		Edit_GetText(edit,txt,5);
		iTxt=atoi(txt);
		if(b12h){
			if(iTxt>12) iTxt=0;
			wsprintf(txt2, "%d", iTxt);
		}else{
			if(iTxt>23) iTxt=0;
			wsprintf(txt2, "%02d", iTxt);
			CheckDlgButton(hDlg,IDC_AMPM_CHECK,iTxt>=12);
		}
		if(strcmp(txt2,txt)){
			Edit_SetText(edit,txt2);
			Edit_SetSel(edit,2,2);
		}
	}
	UpdateAMPMDisplay(hDlg);
	
	m_bTransition=0; // end transition
}
//=============================*
// -- Toggle Display of AM or PM
// ---- Based on CheckBox State
//================================*
void UpdateAMPMDisplay(HWND hDlg)
{
	char time[5];
	int hour,min;
	
	GetDlgItemText(hDlg, IDC_HOURALARM, time, 5);
	hour = atoi(time);
	GetDlgItemText(hDlg, IDC_MINUTEALARM, time, 5);
	min = atoi(time);
	
	if(IsDlgButtonChecked(hDlg,IDC_12HOURALARM)){
		if(IsDlgButtonChecked(hDlg, IDC_AMPM_CHECK)){
			if(!min && hour==12)
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Noon");
			else
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "PM");
		}else{
			if(!min && hour==12)
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Midnight");
			else
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "AM");
		}
	}else{
		if(hour<12){
			if(!min && !hour)
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Midnight");
			else
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "AM");
		}else{
			if(!min && hour==12)
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "Noon");
			else
				SetDlgItemText(hDlg, IDC_AMPM_DISPLAY, "PM");
		}
	}
}
