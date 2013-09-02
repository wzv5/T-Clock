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
BOOL CALLBACK PageMiscProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //------+++-->
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
				for(iter=IDC_CALSTATIC1; iter<=IDC_CALSTATIC4; ++iter) EnableDlgItem(hDlg,iter,enable);
				EnableDlgItem(hDlg,IDCB_SHOW_DOY,enable);
				EnableDlgItem(hDlg,IDCB_SHOWWEEKNUMS,enable);
				EnableDlgItem(hDlg,IDCB_CLOSECAL,enable);
				EnableDlgItem(hDlg,IDCB_CALTOPMOST,enable);
				EnableDlgItem(hDlg,IDC_FIRSTWEEK,enable);
				EnableDlgItem(hDlg,IDC_CALMONTHS,enable);
			}
			
			if(((id==IDCB_USECALENDAR) || // IF Anything Happens to Anything,
				(id==IDCB_CLOSECAL) || //--+++--> Send Changed Message.
				(id==IDCB_SHOWWEEKNUMS) ||
				(id==IDCB_TRANS2KICONS) ||
				(id==IDCB_SHOW_DOY) ||
				(id==IDC_CALMONTHS) ||
				(id==IDCB_MONOFF_ONLOCK) ||
				(id==IDCB_CALTOPMOST)) && (code==BST_CHECKED||code==BST_UNCHECKED||code==EN_CHANGE)) {
				SendPSChanged(hDlg);
			}
			if(id == IDC_FIRSTWEEK && code == CBN_SELCHANGE) SendPSChanged(hDlg);
			
			return TRUE;
		}
		
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			OnApply(hDlg);
		}
		return TRUE;
	case WM_DESTROY:
		DestroyWindow(hDlg);
		break;
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
	HKEY hkey;  DWORD size;  char val[8]= {0};
	
	if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\International", &hkey) == ERROR_SUCCESS) {
		size=8; // Thank you Pascal Aloy - For noticing I screwed this up. :-)
		RegQueryValueEx(hkey, "iFirstWeekOfYear", 0, NULL, (LPBYTE)val, &size);
		RegCloseKey(hkey);
	}
	return atoi(val);
}
//================================================================================================
//------------------------------//---------------+++--> Set Value for iFirstWeekOfYear in Registry:
void SetMySysWeek(char* val)   //-----------------------------------------------------------+++-->
{
	HKEY hkey;
	
	if(RegCreateKey(HKEY_CURRENT_USER, "Control Panel\\International", &hkey) == ERROR_SUCCESS) {
		RegSetValueEx(hkey, "iFirstWeekOfYear", 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val));
		RegCloseKey(hkey);
	}
}
//================================================================================================
//--------------------+++--> Initialize Properties Dialog & Customize T-Clock Controls as Required:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	if(!b2000 && !GetMyRegLongEx("Calendar","bCustom",0)){
		UINT iter=IDC_CALSTATIC1;
		for(; iter<=IDC_CALSTATIC4; ++iter) EnableDlgItem(hDlg,iter,0);
		EnableDlgItem(hDlg,IDCB_SHOW_DOY,0);
		EnableDlgItem(hDlg,IDCB_SHOWWEEKNUMS,0);
		EnableDlgItem(hDlg,IDCB_CLOSECAL,0);
		EnableDlgItem(hDlg,IDCB_CALTOPMOST,0);
		EnableDlgItem(hDlg,IDC_FIRSTWEEK,0);
		EnableDlgItem(hDlg,IDC_CALMONTHS,0);
		CheckDlgButton(hDlg,IDCB_USECALENDAR, 0);
	}else CheckDlgButton(hDlg,IDCB_USECALENDAR, 1);
	CheckDlgButton(hDlg, IDCB_CLOSECAL,
				   GetMyRegLongEx("Calendar", "CloseCalendar", FALSE));
	CheckDlgButton(hDlg, IDCB_SHOWWEEKNUMS,
				   GetMyRegLongEx("Calendar", "ShowWeekNums", FALSE));
	CheckDlgButton(hDlg, IDCB_CALTOPMOST,
				   GetMyRegLongEx("Calendar", "CalendarTopMost", FALSE));
	CheckDlgButton(hDlg, IDCB_SHOW_DOY,
				   GetMyRegLongEx("Calendar", "ShowDayOfYear", FALSE));
	
	SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_SETRANGE,0,MAKELONG(1,12));
	SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_SETPOS,0,GetMyRegLongEx("Calendar","ViewMonths",1));
	
	CBResetContent(hDlg, IDC_FIRSTWEEK);
	CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"0");
	CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"1");
	CBAddString(hDlg, IDC_FIRSTWEEK, (LPARAM)"2");
	CBSetCurSel(hDlg, IDC_FIRSTWEEK, GetMySysWeek());
	
	if(b2000) {
		EnableDlgItem(hDlg, IDCB_USECALENDAR, FALSE);
		EnableDlgItem(hDlg, IDCB_MONOFF_ONLOCK, FALSE);
		CheckDlgButton(hDlg, IDCB_TRANS2KICONS, GetMyRegLongEx("Desktop", "Transparent2kIconText", FALSE));
	} else {
		EnableDlgItem(hDlg, IDCB_TRANS2KICONS, FALSE);
		CheckDlgButton(hDlg, IDCB_MONOFF_ONLOCK, bMonOffOnLock);
	}
}
//================================================================================================
//-------------------------//-----------------------------+++--> Save Current Settings to Registry:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char szWeek[8];
	
	SetMyRegLong("Calendar","bCustom", IsDlgButtonChecked(hDlg,IDCB_USECALENDAR));
	SetMyRegLong("Calendar","CloseCalendar", IsDlgButtonChecked(hDlg,IDCB_CLOSECAL));
	SetMyRegLong("Calendar","ShowWeekNums", IsDlgButtonChecked(hDlg,IDCB_SHOWWEEKNUMS));
	SetMyRegLong("Calendar","ShowDayOfYear", IsDlgButtonChecked(hDlg,IDCB_SHOW_DOY));
	SetMyRegLong("Calendar","CalendarTopMost", IsDlgButtonChecked(hDlg,IDCB_CALTOPMOST));
	SetMyRegLong("Calendar", "ViewMonths", SendDlgItemMessage(hDlg,IDC_CALMONTHSPIN,UDM_GETPOS,0,0));
	SetMyRegLong("Desktop","Transparent2kIconText", IsDlgButtonChecked(hDlg,IDCB_TRANS2KICONS));
	
	GetDlgItemText(hDlg, IDC_FIRSTWEEK, szWeek, 8);
	SetMySysWeek(szWeek);
	
	if(!b2000) { // This Feature is Not For Windows 2000, It's Only XP and Above!
		if((!bMonOffOnLock) &&(IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK))) {
			SetMyRegLong("Desktop", "MonOffOnLock", TRUE);
			RegisterSession(g_hWnd); // Sets bMonOffOnLock to TRUE.
		} else if((bMonOffOnLock) &&(IsDlgButtonChecked(hDlg, IDCB_MONOFF_ONLOCK))) {
			//RegisterSession() Already Set bMonOffOnLock So There is Nothing to do.
		} else {
			SetMyRegLong("Desktop", "MonOffOnLock", FALSE);
			UnregisterSession(g_hWnd); // Sets bMonOffOnLock to FALSE.
		}
	}
}
