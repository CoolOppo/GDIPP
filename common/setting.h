#pragma once

#include "global.h"
#include <objbase.h>
#include <msxml2.h>
#include <list>
#include <map>
#include <set>
#include <sstream>
using namespace std;

enum LOAD_BRANCH
{
	GDIMM_BRANCH = 1,
	SERVICE_BRANCH = 2,
	ALL_BRANCHS = 3
};

struct ci_less
{
	bool operator()(const t_string &string1, const t_string &string2) const
	{ return lstrcmpi(string1.c_str(), string2.c_str()) < 0; }
};

class _gdimm_setting
{
	// map keys are case-insensitive
	typedef map<const t_string, t_string, ci_less> setting_map;
	typedef map<const t_string, setting_map*, ci_less> branch_map;

	list<setting_map> _setting_branchs;
	branch_map _branch_names;
	set<const t_string, ci_less> _exclude_proc;

	void set_default(LOAD_BRANCH branch_type);
	void load_branch(IXMLDOMDocument *xml_doc, TCHAR *xpath);
	void load_exclude_proc(IXMLDOMDocument *xml_doc);

public:
	void load_settings(LOAD_BRANCH branch_type, HMODULE h_module);
	bool is_proc_excluded(const TCHAR *proc_name) const
	{ return (_exclude_proc.find(proc_name) != _exclude_proc.end()); }

	template <typename T>
	T get_gdimm_setting(const TCHAR *setting_name, const TCHAR *branch_name = TEXT("common")) const;
	template <typename T>
	T get_service_setting(const TCHAR *setting_name) const;
};

template <typename T>
T _gdimm_setting::get_gdimm_setting(const TCHAR *setting_name, const TCHAR *branch_name) const
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
	basic_istringstream<TCHAR>(setting_iter->second) >> setting_value;

	return setting_value;
}

template <typename T>
inline T _gdimm_setting::get_service_setting(const TCHAR *setting_name) const
{
	branch_map::const_iterator branch_iter = _branch_names.find(TEXT("service"));
	const setting_map *target_branch = branch_iter->second;

	setting_map::const_iterator setting_iter = target_branch->find(setting_name);
	assert(setting_iter != target_branch->end());

	// convert string to type T
	T setting_value;
	basic_istringstream<TCHAR>(setting_iter->second) >> setting_value;

	return setting_value;
}

typedef singleton<_gdimm_setting> gdimm_setting;