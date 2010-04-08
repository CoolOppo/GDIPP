#include "stdafx.h"
#include "fassert.h"
#include "gdipp_common.h"
using namespace std;

GDIPP_API void __cdecl _fassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line)
{
	wostringstream assert_msg;
	assert_msg << _Message;
	assert_msg << L" at ";
	assert_msg << _File;
	assert_msg << L" line ";
	assert_msg << _Line;

	debug_output(assert_msg.str().c_str());
}