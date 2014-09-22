/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/
// Modified by Stoic Joker: Monday, March 8 2010 - 7:52:55
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg,BOOL preview);
static void InitColor(HWND hDlg);
static void InitComboFont(HWND hDlg);
static void OnChooseColor(HWND hDlg, WORD id);
static void OnDrawItemColorCombo(LPARAM lParam);
static void SetComboFontSize(HWND hDlg, int bInit);
static void OnMeasureItemColorCombo(LPARAM lParam);

static char m_transition=-1; // somehow WM_INITDIALOG gets called quite late, we need to initialize the var here. (related problem caused by spin control)
static __inline void SendPSChanged(HWND hDlg){
	if(m_transition==-1) return;
	g_bApplyClock = 1;
	g_bApplyTaskbar = 1;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	
	OnApply(hDlg,1);
	SendMessage(g_hwndClock, CLOCKM_REFRESHCLOCKPREVIEW, 0, 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageColorProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:{
		HFONT hfontb=(HFONT)SendDlgItemMessage(hDlg,IDC_BOLD,WM_GETFONT,0,0);
		HFONT hfonti=(HFONT)SendDlgItemMessage(hDlg,IDC_ITALIC,WM_GETFONT,0,0);
		SendDlgItemMessage(hDlg,IDC_BOLD,WM_SETFONT,0,0);
		SendDlgItemMessage(hDlg,IDC_ITALIC,WM_SETFONT,0,0);
		DeleteObject(hfontb);
		DeleteObject(hfonti);
		break;}
	case WM_MEASUREITEM:
		OnMeasureItemColorCombo(lParam);
		return TRUE;
	case WM_DRAWITEM:
		OnDrawItemColorCombo(lParam);
		return TRUE;
	case WM_COMMAND: {
		WORD id=LOWORD(wParam);
		switch(HIWORD(wParam)){
		case CBN_SELCHANGE:
			if(id==IDC_COLFORE || id==IDC_COLBACK || id==IDC_FONT || id==IDC_FONTQUAL || id==IDC_FONTSIZE){
				if(id==IDC_FONT) SetComboFontSize(hDlg, FALSE);
				SendPSChanged(hDlg);
			} break;
		case EN_CHANGE:
			if(id==IDC_CLOCKHEIGHT || id==IDC_CLOCKWIDTH || id==IDC_ALPHATB || id==IDC_VERTPOS || id==IDC_LINEHEIGHT || id==IDC_HORIZPOS || id==IDC_ANGLE){
				SendPSChanged(hDlg);
			} break;
		default:
			if(id==IDC_CHOOSECOLFORE || id==IDC_CHOOSECOLBACK){
				OnChooseColor(hDlg, id);
			}else if(id==IDC_BOLD || id==IDC_ITALIC)
				SendPSChanged(hDlg);
		}
		return TRUE;}
	case WM_NOTIFY:{
		PSHNOTIFY* notify=(PSHNOTIFY*)lParam;
		switch(notify->hdr.code) {
		case PSN_APPLY:
			OnApply(hDlg,0);
			if(notify->lParam)
				m_transition=-1;
			break;
		case PSN_RESET:
			if(m_transition==1){
				SendMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
				SendMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
				DelMyRegKey("Preview");
			}
			m_transition=-1;
			break;
		}
		return TRUE;}
	}
	return FALSE;
}

/*------------------------------------------------
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HDC hdc;
	LOGFONT logfont;
	HFONT hfont;
	m_transition=-1; // start transition lock
	// setting of "background" and "text"
	InitColor(hDlg);
	
	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8) {
		EnableDlgItem(hDlg, IDC_CHOOSECOLFORE, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK, FALSE);
	}
	DeleteDC(hdc);
	
	InitComboFont(hDlg);
	
	SetComboFontSize(hDlg, TRUE);
	
	
	CheckDlgButton(hDlg, IDC_BOLD, GetMyRegLong("Clock", "Bold", 0));
	CheckDlgButton(hDlg, IDC_ITALIC, GetMyRegLong("Clock", "Italic", 0));
	
	hfont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfont, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfont = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_BOLD, WM_SETFONT, (WPARAM)hfont, 0);
	
	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfont = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_ITALIC, WM_SETFONT, (WPARAM)hfont, 0);
	
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS32, 0, GetMyRegLong("Clock", "ClockHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS32, 0, GetMyRegLong("Clock", "ClockWidth", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE32, (WPARAM)-64,64);
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS32, 0, GetMyRegLong("Clock", "LineHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS32, 0, GetMyRegLong("Clock", "VertPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETPOS32, 0, GetMyRegLong("Clock", "HorizPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETRANGE32, (WPARAM)-360,360);
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETPOS32, 0, GetMyRegLong("Clock", "Angle", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETRANGE32, 0,180);
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETPOS32, 0, GetMyRegLong("Taskbar", "AlphaTaskbar", 0));
	
	CBAddString(hDlg,IDC_FONTQUAL,"Default");				// DEFAULT_QUALITY			 = 0
	CBAddString(hDlg,IDC_FONTQUAL,"Draft");					// DRAFT_QUALITY			 = 1
	CBAddString(hDlg,IDC_FONTQUAL,"Proof");					// PROOF_QUALITY			 = 2
	CBAddString(hDlg,IDC_FONTQUAL,"NonAntiAliased");		// NONANTIALIASED_QUALITY	 = 3
	CBAddString(hDlg,IDC_FONTQUAL,"AntiAliased (Win7)");	// ANTIALIASED_QUALITY		 = 4
	CBAddString(hDlg,IDC_FONTQUAL,"ClearType (WinXP+)");	// CLEARTYPE_QUALITY		 = 5
	CBAddString(hDlg,IDC_FONTQUAL,"ClearType Natural");		// CLEARTYPE_NATURAL_QUALITY = 6
	CBSetCurSel(hDlg,IDC_FONTQUAL,GetMyRegLong("Clock","FontQuality",CLEARTYPE_QUALITY));
	m_transition=0; // end transition lock, ready to go
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg,BOOL preview)
{
	const char* section=preview?"Preview":"Clock";
	char tmp[80];
	
	SetMyRegLong(section, "ForeColor", (int)CBGetItemData(hDlg,IDC_COLFORE,CBGetCurSel(hDlg,IDC_COLFORE)));
	SetMyRegLong(section, "BackColor", (int)CBGetItemData(hDlg,IDC_COLBACK,CBGetCurSel(hDlg,IDC_COLBACK)));
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), tmp);
	SetMyRegStr(section, "Font", tmp);
	
	if(CBGetCount(hDlg, IDC_FONTSIZE) > 0) {
		CBGetLBText(hDlg, IDC_FONTSIZE, CBGetCurSel(hDlg, IDC_FONTSIZE), tmp);
		SetMyRegLong(section, "FontSize", atoi(tmp));
	} else SetMyRegLong(section, "FontSize", 9);
	
	SetMyRegLong(section, "Bold",   IsDlgButtonChecked(hDlg, IDC_BOLD));
	SetMyRegLong(section, "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
	
	SetMyRegLong(section, "FontQuality", (int)CBGetCurSel(hDlg, IDC_FONTQUAL));
	
	SetMyRegLong(section, "ClockHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINCHEIGHT,UDM_GETPOS32,0,0));
	SetMyRegLong(section, "ClockWidth", (int)SendDlgItemMessage(hDlg,IDC_SPINCWIDTH,UDM_GETPOS32,0,0));
	SetMyRegLong(section, "LineHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINLHEIGHT,UDM_GETPOS32,0,0));
	SetMyRegLong(section, "VertPos", (int)SendDlgItemMessage(hDlg,IDC_SPINVPOS,UDM_GETPOS32,0,0));
	SetMyRegLong(section, "HorizPos", (int)SendDlgItemMessage(hDlg,IDC_SPINHPOS,UDM_GETPOS32,0,0));
	SetMyRegLong(section, "Angle", (int)SendDlgItemMessage(hDlg,IDC_SPINANGLE,UDM_GETPOS32,0,0));
	SetMyRegLong("Taskbar","AlphaTaskbar",(int)SendDlgItemMessage(hDlg,IDC_SPINALPHA,UDM_GETPOS32,0,0));
	
	if(!preview){
		DelMyRegKey("Preview");
		m_transition=0;
	}else
		m_transition=1;
}

/*------------------------------------------------
--------------------------------------------------*/
typedef struct {
	COLORREF col;
	const char* name;
} syscolor_t;
#include "../common/tcolor.h"
const syscolor_t syscolor[]={
	{TCOLOR_DEFAULT,"<default>"},
	{TCOLOR_TRANSPARENT,"<transparent>"},
	{TCOLOR_THEME,"<theme color>"},
//	{TCOLOR_THEME2,"<theme color II>"},
	
	{COLOR_3DFACE,"3DFACE"},
	{COLOR_3DSHADOW,"3DSHADOW"},
	{COLOR_3DHILIGHT,"3DHILIGHT"},
	{COLOR_BTNTEXT,"BTNTEXT"},
	{COLOR_WINDOWTEXT,"WINDOWTEXT"},
	{COLOR_INFOTEXT,"INFOTEXT"},
	{COLOR_INFOBK,"INFOBK"},
};
const size_t syscolor_num=sizeof(syscolor)/sizeof(syscolor_t);
const COLORREF basecolor[]={
	0x00000080, 0x00008000, 0x00800000,
	0x00008080, 0x00800080, 0x00808000,
	0x000000FF, 0x0000FF00, 0x00FF0000,
	0x0000FFFF, 0x00FF00FF, 0x00FFFF00,
	0x00FFFFFF, 0x00C0C0C0, 0x00808080, 0x00000000,
};
const size_t basecolor_num=sizeof(basecolor)/sizeof(COLORREF);
const size_t colorstotal=sizeof(syscolor)/sizeof(syscolor_t)+sizeof(basecolor)/sizeof(COLORREF);
void InitColor(HWND hDlg)
{
	COLORREF col;
	WORD id;
	unsigned icol;
	
	for(id=IDC_COLFORE; id<=IDC_COLBACK; id+=2){
		/// add sys colors
		for(icol=0; icol<syscolor_num; ++icol){
			CBAddString(hDlg, id, TCOLOR(syscolor[icol].col));
//		for(j=0; j<31; ++j){
//			CBAddString(hDlg, id, TCOLOR(j));
		}
		/// add base colors
		for(icol=0; icol<basecolor_num; ++icol)
			CBAddString(hDlg, id, basecolor[icol]);
		/// select last used color
		if(id==IDC_COLFORE)
			col=GetMyRegLong("Clock","ForeColor",TCOLOR(TCOLOR_DEFAULT));
		else
			col=GetMyRegLong("Clock","BackColor",TCOLOR(TCOLOR_DEFAULT));
		for(icol=0; icol<colorstotal; ++icol) {
			if(col==(COLORREF)CBGetItemData(hDlg,id,icol))
				break;
		}
		if(icol==colorstotal) // Add the Selected Custom Color
			CBAddString(hDlg, id, col);
		CBSetCurSel(hDlg, id, icol);
	}
}

/*------------------------------------------------
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pmis;
	pmis = (LPMEASUREITEMSTRUCT)lParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
--------------------------------------------------*/
void OnDrawItemColorCombo(LPARAM lParam)
{
	LPDRAWITEMSTRUCT pdis;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	
	pdis = (LPDRAWITEMSTRUCT)lParam;
	
	if(IsWindowEnabled(pdis->hwndItem)) {
		col = GetTColor((COLORREF)pdis->itemData,2);
	} else col = GetSysColor(COLOR_3DFACE);
	
	switch(pdis->itemAction) {
	case ODA_DRAWENTIRE:
	case ODA_SELECT: {
		hbr = CreateSolidBrush(col);
		FillRect(pdis->hDC, &pdis->rcItem, hbr);
		DeleteObject(hbr);
		
		// print sys color names
		if(pdis->itemID<syscolor_num){
			int y;
			SetBkMode(pdis->hDC, TRANSPARENT);
			GetTextMetrics(pdis->hDC, &tm);
			SetTextColor(pdis->hDC, 0x00FFFFFF-(col&0x00FFFFFF));
			if(col==0x00808080)
				SetTextColor(pdis->hDC, 0);
			y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
			TextOut(pdis->hDC, pdis->rcItem.left + 4, pdis->rcItem.top + y,
					syscolor[pdis->itemID].name, (int)strlen(syscolor[pdis->itemID].name));
		}
		if(!(pdis->itemState & ODS_FOCUS)) break;
		}
	case ODA_FOCUS: {
		HBRUSH hbr2;
		RECT rc;
		rc.left=pdis->rcItem.left-1;
		rc.top=pdis->rcItem.top-1;
		rc.right=pdis->rcItem.right-1;
		rc.bottom=pdis->rcItem.bottom-1;
		if(pdis->itemState & ODS_FOCUS){
			hbr=CreateSolidBrush(0x00000000);
			hbr2=CreateSolidBrush(0x00FFFFFF);
		}else
			hbr=hbr2=CreateSolidBrush(col);
		FrameRect(pdis->hDC, &pdis->rcItem, hbr);
		FrameRect(pdis->hDC, &rc, hbr2);
		if(hbr2!=hbr) DeleteObject(hbr2);
		DeleteObject(hbr);
		break;}
	}
}

/*------------------------------------------------
--------------------------------------------------*/
void OnChooseColor(HWND hDlg, WORD id)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	WORD idCombo;
	unsigned icol;
	
	idCombo = id - 1;
	
	col = GetTColor((COLORREF)CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo)),2);
	
	for(icol = 0; icol < 16; ++icol) colarray[icol] = 0x00FFFFFF;
	
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.hInstance = (HWND)(HINSTANCE)GetModuleHandle(NULL);
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return;
	
	for(icol=0; icol<colorstotal; ++icol) {
		if(cc.rgbResult==(COLORREF)CBGetItemData(hDlg,idCombo,icol))
			break;
	}
	if(icol==colorstotal){
		if((unsigned)CBGetCount(hDlg,idCombo)==colorstotal)
			CBAddString(hDlg,idCombo,cc.rgbResult);
		else
			CBSetItemData(hDlg,idCombo,icol,cc.rgbResult);
	}
	CBSetCurSel(hDlg, idCombo, icol);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
int logpixelsy;
/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	char s[80];
	int i;
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, IDC_FONT);
	
	lf.lfCharSet = (BYTE)GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	
	
	ReleaseDC(NULL, hdc);
	
	GetMyRegStrEx("Clock", "Font", s, 80, "Arial");
	
	i = (int)CBFindStringExact(hDlg, IDC_FONT, s);
	if(i == LB_ERR) i = 0;
	CBSetCurSel(hDlg, IDC_FONT, i);
}

/*------------------------------------------------
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char str[160];
	DWORD size;
	LOGFONT lf;
	
	//以前のsizeを保存
	if(bInit) { // WM_INITDIALOGのとき
		size = GetMyRegLong("Clock", "FontSize", 9);
		if(!size || size>100) size = 9;
	} else { // IDC_FONTが変更されたとき
		CBGetLBText(hDlg, IDC_FONTSIZE,
					CBGetCurSel(hDlg, IDC_FONTSIZE), &str);
		size = atoi(str);
	}
	
	CBResetContent(hDlg, IDC_FONTSIZE);
	
	hdc = GetDC(NULL);
	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), str);
	
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, str);
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT));
	EnumFontFamiliesEx(hdc, &lf, EnumSizeProcEx,
					   (LPARAM)GetDlgItem(hDlg, IDC_FONTSIZE), 0);
					   
	ReleaseDC(NULL, hdc);
	
	for(; size; --size) {
		int pos;
		wsprintf(str, "%d", size);
		pos = (int)CBFindStringExact(hDlg, IDC_FONTSIZE, str);
		if(pos != LB_ERR) {
			CBSetCurSel(hDlg, IDC_FONTSIZE, pos); return;
		}
	}
	CBSetCurSel(hDlg, IDC_FONTSIZE, 0);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	(void)lpntme; (void)FontType;
	if(lpelfe->lfFaceName[0]!='@' && SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, 0, (LPARAM)lpelfe->lfFaceName)==LB_ERR) {
		int index = (int)SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
		if(index >= 0)
			SendMessage((HWND)lParam, CB_SETITEMDATA, index, lpelfe->lfCharSet);
	}
	return 1;
}

/*------------------------------------------------
--------------------------------------------------*/
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	const unsigned char nFontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22,24,26,28,36,48,72};
	char str[8];
	int i;
	
	(void)lpelfe;
	
	if((FontType & TRUETYPE_FONTTYPE) ||
	   !((FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE))) {
		for(i=0; i<sizeof(nFontSizes); ++i) {
			wsprintf(str,"%hu",nFontSizes[i]);
			SendMessage((HWND)lParam,CB_ADDSTRING,0,(LPARAM)str);
		}
		return FALSE;
	}else{
		int num = (lpntme->tmHeight - lpntme->tmInternalLeading) * 72 / logpixelsy;
		int count = (int)SendMessage((HWND)lParam, CB_GETCOUNT, 0, 0);
		for(i = 0; i < count; ++i) {
			SendMessage((HWND)lParam, CB_GETLBTEXT, i, (LPARAM)str);
			if(num == atoi(str)) return TRUE;
			else if(num < atoi(str)) {
				wsprintf(str, "%d", num);
				SendMessage((HWND)lParam, CB_INSERTSTRING, i, (LPARAM)str);
				return TRUE;
			}
		}
		wsprintf(str, "%d", num);
		SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)str);
		return TRUE;
	}
}
