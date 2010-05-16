#include "stdafx.h"
#include "setting_cache.h"
#include "gdimm.h"
#include "lock.h"

font_setting_cache::font_gamma::font_gamma()
:
gray(1.0),
red(1.0),
green(1.0),
blue(1.0)
{
}
font_setting_cache::font_shadow::font_shadow()
:
offset_x(0),
offset_y(0),
alpha(8)
{
}
font_setting_cache::font_setting_cache()
:
auto_hinting(false),
embedded_bitmap(false),
embolden(0),
hinting(true),
light_mode(true),
max_height(72),
render_mono(false),
render_non_aa(false),
renderer(FREETYPE),
subpixel_render(true),
zero_alpha(false)
{
}

const font_setting_cache *gdimm_setting_cache::lookup(const wchar_t *font_name)
{
	// if the setting for the specified font is not found
	// construct setting cache for the font and return
	cache_map::const_iterator iter = _cache.find(font_name);
	if (iter == _cache.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_SETTING_CACHE);

		iter = _cache.find(font_name);
		if (iter == _cache.end())
		{
			font_setting_cache new_cache;

			wcs_convert(gdipp_get_gdimm_setting(L"auto_hinting", font_name), &new_cache.auto_hinting);
			wcs_convert(gdipp_get_gdimm_setting(L"embedded_bitmap", font_name), &new_cache.embedded_bitmap);
			wcs_convert(gdipp_get_gdimm_setting(L"embolden", font_name), &new_cache.embolden);

			wcs_convert(gdipp_get_gdimm_setting(L"gamma/gray", font_name), &new_cache.gamma.gray);
			wcs_convert(gdipp_get_gdimm_setting(L"gamma/red", font_name), &new_cache.gamma.red);
			wcs_convert(gdipp_get_gdimm_setting(L"gamma/green", font_name), &new_cache.gamma.green);
			wcs_convert(gdipp_get_gdimm_setting(L"gamma/blue", font_name), &new_cache.gamma.blue);

			wcs_convert(gdipp_get_gdimm_setting(L"hinting", font_name), &new_cache.hinting);
			wcs_convert(gdipp_get_gdimm_setting(L"light_mode", font_name), &new_cache.light_mode);
			wcs_convert(gdipp_get_gdimm_setting(L"max_height", font_name), &new_cache.max_height);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mono", font_name), &new_cache.render_mono);
			wcs_convert(gdipp_get_gdimm_setting(L"render_non_aa", font_name), &new_cache.render_non_aa);
			wcs_convert(gdipp_get_gdimm_setting(L"renderer", font_name), (int*) &new_cache.renderer);

			wcs_convert(gdipp_get_gdimm_setting(L"shadow/offset_x", font_name), &new_cache.shadow.offset_x);
			wcs_convert(gdipp_get_gdimm_setting(L"shadow/offset_x", font_name), &new_cache.shadow.offset_y);
			wcs_convert(gdipp_get_gdimm_setting(L"shadow/alpha", font_name), (int*) &new_cache.shadow.alpha);

			wcs_convert(gdipp_get_gdimm_setting(L"subpixel_render", font_name), &new_cache.subpixel_render);
			wcs_convert(gdipp_get_gdimm_setting(L"zero_alpha", font_name), &new_cache.zero_alpha);

			_cache[font_name] = new_cache;
		}
	}

	return &_cache[font_name];
}