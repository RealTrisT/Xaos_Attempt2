#pragma once

struct f4color { float r, g, b, a; };

struct f2coord { float x, y; };

__declspec(align(16)) struct colortexel {
	f4color c; f2coord t;
};

__declspec(align(16)) struct f3coord { float x, y, z; };