#include "TracedException.h"

TracedException::TracedException(const char* location, uint32_t error_index, const char** error_strings) 
	: location(location), error_index(error_index), error_strings(error_strings) 
{ trace.push_back(*this); }

TracedException::TracedException(const TracedException& copy) 
	: location(copy.location), error_index(copy.error_index), error_strings(copy.error_strings) 
{}

void TracedException::PrintTrace(FILE* out) {
	for (TracedException& e : trace)
		fprintf(out, "%s in %s\n", e.error_strings[e.error_index], e.location); 
}
void TracedException::ClearTrace() { trace.clear(); }

thread_local std::vector<TracedException> TracedException::trace = std::vector<TracedException>();