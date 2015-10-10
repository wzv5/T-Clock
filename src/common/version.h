#ifndef AUTOVERSION_H
#define AUTOVERSION_H
/* Note: to use integer defines as strings, use STR(), eg. STR(VER_REVISION) */
#ifndef STR
#	define STR_(x) #x
#	define STR(x) STR_(x)
#endif
#ifndef L
#	define L_(x) L##x
#	define L(x) L_(x)
#endif
/**** Version ****/
#	define VER_MAJOR 2
#	define VER_MINOR 4
#	define VER_BUILD 0
	/** status values: 0=Alpha, 1=Beta, 2=RC, 3=Release, 4=Maintenance */
#	define VER_STATUS 2
#	define VER_STATUS_FULL "RC"
#	define VER_STATUS_SHORT "rc"
#	define VER_STATUS_GREEK "гc"
#	define VER_REVISION 340
#	define VER_FULL "2.4.0 RC"
#	define VER_SHORT "2.4rc0"
#	define VER_SHORT_DOTS "2.4.0"
#	define VER_SHORT_GREEK "2.4гc0"
#	define VER_RC_REVISION 2, 4, 0, 340
#	define VER_RC_STATUS 2, 4, 0, 2
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2015-10-10 17:46:05 +0000 (Sat, Oct 10 2015)"
#	define VER_REVISION_HASH "9b25bf2"
#	define VER_REVISION_TAG "v2.4.0#340-rc"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1444499352
#	define VER_TIME_SEC 12
#	define VER_TIME_MIN 49
#	define VER_TIME_HOUR 17
#	define VER_TIME_DAY 10
#	define VER_TIME_MONTH 10
#	define VER_TIME_YEAR 2015
#	define VER_TIME_WDAY 6
#	define VER_TIME_YDAY 282
#	define VER_TIME_WDAY_SHORT "Sat"
#	define VER_TIME_WDAY_FULL "Saturday"
#	define VER_TIME_MONTH_SHORT "Oct"
#	define VER_TIME_MONTH_FULL "October"
#	define VER_TIME "17:49:12"
#	define VER_DATE "2015-10-10"
#	define VER_DATE_LONG "Sat, Oct 10, 2015 17:49:12 UTC"
#	define VER_DATE_SHORT "2015-10-10 17:49:12 UTC"
#	define VER_DATE_ISO "2015-10-10T17:49:12Z"
#endif
