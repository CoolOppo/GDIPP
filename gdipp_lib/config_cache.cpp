#include "stdafx.h"
#include "config_cache.h"
#include <MurmurHash3.h>
#include "gdipp_support/helper.h"
#include "gdipp_support/lock.h"
#include "gdipp_lib/gdipp_lib.h"

namespace gdipp
{

font_config_cache::font_gamma::font_gamma()
	: red(1.0),
	green(1.0),
	blue(1.0)
{
}

font_config_cache::font_render_mode::font_render_mode()
	: mono(0),
	gray(1),
	subpixel(1),
	pixel_geometry(PIXEL_GEOMETRY_RGB),
	aliased_text(0)
{
}

font_config_cache::font_shadow::font_shadow()
	: offset_x(0),
	offset_y(0),
	alpha(0)
{
}

font_config_cache::font_config_cache()
	: auto_hinting(1),
	embedded_bitmap(false),
	embolden(0),
	hinting(1),
	kerning(false),
	renderer(RENDERER_FREETYPE)
{
}

const font_config_cache *config_cache::lookup(const config_trait *trait)
{
	uint32_t setting_id;
	MurmurHash3_x86_32(trait->get_data(), trait->get_size(), 0, &setting_id);

	// if the setting for the specified font is not found
	// construct setting cache for the font and return
	std::map<unsigned int, font_config_cache>::const_iterator iter = _cache.find(setting_id);
	if (iter == _cache.end())
	{
		// double-check lock
		lock l("setting_cache");
		iter = _cache.find(setting_id);
		if (iter == _cache.end())
		{
			font_config_cache new_cache;

			wcs_convert(get_server_config(L"auto_hinting", trait), reinterpret_cast<WORD *>(&new_cache.auto_hinting));
			wcs_convert(get_server_config(L"embedded_bitmap", trait), &new_cache.embedded_bitmap);
			wcs_convert(get_server_config(L"embolden", trait), &new_cache.embolden);

			wcs_convert(get_server_config(L"gamma/red", trait), &new_cache.gamma.red);
			wcs_convert(get_server_config(L"gamma/green", trait), &new_cache.gamma.green);
			wcs_convert(get_server_config(L"gamma/blue", trait), &new_cache.gamma.blue);

			wcs_convert(get_server_config(L"hinting", trait), reinterpret_cast<WORD *>(&new_cache.hinting));
			wcs_convert(get_server_config(L"kerning", trait), &new_cache.kerning);

			wcs_convert(get_server_config(L"renderer", trait), reinterpret_cast<WORD *>(&new_cache.renderer));
			wcs_convert(get_server_config(L"render_mode/mono", trait), reinterpret_cast<WORD *>(&new_cache.render_mode.mono));
			wcs_convert(get_server_config(L"render_mode/gray", trait), reinterpret_cast<WORD *>(&new_cache.render_mode.gray));
			wcs_convert(get_server_config(L"render_mode/subpixel", trait), reinterpret_cast<WORD *>(&new_cache.render_mode.subpixel));
			wcs_convert(get_server_config(L"render_mode/pixel_geometry", trait), reinterpret_cast<WORD *>(&new_cache.render_mode.pixel_geometry));
			wcs_convert(get_server_config(L"render_mode/aliased_text", trait), reinterpret_cast<WORD *>(&new_cache.render_mode.aliased_text));

			wcs_convert(get_server_config(L"shadow/offset_x", trait), &new_cache.shadow.offset_x);
			wcs_convert(get_server_config(L"shadow/offset_x", trait), &new_cache.shadow.offset_y);
			wcs_convert(get_server_config(L"shadow/alpha", trait), reinterpret_cast<WORD *>(&new_cache.shadow.alpha));

			_cache[setting_id] = new_cache;
		}
	}

	return &_cache[setting_id];
}

}
