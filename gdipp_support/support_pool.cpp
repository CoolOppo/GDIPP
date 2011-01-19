#include <cassert>
#include "support_pool.h"
#include "support_lock.h"

gdipp_pool::~gdipp_pool()
{
	assert(_busy.empty());

	bool b_ret;

	for (set<void *>::const_iterator free_iter = _free.begin(); free_iter != _free.end(); free_iter++)
	{
		b_ret = destroy(*free_iter);
		assert(b_ret);
	}
}

bool gdipp_pool::add(void *new_resource)
{
	gdipp_lock lock("pool");

	if (_busy.find(new_resource) != _busy.end() || _free.find(new_resource) != _free.end())
		return false;

	_free.insert(new_resource);

	return true;
}

void *gdipp_pool::claim()
{
	gdipp_lock lock("pool");

	bool b_ret;
	void *resource;

	if (_free.empty())
	{
		resource = create();
		b_ret = add(resource);
		assert(b_ret);
	}
	else
	{
		resource = *_free.begin();
		_free.erase(_free.begin());
		_busy.insert(resource);
	}

	return resource;
}

bool gdipp_pool::free(void *resource)
{
	gdipp_lock lock("pool");
	
	set<void *>::const_iterator busy_iter = _busy.find(resource);
	if (busy_iter == _busy.end())
		return false;

	_free.insert(*busy_iter);
	_busy.erase(busy_iter);

	return true;
}