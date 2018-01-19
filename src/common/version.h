#ifndef AUTOVERSION_H
#define AUTOVERSION_H
/* Note: to use integer defines as strings, use STR(), eg. STR(VER_REVISION) */
/**** Version ****/
#	define VER_MAJOR 2
#	define VER_MINOR 4
#	define VER_BUILD 3
	/** status values: 0=Alpha(α), 1=Beta(β), 2=RC(гc), 3=Release(г), 4=Maintenance(гm) */
#	define VER_STATUS 1
#	define VER_STATUS_FULL "Beta"
#	define VER_STATUS_SHORT "b"
#	define VER_STATUS_GREEK "\u03B2"
#	define VER_REVISION 472
#	define VER_FULL "2.4.3 Beta"
#	define VER_SHORT "2.4b3"
#	define VER_SHORT_DOTS "2.4.3"
#	define VER_SHORT_GREEK "2.4\u03B23"
#	define VER_RC_REVISION 2, 4, 3, 472
#	define VER_RC_STATUS 2, 4, 3, 1
/**** Subversion Information ****/
#	define VER_REVISION_URL "git@github.com:White-Tiger/T-Clock.git"
#	define VER_REVISION_DATE "2017-05-21 12:55:51 +0000 (Sun, May 21 2017)"
#	define VER_REVISION_HASH "3048f3a"
#	define VER_REVISION_TAG "v2.4.3#472-beta"
/**** Date/Time ****/
#	define VER_TIMESTAMP 1516026234
#	define VER_TIME_SEC 54
#	define VER_TIME_MIN 23
#	define VER_TIME_HOUR 14
#	define VER_TIME_DAY 15
#	define VER_TIME_MONTH 1
#	define VER_TIME_YEAR 2018
#	define VER_TIME_WDAY 1
#	define VER_TIME_YDAY 14
#	define VER_TIME_WDAY_SHORT "Mon"
#	define VER_TIME_WDAY_FULL "Monday"
#	define VER_TIME_MONTH_SHORT "Jan"
#	define VER_TIME_MONTH_FULL "January"
#	define VER_TIME "14:23:54"
#	define VER_DATE "2018-01-15"
#	define VER_DATE_LONG "Mon, Jan 15, 2018 14:23:54 UTC"
#	define VER_DATE_SHORT "2018-01-15 14:23:54 UTC"
#	define VER_DATE_ISO "2018-01-15T14:23:54Z"
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
