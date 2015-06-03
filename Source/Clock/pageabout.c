//---------------------------------------------------------
//--------------------+++--> pageabout.c - KAZUBON 1997-1998
//---------------------------------------------------------*/
// Modified by Stoic Joker: Wednesday, March 3 2010 - 7:17:33
#include "tclock.h"
#include "../common/version.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLinkClicked(HWND hDlg, UINT id);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
/////////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK PageAboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:{
		int controlid;
		HFONT hftBold=(HFONT)SendDlgItemMessage(hDlg,IDC_ABT_TITLE,WM_GETFONT,0,0);
		HFONT hftBigger=(HFONT)SendDlgItemMessage(hDlg,IDC_STARTUP,WM_GETFONT,0,0);
		SendDlgItemMessage(hDlg,IDC_STARTUP,WM_SETFONT,0,0);
		for(controlid=IDC_ABT_TITLE; controlid<=IDC_ABT_MAIL; ++controlid){
			SendDlgItemMessage(hDlg,controlid,WM_SETFONT,0,0);
		}
		DeleteObject(hftBold);
		DeleteObject(hftBigger);
		break;}
	case WM_CTLCOLORSTATIC:{
		int id=GetDlgCtrlID((HWND)lParam);
		if(id==IDC_ABT_WEBuri || id==IDC_ABT_MAILuri) {
			return LinkControl_OnCtlColorStatic(hDlg, wParam, lParam);
		}
		break;}
	case WM_COMMAND: {
		WORD id, code;
		id = LOWORD(wParam);
		code = HIWORD(wParam);
		if(id==IDC_ABT_MAILuri) {
			OnLinkClicked(hDlg, id);
		}
		if((id==IDC_STARTUP) && ((code==BST_CHECKED) || (code==BST_UNCHECKED))) {
			SendPSChanged(hDlg);
		}
		return TRUE;}
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY: OnApply(hDlg); break;
		} return TRUE;
	}
	return FALSE;
}
//================================================================================================
//--------------------+++--> Initialize Properties Dialog & Customize T-Clock Controls as Required:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	char path[MAX_PATH];
	int controlid;
	LOGFONT logft;
	HFONT hftBold;
	HFONT hftStartup;
	SetDlgItemText(hDlg, IDC_ABT_TITLE, ABT_TITLE);
	SetDlgItemText(hDlg, IDC_ABT_TCLOCK, ABT_TCLOCK);
	
	hftBold = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hftBold, sizeof(logft), &logft);
	logft.lfWeight = FW_BOLD;
	hftBold = CreateFontIndirect(&logft);
	logft.lfHeight=logft.lfHeight*140/100;
	hftStartup = CreateFontIndirect(&logft);
	
	for(controlid=IDC_ABT_TITLE; controlid<=IDC_ABT_MAIL; ++controlid){
		SendDlgItemMessage(hDlg,controlid,WM_SETFONT,(WPARAM)hftBold,0);
	}
	SendDlgItemMessage(hDlg,IDC_STARTUP,WM_SETFONT,(WPARAM)hftStartup,0);
	
	LinkControl_Setup(GetDlgItem(hDlg,IDC_ABT_WEBuri), LCF_SIMPLE, NULL);
	LinkControl_Setup(GetDlgItem(hDlg,IDC_ABT_MAILuri), LCF_NOTIFYONLY, NULL);
	
	CheckDlgButton(hDlg,IDC_STARTUP,GetStartupFile(hDlg,path));
}
/*--------------------------------------------------
  "Apply" button ----------------- IS NOT USED HERE!
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	if(IsDlgButtonChecked(hDlg,IDC_STARTUP))
		AddStartup(hDlg);
	else
		RemoveStartup(hDlg);
}
/*--------------------------------------------------
 -- IF User Clicks eMail - Fire up their Mail Client
--------------------------------------------------*/
void OnLinkClicked(HWND hDlg, UINT id)
{
	char str[128];
	strcpy(str, "mailto:");
	GetDlgItemText(hDlg, id, str+7, 64);
	strcat(str, "?subject=About "); strcat(str, ABT_TITLE);
	api.Exec(str, NULL, hDlg);
}
