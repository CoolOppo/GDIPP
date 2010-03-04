#pragma once

#include <wbemidl.h>

class sink_inject: public IWbemObjectSink
{
	LONG _ref;

	WCHAR _gdimm_path_64[MAX_PATH];
	WCHAR _gdimm_path_32[MAX_PATH];

	bool inject(LONG proc_id);

public:
	sink_inject();
	virtual ULONG STDMETHODCALLTYPE AddRef()
	{ return InterlockedIncrement(&_ref); }

	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray);
	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		LONG lFlags,
		HRESULT hResult,
		BSTR strParam,
		IWbemClassObject __RPC_FAR *pObjParam)
	{ return WBEM_S_NO_ERROR; }
};