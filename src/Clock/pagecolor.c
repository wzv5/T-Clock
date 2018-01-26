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
static void InitComboFont(HWND hDlg);
static void SetComboFontSize(HWND hDlg, int bInit);

static char m_transition=-1; // somehow WM_INITDIALOG gets called quite late, we need to initialize the var here. (related problem caused by spin control)
static inline void SendPSChanged(HWND hDlg){
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
INT_PTR CALLBACK Page_Color(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		return ColorBox_OnMeasureItem(wParam, lParam);
	case WM_DRAWITEM:
		return ColorBox_OnDrawItem(wParam, lParam);
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
			if(id==IDC_COLFORE_BTN || id==IDC_COLBACK_BTN){
				ColorBox_ChooseColor((HWND)lParam);
			}else if(id==IDC_CHOOSEFONT){
				HWND hwndCombo;
				HDC hdc;
				wchar_t size[8];
				LOGFONT lf = {0};
				CHOOSEFONT chosenfont = {sizeof(CHOOSEFONT)};
				chosenfont.hwndOwner = hDlg;
				chosenfont.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS | CF_SCREENFONTS;
				chosenfont.lpLogFont = &lf;
				hwndCombo = GetDlgItem(hDlg,IDC_FONTSIZE);
				ComboBox_GetText(hwndCombo, size, _countof(size));
				hdc = GetDC(NULL);
				lf.lfWeight = IsDlgButtonChecked(hDlg, IDC_BOLD) ? FW_BOLD : FW_REGULAR;
				lf.lfItalic = (BYTE)IsDlgButtonChecked(hDlg, IDC_ITALIC);
				lf.lfHeight = -MulDiv(_wtoi(size), GetDeviceCaps(hdc,LOGPIXELSY), 72);
				ReleaseDC(NULL, hdc);
				hwndCombo = GetDlgItem(hDlg,IDC_FONT);
				ComboBox_GetText(hwndCombo, lf.lfFaceName, _countof(lf.lfFaceName));
				if(ChooseFont(&chosenfont)){
					int sel;
					CheckDlgButton(hDlg, IDC_BOLD, (chosenfont.lpLogFont->lfWeight >= FW_SEMIBOLD));
					CheckDlgButton(hDlg, IDC_ITALIC, chosenfont.lpLogFont->lfItalic);
					ComboBox_SelectString(hwndCombo, -1, chosenfont.lpLogFont->lfFaceName);
					hwndCombo = GetDlgItem(hDlg,IDC_FONTSIZE);
					wsprintf(size, FMT("%d"), chosenfont.iPointSize/10);
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
				api.DelKey(L"Preview");
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
	ColorBox colors[2];
	HWND quality_cb = GetDlgItem(hDlg, IDC_FONTQUAL);
	HDC hdc;
	LOGFONT logfont;
	HFONT hfont;
	m_transition=-1; // start transition lock
	// setting of "background" and "text"
	colors[0].hwnd = GetDlgItem(hDlg, IDC_COLFORE);
	colors[0].color = api.GetInt(L"Clock",L"ForeColor",TCOLOR(TCOLOR_DEFAULT));
	colors[1].hwnd = GetDlgItem(hDlg, IDC_COLBACK);
	colors[1].color = api.GetInt(L"Clock",L"BackColor",TCOLOR(TCOLOR_DEFAULT));
	ColorBox_Setup(colors, 2);
	
	// if color depth is 256 or less
	hdc = CreateICA("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8) {
		EnableDlgItem(hDlg, IDC_COLFORE_BTN, FALSE);
		EnableDlgItem(hDlg, IDC_COLBACK_BTN, FALSE);
	}
	DeleteDC(hdc);
	
	InitComboFont(hDlg);
	
	SetComboFontSize(hDlg, TRUE);
	
	
	CheckDlgButton(hDlg, IDC_BOLD, api.GetInt(L"Clock", L"Bold", 0));
	CheckDlgButton(hDlg, IDC_ITALIC, api.GetInt(L"Clock", L"Italic", 0));
	
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
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"ClockHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE32, (WPARAM)-999,999);
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"ClockWidth", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE32, (WPARAM)-64,64);
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"LineHeight", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"VertPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETRANGE32, (WPARAM)-200,200);
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"HorizPos", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETRANGE32, (WPARAM)-360,360);
	SendDlgItemMessage(hDlg, IDC_SPINANGLE, UDM_SETPOS32, 0, api.GetInt(L"Clock", L"Angle", 0));
	
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETRANGE32, 0,100);
	SendDlgItemMessage(hDlg, IDC_SPINALPHA, UDM_SETPOS32, 0, api.GetInt(L"Taskbar", L"AlphaTaskbar", 0));
	
	ComboBox_AddString(quality_cb, L"Default");            // 0 = DEFAULT_QUALITY
	ComboBox_AddString(quality_cb, L"Draft");              // 1 = DRAFT_QUALITY
	ComboBox_AddString(quality_cb, L"Proof");              // 2 = PROOF_QUALITY
	ComboBox_AddString(quality_cb, L"NonAntiAliased");     // 3 = NONANTIALIASED_QUALITY
	ComboBox_AddString(quality_cb, L"AntiAliased (Win7)"); // 4 = ANTIALIASED_QUALITY
	ComboBox_AddString(quality_cb, L"ClearType (WinXP+)"); // 5 = CLEARTYPE_QUALITY
	ComboBox_AddString(quality_cb, L"ClearType Natural");  // 6 = CLEARTYPE_NATURAL_QUALITY
	ComboBox_SetCurSel(quality_cb,api.GetInt(L"Clock",L"FontQuality",CLEARTYPE_QUALITY));
	m_transition=0; // end transition lock, ready to go
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg,BOOL preview)
{
	const wchar_t* section = (preview ? L"Preview" : L"Clock");
	wchar_t tmp[LF_FACESIZE];
	HWND fore_cb = GetDlgItem(hDlg, IDC_COLFORE);
	HWND back_cb = GetDlgItem(hDlg, IDC_COLBACK);
	HWND font_cb = GetDlgItem(hDlg, IDC_FONT);
	HWND size_cb = GetDlgItem(hDlg, IDC_FONTSIZE);
	int sel;
	
	api.SetInt(section, L"ForeColor", ColorBox_GetColorRaw(fore_cb));
	api.SetInt(section, L"BackColor", ColorBox_GetColorRaw(back_cb));
	
	ComboBox_GetLBText(font_cb, ComboBox_GetCurSel(font_cb), tmp);
	api.SetStr(section, L"Font", tmp);
	
	sel = ComboBox_GetCurSel(size_cb);
	if(sel == -1)
		ComboBox_GetText(size_cb, tmp, _countof(tmp));
	else
		ComboBox_GetLBText(size_cb, sel, tmp);
	api.SetInt(section, L"FontSize", _wtoi(tmp));
	
	api.SetInt(section, L"Bold",   IsDlgButtonChecked(hDlg, IDC_BOLD));
	api.SetInt(section, L"Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
	
	api.SetInt(section, L"FontQuality", ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_FONTQUAL)));
	
	api.SetInt(section, L"ClockHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINCHEIGHT,UDM_GETPOS32,0,0));
	api.SetInt(section, L"ClockWidth", (int)SendDlgItemMessage(hDlg,IDC_SPINCWIDTH,UDM_GETPOS32,0,0));
	api.SetInt(section, L"LineHeight", (int)SendDlgItemMessage(hDlg,IDC_SPINLHEIGHT,UDM_GETPOS32,0,0));
	api.SetInt(section, L"VertPos", (int)SendDlgItemMessage(hDlg,IDC_SPINVPOS,UDM_GETPOS32,0,0));
	api.SetInt(section, L"HorizPos", (int)SendDlgItemMessage(hDlg,IDC_SPINHPOS,UDM_GETPOS32,0,0));
	api.SetInt(section, L"Angle", (int)SendDlgItemMessage(hDlg,IDC_SPINANGLE,UDM_GETPOS32,0,0));
	
	api.SetInt((preview ? section : L"Taskbar"), L"AlphaTaskbar", (int)SendDlgItemMessage(hDlg,IDC_SPINALPHA,UDM_GETPOS32,0,0));
	if(!preview){
		api.DelKey(L"Preview");
		m_transition = 0;
	}else
		m_transition = 1;
}

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf = {0};
	HWND hcombo;
	wchar_t font[LF_FACESIZE];
	int i;
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	hcombo = GetDlgItem(hDlg, IDC_FONT);
	
	lf.lfCharSet = DEFAULT_CHARSET;  // fonts from any charset
	EnumFontFamiliesEx(hdc, &lf, EnumFontFamExProc, (LPARAM)hcombo, 0);
	

	ReleaseDC(NULL, hdc);
	
	api.GetStrEx(L"Clock", L"Font", font, _countof(font), L"Arial");
	
	i = ComboBox_FindStringExact(hcombo, -1, font);
	if(i == LB_ERR)
		i = 0;
	ComboBox_SetCurSel(hcombo, i);
}

/*------------------------------------------------
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	wchar_t font[LF_FACESIZE];
	int size;
	LOGFONT lf = {0};
	HWND size_cb = GetDlgItem(hDlg, IDC_FONTSIZE);
	HWND font_cb = GetDlgItem(hDlg, IDC_FONT);
	int pos;
	
	// remember old size
	if(bInit) { // on WM_INITDIALOG
		size = api.GetInt(L"Clock", L"FontSize", 9);
		if(!size || size>100) size = 9;
	} else { // when IDC_FONT has been changed
		ComboBox_GetText(size_cb, font, _countof(font));
		size = _wtoi(font);
	}
	
	ComboBox_ResetContent(size_cb);
	
	hdc = GetDC(NULL);
	m_logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	ComboBox_GetLBText(font_cb, ComboBox_GetCurSel(font_cb), font);
	
	wcscpy(lf.lfFaceName, font);
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, EnumSizeProcEx,
					   (LPARAM)size_cb, 0);
					   
	ReleaseDC(NULL, hdc);
	
	wsprintf(font, FMT("%d"), size);
	pos = ComboBox_FindStringExact(size_cb, -1, font);
	if(pos != LB_ERR) {
		ComboBox_SetCurSel(size_cb, pos);
		return;
	}
	ComboBox_SetText(size_cb, font);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	HWND hwndSize = (HWND)lParam;
	(void)lpntme; (void)FontType;
	if(lpelfe->lfFaceName[0]!='@') {
		ComboBox_AddStringOnce(hwndSize, lpelfe->lfFaceName, 0, -1);
	}
	return 1;
}

/*------------------------------------------------
--------------------------------------------------*/
int CALLBACK EnumSizeProcEx(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	HWND hwndSize = (HWND)lParam;
	static const unsigned char nFontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22,24,26,28,32,36,48,72};
	wchar_t str[8];
	int i, size, count;
	(void)lpelfe;
	
	// is modern font which supports any size?
	if((FontType&TRUETYPE_FONTTYPE) || !(FontType&RASTER_FONTTYPE)) {
		for(i=0; i<_countof(nFontSizes); ++i) {
			wsprintf(str, FMT("%u"), (unsigned)nFontSizes[i]);
			ComboBox_AddString(hwndSize, str);
		}
		return 0;
	}
	
	// only add supported sizes for raster type fonts
	size = (lpntme->tmHeight - lpntme->tmInternalLeading) * 72 / m_logpixelsy;
	count = ComboBox_GetCount(hwndSize);
	for(i=0; i<count; ++i) { // dupes check + sorting
		ComboBox_GetLBText(hwndSize, i, str);
		if(size == _wtoi(str))
			return 1;
		else if(size < _wtoi(str)) {
			wsprintf(str, FMT("%d"), size);
			ComboBox_InsertString(hwndSize, i, str);
			return 1;
		}
	}
	wsprintf(str, FMT("%d"), size);
	ComboBox_AddString(hwndSize, str);
	return 1;
}
