#pragma once
#ifndef TCLOCK_UTL_H
#define TCLOCK_UTL_H
//extern char g_bIniSetting;
//extern char g_inifile[];
enum{
	TOS_2000	=0x0001,
	TOS_XP		=0x0002,
	TOS_VISTA	=0x0004,
	TOS_WIN7	=0x0008,
	TOS_WIN8	=0x0010,
	TOS_WIN8_1	=0x0011,//requires weird manifest...
	TOS_NEWER	=0x8000,
};
extern unsigned short g_tos; // holds current OS version flags
BOOL CheckSystemVersion();
void RefreshUs(void);
char PathExists(const char* path);
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL ExecFile(HWND hwnd, const char* command);
int atox(const char* p);
void del_title(char* path);
void ForceForegroundWindow(HWND hwnd);
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
