#pragma once

#include <set>

using std::set;

template <typename T>
class gdipp_pool
{
	// pool for managing costly and reusable resources
	// all operations are thread-safe

public:
	~gdipp_pool()
	{
		assert(_busy.empty());

		bool b_ret;

		for (set<T>::const_iterator free_iter = _free.begin(); free_iter != _free.end(); ++free_iter)
		{
			b_ret = destroy(*free_iter);
			assert(b_ret);
		}
	}

	T claim()
	{
		gdipp_lock lock("pool");

		bool b_ret;
		T resource;

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

	bool free(T resource)
	{
		gdipp_lock lock("pool");

		set<T>::const_iterator busy_iter = _busy.find(resource);
		if (busy_iter == _busy.end())
			return false;

		_free.insert(*busy_iter);
		_busy.erase(busy_iter);

		return true;
	}

private:
	T create() const;
	bool destroy(T resource) const;

	bool add(T new_resource)
	{
		gdipp_lock lock("pool");

		if (_busy.find(new_resource) != _busy.end() || _free.find(new_resource) != _free.end())
			return false;

		_free.insert(new_resource);

		return true;
	}

	// busy resources are claimed and being used
	// free resources are ready to be claimed
	set<T> _busy;
	set<T> _free;
};
