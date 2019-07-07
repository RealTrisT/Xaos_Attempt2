#include "TristGraphics.h"

#include <dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

#define square(startx, starty, width, height, z) {startx, starty, z},{startx + width, starty, z},{startx, starty + height, z},{startx + width, starty + height, z}

UD* elUD = 0;

std::vector<f3coord> vertexes = {


	square(491, 476, 668 - 491, 530 - 476, 0),
	square(695, 475, 865 - 695, 532 - 475, 0),

	square(437, 416, 1242 - 437, 867 - 416, 0),

	//-----------------------------------------------

	square(936, 386, 1093 - 936, 441 - 386, 0),
	square(1121, 386, 1275 - 1121, 439 - 386, 0),

	square(847, 332, 1592 - 847, 681 - 332, 0),


	square(855, 450, 200, 200, 0),

};

std::vector<f4color> colors = {

	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},

	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},

	{0.368, 0.423, 0.458, 0.8},	{0.368, 0.423, 0.458, 0.8},	{0.368, 0.423, 0.458, 0.8},	{0.368, 0.423, 0.458, 0.8},



	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},

	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},

	{0.937, 0.894, 0.690, 0.7},	{0.937, 0.894, 0.690, 0.8},	{0.937, 0.894, 0.690, 0.7},	{0.937, 0.894, 0.690, 0.8},


	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},	{0.0, 0.0, 0.0, 0.0},
};

std::vector<f2coord> texels = {

	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},

	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
	{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},

	{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0},
};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_PAINT:
			puts("painting frame 2");
			break;
		case WM_MOUSEMOVE:
			//HyperionUD->ShitImage(screen); HyperionUD->PresentFrame();
			//elUD->ClearFrame({ 0, 0, 0, 0 });
			//elUD->Draw(vertexes.size());
			elUD->PresentFrame();
			//printf("painting frame");
			break;
	} return DefWindowProc(hWnd, message, wParam, lParam);
};


int main() {

	printf("%d\n\n\n", sizeof(f3coord));

	elUD = new UD(1919, 1080);
	elUD->SetWindowInitCallback([](UD* elUDp) -> void {
		SetLayeredWindowAttributes(elUDp->hWnd, RGB(0, 0, 0), 0xFF, LWA_ALPHA);
		MARGINS margs = { 0, (int)elUDp->width, 0, (int)elUDp->height };
		DwmExtendFrameIntoClientArea(elUDp->hWnd, &margs);

		elUDp->InitD3D();
	});
	elUD->InitWindow(WindowProc);

	Sleep(1000);

	elUD->ClearStencilBuffer();
	elUD->SetRenderColorState();

	elUD->UpdateVertexBuffer(vertexes);
	elUD->UpdateColorsBuffer(colors);
	elUD->UpdateTexelsBuffer(texels);

	elUD->SetSubtractiveStencilState(1);
	elUD->Draw(0, 4);
	elUD->Draw(4, 4);
	elUD->SetPaintingStencilState(1);
	elUD->Draw(8, 4);
	
	elUD->SetSubtractiveStencilState(2);
	elUD->Draw(12, 4);
	elUD->Draw(16, 4);
	elUD->SetPaintingStencilState(2);
	elUD->Draw(20, 4);

	elUD->SetRenderTextureState();
	elUD->Draw(24, 4);


	getchar();

	elUD->TermD3D();
	elUD->TermWindow();
}