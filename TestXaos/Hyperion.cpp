#include "Hyperion.h"
#include "TristGraphics.h"

#include <stdio.h>
#include <intrin.h>

#include <dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")




Hyperion_RGBA::Hyperion_RGBA(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : r(R), g(G), b(B), a(A) {}

Hyperion_RGBA::Hyperion_RGBA(signed C) : rgba(_byteswap_ulong((unsigned)C)) {}

Hyperion_RGBA::Hyperion_RGBA(unsigned char * C) : rgba(*(unsigned*)C) {}

Hyperion_RGBA::operator signed() { return _byteswap_ulong(rgba); }

Hyperion_RGBA::operator unsigned char*() { return (unsigned char*)this; }

Hyperion_RGBA & Hyperion_RGBA::operator=(signed C) { rgba = _byteswap_ulong(C); return *this; }

Hyperion_RGBA & Hyperion_RGBA::operator=(unsigned char * C) { rgba = *(unsigned*)C; return *this; }


struct f4colorTranslatable : public f4color {f4colorTranslatable(Hyperion_RGBA C) : f4color((float(C.r) / 255), (float(C.g) / 255), (float(C.b) / 255), (float(C.a) / 255)) {}};

UD* HyperionUD = 0;
Hyperion_RGBA* HyperionScreen = 0;
PIXEL_VERTEX* screen = 0;
//#define ScrWidth 1920
//#define ScrHeight 1080
unsigned ScrWidth = 1920, ScrHeight = 1080;

void UpdateScreenPixels() {
	unsigned long long totsize = ScrWidth * ScrHeight;
	for (unsigned long long i = 0; i < totsize; i++) {
		screen[i] = f4colorTranslatable(HyperionScreen[i]);
	}
}

void Hyperion_UpdateScreen() {
	UpdateScreenPixels();
	HyperionUD->ShitImage(screen); HyperionUD->PresentFrame();
}

unsigned Hyperion_GetScreenDimensions(unsigned * scrout){ //TODO: fix this whole function
	if (!scrout)return 1; 
	scrout[0] = ScrWidth; 
	scrout[1] = ScrHeight;
	return 0;
}

//----------------------------------

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_PAINT:
		HyperionUD->PresentFrame();
		break;
	case WM_MOUSEMOVE:
		//HyperionUD->ShitImage(screen); HyperionUD->PresentFrame();
		break;
	} return DefWindowProc(hWnd, message, wParam, lParam);
};

bool InitializeHyperion(unsigned char* thismod) {
	HyperionScreen = (Hyperion_RGBA*)calloc(ScrWidth*ScrHeight, sizeof(Hyperion_RGBA));
	screen = (PIXEL_VERTEX*)malloc(ScrWidth*ScrHeight*sizeof(PIXEL_VERTEX));
	
	puts("started initing");

	for (unsigned y = 0; y < ScrHeight; y++) {
		for (unsigned x = 0; x < ScrWidth; x++) {
			unsigned long long index = y * ScrWidth + x;
			screen[index] = { (float)x+1, (float)y+1, 0, {0.f, 0.f, 0.f, 0.f} };
	}	}

	puts("finished initing screen buffer");

	HyperionUD = new UD(ScrWidth, ScrHeight);

	HyperionUD->SetWindowInitCallback([](UD* elUDp) -> void {
		SetLayeredWindowAttributes(elUDp->hWnd, RGB(0, 0, 0), 0xFF, LWA_ALPHA);
		MARGINS margs = { 0, (int)ScrWidth, 0, (int)ScrHeight };
		DwmExtendFrameIntoClientArea(elUDp->hWnd, &margs);
		
		elUDp->InitD3D(); 
	});
	HyperionUD->InitWindow(WindowProc);

	puts("finished initing windowshit");

	HyperionUD->ShitImage(screen); HyperionUD->PresentFrame();
	//Hyperion_UpdateScreen();

	puts("finished shitting image and presenting frame");
	return true;
}

void TerminateHyperion() {
	HyperionUD->TermD3D();
	HyperionUD->TermWindow();
	delete HyperionUD;
	free(screen);
	free(HyperionScreen);
}

extern "C" __declspec(dllexport) Xaos_ModuleInfo ModuleInfo = {
	"----------------------------- HYPERION LOADED -----------------------------",
	"Hyperion: God of Watchfulness, Wisdom and the Light.\nFather of: Helios (the sun), Selene (the moon) and Eos (dawn).\nThis module manages UI.\n",
	0,
	InitializeHyperion,
	TerminateHyperion
};