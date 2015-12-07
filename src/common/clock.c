#include "clock.h"

int LoadClockAPI(const wchar_t* dll_path, TClockAPI* _api) {
	typedef int (*SetupClockAPI_t)(int version, TClockAPI* api);
	SetupClockAPI_t SetupClockAPI;
	HMODULE dll = LoadLibrary(dll_path); // leak / auto-free on process exit
	SetupClockAPI = (SetupClockAPI_t)GetProcAddress(dll, "SetupClockAPI");
	if(!SetupClockAPI)
		return 1;
	return SetupClockAPI(CLOCK_API, _api);
}
