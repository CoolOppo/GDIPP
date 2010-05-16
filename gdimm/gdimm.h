#pragma once

#include "font_link.h"
#include "gamma.h"
#include "hook.h"
#include "setting_cache.h"
#include "glyph_cache.h"

extern HMODULE h_self;

extern gdimm_font_link font_link_instance;
extern gdimm_gamma gamma_instance;
extern gdimm_hook hook_instance;
extern gdimm_setting_cache setting_cache_instance;
extern gdimm_glyph_cache glyph_cache_instance;