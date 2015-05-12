//===============================================================================
//--+++--> tclock.h - KAZUBON  1997-1999 =========================================
//=================== Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
//-------------------------{ Stoic Joker 2006-2010 }-------------------------+++-->
#pragma once

#include "../common/globals.h"

#include <stdio.h>//sprintf
#include <Uxtheme.h>//SetWindowTheme
#include <Shlwapi.h>//PathFileExists
#include <Psapi.h>//EmptyWorkingSet
#include "../common/newapi.h"
#include "../common/utl.h"

// TCDLL.DLL‚API
typedef int (WINAPI *IsCalendarOpen_t)(int focus);
typedef void (WINAPI *HookStart_t)(HWND hwnd);
typedef void (WINAPI *HookEnd_t)();
typedef void (WINAPI *ClockExit_t)();
extern IsCalendarOpen_t IsCalendarOpen;
extern HookStart_t HookStart;
extern HookEnd_t HookEnd;
extern ClockExit_t ClockExit;

// IDs for timer
#define IDTIMER_START				2
#define IDTIMER_MAIN				3
#define IDTIMER_MOUSE				4
#define IDTIMER_DEKSTOPICON			5
#define IDTIMER_DESKTOPICONSTYLE	6

// for mouse.c and pagemouse.c
#define MOUSEFUNC_NONE			0
#define MOUSEFUNC_MENU			1
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

//--+++--> main.c - Application Global Values:
extern char		g_mydir[MAX_PATH];	/**< path to Clock[64].exe */
extern HWND		g_hDlgTimer;		/**< timer dialog handle */
extern HWND		g_hDlgStopWatch;	/**< stopwatch dialog handle */
extern HWND		g_hDlgTimerWatch;	/**< timer watch dialog handle */
extern HWND		g_hwndSheet;		/**< property sheet window */
/** frequently used icon handles */
extern HICON	g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;
extern BOOL bMonOffOnLock; /**< locking workstation turns off monitor(s) */

/**
 * \brief returns full path to currently started Clock[64].exe
 * \return path to Clock.exe incl. filename
 * \remark currently just a wrapper for \c _pgmptr */
inline const char* GetClockExe(){
	return _pgmptr;
}

void RegisterSession(HWND hwnd);
void UnregisterSession(HWND hwnd);
void ToggleCalendar(int type);
int GetStartupFile(HWND hDlg,char filename[MAX_PATH]);
void AddStartup(HWND hDlg);
void RemoveStartup(HWND hDlg);
int CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name);

// settings.c
int CheckSettings();

// propsheet.c
extern char g_bApplyClock;
extern char g_bApplyTaskbar;
void MyPropertySheet(int page);
void SetMyDialgPos(HWND hwnd,int padding);
BOOL SelectMyFile(HWND hDlg, const char* filter, DWORD nFilterIndex, const char* deffile, char* retfile);

typedef struct{
	char name[TNY_BUFF];
	char message[512];
	char settings[TNY_BUFF];
} dlgmsg_t;
// alarm.c
enum{
	ALRM_ENABLED=0x01,
	ALRM_ONESHOT=0x02,
	ALRM_12H	=0x04,
	ALRM_PM		=0x08,
	ALRM_CHIMEHR=0x10,
	ALRM_REPEAT	=0x20,
	ALRM_BLINK	=0x40,
	ALRM_DIALOG	=0x80,
};
#define ALRM_12HPM (ALRM_12H|ALRM_PM)
typedef struct{
	int days;
	int hour;
	int minute;
	int iTimes;
	unsigned char uFlags;
	char fname[MAX_PATH];
	dlgmsg_t dlgmsg;
} alarm_t;
BOOL GetHourlyChime();
void SetHourlyChime(BOOL bEnabled);
char GetAlarmEnabled(int idx);
void SetAlarmEnabled(int idx,char bEnabled);
void ReadAlarmFromReg(alarm_t* pAS, int num);
void SaveAlarmToReg(alarm_t* pAS, int num);
void StopFile();
int IsPlaying();
void EndAlarm();
void InitAlarm();
int OnMCINotify(HWND hwnd);
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

// menu.c
void OnTClockCommand(HWND hwnd, WORD wID);
void OnContextMenu(HWND hwnd, int xPos, int yPos);

// mouse.c
void OnTimerMouse(HWND hwnd);
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
// PageMouse.c
void CheckMouseMenu();

// timer.c
void UpdateTimerMenu(HMENU hMenu);

void StopTimer(int id);
void StartTimer(int id);
void ToggleTimer(int id);

void WatchTimer(int reset);
void CancelAllTimersOnStartUp();

void EndAllTimers();
void DialogTimer();
void OnTimerTimer(HWND hwnd);

// StopWatch.c
BOOL IsDialogStopWatchMessage(HWND hwnd, MSG* msg);
void DialogStopWatch();
void StopWatch_Start(HWND hDlg);
void StopWatch_Stop(HWND hDlg);
void StopWatch_Reset(HWND hDlg);
void StopWatch_Pause(HWND hDlg);
void StopWatch_Resume(HWND hDlg);
void StopWatch_Lap(HWND hDlg,int bFromStop);
void StopWatch_TogglePause(HWND hDlg);

// ExitWindows.c
BOOL ShutDown();
BOOL ReBoot();
BOOL LogOff();

// PageHotKey.c
void GetHotKeyInfo(HWND hWnd);

// SNTP.c
void SyncTimeNow();
void NetTimeConfigDialog();

// BounceWind.c
int BounceWindOptions(HWND hDlg, dlgmsg_t* dlg);
void ReleaseTheHound(HWND hwnd, const char* title, const char* text, char* settings);

// Macros
BOOL EnableDlgItemSafeFocus(HWND hDlg,int control,BOOL bEnable,int nextFocus);
#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)

#define CBSetDroppedWidth(hDlg,id,width) SendDlgItemMessage((hDlg),(id),CB_SETDROPPEDWIDTH,width,0)
#define CBFindStringExact(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)(s))
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
