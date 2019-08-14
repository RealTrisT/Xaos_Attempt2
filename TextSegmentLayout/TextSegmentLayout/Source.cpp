#include <cstdio>

#include "TextSegment.h"
#include "EncodingKits.h"

struct fonite : Font{
	uint16_t GetAdvance(uint32_t codepoint, uint16_t size) {
		return 20;
	}
	uint16_t GetLineSpacing(uint16_t size) {
		return 6;
	}
};

int main() {
	wchar_t string[] = L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut volutpat sollicitudin enim, at pulvinar nunc auctor at. Sed fermentum eros ac magna posuere, tincidunt eleifend lectus imperdiet. Mauris elementum, erat vel tincidunt interdum, lectus massa congue lorem, ac maximus orci ex ornare augue. Nulla facilisi. Maecenas scelerisque, quam id lobortis dignissim, felis nunc accumsan eros, sed mollis leo nulla vel nulla. Donec semper ipsum in cursus pretium. Aenean rutrum imperdiet justo, ut mollis turpis consectetur et.";

	fonite fonite;
	TextSegment seg;
	seg.max_pixel_width = 600;	
	seg.encoding = EncodingKits[UTF_16];
	seg.parser = [](PARSER_PARAMS) -> uint32_t {
		*index_state_ptrptr = 0;
		uint32_t codepoint = encoding_kit.iterator(memory_reader);
		return codepoint;
	};

	seg.Set((uint8_t*)string, sizeof(string), &fonite, 16, 0);

	for (int i = 0; i < seg.string.size(); i++){
		if (seg.string[i] == L'\n') {
			seg.string[i] = L'N';
		}
	}

	printf("-------------------------line mappings--------------------------\n");
	for (int i = 0; i < seg.line_mappings.size()-1; i++) {
		TextSegment::LineMapping& lm = seg.line_mappings[i];
		TextSegment::LineMapping& nlm = seg.line_mappings[i+1];
		printf("%.4llu  ->  %.4llu -> %.4llu : \"%*.*S\"\n", lm.pixel_height, lm.text_index, lm.byte_offset, int(nlm.text_index - lm.text_index), int(nlm.text_index - lm.text_index), &seg.string[lm.byte_offset]);
	}

	getchar();
}