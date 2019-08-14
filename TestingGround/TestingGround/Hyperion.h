#pragma once

#include "MansInterfacin.h"
#include "TracedException.h"

#include <cstdint>
#include <vector>
#include <list>


#include <map>
#include "Datatypes.h"
#include "FreeType/include/ft2build.h"
#include FT_FREETYPE_H
#pragma comment (lib, "FreeType/win32/freetype.lib")

namespace WindowSystem {


	struct WindowInfo {
		bool has_borders;
		struct BorderInfo {
			float left, top, right, bottom;
			bool is_texture;
			union {
				struct {
					MansInterfacin::UI::Texture2D* texture;
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
					MansInterfacin::UI::Texture2D* texture;
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
		MansInterfacin::UI::VertexBuffer* vbuffers[2] = {}; //0 : vertex buffer ; 1 : color and texel buffer

		static void Create(Window* window, WindowInfo* window_info, MansInterfacin* m);

		void Draw(MansInterfacin* m);
	};

	namespace Text {
		struct RenderedGlyph {
			unsigned width;
			unsigned height;
			signed   advance_x;
			signed   advance_y;
			signed	 offs_left;
			signed   offs_top;
			MansInterfacin::UI::Texture2D* texture;
		};

		struct Font {
			Font(MansInterfacin* g, FT_Face face) : g(g), face(face) { codepoint_map = std::map<uint32_t, RenderedGlyph>(); }
			~Font() { for (auto& each : codepoint_map)g->ui.Destroy2DTexture(each.second.texture); }

			FT_Face face;
			MansInterfacin* g;
			std::map<uint32_t, RenderedGlyph> codepoint_map;

			RenderedGlyph* Get(uint16_t code_point, uint16_t size) {
				uint32_t key = (((uint32_t)size) << 16) | code_point;
				auto yeet = codepoint_map.find(key);
				if (yeet == codepoint_map.end()) {
					FT_Set_Pixel_Sizes(face, 0, size);
					if (FT_Load_Char(face, code_point, FT_LOAD_RENDER))return 0;
					auto& val = codepoint_map[key];
					val = {
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows,
						face->glyph->advance.x / 64,
						face->glyph->metrics.vertAdvance / 64,
						face->glyph->bitmap_left,
						face->glyph->bitmap_top,
						face->glyph->bitmap.buffer ?
							g->ui.Create2DTexture(
								face->glyph->bitmap.width,
								face->glyph->bitmap.rows,
								MansInterfacin::UI::ResourceModifyFreq::NEVER,
								MansInterfacin::UI::Texture2D::TextureFormat::A8,
								face->glyph->bitmap.buffer
							) :
						0
					};
					return &val;
				}
				return &yeet->second;
			}
		};


		struct TextSegment {
			TextSegment(void* font);
			~TextSegment();
			void Insert(uint32_t index, const char* text, uint32_t length);
			void Append(const char* text, uint32_t length);
			void Delete(uint32_t index, uint32_t amount);
			void SetWindow(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
			void MoveWindow(uint32_t x, uint32_t y);

			struct SearchEntry {
				uint64_t offset;
				char* line;
			};

			std::vector<SearchEntry> lines;
			std::vector<char> string;

			void* font;
		};


		struct RenderableText {
			RenderableText(Font* font, uint32_t max_size) 
				: font(font), max_size(max_size), coords(std::vector<f3coord>(max_size * 4)), cnt(std::vector<colortexel>(max_size * 4)) 
			{
				textures = std::vector<MansInterfacin::UI::Texture2D*>(max_size);

				vbuffers[0] = font->g->ui.CreateVertexBuffer(sizeof(f3coord), max_size * 4,    MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, 0);
				vbuffers[1] = font->g->ui.CreateVertexBuffer(sizeof(colortexel), max_size * 4, MansInterfacin::UI::ResourceModifyFreq::SOMETIMES, 0);

				if (!vbuffers[0] || !vbuffers[1])throw TracedException("RenderableText::Constructor", 0, 0);
			}
			~RenderableText() {
				font->g->ui.DestroyVertexBuffer(vbuffers[1]);
				font->g->ui.DestroyVertexBuffer(vbuffers[0]);
			}

			uint32_t curr_size = 0;
			uint32_t max_size;
			Font* font;

			std::vector<f3coord> coords;
			std::vector<colortexel> cnt;
			std::vector< MansInterfacin::UI::Texture2D*>textures;
			MansInterfacin::UI::VertexBuffer* vbuffers[2];

			bool ReadyString(const char* string, unsigned fontsize, unsigned width_limit, uint32_t(*runner)(const char** curr_ptr) = [](const char** curr_ptr) -> uint32_t {return *((*curr_ptr)++); }) {
				MansInterfacin* g = font->g;

				auto basecharacter = font->Get('M', fontsize);

				unsigned x = 0, y = basecharacter->height;

				uint32_t index = 0;
				uint32_t current_codepoint = 0;
				while ((current_codepoint = runner(&string)) && index < max_size) {

					if (current_codepoint == '\n') {
						x = 0; 
						y += basecharacter->advance_y; //TODO: very bad, find a way to properly story line size
						continue; 
					}

					auto tex = font->Get(current_codepoint, fontsize);

					if (width_limit && x + tex->width > width_limit) {
						x = 0;
						y += tex->advance_y;
					}

					textures[index] = tex->texture;

					f3coord* these = &coords[index * 4];
					these[0] = { float(x) + tex->offs_left				, float(y) - tex->offs_top					, 0 };
					these[1] = { float(x) + tex->offs_left + tex->width	, float(y) - tex->offs_top					, 0 };
					these[2] = { float(x) + tex->offs_left				, float(y) - tex->offs_top + tex->height	, 0 };
					these[3] = { float(x) + tex->offs_left + tex->width	, float(y) - tex->offs_top + tex->height	, 0 };

					colortexel* theze = &cnt[index * 4];
					theze[0].t = { 0, 0 }; theze[1].t = { 1, 0 }; theze[2].t = { 0, 1 }; theze[3].t = { 1, 1 };

					x += tex->advance_x;
					index++;
				}

				curr_size = index;

				printf("%p, %p, %p\n", vbuffers[0], coords.data(), coords.size());
				g->ui.UpdateVertexBuffer(vbuffers[0], (uint8_t*)coords.data());
				g->ui.UpdateVertexBuffer(vbuffers[1], (uint8_t*)cnt.data());

				return 1;
			}

			void Render() {
				MansInterfacin* g = font->g;
				g->graphics_system.SetTopology(MansInterfacin::GraphicsSystem::Topologies::TRIANGLE_STRIP);
				g->ui.SetIndexBuffer(0);
				g->ui.SetVertexBuffers(vbuffers, 2);
				g->graphics_system.SetRenderFontState();
				for (unsigned i = 0; i < curr_size; i++) {
					g->ui.Set2DTexture(textures[i]);
					g->graphics_system.Draw(i * 4, 4);
				}
			}
		};
	}
};



