#pragma once
#ifndef TCLOCK_UTL_H
#define TCLOCK_UTL_H
//extern char g_bIniSetting;
//extern char g_inifile[];
extern BOOL bV7up;
extern BOOL b2000;
BOOL CheckSystemVersion();
void RefreshUs(void);
char PathExists(const char* path);
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL ExecFile(HWND hwnd, const char* command);
void ToggleCalendar();
int atox(const char* p);
void del_title(char* path);
void ForceForegroundWindow(HWND hWnd);
DWORDLONG M32x32to64(DWORD a, DWORD b);
void parse(char* dst, const char* src, int n);
void add_title(char* path, const char* title);
void get_title(char* dst, const char* path);
int ext_cmp(const char* fname, const char* ext);
void parsechar(char* dst, const char* src, char ch, int n);
int MyMessageBox(HWND hwnd, const char* msg, const char* title, UINT uType, UINT uBeep);
int GetMyRegStr(const char* section, const char* entry, char* val, int len, const char* defval);
int GetMyRegStrEx(const char* section, const char* entry, char* val, int len, const char* defval);
int GetRegStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval);
LONG GetRegLong(HKEY rootkey, const char* section, const char* entry, LONG defval);
BOOL SetRegStr(HKEY rootkey, const char* section, const char* entry, const char* val);
LONG GetMyRegLongEx(const char* section, const char* entry, LONG defval);
LONG GetMyRegLong(const char* section, const char* entry, LONG defval);
BOOL SetMyRegLong(const char* section, const char* entry, LONG val);
BOOL SetMyRegStr(const char* section, const char* entry, const char* val);
BOOL DelMyReg(const char* section, const char* entry);
void str0cat(char* dst, const char* src);
BOOL DelMyRegKey(const char* section);
char* MyString(UINT id);
//void Pause(HWND hWnd, LPCTSTR pszArgs);
#endif // TCLOCK_UTL_H
