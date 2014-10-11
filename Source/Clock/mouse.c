/*-------------------------------------------
  mouse.c - KAZUBON 1997-2001
  mouse operation
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 01/09/2011 @ 4:34:56pm
#include "tclock.h"
static const char m_click_max = 2;

static char m_click_button = -1;
static char m_click = 0;
static UINT m_doubleclick_time=0;

static int GetMouseFuncNum(char button, char nclick) {
	char entry[3];
//	if(button==1 && nclick==1){ // right mouse default menu
//		return MOUSEFUNC_MENU;
//	}
	entry[0]='0'+button;
	entry[1]='0'+nclick;
	entry[2]='\0';
	return GetMyRegLong(REG_MOUSE,entry,0);
}

/*------------------------------------------------------------
   when the clock clicked

   registry format
   name    value
   03      3           left button triple click -> Minimize All
   32      100         x-1 button  double click -> Run Notepad
   32File  C:\Windows\notepad.exe
--------------------------------------------------------------*/
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char bDown=0;
	char button, iter;
	
	(void)lParam;
	
	switch(message) {
	case WM_LBUTTONDOWN: case WM_LBUTTONUP:
		button=0; break;
	case WM_RBUTTONDOWN: case WM_RBUTTONUP:
		button=1; break;
	case WM_MBUTTONDOWN: case WM_MBUTTONUP:
		button=2; break;
	case WM_XBUTTONDOWN: case WM_XBUTTONUP:
		button=(HIWORD(wParam)==XBUTTON1?3:4); break;
	default: return;
	}
	
	switch(message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		if(m_click_button!=button) m_click=0;
		m_click_button=button;
		bDown=1;
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		if(m_click_button!=button){
			m_click_button=-1;
			m_click=0;
			m_doubleclick_time=0;
			return;
		}
		break;
	}
	if(!m_doubleclick_time)
		m_doubleclick_time=GetDoubleClickTime(); // get current mouse double click speed
	
	if(bDown) { // click not counted yet (normally we could remove this code and only handle OnUp, but Calendar doesn't hide properly otherwise)
		int n_func=GetMouseFuncNum(button, m_click+1);
		if(n_func) { // can we execute this click?
			for(iter=m_click+2; iter<=m_click_max; ++iter) { // loop thru possible clicks
				if(GetMouseFuncNum(button,iter))
					return; // if there's a mouse function set, wait for timeout or more clicks
			}
			if(n_func==MOUSEFUNC_SHOWCALENDER){ // calendar only on down, others on up
				++m_click;
				OnTimerMouse(hwnd); // execute now, no more clicks expected
			}
		}
		return;
	}
	if(GetMouseFuncNum(button,++m_click)){
		int waitable=0;
		for(iter=m_click+1; iter<=m_click_max; ++iter) {
			if(GetMouseFuncNum(button,iter)){
				++waitable;
				break;
			}
		}
		if(!waitable){ // execute now! (should never happen btw.. as of now OnDown executes in that case)
			OnTimerMouse(hwnd);
			return;
		}
	}
	SetTimer(hwnd,IDTIMER_MOUSE,m_doubleclick_time,0);
}

/*--------------------------------------------------
----------------------------- Execute Mouse Function
--------------------------------------------------*/
void OnTimerMouse(HWND hwnd)
{
	int func=GetMouseFuncNum(m_click_button,m_click);
	KillTimer(hwnd,IDTIMER_MOUSE);
	switch(func){
	case MOUSEFUNC_MENU:{
		POINT pt; GetCursorPos(&pt);
		OnContextMenu(hwnd, pt.x, pt.y);
		break;}
	case MOUSEFUNC_TIMER:
		DialogTimer();
		break;
	case MOUSEFUNC_SHOWCALENDER:
		ToggleCalendar(0);
		break;
	case MOUSEFUNC_SHOWPROPERTY:
		MyPropertySheet(-1);
		break;
	case MOUSEFUNC_CLIPBOARD:
		PostMessage(g_hwndClock,CLOCKM_COPY,0,MAKELONG(m_click_button,m_click));
		break;
	case MOUSEFUNC_SCREENSAVER:
		SendMessage(GetDesktopWindow(),WM_SYSCOMMAND,SC_SCREENSAVE,0);
		break;
	default:
		PostMessage(g_hwndTClockMain,WM_COMMAND,func,0);
	}
	m_click_button=-1;
	m_click=0;
}
