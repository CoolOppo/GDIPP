#pragma once

namespace gdipp
{

class dc_pool
{
	// pool for managing costly and reusable HDCs
	// all operations are thread-safe

public:
	~dc_pool();
	HDC claim();
	bool free(HDC hdc);

private:
	// free HDCs are ready to be claimed
	// busy HDCs are claimed and being used
	std::list<HDC> _free;
	std::set<HDC> _busy;
};

}
