/*-------------------------------------------
  alarmday.c - Kazubon 1999
  dialog to set days for alarm
---------------------------------------------*/
#include "tclock.h"

INT_PTR CALLBACK AlarmDayProc(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg, unsigned days);
static void OnOK(HWND hDlg);
static void OnEveryDay(HWND hDlg);

//=================================================*
// ----------------------------- Create Dialog Window
//===================================================*
int ChooseAlarmDay(HWND hDlg, unsigned days)
{
	return (unsigned)DialogBoxParam(0, MAKEINTRESOURCE(IDD_ALARMDAY), hDlg, AlarmDayProc, days);
}
//=================================================*
// --------------------------------- Dialog Procedure
//===================================================*
INT_PTR CALLBACK AlarmDayProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
				EndDialog(hDlg,id);
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
	int i, f;
	char s[80];
	HFONT hfont;
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	f = 1;
	for(i = 0; i < 7; i++) {
		if(hfont)
			SendDlgItemMessage(hDlg, IDC_ALARMDAY1 + i,
							   WM_SETFONT, (WPARAM)hfont, 0);
							   
		GetLocaleInfo(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  LOCALE_SDAYNAME1+i, s, 80);
		SetDlgItemText(hDlg, IDC_ALARMDAY1 + i, s);
		if(days & f)
			CheckDlgButton(hDlg, IDC_ALARMDAY1 + i, TRUE);
		f = f << 1;
	}
	
	if((days & 0x7f) == 0x7f) {
		CheckDlgButton(hDlg, IDC_ALARMDAY0, TRUE);
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
