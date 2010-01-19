// dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "hook.h"
#include "ft.h"
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		InitializeFreeType();
		gdimm_hook::instance().hook();
		break;

	case DLL_PROCESS_DETACH:
		gdimm_hook::instance().unhook();
		DestroyFreeType();
		break;
	}
	return TRUE;
}
