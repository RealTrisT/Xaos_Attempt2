#pragma once
#include <vector>
#include "Datatypes.h"

#include <stdexcept>
//line segment
struct line {
	//the 2 points that constitute the line segment
	f2coord p[2];
	//whether or not the line is going down (so if the y of the first point is > than the y of the second)
	bool down;

	//equation data
	struct {
		//whether or not it's vertical, since if that was the case the equation would be undefined
		bool vertical;
		//a union, since we're only gonna be using one of the 2, no need for wasting space
		union {
			//variables of the equation
			struct {
				float m;
				float b;
			};
			//the x in cas it's vertical. This can already be taken from one of the points, but it's not a waste of space either sine it's a union, so just for convenience
			float x;
		};
	}eq;
};

//-----------------------------------------------------------------------------------------------------------------------Nozero


void Nozero_Rays(const std::vector<f2coord>& coords, const std::vector<uint16_t>& skips, std::vector<std::vector<float>>& result, float width, size_t height, size_t runs_per_h);
void Nozero_Simple(const std::vector<std::vector<float>>& rays, uint8_t* result, size_t width, size_t heigh);
void Nozero_AA(const std::vector<std::vector<float>>& rays, uint8_t* result, size_t width, size_t height, size_t multiplier);
