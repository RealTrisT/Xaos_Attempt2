#include "MansInterfacin.h"

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



	//std::vector<f4color> colors = { {0.0f, 0.0f, 0.0f, 0.0f},	{0.0f, 0.0f, 0.0f, 0.0f},	{0.0f, 0.0f, 0.0f, 0.0f},	{0.0f, 0.0f, 0.0f, 0.0f} };
	//elUD->graphics_system.UpdateColorsBuffer(colors);

	TargaFile file = {};
	TargaFile::open("../test_images/splash_chaotic.tga", &file);
	std::vector<uint8_t> buffer(4 * file.header.image_specification.width * file.header.image_specification.height);
	file.readIntoRGBA32(buffer.data());
	file.close();

	std::vector<f3coord> vertexes = { square(640, 220, 640, 640, 0)	};
	std::vector<f2coord> texels = {	{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} };

	elUD->Intialize();

	elUD->graphics_system.UpdateVertexBuffer(vertexes);
	elUD->graphics_system.UpdateTexelsBuffer(texels);

	auto texture = elUD->ui.createTexture2D(
		file.header.image_specification.width, 
		file.header.image_specification.height, 
		MansInterfacin::UI::ResourceModifyFreq::NEVER, 
		MansInterfacin::UI::Texture2D::TextureFormat::RGBA32,
		buffer.data()
	);

	elUD->ui.SetTexture(texture);

	elUD->graphics_system.SetRenderTextureState();
	elUD->graphics_system.Draw(4);

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