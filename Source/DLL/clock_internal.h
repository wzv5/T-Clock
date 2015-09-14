#ifndef CLOCK_INTERNAL_H_
#define CLOCK_INTERNAL_H_
#include <windows.h>

extern REGSAM ms_reg_fullaccess; /**< full registry access. set to \c KEY_ALL_ACCESS | \c KEY_WOW64_64KEY */
extern REGSAM ms_reg_read; /**< read-only registry access. set to \c KEY_READ | \c KEY_WOW64_64KEY */
extern char ms_bIniSetting; /**< use ini or registry */
extern char ms_inifile[MAX_PATH]; /**< path to ini */

typedef ULONGLONG (WINAPI* GetTickCount64_t)();

extern unsigned short gs_tos; /**< \sa TClockAPI::OS */

void Clock_Inject(HWND hwndMain);
void Clock_InjectFinalize();

/** \sa TClockAPI::Exit() */
void Clock_Exit();

/** \sa TClockAPI::GetCalendar() */
HWND Clock_GetCalendar();
/** \sa TClockAPI::Message() */
int Clock_Message(HWND parent, const char* msg, const char* title, UINT uType, UINT uBeep);
/** \sa TClockAPI::PositionWindow() */
void Clock_PositionWindow(HWND hwnd, int padding);
/** \sa TClockAPI::GetColor() */
unsigned Clock_GetColor(unsigned agbr,int useraw);
/** \sa TClockAPI::On_DWMCOLORIZATIONCOLORCHANGED() */
void Clock_On_DWMCOLORIZATIONCOLORCHANGED(unsigned argb, BOOL blend);

/** \sa TClockAPI::GetInt() */
int Clock_GetInt(const char* section, const char* entry, LONG defval);
/** \sa TClockAPI::GetIntEx() */
int Clock_GetIntEx(const char* section, const char* entry, LONG defval);
/** \sa TClockAPI::GetSystemInt() */
int Clock_GetSystemInt(HKEY rootkey, const char* section, const char* entry, LONG defval);
/** \sa TClockAPI::GetStr() */
int Clock_GetStr(const char* section, const char* entry, char* val, int len, const char* defval);
/** \sa TClockAPI::GetStrEx() */
int Clock_GetStrEx(const char* section, const char* entry, char* val, int len, const char* defval);
/** \sa TClockAPI::GetSystemStr() */
int Clock_GetSystemStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval);
/** \sa TClockAPI::SetInt() */
int Clock_SetInt(const char* section, const char* entry, LONG val);
/** \sa TClockAPI::SetStr() */
int Clock_SetStr(const char* section, const char* entry, const char* val);
/** \sa TClockAPI::SetSystemStr() */
int Clock_SetSystemStr(HKEY rootkey, const char* section, const char* entry, const char* val);
/** \sa TClockAPI::DelValue() */
int Clock_DelValue(const char* section, const char* entry);
/** \sa TClockAPI::DelKey() */
int Clock_DelKey(const char* section);

/** \sa TClockAPI::PathExists() */
char Clock_PathExists(const char* path);
/** \sa TClockAPI::GetFileAndOption() */
int Clock_GetFileAndOption(const char* command, char* app, char* params);
/** \sa TClockAPI::ShellExecute() */
int Clock_ShellExecute(const char* method, const char* app, const char* params, HWND parent, int show);
/** \sa TClockAPI::Exec() */
int Clock_Exec(const char* app, const char* params, HWND parent);
/** \sa TClockAPI::ExecElevated() */
int Clock_ExecElevated(const char* app, const char* params, HWND parent);
/** \sa TClockAPI::ExecFile() */
int Clock_ExecFile(const char* command, HWND parent);

// format stuff
/** \sa TClockAPI::GetFormat() */
char Clock_GetFormat(const char** offset, int* minimum, int* padding);
/** \sa TClockAPI::WriteFormatNum() */
int Clock_WriteFormatNum(char* buffer, int number, int minimum, int padding);


// translation API

/** \sa TClockAPI::T() */
const char* Clock_T(int hash);
/** \sa TClockAPI::Translate() */
const char* Clock_Translate(const char* str);
/** \sa TClockAPI::TranslateWindow() */
const char* Clock_TranslateWindow(HWND hwnd);

#endif // CLOCK_INTERNAL_H_
