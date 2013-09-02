/*--------------------------------------------------------
//-------------------+++--> pagemouse.c - KAZUBON 1997-1998
//-------------------------------------------------------*/
// Modified by Stoic Joker: Saturday, March 6, 2010 - 8:11:17pm
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnMouseFunc(HWND hDlg);
static void OnMouseButton(HWND hDlg);
static void OnDropFilesChange(HWND hDlg);
static void OnMouseClickTime(HWND hDlg, int id);
static void OnMouseFileChange(HWND hDlg);
static void OnSansho(HWND hDlg, WORD id);
static void InitMouseFuncList(HWND hDlg);

static char reg_section[] = "Mouse";

#define COL_BUTTON 0 // Mouse Button (Left or Middle)
#define COL_CLKTYP 1 // Click Type (Single or Double)
#define COL_ACTION 2 // Action to Take (Run Program?)
#define COL_OTHERD 3 // Other Format / App Path Data

#define MAX_CLICKS 2 // maximum mouse clicks (single and double only)

//----------------------+++--> Mouse Click Date Configuration,
typedef struct { //--+++--> Manipulation, & Storage Structure.
	BOOL disable;
	int func[MAX_CLICKS];
	char format[MAX_CLICKS][256];
	char fname[MAX_CLICKS][256];
} CLICKDATA;

static CLICKDATA* pData = NULL;

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0)
//================================================================================================
//------------------+++--> Populate ListView Control With Currently Configured Mouse Click Options:
void AddMouseClickSettings(HWND hList, HWND hDlg)   //--------------------------------------+++-->
{
	LVITEM lvItem; // ListView Control Row Identifier				Mouse Buttons:
	int col = 0;  // ListView Control Column to Populate				0 = Left
	int m;  // Mouse (Button Clicked)									1 = Right
	int c; // Click (Number of Times)									2 = Middle
	
	ListView_DeleteAllItems(hList); // Clear ListView Control (Refresh Function)
	for(m=0; m<=4; ++m) {
		if(m==1) continue; // We're Skipping the Right Mouse Button
		for(c=0; c<MAX_CLICKS; ++c) {
			int iBtn;
			iBtn = pData[m].func[c];
			if(iBtn > MOUSEFUNC_NONE) {
				lvItem.iSubItem = COL_BUTTON;
				lvItem.mask = LVIF_TEXT;
				lvItem.iItem = 0;
				
				switch(m){
				case 0: lvItem.pszText = "Left";break;
				case 1: lvItem.pszText = "Right";break;
				case 2: lvItem.pszText = "Middle";break;
				case 3: lvItem.pszText = "Button 4";break;
				case 4: lvItem.pszText = "Button 5";break;
				}
				// FIRST Insert A New Row THEN Populate Its COLUMNS
				ListView_InsertItem(hList, &lvItem);
				
				for(col = COL_BUTTON; col <= COL_OTHERD; col++) {
					lvItem.iSubItem = col;
					switch(col) {
					case COL_CLKTYP:
						if(c == 0) lvItem.pszText = "Single";
						else lvItem.pszText = "Double";
						break;
					case COL_ACTION:
						switch(iBtn) {
						case MOUSEFUNC_TIMER:			// 5
							lvItem.pszText = "Timer";
							break;
						case MOUSEFUNC_CLIPBOARD:		// 6
							lvItem.pszText = "Copy To Clipboard";
							break;
						case MOUSEFUNC_SCREENSAVER:	// 7
							lvItem.pszText = "Screen Save";
							break;
						case MOUSEFUNC_SHOWCALENDER:	// 8
							lvItem.pszText = "Show Calendar";
							break;
						case MOUSEFUNC_SHOWPROPERTY:	// 9
							lvItem.pszText = "T-Clock Properties";
							break;
						default:
							lvItem.pszText = "<unknown>";
							break;
						}
						break;
					case COL_OTHERD:
						if(iBtn==MOUSEFUNC_CLIPBOARD)
							lvItem.pszText=pData[m].format[c];
						else
							lvItem.pszText=" ";
						break;
					}
					ListView_SetItem(hList, &lvItem);
				} //---------------+++--> End of for(;;) Columns LOOP
			} //-//--+++--> End of if(...) Is Mouse/Click Value Valid
		} //-//-//------------+++--> End of for(c;;) Mouse Clicks LOOP
	} //-//-//-//-----------+++--> End of for(m;;) Mouse Button LOOP
	
	if(GetMyRegLong(reg_section, "DropFiles", 0)) {
		char szApp[LRG_BUFF] = {0};
		lvItem.pszText = "Drag";
		lvItem.iSubItem = COL_BUTTON;
		ListView_InsertItem(hList, &lvItem);
		
		ListView_SetItemText(hList, 0, COL_CLKTYP, "DropFiles");
		
		GetDlgItemText(hDlg, IDC_DROPFILES, szApp, LRG_BUFF);
		ListView_SetItemText(hList, 0, COL_ACTION, szApp);
		
		GetDlgItemText(hDlg, IDC_DROPFILESAPP, szApp, LRG_BUFF);
		ListView_SetItemText(hList, 0, COL_OTHERD, szApp);
	}
}
//================================================================================================
//-------------------------------------------+++--> Dialog Procedure for Mouse Tab Dialog Messages:
BOOL CALLBACK PageMouseProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //-----+++-->
{
	HWND hMouseView = FindWindowEx(hDlg, NULL, WC_LISTVIEW, NULL);
	
	switch(message) {
	case WM_INITDIALOG: {
			LVCOLUMN lvCol; int iCol = 0;
			
			OnInit(hDlg);
//==================================================================================
			hMouseView = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD|WS_VSCROLL|
									  LVS_REPORT|LVS_SINGLESEL, 17, 117,
									  430, 160, hDlg, NULL, 0, 0);
			ListView_SetExtendedListViewStyle(hMouseView,
											  LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
											  
			lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			for(iCol = COL_BUTTON; iCol <= COL_OTHERD; iCol++) {
				lvCol.iSubItem = iCol;	// From the String Table
				
				switch(iCol) {
				case COL_BUTTON:
					lvCol.pszText = "Button"; // Column Header Text
					lvCol.fmt = LVCFMT_CENTER; // Column Text Alignment
					lvCol.cx = 60;		// Set Column Width in Pixels
					break;
				case COL_CLKTYP:
					lvCol.pszText = "Click Type"; // Column Header Text
					lvCol.fmt = LVCFMT_CENTER;   // Column Text Alignment
					lvCol.cx = 75;			// Set Column Width in Pixels
					break;
				case COL_ACTION:
					lvCol.pszText = "Action"; // Column Header Text
					lvCol.fmt = LVCFMT_LEFT; // Column Text Alignment
					lvCol.cx = 113;		// Set Column Width in Pixels
					break;
				case COL_OTHERD:
					lvCol.pszText = "Other";  // Column Header Text
					lvCol.fmt = LVCFMT_LEFT; // Column Text Alignment
					lvCol.cx = 192;		// Set Column Width in Pixels
					break;
				}
				// Now Insert the Newly Configured Column Header
				ListView_InsertColumn(hMouseView, iCol, &lvCol);
			}
			AddMouseClickSettings(hMouseView, hDlg);
			ShowWindow(hMouseView, SW_SHOW);
//==================================================================================
			return TRUE;
		}
		
	case WM_COMMAND: {
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			// "Drop files"
			if(id == IDC_DROPFILES && code == CBN_SELCHANGE) {
				OnDropFilesChange(hDlg);
				g_bApplyClock = TRUE;
			}else if(id==IDC_DROPFILESAPP && code==EN_CHANGE) SendPSChanged(hDlg);
			// "..."
			else if(id == IDC_DROPFILESAPPSANSHO) OnSansho(hDlg, id);
			//  "Button"
			else if(id == IDC_MOUSEBUTTON && code == CBN_SELCHANGE) OnMouseButton(hDlg);
			// single .... quadruple
			else if(IDC_RADSINGLE <= id && id <= IDC_RADDOUBLE) OnMouseClickTime(hDlg, id);
			//  Mouse Function
			else if(id == IDC_MOUSEFUNC && code == CBN_SELCHANGE) {
				OnMouseFunc(hDlg);
				SendPSChanged(hDlg);
			}
			// Mouse Function - File
			else if(id == IDC_MOUSEFILE && code == EN_CHANGE) {
				OnMouseFileChange(hDlg);
				SendPSChanged(hDlg);
			} else if((id==IDC_TOOLTIP && code==EN_CHANGE) || id==IDCB_TOOLTIP) {
				if(id==IDCB_TOOLTIP) EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
				g_bApplyClock = TRUE; SendPSChanged(hDlg);
			} return TRUE;
		}
		
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			OnApply(hDlg); // UpDate pData[][] Array First
			AddMouseClickSettings(hMouseView, hDlg);
			break;
			
		} return TRUE;
		
	case WM_DESTROY:
		OnDestroy(hDlg);
		DestroyWindow(hDlg);
		return TRUE;
	}
	return FALSE;
}
//================================================================================================
//------------------------//----------------------------++--> Initialize Mouse Tab Dialog Controls:
void OnInit(HWND hDlg)   //-----------------------------------------------------------------+++-->
{
	char entry[3+4];
	char s[LRG_BUFF] = {0};
	HWND hDlgPage;
	HFONT hfont;
	int i, j;
	entry[2]='\0';
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont) {
		SendDlgItemMessage(hDlg, IDC_DROPFILESAPP,	WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_MOUSEFILE,	WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TOOLTIP,		WM_SETFONT, (WPARAM)hfont, 0);
	}
	
	for(i = IDS_NONE; i <= IDS_MOVETO; i++) CBAddString(hDlg, IDC_DROPFILES, (LPARAM)MyString(i));
	
	CBSetCurSel(hDlg, IDC_DROPFILES, GetMyRegLong(reg_section, "DropFiles", 0));
	GetMyRegStr(reg_section, "DropFilesApp", s, 256, "");
	SetDlgItemText(hDlg, IDC_DROPFILESAPP, s);
	
	pData=malloc(sizeof(CLICKDATA) * 5);
	
	for(i=0; i<=4; ++i) {
		if(i==1) continue; // skip right mouse
		for(j=0; j<MAX_CLICKS; ++j) {
			entry[0]='0'+i;
			entry[1]='1'+j;
			pData[i].disable = FALSE;
			pData[i].func[j] = GetMyRegLong(reg_section, entry, MOUSEFUNC_NONE);
			pData[i].format[j][0] = 0; // Clipboard
			pData[i].fname[j][0] = 0; // Open With...
			if(pData[i].func[j] == MOUSEFUNC_CLIPBOARD) {
				memcpy(entry+2,"Clip",5);
				GetMyRegStr(reg_section, entry, pData[i].format[j], 256, "");
			}
		}
	}
	
	hDlgPage = GetTopWindow(GetParent(hDlg));
	while(hDlgPage) {
		hDlgPage = GetNextWindow(hDlgPage, GW_HWNDNEXT);
	}
	pData[0].disable = GetMyRegLong("StartMenu", "StartMenuClock", FALSE);
	
	for(i = IDS_LEFTBUTTON; i<=IDS_XBUTTON2; ++i)
		CBAddString(hDlg, IDC_MOUSEBUTTON, (LPARAM)MyString(i));
		
	// set mouse functions to combo box
	InitMouseFuncList(hDlg); // Populate Mouse Click Action DropDown Menu
	
	OnDropFilesChange(hDlg);
	CBSetCurSel(hDlg, IDC_MOUSEBUTTON, 0);
	OnMouseButton(hDlg);
	if(!bV7up){
		EnableDlgItem(hDlg, IDCB_TOOLTIP, 0);
		CheckDlgButton(hDlg,IDCB_TOOLTIP, 1);
	}else{
		CheckDlgButton(hDlg,IDCB_TOOLTIP, GetMyRegLongEx("Tooltip","bCustom",0));
		EnableDlgItem(hDlg,IDC_TOOLTIP,IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	}
	GetMyRegStr("Tooltip", "Tooltip", s, LRG_BUFF, "");
	if(!*s) strcpy(s, "\"T-Clock\" LDATE");
	SetDlgItemText(hDlg, IDC_TOOLTIP, s);
}
//================================================================================================
//-------------------------//-------------------------+++--> Apply (Settings) Button Event Handler:
void OnApply(HWND hDlg)   //----------------------------------------------------------------+++-->
{
	char s[LRG_BUFF], entry[3+4];
	int n, i, j;
	entry[2]='\0';
	n = (int)(LRESULT)CBGetCurSel(hDlg, IDC_DROPFILES);
	SetMyRegLong("", "DropFiles", (n > 0));
	SetMyRegLong(reg_section, "DropFiles", n);
	GetDlgItemText(hDlg, IDC_DROPFILESAPP, s, 256);
	SetMyRegStr(reg_section, "DropFilesApp", s);
	
	for(i=0; i<=4; ++i) {
		if(i==1) continue;
		for(j=0; j<MAX_CLICKS; ++j) {
			entry[0]='0'+i;
			entry[1]='1'+j;
			if(pData[i].func[j])
				SetMyRegLong(reg_section, entry, pData[i].func[j]);
			else DelMyReg(reg_section, entry);
			if(pData[i].func[j] == MOUSEFUNC_CLIPBOARD) {
				memcpy(entry+2,"Clip",5);
				SetMyRegStr(reg_section, entry, pData[i].format[j]);
			}
		}
	}
	if(bV7up) SetMyRegLong("Tooltip","bCustom", IsDlgButtonChecked(hDlg,IDCB_TOOLTIP));
	GetDlgItemText(hDlg, IDC_TOOLTIP, s, 256);
	SetMyRegStr("Tooltip", "Tooltip", s);
}
//================================================================================================
//---------------------------//---------------------+++--> Free CLICKDATA Structure Memory on Exit:
void OnDestroy(HWND hDlg)   //--------------------------------------------------------------+++-->
{
	if(pData) {
		free(pData);   // Free, and...? (Crash Unless You Include the Next Line)
		pData = NULL; //<--+++--> Thank You Don Beusee for reminding me to do this.
	}
}
//================================================================================================
//-----------------------------------//----------------+++--> Drag-N-Drop Files Menu Event Handler:
void OnDropFilesChange(HWND hDlg)   //------------------------------------------------------+++-->
{
	int i, n;
	
	n = (int)(LRESULT)CBGetCurSel(hDlg, IDC_DROPFILES);
	SetDlgItemText(hDlg, IDC_LABDROPFILESAPP, MyString(n >= 3?IDS_LABFOLDER:IDS_LABPROGRAM));
	
	for(i = IDC_LABDROPFILESAPP; i <= IDC_DROPFILESAPPSANSHO; i++)
		EnableWindow(GetDlgItem(hDlg, i), (2 <= n && n <= 4));
		
	SendPSChanged(hDlg);
}
//================================================================================================
//-------------------------------//----------+++--> When Mouse Button is Selected (Left or Middle):
void OnMouseButton(HWND hDlg)   //----------------------------------------------------------+++-->
{
	int j;
	int button=(int)(LRESULT)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
	if(button>0) ++button;
	for(j=0; j<MAX_CLICKS; ++j) {
		if(pData[button].func[j]) break;
	}
	
	if(j == 2) j = 0;
	
	CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADDOUBLE, IDC_RADSINGLE + j);
	OnMouseClickTime(hDlg, IDC_RADSINGLE + j);
}
//================================================================================================
//------------------------------------------//+++--> T-Clock's Reaction to Single or Double Clicks:
void OnMouseClickTime(HWND hDlg, int id)   //-----------------------------------------------+++-->
{
	int click, i, count, func;
	int button=(int)(LRESULT)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
	if(button>0) ++button;
	
	click = id - IDC_RADSINGLE;
	func = pData[button].func[click];
	
	count = (int)(LRESULT)CBGetCount(hDlg, IDC_MOUSEFUNC);
	for(i = 0; i < count; i++) {
		if(func == CBGetItemData(hDlg, IDC_MOUSEFUNC, i)) {
			CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
			break;
		}
	}
	OnMouseFunc(hDlg);
}
//================================================================================================
//-----------------------------//----------------------------------+++--> Mouse Functions ComboBox:
void OnMouseFunc(HWND hDlg)   //------------------------------------------------------------+++-->
{
	int click, j, index, func;
	int button=(int)(LRESULT)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
	if(button>0) ++button;
	
	for(j=0; j<MAX_CLICKS; ++j) {
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j))
			break;
	}
	
	if(j==MAX_CLICKS) return;
	click = j;
	
	index = (int)(LRESULT)CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = (int)(LRESULT)CBGetItemData(hDlg, IDC_MOUSEFUNC, index);
	pData[button].func[click] = func;
	
	EnableWindow(GetDlgItem(hDlg, IDC_MOUSEFILE), (func == MOUSEFUNC_CLIPBOARD));
	EnableWindow(GetDlgItem(hDlg, IDC_LABMOUSEFILE), (func == MOUSEFUNC_CLIPBOARD));
	
	if(func == MOUSEFUNC_CLIPBOARD) {
		SetDlgItemText(hDlg, IDC_LABMOUSEFILE, MyString(IDS_FORMAT));
		
		if(!*pData[button].format[click])
			GetMyRegStr("Format", "Format", pData[button].format[click], LRG_BUFF, "");
		
		SetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].format[click]);
	}
}
//================================================================================================
//-----------------------------+++--> Clipboard Format -&- Open File Mouse Function Event Handling:
void OnMouseFileChange(HWND hDlg)   //------------------------------------------------------+++-->
{
	int j, click, index, func;
	int button=(int)(LRESULT)CBGetCurSel(hDlg,IDC_MOUSEBUTTON);
	if(button>0) ++button;
	for(j=0; j<MAX_CLICKS; ++j) {
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j)) break;
	}
	
	if(j==MAX_CLICKS) return;
	click = j;
	
	index = (int)(LRESULT)CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = (int)(LRESULT)CBGetItemData(hDlg, IDC_MOUSEFUNC, index);
	
	if(func == MOUSEFUNC_CLIPBOARD) GetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].format[click], 1024);
}
/*--------------------------------------------------
--------------- Handle File Dropped on Clock Options
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	if(id == IDC_DROPFILESAPPSANSHO) {
		if((int)CBGetCurSel(hDlg, IDC_DROPFILES) >= 3) {
			BROWSEINFO bi;
			LPITEMIDLIST pidl;
			memset(&bi, 0, sizeof(BROWSEINFO));
			bi.hwndOwner = hDlg;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			pidl = SHBrowseForFolder(&bi);
			if(pidl) {
				SHGetPathFromIDList(pidl, fname);
				SetDlgItemText(hDlg, id - 1, fname);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}
	
	filter[0] = 0;
	if(id == IDC_DROPFILESAPPSANSHO) {
		str0cat(filter, MyString(IDS_PROGRAMFILE));
		str0cat(filter, "*.exe");
	}
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
		
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}
//================================================================================================
//-----------------------------------//------+++--> Populate the Mouse Click Actions DropDown Menu:
void InitMouseFuncList(HWND hDlg)   //------------------------------------------------------+++-->
{
	int index;
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_NONE));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_NONE);
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_TIMER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_TIMER);
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC,(LPARAM)MyString(IDS_SCREENSAVER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SCREENSAVER);
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_SHOWCALENDER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SHOWCALENDER);
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_MOUSECOPY));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_CLIPBOARD);
	
	index = (int)(LRESULT)CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_PROPERTY));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SHOWPROPERTY);
}
