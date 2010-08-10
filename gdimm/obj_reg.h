#pragma once

using namespace std;

class gdimm_obj_registry
{
	list<IUnknown *> _com_ptr_store;
	list<void *> _mem_ptr_store;

public:
	~gdimm_obj_registry();

	void register_com_ptr(IUnknown *com_ptr);
	void register_mem_ptr(void *mem_ptr);
};