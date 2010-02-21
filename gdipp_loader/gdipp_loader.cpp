#include "stdafx.h"
#include "global.h"
#include <easyhook.h>
#include <shellapi.h>

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	NTSTATUS eh_error;

	int argc;
	LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
	if (argc == 0)
	{
		MessageBox(NULL, TEXT("Drag an exe file to me and I will load it with gdimm.dll."), TEXT("gdipp_loader"), MB_OK | MB_ICONINFORMATION);
		return EXIT_SUCCESS;
	}

	// concatenate parameters with space
	for (int i = 2; i < argc; i++)
		*(argv[i] - 1) = L' ';

	WCHAR gdimm_path_32[MAX_PATH];
	WCHAR gdimm_path_64[MAX_PATH];
	get_dir_file_path(gdimm_path_32, L"gdimm_32.dll");
	get_dir_file_path(gdimm_path_64, L"gdimm_64.dll");

	const INJECTOR_TYPE injector_type = GDIPP_LOADER;
	ULONG proc_id;
	eh_error = RhCreateAndInject(argv[0], argv[1], 0, EASYHOOK_INJECT_DEFAULT, gdimm_path_32, gdimm_path_64, (PVOID) &injector_type, sizeof(INJECTOR_TYPE), &proc_id);
	assert(eh_error == 0);

	LocalFree(argv);

	return EXIT_SUCCESS;
}
