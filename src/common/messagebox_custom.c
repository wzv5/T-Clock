#include "messagebox_custom.h"
#include "win2k_compat.h"
#include <windowsx.h>
#include <commctrl.h>

typedef struct {
	int is_vista_or_higher;
	int close_id;
	int autodisable;
} MsgData;

static LRESULT CALLBACK Window_MessageBoxCustomDlg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	MsgData* data = (MsgData*)dwRefData;
	enum{kYes, kNo, kCancel, kHelp, kButtons_,};
	const char kBtnId[kButtons_] = {IDYES, IDNO, IDCANCEL, IDHELP};
	LRESULT ret;
	int iter;
	int child_id;
	
	switch(uMsg) {
	case WM_INITDIALOG:{
		HWND btn_hwnd[kButtons_];
		RECT btn_rc[kButtons_];
		HFONT font;
		RECT rc, check_rc;
		HICON icon;
		int unused = 0;
		MessageBoxCustomData* settings;
		
		ret = DefSubclassProc(hwnd, uMsg, wParam, lParam);
		// get settings
		settings = (MessageBoxCustomData*)SendMessage(GetParent(hwnd), WMBC_INITDIALOG, 0, 0); // notify creator
		if(!settings) {
			EndDialog(hwnd, IDCANCEL);
			return 1;
		}
		if(!IsWindowVisible(GetParent(hwnd))){
			iter = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, iter|WS_EX_APPWINDOW);
		}
		icon = settings->icon_title_small;
		if(!icon)
			icon = (HICON)SendMessage(GetParent(hwnd), WM_GETICON, ICON_SMALL, 0);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
		icon = settings->icon_title_big;
		if(!icon)
			icon = (HICON)SendMessage(GetParent(hwnd), WM_GETICON, ICON_BIG, 0);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
		if(settings->icon_text)
			Static_SetIcon(GetDlgItem(hwnd,20), settings->icon_text);
		// read buttons
		data->close_id = settings->close_id;
		if(!data->close_id)
			data->close_id = IDCANCEL;
		if(GetProcAddress(GetModuleHandleA("kernel32"), "FlsFree")) // could also use GetTickCount64
			data->is_vista_or_higher = 1;
		for(iter=0; iter<kButtons_; ++iter) {
			btn_hwnd[iter] = GetDlgItem(hwnd, kBtnId[iter]);
			if(settings->button[iter].style) {
				int style = settings->button[iter].style;
				if(!(style & BS_MBC_RAW_STYLE))
					style |= GetWindowLongPtr(btn_hwnd[iter],GWL_STYLE);
				style &= ~BS_MBC_RAW_STYLE;
				SetWindowLongPtr(btn_hwnd[iter], GWL_STYLE, style);
				UpdateWindow(btn_hwnd[iter]);
			}
			if(settings->button[iter].icon)
				SendMessage(btn_hwnd[iter], BM_SETIMAGE, IMAGE_ICON, (LPARAM)settings->button[iter].icon);
			GetWindowRect(btn_hwnd[iter], &btn_rc[iter]);
			if(unused || !settings->button[iter].text)
				++unused;
			else
				SetWindowText(btn_hwnd[iter], settings->button[iter].text);
		}
		
		if(settings->check[0].text) {
			// resize dialog
			check_rc.left = check_rc.bottom = 0;
			check_rc.top = 0x7fffffff;
			for(iter=0; iter<MBC_MAX_CHECKBOXES && settings->check[iter].text; ++iter){
				if(settings->check[iter].pos.top < check_rc.top && settings->check[iter].pos.top >= 0)
					check_rc.top = settings->check[iter].pos.top;
				check_rc.right = (settings->check[iter].pos.top + settings->check[iter].pos.bottom);
				if(check_rc.right > check_rc.bottom)
					check_rc.bottom = check_rc.right;
			}
			check_rc.bottom += check_rc.top;
			MapDialogRect(hwnd, &check_rc);
			GetWindowRect(hwnd, &rc);
			rc.right -= rc.left;
			rc.bottom = btn_rc[0].bottom - rc.top;
			SetWindowPos(hwnd, 0, 0, 0, rc.right, rc.bottom+check_rc.bottom, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
		}
		MapWindowPoints(NULL, hwnd, (POINT*)&btn_rc, sizeof(btn_rc)/sizeof(POINT));
		
		// adjust buttons
		for(iter=0; iter<kButtons_ - unused; ++iter) {
			SetWindowLongPtr(btn_hwnd[iter], GWLP_ID, ID_MBC1+iter);
			SetWindowPos(btn_hwnd[iter], 0, btn_rc[iter+unused].left+iter, btn_rc[iter+unused].top, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
		}
		for(; iter<kButtons_; ++iter)
			ShowWindow(btn_hwnd[iter], SW_HIDE);
		
		// add checkboxes
		font = GetWindowFont(GetDlgItem(hwnd, (unsigned short)-1));
		for(iter=0; iter<MBC_MAX_CHECKBOXES && settings->check[iter].text; ++iter) {
			HWND check;
			int style;
			check_rc = settings->check[iter].pos;
			if(check_rc.top)
				--check_rc.top;
			MapDialogRect(hwnd, &check_rc);
			style = settings->check[iter].style;
			if(!(settings->check[iter].state & BST_MBC_RAW_STYLE))
				style |= WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX;
			check = CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, settings->check[iter].text, style, check_rc.left, btn_rc[0].bottom+check_rc.top, check_rc.right, check_rc.bottom, hwnd, NULL, NULL, NULL);
			SetWindowLongPtr(check, GWLP_ID, MBC_CHECK1+iter);
			SetWindowFont(check, font, 1);
			Button_SetCheck(check, (settings->check[iter].state&0xffff));
			if(settings->check[iter].state & BST_MBC_AUTODISABLE) {
				int enabled = Button_GetCheck(GetDlgItem(hwnd, MBC_CHECK1+iter-1));
				Button_Enable(check, enabled);
				data->autodisable |= (1<<iter);
			}
		}
		return ret;}
	case WM_DESTROY:
		ret = 0;
		for(iter=0; iter<MBC_MAX_CHECKBOXES; ++iter) {
			HWND check = GetDlgItem(hwnd, MBC_CHECK1+iter);
			if(check)
				ret |= (Button_GetCheck(check) << iter);
		}
		SendMessage(GetParent(hwnd), WMBC_CHECKS, ret, 0); // notify creator
		RemoveWindowSubclass(hwnd, Window_MessageBoxCustomDlg, uIdSubclass);
		free(data);
		break;
	case WM_CTLCOLORSTATIC:
		if(data->is_vista_or_higher) {
			child_id = GetDlgCtrlID((HWND)lParam);
			if(child_id >= MBC_CHECK1 && child_id < MBC_CHECK_END) {
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockBrush(NULL_BRUSH);
			}
		}
		break;
	case WM_COMMAND:
		child_id = LOWORD(wParam);
		switch(child_id) {
		case IDCANCEL:{
			HWND btn = GetDlgItem(hwnd, data->close_id);
			if(btn && !IsWindowEnabled(btn)){
				MessageBeep(MB_OK);
				return 1;
			}
			EndDialog(hwnd, data->close_id);
			return 1;}
		case ID_MBC1:
		case ID_MBC2:
		case ID_MBC3:
		case ID_MBC4:
			EndDialog(hwnd, child_id); // we could call original_window_proc_ first..
			return 1;
		default:
			if(child_id >= MBC_CHECK1 && child_id < MBC_CHECK_END) {
				HWND check = GetDlgItem(hwnd, ++child_id);
				if(check && data->autodisable & (1<<(child_id-MBC_CHECK1)))
					Button_Enable(check, Button_GetCheck((HWND)lParam));
				return 1;
			}
		}
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

static HHOOK hook_ = NULL;
static LRESULT CALLBACK MessageBoxCustom_Hook(int nCode, WPARAM wParam, LPARAM lParam) {
	if(nCode == HCBT_CREATEWND) {
		MsgData* data;
		UnhookWindowsHookEx(hook_);
		hook_ = NULL;
		data = (MsgData*)calloc(1, sizeof(MsgData));
		if(!data || !SetWindowSubclass((HWND)wParam, Window_MessageBoxCustomDlg, 0, (DWORD_PTR)data)) {
			free(data);
			PostMessage((HWND)wParam, WM_CLOSE, 0, 0);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int MessageBoxCustom(HWND parent, const wchar_t* message, const wchar_t* title, unsigned style) {
	int ret;
	if(hook_)
		return 0;
	hook_ = SetWindowsHookEx(WH_CBT, MessageBoxCustom_Hook, NULL, GetCurrentThreadId());
	ret = MessageBox(parent, message, title, MB_YESNOCANCEL|MB_HELP|style);
	return ret;
}

static LRESULT CALLBACK MBC_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MessageBoxCustomData* data = (MessageBoxCustomData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	int i;
	(void)lParam;
	
	switch(message) {
	case WMBC_INITDIALOG:
		return (LRESULT)data;
	case WMBC_CHECKS:
		for(i=0; i<MBC_MAX_CHECKBOXES; ++i){
			if(wParam&1)
				data->check[i].state |= BST_CHECKED;
			else
				data->check[i].state &= ~BST_CHECKED;
			wParam >>= 1;
		}
		return 0;
	}
	return 0;
}
int MessageBoxCustom_Direct(MessageBoxCustomData* settings, const wchar_t* message, const wchar_t* title, unsigned style) {
	int ret;
	HWND parent = CreateWindowA("STATIC", NULL, 0, 0,0,0,0, HWND_MESSAGE, 0, 0, 0);
	if(!parent)
		return 0;
	SetWindowLongPtr(parent, GWLP_USERDATA, (LONG_PTR)settings);
	SubclassWindow(parent, MBC_Proc);
	ret = MessageBoxCustom(parent, message, title, style);
	DestroyWindow(parent);
	return ret;
}
