#pragma once

#include "gdipp_support/freetype_config.h"
#include "gdipp_support/hook_config.h"

namespace gdipp
{

class config_mgr
{
public:
	config_mgr();

	freetype_config ft_conf;
	hook_config hook_conf;
};

}
