#pragma once

//using std::;

class gdimm_mem_man
{
	// helper class to free static memory pointers

public:
	~gdimm_mem_man();

	void register_com_ptr(IUnknown *com_ptr);
	void register_mem_ptr(void *mem_ptr);
	void register_heap_ptr(LPVOID mem_ptr);

private:
	list<IUnknown *> _com_ptr_store;
	list<void *> _mem_ptr_store;
};
