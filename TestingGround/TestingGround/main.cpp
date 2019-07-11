#include "MansInterfacin.h"
#include "Hyperion.h"

#include "Targa.h"

#include <dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

#define square(startx, starty, width, height, z) {startx, starty, z},{startx + width, starty, z},{startx, starty + height, z},{startx + width, starty + height, z}

MansInterfacin* elUD = 0;


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_PAINT:
			elUD->graphics_system.PresentFrame();
			puts("painting frame 2");
			break;
		case WM_MOUSEMOVE:
			//HyperionUD->ShitImage(screen); HyperionUD->PresentFrame();
			//elUD->ClearFrame({ 0, 0, 0, 0 });
			//elUD->Draw(vertexes.size());
			//elUD->graphics_system.PresentFrame();
			//printf("painting frame");
			break;
	} return DefWindowProc(hWnd, message, wParam, lParam);
};


int main() {
	elUD = new MansInterfacin(
		1919, 
		1080, 
		[](MansInterfacin::NativeWindowSystem* ws) -> void {
			SetLayeredWindowAttributes(ws->hWnd, RGB(0, 0, 0), 0xFF, LWA_ALPHA);
			MARGINS margs = { 0, (int)ws->width, 0, (int)ws->height };
			DwmExtendFrameIntoClientArea(ws->hWnd, &margs);

			SetWindowLong(ws->hWnd, GWL_EXSTYLE, GetWindowLong(ws->hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		}, 
		[](MansInterfacin::GraphicsSystem* graphcs) -> void {
			elUD->native_window_system.SetWindowProc(WindowProc);
		}
	);


	TargaFile file = {};
	TargaFile::open("../test_images/splash.tga", &file);
	std::vector<uint8_t> splash(4 * file.header.image_specification.width * file.header.image_specification.height);
	file.readIntoRGBA32(splash.data());
	file.close();

	std::vector<f3coord> vertexes = { square(640, 220, 640, 640, 0)	};
	std::vector<colortexel> colortexs = { {{0, 0, 0, 0}, {0.0f, 0.0f}}, {{0, 0, 0, 0}, {1.0f, 0.0f}}, {{0, 0, 0, 0}, {0.0f, 1.0f}}, {{0, 0, 0, 0}, {1.0f, 1.0f}} };

	elUD->Intialize();

	MansInterfacin::UI::VertexBuffer_ID buffers[2] = {
		elUD->ui.CreateVertexBuffer(sizeof(f3coord),	vertexes.size(),	MansInterfacin::UI::ResourceModifyFreq::NEVER, (uint8_t*)vertexes.data() ),
		elUD->ui.CreateVertexBuffer(sizeof(colortexel), colortexs.size(),   MansInterfacin::UI::ResourceModifyFreq::NEVER, (uint8_t*)colortexs.data()),
	};
	elUD->ui.SetBuffers(buffers, 2);

	auto texture = elUD->ui.CreateTexture2D(
		file.header.image_specification.width, 
		file.header.image_specification.height, 
		MansInterfacin::UI::ResourceModifyFreq::ALWAYS, 
		MansInterfacin::UI::Texture2D::TextureFormat::RGBA32,
		splash.data()
	);

	elUD->ui.SetTexture(texture);

	elUD->graphics_system.SetNullStencilState();
	elUD->graphics_system.SetRenderTextureState();
	elUD->graphics_system.Draw(4);

	
	
	WindowSystem::Window window = {};
	
	WindowSystem::WindowInfo wi = {};
	wi.pos_x = 100; wi.pos_y = 10;
	wi.width = 1000; wi.height = 480;
	
	wi.background.background_is_texture = false;
	wi.background.color_info.color[0] =
	wi.background.color_info.color[1] =
	wi.background.color_info.color[2] =
	wi.background.color_info.color[3] =
		{0.3, 0.8, 0.3, 0.8};

	wi.has_borders = true;
	wi.borders.left   =
	wi.borders.top    =
	wi.borders.right  =
	wi.borders.bottom =
		1;

	wi.borders.is_texture = false;
	wi.borders.color_info.color[0] =
	wi.borders.color_info.color[1] =
	wi.borders.color_info.color[2] =
	wi.borders.color_info.color[3] =
		{0, 0, 1, 1};


	WindowSystem::Window::Create(&window, &wi, elUD);
	window.Draw(elUD);




	TargaFile font = {};
	if (!TargaFile::open("../test_images/consolas_ascii.tga", &font)) { printf("REEEEEEEEEEEEEe\n"); return 0; }
	std::vector<uint8_t> font_buffer = std::vector<uint8_t>(font.header.image_specification.width * font.header.image_specification.height * 4);
	font.readIntoRGBA32(font_buffer.data());
	auto font_texture = elUD->ui.CreateTexture2D(font.header.image_specification.width, font.header.image_specification.height, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, MansInterfacin::UI::Texture2D::TextureFormat::RGBA32, font_buffer.data());

	WindowSystem::AsciiFont ascii_font = {};
	WindowSystem::AsciiFont::Create(&ascii_font, font_texture, 16, 29);
	WindowSystem::AsciiText ascii_text = {};
	WindowSystem::AsciiText::Create(&ascii_text, &ascii_font, (char*)"hello, this is I, autistic text with tits emoji ( . Y . ), thank you for stopping by!", 10, elUD);
	ascii_text.Draw(elUD);



	elUD->graphics_system.PresentFrame();








	/*elUD->SetRenderColorState();

	elUD->SetSubtractiveStencilState(1);
	elUD->Draw(0, 4);
	elUD->Draw(4, 4);
	elUD->SetPaintingStencilState(1);
	elUD->Draw(8, 4);
	
	elUD->SetSubtractiveStencilState(2);
	elUD->Draw(12, 4);
	elUD->Draw(16, 4);
	elUD->SetPaintingStencilState(2);
	elUD->Draw(20, 4);*/

	



	getchar();

	delete elUD;

	//elUD->~MansInterfacin();
}