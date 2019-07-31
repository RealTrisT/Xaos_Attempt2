#include "MansInterfacin.h"
#include "Hyperion.h"

#include "Targa.h"
#include "TTF.h"
#include "Bezier.h"

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
			break;
		case WM_CHAR:
			printf("nyggar: %d\n", wParam);
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


	elUD->Intialize();


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////TESTING SPLASH///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TargaFile file = {};
	TargaFile::open("../test_images/splash.tga", &file);
	std::vector<uint8_t> splash(4 * file.header.image_specification.width * file.header.image_specification.height);
	file.readIntoRGBA32(splash.data());
	file.close();

	std::vector<f3coord> vertexes = { square(640, 220, 640, 640, 0)	};
	std::vector<colortexel> colortexs = { {{0, 0, 0, 0}, {0.0f, 0.0f}}, {{0, 0, 0, 0}, {1.0f, 0.0f}}, {{0, 0, 0, 0}, {0.0f, 1.0f}}, {{0, 0, 0, 0}, {1.0f, 1.0f}} };

	MansInterfacin::UI::VertexBuffer* buffers[2] = {
		elUD->ui.CreateVertexBuffer(sizeof(f3coord),	vertexes.size(),	MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)vertexes.data() ),
		elUD->ui.CreateVertexBuffer(sizeof(colortexel), colortexs.size(),   MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)colortexs.data()),
	};elUD->ui.SetVertexBuffers(buffers, 2);

	auto texture = elUD->ui.Create2DTexture(
		file.header.image_specification.width, 
		file.header.image_specification.height, 
		MansInterfacin::UI::ResourceModifyFreq::ALWAYS, 
		MansInterfacin::UI::Texture2D::TextureFormat::RGBA32,
		splash.data()
	);

	elUD->ui.Set2DTexture(texture);
	elUD->graphics_system.SetNullStencilState();
	elUD->graphics_system.SetRenderTextureState();
	elUD->graphics_system.Draw(4);elUD->graphics_system.PresentFrame();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////TESTING WINDOW///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	WindowSystem::Window window = {};
	
	WindowSystem::WindowInfo wi = {};
	wi.pos_x = 2; wi.pos_y = 2;
	wi.width = 1000; wi.height = 480;
	
	wi.background.background_is_texture = false;
	wi.background.color_info.color[0] =
	wi.background.color_info.color[1] =
	wi.background.color_info.color[2] =
	wi.background.color_info.color[3] =
		{0.3f, 0.8f, 0.3f, 0.8f};

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
		{0.8, 0.8, 0.8, 1};


	WindowSystem::Window::Create(&window, &wi, elUD);
	window.Draw(elUD);
	



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////CONSOLE STANDARD INPUT/OUTPUT REDIRECTION/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	HANDLE this_input_piperead = 0, this_input_pipewrite = 0;
	HANDLE this_output_pipewrite = 0, this_output_piperead = 0;

	SECURITY_ATTRIBUTES pipe_attributes = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
	CreatePipe(&this_input_piperead, &this_input_pipewrite, &pipe_attributes, 4096);
	CreatePipe(&this_output_piperead, &this_output_pipewrite, &pipe_attributes, 4096);
	SetHandleInformation(this_input_pipewrite, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(this_output_piperead, HANDLE_FLAG_INHERIT, 0);

	PROCESS_INFORMATION yeet = {};
	STARTUPINFOA yeetinfo = {};
	yeetinfo.cb = sizeof(STARTUPINFOA);	yeetinfo.dwFlags = STARTF_USESTDHANDLES;
	yeetinfo.hStdInput = this_input_piperead;
	yeetinfo.hStdError = yeetinfo.hStdOutput = this_output_pipewrite;

	if (!CreateProcessA("C:\\Windows\\System32\\cmd.exe", (char*)"", 0, 0, true, 0, 0, "C:\\Users\\TrisT\\Desktop", &yeetinfo, &yeet)) {
		puts("failed to create process fuck");
		return 0;
	}
	puts("process started!");

	unsigned char* byg = new unsigned char[4096];
	DWORD read_past = 0;

	DWORD le_avail = 0;
	DWORD readalready = 0;
	puts("peaking nam3d p1p3z");
	while (true) {
		while (PeekNamedPipe(this_output_piperead, NULL, 0, NULL, &le_avail, NULL) && le_avail) {
			printf("%d bytes avaylabel\n", le_avail);
			ReadFile(this_output_piperead, byg + readalready, 4096, &read_past, 0);
			readalready += read_past;
			puts("fyle read");
		}

		printf("bytes read: %d\n", readalready);
		byg[readalready] = '\0';

		puts("output: ------------------------------------------------------------");
		puts((char*)byg);
		puts("--------------------------------------------------------------------");

		char operation = getchar();
		if (operation == EOF)break;
		else if (operation == 'R')
			WriteFile(this_input_pipewrite, "chcp 65001\r\ndir \"C:\\Users\\TrisT\\Desktop\\Delete\\test_filenames\"\r\n"/*u8"ЖblyatД\r\n"*/, 64, &le_avail, 0);
	}

	for (int i = 0; i < readalready;) {


		//https://en.wikipedia.org/wiki/UTF-8#DescriptionS
		uint32_t code_point = byg[i];
		unsigned bytecount = 1;
		if (code_point > 0x7F) {									//if the first bit is set
			for (; (code_point >> (7-bytecount)) & 1; bytecount++);	//get consecutive bits until it's zero, it'll represent the number of bytes for this codepoint
			
			unsigned bits_before_last = 6 * (bytecount - 1);		//save the amount of bits that would be before it
			code_point <<= bytecount + 1;							//shift it to put the bits that aren't part of the code point on the other byte
			code_point &= 0xFF;										//AND it by FF to eliminate those bits
			code_point <<= bits_before_last - (bytecount + 1);		//and finish shifting it to the position it should be at
			
			for (unsigned u = 1; u < bytecount; u++) {				//for each of the following bytes, put their bits in the code point
				code_point |= ((uint32_t)(byg[i + u] & 0x3F)) << (6 * (bytecount - 1 - u));
			}
		}

		printf("|%.2X", code_point);

		if (code_point == '\n')puts(""); 

		i += bytecount;
	}


	getchar();

	elUD->Terminate();
	delete elUD;
}




