#include "stdafx.h"
#include "setting.h"
using namespace std::tr1;

gdipp_setting::gdipp_setting()
{
	DWORD dw_ret = GetModuleBaseNameW(GetCurrentProcess(), NULL, _process_name, MAX_PATH);
	assert(dw_ret != 0);

	_xml_doc = NULL;
}

void gdipp_setting::parse_gdimm_setting_node(const CComPtr<IXMLDOMNode> setting_node, setting_map &setting_store)
{
	HRESULT hr;

	CComBSTR setting_name;
	hr = setting_node->get_nodeName(&setting_name);
	assert(SUCCEEDED(hr));

	if (setting_name == L"freetype" || setting_name == L"gamma" || setting_name == L"shadow")
	{
		// these settings have nested items

		CComPtr<IXMLDOMNodeList> nested_settings;
		hr = setting_node->get_childNodes(&nested_settings);
		assert(SUCCEEDED(hr));

		long nested_count;
		hr = nested_settings->get_length(&nested_count);
		assert(SUCCEEDED(hr));

		for (long i = 0; i < nested_count; i++)
		{
			CComPtr<IXMLDOMNode> curr_nested;
			hr = nested_settings->get_item(i, &curr_nested);
			assert(SUCCEEDED(hr));

			CComBSTR nested_name;
			hr = curr_nested->get_nodeName(&nested_name);
			assert(SUCCEEDED(hr));

			CComBSTR setting_value;
			hr = curr_nested->get_text(&setting_value);
			assert(SUCCEEDED(hr));

			const wstring final_name = (const wstring) setting_name + L'/' + (const wstring) nested_name;
			setting_store[final_name] = setting_value;
		}
	}
	else
	{
		CComBSTR setting_value;
		hr = setting_node->get_text(&setting_value);
		assert(SUCCEEDED(hr));

		setting_store[(const wstring) setting_name] = setting_value;
	}
}

void gdipp_setting::load_gdimm_process(const CComPtr<IXMLDOMNodeList> process_nodes)
{
	// backward iterate so that first-coming process settings overwrites last-coming ones

	HRESULT hr;

	long proc_count;
	hr = process_nodes->get_length(&proc_count);
	assert(SUCCEEDED(hr));

	for (long i = proc_count - 1; i >= 0; i--)
	{
		CComPtr<IXMLDOMNode> curr_proc;
		
		hr = process_nodes->get_item(i, &curr_proc);
		assert(SUCCEEDED(hr));

		// only store the setting items which match the current process name

		CComPtr<IXMLDOMNamedNodeMap> proc_attr;
		hr = curr_proc->get_attributes(&proc_attr);
		assert(SUCCEEDED(hr));

		CComPtr<IXMLDOMNode> proc_name_node;
		hr = proc_attr->getNamedItem(L"name", &proc_name_node);
		assert(SUCCEEDED(hr));

		CComBSTR proc_name;
		hr = proc_name_node->get_text(&proc_name);
		assert(SUCCEEDED(hr));

		const wregex name_ex(proc_name, regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(_process_name, name_ex))
		{
			CComPtr<IXMLDOMNodeList> setting_nodes;
			hr = curr_proc->get_childNodes(&setting_nodes);
			assert(SUCCEEDED(hr));

			long setting_count;
			hr = setting_nodes->get_length(&setting_count);

			for (long j = 0; j < setting_count; j++)
			{
				CComPtr<IXMLDOMNode> curr_setting;
				hr = setting_nodes->get_item(j, &curr_setting);
				assert(SUCCEEDED(hr));

				parse_gdimm_setting_node(curr_setting, _process_setting);
			}
		}
	}
}

void gdipp_setting::load_gdimm_font(const CComPtr<IXMLDOMNodeList> font_node)
{
	HRESULT hr;

	long font_count;
	hr = font_node->get_length(&font_count);
	assert(SUCCEEDED(hr));

	for (long i = 0; i < font_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_font;

		hr = font_node->get_item(i, &curr_font);
		assert(SUCCEEDED(hr));

		CComPtr<IXMLDOMNodeList> setting_nodes;
		hr = curr_font->get_childNodes(&setting_nodes);
		assert(SUCCEEDED(hr));

		long setting_count;
		hr = setting_nodes->get_length(&setting_count);
		assert(SUCCEEDED(hr));

		setting_map curr_settings;
		for (long j = 0; j < setting_count; j++)
		{
			CComPtr<IXMLDOMNode> curr_setting_node;
			hr = setting_nodes->get_item(j, &curr_setting_node);
			assert(SUCCEEDED(hr));

			parse_gdimm_setting_node(curr_setting_node, curr_settings);
		}

		CComPtr<IXMLDOMNamedNodeMap> font_attr;
		hr = curr_font->get_attributes(&font_attr);
		assert(SUCCEEDED(hr));

		CComPtr<IXMLDOMNode> font_name_node;
		hr = font_attr->getNamedItem(L"name", &font_name_node);
		assert(SUCCEEDED(hr));

		CComBSTR font_name;
		hr = font_name_node->get_text(&font_name);
		assert(SUCCEEDED(hr));

		_gdimm_font.push_back(pair<const wstring, setting_map>((const wstring) font_name, curr_settings));
	}
}

void gdipp_setting::load_demo(const CComPtr<IXMLDOMNode> root_node)
{
	// it is OK if the root node is empty

	HRESULT hr;

	CComPtr<IXMLDOMNodeList> setting_nodes;
	hr = root_node->get_childNodes(&setting_nodes);
	assert(SUCCEEDED(hr));

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(SUCCEEDED(hr));

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(SUCCEEDED(hr));

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(SUCCEEDED(hr));

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(SUCCEEDED(hr));

		if (setting_name == L"font")
			_demo_fonts.push_back((const wstring) setting_value);
		else
			_demo_setting[(const wstring) setting_name] = setting_value;
	}
}

void gdipp_setting::load_service(const CComPtr<IXMLDOMNode> root_node)
{
	HRESULT hr;

	CComPtr<IXMLDOMNodeList> setting_nodes;
	hr = root_node->get_childNodes(&setting_nodes);
	assert(SUCCEEDED(hr));

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(SUCCEEDED(hr));

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(SUCCEEDED(hr));

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(SUCCEEDED(hr));

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(SUCCEEDED(hr));

		_service_setting[(const wstring) setting_name] = setting_value;
	}
}

void gdipp_setting::load_exclude(const CComPtr<IXMLDOMNode> root_node)
{
	HRESULT hr;

	CComPtr<IXMLDOMNodeList> setting_nodes;
	hr = root_node->get_childNodes(&setting_nodes);
	assert(SUCCEEDED(hr));

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(SUCCEEDED(hr));

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(SUCCEEDED(hr));

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(SUCCEEDED(hr));

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(SUCCEEDED(hr));

		_exclude_process.push_back((const wstring) setting_value);
	}
}

const wchar_t *gdipp_setting::get_gdimm_setting(const wchar_t *setting_name, const wchar_t *font_name) const
{
	// check setting for the current process
	setting_map::const_iterator setting_iter = _process_setting.find(setting_name);
	if (setting_iter != _process_setting.end())
		return setting_iter->second.c_str();

	// check setting for the specified font
	for (gdimm_list::const_iterator list_iter = _gdimm_font.begin(); list_iter != _gdimm_font.end(); list_iter++)
	{
		const wregex name_ex(list_iter->first.data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(font_name, name_ex))
		{
			setting_iter = list_iter->second.find(setting_name);
			if (setting_iter != list_iter->second.end())
				return setting_iter->second.c_str();
		}
	}

	return NULL;
}

const wchar_t *gdipp_setting::get_demo_setting(const wchar_t *setting_name) const
{
	setting_map::const_iterator iter = _demo_setting.find(setting_name);

	if (iter == _demo_setting.end())
		return NULL;
	else
		return iter->second.c_str();
}

const vector<const wstring> &gdipp_setting::get_demo_fonts() const
{
	return _demo_fonts;
}

const wchar_t *gdipp_setting::get_service_setting(const wchar_t *setting_name) const
{
	setting_map::const_iterator iter = _service_setting.find(setting_name);

	if (iter == _service_setting.end())
		return NULL;
	else
		return iter->second.c_str();
}

bool gdipp_setting::is_process_excluded(const wchar_t *proc_name) const
{
	// if no process name is specified, return true if the current process is excluded
	// otherwise, return true if the specified process is excluded

	const wchar_t *final_name;
	if (proc_name == NULL)
		final_name = _process_name;
	else
		final_name = proc_name;

	for (list<const wstring>::const_iterator iter = _exclude_process.begin(); iter != _exclude_process.end(); iter++)
	{
		const wregex name_ex(iter->data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(final_name, name_ex))
			return true;
	}

	return false;
}

BOOL gdipp_setting::init_setting()
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	if (_xml_doc != NULL)
	{
		_xml_doc->Release();
		_xml_doc = NULL;
	}

	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_xml_doc));
	if (FAILED(hr))
		return FALSE;

	// these methods should not fail so don't inspect result
	_xml_doc->put_async(VARIANT_FALSE);
	_xml_doc->put_resolveExternals(VARIANT_FALSE);
	_xml_doc->put_validateOnParse(VARIANT_FALSE);

	// clear existing settings
	_process_setting.clear();
	_gdimm_font.clear();
	_demo_setting.clear();
	_service_setting.clear();
	_demo_fonts.clear();
	_exclude_process.clear();

	return TRUE;
}

void gdipp_setting::uninit_setting()
{
	if (_xml_doc != NULL)
		_xml_doc->Release();

	CoUninitialize();
}

BOOL gdipp_setting::load_setting(const wchar_t *setting_path)
{
	HRESULT hr;

	VARIANT_BOOL var_b_ret;
	_xml_doc->load(CComVariant(setting_path), &var_b_ret);
	if (var_b_ret == VARIANT_FALSE)
		return FALSE;

	CComPtr<IXMLDOMNodeList> proc_list;
	hr = _xml_doc->selectNodes(L"/gdipp/gdimm/process", &proc_list);
	if (SUCCEEDED(hr))
		load_gdimm_process(proc_list);

	CComPtr<IXMLDOMNodeList> font_list;
	hr = _xml_doc->selectNodes(L"/gdipp/gdimm/font", &font_list);
	if (SUCCEEDED(hr))
		load_gdimm_font(font_list);

	CComPtr<IXMLDOMNode> demo_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/demo", &demo_node);
	if (SUCCEEDED(hr))
		load_demo(demo_node);

	CComPtr<IXMLDOMNode> service_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/service", &service_node);
	if (SUCCEEDED(hr))
		load_service(service_node);

	CComPtr<IXMLDOMNode> exclude_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/exclude", &exclude_node);
	if (SUCCEEDED(hr))
		load_exclude(exclude_node);

	return TRUE;
}

BOOL gdipp_setting::save_setting(const wchar_t *setting_path)
{
	HRESULT hr = _xml_doc->save(CComVariant(setting_path));
	return SUCCEEDED(hr);
}

BOOL gdipp_setting::insert_setting(const wchar_t *node_name, const wchar_t *node_value, const wchar_t *ref_node_xpath)
{
	HRESULT hr;

	CComPtr<IXMLDOMElement> new_node;
	hr = _xml_doc->createElement(CComBSTR(node_name), &new_node);
	if (FAILED(hr))
		return FALSE;

	hr = new_node->put_nodeValue(CComVariant(node_value));
	if (FAILED(hr))
		return FALSE;

	/*CComPtr<IXMLDOMNode> ref_node;
	hr = _xml_doc->selectSingleNode(ref_node_xpath, &ref_node);
	if (FAILED(hr))
		return FALSE(hr);*/

	hr = _xml_doc->insertBefore(new_node, CComVariant(ref_node_xpath), NULL);
	return SUCCEEDED(hr);
}

BOOL gdipp_setting::set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value)
{
	HRESULT hr;

	CComPtr<IXMLDOMAttribute> new_attr;
	hr = _xml_doc->createAttribute(CComBSTR(attr_name), &new_attr);
	if (FAILED(hr))
		return FALSE;

	hr = new_attr->put_nodeValue(CComVariant(attr_value));
	if (FAILED(hr))
		return FALSE;

	CComPtr<IXMLDOMNode> node;
	hr = _xml_doc->selectSingleNode(CComBSTR(node_xpath), &node);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IXMLDOMNamedNodeMap> attr_map;
	hr = node->get_attributes(&attr_map);
	if (FAILED(hr))
		return FALSE;

	hr = attr_map->setNamedItem(new_attr, NULL);
	return SUCCEEDED(hr);
}

BOOL gdipp_setting::remove_setting_item(const wchar_t *node_xpath)
{
	HRESULT hr;

	CComPtr<IXMLDOMNode> node;
	hr = _xml_doc->selectSingleNode(CComBSTR(node_xpath), &node);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IXMLDOMNode> parent;
	hr = node->get_parentNode(&parent);
	if (FAILED(hr))
		return FALSE;

	hr = parent->removeChild(node, NULL);
	return SUCCEEDED(hr);
}