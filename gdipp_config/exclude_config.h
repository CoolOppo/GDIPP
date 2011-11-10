#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API exclude_config
{
public:
	static bool is_process_excluded(const config &cfg, const wchar_t *proc_name);
};

}
