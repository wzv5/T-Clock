//===============================================================================
//--+++--> alarm.c - KAZUBON 1997-1999 ===========================================
//--+++--> Sound a wave, a media file, or open a file =============================
//================= Last Modified by Stoic Joker: Wednesday, 12/22/2010 @ 11:29:24pm
#include "tclock.h" //---------------{ Stoic Joker 2006-2010 }---------------+++-->
#include <time.h>
static char g_alarmkey[12] = "Alarm";
#define ALARMKEY_OFFSET 5

static Schedule* timetable_begin_ = NULL;
static Schedule* timetable_end_ = NULL;

static time_t AlarmNextTimestamp();
static void AlarmGetMSG(dlgmsg_t* msg);

static void TimetableClean() {
	Schedule* iter = timetable_begin_;
	timetable_begin_ = timetable_end_ = NULL;
	
	while(iter) {
		Schedule* tmp = iter;
		iter = iter->next;
		free(tmp);
	}
	return;
}
Schedule* TimetableAdd(int id, time_t ts, unsigned data) {
	Schedule* schedule;
	schedule = TimetableSearchID(id);
	if(schedule) {
		TimetableQueue(schedule, 0);
	} else {
		schedule = (Schedule*)malloc(sizeof(Schedule));
		if(!schedule)
			return NULL;
		schedule->id = id;
		if(ts <= 86400)
			ts = time(NULL) + ts;
	}
	if(ts > 86400)
		schedule->time = ts;
	schedule->data = data;
	TimetableQueue(schedule, 1);
	return schedule;
}
void TimetableRemove(int id) {
	Schedule* schedule = TimetableSearchID(id);
	if(schedule) {
		TimetableQueue(schedule, 0);
		free(schedule);
	}
}
void TimetableQueue(Schedule* alert, int add) {
	if(add) {
		Schedule* iter = timetable_end_;
		for(;;) {
			if(!iter) {
				alert->prev = NULL;
				alert->next = timetable_begin_;
				timetable_begin_ = alert;
				if(alert->next)
					alert->next->prev = alert;
				else
					timetable_end_ = alert;
				break;
			}
			if(alert->time >= iter->time) {
				alert->prev = iter;
				alert->next = iter->next;
				if(alert->prev)
					alert->prev->next = alert;
				else
					timetable_begin_ = alert;
				if(alert->next)
					alert->next->prev = alert;
				else
					timetable_end_ = alert;
			}
			iter = iter->prev;
		}
	} else {
		if(alert->prev)
			alert->prev->next = alert->next;
		else
			timetable_begin_ = alert->next;
		if(alert->next)
			alert->next->prev = alert->prev;
		else
			timetable_end_ = alert->prev;
	}
}
Schedule* TimetableSearchID(int id) {
	Schedule* iter = timetable_end_;
	for(; iter && iter->id!=id; iter=iter->prev);
	return iter;
}

static WAVEHDR m_wh;
static HPSTR m_pData = NULL;
static HWAVEOUT m_hWaveOut = NULL;
static WAVEFORMATEX* m_pFormat = NULL;

static DWORD m_countPlay=0;
static BOOL m_bMCIPlaying = FALSE;
char g_bPlayingNonstop = 0;
static BOOL m_bTrack;
static int m_nTrack;

static int m_maxAlarm = 0;

typedef struct _stPCBEEP {
	HWND hWnd;
	DWORD dwLoops;
	char szFname[MAX_PATH];
} PCBEEP;

static volatile char m_bKillPCBeep = 1;

BOOL PlayWave(HWND hwnd, const char* fname, DWORD dwLoops);
int PlayMCI(HWND hwnd, int nt);
void StopWave(void);

void AlarmChimeEnable(int enable) {
	if(enable < 0) {
		int status = api.GetInt("", "Jihou", 0);
		if(enable == -1) {
			enable = !status;
			goto update_and_continue;
		}
		enable = status;
	} else {
update_and_continue:
		api.SetInt("", "Jihou", enable);
	}
	if(enable) {
		unsigned data = 0;
		time_t ts = time(NULL) - 1;
		ts += 3600 - (ts % 3600);
		if(api.GetInt("", "JihouRepeat", 0))
			data |= ALRM_CHIMEHR;
		if(api.GetInt("", "JihouBlink", 0))
			data |= ALRM_BLINK;
		TimetableAdd(SCHEDID_CHIME, ts, data);
	} else
		TimetableRemove(SCHEDID_CHIME);
}
void AlarmEnable(int idx, int enable) {
	Schedule* alarm;
	if(idx < 0 || idx >= m_maxAlarm)
		return;
	ltoa(idx+1, (g_alarmkey+ALARMKEY_OFFSET), 10);
	
	if(enable < 0) {
		int status = api.GetInt(g_alarmkey, "Alarm", 0);
		if(enable == -1) {
			enable = !status;
			goto update_and_continue;
		}
		enable = status;
		alarm = NULL;
	} else {
update_and_continue:
		api.SetInt(g_alarmkey, "Alarm", enable);
		alarm = TimetableSearchID(idx);
	}
	if(!enable) {
		if(alarm) {
			TimetableQueue(alarm, 0);
			free(alarm);
		}
		return;
	}
	if(!alarm) {
		alarm = (Schedule*)malloc(sizeof(Schedule));
		if(!alarm)
			return;
		alarm->time = AlarmNextTimestamp();
		alarm->id = idx;
		/// @note : on next backward incompatible change, store a single value including every "flag"
		alarm->data = 0;
		if(api.GetInt(g_alarmkey,"Alarm",0)) alarm->data |= ALRM_ENABLED;
		if(api.GetInt(g_alarmkey,"Once",0)) alarm->data |= ALRM_ONESHOT;
		if(api.GetInt(g_alarmkey,"12h",0)) alarm->data |= ALRM_12H;
		if(api.GetInt(g_alarmkey,"ChimeHr",0)) alarm->data |= ALRM_CHIMEHR;
		if(api.GetInt(g_alarmkey,"Repeat",0)) alarm->data |= ALRM_REPEAT;
		if(api.GetInt(g_alarmkey,"Blink",0)) alarm->data |= ALRM_BLINK;
		if(api.GetInt(g_alarmkey,"jrMsgUsed",0)) alarm->data |= ALRM_DIALOG;
		TimetableQueue(alarm, 1);
	}
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
void AlarmGetMSG(dlgmsg_t* msg) {
	api.GetStr(g_alarmkey, "Name", msg->name, sizeof(msg->name), "");
	api.GetStr(g_alarmkey, "jrMessage", msg->message, sizeof(msg->message), "");
	api.GetStr(g_alarmkey, "jrSettings", msg->settings, sizeof(msg->settings), "");
	
	if(!msg->name[0]) {
		int hour = api.GetInt(g_alarmkey, "Hour", 12) % 24;
		int minute = api.GetInt(g_alarmkey, "Minute", 0);
		sprintf(msg->name, "%02d:%02d", hour, minute);
	}
}

time_t AlarmNextTimestamp() {
	time_t ts;
	struct tm tm;
	int hour, minute, days, dayf;
	
	hour = api.GetInt(g_alarmkey, "Hour", 12) % 24;
	minute = api.GetInt(g_alarmkey, "Minute", 0);
	days = api.GetInt(g_alarmkey, "Days", DAYF_DAILY);
	
	ts = time(NULL);
	localtime_r(&ts, &tm);
	dayf = DAYF_FromWDay(tm.tm_wday);
	
	if(hour < tm.tm_hour || (hour == tm.tm_hour && minute <= tm.tm_min)) {
		++tm.tm_mday;
		dayf <<= 1;
		if(dayf & DAYF_OVERFLOW)
			dayf = DAYF_MONDAY;
	}
	if(days != DAYF_DAILY) {
		while(!(days & dayf)) {
			++tm.tm_mday;
			dayf <<= 1;
			if(dayf & DAYF_OVERFLOW)
				dayf = DAYF_MONDAY;
		}
	}
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = 0;
	
	return mktime(&tm);
}
void ReadAlarmFromReg(alarm_t* pAS, int idx)
{
	if(idx >= MAX_ALARM)
		return;
	ltoa(idx+1, (g_alarmkey+ALARMKEY_OFFSET), 10);
	
	pAS->hour = api.GetInt(g_alarmkey, "Hour", 12) % 24;
	pAS->minute = api.GetInt(g_alarmkey, "Minute", 0);
	pAS->days = api.GetInt(g_alarmkey, "Days", DAYF_DAILY);
	pAS->iTimes = api.GetInt(g_alarmkey, "Times", 1);
	
	/// @note : on next backward incompatible change, store a single value including every "flag"
	pAS->uFlags=0;
	if(api.GetInt(g_alarmkey,"Alarm",0)) pAS->uFlags|=ALRM_ENABLED;
	if(api.GetInt(g_alarmkey,"Once",0)) pAS->uFlags|=ALRM_ONESHOT;
	if(api.GetInt(g_alarmkey,"12h",0)) pAS->uFlags|=ALRM_12H;
	if(api.GetInt(g_alarmkey,"ChimeHr",0)) pAS->uFlags|=ALRM_CHIMEHR;
	if(api.GetInt(g_alarmkey,"Repeat",0)) pAS->uFlags|=ALRM_REPEAT;
	if(api.GetInt(g_alarmkey,"Blink",0)) pAS->uFlags|=ALRM_BLINK;
	if(api.GetInt(g_alarmkey,"jrMsgUsed",0)) pAS->uFlags|=ALRM_DIALOG;
	
	api.GetStr(g_alarmkey, "File", pAS->fname, sizeof(pAS->fname), "");
	
	AlarmGetMSG(&pAS->dlgmsg);
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
	TimetableClean();
	m_maxAlarm = GetAlarmNum();
	if(m_maxAlarm > 0) {
		int i;
		for(i=0; i<m_maxAlarm; ++i) {
			AlarmEnable(i, -2);
		}
	}
	
	AlarmChimeEnable(-2);
	// this is for Windows 2000 only - EasterEgg function:
#	ifdef WIN2K_COMPAT
	if(api.OS == TOS_2000 && api.GetIntEx("Desktop","Transparent2kIconText",0)) {
		TimetableAdd(SCHEDID_WIN2K, 30, 30);
	}
#	endif // WIN2K_COMPAT
}
//================================================================================================
//---------------------//--------------------------------------+++--> Shut Off the God Damn Siren!:
void EndAlarm(void)   //--------------------------------------------------------------------+++-->
{
	StopFile();
	TimetableClean();
}

//============================================== This Code Assumes...	===========================
//-----------------------------------------+++--> 12pm = Noon <--> However (See Below for Details):
void OnTimerAlarm(HWND hwnd, time_t time)   // 12am = Midnight --------------------------+++-->
{
	union {
		struct {
			char fname[MAX_PATH];
			int repeat;
			SYSTEMTIME st;
		} s;
		dlgmsg_t msg;
	} u;
	#define fname_u1 u.s.fname
	#define repeat_u1 u.s.repeat
	#define st_u1 u.s.st
	#define msg_u2 u.msg
	Schedule* schedule = timetable_begin_;
	if(!schedule)
		return;
	
	if(time >= schedule->time) {
		for(; time >= schedule->time; schedule=timetable_begin_) {
			if(schedule->id & SCHEDID_START_FLAG_) {
				switch(schedule->id) {
				case SCHEDID_CHIME:
					if(schedule->data & ALRM_BLINK)
						PostMessage(g_hwndClock, CLOCKM_BLINK, TRUE, 0);
					api.GetStr("", "JihouFile", fname_u1, sizeof(fname_u1), "");
					if(fname_u1[0]) {
						if(schedule->data & ALRM_CHIMEHR) { // chime hour
							GetLocalTime(&st_u1); // faster than localtime(&time)
							repeat_u1 = st_u1.wHour;
							if(repeat_u1 == 0)
								repeat_u1 = 12;
							else if(repeat_u1 > 12)
								repeat_u1 -= 12;
							--repeat_u1;
						} else {
							repeat_u1 = 0;
						}
						PlayFile(hwnd, fname_u1, repeat_u1);
					}
					schedule->time += 3600;
					break;
#				ifdef WIN2K_COMPAT
				case SCHEDID_WIN2K:
					SetDesktopIconTextBk(1);
					schedule->time += schedule->data;
					break;
#				endif // WIN2K_COMPAT
				default:
					schedule->time = 0;
				}
			} else {
				ltoa(schedule->id+1, (g_alarmkey+ALARMKEY_OFFSET), 10);
				
				if(schedule->data & ALRM_BLINK)
					PostMessage(g_hwndClock,CLOCKM_BLINK,FALSE,0);
				if(schedule->data & ALRM_DIALOG) { // From BounceWindow.c
					AlarmGetMSG(&msg_u2);
					ReleaseTheHound(hwnd, msg_u2.name, msg_u2.message, msg_u2.settings);
				}
				api.GetStr(g_alarmkey, "File", fname_u1, sizeof(fname_u1), "");
				if(fname_u1[0]) {
					if(schedule->data & ALRM_REPEAT) {
						repeat_u1 = api.GetInt(g_alarmkey, "Times", 1) - 1;
						if(repeat_u1 < 0)
							repeat_u1 = -1;
					}else if(schedule->data & ALRM_CHIMEHR) { // chime hour
						GetLocalTime(&st_u1); // faster than localtime(&time)
						repeat_u1 = st_u1.wHour;
						if(repeat_u1 == 0)
							repeat_u1 = 12;
						else if(repeat_u1 > 12)
							repeat_u1 -= 12;
						--repeat_u1;
					}else
						repeat_u1 = 0;
					PlayFile(hwnd, fname_u1, repeat_u1);
				}
				if(schedule->data & ALRM_ONESHOT) {
					api.SetInt(g_alarmkey, "Alarm", 0);
					schedule->time = 0;
				} else {
					schedule->time = AlarmNextTimestamp();
				}
			}
			TimetableQueue(schedule, 0);
			if(!schedule->time) {
				free(schedule);
				if(!timetable_begin_)
					break;
			} else {
				TimetableQueue(schedule, 1);
			}
		}
		#ifndef _DEBUG
		if(IsPlaying()) {
			EmptyWorkingSet(GetCurrentProcess());
		}
		#endif
	}
}
#include <stdio.h> //--------------------------+++--> Required Here Only for the Open File Stuff:
//================================================================================================
//-----+++--> For Computers WithOut Speakers, Play a NoSound (PC Beep) File Through the PC Speaker:
BOOL PlayNoSound(const char* fname, DWORD iLoops)   //-----------------------------------+++-->
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
	PCBEEP* pcb = (PCBEEP*)param;
	
	PlayNoSound(pcb->szFname, pcb->dwLoops);
	SendMessage(pcb->hWnd, MM_WOM_DONE, 0, 0);
	
	free(pcb);
	_endthread();
	return 0;
}
//================================================================================================
//--+++-->
void PlayNoSoundThread(HWND hWnd, const char* fname, DWORD dwLoops)
{
	PCBEEP* pcb = (PCBEEP*)malloc(sizeof(PCBEEP));
	if(!pcb)
		return;
	pcb->hWnd = hWnd;
	pcb->dwLoops = dwLoops;
	strncpy_s(pcb->szFname, sizeof(pcb->szFname), fname, _TRUNCATE);
	
	m_bKillPCBeep = 0;
	_beginthreadex(NULL, 0, PlayNoSoundProc, pcb, 0, 0);
}
//=================================================*
// ------------------------ Play Selected Alarm File
//===================================================*
BOOL PlayFile(HWND hwnd, const char* fname, DWORD dwLoops)
{
	char file[MAX_PATH];
	if(!fname[0] || fname[0]=='<') return FALSE;
	if(fname[0]!='/' && fname[0]!='\\' && fname[1]!=':' // no abs path
	&&(fname[0]!='.' || (fname[1]!='/' && fname[1]!='\\' && (fname[1]!='.' ||  (fname[2]!='/' && fname[2]!='\\'))))) // no relative path (strict relative)
	{ // do it relative to "waves/"
		char app[MAX_PATH];
		const size_t tlen=api.root_len;
		const size_t len=strlen(fname)+1; // incl. terminating null
		if(len<MAX_PATH-tlen-7){
			memcpy(file, api.root, tlen); // absolute path is at least required by .pcb / PlayNoSoundThread (not for .wav)
			memcpy(file+tlen, "\\waves\\", 7);
			memcpy(file+tlen+7, fname, len);
			if(api.GetFileAndOption(file, app, NULL) == 0){
				fname = file;
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
				const char* p;
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
BOOL PlayWave(HWND hwnd, const char* fname, DWORD dwLoops)   //-----------------------------+++-->
{
	MMCKINFO mmckinfoSubchunk;
	MMCKINFO mmckinfoParent;
	LONG lDataSize;
	LONG lFmtSize;
	HMMIO hmmio;
	
	if(m_hWaveOut != NULL) return FALSE;
	
	if((hmmio=mmioOpen((char*)fname, NULL, MMIO_READ|MMIO_ALLOCBUF))==0)
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
