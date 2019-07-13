#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>

//The first table in the TTF file

#pragma pack(push, 1) 
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
	void fix_endian() {
		this->trash				= _byteswap_ulong (this->trash);
		this->num_tables		= _byteswap_ushort(this->num_tables);
		this->search_range		= _byteswap_ushort(this->search_range);
		this->query_selectors	= _byteswap_ushort(this->query_selectors);
		this->range_shift		= _byteswap_ushort(this->range_shift);
	}
};
#pragma pack(pop)

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
	void fix_endian() {
		this->checksum	= _byteswap_ulong(this->checksum);
		this->offset	= _byteswap_ulong(this->offset);
		this->length	= _byteswap_ulong(this->length);
	}
};

enum TableTypes {
	TABLE_TYPE_UNKNOWN,
	TABLE_TYPE_EBSC,	TABLE_TYPE_OS_2,	TABLE_TYPE_Zapf,
	TABLE_TYPE_acnt,	TABLE_TYPE_ankr,	TABLE_TYPE_avar,
	TABLE_TYPE_bdat,	TABLE_TYPE_bhed,	TABLE_TYPE_bloc,
	TABLE_TYPE_bsln,	TABLE_TYPE_cmap,	TABLE_TYPE_cvar,
	TABLE_TYPE_cvt,		TABLE_TYPE_fdsc,	TABLE_TYPE_feat,
	TABLE_TYPE_fmtx,	TABLE_TYPE_fond,	TABLE_TYPE_fpgm,
	TABLE_TYPE_fvar,	TABLE_TYPE_gasp,	TABLE_TYPE_gcid,
	TABLE_TYPE_glyf,	TABLE_TYPE_gvar,	TABLE_TYPE_hdmx,
	TABLE_TYPE_head,	TABLE_TYPE_hhea,	TABLE_TYPE_htmx,
	TABLE_TYPE_just,	TABLE_TYPE_kern,	TABLE_TYPE_kerx,
	TABLE_TYPE_lcar,	TABLE_TYPE_loca,	TABLE_TYPE_ltag,
	TABLE_TYPE_maxp,	TABLE_TYPE_meta,	TABLE_TYPE_mort,
	TABLE_TYPE_morx,	TABLE_TYPE_name,	TABLE_TYPE_opbd,
	TABLE_TYPE_post,	TABLE_TYPE_prep,	TABLE_TYPE_prop,
	TABLE_TYPE_sbix,	TABLE_TYPE_trak,	TABLE_TYPE_vhea,
	TABLE_TYPE_vmtx,	TABLE_TYPE_xref,
};

uint32_t TagValues[] = {
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

struct TrueTypeFontFile {

	struct LookupEntry { DirectoryTableEntry* table_entry; void* table_data; };


	static bool Open(TrueTypeFontFile* instance, const char* path) {
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

	void close() { fclose(this->file); }


	FILE* file;

	OffsetSubtable offs_subt;
	std::vector<DirectoryTableEntry> tables_info;
	LookupEntry table_lookup[48];
};