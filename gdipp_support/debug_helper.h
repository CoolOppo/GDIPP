#pragma once

namespace gdipp
{

void debug_buffer(const void *ptr, unsigned int size);
void debug_decimal(double num, bool new_line = true);
void debug_integer(size_t num, bool new_line = true);
void debug_string(const wchar_t *str = L"");

}
