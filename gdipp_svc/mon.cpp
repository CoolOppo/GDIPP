#include "stdafx.h"
#include "gdipp_svc.h"

svc_mon::svc_mon()
{
	_svc = NULL;
	_sink = NULL;
}

bool svc_mon::start_monitor()
{
	BOOL b_ret;
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return false;

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (hr != S_OK)
		return false;

	CComPtr<IWbemLocator> loc;
	hr = loc.CoCreateInstance(CLSID_WbemLocator);
	if (hr != S_OK)
		return false;

	hr = loc->ConnectServer(bstr_t("\\\\.\\root\\CIMV2"), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &_svc);
	if (hr != S_OK)
		return false;

	hr = CoSetProxyBlanket(_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (hr != S_OK)
		return false;

	_sink = new sink_inject;
	_sink->AddRef();

	// get setting file path
	wchar_t setting_path[MAX_PATH];
	b_ret = gdipp_get_dir_file_path(NULL, L"gdipp_setting.xml", setting_path);
	assert(b_ret);

	gdipp_init_setting();
	b_ret = gdipp_load_setting(setting_path);
	assert(b_ret);

	const wchar_t *interval_str = gdipp_get_service_setting(L"poll_interval");
	if (interval_str == NULL)
		interval_str = L"1";

	const int query_str_len = 100;
	wchar_t query_str[query_str_len];
	swprintf(
		query_str,
		query_str_len,
		L"SELECT * FROM __InstanceCreationEvent WITHIN %s WHERE TargetInstance ISA 'Win32_Process'",
		interval_str);
	hr = _svc->ExecNotificationQueryAsync(
		bstr_t("WQL"),
		bstr_t(query_str),
		0,
		NULL,
		_sink);
	if (hr != S_OK)
		return false;

	return true;
}

void svc_mon::stop_monitor()
{
	HRESULT hr;

	hr = _svc->CancelAsyncCall(_sink);
	assert(hr == S_OK);

	if (_svc != NULL)
		_svc->Release();

	if (_sink != NULL)
		_sink->Release();

	CoUninitialize();
}