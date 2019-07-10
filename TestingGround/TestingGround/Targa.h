#pragma once

#include <stdint.h>
#include <stdio.h>


struct TargaFile {
	#pragma pack(push, 1)
	struct Header {
		uint8_t id_length;				// length of the ID field, which immediately follows the header
		uint8_t colormap_type;			// 0 - no color map; 1 - present; 2 to 127 - reserved; above - developer use
		uint8_t image_type;				// buncha shit I don't think I care about rn
		struct ColorMapSpecification {
			uint16_t first_entry_index;
			uint16_t length;
			uint8_t  entry_size;
		}colormap_specification;
		struct ImageSpecification {
			uint16_t x_origin;
			uint16_t y_origin;
			uint16_t width;
			uint16_t height;
			uint8_t  bpp;
			uint8_t  image_descriptor;

			size_t totalSize();
			size_t bytesPerPixel();
			size_t bitsForAlpha();
		}image_specification;
	}header;
	#pragma pack(pop)

private:
	uint8_t* colormap_offset;
	uint8_t* imagedata_offset;
	
	FILE* file_desc;

public:
	static bool open(const char* filepath, TargaFile* targa);
	bool readRawImageData(uint8_t* buffer);
	bool readIntoRGBA32(uint8_t* buffer);
	void close();
};
