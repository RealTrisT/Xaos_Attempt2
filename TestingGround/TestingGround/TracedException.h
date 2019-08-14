#pragma once

#include <stdint.h>
#include <vector>

struct TracedException {
	TracedException(const char* location, uint32_t error_index, const char** error_strings);
	TracedException(const TracedException& copy);
	static void PrintTrace(FILE* out = stdout);
	static void ClearTrace();

	const char* location;
	uint32_t error_index;
	const char** error_strings;
	
	static thread_local std::vector<TracedException> trace;
};