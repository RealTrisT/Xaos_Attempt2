///Hyperion: God of Watchfulness, Wisdom and the Light
#pragma once

#include <Windows.h>

#include "../Xaos/Xaos.h"


struct __declspec(dllexport) Hyperion_RGBA {
	Hyperion_RGBA() {}
	Hyperion_RGBA(unsigned char R, unsigned char G, unsigned char B, unsigned char A);
	Hyperion_RGBA(signed C);
	Hyperion_RGBA(unsigned char* C);
	operator signed();
	operator unsigned char*();
	Hyperion_RGBA& operator =(signed C);
	Hyperion_RGBA& operator =(unsigned char* C);

	union {
		struct { unsigned char r, g, b, a; };
		unsigned int rgba;
	};
};

/// <summary>
///     Gets The Screen's Dimensions
/// </summary>
/// <param name="ret">Pointer to array of unsigned ints to receive the multiple dimensions (rn it's only one screen for testing so 2 unsigneds with width and height).</param>
/// <returns>
///     The number of needed uint32s to carry all the dimensions if ret is 0, zero if ret contains a buffer.
/// </returns>
extern "C" __declspec(dllexport) unsigned Hyperion_GetScreenDimensions(unsigned* ret);

extern "C" __declspec(dllexport) void Hyperion_UpdateScreen();

extern "C" __declspec(dllexport) Hyperion_RGBA* HyperionScreen;
