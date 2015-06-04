#pragma once
#ifndef TCLOCK_UTL_H
#define TCLOCK_UTL_H
/**
 * \brief checks if current process is in admin group
 * \return boolean */
int IsRunAsAdmin();
/**
 * \brief checks if current user is in admin group
 * \return boolean */
int IsUserInAdminGroup();
// clock related
/**
 * \brief finds the tray clock handle */
HWND FindClock();
/**
 * \brief refreshes taskbar and clock */
void RefreshUs();
// unsorted
/**
 * \brief converts a hex string to integer
 * \param p hex string to convert
 * \return parsed integer */
int atox(const char* p);
/**
 * \brief deletes the end of a path from \a path, eg. "C:/out.exe" becomes "C:/"
 * \param[in,out] path path to delete from
 * \sa add_title(), get_title() */
void del_title(char* path);
/**
 * \brief forces hwnd to the foreground,
 * even when the current process isn't the foreground process.
 * (hooks into the current foreground process to complete)
 * \param hwnd target window
 * \sa SetForegroundWindow(), SetActiveWindow() */
void ForceForegroundWindow(HWND hwnd);
/**
 * \brief multiplies two 32bit values and returns 64bit result
 * \param a left side of multiplication
 * \param b right side of multiplication */
ULONGLONG M32x32to64(DWORD a, DWORD b);
/**
 * \brief adds a title to a path, eg. "out.exe" to "C:" results "C:/out.exe"
 * \param[in,out] path to manipulate
 * \param[in] title to add (can be relative or absolute)
 * \sa get_title(), del_title() */
void add_title(char* path, const char* title);
/**
 * \brief get the title from path, eg. "out.exe" from "C:/out.exe"
 * \param[out] dst receives the title
 * \param[in] path to get title from
 * \sa add_title(), del_title() */
void get_title(char* dst, const char* path);
/**
 * \brief case-insensitively compares if ext matches the file extension of fname
 * \param fname file to test for \a ext
 * \param ext extension to find
 * \return 0 if both extensions match, otherwise the difference */
int ext_cmp(const char* fname, const char* ext);
//void parse(char* dst, const char* src, int n);
//void parsechar(char* dst, const char* src, char ch, int n);
/**
 * \brief adds a C string to a double-zero terminated string list. That is ABC,0,EFG,0,0
 * \param[in,out] list string list to add \a str to
 * \param[in] str C string to add */
void str0cat(char* list, const char* str);
/**
 * \brief returns a string with \a id from our string table
 * \param id of string to return
 * \return \c NULL on failure */
char* MyString(UINT id);
//void Pause(HWND hWnd, LPCTSTR pszArgs);
// HaveSetTimePerms.c
/**
 * \brief checks for \c SetSystemTime() permissions (\c SE_SYSTEMTIME_NAME)
 * \return boolean */
int HaveSetTimePermissions();

#include "win2k_compat.h"
#endif // TCLOCK_UTL_H
