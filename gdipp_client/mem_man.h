#pragma once

namespace gdipp
{

class mem_man
{
	// helper class to free static memory pointers

public:
	~mem_man();

	void register_com_ptr(IUnknown *com_ptr);
	void register_mem_ptr(void *mem_ptr);
	void register_heap_ptr(LPVOID mem_ptr);

private:
	std::list<IUnknown *> _com_ptr_store;
	std::list<void *> _mem_ptr_store;
};

}
