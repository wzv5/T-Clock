//---[s]--- For InternetTime 99/03/16@211 M.Takemura -----

/*------------------------------------------------------------------------
// format.c : to make a string to display in the clock -> KAZUBON 1997-1998
//-----------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Friday, 12/16/2011 @ 3:36:00pm
#include "tcdll.h"

static WORD m_codepage = CP_ACP;
static char m_MonthShort[11], m_MonthLong[31];
static char m_DayOfWeekShort[11], m_DayOfWeekLong[31];
static char* m_DayOfWeekEng[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char* m_MonthEng[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char m_AM[10], m_PM[10];
static char m_EraStr[11];
static int m_AltYear;

extern char g_bHour12, g_bHourZero;

//================================================================================================
//---------------------------------//+++--> load Localized Strings for Month, Day, & AM/PM Symbols:
void InitFormat(const char* section, SYSTEMTIME* lt)   //--------------------------------------------------------+++-->
{
	char year[6];
	int i, ilang, ioptcal;
	
	ilang = GetMyRegLong(section, "Locale", GetUserDefaultLangID());
	
	GetLocaleInfo(ilang, LOCALE_IDEFAULTANSICODEPAGE|LOCALE_RETURN_NUMBER, (LPSTR)&m_codepage, sizeof(m_codepage));
	if(!IsValidCodePage(m_codepage)) m_codepage=CP_ACP;
	
	i = lt->wDayOfWeek; i--; if(i < 0) i = 6;
	
	GetLocaleInfo(ilang, LOCALE_SABBREVDAYNAME1 + i, m_DayOfWeekShort, sizeof(m_DayOfWeekShort));
//	GetLocaleInfo(ilang, LOCALE_SSHORTESTDAYNAME1 + i, DayOfWeekShort, sizeof(DayOfWeekShort)); // Vista+
	GetLocaleInfo(ilang, LOCALE_SDAYNAME1 + i, m_DayOfWeekLong, sizeof(m_DayOfWeekLong));
	
	i = lt->wMonth; i--;
	GetLocaleInfo(ilang, LOCALE_SABBREVMONTHNAME1 + i, m_MonthShort, sizeof(m_MonthShort));
	GetLocaleInfo(ilang, LOCALE_SMONTHNAME1 + i, m_MonthLong, sizeof(m_MonthLong));
	
	GetMyRegStr(section, "AMsymbol", m_AM, sizeof(m_AM), "");
	if(!*m_AM){
		GetLocaleInfo(ilang, LOCALE_S1159, m_AM, sizeof(m_AM));
		if(!*m_AM) strcpy(m_AM,"AM");
	}
	GetMyRegStr(section, "PMsymbol", m_PM, sizeof(m_PM), "");
	if(!*m_PM){
		GetLocaleInfo(ilang, LOCALE_S2359, m_PM, sizeof(m_PM));
		if(!*m_PM) strcpy(m_PM,"PM");
	}
	
	m_AltYear = -1;
	
	if(!GetLocaleInfo(ilang, LOCALE_IOPTIONALCALENDAR|LOCALE_RETURN_NUMBER, (LPSTR)&ioptcal, sizeof(ioptcal)))
		ioptcal = 0;
	
	if(ioptcal < 3) ilang = LANG_USER_DEFAULT;
	
	if(!GetDateFormat(ilang, DATE_USE_ALT_CALENDAR, lt, "gg", m_EraStr, sizeof(m_EraStr)))
		*m_EraStr='\0';
	
	if(GetDateFormat(ilang, DATE_USE_ALT_CALENDAR, lt, "yyyy", year, sizeof(year)))
		m_AltYear=atoi(year);
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
unsigned MakeFormat(char buf[FORMAT_MAX_SIZE], const char* fmt, SYSTEMTIME* pt, int beat100)   //------------------+++-->
{
	const char* bufend = buf+FORMAT_MAX_SIZE;
	const char* pos;
	char* out = buf;
	ULONGLONG TickCount = 0;
	
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
				*out++ = m_MonthEng[pt->wMonth-1][0];
				*out++ = m_MonthEng[pt->wMonth-1][1];
				*out++ = m_MonthEng[pt->wMonth-1][2];
				fmt += 3;
			} else if(fmt[1] == 'm' && fmt[2] == 'm') {
				if(*(fmt + 3) == 'm') {
					fmt += 4;
					for(pos=m_MonthLong; *pos; ) *out++=*pos++;
				} else {
					fmt += 3;
					for(pos=m_MonthShort; *pos; ) *out++=*pos++;
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
				for(pos=m_DayOfWeekLong; *pos; ) *out++=*pos++;
			} else {
				fmt += 3;
				for(pos=m_DayOfWeekShort; *pos; ) *out++=*pos++;
			}
		} else if(*fmt=='d') {
			if(fmt[1]=='d' && fmt[2]=='e'){
				fmt+=3;
				for(pos=m_DayOfWeekEng[pt->wDayOfWeek]; *pos; ) *out++=*pos++;
			}else if(fmt[1]=='d' && fmt[2]=='d') {
				fmt+=3;
				if(*fmt=='d'){
					++fmt;
					pos=m_DayOfWeekLong;
				}else{
					pos=m_DayOfWeekShort;
				}
				for(; *pos; ) *out++=*pos++;
			}else{
				if(fmt[1]=='d') {
					fmt+=2;
					*out++ = (char)((int)pt->wDay / 10) + '0';
				}else{
					++fmt;
					if(pt->wDay > 9)
						*out++ = (char)((int)pt->wDay / 10) + '0';
				}
				*out++ = (char)((int)pt->wDay % 10) + '0';
			}
		} else if(*fmt=='h') {
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
			if(pt->wHour < 12) pos = m_AM; else pos = m_PM;
			while(*pos) *out++ = *pos++;
		} else if(*fmt == 'A' && fmt[1] == 'M') {
			if(fmt[2] == '/' &&
			   fmt[3] == 'P' && fmt[4] == 'M') {
				if(pt->wHour < 12) *out++ = 'A'; //--+++--> 2010 - Noon / MidNight Decided Here!
				else *out++ = 'P';
				*out++ = 'M'; fmt += 5;
			} else if(fmt[2] == 'P' && fmt[3] == 'M') {
				fmt += 4;
				if(pt->wHour < 12) pos = m_AM; else pos = m_PM;
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
		else if(*fmt == 'Y' && m_AltYear > -1) {
			int n = 1;
			while(*fmt == 'Y') { n *= 10; ++fmt; }
			if(n < m_AltYear) {
				n = 1; while(n < m_AltYear) n *= 10;
			}
			for(;;) {
				*out++ = (char)((m_AltYear % n) / (n/10)) + '0';
				if(n == 10) break;
				n /= 10;
			}
		} else if(*fmt == 'g') {
			for(pos=m_EraStr; *pos&&*fmt=='g'; ){
				char* p2 = CharNextExA(m_codepage, pos, 0);
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
				if(!TickCount) TickCount = pGetTickCount64();
				st = (int)(TickCount/86400000);
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'a', &len, &slen) == TRUE) {//hours total
				if(!TickCount) TickCount = pGetTickCount64();
				st = (int)(TickCount/3600000);
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'h', &len, &slen) == TRUE) {//hours (max 24)
				if(!TickCount) TickCount = pGetTickCount64();
				st = (TickCount/3600000)%24;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 'n', &len, &slen) == TRUE) {//minutes
				if(!TickCount) TickCount = pGetTickCount64();
				st = (TickCount/60000)%60;
				SetNumFormat(&out, st, len, slen);
			} else if(GetNumFormat(&fmt, 's', &len, &slen) == TRUE) {//seconds
				if(!TickCount) TickCount = pGetTickCount64();
				st = (TickCount/1000)%60;
				SetNumFormat(&out, st, len, slen);
			} else if(*fmt == 'T') { // ST, uptime as h:mm:ss
				ULONGLONG past;
				int sth, stm, sts;
				if(!TickCount) TickCount = pGetTickCount64();
				past = TickCount/1000;
				sts = past%60; past /= 60;
				stm = past%60; past /= 60;
				sth = (int)past;
				
				SetNumFormat(&out, sth, 1, 0);
				*out++ = ':';
				SetNumFormat(&out, stm, 2, 0);
				*out++ = ':';
				SetNumFormat(&out, sts, 2, 0);
				
				fmt++;
			} else
				*out++ = 'S';
		} else if(*fmt == 'W') { // Week-of-Year
			struct tm tmnow;
			time_t tnow;
			time(&tnow);
			localtime_s(&tmnow, &tnow);
			++fmt;
			if(*fmt == 's') { // Week-Of-Year Starts Sunday
				out += strftime(out, bufend-out, "%U", &tmnow);
				++fmt;
			} else if(*fmt == 'm') { // Week-Of-Year Starts Monday
				out += strftime(out, bufend-out, "%W", &tmnow);
				++fmt;
			} else if(*fmt == 'i') { // ISO-8601 week (1st version by henriko.se, 2nd by White-Tiger)
				int wday,borderdays,week;
				for(;;){
					wday = (!tmnow.tm_wday?6:tmnow.tm_wday-1); // convert from Sun-Sat to Mon-Sun (0-5)
					borderdays = (tmnow.tm_yday + 7 - wday) % 7; // +7 to prevent it from going negative
					week = (tmnow.tm_yday + 6 - wday) / 7;
					if(borderdays >= 4){ // year starts with at least 4 days
						++week;
					} else if(!week){ // we're still in last year's week
						--tmnow.tm_year;
						tmnow.tm_mon = 11;
						tmnow.tm_mday = 31;
						tmnow.tm_isdst = -1;
						if(mktime(&tmnow)==-1){ // mktime magically updates tm_yday, tm_wday
							week = 1;
							break; // fail safe
						}
						tmnow.tm_mon = 0; // just to speed up the "if" below, since we know that it can't be week 1
						continue; // repeat (once)
					}
					if(tmnow.tm_mon==11 && tmnow.tm_mday>=29){ // end of year, could be week 1
						borderdays = 31 - tmnow.tm_mday + wday;
						if(borderdays < 3)
							week = 1;
					}
					break;
				}
				out += wsprintf(out,"%d",week);
				++fmt;
			} else if(*fmt == 'w') { // SWN (Simple Week Number)
				out += wsprintf(out,"%d",1 + tmnow.tm_yday / 7);
				++fmt;
			}
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
