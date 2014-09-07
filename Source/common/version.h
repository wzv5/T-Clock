/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */
#pragma once
#ifndef AUTOVERSION_H
#define AUTOVERSION_H
#	define XSTR(x) #x
#	define STR(x) XSTR(x)
/** Version **/
#	define VER_MAJOR 2
#	define VER_MINOR 2
#	define VER_BUILD 0
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 60
#	define VER_FULL "2.2.0 Beta"
#	define VER_SHORT "2.2b0"
#	define VER_SHORT_DOTS "2.2.0"
#	define VER_SHORT_GREEK "2.2β0"
#	define VER_RC_REVISION 2, 2, 0, 60
#	define VER_RC_STATUS 2, 2, 1, 0
/** Subversion Information **/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2014-08-28 10:43:20 +0000 (Thu, Aug 28 2014)"
/** Date/Time **/
#	define VER_TIMESTAMP 1410120294
#	define VER_TIME_SEC 54
#	define VER_TIME_MIN 4
#	define VER_TIME_HOUR 20
#	define VER_TIME_DAY 7
#	define VER_TIME_MONTH 9
#	define VER_TIME_YEAR 2014
#	define VER_TIME_WDAY 0
#	define VER_TIME_YDAY 249
#	define VER_TIME_WDAY_SHORT "Sun"
#	define VER_TIME_WDAY_FULL "Sunday"
#	define VER_TIME_MONTH_SHORT "Sep"
#	define VER_TIME_MONTH_FULL "September"
#	define VER_TIME "20:04:54"
#	define VER_DATE "2014-09-07"
#	define VER_DATE_LONG "Sun, Sep 07, 2014 20:04:54 UTC"
#	define VER_DATE_SHORT "2014-09-07 20:04:54 UTC"
#	define VER_DATE_ISO "2014-09-07T20:04:54Z"
#endif
