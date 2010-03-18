#pragma once

class _gdimm_font_link
{
	typedef map<const wstring, vector<wstring>, wstring_ci_less> link_map;

	link_map _link_table;

	void get_font_link_info();

public:
	_gdimm_font_link()
	{ get_font_link_info(); }

	const WCHAR *lookup(const WCHAR *font_name, size_t index) const;
	size_t get_link_count(const WCHAR *font_name) const;
};

typedef singleton<_gdimm_font_link> gdimm_font_link;