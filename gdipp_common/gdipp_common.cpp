#include "stdafx.h"
#include "gdipp_common.h"
#include "setting.h"
#include "minidump.h"

gdipp_setting setting_instance;

bool wstring_ci_less::operator()(const wstring &string1, const wstring &string2) const
{
	return (_wcsicmp(string1.c_str(), string2.c_str()) < 0);
}

BOOL gdipp_get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path)
{
	// append the file name after the module's resident directory name
	// if the module handle is NULL, use the current exe as the module

	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileNameW(h_module, out_path, MAX_PATH);
	if (dw_ret == 0)
		return FALSE;

	b_ret = PathRemoveFileSpecW(out_path);
	if (!b_ret)
		return FALSE;

	return PathAppendW(out_path, file_name);
}

void gdipp_init_setting()
{
	return setting_instance.init_setting();
}

BOOL gdipp_load_setting(const wchar_t *setting_path)
{
	return setting_instance.load_setting(setting_path);
}

BOOL gdipp_save_setting(const wchar_t *setting_path)
{
	return setting_instance.save_setting(setting_path);
}

BOOL gdipp_insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, wstring &new_node_xpath)
{
	return setting_instance.insert_setting(node_name, node_text, parent_xpath, ref_node_xpath, new_node_xpath);
}

BOOL gdipp_set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value)
{
	return setting_instance.set_setting_attr(node_xpath, attr_name, attr_value);
}

BOOL gdipp_remove_setting(const wchar_t *node_xpath)
{
	return setting_instance.remove_setting(node_xpath);
}

const wchar_t *gdipp_get_gdimm_setting(const wchar_t *setting_name, const gdimm_setting_trait *setting_trait)
{
	return setting_instance.get_gdimm_setting(setting_name, setting_trait);
}

const wchar_t *gdipp_get_demo_setting(const wchar_t *setting_name)
{
	return setting_instance.get_demo_setting(setting_name);
}

const vector<const wstring> &gdipp_get_demo_fonts()
{
	return setting_instance.get_demo_fonts();
}

bool gdipp_is_process_excluded(const wchar_t *proc_name)
{
	return setting_instance.is_process_excluded(proc_name);
}

void gdipp_register_minidump_module(HMODULE h_module)
{
	h_minidump_modules.push_back(h_module);
}

const wchar_t *debug_file_name = L"C:\\gdipp_debug.log";

void gdipp_debug_buffer(const void *ptr, unsigned int size)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");
	
	if (f != NULL)
	{
		fwrite(ptr, 1, size, f);
		fclose(f);
	}
}

void gdipp_debug_decimal(double num)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");
	
	if (f != NULL)
	{
		fwprintf(f, L"%f\n", num);
		fclose(f);
	}
}

void gdipp_debug_integer(size_t num)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");
	
	if (f != NULL)
	{
		fwprintf(f, L"%u\n", num);
		fclose(f);
	}
}

void gdipp_debug_string(const wchar_t *str)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");
	
	if (f != NULL)
	{
		fwprintf(f, L"%s\n", str);
		fclose(f);
	}
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	static PVOID h_ex_handler = NULL;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gdipp_register_minidump_module(hModule);

		// add process-wide vectored exception handler to create minidump
		h_ex_handler = AddVectoredExceptionHandler(0, create_minidump);
		assert(h_ex_handler != NULL);

		break;
	case DLL_PROCESS_DETACH:
		ULONG ul_ret = RemoveVectoredExceptionHandler(h_ex_handler);
		assert(ul_ret != 0);

		break;
	}

	return TRUE;
}