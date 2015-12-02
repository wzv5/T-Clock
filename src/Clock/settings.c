#include "tclock.h"
#include "../common/utl.h"

typedef enum VERSION {
	FRESH = -1, /**< no version / new installation */
	/** \c v2.2.0#84(a507ca5)
	- we no longer use the "FontRotateDirection" as it's replaced by "Angle"
	- timers also work differently
	- text is centered by default */
	_2_2_0,
	/** \c v2.3.0#127(dff0300,63ba670#106)
	- T-Clock file structure changed
	- startup link must be updated */
	_2_3_0,
	/** \c v2.4.0#329(f512dbe)
	- alarms unified to always use 24h format internally
	- added distinct 24h format
	- "new line" format supports switched sides */
	_2_4_0,
	CURRENT, /**< current version */
} VERSION;

static int ParseSettings();
static void ConvertSettings(VERSION ver);
static void FirstTimeSetup(VERSION ver);

int CheckSettings(){
	int ret = ParseSettings(); // do first-time setup or check and convert settings
	InitFormat(); // initialize/reset automated Date/Time format
	CheckMouseMenu(); // add context menu action if undefined
	return ret;
}

enum{
	SFORMAT_NONE		=0x0000,
	SFORMAT_SILENT		=0x0001,
	// show update message box with these:
	SFORMAT_BASIC		=0x0002,
	SFORMAT_EFFICIENT	=0x0004,
	SFORMAT_LESSMEM		=0x0008,
	SFORMAT_FEATURE		=0x0010,
	SFORMAT_TEXTPOSITION=0x0020,
};
static const char* SFORMAT[]={
	"different",//SFORMAT_BASIC
	"more efficient",//SFORMAT_EFFICIENT
	"using less memory",//SFORMAT_LESSMEM
	"more feature rich",//SFORMAT_FEATURE
	"positioning text differently",//SFORMAT_TEXTPOSITION
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

int ParseSettings(){
	char msg[1024];
	int updateflags = SFORMAT_NONE;
	int compatibilityflags = SCOMPAT_NONE;
	VERSION ver = api.GetInt("", "Ver", 0);
	
//	api.SetStr(NULL, "ExePath", api.root);
	api.GetStr("Clock", "Font", msg, 80, "");
	/// new installation?
	if(!*msg) {
		NONCLIENTMETRICS metrics = {sizeof(NONCLIENTMETRICS)};
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
		api.SetStr("Clock", "Font", metrics.lfCaptionFont.lfFaceName);
		
		if(ver == 0){ // very likely a new installation
			FirstTimeSetup(FRESH);
			return 1;
		}
	}
	
	
	/// old installation, set update flags if any
	switch(ver) {
	case _2_2_0:
		updateflags |= SFORMAT_EFFICIENT|SFORMAT_LESSMEM|SFORMAT_FEATURE|SFORMAT_TEXTPOSITION;
		compatibilityflags |= SCOMPAT_FORMAT|SCOMPAT_TIMERS;
		/* fall through */
		
	case _2_3_0:
		//updateflags |= SFORMAT_SILENT;
		/* fall through */
		
	case _2_4_0:
		updateflags |= SFORMAT_SILENT;
		/* fall through */
		
	case CURRENT:
		break;
		
	default:{
		int ans = MessageBox(NULL,
							"Seems like you've been using a newer version.\n"
							"Some settings might not be readable\n"
							"by this older version and you could loose them.\n"
							"\n"
							"Run this version anyway?","T-Clock downgraded?",MB_OKCANCEL|MB_ICONINFORMATION);
		if(ans != IDOK)
			return -1;
		ConvertSettings(ver); // should do nothing, just downgrade our version number
		return 0;}
	}
	
	
	/// check if update is "required"
	if(updateflags){
		int ans;
		if(updateflags!=SFORMAT_SILENT){
			int flags;
			char* pos = msg;
			pos += wsprintf(pos,
							"T-Clock stores now some of its settings differently.\n"
							"Don't worry though, we'll convert your settings\n"
							"to the new format and nothing should get lost.\n"
							"\n"
							"The new format is simply:");
			for(flags=updateflags>>1/*ignore SFORMAT_SILENT*/,ans=0; flags; flags>>=1,++ans){
				if(flags&1)
					pos += wsprintf(pos,"\n	+ %s",SFORMAT[ans]);
			}
			if(compatibilityflags){
				pos += wsprintf(pos,"\n\n"
								"If you want to go back to an old version later on,\n"
								"you could/will lose: (be warned)");
				for(flags=compatibilityflags,ans=0; flags; flags>>=1,++ans){
					if(flags&1)
						pos+=wsprintf(pos,"\n	- %s",SCOMPAT[ans]);
				}
			}
			pos += wsprintf(pos,"\n\nRun T-Clock and convert settings now?");
			ans = MessageBox(NULL,msg,"You've just updated T-Clock",MB_OKCANCEL|MB_ICONINFORMATION);
		}else // silent update
			ans = IDOK;
		if(ans != IDOK)
			return -1;
		ConvertSettings(ver);
		return 2;
	}
	return 0;
}

void ConvertSettings(VERSION ver) {
	char buf[MAX_PATH];
	int idx, idx2;
	size_t len;
	
	switch(ver){
	case FRESH:
	case _2_2_0:
		// update font rotate (from none,left,right to 0-360 angle)
		api.GetStr("Clock", "FontRotateDirection", buf, sizeof(buf), "");
		switch(*buf){
		case 'L': api.SetInt("Clock", "Angle", 90); break;
		case 'R': api.SetInt("Clock", "Angle", 270); break;
		}
		api.DelValue("Clock","FontRotateDirection");
		// Reset text position; we now center text automatically (and it's relative to that, not the upper area)
		// All settings were supposed to be one-off converts, not repeating if "Ver" resets, but that wouldn't work here
		api.SetInt("Clock", "ClockHeight", 0);
		api.SetInt("Clock", "ClockWidth", 0);
		api.SetInt("Clock", "HorizPos", 0);
		api.SetInt("Clock", "VertPos", 0);
		// remove "ID" from "Timers" as no longer used
		len = wsprintf(buf, "Timers\\Timer");
		idx2 = api.GetInt("Timers","NumberOfTimers",0);
		for(idx=0; idx<idx2; ){
			wsprintf(buf+len, "%d", ++idx);
			api.DelValue(buf, "ID");
		}
		/* fall through */
		
	case _2_3_0:
		if(GetStartupFile(NULL,buf)){
			DeleteFile(buf);
			AddStartup(NULL);
		}
		/* fall through */
		
	case _2_4_0:{
		int is12h, hour;
		// convert alarms to use 24h format
		len = wsprintf(buf, "Alarm");
		idx2 = GetAlarmNum();
		for(idx=0; idx<idx2; ){
			wsprintf(buf+len, "%d", ++idx);
			is12h = api.GetInt(buf, "Hour12", 0);
			if(is12h){ // convert to 24h format
				hour = api.GetInt(buf, "Hour", 12);
				api.SetInt(buf, "Hour", _12hTo24h(hour,api.GetInt(buf,"PM",0)));
				api.SetInt(buf, "12h", is12h);
			}
			api.DelValue(buf, "Hour12");
			api.DelValue(buf, "PM");
		}
		// make sure "new line" format was set to one or zero
		/// @note : on next backward incompatible change, remove "Kaigyo" key and this "if"
		if(api.GetInt("Format", "Lf", -1) == -1)
			api.SetInt("Format", "Lf", (api.GetInt("Format","Kaigyo",0) ? BST_CHECKED : BST_UNCHECKED));
		// convert old 12h switch - h/w(±) -> HH/W
		if(!api.GetInt("Format","Hour12",1)){
			char converted = 0;
			char fmt[MAX_FORMAT];
			size_t fmtlen;
			char* pos;
			fmtlen = api.GetStr("Format", "CustomFormat", fmt, MAX_FORMAT, "");
			for(pos=fmt; *pos; ){
				if(pos[0] == '"') {
					do{
						for(++pos; *pos&&*pos++!='"'; );
					}while(*pos == '"');
					if(!*pos)
						break;
				}
				if(pos[0] == 'S'){ // only format that also includes "h"
					int width, padding;
					++pos;
					api.GetFormat((const char**)&pos, &width, &padding);
					continue;
				}
				if(pos[0] == 'h'){
					++converted;
					pos[0] = 'H';
					if(pos[1] == 'h'){
						pos[1] = 'H';
						++pos;
					}else if(fmtlen+1 < MAX_FORMAT){
						++pos;
						++fmtlen; // we also copy null char
						memmove(pos+1, pos, fmtlen-(pos-fmt));
						*pos = 'H';
					}
				}else if(pos[0] == 'w' && (pos[1]=='+'||pos[1]=='-')){
					++converted;
					pos[0] = 'W';
				}
				++pos;
			}
			if(converted){
				api.SetStr("Format", "CustomFormat", fmt);
				if(api.GetInt("Format", "Custom", 0)){
					api.SetStr("Format", "Format", fmt);
				}
			}
		}}
		/* fall through */
		
	case CURRENT:
		break;
	}
	FirstTimeSetup(ver);
}

void FirstTimeSetup(VERSION from_version) {
	union{
		unsigned short entryS;
		char entry[3];
	} u;
	switch(from_version) {
	case FRESH:
		u.entry[2] = '\0';
		u.entryS = '0'|('1'<<8); // left, 1 click
		api.SetInt(REG_MOUSE, u.entry, MOUSEFUNC_SHOWCALENDER);
		u.entryS = '1'|('1'<<8); // right, 1 click
		api.SetInt(REG_MOUSE, u.entry, MOUSEFUNC_MENU);
		u.entryS = '2'|('1'<<8); // middle, 1 click
		api.SetInt(REG_MOUSE, u.entry, IDM_STOPWATCH);
		
		u.entryS = (api.OS >= TOS_VISTA ? BST_INDETERMINATE : BST_UNCHECKED);
		if(!u.entryS) { /// @todo : XP: measure taskbar height to choose font size and or multiline/singleline (small vs "normal" taskbar)
			HWND hwnd = FindWindow("Shell_TrayWnd", NULL);
			if(hwnd) {
				RECT rc; GetClientRect(hwnd, &rc);
				if(rc.bottom > rc.right) // vertical taskbar
					u.entryS = BST_CHECKED;
			}
		}
		api.SetInt("Format", "Lf", u.entryS);
		/* fall through */
	case _2_2_0:
	case _2_3_0:
		/* fall through */
	case _2_4_0:
		// first time update check (firewall warning etc.)
		api.ShellExecute(NULL, "misc\\Options", "-unotify", NULL, SW_HIDE);
		/* fall through */
	case CURRENT:
		break;
	}
	api.SetInt("", "Ver", CURRENT);
}
