#include "Hyperion.h"

#include <stdexcept>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define __LINESTR__ STR(__LINE__)
#define DEBUG_INFO_STR " - " __FILE__ " : " __LINESTR__

void WindowSystem::Window::Create(Window* window, WindowInfo* window_info, MansInterfacin* m){
	memcpy(&window->window_info, window_info, sizeof(WindowInfo));

	//------------------------------------background------------------------------------
	window->bg_and_border_vb[0] = { 0,						0,							0 };
	window->bg_and_border_vb[1] = { 0 + window_info->width,	0,							0 };
	window->bg_and_border_vb[2] = { 0,						0 + window_info->height,	0 };
	window->bg_and_border_vb[3] = { 0 + window_info->width,	0 + window_info->height,	0 };
			
	if (window_info->background.background_is_texture) {
		window->bg_and_border_ct[0].t = window_info->background.texture_info.corner_texel_coords[0];
		window->bg_and_border_ct[1].t = window_info->background.texture_info.corner_texel_coords[1];
		window->bg_and_border_ct[2].t = window_info->background.texture_info.corner_texel_coords[2];
		window->bg_and_border_ct[3].t = window_info->background.texture_info.corner_texel_coords[3];
	} else {
		window->bg_and_border_ct[0].c = window_info->background.color_info.color[0];
		window->bg_and_border_ct[1].c = window_info->background.color_info.color[1];
		window->bg_and_border_ct[2].c = window_info->background.color_info.color[2];
		window->bg_and_border_ct[3].c = window_info->background.color_info.color[3];
	}


	//------------------------------------borders---------------------------------------
	if (window_info->has_borders) {
		window->bg_and_border_vb[4] = { 0 - window_info->borders.left,							0 - window_info->borders.top,							0 };
		window->bg_and_border_vb[5] = { 0 + window_info->width + window_info->borders.right,	0 - window_info->borders.top,							0 };
		window->bg_and_border_vb[6] = { 0 - window_info->borders.left,							0 + window_info->height + window_info->borders.bottom,	0 };
		window->bg_and_border_vb[7] = { 0 + window_info->width + window_info->borders.right,	0 + window_info->height + window_info->borders.bottom,	0 };

		if (window_info->borders.is_texture) {
			window->bg_and_border_ct[4].t = window_info->borders.texture_info.corner_texel_coords[0];
			window->bg_and_border_ct[5].t = window_info->borders.texture_info.corner_texel_coords[1];
			window->bg_and_border_ct[6].t = window_info->borders.texture_info.corner_texel_coords[2];
			window->bg_and_border_ct[7].t = window_info->borders.texture_info.corner_texel_coords[3];
		} else {
			window->bg_and_border_ct[4].c = window_info->borders.color_info.color[0];
			window->bg_and_border_ct[5].c = window_info->borders.color_info.color[1];
			window->bg_and_border_ct[6].c = window_info->borders.color_info.color[2];
			window->bg_and_border_ct[7].c = window_info->borders.color_info.color[3];
		}
	}
	window->vbuffers[0] = m->ui.CreateVertexBuffer(sizeof(f3coord),		8, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)window->bg_and_border_vb);
	window->vbuffers[1] = m->ui.CreateVertexBuffer(sizeof(colortexel),	8, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, (uint8_t*)window->bg_and_border_ct);
	if (!window->vbuffers[0] || !window->vbuffers[0])throw std::runtime_error("Failed to initiate vertex buffers WindowSystem::Window" DEBUG_INFO_STR);
}

void WindowSystem::Window::Draw(MansInterfacin* m){
	m->graphics_system.SetRenderOffset({ this->window_info.pos_x, this->window_info.pos_y, 0 });
	m->ui.SetVertexBuffers(vbuffers, 2);

	if (this->window_info.background.background_is_texture) {
		m->graphics_system.SetRenderTextureState();
		m->ui.Set2DTexture(this->window_info.background.texture_info.texture);
	}else{
		m->graphics_system.SetRenderColorState();
	}
	m->graphics_system.SetSubtractiveStencilState(1);
	m->graphics_system.Draw(0, 4);
			
			
	if (window_info.has_borders) {
		if (this->window_info.borders.is_texture) {
			m->graphics_system.SetRenderTextureState();
			m->ui.Set2DTexture(this->window_info.borders.texture_info.texture);
		} else {
			m->graphics_system.SetRenderColorState();
		}

		m->graphics_system.SetPaintingStencilState(1);
		m->graphics_system.Draw(4, 4);
	}
	m->graphics_system.ClearStencilBuffer();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////// TextSegment ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


using WindowSystem::Text::TextSegment;

TextSegment::TextSegment(void* fount) : font(font) {
}

TextSegment::~TextSegment(){}

void TextSegment::Insert(uint32_t index, const char* text, uint32_t length) {
	
}

void TextSegment::Append(const char* text, uint32_t length) {
	//if (this->string[this->string.size() - 1] == '\n') {
	//	this->lines.push_back({lines})
	//}
}

void TextSegment::Delete(uint32_t index, uint32_t amount) {

}

void TextSegment::SetWindow(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

}

void TextSegment::MoveWindow(uint32_t x, uint32_t y) {

}




