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
#	define VER_REVISION 351
#	define VER_FULL "2.4.0 RC"
#	define VER_SHORT "2.4rc0"
#	define VER_SHORT_DOTS "2.4.0"
#	define VER_SHORT_GREEK "2.4гc0"
#	define VER_RC_REVISION 2, 4, 0, 351
#	define VER_RC_STATUS 2, 4, 0, 2
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2015-10-19 15:45:03 +0000 (Mon, Oct 19 2015)"
#	define VER_REVISION_HASH "2d6f529"
#	define VER_REVISION_TAG "v2.4.0#351-rc"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1445277262
#	define VER_TIME_SEC 22
#	define VER_TIME_MIN 54
#	define VER_TIME_HOUR 17
#	define VER_TIME_DAY 19
#	define VER_TIME_MONTH 10
#	define VER_TIME_YEAR 2015
#	define VER_TIME_WDAY 1
#	define VER_TIME_YDAY 291
#	define VER_TIME_WDAY_SHORT "Mon"
#	define VER_TIME_WDAY_FULL "Monday"
#	define VER_TIME_MONTH_SHORT "Oct"
#	define VER_TIME_MONTH_FULL "October"
#	define VER_TIME "17:54:22"
#	define VER_DATE "2015-10-19"
#	define VER_DATE_LONG "Mon, Oct 19, 2015 17:54:22 UTC"
#	define VER_DATE_SHORT "2015-10-19 17:54:22 UTC"
#	define VER_DATE_ISO "2015-10-19T17:54:22Z"
#endif
