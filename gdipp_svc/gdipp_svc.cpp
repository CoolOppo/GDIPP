#include "stdafx.h"
#include "mon.h"
#include "inject.h"

#ifdef _M_X64
#define SVC_NAME TEXT("gdipp_svc_64")
#else
#define SVC_NAME TEXT("gdipp_svc_32")
#endif

SERVICE_STATUS			svc_status = {0};
SERVICE_STATUS_HANDLE	svc_status_handle = NULL;
HANDLE					svc_stop_event = NULL;

VOID set_svc_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// fill in the SERVICE_STATUS structure
	svc_status.dwCurrentState = dwCurrentState;
	svc_status.dwWin32ExitCode = dwWin32ExitCode;
	svc_status.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		// no control is accepted in start pending state
		svc_status.dwControlsAccepted = 0;
	else
		svc_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
		svc_status.dwCheckPoint = 0;
	else
		svc_status.dwCheckPoint = dwCheckPoint++;

	// report the status of the service to the SCM
	SetServiceStatus(svc_status_handle, &svc_status);
}

VOID WINAPI svc_ctrl_handler(DWORD dwCtrl)
{
	// handle the requested control code
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP:
		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(svc_stop_event);
		set_svc_status(svc_status.dwCurrentState, NO_ERROR, 0);
		return;
	}
}

VOID WINAPI svc_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// register the handler function for the service
	svc_status_handle = RegisterServiceCtrlHandler(SVC_NAME, svc_ctrl_handler);
	if (svc_status_handle == NULL)
		return;

	// these SERVICE_STATUS members remain as set here
	svc_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	svc_status.dwWin32ExitCode = NO_ERROR;

	// report initial status to the SCM
	set_svc_status(SERVICE_START_PENDING, NO_ERROR, 3000);

	gdipp_setting::instance().init(NULL);

	// create named event to synchronize between service and gdimm
	svc_stop_event = CreateEventW(NULL, TRUE, FALSE, SVC_EVENT_NAME);
	if (svc_stop_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	svc_injector::instance().init_inject();

	// monitor future processes
	if (svc_mon::instance().start_monitor())
	{
		// report running status when initialization is complete
		set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);

		// wait for stop event
		WaitForSingleObject(svc_stop_event, INFINITE);

		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

		svc_mon::instance().stop_monitor();
	}

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
}

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
#ifdef NDEBUG
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, (LPSERVICE_MAIN_FUNCTION) svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcher(dispatch_table);
#else
	gdipp_setting::instance().init(NULL);

	svc_injector::instance().init_inject();

	if (svc_mon::instance().start_monitor())
	{
		Sleep(3000);
		svc_mon::instance().stop_monitor();
	}
#endif
}