typedef struct{
	const wchar_t* reg;
	char mcsc;
} CalendarColor;

const CalendarColor g_calendar_color[GROUP_CALENDAR_COLOR_NUM] = {
	{L"COuterBG", MCSC_BACKGROUND},
	{L"CFore", MCSC_TEXT},
	{L"CBack", MCSC_MONTHBK},
	{L"CTitle", MCSC_TITLETEXT},
	{L"CTitleBG", MCSC_TITLEBK},
	{L"CTrail", MCSC_TRAILINGTEXT},
};
