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
		{0, 0, 1, 1};


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

	for (uint16_t i = 0; i < ttf.offs_subt.num_tables; i++) {
		printf("%4.4s : 0x%X\n", ttf.tables_info[i].tag_c, ttf.tables_info[i].offset);
	}

	printf("GLYF table offset: %X\n", ttf.table_lookup[TrueTypeFontFile::FONT_TABLE_glyf].table_entry->offset);


	//---------------------------------------------------------head

	HeadTable* head = (HeadTable*)ttf.loadTable(TrueTypeFontFile::FONT_TABLE_head);
	head->fix_endian();


	printf("Value of Index To Loc Format: %.4X\n", head->index_to_loc_format);

	//---------------------------------------------------------mapx

	MaxpTable* maxp = (MaxpTable*)ttf.loadTable(TrueTypeFontFile::FONT_TABLE_maxp);
	maxp->fix_endian();

	//---------------------------------------------------------cmap
	
	CmapTable* cmap = (CmapTable*)ttf.loadTable(TrueTypeFontFile::FONT_TABLE_cmap);
	cmap->fix_endian();

	printf("CMAP: %X\n", cmap);
	CmapSubtable* unicode_subtable = 0;

	for (uint16_t i = 0; i < cmap->num_subtables; i++) {
		cmap->entries[i].fix_endian();
		if (cmap->entries[i].platform_id == CmapEncodingSubtableEntry::PlatformIDs::UNICODE && cmap->entries[i].platform_specific_id >= CmapEncodingSubtableEntry::UnicodeSpecificIDs::UNICODE_2_0_BMPONLY)
			unicode_subtable = (CmapSubtable*)(((uint8_t*)cmap) + cmap->entries[i].offset);
		printf("CMAP Table: %u -> %u, with offset 0x%.8X\n", cmap->entries[i].platform_id, cmap->entries[i].platform_specific_id, cmap->entries[i].offset);
	}

	printf("UNICODE_SUBTABLE: %X\n", unicode_subtable);

	unicode_subtable->fix_endian();
	printf("unicode subtable version: %u; length: 0x%X\n", unicode_subtable->version, unicode_subtable->length);

	CmapFormat4* unicode_subtable_format4 = (CmapFormat4*)unicode_subtable;
	unicode_subtable_format4->fix_endian();

	puts("RANGES: ");

	uint16_t ranges = unicode_subtable_format4->seg_count_x2 / 2;
	for (uint16_t i = 0; i < ranges; i++) {
		uint16_t* endcodes = unicode_subtable_format4->end_codes;
		uint16_t* startcodes = &endcodes[ranges + 1];
		printf("%.4X-%.4X%c", startcodes[i], endcodes[i], i % 2 ? '\n' : ' ');
	}

	uint16_t rusha = unicode_subtable_format4->GetCheeseGlyphIndex(0x416);  // <----------------------
		
	printf("0x416: the russian thang is glyph index 0x%X or %dd\n", rusha, rusha);

	//---------------------------------------------------------loca

	LocaEntryLong* loca = (LocaEntryLong*)ttf.loadTable(TrueTypeFontFile::FONT_TABLE_loca);
	for (uint16_t i = 0; i < maxp->num_glyphs; i++) { loca[i].fix_endian(); }
		
	for (int i = 0; i < 20; i++) {
		printf("%d : 0x%.8X\n", i, loca[i].offset);
	}

	//---------------------------------------------------------glyf

	uint8_t* glyf_table = (uint8_t*)ttf.loadTable(TrueTypeFontFile::FONT_TABLE_glyf);

	GlyfEntry* glyf = (GlyfEntry*)(glyf_table + loca[rusha].offset);
	glyf->fix_endian();

	GlyfData data_glyf; 

	if (!glyf->getSimpleCoords(data_glyf)) {
		puts("done fucked up");
		getchar(); return 0;
	}

	puts("contours:");
	for (size_t i = 0, si = 0; i < data_glyf.coords.size(); i++) {
		if (si < data_glyf.skips.size() && i == data_glyf.skips[si]) {
			printf(" L ");
			si++;
		} else printf(" | ");
		
		printf("[%c (%d, %d)\n", 
			data_glyf.coords[i].on_curve 
				? '+' 
				: '*', 
			data_glyf.coords[i].x, 
			data_glyf.coords[i].y
		);
		
	}

	printf("glyf height: %d to %d\n", glyf->y_min, glyf->y_max);

	uint32_t width = (glyf->x_max - glyf->x_min);
	uint32_t height = (glyf->y_max - glyf->y_min);

	//array for the totality of the points
	std::vector<f2coord> whole = std::vector<f2coord>(data_glyf.coords.size());
	//index of the contour
	size_t index = 0;

	//for each contour
	for (auto point : data_glyf.coords) {
		whole[index++] = {float((point.x - glyf->x_min)), float((point.y - glyf->y_min))};
	}

	std::vector<uint8_t> endresult(width* height);
	Nozero(whole, data_glyf.skips, endresult.data(), width, height);
	
	auto tex = elUD->ui.Create2DTexture(width, height, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, MansInterfacin::UI::Texture2D::TextureFormat::A8, (uint8_t*)endresult.data());

	vertexes[0].x += 640;	vertexes[1].x += 640;	vertexes[2].x += 640;	vertexes[3].x += 640;
	colortexs[0] = { {1, 0, 0, 1}, { 0.0, 1.0 } }; colortexs[1] = { {0, 1, 0, 1}, { 1.0, 1.0 } }; colortexs[2] = { {0, 0, 1, 1}, { 0.0, 0.0 } }; colortexs[3] = { {1, 1, 0, 1}, { 1.0, 0.0 } };

	
	elUD->ui.UpdateVertexBuffer(buffers[0], (uint8_t*)vertexes.data());
	elUD->ui.UpdateVertexBuffer(buffers[1], (uint8_t*)colortexs.data());

	elUD->ui.SetVertexBuffers(buffers, 2);
	elUD->ui.Set2DTexture(tex);

	elUD->graphics_system.SetRenderFontState();
	elUD->graphics_system.Draw(4);
	elUD->graphics_system.PresentFrame();
	

	ttf.close();

	getchar();

	delete elUD;

	//elUD->~MansInterfacin();
}




