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

HANDLE h_svc_events, h_wait_cleanup;

wchar_t gdipp_hook_path[MAX_PATH];
const size_t MAX_ENV_LEN = 64;
wchar_t hook_env_str[MAX_ENV_LEN];

map<ULONG, HANDLE> h_hook_events, h_user_tokens;
map<ULONG, PROCESS_INFORMATION> pi_hooks;

BOOL start_hook(ULONG session_id)
{
	BOOL b_ret;

	h_user_tokens.insert(pair<ULONG, HANDLE>(session_id, HANDLE()));
	b_ret = WTSQueryUserToken(session_id, &h_user_tokens[session_id]);
	if (!b_ret)
		return FALSE;

	// this event handle is inheritable
	SECURITY_ATTRIBUTES inheritable_sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	h_hook_events[session_id] = CreateEvent(&inheritable_sa, TRUE, FALSE, NULL);
	if (h_hook_events[session_id] == NULL)
		return FALSE;

	swprintf_s(hook_env_str, L"h_gdipp_hook_wait=%p%c", h_hook_events[session_id], 0);

	STARTUPINFOW si = {sizeof(STARTUPINFO)};

	pi_hooks.insert(pair<ULONG, PROCESS_INFORMATION>(session_id, PROCESS_INFORMATION()));
	return CreateProcessAsUserW(h_user_tokens[session_id], gdipp_hook_path, NULL, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, hook_env_str, NULL, &si, &pi_hooks[session_id]);
}

void stop_hook(ULONG session_id)
{
	SetEvent(h_hook_events[session_id]);
	WaitForSingleObject(pi_hooks[session_id].hProcess, INFINITE);

	CloseHandle(pi_hooks[session_id].hThread);
	CloseHandle(pi_hooks[session_id].hProcess);
	CloseHandle(h_user_tokens[session_id]);
	CloseHandle(h_hook_events[session_id]);
}

VOID set_svc_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// fill in the SERVICE_STATUS structure
	svc_status.dwCurrentState = dwCurrentState;
	svc_status.dwWin32ExitCode = dwWin32ExitCode;
	svc_status.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_RUNNING)
		svc_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;
	else
		svc_status.dwControlsAccepted = 0;

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
		set_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
		
		b_ret = SetEvent(h_svc_events);
		assert(b_ret);

		return NO_ERROR;
	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;
	case SERVICE_CONTROL_SESSIONCHANGE:
		if (dwEventType == WTS_SESSION_LOGON)
		{
			b_ret = start_hook(reinterpret_cast<WTSSESSION_NOTIFICATION *>(lpEventData)->dwSessionId);

			if (b_ret)
				return NO_ERROR;
			else
				return GetLastError();
		}
		else if (dwEventType == WTS_SESSION_LOGOFF)
		{
			stop_hook(reinterpret_cast<WTSSESSION_NOTIFICATION *>(lpEventData)->dwSessionId);
			
			return NO_ERROR;
		}
		else
			return ERROR_CALL_NOT_IMPLEMENTED;
	default:
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

VOID CALLBACK exit_cleanup(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	BOOL b_ret;

	b_ret = UnregisterWait(h_wait_cleanup);
	assert(b_ret || GetLastError() == ERROR_IO_PENDING);

	for (map<ULONG, HANDLE>::const_iterator session_iter = h_hook_events.begin(); session_iter != h_hook_events.end(); session_iter++)
		stop_hook(session_iter->first);

	set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
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

	h_svc_events = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (h_svc_events == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	b_ret = RegisterWaitForSingleObject(&h_wait_cleanup, h_svc_events, exit_cleanup, NULL, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	assert(b_ret);

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
	
	/*
	service process and its child processes run in session 0
	some functions of gdipp Enumerator may require interactive session (session ID > 0)
	use CreateProcessAsUser to create process in the active user's session
	*/
	const DWORD active_session_id = WTSGetActiveConsoleSessionId();
	if (active_session_id != 0 && active_session_id != 0xFFFFFFFF)
		start_hook(active_session_id);

	// report running status when initialization is complete
	set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);
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
	swprintf_s(env_str, L"gdipp_wait_handle=%p%c", h_svc_events, 0);

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