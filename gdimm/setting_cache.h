#pragma once

using namespace std;

class gdimm_setting_cache
{
	typedef map<string, void*> setting_cache;
	typedef map<wstring, setting_cache> font_cache;

	font_cache _cache;

public:
	~gdimm_setting_cache();

	template <typename T>
	bool lookup(const char *setting_name, const WCHAR *font_name, T &setting_value);
};

template <typename T>
bool gdimm_setting_cache::lookup(const char *setting_name, const WCHAR *font_name, T &setting_value)
{
	// if the setting for the specified font is found in the cache, use it
	font_cache::const_iterator font_iter = _cache.find(font_name);
	if (font_iter != _cache.end())
	{
		setting_cache &target_settings = _cache[font_name];

		setting_cache::const_iterator set_iter = target_settings.find(setting_name);
		if (set_iter != target_settings.end())	
		{
			setting_value = *(T*) target_settings[setting_name];
			return true;
		}
	}

	// otherwise, use the value from the setting file
	const WCHAR *raw_value = get_gdimm_setting(setting_name, font_name);
	if (raw_value == NULL)
		return false;

	T *new_value = new T;
	wcs_convert(raw_value, setting_value);
	*new_value = setting_value;

	// store the value in cache
	_cache[font_name][setting_name] = new_value;

	return true;
}