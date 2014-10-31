/*-------------------------------------------
  pageformat.c
  "Format" page of properties
                       KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"
#define MAX_FORMAT 256

static char* m_entrydate[FORMAT_NUM]={
	"Year4", "Year", "Month", "MonthS", "Day", "Weekday",
	"Hour", "Minute", "Second", "AMPM", "InternetTime",
	"Kaigyo", "Hour12", "HourZero", "Custom",
};
#define ENTRY(id) m_entrydate[(id)-FORMAT_BEGIN]
#define CHECKS(id) checks[(id)-FORMAT_BEGIN]
static void CreateFormat(char* s, char* checks);

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg,BOOL preview);
static void OnLocale(HWND hDlg);
static void OnCustom(HWND hDlg, BOOL bmouse);
static void OnFormatCheck(HWND hDlg, WORD id);

static int m_ilang;  // language code. ex) 0x411 - Japanese
static int m_idate;  // 0: mm/dd/yy 1: dd/mm/yy 2: yy/mm/dd
static char* m_pCustomFormat = NULL;
static char m_sMon[10];  //

static char m_bDayOfWeekIsLast;   // yy/mm/dd ddd
static char m_bTimeMarkerIsFirst; // AM/PM hh:nn:ss

static char m_sep_date[4]; // date seperator such as / .
static char m_sep_time[4]; // time seperator such as : .


static char m_transition=-1; // can become a problem if not initializes.. see pagecolor.c
static __inline void SendPSChanged(HWND hDlg){
	if(m_transition==-1) return;
	g_bApplyClock = 1;
	g_bApplyTaskbar = 1;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	
	OnApply(hDlg,1);
	SendMessage(g_hwndClock, CLOCKM_REFRESHCLOCKPREVIEWFORMAT, 0, 0);
}
/*------------------------------------------------
   Dialog Procedure for the "Format" page
--------------------------------------------------*/
INT_PTR CALLBACK PageFormatProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:
		if(m_pCustomFormat) {
			free(m_pCustomFormat);
			m_pCustomFormat = NULL;
		}
		break;
	case WM_COMMAND: {
		WORD id=LOWORD(wParam);
		switch(id){
		case IDC_LOCALE:
			if(HIWORD(wParam)==CBN_SELCHANGE)
				OnLocale(hDlg);
			break;
		case IDC_CUSTOM:
			OnCustom(hDlg, TRUE);
			break;
		case IDC_AMSYMBOL:
		case IDC_PMSYMBOL:
			if(HIWORD(wParam)==CBN_EDITCHANGE || HIWORD(wParam)==CBN_SELCHANGE)
				SendPSChanged(hDlg);
			break;
		case IDC_ZERO:
		case IDC_FORMAT:
			SendPSChanged(hDlg);
			break;
		default: // "year" -- "Internet Time"
			if(id>=IDC_YEAR4 && id<=IDC_12HOUR)
				OnFormatCheck(hDlg, id);
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
  Initialize Checks with locale settings (2nd used to overwrite values based on locale default instead of users)
--------------------------------------------------*/
void ChecksLocaleInit(char checks[FORMAT_NUM], int ilang/*=0*/)
{
	char ampm[10];
	int ival;
	const int aLangDayOfWeekIsLast[]={LANG_JAPANESE,LANG_KOREAN};
	char bTimeMarker; // use AM/PM (+12h format)
	
	if(ilang)
		m_ilang=ilang;
	else
		m_ilang = GetMyRegLong("Format", "Locale", GetUserDefaultLangID());
	/// init language "member" variables
	// seperator
	GetLocaleInfo(m_ilang,LOCALE_SDATE,m_sep_date,sizeof(m_sep_date));
	GetLocaleInfo(m_ilang,LOCALE_STIME,m_sep_time,sizeof(m_sep_time));
	// AM/PM (seems to always use user's choice, not matter which locale. Also true for "is first"?)
	*ampm='\0';
	GetLocaleInfo(m_ilang,LOCALE_S1159,ampm,sizeof(ampm));/*!AM*/
	if(!*ampm)
		GetLocaleInfo(m_ilang,LOCALE_S2359,ampm,sizeof(ampm));/*!PM*/
	bTimeMarker=*ampm; // use 24h w/o AM/PM or 12h w/ AM/PM based on user locale
	GetLocaleInfo(m_ilang, LOCALE_ITIMEMARKPOSN|LOCALE_RETURN_NUMBER, (LPSTR)&ival, sizeof(ival));
	m_bTimeMarkerIsFirst=(char)ival;
	// date
	GetLocaleInfo(m_ilang, LOCALE_IDATE|LOCALE_RETURN_NUMBER, (LPSTR)&m_idate, sizeof(m_idate));
	GetLocaleInfo(m_ilang, LOCALE_SABBREVDAYNAME1, m_sMon, sizeof(m_sMon));
	
	m_bDayOfWeekIsLast = 0;
	for(ival=0; ival<(sizeof(aLangDayOfWeekIsLast)/sizeof(aLangDayOfWeekIsLast[0])); ++ival) {
		if((m_ilang&0x00ff) == aLangDayOfWeekIsLast[ival]) {
			m_bDayOfWeekIsLast = 1; break;
		}
	}
	/// init checks
	for(ival=IDC_YEAR4; ival<=IDC_SECOND; ++ival) {
		CHECKS(ival) = (char)GetMyRegLong("Format", ENTRY(ival), 1);
	}
	
	if(CHECKS(IDC_YEAR)) CHECKS(IDC_YEAR4) = 0;
	else if(CHECKS(IDC_YEAR4)) CHECKS(IDC_YEAR) = 0;
	
	if(CHECKS(IDC_MONTH)) CHECKS(IDC_MONTHS) = 0;
	else if(CHECKS(IDC_MONTHS)) CHECKS(IDC_MONTH) = 0;
	
	for(ival=IDC_AMPM; ival<=IDC_CUSTOM; ++ival) {
		CHECKS(ival) = (char)GetMyRegLong("Format", ENTRY(ival), 0);
	}
	/// init locale based checks
	// use AM/PM and 12h format from new selected locale
	CHECKS(IDC_AMPM)=(char)bTimeMarker;
	CHECKS(IDC_12HOUR)=(char)bTimeMarker;
	// leading zero for 12h format // 05 vs 5 am
	GetLocaleInfo(m_ilang, LOCALE_ITLZERO|LOCALE_RETURN_NUMBER, (LPSTR)&ival, sizeof(ival));
	CHECKS(IDC_HOUR)=(ival?2:1);
	if(!ilang){
		CHECKS(IDC_AMPM)=(char)GetMyRegLongEx("Format",ENTRY(IDC_AMPM),CHECKS(IDC_AMPM));
		CHECKS(IDC_12HOUR)=(char)GetMyRegLongEx("Format",ENTRY(IDC_12HOUR),CHECKS(IDC_12HOUR));
		CHECKS(IDC_HOUR)=(char)GetMyRegLongEx("Format",ENTRY(IDC_HOUR),CHECKS(IDC_HOUR));
	}
}

/*------------------------------------------------
  Initialize Locale Infomation (2nd to 3rd param used to overwrite values based on locale default)
--------------------------------------------------*/
void Checks2Dialog(char checks[FORMAT_NUM], HWND hDlg, int bLocaleOnly/*=0*/)
{
	if(!bLocaleOnly){
		int i;
		for(i=FORMAT_BEGIN; i<=FORMAT_END; ++i) {
			CheckDlgButton(hDlg,i,CHECKS(i));
		}
	}else{
		// use AM/PM and 12h format from new selected locale
		CheckDlgButton(hDlg,IDC_AMPM,CHECKS(IDC_AMPM));
		CheckDlgButton(hDlg,IDC_12HOUR,CHECKS(IDC_12HOUR));
		// leading zero for 12h format
		CheckDlgButton(hDlg,IDC_HOUR,CHECKS(IDC_HOUR));
	}
}

static HWND m_hwndPage;
/*------------------------------------------------
  for EnumSystemLocales function
--------------------------------------------------*/
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	char str[80];
	int x, index;
	
	x = atox(lpLocaleString);
	if(GetLocaleInfo(x, LOCALE_SLANGUAGE, str, sizeof(str)) > 0)
		index = (int)CBAddString(m_hwndPage, IDC_LOCALE, str);
	else
		index = (int)CBAddString(m_hwndPage, IDC_LOCALE, lpLocaleString);
	CBSetItemData(m_hwndPage, IDC_LOCALE, index, x);
	return TRUE;
}

/*------------------------------------------------
  Initialize the "Format" page
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	const char* AM[]={"AM","am","A","a"," ",};
	const char* PM[]={"PM","pm","P","p"," ",};
	const int AMPMs=sizeof(AM)/sizeof(AM[0]);
	HFONT hfont;
	char fmt[MAX_FORMAT];
	int i, count;
	char ampm_user[TNY_BUFF];
	char ampm_locale[TNY_BUFF];
	char checks[FORMAT_NUM];
	
	m_transition=-1; // start transition lock
	m_hwndPage = hDlg;
	
//	hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT); // pixel, bold
//	hfont = (HFONT)GetStockObject(ANSI_FIXED_FONT); // pixel, "normal"
	hfont = (HFONT)GetStockObject(OEM_FIXED_FONT); // cleartype, bold (same as console?)
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FORMAT, WM_SETFONT, (WPARAM)hfont, 0);
		
	// Fill and select the "Locale" combobox
	EnumSystemLocales(EnumLocalesProc, LCID_INSTALLED);
	CBSetCurSel(hDlg, IDC_LOCALE, 0);
	
	ChecksLocaleInit(checks,0);
	Checks2Dialog(checks,hDlg,0);
	count = (int)CBGetCount(hDlg, IDC_LOCALE);
	for(i=0; i < count; ++i) {
		int x;
		x = (int)CBGetItemData(hDlg, IDC_LOCALE, i);
		if(x == m_ilang) {
			CBSetCurSel(hDlg, IDC_LOCALE, i); break;
		}
	}
	
	GetMyRegStr("Format", "Format", fmt, MAX_FORMAT, "");
	SetDlgItemText(hDlg, IDC_FORMAT, fmt);
	
	m_pCustomFormat = malloc(MAX_FORMAT);
	if(m_pCustomFormat)
		GetMyRegStr("Format", "CustomFormat", m_pCustomFormat, MAX_FORMAT, "");
	
	// "AM Symbol" and "PM Symbol"
	CBResetContent(hDlg, IDC_AMSYMBOL);
	GetMyRegStr("Format", "AMsymbol", ampm_user, sizeof(ampm_user), "");
	if(*ampm_user)
		CBAddString(hDlg, IDC_AMSYMBOL, ampm_user);
	if(GetLocaleInfo(m_ilang, LOCALE_S1159, ampm_locale, sizeof(ampm_locale)) && strcmp(ampm_user,ampm_locale))
		CBAddString(hDlg,IDC_AMSYMBOL,ampm_locale);
	else
		*ampm_locale='\0';
	for(i=0; i<AMPMs; ++i){
		if(strcmp(ampm_locale,AM[i]) && strcmp(ampm_user,AM[i]))
			CBAddString(hDlg,IDC_AMSYMBOL,AM[i]);
	}
	CBSetCurSel(hDlg, IDC_AMSYMBOL, 0);
	
	CBResetContent(hDlg, IDC_PMSYMBOL);
	GetMyRegStr("Format", "PMsymbol", ampm_user, sizeof(ampm_user), "");
	if(*ampm_user)
		CBAddString(hDlg, IDC_PMSYMBOL, ampm_user);
	if(GetLocaleInfo(m_ilang, LOCALE_S2359, ampm_locale, sizeof(ampm_locale)) && strcmp(ampm_user,ampm_locale))
		CBAddString(hDlg,IDC_PMSYMBOL,ampm_locale);
	else
		*ampm_locale='\0';
	for(i=0; i<AMPMs; ++i){
		if(strcmp(ampm_locale,PM[i]) && strcmp(ampm_user,PM[i]))
			CBAddString(hDlg,IDC_PMSYMBOL,PM[i]);
	}
	CBSetCurSel(hDlg, IDC_PMSYMBOL, 0);
	
	OnCustom(hDlg, FALSE);
	m_transition=0; // end transition lock, ready to go
}

//================================================================================================
//---------------------------------------------------------------------------+++--> "Apply" button:
void OnApply(HWND hDlg,BOOL preview)   //---------------------------------------------------+++-->
{
	const char* section=preview?"Preview":"Format";
	char str[MAX_FORMAT];
	int i;
	
	SetMyRegLong(section, "Locale",
				 (DWORD)CBGetItemData(hDlg, IDC_LOCALE, CBGetCurSel(hDlg, IDC_LOCALE)));
				 
	for(i = IDC_YEAR4; i <= IDC_CUSTOM; i++) {
		SetMyRegLong(section, ENTRY(i), IsDlgButtonChecked(hDlg, i));
	}
	
	i=(int)SendDlgItemMessage(hDlg,IDC_AMSYMBOL,CB_GETCURSEL,0,0);
	if(i!=CB_ERR)
		SendDlgItemMessage(hDlg,IDC_AMSYMBOL,CB_GETLBTEXT,i,(LPARAM)str);
	else
		GetDlgItemText(hDlg, IDC_AMSYMBOL, str, sizeof(str));
	SetMyRegStr(section, "AMsymbol", str);
	i=(int)SendDlgItemMessage(hDlg,IDC_PMSYMBOL,CB_GETCURSEL,0,0);
	if(i!=CB_ERR)
		SendDlgItemMessage(hDlg,IDC_PMSYMBOL,CB_GETLBTEXT,i,(LPARAM)str);
	else
		GetDlgItemText(hDlg, IDC_PMSYMBOL, str, sizeof(str));
	SetMyRegStr(section, "PMsymbol", str);
	
	GetDlgItemText(hDlg, IDC_FORMAT, str, sizeof(str));
	SetMyRegStr(section, "Format", str);
	
	if(m_pCustomFormat) {
		if(IsDlgButtonChecked(hDlg, IDC_CUSTOM)) {
			strcpy(m_pCustomFormat, str);
			SetMyRegStr(section, "CustomFormat", m_pCustomFormat);
		}
	}
	if(!preview){
		DelMyRegKey("Preview");
		m_transition=0;
	}else
		m_transition=1;
}
//================================================================================================
//-------------------------------------------+++--> When User's Location (Locale ComboBox) Changes:
void OnLocale(HWND hDlg)   //---------------------------------------------------------------+++-->
{
	char checks[FORMAT_NUM];
	char fmt[MAX_FORMAT];
	int ilang=(int)CBGetItemData(hDlg,IDC_LOCALE,CBGetCurSel(hDlg,IDC_LOCALE));
	// change locale
	ChecksLocaleInit(checks,ilang);
	Checks2Dialog(checks,hDlg,1);
	// update format
	CreateFormat(fmt,checks);
	SetDlgItemText(hDlg,IDC_FORMAT,fmt);
}
//================================================================================================
//-----------------------------------+++--> Handler for Enable/Disable "Customize format" CheckBox:
void OnCustom(HWND hDlg, BOOL bmouse)   //--------------------------------------------------+++-->
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_CUSTOM);
	EnableDlgItem(hDlg, IDC_FORMAT, b);
	
	for(i = IDC_YEAR4; i <= IDC_12HOUR; i++)
		EnableDlgItem(hDlg, i, !b);
	
	if(m_pCustomFormat && bmouse) {
		if(b) {
			if(m_pCustomFormat[0])
				SetDlgItemText(hDlg, IDC_FORMAT, m_pCustomFormat);
		} else {
			GetDlgItemText(hDlg, IDC_FORMAT, m_pCustomFormat, MAX_FORMAT);
		}
	}
	
	if(!b) OnFormatCheck(hDlg, 0);
	SendPSChanged(hDlg);
}
/*------------------------------------------------
  When clicked "year" -- "am/pm"
--------------------------------------------------*/

void OnFormatCheck(HWND hDlg, WORD id)
{
	int i;
	char fmt[MAX_FORMAT];
	char checks[FORMAT_NUM];
	char oldtransition=m_transition;
	m_transition=-1; // start transition lock
	
	for(i = IDC_YEAR4; i <= IDC_12HOUR; ++i) {
		CHECKS(i) = (char)IsDlgButtonChecked(hDlg, i);
	}
	
	if(id == IDC_YEAR4 || id == IDC_YEAR) {
		if(id == IDC_YEAR4 && CHECKS(IDC_YEAR4)) {
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
			CHECKS(IDC_YEAR) = FALSE;
		}
		if(id == IDC_YEAR && CHECKS(IDC_YEAR)) {
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
			CHECKS(IDC_YEAR4) = FALSE;
		}
	}
	
	else if(id == IDC_MONTH || id == IDC_MONTHS) {
		if(id == IDC_MONTH && CHECKS(IDC_MONTH)) {
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
			CHECKS(IDC_MONTHS) = FALSE;
		}
		if(id == IDC_MONTHS && CHECKS(IDC_MONTHS)) {
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
			CHECKS(IDC_MONTH) = FALSE;
		}
	}
	
	else if(id == IDC_AMPM) {
		CHECKS(IDC_12HOUR) = 1;
		CheckDlgButton(hDlg, IDC_12HOUR, 1);
	}
	else if(id == IDC_12HOUR && !IsDlgButtonChecked(hDlg,IDC_12HOUR)) {
		CHECKS(IDC_AMPM) = 0;
		CheckDlgButton(hDlg, IDC_AMPM, 0);
	}
	
	CreateFormat(fmt, checks);
	SetDlgItemText(hDlg, IDC_FORMAT, fmt);
	m_transition=oldtransition; // end transition lock
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  Initialize a format string. Called from main.c
--------------------------------------------------*/
void InitFormat()
{
	char format[LRG_BUFF];
	char checks[FORMAT_NUM];
	
	if(GetMyRegLong("Format", ENTRY(IDC_CUSTOM), FALSE))
		return;
	ChecksLocaleInit(checks,0);	
	CreateFormat(format, checks);
	SetMyRegStr("Format", "Format", format);
}

/*--------------------------------------------------
=============== Create a format string automatically
--------------------------------------------------*/
void CreateFormat(char* dst, char* checks)
{
	char bdate=0, btime=0;
	int control;
	
	for(control=IDC_YEAR4; control<=IDC_WEEKDAY; ++control) {
		if(CHECKS(control)) {
			bdate=1;
			break;
		}
	}
	
	for(control=IDC_HOUR; control<=IDC_INTERNETTIME; ++control) {
		if(CHECKS(control)) {
			btime=1;
			break;
		}
	}
	
	*dst = '\0';
	
	if(!m_bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY)) {
		strcat(dst, "ddd");
		for(control=IDC_YEAR4; control<=IDC_DAY; ++control) {
			if(CHECKS(control)) {
				if((m_ilang&0x00ff) == LANG_CHINESE) strcat(dst," ");
				else if(m_sMon[0] && m_sMon[ strlen(m_sMon) - 1 ] == '.')
					strcat(dst," ");
				else strcat(dst, ", ");
				break;
			}
		}
	}
	
	switch(m_idate){
	case 0: // m/d/y
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)) {
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY) || CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) {
				if(CHECKS(IDC_MONTH)) strcat(dst, m_sep_date);
				else strcat(dst," ");
			}
		}
		if(CHECKS(IDC_DAY)) {
			strcat(dst, "dd");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) {
				if(CHECKS(IDC_MONTH)) strcat(dst, m_sep_date);
				else strcat(dst, ", ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
		break;
	case 1: // d/m/y
		if(CHECKS(IDC_DAY)) {
			strcat(dst, "dd");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)) {
				if(CHECKS(IDC_MONTH)) strcat(dst, m_sep_date);
				else strcat(dst," ");
			} else if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) strcat(dst, m_sep_date);
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)) {
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) {
				if(CHECKS(IDC_MONTH)) strcat(dst, m_sep_date);
				else strcat(dst," ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
		break;
	default:  // y/m/d
		if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) {
			if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
			if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)
			   || CHECKS(IDC_DAY)) {
				if(CHECKS(IDC_MONTHS)) strcat(dst," ");
				else strcat(dst, m_sep_date);
			}
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)) {
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY)) {
				if(CHECKS(IDC_MONTHS)) strcat(dst," ");
				else strcat(dst, m_sep_date);
			}
		}
		if(CHECKS(IDC_DAY)) strcat(dst, "dd");
	}
	
	if(m_bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY)) {
		for(control=IDC_YEAR4; control<=IDC_DAY; ++control) {
			if(CHECKS(control)) { strcat(dst," "); break; }
		}
		strcat(dst, "ddd");
	}
	
	if(bdate && btime) {
		if(CHECKS(IDC_KAIGYO))
			strcat(dst,"\\n");
		else{
			if(m_idate < 2 && CHECKS(IDC_MONTHS) &&
			   (CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)))
				strcat(dst," ");
			strcat(dst," ");
		}
	}
	
	if(m_bTimeMarkerIsFirst && CHECKS(IDC_AMPM)) {
		strcat(dst, "tt");
		if(btime) strcat(dst," ");
	}
	btime=0;
	
	if(CHECKS(IDC_HOUR)) {
		if(CHECKS(IDC_12HOUR) && CHECKS(IDC_HOUR)!=BST_INDETERMINATE)
			strcat(dst, "h");
		else
			strcat(dst, "hh");
		++btime;
	}
	if(CHECKS(IDC_MINUTE)) {
		if(btime++) strcat(dst, m_sep_time);
		strcat(dst, "nn");
	}
	if(CHECKS(IDC_SECOND)) {
		if(btime++) strcat(dst, m_sep_time);
		strcat(dst, "ss");
	}
	
	if(!m_bTimeMarkerIsFirst && CHECKS(IDC_AMPM)) {
		if(btime) strcat(dst," ");
		strcat(dst, "tt");
	}
	
	if(CHECKS(IDC_INTERNETTIME)){
		if(btime||CHECKS(IDC_AMPM)) strcat(dst," ");
		strcat(dst,"@@@");
	}
}
