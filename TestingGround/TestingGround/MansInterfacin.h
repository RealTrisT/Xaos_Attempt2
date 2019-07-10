#pragma once

#include <Windows.h>
#include <vector>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

struct f4color { float r, g, b, a; };

struct f2coord { float x, y; };

__declspec(align(16)) struct colortexel {
	f4color c; f2coord t;
};

__declspec(align(16)) struct f3coord { float x, y, z; };

class MansInterfacin {		//just imagine big shak saying it, then it'll make sense, you can even add his "boom" at the end of it in your head for effect 
public:

	class NativeWindowSystem;
	class GraphicsSystem;
	class UI;
	


	class NativeWindowSystem {
	public:
		NativeWindowSystem(unsigned width, unsigned height, void(*Callback_initedWindow)(NativeWindowSystem*) = 0);

		unsigned const int width, height;
		HWND hWnd = 0;

	private:
		bool windInited = false;

		void	(*callback_initedWindow)(NativeWindowSystem*)									= 0;									//callback for after the window is initiated
		LRESULT(CALLBACK* WindowProc_p)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)	= DefaultWindowProc;					//window procedure

		static LRESULT CALLBACK DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);								//answers quit message

		HANDLE windowThread = INVALID_HANDLE_VALUE;																						//thread for window creation and message loop
		static DWORD WINAPI windowInitializationAndRunningThreadFunc(NativeWindowSystem*);												//actually inits the window, calls callback, and starts message loop (thread entry point)

	public:
		void InitWindow(LRESULT(CALLBACK* WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0);						//initializes the window with optional windowProc parameter
		void TermWindow();																												//this ends it, obviously lol u fucking idiot

		void SetWindowProc(LRESULT(CALLBACK* WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam));						//this sets an alternative window procedure (if the window is already initiated it will use SetWindowLongPtr)
		void SetWindowInitCallback(void(*callback_initedWindow_a)(NativeWindowSystem*));												//this sets the callback for after the window is initiated
	}native_window_system;

	class GraphicsSystem { friend class UI; friend int main();
	public:
		GraphicsSystem(NativeWindowSystem* native_window_system, void(*Callback_initedGraphics)(GraphicsSystem*) = 0);

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

	private:
		NativeWindowSystem* WindowSystem;
		void(*callback_initedgraphics)(GraphicsSystem*);

		IDXGISwapChain* swapchain			= 0;		// the pointer to the swap chain interface
		ID3D11Device* device				= 0;		// the pointer to our Direct3D device interface
		ID3D11DeviceContext* context		= 0;		// the pointer to our Direct3D device context
		ID3D11RenderTargetView* backbuffer	= 0;		// the pointer to our backbuffer as a target view

		static const char* shader_color;
		static const char* shader_texture;
	}graphics_system;


	class UI {
	public:
		enum struct ResourceModifyFreq {
			NEVER,		//immutable resourse
			SOMETIMES,	//default -> will be written to sometimes
			ALWAYS		//dynamic -> will be written to every frame or more often even
		};

		struct Texture2D { friend class UI;
			enum struct TextureFormat{RGBA32};
		private:
			unsigned width, height;
			ResourceModifyFreq modfreq;
			TextureFormat		format;

			ID3D11Texture2D* d3d_obj;
			ID3D11ShaderResourceView* d3d_obj_resourceview;
		};
		typedef size_t Texture2D_ID;

		struct VertexBuffer {
			friend class UI;
			unsigned element_size;
			unsigned buffer_size;
			ID3D11Buffer* d3d_obj;
			ResourceModifyFreq modfreq;
		};
		typedef size_t VertexBuffer_ID;

		UI(GraphicsSystem* graphics_system);

		Texture2D_ID CreateTexture2D(unsigned width, unsigned height, ResourceModifyFreq modfreq, Texture2D::TextureFormat format, uint8_t* init_data = 0);
		bool UpdateTexture2D(Texture2D_ID textureID, uint8_t* data);
		void SetTexture(Texture2D_ID textureID);

		Texture2D_ID CreateVertexBuffer(unsigned element_size, unsigned buffer_size, ResourceModifyFreq modfreq, uint8_t* int_data = 0);
		bool UpdateVertexBuffer(Texture2D_ID bufferID, uint8_t* data);
		void SetBuffers(Texture2D_ID* bufferIDs, uint32_t amount);


		GraphicsSystem* graphics_system;
		std::vector<Texture2D> textures = std::vector<Texture2D>();
		std::vector<VertexBuffer> buffers = std::vector<VertexBuffer>();
	}ui;
public:

	MansInterfacin(unsigned int Width, unsigned int Height, void(*Callback_initedWindow)(NativeWindowSystem*) = 0, void(*callback_initedGraphics)(GraphicsSystem*) = 0);
	bool Intialize();
	void Terminate();
};