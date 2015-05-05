//#include "../Clock/tclock.h"
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
//#include <WinUser.h>
#include <time.h>
#include "../common/utl.h"
//other
BOOL m_bAutoClose;
BOOL m_bTopMost;
//================================================================================================
//------------------------------+++--> Adjust the Window Position Based on Taskbar Size & Location:
void SetMyDialgPos(HWND hwnd)   //----------------------------------------------------------+++-->
{
	#define padding 11 // 11 is Win8 default
	MONITORINFO moni;
	int wProp, hProp;
	
	GetWindowRect(hwnd,&moni.rcWork); // Properties Dialog Dimensions
	wProp = moni.rcWork.right-moni.rcWork.left;  //----------+++--> Width
	hProp = moni.rcWork.bottom-moni.rcWork.top; //----------+++--> Height
	
	GetCursorPos((POINT*)&moni.rcWork);
	moni.cbSize=sizeof(moni);
	GetMonitorInfo(MonitorFromPoint(*(POINT*)&moni.rcWork,MONITOR_DEFAULTTONEAREST),&moni);
	
	if(moni.rcWork.top!=moni.rcMonitor.top || moni.rcWork.bottom!=moni.rcMonitor.bottom) { // taskbar is horizontal
		moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
		if(moni.rcWork.top!=moni.rcMonitor.top) // top
			moni.rcMonitor.top=moni.rcWork.top+padding;
		else // bottom
			moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
	}else if(moni.rcWork.left!=moni.rcMonitor.left || moni.rcWork.right!=moni.rcMonitor.right){ // vertical
		moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
		if(moni.rcWork.left!=moni.rcMonitor.left) // left
			moni.rcMonitor.left=moni.rcWork.left+padding;
		else // right
			moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
	}else{ // autohide taskbar
		MONITORINFO taskbarMoni;
		HWND taskbar=FindWindow("Shell_TrayWnd",NULL);
		RECT taskbarRC;
		int tbW,tbH;
		if(!taskbar)
			return;
		taskbarMoni.cbSize=sizeof(moni);
		GetMonitorInfo(MonitorFromWindow(taskbar,MONITOR_DEFAULTTONEAREST),&taskbarMoni); // correct monitor for single monitor setup, probably wrong for multimon
		GetWindowRect(taskbar,&taskbarRC);
		tbW=taskbarRC.right-taskbarRC.left;
		tbH=taskbarRC.bottom-taskbarRC.top;
		if(tbW > tbH){ // horizontal
			int diff=taskbarMoni.rcMonitor.top-taskbarRC.top;
			int visiblesize=taskbarMoni.rcMonitor.bottom-taskbarMoni.rcMonitor.top+diff;
			moni.rcMonitor.left=moni.rcWork.right-wProp-padding;
			if((diff>0 && diff>tbH/10) || !diff || (diff<0 && (visiblesize!=tbH && visiblesize>tbH/10))) // top
				moni.rcMonitor.top=moni.rcWork.top+padding+tbH;
			else // bottom
				moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding-tbH;
		}else{
			int diff=taskbarMoni.rcMonitor.left-taskbarRC.left;
			int visiblesize=taskbarMoni.rcMonitor.right-taskbarMoni.rcMonitor.left+diff;
			moni.rcMonitor.top=moni.rcWork.bottom-hProp-padding;
			if((diff>0 && diff>tbW/10) || !diff || (diff<0 && (visiblesize!=tbW && visiblesize>tbW/10))) // left
				moni.rcMonitor.left=moni.rcWork.left+padding+tbW;
			else // right
				moni.rcMonitor.left=moni.rcWork.right-wProp-padding-tbW;
		}
	}
	SetWindowPos(hwnd,HWND_TOP,moni.rcMonitor.left,moni.rcMonitor.top,0,0,SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
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
	
	if(g_tos<TOS_VISTA && ivMonths==1) {
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
		iMonths=GetMyRegLongEx("Calendar","ViewMonths",3);
		iMonthsPast=GetMyRegLongEx("Calendar","ViewMonthsPast",1);
		
		if(GetMyRegLong("Calendar", "ShowDayOfYear", 1)) {
			char szTitle[32];
			GetDayOfYearTitle(szTitle,iMonths);
			SetWindowText(hwnd,szTitle);
		}
		
		dwCalStyle=WS_BORDER|WS_CHILD|WS_VISIBLE|MCS_NOTODAYCIRCLE;
		if(GetMyRegLong("Calendar","ShowWeekNums",0))
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
			if(g_tos>=TOS_VISTA)
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
		SetMyDialgPos(hwnd);
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
	m_bAutoClose = GetMyRegLong("Calendar", "CloseCalendar", 1);
	m_bTopMost = GetMyRegLong("Calendar", "CalendarTopMost", 0);
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
	CheckSystemVersion();
	if(!CreateCalender(hInstance,0)) return 1;
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
