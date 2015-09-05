#pragma once
#include "algebra3.h"

class BezCurve
{
public:
	vec3 p0, p1, p2, p3;
	double u;
	BezCurve() {};
	BezCurve(vec3 p0, vec3 p1, vec3 p2, vec3 p3, double u);
	//pair<vec3,vec3> interpolate();
	//pair<vec3,vec3> interpolate(int u);
	//pair<vec3,vec3> interpolate(vec3 curves [], int u);
	//draw
};

