// dll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "hook.h"
#include "ft.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

gdimm_hook hook_ins;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		InitializeFreeType();
		hook_ins.hook();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		hook_ins.unhook();
		DestroyFreeType();
	}

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

