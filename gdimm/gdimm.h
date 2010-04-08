#pragma once

#include "font_link.h"
#include "font_man.h"
#include "gamma.h"
#include "hook.h"
#include "setting_cache.h"

extern HMODULE h_self;

extern gdimm_font_link font_link_instance;
extern gdimm_font_man font_man_instance;
extern gdimm_gamma gamma_instance;
extern gdimm_hook hook_instance;
extern gdimm_setting_cache setting_cache_instance;

enum CS_TYPE
{
	CS_GAMMA,
	CS_HOOK,
	CS_LINKED_FONT,
	CS_REG_FONT,
	CS_TEXT,
	CS_DEBUG,

	// count of critical types
	// never use this type
	_CS_TYPE_COUNT_
};

class critical_section
{
	static CRITICAL_SECTION _cs[_CS_TYPE_COUNT_];
	CS_TYPE _type;

public:
	critical_section(CS_TYPE type)
	{
		_type = type;
		EnterCriticalSection(&_cs[type]);
	}

	~critical_section()
	{
		LeaveCriticalSection(&_cs[_type]);
	}

	static void initialize()
	{
		for (int i = 0; i < _CS_TYPE_COUNT_; i++)
			InitializeCriticalSection(&_cs[i]);
	}

	static void release()
	{
		for (int i = 0; i < _CS_TYPE_COUNT_; i++)
			DeleteCriticalSection(&_cs[i]);
	}
};

const WCHAR *metric_family_name(const OUTLINETEXTMETRICW *outline_metric);
const WCHAR *metric_face_name(const OUTLINETEXTMETRICW *outline_metric);
const WCHAR *metric_style_name(const OUTLINETEXTMETRICW *outline_metric);