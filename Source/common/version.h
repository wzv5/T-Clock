/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */
#pragma once
#ifndef AUTOVERSION_H
#define AUTOVERSION_H
#	define XSTR(x) #x
#	define STR(x) XSTR(x)
/** Version **/
#	define VER_MAJOR 2
#	define VER_MINOR 3
#	define VER_BUILD 0
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 127
#	define VER_FULL "2.3.0 Beta"
#	define VER_SHORT "2.3b0"
#	define VER_SHORT_DOTS "2.3.0"
#	define VER_SHORT_GREEK "2.3β0"
#	define VER_RC_REVISION 2, 3, 0, 127
#	define VER_RC_STATUS 2, 3, 1, 0
/** Subversion Information **/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2014-10-10 17:42:27 +0000 (Fri, Oct 10 2014)"
#	define VER_REVISION_HASH "dff0300"
/** Date/Time **/
#	define VER_TIMESTAMP 1412963092
#	define VER_TIME_SEC 52
#	define VER_TIME_MIN 44
#	define VER_TIME_HOUR 17
#	define VER_TIME_DAY 10
#	define VER_TIME_MONTH 10
#	define VER_TIME_YEAR 2014
#	define VER_TIME_WDAY 5
#	define VER_TIME_YDAY 282
#	define VER_TIME_WDAY_SHORT "Fri"
#	define VER_TIME_WDAY_FULL "Friday"
#	define VER_TIME_MONTH_SHORT "Oct"
#	define VER_TIME_MONTH_FULL "October"
#	define VER_TIME "17:44:52"
#	define VER_DATE "2014-10-10"
#	define VER_DATE_LONG "Fri, Oct 10, 2014 17:44:52 UTC"
#	define VER_DATE_SHORT "2014-10-10 17:44:52 UTC"
#	define VER_DATE_ISO "2014-10-10T17:44:52Z"
#endif
