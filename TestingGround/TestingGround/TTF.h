#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>

#include "Datatypes.h"

//-----------------------------------------------------------------------------------------------------------------------TYPES

//fixed point number with 16 bits for integer and 16 for decimal
typedef struct { uint16_t first, second; } Fixed;
//Font Word, represents font units aka EM - which was orignally used in the press as a measure of printers, and is represented by the width of an upper case M
typedef int16_t FWord;

//-----------------------------------------------------------------------------------------------------------------------Offset subtable

#pragma pack(push, 1) 
//The first table in the TTF file
struct OffsetSubtable {
	//this is documented differently in windows' and apples' specifications.
	//Apple says it's 'scaler type', Windows says it's 'sfntVersion'
	uint32_t trash;
	//Number of tables in the font
	uint16_t num_tables;
	//Spec: the largest power of two less than or equal to the number of items in the table (i.e. the largest number of items that can be easily searched)
	uint16_t search_range;
	//Spec: tells how many iterations of the search loop are needed. (i.e. how many times to cut the range in half)
	uint16_t query_selectors;
	//Spec: the number of items minus searchRange (i.e. the number of items that will not get looked at if you only look at searchRange items)
	uint16_t range_shift;

	//set correct endianness, since ttf fonts are big endian
	void fix_endian();
};

//-----------------------------------------------------------------------------------------------------------------------Directory table

//Directory table
struct DirectoryTableEntry {
	//Name of the table.
	union { char tag_c[4]; uint32_t tag_i; };
	//Table checksum
	uint32_t checksum;
	//Offset from start of file
	uint32_t offset;
	//Unpadded size of the table
	uint32_t length;

	//set correct endianness, since ttf fonts are big endian
	void fix_endian();
};

//-----------------------------------------------------------------------------------------------------------------------Head table

struct HeadTable {
	Fixed	 version;
	Fixed	 font_revision;
	uint32_t checksum_adjustment;
	uint32_t magic;
	uint16_t flags;
	uint16_t units_per_EM;
	int64_t  date_created;
	int64_t  date_modified;
	FWord    x_min;
	FWord    y_min;
	FWord    x_max;
	FWord    y_max;
	uint16_t mac_style;
	uint16_t lowest_rec_ppEM;
	int16_t  font_direction_hint;
	int16_t  index_to_loc_format;
	int16_t  glyph_data_format;

	void fix_endian();
};

//-----------------------------------------------------------------------------------------------------------------------Maxp table

struct MaxpTable {	//TODO: implement version 1.0
	Fixed version;
	uint16_t num_glyphs;

	void fix_endian();
};

//-----------------------------------------------------------------------------------------------------------------------Loca table

struct LocaEntryShort {
	uint16_t offset;
	void fix_endian() { this->offset = _byteswap_ushort(this->offset); }
};

struct LocaEntryLong {
	uint32_t offset;
	void fix_endian() { this->offset = _byteswap_ulong(this->offset); }
};

//-----------------------------------------------------------------------------------------------------------------------Glyf table

struct GlyfData {
	struct GlyfCoords { int16_t x, y; bool on_curve; };
	std::vector<GlyfCoords> coords;
	std::vector<uint16_t> skips;
};

struct GlyfEntry {
	typedef uint8_t VariableGlyfData;	

	int16_t num_contours;
	int16_t x_min;
	int16_t y_min;
	int16_t x_max;
	int16_t y_max;
	VariableGlyfData simple_data[0];

	void fix_endian();
	bool getSimpleCoords(GlyfData& vec);
};

//-----------------------------------------------------------------------------------------------------------------------Cmap table


struct CmapEncodingSubtableEntry {
	enum struct PlatformIDs : uint16_t {
		UNICODE,
		MACINTOSH,
		RESERVED,
		MICROSOFT
	};

	enum struct UnicodeSpecificIDs : uint16_t {
		DEFAULT_SEMANTICS,
		VERSION_1_1_SEMANTICS,
		ISO_1993,
		UNICODE_2_0_BMPONLY,
		UNICODE_2_0_NONBMP_ALLOWED,
		UNICODE_VARIATION_SEQUENCES,
		FULL_UNICODE_COVERAGE
	};

	PlatformIDs platform_id;
	UnicodeSpecificIDs platform_specific_id;
	uint32_t offset;

	void fix_endian();
};

struct CmapTable {
	uint16_t version;
	uint16_t num_subtables;
	CmapEncodingSubtableEntry entries[0];

	void fix_endian();
};


struct CmapSubtable {
	uint16_t version;
	uint16_t length;

	void fix_endian();
};

struct CmapFormat4 : public CmapSubtable {
	uint16_t language;
	uint16_t seg_count_x2;
	uint16_t search_range;
	uint16_t entry_selector;
	uint16_t range_shift;
	uint16_t end_codes[0];

	void fix_endian();

	uint16_t GetCheeseGlyphIndex(uint16_t cheese_code);
};

#pragma pack(pop)

//-----------------------------------------------------------------------------------------------------------------------Nozero

void Nozero(const std::vector<f2coord>& coords, const std::vector<uint16_t>& skips, uint8_t* result, size_t width, size_t height);


//-----------------------------------------------------------------------------------------------------------------------Font file class

struct TrueTypeFontFile {
	enum TableTypes {
		FONT_TABLE_UNKNOWN,
		FONT_TABLE_EBSC, FONT_TABLE_OS_2, FONT_TABLE_Zapf,
		FONT_TABLE_acnt, FONT_TABLE_ankr, FONT_TABLE_avar,
		FONT_TABLE_bdat, FONT_TABLE_bhed, FONT_TABLE_bloc,
		FONT_TABLE_bsln, FONT_TABLE_cmap, FONT_TABLE_cvar,
		FONT_TABLE_cvt,  FONT_TABLE_fdsc, FONT_TABLE_feat,
		FONT_TABLE_fmtx, FONT_TABLE_fond, FONT_TABLE_fpgm,
		FONT_TABLE_fvar, FONT_TABLE_gasp, FONT_TABLE_gcid,
		FONT_TABLE_glyf, FONT_TABLE_gvar, FONT_TABLE_hdmx,
		FONT_TABLE_head, FONT_TABLE_hhea, FONT_TABLE_htmx,
		FONT_TABLE_just, FONT_TABLE_kern, FONT_TABLE_kerx,
		FONT_TABLE_lcar, FONT_TABLE_loca, FONT_TABLE_ltag,
		FONT_TABLE_maxp, FONT_TABLE_meta, FONT_TABLE_mort,
		FONT_TABLE_morx, FONT_TABLE_name, FONT_TABLE_opbd,
		FONT_TABLE_post, FONT_TABLE_prep, FONT_TABLE_prop,
		FONT_TABLE_sbix, FONT_TABLE_trak, FONT_TABLE_vhea,
		FONT_TABLE_vmtx, FONT_TABLE_xref,
	};

	struct LookupEntry { DirectoryTableEntry* table_entry; void* table_data; };


	static bool Open(TrueTypeFontFile* instance, const char* path);
	void close();

	void* loadTable(TableTypes type);




	FILE* file;

	OffsetSubtable offs_subt;
	std::vector<DirectoryTableEntry> tables_info;
	LookupEntry table_lookup[48];

	static const uint32_t TagValues[48];
};