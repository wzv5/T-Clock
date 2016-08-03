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
#	define VER_BUILD 1
	/** status values: 0=Alpha, 1=Beta, 2=RC, 3=Release, 4=Maintenance */
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 414
#	define VER_FULL "2.4.1 Beta"
#	define VER_SHORT "2.4b1"
#	define VER_SHORT_DOTS "2.4.1"
#	define VER_SHORT_GREEK "2.4β1"
#	define VER_RC_REVISION 2, 4, 1, 414
#	define VER_RC_STATUS 2, 4, 1, 1
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2016-08-03 18:55:55 +0000 (Wed, Aug 03 2016)"
#	define VER_REVISION_HASH "c82bc2b"
#	define VER_REVISION_TAG "v2.4.1#414-beta"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1470250661
#	define VER_TIME_SEC 41
#	define VER_TIME_MIN 57
#	define VER_TIME_HOUR 18
#	define VER_TIME_DAY 3
#	define VER_TIME_MONTH 8
#	define VER_TIME_YEAR 2016
#	define VER_TIME_WDAY 3
#	define VER_TIME_YDAY 215
#	define VER_TIME_WDAY_SHORT "Wed"
#	define VER_TIME_WDAY_FULL "Wednesday"
#	define VER_TIME_MONTH_SHORT "Aug"
#	define VER_TIME_MONTH_FULL "August"
#	define VER_TIME "18:57:41"
#	define VER_DATE "2016-08-03"
#	define VER_DATE_LONG "Wed, Aug 03, 2016 18:57:41 UTC"
#	define VER_DATE_SHORT "2016-08-03 18:57:41 UTC"
#	define VER_DATE_ISO "2016-08-03T18:57:41Z"
#endif
