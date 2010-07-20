#pragma once

#include "font_link.h"
#include "font_store.h"
#include "gamma.h"
#include "hook.h"
#include "setting_cache.h"

extern HMODULE h_self;

extern gdimm_font_link font_link_instance;
extern gdimm_font_store font_store_instance;
extern gdimm_gamma gamma_instance;
extern gdimm_hook hook_instance;
extern gdimm_setting_cache setting_cache_instance;