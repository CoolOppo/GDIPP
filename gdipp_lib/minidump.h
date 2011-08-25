#pragma once

namespace gdipp
{

extern std::vector<HMODULE> h_minidump_modules;

BOOL WINAPI minidump_callback(IN PVOID CallbackParam,
	IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
	IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

LONG WINAPI minidump_filter(EXCEPTION_POINTERS *ExceptionInfo);

}
