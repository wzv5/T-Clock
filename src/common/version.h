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
#	define VER_REVISION 419
#	define VER_FULL "2.4.1 Beta"
#	define VER_SHORT "2.4b1"
#	define VER_SHORT_DOTS "2.4.1"
#	define VER_SHORT_GREEK "2.4β1"
#	define VER_RC_REVISION 2, 4, 1, 419
#	define VER_RC_STATUS 2, 4, 1, 1
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2016-08-04 17:46:56 +0000 (Thu, Aug 04 2016)"
#	define VER_REVISION_HASH "1cc0441"
#	define VER_REVISION_TAG "v2.4.1#419-beta"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1470333783
#	define VER_TIME_SEC 3
#	define VER_TIME_MIN 3
#	define VER_TIME_HOUR 18
#	define VER_TIME_DAY 4
#	define VER_TIME_MONTH 8
#	define VER_TIME_YEAR 2016
#	define VER_TIME_WDAY 4
#	define VER_TIME_YDAY 216
#	define VER_TIME_WDAY_SHORT "Thu"
#	define VER_TIME_WDAY_FULL "Thursday"
#	define VER_TIME_MONTH_SHORT "Aug"
#	define VER_TIME_MONTH_FULL "August"
#	define VER_TIME "18:03:03"
#	define VER_DATE "2016-08-04"
#	define VER_DATE_LONG "Thu, Aug 04, 2016 18:03:03 UTC"
#	define VER_DATE_SHORT "2016-08-04 18:03:03 UTC"
#	define VER_DATE_ISO "2016-08-04T18:03:03Z"
#endif
