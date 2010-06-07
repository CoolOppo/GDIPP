#include "stdafx.h"
#include "sink.h"
#include "gdipp_svc.h"
#include <gdipp_common.h>

inject_sink::inject_sink()
:
_ref_count(0)
{
}

IFACEMETHODIMP inject_sink::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject
	)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppvObject = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else
		return E_NOINTERFACE;
}

IFACEMETHODIMP_(ULONG) inject_sink::AddRef()
{
	return InterlockedIncrement(&_ref_count);
}

IFACEMETHODIMP_(ULONG) inject_sink::Release()
{
	long new_count = InterlockedDecrement(&_ref_count);

	if (new_count == 0)
		delete this;

	return new_count;
}

unsigned __stdcall process_obj(void *arglist)
{
	HRESULT hr;

	IWbemClassObject *obj = (IWbemClassObject *)arglist;

	CComVariant var_event;
	VariantInit(&var_event);
	hr = obj->Get(L"TargetInstance", 0, &var_event, NULL, NULL);
	assert(hr == S_OK);

	CComPtr<IWbemClassObject> proc_obj;
	hr = V_UNKNOWN(&var_event)->QueryInterface(IID_IWbemClassObject, (void **)&proc_obj);
	assert(hr == S_OK);

	CComVariant var_exe_name;
	VariantInit(&var_exe_name);
	hr = proc_obj->Get(L"Name", 0, &var_exe_name, NULL, NULL);
	assert(hr == S_OK);

	if (gdipp_is_process_excluded(V_BSTR(&var_exe_name)))
		return 1;

	CComVariant var_proc_id;
	VariantInit(&var_proc_id);
	hr = proc_obj->Get(L"ProcessId", 0, &var_proc_id, NULL, NULL);
	assert(hr == S_OK);

	return gdipp_inject_process(V_I4(&var_proc_id));
}

IFACEMETHODIMP inject_sink::Indicate(
	/* [in] */ long lObjectCount,
	/* [size_is][in] */ __RPC__in_ecount_full(lObjectCount) IWbemClassObject **apObjArray)
{
	BOOL b_ret;
	DWORD dw_ret;
	vector<HANDLE> h_threads;

	// inject process in separate thread
	for (LONG i = 0; i < lObjectCount; i++)
	{
		HANDLE h_thread = (HANDLE) _beginthreadex(NULL, 0, process_obj, apObjArray[i], 0, 0);
		assert(h_thread != NULL);
		h_threads.push_back(h_thread);
	}

	// wait until all threads end, or time-out is reached
	dw_ret = WaitForMultipleObjects(lObjectCount, &h_threads[0], TRUE, 3000);
	assert(dw_ret != WAIT_FAILED);

	for (vector<HANDLE>::const_iterator iter = h_threads.begin(); iter != h_threads.end(); iter++)
	{
		if (dw_ret == WAIT_TIMEOUT)
		{
			b_ret = TerminateThread(*iter, 0);
			assert(b_ret);
		}

		CloseHandle(*iter);
	}

	return WBEM_S_NO_ERROR;
}

IFACEMETHODIMP inject_sink::SetStatus(
	/* [in] */ long lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ __RPC__in BSTR strParam,
	/* [in] */ __RPC__in_opt IWbemClassObject *pObjParam)
{
	return WBEM_S_NO_ERROR;
}