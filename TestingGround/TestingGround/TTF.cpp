#include "TTF.h"

void OffsetSubtable::fix_endian() {
	this->trash							= _byteswap_ulong(this->trash);
	this->num_tables					= _byteswap_ushort(this->num_tables);
	this->search_range					= _byteswap_ushort(this->search_range);
	this->query_selectors				= _byteswap_ushort(this->query_selectors);
	this->range_shift					= _byteswap_ushort(this->range_shift);
}

void DirectoryTableEntry::fix_endian() {
	this->checksum						= _byteswap_ulong(this->checksum);
	this->offset						= _byteswap_ulong(this->offset);
	this->length						= _byteswap_ulong(this->length);
}


void HeadTable::fix_endian() {
	*(uint32_t*)&this->version			= _byteswap_ulong(*(uint32_t*)&this->version);
	*(uint32_t*)&this->font_revision	= _byteswap_ulong(*(uint32_t*)&this->font_revision);
	this->checksum_adjustment			= _byteswap_ulong(this->checksum_adjustment);
	this->magic							= _byteswap_ulong(this->magic);
	this->flags							= _byteswap_ushort(this->flags);
	this->units_per_EM					= _byteswap_ushort(this->units_per_EM);
	this->date_created					= _byteswap_uint64(this->date_created);
	this->date_modified					= _byteswap_uint64(this->date_modified);
	this->x_min							= _byteswap_ushort(this->x_min);
	this->y_min							= _byteswap_ushort(this->y_min);
	this->x_max							= _byteswap_ushort(this->x_max);
	this->y_max							= _byteswap_ushort(this->y_max);
	this->mac_style						= _byteswap_ushort(this->mac_style);
	this->lowest_rec_ppEM				= _byteswap_ushort(this->lowest_rec_ppEM);
	this->font_direction_hint			= _byteswap_ushort(this->font_direction_hint);
	this->index_to_loc_format			= _byteswap_ushort(this->index_to_loc_format);
	this->glyph_data_format				= _byteswap_ushort(this->glyph_data_format);
}


void GlyfEntry::fix_endian(){
	this->num_contours					= _byteswap_ushort(this->num_contours);
	this->x_min							= _byteswap_ushort(this->x_min);
	this->y_min							= _byteswap_ushort(this->y_min);
	this->x_max							= _byteswap_ushort(this->x_max);
	this->y_max							= _byteswap_ushort(this->y_max);
}

void MaxpTable::fix_endian(){
	*(uint32_t*)& this->version			= _byteswap_ulong(*(uint32_t*)& this->version);
	this->num_glyphs					= _byteswap_ushort(this->num_glyphs);
}

bool TrueTypeFontFile::Open(TrueTypeFontFile* instance, const char* path) {
		#define this instance
		
		//number of tables
		uint16_t nt = 0;
		
		//if can't open file, fail. pretty obvious
		if (fopen_s(&this->file, path, "rb")) goto fail;
		
		//could not read the offset subtable
		if (fread(&this->offs_subt, sizeof(OffsetSubtable), 1, this->file) != 1) goto fail_afteropen;

		//reverse endianness, these are big endian by default
		this->offs_subt.fix_endian();

		//save nt in a smaller variable just for convenience
		nt = this->offs_subt.num_tables;

		//did not have all the required subtables, which are 9, as can be read in https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html#Overview
		if (nt < 9) goto fail_afteropen;

		//initialize the vector (basically just using it as a buffer) - this is good because no surprise realocations to mess with our pointers
		this->tables_info = std::vector<DirectoryTableEntry>(this->offs_subt.num_tables);

		//attempt to read the tables' information
		if (fread(this->tables_info.data(), sizeof(DirectoryTableEntry), nt, this->file) != nt)goto fail_afteropen;

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
		
		//TODO: fail if not all the required tables are there, but that seems kinda lame rn

		//for now do it
		return true;

		fail_afteropen:
			fclose(this->file);
		fail:
			return false;
		#undef this
}


void TrueTypeFontFile::close() { 
	fclose(this->file); 
}

void* TrueTypeFontFile::loadTable(TableTypes type){  //TODO deallocate the shit allocated here
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
	0x68656164 /*head*/,	0x68686561 /*hhea*/,	0x68746D78 /*htmx*/,
	0x6A757374 /*just*/,	0x6B65726E /*kern*/,	0x6B657278 /*kerx*/,
	0x6C636172 /*lcar*/,	0x6C6F6361 /*loca*/,	0x6C746167 /*ltag*/,
	0x6D617870 /*maxp*/,	0x6D657461 /*meta*/,	0x6D6F7274 /*mort*/,
	0x6D6F7278 /*morx*/,	0x6E616D65 /*name*/,	0x6F706264 /*opbd*/,
	0x706F7374 /*post*/,	0x70726570 /*prep*/,	0x70726F70 /*prop*/,
	0x73626978 /*sbix*/,	0x7472616B /*trak*/,	0x76686561 /*vhea*/,
	0x766D7478 /*vmtx*/,	0x78726566 /*xref*/,
};



void Nozero(const std::vector<f2coord>& coords, const std::vector<uint16_t>& skips, uint8_t (*result)[4], size_t width, size_t height){

	//now for every row
	for (size_t y = 0; y < height; y++) {
		
		//y position, which is going to be the middle of the pixel (which is a square)
		float y_pix_center = float(y) + 0.5f;

		//line segment
		struct line {
			//the 2 points that constitute the line segment
			f2coord p[2];
			//whether or not the line is going down (so if the y of the first point is > than the y of the second)
			bool down;

			//equation data
			struct {
				//whether or not it's vertical, since if that was the case the equation would be undefined
				bool vertical;
				//a union, since we're only gonna be using one of the 2, no need for wasting space
				union {
					//variables of the equation
					struct {
						float m;
						float b;
					};
					//the x in cas it's vertical. This can already be taken from one of the points, but it's not a waste of space either sine it's a union, so just for convenience
					float x;
				};
			}eq;
		};

		//vector for the lines the current y value might intersect with
		std::vector<line> lines;
		
		//loop to fill the vector
		for (size_t i = 0, si = 0; i < coords.size()-1; i++) {

			if (si != skips.size() && i == skips[si]) { si++; continue; }

			//variable for whether or not the segment is going down
			bool down;
			if (coords[i + 0].y <= y_pix_center && coords[i + 1].y >= y_pix_center) {
				//if the current y is between the 2 points of this line and the first point is bellow the second
				down = false;
			} else if (coords[i + 0].y >= y_pix_center && coords[i + 1].y <= y_pix_center) {
				//if the current y is between the 2 points of this line and the fist point is above the second
				down = true;
			} else continue;//if neither, go on to the next segment

			//object to be added into the vector
			line cl = {};
			cl.p[0] = coords[i + 0];
			cl.p[1] = coords[i + 1];
			cl.down = down;

			if (cl.p[0].x == cl.p[1].x) {
				cl.eq.vertical = true;
				cl.eq.x = cl.p[0].x;
			} else {
				cl.eq.m = (cl.p[0].y - cl.p[1].y) / (cl.p[0].x - cl.p[1].x);
				cl.eq.b = cl.eq.m * (-cl.p[0].x) + cl.p[0].y;
			}

			//do it
			lines.push_back(cl);
		}

		//now for each of the pixels in the row
		for (size_t x = 0, touchy = 0; x < width; x++) {

			float x_pix_center = float(x) + 0.5f;

			for (size_t i = 0; i < lines.size(); i++) { 
				line cl = lines[i];
				
				//if we have passed one of the segments, remove it from the vector and increment/decrement the state appropriately
				if (x_pix_center < cl.p[0].x && x_pix_center < cl.p[1].x) {
					continue;
				}else if (
					(x_pix_center > cl.p[0].x && x_pix_center > cl.p[1].x)
					||
					(cl.eq.vertical && x_pix_center >= cl.eq.x)
					||
					(x_pix_center > ((y_pix_center - cl.eq.b) / cl.eq.m))
				) {
					lines.erase(lines.begin() + i--);
					touchy += cl.down ? -1 : 1;
				}
			}

			//and so just set the pixel acording to the current state
			result[y * width + x][3] = touchy?0xFF:0;
		}
	}
	
}

enum GlyfFlags {
	//1 -> is on the curve, 0 is not on the curve
	ON_CURVE		= 0x01,
	//whether or not the x coordinate for this point is an 8 bit unsigned or 16 bit signed
	X_SHORT			= 0x02,
	//whether or not the y coordinate for this point is an 8 bit unsigned or 16 bit signed
	Y_SHORT			= 0x04,
	//if this flag repeats - if 1, the next byte after this flag will be the number of repetitions
	REPEAT			= 0x08,
	//if X_SHORT is set, this is the sign bit for the coord (1 = positive, 0 = negative)
	//if X_SHORT IS not set, this indicates whether or not this X coord is equal to the last
	X_SIGN_SAME		= 0x10,
	//if Y_SHORT is set, this is the sign bit for the coord (1 = positive, 0 = negative)
	//if Y_SHORT IS not set, this indicates whether or not this Y coord is equal to the last
	Y_SIGN_SAME		= 0x20,
	//doesn't matter
	OVERLAP_SIMPLE	= 0x40,
	//doesn't matter
	reserved		= 0x80,
};

bool GlyfEntry::getSimpleCoords(GlyfData& vec){
	//if number of contours < 0 it's a compound glyph
	if (this->num_contours < 0)return false;

	//if number of contours is zero then it's blank, prolly like a space or something
	else if (!this->num_contours)return true;
	
	
	
	//pointer to array of end points
	uint16_t* contour_end_points = (uint16_t*)simple_data;

	//length of instructions for this glyph
	uint16_t instruction_len = _byteswap_ushort(contour_end_points[this->num_contours]);

	//pointer to instructions
	uint8_t* instructions = (uint8_t*)&(contour_end_points[this->num_contours+1]);

	//pointer to flags (this pointer will be incremented as we parse the flags, and will end pointing to the beginning of the x coords array)
	uint8_t* flags_ptr = &instructions[instruction_len];

	//now we straight up fix the endianness of the whole contour end array
	for (int16_t i = 0; i < this->num_contours; i++)contour_end_points[i] = _byteswap_ushort(contour_end_points[i]);


	//the total dimension is gonna be the full amount of flags, which represent points, plus the number of contours for the repeated one at the end
	uint32_t total_dimension = (contour_end_points[this->num_contours - 1] + 1) + this->num_contours;
	//now we create the vector of vectors
	vec = { std::vector<GlyfData::GlyfCoords>(total_dimension), std::vector<uint16_t>(this->num_contours) };
	
	
	
	//vector for the flags, the size will be the index of the last contour end point +1
	std::vector<uint8_t> flags (contour_end_points[this->num_contours - 1] + 1);

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
	GlyfData::GlyfCoords* first = 0;
	for (uint16_t x = 0, point_index = 0, i = 0, si = 0; point_index < flags.size(); point_index++, i++) {

		//we first of all set the on curve bool, since it's now handy to us (we previously didn't have the vectors created)
		vec.coords[i].on_curve = flags[point_index] & ON_CURVE;

		//and if we have a short x, we read an 8-byte unsigned integer, and multiply it by what SIGN_SAME says the sign is
		if (flags[point_index] & X_SHORT) {
			x += (((flags[point_index] & X_SIGN_SAME) ? 1 : -1) * (*(point_location_8++)));
			vec.coords[i].x = x;
		} else {
			//otherwise, if SIGN_SAME is set, then that means this x is equal to the previous, so just set it on the vector
			if (flags[point_index] & X_SIGN_SAME) {
				vec.coords[i].x = x;
			} else {
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
		} else {
			if (flags[point_index] & Y_SIGN_SAME) {
				vec.coords[i].y = y;
			} else {
				y += _byteswap_ushort(*(point_location_16++));
				vec.coords[i].y = y;
			}
		}

		//if we've found the end of a contour
		if (si != this->num_contours && point_index == contour_end_points[si]) {
			//add the closing point to it
			vec.coords[++i].y = vec.coords[(si) ? vec.skips[si - 1] + 1 : 0].y;
			//and move on
			si++;
		}		
	}

	return true;
}


