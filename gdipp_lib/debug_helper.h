#pragma once

void gdipp_debug_buffer(const void *ptr, unsigned int size);
void gdipp_debug_decimal(double num, bool new_line = true);
void gdipp_debug_integer(size_t num, bool new_line = true);
void gdipp_debug_string(const wchar_t *str = L"");
