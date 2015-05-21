/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/
// Modified by Stoic Joker: Monday, March 8 2010 - 7:52:55
#include "tclock.h"

static int m_logpixelsy;
static int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);
static int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam);

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg,BOOL preview);
static void InitColor(HWND hDlg);
static void InitComboFont(HWND hDlg);
static void OnChooseColor(HWND hDlg, WORD color_btn);
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
		case CBN_EDITCHANGE:
			if(id==IDC_FONTSIZE)
				SendPSChanged(hDlg);
			break;
		case EN_CHANGE:
			if(id==IDC_CLOCKHEIGHT || id==IDC_CLOCKWIDTH || id==IDC_ALPHATB || id==IDC_VERTPOS || id==IDC_LINEHEIGHT || id==IDC_HORIZPOS || id==IDC_ANGLE){
				SendPSChanged(hDlg);
			} break;
		default:
			if(id==IDC_CHOOSECOLFORE || id==IDC_CHOOSECOLBACK){
				OnChooseColor(hDlg, id);
			}else if(id==IDC_CHOOSEFONT){
				HWND hwndCombo;
				HDC hdc;
				char size[8];
				LOGFONT lf = {0};
				CHOOSEFONT chosenfont = {sizeof(CHOOSEFONT)};
				chosenfont.hwndOwner = hDlg;
				chosenfont.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
				chosenfont.lpLogFont = &lf;
				hwndCombo = GetDlgItem(hDlg,IDC_FONTSIZE);
				ComboBox_GetText(hwndCombo, size, sizeof(size));
				hdc = GetDC(NULL);
				lf.lfWeight = IsDlgButtonChecked(hDlg, IDC_BOLD) ? FW_BOLD : FW_REGULAR;
				lf.lfItalic = (BYTE)IsDlgButtonChecked(hDlg, IDC_ITALIC);
				lf.lfHeight = -MulDiv(atoi(size), GetDeviceCaps(hdc,LOGPIXELSY), 72);
				ReleaseDC(NULL, hdc);
				hwndCombo = GetDlgItem(hDlg,IDC_FONT);
				ComboBox_GetText(hwndCombo, lf.lfFaceName, sizeof(lf.lfFaceName));
				if(ChooseFont(&chosenfont)){
					int sel;
					CheckDlgButton(hDlg, IDC_BOLD, (chosenfont.lpLogFont->lfWeight >= FW_SEMIBOLD));
					CheckDlgButton(hDlg, IDC_ITALIC, chosenfont.lpLogFont->lfItalic);
					ComboBox_SelectString(hwndCombo, -1, chosenfont.lpLogFont->lfFaceName);
					hwndCombo = GetDlgItem(hDlg,IDC_FONTSIZE);
					wsprintf(size,"%d",chosenfont.iPointSize/10);
					sel = ComboBox_FindStringExact(hwndCombo, -1, size);
					if(sel != CB_ERR)
						ComboBox_SetCurSel(hwndCombo, sel);
					else
						ComboBox_SetText(hwndCombo, size);
					SendPSChanged(hDlg);
				}
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
				api.DelKey("Preview");
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
	HWND quality_cb = GetDlgItem(hDlg, IDC_FONTQUAL);
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
	
	
	CheckDlgButton(hDlg, IDC_BOLD, api.GetInt("Clock", "Bold", 0));
	CheckDlgButton(hDlg, IDC_ITALIC, api.GetInt("Clock", "Italic", 0));
	
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
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS32, 0, api.GetInt("Clock", "ClockHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS32, 0, api.GetInt("Clock", "ClockWidth", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE32, (WPARAM)-64,64);
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS32, 0, api.GetInt("Clock", "LineHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS32, 0, api.GetInt("Clock", "VertPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETPOS32, 0, api.GetInt("Clock", "HorizPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETRANGE32, (WPARAM)-360,360);
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETPOS32, 0, api.GetInt("Clock", "Angle", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETRANGE32, 0,100);
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETPOS32, 0, api.GetInt("Taskbar", "AlphaTaskbar", 0));
	
	ComboBox_AddString(quality_cb,"Default");            // 0 = DEFAULT_QUALITY
	ComboBox_AddString(quality_cb,"Draft");              // 1 = DRAFT_QUALITY
	ComboBox_AddString(quality_cb,"Proof");              // 2 = PROOF_QUALITY
	ComboBox_AddString(quality_cb,"NonAntiAliased");     // 3 = NONANTIALIASED_QUALITY
	ComboBox_AddString(quality_cb,"AntiAliased (Win7)"); // 4 = ANTIALIASED_QUALITY
	ComboBox_AddString(quality_cb,"ClearType (WinXP+)"); // 5 = CLEARTYPE_QUALITY
	ComboBox_AddString(quality_cb,"ClearType Natural");  // 6 = CLEARTYPE_NATURAL_QUALITY
	ComboBox_SetCurSel(quality_cb,api.GetInt("Clock","FontQuality",CLEARTYPE_QUALITY));
	m_transition=0; // end transition lock, ready to go
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg,BOOL preview)
{
	const char* section=preview?"Preview":"Clock";
	char tmp[LF_FACESIZE];
	HWND fore_cb = GetDlgItem(hDlg, IDC_COLFORE);
	HWND back_cb = GetDlgItem(hDlg, IDC_COLBACK);
	HWND font_cb = GetDlgItem(hDlg, IDC_FONT);
	HWND size_cb = GetDlgItem(hDlg, IDC_FONTSIZE);
	int sel;
	
	api.SetInt(section, "ForeColor", (COLORREF)ComboBox_GetItemData(fore_cb,ComboBox_GetCurSel(fore_cb)));
	api.SetInt(section, "BackColor", (COLORREF)ComboBox_GetItemData(back_cb,ComboBox_GetCurSel(back_cb)));
	
	ComboBox_GetLBText(font_cb, ComboBox_GetCurSel(font_cb), tmp);
	api.SetStr(section, "Font", tmp);
	
	sel = ComboBox_GetCurSel(size_cb);
	if(sel == -1)
		ComboBox_GetText(size_cb, tmp, LF_FACESIZE);
	else
		ComboBox_GetLBText(size_cb, sel, tmp);
	api.SetInt(section, "FontSize", atoi(tmp));
	
	api.SetInt(section, "Bold",   IsDlgButtonChecked(hDlg, IDC_BOLD));
	api.SetInt(section, "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
	
	api.SetInt(section, "FontQuality", ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_FONTQUAL)));
	
	api.SetInt(section, "ClockHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINCHEIGHT,UDM_GETPOS32,0,0));
	api.SetInt(section, "ClockWidth", (int)SendDlgItemMessage(hDlg,IDC_SPINCWIDTH,UDM_GETPOS32,0,0));
	api.SetInt(section, "LineHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINLHEIGHT,UDM_GETPOS32,0,0));
	api.SetInt(section, "VertPos", (int)SendDlgItemMessage(hDlg,IDC_SPINVPOS,UDM_GETPOS32,0,0));
	api.SetInt(section, "HorizPos", (int)SendDlgItemMessage(hDlg,IDC_SPINHPOS,UDM_GETPOS32,0,0));
	api.SetInt(section, "Angle", (int)SendDlgItemMessage(hDlg,IDC_SPINANGLE,UDM_GETPOS32,0,0));
	
	if(!preview){
		api.SetInt("Taskbar","AlphaTaskbar",(int)SendDlgItemMessage(hDlg,IDC_SPINALPHA,UDM_GETPOS32,0,0));
		api.DelKey("Preview");
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
const size_t syscolor_num = sizeof(syscolor)/sizeof(syscolor_t);
const COLORREF basecolor[]={
	0x00000080, 0x00008000, 0x00800000,
	0x00008080, 0x00800080, 0x00808000,
	0x000000FF, 0x0000FF00, 0x00FF0000,
	0x0000FFFF, 0x00FF00FF, 0x00FFFF00,
	0x00FFFFFF, 0x00C0C0C0, 0x00808080, 0x00000000,
};
const size_t basecolor_num = sizeof(basecolor)/sizeof(COLORREF);
const size_t colorstotal = sizeof(syscolor)/sizeof(syscolor_t)+sizeof(basecolor)/sizeof(COLORREF);
void InitColor(HWND hDlg)
{
	COLORREF col;
	WORD id;
	unsigned icol;
	
	for(id=IDC_COLFORE; id<=IDC_COLBACK; id+=2){
		HWND color_cb = GetDlgItem(hDlg, id);
		/// add sys colors
		for(icol=0; icol<syscolor_num; ++icol){
			ComboBox_AddString(color_cb, (size_t)TCOLOR(syscolor[icol].col));
		}
		/// add base colors
		for(icol=0; icol<basecolor_num; ++icol)
			ComboBox_AddString(color_cb, (size_t)basecolor[icol]);
		/// select last used color
		if(id == IDC_COLFORE)
			col=api.GetInt("Clock","ForeColor",TCOLOR(TCOLOR_DEFAULT));
		else
			col=api.GetInt("Clock","BackColor",TCOLOR(TCOLOR_DEFAULT));
		for(icol=0; icol<colorstotal; ++icol) {
			if(col==(COLORREF)ComboBox_GetItemData(color_cb,icol))
				break;
		}
		if(icol==colorstotal) // Add the Selected Custom Color
			ComboBox_AddString(color_cb, (size_t)col);
		ComboBox_SetCurSel(color_cb, icol);
	}
}

/*------------------------------------------------
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPARAM lParam)
{
	MEASUREITEMSTRUCT* pmis;
	pmis = (MEASUREITEMSTRUCT*)lParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
--------------------------------------------------*/
void OnDrawItemColorCombo(LPARAM lParam)
{
	DRAWITEMSTRUCT* pdis;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	
	pdis = (DRAWITEMSTRUCT*)lParam;
	
	if(IsWindowEnabled(pdis->hwndItem)) {
		col = api.GetColor((COLORREF)pdis->itemData,2);
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
void OnChooseColor(HWND hDlg, WORD color_btn)
{
	CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
	COLORREF col, colarray[16];
	HWND color_cb = GetDlgItem(hDlg, color_btn-1);
	unsigned icol;
	
	col = api.GetColor((COLORREF)ComboBox_GetItemData(color_cb, ComboBox_GetCurSel(color_cb)),2);
	
	for(icol = 0; icol < 16; ++icol) colarray[icol] = 0x00FFFFFF;
	
	cc.hwndOwner = hDlg;
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return;
	
	for(icol=0; icol<colorstotal; ++icol) {
		if(cc.rgbResult==(COLORREF)ComboBox_GetItemData(color_cb,icol))
			break;
	}
	if(icol == colorstotal){
		if(ComboBox_GetCount(color_cb) == colorstotal)
			ComboBox_AddString(color_cb, (size_t)cc.rgbResult);
		else
			ComboBox_SetItemData(color_cb, icol, cc.rgbResult);
	}
	ComboBox_SetCurSel(color_cb, icol);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf = {0};
	HWND hcombo;
	char s[80];
	int i;
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	hcombo = GetDlgItem(hDlg, IDC_FONT);
	
	lf.lfCharSet = DEFAULT_CHARSET;  // fonts from any charset
	EnumFontFamiliesEx(hdc, &lf, EnumFontFamExProc, (LPARAM)hcombo, 0);
	

	ReleaseDC(NULL, hdc);
	
	api.GetStrEx("Clock", "Font", s, 80, "Arial");
	
	i = ComboBox_FindStringExact(hcombo, -1, s);
	if(i == LB_ERR) i = 0;
	ComboBox_SetCurSel(hcombo, i);
}

/*------------------------------------------------
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char str[LF_FACESIZE];
	DWORD size;
	LOGFONT lf = {0};
	HWND size_cb = GetDlgItem(hDlg, IDC_FONTSIZE);
	HWND font_cb = GetDlgItem(hDlg, IDC_FONT);
	int pos;
	
	// remember old size
	if(bInit) { // on WM_INITDIALOG
		size = api.GetInt("Clock", "FontSize", 9);
		if(!size || size>100) size = 9;
	} else { // when IDC_FONT has been changed
		ComboBox_GetText(size_cb, str, LF_FACESIZE);
		size = atoi(str);
	}
	
	ComboBox_ResetContent(size_cb);
	
	hdc = GetDC(NULL);
	m_logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	ComboBox_GetLBText(font_cb, ComboBox_GetCurSel(font_cb), str);
	
	strcpy(lf.lfFaceName, str);
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, EnumSizeProcEx,
					   (LPARAM)size_cb, 0);
					   
	ReleaseDC(NULL, hdc);
	
	wsprintf(str, "%d", size);
	pos = ComboBox_FindStringExact(size_cb, -1, str);
	if(pos != LB_ERR) {
		ComboBox_SetCurSel(size_cb, pos);
		return;
	}
	ComboBox_SetText(size_cb, str);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	HWND hwndSize = (HWND)lParam;
	(void)lpntme; (void)FontType;
	if(lpelfe->lfFaceName[0]!='@' && ComboBox_FindStringExact(hwndSize, -1, lpelfe->lfFaceName)==LB_ERR) {
		ComboBox_AddString(hwndSize, lpelfe->lfFaceName);
	}
	return 1;
}

/*------------------------------------------------
--------------------------------------------------*/
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	HWND hwndSize = (HWND)lParam;
	const unsigned char nFontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22,24,26,28,32,36,48,72};
	char str[8];
	int i, size, count;
	(void)lpelfe;
	
	// is modern font which supports any size?
	if((FontType&TRUETYPE_FONTTYPE) || !(FontType&RASTER_FONTTYPE)) {
		for(i=0; i<sizeof(nFontSizes); ++i) {
			wsprintf(str,"%hu",nFontSizes[i]);
			ComboBox_AddString(hwndSize,str);
		}
		return 0;
	}
	
	// only add supported sizes for raster type fonts
	size = (lpntme->tmHeight - lpntme->tmInternalLeading) * 72 / m_logpixelsy;
	count = ComboBox_GetCount(hwndSize);
	for(i=0; i<count; ++i) { // dupes check + sorting
		ComboBox_GetLBText(hwndSize, i, str);
		if(size == atoi(str))
			return 1;
		else if(size < atoi(str)) {
			wsprintf(str, "%d", size);
			ComboBox_InsertString(hwndSize, i, str);
			return 1;
		}
	}
	wsprintf(str, "%d", size);
	ComboBox_AddString(hwndSize, str);
	return 1;
}
