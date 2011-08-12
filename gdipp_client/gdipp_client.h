#pragma once

#include "gdimm/gamma.h"
#include "gdimm/hook.h"
#include "gdimm/mem_man.h"

namespace gdipp
{

extern HMODULE h_self;
extern bool os_support_directwrite;
extern RPC_BINDING_HANDLE h_gdipp_rpc;

extern gdimm_gamma gamma_instance;
extern hook hook_instance;
extern gdimm_mem_man mem_man_instance;

}
