//---------------------------------------------------------
//--------------------+++--> pageabout.c - KAZUBON 1997-1998
//---------------------------------------------------------*/
// Modified by Stoic Joker: Wednesday, March 3 2010 - 7:17:33
#include "tclock.h"
#include <time.h>
#include "../common/version.h"
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLinkClicked(HWND hDlg, UINT id);

/** \brief Loads or synchronizes updater checkboxes (disable beta checkbox if release isn't set)
 * \param hDlg
 * \param state either \c 0 to load from registry or \c 1 to synchronize
 * \c IDC_UPDATE_BETA with current state of \c IDC_UPDATE_RELEASE */     
static void SetUpdateChecks(HWND hDlg, int state/* = 0*/) {
	if(state == 0) {
		state = api.GetInt(L"",UPDATE_BETA,0) ? 1 : 0;
		CheckDlgButton(hDlg, IDC_UPDATE_BETA, state);
		state = api.GetInt(L"",UPDATE_RELEASE,1) ? 1 : 0;
		CheckDlgButton(hDlg, IDC_UPDATE_RELEASE, state);
	} else {
		state = IsDlgButtonChecked(hDlg, IDC_UPDATE_RELEASE);
	}
	#if !VER_IsReleaseOrHigher() // non-release build, forced beta check
		state = 0;
	#endif
	EnableDlgItem(hDlg, IDC_UPDATE_BETA, state);
}

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
/////////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK Page_About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
#		ifdef LOGGING
		CheckDlgButton(hDlg, IDC_LOGGING, (api.GetInt(L"",L"NoLog",0) != VER_REVISION));
#		endif // LOGGING
		return TRUE;
	case WM_DESTROY:{
		int controlid;
		HFONT hftBold=(HFONT)SendDlgItemMessage(hDlg,IDC_ABT_TITLE,WM_GETFONT,0,0);
		HFONT hftBigger=(HFONT)SendDlgItemMessage(hDlg,IDC_STARTUP,WM_GETFONT,0,0);
		
		// first time update check (firewall warning etc.)
		if(api.GetInt(L"",UPDATE_RELEASE,1) && !api.GetInt64(L"",UPDATE_TIMESTAMP,0)) {
			TimetableAdd(SCHEDID_UPDATE, 0, 0);
		}
		
		SendDlgItemMessage(hDlg,IDC_STARTUP,WM_SETFONT,0,0);
		for(controlid=GROUP_ABT_B_BEGIN; controlid<=GROUP_ABT_B_END; ++controlid){
			SendDlgItemMessage(hDlg,controlid,WM_SETFONT,0,0);
		}
		DeleteObject(hftBold);
		DeleteObject(hftBigger);
		break;}
	case WM_CTLCOLORSTATIC:{
		int id=GetDlgCtrlID((HWND)lParam);
		if(id==IDC_ABT_WEBuri || id==IDC_ABT_MAILuri || id==IDC_ABT_FORUMuri) {
			return LinkControl_OnCtlColorStatic(hDlg, wParam, lParam);
		}
		break;}
	case WM_COMMAND: {
		WORD id = LOWORD(wParam);
		if(id == IDC_UPDATE_CHECK) {
			HWND options = GetParent(hDlg);
			HANDLE proc;
			int ret;
			MSG msg;
			msg.message = 0;
			EnableWindow((HWND)lParam, 0);
			EnableDlgItem(hDlg, IDC_UPDATE_RELEASE, 0);
			EnableDlgItem(hDlg, IDC_UPDATE_BETA, 0);
			api.ShellExecute(NULL, L"misc\\Options", L"-u", options, SW_HIDE, &proc); // SW_MINIMIZE is buggy
			if(proc) {
				for(;;) {
					ret = MsgWaitForMultipleObjectsEx(1, &proc, INFINITE, QS_ALLEVENTS, MWMO_INPUTAVAILABLE);
					if(ret == WAIT_OBJECT_0)
						break;
					while(PeekMessage(&msg,NULL,0,0,PM_REMOVE) && msg.message != WM_QUIT) {
						TranslateDispatchTClockMessage(&msg);
					}
					if(msg.message == WM_QUIT || !IsWindow(options)) {
						if(msg.message == WM_QUIT)
							PostQuitMessage((int)msg.wParam);
						CloseHandle(proc);
						return FALSE;
					}
				}
				CloseHandle(proc);
				SetUpdateChecks(hDlg, 0);
			}
			SetUpdateChecks(hDlg, 1); // might enable IDC_UPDATE_BETA
			EnableDlgItem(hDlg, IDC_UPDATE_RELEASE, 1);
			EnableWindow((HWND)lParam, 1);
			SetForegroundWindow(options);
			return TRUE;
		}else if(id == IDC_ABT_MAILuri) {
			OnLinkClicked(hDlg, id);
			return TRUE;
		}else if(id == IDC_UPDATE_RELEASE) {
			SetUpdateChecks(hDlg, 1);
		}
#		ifdef LOGGING
		else if(id == IDC_LOGGING) {
			if(!IsDlgButtonChecked(hDlg,id))
				api.SetInt(L"", L"NoLog", VER_REVISION);
			else
				api.DelValue(L"", L"NoLog");
			SendMessage(g_hwndClock, CLOCKM_LOGGING_CLEANUP, 0, 0);
			DebugLogFree();
			return TRUE;
		}
#		endif // LOGGING
		SendPSChanged(hDlg);
		return TRUE;}
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY: OnApply(hDlg); break;
		} return TRUE;
	}
	return FALSE;
}
//================================================================================================
//--------------------+++--> Initialize Options dialog & customize T-Clock controls as required:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	int controlid;
	LOGFONT logft;
	HFONT hftBold;
	HFONT hftStartup;
	time_t tt;
	struct tm tm;
	SetDlgItemText(hDlg, IDC_ABT_TITLE, ABT_TITLE);
	SetDlgItemText(hDlg, IDC_ABT_TCLOCK, ABT_TCLOCK);
	
	time(&tt);
	gmtime_r(&tt, &tm);
	if((tm.tm_mon == 11 && tm.tm_mday >= 18)) {
		HWND logo = GetDlgItem(hDlg, IDI_ABOUT_LOGO);
		HICON ico = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_STOICJOKER_XMAS), IMAGE_ICON, 0,0, LR_SHARED);
		Static_SetIcon(logo, ico);
		SetDlgItemText(hDlg, IDC_ABT_StoicJoker, L"Merry");
		SetDlgItemText(hDlg, IDC_ABT_StoicJoker2, L"Xmas");
//		SetDlgItemText(hDlg, IDC_ABT_StoicJoker, L"Happy");
//		SetDlgItemText(hDlg, IDC_ABT_StoicJoker2, L"Holidays");
	} else if(!(tm.tm_sec % 11) && tm.tm_sec) {
		HWND logo = GetDlgItem(hDlg, IDI_ABOUT_LOGO);
		HICON ico = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_STOICJOKER), IMAGE_ICON, 0,0, LR_SHARED);
		Static_SetIcon(logo, ico);
		SetDlgItemText(hDlg, IDC_ABT_StoicJoker, L"Stoic");
		SetDlgItemText(hDlg, IDC_ABT_StoicJoker2, L"Joker");
	}
	
	hftBold = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hftBold, sizeof(logft), &logft);
	logft.lfWeight = FW_BOLD;
	hftBold = CreateFontIndirect(&logft);
	logft.lfHeight = (logft.lfHeight * 140 / 100);
	hftStartup = CreateFontIndirect(&logft);
	
	for(controlid=GROUP_ABT_B_BEGIN; controlid<=GROUP_ABT_B_END; ++controlid){
		SendDlgItemMessage(hDlg,controlid,WM_SETFONT,(WPARAM)hftBold,0);
	}
	
	SetUpdateChecks(hDlg, 0);
	
	SendDlgItemMessage(hDlg,IDC_STARTUP,WM_SETFONT,(WPARAM)hftStartup,0);
	
	LinkControl_Setup(GetDlgItem(hDlg,IDC_ABT_WEBuri), LCF_SIMPLE, NULL);
	LinkControl_Setup(GetDlgItem(hDlg,IDC_ABT_MAILuri), LCF_NOTIFYONLY, NULL);
	LinkControl_Setup(GetDlgItem(hDlg,IDC_ABT_FORUMuri), LCF_SIMPLE, NULL);
	
	CheckDlgButton(hDlg, IDC_STARTUP, IsStartupAdded());
}
/*--------------------------------------------------
  "Apply" button ----------------- IS NOT USED HERE!
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	// to not mess up changes made by "check for updates"
	if(api.GetInt(L"",UPDATE_RELEASE,1)) {
		if(!IsDlgButtonChecked(hDlg,IDC_UPDATE_RELEASE))
			api.SetInt(L"", UPDATE_RELEASE, 0);
	} else {
		if(IsDlgButtonChecked(hDlg,IDC_UPDATE_RELEASE))
			api.SetInt(L"", UPDATE_RELEASE, 1);
	}
	if(api.GetInt(L"",UPDATE_BETA,1)) {
		if(!IsDlgButtonChecked(hDlg,IDC_UPDATE_BETA))
			api.SetInt(L"", UPDATE_BETA, 0);
	} else {
		if(IsDlgButtonChecked(hDlg,IDC_UPDATE_BETA))
			api.SetInt(L"", UPDATE_BETA, 1);
	}
	
	if(IsDlgButtonChecked(hDlg,IDC_STARTUP))
		AddStartup();
	else
		RemoveStartup();
}
/*--------------------------------------------------
 -- IF User Clicks eMail - Fire up their Mail Client
--------------------------------------------------*/
void OnLinkClicked(HWND hDlg, UINT id)
{
	wchar_t str[MAX_PATH];
	size_t len;
	len = swprintf(str, _countof(str), FMT("mailto:"));
	len += GetDlgItemText(hDlg, id, str+len, 64);
	len += swprintf(str+len,  _countof(str)-len, FMT("?subject=About %s (%hs)"), ABT_TITLE, COMPILER_STRING);
	api.Exec(str, NULL, hDlg);
}
