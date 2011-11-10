#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API render_config_delta
{
	friend class render_config_delta_cache;

public:
	void load(const config_file &file);

	std::wstring config_delta;

private:
	void parse(const void *root);
};

}
