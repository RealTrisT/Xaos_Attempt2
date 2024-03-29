#include "TTF.h"

void OffsetSubtable::fix_endian() {
	this->trash								= _byteswap_ulong(this->trash);
	this->num_tables						= _byteswap_ushort(this->num_tables);
	this->search_range						= _byteswap_ushort(this->search_range);
	this->query_selectors					= _byteswap_ushort(this->query_selectors);
	this->range_shift						= _byteswap_ushort(this->range_shift);
}

void DirectoryTableEntry::fix_endian() {
	this->checksum							= _byteswap_ulong(this->checksum);
	this->offset							= _byteswap_ulong(this->offset);
	this->length							= _byteswap_ulong(this->length);
}


void HeadTable::fix_endian() {
	this->version.fix_endian();
	this->font_revision.fix_endian();
	this->checksum_adjustment				= _byteswap_ulong(this->checksum_adjustment);
	this->magic								= _byteswap_ulong(this->magic);
	this->flags								= _byteswap_ushort(this->flags);
	this->units_per_EM						= _byteswap_ushort(this->units_per_EM);
	this->date_created						= _byteswap_uint64(this->date_created);
	this->date_modified						= _byteswap_uint64(this->date_modified);
	this->x_min								= _byteswap_ushort(this->x_min);
	this->y_min								= _byteswap_ushort(this->y_min);
	this->x_max								= _byteswap_ushort(this->x_max);
	this->y_max								= _byteswap_ushort(this->y_max);
	this->mac_style							= _byteswap_ushort(this->mac_style);
	this->lowest_rec_ppEM					= _byteswap_ushort(this->lowest_rec_ppEM);
	this->font_direction_hint				= _byteswap_ushort(this->font_direction_hint);
	this->index_to_loc_format				= _byteswap_ushort(this->index_to_loc_format);
	this->glyph_data_format					= _byteswap_ushort(this->glyph_data_format);
}


void GlyfEntry::fix_endian(){
	this->num_contours						= _byteswap_ushort(this->num_contours);
	this->x_min								= _byteswap_ushort(this->x_min);
	this->y_min								= _byteswap_ushort(this->y_min);
	this->x_max								= _byteswap_ushort(this->x_max);
	this->y_max								= _byteswap_ushort(this->y_max);
}

void MaxpTable::fix_endian(){
	*(uint32_t*)& this->version				= _byteswap_ulong(*(uint32_t*)& this->version);
	this->num_glyphs						= _byteswap_ushort(this->num_glyphs);
}

void CmapTable::fix_endian() {
	this->version							= _byteswap_ushort(this->version);
	this->num_subtables						= _byteswap_ushort(this->num_subtables);
}

void CmapEncodingSubtableEntry::fix_endian() {
	*(uint16_t*)&this->platform_id			= _byteswap_ushort((uint16_t)this->platform_id);
	*(uint16_t*)&this->platform_specific_id	= _byteswap_ushort((uint16_t)this->platform_specific_id);
	this->offset							= _byteswap_ulong(this->offset);
}

void CmapSubtable::fix_endian() {
	this->version							= _byteswap_ushort(this->version);
	this->length							= _byteswap_ushort(this->length);
}

void CmapFormat4::fix_endian() {
	this->language							= _byteswap_ushort(this->language);
	this->seg_count_x2						= _byteswap_ushort(this->seg_count_x2);
	this->search_range						= _byteswap_ushort(this->search_range);
	this->entry_selector					= _byteswap_ushort(this->entry_selector);
	this->range_shift						= _byteswap_ushort(this->range_shift);

	//segment count
	uint16_t seg_count = this->seg_count_x2 / 2;
	//pointer we're gonna be using to increment and change all the values
	uint16_t* current_array = this->end_codes;
	//do the end codes
	for (uint16_t i = 0; i < seg_count; i++, current_array++)*current_array = _byteswap_ushort(*current_array);
	//move past the reserved padding
	current_array++;
	//do the start codes
	for (uint16_t i = 0; i < seg_count; i++, current_array++)*current_array = _byteswap_ushort(*current_array);
	//do the id deltas
	for (uint16_t i = 0; i < seg_count; i++, current_array++)*current_array = _byteswap_ushort(*current_array);

	//variable for the amount of entries in glyphIndexArray
	uint16_t glyfarr_s = 0;
	//do the id range offsets
	for (uint16_t i = 0; i < seg_count; i++, current_array++) {
		if (*current_array) {
			*current_array = _byteswap_ushort(*current_array);
			//if it's not zero, then there's an equivalent in the glyph index array, so increment the size
			glyfarr_s++;
		}
	}

	//and now do all the entries in the glyph index array //TODO: fix this, make it switch endianness off the bat
	//for (uint16_t i = 0; i < glyfarr_s; i++, current_array++)*current_array = _byteswap_ushort(*current_array);
}

void HheaTable::fix_endian() {
	this->version.fix_endian();
	this->ascent					= _byteswap_ushort(this->ascent);
	this->descent					= _byteswap_ushort(this->descent);
	this->line_gap					= _byteswap_ushort(this->line_gap);
	this->advance_width_max			= _byteswap_ushort(this->advance_width_max);
	this->min_left_side_bearing		= _byteswap_ushort(this->min_left_side_bearing);
	this->min_right_side_bearing	= _byteswap_ushort(this->min_right_side_bearing);
	this->x_max_extent				= _byteswap_ushort(this->x_max_extent);
	this->caret_slope_rise			= _byteswap_ushort(this->caret_slope_rise);
	this->caret_slope_run			= _byteswap_ushort(this->caret_slope_run);
	this->caret_offset				= _byteswap_ushort(this->caret_offset);
	this->metric_data_format		= _byteswap_ushort(this->metric_data_format);
	this->num_long_hor_metrics		= _byteswap_ushort(this->num_long_hor_metrics);
}

void HmtxEntry::fix_endian() {
	this->advance_width				= _byteswap_ushort(this->advance_width);
	this->left_side_bearing			= _byteswap_ushort(this->left_side_bearing);
}



//--------------------------------------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------GlyfEntry------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  GlyfEntry -> getSimpleCoords

bool GlyfEntry::getSimpleCoords(GlyfContours& vec) {
	enum GlyfFlags {
		//1 -> is on the curve, 0 is not on the curve
		ON_CURVE = 0x01,
		//whether or not the x coordinate for this point is an 8 bit unsigned or 16 bit signed
		X_SHORT = 0x02,
		//whether or not the y coordinate for this point is an 8 bit unsigned or 16 bit signed
		Y_SHORT = 0x04,
		//if this flag repeats - if 1, the next byte after this flag will be the number of repetitions
		REPEAT = 0x08,
		//if X_SHORT is set, this is the sign bit for the coord (1 = positive, 0 = negative)
		//if X_SHORT IS not set, this indicates whether or not this X coord is equal to the last
		X_SIGN_SAME = 0x10,
		//if Y_SHORT is set, this is the sign bit for the coord (1 = positive, 0 = negative)
		//if Y_SHORT IS not set, this indicates whether or not this Y coord is equal to the last
		Y_SIGN_SAME = 0x20,
		//doesn't matter
		OVERLAP_SIMPLE = 0x40,
		//doesn't matter
		reserved = 0x80,
	};

	//if number of contours < 0 it's a compound glyph
	if (this->num_contours < 0)return false;

	//if number of contours is zero then it's blank, prolly like a space or something
	else if (!this->num_contours)return true;



	//pointer to array of end points
	uint16_t* contour_end_points = (uint16_t*)simple_data;

	//length of instructions for this glyph
	uint16_t instruction_len = _byteswap_ushort(contour_end_points[this->num_contours]);

	//pointer to instructions
	uint8_t* instructions = (uint8_t*) &(contour_end_points[this->num_contours + 1]);

	//save instructions
	vec.instructions = std::vector<uint8_t>(instruction_len);
	memcpy(vec.instructions.data(), instructions, instruction_len);

	//pointer to flags (this pointer will be incremented as we parse the flags, and will end pointing to the beginning of the x coords array)
	uint8_t* flags_ptr = &instructions[instruction_len];

	//now we straight up fix the endianness of the whole contour end array
	for (int16_t i = 0; i < this->num_contours; i++)contour_end_points[i] = _byteswap_ushort(contour_end_points[i]);


	//the total dimension is gonna be the full amount of flags, which represent points, plus the number of contours for the repeated one at the end
	uint32_t total_dimension = (contour_end_points[this->num_contours - 1] + 1) + this->num_contours;
	//now we create the vector of vectors
	vec = { std::vector<GlyfContours::GlyfCoords>(total_dimension), std::vector<uint16_t>(this->num_contours) };



	//vector for the flags, the size will be the index of the last contour end point +1
	std::vector<uint8_t> flags(contour_end_points[this->num_contours - 1] + 1);

	//and now, for every flag that we will eventually have
	for (uint16_t i = 0; i < flags.size(); i++) {
		//get a flag from where the pointer points to
		uint8_t current = *(flags_ptr++);
		//and update the vector with it
		flags[i] = current;

		//if it contains the repeat flag
		if (current & REPEAT) {
			//get the amount of repetitions ( which is in the byte next to our current flag )
			uint8_t repetitions = *(flags_ptr++);
			//and fill the buffer with it
			for (uint8_t ri = 0; ri < repetitions; ri++) {
				//at our current index + repetition + 1
				flags[i + ri + 1] = current;
			}
			//and then increase the index by the amount of repititions so we're on a new flag next loop
			i += repetitions;
		}
	}



	//then we create a union of a pointer, since we're gonna have coordinates which are 1 byte, and coordinates which are
	//2 bytes, I felt this was a nice way to correctly increment the pointers without much hassle
	union { uint8_t* point_location_8; int16_t* point_location_16; };
	//so set this pointer to the flags pointer, which after we've parsed the flags, will be pointing to the x coords
	point_location_8 = flags_ptr;

	//and we define:
	//	an x variable (because the coordinates represent movements, not absolute coords
	//	a point index, that will increase through contours without reset so we can know where we are in the flag array
	//	a contour end points index, so we know in which contour we at
	//  a regular index so we know what we're adding to the stuff
	//and we loop through the contours
	GlyfContours::GlyfCoords* first = 0;
	for (uint16_t x = 0, point_index = 0, i = 0, si = 0; point_index < flags.size(); point_index++, i++) {

		//we first of all set the on curve bool, since it's now handy to us (we previously didn't have the vectors created)
		vec.coords[i].on_curve = flags[point_index] & ON_CURVE;

		//and if we have a short x, we read an 8-byte unsigned integer, and multiply it by what SIGN_SAME says the sign is
		if (flags[point_index] & X_SHORT) {
			x += (((flags[point_index] & X_SIGN_SAME) ? 1 : -1) * (*(point_location_8++)));
			vec.coords[i].x = x;
		}
		else {
			//otherwise, if SIGN_SAME is set, then that means this x is equal to the previous, so just set it on the vector
			if (flags[point_index] & X_SIGN_SAME) {
				vec.coords[i].x = x;
			}
			else {
				//otherwise, it's a signed 16-bit value, so just add it up
				x += (int16_t)_byteswap_ushort(*(point_location_16++));
				vec.coords[i].x = x;
			}
		}
		//we do this while increasing the x, since as previously mentioned, the coords are dislocations, not absolute coordinates

		//if the first of the contour isn't set, we set it to the current
		if (!first)first = &vec.coords[i];
		//if we have found the end of a contour
		if (si != this->num_contours && point_index == contour_end_points[si]) {
			//ad the first point to the array to close the shape
			vec.coords[++i].x = first->x;
			//set the si'th skip to the current index, which is the last point in this contour
			vec.skips[si++] = i;
			//unset first, so the first will be set in the next iteration (since this is the 
			//last iteration of the current contour, then it'll be set to the first of the next)
			first = 0;
		};

	}

	//here the same thing applies, but for the Y, and the vectors are already created with on_curve flags set, so we don't need to check those
	for (uint16_t y = 0, point_index = 0, i = 0, si = 0; point_index < flags.size(); point_index++, i++) {

		if (flags[point_index] & Y_SHORT) {
			y += (((flags[point_index] & Y_SIGN_SAME) ? 1 : -1) * (*(point_location_8++)));
			vec.coords[i].y = y;
		}
		else {
			if (flags[point_index] & Y_SIGN_SAME) {
				vec.coords[i].y = y;
			}
			else {
				y += _byteswap_ushort(*(point_location_16++));
				vec.coords[i].y = y;
			}
		}

		//if we've found the end of a contour
		if (si != this->num_contours && point_index == contour_end_points[si]) {
			//add the closing point to it
			vec.coords[++i] = vec.coords[(si) ? vec.skips[si - 1] + 1 : 0];
			//and move on
			si++;
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------Cmap Formats----------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------------------------//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CMAP FORMAT 4 -> GLYPH INDEX
FontTTF::GlyphID CmapFormat4::GetCheeseGlyphIndex(uint32_t cheese_code){
	if (cheese_code >> 16)return 0; //format 4 only supports basic multilingual plane

	uint16_t seg_count = this->seg_count_x2 / 2;
	
	uint16_t* end_codes			=				this->end_codes;
	uint16_t* start_codes		=				&end_codes[seg_count + 1];
	 int16_t* id_delta			= (int16_t*)	&start_codes[seg_count];
	uint16_t* id_range_offss	= (uint16_t*)	&id_delta[seg_count];
	uint16_t* glyph_id_arr		=				&id_range_offss[seg_count];
	
	uint16_t i = 0;
	for (; i < seg_count; i++) {
		if (cheese_code <= end_codes[i]) {
			if (cheese_code >= start_codes[i])	break;
			else								return 0;
		}
	}

	if (!id_range_offss[i]) {
		return cheese_code + id_delta[i];
	}else{
		return _byteswap_ushort(*(&id_range_offss[i] + (cheese_code - start_codes[i]) + id_range_offss[i]/2));
	}
}



//--------------------------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------TrueTypeFontFile-------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------------------------//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRUETYPEFILE -> OPEN

const char* TrueTypeFontFile_errorlist[] = {
	"Failed to open file",
	"Could not read the offset subtable",
	"Does not have the minimum number of required subtables",
	"Failed to read the subtable information",
	"Not all required tables present",
};

TrueTypeFontFile::TrueTypeFontFile(const char* path) {
		uint32_t error_number;

		//number of tables
		uint16_t nt = 0;
		
		//if can't open file, fail. pretty obvious
		if (fopen_s(&this->file, path, "rb")) { error_number = 0; goto fail; }
		
		//could not read the offset subtable
		if (fread(&this->offs_subt, sizeof(OffsetSubtable), 1, this->file) != 1) { error_number = 1; goto fail_afteropen; }

		//reverse endianness, these are big endian by default
		this->offs_subt.fix_endian();

		//save nt in a smaller variable just for convenience
		nt = this->offs_subt.num_tables;

		//did not have all the required subtables, which are 9, as can be read in https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html#Overview
		if (nt < 9) { error_number = 2; goto fail_afteropen; }

		//initialize the vector (basically just using it as a buffer) - this is good because no surprise realocations to mess with our pointers
		this->tables_info = std::vector<DirectoryTableEntry>(this->offs_subt.num_tables);

		//attempt to read the tables' information
		if (fread(this->tables_info.data(), sizeof(DirectoryTableEntry), nt, this->file) != nt) { error_number = 3; goto fail_afteropen; }

		//fix all the endianness stuff, and fill in the lookup table
		for (uint16_t i = 0, current_table_tag = 0; i < nt; i++) {
			//first, fix the endianness
			this->tables_info[i].fix_endian();
			//temp variable for little endian file tag
			uint32_t fixed_tag = _byteswap_ulong(this->tables_info[i].tag_i);
			//since it's in order, look through all the tags until we find the one that isn't smaller than the current one we got
			for (; TagValues[current_table_tag] < fixed_tag; current_table_tag++);
			//now if it's equal, then that means we found our table
			if (TagValues[current_table_tag] == fixed_tag)
				//and that we need to put it's pointer on the lookup table
				this->table_lookup[current_table_tag].table_entry = &this->tables_info[i];
				//otherwise just ignore the table completely since we don't support it (kinda bad but it's my code so I can just make support for it if I wanna)
				//plus, the user can just manually walkthrough the directory table, and then also manually read it into memory. Fuck the user lmao.
		}
		
		if(
			!this->table_lookup[FONT_TABLE_cmap].table_entry || !this->table_lookup[FONT_TABLE_glyf].table_entry || !this->table_lookup[FONT_TABLE_head].table_entry ||
			!this->table_lookup[FONT_TABLE_hhea].table_entry ||	!this->table_lookup[FONT_TABLE_hmtx].table_entry || !this->table_lookup[FONT_TABLE_loca].table_entry ||
			!this->table_lookup[FONT_TABLE_maxp].table_entry || !this->table_lookup[FONT_TABLE_name].table_entry ||	!this->table_lookup[FONT_TABLE_post].table_entry
		) {
			error_number = 4;
			goto fail_afteropen;
		}


		//for now do it
		return;

		fail_afteropen:
			fclose(this->file);
		fail:
			throw TracedException("TrueTypeFontFile::Constructor", error_number, TrueTypeFontFile_errorlist);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRUETYPEFILE -> DESTRUCTOR

TrueTypeFontFile::~TrueTypeFontFile() {
	for (LookupEntry& lu : this->table_lookup)if (lu.table_data)delete[]lu.table_data;
	this->close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRUETYPEFILE -> CLOSE
void TrueTypeFontFile::close() { 
	if (!this->file)return;
	fclose(this->file); 
	this->file = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  TRUETYPEFILE -> LOAD TABLE (or return loaded table)

void* TrueTypeFontFile::loadTable(TableTypes type){
	auto& tl = this->table_lookup[type];
	if (tl.table_data)
		return tl.table_data;
	else if (tl.table_entry) {
		tl.table_data = new char[tl.table_entry->length];
		fseek(this->file, tl.table_entry->offset, SEEK_SET);
		fread(tl.table_data, 1, tl.table_entry->length, this->file);
		return tl.table_data;
	}
	return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------FontTTF----------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  TRUETYPEFILE -> TAG VALUES

const uint32_t TrueTypeFontFile::TagValues[] = {
	0x00000000 /*UNKNOWN*/,
	0x45425343 /*EBSC*/,	0x4F532F32 /*OS/2*/,	0x5A617066 /*Zapf*/,
	0x61636E74 /*acnt*/,	0x616E6B72 /*ankr*/,	0x61766172 /*avar*/,
	0x62646174 /*bdat*/,	0x62686564 /*bhed*/,	0x626C6F63 /*bloc*/,
	0x62736C6E /*bsln*/,	0x636D6170 /*cmap*/,	0x63766172 /*cvar*/,
	0x63767400 /* cvt*/,	0x66647363 /*fdsc*/,	0x66656174 /*feat*/,
	0x666D7478 /*fmtx*/,	0x666F6E64 /*fond*/,	0x6670676D /*fpgm*/,
	0x66766172 /*fvar*/,	0x67617370 /*gasp*/,	0x67636964 /*gcid*/,
	0x676C7966 /*glyf*/,	0x67766172 /*gvar*/,	0x68646D78 /*hdmx*/,
	0x68656164 /*head*/,	0x68686561 /*hhea*/,	0x686D7478 /*hmtx*/,
	0x6A757374 /*just*/,	0x6B65726E /*kern*/,	0x6B657278 /*kerx*/,
	0x6C636172 /*lcar*/,	0x6C6F6361 /*loca*/,	0x6C746167 /*ltag*/,
	0x6D617870 /*maxp*/,	0x6D657461 /*meta*/,	0x6D6F7274 /*mort*/,
	0x6D6F7278 /*morx*/,	0x6E616D65 /*name*/,	0x6F706264 /*opbd*/,
	0x706F7374 /*post*/,	0x70726570 /*prep*/,	0x70726F70 /*prop*/,
	0x73626978 /*sbix*/,	0x7472616B /*trak*/,	0x76686561 /*vhea*/,
	0x766D7478 /*vmtx*/,	0x78726566 /*xref*/,
};

const char* FontTTF_errorlist[] = {
	"Cmap format for unicode unsupported",
	"Failed to init cmap",
};

FontTTF::FontTTF(TrueTypeFontFile* ttff){
	HeadTable* head = (HeadTable*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_head);
	MaxpTable* maxp = (MaxpTable*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_maxp);
	void*     vloca =      (void*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_loca);
	GlyfEntry* glyf = (GlyfEntry*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_glyf);
	CmapTable* cmap = (CmapTable*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_cmap);
	HheaTable* hhea = (HheaTable*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_hhea);
	HmtxEntry* hmtx = (HmtxEntry*)ttff->loadTable(TrueTypeFontFile::FONT_TABLE_hmtx);

	head->fix_endian();	maxp->fix_endian(); hhea->fix_endian();

	this->units_per_EM = head->units_per_EM;
	this->line_gap = hhea->line_gap;

	this->x_min = head->x_min; this->y_min = head->y_min; this->x_max = head->x_max; this->y_max = head->y_max;

	try { this->InitCmap(cmap); }
	catch (TracedException& e) { throw TracedException("FontTTF::Constructor", 1, FontTTF_errorlist); }

	if (head->index_to_loc_format)	this->InitGlyphs( (LocaEntryLong*)vloca, glyf, maxp, this->InitIndexes( (LocaEntryLong*)vloca, maxp));
	else							this->InitGlyphs((LocaEntryShort*)vloca, glyf, maxp, this->InitIndexes((LocaEntryShort*)vloca, maxp));

	this->InitHorMetrics(maxp, hhea, head, hmtx);

	return;
}

FontTTF::~FontTTF() {
	if (leftside_bearings)
		delete[] leftside_bearings;
	delete[] adv_widths;
	delete[] glyphs;
	delete[] textures;
	delete[] indexes;
	delete[] unicode_lookup;
}

#pragma region InitializationFunctions

void FontTTF::InitCmap(CmapTable* cmap) {
	cmap->fix_endian();

	for (uint16_t i = 0; i < cmap->num_subtables; i++) {
		cmap->entries[i].fix_endian();
		if (
			cmap->entries[i].platform_id == CmapEncodingSubtableEntry::PlatformIDs::UNICODE
			&&
			cmap->entries[i].platform_specific_id >= CmapEncodingSubtableEntry::UnicodeSpecificIDs::UNICODE_2_0_BMPONLY
		)		{
			{
				CmapSubtable* unicode_subtable = (CmapSubtable*)(((uint8_t*)cmap) + cmap->entries[i].offset); unicode_subtable->fix_endian();
				this->unicode_lookup = (CmapSubtable*)new uint8_t[unicode_subtable->length];
				memcpy(this->unicode_lookup, unicode_subtable, unicode_subtable->length);
			}
			switch (this->unicode_lookup->version) {
				case 4:{
					CmapFormat4 * unicode_subtable_f4 = (CmapFormat4*)this->unicode_lookup;
					unicode_subtable_f4->fix_endian();
					this->lookup_func = (GlyphID(CmapSubtable::*)(uint32_t cheese_code))&CmapFormat4::GetCheeseGlyphIndex;
					break;
				}
				default: {
					delete[] this->unicode_lookup;
					throw TracedException("FontTTF::InitCmap", 0, FontTTF_errorlist);
				}
			}
		}
	}
}

template<class loca_type> 
uint16_t FontTTF::InitIndexes(loca_type* loca, MaxpTable* maxp) {
	this->indexes = new uint16_t[maxp->num_glyphs + 1];
	uint16_t real_number_of_glyphs = 0;

	for (uint16_t i = 0; i < maxp->num_glyphs; i++) { loca[i].fix_endian();
		this->indexes[i] = real_number_of_glyphs;
		if (loca[i].offset != loca[i + 1].offset)real_number_of_glyphs++;
	}this->indexes[maxp->num_glyphs] = real_number_of_glyphs;

	return real_number_of_glyphs;
}

template<class loca_type>
void FontTTF::InitGlyphs(loca_type* loca, GlyfEntry* glyf, MaxpTable* maxp, uint16_t real_number_of_glyphs) {

	this->glyphs   = new GlyphInfo[real_number_of_glyphs];
	this->textures = new std::vector<RenderedGlyphIndexing>[real_number_of_glyphs];

	for (uint16_t i = 0; i < maxp->num_glyphs; i++) {
		if (loca[i].offset == loca[i + 1].offset)continue;

		GlyfEntry* glyf_entry = (GlyfEntry*)(((uint8_t*)glyf) + loca[i].GetOffset()); glyf_entry->fix_endian();

		this->glyphs[this->indexes[i]] = { glyf_entry->x_min, glyf_entry->y_min, glyf_entry->x_max, glyf_entry->y_max, {} };
		glyf_entry->getSimpleCoords(this->glyphs[this->indexes[i]].contours);
	}
}

void FontTTF::InitHorMetrics(MaxpTable* maxp, HheaTable* hhea, HeadTable* head, HmtxEntry* hmtx) {
	this->adv_widths_count = hhea->num_long_hor_metrics;
	this->adv_widths = new uint16_t[this->adv_widths_count];

	if (head->flags & 1) {
		this->leftside_bearings = 0;

		for (uint16_t i = 0; i < this->adv_widths_count; i++) {
			hmtx[i].fix_endian();
			this->adv_widths[i] = hmtx[i].advance_width;
		}
	}
	else {
		this->leftside_bearings = new int16_t[maxp->num_glyphs];

		for (uint16_t i = 0; i < this->adv_widths_count; i++) {
			hmtx[i].fix_endian();
			this->adv_widths[i] = hmtx[i].advance_width;
			this->leftside_bearings[i] = hmtx[i].left_side_bearing;
		}for (uint16_t i = 0, *LSBs = (uint16_t*)& hmtx[this->adv_widths_count]; i < maxp->num_glyphs - this->adv_widths_count; i++) {
			this->leftside_bearings[i] = LSBs[i];
		}

	}
}

#pragma endregion functions called by the constructor


uint32_t FontTTF::UnicodeGlyphLookup(uint32_t cheese_code) {
	return (this->unicode_lookup->*this->lookup_func)(cheese_code);
}

void FontTTF::Term() {
	delete[] this->glyphs;
	delete[] this->unicode_lookup;
}

#include "Bezier.h"
#include "Nozero.h"
//TODO: implement the whole bezier shit
FontTTF::RenderedGlyph* FontTTF::GetTexture(GlyphID glyph_index, float pixels_per_em, uint8_t AA_upscale_multiplier) {
	
	auto& code_point_texture_data = this->textures[glyph_index];
	if (code_point_texture_data.size()) {
		for (RenderedGlyphIndexing& indexed_glyph : code_point_texture_data) {
			if (indexed_glyph.pixels_per_em == pixels_per_em)return indexed_glyph.data;
		}
	}

	GlyphInfo* info = &this->glyphs[this->indexes[glyph_index]];

	RenderedGlyph* render = new RenderedGlyph();

	float units_to_pixels_ratio = pixels_per_em / float(this->units_per_EM);

	float width_in_px = float(info->x_max - info->x_min) * units_to_pixels_ratio;
	float height_in_px = float(info->y_max - info->y_min) * units_to_pixels_ratio;

	render->width  = ceil(width_in_px);
	render->height = ceil(height_in_px);
	render->offset_x = ((this->leftside_bearings)?this->leftside_bearings[glyph_index]:info->x_min) * units_to_pixels_ratio;
	render->offset_y = -float(info->y_max) * units_to_pixels_ratio;  //minus y_max because the font is upside down by default, and when we turn it up, we have to push it down
	render->advance_width = float((glyph_index < this->adv_widths_count) ? this->adv_widths[glyph_index] : this->adv_widths[this->adv_widths_count - 1]) * units_to_pixels_ratio;

	if (!info->contours.coords.size()) {
		render->texture = 0;
		code_point_texture_data.push_back({ pixels_per_em, render });
		return render;
	}
	
	render->texture = new uint8_t[render->width * render->height];

	std::vector<f2coord> contours_f2 = std::vector<f2coord>(info->contours.coords.size());

	for (size_t index = 0; index < contours_f2.size();)	{
		//opoint: original point at start of iteration; point: temp point pointer
		GlyfContours::GlyfCoords* opoint = &info->contours.coords[index], * point = opoint;
		//original contour translated point
		f2coord* oc = &contours_f2[index];
		
		//now we loop, if the current point is a control point, we do the next (which won't 		  flip           fix
		//be) as well, this is so we can then do a bezier using the info of the next point			   v              v
		do contours_f2[index++] = { float(point->x - info->x_min) * units_to_pixels_ratio, (float(-point->y + info->y_max)) * units_to_pixels_ratio }; while (!(point++)->on_curve);
		//now if is in fact a control point, just make it into the halfway point of the quadratic bezier
		if (!opoint->on_curve) *oc = Bezier::Quadratic(oc[-1], oc[0], oc[1], 0.5f);
	}


	std::vector<std::vector<float>> rays(render->height * AA_upscale_multiplier);
	Nozero_Rays(contours_f2, info->contours.skips, rays, render->width, render->height, AA_upscale_multiplier);
	Nozero_AA(rays, render->texture, render->width, render->height, AA_upscale_multiplier);


	code_point_texture_data.push_back({ pixels_per_em, render });

	return render;
}

bool FontTTF::GetRGBA32RenderedGlyphFromUTF8(uint32_t code_point, float pixels_per_em, uint32_t* target, float* offsets){
	return false;
}
