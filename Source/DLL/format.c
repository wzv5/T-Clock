//---[s]--- For InternetTime 99/03/16@211 M.Takemura -----

/*------------------------------------------------------------------------
// format.c : to make a string to display in the clock -> KAZUBON 1997-1998
//-----------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Friday, 12/16/2011 @ 3:36:00pm
#include "tcdll.h"

int codepage = CP_ACP;
static char MonthShort[11], MonthLong[31];
static char DayOfWeekShort[11], DayOfWeekLong[31];
static char* DayOfWeekEng[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char* MonthEng[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char AM[11], PM[11], SDate[5], STime[5];
static char EraStr[11];
static int AltYear;

extern char g_bHour12, g_bHourZero;

//================================================================================================
//---------------------------------//+++--> load Localized Strings for Month, Day, & AM/PM Symbols:
void InitFormat(SYSTEMTIME* lt)   //--------------------------------------------------------+++-->
{
	char s[80], *p;
	int i, ilang, ioptcal;
	
	ilang = GetMyRegLong(NULL, "Locale", (int)GetUserDefaultLangID());
	codepage = CP_ACP;
	
	if(GetLocaleInfo((WORD)ilang, LOCALE_IDEFAULTANSICODEPAGE, s, 10) > 0) {
		p = s; codepage = 0;
		while('0' <= *p && *p <= '9') codepage = codepage * 10 + *p++ - '0';
		if(!IsValidCodePage(codepage)) codepage = CP_ACP;
	}
	
	i = lt->wDayOfWeek; i--; if(i < 0) i = 6;
	
	GetLocaleInfo((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i, DayOfWeekShort, 10);
	GetLocaleInfo((WORD)ilang, LOCALE_SDAYNAME1 + i, DayOfWeekLong, 30);
	
	i = lt->wMonth; i--;
	GetLocaleInfo((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i, MonthShort, 10);
	GetLocaleInfo((WORD)ilang, LOCALE_SMONTHNAME1 + i, MonthLong, 30);
	
	GetLocaleInfo((WORD)ilang, LOCALE_S1159, AM, 10);
	GetMyRegStr("Format", "AMsymbol", s, 80, AM);
	if(s[0] == 0) strcpy(s, "AM");
	strcpy(AM, s);
	
	GetLocaleInfo((WORD)ilang, LOCALE_S2359, PM, 10);
	GetMyRegStr("Format", "PMsymbol", s, 80, PM);
	if(s[0] == 0) strcpy(s, "PM");
	strcpy(PM, s);
	
	GetLocaleInfo((WORD)ilang, LOCALE_SDATE, SDate, 4);
	GetLocaleInfo((WORD)ilang, LOCALE_STIME, STime, 4);
	
	EraStr[0] = 0;
	AltYear = -1;
	
	ioptcal = 0;
	if(GetLocaleInfo((WORD)ilang, LOCALE_IOPTIONALCALENDAR, s, 10)) {
		ioptcal = 0;
		p = s;
		
		while('0' <= *p && *p <= '9') ioptcal = ioptcal * 10 + *p++ - '0';
	}
	
	if(ioptcal < 3) ilang = LANG_USER_DEFAULT;
	
	if(GetDateFormat((WORD)ilang, DATE_USE_ALT_CALENDAR, lt, "gg", s, 12) != 0) strcpy(EraStr, s);
	
	if(GetDateFormat((WORD)ilang, DATE_USE_ALT_CALENDAR, lt, "yyyy", s, 6) != 0) {
		if(s[0]) {
			p = s;
			AltYear = 0;
			while('0' <= *p && *p <= '9') AltYear = AltYear * 10 + *p++ - '0';
		}
	}
}
//================================================================================================
//--+++-->
BOOL GetNumFormat(const char** sp, char x, int* len, int* slen)
{
	const char* p;
	int n, ns;
	
	p = *sp;
	n = 0;
	ns = 0;
	
	while(*p == '_') {
		ns++;
		p++;
	}
	if(*p != x) return FALSE;
	while(*p == x) {
		n++;
		p++;
	}
	
	*len = n+ns;
	*slen = ns;
	*sp = p;
	return TRUE;
}
//================================================================================================
//--+++-->
void SetNumFormat(char** dp, int n, int len, int slen)
{
	char* p;
	int minlen,i;
	
	p = *dp;
	
	for(i=10,minlen=1; i<1000000000; i*=10,minlen++)
		if(n < i) break;
		
	while(minlen < len) {
		if(slen > 0) { *p++ = ' '; slen--; }
		else { *p++ = '0'; }
		len--;
	}
	
	for(i=minlen-1; i>=0; i--) {
		*(p+i) = (char)((n%10)+'0');
		n/=10;
	}
	p += minlen;
	
	*dp = p;
}
//================================================================================================
//-------------+++--> Format T-Clock's OutPut String From Current Date, Time, & System Information:
unsigned MakeFormat(char* buf, const char* fmt, SYSTEMTIME* pt, int beat100)   //------------------+++-->
{
	const char* pos;
	char* out = buf;
	DWORD TickCount = 0;
	
	while(*fmt) {
		if(*fmt == '\"') {
			++fmt;
			while(*fmt != '\"' && *fmt) {
				for(pos=CharNext(fmt); fmt!=pos; )
					*out++=*fmt++;
			}
			if(*fmt == '\"') ++fmt;
		} else if(*fmt=='\\' && fmt[1]=='n') {
			fmt+=2;
			*out++='\n';
		} else if(*fmt == '/') {
			++fmt;
			for(pos=SDate; *pos; ) *out++=*pos++;
		} else if(*fmt == ':') {
			++fmt;
			for(pos=STime; *pos; ) *out++=*pos++;
		}
		/// for testing
		else if(*fmt == 'S' && fmt[1] == 'S' && fmt[2] == 'S') {
			fmt += 3;
			SetNumFormat(&out, (int)pt->wMilliseconds, 3, 0);
		}
		
		else if(*fmt == 'y' && fmt[1] == 'y') {
			int len;
			len = 2;
			if(*(fmt + 2) == 'y' && *(fmt + 3) == 'y') len = 4;
			
			SetNumFormat(&out, (len==2)?(int)pt->wYear%100:(int)pt->wYear, len, 0);
			fmt += len;
		} else if(*fmt == 'm') {
			if(*(fmt + 1) == 'm' && *(fmt + 2) == 'e') {
				*out++ = MonthEng[pt->wMonth-1][0];
				*out++ = MonthEng[pt->wMonth-1][1];
				*out++ = MonthEng[pt->wMonth-1][2];
				fmt += 3;
			} else if(fmt[1] == 'm' && fmt[2] == 'm') {
				if(*(fmt + 3) == 'm') {
					fmt += 4;
					for(pos=MonthLong; *pos; ) *out++=*pos++;
				} else {
					fmt += 3;
					for(pos=MonthShort; *pos; ) *out++=*pos++;
				}
			} else {
				if(fmt[1] == 'm') {
					fmt += 2;
					*out++ = (char)((int)pt->wMonth / 10) + '0';
				} else {
					++fmt;
					if(pt->wMonth > 9)
						*out++ = (char)((int)pt->wMonth / 10) + '0';
				}
				*out++ = (char)((int)pt->wMonth % 10) + '0';
			}
		} else if(*fmt == 'a' && fmt[1] == 'a' && fmt[2] == 'a') {
			if(*(fmt + 3) == 'a') {
				fmt += 4;
				for(pos=DayOfWeekLong; *pos; ) *out++=*pos++;
			} else {
				fmt += 3;
				for(pos=DayOfWeekShort; *pos; ) *out++=*pos++;
			}
		} else if(*fmt == 'd') {
			if(*(fmt + 1) == 'd' && *(fmt + 2) == 'e') {
				fmt += 3;
				for(pos=DayOfWeekEng[pt->wDayOfWeek]; *pos; ) *out++=*pos++;
			} else if(fmt[1] == 'd' && fmt[2] == 'd') {
				if(fmt[3] == 'd') {
					fmt += 4;
					for(pos=DayOfWeekLong; *pos; ) *out++=*pos++;
				} else {
					fmt += 3;
					for(pos=DayOfWeekShort; *pos; ) *out++=*pos++;
				}
			} else {
				if(fmt[1] == 'd') {
					fmt += 2;
					*out++ = (char)((int)pt->wDay / 10) + '0';
				} else {
					++fmt;
					if(pt->wDay > 9)
						*out++ = (char)((int)pt->wDay / 10) + '0';
				}
				*out++ = (char)((int)pt->wDay % 10) + '0';
			}
		} else if(*fmt == 'h') {
			int hour;
			hour = pt->wHour;
			if(g_bHour12) {
				if(hour > 12) hour -= 12;
				else if(hour == 0) hour = 12;
				if(hour == 12 && g_bHourZero) hour = 0;
			}
			if(fmt[1] == 'h') {
				fmt += 2;
				*out++ = (char)(hour / 10) + '0';
			} else {
				++fmt;
				if(hour > 9) {
					*out++ = (char)(hour / 10) + '0';
				}
			}
			*out++ = (char)(hour % 10) + '0';
		} else if(*fmt == 'w' && (fmt[1]=='+'||fmt[1]=='-')) {
			char bAdd=*++fmt=='+';
			int hour=0;
			for(; *++fmt<='9'&&*fmt>='0'; ){
				hour*=10;
				hour+=*fmt-'0';
			}
			if(!bAdd) hour=-hour;
			hour = (pt->wHour + hour)%24;
			if(hour < 0) hour += 24;
			if(g_bHour12) {
				if(hour > 12) hour -= 12;
				else if(hour == 0) hour = 12;
				if(hour == 12 && g_bHourZero) hour = 0;
			}
			*out++ = (char)(hour / 10) + '0';
			*out++ = (char)(hour % 10) + '0';
		} else if(*fmt == 'n') {
			if(fmt[1] == 'n') {
				fmt += 2;
				*out++ = (char)((int)pt->wMinute / 10) + '0';
			} else {
				++fmt;
				if(pt->wMinute > 9)
					*out++ = (char)((int)pt->wMinute / 10) + '0';
			}
			*out++ = (char)((int)pt->wMinute % 10) + '0';
		} else if(*fmt == 's') {
			if(fmt[1] == 's') {
				fmt += 2;
				*out++ = (char)((int)pt->wSecond / 10) + '0';
			} else {
				++fmt;
				if(pt->wSecond > 9)
					*out++ = (char)((int)pt->wSecond / 10) + '0';
			}
			*out++ = (char)((int)pt->wSecond % 10) + '0';
		} else if(*fmt == 't' && fmt[1] == 't') {
			fmt += 2;
			if(pt->wHour < 12) pos = AM; else pos = PM;
			while(*pos) *out++ = *pos++;
		} else if(*fmt == 'A' && fmt[1] == 'M') {
			if(fmt[2] == '/' &&
			   fmt[3] == 'P' && fmt[4] == 'M') {
				if(pt->wHour < 12) *out++ = 'A'; //--+++--> 2010 - Noon / MidNight Decided Here!
				else *out++ = 'P';
				*out++ = 'M'; fmt += 5;
			} else if(fmt[2] == 'P' && fmt[3] == 'M') {
				fmt += 4;
				if(pt->wHour < 12) pos = AM; else pos = PM;
				while(*pos) *out++ = *pos++;
			} else *out++ = *fmt++;
		} else if(*fmt == 'a' && fmt[1] == 'm' && fmt[2] == '/' &&
				  fmt[3] == 'p' && fmt[4] == 'm') {
			if(pt->wHour < 12) *out++ = 'a';
			else *out++ = 'p';
			*out++ = 'm'; fmt += 5;
		}
		// internet time
		else if(*fmt == '@' && fmt[1] == '@' && fmt[2] == '@') {
			fmt += 3;
			*out++ = '@';
			*out++ = (char)(beat100 / 10000) + '0';
			*out++ = (char)((beat100 % 10000) / 1000) + '0';
			*out++ = (char)((beat100 % 1000) / 100) + '0';
			if(*fmt=='.' && fmt[1]=='@') {
				fmt += 2;
				*out++ = '.';
				*out++ = (char)((beat100 % 100) / 10) + '0';
				if(*fmt=='@'){
					++fmt;
					*out++ = (char)((beat100 % 10)) + '0';
				}
			}
		}
		// alternate calendar
		else if(*fmt == 'Y' && AltYear > -1) {
			int n = 1;
			while(*fmt == 'Y') { n *= 10; ++fmt; }
			if(n < AltYear) {
				n = 1; while(n < AltYear) n *= 10;
			}
			for(;;) {
				*out++ = (char)((AltYear % n) / (n/10)) + '0';
				if(n == 10) break;
				n /= 10;
			}
		} else if(*fmt == 'g') {
			for(pos=EraStr; *pos&&*fmt=='g'; ){
				char* p2 = CharNextExA((WORD)codepage, pos, 0);
				while(pos != p2) *out++ = *pos++;
				++fmt;
			}
			while(*fmt == 'g') fmt++;
		}
		
		else if(*fmt == 'L' && strncmp(fmt, "LDATE", 5) == 0) {
			char s[80];
			GetDateFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						  DATE_LONGDATE, pt, NULL, s, 80);
			for(pos=s; *pos; )  *out++=*pos++;
			fmt += 5;
		}
		
		else if(*fmt == 'D' && strncmp(fmt, "DATE", 4) == 0) {
			char s[80];
			GetDateFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						  DATE_SHORTDATE, pt, NULL, s, 80);
			for(pos=s; *pos; ) *out++=*pos++;
			fmt += 4;
		}
		
		else if(*fmt == 'T' && strncmp(fmt, "TIME", 4) == 0) {
			char s[80];
			GetTimeFormat(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						  TIME_FORCE24HOURFORMAT, pt, NULL, s, 80);
			for(pos=s; *pos; )  *out++=*pos++;
			fmt += 4;
		} else if(*fmt == 'S') { // uptime
			int len, slen, st;
			fmt++;
			if(GetNumFormat(&fmt, 'd', &len, &slen) == TRUE) {//days
				if(!TickCount) TickCount = GetTickCount();
				st = TickCount/86400000;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'a', &len, &slen) == TRUE) {//hours total
				if(!TickCount) TickCount = GetTickCount();
				st = TickCount/3600000;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'h', &len, &slen) == TRUE) {//hours (max 24)
				if(!TickCount) TickCount = GetTickCount();
				st = (TickCount/3600000)%24;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'n', &len, &slen) == TRUE) {//minutes
				if(!TickCount) TickCount = GetTickCount();
				st = (TickCount/60000)%60;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 's', &len, &slen) == TRUE) {//seconds
				if(!TickCount) TickCount = GetTickCount();
				st = (TickCount/1000)%60;
				SetNumFormat(&out, st, len, slen);
			} else if(*fmt == 'T') { // ST, uptime as h:mm:ss
				DWORD dw;
				int sth, stm, sts;
				if(!TickCount) TickCount = GetTickCount();
				dw = TickCount;
				dw /= 1000;
				sts = dw%60; dw /= 60;
				stm = dw%60; dw /= 60;
				sth = dw;
				
				SetNumFormat(&out, sth, 1, 0);
				*out++ = ':';
				SetNumFormat(&out, stm, 2, 0);
				*out++ = ':';
				SetNumFormat(&out, sts, 2, 0);
				
				fmt++;
			} else
				*out++ = 'S';
		} else if(*fmt == 'W') { //----//--+++--> 3/21/2010 is 80th day of year
			char szWkNum[8] = {0}; //-----+++--> WEEK NUMBER CODE IS HERE!!!
			char* Wk;
			struct tm today;
			time_t ltime;
			time(&ltime);
			localtime_s(&today, &ltime);
			if(*(fmt + 1) == 's') { // Week-Of-Year Starts Sunday
				strftime(szWkNum, 8, "%U", &today);
				Wk = szWkNum;
				while(*Wk) *out++ = *Wk++;
				fmt++;
			} else if(*(fmt + 1) == 'm') { // Week-Of-Year Starts Monday
				strftime(szWkNum, 8, "%W", &today);
				Wk = szWkNum;
				while(*Wk) *out++ = *Wk++;
				fmt++;
			} else if(*(fmt + 1) == 'i') { // Week ISO-8601 (by henriko.se)
				int ISOWeek;
				struct tm tmCurrentTime;
				struct tm tmStartOfCurrentYear;
				localtime_s(&tmCurrentTime,&ltime);
				mktime(&tmCurrentTime);
				if(tmCurrentTime.tm_wday == 0) {
					tmCurrentTime.tm_wday = 7;
				}
				tmStartOfCurrentYear.tm_year = tmCurrentTime.tm_year;
				tmStartOfCurrentYear.tm_mon = 1 - 1;
				tmStartOfCurrentYear.tm_mday = 1;
				tmStartOfCurrentYear.tm_hour = 0;
				tmStartOfCurrentYear.tm_min = 0;
				tmStartOfCurrentYear.tm_sec = 0;
				tmStartOfCurrentYear.tm_isdst = 0;
				mktime(&tmStartOfCurrentYear);
				if(tmStartOfCurrentYear.tm_wday == 0) {
					tmStartOfCurrentYear.tm_wday = 7;
				}
				ISOWeek = (tmCurrentTime.tm_yday + (tmStartOfCurrentYear.tm_wday - 1)) / 7 + (tmStartOfCurrentYear.tm_wday <= 4 ? 1 : 0);
				if(ISOWeek == 0) {
					struct tm tmStartOfLastYear;
					struct tm tmEndOfLastYear;
					tmStartOfLastYear.tm_year = tmCurrentTime.tm_year - 1;
					tmStartOfLastYear.tm_mon = 1 - 1;
					tmStartOfLastYear.tm_mday = 1;
					tmStartOfLastYear.tm_hour = 0;
					tmStartOfLastYear.tm_min = 0;
					tmStartOfLastYear.tm_sec = 0;
					tmStartOfLastYear.tm_isdst = 0;
					mktime(&tmStartOfLastYear);
					if(tmStartOfLastYear.tm_wday == 0) {
						tmStartOfLastYear.tm_wday = 7;
					}
					tmEndOfLastYear.tm_year = tmCurrentTime.tm_year - 1;
					tmEndOfLastYear.tm_mon = 12 - 1;
					tmEndOfLastYear.tm_mday = 31;
					tmEndOfLastYear.tm_hour = 0;
					tmEndOfLastYear.tm_min = 0;
					tmEndOfLastYear.tm_sec = 0;
					tmEndOfLastYear.tm_isdst = 0;
					mktime(&tmEndOfLastYear);
					ISOWeek = (tmEndOfLastYear.tm_yday + (tmStartOfLastYear.tm_wday - 1)) / 7 + (tmStartOfLastYear.tm_wday <= 4 ? 1 : 0);
				}
				if(tmCurrentTime.tm_mon == 12 - 1 && tmCurrentTime.tm_mday >= 29) {
					if(tmCurrentTime.tm_wday <= 3) {
						struct tm tmStartOfNextYear;
						tmStartOfNextYear.tm_year = tmCurrentTime.tm_year + 1;
						tmStartOfNextYear.tm_mon = 1 - 1;
						tmStartOfNextYear.tm_mday = 1;
						tmStartOfNextYear.tm_hour = 0;
						tmStartOfNextYear.tm_min = 0;
						tmStartOfNextYear.tm_sec = 0;
						tmStartOfNextYear.tm_isdst = 0;
						mktime(&tmStartOfNextYear);
						if(tmStartOfNextYear.tm_wday == 0) {
							tmStartOfNextYear.tm_wday = 7;
						}
						if(tmStartOfNextYear.tm_wday <= 4) {
							ISOWeek = 1;
						}
					}
				}
				wsprintf(szWkNum, "%d", ISOWeek);
				Wk = szWkNum;
				while(*Wk) *out++ = *Wk++;
				fmt++;
			}
			// Need DOY + 6 / 7 (as float) DO NOT ROUND - Done!
			else if(*(fmt + 1) == 'w') { // SWN (Simple Week Number)
				double dy; int d, s;
				//------+++--> Stoic Joker's (Pipe Bomb Crude) Simple Week Number Calculation!
				strftime(szWkNum, 8, "%j", &today);   // Day-Of-Year as Decimal Number (1 - 366)
				dy = floor((atof(szWkNum) + 6) / 7); // DoY + 6 / 7 with the Fractional Part...
				//-------------------------+++--> Truncated.
				Wk = _fcvt(dy, 0, &d, &s); // Make it a String
				while(*Wk) *out++ = *Wk++; // Done!
				fmt++;
			}
			fmt++; // Might Not be Needed!!!
		}
//================================================================================================
//======================================= JULIAN DATE Code ========================================
		else if(*fmt == 'J' && *(fmt + 1) == 'D') {
			double y, M, d, h, m, s, bc, JD;
			struct tm Julian;
			int id, is, i=0;
			char* szJulian;
			time_t UTC;
			
			time(&UTC);
			gmtime_s(&Julian, &UTC);
			
			y = Julian.tm_year +1900;	// Year
			M = Julian.tm_mon +1;		// Month
			d = Julian.tm_mday;			// Day
			h = Julian.tm_hour;			// Hours
			m = Julian.tm_min;			// Minutes
			s = Julian.tm_sec;			// Seconds
			// This Handles the January 1, 4713 B.C up to
			bc = 100.0 * y + M - 190002.5; // Year 0 Part.
			JD = 367.0 * y;
			
			JD -= floor(7.0*(y + floor((M+9.0)/12.0))/4.0);
			JD += floor(275.0*M/9.0);
			JD += d;
			JD += (h + (m + s/60.0)/60.0)/24.0;
			JD += 1721013.5; // BCE 2 November 18 00:00:00.0 UT - Tuesday
			JD -= 0.5*bc/fabs(bc);
			JD += 0.5;
			
			szJulian = _fcvt(JD, 4, &id, &is); // Make it a String
			while(*szJulian) {
				if(i == id) { //--//-++-> id = Decimal Point Precision/Position
					*out++ = '.'; // ReInsert the Decimal Point Where it Belongs.
				} else {
					*out++ = *szJulian++; //--+++--> Done!
				}
				i++;
			}
			fmt +=2;
		}
//================================================================================================
//======================================= ORDINAL DATE Code =======================================
		else if(*fmt == 'O' && *(fmt + 1) == 'D') { //--------+++--> Ordinal Date UTC:
			char szOD[16] = {0};
			struct tm today;
			time_t UTC;
			char* od;
			
			time(&UTC);
			gmtime_s(&today, &UTC);
			strftime(szOD, 16, "%Y-%j", &today);
			od = szOD;
			while(*od) *out++ = *od++;
			fmt +=2;
		}
		//==========================================================================
		else if(*fmt == 'O' && *(fmt + 1) == 'd') { //------+++--> Ordinal Date Local:
			char szOD[16] = {0};
			struct tm today;
			time_t ltime;
			char* od;
			
			time(&ltime);
			localtime_s(&today, &ltime);
			strftime(szOD, 16, "%Y-%j", &today);
			od = szOD;
			while(*od) *out++ = *od++;
			fmt +=2;
		}
		//==========================================================================
		else if(*fmt == 'D' && strncmp(fmt, "DOY", 3) == 0) { //--+++--> Day-Of-Year:
			char szDoy[8] = {0};
			struct tm today;
			time_t ltime;
			char* doy;
			
			time(&ltime);
			localtime_s(&today, &ltime);
			strftime(szDoy, 8, "%j", &today);
			doy = szDoy;
			while(*doy) *out++ = *doy++;
			fmt +=3;
		}
		//==========================================================================
		else if(*fmt == 'P' && strncmp(fmt, "POSIX", 5) == 0) { //-> Posix/Unix Time:
			char szPosix[16] = {0}; // This will Give the Number of Seconds That Have
			char* posix; //--+++--> Elapsed Since the Unix Epoch: 1970-01-01 00:00:00
			
			wsprintf(szPosix, "%ld", time(NULL));
			posix = szPosix;
			while(*posix) *out++ = *posix++;
			fmt +=5;
		}
		//==========================================================================
		else if(*fmt == 'T' && strncmp(fmt, "TZN", 3) == 0) { //--++-> TimeZone Name:
			char szTZName[TZNAME_MAX] = {0};
			size_t lRet;
			char* tzn;
			int iDST;
			
			_get_daylight(&iDST);
			if(iDST) {
				_get_tzname(&lRet, szTZName, TZNAME_MAX, 1);
			} else {
				_get_tzname(&lRet, szTZName, TZNAME_MAX, 0);
			}
			
			tzn = szTZName;
			while(*tzn) *out++ = *tzn++;
			fmt +=3;
		}
//=================================================================================================
		else {
			for(pos=CharNext(fmt); fmt!=pos; )  *out++=*fmt++;
		}
	}
	*out='\0';
	return (unsigned)(out-buf);
}

/*--------------------------------------------------
--------------------------------------- Check Format
--------------------------------------------------*/
DWORD FindFormat(const char* fmt)
{
	DWORD ret = 0;
	
	while(*fmt) {
		if(*fmt == '\"') {
			fmt++;
			while(*fmt != '\"' && *fmt) fmt++;
			if(*fmt == '\"') fmt++;
		}
		
		else if(*fmt == 's') {
			fmt++;
			ret |= FORMAT_SECOND;
		}
		
		else if(*fmt == '@' && fmt[1] == '@' && fmt[2] == '@') {
			fmt += 3;
			if(*fmt == '.' && fmt[1] == '@') {
				ret |= FORMAT_BEAT2;
				fmt += 2;
			} else ret |= FORMAT_BEAT1;
		}
		
		else fmt = CharNext(fmt);
	}
	return ret;
}
