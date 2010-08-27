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

HANDLE h_svc_event;
HANDLE h_wait_cleanup, h_wait_hook;

wchar_t gdipp_hook_path[MAX_PATH];
const size_t MAX_ENV_LEN = 64;
wchar_t hook_env_str[MAX_ENV_LEN];

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

DWORD WINAPI svc_ctrl_handler(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	BOOL b_ret;

	// handle the requested control code
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
		
		b_ret = SetEvent(h_svc_event);
		assert(b_ret);

		return NO_ERROR;
	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;
	default:
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

BOOL start_hook()
{
	BOOL b_ret;

	/*
	service process and its child processes run in session 0
	some functions of gdipp Enumerator may require interactive session (session ID > 0)
	use CreateProcessAsUser to create process in the active user's session
	*/
	const DWORD user_session_id = WTSGetActiveConsoleSessionId();
	if (user_session_id == 0xFFFFFFFF)
		return FALSE;
	assert(user_session_id > 0);

	b_ret = WTSQueryUserToken(user_session_id, &h_user_token);
	assert(b_ret);

// 	if (!ImpersonateLoggedOnUser(h_user_token))
// 	{
// 		gdipp_debug_output(L"Cannot impersonate");
// 		return FALSE;
// 	}

	STARTUPINFOW si = {sizeof(STARTUPINFO)};
	//si.lpDesktop = L"winsta0\\default";
	
	return CreateProcessAsUserW(h_user_token, gdipp_hook_path, NULL, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, hook_env_str, NULL, &si, &pi_hook);
}

void stop_hook()
{
//	BOOL b_ret;

	CloseHandle(pi_hook.hThread);
	CloseHandle(pi_hook.hProcess);

// 	b_ret = RevertToSelf();
// 	assert(b_ret);

	CloseHandle(h_user_token);
}

VOID CALLBACK exit_cleanup(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	BOOL b_ret;

	b_ret = UnregisterWait(h_wait_hook);
	assert(b_ret || GetLastError() == ERROR_IO_PENDING);

	b_ret = UnregisterWait(h_wait_cleanup);
	assert(b_ret || GetLastError() == ERROR_IO_PENDING);

	stop_hook();

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
}

VOID CALLBACK respawn_hook(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	BOOL b_ret;

	b_ret = UnregisterWait(h_wait_hook);
	assert(b_ret || GetLastError() == ERROR_IO_PENDING);

	stop_hook();

	// wait 5 seconds before respawning hook
	Sleep(5000);

	start_hook();

	b_ret = RegisterWaitForSingleObject(&h_wait_hook, pi_hook.hProcess, respawn_hook, NULL, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	assert(b_ret);
}

VOID WINAPI svc_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	BOOL b_ret;

	// register the handler function for the service
	h_svc_status = RegisterServiceCtrlHandlerExW(SVC_NAME, svc_ctrl_handler, NULL);
	if (h_svc_status == NULL)
		return;

	// these SERVICE_STATUS members remain as set here
	svc_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	svc_status.dwWin32ExitCode = NO_ERROR;

	// report initial status to the SCM
	set_svc_status(SERVICE_START_PENDING, NO_ERROR, INFINITE);

	// the event is inheritable
	SECURITY_ATTRIBUTES event_sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	h_svc_event = CreateEvent(&event_sa, TRUE, FALSE, NULL);
	if (h_svc_event == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

#ifdef _M_X64
	const wchar_t *gdipp_hook_name = L"gdipp_hook_64.exe";
#else
	const wchar_t *gdipp_hook_name = L"gdipp_hook_32.exe";
#endif

	if (!gdipp_get_dir_file_path(NULL, gdipp_hook_name, gdipp_hook_path))
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	swprintf_s(hook_env_str, L"gdipp_wait_handle=%p%c", h_svc_event, 0);

	b_ret = start_hook();
	if (!b_ret)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	// report running status when initialization is complete
	set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);

	b_ret = RegisterWaitForSingleObject(&h_wait_cleanup, h_svc_event, exit_cleanup, NULL, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	assert(b_ret);

	b_ret = RegisterWaitForSingleObject(&h_wait_hook, pi_hook.hProcess, respawn_hook, NULL, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	assert(b_ret);
}

 // #define svc_debug

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef svc_debug
	BOOL b_ret;

	h_svc_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!b_ret)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return EXIT_FAILURE;
	}

	const size_t MAX_ENV_LEN = 64;
	wchar_t env_str[MAX_ENV_LEN];
	swprintf_s(env_str, L"gdipp_wait_handle=%p%c", h_svc_event, 0);

	b_ret = start_hook();
	if (!b_ret)
		return EXIT_FAILURE;

	Sleep(10000);
#else
	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcherW(dispatch_table);
#endif // svc_debug

	return EXIT_SUCCESS;
}