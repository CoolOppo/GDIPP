#pragma once

#include "global.h"

class _gdimm_pe
{

public:
	static bool lookup_import(char *module_name);
};

typedef singleton<_gdimm_pe> gdimm_pe;