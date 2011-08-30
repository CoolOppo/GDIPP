#pragma once

#include "gdipp_lib/api.h"

namespace gdipp
{

class GDIPP_API config_file
{
public:
	config_file(const wchar_t *filename);
	~config_file();

	const void *get_config_xml() const;
	bool empty() const;

private:
	void *_config_xml;
};

}
