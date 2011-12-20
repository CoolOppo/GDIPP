#pragma once

namespace gdipp
{

template<typename T>
class lru_list
{
public:
	lru_list()
		: _capacity(0)
	{
	}

	explicit lru_list(size_t capacity)
		: _capacity(capacity)
	{
	}

	void resize(size_t new_capacity)
	{
		_capacity = new_capacity;
	}

	bool access(const T data, T &erased)
	{
		lock l(lock::SERVER_LRU);

		bool overflow = false;

		_map_type::iterator iter = _data_map.find(data);
		if (iter == _data_map.end())
		{
			// data never accessed

			if (_data_map.size() == _capacity && _capacity > 0)
			{
				// list is full
				// erase and return the last accessed data
				erased = _access_list.back();
				_access_list.pop_back();
				_data_map.erase(erased);
				overflow = true;
			}

			// add the data to the most recent position
			_access_list.push_front(data);
			_data_map[data] = _access_list.begin();
		}
		else
		{
			// data accessed before
			// move it to the most recent position
			//
			_list_iter_type node = iter->second;

			if (node != _access_list.begin())
				_access_list.splice(_access_list.begin(), _access_list, node);
		}

		return overflow;
	}

private:
	typedef typename std::list<T> _list_type;
	typedef typename _list_type::iterator _list_iter_type;
	typedef typename std::map<T, _list_iter_type> _map_type;

	_list_type _access_list;
	_map_type _data_map;
	size_t _capacity;
};

}
