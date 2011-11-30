#pragma once

#include "gdipp_config/config_file.h"
#include "gdipp_lib/api.h"

namespace gdipp
{

class GDIPP_API config
{
public:
	config(const config_file &config_file);
	config(const void *root_node);

	const wchar_t *get_string(const wchar_t *config_path, const wchar_t *default_value) const;
	size_t get_string_list(const wchar_t *config_path, const wchar_t **list_values) const;

	template<typename T>
	T get_number(const wchar_t *config_path, T default_value) const;
	template<typename T>
	size_t get_number_list(const wchar_t *config_path, T *list_values) const;

private:
	const void *_root_node;
};

}
