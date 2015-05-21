#ifndef WIN2K_COMPAT_H_
#define WIN2K_COMPAT_H_

#ifdef WIN2K_COMPAT

#include <inttypes.h>
#include <string.h>
errno_t win2k_strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count);
errno_t win2k_wcsncpy_s(wchar_t* strDest, size_t numberOfElements, const wchar_t* strSource, size_t count);
#define strncpy_s win2k_strncpy_s
#define wcsncpy_s win2k_wcsncpy_s
char* win2k_strtok_s(char* strToken, const char* strDelimit, char** context);
#define strtok_s win2k_strtok_s

#endif // WIN2K_COMPAT

#endif // WIN2K_COMPAT_H_
