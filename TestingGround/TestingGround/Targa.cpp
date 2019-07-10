#include "Targa.h"

#include <vector>

size_t TargaFile::Header::ImageSpecification::totalSize(){
	return width * height;
}

size_t TargaFile::Header::ImageSpecification::bytesPerPixel(){
	return bpp / 8;
}

size_t TargaFile::Header::ImageSpecification::bitsForAlpha(){
	return image_descriptor & 0xF;
}


bool TargaFile::open(const char* filepath, TargaFile* targa) {
	if (fopen_s(&targa->file_desc, filepath, "rb"))return false;
	if (fread(&targa->header, sizeof(TargaFile::Header), 1, targa->file_desc) != 1) {
		fclose(targa->file_desc);
		return false;
	}
	targa->colormap_offset = (uint8_t*)(sizeof(TargaFile::Header) + targa->header.id_length);
	targa->imagedata_offset = targa->colormap_offset + targa->header.colormap_specification.length;
	return true;
}

bool TargaFile::readRawImageData(uint8_t* buffer) {
	if (fseek(this->file_desc, (long)this->imagedata_offset, SEEK_SET))return false;
	size_t area = this->header.image_specification.width * this->header.image_specification.height;
	if (fread(buffer, this->header.image_specification.bpp / 8, area, this->file_desc) != area)return false;
	return true;
}

bool TargaFile::readIntoRGBA32(uint8_t* buffer){
	size_t area = this->header.image_specification.totalSize();
	std::vector<uint8_t> raw(area*this->header.image_specification.bytesPerPixel());
	if(!this->readRawImageData(raw.data()))return false;
	
	switch (this->header.image_specification.bpp) {
		case 32:
			for (size_t i = 0, yi = this->header.image_specification.height - 1; i < area; yi--) {
				for (size_t x = 0; x < this->header.image_specification.width; x++, i++) {
					uint8_t* rawcell = &raw[yi * this->header.image_specification.width*4 + x*4];
					buffer[(i * 4) + 0] = rawcell[2];
					buffer[(i * 4) + 1] = rawcell[1];
					buffer[(i * 4) + 2] = rawcell[0];
					buffer[(i * 4) + 3] = rawcell[3];
				}
			}
			break;
		case 24:
			for (size_t i = 0, t = 0; i < raw.size(); i += 3, t += 4) {															//and for each pixel
				buffer[t + 0] = raw[i + 2];																						//copy it over to the 32-bit
				buffer[t + 1] = raw[i + 1];																						//allocation, and beware of
				buffer[t + 2] = raw[i + 0];																						//the endianness (not sure about this yet)
				buffer[t + 3] = 0xFF;																							//set alpha to 1, since it's got no alpha channel that's all it can be
			}
			break;
		default:
			return false;
	}
	return true;
}

void TargaFile::close() {
	fclose(this->file_desc);
}