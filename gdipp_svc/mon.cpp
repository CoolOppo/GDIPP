#include "stdafx.h"
#include "mon.h"
#include "sink.h"
#include <objbase.h>
#include <wbemidl.h>
#include <comutil.h>

void _svc_mon::start_monitor()
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	assert(SUCCEEDED(hr));

	hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*) &_loc);
	assert(SUCCEEDED(hr));

	hr = _loc->ConnectServer(_bstr_t(TEXT("ROOT\\CIMV2")), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &_svc);
	assert(SUCCEEDED(hr));

	hr = CoSetProxyBlanket(_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	assert(SUCCEEDED(hr));

	hr = CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**) &_unsec_app);
	assert(SUCCEEDED(hr));

	_sink = new sink_inject;
	_sink->AddRef();

	_stub_unk = NULL;
	hr = _unsec_app->CreateObjectStub(_sink, &_stub_unk);
	assert(SUCCEEDED(hr));

	_stub_sink = NULL;
	hr = _stub_unk->QueryInterface(IID_IWbemObjectSink, (void**) &_stub_sink);
	assert(SUCCEEDED(hr));

	hr = _svc->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'"), WBEM_FLAG_SEND_STATUS, NULL, _stub_sink);
	assert(SUCCEEDED(hr));
}

void _svc_mon::stop_monitor()
{
	HRESULT hr;

	hr = _svc->CancelAsyncCall(_stub_sink);
	assert(SUCCEEDED(hr));

	_stub_sink->Release();
	_sink->Release();
	_stub_unk->Release();
	_unsec_app->Release();
	_loc->Release();
	_svc->Release();

	CoUninitialize();
}