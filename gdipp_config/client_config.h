#pragma once

#include "gdipp_config/config.h"
#include "gdipp_lib/type_enum.h"

namespace gdipp
{

class GDIPP_API client_config : public config
{
public:
	client_config();
	void load(const void *root);

	PAINTER_TYPE painter;
};

}
