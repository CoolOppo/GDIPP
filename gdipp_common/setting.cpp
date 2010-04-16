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
	assert(hr == S_OK);

	if (setting_name == L"freetype" || setting_name == L"gamma" || setting_name == L"shadow")
	{
		// these settings have nested items

		CComPtr<IXMLDOMNodeList> nested_settings;
		hr = setting_node->get_childNodes(&nested_settings);
		assert(hr == S_OK);

		long nested_count;
		hr = nested_settings->get_length(&nested_count);
		assert(hr == S_OK);

		for (long i = 0; i < nested_count; i++)
		{
			CComPtr<IXMLDOMNode> curr_nested;
			hr = nested_settings->get_item(i, &curr_nested);
			assert(hr == S_OK);

			CComBSTR nested_name;
			hr = curr_nested->get_nodeName(&nested_name);
			assert(hr == S_OK);

			CComBSTR setting_value;
			hr = curr_nested->get_text(&setting_value);
			assert(hr == S_OK);

			const wstring final_name = (const wstring) setting_name + L'/' + (const wstring) nested_name;
			setting_store[final_name] = setting_value;
		}
	}
	else
	{
		CComBSTR setting_value;
		hr = setting_node->get_text(&setting_value);
		assert(hr == S_OK);

		setting_store[(const wstring) setting_name] = setting_value;
	}
}

void gdipp_setting::load_gdimm_process(const CComPtr<IXMLDOMNodeList> process_nodes)
{
	// backward iterate so that first-coming process settings overwrites last-coming ones

	HRESULT hr;

	long proc_count;
	hr = process_nodes->get_length(&proc_count);
	assert(hr == S_OK);

	for (long i = proc_count - 1; i >= 0; i--)
	{
		CComPtr<IXMLDOMNode> curr_proc;
		
		hr = process_nodes->get_item(i, &curr_proc);
		assert(hr == S_OK);

		// only store the setting items which match the current process name

		CComPtr<IXMLDOMNamedNodeMap> proc_attr;
		hr = curr_proc->get_attributes(&proc_attr);
		assert(hr == S_OK);

		CComPtr<IXMLDOMNode> proc_name_node;
		hr = proc_attr->getNamedItem(L"name", &proc_name_node);
		assert(hr == S_OK);

		CComBSTR proc_name;
		hr = proc_name_node->get_text(&proc_name);
		assert(hr == S_OK);

		const wregex name_ex(proc_name, regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(_process_name, name_ex))
		{
			CComPtr<IXMLDOMNodeList> setting_nodes;
			hr = curr_proc->get_childNodes(&setting_nodes);
			assert(hr == S_OK);

			long setting_count;
			hr = setting_nodes->get_length(&setting_count);

			for (long j = 0; j < setting_count; j++)
			{
				CComPtr<IXMLDOMNode> curr_setting;
				hr = setting_nodes->get_item(j, &curr_setting);
				assert(hr == S_OK);

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
	assert(hr == S_OK);

	for (long i = 0; i < font_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_font;

		hr = font_node->get_item(i, &curr_font);
		assert(hr == S_OK);

		CComPtr<IXMLDOMNodeList> setting_nodes;
		hr = curr_font->get_childNodes(&setting_nodes);
		assert(hr == S_OK);

		long setting_count;
		hr = setting_nodes->get_length(&setting_count);
		assert(hr == S_OK);

		setting_map curr_settings;
		for (long j = 0; j < setting_count; j++)
		{
			CComPtr<IXMLDOMNode> curr_setting_node;
			hr = setting_nodes->get_item(j, &curr_setting_node);
			assert(hr == S_OK);

			parse_gdimm_setting_node(curr_setting_node, curr_settings);
		}

		CComPtr<IXMLDOMNamedNodeMap> font_attr;
		hr = curr_font->get_attributes(&font_attr);
		assert(hr == S_OK);

		CComPtr<IXMLDOMNode> font_name_node;
		hr = font_attr->getNamedItem(L"name", &font_name_node);
		assert(hr == S_OK);

		CComBSTR font_name;
		hr = font_name_node->get_text(&font_name);
		assert(hr == S_OK);

		_gdimm_font.push_back(pair<const wstring, setting_map>((const wstring) font_name, curr_settings));
	}
}

void gdipp_setting::load_demo(const CComPtr<IXMLDOMNode> root_node)
{
	// it is OK if the root node is empty

	HRESULT hr;

	CComPtr<IXMLDOMNodeList> setting_nodes;
	hr = root_node->get_childNodes(&setting_nodes);
	assert(hr == S_OK);

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(hr == S_OK);

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(hr == S_OK);

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(hr == S_OK);

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(hr == S_OK);

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
	assert(hr == S_OK);

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(hr == S_OK);

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(hr == S_OK);

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(hr == S_OK);

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(hr == S_OK);

		_service_setting[(const wstring) setting_name] = setting_value;
	}
}

void gdipp_setting::load_exclude(const CComPtr<IXMLDOMNode> root_node)
{
	HRESULT hr;

	CComPtr<IXMLDOMNodeList> setting_nodes;
	hr = root_node->get_childNodes(&setting_nodes);
	assert(hr == S_OK);

	long setting_count;
	hr = setting_nodes->get_length(&setting_count);
	assert(hr == S_OK);

	for (long i = 0; i < setting_count; i++)
	{
		CComPtr<IXMLDOMNode> curr_node;
		hr = setting_nodes->get_item(i, &curr_node);
		assert(hr == S_OK);

		CComBSTR setting_name;
		hr = curr_node->get_nodeName(&setting_name);
		assert(hr == S_OK);

		CComBSTR setting_value;
		hr = curr_node->get_text(&setting_value);
		assert(hr == S_OK);

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

void gdipp_setting::init_setting()
{
	HRESULT hr;

	if (_xml_doc == NULL)
	{
		// first time called, initialize COM library
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		assert(SUCCEEDED(hr));
	}
	else
	{
		// called again
		// release previous DOM document
		_xml_doc->Release();
		_xml_doc = NULL;
	}

	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_xml_doc));
	assert(hr == S_OK);

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
}

void gdipp_setting::uninit_setting()
{
	// release any DOM node that is not released yet
	for (list<IXMLDOMNode*>::const_iterator iter = _not_released_nodes.begin(); iter != _not_released_nodes.end(); iter++)
		(*iter)->Release();

	// release the DOM document
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
	if (hr == S_OK)
		load_gdimm_process(proc_list);

	CComPtr<IXMLDOMNodeList> font_list;
	hr = _xml_doc->selectNodes(L"/gdipp/gdimm/font", &font_list);
	if (hr == S_OK)
		load_gdimm_font(font_list);

	CComPtr<IXMLDOMNode> demo_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/demo", &demo_node);
	if (hr == S_OK)
		load_demo(demo_node);

	CComPtr<IXMLDOMNode> service_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/service", &service_node);
	if (hr == S_OK)
		load_service(service_node);

	CComPtr<IXMLDOMNode> exclude_node;
	hr = _xml_doc->selectSingleNode(L"/gdipp/exclude", &exclude_node);
	if (hr == S_OK)
		load_exclude(exclude_node);

	return TRUE;
}

BOOL gdipp_setting::save_setting(const wchar_t *setting_path)
{
	return (_xml_doc->save(CComVariant(setting_path)) == S_OK);
}

void *gdipp_setting::insert_setting(const wchar_t *node_name, const wchar_t *node_value, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath)
{
	HRESULT hr;

	CComPtr<IXMLDOMElement> new_node;
	hr = _xml_doc->createElement(CComBSTR(node_name), &new_node);
	if (hr != S_OK)
		return NULL;

	hr = new_node->put_text(CComBSTR(node_value));
	if (hr != S_OK)
		return NULL;

	CComPtr<IXMLDOMNode> parent_node;
	hr = _xml_doc->selectSingleNode(CComBSTR(parent_xpath), &parent_node);
	if (hr != S_OK)
		return NULL;

	CComVariant var_ref;
	if (ref_node_xpath != NULL)
	{
		CComPtr<IXMLDOMNode> ref_node;
		hr = parent_node->selectSingleNode(CComBSTR(ref_node_xpath), &ref_node);
		if (hr != S_OK)
			return NULL;

		var_ref = ref_node;
	}

	// record this not-released node
	IXMLDOMNode *out_new_node = NULL;
	hr = parent_node->insertBefore(new_node, var_ref, &out_new_node);
	if (hr != S_OK)
		return NULL;

	_not_released_nodes.push_back(out_new_node);
	return out_new_node;
}

BOOL gdipp_setting::set_setting_attr(const void *node_ptr, const wchar_t *attr_name, const wchar_t *attr_value)
{
	HRESULT hr;

	if (node_ptr == NULL)
		return FALSE;

	IXMLDOMNode *node = (IXMLDOMNode*) node_ptr;

	CComPtr<IXMLDOMAttribute> new_attr;
	hr = _xml_doc->createAttribute(CComBSTR(attr_name), &new_attr);
	if (hr != S_OK)
		return FALSE;

	hr = new_attr->put_nodeValue(CComVariant(attr_value));
	if (hr != S_OK)
		return FALSE;

	CComPtr<IXMLDOMNamedNodeMap> attr_map;
	hr = node->get_attributes(&attr_map);
	if (hr != S_OK)
		return FALSE;

	return (attr_map->setNamedItem(new_attr, NULL) == S_OK);
}

BOOL gdipp_setting::remove_setting_item(const wchar_t *node_xpath)
{
	HRESULT hr;

	CComPtr<IXMLDOMNode> node;
	hr = _xml_doc->selectSingleNode(CComBSTR(node_xpath), &node);
	if (hr != S_OK)
		return FALSE;

	CComPtr<IXMLDOMNode> parent;
	hr = node->get_parentNode(&parent);
	if (hr != S_OK)
		return FALSE;

	return (parent->removeChild(node, NULL) == S_OK);
}