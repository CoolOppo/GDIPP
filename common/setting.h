#pragma once

#include "global.h"
#include <map>
#include <sstream>
using namespace std;

class _gdimm_setting
{
	map<const t_string, t_string> _settings;

public:
	_gdimm_setting();
	void load_settings(HMODULE hModule);

	template <typename T>
	T get_value(const TCHAR *name) const;
};


template <typename T>
T _gdimm_setting::get_value(const TCHAR *name) const
{
	map<const t_string, t_string>::const_iterator iter = _settings.find(name);
	assert(iter != _settings.end());

	T setting_value;
	basic_istringstream<TCHAR>(iter->second) >> setting_value;

	return setting_value;
}

typedef singleton<_gdimm_setting> gdimm_setting;