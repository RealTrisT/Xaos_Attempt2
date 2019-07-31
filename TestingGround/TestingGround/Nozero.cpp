#include "Nozero.h"

#include <cstdlib>

void Nozero_Rays(const std::vector<f2coord>& coords, const std::vector<uint16_t>& skips, std::vector<std::vector<float>>& result, float width, size_t height, size_t runs_per_h) {
	//line vector
	std::vector<line> lines(coords.size() - skips.size());

	//////////////////////////////// Filling the lines array with info regarding the segments

	//i = index on the coords array; si = index on the skips array; li = index on the lines array
	for (size_t i = 0, si = 0, li = 0; i < coords.size() - 1; i++, li++) {

		//if this point is a skip, don't add it as a line
		if (si != skips.size() && i == skips[si]) { si++; li--; continue; }

		//build the object
		lines[li].p[0] = coords[i + 0];
		lines[li].p[1] = coords[i + 1];
		lines[li].down = coords[i + 0].y < coords[i + 1].y;

		if (lines[li].p[0].x == lines[li].p[1].x) {
			lines[li].eq.vertical = true;
			lines[li].eq.x = lines[li].p[0].x;
		} else {
			lines[li].eq.m = (lines[li].p[0].y - lines[li].p[1].y) / (lines[li].p[0].x - lines[li].p[1].x);
			lines[li].eq.b = lines[li].eq.m * (-lines[li].p[0].x) + lines[li].p[0].y;
		}
	}

	//////////////////////////////// Computing the rays for each row separation

	float y_pix_incr = 1.f / (runs_per_h + 1);

	//now for every row
	for (size_t y = 0; y < height; y++) {
		float y_pix_center = float(y);
		for (size_t r = 0; r < runs_per_h; r++) {
			y_pix_center += y_pix_incr;

			std::vector<float>& rays = result[y * runs_per_h + r] = std::vector<float>();

			//and add all the lines that apply to our current Y value
			for (auto& line : lines) {
				if (
					(line.p[0].y <= y_pix_center && line.p[1].y >= y_pix_center)
					||
					(line.p[0].y >= y_pix_center && line.p[1].y <= y_pix_center)
					) {
					rays.push_back((line.eq.vertical) ? line.eq.x : ((y_pix_center - line.eq.b) / line.eq.m));
				}
			}

			qsort(rays.data(), rays.size(), sizeof(float), [](const void* a, const void* b) -> int {
				if (*(float*)a < *(float*)b)		return -1;
				else if (*(float*)a == *(float*)b)	return  0;
				else								return  1;
			});

			rays.push_back(width);
		}
	}
}

void Nozero_Simple(const std::vector<std::vector<float>>& rays, uint8_t* result, size_t width, size_t height){
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0, ray_i = 0, set = 0; x < width; x++) {
			while (rays[y][ray_i] < (float(x) + 0.5)) { ray_i++; set = !set; }
			result[y * width + x] = set ? 0xFF : 0;
		}
	}
}

void Nozero_AA(const std::vector<std::vector<float>>& rays, uint8_t* result, size_t width, size_t height, size_t multiplier) {
	if (height * multiplier != rays.size())throw std::invalid_argument("Amount of rays must be height * multiplier.");

	float* row = new float[width] {0};

	size_t pixel_subdivisions = multiplier * multiplier;
	float pixel_subdivision_value = 1.f / float(pixel_subdivisions);
	float x_advance = 1.f / float(multiplier + 1);

	for (size_t y = 0; y < height; y++) {
		for (size_t m_y = 0; m_y < multiplier; m_y++) {
			for (size_t x = 0, ray_i = 0, set = 0; x < width; x++) {
				float curr_x = float(x);
				for (size_t m_x = 0; m_x < multiplier; m_x++) {
					curr_x += x_advance;
					while (rays[y*multiplier+m_y][ray_i] < curr_x) { ray_i++; set = !set; }
					if(set)row[x] += pixel_subdivision_value;
				}
			}
		}
		for (size_t x = 0; x < width; x++) { result[y * width + x] = 0xFF * row[x]; row[x] = 0; }
	}

	delete[] row;
}
