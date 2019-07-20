#pragma once

#include <vector>

struct Font {
	virtual bool GetRGBA32RenderedGlyphFromUTF8(uint32_t code_point, float pixels_per_em, uint32_t* target, float* offsets) = 0;
};

#include <map>
struct VectorFont : Font {
	struct GlyphData {
		struct GlyfCoords { int16_t x, y; bool on_curve; };
		std::vector<GlyfCoords> coords;
		std::vector<uint16_t> skips;
		uint16_t x_offset, y_offset;
		uint16_t width, height;
	};

	std::map<uint32_t, GlyphData> glyphs;

	bool AddCharacterUTF8(uint32_t code_point, GlyphData data);
	bool GetRGBA32RenderedGlyphFromUTF8(uint32_t code_point, float pixels_per_em, uint32_t* target, float* offsets);
}; 