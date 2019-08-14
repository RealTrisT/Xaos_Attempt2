#pragma once
#include <cstdint>
#include <stdexcept>

template<class T>
struct MemoryReader {

	template<class Y>MemoryReader(Y* begin, Y* end) : begin((T*)begin), end((T*)end), position((T*)begin) {}
private:
	MemoryReader(T* begin, T* end, T* position) : begin(begin), end(end), position(position) {}
public:

	MemoryReader operator++(int) {
		if (this->position == this->end)throw std::out_of_range("trying to increment pointer past valid range");
		return MemoryReader(this->begin, this->end, this->position++);
	}
	template<class Y> MemoryReader& operator+=(Y val) {//TOOD: fix
		position += val;
		return *this;
	}
	T operator*() { return *position; }

	T* begin;
	T* position;
	T* end;
};