/*--------------------------------------------------------
// tclock.c : customize the tray clock -> KAZUBON 1997-2001
//--------------------------------------------------------*/
// Modified by Stoic Joker: Tuesday, March 2 2010 - 10:42:42
#include "tcdll.h"
#include "../common/tcolor.h"

#define CLOCK_BORDER_MARGIN 2

void EndClock(void);
void OnTimer(HWND hwnd);
void ReadData(HWND hwnd, BOOL preview);
void InitClock(HWND hwnd);
int DestroyClock();
int UpdateClock(HWND hwnd, HFONT fnt);
int UpdateClockSize(HWND hwnd);
LRESULT OnCalcRect(HWND hwnd);
void InitDaylightTimeTransition(void);
void OnCopy(HWND hwnd, LPARAM lParam);
BOOL CheckDaylightTimeTransition(SYSTEMTIME* lt);
void OnTooltipNeedText(UINT code, LPARAM lParam);
void DrawClockSub(HDC hdc, SYSTEMTIME* pt, int beat100);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcMultiClock(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcMultiClockWorker(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//================================================================================================
//----------------------------------------+++--> Definition of Data Segment Shared Among Processes:
#ifndef __GNUC__
#pragma data_seg(".MYDATA") //--------------------------------------------------------------+++-->
HWND g_hwndTClockMain = NULL;
HWND g_hwndClock = NULL;
HHOOK g_hhook = 0;
#pragma data_seg()
#else
__attribute__((section(".MYDATA"),shared)) HWND g_hwndTClockMain = NULL;
__attribute__((section(".MYDATA"),shared)) HWND g_hwndClock = NULL;
__attribute__((section(".MYDATA"),shared)) HHOOK g_hhook = 0;
#endif // __GNUC__

/*------------------------------------------------
  globals
--------------------------------------------------*/
HINSTANCE hInstance = 0;
WNDPROC m_oldClockProc=NULL; // original clock procedure
WNDPROC m_oldWorkerProc=NULL; // original worker procedure used by multi clocks (Win8+)
#define MAX_MULTIMON_CLOCKS 4
int m_bMultimon;
ATOM m_multiClockClass=0;
struct {
	HWND worker;
	HWND clock;
	RECT workerRECT;
	LONG xdiff;
	LONG ydiff;
} m_multiClock[MAX_MULTIMON_CLOCKS];
HDC m_multiClockDC;
int m_multiClocks=0;
/// draw variables
static HDC m_hdcClock=NULL;
static HDC m_hdcClockBG;
static char m_bHorizontalTaskbar=1;
static RECT m_rcClock={0};
static RGBQUAD* m_color_start=NULL,* m_color_end;
static RGBQUAD* m_colorBG_start=NULL,* m_colorBG_end;
static HGDIOBJ m_oldfnt=NULL;
static HGDIOBJ m_oldbmp=NULL,m_oldbmpB=NULL;
/// text offsets
static double m_radian;
static int m_textheight,m_textwidth,m_textpadding;
static int m_leading;
static int m_vertfeed,m_vertpos;
static int m_horizfeed,m_horizpos;
/// colors
COLORREF m_basecolorBG, m_basecolorFont;
typedef struct tagBGRQUAD{
	BYTE rgbRed;
	BYTE rgbGreen;
	BYTE rgbBlue;
	BYTE rgbReserved;
} BGRQUAD;
union{
	BGRQUAD quad;
	COLORREF ref;
} m_col;
union{
	BGRQUAD quad;
	COLORREF ref;
} m_colBG;
#define g_col_update(col,m_basecolorBG) do{\
	COLORREF oldbg;\
	m_col.ref=GetTColor(col,0);\
	oldbg=m_colBG.ref;\
	m_colBG.ref=GetTColor(m_basecolorBG,1);\
	if(m_colBG.ref!=oldbg)\
		FillClockBG();\
	}while(0)
/// misc variables
int m_TipState=0;
HWND m_TipHwnd = NULL;
TOOLINFO m_TipInfo;
char m_format[256];
SYSTEMTIME m_LastTime={0};
int m_beatLast = -1;
int m_bDispSecond = FALSE;
int m_nDispBeat = 0;
enum{
	BLINK_NONE=0,
	BLINK_ON,
	BLINK_HOUR,
};
int m_BlinkState = BLINK_NONE;
int m_width=0, m_height=0, dvpos=0, dlineheight=0, dhpos=0;
char m_bTimer=0;
char g_bHour12, g_bHourZero;
char m_bNoClock=0;

//================================================================================================
//---------------------------------------------------------+++--> Create Mouse-Over ToolTip Window:
void CreateTip(HWND hwnd)   //--------------------------------------------------------------+++-->
{
//	hwndTip = CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,NULL, WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX|TTS_BALLOON,
	m_TipHwnd = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TRANSPARENT,TOOLTIPS_CLASS,NULL, WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,
							CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, NULL,NULL,hInstance,NULL);
	if(!m_TipHwnd) return;
	memset(&m_TipInfo,0,sizeof(TOOLINFO));
	m_TipInfo.cbSize = sizeof(TOOLINFO);
	m_TipInfo.uFlags = TTF_IDISHWND|TTF_TRACK|TTF_TRANSPARENT;
	m_TipInfo.hwnd = hwnd;
	m_TipInfo.uId = (UINT_PTR)hwnd;
	m_TipInfo.lpszText = LPSTR_TEXTCALLBACK;
	
	SendMessage(m_TipHwnd,TTM_ADDTOOL,0,(LPARAM)&m_TipInfo);
	SendMessage(m_TipHwnd,TTM_SETMAXTIPWIDTH,0,300);
	SendMessage(m_TipHwnd,TTM_TRACKPOSITION,0,MAKELPARAM(0x7FFF,0x7FFF));
}
void DestroyTip()
{
	if(!m_TipHwnd) return;
	SendMessage(m_TipHwnd,TTM_DELTOOL,0,(LPARAM)&m_TipInfo);
	DestroyWindow(m_TipHwnd); m_TipHwnd=NULL;
}
/// @todo (White-Tiger#1#09/06/14): use taskbar type detection here
void ShowTip(HWND clock){
	RECT rc; GetClientRect(clock,&rc);
	ClientToScreen(clock,(POINT*)&rc.left);
	ClientToScreen(clock,(POINT*)&rc.right);
	if(rc.left<128){//is left
//		PostMessage(g_Tip,TTM_TRACKPOSITION,0,MAKELPARAM(0,0x7FFF));
		PostMessage(m_TipHwnd,TTM_TRACKPOSITION,0,MAKELPARAM(rc.top,rc.right+3));
	}else{
//		PostMessage(g_Tip,TTM_TRACKPOSITION,0,MAKELPARAM(0x7FFF,0x7FFF));//it's some magic vodoo.. will always be over the clock and right^^ (died in multimonitor support! :/ RIP)
		if(rc.top<128){//is top
			PostMessage(m_TipHwnd,TTM_TRACKPOSITION,0,MAKELPARAM(rc.left,rc.bottom+3));
//		}else if(rc.top<128){//is right
//			PostMessage(g_Tip,TTM_TRACKPOSITION,0,MAKELPARAM(rc.left,rc.top-3));
		}else
			PostMessage(m_TipHwnd,TTM_TRACKPOSITION,0,MAKELPARAM(rc.right,rc.top-3));
	}
	PostMessage(m_TipHwnd,TTM_TRACKACTIVATE,TRUE,(LPARAM)&m_TipInfo);
}

void SubsDestroy(){
	for(; m_multiClocks; ){
		if(IsWindow(m_multiClock[--m_multiClocks].worker)){
			SetWindowLongPtr(m_multiClock[m_multiClocks].worker,GWL_WNDPROC,(LONG_PTR)m_oldWorkerProc);
			SendMessage(m_multiClock[m_multiClocks].clock,WM_CLOSE,0,0);
			SetWindowPos(m_multiClock[m_multiClocks].worker, HWND_TOP, 0,0,
						m_multiClock[m_multiClocks].workerRECT.right, m_multiClock[m_multiClocks].workerRECT.bottom,
						SWP_NOMOVE);
		}
	}
}
void SubsSendResize(){
	int i;
	for(i=0; i<m_multiClocks; ++i){
		SetWindowPos(m_multiClock[i].worker,0,0,0,10,10,0); // set new clock size and position
	}
}
void SubsCreate(){
	char classname[GEN_BUFF];
	HWND hwndBar;
	HWND hwndChild;
	int i;
	if(m_multiClocks) return;
	// loop all secondary taskbars
	if(m_bMultimon){
		hwndBar=FindWindowEx(NULL,NULL,"Shell_SecondaryTrayWnd",NULL);
		while(hwndBar){
			hwndChild=GetWindow(hwndBar,GW_CHILD);
			while(hwndChild){
				GetClassName(hwndChild,classname,sizeof(classname));
				if(!lstrcmpi(classname,"WorkerW")){
					if(m_multiClocks==MAX_MULTIMON_CLOCKS)
						break;
					for(i=0; i<m_multiClocks && hwndChild!=m_multiClock[i].worker; ++i);
					if(i==m_multiClocks){
						if(!m_multiClockClass){
							WNDCLASSEX wndclass={sizeof(WNDCLASSEX),CS_CLASSDC,WndProcMultiClock,0,0,0/*hInstance*/,NULL,NULL,NULL,NULL,"SecondaryTrayClockWClass",NULL};
							wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
							wndclass.hInstance=hInstance;
							m_multiClockClass=RegisterClassEx(&wndclass);
						}
						m_multiClock[i].clock=CreateWindowEx(0,(LPCSTR)m_multiClockClass,NULL,WS_CHILD|WS_VISIBLE,0,0,5,5,GetParent(hwndChild),0,0,0);
						if(!m_multiClock[i].clock)
							break;
						if(!i) m_multiClockDC=GetDC(m_multiClock[0].clock);
						GetClientRect(hwndChild,&m_multiClock[i].workerRECT);
						m_multiClock[i].worker=hwndChild;
						m_oldWorkerProc=(WNDPROC)GetWindowLongPtr(hwndChild,GWL_WNDPROC);
						SetWindowLongPtr(hwndChild,GWL_WNDPROC,(LONG_PTR)WndProcMultiClockWorker);
						++m_multiClocks;
					}
					break;
				}
				hwndChild=GetWindow(hwndChild,GW_HWNDNEXT);
			}
			hwndBar=FindWindowEx(NULL,hwndBar,"Shell_SecondaryTrayWnd",NULL);
		}
	}
}
//================================================================================================
//---------------------------------------------------------------------+++--> Initialize the Clock:
void InitClock(HWND hwnd)   //--------------------------------------------------------------+++-->
{
	CheckSystemVersion();
	g_hwndClock = hwnd;
	PostMessage(g_hwndTClockMain, MAINM_CLOCKINIT, 0, (LPARAM)g_hwndClock);
	
	ReadData(hwnd,0); //-+-> Get Configuration Information From Registry
//	SubsCreate();
	InitDaylightTimeTransition(); // Get User's Local Time-Zone Information
	
	m_oldClockProc = (WNDPROC)GetWindowLongPtr(g_hwndClock, GWL_WNDPROC);
	SetWindowLongPtr(g_hwndClock, GWL_WNDPROC, (LONG_PTR)WndProc);
	SetClassLong(g_hwndClock, GCL_STYLE, GetClassLong(g_hwndClock, GCL_STYLE) & ~CS_DBLCLKS);
	
	CreateTip(g_hwndClock); // Create Mouse-Over ToolTip Window & Contents
	
	DragAcceptFiles(hwnd, GetMyRegLong(NULL, "DropFiles", FALSE)); // Enable/Disable DropFiles on Clock Based on Reg Info.
	
//	SetLayeredTaskbar(g_hwndClock,0); // transparent taskbar & more?
//	PostMessage(GetParent(GetParent(g_hwndClock)), WM_SIZE, SIZE_RESTORED, 0);
//	InvalidateRect(GetParent(GetParent(g_hwndClock)), NULL, TRUE);
	
	// somehow required... first size is smaller... taskbar refresh is also required to update clock position
	// delaying the call to ReadData here or adding a second later doesn't seem to help...
	PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
	PostMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
}
//================================================================================================
//----------------------------------+++--> End Clock Procedure (WndProc) - (Before?) Removing Hook:
void EndClock(void)   //--------------------------------------------------------------------+++-->
{
	DragAcceptFiles(g_hwndClock, FALSE);
	DestroyTip();
	SubsDestroy();
	if(m_multiClockClass){
		UnregisterClass((LPCSTR)m_multiClockClass,0);
		m_multiClockClass=0;
	}
	DestroyClock();
	EndNewAPI(g_hwndClock);
	if(g_hwndClock && IsWindow(g_hwndClock)) {
		if(m_bTimer) KillTimer(g_hwndClock, 1); m_bTimer = FALSE;
		SetWindowLongPtr(g_hwndClock, GWL_WNDPROC, (LONG_PTR)m_oldClockProc);
		m_oldClockProc=m_oldWorkerProc=NULL;
	}
	
	if(IsWindow(g_hwndTClockMain)) PostMessage(g_hwndTClockMain, MAINM_EXIT, 0, 0);
//  bClockUseTrans = FALSE;
}
/*------------------------------------------------
  subclass procedure of the clock
--------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
	case WM_DESTROY:
		DestroyClock();
		break;
	case(WM_USER+100):// send by windows to get clock size
		if(m_bNoClock) break;
		if(!(GetWindowLongPtr(hwnd,GWL_STYLE)&WS_VISIBLE))
			return 0;
		return ((LRESULT)m_rcClock.bottom << 16) | (LRESULT)m_rcClock.right;
	case WM_WINDOWPOSCHANGING:{
		WINDOWPOS* pwp=(WINDOWPOS*)lParam;
		if(m_bNoClock) break;
		if(IsWindowVisible(hwnd)){
			if(!(pwp->flags&SWP_NOSIZE) && !(pwp->flags&SWP_NOMOVE)){
				if(pwp->x >= pwp->y){ // horizontal
					pwp->cx=m_rcClock.right;
					if(m_height){ // explorer auto-updates our height, but we use custom
						pwp->cy=m_rcClock.bottom;
					}
				}else{
					pwp->cy=m_rcClock.bottom;
					if(m_width){ // explorer auto-updates our width, but we use custom
						pwp->cx=m_rcClock.right;
					}
				}
			}
		}
		return 0;}
	case WM_WINDOWPOSCHANGED:{
		WINDOWPOS* pwp=(WINDOWPOS*)lParam;
		if(m_bNoClock) break;
		if(!(pwp->flags&SWP_NOSIZE)){
			UpdateClockSize(hwnd);
			pwp->cx=m_rcClock.right;
			pwp->cy=m_rcClock.bottom;
		}
		return 0;}
	case WM_DWMCOLORIZATIONCOLORCHANGED://forwarded by T-Clock itself
		OnTColor_DWMCOLORIZATIONCOLORCHANGED((unsigned)wParam);
	case WM_THEMECHANGED:
		if(message==WM_THEMECHANGED)
			ReloadXPClockTheme(hwnd);
	case WM_SYSCOLORCHANGE:
		g_col_update(m_basecolorFont,m_basecolorBG);
		break;
	case WM_TIMECHANGE:
	case(WM_USER+101): {
		HDC hdc;
		if(m_bNoClock) break;
		hdc = GetDC(hwnd);
		DrawClock(hdc);
		ReleaseDC(hwnd, hdc);
		return 0;}
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc;
		if(m_bNoClock) break;
		hdc=BeginPaint(hwnd,&ps);
		DrawClock(hdc);
		EndPaint(hwnd,&ps);
//		InvalidateRect(hwnd,NULL,0); /// uncomment for debugging purpose, eg. does our drawing flicker
		return 0;}
	case WM_TIMER:
		if(wParam == 1)
			OnTimer(hwnd);
		else {
			if(m_bNoClock) break;
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		SetForegroundWindow(g_hwndTClockMain); // set T-Clock to foreground so we can open menus, etc.
		if(m_BlinkState){
			m_BlinkState=BLINK_NONE;
			InvalidateRect(hwnd, NULL, 1);
			PostMessage(g_hwndTClockMain,MAINM_BLINKOFF,0,0);
			return 0;
		}
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		if(message == WM_RBUTTONUP) break;
		return 0;
	case WM_MOUSEMOVE:
		if(!m_TipState){
			TRACKMOUSEEVENT tme;
			tme.cbSize=sizeof(TRACKMOUSEEVENT);
			tme.dwFlags=TME_HOVER|TME_LEAVE;
			tme.hwndTrack=hwnd;
			tme.dwHoverTime=HOVER_DEFAULT;
			m_TipState=1;
			TrackMouseEvent(&tme);
			FillClockBGHover(hwnd);
			InvalidateRect(hwnd,NULL,0);
		}
		return 0;
	case WM_MOUSEHOVER:
		m_TipState=2;
		if(g_tos<TOS_VISTA || GetMyRegLong("Tooltip","bCustom",0)){
			ShowTip(hwnd);//show custom tooltip
		}else{
			PostMessage(g_hwndClock, WM_USER+103,1,0);//show system tooltip
		}
		return 0;
	case WM_MOUSELEAVE:
		if(m_TipState){
			if(m_TipState==2){
				if(g_tos<TOS_VISTA || GetMyRegLong("Tooltip","bCustom",0))
					PostMessage(m_TipHwnd, TTM_TRACKACTIVATE , FALSE, (LPARAM)&m_TipInfo);//hide custom tooltip
				else
					PostMessage(g_hwndClock, WM_USER+103,0,0);//hide system tooltip
			}
			FillClockBG(hwnd);
			InvalidateRect(hwnd,NULL,0);
		}
		m_TipState=0;
		return 0;
	case WM_CONTEXTMENU:
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		return 0;
	case WM_NCHITTEST: // original clock uses this message for context menu and hover, etc. and we need our own "handler"
//		return HTCAPTION; // xD
		return DefWindowProc(hwnd, message, wParam, lParam);
	case WM_MOUSEACTIVATE:
		return MA_ACTIVATE;
	case WM_DROPFILES:
		PostMessage(g_hwndTClockMain, WM_DROPFILES, wParam, lParam);
		return 0;
	case WM_NOTIFY: {
		UINT code=((LPNMHDR)lParam)->code;
		if(code==TTN_NEEDTEXT || code==TTN_NEEDTEXTW)
			OnTooltipNeedText(code,lParam);
		return 0;}
	case WM_COMMAND:
		if(LOWORD(wParam) == IDM_EXIT) EndClock();
		return 0;
	case CLOCKM_REFRESHCLOCK: { // refresh the clock
		BOOL b;
		SubsDestroy();
		ReadData(hwnd,0); // also creates/updates clock
		b = GetMyRegLong(NULL, "DropFiles", FALSE);
		DragAcceptFiles(hwnd, b);
		InvalidateRect(hwnd, NULL, 0);
		InvalidateRect(GetParent(g_hwndClock), NULL, 1);
		SubsCreate();
		SubsSendResize();
		return 0;
		}
	case CLOCKM_REFRESHCLOCKPREVIEW: // refresh the clock
		ReadData(hwnd,1); // also creates/updates clock
		InvalidateRect(hwnd, NULL, 0);
		InvalidateRect(GetParent(g_hwndClock), NULL, 1);
	case CLOCKM_REFRESHTASKBAR: // refresh other elements than clock (somehow required to actually change the clock's size)
		SetLayeredTaskbar(g_hwndClock,0);
		PostMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(GetParent(GetParent(g_hwndClock)), NULL, 1);
		return 0;
	case CLOCKM_BLINK: // blink the clock
		if(wParam)
			m_BlinkState|=BLINK_HOUR;
		else
			m_BlinkState|=BLINK_ON;;
		InvalidateRect(hwnd, NULL, 0);
		return 0;
	case CLOCKM_COPY: // copy format to clipboard
		OnCopy(hwnd, lParam);
		return 0;
	case CLOCKM_REFRESHCLEARTASKBAR: {
		SetLayeredTaskbar(g_hwndClock,1);
		return 0;}
	}
	return CallWindowProc(m_oldClockProc, hwnd, message, wParam, lParam);
}
/*------------------------------------------------
  subclass procedure of the 2nd+ taskbar clock (used to handle clicks, etc.)
--------------------------------------------------*/
LRESULT CALLBACK WndProcMultiClock(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
	case WM_NCCREATE:{
		return 1;
	}
	case WM_CREATE:{
		return 0;
	}
	case WM_CLOSE:{
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_WINDOWPOSCHANGING:{
		return 0;}
	case WM_WINDOWPOSCHANGED:
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:{
		PAINTSTRUCT ps;
		BeginPaint(hwnd,&ps);
//		BitBlt(BeginPaint(hwnd,&ps),0,0,g_rcClock.right,g_rcClock.bottom,g_hdcClock,0,0,SRCCOPY);
		EndPaint(hwnd,&ps);
		return 0;}
	/// clock features
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		if(m_BlinkState){
			m_BlinkState=BLINK_NONE;
			InvalidateRect(g_hwndClock, NULL, 1);
			return 0;
		}
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		if(message == WM_RBUTTONUP) break;
		return 0;
	case WM_MOUSEMOVE:
		if(!m_TipState){
			TRACKMOUSEEVENT tme;
			tme.cbSize=sizeof(TRACKMOUSEEVENT);
			tme.dwFlags=TME_HOVER|TME_LEAVE;
			tme.hwndTrack=hwnd;
			tme.dwHoverTime=HOVER_DEFAULT;
			m_TipState=1;
			TrackMouseEvent(&tme);
			FillClockBGHover(g_hwndClock);
			InvalidateRect(g_hwndClock,NULL,0);
		}
		return 0;
	case WM_MOUSEHOVER:
		m_TipState=2;
		if(g_tos<TOS_VISTA || GetMyRegLong("Tooltip","bCustom",0)){
			ShowTip(hwnd);//show custom tooltip
		}else{
			SendMessage(g_hwndClock, WM_USER+103,1,0);//show system tooltip
		}
		return 0;
	case WM_MOUSELEAVE:
		if(m_TipState){
			if(m_TipState==2){
				if(g_tos<TOS_VISTA || GetMyRegLong("Tooltip","bCustom",0))
					PostMessage(m_TipHwnd, TTM_TRACKACTIVATE , FALSE, (LPARAM)&m_TipInfo);//hide custom tooltip
				else
					PostMessage(g_hwndClock, WM_USER+103,0,0);//hide system tooltip
			}
			FillClockBG(g_hwndClock);
			InvalidateRect(g_hwndClock,NULL,0);
		}
		m_TipState=0;
		return 0;
	case WM_CONTEXTMENU:
		PostMessage(g_hwndTClockMain, message, wParam, lParam);
		return 0;
	case WM_DROPFILES:
		PostMessage(g_hwndTClockMain, WM_DROPFILES, wParam, lParam);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}
/*------------------------------------------------
  subclass procedure of the 2nd+ taskbar "worker" area (used to resize and allow own clock)
--------------------------------------------------*/
#define SHOW_DESKTOP_BUTTONSIZE 10
LRESULT CALLBACK WndProcMultiClockWorker(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
	case WM_WINDOWPOSCHANGING:{
		WINDOWPOS* pwp=(WINDOWPOS*)lParam;
		int i;
		if(m_bNoClock) break;
		for(i=0; i<m_multiClocks; ++i){
			if(m_multiClock[i].worker==hwnd){
//				MessageBox(0,"WM_WINDOWPOSCHANGING",__FUNCTION__,0);
				if(!(pwp->flags&SWP_NOSIZE)){
					if(!pwp->flags && !pwp->x && !pwp->y && pwp->cx==10 && pwp->cy==10){ // special case for us
//						MessageBox(0,"special",__FUNCTION__,0);
						if(!m_multiClock[i].workerRECT.left && !m_multiClock[i].workerRECT.top){
							ClientToScreen(m_multiClock[i].worker,(POINT*)&m_multiClock[i].workerRECT);
							ScreenToClient(GetParent(m_multiClock[i].worker),(POINT*)&m_multiClock[i].workerRECT);
						}
						pwp->flags=SWP_NOMOVE;
						pwp->cx=m_multiClock[i].workerRECT.right;
						pwp->cy=m_multiClock[i].workerRECT.bottom;
					}
					CallWindowProc(m_oldWorkerProc,hwnd,message,wParam,lParam); // adjusts left margin? (so changes size and pos)
					m_multiClock[i].workerRECT.right=pwp->cx;
					m_multiClock[i].workerRECT.bottom=pwp->cy;
					if(pwp->cx > pwp->cy){ // horizontal
						pwp->cx=pwp->cx-m_rcClock.right-SHOW_DESKTOP_BUTTONSIZE;
					}else{
						pwp->cy=pwp->cy-m_rcClock.bottom-SHOW_DESKTOP_BUTTONSIZE;
					}
				}
				if(!(pwp->flags&SWP_NOMOVE)){
					m_multiClock[i].workerRECT.left=pwp->x;
					m_multiClock[i].workerRECT.top=pwp->y;
				}
				{int x,y,cx,cy;
				cx=m_rcClock.right;
				cy=m_rcClock.bottom;
				if(m_multiClock[i].workerRECT.right > m_multiClock[i].workerRECT.bottom){ // horizontal
					x=m_multiClock[i].workerRECT.left+m_multiClock[i].workerRECT.right-m_rcClock.right-SHOW_DESKTOP_BUTTONSIZE;
					y=m_multiClock[i].workerRECT.top+CLOCK_BORDER_MARGIN;
				}else{
					x=m_multiClock[i].workerRECT.left+CLOCK_BORDER_MARGIN;
					y=m_multiClock[i].workerRECT.top+m_multiClock[i].workerRECT.bottom-m_rcClock.bottom-SHOW_DESKTOP_BUTTONSIZE;
				}
				SetWindowPos(m_multiClock[i].clock,0,x,y,cx,cy,0);}
				break;
			}
		}
		return 0;}
	case WM_WINDOWPOSCHANGED:{
		break;}
	}
	return CallWindowProc(m_oldWorkerProc,hwnd,message,wParam,lParam);
}
//================================================================================================
//---------------------------------+++--> Retreive T-Clock Configuration Information From Registry:
void ReadData(HWND hwnd, BOOL preview)   //---------------------------------------------------------------+++-->
{
	const char* section=preview?"Preview":"Clock";
	char fontname[80];
	LONG weight, italic;
	int fontsize;
	int angle;
	BYTE fontquality;
	DWORD dwInfoFormat;
	SYSTEMTIME lt;
	HFONT hFon;
	
	m_basecolorFont = GetMyRegLong(section, "ForeColor", TCOLOR(TCOLOR_DEFAULT));
	m_basecolorBG = GetMyRegLong(section, "BackColor", TCOLOR(TCOLOR_DEFAULT));
	g_col_update(m_basecolorFont,m_basecolorBG);
	
	GetMyRegStr(section, "Font", fontname, 80, "Arial");
	
	fontsize = GetMyRegLong(section, "FontSize", 9);
	if(fontsize>100 || fontsize<=0) fontsize=9;
	italic = GetMyRegLong(section, "Italic", 0);
	weight = GetMyRegLong(section, "Bold", 0);
	switch(weight){
	case 0: break;
	case 1: weight=FW_BOLD; break;
	default:
		weight=FW_SEMIBOLD;
	}
	fontquality=(BYTE)GetMyRegLong(section, "FontQuality", CLEARTYPE_QUALITY);
	
	angle=GetMyRegLong(section,"Angle",0)%360;
	if(angle<0) angle+=360;
	
	m_radian=(double)angle*3.14159265358979323/180.;// ye π doesn't need to be that long :P
	
	dlineheight = GetMyRegLong(section, "LineHeight", 0);
	m_height = GetMyRegLong(section, "ClockHeight", 0);
	m_width = GetMyRegLong(section, "ClockWidth", 0);
	dhpos = GetMyRegLong(section, "HorizPos", 0);
	dvpos = GetMyRegLong(section, "VertPos", 0);
	
	m_bNoClock = (char)GetMyRegLong(section, "NoClockCustomize", FALSE);
	
	if(!GetMyRegStr("Format", "Format", m_format, sizeof(m_format), "") || !m_format[0]) {
		m_bNoClock = TRUE;
	}
	
	dwInfoFormat = FindFormat(m_format);
	m_bDispSecond = (dwInfoFormat&FORMAT_SECOND)? TRUE:FALSE;
	m_nDispBeat = dwInfoFormat & (FORMAT_BEAT1 | FORMAT_BEAT2);
	if(!m_bTimer) SetTimer(g_hwndClock, 1, 1000, NULL);
	m_bTimer = TRUE;
	
	g_bHour12 = (char)GetMyRegLong("Format", "Hour12", FALSE);
	g_bHourZero = (char)GetMyRegLong("Format", "HourZero", 0);
	
	GetLocalTime(&lt);
	m_LastTime.wDay = lt.wDay;
	
	InitFormat(&lt);      // format.c
	
	m_bMultimon = GetMyRegLong("Desktop", "Multimon", 1);

	hFon = CreateMyFont(fontname, fontsize, weight, italic, angle*10, fontquality);
	UpdateClock(hwnd,hFon);
}

/*------------------------------------------------
   get date/time and beat to display
--------------------------------------------------*/
void GetDisplayTime(SYSTEMTIME* pt, int* beat100)
{
	if(beat100) {
		GetSystemTime(pt);
		if(++pt->wHour>23)
			pt->wHour=0;
		*beat100 = pt->wHour*3600 + pt->wMinute*60 + pt->wSecond;
		*beat100 = (*beat100 * 1000) / 864;
	}
	GetLocalTime(pt);
}

/*--------------------------------------------------
------------------------------------------- WM_TIMER
--------------------------------------------------*/
void OnTimer(HWND hwnd)
{
	SYSTEMTIME t;
	int beat100=0;
	BOOL bRedraw;
	static char bCalibration=0;
	
	GetDisplayTime(&t, m_nDispBeat ? &beat100 : NULL);
	
	if(t.wMilliseconds > 200) {
		KillTimer(hwnd, 1);
		bCalibration = 1;
		SetTimer(hwnd, 1, 1001 - t.wMilliseconds, NULL);
	} else if(bCalibration) {
		KillTimer(hwnd, 1);
		bCalibration = 0;
		SetTimer(hwnd, 1, 1000, NULL);
	}
	
	if(CheckDaylightTimeTransition(&t)) {
		CallWindowProc(m_oldClockProc, hwnd, WM_TIMER, 0, 0);
		GetDisplayTime(&t, m_nDispBeat ? &beat100 : NULL);
	}
	
	bRedraw = FALSE;
	if(m_BlinkState){
		if(m_LastTime.wMinute && m_BlinkState&BLINK_HOUR)
			m_BlinkState^=BLINK_HOUR; // disable hourly blink
		bRedraw = TRUE;
		/* --+++--> This Will Disable the AutoHide...
				abd.cbSize = sizeof(APPBARDATA);
				abd.hWnd = FindWindow("Shell_TrayWnd","");
				abd.lParam = ABS_ALWAYSONTOP;
				SHAppBarMessage(ABM_SETSTATE, &abd); ...Which Ain't What We're After! <+-*/
		
	}
	
	else if(m_bDispSecond) bRedraw = TRUE;
	else if(m_nDispBeat == 1 && m_beatLast != (beat100/100)) bRedraw = TRUE;
	else if(m_nDispBeat == 2 && m_beatLast != beat100) bRedraw = TRUE;
//	else if(bDispSysInfo) bRedraw = TRUE;
	else if(m_LastTime.wHour != (int)t.wHour
			|| m_LastTime.wMinute != (int)t.wMinute) bRedraw = TRUE;
	
	if(m_LastTime.wDay != t.wDay || m_LastTime.wMonth != t.wMonth ||
	   m_LastTime.wYear != t.wYear) {
		InitFormat(&t); // format.c
		InitDaylightTimeTransition();
	}
	
	memcpy(&m_LastTime, &t, sizeof(t));
	
	if(m_nDispBeat == 1) m_beatLast = beat100/100;
	else if(m_nDispBeat > 1) m_beatLast = beat100;
	
	if(bRedraw)
		InvalidateRect(hwnd,NULL,0);
}

void FillClockBG()
{
	RGBQUAD* color;
	BYTE alpha;
	union{
		BGRQUAD quad;
		COLORREF ref;
	} col;
	if(!m_colorBG_end) return;
	if(m_colBG.ref==TCOLOR(TCOLOR_DEFAULT)){
		if(IsXPThemeActive()){
			DrawXPClockBackground(g_hwndClock,m_hdcClockBG,&m_rcClock);
			return;
		}
		col.ref=GetSysColor(COLOR_3DFACE);
	}else
		col.ref=m_colBG.ref;
	// fill with color (Win8 uses ~0x37 alpha)
//	col.ref=0x37000000;//Win8 like
	alpha=255-col.quad.rgbReserved;
	col.ref=alpha<<24|(col.quad.rgbBlue*alpha/255)|(col.quad.rgbGreen*alpha/255)<<8|(col.quad.rgbRed*alpha/255)<<16;
	for(color=m_colorBG_start; color<m_colorBG_end; ++color)
		*(unsigned*)color=col.ref;
}
void FillClockBGHover()
{
	if(!m_colorBG_end) return;
	if(IsXPThemeActive()) {
		DrawXPClockHover(g_hwndClock,m_hdcClockBG,&m_rcClock);
	}
}
void CalculateClockTextPosition(){
	double cos_=cos(m_radian);
	double sin_=sin(m_radian);
	int hleading=(int)(sin_*m_leading);
	int leading=(int)(cos_*m_leading);
	int textwidth=(int)(sin_*m_textheight);
	int textheight=(int)(cos_*m_textheight);
	/// use width / height based on angle.
	GetClientRect(GetParent(GetParent(g_hwndClock)),&m_rcClock);
	m_bHorizontalTaskbar=m_rcClock.right>m_rcClock.bottom;
	if(m_bHorizontalTaskbar){
		m_rcClock.bottom-=CLOCK_BORDER_MARGIN;//2px top
		if(m_height){//user-defined height
			m_rcClock.bottom=m_textheight;
		}else
			textheight+=CLOCK_BORDER_MARGIN; // ignore top margin on center calculation
		m_rcClock.right=abs((int)(cos_*m_textwidth))+abs((int)(sin_*m_textheight)) + m_textpadding;
	}else{
		m_rcClock.right-=CLOCK_BORDER_MARGIN;//2px top
		if(m_width){//user-defined height
			m_rcClock.right=m_textwidth;
		}else
			textwidth+=CLOCK_BORDER_MARGIN; // ignore left margin on center calculation
		m_rcClock.bottom=abs((int)(cos_*m_textheight))+abs((int)(sin_*m_textwidth)) + m_textpadding;
	}
	m_rcClock.right+=m_width;
	m_rcClock.bottom+=m_height;
	if(m_rcClock.right<5) m_rcClock.right=5;
	if(m_rcClock.bottom<5) m_rcClock.bottom=5;
	/// position
	m_vertpos=(m_rcClock.bottom-textheight-leading+(int)(cos_*dlineheight))/2 + dvpos;
	m_horizpos=(m_rcClock.right-textwidth-hleading+(int)(sin_*dlineheight))/2 + dhpos;
}
void CalculateClockTextSize(){
	SYSTEMTIME time;
	int beat100=0;
	char buf[1024], *pos, *str;
	unsigned len;
	SIZE sz;
	TEXTMETRIC tm;
	GetDisplayTime(&time, m_nDispBeat ? &beat100 : NULL);
	len=MakeFormat(buf, m_format, &time, beat100);
	GetTextMetrics(m_hdcClock,&tm);
	m_textpadding=tm.tmAveCharWidth*2;
	m_textheight=m_textwidth=0;
	m_leading=tm.tmInternalLeading;
	m_vertfeed=tm.tmHeight-m_leading+dlineheight;
	for(pos=buf; *pos; ){
		for(str=pos; *pos&&*pos!='\n'; ++pos);
		len=(unsigned)(pos-str);
		if(*pos=='\n') {*pos++='\0';}
		/// width
		if(GetTextExtentPoint32(m_hdcClock,str,len,&sz)==0)
			sz.cx=len*tm.tmAveCharWidth;
		if(m_textwidth<sz.cx)
			m_textwidth=sz.cx;
		///height
		m_textheight+=m_vertfeed;
	}
	m_horizfeed=(int)(sin(m_radian)*m_vertfeed);
	m_vertfeed=(int)(cos(m_radian)*m_vertfeed);
	CalculateClockTextPosition();
}
int DestroyClock()
{
	if(m_oldfnt){
		DeleteObject(SelectObject(m_hdcClock,m_oldfnt));
		m_oldfnt=NULL;
	}
	if(m_oldbmpB){
		DeleteObject(SelectObject(m_hdcClockBG,m_oldbmpB));
		m_oldbmpB=NULL;
		m_colorBG_start=NULL;
	}
	if(m_oldbmp){
		DeleteObject(SelectObject(m_hdcClock,m_oldbmp));
		m_oldbmp=NULL;
		m_color_start=NULL;
	}
	if(m_hdcClock){
		DeleteDC(m_hdcClockBG);
		DeleteDC(m_hdcClock);
		m_hdcClock=NULL;
	}
	memset(&m_rcClock,0,sizeof(m_rcClock));
	return 0;
}
int UpdateClock(HWND hwnd, HFONT fnt)
{
	if(m_bNoClock)
		return 0;
	if(!m_hdcClock){
		HDC hdc=GetDC(NULL);
		m_hdcClock=CreateCompatibleDC(hdc);
		m_hdcClockBG=CreateCompatibleDC(hdc);
		ReleaseDC(NULL,hdc);
		SetBkMode(m_hdcClock,TRANSPARENT);
		SetTextAlign(m_hdcClock,TA_CENTER|TA_TOP);
		SetTextColor(m_hdcClock,0x00000000);
	}
	if(!m_oldfnt)
		m_oldfnt=SelectObject(m_hdcClock,fnt);
	else
		DeleteObject(SelectObject(m_hdcClock,fnt));
	CalculateClockTextSize(); // height change only, bugs...
	SetWindowPos(hwnd,HWND_TOP,0,0,m_rcClock.right+1,m_rcClock.bottom,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE|SWP_NOCOPYBITS); // without doing this...
	return 1;
}
int UpdateClockSize(HWND hwnd)
{
	static BITMAPINFO bmi={{sizeof(BITMAPINFO),0,0,1,32,BI_RGB},};
	HBITMAP hbm;
	if(!m_hdcClock)
		return 0;
	bmi.bmiHeader.biWidth=m_rcClock.right;
	bmi.bmiHeader.biHeight=m_rcClock.bottom;
	/// create/select text bitmap
	hbm=CreateDIBSection(m_hdcClock,&bmi,DIB_RGB_COLORS,&m_color_start,NULL,0);
	if(!hbm) return DestroyClock();
	m_color_end=m_color_start+(m_rcClock.right*m_rcClock.bottom);
	if(!m_oldbmp) m_oldbmp=SelectObject(m_hdcClock,hbm);
	else DeleteObject(SelectObject(m_hdcClock,hbm));
	/// create/select background bitmap
	hbm=CreateDIBSection(m_hdcClockBG,&bmi,DIB_RGB_COLORS,&m_colorBG_start,NULL,0);
	if(!hbm) return DestroyClock();
	m_colorBG_end=m_colorBG_start+(m_rcClock.right*m_rcClock.bottom);
	if(!m_oldbmpB) m_oldbmpB=SelectObject(m_hdcClockBG,hbm);
	else DeleteObject(SelectObject(m_hdcClockBG,hbm));
	FillClockBG(hwnd);
	CalculateClockTextPosition();
	SubsSendResize();
	return 1;
}


/*------------------------------------------------
  draw the clock
--------------------------------------------------*/
void DrawClock(HDC hdc)
{
	SYSTEMTIME t;
	int beat100=0;
	if(!m_color_start)
		return;
	GetDisplayTime(&t, m_nDispBeat ? &beat100 : NULL);
	DrawClockSub(hdc, &t, beat100);
}
void DrawClockSub(HDC hdc, SYSTEMTIME* pt, int beat100)
{
	RGBQUAD* color,* back;
	char buf[1024],* pos,* str;
	unsigned len;
	int vpos,hpos;
	const unsigned opacity=255-m_col.quad.rgbReserved;
	for(color=m_color_start; color<m_color_end; ++color)
		*(unsigned*)color=0xFFFFFFFF;
	len=MakeFormat(buf, m_format, pt, beat100);
	
	vpos=m_vertpos;
	hpos=m_horizpos;
	for(pos=buf; *pos; ){
		for(str=pos; *pos&&*pos!='\n'; ++pos);
		len=(unsigned)(pos-str);
		if(*pos=='\n') {*pos++='\0';}
		ExtTextOut(m_hdcClock, hpos, vpos, 0, NULL, str, len, NULL);
		vpos+=m_vertfeed;
		hpos+=m_horizfeed;
	}
	GdiFlush();//flush before bit manipulation
	for(color=m_color_start,back=m_colorBG_start; color<m_color_end; ++color,++back){
		if(!color->rgbReserved){
			unsigned channel;
			unsigned trans=(255-(color->rgbGreen+color->rgbBlue+color->rgbRed)/3)*opacity/255;
			unsigned bgtrans=255-trans;
			channel=m_col.quad.rgbBlue*trans/0xE7 + back->rgbBlue*bgtrans/255;
			color->rgbBlue=(channel>255?255:(BYTE)channel);
			channel=m_col.quad.rgbGreen*trans/0xE7 + back->rgbGreen*bgtrans/255;
			color->rgbGreen=(channel>255?255:(BYTE)channel);
			channel=m_col.quad.rgbRed*trans/0xE7 + back->rgbRed*bgtrans/255;
			color->rgbRed=(channel>255?255:(BYTE)channel);
			
			channel=back->rgbReserved+trans;
			color->rgbReserved=(channel>255?255:(BYTE)channel);
		}else{
			*(unsigned*)color=*(unsigned*)back;
		}
	}
	if(!m_BlinkState || pt->wSecond%2){
//		BLENDFUNCTION fnc={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
//		GdiAlphaBlend(hdc,0,0,g_rcClock.right,g_rcClock.bottom,g_hdcClock,0,0,g_rcClock.right,g_rcClock.bottom,fnc);
//		BitBlt(hdc,0,0,g_rcClock.right,g_rcClock.bottom,g_hdcClock,0,0,SRCPAINT);
		BitBlt(hdc,0,0,m_rcClock.right,m_rcClock.bottom,m_hdcClock,0,0,SRCCOPY);
		if(m_multiClocks)
			BitBlt(m_multiClockDC,0,0,m_rcClock.right,m_rcClock.bottom,m_hdcClock,0,0,SRCCOPY);
	}else{
		BitBlt(hdc,0,0,m_rcClock.right,m_rcClock.bottom,m_hdcClock,0,0,NOTSRCCOPY);
		if(m_multiClocks)
			BitBlt(m_multiClockDC,0,0,m_rcClock.right,m_rcClock.bottom,m_hdcClock,0,0,NOTSRCCOPY);
	}
}

void OnTooltipNeedText(UINT code, LPARAM lParam)
{
	SYSTEMTIME t;
	int beat100;
	char fmt[1024], s[1024];
	
	GetMyRegStr("Tooltip", "Tooltip", fmt, 1024, "");
	if(!*fmt) strcpy(fmt, "\"TClock\" LDATE");
	
	GetDisplayTime(&t, &beat100);
	MakeFormat(s, fmt, &t, beat100);
	
	if(code == TTN_NEEDTEXT) strcpy(((LPTOOLTIPTEXT)lParam)->szText, s);
	else {
		MultiByteToWideChar(CP_ACP, 0, s, -1, ((LPTOOLTIPTEXTW)lParam)->szText, 80);
	}
}

/*--------------------------------------------------
------------------- copy date/time text to clipboard
--------------------------------------------------*/
void OnCopy(HWND hwnd, LPARAM lParam)
{
	SYSTEMTIME t;	HGLOBAL hg;
	char entry[7], fmt[256], s[1024], *pbuf;
	int beat100;
	
	GetDisplayTime(&t, &beat100);
	entry[0]='0'+(char)LOWORD(lParam);
	entry[1]='0'+(char)HIWORD(lParam);
	memcpy(entry+2,"Clip",5);
	GetMyRegStr("Mouse",entry,fmt,256,"");
	if(!*fmt) strcpy(fmt,m_format);
	
	MakeFormat(s, fmt, &t, beat100);
	
	if(!OpenClipboard(hwnd)) return;
	EmptyClipboard();
	hg = GlobalAlloc(GMEM_DDESHARE, strlen(s) + 1);
	pbuf = (char*)GlobalLock(hg);
	strcpy(pbuf, s);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
}
//============================================================================================
//	char szTZone[32] = {0}; //---+++--> TimeZone String Buffer, Also Used (as External) in Format.c
//==============================================================================================
int iHourTransition = -1, iMinuteTransition = -1; //--++--> Used Only in the Two Functions Below!
//================================================================================================
//-----------------------------+++--> Initialize Clock With the User's Local Time-Zone Information:
void InitDaylightTimeTransition(void)   //--------------------------------------------------+++-->
{
	TIME_ZONE_INFORMATION tzi;
	SYSTEMTIME lt, *plt=NULL;
	DWORD dw;
	
	iHourTransition = iMinuteTransition = -1;
	
	GetLocalTime(&lt);
	
	memset(&tzi, 0, sizeof(tzi));
	dw = GetTimeZoneInformation(&tzi);
	if(dw == TIME_ZONE_ID_STANDARD // This Will Only Apply in the Fall/Winter Months When DST is NOT in Effect.
	   && tzi.DaylightDate.wMonth == lt.wMonth
	   && tzi.DaylightDate.wDayOfWeek == lt.wDayOfWeek) {
		plt = &(tzi.DaylightDate);
//		strcpy(szTZone, (char *)tzi.StandardName);
//		wcstombs(szTZone, tzi.StandardName, 32);
//		wsprintf(szTZone, "%S", tzi.StandardName);
	}
	if(dw == TIME_ZONE_ID_DAYLIGHT // This Will Only Apply in the Spring/Summer Months When DST IS in Effect.
	   && tzi.StandardDate.wMonth == lt.wMonth
	   && tzi.StandardDate.wDayOfWeek == lt.wDayOfWeek) {
		plt = &(tzi.StandardDate);
//		strcpy(szTZone, tzi.DaylightName);
//		wcstombs(szTZone, tzi.DaylightName, 32);
//		wsprintf(szTZone, "%S", tzi.DaylightName);
	}
	
	if(plt && plt->wDay < 5) {
		if(((lt.wDay - 1) / 7 + 1) == plt->wDay) {
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	} else if(plt && plt->wDay == 5) {
		FILETIME ft;
		DWORDLONG ftqw;
		SystemTimeToFileTime(&lt,&ft);
//		*(DWORDLONG*)&ft += 6048000000000ULL;// it's unsave :(
		ftqw=(((DWORDLONG)ft.dwHighDateTime<<32) | ft.dwLowDateTime) + 6048000000000ULL;
		ft.dwLowDateTime=ftqw&0xFFFFFFFF;
		ft.dwHighDateTime=ftqw>>32;
		FileTimeToSystemTime(&ft, &lt);
		if(lt.wDay < 8) {
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	}
//	wsprintf(szTZone, "Day: %S, Std: %S", tzi.DaylightName, TEXT(tzi.StandardName));
//		MessageBox(0, szTZone, "is TimeZone??", MB_OK);
}
//================================================================================================
//-+++--> iHourTransition & iMinuteTransition Are Now Used to Pass Local Time-Zone Offset to Clock:
BOOL CheckDaylightTimeTransition(SYSTEMTIME* plt)   //--------------------------------------+++-->
{
	if((int)plt->wHour == iHourTransition &&
	   (int)plt->wMinute >= iMinuteTransition) {
		iHourTransition = iMinuteTransition = -1;
		return TRUE;
	} else return FALSE;
}
