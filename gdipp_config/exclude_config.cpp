#include "stdafx.h"
#include "exclude_config.h"

namespace gdipp
{

bool exclude_config::is_process_excluded(const config &cfg, const wchar_t *proc_name)
{
	if (proc_name == NULL)
		return false;

	const wchar_t *exclude_path = L"/gdipp/client/exclude/process/text()";
	size_t process_count = cfg.get_string_list(exclude_path, NULL);
	if (process_count == 0)
		return false;

	const wchar_t **processes = new const wchar_t *[process_count];
	process_count = cfg.get_string_list(exclude_path, processes);
	assert(process_count > 0);

	bool is_excluded = false;
	for (size_t i = 0; i < process_count; ++i)
	{
		const std::tr1::wregex proc_name_regex(processes[i],
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		if (regex_match(proc_name, proc_name_regex))
		{
			is_excluded = true;
			break;
		}
	}

	delete[] processes;

	return is_excluded;
}

}
