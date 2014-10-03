//---------------------------------------------------------
//--------------------+++--> pageabout.c - KAZUBON 1997-1998
//---------------------------------------------------------*/
// Modified by Stoic Joker: Wednesday, March 3 2010 - 7:17:33
#include "tclock.h"
#include "../common/version.h"

#ifdef _WIN64
#	define TCLOCK_SUFFIX " x64"
#else
#	define TCLOCK_SUFFIX ""
#endif
#define ABT_TITLE "T-Clock Redux" TCLOCK_SUFFIX " - " VER_SHORT_DOTS " build " STR(VER_REVISION)
#define ABT_TCLOCK "T-Clock 2010 is Stoic Joker's rewrite of their code which allows it to run on Windows XP and up. While he removed some of T-Clock's previous functionality. He felt this makes it a more \"Administrator Friendly\" application as it no longer required elevated privileges to run.\n\nT-Clock Redux tries to continue Stoic Joker's efforts."
#define CONF_START "T-Clock Redux" TCLOCK_SUFFIX
#define CONF_START_OLD "Stoic Joker's T-Clock 2010" TCLOCK_SUFFIX

static WNDPROC m_oldLabProc = NULL;
static HCURSOR m_hCurHand = NULL;

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLinkClicked(HWND hDlg, UINT id);

LRESULT CALLBACK LabLinkProc(HWND, UINT, WPARAM, LPARAM);

BOOL GetStartupFile(HWND hDlg,char filename[MAX_PATH]);
void AddStartup(HWND hDlg);
void RemoveStartup(HWND hDlg);
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name);
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
			SetTextColor((HDC)wParam,RGB(0,0,255));
		}
		break;}
	case WM_COMMAND: {
		WORD id, code;
		id = LOWORD(wParam);
		code = HIWORD(wParam);
		if((id==IDC_ABT_WEBuri || id==IDC_ABT_MAILuri) && code==STN_CLICKED) {
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
	if(!m_hCurHand) m_hCurHand = LoadCursor(NULL, IDC_HAND);
	
	m_oldLabProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_ABT_MAILuri), GWL_WNDPROC);
	SetWindowLongPtr(GetDlgItem(hDlg, IDC_ABT_WEBuri), GWL_WNDPROC, (LONG_PTR)LabLinkProc);
	SetWindowLongPtr(GetDlgItem(hDlg, IDC_ABT_MAILuri), GWL_WNDPROC, (LONG_PTR)LabLinkProc);
//==================================================================================

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
	if(id==IDC_ABT_MAILuri) {
		strcpy(str, "mailto:");
		GetDlgItemText(hDlg, id, str+strlen(str), 64);
		strcat(str, "?subject=About "); strcat(str, ABT_TITLE);
	}else
		GetDlgItemText(hDlg, id, str, 64);
	ShellExecute(hDlg, NULL, str, NULL, "", SW_SHOWNORMAL);
}
//================================================================================================
//-------{ Give me a Hand...(Icon) }------+++--> Change Curser to Hand When Mousing Over Web Links:
LRESULT CALLBACK LabLinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   //----+++-->
{
	switch(message) {
	case WM_SETCURSOR:
		SetCursor(m_hCurHand);
		return TRUE;
	}
	return CallWindowProc(m_oldLabProc, hwnd, message, wParam, lParam);
}
//================================================================================
//---------------------------+++--> Does startup file exist? Also creates filename:
BOOL GetStartupFile(HWND hDlg,char filename[MAX_PATH]){   //-------------------------+++-->
	size_t offset;
	if(SHGetFolderPath(hDlg,CSIDL_STARTUP,NULL,SHGFP_TYPE_CURRENT,filename)!=S_OK){
		return 0;
	}
	offset=strlen(filename);
	filename[offset]='\\';
	filename[offset+1]='\0'; // old Stoic Joker link
	strcat(filename,CONF_START_OLD);
	strcat(filename,".lnk");
	if(PathFileExists(filename))
		return 1;
	filename[offset+1]='\0'; // new name
	strcat(filename,CONF_START);
	strcat(filename,".lnk");
	if(PathFileExists(filename))
		return 1;
	return 0;
}
//================================================================================================
//----------------------------------------+++--> Remove Launch T-Clock on Windows Startup ShortCut:
void RemoveStartup(HWND hDlg)   //----------------------------------------------------------+++-->
{
	char path[MAX_PATH];
	if(!GetStartupFile(hDlg,path))
		return;
	DeleteFile(path);
}
//======================================
//--+++-->
void AddStartup(HWND hDlg)
{
	char path[MAX_PATH], myexe[MAX_PATH];
	if(GetStartupFile(hDlg,path) || !*path)
		return;
	*strrchr(path,'\\')='\0';
	GetModuleFileName(GetModuleHandle(NULL),myexe,MAX_PATH);
	CreateLink(myexe,path,CONF_START);
}
//==========================
//--+++--> Create Launch T-Clock on Windows Startup ShortCut:
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name)
{
	HRESULT hres;
	IShellLink* psl;
	
	CoInitialize(NULL);
	
	hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);
	if(SUCCEEDED(hres)) {
		IPersistFile* ppf;
		char path[MAX_PATH];
		
		psl->lpVtbl->SetPath(psl, fname);
		psl->lpVtbl->SetDescription(psl, name);
		strcpy(path, fname);
		del_title(path);
		psl->lpVtbl->SetWorkingDirectory(psl, path);
		
		hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf);
		
		if(SUCCEEDED(hres)) {
			WORD wsz[MAX_PATH];
			char lnkfile[MAX_PATH];
			strcpy(lnkfile, dstpath);
			add_title(lnkfile, (char*)name);
			strcat(lnkfile, ".lnk");
			
			MultiByteToWideChar(CP_ACP, 0, lnkfile, -1, wsz, MAX_PATH);
			
			hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
			ppf->lpVtbl->Release(ppf);
		}
		psl->lpVtbl->Release(psl);
	}
	CoUninitialize();
	
	if(SUCCEEDED(hres)) return TRUE;
	else return FALSE;
}
