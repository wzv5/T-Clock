#ifndef WIN2K_COMPAT_H_
#define WIN2K_COMPAT_H_
#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN2K_COMPAT

#include <stdint.h>
#include <string.h>
errno_t win2k_strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count);
errno_t win2k_wcsncpy_s(wchar_t* strDest, size_t numberOfElements, const wchar_t* strSource, size_t count);
#define strncpy_s win2k_strncpy_s
#define wcsncpy_s win2k_wcsncpy_s
char* win2k_strtok_s(char* strToken, const char* strDelimit, char** context);
wchar_t* win2k_wcstok_s(wchar_t* strToken, const wchar_t* strDelimit, wchar_t** context);
#define strtok_s win2k_strtok_s
#define wcstok_s win2k_wcstok_s

#include <windows.h>
#include <commctrl.h>
BOOL win2k_SetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
BOOL win2k_GetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR* pdwRefData);
BOOL win2k_RemoveWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass);
typedef LRESULT (WINAPI *DefSubclassProc_ptr)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern DefSubclassProc_ptr win2k_DefSubclassProc;
#define SetWindowSubclass win2k_SetWindowSubclass
#define GetWindowSubclass win2k_GetWindowSubclass
#define RemoveWindowSubclass win2k_RemoveWindowSubclass
#define DefSubclassProc win2k_DefSubclassProc

#include <wtsapi32.h>
DWORD win2k_WTSGetActiveConsoleSessionId(void);
BOOL win2k_WTSRegisterSessionNotification(HWND hWnd, DWORD dwFlags);
BOOL win2k_WTSUnRegisterSessionNotification(HWND hWnd);
#define WTSGetActiveConsoleSessionId win2k_WTSGetActiveConsoleSessionId
#define WTSRegisterSessionNotification win2k_WTSRegisterSessionNotification
#define WTSUnRegisterSessionNotification win2k_WTSUnRegisterSessionNotification

void OpportunisticConsole();

#else
#	define OpportunisticConsole() {FreeConsole();AttachConsole(ATTACH_PARENT_PROCESS);}
#endif // WIN2K_COMPAT
#ifdef __cplusplus
}
#endif
#endif // WIN2K_COMPAT_H_
