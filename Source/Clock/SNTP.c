/*-------------------------------------------------------------
  sntp.c
  KAZUBON 1998-1999
               Special thanks to Tomoaki Nakashima
---------------------------------------------------------------*/
// Modified by Stoic Joker: Monday, 04/12/2010 @ 7:42:04pm
#include "tclock.h"
#include <winsock.h>

//===============================================================
struct NTP_Packet { // NTP (Network Time Protocol) Request Packet
	int Control_Word;
	int root_delay;
	int root_dispersion;
	int reference_identifier;
	__int64 reference_timestamp;
	__int64 originate_timestamp;
	__int64 receive_timestamp;
	int transmit_timestamp_seconds;
	int transmit_timestamp_fractions;
};
//===========================================================
typedef struct { // Close Socket on Request TimeOut Structure
	BOOL bComplete;
	DWORD dwSent;
	SOCKET soc;
} KILLSOC, *LPKILLSOC;

// PageHotKey.c
extern hotkey_t* tchk;
extern WNDPROC OldEditClassProc;
LRESULT APIENTRY SubClassEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static const char m_subkey[] = "SNTP";
static const DWORD m_nTimeout = 1000;
static HWND m_dlg = NULL;
static BOOL m_bSaveLog;
static BOOL m_bMessage;
static DWORD m_dwTickCountOnSend = 0;

BOOL GetSetTimePermissions(void);

static void OnInit(HWND);
static unsigned __stdcall KillSocketProc(void*);
static void OnSanshoAlarm(HWND hDlg, WORD id);
static INT_PTR CALLBACK DlgProcSNTPConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//================================================================================================
//---------------------------//----------------------------+++--> Save Request Results in SNTP.log:
void Log(const char* msg)   //--------------------------------------------------------------+++-->
{
	char logmsg[GEN_BUFF];
	size_t len;
	SYSTEMTIME st;
	
	GetLocalTime(&st);
	len = wsprintf(logmsg, "%d/%02d/%02d %02d:%02d:%02d ", st.wYear,
					st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strncpy_s(logmsg+len, sizeof(logmsg)-len, msg, _TRUNCATE);
	
	// save to file
	if(m_bSaveLog) {
		char fname[MAX_PATH];
		HFILE hf;
		
		strcpy(fname, g_mydir);
		add_title(fname, "SNTP.log");
		hf = _lopen(fname, OF_WRITE);
		if(hf == HFILE_ERROR)
			hf = _lcreat(fname, 0);
		if(hf == HFILE_ERROR) return;
		_llseek(hf, 0, 2);
		_lwrite(hf, logmsg, lstrlen(logmsg));
		_lwrite(hf, "\r\n", 2);
		_lclose(hf);
	}
	
	if(m_dlg) { // IF Configure NTP Server is Open, Display Results in Sync History.
		LVITEM lvItem; //-----+++--> Even if Activity is Not Saved to the Log File.
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0; // Hold These at Zero So the File Loads Backwards
		lvItem.iItem = 0; //-----+++--> Which Puts the Most Recent Info on Top.
		lvItem.pszText = logmsg;
		ListView_InsertItem(GetDlgItem(m_dlg,IDC_LIST), &lvItem);
	}
	
	if(m_bMessage) {
		MessageBox(0, logmsg, "T-Clock Time Sync", MB_OK);
	}
}
//================================================================================================
//-------------------------------------------------------+++--> Set System Time With Received Data:
void SynchronizeSystemTime(DWORD seconds, DWORD fractions)   //-----------------------------+++-->
{
	BOOL bOk;
	char msg[GEN_BUFF];
	size_t msglen;
	char szWave[MAX_BUFF];
	SYSTEMTIME st;
	union{
		FILETIME ft;
		ULONGLONG ftqw;
	} tnew;
	union{
		FILETIME ft;
		ULONGLONG ftqw;
	} told;
	DWORD sr_time;
	
	// timeout ?
	sr_time = GetTickCount() - m_dwTickCountOnSend;
	if(sr_time >= m_nTimeout) {
		wsprintf(msg, "timeout (%04d)", sr_time);
		Log(msg);
		return;
	}
	
	// current time
	GetSystemTimeAsFileTime(&told.ft);
	
	// NTP data -> FILETIME
	// seconds since 1900/01/01
	// + 100 nano-seconds from 1601/01/01 (FILETIME) to 1900/01/01 (NTP)
	tnew.ftqw = M32x32to64(seconds, 10000000) + 94354848000000000ULL;
	// + fractions ranging from 0 to MAXUINT(0xFFFFFFFF,4294967295) as rounded up milliseconds
	tnew.ftqw += (fractions / (429496+1) + 5) / 10 * 10000;
	// we now have our time with 500 nanosecond precession (as windows can only handle milliseconds)
	
	// set system time
	bOk = FileTimeToSystemTime(&tnew.ft, &st);
	if(bOk)
		bOk = SetSystemTime(&st);
	if(!bOk) {
		Log("failed to set time"); return;
	}
	// delayed or advanced
	bOk = (tnew.ftqw > told.ftqw);
	// get difference
	if(bOk) tnew.ftqw = tnew.ftqw - told.ftqw;
	else  tnew.ftqw = told.ftqw - tnew.ftqw;
	FileTimeToSystemTime(&tnew.ft, &st);
	
	// save log
	msglen = 13;
	memcpy(msg, "synchronized ", msglen+1);
	if(st.wYear == 1601 && st.wMonth == 1
	&& st.wDay == 1 && st.wHour == 0) {
		msg[msglen++] = (bOk?'+':'-');
		msglen += wsprintf(msg+msglen, "%02d:%02d.%03d ",
				 st.wMinute, st.wSecond, st.wMilliseconds);
	}
	
	GetMyRegStr(m_subkey, "Sound", szWave, MAX_BUFF, "");
	if(szWave[0]){
		PlayFile(g_hwndTClockMain, szWave, 0);
	}
	msglen += wsprintf(msg+msglen, " (%04d)", sr_time);
	Log(msg);
}
//================================================================================================
//--------------------------------------------------+++--> Close Socket, and the WinSOCK Interface:
void SocketClose(SOCKET Sntp, const char* msgbuf)   //--------------------------------------+++-->
{

	if(Sntp != -1) {
		closesocket(Sntp);
	}
	
	WSACleanup();
	if(msgbuf) Log(msgbuf);
}
/*---------------------------------------------------
	get server name and port number from string
		buf: "ntp.xxxxx.ac.jp:123"
---------------------------------------------------*/
unsigned short GetServerPort(const char* uri, char* server)
{
	char* pos;
	unsigned short port = 123;
	if(!*uri) return 0;
	strcpy(server,uri);
	for(pos=server; *pos && *pos!=':'; ++pos);
	if(*pos++==':') {
		*pos='\0'; port=0;
		for(; *pos; ++pos) {
			if(*pos>='0' && *pos <= '9'){
				port*=10;
				port+=*pos-'0';
			}else{
				port=0; break;
			}
		}
	}
	return port;
}
//================================================================================================
//-------------------------+++--> Looks Like the Server Has SomeThing to Say - Find Out What it is:
void ReceiveSNTPReply(SOCKET Sntp)   //-----------------------------------------------------+++-->
{
	struct sockaddr_in FromAddr;
	struct NTP_Packet NTP_Recv;
	int sockaddr_Size;
	int nRet;
	
	//-----------------------------------------------+++--> Receive UpDated Time Data:
	sockaddr_Size = sizeof(FromAddr);
	nRet = recvfrom(Sntp, (char*)&NTP_Recv, sizeof(NTP_Recv), 0,
					(struct sockaddr*)&FromAddr, &sockaddr_Size);
	if(nRet == SOCKET_ERROR) {
		char szErr[MIN_BUFF];
		wsprintf(szErr, "Receive SOCKET ERROR: %d", WSAGetLastError());
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		SocketClose(Sntp, szErr);
		return;
	}
	
	//-------------------------------+++--> (Message Received) Now Set the System Time!
	SynchronizeSystemTime(ntohl(NTP_Recv.transmit_timestamp_seconds),
						  ntohl(NTP_Recv.transmit_timestamp_fractions));
	SocketClose(Sntp, 0);
}
//================================================================================================
//-------------------------------------------------------------------+++--> Send SNTP Sync Request:
int SNTPSend(SOCKET Sntp, LPSOCKADDR_IN lpstToAddr)
{
	struct NTP_Packet NTP_Send;
	int nRet;
	
	// init a packet
	memset(&NTP_Send, 0, sizeof(struct NTP_Packet));
	NTP_Send.Control_Word = htonl(0x0B000000);
	
	// send a packet
	nRet = sendto(Sntp, (const char*)&NTP_Send, sizeof(NTP_Send),
				  0, (SOCKADDR*)lpstToAddr, sizeof(SOCKADDR_IN));
				  
	if(nRet == SOCKET_ERROR) { // Tell Us if "We" Failed!
		char szErr[MIN_BUFF];
		wsprintf(szErr, "Send SOCKET ERROR: %d", WSAGetLastError());
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		SocketClose(Sntp, szErr);
	}
	// save tickcount
	m_dwTickCountOnSend = GetTickCount();
	return(nRet);
}

//================================================================================================
//-------------------------------------------------------------+++--> Open Socket for SNTP Session:
SOCKET OpenTimeSocket(char* szRegString)
{
	struct sockaddr_in serveraddr;
	char szServer[256];
	char szErr[MIN_BUFF] = {0};
	int nRet;
	unsigned short port;
	SOCKET Sntp;
	
	LPHOSTENT lpHost;
	
	port = GetServerPort(szRegString, szServer);
	lpHost = gethostbyname(szServer); // Verify that Host Name Exists
	
	if(lpHost == NULL) {
		wsprintf(szErr, "Host not found: %s", szRegString);
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	// make a socket
	Sntp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(Sntp == SOCKET_ERROR) {
		wsprintf(szErr, "Create SOCKET Failed! ERROR: %d", WSAGetLastError());
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		SocketClose(Sntp, szErr);
		return FALSE;
	}
	
	// Setup destination socket address
	serveraddr.sin_addr.s_addr = *((u_long FAR*)(lpHost->h_addr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	
	// send data
	nRet = SNTPSend(Sntp, &serveraddr);
	if(nRet == SOCKET_ERROR) {
		wsprintf(szErr, "SendTo ERROR: %d", WSAGetLastError());
		MessageBox(0, szErr, "Send Time Request Failed:", MB_OK|MB_ICONERROR);
		SocketClose(Sntp, szErr);
		return FALSE;
	}
	return Sntp;
}
//====================//========================================================================
// Required for SNTP/UDP Socket Operation TimeOut Thread Only ===================================
#include <process.h>   //--+++-->				 <--+++--<<<<< SNTP Code Starts Here >>>>>--+++-->
//====================//===========================================================================
void SyncTimeNow()   //============================================================================
{
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData; // Okay...Now We Want WinSock v2.2
	char szServer[MIN_BUFF];
	char szErr[GEN_BUFF];
	DWORD dwTickCount = 0;
	SOCKET Sntp = 0;
	KILLSOC ks;
	int nRet;
	
	if(!m_dlg) {
		m_bSaveLog = GetMyRegLongEx(m_subkey, "SaveLog", 0);
		m_bMessage = GetMyRegLongEx(m_subkey, "MessageBox", 0);
	}
	GetMyRegStrEx(m_subkey, "Server", szServer, sizeof(szServer), "");
	if(!strlen(szServer)) { //-------+++--> If SNTP Server is NOT Configured:
		wsprintf(szErr, "No SNTP Server Specified!");
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		if(!m_dlg) NetTimeConfigDialog();
		return;
	}
	
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(nRet) { //-----------------------------------------+++--> If WinSock Startup Fails...
		wsprintf(szErr, "Error initializing WinSock");
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		return;
	}
	
	if(wsaData.wVersion != wVersionRequested) { //-+++-> Check WinSOCKET's Version:
		wsprintf(szErr, "WinSock version not supported");
		MessageBox(0, szErr, "Time Sync Failed:", MB_OK|MB_ICONERROR);
		return;
	}
	
	Sntp = OpenTimeSocket(szServer);
	if(!Sntp) return;
	
	dwTickCount = GetTickCount();
	
	ks.soc = Sntp;
	ks.dwSent = dwTickCount;
	//--------------------------------+++--> Start CutOff Timer:
	_beginthreadex(NULL, 0, KillSocketProc,(void*)&ks, 0, NULL);
	
	ReceiveSNTPReply(Sntp);
	ks.bComplete = TRUE;
}
//================================================================================================
//---------------------------------------+++--> SNTP/UDP Socket Operation TimeOut Thread Procedure:
unsigned __stdcall KillSocketProc(void* param)    //----------------------------------------+++-->
{
	KILLSOC* ks;
	DWORD dwNow = 0;
	DWORD dwKillTime;
	ks = (KILLSOC*)param;
	
	dwKillTime = ks->dwSent + 1000;
	while(dwNow < dwKillTime) {
		Sleep(10);
		dwNow = GetTickCount();
		if(ks->bComplete) dwNow = dwKillTime;
	}
	if(!ks->bComplete) {
		closesocket(ks->soc);
		WSACleanup();
	}
	_endthread();
	return 0;
}
//=================================================================================================
/* SetSystemTime(&st); Requires SE_SYSTEMTIME_NAME Priviledge: Are you running as a limited user?

If so, you'll need to use the group policy editor (Run gpedit.msc as Administrator) to assign the
rights here:

(Computer Configuration\Windows Settings\Security Settings\Local Policies\User Rights Assignments
and add your username to "Change the system time"). I don't know of any specific registry key. */

// Note: Changing Privileges in a Token - Only Works IF the Account Has Access to that Priviledge
//---------------------------------------------------+++--> e.g. IS An Administrator.
//================================================================================================
//--------------------------------//---------------------+++--> Open the SNTP Configuration Dialog:
void NetTimeConfigDialog(void)   //---------------------------------------------------------+++-->
{
	DialogBox(0, MAKEINTRESOURCE(IDD_SNTPCONFIG), g_hwndTClockMain, DlgProcSNTPConfig);
}
//================================================================================================
//--------------------------//--+++--> Save Network Time Server Configuration Settings to Registry:
void OkaySave(HWND hDlg)   //---------------------------------------------------------------+++-->
{
	char szServer[MIN_BUFF];
	char szSound[MAX_PATH];
	char entry[TNY_BUFF];
	HWND hServer = GetDlgItem(hDlg,IDCBX_NTPSERVER);
	int i, count;
	
	SetMyRegLong(m_subkey, "SaveLog", IsDlgButtonChecked(hDlg, IDCBX_SNTPLOG));
	SetMyRegLong(m_subkey, "MessageBox", IsDlgButtonChecked(hDlg, IDCBX_SNTPMESSAGE));
	
	GetDlgItemText(hDlg, IDCE_SYNCSOUND, szSound, MAX_PATH);
	SetMyRegStr(m_subkey, "Sound", szSound);
	
	if(tchk[0].bValid) { // Synchronize System Clock With Remote Time Server
		RegisterHotKey(g_hwndTClockMain, HOT_TSYNC, tchk[0].fsMod, tchk[0].vk);
	} else {						// I'm Calling This One Mouser's HotKey...
		tchk[0].vk = 0;		   // I'm Not Explaining It You Either Already
		tchk[0].fsMod = 0;	  // Understand Why or You're Not Going Too...
		strcpy(tchk[0].szText, "None");
		UnregisterHotKey(g_hwndTClockMain, HOT_TSYNC);
	}
	SetMyRegLong("HotKeys\\HK5", "bValid", tchk[0].bValid);
	SetMyRegLong("HotKeys\\HK5", "fsMod",  tchk[0].fsMod);
	SetMyRegStr("HotKeys\\HK5", "szText", tchk[0].szText);
	SetMyRegLong("HotKeys\\HK5", "vk",  tchk[0].vk);
	
	
	ComboBox_GetText(hServer, szServer, sizeof(szServer));
	SetMyRegStr(m_subkey, "Server", szServer);
	
	if(szServer[0]) {
		int index = ComboBox_FindStringExact(hServer, -1, szServer);
		if(index != CB_ERR)
			ComboBox_DeleteString(hServer, index);
		ComboBox_InsertString(hServer, 0, szServer);
		ComboBox_SetCurSel(hServer, 0);
	}
	count = ComboBox_GetCount(hServer);
	// removed deleted servers
	for(i=GetMyRegLong(m_subkey,"ServerNum",0); i>count; --i){
		wsprintf(entry, "Server%d", i);
		DelMyReg(m_subkey, entry);
	}
	// update server list
	for(i=0; i < count; ++i) {
		ComboBox_GetLBText(hServer, i, szServer);
		wsprintf(entry, "Server%d", i+1);
		SetMyRegStr(m_subkey, entry, szServer);
	}
	SetMyRegLong(m_subkey, "ServerNum", count);
}
//================================================================================================
//------------------------------------------------------+++--> SNTP Configuration Dialog Procedure:
INT_PTR CALLBACK DlgProcSNTPConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	(void)lParam;
	
	switch(msg)  {
	case WM_INITDIALOG:
		m_dlg = hDlg;
		OnInit(hDlg);
		return TRUE;
	case WM_DESTROY:
		m_dlg = NULL;
		break;
		
	case WM_COMMAND:
		switch(LOWORD(wParam))  {
		case IDCB_SYNCNOW:{
			OkaySave(hDlg);
			SyncTimeNow();
			return TRUE;}
			
		case IDCB_SYNCSOUNDBROWSE:
			OnSanshoAlarm(hDlg, IDCE_SYNCSOUND);
			return TRUE;
			
		case IDCB_DELSERVER:{
			HWND hServer = GetDlgItem(hDlg,IDCBX_NTPSERVER);
			int index = ComboBox_GetCurSel(hServer);
			if(index != CB_ERR){
				ComboBox_DeleteString(hServer, index);
			}
			return TRUE;}
			
		case IDOK:
			OkaySave(hDlg);
			/* fall through */
		case IDCANCEL:
			if(tchk) {
				free(tchk);   // Free, and...? (Crash Unless You Include the Next Line)
				tchk = NULL; //<--+++--> Thank You Don Beusee for reminding me to do this.
			}
			EndDialog(hDlg, /*wParam*/TRUE);
			return TRUE;
		}
	}
	return FALSE;
}
//=================//=====//>>>>>---------------------------------------------+++-->
#include <stdio.h> //-----//--+++--> Required Here For Log FILE Open Functions Only.
//=================//=====//======================================================================
//------------------------//---------------------------+++--> To-Do List for Dialog Initialization:
void OnInit(HWND hDlg)   //-----------------------------------------------------------------+++-->
{
	char str[MAX_PATH];
	FILE* stReport;
	LVCOLUMN lvCol;
	LVITEM lvItem;
	int i, count;
	HWND hList = GetDlgItem(hDlg,IDC_LIST);
	HWND hServer = GetDlgItem(hDlg,IDCBX_NTPSERVER);
	
	SetMyDialgPos(hDlg,21);
	
	// Get the List of Configured Time Servers:
	GetMyRegStr(m_subkey, "Server", str, sizeof(str), "");
	count = GetMyRegLong(m_subkey, "ServerNum", 0);
	for(i = 1; i <= count; i++) {
		char s[MAX_BUFF], entry[TNY_BUFF];
		
		wsprintf(entry, "Server%d", i);
		GetMyRegStr(m_subkey, entry, s, 80, "");
		if(s[0]) ComboBox_AddString(hServer, s);
	}
	if(!ComboBox_GetCount(hServer)){
		ComboBox_AddString(hServer,"europe.pool.ntp.org");
		ComboBox_AddString(hServer,"north-america.pool.ntp.org");
		ComboBox_AddString(hServer,"asia.pool.ntp.org");
		ComboBox_AddString(hServer,"oceania.pool.ntp.org");
		ComboBox_AddString(hServer,"south-america.pool.ntp.org");
		ComboBox_AddString(hServer,"africa.pool.ntp.org");
	}
	if(!str[0])
		strcpy(str,"pool.ntp.org");
	i = ComboBox_FindStringExact(hServer, -1, str);
	if(i == CB_ERR) {
		i = ComboBox_InsertString(hServer, 0, str);
	}
	ComboBox_SetCurSel(hServer, i);
	
	if(!g_hIconDel) {
		g_hIconDel = LoadImage(GetModuleHandle(NULL),
							   MAKEINTRESOURCE(IDI_DEL),
							   IMAGE_ICON, 16, 16,
							   LR_DEFAULTCOLOR);
	}
	SendDlgItemMessage(hDlg, IDCB_DELSERVER, BM_SETIMAGE,
					   IMAGE_ICON, (LPARAM)g_hIconDel);
					   
	// Get the Sync Sound File:
	GetMyRegStr(m_subkey, "Sound", str, sizeof(str), "");
	SetDlgItemText(hDlg, IDCE_SYNCSOUND, str);
	
	// Get the Confirmation Options:
	m_bSaveLog = GetMyRegLongEx(m_subkey, "SaveLog", 0);
	CheckDlgButton(hDlg, IDCBX_SNTPLOG, m_bSaveLog);
	m_bMessage = GetMyRegLongEx(m_subkey, "MessageBox", 0);
	CheckDlgButton(hDlg, IDCBX_SNTPMESSAGE, m_bMessage);
	
	// Load & Display the Configured Synchronization HotKey:
	tchk = (hotkey_t*)malloc(sizeof(hotkey_t));
	tchk[0].bValid = GetMyRegLongEx("HotKeys\\HK5", "bValid", 0);
	GetMyRegStrEx("HotKeys\\HK5", "szText", tchk[0].szText, TNY_BUFF, "None");
	tchk[0].fsMod = GetMyRegLongEx("HotKeys\\HK5", "fsMod", 0);
	tchk[0].vk = GetMyRegLongEx("HotKeys\\HK5", "vk", 0);
	
	SetDlgItemText(hDlg, IDCE_SYNCHOTKEY, tchk[0].szText);
	
	// Subclass the Edit Controls
	OldEditClassProc  = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDCE_SYNCHOTKEY), GWLP_WNDPROC);
	SetWindowLongPtr(GetDlgItem(hDlg, IDCE_SYNCHOTKEY), GWLP_WNDPROC, (LONG_PTR)SubClassEditProc);
	
	// init listview
	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
	SetXPWindowTheme(hList,L"Explorer",NULL);
	
	lvCol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvCol.cx = 280; //-+-// Set Column Width in Pixels
	lvCol.iSubItem = 0; // This is the First & Only Column
	lvCol.pszText = "Synchronization History"; // Header Text
	ListView_InsertColumn(hList, 0, &lvCol);
	
	// Test For: SE_SYSTEMTIME_NAME Priviledge Before Enabling Sync Now Button:
	EnableDlgItem(hDlg, IDCB_SYNCNOW, GetSetTimePermissions());
	
	// Load the Time Synchronization Log File:
	strcpy(str, g_mydir);
	add_title(str, "SNTP.log");
	
	stReport = fopen(str, "r");
	if(stReport) {
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0; // Hold These at Zero So the File Loads Backwards
		lvItem.iItem = 0; //-----+++--> Which Puts the Most Recent Info on Top.
		
		for(;;) {   // (for) Ever Basically.
			if(fgets(str, sizeof(str), stReport)) {
				str[strcspn(str,"\n")] = '\0'; // Remove the Newline Character
				lvItem.pszText = str;
				ListView_InsertItem(hList, &lvItem);
			}else
				break;
		}
		fclose(stReport);
	}
}
//================================================================================================
//----------------------------------------//---------------+++--> Browse for Sync Event Sound File:
void OnSanshoAlarm(HWND hDlg, WORD id)   //-------------------------------------------------+++-->
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, id, deffile, MAX_PATH);
	if(!BrowseSoundFile(hDlg, deffile, fname)) // soundselect.c
		return;
		
	SetDlgItemText(hDlg, id, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}
