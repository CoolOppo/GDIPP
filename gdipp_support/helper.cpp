#include "helper.h"

namespace gdipp
{

bool wstring_ci_less::operator()(const std::wstring &string1, const std::wstring &string2) const
{
	return (_wcsicmp(string1.c_str(), string2.c_str()) < 0);
}

}
