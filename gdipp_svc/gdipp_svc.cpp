#include "stdafx.h"
#include "mon.h"
#include "inject.h"
#include "setting.h"
#include <tlhelp32.h>
#include <vector>
using namespace std;

#define SVC_NAME TEXT("gdipp_svc")

SERVICE_STATUS			svc_status;
SERVICE_STATUS_HANDLE	svc_status_handle;

// enumerate all running processes
bool enum_processes(vector<DWORD> &proc_ids)
{
	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	const HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return false;

	if (Process32First(h_snapshot, &pe32))
	{
		do
		{
			if (!gdimm_setting::instance().is_proc_excluded(pe32.szExeFile))
				proc_ids.push_back(pe32.th32ProcessID);
		} while (Process32Next(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);
	return true;
}

VOID set_svc_status(DWORD dwCurrentState)
{
	// fill in the SERVICE_STATUS structure
	svc_status.dwCurrentState = dwCurrentState;

	if (dwCurrentState == SERVICE_START_PENDING)
		// no control is accepted in start pending state
		svc_status.dwControlsAccepted = 0;
	else
		svc_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	// report the status of the service to the SCM
	SetServiceStatus(svc_status_handle, &svc_status);
}

VOID WINAPI svc_ctrl_handler(DWORD dwCtrl)
{
	// handle the requested control code
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		set_svc_status(SERVICE_STOP_PENDING);
		break;
	default:
		// send current status
		set_svc_status(svc_status.dwCurrentState);
		break;
	}
}

VOID WINAPI svc_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// register the handler function for the service
	svc_status_handle = RegisterServiceCtrlHandler(SVC_NAME, svc_ctrl_handler);
	if (svc_status_handle == 0)
		return;

	// these SERVICE_STATUS members remain as set here
	svc_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	svc_status.dwWin32ExitCode = NO_ERROR;
	svc_status.dwServiceSpecificExitCode = 0;
	svc_status.dwCheckPoint = 0;
	svc_status.dwWaitHint = 0;

	// report initial status to the SCM
	set_svc_status(SERVICE_START_PENDING);

	gdimm_setting::instance().load_settings(SERVICE_BRANCH, NULL);

	// inject running processes
	if (gdimm_setting::instance().get_service_setting<bool>(TEXT("init_inject")))
	{
		vector<DWORD> proc_ids;
		if (enum_processes(proc_ids))
		{
			for (vector<DWORD>::const_iterator iter = proc_ids.begin(); iter != proc_ids.end(); iter++)
				svc_injector::instance().inject(*iter);
		}
	}

	// monitor future processes
	svc_mon::instance().start_monitor();

	// report running status when initialization is complete
	set_svc_status(SERVICE_RUNNING);

	while (svc_status.dwCurrentState == SERVICE_RUNNING)
		Sleep(1000);

	set_svc_status(SERVICE_STOP_PENDING);

	svc_mon::instance().stop_monitor();

	set_svc_status(SERVICE_STOPPED);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	BOOL b_ret;

	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{SVC_NAME, (LPSERVICE_MAIN_FUNCTION) svc_main},
		{NULL, NULL},
	};

	b_ret = StartServiceCtrlDispatcher(dispatch_table);
	assert(b_ret);
}