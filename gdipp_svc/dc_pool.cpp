#include "stdafx.h"
#include <support_pool.h>

HDC gdipp_pool<HDC>::create() const
{
	return CreateCompatibleDC(NULL);
}

bool gdipp_pool<HDC>::destroy(HDC resource) const
{
	return (DeleteDC(reinterpret_cast<HDC>(resource)) == TRUE);
}