//===============================================================================
//--+++--> alarm.c - KAZUBON 1997-1999 ===========================================
//--+++--> Sound a wave, a media file, or open a file =============================
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->
#define MAX_ALARM 999999
static char g_alarmkey[12]="Alarm";
#define ALARMKEY_OFFSET 5

static WAVEHDR m_wh;
static HPSTR m_pData = NULL;
static HWAVEOUT m_hWaveOut = NULL;
static WAVEFORMATEX* m_pFormat = NULL;

static DWORD m_countPlay=0;
static BOOL m_bMCIPlaying = FALSE;
char g_bPlayingNonstop = 0;
static BOOL m_bTrack;
static int m_nTrack;

static int m_maxAlarm = 1;
static alarm_t* m_pAS = NULL;
static BOOL m_bJihou, m_bJihouRepeat, m_bJihouBlink;

typedef struct _stPCBEEP {
	HWND hWnd;
	char szFname[MAX_BUFF];
	DWORD dwLoops;
} PCBEEP;

static PCBEEP m_pcb;
static volatile char m_bKillPCBeep = 1;

BOOL PlayWave(HWND hwnd, char* fname, DWORD dwLoops);
int PlayMCI(HWND hwnd, int nt);
void StopWave(void);

BOOL GetHourlyChime(){
	return m_bJihou;
}
void SetHourlyChime(BOOL bEnabled){
	m_bJihou=bEnabled;
	api.SetInt("","Jihou",m_bJihou);
}
char GetAlarmEnabled(int idx){
	if(idx<0||idx>=m_maxAlarm)
		return 0;
	return m_pAS[idx].uFlags&ALRM_ENABLED;
}
void SetAlarmEnabled(int idx,char bEnabled){
	if(idx<0||idx>=m_maxAlarm)
		return;
	if(bEnabled) m_pAS[idx].uFlags |= ALRM_ENABLED;
	else m_pAS[idx].uFlags &= ~ALRM_ENABLED;
	wsprintf(g_alarmkey+ALARMKEY_OFFSET, "%d", idx+1);
	api.SetInt(g_alarmkey, "Alarm", m_pAS[idx].uFlags&ALRM_ENABLED);
}
int GetAlarmNum()
{
	int num = api.GetInt("", "AlarmNum", 0);
	if(num < 0)
		num = 0;
	if(num > MAX_ALARM)
		num = MAX_ALARM;
	return num;
}
void SetAlarmNum(int num)
{
	if(num > MAX_ALARM)
		num = MAX_ALARM;
	api.SetInt("", "AlarmNum", num);
}
void ReadAlarmFromReg(alarm_t* pAS, int idx)
{
	if(idx >= MAX_ALARM)
		return;
	wsprintf(g_alarmkey+ALARMKEY_OFFSET, "%d", idx+1);
	api.GetStr(g_alarmkey, "Name", pAS->dlgmsg.name, sizeof(pAS->dlgmsg.name), "");
	pAS->hour = api.GetInt(g_alarmkey, "Hour", 12) % 24;
	pAS->minute = api.GetInt(g_alarmkey, "Minute", 0);
	pAS->days = api.GetInt(g_alarmkey, "Days", 0x7f);
	pAS->iTimes = api.GetInt(g_alarmkey, "Times", 1);
	
	pAS->uFlags=0;
	if(api.GetInt(g_alarmkey,"Alarm",0)) pAS->uFlags|=ALRM_ENABLED;
	if(api.GetInt(g_alarmkey,"Once",0)) pAS->uFlags|=ALRM_ONESHOT;
	if(api.GetInt(g_alarmkey,"12h",0)) pAS->uFlags|=ALRM_12H;
	if(api.GetInt(g_alarmkey,"ChimeHr",0)) pAS->uFlags|=ALRM_CHIMEHR;
	if(api.GetInt(g_alarmkey,"Repeat",0)) pAS->uFlags|=ALRM_REPEAT;
	if(api.GetInt(g_alarmkey,"Blink",0)) pAS->uFlags|=ALRM_BLINK;
	if(api.GetInt(g_alarmkey,"jrMsgUsed",0)) pAS->uFlags|=ALRM_DIALOG;
	
	api.GetStr(g_alarmkey, "File", pAS->fname, sizeof(pAS->fname), "");
	
	api.GetStr(g_alarmkey, "jrMessage", pAS->dlgmsg.message, sizeof(pAS->dlgmsg.message), "");
	api.GetStr(g_alarmkey, "jrSettings", pAS->dlgmsg.settings, sizeof(pAS->dlgmsg.settings), "");
	
	if(!pAS->dlgmsg.name[0])
		wsprintf(pAS->dlgmsg.name, "%02d:%02d", pAS->hour, pAS->minute);
}
void SaveAlarmToReg(alarm_t* pAS, int idx)
{
	if(idx >= MAX_ALARM)
		return;
	wsprintf(g_alarmkey+ALARMKEY_OFFSET, "%d", idx+1);
	api.SetStr(g_alarmkey, "Name", pAS->dlgmsg.name);
	api.SetInt(g_alarmkey, "Hour", pAS->hour);
	api.SetInt(g_alarmkey, "Minute", pAS->minute);
	api.SetStr(g_alarmkey, "File", pAS->fname);
	
	api.SetStr(g_alarmkey, "jrMessage", pAS->dlgmsg.message);
	api.SetStr(g_alarmkey, "jrSettings", pAS->dlgmsg.settings);
	
	api.SetInt(g_alarmkey, "Days", pAS->days);
	api.SetInt(g_alarmkey, "Times", pAS->iTimes);
	
	api.SetInt(g_alarmkey,"Alarm",pAS->uFlags&ALRM_ENABLED);
	api.SetInt(g_alarmkey,"Once",pAS->uFlags&ALRM_ONESHOT);
	api.SetInt(g_alarmkey,"12h",pAS->uFlags&ALRM_12H);
	api.SetInt(g_alarmkey,"ChimeHr",pAS->uFlags&ALRM_CHIMEHR);
	api.SetInt(g_alarmkey,"Repeat",pAS->uFlags&ALRM_REPEAT);
	api.SetInt(g_alarmkey,"Blink",pAS->uFlags&ALRM_BLINK);
	api.SetInt(g_alarmkey,"jrMsgUsed",pAS->uFlags&ALRM_DIALOG);
}
int DeleteAlarmFromReg(int idx)
{
	if(idx >= MAX_ALARM)
		return 1;
	wsprintf(g_alarmkey+ALARMKEY_OFFSET, "%d", idx+1);
	if(api.GetInt(g_alarmkey, "Hour", -1) == -1)
		return 1;
	api.DelKey(g_alarmkey);
	return 0;
}

//=============================================================================================
//----------------------------------------------+++--> Load Configured Alarm Data From Registry:
void InitAlarm()   //--------------------------------------------------------------------+++-->
{
	m_maxAlarm = GetAlarmNum();
	if(m_pAS)
		free(m_pAS);
	m_pAS = NULL;
	if(m_maxAlarm > 0) {
		int i;
		m_pAS = malloc(sizeof(alarm_t) * m_maxAlarm);
		for(i=0; i<m_maxAlarm; ++i) {
			ReadAlarmFromReg(&m_pAS[i],i);
		}
	}
	
	m_bJihou = api.GetInt("", "Jihou", FALSE);
	if(m_bJihou) {
		m_bJihouRepeat = api.GetInt("", "JihouRepeat", FALSE);
		m_bJihouBlink = api.GetInt("", "JihouBlink", FALSE);
	}
}
//================================================================================================
//---------------------//--------------------------------------+++--> Shut Off the God Damn Siren!:
void EndAlarm(void)   //--------------------------------------------------------------------+++-->
{
	StopFile();
	if(m_pAS) free(m_pAS); m_pAS = NULL;
}
//============================================== This Code Assumes...	===========================
//-----------------------------------------+++--> 12pm = Noon <--> However (See Below for Details):
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st)   // 12am = Midnight --------------------------+++-->
{
	int i, rep, fday;
	
	if(st->wDayOfWeek > 0) fday = 1 << (st->wDayOfWeek - 1);
	else fday = 1 << 6;
	
	for(i = 0; i < m_maxAlarm; ++i) {
		if(!(m_pAS[i].uFlags&ALRM_ENABLED)) continue;
		
		if(m_pAS[i].hour == st->wHour && m_pAS[i].minute == st->wMinute && (m_pAS[i].days & fday)) {
			if(m_pAS[i].uFlags&ALRM_ONESHOT)
				SetAlarmEnabled(i,0);
			if(m_pAS[i].uFlags&ALRM_BLINK)
				PostMessage(g_hwndClock,CLOCKM_BLINK,FALSE,0);
			if(m_pAS[i].uFlags&ALRM_DIALOG) // From BounceWindow.c
				ReleaseTheHound(hwnd,m_pAS[i].dlgmsg.name,m_pAS[i].dlgmsg.message,m_pAS[i].dlgmsg.settings);
			if(m_pAS[i].fname[0]) {
				if(m_pAS[i].uFlags&ALRM_REPEAT){
					if(m_pAS[i].iTimes > 1)
						rep = m_pAS[i].iTimes;
					else // -1 or 0 (we might change the later one)
						rep = -1; // until stopped
				}else if(m_pAS[i].uFlags&ALRM_CHIMEHR){ // chime hour
					rep = st->wHour;
					if(rep == 0) rep = 12;
					else if(rep > 12) rep -= 12;
					--rep;
				}else{
					rep = 0;
				}
				PlayFile(hwnd, m_pAS[i].fname, rep);
				#ifndef _DEBUG
				EmptyWorkingSet(GetCurrentProcess());
				#endif
			}
		}
	}
	
	if(m_bJihou && st->wMinute == 0) {
		char fname[MAX_PATH];
		if(m_bJihouBlink) PostMessage(g_hwndClock, CLOCKM_BLINK, TRUE, 0);
		api.GetStr("", "JihouFile", fname, sizeof(fname), "");
		if(fname[0]) {
			if(m_bJihouRepeat) { // chime hour
				rep = st->wHour;
				if(rep == 0) rep = 12;
				else if(rep > 12) rep -= 12;
				--rep;
			} else {
				rep = 0;
			}
			PlayFile(hwnd, fname, rep);
			#ifndef _DEBUG
			EmptyWorkingSet(GetCurrentProcess());
			#endif
		}
	}
}
#include <stdio.h> //--------------------------+++--> Required Here Only for the Open File Stuff:
//================================================================================================
//-----+++--> For Computers WithOut Speakers, Play a NoSound (PC Beep) File Through the PC Speaker:
BOOL PlayNoSound(char* fname, DWORD iLoops)   //-----------------------------------+++-->
{
	static const char seps[] = ", \r\n";
	char* szToken,*nxToken;
	FILE* file = fopen(fname, "r");
	if(!file) {
		m_bKillPCBeep = 1;
		return FALSE;
	}
	while(!m_bKillPCBeep) {
		// If We Have a Line, Play Its Beep!
		char szTmp[TNY_BUFF];
		while(fgets(szTmp, sizeof(szTmp), file)) {
			unsigned duration=10, freq=0, i;
			szToken = strtok_s(szTmp, seps, &nxToken);
			for(i=0; szToken; ++i) {
				int num = atoi(szToken);
				switch(i) {
				case 0: // Length of Beep
					if(num < 10) duration = 10;
					else duration = num;
					break;
				case 1: // Frequency
					if(num < 0) freq = 0;
					else freq = num;
					break;
				}
				szToken = strtok_s(NULL, seps, &nxToken);
			} // Get Next Beep Line.
			if(duration > 3600000) // don't allow a duration longer than 1 hour
				duration = 3600000;
			if(m_bKillPCBeep) {  // Just in case It's a Long File...
				fclose(file); // Check for Kill Code between Beeps.
				return TRUE;
			} else if(!freq) {
				Sleep(duration);
			} else {
				Beep(freq, duration);
			}
		} // END OF IF LINE
		
		if(iLoops > 0) { //-> IF We're Looping, Go Back to
			if(ftell(file) <= 0) // empty file, don't do an "idle loop" (high CPU usage)
				Sleep(100);
			fseek(file, 0, SEEK_SET); // Beginning of File
			--iLoops;
		}else{ // Or Die, Exit, Quit, Close, End
			m_bKillPCBeep = 1;
			fclose(file);
			return TRUE;
		}
	} // END OF WHILE NOT KILL BEEP
	fclose(file); // Make Sure File Gets Closed on Early Exit.
	return TRUE;
}
#include <process.h> // Required for Worker Thread Creation - So Clock Can Send Alarm Kill Code.
//===============================================================================================
//--+++-->
unsigned __stdcall PlayNoSoundProc(void* param)
{
	PCBEEP* lppcb;
	lppcb = (PCBEEP*)param;
	
	PlayNoSound(lppcb->szFname, lppcb->dwLoops);
	SendMessage(lppcb->hWnd, MM_WOM_DONE, 0, 0);
	
	_endthread();
	return 0;
}
//================================================================================================
//--+++-->
void PlayNoSoundThread(HWND hWnd, char* fname, DWORD dwLoops)
{
	strncpy_s(m_pcb.szFname, sizeof(m_pcb.szFname), fname, _TRUNCATE);
	m_pcb.dwLoops = dwLoops;
	m_pcb.hWnd = hWnd;
	
	m_bKillPCBeep = 0;
	_beginthreadex(NULL, 0, PlayNoSoundProc, (void*)&m_pcb, 0, 0);
}
//=================================================*
// ------------------------ Play Selected Alarm File
//===================================================*
BOOL PlayFile(HWND hwnd, char* fname, DWORD dwLoops)
{
	if(!fname[0] || fname[0]=='<') return FALSE;
	if(fname[0]!='/' && fname[0]!='\\' && fname[1]!=':' // no abs path
	&&(fname[0]!='.' || (fname[1]!='/' && fname[1]!='\\' && (fname[1]!='.' ||  (fname[2]!='/' && fname[2]!='\\'))))) // no relative path (strict relative)
	{ // do it relative to "waves/"
		char app[MAX_PATH];
		const size_t tlen=api.root_len;
		const size_t len=strlen(fname)+1; // incl. terminating null
		if(len<MAX_PATH-tlen-7){
			memmove(fname+tlen+7/* <mydir>/waves/ */,fname,len);
			memcpy(fname,api.root,tlen); /// absolute path is at least required by .pcb / PlayNoSoundThread (not for .wav)
			memcpy(fname+tlen,"\\waves\\",7);
			if(api.GetFileAndOption(fname, app, NULL) != 0){
				memmove(fname,fname+tlen+7,len); // restore relative
			}
		}
	}
	
	if(ext_cmp(fname, "wav") == 0) {
		if(m_bMCIPlaying) return FALSE;
		return PlayWave(hwnd, fname, dwLoops);
	}
	
	if(ext_cmp(fname, "pcb") == 0) {
		if(m_bMCIPlaying) return FALSE;
		PlayNoSoundThread(hwnd, fname, dwLoops);
		return TRUE;
	}
	
	else if(IsMMFile(fname)) {
		char command[MIN_BUFF+MAX_PATH];
		if(m_bMCIPlaying) return FALSE;
		strcpy(command, "open \"");
		strcat(command, fname);
		strcat(command, "\" alias myfile");
		if(mciSendString(command, NULL, 0, NULL) == 0) {
			strcpy(command, "set myfile time format ");
			if(strcasecmp(fname, "cdaudio") == 0 || ext_cmp(fname, "cda") == 0) {
				strcat(command, "tmsf"); m_bTrack = TRUE;
			} else {
				strcat(command, "milliseconds"); m_bTrack = FALSE;
			}
			mciSendString(command, NULL, 0, NULL);
			
			m_nTrack = -1;
			if(ext_cmp(fname, "cda") == 0) {
				char* p;
				p = fname; m_nTrack = 0;
				while(*p) {
					if('0' <= *p && *p <= '9') m_nTrack = m_nTrack * 10 + *p - '0';
					p++;
				}
			}
			
			if(PlayMCI(hwnd, m_nTrack) == 0) {
				m_bMCIPlaying = TRUE;
				m_countPlay = dwLoops;
			} else mciSendString("close myfile", NULL, 0, NULL);
		} return m_bMCIPlaying;
	} else api.ExecFile(fname,hwnd);
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
		char cmdpos[64],out[32];
		wsprintf(cmdpos, "status myfile position track %d", nt);
		if(mciSendString(cmdpos, out, sizeof(out), NULL) == 0) {
			strcat(command, " from ");
			strcat(command, out);
			wsprintf(cmdpos+29, "%d", nt+1); // status myfile position track XXX
			if(mciSendString(cmdpos, out, sizeof(out), NULL) == 0) {
				strcat(command, " to ");
				strcat(command, out);
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
	m_bKillPCBeep = 1;
	StopWave();
	if(m_bMCIPlaying) {
		mciSendString("stop myfile", NULL, 0, NULL);
		mciSendString("close myfile", NULL, 0, NULL);
		m_bMCIPlaying = FALSE;
		m_countPlay = 0;
	}
	g_bPlayingNonstop = 0;
}
int IsPlaying()
{
	return m_hWaveOut || m_bMCIPlaying || !m_bKillPCBeep;
}
//=================================================*
// ----------------------------- Loop Play as Needed
//===================================================*
int OnMCINotify(HWND hwnd)
{
	if(m_bMCIPlaying) {
		if(m_countPlay) {
			mciSendString("seek myfile to start wait", NULL, 0, NULL);
			if(PlayMCI(hwnd, m_nTrack) == 0) {
				--m_countPlay;
				return 1;
			}
		}
		StopFile();
	}
	return 0;
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
	
	if(m_hWaveOut != NULL) return FALSE;
	
	if((hmmio=mmioOpen(fname, NULL, MMIO_READ|MMIO_ALLOCBUF))==0)
		return FALSE;
		
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(hmmio, &mmckinfoParent, NULL, MMIO_FINDRIFF)) {
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
	m_pFormat = (WAVEFORMATEX*)malloc(lFmtSize);
	if(m_pFormat == NULL) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, (HPSTR)m_pFormat, lFmtSize) != lFmtSize) {
		free(m_pFormat); m_pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(waveOutOpen(&m_hWaveOut, WAVE_MAPPER, m_pFormat,
				   0, 0, WAVE_FORMAT_QUERY)) {
		free(m_pFormat); m_pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);
	
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
				   MMIO_FINDCHUNK)) {
		free(m_pFormat); m_pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lDataSize = mmckinfoSubchunk.cksize;
	if(lDataSize == 0) {
		free(m_pFormat); m_pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	m_pData = (HPSTR)malloc(lDataSize);
	if(m_pData == NULL) {
		free(m_pFormat); m_pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, m_pData, lDataSize) != lDataSize) {
		free(m_pFormat); m_pFormat = NULL;
		free(m_pData); m_pData = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	mmioClose(hmmio, 0);
	
	if(waveOutOpen(&m_hWaveOut, WAVE_MAPPER,
				   m_pFormat, (DWORD_PTR)hwnd, 0,
				   CALLBACK_WINDOW)) {
		free(m_pFormat); m_pFormat = NULL;
		free(m_pData); m_pData = NULL;
		return FALSE;
	}
	
	memset(&m_wh, 0, sizeof(WAVEHDR));
	m_wh.dwBufferLength = lDataSize;
	m_wh.lpData = m_pData;
	if(dwLoops != 0) {
		if(dwLoops!=0xFFFFFFFF) ++dwLoops;
		m_wh.dwFlags = WHDR_BEGINLOOP|WHDR_ENDLOOP;
		m_wh.dwLoops = dwLoops;
	}
	if(waveOutPrepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR))) {
		waveOutClose(m_hWaveOut); m_hWaveOut = NULL;
		free(m_pFormat); m_pFormat = NULL;
		free(m_pData); m_pData = NULL;
		return FALSE;
	}
	
	if(waveOutWrite(m_hWaveOut, &m_wh, sizeof(WAVEHDR)) != 0) {
		waveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));
		waveOutClose(m_hWaveOut);	m_hWaveOut = NULL;
		free(m_pFormat); m_pFormat = NULL;
		free(m_pData); m_pData = NULL;
		return FALSE;
	}
	
	return TRUE;
}
//================================================================================================
//---------------------//---------------------------------------+++--> End Play of Wave Audio File:
void StopWave(void)   //--------------------------------------------------------------------+++-->
{
	if(!m_hWaveOut) return;
	waveOutReset(m_hWaveOut);
	waveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));
	waveOutClose(m_hWaveOut);
	m_hWaveOut = NULL;
	free(m_pFormat); m_pFormat = NULL;
	free(m_pData); m_pData = NULL;
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
