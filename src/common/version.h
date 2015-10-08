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
#	define VER_REVISION 329
#	define VER_FULL "2.4.0 RC"
#	define VER_SHORT "2.4rc0"
#	define VER_SHORT_DOTS "2.4.0"
#	define VER_SHORT_GREEK "2.4гc0"
#	define VER_RC_REVISION 2, 4, 0, 329
#	define VER_RC_STATUS 2, 4, 0, 2
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2015-10-07 16:31:26 +0000 (Wed, Oct 07 2015)"
#	define VER_REVISION_HASH "3a626d6"
#	define VER_REVISION_TAG "v2.4.0#329-rc"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1444236527
#	define VER_TIME_SEC 47
#	define VER_TIME_MIN 48
#	define VER_TIME_HOUR 16
#	define VER_TIME_DAY 7
#	define VER_TIME_MONTH 10
#	define VER_TIME_YEAR 2015
#	define VER_TIME_WDAY 3
#	define VER_TIME_YDAY 279
#	define VER_TIME_WDAY_SHORT "Wed"
#	define VER_TIME_WDAY_FULL "Wednesday"
#	define VER_TIME_MONTH_SHORT "Oct"
#	define VER_TIME_MONTH_FULL "October"
#	define VER_TIME "16:48:47"
#	define VER_DATE "2015-10-07"
#	define VER_DATE_LONG "Wed, Oct 07, 2015 16:48:47 UTC"
#	define VER_DATE_SHORT "2015-10-07 16:48:47 UTC"
#	define VER_DATE_ISO "2015-10-07T16:48:47Z"
#endif
