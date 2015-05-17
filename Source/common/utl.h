#pragma once
#ifndef TCLOCK_UTL_H
#define TCLOCK_UTL_H
enum{
	TOS_2000	=0x0001, /**< does anyone still use it? \sa g_tos */
	TOS_XP		=0x0002, /**< the king is dead, long live the king! \sa g_tos */
	TOS_VISTA	=0x0004, /**< first one with lots of cool stuff... poorly implemented though \sa g_tos */
	TOS_WIN7	=0x0008, /**< best OS as of 2009-2015 as some might say \sa g_tos */
	TOS_WIN8	=0x0010, /**< first to support multiple taskbars, yet buggy \sa g_tos */
	TOS_WIN8_1	=0x0020, /**< first to require weird/stupid manifest... \sa g_tos */
	TOS_WIN10	=0x0040, /**< latest, didn't add anything new to T-Clock yet \sa g_tos */
	TOS_NEWER	=0x8000, /**< in case we're "outdated" and the curent OS is newer than our known ones \sa g_tos */
};
/**
 * \brief holds current OS version flags
 * \sa CheckSystemVersion(), TOS_2000, TOS_XP, TOS_VISTA, TOS_WIN7, TOS_WIN8, TOS_WIN8_1, TOS_WIN10, TOS_NEWER */
extern unsigned short g_tos;
/**
 * \brief checks current system version and writes into \c g_tos
 * \return 1 on success, 0 on failure (either API failure or OS too old)
 * \remarks call only once to initialize \c g_tos
 * \sa g_tos, TOS_2000, TOS_XP, TOS_VISTA, TOS_WIN7, TOS_WIN8, TOS_WIN8_1, TOS_WIN10, TOS_NEWER */
BOOL CheckSystemVersion();
typedef ULONGLONG (WINAPI* GetTickCount64_t)();
/**
 * \brief our GetTickCount64 helper that uses GetTickCount64() if available, otherwise GetTickCount()
 * \sa GetTickCount(), GetTickCount64() */
extern GetTickCount64_t pGetTickCount64;
// clock related
/**
 * \brief finds the tray clock handle */
HWND FindClock();
/**
 * \brief refreshes taskbar and clock */
void RefreshUs();
/**
 * \brief message box wrapper that uses our icon */
int MyMessageBox(HWND hwnd, const char* msg, const char* title, UINT uType, UINT uBeep);
// unsorted
/**
 * \brief checks if given path exists */
char PathExists(const char* path);
/**
 * \brief extracts filename and parameters from command (used by ExecFile()) */
void GetFileAndOption(const char* command, char* fname, char* opt);
/**
 * \brief a wrapper for ShellExecuteEx()
 * \param method "open", "runas", etc.
 * \param app path to run
 * \param params = \c NULL (optional program arguments)
 * \param parent = \c NULL (parent window)
 * \param show = \c SW_SHOWNORMAL
 * \return -1 on failure, 0 on success,1 if user cancled
 * \sa ShellExecute(), ShellExecuteEx(), Exec() */
int MyShellExecute(const char* method, const char* app, const char* params, HWND parent, int show);
/**
 * \brief starts an application
 * \param app path to run
 * \param params = \c NULL (optional program arguments)
 * \param parent = \c NULL (parent window)
 * \return -1 on failure, 0 on success, 1 if user cancled
 * \sa ExecElevated(), ExecFile(), MyShellExecute() */
int Exec(const char* app, const char* params, HWND parent);
/**
 * \brief opens a file or starts an application
 * \param command full commandline with filename and optional arguments
 * \param parent = \c NULL (parent window)
 * \return -1 on failure, 0 on success, 1 if user cancled
 * \sa Exec(), ExecElevated(), MyShellExecute() */
int ExecFile(const char* command, HWND parent);
/**
 * \brief converts a hex string to integer
 * \param p hex string to convert
 * \return parsed integer */
int atox(const char* p);
/**
 * \brief deletes the end of a path from \c path, eg. "C:/out.exe" becomes "C:/"
 * \param[in,out] path path to delete from
 * \sa add_title(), get_title() */
void del_title(char* path);
/**
 * \brief forces hwnd to the foreground,
 * even when the current process isn't the foreground process.
 * (hooks into the current foreground process to complete)
 * \param hwnd target window
 * \sa SetForegroundWindow(), SetActiveWindow() */
void ForceForegroundWindow(HWND hwnd);
/**
 * \brief multiplies two 32bit values and returns 64bit result
 * \param a left side of multiplication
 * \param b right side of multiplication */
ULONGLONG M32x32to64(DWORD a, DWORD b);
/**
 * \brief adds a title to a path, eg. "out.exe" to "C:" results "C:/out.exe"
 * \param[in,out] path to manipulate
 * \param[in] title to add (can be relative or absolute)
 * \sa get_title(), del_title() */
void add_title(char* path, const char* title);
/**
 * \brief get the title from path, eg. "out.exe" from "C:/out.exe"
 * \param[out] dst receives the title
 * \param[in] path to get title from
 * \sa add_title(), del_title() */
void get_title(char* dst, const char* path);
/**
 * \brief case-insensitively compares if ext matches the file extension of fname
 * \param fname file to test for \c ext
 * \param ext extension to find
 * \return 0 if both extensions match, otherwise the difference */
int ext_cmp(const char* fname, const char* ext);
//void parse(char* dst, const char* src, int n);
//void parsechar(char* dst, const char* src, char ch, int n);
/**
 * \brief adds a C string to a double-zero terminated string list. That is ABC,0,EFG,0,0
 * \param[in,out] list string list to add \c str to
 * \param[in] str C string to add */
void str0cat(char* list, const char* str);
// Settings
/**
 * \brief read a string value from our registry
 * \param[in] section,entry
 * \param[out] val output buffer of \c len size
 * \param[in] len size of \c val
 * \param[in] defval default value to return if \c entry wasn't found
 * \return size of returned string excl. zero terminator */
int GetMyRegStr(const char* section, const char* entry, char* val, int len, const char* defval);
/**
 * \brief try to read a string value from our registry or add it if missing
 * \param[in] section,entry
 * \param[out] val output buffer of \c len size
 * \param[in] len size of \c val
 * \param[in] defval default value to write and return if \c entry wasn't found
 * \return size of returned string excl. zero terminator */
int GetMyRegStrEx(const char* section, const char* entry, char* val, int len, const char* defval);
/**
 * \brief read a string value from Windows' registry
 * \param[in] rootkey,section,entry
 * \param[out] val output buffer of \c len size
 * \param[in] len size of \c val
 * \param[in] defval default value to return if \c entry wasn't found
 * \return size of returned string excl. zero terminator */
int GetRegStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval);
/**
 * \brief read a long value from Windows' registry
 * \param rootkey,section,entry
 * \param defval default value to return if entry wasn't found
 * \return read long or defval on failure */
LONG GetRegLong(HKEY rootkey, const char* section, const char* entry, LONG defval);
/**
 * \brief update or add a string value in Windows' registry
 * \param rootkey,section,entry
 * \param val new value
 * \return boolean */
BOOL SetRegStr(HKEY rootkey, const char* section, const char* entry, const char* val);
/**
 * \brief try to read a long value from our registry or add it if missing
 * \param section,entry
 * \param defval default value to write and return
 * \return read long or defval on failure */
LONG GetMyRegLongEx(const char* section, const char* entry, LONG defval);
/**
 * \brief read a long value from our registry
 * \param section,entry
 * \param defval default value to return
 * \return read long or defval */
LONG GetMyRegLong(const char* section, const char* entry, LONG defval);
/**
 * \brief update or add a long value in our registry
 * \param section,entry
 * \param val new value
 * \return boolean */
BOOL SetMyRegLong(const char* section, const char* entry, LONG val);
/**
 * \brief update or add a string value in our registry
 * \param section,entry
 * \param val new value
 * \return boolean */
BOOL SetMyRegStr(const char* section, const char* entry, const char* val);
/**
 * \brief deletes a value from our registry
 * \param section
 * \param entry value to delete
 * \return boolean */
BOOL DelMyReg(const char* section, const char* entry);
/**
 * \brief deletes an entire key from our registry
 * \param section key to delete
 * \return boolean */
BOOL DelMyRegKey(const char* section);
/**
 * \brief returns a string with \c id from our string table
 * \param id of string to return
 * \return \c NULL on failure */
char* MyString(UINT id);
//void Pause(HWND hWnd, LPCTSTR pszArgs);
#endif // TCLOCK_UTL_H
