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

	if (lpCmdLine == NULL || wcslen(lpCmdLine) == 0)
	{
		MessageBox(NULL, TEXT("Drag an exe file to me and I will load it with gdimm.dll."), TEXT("gdipp_loader"), MB_OK | MB_ICONINFORMATION);
		return EXIT_SUCCESS;
	}

	int argc;
	LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
	assert(argv != NULL);

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
	if (eh_error != 0)
		MessageBox(NULL, TEXT("Unable to create the target process."), TEXT("gdipp_loader"), MB_OK | MB_ICONERROR);

	LocalFree(argv);

	return EXIT_SUCCESS;
}
