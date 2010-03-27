#include "stdafx.h"
#include "gdipp_svc.h"
#include "sink.h"

void svc_mon::release_all()
{
	_svc->Release();
	_loc->Release();
	_unsec_app->Release();
	_stub_unk->Release();
	_sink->Release();
	_stub_sink->Release();
}

bool svc_mon::start_monitor()
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return false;

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hr))
	{
		CoUninitialize();
		return false;
	}

	hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_loc));
	if (FAILED(hr))
	{
		CoUninitialize();
		return false;
	}

	hr = _loc->ConnectServer(bstr_t("\\\\.\\root\\CIMV2"), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &_svc);
	if (FAILED(hr))
	{
		_loc->Release();
		CoUninitialize();
		return false;
	}

	hr = CoSetProxyBlanket(_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hr))
	{
		_svc->Release();
		_loc->Release();
		CoUninitialize();
		return false;
	}

	hr = CoCreateInstance(__uuidof(UnsecuredApartment), NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&_unsec_app));
	assert(SUCCEEDED(hr));

	_sink = new sink_inject;
	_sink->AddRef();

	_stub_unk = NULL;
	hr = _unsec_app->CreateObjectStub(_sink, &_stub_unk);
	assert(SUCCEEDED(hr));

	_stub_sink = NULL;
	hr = _stub_unk->QueryInterface(IID_IWbemObjectSink, (void**) &_stub_sink);
	assert(SUCCEEDED(hr));

	const WCHAR *interval_str = setting_instance.get_service_setting("poll_interval");
	if (interval_str == NULL)
		interval_str = L"1";

	const int query_str_len = 100;
	WCHAR query_str[query_str_len];
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
		_stub_sink);
	if (FAILED(hr))
	{
		release_all();
		CoUninitialize();
		return false;
	}

	return true;
}

void svc_mon::stop_monitor()
{
	HRESULT hr;

	hr = _svc->CancelAsyncCall(_stub_sink);
	assert(SUCCEEDED(hr));

	release_all();
	CoUninitialize();
}