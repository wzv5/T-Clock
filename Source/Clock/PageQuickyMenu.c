//============================================================================
//  PageQuickyMenu.c  -  Stoic Joker 2006  ====================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
/* Modified by Stoic Joker: Sunday, 03/14/2010 @ 10:48:18AM */

void BrowseForTargetFile(HWND);
void DeleteMenuItem(HWND);
void SaveNewMenuOptions(HWND hDlg);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0)
/*-------------------------------------------------
End edit and go back to Quicky-list
-------------------------------------------------*/
void EndQuickyEdit(HWND hDlg)
{
	HWND hParent=GetParent(hDlg);
	int i;
	for(i=IDC_QMEN_GROUP1; i<=IDC_QMEN_LIST; ++i) {
		ShowDlgItem(hParent,i,1);
	}
	SendMessage(hParent,WM_COMMAND,IDM_QMEM_REFRESH,0);
	SetFocus(GetDlgItem(hParent,IDC_QMEN_LIST));
	DestroyWindow(hDlg);
}
//===================================================================================================
//--------------------------------------+++--> Dialog Procedure for Menu Item Details Dialog Messages:
INT_PTR CALLBACK PageQuickyMenuProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //-++-->
{
	(void)lParam;
	switch(message) {
	case WM_INITDIALOG:{
		int idx=(int)lParam;
		HWND hParent=GetParent(hDlg);
		char tmp[LRG_BUFF];
		SetWindowLongPtr(hDlg,GWLP_USERDATA,idx);
		wsprintf(tmp,"Item #%i",idx+1);
		SetDlgItemText(hDlg,IDC_MID_TASKNUM,tmp);
		ListView_GetItemText(GetDlgItem(hParent,IDC_QMEN_LIST), idx, 1, tmp,sizeof(tmp));
		SetDlgItemText(hDlg,IDC_MID_TARGET,tmp);
		if(!*tmp){ // new item
			EnableDlgItem(hDlg,IDC_MID_DELETE,0);
		}else
			ListView_GetItemText(GetDlgItem(hParent,IDC_QMEN_LIST), idx, 0, tmp,sizeof(tmp));
		SetDlgItemText(hDlg,IDC_MID_MENUTEXT,tmp);
		ListView_GetItemText(GetDlgItem(hParent,IDC_QMEN_LIST), idx, 2, tmp,sizeof(tmp));
		SetDlgItemText(hDlg,IDC_MID_SWITCHES,tmp);
		SetFocus(GetDlgItem(hDlg,IDC_MID_MENUTEXT));
		return TRUE;}
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_MID_TARGETSEL:
			BrowseForTargetFile(hDlg);
			break;
		case IDC_MID_SAVE:
			SaveNewMenuOptions(hDlg);
			break;
		case IDC_MID_DELETE:
			DeleteMenuItem(hDlg);
		case IDC_MID_CANCEL:
			EndQuickyEdit(hDlg);
		}
		return TRUE;
	}
	return FALSE;
}
/*-------------------------------------------------------------
Save the New Menu Item Options - From the Menu Item Details Tab
-------------------------------------------------------------*/
void SaveNewMenuOptions(HWND hDlg)
{
	char key[TNY_BUFF];
	int offset=9;
	char szmText[TNY_BUFF];
	char szmTarget[LRG_BUFF];
	char szmSwitches[LRG_BUFF];
	GetDlgItemText(hDlg, IDC_MID_MENUTEXT, szmText,sizeof(szmText));
	GetDlgItemText(hDlg, IDC_MID_TARGET,   szmTarget,sizeof(szmTarget));
	GetDlgItemText(hDlg, IDC_MID_SWITCHES, szmSwitches,sizeof(szmSwitches));
	memcpy(key,"MenuItem-",offset);
	offset+=wsprintf(key+offset,"%i",GetWindowLong(hDlg,GWLP_USERDATA));
	if((strlen(szmText)) && (strlen(szmTarget))) {
		SetMyRegLong("QuickyMenu\\MenuItems", key, 1);
		
		memcpy(key+offset,"-Text",6);
		SetMyRegStr("QuickyMenu\\MenuItems", key, szmText);
		
		memcpy(key+offset,"-Target",8);
		SetMyRegStr("QuickyMenu\\MenuItems", key, szmTarget);
		
		memcpy(key+offset,"-Switches",10);
		SetMyRegStr("QuickyMenu\\MenuItems", key, szmSwitches);
		
		EndQuickyEdit(hDlg);
	} else {
		DeleteMenuItem(hDlg);
		wsprintf(szmSwitches, "%s%s",
			(!strlen(szmText)?"* Menu text can't be empty!\n":""),
			(!strlen(szmTarget)?"* Target file must be filled!":""));
		MessageBox(0,szmSwitches,"ERROR: Missing Information!",MB_OK|MB_ICONERROR);
	}
}
/*---------------------------------------------------------------------------
-------------- DELETE a Menu Item - From the ListBox Control and the Registry
---------------------------------------------------------------------------*/
void DeleteMenuItem(HWND hDlg)
{
	char key[TNY_BUFF];
	int offset=9;
	memcpy(key,"MenuItem-",offset);
	offset+=wsprintf(key+offset,"%i",GetWindowLong(hDlg,GWLP_USERDATA));
	DelMyReg("QuickyMenu\\MenuItems", key);
	
	memcpy(key+offset,"-Text",6);
	DelMyReg("QuickyMenu\\MenuItems", key);
	
	memcpy(key+offset,"-Target",8);
	DelMyReg("QuickyMenu\\MenuItems", key);
	
	memcpy(key+offset,"-Switches",10);
	DelMyReg("QuickyMenu\\MenuItems", key);
}
//================================================================================================
//-------------------------------------//------+++--> Browse to the Quicky Menu Item's Target File:
void BrowseForTargetFile(HWND hBft)   //----------------------------------------------------+++-->
{
	char szFile[MAX_PATH];
	char* Filters = "Program Files (*.exe)\0*.exe\0" "All Files (*.*)\0*.*\0";
	OPENFILENAME ofn;
	
	ZeroMemory(szFile, MAX_PATH);
	ZeroMemory(&ofn, sizeof(ofn)); // Initialize OPENFILENAME
	
	ofn.lStructSize  = sizeof(ofn);
	ofn.hwndOwner    = hBft;
	ofn.hInstance	 = NULL;
	ofn.lpstrFilter  = Filters;
	ofn.lpstrFile	 = szFile;
	ofn.nMaxFile     = MAX_PATH;
	ofn.lpstrInitialDir = g_mydir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;
	
	if(GetOpenFileName(&ofn)) {
		SetDlgItemText(hBft, IDC_MID_TARGET, szFile);
	}
}
