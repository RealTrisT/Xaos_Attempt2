#pragma once
#include <Windows.h>
#include <cstdint>

	/*
	//https://en.wikipedia.org/wiki/UTF-8#DescriptionS
	uint32_t code_point = byg[i];
	unsigned bytecount = 1;
	if (code_point > 0x7F) {									//if the first bit is set
		for (; (code_point >> (7 - bytecount)) & 1; bytecount++);	//get consecutive bits until it's zero, it'll represent the number of bytes for this codepoint

		unsigned bits_before_last = 6 * (bytecount - 1);		//save the amount of bits that would be before it
		code_point <<= bytecount + 1;							//shift it to put the bits that aren't part of the code point on the other byte
		code_point &= 0xFF;										//AND it by FF to eliminate those bits
		code_point <<= bits_before_last - (bytecount + 1);		//and finish shifting it to the position it should be at

		for (unsigned u = 1; u < bytecount; u++) {				//for each of the following bytes, put their bits in the code point
			code_point |= ((uint32_t)(byg[i + u] & 0x3F)) << (6 * (bytecount - 1 - u));
		}
	}*/


#include <string>
struct CreatedChildProcess {
	CreatedChildProcess(std::string path) : path(path) { startup_info.cb = sizeof(STARTUPINFOA); }

	STARTUPINFOA startup_info;
	PROCESS_INFORMATION process_info;
	std::string path;
	std::string directory = "C:\\";

	virtual bool StartProcess(std::string args = "") {
		return CreateProcessA(this->path.c_str(), (char*)args.c_str(), 0, 0, true, 0, 0, this->directory.c_str(), &this->startup_info, &this->process_info);
	}
};

struct RedirectedStandardIOProcess : public CreatedChildProcess {
	RedirectedStandardIOProcess(std::string path) : CreatedChildProcess(path) {}

	HANDLE standard_input_piperead = 0, standard_input_pipewrite = 0;
	HANDLE standard_output_pipewrite = 0, standard_output_piperead = 0;
	HANDLE standard_error_pipewrite = 0, standard_error_piperead = 0;
	bool CreatePipes(uint32_t max_buffering) {
		SECURITY_ATTRIBUTES pipe_attributes = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
		CreatePipe(&standard_input_piperead, &standard_input_pipewrite, &pipe_attributes, max_buffering);
		CreatePipe(&standard_output_piperead, &standard_output_pipewrite, &pipe_attributes, max_buffering);
		CreatePipe(&standard_error_piperead, &standard_error_pipewrite, &pipe_attributes, max_buffering);
		SetHandleInformation(standard_input_pipewrite, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(standard_output_piperead, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(standard_error_piperead, HANDLE_FLAG_INHERIT, 0);
	}

	bool StartProcess(std::string args = "") {
		this->startup_info.dwFlags	  = STARTF_USESTDHANDLES;
		this->startup_info.hStdInput  = this->standard_input_piperead;	
		this->startup_info.hStdError  = this->standard_error_pipewrite;
		this->startup_info.hStdOutput = this->standard_output_pipewrite;
		return this->CreatedChildProcess::StartProcess(args);
	}


	uint32_t ByteCountInOutput() {
		DWORD available = 0;
		if (!PeekNamedPipe(this->standard_output_piperead, NULL, 0, NULL, &available, NULL))return -1;
		return available;
	}
	uint32_t ReadOutput(const char* buffer, uint32_t max) {
		DWORD amount_read = 0;
		if (!ReadFile(this->standard_output_piperead, (void*)buffer, max, &amount_read, 0))return 0;
		return amount_read;
	}

	uint32_t ByteCountInError() {
		DWORD available = 0;
		if (PeekNamedPipe(this->standard_error_piperead, NULL, 0, NULL, &available, NULL) && available)return available;
		return false;
	}
	uint32_t ReadError(const char* buffer, uint32_t max) {
		DWORD amount_read = 0;
		if (!ReadFile(this->standard_error_piperead, (void*)buffer, max, &amount_read, 0))return 0;
		return amount_read;
	}
	uint32_t WriteToInput(const char* buffer, uint32_t amount) {
		DWORD bytes_written = 0;
		if(!WriteFile(this->standard_input_pipewrite, buffer, amount, &bytes_written, 0))return 0;
		return bytes_written;
	}
};