#include <windows.h>

int HaveSetTimePermissions()
{
	int ret = 0;
	HANDLE hToken;
	TOKEN_PRIVILEGES privs = {0};
	TOKEN_PRIVILEGES privs_old;
	DWORD privs_old_len;
	
	privs.PrivilegeCount = 1;
	privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
		return 0;
	if(LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &privs.Privileges[0].Luid)){
		if(AdjustTokenPrivileges(hToken, 0, &privs, sizeof(privs_old), &privs_old, &privs_old_len)
		&& GetLastError() == ERROR_SUCCESS){
			AdjustTokenPrivileges(hToken, 0, &privs_old, 0, NULL, NULL);
			ret = 1;
		}
	}
	CloseHandle(hToken);
	return ret;
}
