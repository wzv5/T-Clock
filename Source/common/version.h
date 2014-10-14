/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */
#pragma once
#ifndef AUTOVERSION_H
#define AUTOVERSION_H
#	define XSTR(x) #x
#	define STR(x) XSTR(x)
/** Version **/
#	define VER_MAJOR 2
#	define VER_MINOR 3
#	define VER_BUILD 1
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 133
#	define VER_FULL "2.3.1 Beta"
#	define VER_SHORT "2.3b1"
#	define VER_SHORT_DOTS "2.3.1"
#	define VER_SHORT_GREEK "2.3β1"
#	define VER_RC_REVISION 2, 3, 1, 133
#	define VER_RC_STATUS 2, 3, 1, 1
/** Subversion Information **/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2014-10-14 02:21:21 +0000 (Tue, Oct 14 2014)"
#	define VER_REVISION_HASH "17d8d04"
/** Date/Time **/
#	define VER_TIMESTAMP 1413256501
#	define VER_TIME_SEC 1
#	define VER_TIME_MIN 15
#	define VER_TIME_HOUR 3
#	define VER_TIME_DAY 14
#	define VER_TIME_MONTH 10
#	define VER_TIME_YEAR 2014
#	define VER_TIME_WDAY 2
#	define VER_TIME_YDAY 286
#	define VER_TIME_WDAY_SHORT "Tue"
#	define VER_TIME_WDAY_FULL "Tuesday"
#	define VER_TIME_MONTH_SHORT "Oct"
#	define VER_TIME_MONTH_FULL "October"
#	define VER_TIME "03:15:01"
#	define VER_DATE "2014-10-14"
#	define VER_DATE_LONG "Tue, Oct 14, 2014 03:15:01 UTC"
#	define VER_DATE_SHORT "2014-10-14 03:15:01 UTC"
#	define VER_DATE_ISO "2014-10-14T03:15:01Z"
#endif
