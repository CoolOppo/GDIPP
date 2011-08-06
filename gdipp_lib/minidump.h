#pragma once

#include <vector>
#include "gdipp_lib/gdipp_lib.h"

namespace gdipp
{

extern std::vector<HMODULE> h_minidump_modules;

BOOL WINAPI minidump_callback(IN PVOID CallbackParam,
	IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
	IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

LONG WINAPI minidump_filter(EXCEPTION_POINTERS *ExceptionInfo);

GDIPP_API void init_minidump();
GDIPP_API void register_minidump_module(HMODULE h_module);

}
