#pragma once

#include <Windows.h>
#include <vector>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

struct f4color { float r, g, b, a; };

struct f2coord { float x, y; };

__declspec(align(16)) struct f3coord { float x, y, z; };

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



	IDXGISwapChain* swapchain;				// the pointer to the swap chain interface
	ID3D11Device* device;					// the pointer to our Direct3D device interface
	ID3D11DeviceContext* context;			// the pointer to our Direct3D device context
	ID3D11RenderTargetView* backbuffer;		// the pointer to our backbuffer as a target view

	static const char* shader_color;
	static const char* shader_texture;
											
	void InitD3D();				            // sets up and initializes Direct3D
	void TermD3D();							// closes Direct3D and releases memory

	void UpdateVertexBuffer(const std::vector<f3coord>& vertex_array);
	void UpdateColorsBuffer(const std::vector<f4color>& colors_array);
	void UpdateTexelsBuffer(const std::vector<f2coord>& coords_array);

	void SetRenderColorState();
	void SetRenderTextureState();

	void SetSubtractiveStencilState(uint8_t stencil_level);
	void SetPaintingStencilState(uint8_t stencil_level);
	void SetNullStencilState();
	void ClearStencilBuffer();


	void Draw(UINT vertex_amount) { this->Draw(0, vertex_amount); }
	void Draw(UINT offset, UINT vertex_amount);
	void PresentFrame();
	void ClearFrame(f4color col);
												

public:
	UD(unsigned int Width, unsigned int Height, void(*Callback_initedWindow)(UD*) = 0);
};