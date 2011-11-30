#pragma once

#include "gdipp_client/gamma.h"
#include "gdipp_client/hook.h"
#include "gdipp_client/mem_man.h"
#include "gdipp_config/client_config_static.h"
#include "gdipp_config/config.h"
#include "gdipp_config/render_config_delta_cache.h"

namespace gdipp
{

extern HMODULE h_self;
extern bool os_support_directwrite;
extern RPC_BINDING_HANDLE h_gdipp_rpc;

extern config_file config_file_instance;
extern config config_instance;
extern client_config_static client_config_instance;
extern gamma gamma_instance;
extern hook hook_instance;
extern mem_man mem_man_instance;
extern render_config_delta_cache render_config_delta_cache_instance;

}
