#include "stdafx.h"
#include "lock.h"

CRITICAL_SECTION gdimm_lock::_cs[_LOCK_TYPE_COUNT_];