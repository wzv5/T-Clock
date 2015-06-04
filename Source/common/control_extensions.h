#ifndef CONTROL_EXTENSIONS_H_
#define CONTROL_EXTENSIONS_H_
#include <windows.h>

/*
	GENERIC
*/
/**
 * \brief add a string if not already present and select it
 * \param box
 * \param str string to add
 * \param select if true, also select newly added string
 * \sa ComboBox_AddString() */
void ComboBox_AddStringOnce(HWND box, const char* str, int select);

/*
	LINK CONTROLS
*/
#define LCF_SIMPLE       0x00 /**< use link target as is */
#define LCF_NOTIFY       0x80 /**< notify after execution \sa LCF_NOTIFYONLY */
#define LCF_NOTIFYONLY   0xff /**< only notify parent; no execution \sa LCF_NOTIFY */
#define LCF_HTTP         0x01 /**< execute link as \c http:// by appending it \sa LCF_HTTPS, LCF_MAIL */
#define LCF_HTTPS        0x02 /**< execute link as \c https:// by appending it \sa LCF_HTTP, LCF_MAIL */
#define LCF_MAIL         0x04 /**< execute link as a \c mailto: by appending it \sa LCF_HTTP, LCF_HTTPS */
#define LCF_PARAMS       0x10 /**< link target is a command line \sa LCF_RELATIVE */
#define LCF_RELATIVE     0x20 /**< link target is a relative to \c api.root \sa LCF_PARAMS, TClockApi::root */

/**
 * \brief "converts" a static text control into a link like control
 * \param link_control static text control to become a link (with \c SS_NOTIFY style)
 * \param flags "any" combination of \c LCF_*
 * \param target link target; can be \c NULL if \p link_control's text is to be used
 * \remark \p target is used directly without a copy. You must guarantee its existence for the lifetime of our link control
 * \see LCF_SIMPLE, LCF_NOTIFY, LCF_NOTIFYONLY, LCF_HTTP, LCF_HTTPS, LCF_MAIL, LCF_PARAMS, LCF_RELATIVE */
void LinkControl_Setup(HWND link_control, unsigned char flags, const char* target);
/**
 * \brief call on \c WM_CTLCOLORSTATIC messages from our link control
 * \param hwnd parent of our link control; that is the window that received \c WM_CTLCOLORSTATIC
 * \param wParam,lParam
 * \return returncode for \c WM_CTLCOLORSTATIC ( \c handle to a background brush ) */
LRESULT LinkControl_OnCtlColorStatic(HWND hwnd, WPARAM wParam, LPARAM lParam);

/*
	COLOR BOXES
*/
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
