#ifndef AUTOVERSION_H
#define AUTOVERSION_H
/* Note: to use integer defines as strings, use STR(), eg. STR(VER_REVISION) */
/**** Version ****/
#	define VER_MAJOR 2
#	define VER_MINOR 4
#	define VER_BUILD 1
	/** status values: 0=Alpha, 1=Beta, 2=RC, 3=Release, 4=Maintenance */
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 421
#	define VER_FULL "2.4.1 Beta"
#	define VER_SHORT "2.4b1"
#	define VER_SHORT_DOTS "2.4.1"
#	define VER_SHORT_GREEK "2.4β1"
#	define VER_RC_REVISION 2, 4, 1, 421
#	define VER_RC_STATUS 2, 4, 1, 1
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2016-08-11 18:13:20 +0000 (Thu, Aug 11 2016)"
#	define VER_REVISION_HASH "68fc90c"
#	define VER_REVISION_TAG "v2.4.1#421-beta"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1470958320
#	define VER_TIME_SEC 0
#	define VER_TIME_MIN 32
#	define VER_TIME_HOUR 23
#	define VER_TIME_DAY 11
#	define VER_TIME_MONTH 8
#	define VER_TIME_YEAR 2016
#	define VER_TIME_WDAY 4
#	define VER_TIME_YDAY 223
#	define VER_TIME_WDAY_SHORT "Thu"
#	define VER_TIME_WDAY_FULL "Thursday"
#	define VER_TIME_MONTH_SHORT "Aug"
#	define VER_TIME_MONTH_FULL "August"
#	define VER_TIME "23:32:00"
#	define VER_DATE "2016-08-11"
#	define VER_DATE_LONG "Thu, Aug 11, 2016 23:32:00 UTC"
#	define VER_DATE_SHORT "2016-08-11 23:32:00 UTC"
#	define VER_DATE_ISO "2016-08-11T23:32:00Z"
/**** Helper 'functions' ****/
#	define VER_IsReleaseOrHigher() ( VER_STATUS >= 3 )
#	define VER_IsAlpha() ( VER_STATUS == 0 )
#	define VER_IsBeta() ( VER_STATUS == 1 )
#	define VER_IsRC() ( VER_STATUS == 2 )
#	define VER_IsRelease() ( VER_STATUS == 3 )
#	define VER_IsMaintenance() ( VER_STATUS == 4 )
#ifndef STR
#	define STR_(x) #x
#	define STR(x) STR_(x)
#endif
#ifndef L
#	define L_(x) L##x
#	define L(x) L_(x)
#endif
#endif
