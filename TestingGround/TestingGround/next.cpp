#include "Hyperion.h"

#include <dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

#include <stdexcept>

#include "Cmd.h"
struct CMD {
	CMD(MansInterfacin* g, WindowSystem::Text::Font* font, unsigned x, unsigned y, unsigned width, unsigned height) : 
		g(g), font(font), x(x), y(y), width(width), height(height),
		p(RedirectedStandardIOProcess("C:\\Windows\\System32\\cmd.exe")),
		line_numbers(WindowSystem::Text::RenderableText(font, sizeof("StdOut     StdErr"))),
		tab_names(WindowSystem::Text::RenderableText(font, 1024))
	{
		outputs = new WindowSystem::Text::RenderableText[2]{
			WindowSystem::Text::RenderableText(font, 4096),
			WindowSystem::Text::RenderableText(font, 4096),
		};
		inputs = new WindowSystem::Text::RenderableText[2]{
			WindowSystem::Text::RenderableText(font, 4096),
			WindowSystem::Text::RenderableText(font, 4096),
		};
		vertexes = std::vector<f3coord>(4 * 7); //7 squares, 2 for input and output background, one for borders, 2 for scroll bars, 2 for separator between tab sizes
		colors = std::vector<colortexel>(4 * 7) = {
			{{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}},
			{{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}}, {{0.561, 0.212, 0.537, 0.4}, {0, 0}},
			{{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}},

			{{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}},
			{{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}},

			{{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}},
			{{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}}, {{1, 1, 1, 1}, {0, 0}},
		}; *((f4color*)& colors[0].t) = { 1, 1, 1, 1 };
		indexes = std::vector<uint32_t>(4 * 7) = {
			0, 1, 2, 2, 1, 3,
			4, 5, 6, 6, 5, 7,
			8, 9, 10, 10, 9, 11,
		};

		vbuffers[0] = g->ui.CreateVertexBuffer(sizeof(f3coord),		vertexes.size(), MansInterfacin::UI::ResourceModifyFreq::SOMETIMES);
		vbuffers[1] = g->ui.CreateVertexBuffer(sizeof(colortexel),	colors.size(),   MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)colors.data());
		index_buffer = g->ui.CreateIndexBuffer(sizeof(uint32_t),	indexes.size(),  MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)indexes.data());

		input_height = 16 + 2;
	}

	void InitLayout() {
		f3coord* output_bg = &vertexes[4 * 0];
		f3coord* input_bg = &vertexes[4 * 1];
		f3coord* border_bg = &vertexes[4 * 2];
		unsigned textboxes_width = width - 4;
		unsigned textboxes_height = height - 4;
		unsigned output_height = textboxes_height - 1 - input_height;
		output_bg[0] = { 2.f, 2.f, 0 };					output_bg[1] = { 2.f + textboxes_width, 2.f, 0 };
		output_bg[2] = { 2.f, 2.f + output_height, 0 };	output_bg[3] = { 2.f + textboxes_width, 2.f + output_height, 0 };
		
		float input_y = height - 2 - input_height;
		input_bg[0] = { 2.f, input_y, 0};				input_bg[1] = { 2.f + textboxes_width, input_y, 0 };
		input_bg[2] = { 2.f, float(height - 2), 0 };	input_bg[3] = { 2.f + textboxes_width, float(height - 2), 0 };

		border_bg[0] = { 0, 0, 0 };				border_bg[1] = { float(width), 0, 0 };
		border_bg[2] = { 0, float(height), 0 }; border_bg[3] = { float(width), float(height), 0 };

		g->ui.UpdateVertexBuffer(vbuffers[0], (uint8_t*)vertexes.data());
	}

	void Draw() {
		g->ui.SetIndexBuffer(index_buffer);
		g->ui.SetVertexBuffers(vbuffers, 2);
		g->graphics_system.SetTopology(MansInterfacin::GraphicsSystem::Topologies::TRIANGLE_LIST);
		g->graphics_system.SetSubtractiveStencilState(1);
		g->graphics_system.SetRenderColorState();
		g->graphics_system.SetRenderOffset({ float(x), float(y), 0 });
		g->graphics_system.Draw(12);
		g->graphics_system.SetPaintingStencilState(1);
		g->graphics_system.Draw(12, 6);
		g->graphics_system.ClearStencilBuffer();
	}

	MansInterfacin* g;
	RedirectedStandardIOProcess p;
	WindowSystem::Text::Font* font;
	WindowSystem::Text::RenderableText line_numbers, tab_names;
	WindowSystem::Text::RenderableText* inputs, * outputs;
	std::vector<f3coord> vertexes;
	std::vector<colortexel> colors;
	std::vector<uint32_t> indexes;
	MansInterfacin::UI::VertexBuffer* vbuffers[2];
	MansInterfacin::UI::IndexBuffer* index_buffer;

	unsigned x, y;
	unsigned width, height;
	unsigned input_height;
};




using namespace WindowSystem::Text;

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
	elUD->graphics_system.PresentFrame();

	FT_Library ft;
	if (FT_Init_FreeType(&ft))puts("issa fuqqe");
	FT_Face face;
	if (FT_New_Face(ft, "../test_images/consola.ttf", 0, &face))puts("NIGGGAAAA");
	FT_Set_Pixel_Sizes(face, 0, 16);
	if(FT_Load_Char(face, 0x416, FT_LOAD_RENDER))puts("KILL MYSELF");

	Font fonti(elUD, face);
	RenderableText niga(&fonti, 400);


	for (colortexel& yeet : niga.cnt) { yeet.c = { 1, 1, 1, 1 }; }


	elUD->graphics_system.PresentFrame();

	CMD mygaycmd(elUD, &fonti, 678, 121, 1785-678, 721-121);
	mygaycmd.InitLayout();
	mygaycmd.Draw();


	elUD->graphics_system.SetRenderOffset({ float(mygaycmd.x + 6), float(mygaycmd.y + 19), 0 });
	niga.ReadyString("Console is on the way, yes.", 18, 0);
	niga.Render();
	elUD->graphics_system.SetRenderOffset({ float(mygaycmd.x - 23), float(mygaycmd.y + 19), 0 });
	niga.ReadyString(" 1\n 2\n 3\n 4\n 5\n 6\n 7\n 8\n 9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44", 18, 0);
	for (size_t i = 0; i < niga.cnt.size(); i += 4) {
		float alpha_upper = (float(mygaycmd.height) - niga.coords[i].y) / mygaycmd.height;
		float alpha_lower = (float(mygaycmd.height) - niga.coords[i+2].y) / mygaycmd.height;
		niga.cnt[i+0].c.a = alpha_upper; niga.cnt[i+1].c.a = alpha_upper;
		niga.cnt[i+2].c.a = alpha_lower; niga.cnt[i+3].c.a = alpha_lower;
	}
	niga.font->g->ui.UpdateVertexBuffer(niga.vbuffers[1], (uint8_t*)niga.cnt.data());
	niga.Render();

	elUD->graphics_system.PresentFrame();


	for (unsigned i = 0; i < 12; i++) {
		printf("(%f, %f, %f)\t color (%f, %f, %f, %f)\n",
			mygaycmd.vertexes[i].x, mygaycmd.vertexes[i].y, mygaycmd.vertexes[i].z,
			mygaycmd.colors[i].c.r, mygaycmd.colors[i].c.g, mygaycmd.colors[i].c.b, mygaycmd.colors[i].c.a
		);
	}

	while(getchar() != EOF);

	return 0;
}