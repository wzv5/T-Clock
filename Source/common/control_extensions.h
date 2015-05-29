#ifndef CONTROL_EXTENSIONS_H_
#define CONTROL_EXTENSIONS_H_
#include <windows.h>


typedef struct {
	HWND hwnd;
	COLORREF color;
} ColorBox; /**< ColorBox definition to pass to ColorBox_Setup() \sa ColorBox_Setup() */

/**
 * \brief initializes a ComboBox to a ColorBox.
 * \param boxes[] ColorBox array
 * \param num number of ColorBoxes in \a boxes
 * \remark * The ComoboBox must be created with \c WS_VSCROLL|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED styles
 * \remark * Recommended width \c 70-76, height \c 180
 * \remark * The choose color button must be right next to the ComboBox (next Z order)
 * \remark * make sure you've setup \c ColorBox_OnMeasureItem(), \c ColorBox_OnDrawItem() calls
 * \sa ColorBox_OnMeasureItem(), ColorBox_OnDrawItem() */
void ColorBox_Setup(ColorBox boxes[], size_t num);
/**
 * \brief sets the currently selected color
 * \param box
 * \param new_color
 * \return the internal index of the newly set color
 * \sa ColorBox_GetColor(), ColorBox_GetColorRaw() */
int ColorBox_SetColor(HWND box, COLORREF new_color);
/**
 * \brief gets the currently selected color in raw format
 * \param hwnd box to get color from
 * \return currently selected color in raw format, that is in \c TCOLOR_* format (or raw COLORREF)
 * \sa ColorBox_GetColor(), TClockAPI::GetColor(), TCOLOR, TCOLOR_DEFAULT */
#define ColorBox_GetColorRaw(hwnd) ((COLORREF)ComboBox_GetItemData(hwnd,ComboBox_GetCurSel(hwnd)))
/**
 * \brief gets the currently selected color
 * \param hwnd box to get color from
 * \return currently selected and parsed color, ready for display
 * \sa ColorBox_GetColorRaw(), TClockAPI::GetColor(), TCOLOR, TCOLOR_DEFAULT */
#define ColorBox_GetColor(hwnd) api.GetColor(ColorBox_GetColorRaw(hwnd), 0)

/**
 * \brief call on \c WM_MEASUREITEM messages from our ColorBox (ComboBox)
 * \param wParam,lParam
 * \return returncode for \c WM_MEASUREITEM ( \c 1 if message got processed) */
LRESULT ColorBox_OnMeasureItem(WPARAM wParam, LPARAM lParam);
/**
 * \brief call on \c WM_DRAWITEM messages from our ColorBox (ComboBox)
 * \param wParam,lParam
 * \return returncode for \c WM_DRAWITEM ( \c 1 if message got processed) */
LRESULT ColorBox_OnDrawItem(WPARAM wParam, LPARAM lParam);
/**
 * \brief call for \c WM_COMMAND messages from a color choose button
 * \param button HWND of a choose color button from a ColorBox
 * \return returns 1 if a color was selected */
int ColorBox_ChooseColor(HWND button);

#endif // CONTROL_EXTENSIONS_H_
