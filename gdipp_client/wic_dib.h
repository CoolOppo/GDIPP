#pragma once

namespace gdipp
{

class gdimm_wic_dib_lock : public IWICBitmapLock
{
public:
	IFACEMETHOD(QueryInterface)(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	IFACEMETHOD_(ULONG, AddRef)( void);

	IFACEMETHOD_(ULONG, Release)( void);

public:
	IFACEMETHOD(GetSize)(
		/* [out] */ __RPC__out UINT *puiWidth,
		/* [out] */ __RPC__out UINT *puiHeight);

	IFACEMETHOD(GetStride)(
		/* [out] */ __RPC__out UINT *pcbStride);

	/* [local] */ IFACEMETHOD(GetDataPointer)(
		/* [out] */ UINT *pcbBufferSize,
		/* [out] */ BYTE **ppbData);

	IFACEMETHOD(GetPixelFormat)(
		/* [out] */ __RPC__out WICPixelFormatGUID *pPixelFormat);

public:
	void initialize(const BITMAPINFO *bmp_info, VOID *bits);
	void set_rect(const WICRect *lock_rect);

private:
	const WICRect *_lock_rect;
	const BITMAPINFO *_bmp_info;
	VOID *_bits;
};

class gdimm_wic_dib : public IWICBitmap
{
public:
	IFACEMETHOD(QueryInterface)(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	IFACEMETHOD_(ULONG, AddRef)( void);

	IFACEMETHOD_(ULONG, Release)( void);

public:
	IFACEMETHOD(GetSize)(
		/* [out] */ __RPC__out UINT *puiWidth,
		/* [out] */ __RPC__out UINT *puiHeight);

	IFACEMETHOD(GetPixelFormat)(
		/* [out] */ __RPC__out WICPixelFormatGUID *pPixelFormat);

	IFACEMETHOD(GetResolution)(
		/* [out] */ __RPC__out double *pDpiX,
		/* [out] */ __RPC__out double *pDpiY);

	IFACEMETHOD(CopyPalette)(
		/* [in] */ __RPC__in_opt IWICPalette *pIPalette);

	IFACEMETHOD(CopyPixels)(
		/* [unique][in] */ __RPC__in_opt const WICRect *prc,
		/* [in] */ UINT cbStride,
		/* [in] */ UINT cbBufferSize,
		/* [size_is][out] */ __RPC__out_ecount_full(cbBufferSize) BYTE *pbBuffer);

public:
	IFACEMETHOD(Lock)(
		/* [in] */ __RPC__in const WICRect *prcLock,
		/* [in] */ DWORD flags,
		/* [out] */ __RPC__deref_out_opt IWICBitmapLock **ppILock);

	IFACEMETHOD(SetPalette)(
		/* [in] */ __RPC__in_opt IWICPalette *pIPalette);

	IFACEMETHOD(SetResolution)(
		/* [in] */ double dpiX,
		/* [in] */ double dpiY);

public:
	void initialize(const BITMAPINFO *bmp_info, VOID *bits);

private:
	const BITMAPINFO *_bmp_info;
	gdimm_wic_dib_lock _lock;
};

}
