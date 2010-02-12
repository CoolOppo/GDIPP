#include "stdafx.h"
#include "pe.h"
#include "dbghelp.h"

bool _gdimm_pe::lookup_import(char *module_name)
{
	DWORD dw_ret;

	// all DbgHelp functions are single threaded
	critical_section lock;

	TCHAR exe_name[MAX_PATH];
	dw_ret = GetModuleFileName(NULL, exe_name, MAX_PATH);
	assert(dw_ret != 0);

	const HMODULE exe_base = GetModuleHandle(exe_name);
	assert(exe_base != NULL);

	ULONG table_size;
	IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*) ImageDirectoryEntryToDataEx(exe_base, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &table_size, NULL);
	assert(import_desc != NULL);

	//	if the RVA is 0, all IMAGE_IMPORT_DESCRIPTORs have been enumerated
	while (import_desc->OriginalFirstThunk != 0)
	{
		//	the offest of the module's name is in IMAGE_IMPORT_DESCRIPTOR.Name	
		char *curr_name = (char*) exe_base + import_desc->Name;
		if (_stricmp(module_name, curr_name) == 0)
			return true;
		
		import_desc++;
	}

	return false;
}