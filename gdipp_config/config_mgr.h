#pragma once

#include "gdipp_config/freetype_config.h"
#include "gdipp_config/hook_config.h"
#include "gdipp_config/font_render_config_cache.h"
#include "gdipp_lib/api.h"

namespace gdipp
{

class GDIPP_API config_mgr
{
public:
	config_mgr(const wchar_t *config_filename);
	~config_mgr();

	freetype_config get_freetype_config();
	hook_config get_hook_config();
	render_config get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name);

private:
	void *_config_xml;
	font_render_config_cache *_cache;
	freetype_config *_ft_conf;
	hook_config *_hook_conf;
};

}
