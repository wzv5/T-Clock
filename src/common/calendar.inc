typedef struct{
	const char* reg;
	char mcsc;
} CalendarColor;

const CalendarColor g_calendar_color[CALENDAR_COLOR_NUM] = {
	{"COuterBG", MCSC_BACKGROUND},
	{"CFore", MCSC_TEXT},
	{"CBack", MCSC_MONTHBK},
	{"CTitle", MCSC_TITLETEXT},
	{"CTitleBG", MCSC_TITLEBK},
	{"CTrail", MCSC_TRAILINGTEXT},
};
