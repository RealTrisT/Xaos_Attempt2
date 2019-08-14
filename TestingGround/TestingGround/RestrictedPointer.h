#pragma once

#include "TracedException.h"
#include <stdint.h>

const char* RestrictedPointer_errorlist[] = { "Pointer set attempt out of valid range" };

template<class T>
struct RestrictedPointer {
	RestrictedPointer(T* min, T* max, T* curr = min) : min(min), max(max), curr(curr) {}

	template<class Y>	RestrictedPointer		operator+	(Y o)	{ return RestrictedPointer(this->min, this->max, this->Check(this->curr + o)); }
	template<class Y>	RestrictedPointer&		operator+=	(Y o)	{ this->curr = this->Check(this->curr + o); printf("sum %d -> %d\n", this->curr - o, this->curr); return *this; }
						RestrictedPointer&		operator++	()		{ this->curr = this->Check(this->curr + 1); return *this; }
						RestrictedPointer		operator++	(int)	{ this->Check(this->curr + 1); return RestrictedPointer(this->min, this->max, this->curr++); }
						
	template<class Y>	RestrictedPointer		operator-	(Y o)	{ return RestrictedPointer(this->min, this->max, this->Check(this->curr - o)); }
	template<class Y>	RestrictedPointer&		operator-=	(Y o)	{ this->curr = this->Check(this->curr - o); return *this; }
						RestrictedPointer&		operator--	()		{ this->curr = this->Check(this->curr - 1); return *this; }
						RestrictedPointer		operator--	(int)	{ this->Check(this->curr + 1); return RestrictedPointer(this->min, this->max, this->curr--); }

						T&						operator*	()		{ return *this->curr; }
						T*						operator->	()		{ return this->curr; }

	template<class Y>	RestrictedPointer&		operator=(Y o)		{ return this->curr = this->Check(o); }
	template<class Y>							operator Y*()		{ printf("cast %d\n", this->curr); return (Y*)this->curr; }

	T* Get()			{ return this->curr; }
	T* Set(T* o)		{ return this->curr = this->Check(o); }
	T* Check(T* val)	{ if (!(min <= val && max > val)) throw TracedException("RestrictedPointer::Set", 0, RestrictedPointer_errorlist); return val; }

private:
	T* curr, * min, * max;
};