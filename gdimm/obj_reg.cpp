#include "stdafx.h"
#include "obj_reg.h"

gdimm_obj_registry::~gdimm_obj_registry()
{
	for (list<IUnknown *>::const_iterator iter = _com_ptr_store.begin(); iter != _com_ptr_store.end(); iter++)
		(*iter)->Release();

	for (list<void *>::const_iterator iter = _mem_ptr_store.begin(); iter != _mem_ptr_store.end(); iter++)
		delete *iter;
}

void gdimm_obj_registry::register_com_ptr(IUnknown *com_ptr)
{
	_com_ptr_store.push_back(com_ptr);
}

void gdimm_obj_registry::register_mem_ptr(void *mem_ptr)
{
	_mem_ptr_store.push_back(mem_ptr);
}