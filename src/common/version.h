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
#	define VER_REVISION 369
#	define VER_FULL "2.4.0 RC"
#	define VER_SHORT "2.4rc0"
#	define VER_SHORT_DOTS "2.4.0"
#	define VER_SHORT_GREEK "2.4гc0"
#	define VER_RC_REVISION 2, 4, 0, 369
#	define VER_RC_STATUS 2, 4, 0, 2
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2015-11-13 15:42:08 +0000 (Fri, Nov 13 2015)"
#	define VER_REVISION_HASH "c7353c4"
#	define VER_REVISION_TAG "v2.4.0#369-rc"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1447429391
#	define VER_TIME_SEC 11
#	define VER_TIME_MIN 43
#	define VER_TIME_HOUR 15
#	define VER_TIME_DAY 13
#	define VER_TIME_MONTH 11
#	define VER_TIME_YEAR 2015
#	define VER_TIME_WDAY 5
#	define VER_TIME_YDAY 316
#	define VER_TIME_WDAY_SHORT "Fri"
#	define VER_TIME_WDAY_FULL "Friday"
#	define VER_TIME_MONTH_SHORT "Nov"
#	define VER_TIME_MONTH_FULL "November"
#	define VER_TIME "15:43:11"
#	define VER_DATE "2015-11-13"
#	define VER_DATE_LONG "Fri, Nov 13, 2015 15:43:11 UTC"
#	define VER_DATE_SHORT "2015-11-13 15:43:11 UTC"
#	define VER_DATE_ISO "2015-11-13T15:43:11Z"
#endif
