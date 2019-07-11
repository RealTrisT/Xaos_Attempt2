#pragma once

#include "MansInterfacin.h"

#include <vector>
#include <list>

namespace WindowSystem {
	struct AsciiFont {
		static void Create(AsciiFont* instance, MansInterfacin::UI::Texture2D_ID texture, unsigned letter_width, unsigned letter_height) {
			instance->texture = texture; 
			instance->letter_width = letter_width;
			instance->letter_height = letter_height;

			instance->texel_width  = (float)letter_width  / (float)(letter_width*32);
			instance->texel_height = (float)letter_height / (float)(letter_height*3);
		}
		MansInterfacin::UI::Texture2D_ID texture;
		unsigned letter_width, letter_height;

		float texel_width, texel_height;

		f2coord PullLetterCoord(char code_point) {
			code_point -= 32; //because we don't have characters until the 32nd, since those are control characters
			return { this->texel_width * (code_point % 32), this->texel_height * (code_point / 32) };
		}
	};
	struct AsciiText{
		static bool Create(AsciiText* instance, AsciiFont* font, char* text, float font_width, MansInterfacin* m) {
			instance->font = font;
			instance->font_width = font_width;
			instance->font_height = font_width * (float)font->letter_height / (float)font->letter_width;

			size_t string_length = strlen(text); //ASSUMING NO CONTROL CHARACTERS FOR NOW

			instance->ascii_buffer = std::vector<f3coord>(4 * string_length);
			instance->ascii_texture_points = std::vector<colortexel>(4*string_length);

			float current_x = 0, current_y = 0;	//CURRENT Y DOESN'T REALLY MATTER FOR NOW, WERE NOT DOING NEWLINES

			for (size_t i = 0, bffr_i = 0; i < string_length; i++, bffr_i += 4, current_x += +instance->font_width) {
				instance->ascii_buffer[bffr_i + 0] = { current_x,							current_y + instance->font_height,	0 };
				instance->ascii_buffer[bffr_i + 1] = { current_x,							current_y,							0 };
				instance->ascii_buffer[bffr_i + 2] = { current_x + instance->font_width,	current_y + +instance->font_height, 0 };
				instance->ascii_buffer[bffr_i + 3] = { current_x + instance->font_width,	current_y,							0 };

				f2coord l_tl = font->PullLetterCoord(text[i]); //letter_topleft

				instance->ascii_texture_points[bffr_i + 0].t = { l_tl.x,						l_tl.y + font->texel_height };
				instance->ascii_texture_points[bffr_i + 1].t = { l_tl.x,						l_tl.y						};
				instance->ascii_texture_points[bffr_i + 2].t = { l_tl.x + font->texel_width,	l_tl.y + font->texel_height };
				instance->ascii_texture_points[bffr_i + 3].t = { l_tl.x + font->texel_width,	l_tl.y						};
			}

			instance->vbuffers[0] = m->ui.CreateVertexBuffer(sizeof(f3coord),    string_length * 4, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES,	(uint8_t*)instance->ascii_buffer.data()			);
			instance->vbuffers[1] = m->ui.CreateVertexBuffer(sizeof(colortexel), string_length * 4, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES,	(uint8_t*)instance->ascii_texture_points.data()	);
			
			return true;
		}

		void Draw(MansInterfacin* m) {
			m->ui.SetTexture(this->font->texture);
			m->ui.SetBuffers(this->vbuffers, 2);		//TODO: create a buffer list class for making setbuffers faster
			m->graphics_system.SetRenderTextureState();
			m->graphics_system.SetNullStencilState();
			m->graphics_system.Draw(this->ascii_buffer.size());
		}

		AsciiFont* font;

		float font_width, font_height;


		std::vector<f3coord> ascii_buffer;
		std::vector<colortexel> ascii_texture_points;
		MansInterfacin::UI::VertexBuffer_ID vbuffers[2] = {}; //0 : vertex buffer ; 1 : color and texel buffer
	};


	struct WindowInfo {
		bool has_borders;
		struct BorderInfo {
			float left, top, right, bottom;
			bool is_texture;
			union {
				struct {
					MansInterfacin::UI::Texture2D_ID texture;
					f2coord corner_texel_coords[4];
				}texture_info;
				struct {
					f4color color[4];
				}color_info;
			};
		}borders;

		struct BackgroundInfo {
			bool background_is_texture;
			union {
				struct {
					MansInterfacin::UI::Texture2D_ID texture;
					f2coord corner_texel_coords[4];
				}texture_info;
				struct {
					f4color color[4];
				}color_info;
			};
		}background;

		float width, height;
		float pos_x, pos_y;
		
		bool can_resize_horizontal;
		bool can_resize_vertical;
	};

	struct Window {
		Window* next; Window* prev;
		WindowInfo window_info;

		f3coord bg_and_border_vb[8] = {};
		colortexel bg_and_border_ct[8] = {};
		MansInterfacin::UI::VertexBuffer_ID vbuffers[2] = {}; //0 : vertex buffer ; 1 : color and texel buffer

		static void Create(Window* window, WindowInfo* window_info, MansInterfacin* m) {
			memcpy(&window->window_info, window_info, sizeof(WindowInfo));

			//------------------------------------background------------------------------------
			window->bg_and_border_vb[0] = { window_info->pos_x,							window_info->pos_y,							0 };
			window->bg_and_border_vb[1] = { window_info->pos_x + window_info->width,	window_info->pos_y,							0 };
			window->bg_and_border_vb[2] = { window_info->pos_x,							window_info->pos_y + window_info->height,	0 };
			window->bg_and_border_vb[3] = { window_info->pos_x + window_info->width,	window_info->pos_y + window_info->height,	0 };
			
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
				window->bg_and_border_vb[4] = { window_info->pos_x - window_info->borders.left,							window_info->pos_y - window_info->borders.top,							0 };
				window->bg_and_border_vb[5] = { window_info->pos_x + window_info->width + window_info->borders.right,	window_info->pos_y - window_info->borders.top,							0 };
				window->bg_and_border_vb[6] = { window_info->pos_x - window_info->borders.left,							window_info->pos_y + window_info->height + window_info->borders.bottom,	0 };
				window->bg_and_border_vb[7] = { window_info->pos_x + window_info->width + window_info->borders.right,	window_info->pos_y + window_info->height + window_info->borders.bottom,	0 };

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
		}

		void Draw(MansInterfacin* m) {
			m->ui.SetBuffers(vbuffers, 2);
			m->graphics_system.SetSubtractiveStencilState(1);

			if (this->window_info.background.background_is_texture) {
				m->graphics_system.SetRenderTextureState();
				m->ui.SetTexture(this->window_info.background.texture_info.texture);
			}else{
				m->graphics_system.SetRenderColorState();
			}
			
			m->graphics_system.SetSubtractiveStencilState(1);
			m->graphics_system.Draw(0, 4);
			if (window_info.has_borders) {
				m->graphics_system.SetPaintingStencilState(1);
				m->graphics_system.Draw(4, 4);
			}
			m->graphics_system.ClearStencilBuffer();
		}

		/*void move_outer(float x, float y);
		void resize_outer(float x, float y);
		void move_inner(float x, float y);
		void resize_inner(float x, float y);*/
	};

	struct WindowManager {
		Window* window_list_first;
		
		Window* createWindow(WindowInfo* window_info) {
			
		}
	};
};



