#include "stdafx.h"
#include "com_override.h"
#include "gdipp_client/global.h"
#include "gdipp_lib/scoped_rw_lock.h"

namespace gdipp
{

HMODULE WINAPI LoadLibraryA_hook(LPCSTR lpLibFileName)
{
	const HMODULE h_new_module = LoadLibraryA(lpLibFileName);

	hook_instance.install_delayed_hook(lpLibFileName, h_new_module);

	return h_new_module;
}

HMODULE WINAPI LoadLibraryW_hook(LPCWSTR lpLibFileName)
{
	const HMODULE h_new_module = LoadLibraryW(lpLibFileName);

	hook_instance.install_delayed_hook(lpLibFileName, h_new_module);

	return h_new_module;
}

HMODULE WINAPI LoadLibraryExA_hook(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	const HMODULE h_new_module = LoadLibraryExA(lpLibFileName, hFile, dwFlags);

	hook_instance.install_delayed_hook(lpLibFileName, h_new_module);

	return h_new_module;
}

HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	const HMODULE h_new_module = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	hook_instance.install_delayed_hook(lpLibFileName, h_new_module);

	return h_new_module;
}

//////////////////////////////////////////////////////////////////////////

typedef HRESULT (STDMETHODCALLTYPE *CreateDxgiSurfaceRenderTarget_type)(
	ID2D1Factory *,
	IDXGISurface *,
	CONST D2D1_RENDER_TARGET_PROPERTIES &,
	ID2D1RenderTarget **
	);
typedef void (STDMETHODCALLTYPE *DrawGlyphRun_type)(
	ID2D1RenderTarget *,
	D2D1_POINT_2F,
	CONST DWRITE_GLYPH_RUN *,
	ID2D1Brush *,
	DWRITE_MEASURING_MODE
	);

CreateDxgiSurfaceRenderTarget_type CreateDxgiSurfaceRenderTarget_orig = NULL;
DrawGlyphRun_type DrawGlyphRun_orig = NULL;

IFACEMETHODIMP_(void) DrawGlyphRun_hook(
	ID2D1RenderTarget *renderTarget,
	D2D1_POINT_2F baselineOrigin,
	CONST DWRITE_GLYPH_RUN *glyphRun,
	ID2D1Brush *foregroundBrush,
	DWRITE_MEASURING_MODE measuringMode
	)
{

}

IFACEMETHODIMP CreateDxgiSurfaceRenderTarget_hook(
	ID2D1Factory *pIFactory,
	IDXGISurface *dxgiSurface,
	CONST D2D1_RENDER_TARGET_PROPERTIES &renderTargetProperties,
	ID2D1RenderTarget **renderTarget
	)
{
	BOOL b_ret;

	const HRESULT hr = reinterpret_cast<CreateDxgiSurfaceRenderTarget_type>(*CreateDxgiSurfaceRenderTarget_orig)(pIFactory, dxgiSurface, renderTargetProperties, renderTarget);
	if (hr != S_OK)
		return hr;

	if (DrawGlyphRun_orig == NULL)
	{
		const scoped_rw_lock lock_w(scoped_rw_lock::CLIENT_COM_HOOK, false);
		if (DrawGlyphRun_orig == NULL)
		{
			const void **vfptr = *reinterpret_cast<const void ***>(*renderTarget);
			const void **hook_addr = vfptr + 29;

			// temporarily remove protection of the memory region
			DWORD new_protect = PAGE_READWRITE, old_protect;
			b_ret = VirtualProtect(hook_addr, sizeof(void *), new_protect, &old_protect);
			if (!b_ret)
				return hr;

			DrawGlyphRun_orig = reinterpret_cast<DrawGlyphRun_type>(*hook_addr);
			*hook_addr = &DrawGlyphRun_hook;

			VirtualProtect(hook_addr, sizeof(void *), old_protect, &new_protect);
		}
	}

	return hr;
}

HRESULT WINAPI D2D1CreateFactory_hook(D2D1_FACTORY_TYPE factoryType, REFIID riid, CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, void **ppIFactory)
{
	BOOL b_ret;

	const HRESULT hr = D2D1CreateFactory(factoryType, riid, pFactoryOptions, ppIFactory);
	if (hr != S_OK)
		return hr;

	if (CreateDxgiSurfaceRenderTarget_orig == NULL)
	{
		const scoped_rw_lock lock_w(scoped_rw_lock::CLIENT_COM_HOOK, false);
		if (CreateDxgiSurfaceRenderTarget_orig == NULL)
		{
			ID2D1Factory *pIFactory = *reinterpret_cast<ID2D1Factory **>(ppIFactory);
			const void **vfptr = *reinterpret_cast<const void ***>(pIFactory);
			const void **hook_addr = vfptr + 15;

			// temporarily remove protection of the memory region
			DWORD new_protect = PAGE_READWRITE, old_protect;
			b_ret = VirtualProtect(hook_addr, sizeof(void *), new_protect, &old_protect);
			if (!b_ret)
				return hr;

			CreateDxgiSurfaceRenderTarget_orig = reinterpret_cast<CreateDxgiSurfaceRenderTarget_type>(*hook_addr);
			*hook_addr = &CreateDxgiSurfaceRenderTarget_hook;

			VirtualProtect(hook_addr, sizeof(void *), old_protect, &new_protect);
		}
	}

	return hr;
}

}
