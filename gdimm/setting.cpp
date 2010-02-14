#include "stdafx.h"
#include "setting.h"
#include <objbase.h>
#include <xmllite.h>
#include <shlwapi.h>
#include <sstream>
using namespace std;

_gdimm_setting::_gdimm_setting()
{
	_settings[TEXT("bold_strength")] = TEXT("0.0");
	_settings[TEXT("hinting")] = TEXT("0");
	_settings[TEXT("gray_render")] = TEXT("0");
	_settings[TEXT("lcd_filter")] = TEXT("1");
	_settings[TEXT("max_height")] = TEXT("72");
	_settings[TEXT("render_mono")] = TEXT("0");
}

void _gdimm_setting::load_settings(HMODULE h_module)
{
	HRESULT hr;

	// get setting file path
	TCHAR setting_path[MAX_PATH];
	get_dir_file_path(setting_path, TEXT("setting.xml"), h_module);

	IStream *stream;
	hr = SHCreateStreamOnFileEx(setting_path, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &stream);
	if (FAILED(hr))
		return;

	// read XML

	IXmlReader *reader;
	hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &reader, NULL);
	if (FAILED(hr))	
	{
		reader->Release();
		return;
	}

	hr = reader->SetInput(stream);
	if (FAILED(hr))
	{
		reader->Release();
		stream->Release();
		return;
	}

	XmlNodeType node_type;
	t_string curr_name;

	while (reader->Read(&node_type) == S_OK)
	{
		switch (node_type)
		{
		case XmlNodeType_Element:
			{
				const TCHAR *str;
				hr = reader->GetLocalName(&str, NULL);
				assert(SUCCEEDED(hr));

				curr_name = str;
				break;
			}
		case XmlNodeType_Text:
			{
				if (!curr_name.empty())
				{
					const TCHAR *curr_value;
					hr = reader->GetValue(&curr_value, NULL);
					assert(SUCCEEDED(hr));

					_settings[curr_name] = curr_value;
					curr_name.clear();
				}
				break;
			}
		}
	}

	reader->Release();
	stream->Release();
}