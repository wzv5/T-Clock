/*--------------------------------------------------------
//-------------------+++--> pagemouse.c - KAZUBON 1997-1998
//-------------------------------------------------------*/
// Modified by Stoic Joker: Saturday, March 6, 2010 - 8:11:17pm
#include "tclock.h"
#include <shlobj.h>//SHBrowseForFolder

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy();
static void OnSansho(HWND hDlg, WORD id);
static void InitMouseControls(HWND hDlg);

static const char* m_mouseButton[]={
	"Left",
	"Right",
	"Middle",
	"Button 4",
	"Button 5",
};
static const short m_mouseButtonCount=sizeof(m_mouseButton)/sizeof(char*);
static const char* m_mouseClick[]={
	"Single",
	"Double",
};
//static const int m_mouseClickCount=sizeof(m_mouseClick)/sizeof(char*);
#define m_mouseClickCount (sizeof(m_mouseClick)/sizeof(char*))
typedef struct{
	const int id;
	const char* name;
} action_t;
static const action_t m_mouseAction[]={
//	{MOUSEFUNC_NONE,"<unknown>"},
	{MOUSEFUNC_MENU,"Context Menu"},
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
static const int m_mouseActionCount=sizeof(m_mouseAction)/sizeof(action_t);

//----------------------+++--> Mouse Click Date Configuration,
typedef struct { //--+++--> Manipulation, & Storage Structure.
	int func[m_mouseClickCount];
	char format[m_mouseClickCount][256];
	char fname[m_mouseClickCount][256];
} CLICKDATA;
static CLICKDATA* m_pData=NULL;

static char m_bTransition=0;
static void SendPSChanged(HWND hDlg){
	if(!m_bTransition)
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

//========================================================================================
//------------------------------------------------------+++--> Update UI, listview control:
static void UpdateUIList(HWND hDlg, int selButton, int selClick)   //---+++-->
{
	HWND hList=GetDlgItem(hDlg,IDC_LIST);
	LVITEM lvItem; // ListView item		Mouse Buttons:
	int button;  // mouse button				0=>Left, 1=>Right, 2=>Middle, 
	int click; // click count					3=>XButton 1, 4=>XButton 2
	(void)hDlg;
	lvItem.mask=LVIF_TEXT;
	lvItem.iItem=0;
	ListView_DeleteAllItems(hList); // cleanup ListView
	/*
	if(api.GetInt(REG_MOUSE,"DropFiles",DF_RECYCLE)){
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
	for(button=0; button<m_mouseButtonCount; ++button){
		for(click=0; click<m_mouseClickCount; ++click){
			int func=m_pData[button].func[click];
			if(func){
				lvItem.iSubItem=0;
				lvItem.pszText=(char*)m_mouseButton[button];// we set it, so it's "const"
				ListView_InsertItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				lvItem.pszText=(char*)m_mouseClick[click];
				ListView_SetItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				lvItem.pszText=(char*)"<unknown>";
				#ifdef _DEBUG
				{char* leak=malloc(16);wsprintf(leak,"#%i",func);lvItem.pszText=leak;}
				#endif // _DEBUG
				{int iter; for(iter=0; iter<m_mouseActionCount; ++iter){
					if(func!=m_mouseAction[iter].id) continue;
					lvItem.pszText=(char*)m_mouseAction[iter].name;
					break;
				}}
				ListView_SetItem(hList,&lvItem);
				
				++lvItem.iSubItem;
				if(func==MOUSEFUNC_CLIPBOARD)
					lvItem.pszText=m_pData[button].format[click];
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
static void UpdateUIControls(HWND hDlg, int button, int click, int type)   //-------+++-->
{
	HWND btn_cb = GetDlgItem(hDlg, IDC_MOUSEBUTTON);
	HWND func_cb = GetDlgItem(hDlg, IDC_MOUSEFUNC);
	int func;
	if(m_bTransition) return;
	m_bTransition=1; // start transition
	if(button==-1){
		button=ComboBox_GetCurSel(btn_cb);
	}else{
		ComboBox_SetCurSel(btn_cb,button);
		if(click==-1){
			for(click=0; click<m_mouseClickCount && !m_pData[button].func[click]; ++click);
		}
	}
	if(click==-1){
		for(click=0; click<m_mouseClickCount && !IsDlgButtonChecked(hDlg,IDC_RADSINGLE+click); ++click);
	}
	if(click==m_mouseClickCount)
		click=0;
	CheckRadioButton(hDlg,IDC_RADSINGLE,IDC_RADDOUBLE,IDC_RADSINGLE+click);
	if(type==1){
		func = (int)ComboBox_GetItemData(func_cb,ComboBox_GetCurSel(func_cb));
		m_pData[button].func[click] = func;
	}else{
		int iter;
		func = m_pData[button].func[click];
		for(iter=0; iter<m_mouseActionCount+1; ++iter) {
			if(func==ComboBox_GetItemData(func_cb,iter)) {
				ComboBox_SetCurSel(func_cb,iter);
				break;
			}
		}
	}
	if(type!=2)
		UpdateUIList(hDlg,button,click); // little recursion here, will call UpdateUIControls later on selection change
	EnableDlgItem(hDlg,IDC_MOUSEFILE,(func==MOUSEFUNC_CLIPBOARD));
	EnableDlgItem(hDlg,IDC_LABMOUSEFILE,(func==MOUSEFUNC_CLIPBOARD));
	if(func==MOUSEFUNC_CLIPBOARD){
		if(!*m_pData[button].format[click])
			api.GetStr("Format","Format",m_pData[button].format[click],LRG_BUFF,"");
		SetDlgItemText(hDlg,IDC_MOUSEFILE,m_pData[button].format[click]);
	}
	m_bTransition=0; // end transition
}
//================================================================================================
//-------------------------------------------+++--> Dialog Procedure for Mouse Tab Dialog Messages:
INT_PTR CALLBACK PageMouseProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //-----+++-->
{
	switch(message){
	case WM_INITDIALOG:{
		OnInit(hDlg);
		return TRUE;}
	case WM_DESTROY:
		OnDestroy(hDlg);
		break;
	case WM_COMMAND:{
		HWND control = (HWND)lParam;
		WORD id=LOWORD(wParam);
		WORD code=HIWORD(wParam);
		/// "Drop files"
		if(id == IDC_DROPFILES && code == CBN_SELCHANGE){
			int iter, sel;
			sel = ComboBox_GetCurSel(control);
			SetDlgItemText(hDlg, IDC_LABDROPFILESAPP, MyString(sel>=3?IDS_LABFOLDER:IDS_LABPROGRAM));
			for(iter=IDC_LABDROPFILESAPP; iter<=IDC_DROPFILESAPPSANSHO; ++iter)
				EnableDlgItem(hDlg,iter,(sel>=2 && sel<=4));
			if(!m_bTransition){
				UpdateUIControls(hDlg,-1,-1,0);
				g_bApplyClock=1;
				SendPSChanged(hDlg);
			}
		// drop file path
		}else if(id==IDC_DROPFILESAPP && code==EN_CHANGE){
			g_bApplyClock=1;
			SendPSChanged(hDlg);
		// drag&drop file chooser
		}else if(id == IDC_DROPFILESAPPSANSHO){
			OnSansho(hDlg, id);
		///  button
		}else if(id == IDC_MOUSEBUTTON && code == CBN_SELCHANGE){
			int button=ComboBox_GetCurSel(control);
			UpdateUIControls(hDlg,button,-1,0);
		/// clicks
		}else if(id >= IDC_RADSINGLE && id <= IDC_RADDOUBLE){
			UpdateUIControls(hDlg,-1,id-IDC_RADSINGLE,0);
		///  Mouse Function
		}else if(id == IDC_MOUSEFUNC && code == CBN_SELCHANGE){
			UpdateUIControls(hDlg,-1,-1,1);
			SendPSChanged(hDlg);
		/// Mouse Function - File
		}else if(id == IDC_MOUSEFILE && code==EN_CHANGE){
			HWND func_cb = GetDlgItem(hDlg, IDC_MOUSEFUNC);
			int click;
			int button=ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_MOUSEBUTTON));
			for(click=0; click<m_mouseClickCount && !IsDlgButtonChecked(hDlg,IDC_RADSINGLE+click); ++click);
			if(click<m_mouseClickCount){
				if(ComboBox_GetItemData(func_cb,ComboBox_GetCurSel(func_cb)==MOUSEFUNC_CLIPBOARD))
					ComboBox_GetText(control, m_pData[button].format[click], 256);
				SendPSChanged(hDlg);
			}
		}else if((id==IDC_TOOLTIP && code==EN_CHANGE) || id==IDCB_TOOLTIP){
			if(id==IDCB_TOOLTIP) EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
			if(!m_bTransition){
				g_bApplyClock=1;
				SendPSChanged(hDlg);
			}
		}else if(id==666){
			MessageBox(hDlg,"You need to set at least one \"Context Menu\" action\nYou might be unable to control T-Clock otherwise","Invalid Setting",MB_OK|MB_ICONERROR);
		}
		return TRUE;}
	case WM_NOTIFY:{
		NMLISTVIEW* itm=(NMLISTVIEW*)lParam;
		switch(itm->hdr.code) {
		case PSN_KILLACTIVE:{
			int button,click;
			/// verify settings
			SetWindowLongPtr(hDlg,DWLP_MSGRESULT,1); // prevent close
			for(button=0; button<m_mouseButtonCount; ++button) {
				for(click=0; click<m_mouseClickCount; ++click) {
					if(m_pData[button].func[click]==MOUSEFUNC_MENU) {
						SetWindowLongPtr(hDlg,DWLP_MSGRESULT,0); // allow close
						return TRUE;
					}
				}
			}
			PostMessage(hDlg,WM_COMMAND,666,0); // show error message
			break;}
		case PSN_APPLY:
			OnApply(hDlg);
			break;
		case LVN_ITEMCHANGED:
			if(itm->uNewState&LVIS_SELECTED){
				HWND hList=GetDlgItem(hDlg,IDC_LIST);
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
				ListView_GetItem(hList,&lvItem);
				szButton=lvItem.pszText;
				lvItem.iSubItem=1;
				lvItem.pszText=szClickBuf;
				ListView_GetItem(hList,&lvItem);
				for(button=0; button<m_mouseButtonCount; ++button){
					if(strcmp(m_mouseButton[button],szButton)) continue;
					for(click=0; click<m_mouseClickCount; ++click){
						if(strcmp(m_mouseClick[click],lvItem.pszText)) continue;
						UpdateUIControls(hDlg,button,click,2);
						button=m_mouseButtonCount;
						break;
					}
				}
			}
			break;
		}
		return TRUE;}
	}
	return FALSE;
}
//================================================================================================
//------------------------//----------------------------++--> Initialize Mouse Tab Dialog Controls:
void OnInit(HWND hDlg)   //-----------------------------------------------------+++-->
{
	HWND drop_cb = GetDlgItem(hDlg, IDC_DROPFILES);
	HWND listview = GetDlgItem(hDlg, IDC_LIST);
	char entry[3+4];
	char buf[LRG_BUFF];
	int button, click;
	LVCOLUMN lvCol;
	m_bTransition=1; // start transition
	/// setup basic controls
	for(click=IDS_NONE; click<=IDS_MOVETO; ++click)
		ComboBox_AddString(drop_cb,MyString(click));
	ComboBox_SetCurSel(drop_cb,api.GetInt(REG_MOUSE,"DropFiles",DF_RECYCLE));
	SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_DROPFILES,CBN_SELCHANGE),0); // update IDC_DROPFILES related
	api.GetStr(REG_MOUSE,"DropFilesApp",buf,256,"");
	SetDlgItemText(hDlg, IDC_DROPFILESAPP,buf);
	/// read mouse click settings
	entry[2]='\0';
	m_pData=malloc(sizeof(CLICKDATA)*m_mouseButtonCount);
	for(button=0; button<m_mouseButtonCount; ++button) {
		for(click=0; click<m_mouseClickCount; ++click) {
			entry[0]='0'+(char)button;
			entry[1]='1'+(char)click;
			m_pData[button].func[click]=api.GetInt(REG_MOUSE, entry, MOUSEFUNC_NONE);
			m_pData[button].format[click][0]=0; // for Clipboard
			m_pData[button].fname[click][0]=0; // for open file (N/A)
			if(m_pData[button].func[click]==MOUSEFUNC_CLIPBOARD){
				memcpy(entry+2,"Clip",5);
				api.GetStr(REG_MOUSE,entry,m_pData[button].format[click],256,"");
				entry[2]='\0';
			}
		}
	}
	// set mouse buttons/functions to combo boxes
	InitMouseControls(hDlg); // Populate Mouse Click Action DropDown Menu
	
	if(api.OS < TOS_VISTA){
		EnableDlgItem(hDlg, IDCB_TOOLTIP, 0);
		CheckDlgButton(hDlg,IDCB_TOOLTIP, 1);
	}else{
		CheckDlgButton(hDlg,IDCB_TOOLTIP, api.GetIntEx("Tooltip","bCustom",0));
		EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	}
	api.GetStr("Tooltip","Tooltip",buf,sizeof(buf),"");
	if(!*buf) memcpy(buf,TC_TOOLTIP,sizeof(TC_TOOLTIP));
	SetDlgItemText(hDlg,IDC_TOOLTIP,buf);
	/// setup list view
	ListView_SetExtendedListViewStyle(listview,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
	SetXPWindowTheme(listview,L"Explorer",NULL);
	
	lvCol.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvCol.iSubItem=0;
	lvCol.pszText="Button";
	lvCol.fmt=LVCFMT_CENTER;
	lvCol.cx=60;
	ListView_InsertColumn(listview,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Click type";
	lvCol.fmt=LVCFMT_CENTER;
	lvCol.cx=70;
	ListView_InsertColumn(listview,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Action";
	lvCol.fmt=LVCFMT_LEFT;
	lvCol.cx=160;
	ListView_InsertColumn(listview,lvCol.iSubItem,&lvCol);
	
	++lvCol.iSubItem;
	lvCol.pszText="Other";
	lvCol.fmt=LVCFMT_LEFT;
	lvCol.cx=151;
	ListView_InsertColumn(listview,lvCol.iSubItem,&lvCol);
	ShowWindow(listview,SW_SHOW);
	m_bTransition=0; // end transition
	/// select first mouse setup and UpdateMouseClickList
	UpdateUIControls(hDlg,0,-1,0); // pre-select first mouse button
}
//================================================================================================
//-------------------------//-------------------------+++--> Apply (Settings) Button Event Handler:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char buf[LRG_BUFF], entry[3+4];
	int sel, button, click;
	sel = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_DROPFILES));
	api.SetInt(REG_MOUSE,"DropFiles",sel);
	GetDlgItemText(hDlg,IDC_DROPFILESAPP,buf,256);
	api.SetStr(REG_MOUSE,"DropFilesApp",buf);
	entry[2]='\0';
	for(button=0; button<m_mouseButtonCount; ++button) {
		for(click=0; click<m_mouseClickCount; ++click) {
			entry[0]='0'+(char)button;
			entry[1]='1'+(char)click;
			if(m_pData[button].func[click])
				api.SetInt(REG_MOUSE, entry, m_pData[button].func[click]);
			else
				api.DelValue(REG_MOUSE, entry);
			if(m_pData[button].func[click]==MOUSEFUNC_CLIPBOARD) {
				memcpy(entry+2,"Clip",5);
				api.SetStr(REG_MOUSE, entry, m_pData[button].format[click]);
				entry[2]='\0';
			}
		}
	}
	if(api.OS >= TOS_VISTA) api.SetInt("Tooltip","bCustom",IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	GetDlgItemText(hDlg, IDC_TOOLTIP,buf,256);
	api.SetStr("Tooltip","Tooltip",buf);
}
//=======================================================================================
//---------------------------//------------+++--> Free CLICKDATA Structure Memory on Exit:
void OnDestroy()   //--------------------------------------------------------------+++-->
{
	free(m_pData), m_pData=NULL;
}
/*--------------------------------------------------
--------------- Handle File Dropped on Clock Options
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	if(id==IDC_DROPFILESAPPSANSHO) {
		if(ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_DROPFILES)) >= 3) {
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
	
	filter[0]=filter[1]='\0';
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
void InitMouseControls(HWND hDlg)   //------------------------------------------------------+++-->
{
	HWND btn_cb = GetDlgItem(hDlg, IDC_MOUSEBUTTON);
	HWND func_cb = GetDlgItem(hDlg, IDC_MOUSEFUNC);
	int iter;
	for(iter=0; iter<m_mouseButtonCount;++iter)
		ComboBox_AddString(btn_cb, m_mouseButton[iter]);
	ComboBox_SetDroppedWidth(func_cb, 180);
	ComboBox_AddString(func_cb, MyString(IDS_NONE));
	ComboBox_SetItemData(func_cb, 0, NULL);
	for(iter=0; iter<m_mouseActionCount; ++iter){
		ComboBox_AddString(func_cb, m_mouseAction[iter].name);
		ComboBox_SetItemData(func_cb, iter+1, m_mouseAction[iter].id);
	}
}
void CheckMouseMenu()
{
	short button,click;
	int hasmenu=0;
	union{
		unsigned short entryS;
		char entry[3];
	} u;
	u.entry[2]='\0';
	for(button=0; button<m_mouseButtonCount; ++button) {
		for(click=0; click<m_mouseClickCount; ++click) {
			u.entryS=('0'+button)|(('0'+click)<<8);
			if(api.GetInt(REG_MOUSE,u.entry,0)==MOUSEFUNC_MENU) {
				hasmenu=1;
				button=(short)m_mouseButtonCount; break;
			}
		}
	}
	if(!hasmenu){
		u.entryS='1'|('1'<<8); // right, 1 click
		api.SetInt(REG_MOUSE,u.entry,MOUSEFUNC_MENU);
	}
}
