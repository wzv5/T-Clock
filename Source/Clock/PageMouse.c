/*--------------------------------------------------------
//-------------------+++--> pagemouse.c - KAZUBON 1997-1998
//-------------------------------------------------------*/
// Modified by Stoic Joker: Saturday, March 6, 2010 - 8:11:17pm
#include "tclock.h"

static void OnInit(HWND hDlg,HWND* hList);
static void OnApply(HWND hDlg);
static void OnDestroy();
static void OnMouseFileChange(HWND hDlg);
static void OnSansho(HWND hDlg, WORD id);
static void InitMouseFuncList(HWND hDlg);

extern const char g_reg_mouse[];

static const char* g_mouseButton[]={
	"Left",
	"Right",
	"Middle",
	"Button 4",
	"Button 5",
};
static const int g_mouseButtonCount=sizeof(g_mouseButton)/sizeof(char*);
//#define g_mouseButtonCount (sizeof(g_mouseButton)/sizeof(char*))
static const char* g_mouseClick[]={
	"Single",
	"Double",
};
//static const int g_mouseClickCount=sizeof(g_mouseClick)/sizeof(char*);
#define g_mouseClickCount (sizeof(g_mouseClick)/sizeof(char*))
typedef struct{
	const int id;
	const char* name;
} action_t;
static const action_t g_mouseAction[]={
//	{MOUSEFUNC_NONE,"<unknown>"},
	{MOUSEFUNC_TIMER,"Timer"},
	{IDM_TIMEWATCH,"Timer watch"},
	{MOUSEFUNC_CLIPBOARD,"Copy To Clipboard"},
	{MOUSEFUNC_SCREENSAVER,"Screensaver"},
	{MOUSEFUNC_SHOWCALENDER,"Calendar"},
	{MOUSEFUNC_SHOWPROPERTY,"T-Clock Properties"},
	{IDM_STOPWATCH,"Stopwatch"},
	{IDM_PROP_ALARM,"Alarms"},
	{IDM_FWD_DATETIME,"Adjust date/time"},
	{IDM_DATETIME_EX,"Adjust date/time ex"},
	{IDM_FWD_STACKED,"Show windows stacked"},
	{IDM_FWD_SIDEBYSIDE,"Show windows side by side"},
	{IDM_FWD_UNDO,"Undo last window change"},
};
static const int g_mouseActionCount=sizeof(g_mouseAction)/sizeof(action_t);
//#define g_mouseActionCount (sizeof(g_mouseAction)/sizeof(action_t))

//----------------------+++--> Mouse Click Date Configuration,
typedef struct { //--+++--> Manipulation, & Storage Structure.
	int func[g_mouseClickCount];
	char format[g_mouseClickCount][256];
	char fname[g_mouseClickCount][256];
} CLICKDATA;
static CLICKDATA* pData=NULL;

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0)

//========================================================================================
//------------------------------------------------------+++--> Update UI, listview control:
static void UpdateUIList(HWND hDlg, HWND hList, int selButton, int selClick)   //---+++-->
{
	LVITEM lvItem; // ListView item		Mouse Buttons:
	int button;  // mouse button				0=>Left, 1=>Right, 2=>Middle, 
	int click; // click count					3=>XButton 1, 4=>XButton 2
	(void)hDlg;
	lvItem.mask=LVIF_TEXT;
	lvItem.iItem=0;
	ListView_DeleteAllItems(hList); // cleanup ListView
	/*
	if(GetMyRegLong(g_reg_mouse,"DropFiles",0)){
		char szApp[LRG_BUFF];
		lvItem.iSubItem=0;
		lvItem.pszText="Drag";
		ListView_InsertItem(hList, &lvItem);
		
		++lvItem.iSubItem;
		lvItem.pszText="DropFiles";
		ListView_SetItem(hList,&lvItem);
		
		++lvItem.iSubItem;
		lvItem.pszText=szApp;
		GetDlgItemText(hDlg,IDC_DROPFILES,szApp,LRG_BUFF);
		ListView_SetItem(hList,&lvItem);
		
		++lvItem.iSubItem;
		GetDlgItemText(hDlg,IDC_DROPFILESAPP,szApp,LRG_BUFF);
		ListView_SetItem(hList,&lvItem);
		++lvItem.iItem;
	}// */
	for(button=0; button<g_mouseButtonCount; ++button){
		if(button==1) continue; // We're Skipping the Right Mouse Button
		for(click=0; click<g_mouseClickCount; ++click){
			int func=pData[button].func[click];
			if(func){
				lvItem.iSubItem=0;
				lvItem.pszText=(char*)g_mouseButton[button];// we set it, so it's "const"
				ListView_InsertItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				lvItem.pszText=(char*)g_mouseClick[click];
				ListView_SetItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				lvItem.pszText=(char*)"<unknown>";
				{int iter; for(iter=0; iter<g_mouseActionCount; ++iter){
					if(func!=g_mouseAction[iter].id) continue;
					lvItem.pszText=(char*)g_mouseAction[iter].name;
					break;
				}}
				ListView_SetItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				if(func==MOUSEFUNC_CLIPBOARD)
					lvItem.pszText=pData[button].format[click];
				else
					lvItem.pszText="";
				ListView_SetItem(hList,&lvItem);
				if(button==selButton && click==selClick)
					ListView_SetItemState(hList,lvItem.iItem,LVIS_FOCUSED|LVIS_SELECTED,0x0F);
				++lvItem.iItem; // increment item index
			}
		} //- end of click count for
	} //-//- end of mouse button for
}
//====================================================================================================
//----------------------------------------------+++--> Update UI controls: combo boxes and radiobutton:
static void UpdateUIControls(HWND hDlg, HWND hList, int button, int click, int type)   //-------+++-->
{
	int iter;
	int func;
	if(button==-1){
		button=(int)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
		if(button>0) ++button; // skip right mouse (relative)
	}else{
		CBSetCurSel(hDlg,IDC_MOUSEBUTTON,(button>1?button-1:button));
		if(click==-1){
			for(click=0; click<g_mouseClickCount && !pData[button].func[click]; ++click);
		}
	}
	if(click==-1){
		for(click=0; click<g_mouseClickCount && !IsDlgButtonChecked(hDlg,IDC_RADSINGLE+click); ++click);
	}else
		CheckRadioButton(hDlg,IDC_RADSINGLE,IDC_RADDOUBLE,IDC_RADSINGLE+click);
	if(click==g_mouseClickCount)
		click=0;
	if(type==1){
		func=(int)CBGetItemData(hDlg,IDC_MOUSEFUNC,CBGetCurSel(hDlg,IDC_MOUSEFUNC));
		pData[button].func[click]=func;
	}else{
		func=pData[button].func[click];
		for(iter=0; iter<g_mouseActionCount+1; ++iter) {
			if(func==CBGetItemData(hDlg,IDC_MOUSEFUNC,iter)) {
				CBSetCurSel(hDlg,IDC_MOUSEFUNC,iter);
				break;
			}
		}
	}
	if(type!=2)
		UpdateUIList(hDlg,hList,button,click); // little recursion here, will call UpdateUIControls later on selection change
	EnableWindow(GetDlgItem(hDlg,IDC_MOUSEFILE),(func==MOUSEFUNC_CLIPBOARD));
	EnableWindow(GetDlgItem(hDlg,IDC_LABMOUSEFILE),(func==MOUSEFUNC_CLIPBOARD));
	if(func==MOUSEFUNC_CLIPBOARD){
		if(!*pData[button].format[click])
			GetMyRegStr("Format","Format",pData[button].format[click],LRG_BUFF,"");
		SetDlgItemText(hDlg,IDC_MOUSEFILE,pData[button].format[click]);
	}
}
//================================================================================================
//-------------------------------------------+++--> Dialog Procedure for Mouse Tab Dialog Messages:
BOOL CALLBACK PageMouseProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //-----+++-->
{
	static HWND g_hList=NULL;
	switch(message){
	case WM_INITDIALOG:{
		OnInit(hDlg,&g_hList);
		return TRUE;}
	case WM_COMMAND:{
		WORD id=LOWORD(wParam);
		WORD code=HIWORD(wParam);
		/// "Drop files"
		if(id == IDC_DROPFILES && code == CBN_SELCHANGE){
			int iter, sel;
			sel=(int)CBGetCurSel(hDlg,IDC_DROPFILES);
			SetDlgItemText(hDlg, IDC_LABDROPFILESAPP, MyString(sel>=3?IDS_LABFOLDER:IDS_LABPROGRAM));
			for(iter=IDC_LABDROPFILESAPP; iter<=IDC_DROPFILESAPPSANSHO; ++iter)
				EnableWindow(GetDlgItem(hDlg,iter),(sel>=2 && sel<=4));
			UpdateUIControls(hDlg,g_hList,-1,-1,0);
			g_bApplyClock=TRUE; SendPSChanged(hDlg);
		}else if(id==IDC_DROPFILESAPP && code==EN_CHANGE)
			SendPSChanged(hDlg);
		/// non-functioning drag&drop handler (at least on Win8)
		else if(id == IDC_DROPFILESAPPSANSHO)
			OnSansho(hDlg, id);
		///  button
		else if(id == IDC_MOUSEBUTTON && code == CBN_SELCHANGE){
			int button=(int)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
			if(button>0) ++button; // skip right mouse (relative)
			UpdateUIControls(hDlg,g_hList,button,-1,0);
		}
		/// clicks
		else if(id >= IDC_RADSINGLE && id <= IDC_RADDOUBLE){
			UpdateUIControls(hDlg,g_hList,-1,id-IDC_RADSINGLE,0);
		}
		///  Mouse Function
		else if(id == IDC_MOUSEFUNC && code == CBN_SELCHANGE){
			UpdateUIControls(hDlg,g_hList,-1,-1,1);
			SendPSChanged(hDlg);
		}
		/// Mouse Function - File
		else if(id == IDC_MOUSEFILE && code==EN_CHANGE){
			int click;
			int button=(int)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
			if(button>0) ++button; // skip right mouse (relative)
			for(click=0; click<g_mouseClickCount && !IsDlgButtonChecked(hDlg,IDC_RADSINGLE+click); ++click);
			if(click<g_mouseClickCount){
				if(CBGetItemData(hDlg,IDC_MOUSEFUNC,CBGetCurSel(hDlg,IDC_MOUSEFUNC)==MOUSEFUNC_CLIPBOARD))
					GetDlgItemText(hDlg,IDC_MOUSEFILE,pData[button].format[click],256);
//				UpdateUIControls(hDlg,g_hList,-1,-1,0); // recursion since it updates selection which in place updates list editbox
				SendPSChanged(hDlg);
			}
		}else if((id==IDC_TOOLTIP && code==EN_CHANGE) || id==IDCB_TOOLTIP){
			if(id==IDCB_TOOLTIP) EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
			g_bApplyClock=TRUE; SendPSChanged(hDlg);
		}
		return TRUE;}
	case WM_NOTIFY:{
		NMLISTVIEW* itm=(NMLISTVIEW*)lParam;
		switch(itm->hdr.code) {
		case PSN_APPLY:
			OnApply(hDlg);
			break;
		case LVN_ITEMCHANGED:
			if(itm->uNewState&LVIS_SELECTED){
				char szButtonBuf[TNY_BUFF];
				char szClickBuf[TNY_BUFF];
				char* szButton;
				int button;
				int click;
				LVITEM lvItem;
				lvItem.mask=LVIF_TEXT;
				lvItem.iItem=itm->iItem;
				lvItem.cchTextMax=TNY_BUFF;
				lvItem.iSubItem=0;
				lvItem.pszText=szButtonBuf;
				ListView_GetItem(g_hList,&lvItem);
				szButton=lvItem.pszText;
				lvItem.iSubItem=1;
				lvItem.pszText=szClickBuf;
				ListView_GetItem(g_hList,&lvItem);
				for(button=0; button<g_mouseButtonCount; ++button){
					if(button==1) continue; // We're Skipping the Right Mouse Button
					if(strcmp(g_mouseButton[button],szButton)) continue;
					for(click=0; click<g_mouseClickCount; ++click){
						if(strcmp(g_mouseClick[click],lvItem.pszText)) continue;
						UpdateUIControls(hDlg,g_hList,button,click,2);
						button=g_mouseButtonCount;
						break;
					}
				}
			}
			break;
		}
		return TRUE;}
	case WM_DESTROY:
		OnDestroy(hDlg);
		DestroyWindow(hDlg);
		return TRUE;
	}
	return FALSE;
}
//================================================================================================
//------------------------//----------------------------++--> Initialize Mouse Tab Dialog Controls:
void OnInit(HWND hDlg,HWND* hList)   //-----------------------------------------------------+++-->
{
	char entry[3+4];
	char buf[LRG_BUFF];
	int button, click;
	LVCOLUMN lvCol;
	HFONT hfont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont){
		SendDlgItemMessage(hDlg,IDC_DROPFILESAPP,WM_SETFONT,(WPARAM)hfont,0);
		SendDlgItemMessage(hDlg,IDC_MOUSEFILE,WM_SETFONT,(WPARAM)hfont,0);
		SendDlgItemMessage(hDlg,IDC_TOOLTIP,WM_SETFONT,(WPARAM)hfont,0);
	}
	/// setup basic controls
	for(click=IDS_NONE; click<=IDS_MOVETO; ++click)
		CBAddString(hDlg,IDC_DROPFILES,MyString(click));
	CBSetCurSel(hDlg,IDC_DROPFILES,GetMyRegLong(g_reg_mouse,"DropFiles",0));
	PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_DROPFILES,CBN_SELCHANGE),0); // update IDC_DROPFILES related
	GetMyRegStr(g_reg_mouse,"DropFilesApp",buf,256,"");
	SetDlgItemText(hDlg, IDC_DROPFILESAPP,buf);
	/// read mouse click settings
	entry[2]='\0';
	pData=malloc(sizeof(CLICKDATA)*g_mouseButtonCount);
	for(button=0; button<g_mouseButtonCount; ++button) {
		if(button==1) continue; // skip right mouse
		for(click=0; click<g_mouseClickCount; ++click) {
			entry[0]='0'+(char)button;
			entry[1]='1'+(char)click;
			pData[button].func[click]=GetMyRegLong(g_reg_mouse, entry, MOUSEFUNC_NONE);
			pData[button].format[click][0]=0; // for Clipboard
			pData[button].fname[click][0]=0; // for open file (N/A)
			if(pData[button].func[click]==MOUSEFUNC_CLIPBOARD){
				memcpy(entry+2,"Clip",5);
				GetMyRegStr(g_reg_mouse,entry,pData[button].format[click],256,"");
				entry[2]='\0';
			}
		}
	}
	/// setup mouse click settings controls
	for(button=IDS_LEFTBUTTON; button<=IDS_XBUTTON2;++button)
		CBAddString(hDlg,IDC_MOUSEBUTTON,MyString(button));
	// set mouse functions to combo box
	InitMouseFuncList(hDlg); // Populate Mouse Click Action DropDown Menu
	
	if(!bV7up){
		EnableDlgItem(hDlg, IDCB_TOOLTIP, 0);
		CheckDlgButton(hDlg,IDCB_TOOLTIP, 1);
	}else{
		CheckDlgButton(hDlg,IDCB_TOOLTIP, GetMyRegLongEx("Tooltip","bCustom",0));
		EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	}
	GetMyRegStr("Tooltip","Tooltip",buf,LRG_BUFF,"");
	if(!*buf) memcpy(buf,"\"T-Clock\" LDATE",16);
	SetDlgItemText(hDlg,IDC_TOOLTIP,buf);
	/// setup list view
	*hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|
							LVS_NOSORTHEADER|LVS_REPORT|LVS_SINGLESEL, 17, 117, 430, 160, hDlg, 0, 0, NULL);
	ListView_SetExtendedListViewStyle(*hList,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	SetWindowTheme(*hList,L"Explorer",NULL);
	SetWindowLongPtr(*hList,GWLP_ID,IDC_LIST);
	
	lvCol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvCol.iSubItem=0;
	lvCol.pszText="Button";
	lvCol.fmt=LVCFMT_CENTER;
	lvCol.cx=60;
	ListView_InsertColumn(*hList,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Click Type";
	lvCol.fmt=LVCFMT_CENTER;
	lvCol.cx=70;
	ListView_InsertColumn(*hList,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Action";
	lvCol.fmt=LVCFMT_LEFT;
	lvCol.cx=160;
	ListView_InsertColumn(*hList,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Other";
	lvCol.fmt=LVCFMT_LEFT;
	lvCol.cx=140;
	ListView_InsertColumn(*hList,lvCol.iSubItem,&lvCol);
	ShowWindow(*hList,SW_SHOW);
	/// select first mouse setup and UpdateMouseClickList
	UpdateUIControls(hDlg,*hList,0,-1,0); // pre-select first mouse button
}
//================================================================================================
//-------------------------//-------------------------+++--> Apply (Settings) Button Event Handler:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char buf[LRG_BUFF], entry[3+4];
	int sel, button, click;
	sel = (int)CBGetCurSel(hDlg,IDC_DROPFILES);
	SetMyRegLong("","DropFiles",(sel>0));
	SetMyRegLong(g_reg_mouse,"DropFiles",sel);
	GetDlgItemText(hDlg,IDC_DROPFILESAPP,buf,256);
	SetMyRegStr(g_reg_mouse,"DropFilesApp",buf);
	
	entry[2]='\0';
	for(button=0; button<g_mouseButtonCount; ++button) {
		if(button==1) continue; // skip right mouse
		for(click=0; click<g_mouseClickCount; ++click) {
			entry[0]='0'+(char)button;
			entry[1]='1'+(char)click;
			if(pData[button].func[click])
				SetMyRegLong(g_reg_mouse, entry, pData[button].func[click]);
			else
				DelMyReg(g_reg_mouse, entry);
			if(pData[button].func[click]==MOUSEFUNC_CLIPBOARD) {
				memcpy(entry+2,"Clip",5);
				SetMyRegStr(g_reg_mouse, entry, pData[button].format[click]);
				entry[2]='\0';
			}
		}
	}
	if(bV7up) SetMyRegLong("Tooltip","bCustom",IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	GetDlgItemText(hDlg, IDC_TOOLTIP,buf,256);
	SetMyRegStr("Tooltip","Tooltip",buf);
}
//=======================================================================================
//---------------------------//------------+++--> Free CLICKDATA Structure Memory on Exit:
void OnDestroy()   //--------------------------------------------------------------+++-->
{
	free(pData), pData=NULL;
}
/*--------------------------------------------------
--------------- Handle File Dropped on Clock Options
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	if(id==IDC_DROPFILESAPPSANSHO) {
		if((int)CBGetCurSel(hDlg, IDC_DROPFILES) >= 3) {
			BROWSEINFO bi;
			LPITEMIDLIST pidl;
			memset(&bi, 0, sizeof(BROWSEINFO));
			bi.hwndOwner = hDlg;
			bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_USENEWUI;
			pidl = SHBrowseForFolder(&bi);
			if(pidl) {
				SHGetPathFromIDList(pidl, fname);
				SetDlgItemText(hDlg, id-1, fname);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}
	
	*filter='\0';
	if(id==IDC_DROPFILESAPPSANSHO) {
		str0cat(filter,MyString(IDS_PROGRAMFILE));
		str0cat(filter,"*.exe;*.cmd");
	}
	str0cat(filter,MyString(IDS_ALLFILE));
	str0cat(filter,"*.*");
	
	GetDlgItemText(hDlg, id-1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	SetDlgItemText(hDlg, id-1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}
//================================================================================================
//-----------------------------------//------+++--> Populate the Mouse Click Actions DropDown Menu:
void InitMouseFuncList(HWND hDlg)   //------------------------------------------------------+++-->
{
	int iter;
	CBSetDroppedWidth(hDlg,IDC_MOUSEFUNC,180);
	CBAddString(hDlg,IDC_MOUSEFUNC,MyString(IDS_NONE));
	CBSetItemData(hDlg,IDC_MOUSEFUNC,0,0);
	for(iter=0; iter<g_mouseActionCount; ++iter){
		CBAddString(hDlg,IDC_MOUSEFUNC,g_mouseAction[iter].name);
		CBSetItemData(hDlg,IDC_MOUSEFUNC,iter+1,g_mouseAction[iter].id);
	}
}
