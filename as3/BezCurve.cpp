#include "BezCurve.h"


BezCurve::BezCurve(vec3 p0, vec3 p1, vec3 p2, vec3 p3, double u)
{
	this->p0 = p0;
	this->p1 = p1;
	this->p2 = p2;
	this->p3 = p3;
	this->u = u;
}

////Return point and derivative
//pair<vec3, vec3> BezCurve::interpolate()
//{
//	vec3 A = p0 * (1.0-u) + p1 * u;
//	vec3 B = p1 * (1.0-u) + p2 * u;
//	vec3 C = p2 * (1.0-u) + p3 * u;
//
//	vec3 D = A * (1.0-u) + B * u;
//	vec3 E = B * (1.0-u) + C * u;
//
//	return pair<vec3, vec3>(D * (1.0-u) + E * u, 3*(E - D));
//}
//
//pair<vec3, vec3> BezCurve::interpolate(int u)
//{
//	vec3 A = p0 * (1.0-u) + p1 * u;
//	vec3 B = p1 * (1.0-u) + p2 * u;
//	vec3 C = p2 * (1.0-u) + p3 * u;
//
//	vec3 D = A * (1.0-u) + B * u;
//	vec3 E = B * (1.0-u) + C * u;
//
//	return pair<vec3, vec3>(D * (1.0-u) + E * u, 3*(E - D));
//}


