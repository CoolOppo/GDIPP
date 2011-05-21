#include "stdafx.h"
#include "wic_dib.h"
#include "helper_func.h"

IFACEMETHODIMP gdimm_wic_dib_lock::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) gdimm_wic_dib_lock::AddRef( void)
{
	return 0;
}

IFACEMETHODIMP_(ULONG) gdimm_wic_dib_lock::Release( void)
{
	return 0;
}

IFACEMETHODIMP gdimm_wic_dib_lock::GetSize(
	/* [out] */ __RPC__out UINT *puiWidth,
	/* [out] */ __RPC__out UINT *puiHeight)
{
	*puiWidth = _lock_rect->Width;
	*puiHeight = _lock_rect->Height;

	return S_OK;
}

IFACEMETHODIMP gdimm_wic_dib_lock::GetStride(
	/* [out] */ __RPC__out UINT *pcbStride)
{
	*pcbStride = get_bmp_pitch(_lock_rect->Width, _bmp_info->bmiHeader.biBitCount);

	return S_OK;
}

/* [local] */ IFACEMETHODIMP gdimm_wic_dib_lock::GetDataPointer(
	/* [out] */ UINT *pcbBufferSize,
	/* [out] */ BYTE **ppbData)
{
	*pcbBufferSize = get_bmp_pitch(_lock_rect->Width, _bmp_info->bmiHeader.biBitCount) * _lock_rect->Height;
	*ppbData = (BYTE *)_bits;

	return S_OK;
}

IFACEMETHODIMP gdimm_wic_dib_lock::GetPixelFormat(
	/* [out] */ __RPC__out WICPixelFormatGUID *pPixelFormat)
{
	*pPixelFormat = GUID_WICPixelFormat32bppBGR;

	return S_OK;
}

void gdimm_wic_dib_lock::initialize(const BITMAPINFO *bmp_info, VOID *bits)
{
	_bmp_info = bmp_info;
	_bits = bits;
}

void gdimm_wic_dib_lock::set_rect(const WICRect *lock_rect)
{
	_lock_rect = lock_rect;
}

IFACEMETHODIMP gdimm_wic_dib::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) gdimm_wic_dib::AddRef( void)
{
	return 0;
}

IFACEMETHODIMP_(ULONG) gdimm_wic_dib::Release( void)
{
	return 0;
}

IFACEMETHODIMP gdimm_wic_dib::GetSize(
	/* [out] */ __RPC__out UINT *puiWidth,
	/* [out] */ __RPC__out UINT *puiHeight)
{
	*puiWidth = _bmp_info->bmiHeader.biWidth;
	*puiHeight = abs(_bmp_info->bmiHeader.biHeight);

	return S_OK;
}

IFACEMETHODIMP gdimm_wic_dib::GetPixelFormat(
	/* [out] */ __RPC__out WICPixelFormatGUID *pPixelFormat)
{
	switch (_bmp_info->bmiHeader.biBitCount)
	{
	case 1:
		*pPixelFormat = GUID_WICPixelFormatBlackWhite;
	case 8:
		*pPixelFormat = GUID_WICPixelFormat8bppGray;
	case 24:
	case 32:
		*pPixelFormat = GUID_WICPixelFormat32bppBGR;
		break;
	}

	return S_OK;
}

IFACEMETHODIMP gdimm_wic_dib::GetResolution(
	/* [out] */ __RPC__out double *pDpiX,
	/* [out] */ __RPC__out double *pDpiY)
{
	return E_NOTIMPL;
}

IFACEMETHODIMP gdimm_wic_dib::CopyPalette(
	/* [in] */ __RPC__in_opt IWICPalette *pIPalette)
{
	// we do not use palette
	return E_NOTIMPL;
}

IFACEMETHODIMP gdimm_wic_dib::CopyPixels(
	/* [unique][in] */ __RPC__in_opt const WICRect *prc,
	/* [in] */ UINT cbStride,
	/* [in] */ UINT cbBufferSize,
	/* [size_is][out] */ __RPC__out_ecount_full(cbBufferSize) BYTE *pbBuffer)
{
	// use lock instead
	return E_NOTIMPL;
}

IFACEMETHODIMP gdimm_wic_dib::Lock(
	/* [in] */ __RPC__in const WICRect *prcLock,
	/* [in] */ DWORD flags,
	/* [out] */ __RPC__deref_out_opt IWICBitmapLock **ppILock)
{
	_lock.set_rect(prcLock);
	*ppILock = &_lock;

	return S_OK;
}

IFACEMETHODIMP gdimm_wic_dib::SetPalette(
	/* [in] */ __RPC__in_opt IWICPalette *pIPalette)
{
	// we do not use palette
	return E_NOTIMPL;
}

IFACEMETHODIMP gdimm_wic_dib::SetResolution(
	/* [in] */ double dpiX,
	/* [in] */ double dpiY)
{
	return E_NOTIMPL;
}

void gdimm_wic_dib::initialize(const BITMAPINFO *bmp_info, VOID *bits)
{
	_bmp_info = bmp_info;
	_lock.initialize(bmp_info, bits);
}
