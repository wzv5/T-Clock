//============================================================================
//  PageQuicky.c  -  Stoic Joker 2006  ========================================
//==============================================================================
#include "tclock.h"
//------------------------------------------------------------------------------
/* Modified by Stoic Joker: Sunday, 03/14/2010 @ 10:48:18AM */

void AddListBoxRows(HWND);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

INT_PTR CALLBACK PageQuickyMenuProc(HWND, UINT, WPARAM, LPARAM); // PageQuickyMenu.c

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)
//================================================================================================
//----------------------------------------+++--> Dialog Procedure for Quicky Menus Dialog Messages:
INT_PTR CALLBACK PageQuickyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //----+++-->
{
	char szText[TNY_BUFF] = {0};
	LVCOLUMN lvCol;
	int iCol = 0;
	
	switch(message) {
		
	case WM_INITDIALOG:
		OnInit(hDlg);
		ListView_SetExtendedListViewStyle(GetDlgItem(hDlg,IDC_QMEN_LIST), LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
		SetXPWindowTheme(GetDlgItem(hDlg,IDC_QMEN_LIST),L"Explorer",NULL);
		
		lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;		 // Load the Column Headers.
		for(iCol = IDS_LIST_TASKNAME; iCol <= IDS_LIST_TASKSWITCHES; iCol++) {
			lvCol.iSubItem = iCol;								   // From the String Table
			lvCol.pszText = szText;							      // Into the Temporary Buffer.
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
			LoadString(0, iCol, szText, sizeof(szText));    // <-- String Loads Here.
			ListView_InsertColumn(GetDlgItem(hDlg,IDC_QMEN_LIST), iCol, &lvCol); // <- Now It's a Column Header
		}
		AddListBoxRows(GetDlgItem(hDlg,IDC_QMEN_LIST));
		return TRUE;
		
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
			CreateDialogParam(0,MAKEINTRESOURCE(IDD_QUICKY_ADD),hDlg,PageQuickyMenuProc,item); // initializes and kills itself
		}else if(noti->code==NM_DBLCLK) {
			NMITEMACTIVATE* itm=(NMITEMACTIVATE*)noti;
			int i;
			if(itm->iItem==-1)
				return TRUE;
			for(i=IDC_QMEN_GROUP1; i<=IDC_QMEN_LIST; ++i) {
				ShowDlgItem(hDlg,i,0);
			}
			CreateDialogParam(0,MAKEINTRESOURCE(IDD_QUICKY_ADD),hDlg,PageQuickyMenuProc,itm->iItem); // initializes and kills itself
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
	CheckDlgButton(hDlg, IDC_QMEN_AUDIO, GetMyRegLong("QuickyMenu", "AudioProperties", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_DISPLAY, GetMyRegLong("QuickyMenu", "DisplayProperties", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_EXITWIN, GetMyRegLong("QuickyMenu", "ExitWindows", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_LAUNCH, GetMyRegLong("QuickyMenu", "QuickyMenu", TRUE));
	CheckDlgButton(hDlg, IDC_QMEN_NET, GetMyRegLong("QuickyMenu", "NetworkDrives", TRUE));
	
}
/*--------------------------------------------------
--------------------- When "Apply" Button is Clicked
--------------------------------------------------*/
void OnApply(HWND hDlg)   /*---------------------*/
{
	SetMyRegLong("QuickyMenu", "DisplayProperties", IsDlgButtonChecked(hDlg, IDC_QMEN_DISPLAY));
	SetMyRegLong("QuickyMenu", "AudioProperties",   IsDlgButtonChecked(hDlg, IDC_QMEN_AUDIO));
	SetMyRegLong("QuickyMenu", "NetworkDrives",     IsDlgButtonChecked(hDlg, IDC_QMEN_NET));
	SetMyRegLong("QuickyMenu", "ExitWindows",       IsDlgButtonChecked(hDlg, IDC_QMEN_EXITWIN));
	SetMyRegLong("QuickyMenu", "QuickyMenu",        IsDlgButtonChecked(hDlg, IDC_QMEN_LAUNCH));
}
//================================================================================================
//------------------------------+++--> Populate ListView Control With Currently Configured Options:
void AddListBoxRows(HWND hList)   //--------------------------------------------------------+++-->
{
	LVITEM lvItem;
	char key[TNY_BUFF];
	int offset=9;
	int idx;
	ListView_DeleteAllItems(hList); // Clear ListView Control (Refresh Function)
	
	memcpy(key,"MenuItem-",offset);
	for(idx=12; idx--; ) {
		char szValue[LRG_BUFF];
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = 0;
		lvItem.pszText = szValue;
		lvItem.iSubItem = 0;
		offset=9+wsprintf(key+9,"%i",idx);
		memcpy(key+offset,"-Text",6);
		GetMyRegStr("QuickyMenu\\MenuItems", key, szValue, sizeof(szValue), "-");
		ListView_InsertItem(hList, &lvItem);
		
		lvItem.iSubItem = 1;
		memcpy(key+offset,"-Target",8);
		GetMyRegStr("QuickyMenu\\MenuItems", key, szValue, sizeof(szValue), "");
		ListView_SetItem(hList, &lvItem);
		
		lvItem.iSubItem = 2;
		memcpy(key+offset,"-Switches",10);
		GetMyRegStr("QuickyMenu\\MenuItems", key, szValue, sizeof(szValue), "");
		ListView_SetItem(hList, &lvItem);
	}
}
