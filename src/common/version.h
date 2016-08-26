#ifndef AUTOVERSION_H
#define AUTOVERSION_H
/* Note: to use integer defines as strings, use STR(), eg. STR(VER_REVISION) */
/**** Version ****/
#	define VER_MAJOR 2
#	define VER_MINOR 4
#	define VER_BUILD 2
	/** status values: 0=Alpha, 1=Beta, 2=RC, 3=Release, 4=Maintenance */
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "β"
#	define VER_REVISION 445
#	define VER_FULL "2.4.2 Beta"
#	define VER_SHORT "2.4b2"
#	define VER_SHORT_DOTS "2.4.2"
#	define VER_SHORT_GREEK "2.4β2"
#	define VER_RC_REVISION 2, 4, 2, 445
#	define VER_RC_STATUS 2, 4, 2, 1
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2016-08-26 17:53:22 +0000 (Fri, Aug 26 2016)"
#	define VER_REVISION_HASH "c1e0cbc"
#	define VER_REVISION_TAG "v2.4.2#445-beta"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1472234009
#	define VER_TIME_SEC 29
#	define VER_TIME_MIN 53
#	define VER_TIME_HOUR 17
#	define VER_TIME_DAY 26
#	define VER_TIME_MONTH 8
#	define VER_TIME_YEAR 2016
#	define VER_TIME_WDAY 5
#	define VER_TIME_YDAY 238
#	define VER_TIME_WDAY_SHORT "Fri"
#	define VER_TIME_WDAY_FULL "Friday"
#	define VER_TIME_MONTH_SHORT "Aug"
#	define VER_TIME_MONTH_FULL "August"
#	define VER_TIME "17:53:29"
#	define VER_DATE "2016-08-26"
#	define VER_DATE_LONG "Fri, Aug 26, 2016 17:53:29 UTC"
#	define VER_DATE_SHORT "2016-08-26 17:53:29 UTC"
#	define VER_DATE_ISO "2016-08-26T17:53:29Z"
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
