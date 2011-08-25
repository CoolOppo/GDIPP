#include "stdafx.h"
#include "gdipp_lib/helper.h"
#include "gdipp_server/freetype.h"
#include "gdipp_server/rpc_impl.h"

namespace gdipp
{

#define SVC_NAME L"gdipp_svc"

SERVICE_STATUS svc_status = {};
SERVICE_STATUS_HANDLE h_svc_status = NULL;

HANDLE h_svc_events, h_wait_cleanup, h_rpc_thread;

const size_t MAX_ENV_LEN = 64;
wchar_t hook_env_str[MAX_ENV_LEN];

std::map<ULONG, HANDLE> h_user_tokens, h_hook_events;
std::map<ULONG, PROCESS_INFORMATION> pi_hooks_32, pi_hooks_64;

BOOL hook_proc(HANDLE h_user_token, HANDLE h_hook_event, const wchar_t *gdipp_hook_name, PROCESS_INFORMATION &pi)
{
	wchar_t gdipp_hook_path[MAX_PATH];

	if (!get_dir_file_path(NULL, gdipp_hook_name, gdipp_hook_path))
		return FALSE;

	swprintf_s(hook_env_str, L"h_gdipp_hook_wait=%p%c", h_hook_event, 0);
	STARTUPINFOW si = {sizeof(STARTUPINFO)};

	return CreateProcessAsUserW(h_user_token, gdipp_hook_path, NULL, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, hook_env_str, NULL, &si, &pi);
}

BOOL start_hook(ULONG session_id)
{
	hook_config hook_conf = config_mgr_instance.get_hook_config();

	// make the event handle inheritable
	SECURITY_ATTRIBUTES inheritable_sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	BOOL b_ret, hook_success = TRUE;
	HANDLE h_user_token, h_hook_event;

	b_ret = WTSQueryUserToken(session_id, &h_user_token);
	if (!b_ret)
	{
		hook_success = FALSE;
		goto post_hook;
	}

	// use the linked token if exists
	// needed in UAC-enabled scenarios and Run As Administrator
	TOKEN_LINKED_TOKEN linked_token;
	DWORD token_info_len;
	b_ret = GetTokenInformation(h_user_token, TokenLinkedToken, &linked_token, sizeof(TOKEN_LINKED_TOKEN), &token_info_len);
	if (b_ret)
	{
		CloseHandle(h_user_token);
		h_user_token = linked_token.LinkedToken;
	}

	h_hook_event = CreateEvent(&inheritable_sa, TRUE, FALSE, NULL);
	if (h_hook_event == NULL)
	{
		hook_success = FALSE;
		goto post_hook;
	}

	if (hook_conf.proc_32_bit)
	{
		const wchar_t *gdipp_hook_name_32 = L"gdipp_hook_32.exe";
		PROCESS_INFORMATION pi;

		if (hook_proc(h_user_token, h_hook_event, gdipp_hook_name_32, pi))
			pi_hooks_32[session_id] = pi;
		else
			hook_success = FALSE;
	}

	if (hook_conf.proc_32_bit)
	{
		const wchar_t *gdipp_hook_name_64 = L"gdipp_hook_64.exe";
		PROCESS_INFORMATION pi;

		if (hook_proc(h_user_token, h_hook_event, gdipp_hook_name_64, pi))
			pi_hooks_64[session_id] = pi;
		else
			hook_success = FALSE;
	}

post_hook:
	if (hook_success)
	{
		h_user_tokens[session_id] = h_user_token;
		h_hook_events[session_id] = h_hook_event;
	}
	else
	{
		if (h_user_token)
			CloseHandle(h_user_token);

		if (h_hook_event)
			CloseHandle(h_hook_event);
	}

	return b_ret;
}

void stop_hook(ULONG session_id)
{
	std::map<ULONG, PROCESS_INFORMATION>::const_iterator pi_iter_32, pi_iter_64;
	HANDLE h_hook_processes[2] = {};

	pi_iter_32 = pi_hooks_32.find(session_id);
	if (pi_iter_32 != pi_hooks_32.end())
		h_hook_processes[0] = pi_iter_32->second.hProcess;

	pi_iter_64 = pi_hooks_64.find(session_id);
	if (pi_iter_64 != pi_hooks_64.end())
		h_hook_processes[1] = pi_iter_64->second.hProcess;

	// notify and wait hook subprocesses to exit
	SetEvent(h_hook_events[session_id]);
	WaitForMultipleObjects(2, h_hook_processes, TRUE, INFINITE);

	// clean up

	if (pi_iter_32 != pi_hooks_32.end())
	{
		CloseHandle(pi_iter_32->second.hThread);
		CloseHandle(pi_iter_32->second.hProcess);
		pi_hooks_32.erase(pi_iter_32);
	}

	if (pi_iter_64 != pi_hooks_64.end())
	{
		CloseHandle(pi_iter_64->second.hThread);
		CloseHandle(pi_iter_64->second.hProcess);
		pi_hooks_64.erase(pi_iter_64);
	}

	CloseHandle(h_user_tokens[session_id]);
	CloseHandle(h_hook_events[session_id]);
	h_user_tokens.erase(session_id);
	h_hook_events.erase(session_id);
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
		svc_status.dwCheckPoint = ++dwCheckPoint;

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

	for (std::map<ULONG, HANDLE>::const_iterator session_iter = h_hook_events.begin(); session_iter != h_hook_events.end(); ++session_iter)
		stop_hook(session_iter->first);

	b_ret = stop_gdipp_rpc_server();
	if (b_ret)
	{
		const DWORD wait_ret = WaitForSingleObject(h_rpc_thread, INFINITE);
		assert(wait_ret == WAIT_OBJECT_0);
	}

	destroy_freetype();

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

	// clean up when event is set
	b_ret = RegisterWaitForSingleObject(&h_wait_cleanup, h_svc_events, exit_cleanup, NULL, INFINITE, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	assert(b_ret);

	initialize_freetype();

	// initialize RPC for font service
	h_rpc_thread = CreateThread(NULL, 0, start_gdipp_rpc_server, NULL, 0, NULL);
	if (h_rpc_thread == NULL)
	{
		set_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	/*
	service process and its child processes run in session 0
	some functions in gdipp may require interactive session (session ID > 0)
	use CreateProcessAsUser to create process in the active user's session
	*/
	const DWORD active_session_id = WTSGetActiveConsoleSessionId();
	if (active_session_id != 0xFFFFFFFF)
		start_hook(active_session_id);

	// report running status when initialization is complete
	set_svc_status(SERVICE_RUNNING, NO_ERROR, 0);
}

}

// #define svc_debug

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef svc_debug
	Sleep(5000);
#endif  // svc_debug

	SERVICE_TABLE_ENTRY dispatch_table[] =
	{
		{ SVC_NAME, gdipp::svc_main },
		{ NULL, NULL },
	};

	StartServiceCtrlDispatcherW(dispatch_table);

	return EXIT_SUCCESS;
}
