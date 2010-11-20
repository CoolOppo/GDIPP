#pragma once

#include "font_link.h"
#include "font_man.h"
#include "gamma.h"
#include "glyph_cache.h"
#include "hook.h"
#include "mem_man.h"
#include "setting_cache.h"

extern HMODULE h_self;
extern bool os_support_directwrite;
extern RPC_BINDING_HANDLE h_gdipp_rpc;

extern gdimm_font_link font_link_instance;
extern gdimm_font_man font_man_instance;
extern gdimm_gamma gamma_instance;
extern gdimm_glyph_cache glyph_cache_instance;
extern gdimm_hook hook_instance;
extern gdimm_mem_man mem_man_instance;
extern gdimm_setting_cache setting_cache_instance;