#pragma once

#include "global.h"
#include <pugixml.hpp>
#include <list>
#include <map>
#include <set>
#include <sstream>
using namespace std;

struct ci_less
{
	bool operator()(const wstring &string1, const wstring &string2) const
	{ return _wcsicmp(string1.c_str(), string2.c_str()) < 0; }
};

class _gdimm_setting
{
	// map keys are case-insensitive
	typedef map<const wstring, wstring, ci_less> setting_map;
	typedef map<const wstring, setting_map*, ci_less> branch_map;

	list<setting_map> _setting_branchs;
	branch_map _branch_names;
	set<const wstring, ci_less> _exclude_names;

	void set_default();
	void load_branch(const pugi::xml_document &xml_doc, const char *xpath);
	void load_exclude(const pugi::xml_document &xml_doc);

public:
	_gdimm_setting();
	bool load_settings(HMODULE h_module);
	bool is_name_excluded(const WCHAR *name) const
	{ return (_exclude_names.find(name) != _exclude_names.end()); }

	template <typename T>
	T get_gdimm_setting(const WCHAR *setting_name, const WCHAR *branch_name = L"common") const;
};

template <typename T>
T _gdimm_setting::get_gdimm_setting(const WCHAR *setting_name, const WCHAR *branch_name) const
{
	branch_map::const_iterator branch_iter = _branch_names.find(branch_name);
	const setting_map *common_branch = &_setting_branchs.front();
	const setting_map *target_branch;

	// if not exist the specified branch, use the common branch
	if (branch_iter == _branch_names.end())
		target_branch = common_branch;
	else
		target_branch = branch_iter->second;

	setting_map::const_iterator setting_iter = target_branch->find(setting_name);
	
	// if not exist the specified setting, use the counterpart in the commomn branch
	if (setting_iter == target_branch->end())
	{
		target_branch = common_branch;
		setting_iter = target_branch->find(setting_name);
	}
	assert(setting_iter != target_branch->end());

	// convert string to type T
	T setting_value;
	wistringstream(setting_iter->second) >> setting_value;

	return setting_value;
}

typedef singleton<_gdimm_setting> gdimm_setting;