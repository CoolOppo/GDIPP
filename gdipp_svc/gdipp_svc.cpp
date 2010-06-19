#include "stdafx.h"
#include "gdipp_svc.h"
#include <gdipp_common.h>

using namespace std;

#ifdef _M_X64
#define SVC_NAME TEXT("gdipp_svc_64")
#define SVC_EVENT_NAME L"Local\\gdipp_svc_event_64"
#else
#define SVC_NAME TEXT("gdipp_svc_32")
#define SVC_EVENT_NAME L"Local\\gdipp_svc_event_32"
#endif // _M_X64

SERVICE_STATUS svc_status = {};
SERVICE_STATUS_HANDLE svc_status_handle = NULL;
HANDLE svc_stop_event = NULL;

svc_mon mon_instance;
DWORD user_session_id;

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

BOOL invoke_enum(const wchar_t *parameter)
{
#ifdef _M_X64
	const wchar_t *gdipp_enum_name = L"gdipp_enum_64.exe";
#else
	const wchar_t *gdipp_enum_name = L"gdipp_enum_32.exe";
#endif

	BOOL b_ret;

	HANDLE user_token;
	b_ret = WTSQueryUserToken(user_session_id, &user_token);
	if (!b_ret)
		return FALSE;

	wchar_t gdipp_enum_path[MAX_PATH];
	b_ret = gdipp_get_dir_file_path(NULL, gdipp_enum_name, gdipp_enum_path);
	if (!b_ret)
		return FALSE;

	wstring cmd_line = gdipp_enum_path;
	if (parameter != NULL)
		cmd_line += parameter;

	STARTUPINFO si = {};
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	
	b_ret = CreateProcessAsUserW(user_token, gdipp_enum_path, &cmd_line[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (b_ret)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	return FALSE;
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

	svc_stop_event = CreateEventW(NULL, TRUE, FALSE, SVC_EVENT_NAME);
	if (svc_stop_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
	
	gdipp_init_payload(GDIPP_INJECTOR_SERVICE);

	invoke_enum(NULL);

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

	invoke_enum(L" --eject");

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
}

 // #define svc_debug

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	/*
	service process and its child processes run in session 0
	some functions of gdipp Enumerator may require interactive session (session ID > 0)
	use CreateProcessAsUser to create process in the active user's session
	*/
	user_session_id = WTSGetActiveConsoleSessionId();
	if (user_session_id == 0xFFFFFFFF)
		return FALSE;
	assert(user_session_id > 0);

#ifdef svc_debug
	svc_stop_event = CreateEventW(NULL, TRUE, FALSE, SVC_EVENT_NAME);
	if (svc_stop_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
 
	gdipp_init_payload(GDIPP_INJECTOR_SERVICE);
 
	invoke_enum(NULL);
 
	if (mon_instance.start_monitor())
	{
		Sleep(5000);
		mon_instance.stop_monitor();
	}

	invoke_enum(L" --eject");
#else
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, (LPSERVICE_MAIN_FUNCTION) svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcher(dispatch_table);
#endif // svc_debug

	return 0;
}