#include "stdafx.h"
#include "gdipp_svc.h"

#ifdef _M_X64
#define SVC_NAME TEXT("gdipp_svc_64")
#else
#define SVC_NAME TEXT("gdipp_svc_32")
#endif

#ifdef _M_X64
#define SVC_EVENT_PREFIX L"Global\\gdipp_svc_event_64"
#else
#define SVC_EVENT_PREFIX L"Global\\gdipp_svc_event_32"
#endif

SERVICE_STATUS			svc_status = {0};
SERVICE_STATUS_HANDLE	svc_status_handle = NULL;
HANDLE					svc_stop_event = NULL;

svc_injector injector_instance;
svc_mon mon_instance;
gdipp_setting setting_instance;

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

bool create_svc_event()
{
	// create named event to synchronize between service and gdimm

	// service event name = event name prefix + non-duplicable number (tick count)
	// use this dynamic event name to avoid existing same-named event, opened by lingering gdimm.dll
	wstringstream ss;
	ss << SVC_EVENT_PREFIX;
	ss << GetTickCount();

	svc_stop_event = CreateEventW(NULL, TRUE, FALSE, ss.str().c_str());
	if (svc_stop_event == NULL)
		return false;

	injector_instance.init_payload(ss.str().data());
	return true;
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
	set_svc_status(SERVICE_START_PENDING, NO_ERROR, INFINITE);

	if (!create_svc_event())
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	setting_instance.init(NULL);

	injector_instance.initial_inject();

	// monitor future processes
	if (mon_instance.start_monitor())
	{
		// report running status when initialization is complete
		set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);

		// wait for stop event
		WaitForSingleObject(svc_stop_event, INFINITE);

		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

		mon_instance.stop_monitor();
	}

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
}

 // #define svc_debug

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
#ifdef svc_debug
	if (!create_svc_event())
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return 0;
	}

	setting_instance.init(NULL);

	//injector_instance.initial_inject();

	if (mon_instance.start_monitor())
	{
		Sleep(10000);
		mon_instance.stop_monitor();
	}
#else
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, (LPSERVICE_MAIN_FUNCTION) svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcher(dispatch_table);
#endif

	return 0;
}