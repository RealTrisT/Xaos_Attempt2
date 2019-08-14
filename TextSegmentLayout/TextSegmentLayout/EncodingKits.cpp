#include "EncodingKits.h"

uint32_t ascii_cpg(MemoryReader<char>& mr) {
	return *mr++;
}

uint64_t ascii_strlen(MemoryReader<char>& mr) {
	uint64_t result = 0;
	while (*mr++)result++;
	return result;
}


uint32_t utf16_cpg(MemoryReader<uint16_t>& mr) {
	return *mr++;
}

uint64_t utf16_strlen(MemoryReader<uint16_t>& mr) {
	uint64_t result = 0;
	while (*mr++)result++;
	return result;
}

uint32_t utf8_cpg(MemoryReader<uint8_t>& mr) {		//TODO: fix this
	//https://en.wikipedia.org/wiki/UTF-8#DescriptionS
	uint32_t code_point = *mr++;
	unsigned bytecount = 1;
	if (code_point > 0x7F) {										//if the first bit is set
		for (; (code_point >> (7 - bytecount)) & 1; bytecount++);	//get consecutive bits until it's zero, it'll represent the number of bytes for this codepoint

		unsigned bits_before_last = 6 * (bytecount - 1);			//save the amount of bits that would be before it
		code_point <<= bytecount + 1;								//shift it to put the bits that aren't part of the code point on the other byte
		code_point &= 0xFF;											//AND it by FF to eliminate those bits
		code_point <<= bits_before_last - (bytecount + 1);			//and finish shifting it to the position it should be at

		for (unsigned u = 1; u < bytecount; u++) {					//for each of the following bytes, put their bits in the code point
			code_point |= ((uint32_t)((*mr++) & 0x3F)) << (6 * (bytecount - 1 - u));
		}
	}
	return code_point;
}

uint64_t utf8_strlen(MemoryReader<uint8_t>& mr) {
	return 0;
}

EncodingKit EncodingKits[] = {
	{(EncodingIterator)ascii_cpg, (EncodingStrLength)ascii_strlen},
	{(EncodingIterator)utf16_cpg, (EncodingStrLength)utf16_strlen},
	{(EncodingIterator)utf8_cpg,  (EncodingStrLength)utf8_strlen }
};