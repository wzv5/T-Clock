#ifndef CLOCK_INTERNAL_H_
#define CLOCK_INTERNAL_H_
#include <windows.h>

extern REGSAM ms_reg_fullaccess; /**< full registry access. set to \c KEY_ALL_ACCESS | \c KEY_WOW64_64KEY */
extern REGSAM ms_reg_read; /**< read-only registry access. set to \c KEY_READ | \c KEY_WOW64_64KEY */
extern wchar_t ms_inifile[MAX_PATH]; /**< path to ini if set */

typedef ULONGLONG (WINAPI* GetTickCount64_t)();

extern unsigned short gs_tos; /**< \sa TClockAPI::OS */

/** \sa TClockAPI::Inject() */
int Clock_Inject(HWND hwndMain);
/** \sa TClockAPI::InjectFinalize() */
void Clock_InjectFinalize();
/** \sa TClockAPI::Exit() */
void Clock_Exit();

/** \sa TClockAPI::GetClock() */
HWND Clock_GetClock(int uncached);
/** \sa TClockAPI::GetCalendar() */
HWND Clock_GetCalendar();
/** \sa TClockAPI::Message() */
int Clock_Message(HWND parent, const wchar_t* msg, const wchar_t* title, UINT uType, UINT uBeep);
/** \sa TClockAPI::PositionWindow() */
void Clock_PositionWindow(HWND hwnd, int padding);
/** \sa TClockAPI::GetColor() */
unsigned Clock_GetColor(unsigned agbr,int useraw);
/** \sa TClockAPI::On_DWMCOLORIZATIONCOLORCHANGED() */
void Clock_On_DWMCOLORIZATIONCOLORCHANGED(unsigned argb, BOOL blend);

/** \sa TClockAPI::GetInt() */
int Clock_GetInt(const wchar_t* section, const wchar_t* entry, int defval);
/** \sa TClockAPI::GetInt64() */
int64_t Clock_GetInt64(const wchar_t* section, const wchar_t* entry, int64_t defval);
/** \sa TClockAPI::GetIntEx() */
int Clock_GetIntEx(const wchar_t* section, const wchar_t* entry, int defval);
/** \sa TClockAPI::GetSystemInt() */
int Clock_GetSystemInt(HKEY rootkey, const wchar_t* section, const wchar_t* entry, int defval);
/** \sa TClockAPI::GetStr() */
int Clock_GetStr(const wchar_t* section, const wchar_t* entry, wchar_t* val, int len, const wchar_t* defval);
/** \sa TClockAPI::GetStrEx() */
int Clock_GetStrEx(const wchar_t* section, const wchar_t* entry, wchar_t* val, int len, const wchar_t* defval);
/** \sa TClockAPI::GetSystemStr() */
int Clock_GetSystemStr(HKEY rootkey, const wchar_t* section, const wchar_t* entry, wchar_t* val, int len, const wchar_t* defval);
/** \sa TClockAPI::SetInt() */
int Clock_SetInt(const wchar_t* section, const wchar_t* entry, int val);
/** \sa TClockAPI::SetInt64() */
int Clock_SetInt64(const wchar_t* section, const wchar_t* entry, int64_t val);
/** \sa TClockAPI::SetSystemInt() */
int Clock_SetSystemInt(HKEY rootkey, const wchar_t* section, const wchar_t* entry, int val);
/** \sa TClockAPI::SetStr() */
int Clock_SetStr(const wchar_t* section, const wchar_t* entry, const wchar_t* val);
/** \sa TClockAPI::SetSystemStr() */
int Clock_SetSystemStr(HKEY rootkey, const wchar_t* section, const wchar_t* entry, const wchar_t* val);
/** \sa TClockAPI::DelValue() */
int Clock_DelValue(const wchar_t* section, const wchar_t* entry);
/** \sa TClockAPI::DelSystemValue() */
int Clock_DelSystemValue(HKEY rootkey, const wchar_t* section, const wchar_t* entry);
/** \sa TClockAPI::DelKey() */
int Clock_DelKey(const wchar_t* section);

/** \sa TClockAPI::PathExists() */
int Clock_PathExists(const wchar_t* path);
/** \sa TClockAPI::GetFileAndOption() */
int Clock_GetFileAndOption(const wchar_t* command, wchar_t* app, wchar_t* params);
/** \sa TClockAPI::ShellExecute() */
int Clock_ShellExecute(const wchar_t* method, const wchar_t* app, const wchar_t* params, HWND parent, int show, HANDLE* hProcess);
/** \sa TClockAPI::Exec() */
int Clock_Exec(const wchar_t* app, const wchar_t* params, HWND parent);
/** \sa TClockAPI::ExecElevated() */
int Clock_ExecElevated(const wchar_t* app, const wchar_t* params, HWND parent);
/** \sa TClockAPI::ExecFile() */
int Clock_ExecFile(const wchar_t* command, HWND parent);

// format stuff
/** \sa TClockAPI::GetFormat() */
wchar_t Clock_GetFormat(const wchar_t** offset, int* minimum, int* padding);
/** \sa TClockAPI::WriteFormatNum() */
int Clock_WriteFormatNum(wchar_t* buffer, int number, int minimum, int padding);


// translation API

/** \sa TClockAPI::T() */
const wchar_t* Clock_T(int hash);
/** \sa TClockAPI::Translate() */
const wchar_t* Clock_Translate(const wchar_t* str);
/** \sa TClockAPI::TranslateWindow() */
const wchar_t* Clock_TranslateWindow(HWND hwnd);

#endif // CLOCK_INTERNAL_H_
