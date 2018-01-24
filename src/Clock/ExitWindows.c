#include "tclock.h"

static int GetShutdownPrivilege()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken)) {
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, NULL);
		return GetLastError() == ERROR_SUCCESS;
	}
	return 0;
}

int WindowsExit(int ewx_type)
{
	if(ewx_type == EWX_LOGOFF || GetShutdownPrivilege())
		return ExitWindowsEx(ewx_type | EWX_FORCEIFHUNG, 0);
	return 0;
}

int WindowsSleep(int standby)
{
	if(GetShutdownPrivilege())
		return SetSystemPowerState(standby, 0); // Win2k reports ERROR_NOT_SUPPORTED
	return 0;
}
