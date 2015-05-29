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
			if(id==IDC_CHOOSECOLFORE || id==IDC_CHOOSECOLBACK){
				ColorBox_ChooseColor((HWND)lParam);
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
	ColorBox colors[2];
	HWND quality_cb = GetDlgItem(hDlg, IDC_FONTQUAL);
	HDC hdc;
	LOGFONT logfont;
	HFONT hfont;
	m_transition=-1; // start transition lock
	// setting of "background" and "text"
	colors[0].hwnd = GetDlgItem(hDlg, IDC_COLFORE);
	colors[0].color = api.GetInt("Clock","ForeColor",TCOLOR(TCOLOR_DEFAULT));
	colors[1].hwnd = GetDlgItem(hDlg, IDC_COLBACK);
	colors[1].color = api.GetInt("Clock","BackColor",TCOLOR(TCOLOR_DEFAULT));
	ColorBox_Setup(colors, 2);
	
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
	
	api.SetInt(section, "ForeColor", ColorBox_GetColorRaw(fore_cb));
	api.SetInt(section, "BackColor", ColorBox_GetColorRaw(back_cb));
	
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
