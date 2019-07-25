#include "MansInterfacin.h"
#include "Hyperion.h"

#include "Targa.h"
#include "TTF.h"

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

	elUD->Intialize();

	MansInterfacin::UI::VertexBuffer* buffers[2] = {
		elUD->ui.CreateVertexBuffer(sizeof(f3coord),	vertexes.size(),	MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)vertexes.data() ),
		elUD->ui.CreateVertexBuffer(sizeof(colortexel), colortexs.size(),   MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)colortexs.data()),
	};
	elUD->ui.SetVertexBuffers(buffers, 2);

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
	elUD->graphics_system.Draw(4);

	elUD->graphics_system.PresentFrame();

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
	/////////////////////////////////////////////////////////////////////////TESTING FONT////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	TargaFile font = {};
	if (!TargaFile::open("../test_images/consolas_ascii_alpha.tga", &font)) { printf("REEEEEEEEEEEEEe\n"); return 0; }
	std::vector<uint8_t> font_buffer = std::vector<uint8_t>(font.header.image_specification.width * font.header.image_specification.height * 4);
	font.readIntoRGBA32(font_buffer.data());
	auto font_texture = elUD->ui.Create2DTexture(font.header.image_specification.width, font.header.image_specification.height, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, MansInterfacin::UI::Texture2D::TextureFormat::RGBA32, font_buffer.data());

	WindowSystem::AsciiFont ascii_font = {};
	WindowSystem::AsciiFont::Create(&ascii_font, font_texture, 16, 29);
	WindowSystem::AsciiText ascii_text = {};
	WindowSystem::AsciiText::Create(&ascii_text, &ascii_font, (char*)"hello, this is I, autistic text with tits emoji ( . Y . ), thank you for stopping by!", 8, elUD);
	ascii_text.Draw(elUD);


	elUD->graphics_system.PresentFrame();
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////TESTING TTF/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	TrueTypeFontFile ttf = {};
	if (!TrueTypeFontFile::Open(&ttf, "../test_images/consola.ttf")) { puts("FUCK"); getchar(); return 0; }	

	FontTTF ttffont = {};
	FontTTF::Init(&ttffont, &ttf);



	unsigned string_len = 0;
	char lestring[1000] = { 0 };
	fgets(lestring, 1000, stdin);
	string_len = strlen(lestring)-1;


	std::vector<FontTTF::RenderedGlyph*> glyph_info(string_len);
	std::vector<MansInterfacin::UI::Texture2D*> tex(string_len);
	for (unsigned i = 0; i < string_len; i++) {
		glyph_info[i] = ttffont.GetTexture(ttffont.UnicodeGlyphLookup(lestring[i]), 28, 2);
		
		printf("glyph info: %p -> {%d, %d, %p}\n", glyph_info[i], glyph_info[i]->width, glyph_info[i]->height, glyph_info[i]->texture);
		tex[i] = elUD->ui.Create2DTexture(
			glyph_info[i]->width,
			glyph_info[i]->height,
			MansInterfacin::UI::ResourceModifyFreq::SOMETIMES,
			MansInterfacin::UI::Texture2D::TextureFormat::A8,
			glyph_info[i]->texture
		);
	}

//___________________________________________________________________________________________________________________________

	float base_x = 1100.f, base_y = 200.f;
	std::vector<f3coord> font_positions(string_len * 4);
	std::vector<colortexel> font_cts(string_len * 4);
	
	for (unsigned i = 0, p = 0; i < string_len; i++, p += 4) {
		font_positions[p + 0] = { base_x + glyph_info[i]->offset_x,							base_y + glyph_info[i]->offset_y,							0 };
		font_positions[p + 1] = { base_x + glyph_info[i]->offset_x + glyph_info[i]->width,	base_y + glyph_info[i]->offset_y,							0 };
		font_positions[p + 2] = { base_x + glyph_info[i]->offset_x,							base_y + glyph_info[i]->offset_y + glyph_info[i]->height,	0 };
		font_positions[p + 3] = { base_x + glyph_info[i]->offset_x + glyph_info[i]->width,	base_y + glyph_info[i]->offset_y + glyph_info[i]->height,	0 };

		font_cts[p + 0] = { {0.8, 0.8, 0.8, 1}, { 0.0, 0.0 } };
		font_cts[p + 1] = { {0.8, 0.8, 0.8, 1}, { 1.0, 0.0 } };
		font_cts[p + 2] = { {0.8, 0.8, 0.8, 1}, { 0.0, 1.0 } };
		font_cts[p + 3] = { {0.8, 0.8, 0.8, 1}, { 1.0, 1.0 } };
	};


	MansInterfacin::UI::VertexBuffer* font_buffers[2] = {
		elUD->ui.CreateVertexBuffer(sizeof(f3coord),	font_positions.size(), MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)font_positions.data()),
		elUD->ui.CreateVertexBuffer(sizeof(colortexel), font_cts      .size(), MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)font_cts      .data())
	};

	elUD->graphics_system.SetRenderFontState();

	elUD->ui.SetVertexBuffers(font_buffers, 2);


	float forward = 0;
	for (unsigned i = 0, offs = 0; i < string_len; forward += 13 + 1, i++, offs += 4) {
		elUD->ui.Set2DTexture(tex[i]);
		elUD->graphics_system.SetRenderOffset({ forward, 0, 0 });
		elUD->graphics_system.Draw(offs, 4);
	}
	

	elUD->graphics_system.PresentFrame();
	
	//ttffont.GetTexture(ttffont.UnicodeGlyphLookup('|'), 16);

	ttf.close();

	puts("\n\n\n\n");

	getchar();

	delete elUD;

	//elUD->~MansInterfacin();
}




