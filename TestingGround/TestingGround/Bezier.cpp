#include "Bezier.h"

f2coord Bezier::InterpolateSegment(f2coord a, f2coord b, float far) {
	f2coord vec = { b.x - a.x, b.y - a.y };
	return { a.x + vec.x * far, a.y + vec.y * far };
}

f2coord Bezier::Quadratic(f2coord a, f2coord cp, f2coord b, float far) {
	return InterpolateSegment(
		InterpolateSegment(a, cp, far),
		InterpolateSegment(cp, b, far),
		far
	);
}