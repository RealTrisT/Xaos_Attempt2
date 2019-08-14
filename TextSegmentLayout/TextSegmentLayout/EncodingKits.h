#pragma once

#include "MemoryReader.hpp"

typedef uint32_t(*EncodingIterator)(MemoryReader<uint8_t>&);
typedef uint64_t(*EncodingStrLength)(MemoryReader<uint8_t>&);

struct EncodingKit {
	EncodingIterator iterator;
	EncodingStrLength strlen;
};

enum Encodings {
	ASCII,
	UTF_16,
	UTF_8,
};

EncodingKit EncodingKits[];
