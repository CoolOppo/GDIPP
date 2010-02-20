#include "stdafx.h"
#include "sink.h"
#include "inject.h"
#include "setting.h"

sink_inject::sink_inject()
{
	_ref = 0;
}

ULONG sink_inject::AddRef()
{
	return InterlockedIncrement(&_ref);
}

ULONG sink_inject::Release()
{
	LONG _new_ref = InterlockedDecrement(&_ref);
	
	if (_new_ref == 0)
		delete this;

	return _new_ref;
}

HRESULT sink_inject::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink*) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else
		return E_NOINTERFACE;
}

HRESULT sink_inject::Indicate(LONG lObjectCount, IWbemClassObject **apObjArray)
{
	HRESULT hr;

	for (LONG i = 0; i < lObjectCount; i++)
	{
		VARIANT var_event;
		BSTR a;
		apObjArray[i]->GetObjectText(0, &a);

		VariantInit(&var_event);
		hr = apObjArray[i]->Get(TEXT("TargetInstance"), 0, &var_event, NULL, NULL);
		assert(SUCCEEDED(hr));

		IWbemClassObject *proc_obj;
		hr = V_UNKNOWN(&var_event)->QueryInterface(IID_IWbemClassObject, (void**) &proc_obj);
		assert(SUCCEEDED(hr));
		VariantClear(&var_event);

		VARIANT var_exe_name;
		VariantInit(&var_exe_name);
		hr = proc_obj->Get(TEXT("Name"), 0, &var_exe_name, NULL, NULL);
		assert(SUCCEEDED(hr));

		if (gdimm_setting::instance().is_proc_excluded(V_BSTR(&var_exe_name)))
		{
			VariantClear(&var_exe_name);
			proc_obj->Release();
			continue;
		}

		VARIANT var_proc_id;
		VariantInit(&var_proc_id);
		hr = proc_obj->Get(TEXT("ProcessId"), 0, &var_proc_id, NULL, NULL);
		assert(SUCCEEDED(hr));

		svc_injector::instance().inject(V_I4(&var_proc_id));

		VariantClear(&var_proc_id);
		proc_obj->Release();
	}

	return WBEM_S_NO_ERROR;
}

HRESULT sink_inject::SetStatus(
	LONG lFlags,
	HRESULT hResult,
	BSTR strParam,
	IWbemClassObject __RPC_FAR *pObjParam)
{
	return WBEM_S_NO_ERROR;
}