#include "stdafx.h"
#include "setting_cache.h"
#include "gdimm.h"
#include "lock.h"

font_setting_cache::font_gamma::font_gamma()
:
red(1.0),
green(1.0),
blue(1.0)
{
}

font_setting_cache::font_render_mode::font_render_mode()
:
mono(0),
gray(1),
subpixel(1),
pixel_geometry(PIXEL_GEOMETRY_RGB),
aliased_text(0)
{
};

font_setting_cache::font_shadow::font_shadow()
:
offset_x(0),
offset_y(0),
alpha(0)
{
}

font_setting_cache::font_setting_cache()
:
auto_hinting(1),
embedded_bitmap(false),
embolden(0),
hinting(1),
kerning(false),
max_height(72),
renderer(RENDERER_FREETYPE)
{
}

gdimm_setting_cache::cache_trait::cache_trait(const gdimm_font_trait &font_trait)
{
	font_name = font_trait.font_name;
	weight_class = font_trait.weight_class;
	italic = font_trait.italic;
}

bool gdimm_setting_cache::cache_trait::operator<(const cache_trait &trait) const
{
	if (font_name != trait.font_name)
		return (font_name < trait.font_name);

	if (weight_class != trait.weight_class)
		return (weight_class < trait.weight_class);

	return (italic < trait.italic);
}

const font_setting_cache *gdimm_setting_cache::lookup(const gdimm_font_trait &font_trait)
{
	const cache_trait key(font_trait);

	// if the setting for the specified font is not found
	// construct setting cache for the font and return
	map<cache_trait, font_setting_cache>::const_iterator iter = _cache.find(key);
	if (iter == _cache.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_SETTING_CACHE);

		iter = _cache.find(key);
		if (iter == _cache.end())
		{
			font_setting_cache new_cache;

			wcs_convert(gdipp_get_gdimm_setting(L"auto_hinting", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.auto_hinting);
			wcs_convert(gdipp_get_gdimm_setting(L"embedded_bitmap", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.embedded_bitmap);
			wcs_convert(gdipp_get_gdimm_setting(L"embolden", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.embolden);

			wcs_convert(gdipp_get_gdimm_setting(L"gamma/red", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.gamma.red);
			wcs_convert(gdipp_get_gdimm_setting(L"gamma/green", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.gamma.green);
			wcs_convert(gdipp_get_gdimm_setting(L"gamma/blue", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.gamma.blue);

			wcs_convert(gdipp_get_gdimm_setting(L"hinting", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.hinting);
			wcs_convert(gdipp_get_gdimm_setting(L"kerning", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.kerning);
			wcs_convert(gdipp_get_gdimm_setting(L"max_height", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.max_height);

			wcs_convert(gdipp_get_gdimm_setting(L"renderer", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.renderer);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mode/mono", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.render_mode.mono);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mode/gray", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.render_mode.gray);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mode/subpixel", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.render_mode.subpixel);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mode/pixel_geometry", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.render_mode.pixel_geometry);
			wcs_convert(gdipp_get_gdimm_setting(L"render_mode/aliased_text", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.render_mode.aliased_text);

			wcs_convert(gdipp_get_gdimm_setting(L"shadow/offset_x", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.shadow.offset_x);
			wcs_convert(gdipp_get_gdimm_setting(L"shadow/offset_x", font_trait.font_name, font_trait.weight_class, font_trait.italic), &new_cache.shadow.offset_y);
			wcs_convert(gdipp_get_gdimm_setting(L"shadow/alpha", font_trait.font_name, font_trait.weight_class, font_trait.italic), (WORD *)&new_cache.shadow.alpha);

			_cache[key] = new_cache;
		}
	}

	return &_cache[key];
}