#pragma once

#include "gdipp_client/gamma.h"
#include "gdipp_client/hook.h"
#include "gdipp_client/mem_man.h"

namespace gdipp
{

extern HMODULE h_self;
extern bool os_support_directwrite;
extern RPC_BINDING_HANDLE h_gdipp_rpc;

extern gamma gamma_instance;
extern hook hook_instance;
extern gdimm_mem_man mem_man_instance;

}
