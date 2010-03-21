#include "stdafx.h"
#include "setting_cache.h"

gdimm_setting_cache::~gdimm_setting_cache()
{
	for (font_cache::const_iterator font_iter = _cache.begin(); font_iter != _cache.end(); font_iter++)
	{
		for (setting_cache::const_iterator set_iter = font_iter->second.begin(); set_iter != font_iter->second.end(); set_iter++)
			delete set_iter->second;
	}
}