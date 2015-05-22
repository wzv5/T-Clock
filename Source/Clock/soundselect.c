/*-------------------------------------------
  soundselect.c - KAZUBON 1997-2001
  select a sound file with "Open" dialog
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

void ComboBoxArray_AddSoundFiles(HWND boxes[], int num)
{
	int i;
	char search[MAX_PATH];
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	memcpy(search, api.root, api.root_len);
	memcpy(search+api.root_len, "/waves/*", 9);
	for(i=0; i<num; ++i)
		ComboBox_AddString(boxes[i],"<  no sound  >");
	if((hFind=FindFirstFile(search, &FindFileData)) != INVALID_HANDLE_VALUE) {
		do{
			if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) { // only files (also ignores . and ..)
				for(i=0; i<num; ++i)
					ComboBox_AddString(boxes[i], FindFileData.cFileName);
			}
		}while(FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	for(i=0; i<num; ++i){
		if(!ComboBox_GetTextLength(boxes[i]))
			ComboBox_SetCurSel(boxes[i], 0);
	}
}

void GetMMFileExts(char* dst)
{
	char extlist[1024], *ext, *pout;
	GetProfileString("mci extensions",NULL,"",extlist,1024);
	for(pout=dst,ext=extlist; *ext; ++ext) {
		*pout++='*';*pout++='.';
		while(*ext) *pout++=*ext++;
		*pout++=';';
	}
	memcpy(pout,"*.pcb",6);
}

/*------------------------------------------------------------------
---------------------------------- open dialog to browse sound files
------------------------------------------------------------------*/
BOOL BrowseSoundFile(HWND hDlg, const char* deffile, char* fname)
{
	char filter[1024], mmfileexts[1024];
	char ftitle[MAX_PATH], initdir[MAX_PATH];
	
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn)); // Initialize OPENFILENAME
	ofn.lStructSize = sizeof(ofn);
	
	filter[0]=filter[1]='\0';
	str0cat(filter, MyString(IDS_MMFILE));
	GetMMFileExts(mmfileexts);
	str0cat(filter, mmfileexts);
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");
	
	if(!deffile[0] || IsMMFile(deffile)) ofn.nFilterIndex = 1;
	else ofn.nFilterIndex = 2;
	
	memcpy(initdir, api.root, api.root_len+1);
	if(deffile[0]) {
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		hfind = FindFirstFile(deffile, &fd);
		if(hfind != INVALID_HANDLE_VALUE) {
			FindClose(hfind);
			strncpy_s(initdir,sizeof(initdir),deffile,_TRUNCATE);
			del_title(initdir);
		}
	}
	
	*fname = '\0';
	
	ofn.hwndOwner = hDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;
	
	if(GetOpenFileName(&ofn)) {
		size_t tlen=api.root_len;
		if(!strncmp(fname,api.root,tlen)) { // make relative to waves/ if possible
			if(!strncmp(fname+tlen,"\\waves\\",7)) {
				memmove(fname,fname+tlen+7,strlen(fname)-tlen-6);
			}
		}
		return 1;
	}
	return 0;
}

BOOL IsMMFile(const char* fname)
{
	char extlist[1024], *ext;
	if(!lstrcmpi(fname,"cdaudio")) return TRUE;
	if(!ext_cmp(fname,"pcb")) return TRUE;
	GetProfileString("mci extensions",NULL,"",extlist,sizeof(extlist));
	for(ext=extlist; *ext; ++ext) {
		if(!ext_cmp(fname,ext)) return TRUE;
		while(*++ext);
	}
	return FALSE;
}
/*
static BOOL bPlaying = FALSE;
void OnInitDialog(HWND hDlg)
{
	HWND hwndStatic;
	RECT rc1, rc2;
	POINT pt;
	int dx;
	
	SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
	EnableDlgItem(hDlg, IDC_TESTSOUND, FALSE);
	
	bPlaying = FALSE;
	
	// find "File Name:" Label
	hwndStatic = GetDlgItem(GetParent(hDlg), 0x442);
	if(hwndStatic == NULL) return;
	GetWindowRect(hwndStatic, &rc1);
	
	// move "Test:" Label
	GetWindowRect(GetDlgItem(hDlg, IDC_LABTESTSOUND), &rc2);
	dx = rc1.left - rc2.left;
	pt.x = rc2.left + dx; pt.y = rc2.top;
	ScreenToClient(hDlg, &pt);
	SetWindowPos(GetDlgItem(hDlg, IDC_LABTESTSOUND), NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	
	// move play button
	GetWindowRect(GetDlgItem(hDlg, IDC_TESTSOUND), &rc2);
	pt.x = rc2.left + dx; pt.y = rc2.top;
	ScreenToClient(hDlg, &pt);
	SetWindowPos(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}// */
/*
void OnFileNameChanged(HWND hDlg)
{
	char fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	BOOL b = FALSE;
	
	HANDLE hfind = INVALID_HANDLE_VALUE;
	
	if(CommDlg_OpenSave_GetFilePath(GetParent(hDlg), fname, sizeof(fname)) <= sizeof(fname)) {
		hfind = FindFirstFile(fname, &fd);
		if(hfind != INVALID_HANDLE_VALUE) {
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) b = TRUE;
			FindClose(hfind);
		}
	}
	EnableDlgItem(hDlg, IDC_TESTSOUND, b);
}// */
/*
void OnTestSound(HWND hDlg)
{
	char fname[MAX_PATH];
	
	if(CommDlg_OpenSave_GetFilePath(GetParent(hDlg), fname, sizeof(fname)) <= sizeof(fname)) {
		if((HICON)SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay) {
			if(PlayFile(hDlg, fname, 0)) {
				SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
				InvalidateRect(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, FALSE);
				bPlaying = TRUE;
			}
		} else {
			StopFile(); bPlaying = FALSE;
		}
	}
}// */
