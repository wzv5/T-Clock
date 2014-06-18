/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */
#pragma once
#ifndef AUTOVERSION_H
#define AUTOVERSION_H
#	define XSTR(x) #x
#	define STR(x) XSTR(x)
/** Version **/
	#define VER_MAJOR 2
	#define VER_MINOR 1
	#define VER_BUILD 0
	#define VER_STATUS 2
	#define VER_STATUS_S "Release Candidate"
	#define VER_STATUS_SS "rc"
	#define VER_STATUS_SS2 "гc"
	#define VER_REVISION 57
	#define VER_FULL "2.1.0 Release Candidate"
	#define VER_SHORT "2.1rc0"
	#define VER_SHORT2 "2.1.0"
	#define VER_SHORT3 "2.1гc0"
	#define VER_RC 2, 1, 0, 57
/** Date/Time **/
	#define VER_TIMESTAMP 1403117331
	#define VER_DATE_SEC 51
	#define VER_DATE_MIN 48
	#define VER_DATE_HOUR 18
	#define VER_DATE_DAY 18
	#define VER_DATE_MONTH 06
	#define VER_DATE_YEAR 2014
	#define VER_DATE_WDAY 0
	#define VER_DATE_YDAY 169
	#define VER_DATE_WDAY_SS "Wed"
	#define VER_DATE_WDAY_S "Wednesday"
	#define VER_DATE_MONTH_SS "Jun"
	#define VER_DATE_MONTH_S "June"
	#define VER_DATE "Wed, June 18, 2014 18:48:51 UTC"
	#define VER_DATES "2013-05-26 18:48:51 UTC"
	#define VER_DATE_TIME "18:48:51"
	#define VER_DATE_DATE "2014-06-18"
#endif
