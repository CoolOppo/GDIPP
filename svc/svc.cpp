#include "stdafx.h"
#include <tlhelp32.h>

#define SVCNAME TEXT("gdimm")

SERVICE_STATUS			gSvcStatus;
SERVICE_STATUS_HANDLE	gSvcStatusHandle;
HANDLE					ghSvcStopEvent = NULL;
BOOL hooked = FALSE;

void DebugOutput(const TCHAR *str)
{
	FILE *f = fopen("C:\\svc_debug.txt", "a+");
	fwprintf(f, TEXT("%s\n"), str);
	fclose(f);
}

void DebugOutput(DWORD num)
{
	FILE *f = fopen("C:\\svc_debug.txt", "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
}

DWORD GetProcIdByName(TCHAR *procName)
{
	DWORD ret = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	if (!Process32First(hSnapshot, &pe32))
	{
		CloseHandle(hSnapshot);
		return 0;
	}

	do
	{
		if (lstrcmpi(pe32.szExeFile, procName) == 0)
		{
			ret = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return ret;
}

VOID ReportSvcStatus(DWORD dwCurrentState)
{
	// fill in the SERVICE_STATUS structure
	gSvcStatus.dwCurrentState = dwCurrentState;

	if (dwCurrentState == SERVICE_START_PENDING)
		// no control is accepted in start pending state
		gSvcStatus.dwControlsAccepted = 0;
	else
		gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	// report the status of the service to the SCM
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
	// handle the requested control code
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ReportSvcStatus(SERVICE_STOP_PENDING);
		return;
	default:
		break;
	}

	// send current status
	ReportSvcStatus(gSvcStatus.dwCurrentState);
}

VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
	// init here

	// report running status when initialization is complete
	ReportSvcStatus(SERVICE_RUNNING);
}

DWORD __stdcall TestThread(void* InParams)
{
	while(TRUE)
	{
		DebugOutput(TEXT("TestThread"));
		Sleep(1000);
	}

	return 0;
}

DWORD __stdcall HijackEntry(void* InParams)
{
	DebugOutput(TEXT("HijackEntry"));
	return 0;
}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// register the handler function for the service
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
	if (gSvcStatusHandle == 0)
		return;

	// these SERVICE_STATUS members remain as set here
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwWin32ExitCode = NO_ERROR;
	gSvcStatus.dwServiceSpecificExitCode = 0;
	gSvcStatus.dwCheckPoint = 0;
	gSvcStatus.dwWaitHint = 0;

	// report initial status to the SCM
	ReportSvcStatus(SERVICE_START_PENDING);

	// perform service-specific initialization and work
	SvcInit(dwArgc, lpszArgv);

	while (gSvcStatus.dwCurrentState == SERVICE_RUNNING)
	{
		// service code here

		Sleep(100);
	}

	ReportSvcStatus(SERVICE_STOPPED);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain},
		{NULL, NULL},
	};

	StartServiceCtrlDispatcher(DispatchTable);
}