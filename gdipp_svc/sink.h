#pragma once

class inject_sink: public IWbemObjectSink
{
	LONG _ref_count;

public:
	inject_sink();

public:
	IFACEMETHOD(QueryInterface)(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
	IFACEMETHOD_(ULONG, AddRef)();
	IFACEMETHOD_(ULONG, Release)();

public:
	IFACEMETHOD(Indicate)(
		/* [in] */ long lObjectCount,
		/* [size_is][in] */ __RPC__in_ecount_full(lObjectCount) IWbemClassObject **apObjArray);
	IFACEMETHOD(SetStatus)(
		/* [in] */ long lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ __RPC__in BSTR strParam,
		/* [in] */ __RPC__in_opt IWbemClassObject *pObjParam);
};