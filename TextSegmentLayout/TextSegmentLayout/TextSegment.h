#pragma once
#include <cstdint>

#include "EncodingKits.h"
#include "MemoryReader.hpp"

struct Font {
	virtual uint16_t GetAdvance(uint32_t codepoint, uint16_t size) = 0;
	virtual uint16_t GetLineSpacing(uint16_t size) = 0;
};

#include <vector>

struct TextSegment {

	//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] TYPE DEFINITIONS

	struct LineMapping {
		uint64_t pixel_height;
		uint64_t text_index;
		uint64_t byte_offset;
	};
	struct SizeChange {
		uint64_t text_index;
		uint16_t size;
	};
	struct FontChange {
		uint64_t text_index;
		Font* font;
	};
	struct ColorChange {
		uint64_t text_index;
		uint32_t color;
	};

	struct IndexAndState { 
		uint64_t text_index;
		enum StateChangeType {SC_SIZE, SC_FONT, SC_COLOR}change_type;
		union {
			uint16_t size;
			Font* font;
			uint32_t color;
		};
	};
	typedef uint32_t(*TextParser)(EncodingKit, MemoryReader<uint8_t>&, IndexAndState**, void*); //if it is character, set IndexAndState to zero and return number, otherwise just set IndexAndState appropriately
	#define PARSER_PARAMS EncodingKit encoding_kit, MemoryReader<uint8_t>& memory_reader, TextSegment::IndexAndState** index_state_ptrptr, void* param


	//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] DATA

	uint32_t max_pixel_width;


	std::vector<uint8_t> string;
	TextParser parser;

	std::vector<LineMapping> line_mappings;

	std::vector<SizeChange> size_changes;
	std::vector<FontChange> font_changes;
	std::vector<ColorChange> color_changes;

	EncodingKit encoding;


	//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] FUNCTIONS

	void Set(uint8_t* string, uint64_t length, Font* first_font, uint16_t first_size, void* param) {
		line_mappings.push_back({ 0, 0 });
		
		MemoryReader<uint8_t> mr(string, string + length);
		
		uint32_t curr_codepoint = 0;
		uint16_t curr_advance = 0;
		IndexAndState ias = {};
		IndexAndState* ias_ptr = &ias;

		uint32_t curr_pixel_width = 0;
		uint64_t curr_pixel_height = 0;

		uint16_t curr_size = first_size;
		Font* curr_font = first_font;
		uint16_t curr_line_largest_size = curr_size;

		uint64_t i = 0;
		for (; ias_ptr = &ias, true; i++) {

			uint8_t* pos_before = mr.position;
			curr_codepoint = parser(encoding, mr, &ias_ptr, param);
			uint8_t* pos_after = mr.position;

			if (ias_ptr) {										//if it's formatting----------------------------------------------------------------------------------
				ias.text_index = i--;												//set index
				switch (ias_ptr->change_type) {										//and for the correct option, add to the correct array
					case IndexAndState::SC_COLOR:
						color_changes.push_back({ ias.text_index, ias.color });
						break;
					case IndexAndState::SC_FONT:
						font_changes.push_back({ ias.text_index, ias.font });
						curr_font = ias.font;
						break;
					case IndexAndState::SC_SIZE:
						size_changes.push_back({ ias.text_index, ias.size });
						curr_size = ias.size;
						if (curr_size > curr_line_largest_size)curr_line_largest_size = curr_size;
						break;
				}
			}else if(curr_codepoint){							//if it's a character---------------------------------------------------------------------------------
				size_t string_oldsize = this->string.size(), change_length = (pos_after - pos_before);
				this->string.resize(string_oldsize + change_length); memcpy(&this->string[string_oldsize], pos_before, change_length);

				bool isNewline = curr_codepoint == '\n';

				curr_pixel_width += curr_advance = (isNewline) ? 0 : curr_font->GetAdvance(curr_codepoint, curr_size);
				if (isNewline || curr_pixel_width > this->max_pixel_width) {
					curr_pixel_width = curr_advance;
					curr_pixel_height += curr_line_largest_size + curr_font->GetLineSpacing(curr_line_largest_size);
					this->line_mappings.push_back({ curr_pixel_height, i + isNewline, isNewline ? this->string.size() : string_oldsize});
				}
			}else break;										//if it's a null terminator---------------------------------------------------------------------------
		}
		this->line_mappings.push_back({ curr_pixel_height + curr_pixel_height + curr_font->GetLineSpacing(curr_line_largest_size), i, this->string.size() });
	}
};