#pragma once

#include "gdipp_lib/api.h"
#include "gdipp_config/config_file.h"

namespace gdipp
{

class GDIPP_API config
{
public:
	virtual void load(const config_file &file) = 0;

private:
	virtual void parse(const void *root) = 0;
};

}
