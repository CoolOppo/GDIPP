// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "global.h"
#include "hook.h"
#include "ft.h"
#include "setting.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_self = hModule;

		critical_section::initialize();
		gdimm_setting::instance().load_settings(hModule);
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
