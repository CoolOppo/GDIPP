#include "stdafx.h"
#include "mem_man.h"

namespace gdipp
{

gdimm_mem_man::~gdimm_mem_man()
{
	for (std::list<IUnknown *>::const_iterator iter = _com_ptr_store.begin(); iter != _com_ptr_store.end(); ++iter)
		(*iter)->Release();

	for (std::list<void *>::const_iterator iter = _mem_ptr_store.begin(); iter != _mem_ptr_store.end(); ++iter)
		delete *iter;
}

void gdimm_mem_man::register_com_ptr(IUnknown *com_ptr)
{
	_com_ptr_store.push_back(com_ptr);
}

void gdimm_mem_man::register_mem_ptr(void *mem_ptr)
{
	_mem_ptr_store.push_back(mem_ptr);
}

void gdimm_mem_man::register_heap_ptr(LPVOID mem_ptr)
{
	HeapFree(GetProcessHeap(), 0, mem_ptr);
}

}
