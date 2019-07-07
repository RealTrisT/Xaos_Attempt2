#pragma once

#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

struct b3icolor {
	b3icolor() {}
	b3icolor(unsigned char R, unsigned char G, unsigned char B) : r(R), g(G), b(B) {}
	operator unsigned char*() { return (unsigned char*)this; }
	unsigned char b, g, r;
};
struct b3color {
	b3color() {}
	b3color(unsigned char R, unsigned char G, unsigned char B) : r(R), g(G), b(B) {}
	operator unsigned char*() { return (unsigned char*)this; }
	unsigned char r, g, b;
};
struct b4color {
	b4color() {}
	b4color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : r(R), g(G), b(B), a(A) {}
	b4color(unsigned char* C) : rgba(*(unsigned*)C){}
	operator unsigned char*() { return (unsigned char*)this; }
	b4color& operator=(unsigned char* C) { rgba = *(unsigned*)C; return *this; }
	union {
		struct { unsigned char r, g, b, a; };
		unsigned int rgba;
	};
};
struct f3color {
	f3color() {}
	f3color(float R, float G, float B) : r(R), g(G), b(B) {}
	f3color(b3color C) : r(float(C.r)/255), g(float(C.g)/255), b(float(C.b)/255) {}
	f3color(b3icolor C) : r(float(C.r) / 255), g(float(C.g) / 255), b(float(C.b) / 255) {}
	operator float*() { return (float*)this; }
	float r, g, b;
};
struct f4color {
	f4color() {}
	f4color(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {}
	f4color(f3color C, float alpha) : r(C.r), g(C.g), b(C.b), a(alpha) {}
	f4color(b4color C) : r(float(C.r) / 255), g(float(C.g) / 255), b(float(C.b) / 255), a(float(C.a) / 255) {}
	f4color& operator=(b4color C) { return (*this = f4color(C)); }
	operator float*() { return (float*)this; }
	float r, g, b, a;
};
struct PIXEL_VERTEX {
	void operator=(f4color C) { c = C; }
	float x, y, z;
	f4color c;
};

class UD {
public:
	HWND hWnd;
	unsigned int width, height;


	bool windInited = false;
	void	(			*callback_initedWindow	)(UD*)														= DefaultCallbackAfterWindowInit;		//callback for after the window is initiated
	void	(			*callback_initedDirectx	)(UD*)														= 0;									//callback for after directx is initiated
	LRESULT	(CALLBACK	*WindowProc_p			)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)	= WindowProc;							//window procedure

	static void DefaultCallbackAfterWindowInit(UD*);												// calls InitD3D
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);		//answers quit message

	HANDLE windowThread;
	static DWORD WINAPI windowFunc(UD*);				//actually inits the window, calls callback, and starts message loop



	void InitWindow(LRESULT(CALLBACK *WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0);
	void TermWindow();

	void SetWindowProc(LRESULT (CALLBACK *WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam));
	void SetWindowInitCallback(void(*callback_initedWindow_a)(UD*));
	void SetDirectxInitCallback(void(*callback_initedDirectx_a)(UD*));



	IDXGISwapChain *pSwapchain;             // the pointer to the swap chain interface
	ID3D11Device *pDevice;                  // the pointer to our Direct3D device interface
	ID3D11DeviceContext *pCon;              // the pointer to our Direct3D device context
	ID3D11RenderTargetView *backbuffer;		// the pointer to our backbuffer as a target view

	static const char* shader;
											
	void InitD3D();				            // sets up and initializes Direct3D
	void TermD3D();							// closes Direct3D and releases memory

	void ClearFrame(f4color);
	void PresentFrame();
	void ShitImage(b3color*, float);
	void ShitImage(f3color*, float);
	void ShitImage(f4color*);
	void ShitImage(PIXEL_VERTEX*);
												

public:
	UD(unsigned int Width, unsigned int Height, void(*Callback_initedWindow)(UD*) = 0);
};