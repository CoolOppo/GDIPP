// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ft.h"
#include "hook.h"

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gdipp_setting::instance().init(hModule);
		if (gdipp_setting::instance().is_process_excluded(NULL))
			return FALSE;

		h_self = hModule;

		critical_section::initialize();
		initialize_freetype();
		return gdimm_hook::instance().hook();

	case DLL_PROCESS_DETACH:
		gdimm_hook::instance().unhook();
		destroy_freetype();
		critical_section::release();

		break;
	}

	return TRUE;
}
