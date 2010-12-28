#pragma once

#include "gdipp_lib.h"

using namespace std;

extern vector<HMODULE> h_minidump_modules;

BOOL WINAPI minidump_callback(IN PVOID CallbackParam,
	IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
	IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

LONG WINAPI minidump_filter(EXCEPTION_POINTERS *ExceptionInfo);

GDIPP_API void gdipp_init_minidump();
GDIPP_API void gdipp_register_minidump_module(HMODULE h_module);