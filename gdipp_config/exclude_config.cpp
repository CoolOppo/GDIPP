#include "stdafx.h"
#include "exclude_config.h"

namespace gdipp
{

bool exclude_config::is_process_excluded(const config &cfg, const wchar_t *proc_name)
{
	bool b_ret;

	if (proc_name == NULL)
		return false;

	const wchar_t *exclude_path = L"gdipp/exclude/process";
	std::vector<const wchar_t *> processes;
	b_ret = cfg.get_string_list(exclude_path, processes);
	assert(b_ret);

	for (std::vector<const wchar_t *>::const_iterator iter = processes.begin(); iter != processes.end(); ++iter)
	{
		const std::tr1::wregex proc_name_regex(*iter,
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		if (regex_match(proc_name, proc_name_regex))
			return true;
	}

	return false;
}

}
