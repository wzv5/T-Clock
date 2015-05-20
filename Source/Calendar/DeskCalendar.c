#include "../common/globals.h"
#include <CommCtrl.h>
#include <time.h>
#include "resource.h"
//other
TClockAPI api;
BOOL m_bAutoClose;
BOOL m_bTopMost;
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
	
	if(api.OS < TOS_VISTA && ivMonths==1) {
		wsprintf(szTitle, "Calendar:  Day: %s", szDoY);
	} else {
		wsprintf(szTitle, "T-Clock: Calendar  Day: %s", szDoY);
	}
}
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
	case WM_CREATE:{
		int iMonths,iMonthsPast;
		DWORD dwCalStyle;
		RECT rc; HWND hCal;
		iMonths=api.GetIntEx("Calendar","ViewMonths",3);
		iMonthsPast=api.GetIntEx("Calendar","ViewMonthsPast",1);
		
		if(api.GetInt("Calendar", "ShowDayOfYear", 1)) {
			char szTitle[32];
			GetDayOfYearTitle(szTitle,iMonths);
			SetWindowText(hwnd,szTitle);
		}
		
		dwCalStyle=WS_BORDER|WS_CHILD|WS_VISIBLE|MCS_NOTODAYCIRCLE;
		if(api.GetInt("Calendar","ShowWeekNums",0))
			dwCalStyle|=MCS_WEEKNUMBERS;
		
		hCal=CreateWindowEx(0,MONTHCAL_CLASS,"",dwCalStyle,0,0,0,0,hwnd,NULL,NULL,NULL);
		if(!hCal) return -1;
		
		MonthCal_GetMinReqRect(hCal,&rc);//size for a single month
		if(iMonths>1){
			#define CALBORDER 4
			#define CALTODAYTEXTHEIGHT 13
			rc.right+=CALBORDER;
			rc.bottom-=CALTODAYTEXTHEIGHT;
			switch(iMonths){
			/*case 1:*/ case 2: case 3:
			case 4: case 5:
				rc.right*=iMonths;
				break;
			case 6:
				rc.bottom*=3;
				/* fall through */
			case 7: case 8:
				rc.right*=2;
				if(iMonths!=6) rc.bottom*=4;
				break;
			case 9:
				rc.bottom*=3;
			case 11: case 12:
				rc.right*=3;
				if(iMonths!=9) rc.bottom*=4;
				break;
			case 10:
				rc.right*=5;
				rc.bottom*=2;
				break;
			}
			rc.right-=CALBORDER;
			rc.bottom+=CALTODAYTEXTHEIGHT;
			if(api.OS >= TOS_VISTA)
				MonthCal_SizeRectToMin(hCal,&rc);//removes some empty space.. (eg at 4 months)
		}else{
			if(rc.right<(LONG)MonthCal_GetMaxTodayWidth(hCal))
				rc.right=MonthCal_GetMaxTodayWidth(hCal);
		}
		SetWindowPos(hCal,HWND_TOP,0,0,rc.right,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
		if(iMonthsPast){
			SYSTEMTIME st,stnew; MonthCal_GetCurSel(hCal,&st); stnew=st;
			stnew.wDay=1;
			if(iMonthsPast>=stnew.wMonth){ --stnew.wYear; stnew.wMonth+=12; }
			stnew.wMonth-=(short)iMonthsPast;
			if(stnew.wMonth>12){ ++stnew.wYear; stnew.wMonth-=12; }  // in case iMonthsPast is negative
			MonthCal_SetCurSel(hCal,&stnew);
			MonthCal_SetCurSel(hCal,&st);
		}
		SetFocus(hCal);
		AdjustWindowRectEx(&rc,WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE,FALSE,0);
		SetWindowPos(hwnd,HWND_TOP,0,0, rc.right-rc.left,rc.bottom-rc.top, SWP_NOMOVE);//force to be on top
		if(m_bTopMost)
			SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		api.PositionWindow(hwnd, 11);
		if(m_bAutoClose && GetForegroundWindow()!=hwnd)
			PostMessage(hwnd,WM_CLOSE,0,0);
		return 0;}
	case WM_ACTIVATE:
		if(!m_bTopMost){
			if(LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE){
				SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			}else{
				SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				SetWindowPos((HWND)wParam,hwnd,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			}
		}
		if(m_bAutoClose){
			if(LOWORD(wParam)!=WA_ACTIVE && LOWORD(wParam)!=WA_CLICKACTIVE){
				PostMessage(hwnd,WM_CLOSE,0,0);//adds a little more delay which is good
			}
		}
		break;
	case WM_DESTROY:
		Sleep(50);//we needed more delay... 50ms looks good (to allow T-Clock's FindWindow to work) 100ms is slower then Vista's native calendar
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//==========================================================================
//---------------------------------------------------+++--> Open "Calendar":
HWND CreateCalender(HINSTANCE hInstance,HWND hwnd)   //---------------+++-->
{
	INITCOMMONCONTROLSEX icex;
	WNDCLASSEX wcx;
	ATOM calclass;
	m_bAutoClose = api.GetInt("Calendar", "CloseCalendar", 1);
	m_bTopMost = api.GetInt("Calendar", "CalendarTopMost", 0);
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
	hwnd=CreateWindowEx(0,MAKEINTATOM(calclass),"T-Clock: Calendar",WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE,0,0,0,0,hwnd,0,hInstance,NULL);
	return hwnd;
}

//int main(int argc,char* argv[])
int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	BOOL bRet;
	(void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;// don't warn me about they being unused
	if(LoadClockAPI("T-Clock" ARCH_SUFFIX, &api))
		return 2;
	if(!CreateCalender(hInstance,0))
		return 1;
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
