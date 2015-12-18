//============================================================================
//  PageQuicky.c  -  Stoic Joker 2006  ========================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
/* Modified by Stoic Joker: Sunday, 03/14/2010 @ 10:48:18AM */

void AddListBoxRows(HWND);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

INT_PTR CALLBACK Page_QuickyMenu(HWND, UINT, WPARAM, LPARAM); // PageQuickyMenu.c

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
//================================================================================================
//----------------------------------------+++--> Dialog Procedure for Quicky Menus Dialog Messages:
INT_PTR CALLBACK Page_Quicky(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //----+++-->
{
	switch(message) {
	case WM_INITDIALOG:{
		wchar_t szText[TNY_BUFF];
		LVCOLUMN lvCol;
		int iCol = 0;
		
		lvCol.pszText = szText;
		
		OnInit(hDlg);
		ListView_SetExtendedListViewStyle(GetDlgItem(hDlg,IDC_QMEN_LIST), LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
		SetXPWindowTheme(GetDlgItem(hDlg,IDC_QMEN_LIST), L"Explorer", NULL);
		
		lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;		 // Load the Column Headers.
		for(iCol = IDS_LIST_TASKNAME; iCol <= IDS_LIST_TASKSWITCHES; iCol++) {
			lvCol.iSubItem = iCol;								   // From the String Table
			if(iCol == IDS_LIST_TASKNAME) {
				lvCol.cx = 100;		  // Set Column Width in Pixels
				lvCol.fmt = LVCFMT_LEFT; // left-aligned column
			} else if(iCol == IDS_LIST_TASKTARGET) {
				lvCol.cx = 220;		  // Set Column Width in Pixels
				lvCol.fmt = LVCFMT_LEFT; // left-aligned column
			} else if(iCol == IDS_LIST_TASKSWITCHES) {
				lvCol.cx = 105;		  // Set Column Width in Pixels
				lvCol.fmt = LVCFMT_LEFT; // left-aligned column
			}
			LoadString(0, iCol, lvCol.pszText, _countof(szText));    // <-- String Loads Here.
			ListView_InsertColumn(GetDlgItem(hDlg,IDC_QMEN_LIST), iCol, &lvCol); // <- Now It's a Column Header
		}
		AddListBoxRows(GetDlgItem(hDlg,IDC_QMEN_LIST));
		return TRUE;}
		
	case WM_COMMAND: {
		WORD id, code;
		id = LOWORD(wParam);
		code = HIWORD(wParam);
		if((IDC_QMEN_EXITWIN <= id && id <= IDC_QMEN_DISPLAY) &&
		   ((code == BST_CHECKED) || (code == BST_UNCHECKED))) {
			SendPSChanged(hDlg);
		}
		if(id == IDM_QMEM_REFRESH) {
			AddListBoxRows(GetDlgItem(hDlg,IDC_QMEN_LIST));
		}
		return TRUE;}
	case WM_NOTIFY: {
		NMHDR* noti=(NMHDR*)lParam;
		if(noti->code == PSN_APPLY) {
			OnApply(hDlg);
		}else if(noti->code == LVN_KEYDOWN) {
			NMLVKEYDOWN* key=(NMLVKEYDOWN*)noti;
			int item=ListView_GetNextItem(GetDlgItem(hDlg,IDC_QMEN_LIST),-1,LVNI_SELECTED);
			int i;
			if(key->wVKey!=VK_SPACE || item==-1)
				return TRUE;
			for(i=IDC_QMEN_GROUP1; i<=IDC_QMEN_LIST; ++i) {
				ShowDlgItem(hDlg,i,0);
			}
			CreateDialogParam(0, MAKEINTRESOURCE(IDD_QUICKY_ADD), hDlg, Page_QuickyMenu, item); // initializes and kills itself
		}else if(noti->code == NM_DBLCLK) {
			NMITEMACTIVATE* itm=(NMITEMACTIVATE*)noti;
			int i;
			if(itm->iItem==-1)
				return TRUE;
			for(i=IDC_QMEN_GROUP1; i<=IDC_QMEN_LIST; ++i) {
				ShowDlgItem(hDlg,i,0);
			}
			CreateDialogParam(0, MAKEINTRESOURCE(IDD_QUICKY_ADD), hDlg, Page_QuickyMenu, itm->iItem); // initializes and kills itself
		}
		return TRUE;}
	}
	return FALSE;
}
/*--------------------------------------------------
---------- Initialize Quicky Menu Options Dialog Box
--------------------------------------------------*/
static void OnInit(HWND hDlg)   /*---------------*/
{
	CheckDlgButton(hDlg, IDC_QMEN_AUDIO, api.GetInt(L"QuickyMenu", L"AudioProperties", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_DISPLAY, api.GetInt(L"QuickyMenu", L"DisplayProperties", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_EXITWIN, api.GetInt(L"QuickyMenu", L"ExitWindows", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_LAUNCH, api.GetInt(L"QuickyMenu", L"QuickyMenu", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_NET, api.GetInt(L"QuickyMenu", L"NetworkDrives", TRUE));
	
}
/*--------------------------------------------------
--------------------- When "Apply" Button is Clicked
--------------------------------------------------*/
void OnApply(HWND hDlg)   /*---------------------*/
{
	api.SetInt(L"QuickyMenu", L"DisplayProperties", IsDlgButtonChecked(hDlg, IDC_QMEN_DISPLAY));
	api.SetInt(L"QuickyMenu", L"AudioProperties",   IsDlgButtonChecked(hDlg, IDC_QMEN_AUDIO));
	api.SetInt(L"QuickyMenu", L"NetworkDrives",     IsDlgButtonChecked(hDlg, IDC_QMEN_NET));
	api.SetInt(L"QuickyMenu", L"ExitWindows",       IsDlgButtonChecked(hDlg, IDC_QMEN_EXITWIN));
	api.SetInt(L"QuickyMenu", L"QuickyMenu",        IsDlgButtonChecked(hDlg, IDC_QMEN_LAUNCH));
}
//================================================================================================
//------------------------------+++--> Populate ListView Control With Currently Configured Options:
void AddListBoxRows(HWND hList)   //--------------------------------------------------------+++-->
{
	LVITEM lvItem;
	wchar_t key[TNY_BUFF];
	int offset = 9;
	int idx;
	ListView_DeleteAllItems(hList); // Clear ListView Control (Refresh Function)
	
	memcpy(key, L"MenuItem-", offset*sizeof(wchar_t));
	for(idx=12; idx--; ) {
		wchar_t szValue[LRG_BUFF];
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = 0;
		lvItem.pszText = szValue;
		lvItem.iSubItem = 0;
		offset = 9 + wsprintf(key+9, FMT("%i"), idx);
		wcscpy(key+offset, L"-Text");
		api.GetStr(L"QuickyMenu\\MenuItems", key, szValue, _countof(szValue), L"-");
		ListView_InsertItem(hList, &lvItem);
		
		lvItem.iSubItem = 1;
		wcscpy(key+offset, L"-Target");
		api.GetStr(L"QuickyMenu\\MenuItems", key, szValue, _countof(szValue), L"");
		ListView_SetItem(hList, &lvItem);
		
		lvItem.iSubItem = 2;
		wcscpy(key+offset, L"-Switches");
		api.GetStr(L"QuickyMenu\\MenuItems", key, szValue, _countof(szValue), L"");
		ListView_SetItem(hList, &lvItem);
	}
}
