/*----------------------------------------------------------------------
// deskcal.c : Update Desktop Calendar automatically -> KAZUBON 1997-1999
//---------------------------------------------------------------------*/
// Last Modified by Stoic Joker: Saturday, 06/05/2010 @ 3:48:15pm
//#include "../Clock/tclock.h"
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
//#include <WinUser.h>
#include <time.h>
//utl.c
LONG GetMyRegLongEx(char* section, char* entry, LONG defval);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
//void ForceForegroundWindow(HWND hwnd);
//other
BOOL bAutoClose;
BOOL bV7up = FALSE;
//BOOL b2000 = FALSE;
//================================================================================================
//------------------------------+++--> Adjust the Window Position Based on Taskbar Size & Location:
void SetMyDialgPos(HWND hwnd)   //----------------------------------------------------------+++-->
{
	#define PADDING 12
	int wscreen, hscreen, wProp, hProp;
	int wTray, hTray, x, y;
	RECT rc;
	HWND hwndTray;
	
	GetWindowRect(hwnd, &rc); // Properties Dialog Dimensions
	wProp = rc.right - rc.left;  //----------+++--> Width
	hProp = rc.bottom - rc.top; //----------+++--> Height
	
	wscreen = GetSystemMetrics(SM_CXSCREEN);  // Desktop Width
	hscreen = GetSystemMetrics(SM_CYSCREEN); // Desktop Height
	
	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(!hwndTray) return;
	
	GetWindowRect(hwndTray, &rc);
	wTray = rc.right - rc.left;
	hTray = rc.bottom - rc.top;
	
	if(wTray > hTray) { // IF Width is Greater Than Height, Taskbar is
		x = wscreen - wProp - PADDING; // at Either Top or Bottom of Screen
		if(rc.top < hscreen / 2)
			y = rc.bottom + PADDING; // Taskbar is on Top of Screen
		else // ELSE Taskbar is Where it Belongs! (^^^Mac Fag?^^^)
			y = rc.top - hProp - PADDING;
		if(y < 0) y = 0;
	} else { //---+++--> ELSE Taskbar is on Left or Right Side of Screen
		y = hscreen - hProp - PADDING; // Down is a Fixed Position
		if(rc.left < wscreen / 2)
			x = rc.right + PADDING; //--+++--> Taskbar is on Left Side of Screen
		else
			x = rc.left - wProp - PADDING; // Taskbar is on Right Side of Screen
		if(x < 0) x = 0;
	}
	MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
}
//===========================================================================================
//----------------------------------------------------+++--> Get the Current Day of the Year:
void GetDayOfYearTitle(char* szTitle, int ivMonths)   //-------------------------------+++-->
{
	struct tm today;
	char szDoY[8];
	time_t ltime;
	
	time(&ltime);
	_localtime64_s(&today, &ltime);
//  strftime(szDoY, 8, "%#j", &today); // <--{OutPut}--> Day 95
	strftime(szDoY, 8, "%j", &today);   // <--{OutPut}--> Day 095
	
	if(!bV7up && ivMonths==1) {
		wsprintf(szTitle, "Calendar:  Day: %s", szDoY);
	} else {
		wsprintf(szTitle, "T-Clock: Calendar  Day: %s", szDoY);
	}
}
//#include <stddef.h>
//void ForceForegroundWindow(HWND hwnd)
//{
//	DWORD fgthread=GetWindowThreadProcessId(GetForegroundWindow(),0);
//	if(fgthread && _threadid^fgthread && AttachThreadInput(_threadid,fgthread,1)){
//		SetForegroundWindow(hwnd);
//		AttachThreadInput(_threadid,fgthread,0);
//		return;
//	}
//	SetForegroundWindow(hwnd);
//}
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
	case WM_CREATE:{
		int ivMonths;
		DWORD dwCalStyle;
		RECT rc; HWND hCal;
		ivMonths=GetMyRegLongEx("Calendar","ViewMonths",1);
		
		if(GetMyRegLong("Calendar", "ShowDayOfYear", FALSE)) {
			char szTitle[32];
			GetDayOfYearTitle(szTitle,ivMonths);
			SetWindowText(hwnd,szTitle);
		}
		
		dwCalStyle=WS_BORDER|WS_CHILD|WS_VISIBLE|MCS_NOTODAYCIRCLE;
		if(GetMyRegLong("Calendar","ShowWeekNums",FALSE))
			dwCalStyle|=MCS_WEEKNUMBERS;
		
		hCal=CreateWindowEx(0,MONTHCAL_CLASS,"",dwCalStyle,0,0,0,0,hwnd,NULL,NULL,NULL);
		if(!hCal) return -1;
		
		MonthCal_GetMinReqRect(hCal,&rc);//size for a single month
		if(ivMonths>1){
			rc.right*=ivMonths;
			if(ivMonths>5){//multi row
				rc.right/=3;
				if(ivMonths==6 || ivMonths==9){//6 or 9
					rc.bottom*=3;
				}else{//8 or 12
					rc.bottom*=4;
				}
			}
			MonthCal_SizeRectToMin(hCal,&rc);//removes some empty space.. (eg at 4 months)
		}else{
			if(rc.right<(LONG)MonthCal_GetMaxTodayWidth(hCal))
				rc.right=MonthCal_GetMaxTodayWidth(hCal);
		}
		SetWindowPos(hCal,NULL,0,0,rc.right,rc.bottom,SWP_NOZORDER);
		AdjustWindowRectEx(&rc,WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE,FALSE,0);
//		AdjustWindowRectEx(&rc,GetWindowLongPtr(hwnd,GWL_STYLE),GetMenu(hwnd)?TRUE:FALSE,GetWindowLongPtr(hwnd,GWL_EXSTYLE));
		SetWindowPos(hwnd,HWND_TOPMOST,0,0, rc.right-rc.left,rc.bottom-rc.top, SWP_NOMOVE);//force to be on top
		if(!bAutoClose && !GetMyRegLong("Calendar","CalendarTopMost",FALSE))
			SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetMyDialgPos(hwnd);
		return 0;}
//	case WM_ACTIVATE:
//		if(LOWORD(wParam)!=WA_INACTIVE) break;
//	case WM_ACTIVATEAPP:
//		if(uMsg==WM_ACTIVATEAPP && wParam) break;
	case WM_KILLFOCUS:
		if(bAutoClose)
			PostMessage(hwnd,WM_CLOSE,0,0);//adds a little more delay which is good
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 1;
	case WM_DESTROY:
		Sleep(50);//we needed more delay... 50ms looks good (to allow T-Clock's FindWindow to work) 100ms is slower then Vista's native calendar
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
//==========================================================================
//---------------------------------------------------+++--> Open "Calendar":
HWND CreateCalender(HINSTANCE hInstance,HWND hwnd)   //---------------+++-->
{
	INITCOMMONCONTROLSEX icex;
	WNDCLASSEX wcx;
	ATOM calclass;
	bAutoClose = GetMyRegLong("Calendar", "CloseCalendar", FALSE);
	icex.dwSize=sizeof(icex);
	icex.dwICC=ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);
	
	wcx.cbSize = sizeof(wcx);
	wcx.style =0;
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = NULL;
	wcx.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MAIN));
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "ClockFlyoutWindow";
	wcx.hIconSm=NULL;
	calclass=RegisterClassEx(&wcx);
	hwnd=CreateWindowEx(0,(LPCSTR)MAKELPARAM(calclass,0),"T-Clock: Calendar",WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE,0,0,0,0,hwnd,NULL,hInstance,NULL);
//	hwnd=CreateWindowEx(WS_EX_TOOLWINDOW, (LPCSTR)MAKELPARAM(calclass,0), "T-Clock: Calendar", WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE, 0,0,100,100, NULL, NULL, hInstance, NULL);
//	ForceForegroundWindow(hwnd);
	if(bAutoClose && GetForegroundWindow()!=hwnd)
		PostMessage(hwnd,WM_CLOSE,0,0);
	return hwnd;
}

//int main(int argc,char* argv[])
int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	HWND hiddenwnd;
	MSG msg;
	BOOL bRet;
	OSVERSIONINFOEX osvi;
	(void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;// don't warn me about they being unused
	memset(&osvi,0,sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*)&osvi)) return -1;
	if(osvi.dwMajorVersion>=6) bV7up=TRUE;
//	else if((osvi.dwMajorVersion==5) && (osvi.dwMinorVersion==0)) b2000=TRUE;
//	if(osvi.dwMajorVersion>=5) return TRUE;
	hiddenwnd=CreateWindow("Edit","",0,0,0,0,0,NULL,NULL,hInstance,NULL);
	if(!CreateCalender(hInstance,hiddenwnd)) return 1;
	while((bRet=GetMessage(&msg,NULL,0,0))!=0){
		if(bRet==-1){//handle the error and possibly exit
			break;
		}else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}
