#include "stdafx.h"
#include "geometry_sink.h"

gdimm_geometry_sink::gdimm_geometry_sink()
:
_ref_count(0)
{
}

IFACEMETHODIMP gdimm_geometry_sink::QueryInterface(
	IID const& riid,
	void** ppvObject
	)
{
	if (__uuidof(IDWriteTextRenderer) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IDWritePixelSnapping) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IUnknown) == riid)
	{
		*ppvObject = this;
	}
	else
	{
		*ppvObject = NULL;
		return E_FAIL;
	}

	return S_OK;
}

IFACEMETHODIMP_(ULONG) gdimm_geometry_sink::AddRef()
{
	return InterlockedIncrement(&_ref_count);
}

IFACEMETHODIMP_(ULONG) gdimm_geometry_sink::Release()
{
	long newCount = InterlockedDecrement(&_ref_count);

	if (newCount == 0)
	{
		delete this;
		return 0;
	}
	return newCount;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::SetFillMode(
	D2D1_FILL_MODE fillMode 
	)
{
	int a = 0;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::SetSegmentFlags(
	D2D1_PATH_SEGMENT vertexFlags 
	)
{
	int a = 0;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::BeginFigure(
	D2D1_POINT_2F startPoint,
	D2D1_FIGURE_BEGIN figureBegin 
	)
{
	int a = 0;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::AddLines(
	__in_ecount(pointsCount) CONST D2D1_POINT_2F *points,
	UINT pointsCount 
	)
{
	int a = 0;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::AddBeziers(
	__in_ecount(beziersCount) CONST D2D1_BEZIER_SEGMENT *beziers,
	UINT beziersCount 
	)
{
	int a = 0;
}

IFACEMETHODIMP_(void) gdimm_geometry_sink::EndFigure(
	D2D1_FIGURE_END figureEnd 
	)
{
	int a = 0;
}

IFACEMETHODIMP gdimm_geometry_sink::Close()
{
	return S_OK;
}