#include "stdafx.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		register_minidump_module(hModule);
		break;
	}

	return TRUE;
}

}
