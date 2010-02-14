#pragma once

#include <wbemidl.h>

class sink_inject: public IWbemObjectSink
{
	LONG _ref;

public:
	sink_inject();
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray);
	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		LONG lFlags,
		HRESULT hResult,
		BSTR strParam,
		IWbemClassObject __RPC_FAR *pObjParam);
};