//===============================================================================
//--+++--> alarm.c - KAZUBON 1997-1999 ===========================================
//--+++--> Sound a wave, a media file, or open a file =============================
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->

static WAVEHDR wh;
static HPSTR pData = NULL;
static HWAVEOUT hWaveOut = NULL;
static WAVEFORMATEX* pFormat = NULL;

static int countPlay = 0, countPlayNum = 0;
static BOOL bMCIPlaying = FALSE;
BOOL bPlayingNonstop = FALSE;
static BOOL bTrack;
static int nTrack;

static int maxAlarm = 1;
static PALARMSTRUCT pAS = NULL;
static BOOL bJihou, bJihouRepeat, bJihouBlink;

typedef struct _stPCBEEP {
	HWND hWnd;
	char szFname[MAX_BUFF];
	DWORD dwLoops;
} PCBEEP, *lpPCBEEP;

static PCBEEP pcb;

BOOL bKillPCBeep = TRUE;

BOOL PlayWave(HWND hwnd, char* fname, DWORD dwLoops);
int PlayMCI(HWND hwnd, int nt);
void StopWave(void);
//================================================================================================
//-------------------------------------------------+++--> Load Configured Alarm Data From Registry:
void InitAlarm(void)   //-------------------------------------------------------------------+++-->
{
	maxAlarm = GetMyRegLong("", "AlarmNum", 0);
	if(maxAlarm < 1) maxAlarm = 0;
	if(pAS) free(pAS); pAS = NULL;
	if(maxAlarm > 0) {
		int i;
		pAS = malloc(sizeof(ALARMSTRUCT) * maxAlarm);
		for(i = 0; i < maxAlarm; i++) {
			char subkey[20];
			wsprintf(subkey, "Alarm%d", i + 1);
			GetMyRegStr(subkey, "Name", pAS[i].name, 40, "");
			pAS[i].bAlarm = GetMyRegLong(subkey, "Alarm", FALSE);
			pAS[i].hour = GetMyRegLong(subkey, "Hour", 12);
			pAS[i].minute = GetMyRegLong(subkey, "Minute", 0);
			GetMyRegStr(subkey, "File", pAS[i].fname, MAX_BUFF, "");
			
			GetMyRegStr(subkey, "jrMessage", pAS[i].jrMessage, MAX_BUFF, "");
			GetMyRegStr(subkey, "jrSettings", pAS[i].jrSettings, TNY_BUFF, "3,4,3,90,42,1");
			pAS[i].jrMsgUsed = GetMyRegLong(subkey, "jrMsgUsed", FALSE);
			
			pAS[i].bHour12 = GetMyRegLong(subkey, "Hour12", TRUE);
			pAS[i].bChimeHr = GetMyRegLong(subkey, "ChimeHr", FALSE);
			pAS[i].bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
			pAS[i].iTimes = GetMyRegLong(subkey, "Times", 1);
			pAS[i].bBlink = GetMyRegLong(subkey, "Blink", FALSE);
			pAS[i].days = GetMyRegLong(subkey, "Days", 0x7f);
			pAS[i].bPM = GetMyRegLong(subkey, "PM", FALSE);
		}
	}
	
	bJihou = GetMyRegLong("", "Jihou", FALSE);
	if(bJihou) {
		bJihouRepeat = GetMyRegLong("", "JihouRepeat", FALSE);
		bJihouBlink = GetMyRegLong("", "JihouBlink", FALSE);
	}
}
//================================================================================================
//---------------------//--------------------------------------+++--> Shut Off the God Damn Siren!:
void EndAlarm(void)   //--------------------------------------------------------------------+++-->
{
	if(pAS) free(pAS); pAS = NULL;
	StopFile();
}
//============================================== This Code Assumes...	===========================
//-----------------------------------------+++--> 12pm = Noon <--> However (See Below for Details):
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st)   // 12am = Midnight --------------------------+++-->
{
	int i, rep, h, fday;
	
	if(st->wDayOfWeek > 0) fday = 1 << (st->wDayOfWeek - 1);
	else fday = 1 << 6;
	
	for(i = 0; i < maxAlarm; i++) {
		if(!pAS[i].bAlarm) continue;
		h = st->wHour;
		
		if(pAS[i].bHour12 && pAS[i].bPM) {
			if(h != 12) h -= 12;
		}
		
		if(pAS[i].hour == h && pAS[i].minute == st->wMinute && (pAS[i].days & fday)) {
			if(pAS[i].bBlink) PostMessage(g_hwndClock, CLOCKM_BLINK, FALSE, 0);
			if(pAS[i].jrMsgUsed) { // From BounceWindow.c
				extern char szCaption[TNY_BUFF];  // Alarm Name
				extern char szMessage[MAX_BUFF];  // Window Text
				extern char szSettings[TNY_BUFF]; // Hop Settings
				
				strcpy(szCaption, pAS[i].name);
				strcpy(szMessage, pAS[i].jrMessage);
				strcpy(szSettings, pAS[i].jrSettings);
				PostMessage(hwnd, WM_COMMAND, JRMSG_BOING, 0);
			}
			if(pAS[i].fname[0]) {
				if(pAS[i].bRepeat && pAS[i].iTimes > 1) rep = pAS[i].iTimes; //-+> Ring X Times
				else if(pAS[i].bRepeat) rep = -1; //-+> Ring To Infinity > ∞ < Or Until Stopped
				else if(pAS[i].bChimeHr) rep = h; //-+> Ring the Hour
				else rep = 0;
				
				bKillPCBeep = FALSE;
				if(PlayFile(hwnd, pAS[i].fname, rep)) {
					EmptyWorkingSet(GetCurrentProcess());
					return; // ^^Return^^ Memory Used by File Play.
				}
			}
		}
	}
	
	if(bJihou && st->wMinute == 0) {
		char fname[MAX_BUFF];
		h = st->wHour;
		if(bJihouBlink) PostMessage(g_hwndClock, CLOCKM_BLINK, TRUE, 0);
		if(h == 0) h = 12;
		else if(h >= 13) h -= 12;
		GetMyRegStr("", "JihouFile", fname, GEN_BUFF, "");
		if(fname[0]) {
			if(bJihouRepeat) {
				rep = h; // Chime the Hour as Requested (If Requested)
			} else {
				rep = 0; // Ring Once & Go Away!
			}
			PlayFile(hwnd, fname, rep);
			EmptyWorkingSet(GetCurrentProcess());
		} // ^^Return^^ Memory Used by File Play.
	}
}
#include <stdio.h> //--------------------------+++--> Required Here Only for the Open File Stuff:
//================================================================================================
//-----+++--> For Computers WithOut Speakers, Play a NoSound (PC Beep) File Through the PC Speaker:
BOOL PlayNoSound(char* fname, DWORD iLoops)   //-----------------------------------+++-->
{
	static const char seps[] = ", \r\n";
	FILE* file;
	
	if(fopen_s(&file, fname, "r") > 0) {
		bKillPCBeep = TRUE;
		return FALSE;
	} else {
		char* szToken,*nxToken;
		while(!bKillPCBeep) {
			// If We Have a Line, Play Its Beep!
			char szTmp[TNY_BUFF];
			while(fgets(szTmp, TNY_BUFF, file)) {
				int iDur=0, iFeq=0, i = 0;
				szToken = strtok_s(szTmp, seps, &nxToken);
				while(szToken != NULL) {
					switch(i) {
					case 0: // Length of Beep
						iDur = atoi(szToken);
						break;
					case 1: // Frequency
						iFeq = atoi(szToken);
						break;
					default: break;
					}
					szToken = strtok_s(NULL, seps, &nxToken);
					i++;
				} // Get Next Beep Line.
				
				if(bKillPCBeep) {  // Just in case It's a Long File...
					fclose(file); // Check for Kill Code between Beeps.
					return TRUE;
				} else if(iFeq == -1) {
					Sleep(iDur);
				} else {
					Beep(iFeq, iDur);
				}
			} // END OF IF LINE
			
			if(iLoops > 0) { //-> IF We're Looping, Go Back to
				fseek(file, 0, SEEK_SET); // Beginning of File
				iLoops--;
			}
			if(iLoops == 0) { // Or Die, Exit, Quit, Close, End
				bKillPCBeep = TRUE;
				fclose(file);
				return TRUE;
			}
		} // END OF WHILE NOT KILL BEEP
		fclose(file); // Make Sure File Gets Closed on Early Exit.
		return TRUE;
	}
}
#include <process.h> // Required for Worker Thread Creation - So Clock Can Send Alarm Kill Code.
//===============================================================================================
//--+++-->
unsigned __stdcall PlayNoSoundProc(void* param)
{
	lpPCBEEP lppcb;
	lppcb = (lpPCBEEP)param;
	
	PlayNoSound(lppcb->szFname, lppcb->dwLoops);
	SendMessage(lppcb->hWnd, MM_WOM_DONE, 0, 0);
	
	_endthread();
	return 0;
}
//================================================================================================
//--+++-->
void PlayNoSoundThread(HWND hWnd, char* fname, DWORD dwLoops)
{
	strcpy_s(pcb.szFname, MAX_BUFF, fname);
	pcb.dwLoops = dwLoops;
	pcb.hWnd = hWnd;
	
	_beginthreadex(NULL, 0, PlayNoSoundProc, (void*)&pcb, 0, 0);
	
}
//=================================================*
// ------------------------ Play Selected Alarm File
//===================================================*
BOOL PlayFile(HWND hwnd, char* fname, DWORD dwLoops)
{

	if(*fname == 0) return FALSE;
	
	if(ext_cmp(fname, "wav") == 0) {
		if(bMCIPlaying) return FALSE;
		return PlayWave(hwnd, fname, dwLoops);
	}
	
	if(ext_cmp(fname, "pcb") == 0) {
		if(bMCIPlaying) return FALSE;
		bKillPCBeep = FALSE;
		PlayNoSoundThread(hwnd, fname, dwLoops);
		return TRUE;
	}
	
	else if(IsMMFile(fname)) {
		char command[GEN_BUFF];
		if(bMCIPlaying) return FALSE;
		strcpy(command, "open \"");
		strcat(command, fname);
		strcat(command, "\" alias myfile");
		if(mciSendString(command, NULL, 0, NULL) == 0) {
			strcpy(command, "set myfile time format ");
			if(_stricmp(fname, "cdaudio") == 0 || ext_cmp(fname, "cda") == 0) {
				strcat(command, "tmsf"); bTrack = TRUE;
			} else {
				strcat(command, "milliseconds"); bTrack = FALSE;
			}
			mciSendString(command, NULL, 0, NULL);
			
			nTrack = -1;
			if(ext_cmp(fname, "cda") == 0) {
				char* p;
				p = fname; nTrack = 0;
				while(*p) {
					if('0' <= *p && *p <= '9') nTrack = nTrack * 10 + *p - '0';
					p++;
				}
			}
			
			if(PlayMCI(hwnd, nTrack) == 0) {
				bMCIPlaying = TRUE;
				countPlay = 1; countPlayNum = dwLoops;
			} else mciSendString("close myfile", NULL, 0, NULL);
		} return bMCIPlaying;
	} else ExecFile(hwnd, fname);
	return FALSE;
}
//=================================================*
// ------------ Tie Into the Media Control Interface
//===================================================*
int PlayMCI(HWND hwnd, int nt)
{
	char command[80];
	
	strcpy(command, "play myfile");
	if(nt >= 0) {
		char s[80],start[40],end[40];
		wsprintf(s, "status myfile position track %d", nt);
		if(mciSendString(s, start, 40, NULL) == 0) {
			strcat(command, " from ");
			strcat(command, start);
			wsprintf(s, "status myfile position track %d", nt+1);
			if(mciSendString(s, end, 40, NULL) == 0) {
				strcat(command, " to ");
				strcat(command, end);
			}
		}
	}
	strcat(command, " notify");
	return mciSendString(command, NULL, 0, hwnd);
}
//=================================================*
// ----------------------- Stop Playing If/When Told
//===================================================*
void StopFile(void)
{
	bKillPCBeep = TRUE;
	StopWave();
	if(bMCIPlaying) {
		mciSendString("stop myfile", NULL, 0, NULL);
		mciSendString("close myfile", NULL, 0, NULL);
		bMCIPlaying = FALSE;
		countPlay = 0; countPlayNum = 0;
	}
	bPlayingNonstop = FALSE;
}
//=================================================*
// ----------------------------- Loop Play as Needed
//===================================================*
void OnMCINotify(HWND hwnd)
{
	if(bMCIPlaying) {
		if(countPlay < countPlayNum || countPlayNum < 0) {
			mciSendString("seek myfile to start wait", NULL, 0, NULL);
			if(PlayMCI(hwnd, nTrack) == 0) {
				countPlay++;
			} else StopFile();
		} else StopFile();
	}
}
/*-------------------------------------------------------------------
--------------- Retreive a file name and option from a command string
-------------------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* opt)
{
	const char* p, *pe;	char* pd;
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	p = command; pd = fname;
	pe = NULL;
	for(; ;) {
		if(*p == ' ' || *p == 0) {
			*pd = 0;
			hfind = FindFirstFile(fname, &fd);
			if(hfind != INVALID_HANDLE_VALUE) {
				FindClose(hfind);
				pe = p;
			}
			if(*p == 0) break;
		}
		*pd++ = *p++;
	}
	
	if(pe == NULL) pe = p;
	
	p = command; pd = fname;
	for(; p != pe;) {
		*pd++ = *p++;
	}
	*pd = 0;
	if(*p == ' ') p++;
	
	pd = opt;
	for(; *p;) *pd++ = *p++;
	*pd = 0;
}
/*--------------------------------------------------
 --------------------------------------- Open a file
--------------------------------------------------*/
BOOL ExecFile(HWND hwnd, char* command)
{
	char fname[MAX_PATH], opt[MAX_PATH];
	
	if(*command){
		GetFileAndOption(command,fname,opt);
		if((size_t)ShellExecute(hwnd,NULL,fname,*opt?opt:NULL,NULL,SW_SHOW)>32) return TRUE;
	}
	return FALSE;
}
//================================================================================================
//------------------------------------------------------//----+++--> Load & Play a Wave Audio File:
BOOL PlayWave(HWND hwnd, char* fname, DWORD dwLoops)   //-----------------------------------+++-->
{
	MMCKINFO mmckinfoSubchunk;
	MMCKINFO mmckinfoParent;
	LONG lDataSize;
	LONG lFmtSize;
	HMMIO hmmio;
	
	if(hWaveOut != NULL) return FALSE;
	
	if((hmmio=mmioOpen(fname, NULL, MMIO_READ|MMIO_ALLOCBUF))==0)
		return FALSE;
		
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
				   MMIO_FINDCHUNK)) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lFmtSize = mmckinfoSubchunk.cksize;
	pFormat = (WAVEFORMATEX*)malloc(lFmtSize);
	if(pFormat == NULL) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, (HPSTR)pFormat, lFmtSize) != lFmtSize) {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMATEX)pFormat,
				   0, 0, (DWORD)WAVE_FORMAT_QUERY)) {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);
	
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
				   MMIO_FINDCHUNK)) {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lDataSize = mmckinfoSubchunk.cksize;
	if(lDataSize == 0) {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	pData = (HPSTR)malloc(lDataSize);
	if(pData == NULL) {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, pData, lDataSize) != lDataSize) {
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	mmioClose(hmmio, 0);
	
	if(waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
				   (LPWAVEFORMATEX)pFormat, (DWORD_PTR)(HWND)hwnd, 0,
				   (DWORD)CALLBACK_WINDOW)) {
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}
	
	memset(&wh, 0, sizeof(WAVEHDR));
	wh.dwBufferLength = lDataSize;
	wh.lpData = pData;
	if(dwLoops != 0) {
		wh.dwFlags = WHDR_BEGINLOOP|WHDR_ENDLOOP;
		wh.dwLoops = dwLoops;
	}
	if(waveOutPrepareHeader(hWaveOut, &wh, sizeof(WAVEHDR))) {
		waveOutClose(hWaveOut); hWaveOut = NULL;
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}
	
	if(waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR)) != 0) {
		waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
		waveOutClose(hWaveOut);	hWaveOut = NULL;
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}
	
	return TRUE;
}
//================================================================================================
//---------------------//---------------------------------------+++--> End Play of Wave Audio File:
void StopWave(void)   //--------------------------------------------------------------------+++-->
{
	if(hWaveOut == NULL) return;
	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
	hWaveOut = NULL;
	free(pFormat); pFormat = NULL;
	free(pData); pData = NULL;
}
//===============================================================================*
// -------------------------------------------------------------- The NIST Says...
//=================================================================================*
/*
From the National Institute of Standards and Technology - Time & Frequency Division

Q. Are noon and midnight 12 a.m. or 12 p.m.?

A. This is a tricky question. The answer is that the terms 12 a.m. and 12 p.m. are wrong
and should not be used.

To illustrate this, consider that "a.m" and "p.m." are abbreviations for "ante meridiem"
and "post meridiem." They mean "before noon" and "after noon," respectively. Noon is neither
before or after noon; it is simply noon. Therefore, neither the "a.m." nor "p.m." designation
is correct. On the other hand, midnight is both 12 hours before noon and 12 hours after noon.
Therefore, either 12 a.m. or 12 p.m. could work as a designation for midnight, but both would
be ambiguous as to the date intended.

When a specific date is important, and when we can use a 24-hour clock, we prefer to designate
that moment not as 1200 midnight, but rather as 0000 if we are referring to the beginning of a
given day (or date), or 2400 if we are designating the end of a given day (or date).

To be certain of avoiding ambiguity (while still using a 12-hour clock), specify an event as
beginning at 1201 a.m. or ending at 1159 p.m., for example; this method is used by the railroads
and airlines for schedules, and is often found on legal papers such as contracts and insurance
policies.

If one is referring not to a specific date, but rather to several days, or days in general, use
the terms noon and midnight instead of 12 a.m. and 12 p.m. For example, a bank might be open on
Saturdays from 8 a.m. to noon. Or a grocery store might be open daily until midnight. The terms
"12 noon" and "12 midnight" are also correct, though redundant.
*/
