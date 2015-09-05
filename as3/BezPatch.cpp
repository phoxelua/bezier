#include "BezPatch.h"


BezPatch::BezPatch(BezCurve c0, BezCurve c1, BezCurve c2, BezCurve c3, double u, double v)
{
	this->c0 = c0;
	this->c1 = c1;
	this->c2 = c2;
	this->c3 = c3;
	this->u = u;
	this->v = v;
}

