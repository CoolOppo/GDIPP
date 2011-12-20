#include "stdafx.h"
#include "dc_pool.h"
#include "gdipp_lib/lock.h"

namespace gdipp
{

dc_pool::~dc_pool()
{
	// TODO: change to blockingly wait until _busy.empty() is true
	assert(_busy.empty());

	BOOL b_ret;

	for (std::list<HDC>::const_iterator free_iter = _free.begin(); free_iter != _free.end(); ++free_iter)
	{
		b_ret = DeleteDC(*free_iter);
		assert(b_ret);
	}
}

HDC dc_pool::claim()
{
	// acquire a resource from the pool
	// if no resource exists, create one by calling create() of the template class
	// otherwise, remove one from the free resource set and add to busy set

	lock l(lock::SERVER_DC_POOL);

	HDC hdc;

	if (_free.empty())
	{
		hdc = CreateCompatibleDC(NULL);
	}
	else
	{
		hdc = _free.front();
		_free.pop_front();
	}
	_busy.insert(hdc);

	return hdc;
}

bool dc_pool::free(HDC hdc)
{
	// return claimed resource back to the pool

	lock l(lock::SERVER_DC_POOL);

	std::set<HDC>::const_iterator busy_iter = _busy.find(hdc);
	if (busy_iter == _busy.end())
		return false;

	_free.push_back(*busy_iter);
	_busy.erase(busy_iter);

	return true;
}

}
