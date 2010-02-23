#include "stdafx.h"
#include "sink.h"
#include "global.h"
#include "setting.h"
#include <easyhook.h>

DWORD WINAPI start(LPVOID lpThreadParameter)
{
	return 0;
}

sink_inject::sink_inject()
{
	_ref = 0;
	get_dir_file_path(_gdimm_path_32, L"gdimm_32.dll");
	get_dir_file_path(_gdimm_path_64, L"gdimm_64.dll");
}

bool sink_inject::inject(LONG proc_id)
{
	NTSTATUS eh_error;

	const INJECTOR_TYPE injector_type = GDIPP_SERVICE;
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, _gdimm_path_32, _gdimm_path_64, (PVOID) &injector_type, sizeof(INJECTOR_TYPE));
	return (eh_error == 0);
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
		VariantInit(&var_event);
		hr = apObjArray[i]->Get(L"TargetInstance", 0, &var_event, NULL, NULL);
		assert(SUCCEEDED(hr));

		IWbemClassObject *proc_obj;
		hr = V_UNKNOWN(&var_event)->QueryInterface(IID_IWbemClassObject, (void**) &proc_obj);
		assert(SUCCEEDED(hr));
		VariantClear(&var_event);

		VARIANT var_exe_name;
		VariantInit(&var_exe_name);
		hr = proc_obj->Get(L"Name", 0, &var_exe_name, NULL, NULL);
		assert(SUCCEEDED(hr));

		if (gdimm_setting::instance().is_name_excluded(V_BSTR(&var_exe_name)))
		{
			VariantClear(&var_exe_name);
			proc_obj->Release();
			continue;
		}

		VARIANT var_proc_id;
		VariantInit(&var_proc_id);
		hr = proc_obj->Get(L"ProcessId", 0, &var_proc_id, NULL, NULL);
		assert(SUCCEEDED(hr));

		bool a = inject(V_I4(&var_proc_id));

		VariantClear(&var_proc_id);
		proc_obj->Release();
	}

	return WBEM_S_NO_ERROR;
}