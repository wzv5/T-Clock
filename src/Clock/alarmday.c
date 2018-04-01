/*-------------------------------------------
  alarmday.c - Kazubon 1999
  dialog to set days for alarm
---------------------------------------------*/
#include "tclock.h"

INT_PTR CALLBACK Window_AlarmDaySelectDlg(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg, unsigned days);
static void OnOK(HWND hDlg);
static void OnEveryDay(HWND hDlg);

//=================================================*
// ----------------------------- Create Dialog Window
//===================================================*
int ChooseAlarmDay(HWND hDlg, unsigned days)
{
	return (unsigned)DialogBoxParam(0, MAKEINTRESOURCE(IDD_ALARMDAY), hDlg, Window_AlarmDaySelectDlg, days);
}
//=================================================*
// --------------------------------- Dialog Procedure
//===================================================*
INT_PTR CALLBACK Window_AlarmDaySelectDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	(void)lParam;
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg,(unsigned)lParam);
		return TRUE;
		
	case WM_COMMAND: {
			WORD id = LOWORD(wParam);
			switch(id) {
			case IDC_ALARMDAY0:
				OnEveryDay(hDlg);
				break;
			case IDOK:
				OnOK(hDlg);
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
			}
			return TRUE;
		}
	}
	return FALSE;
}
//=================================================*
// ------------------------------- Initialize Dialog
//===================================================*
void OnInit(HWND hDlg, unsigned days)
{
	int day, flag;
	wchar_t str[80];
	flag = 1;
	for(day=0; day<7; ++day) {
		GetLocaleInfo(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  LOCALE_SDAYNAME1+day, str, _countof(str));
		SetDlgItemText(hDlg, IDC_ALARMDAY1+day, str);
		if(days & flag)
			CheckDlgButton(hDlg, IDC_ALARMDAY1+day, 1);
		flag <<= 1;
	}
	
	if(!days || days == DAYF_EVERYDAY_BITMASK) {
		CheckDlgButton(hDlg, IDC_ALARMDAY0, 1);
		OnEveryDay(hDlg);
	}
}
//=================================================*
// ------------ Retrieve Settings When OK is Clicked
//===================================================*
void OnOK(HWND hDlg)
{
	int ret=0;
	int i, dflag=1; 
	for(i=0; i<7; ++i) {
		if(IsDlgButtonChecked(hDlg, IDC_ALARMDAY1 + i))
			ret|=dflag;
		dflag<<=1;
	}
	if(!ret)
		ret = DAYF_EVERYDAY_BITMASK;
	EndDialog(hDlg,ret|ALARMDAY_OKFLAG);
}
//=================================================*
// ------------------------ If Every Day is Selected
//===================================================*
void OnEveryDay(HWND hDlg)
{
	int i;
	
	if(IsDlgButtonChecked(hDlg, IDC_ALARMDAY0)) {
		for(i = 0; i < 7; i++) {
			CheckDlgButton(hDlg, IDC_ALARMDAY1 + i, TRUE);
			EnableDlgItem(hDlg, IDC_ALARMDAY1+i, FALSE);
		}
	} else {
		for(i = 0; i < 7; i++) {
			CheckDlgButton(hDlg, IDC_ALARMDAY1 + i, FALSE);
			EnableDlgItem(hDlg, IDC_ALARMDAY1+i, TRUE);
		}
	}
}
