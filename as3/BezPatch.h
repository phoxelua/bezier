#pragma once
#include "BezCurve.h"

class BezPatch
{
public:
	BezCurve c0, c1, c2, c3;
	double u, v;
	BezPatch() {};
	BezPatch(BezCurve c0, BezCurve c1, BezCurve c2, BezCurve c3, double u, double v);
	//pair<vec3,vec3> interpolate();
};

