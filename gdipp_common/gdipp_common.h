#pragma once

using namespace std;

#ifdef GDIPP_EXPORTS
#define GDIPP_API __declspec(dllexport)
#else
#define GDIPP_API __declspec(dllimport)
#endif // GDIPP_EXPORTS

struct wstring_ci_less
{
	GDIPP_API bool operator()(const wstring &string1, const wstring &string2) const;
};

GDIPP_API BOOL gdipp_get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path);

// setting wrapper APIs
// gdipp_setting uses STL templates, which makes it frustrating to export

class gdimm_setting_trait
{
/*
	char weight_class;
	bool italic;
	LONG height;
	const wchar_t *font_name;
*/

	BYTE *_setting_data;
	int _setting_size;

public:
	gdimm_setting_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name)
	{
		const int font_name_len = static_cast<int>(wcslen(font_name) + 1);
		_setting_size = sizeof(weight_class) + sizeof(italic) + sizeof(height) + font_name_len * sizeof(wchar_t);
		_setting_data = new BYTE[_setting_size];

		*reinterpret_cast<char *>(_setting_data) = weight_class;
		*reinterpret_cast<bool *>(_setting_data + sizeof(char)) = italic;
		*reinterpret_cast<LONG *>(_setting_data + sizeof(char) + sizeof(bool)) = height;
		wcscpy_s(reinterpret_cast<wchar_t *>(_setting_data + sizeof(char) + sizeof(bool) + sizeof(LONG)), font_name_len, font_name);
	}

	~gdimm_setting_trait()
	{
		delete[] _setting_data;
	}

	char get_weight_class() const
	{
		return *reinterpret_cast<char *>(_setting_data);
	}

	bool get_italic() const
	{
		return *reinterpret_cast<bool *>(_setting_data + sizeof(char));
	}

	LONG get_height() const
	{
		return *reinterpret_cast<LONG *>(_setting_data + sizeof(char) + sizeof(bool));
	}

	const wchar_t *get_font_name() const
	{
		return reinterpret_cast<const wchar_t *>(_setting_data + sizeof(char) + sizeof(bool) + sizeof(LONG));
	}

	const BYTE *get_data() const
	{
		return _setting_data;
	}

	int get_size() const
	{
		return _setting_size;
	}
};

GDIPP_API void gdipp_init_setting();
GDIPP_API BOOL gdipp_load_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_save_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, wstring &new_node_xpath);
GDIPP_API BOOL gdipp_set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
GDIPP_API BOOL gdipp_remove_setting(const wchar_t *node_xpath);

GDIPP_API const wchar_t *gdipp_get_gdimm_setting(const wchar_t *setting_name, const gdimm_setting_trait *setting_trait);
GDIPP_API const wchar_t *gdipp_get_demo_setting(const wchar_t *setting_name);
GDIPP_API const vector<const wstring> &gdipp_get_demo_fonts();
GDIPP_API const wchar_t *gdipp_get_service_setting(const wchar_t *setting_name);
GDIPP_API bool gdipp_is_process_excluded(const wchar_t *proc_name);

// convert a string to template value if possible
// helper function to convert raw setting strings to values
template <typename T>
void wcs_convert(const wchar_t *str, T *converted)
{
	if (str != NULL)
		wistringstream(str) >> *converted;
}

// injector APIs

typedef LONG NTSTATUS;
GDIPP_API NTSTATUS gdipp_inject_process(ULONG process_id, ULONG thread_id = 0);

// debug APIs
GDIPP_API void gdipp_init_minidump();
GDIPP_API void gdipp_register_minidump_module(HMODULE h_module);

GDIPP_API void gdipp_debug_buffer(const void *ptr, unsigned int size);
GDIPP_API void gdipp_debug_decimal(double num, bool new_line = true);
GDIPP_API void gdipp_debug_integer(size_t num, bool new_line = true);
GDIPP_API void gdipp_debug_string(const wchar_t *str = L"");