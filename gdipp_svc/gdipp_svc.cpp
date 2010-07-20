#include "stdafx.h"
#include <gdipp_common.h>

using namespace std;

#ifdef _M_X64
#define SVC_NAME TEXT("gdipp_svc_64")
#else
#define SVC_NAME TEXT("gdipp_svc_32")
#endif // _M_X64

SERVICE_STATUS svc_status = {};
SERVICE_STATUS_HANDLE h_svc_status = NULL;
HANDLE h_svc_stop_event = NULL;

HANDLE h_user_token;
PROCESS_INFORMATION pi_hook;

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
	SetServiceStatus(h_svc_status, &svc_status);
}

VOID WINAPI svc_ctrl_handler(DWORD dwCtrl)
{
	// handle the requested control code
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP:
		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(h_svc_stop_event);
		set_svc_status(svc_status.dwCurrentState, NO_ERROR, 0);
		return;
	}
}

BOOL start_hook()
{
#ifdef _M_X64
	const wchar_t *gdipp_hook_name = L"gdipp_hook_64.exe";
#else
	const wchar_t *gdipp_hook_name = L"gdipp_hook_32.exe";
#endif

	wchar_t gdipp_hook_path[MAX_PATH];
	if (!gdipp_get_dir_file_path(NULL, gdipp_hook_name, gdipp_hook_path))
		return FALSE;

	/*
	service process and its child processes run in session 0
	some functions of gdipp Enumerator may require interactive session (session ID > 0)
	use CreateProcessAsUser to create process in the active user's session
	*/
	const DWORD user_session_id = WTSGetActiveConsoleSessionId();
	if (user_session_id == 0xFFFFFFFF)
		return FALSE;
	assert(user_session_id > 0);

	//HANDLE h_u;
	if (!WTSQueryUserToken(user_session_id, &h_user_token))
	//if (!WTSQueryUserToken(user_session_id, &h_u))
	{
		gdipp_debug_output(L"Cannot query user token");
		return FALSE;
	}

// 	if (!DuplicateTokenEx(h_u, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &h_user_token))
// 	{
// 		gdipp_debug_output(L"Cannot duplicate user token");
// 		return FALSE;
// 	}
//  
// 	CloseHandle(h_u);

	if (!ImpersonateLoggedOnUser(h_user_token))
	{
		gdipp_debug_output(L"Cannot impersonate");
		return FALSE;
	}

	STARTUPINFOW si = {};
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = L"winsta0\\default";
	
	return CreateProcessAsUserW(h_user_token, gdipp_hook_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi_hook);
}

void stop_hook()
{
	BOOL b_ret;

	CloseHandle(pi_hook.hThread);
	CloseHandle(pi_hook.hProcess);

	b_ret = RevertToSelf();
	assert(b_ret);

	CloseHandle(h_user_token);
}

VOID WINAPI svc_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// register the handler function for the service
	h_svc_status = RegisterServiceCtrlHandler(SVC_NAME, svc_ctrl_handler);
	if (h_svc_status == NULL)
		return;

	// these SERVICE_STATUS members remain as set here
	svc_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	svc_status.dwWin32ExitCode = NO_ERROR;

	// report initial status to the SCM
	set_svc_status(SERVICE_START_PENDING, NO_ERROR, INFINITE);

	h_svc_stop_event = CreateEventW(NULL, TRUE, FALSE, GDIPP_SVC_EVENT_NAME);
	if (h_svc_stop_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		gdipp_debug_output(L"Cannot create event.");
		return;
	}
	
	while (true)
	{
		if (start_hook())
		{
			gdipp_debug_output(L"successfully started hook");

			// report running status when initialization is complete
			set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);

			/*
			wait for stop event and termination of the hook subprocess
			if stop event is fired first, stop the service
			otherwise, respawn the hook subprocess
			*/

			HANDLE h_wait[2] = {h_svc_stop_event, pi_hook.hProcess};
			const DWORD wait_ret = WaitForMultipleObjects(2, h_wait, FALSE, INFINITE);

			if (wait_ret - WAIT_OBJECT_0 != 1)
			{
				set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

				stop_hook();

				break;
			}
			else
			{
				gdipp_debug_output(wait_ret + 20);
				gdipp_debug_output((DWORD) h_svc_stop_event);
				gdipp_debug_output((DWORD) pi_hook.hProcess);

				stop_hook();

				// wait 5 seconds before respawning hook
				Sleep(5000);
			}
		}
		else
			gdipp_debug_output(GetLastError());
	}

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
}

 // #define svc_debug

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef svc_debug
	BOOL b_ret;

	h_svc_stop_event = CreateEventW(NULL, TRUE, FALSE, GDIPP_SVC_EVENT_NAME);
	if (h_svc_stop_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return EXIT_FAILURE;
	}

	PROCESS_INFORMATION pi = {};
	b_ret = start_hook(&pi);
	if (!b_ret)
		return EXIT_FAILURE;

	Sleep(5000);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

#else
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, (LPSERVICE_MAIN_FUNCTION) svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcher(dispatch_table);
#endif // svc_debug

	return EXIT_SUCCESS;
}