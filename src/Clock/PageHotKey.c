// Created by Stoic Joker: Tuesday, 03/30/2010 @ 10:27:12pm
#include "tclock.h"

/// @note (White-Tiger#1#11/26/15): don't forget multimedia keys such as "mute volume" as they weren't directly supported by T-Clock nor are supported by HOTKEY_CLASS ...

static char szHotKeySubKey[] = "HotKeys";
#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

hotkey_t GetHotkey(int idx) {
	char subkey[TNY_BUFF];
	hotkey_t hotkey;
	
	wsprintf(subkey, "%s\\HK%d", szHotKeySubKey, idx);
	hotkey.key.fsMod = (uint8_t)api.GetInt(subkey, "fsMod", 0);
	hotkey.key.vk = (uint8_t)api.GetInt(subkey, "vk", 0);
	return hotkey;
}
void SetHotkey(int idx, hotkey_t hotkey) {
	char subkey[TNY_BUFF];
	wsprintf(subkey, "%s\\HK%d", szHotKeySubKey, idx);
	if(hotkey.key.vk){
		api.SetInt(subkey, "fsMod", hotkey.key.fsMod);
		api.SetInt(subkey, "vk", hotkey.key.vk);
		/// @note (White-Tiger#1#11/26/15): on next backward incompatible change, remove this and cleanup leftovers
		api.SetInt(subkey, "bValid", 1);
		api.SetStr(subkey, "szText", "?");
	} else {
		api.DelKey(subkey);
	}
}

// HOTKEY_CLASS translation list
const uint8_t kHotkeyBox_ExKeys[][3] = {
	// key , "normal" , "extended" (HOTKEYF_EXT)
	{VK_LEFT, VK_NUMPAD4, VK_LEFT},
	{VK_RIGHT, VK_NUMPAD6, VK_RIGHT},
	{VK_UP, VK_NUMPAD8, VK_UP},
	{VK_DOWN, VK_NUMPAD2, VK_DOWN},
	{VK_INSERT, VK_NUMPAD0, VK_INSERT},
	{VK_END, VK_NUMPAD1, VK_END},
	{VK_NEXT, VK_NUMPAD3, VK_NEXT},
	// <VK_NUMPAD5>
	{VK_HOME, VK_NUMPAD7, VK_HOME},
	{VK_PRIOR, VK_NUMPAD9, VK_PRIOR},
	{VK_DIVIDE, VK_DIVIDE, VK_DIVIDE}, // some weird key without HOTKEYF_EXT
};

void HotkeyBox_Init(HWND hDlg, int idx) {
	HWND control;
	hotkey_t hk = GetHotkey(idx);
	
	control = GetDlgItem(hDlg, (HOTKEY_BEGIN+idx));
	SendMessage(control, HKM_SETRULES, HKCOMB_NONE, (HOTKEYF_CONTROL|HOTKEYF_SHIFT));
	HotkeyBox_SetValue(control, hk);
	
	control = GetDlgItem(hDlg, (HOTKEY_BTN_BEGIN+idx));
	EnableWindow(control, 0);
}

hotkey_t HotkeyBox_GetValue(HWND box) {
	int i;
	hotkey_t hotkey;
	uint8_t flags = 0;
	hotkey.word = (uint16_t)SendMessage(box, HKM_GETHOTKEY, 0, 0);
	if(hotkey.key.fsMod & HOTKEYF_SHIFT)
		flags |= MOD_SHIFT;
	if(hotkey.key.fsMod & HOTKEYF_CONTROL)
		flags |= MOD_CONTROL;
	if(hotkey.key.fsMod & HOTKEYF_ALT)
		flags |= MOD_ALT;
	for(i=0; i<_countof(kHotkeyBox_ExKeys); ++i) {
		if(hotkey.key.vk == kHotkeyBox_ExKeys[i][0]) {
			hotkey.key.vk = kHotkeyBox_ExKeys[i][ (hotkey.key.fsMod & HOTKEYF_EXT ? 2 : 1) ];
			hotkey.key.fsMod &= ~hotkey.key.fsMod;
			break;
		}
	}
	hotkey.key.fsMod = flags;
	return hotkey;
}
void HotkeyBox_SetValue(HWND box, hotkey_t hotkey) {
	int i;
	uint8_t flags = 0;
	if(hotkey.key.fsMod & MOD_SHIFT)
		flags |= HOTKEYF_SHIFT;
	if(hotkey.key.fsMod & MOD_CONTROL)
		flags |= HOTKEYF_CONTROL;
	if(hotkey.key.fsMod & MOD_ALT)
		flags |= HOTKEYF_ALT;
	for(i=0; i<_countof(kHotkeyBox_ExKeys); ++i) {
		if(hotkey.key.vk == kHotkeyBox_ExKeys[i][2]) {
			hotkey.key.vk = kHotkeyBox_ExKeys[i][0];
			flags |= HOTKEYF_EXT;
			break;
		}
		if(hotkey.key.vk == kHotkeyBox_ExKeys[i][1]) {
			hotkey.key.vk = kHotkeyBox_ExKeys[i][0];
			break;
		}
	}
	hotkey.key.fsMod = flags;
	SendMessage(box, HKM_SETHOTKEY, hotkey.word, 0);
}

//================================================================================================
//-----------------------------------------------+++--> Save the New HotKey Configuration Settings:
static void OnApply(HWND hDlg)   //---------------------------------------------------------+++-->
{
	int i;
	hotkey_t hotkey;
	
	for(i=0; i < HOTKEY_NUM; ++i) {
		hotkey = HotkeyBox_GetValue(GetDlgItem(hDlg,(HOTKEY_BEGIN+i)));
		SetHotkey(i, hotkey);
		
		EnableDlgItem(hDlg, (HOTKEY_BTN_BEGIN+i), 0);
	}
}
//================================================================================================
//-------------------------------//---------------------------+++--> Initialize the HotKeys Dialog:
static void OnInit(HWND hDlg)   //----------------------------------------------------------+++-->
{
	int i;
	
	for(i=0; i < HOTKEY_NUM; ++i) {
		HotkeyBox_Init(hDlg, i);
	}
}
//================================================================================================
//---------------------------------+++--> Dialog Precedure For Configurable HotKeys Properties Tab:
INT_PTR CALLBACK PageHotKeyProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)   //----+++-->
{
	switch(message) {
	case WM_INITDIALOG:
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:
		break;
	case WM_NOTIFY:{
		PSHNOTIFY* notify = (PSHNOTIFY*)lParam;
		switch(notify->hdr.code) {
		case PSN_QUERYINITIALFOCUS:
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)GetDlgItem(hDlg,-1/*first static*/));
			break;
		case PSN_SETACTIVE:
			RegisterHotkeys(g_hwndTClockMain, 0);
			break;
		case PSN_RESET: // in case page was active and dialog closes
		case PSN_KILLACTIVE:
			RegisterHotkeys(g_hwndTClockMain, 1);
			break;
		case PSN_APPLY:
			OnApply(hDlg);
			RegisterHotkeys(g_hwndTClockMain, 2); // PSN_KILLACTIVE is called first on "OK"
			break;
		}
		return TRUE;}
	
	case WM_COMMAND: {
		int idx;
		WORD id = LOWORD(wParam);
//		WORD code = HIWORD(wParam);
		
		switch(id) {
		default:
			if(id >= HOTKEY_BEGIN && id <= HOTKEY_END) {
				idx = (id - HOTKEY_BEGIN);
				EnableDlgItem(hDlg, (HOTKEY_BTN_BEGIN+idx), 1);
				SendPSChanged(hDlg);
			} else if(id >= HOTKEY_BTN_BEGIN && id <= HOTKEY_BTN_END) {
				HWND control;
				idx = (id - HOTKEY_BTN_BEGIN);
				control = GetDlgItem(hDlg, (HOTKEY_BEGIN+idx));
				EnableWindow((HWND)lParam, 0);
				HotkeyBox_SetValue(control, GetHotkey(idx));
				SetFocus(control);
			}
		}
		return TRUE;}
	}
	return FALSE;
}
//================================================================================================
//----------------------------------+++--> Retrieve HotKey Configuration Information From Registry:
void RegisterHotkeys(HWND hwnd, int want_register) {
	int i;
	
	for(i=0; i < HOTKEY_NUM; ++i) {
		if(want_register != 1)
			UnregisterHotKey(hwnd, (HOTKEY_BEGIN+i));
		if(want_register) {
			hotkey_t hk = GetHotkey(i);
			if(hk.key.vk)
				RegisterHotKey(hwnd, (HOTKEY_BEGIN+i), hk.key.fsMod, hk.key.vk);
		}
	}
}
