#include "../common/globals.h"
#include "control_extensions.h"

/*
	LINK CONTROLS
*/
//#define WM_LINKSETTARGET    WM_USER+0x4910

static HCURSOR m_cursor_hand = NULL;
static WNDPROC m_proc_static = NULL;
static LRESULT CALLBACK LinkControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef struct{
	const char* target;
	unsigned char flags;
} link_data;

void LinkControl_Setup(HWND link_control, unsigned char flags, const char* target) {
	link_data* data;
	if(!m_cursor_hand)
		m_cursor_hand = LoadCursor(NULL, IDC_HAND);
	if(GetWindowLongPtr(link_control, GWLP_USERDATA))
		return;
	
	data = (link_data*)malloc(sizeof(link_data));
	data->target = target;
	data->flags = flags;
	SetWindowLongPtr(link_control, GWLP_USERDATA, (LONG_PTR)data);
	m_proc_static = SubclassWindow(link_control, LinkControlProc);
}

LRESULT LinkControl_OnCtlColorStatic(HWND hwnd, WPARAM wParam, LPARAM lParam) {
	SetTextColor((HDC)wParam,RGB(0,0,255));
	return DefWindowProc(hwnd, WM_CTLCOLORSTATIC, wParam, lParam);
}

LRESULT CALLBACK LinkControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_DESTROY:{
		link_data* data = (link_data*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		free(data);
		break;}
	//case WM_LINKSETTARGET:
	//	return 0;
	case WM_SETCURSOR:
		SetCursor(m_cursor_hand);
		return TRUE;
//	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:{
		char str[MAX_PATH];
		char* offset;
		link_data* data = (link_data*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		HWND parent = GetParent(hwnd);
		if(data->flags != LCF_NOTIFYONLY){
			str[0] = '\0';
			offset = str;
			if(data->flags & LCF_HTTP){
				memcpy(offset, "http://", 7);
				offset += 7;
			}else if(data->flags & LCF_HTTPS){
				memcpy(offset, "https://", 8);
				offset += 8;
			}else if(data->flags & LCF_MAIL){
				memcpy(offset, "mailto:", 7);
				offset += 7;
			}else if(data->flags & LCF_RELATIVE){
				memcpy(offset, api.root, api.root_len);
				offset += api.root_len;
				*offset++ = '\\';
			}
			
			if(data->target){
				strncpy_s(offset, (sizeof(str)-(offset-str)), data->target, _TRUNCATE);
			}else{
				GetWindowText(hwnd, offset, (int)(sizeof(str)-(offset-str)));
			}
			
			if(str[0]){
				if(data->flags & LCF_PARAMS)
					api.ExecFile(str, parent);
				else
					api.Exec(str, NULL, parent);
			}
		}
		
		if(data->flags & LCF_NOTIFY)
			SendMessage(parent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd),STN_CLICKED), (LPARAM)hwnd);
		return 0;}
//	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
//	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:
		return 0;
	}
	return CallWindowProc(m_proc_static, hwnd, msg, wParam, lParam);
}

/*
	COLOR BOXES
*/

typedef struct {
	COLORREF col;
	const char* name;
} syscolor_t;
const syscolor_t m_syscolor[]={
	{TCOLOR_DEFAULT,"<default>"},
	{TCOLOR_TRANSPARENT,"<transparent>"},
	{TCOLOR_THEME,"<theme color>"},
//	{TCOLOR_THEME2,"<theme color II>"},
	
	{COLOR_3DFACE,"3DFACE"},
	{COLOR_3DSHADOW,"3DSHADOW"},
	{COLOR_3DHILIGHT,"3DHILIGHT"},
	{COLOR_BTNTEXT,"BTNTEXT"},
	{COLOR_WINDOWTEXT,"WINDOWTEXT"},
	{COLOR_INFOTEXT,"INFOTEXT"},
	{COLOR_INFOBK,"INFOBK"},
};
static const size_t m_syscolor_num = sizeof(m_syscolor)/sizeof(syscolor_t);
static const COLORREF m_basecolor[]={
	0x00000080, 0x00008000, 0x00800000,
	0x00008080, 0x00800080, 0x00808000,
	0x000000FF, 0x0000FF00, 0x00FF0000,
	0x0000FFFF, 0x00FF00FF, 0x00FFFF00,
	0x00FFFFFF, 0x00C0C0C0, 0x00808080, 0x00000000,
};
static const size_t m_basecolor_num = sizeof(m_basecolor)/sizeof(COLORREF);
static const size_t m_colorstotal = sizeof(m_syscolor)/sizeof(syscolor_t)+sizeof(m_basecolor)/sizeof(COLORREF);

static COLORREF m_usercolors[16] = {
	0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF, 0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,
	0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF, 0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF
};

static WNDPROC m_proc_combo = NULL;
static LRESULT CALLBACK ColorBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ColorBox_Setup(ColorBox boxes[], size_t num)
{
	COLORREF col;
	size_t idx;
	while(num--) {
		m_proc_combo = SubclassWindow(boxes[num].hwnd, ColorBoxProc);
		// add sys colors
		for(idx=0; idx<m_syscolor_num; ++idx){
			ComboBox_AddString(boxes[num].hwnd, (size_t)TCOLOR(m_syscolor[idx].col));
		}
		// add base colors
		for(idx=0; idx<m_basecolor_num; ++idx)
			ComboBox_AddString(boxes[num].hwnd, (size_t)m_basecolor[idx]);
		// select last used color
		col = boxes[num].color;
		for(idx=0; idx<m_colorstotal; ++idx) {
			if(col==(COLORREF)ComboBox_GetItemData(boxes[num].hwnd, idx))
				break;
		}
		if(idx==m_colorstotal) // Add the Selected Custom Color
			ComboBox_AddString(boxes[num].hwnd, (size_t)col);
		ComboBox_SetCurSel(boxes[num].hwnd, idx);
	}
}

LRESULT CALLBACK ColorBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_MEASUREITEM:
		return ColorBox_OnMeasureItem(wParam, lParam);
	case WM_DRAWITEM:
		return ColorBox_OnDrawItem(wParam, lParam);
	case WM_KEYDOWN:
		if(lParam&0x4000)
			break; // repeated msg, key was down
		if(wParam == 'C' || wParam == 'X' || wParam == 'V'){
			if(GetAsyncKeyState(VK_CONTROL)&0x8000){
				SendMessage(hwnd, (wParam=='V'?WM_PASTE:WM_COPY), 0, 0);
				return 0;
			}
		}
		break;
	case WM_CUT:
	case WM_COPY:{
		char color_str[8];
		HGLOBAL hg;
		char* pbuf;
		unsigned color = api.GetColor(ColorBox_GetColorRaw(hwnd),2);
		color = ((color&0xff)<<16) | (color&0xff00) | ((color&0xff0000)>>16);
		__pragma(warning(suppress:4996)) sprintf(color_str, "#%06x", color);
		if(!OpenClipboard(hwnd))
			return 0;
		EmptyClipboard();
		hg = GlobalAlloc(GMEM_DDESHARE, 8);
		pbuf = (char*)GlobalLock(hg);
		memcpy(pbuf, color_str, 8);
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		return 0;}
	case WM_PASTE:{
		HGLOBAL hg;
		const char* pbuf;
		size_t size;
		if(!OpenClipboard(hwnd))
			return 0;
		for(;;){ // only once loop, MSVC can't handle do{}while(0)
			hg = GetClipboardData(CF_TEXT);
			if(!hg)
				break;
			pbuf = (const char*)GlobalLock(hg);
			if(!pbuf)
				break;
			size = GlobalSize(hg);
			if(size > 6){ // could be a hex color (HTML format)
				unsigned color = 0;
				if(pbuf[0] == '#' && size > 7)
					++pbuf;
				for(size=6; size; ){
					char c = (char)toupper(pbuf[--size]);
					color <<= 8;
					if(c >= 'A' && c <= 'F'){
						color |= c-('A'-10);
					}else if(c >= '0' && c <= '9'){
						color |= c-'0';
					}else
						break;
					c = (char)toupper(pbuf[size-1]); // in case we break;
					if(c >= 'A' && c <= 'F'){
						color |= (c-('A'-10)) << 4;
					}else if(c >= '0' && c <= '9'){
						color |= (c-'0') << 4;
					}else
						break;
					--size;
				}
				if(!size){ // valid hex value
					for(size=TCOLOR_BEGIN_; size<TCOLOR_END_; ++size){
						if(color == (api.GetColor(TCOLOR(size),2) & 0xffffff))
							break;
					}
					if(size != TCOLOR_END_)
						ColorBox_SetColor(hwnd, TCOLOR(size));
					else
						ColorBox_SetColor(hwnd, color);
					PostMessage(GetParent(hwnd),WM_COMMAND, MAKEWPARAM(GetWindowID(hwnd),CBN_SELCHANGE), (LPARAM)hwnd);
				}
			}
			GlobalUnlock(hg);
			break;
		}
		CloseClipboard();
		return 0;}
	}
	return CallWindowProc(m_proc_combo, hwnd, msg, wParam, lParam);
}

/*------------------------------------------------
--------------------------------------------------*/
LRESULT ColorBox_OnMeasureItem(WPARAM wParam, LPARAM lParam) {
	MEASUREITEMSTRUCT* pmis = (MEASUREITEMSTRUCT*)lParam;
	(void)wParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
	return 1;
}
LRESULT ColorBox_OnDrawItem(WPARAM wParam, LPARAM lParam) {
	DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*)lParam;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	
	(void)wParam;
	
	if(IsWindowEnabled(pdis->hwndItem)) {
		col = api.GetColor((COLORREF)pdis->itemData,2);
	} else col = GetSysColor(COLOR_3DFACE);
	
	switch(pdis->itemAction) {
	case ODA_DRAWENTIRE:
	case ODA_SELECT: {
		hbr = CreateSolidBrush(col);
		FillRect(pdis->hDC, &pdis->rcItem, hbr);
		DeleteObject(hbr);
		
		// print sys color names
		if((int)pdis->itemID < m_syscolor_num){
			int y;
			SetBkMode(pdis->hDC, TRANSPARENT);
			GetTextMetrics(pdis->hDC, &tm);
			SetTextColor(pdis->hDC, 0x00FFFFFF-(col&0x00FFFFFF));
			if(col==0x00808080)
				SetTextColor(pdis->hDC, 0);
			y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
			TextOut(pdis->hDC, pdis->rcItem.left + 4, pdis->rcItem.top + y,
					m_syscolor[pdis->itemID].name, (int)strlen(m_syscolor[pdis->itemID].name));
		}
		if(!(pdis->itemState & ODS_FOCUS)) break;
		}
	case ODA_FOCUS: {
		HBRUSH hbr2;
		RECT rc;
		rc.left=pdis->rcItem.left-1;
		rc.top=pdis->rcItem.top-1;
		rc.right=pdis->rcItem.right-1;
		rc.bottom=pdis->rcItem.bottom-1;
		if(pdis->itemState & ODS_FOCUS){
			hbr=CreateSolidBrush(0x00000000);
			hbr2=CreateSolidBrush(0x00FFFFFF);
		}else
			hbr=hbr2=CreateSolidBrush(col);
		FrameRect(pdis->hDC, &pdis->rcItem, hbr);
		FrameRect(pdis->hDC, &rc, hbr2);
		if(hbr2!=hbr) DeleteObject(hbr2);
		DeleteObject(hbr);
		break;}
	}
	return 1;
}

int ColorBox_ChooseColor(HWND button)
{
	CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
	COLORREF color;
	HWND color_cb = GetWindow(button, GW_HWNDPREV);
	size_t idx;
	
	color = api.GetColor((COLORREF)ComboBox_GetItemData(color_cb, ComboBox_GetCurSel(color_cb)),2);
	
	cc.hwndOwner = GetParent(button);
	cc.rgbResult = color;
	cc.lpCustColors = m_usercolors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc))
		return 0;
	
	idx = ComboBox_GetCurSel(color_cb);
	if(idx != (size_t)ColorBox_SetColor(color_cb, cc.rgbResult) || cc.rgbResult != color)
		PostMessage(cc.hwndOwner,WM_COMMAND, MAKEWPARAM(GetWindowID(color_cb),CBN_SELCHANGE), (LPARAM)color_cb);
	
	return 1;
}

int ColorBox_SetColor(HWND box, COLORREF new_color){
	size_t idx;
	
	for(idx=0; idx<m_colorstotal; ++idx) {
		if(new_color == (COLORREF)ComboBox_GetItemData(box,idx))
			break;
	}
	if(idx == m_colorstotal){
		if(ComboBox_GetCount(box) == m_colorstotal)
			ComboBox_AddString(box, (size_t)new_color);
		else
			ComboBox_SetItemData(box, idx, new_color);
	}
	ComboBox_SetCurSel(box, idx);
	return (int)idx;
}
