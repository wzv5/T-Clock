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
//	api.SetStr(NULL,"ExePath",api.root);
	api.GetStr("Clock","Font",msg,80,"");
	/// new installation?
	if(!*msg){
		NONCLIENTMETRICS metrics={sizeof(NONCLIENTMETRICS)};
		union{
			unsigned short entryS;
			char entry[3];
		} u;
		u.entry[2]='\0';
		u.entryS='0'|('1'<<8); // left, 1 click
		api.SetInt(REG_MOUSE,u.entry,MOUSEFUNC_SHOWCALENDER);
		u.entryS='1'|('1'<<8); // right, 1 click
		api.SetInt(REG_MOUSE,u.entry,MOUSEFUNC_MENU);
		u.entryS='2'|('1'<<8); // middle, 1 click
		api.SetInt(REG_MOUSE,u.entry,IDM_STOPWATCH);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);
		api.SetStr("Clock","Font",metrics.lfCaptionFont.lfFaceName);
		if(!u.entryS){ /// @todo : XP: measure taskbar height to chose font size and or multiline/singleline (small vs "normal" taskbar)
			HWND hwnd = FindWindow("Shell_TrayWnd", NULL);
			if(hwnd) {
				RECT rc; GetClientRect(hwnd, &rc);
				if(rc.right > rc.bottom) u.entryS = 1;
			}
		}
		api.SetInt("Format", "Kaigyo", u.entryS);
		api.SetInt("","Ver",CURRENT_VER);
		return 1;
	}
	/// old installation, set update flags if any
	switch(api.GetInt("","Ver",0)){
	case 0: /// v2.2.0#84(a507ca5) we no longer use the "FontRotateDirection" as it's replaced by "Angle", timers also work differently
		updateflags|=SFORMAT_EFFICIENT|SFORMAT_LESSMEM|SFORMAT_FEATURE;
		compatibilityflags|=SCOMPAT_FORMAT|SCOMPAT_TIMERS;
		/* fall through */
		
	case 1: /// v2.3.0#127(dff0300,63ba670#106) T-Clock file structure changed, startup link must be updated.
		updateflags|=SFORMAT_SILENT;
		/* fall through */
		
	case 2: /// v2.4.0#000(dff0300,63ba670#106) alarms unified to always use 24h format internally
		updateflags|=SFORMAT_SILENT;
		/* fall through */
		
//	case CURRENT_VER: // current version
		CheckMouseMenu(); // adds right mouse button click to handle context menu if missing
		break;
		
	default:{
		int ans=MessageBox(NULL,"This version of T-Clock looks older than what you've used before.\nSome settings might not be readable and you might loose some stuff.\n\nDo you want to run this old version anyway?","T-Clock downgraded?",MB_OKCANCEL|MB_ICONINFORMATION);
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
	switch(api.GetInt("","Ver",0)){
	case 0:
		// update font rotate (from none,left,right to 0-360 angle)
		api.GetStr("Clock","FontRotateDirection",buf,sizeof(buf),"");
		switch(*buf){
		case 'L': api.SetInt("Clock","Angle",90); break;
		case 'R': api.SetInt("Clock","Angle",270); break;
		}
		api.DelValue("Clock","FontRotateDirection");
		// remove "ID" from "Timers" as no longer used
		len=wsprintf(buf, "Timers\\Timer");
		idx2=api.GetInt("Timers","NumberOfTimers",0);
		for(idx=0; idx<idx2; ){
			wsprintf(buf+len,"%d",++idx);
			api.DelValue(buf,"ID");
		}
		/* fall through */
		
	case 1:
		if(GetStartupFile(NULL,buf)){
			DeleteFile(buf);
			AddStartup(NULL);
		}
		/* fall through */
		
	case 2:{
		int b12h, hour;
		// convert alarms to use 24h format
		len = wsprintf(buf, "Alarm");
		idx2 = GetAlarmNum();
		for(idx=0; idx<idx2; ){
			wsprintf(buf+len, "%d", ++idx);
			b12h = api.GetInt(buf, "Hour12", 0);
			if(b12h){ // convert to 24h format
				hour = api.GetInt(buf, "Hour", 12);
				api.SetInt(buf, "Hour", _12hTo24h(hour,api.GetInt(buf,"PM",0)));
				api.SetInt(buf, "12h", b12h);
			}
			api.DelValue(buf, "Hour12");
			api.DelValue(buf, "PM");
		}}
		/* fall through */
		
//	case CURRENT_VER:
//		;
	}
	api.SetInt("","Ver",CURRENT_VER);
}
