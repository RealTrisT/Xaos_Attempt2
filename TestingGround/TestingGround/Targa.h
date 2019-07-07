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
		}image_specification;
	}header;
	#pragma pack(pop)

private:
	uint8_t* colormap_offset;
	uint8_t* imagedata_offset;
	
	FILE* file_desc;

public:

	static bool open(const char* filepath, TargaFile* targa);
	bool readImageData(uint8_t* buffer);
	void close();
};

bool TargaFile::open(const char* filepath, TargaFile* targa) {
	if (fopen_s(&targa->file_desc, filepath, "rb"))return false;
	if (fread(&targa->header, sizeof(TargaFile::Header), 1, targa->file_desc) != 1) {
		fclose(targa->file_desc);
		return false;
	}
	targa->colormap_offset = (uint8_t*)(sizeof(TargaFile::Header) + targa->header.id_length);
	targa->imagedata_offset = targa->colormap_offset + targa->header.colormap_specification.length;
}

bool TargaFile::readImageData(uint8_t* buffer) {
	if (fseek(this->file_desc, (long)this->imagedata_offset, SEEK_SET))return false;
	size_t area = this->header.image_specification.width * this->header.image_specification.height;
	if (fread(buffer, this->header.image_specification.bpp/8, area, this->file_desc) != area)return false;
}

void TargaFile::close() {
	fclose(this->file_desc);
}