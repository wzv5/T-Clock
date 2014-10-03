#include "tclock.h"
#include "../common/utl.h"

#define CURRENT_VER 2

enum{
	SFORMAT_NONE		=0x0000,
	SFORMAT_SILENT		=0x0001,
	// show update message box with these:
	SFORMAT_BASIC		=0x0002,
	SFORMAT_EFFICIENT	=0x0004,
	SFORMAT_LESSMEM		=0x0008,
	SFORMAT_FEATURE		=0x0010,
};
static const char* SFORMAT[]={
	"different",//SFORMAT_BASIC
	"more efficient",//SFORMAT_EFFICIENT
	"using less memory",//SFORMAT_LESSMEM
	"more feature rich",//SFORMAT_FEATURE
};

enum{
	SCOMPAT_NONE		=0x0000,
	SCOMPAT_FORMAT		=0x0001,
	SCOMPAT_FORMATALL	=0x0002,
	SCOMPAT_TIMERS		=0x0004,
};
static const char* SCOMPAT[]={
	"some clock text options",
	"all clock text options",
	"your timers",
};

static void ConvertSettings();
int CheckSettings(){
	char msg[1024];
	int updateflags=SFORMAT_NONE;
	int compatibilityflags=SCOMPAT_NONE;
//	SetMyRegStr(NULL,"ExePath",g_mydir);
	GetMyRegStr("Clock","Font",msg,80,"");
	/// new installation?
	if(!*msg){
		NONCLIENTMETRICS metrics={sizeof(NONCLIENTMETRICS)};
		union{
			unsigned short entryS;
			char entry[3];
		} u;
		u.entry[2]='\0';
		u.entryS='0'|('1'<<8); // left, 1 click
		SetMyRegLong(g_reg_mouse,u.entry,MOUSEFUNC_SHOWCALENDER);
		u.entryS='1'|('1'<<8); // right, 1 click
		SetMyRegLong(g_reg_mouse,u.entry,MOUSEFUNC_MENU);
		u.entryS='2'|('1'<<8); // middle, 1 click
		SetMyRegLong(g_reg_mouse,u.entry,IDM_STOPWATCH);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);
		SetMyRegStr("Clock","Font",metrics.lfCaptionFont.lfFaceName);
		SetMyRegLong("Format", "Hour12", 1);
		u.entryS=g_tos>=TOS_VISTA;
		SetMyRegLong("Format", "AMPM", u.entryS);
		if(!u.entryS){ /// @todo : XP: measure taskbar height to chose font size and or multiline/singleline (small vs "normal" taskbar)
			HWND hwnd = FindWindow("Shell_TrayWnd", NULL);
			if(hwnd != NULL) {
				RECT rc; GetClientRect(hwnd, &rc);
				if(rc.right > rc.bottom) u.entryS = 1;
			}
		}
		SetMyRegLong("Format", "Kaigyo", u.entryS);
		SetMyRegLong("","Ver",CURRENT_VER);
		return 1;
	}
	/// old installation, set update flags if any
	switch(GetMyRegLong("","Ver",0)){
	case 0: /// v2.2.0#84(a507ca5) we no longer use the "FontRotateDirection" as it's replaced by "Angle", timers also work differently
		updateflags|=SFORMAT_EFFICIENT|SFORMAT_LESSMEM|SFORMAT_FEATURE;
		compatibilityflags|=SCOMPAT_FORMAT|SCOMPAT_TIMERS;
		
		
	case 1: /// v2.3.0#() T-Clock file structure changed, startup link must be updated.
		updateflags|=SFORMAT_SILENT;
		
		
	case CURRENT_VER: /// current version
		CheckMouseMenu(); // adds right mouse button click to handle context menu if missing
		break;
		
		
	default:{
		int ans=MessageBox(NULL,"This version of T-Clock looks older than what you've used before.\nSome settings might not be readable and you might lose some stuff.\n\nDo you want to run this old version anyway?","T-Clock downgraded?",MB_OKCANCEL|MB_ICONINFORMATION);
		if(ans==IDOK){
			ConvertSettings(); // should do nothing, just downgrade our version number
			return 0;
		}else
			return -1;
		}
	}
	/// check if update is "required"
	if(updateflags){
		int ans;
		if(updateflags!=SFORMAT_SILENT){
			int flags;
			char* pos=msg;
			pos+=wsprintf(pos,"Seem like you've been using an older version before,\nthis version now saves some settings differently.\nThe new format is:");
			for(flags=updateflags>>1/*ignore SFORMAT_SILENT*/,ans=0; flags; flags>>=1,++ans){
				if(flags&1)
					pos+=wsprintf(pos,"\n	+ %s",SFORMAT[ans]);
			}
			if(compatibilityflags){
				pos+=wsprintf(pos,"\n\nBut if you want to use an older version, you might lose:");
				for(flags=compatibilityflags,ans=0; flags; flags>>=1,++ans){
					if(flags&1)
						pos+=wsprintf(pos,"\n	- %s",SCOMPAT[ans]);
				}
			}
			pos+=wsprintf(pos,"\n\nConvert now? Or cancle and exit T-Clock?");
			ans=MessageBox(NULL,msg,"T-Clock updated?",MB_OKCANCEL|MB_ICONINFORMATION);
		}else // silent update
			ans=IDOK;
		if(ans==IDOK){
			ConvertSettings();
			return 2;
		}else
			return -1;
	}
	return 0;
}

void ConvertSettings(){
	char buf[MAX_PATH];
	int idx, idx2;
	size_t len;
	switch(GetMyRegLong("","Ver",0)){
	case 0:
		// update font rotate (from none,left,right to 0-360 angle)
		GetMyRegStr("Clock","FontRotateDirection",buf,sizeof(buf),"");
		switch(*buf){
		case 'L': SetMyRegLong("Clock","Angle",90); break;
		case 'R': SetMyRegLong("Clock","Angle",270); break;
		}
		DelMyReg("Clock","FontRotateDirection");
		// remove "ID" from "Timers" as no longer used
		len=wsprintf(buf, "Timers\\Timer");
		idx2=GetMyRegLong("Timers","NumberOfTimers",0);
		for(idx=0; idx<idx2; ){
			wsprintf(buf+len,"%d",++idx);
			DelMyReg(buf,"ID");
		}
		
		
	case 1:
		if(GetStartupFile(NULL,buf)){
			DeleteFile(buf);
			AddStartup(NULL);
		}
		
		
	case CURRENT_VER:
		;
	}
	SetMyRegLong("","Ver",CURRENT_VER);
}
