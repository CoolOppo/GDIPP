#pragma once

#include <crtdefs.h>

#undef  assert

#ifdef  NDEBUG

#define assert(_Expression)     ((void)0)

#else

void __cdecl _fassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);

#define assert(_Expression) (void)( (!!(_Expression)) || (_fassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )

#endif  /* NDEBUG */
