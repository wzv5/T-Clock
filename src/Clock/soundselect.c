/*-------------------------------------------
  soundselect.c - KAZUBON 1997-2001
  select a sound file with "Open" dialog
---------------------------------------------*/
// Last Modified by Stoic Joker: Sunday, 03/13/2011 @ 11:54:05am
#include "tclock.h"

void ComboBoxArray_AddSoundFiles(HWND boxes[], int num)
{
	int i;
	wchar_t search[MAX_PATH];
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	memcpy(search, api.root, api.root_size);
	wcscpy(search+api.root_len, L"/waves/*");
	for(i=0; i<num; ++i)
		ComboBox_AddString(boxes[i], L"<  no sound  >");
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
		ComboBox_GetText(boxes[i], search, _countof(search));
		ComboBox_AddStringOnce(boxes[i], search, 1, 0);
	}
}

void GetMMFileExts(wchar_t* dst)
{
	wchar_t extlist[1024], *ext, *pout;
	GetProfileString(L"mci extensions", NULL, L"", extlist, _countof(extlist));
	for(pout=dst,ext=extlist; *ext; ++ext) {
		*pout++='*';*pout++='.';
		while(*ext) *pout++=*ext++;
		*pout++=';';
	}
	wcscpy(pout, L"*.pcb");
}

/*------------------------------------------------------------------
---------------------------------- open dialog to browse sound files
------------------------------------------------------------------*/
BOOL BrowseSoundFile(HWND hDlg, const wchar_t* deffile, wchar_t fname[MAX_PATH])
{
	wchar_t filter[1024], mmfileexts[1024];
	DWORD index;
	
	filter[0]=filter[1]='\0';
	str0cat(filter, MyString(IDS_MMFILE));
	GetMMFileExts(mmfileexts);
	str0cat(filter, mmfileexts);
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, L"*.*");
	
	if(!deffile[0] || IsMMFile(deffile))
		index = 1;
	else
		index = 2;
	
	if(SelectMyFile(hDlg, filter, index, deffile, fname)) {
		if(!wcsncmp(fname,api.root,api.root_len)) { // make relative to waves/ if possible
			if(!wcsncmp(fname+api.root_len, L"\\waves\\", 7)) {
				memmove(fname, fname+api.root_len+7, ((wcslen(fname)-api.root_len-7+1) * sizeof fname[0]));
			}
		}
		return 1;
	}
	return 0;
}

BOOL IsMMFile(const wchar_t* fname)
{
	wchar_t extlist[1024], *ext;
	if(!wcscasecmp(fname,L"cdaudio") || !ext_cmp(fname,L"pcb"))
		return 1;
	GetProfileString(L"mci extensions", NULL, L"", extlist, _countof(extlist));
	for(ext=extlist; *ext; ++ext) {
		if(!ext_cmp(fname,ext))
			return 1;
		while(*++ext);
	}
	return 0;
}
