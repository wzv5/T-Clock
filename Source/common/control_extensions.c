#include "control_extensions.h"
#include "clock.h"

#include <windowsx.h>



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

void ColorBox_Setup(ColorBox boxes[], size_t num)
{
	COLORREF col;
	size_t idx;
	while(num--) {
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
	COLORREF col, colarray[16];
	HWND color_cb = GetWindow(button, GW_HWNDPREV);
	size_t idx;
	
	col = api.GetColor((COLORREF)ComboBox_GetItemData(color_cb, ComboBox_GetCurSel(color_cb)),2);
	
	for(idx=0; idx<16; ++idx) colarray[idx] = 0x00FFFFFF;
	
	cc.hwndOwner = GetParent(button);
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc))
		return 0;
	
	idx = ComboBox_GetCurSel(color_cb);
	if(idx != (size_t)ColorBox_SetColor(color_cb, cc.rgbResult) || cc.rgbResult != col)
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
