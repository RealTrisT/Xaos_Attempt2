#pragma once

#include "Datatypes.h"

namespace Bezier {

#pragma push_macro("far")
#undef far
	//this returns the point 'far' along the 'a'-'b' segment
	f2coord InterpolateSegment(f2coord a, f2coord b, float far);
	f2coord Quadratic(f2coord a, f2coord cp, f2coord b, float far);
#pragma pop_macro("far")
};