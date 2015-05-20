//-------------------------------------------------------------------
//--+++--> PageMisc.c - Stoic Joker: Saturday, 06/05/2010 @ 10:46:02pm
//------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
//================================================================================================
//---------------------------------------------+++--> Dialog Procedure of Miscellaneous Tab Dialog:
INT_PTR CALLBACK PageMiscProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //------+++-->
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
		
	case WM_COMMAND: {
			WORD id, code;
			id = LOWORD(wParam);
			code = HIWORD(wParam);
			
			if(id==IDCB_CLOSECAL) {
				if(IsDlgButtonChecked(hDlg, IDCB_CALTOPMOST))
					CheckDlgButton(hDlg, IDCB_CALTOPMOST, FALSE);
			}else if(id==IDCB_CALTOPMOST) {
				if(IsDlgButtonChecked(hDlg, IDCB_CLOSECAL))
					CheckDlgButton(hDlg, IDCB_CLOSECAL, FALSE);
			}else if(id==IDCB_USECALENDAR) {
				UINT iter,enable=IsDlgButtonChecked(hDlg,IDCB_USECALENDAR);
				for(iter=IDCB_SHOW_DOY; iter<=IDC_CALSTATIC5; ++iter) EnableDlgItem(hDlg,iter,enable);
				SendPSChanged(hDlg);
				return TRUE;
			}
			
			if((id==IDC_FIRSTWEEK&&code==CBN_SELCHANGE) || (id==IDC_CALMONTHS&&code==EN_CHANGE) || (id==IDC_CALMONTHSPAST&&code==EN_CHANGE))
				SendPSChanged(hDlg);
			else if(((id==IDCB_CLOSECAL) ||  // IF Anything Happens to Anything,
				(id==IDCB_SHOWWEEKNUMS) || //--+++--> Send Changed Message.
				(id==IDCB_SHOW_DOY) ||
				(id==IDCB_TRANS2KICONS) ||
				(id==IDCB_MONOFF_ONLOCK) ||
				(id==IDCB_MULTIMON) ||
				(id==IDCB_CALTOPMOST)) && (code==BST_CHECKED||code==BST_UNCHECKED)) {
				SendPSChanged(hDlg);
			}
			
			return TRUE;
		}
		
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			OnApply(hDlg);
		}
		return TRUE;
	}
	return FALSE;
}
//-------+++--> Make Adjustable: HKEY_CURRENT_USER\Control Panel\International\iFirstWeekOfYear
//-------+++--> Data Type: DWORD Possible Valid Values: 0, 1, or 2 - Else it Should Return Fail!
//-------+++--> Purpose: Required for Reading and/or Correcting the Calendar Week Numbers Offset.
//================================================================================================
//--------------------//----------+++--> This is to Access the System Level iFirstWeekOfYear Value:
int GetMySysWeek()   //---------------------------------------------------------------------+++-->
{
	char val[8];
	api.GetSystemStr(HKEY_CURRENT_USER,"Control Panel\\International","iFirstWeekOfYear",val,sizeof(val),"");
	return atoi(val);
}
//================================================================================================
//------------------------------//---------------+++--> Set Value for iFirstWeekOfYear in Registry:
void SetMySysWeek(char* val)   //-----------------------------------------------------------+++-->
{
	HKEY hkey;
	
	if(RegCreateKey(HKEY_CURRENT_USER, "Control Panel\\International", &hkey) == ERROR_SUCCESS) {
		RegSetValueEx(hkey, "iFirstWeekOfYear", 0, REG_SZ, (CONST BYTE*)val, (unsigned)strlen(val));
		RegCloseKey(hkey);
	}
}
//================================================================================================
//--------------------+++--> Initialize Properties Dialog & Customize T-Clock Controls as Required:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	UINT iter;
	if(api.OS >= TOS_VISTA && !api.GetIntEx("Calendar","bCustom",0)){
		for(iter=IDCB_SHOW_DOY; iter<=IDC_CALSTATIC5; ++iter) EnableDlgItem(hDlg,iter,0);
		CheckDlgButton(hDlg,IDCB_USECALENDAR, 0);
	}else CheckDlgButton(hDlg,IDCB_USECALENDAR, 1);
	/// on Calendar defaults change, also update the Calendar itself to stay sync!
	CheckDlgButton(hDlg, IDCB_SHOW_DOY, api.GetIntEx("Calendar","ShowDayOfYear",1));
	CheckDlgButton(hDlg, IDCB_SHOWWEEKNUMS, api.GetIntEx("Calendar","ShowWeekNums",0));
	CheckDlgButton(hDlg, IDCB_CLOSECAL, api.GetIntEx("Calendar","CloseCalendar",1));
	CheckDlgButton(hDlg, IDCB_CALTOPMOST, api.GetIntEx("Calendar","CalendarTopMost",0));
	CheckDlgButton(hDlg, IDCB_TRANS2KICONS, api.GetInt("Desktop","Transparent2kIconText",0));
	CheckDlgButton(hDlg, IDCB_MONOFF_ONLOCK, bMonOffOnLock);
	CheckDlgButton(hDlg, IDCB_MULTIMON, api.GetInt("Desktop","Multimon",1));
	
	SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_SETRANGE32,1,12);
	SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_SETPOS32,0,api.GetInt("Calendar","ViewMonths",3));
	SendDlgItemMessage(hDlg,IDC_CALMONTHPASTSPIN,UDM_SETRANGE32,0,2);
	SendDlgItemMessage(hDlg,IDC_CALMONTHPASTSPIN,UDM_SETPOS32,0,api.GetInt("Calendar","ViewMonthsPast",1));
	
	CBResetContent(hDlg, IDC_FIRSTWEEK);
	CBAddString(hDlg, IDC_FIRSTWEEK, "0");
	CBAddString(hDlg, IDC_FIRSTWEEK, "1");
	CBAddString(hDlg, IDC_FIRSTWEEK, "2");
	CBSetCurSel(hDlg, IDC_FIRSTWEEK, GetMySysWeek());
	
	if(api.OS > TOS_2000) {
		for(iter=IDCB_TRANS2KICONS_GRP; iter<=IDCB_TRANS2KICONS; ++iter)
			EnableDlgItem(hDlg,iter,FALSE);
	}else{
		for(iter=IDCB_MONOFF_ONLOCK_GRP; iter<=IDCB_MONOFF_ONLOCK; ++iter)
			EnableDlgItem(hDlg,iter,FALSE);
	}
	if(api.OS < TOS_WIN8){
		for(iter=IDCB_MULTIMON_GRP; iter<=IDCB_MULTIMON; ++iter)
			EnableDlgItem(hDlg,iter,FALSE);
		if(api.OS < TOS_VISTA){
			EnableDlgItem(hDlg, IDCB_USECALENDAR, FALSE);
		}
	}
}
//================================================================================================
//-------------------------//-----------------------------+++--> Save Current Settings to Registry:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char szWeek[8];
	char bRefresh=((unsigned)api.GetInt("Desktop","Multimon",1) != IsDlgButtonChecked(hDlg,IDCB_MULTIMON));
	
	api.SetInt("Calendar","bCustom", IsDlgButtonChecked(hDlg,IDCB_USECALENDAR));
	api.SetInt("Calendar","CloseCalendar", IsDlgButtonChecked(hDlg,IDCB_CLOSECAL));
	api.SetInt("Calendar","ShowWeekNums", IsDlgButtonChecked(hDlg,IDCB_SHOWWEEKNUMS));
	api.SetInt("Calendar","ShowDayOfYear", IsDlgButtonChecked(hDlg,IDCB_SHOW_DOY));
	api.SetInt("Calendar","CalendarTopMost", IsDlgButtonChecked(hDlg,IDCB_CALTOPMOST));
	api.SetInt("Calendar","ViewMonths", (int)SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_GETPOS32,0,0));
	api.SetInt("Calendar","ViewMonthsPast", (int)SendDlgItemMessage(hDlg,IDC_CALMONTHPASTSPIN,UDM_GETPOS32,0,0));
	api.SetInt("Desktop","Transparent2kIconText", IsDlgButtonChecked(hDlg,IDCB_TRANS2KICONS));
	api.SetInt("Desktop","MonOffOnLock", IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK));
	api.SetInt("Desktop","Multimon", IsDlgButtonChecked(hDlg,IDCB_MULTIMON));
	
	GetDlgItemText(hDlg, IDC_FIRSTWEEK, szWeek, 8);
	SetMySysWeek(szWeek);
	
	if(api.OS >= TOS_XP) { // This feature requires XP+
		BOOL enabled=IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK);
		if(enabled){
			if(!bMonOffOnLock) {
				RegisterSession(g_hwndTClockMain); // Sets bMonOffOnLock to TRUE.
			}
		} else {
			UnregisterSession(g_hwndTClockMain); // Sets bMonOffOnLock to FALSE.
		}
	}
	if(bRefresh){
		SendMessage(g_hwndClock,CLOCKM_REFRESHCLOCK,0,0);
	}
}
