#pragma once

class sink_inject: public IWbemObjectSink
{
	LONG _ref;

public:
	sink_inject()
	{ _ref = 0; }
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