/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */
#pragma once
#ifndef AUTOVERSION_H
#define AUTOVERSION_H
#	define XSTR(x) #x
#	define STR(x) XSTR(x)
/** Version **/
#	define VER_MAJOR 2
#	define VER_MINOR 3
#	define VER_BUILD 2
	/* status values: 0=Alpha, 1=Beta, 2=Release Candidate, 3=Release, 4=Release Maintenance */
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 151
#	define VER_FULL "2.3.2 Beta"
#	define VER_SHORT "2.3b2"
#	define VER_SHORT_DOTS "2.3.2"
#	define VER_SHORT_GREEK "2.3β2"
#	define VER_RC_REVISION 2, 3, 2, 151
#	define VER_RC_STATUS 2, 3, 2, 1
/** Subversion Information **/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2014-11-03 15:18:07 +0000 (Mon, Nov 03 2014)"
#	define VER_REVISION_HASH "483b3f8"
#	define VER_REVISION_TAG "v2.3.2#151-beta"
/** Date/Time **/
#	define VER_TIMESTAMP 1415229325
#	define VER_TIME_SEC 25
#	define VER_TIME_MIN 15
#	define VER_TIME_HOUR 23
#	define VER_TIME_DAY 5
#	define VER_TIME_MONTH 11
#	define VER_TIME_YEAR 2014
#	define VER_TIME_WDAY 3
#	define VER_TIME_YDAY 308
#	define VER_TIME_WDAY_SHORT "Wed"
#	define VER_TIME_WDAY_FULL "Wednesday"
#	define VER_TIME_MONTH_SHORT "Nov"
#	define VER_TIME_MONTH_FULL "November"
#	define VER_TIME "23:15:25"
#	define VER_DATE "2014-11-05"
#	define VER_DATE_LONG "Wed, Nov 05, 2014 23:15:25 UTC"
#	define VER_DATE_SHORT "2014-11-05 23:15:25 UTC"
#	define VER_DATE_ISO "2014-11-05T23:15:25Z"
#endif
