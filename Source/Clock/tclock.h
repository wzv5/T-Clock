//===============================================================================
//--+++--> tclock.h - KAZUBON  1997-1999 =========================================
//=================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
//-------------------------{ Stoic Joker 2006-2010 }-------------------------+++-->
#pragma once

#include "../common/globals.h"

#include <stdio.h>//sprintf
#include <Uxtheme.h>//SetWindowTheme
#include <WindowsX.h>//Edit_SetText
#include <ShlObj.h>//IShellDispatch4
#include <Shlwapi.h>//PathFileExists
#include <Psapi.h>//EmptyWorkingSet
#include "../common/newapi.h"
#include "../common/utl.h"

// replacement of standard library's functions
//int _strnicmp(const char* d, const char* s, size_t n);
//int _stricmp(const char* d, const char* s);
//#define malloc(s) GlobalAllocPtr(GHND,(s))
//#define free(p) GlobalFreePtr(p);
//int atoi(const char* p);

// IDs for timer
#define IDTIMER_START				2
#define IDTIMER_MAIN				3
#define IDTIMER_MOUSE				4
#define IDTIMER_DEKSTOPICON			5
#define IDTIMER_DESKTOPICONSTYLE	6

// for mouse.c and pagemouse.c
#define MOUSEFUNC_NONE			0
#define MOUSEFUNC_TIMER			5
#define MOUSEFUNC_CLIPBOARD		6
#define MOUSEFUNC_SCREENSAVER	7
#define MOUSEFUNC_SHOWCALENDER	8
#define MOUSEFUNC_SHOWPROPERTY	9

// System Global HotKey Identifiers
#define HOT_WATCH	200
#define HOT_TIMER	210
#define HOT_STOPW	220
#define HOT_PROPR	230
#define HOT_CALEN	240
#define HOT_TSYNC	250

// Jack Russel Message Windo Goes Boing!
#define JRMSG_BOING 15000

//--+++--> main.c - Application Global Values:
extern char		g_mydir[];			// Path to Clock.exe
extern HWND		g_hDlgTimer;		// Timer Dialog Window Handle
extern HWND		g_hDlgStopWatch;	// Stopwatch Dialog Window Handle
extern HWND		g_hDlgTimerWatch;	// Timwe Watch Dialog Window Handle
extern HWND		g_hwndSheet;		// (TCM Property Sheet Window Handle
extern HICON	g_hIconTClock, g_hIconPlay, // Frequently Used Icon Handles
				g_hIconStop, g_hIconDel; // Frequently Used Icon Handles
extern BOOL bMonOffOnLock; //-+> Locking Workstation Turns Off Monitor(s).

void RegisterSession(HWND hwnd);
void UnregisterSession(HWND hwnd);
void RefreshUs();

// propsheet.c
extern BOOL g_bApplyClock;
void MyPropertySheet(int page);
extern BOOL g_bApplyTaskbar;
void SetMyDialgPos(HWND hwnd,int padding);
BOOL SelectMyFile(HWND hDlg, const char* filter, DWORD nFilterIndex, const char* deffile, char* retfile);

// alarm.c
typedef struct{
	char name[TNY_BUFF];
	BOOL bAlarm;
	int hour;
	int minute;
	char fname[MAX_BUFF];
	char jrMessage[MAX_BUFF];
	char jrSettings[TNY_BUFF];
	BOOL jrMsgUsed;
	BOOL bHour12;
	BOOL bChimeHr;
	BOOL bRepeat;
	int iTimes;
	BOOL bBlink;
	BOOL bPM;
	int days;
} alarm_t;
BOOL GetHourlyChime();
void SetHourlyChime(BOOL bEnabled);
BOOL GetAlarmEnabled(int idx);
void SetAlarmEnabled(int idx,BOOL bEnabled);
void ReadAlarmFromReg(alarm_t* pAS, int num);
void SaveAlarmToReg(alarm_t* pAS, int num);
void StopFile();
void EndAlarm();
void InitAlarm();
void OnMCINotify(HWND hwnd);
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st);
BOOL PlayFile(HWND hwnd, char* fname, DWORD dwLoops);

// alarmday.c
#define ALARMDAY_OKFLAG 0x80000000
int ChooseAlarmDay(HWND hDlg, unsigned days);

// soundselect.c
BOOL IsMMFile(const char* fname);
BOOL BrowseSoundFile(HWND hDlg, const char* deffile, char* fname);

// pageformat.c
void InitFormat();
void CreateFormat(char* s, int* checks);

// menu.c
void OnTClockCommand(HWND hwnd, WORD wID);
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);

// mouse.c
extern const char g_reg_mouse[];
void OnTimerMouse(HWND hwnd);
void OnDropFiles(HWND hwnd, HDROP hdrop);
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// timer.c
#define MAX_TIMER  7
void WatchTimer(void);
void CancelAllTimersOnStartUp(void);

void EndTimer(void);
void DialogTimer();
void StopTimer(int n);
void OnTimerTimer(HWND hwnd);
int GetTimerInfo(char* dst, int num, BOOL bNameOnly);

// StopWatch.c
void DialogStopWatch();

// ExitWindows.c
BOOL ShutDown();
BOOL ReBoot();
BOOL LogOff();

// TCDLL.DLL‚ÌAPI
void WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd();

// PageHotKey.c
void GetHotKeyInfo(HWND hWnd);

// SNTP.c
void SyncTimeNow();
void NetTimeConfigDialog();

// BounceWind.c
void OnMsgWindOpt(HWND hDlg);
void ReleaseTheHound(HWND hWnd, BOOL);

// Macros
#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)

#define CBSetDroppedWidth(hDlg,id,width) SendDlgItemMessage((hDlg),(id),CB_SETDROPPEDWIDTH,width,0)
#define CBFindStringExact(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,0,(LPARAM)(s))
#define CBGetLBText(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_GETLBTEXT,(i),(LPARAM)(s))
#define CBAddString(hDlg,id,lParam) SendDlgItemMessage((hDlg),(id),CB_ADDSTRING,0,(LPARAM)(lParam))
#define CBInsertString(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_INSERTSTRING,(i),(LPARAM)(s))
//#define CBFindString(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRING,0,(LPARAM)(s))
#define CBDeleteString(hDlg,id, i) SendDlgItemMessage((hDlg),(id),CB_DELETESTRING,(i),0)
#define CBResetContent(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_RESETCONTENT,0,0)
#define CBSetItemData(hDlg,id,i,lParam) SendDlgItemMessage((hDlg),(id),CB_SETITEMDATA,(i),(LPARAM)(lParam))
#define CBGetItemData(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_GETITEMDATA,(i),0)
#define CBSetCurSel(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_SETCURSEL,(i),0)
#define CBGetCurSel(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCURSEL,0,0)
#define CBGetCount(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCOUNT,0,0)

//----------------//--------------+++--> HotKey Configuration,
typedef struct { //--+++--> Manipulation, & Storage Structure.
	UINT vk;
	UINT fsMod;
	BOOL bValid;
	char szText[TNY_BUFF];
} hotkey_t;
