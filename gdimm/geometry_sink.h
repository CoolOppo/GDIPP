#pragma once

class gdimm_geometry_sink : public IDWriteGeometrySink
{
	LONG _ref_count;

public:
	gdimm_geometry_sink();

public:
	IFACEMETHOD(QueryInterface) ( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	IFACEMETHOD_(ULONG, AddRef) ();

	IFACEMETHOD_(ULONG, Release) ();

public:
	IFACEMETHOD_(void, SetFillMode)(
		D2D1_FILL_MODE fillMode 
		);

	IFACEMETHOD_(void, SetSegmentFlags)(
		D2D1_PATH_SEGMENT vertexFlags 
		);

	IFACEMETHOD_(void, BeginFigure)(
		D2D1_POINT_2F startPoint,
		D2D1_FIGURE_BEGIN figureBegin 
		);

	IFACEMETHOD_(void, AddLines)(
		__in_ecount(pointsCount) CONST D2D1_POINT_2F *points,
		UINT pointsCount 
		);

	IFACEMETHOD_(void, AddBeziers)(
		__in_ecount(beziersCount) CONST D2D1_BEZIER_SEGMENT *beziers,
		UINT beziersCount 
		);

	IFACEMETHOD_(void, EndFigure)(
		D2D1_FIGURE_END figureEnd 
		);

	IFACEMETHOD(Close)(
		);
};