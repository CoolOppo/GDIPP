#pragma once

#include "gdipp_lib/api.h"

namespace gdipp
{

GDIPP_API void debug_buffer(const void *ptr, unsigned int size);
GDIPP_API void debug_decimal(double num, bool new_line = true);
GDIPP_API void debug_integer(size_t num, bool new_line = true);
GDIPP_API void debug_string(const wchar_t *str = L"", bool new_line = true);

}
