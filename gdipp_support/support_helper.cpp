#include "support_helper.h"

bool wstring_ci_less::operator()(const wstring &string1, const wstring &string2) const
{
	return (_wcsicmp(string1.c_str(), string2.c_str()) < 0);
}