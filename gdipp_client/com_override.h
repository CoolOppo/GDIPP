#pragma once

namespace gdipp
{

HMODULE WINAPI LoadLibraryA_hook(LPCSTR lpLibFileName);
HMODULE WINAPI LoadLibraryW_hook(LPCWSTR lpLibFileName);
HMODULE WINAPI LoadLibraryExA_hook(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI LoadLibraryExW_hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

HRESULT WINAPI D2D1CreateFactory_hook(D2D1_FACTORY_TYPE factoryType, REFIID riid, CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, void **ppIFactory);

}
