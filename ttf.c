#include "ttf.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

const char *mac_encoding[258] = {
	[0] =	".notdef",
	[1] =	".null",
	[2] =	"nonmarkingreturn",
	[3] =	"space",
	[4] =	"exclam",
	[5] =	"quotedbl",
	[6] =	"numbersign",
	[7] =	"dollar",
	[8] =	"percent",
	[9] =	"ampersand",
	[10] =	"quotesingle",
	[11] =	"parenleft",
	[12] =	"parenright",
	[13] =	"asterisk",
	[14] =	"plus",
	[15] =	"comma",
	[16] =	"hyphen",
	[17] =	"period",
	[18] =	"slash",
	[19] =	"zero",
	[20] =	"one",
	[21] =	"two",
	[22] =	"three",
	[23] =	"four",
	[24] =	"five",
	[25] =	"six",
	[26] =	"seven",
	[27] =	"eight",
	[28] =	"nine",
	[29] =	"colon",
	[30] =	"semicolon",
	[31] =	"less",
	[32] =	"equal",
	[33] =	"greater",
	[34] =	"question",
	[35] =	"at",
	[36] =	"A",
	[37] =	"B",
	[38] =	"C",
	[39] =	"D",
	[40] =	"E",
	[41] =	"F",
	[42] =	"G",
	[43] =	"H",
	[44] =	"I",
	[45] =	"J",
	[46] =	"K",
	[47] =	"L",
	[48] =	"M",
	[49] =	"N",
	[50] =	"O",
	[51] =	"P",
	[52] =	"Q",
	[53] =	"R",
	[54] =	"S",
	[55] =	"T",
	[56] =	"U",
	[57] =	"V",
	[58] =	"W",
	[59] =	"X",
	[60] =	"Y",
	[61] =	"Z",
	[62] =	"bracketleft",
	[63] =	"backslash",
	[64] =	"bracketright",
	[65] =	"asciicircum",
	[66] =	"underscore",
	[67] =	"grave",
	[68] =	"a",
	[69] =	"b",
	[70] =	"c",
	[71] =	"d",
	[72] =	"e",
	[73] =	"f",
	[74] =	"g",
	[75] =	"h",
	[76] =	"i",
	[77] =	"j",
	[78] =	"k",
	[79] =	"l",
	[80] =	"m",
	[81] =	"n",
	[82] =	"o",
	[83] =	"p",
	[84] =	"q",
	[85] =	"r",
	[86] =	"s",
	[87] =	"t",
	[88] =	"u",
	[89] =	"v",
	[90] =	"w",
	[91] =	"x",
	[92] =	"y",
	[93] =	"z",
	[94] =	"braceleft",
	[95] =	"bar",
	[96] =	"braceright",
	[97] =	"asciitilde",
	[98] =	"Adieresis",
	[99] =	"Aring",
	[100] =	"Ccedilla",
	[101] =	"Eacute",
	[102] =	"Ntilde",
	[103] =	"Odieresis",
	[104] =	"Udieresis",
	[105] =	"aacute",
	[106] =	"agrave",
	[107] =	"acircumflex",
	[108] =	"adieresis",
	[109] =	"atilde",
	[110] =	"aring",
	[111] =	"ccedilla",
	[112] =	"eacute",
	[113] =	"egrave",
	[114] =	"ecircumflex",
	[115] =	"edieresis",
	[116] =	"iacute",
	[117] =	"igrave",
	[118] =	"icircumflex",
	[119] =	"idieresis",
	[120] =	"ntilde",
	[121] =	"oacute",
	[122] =	"ograve",
	[123] =	"ocircumflex",
	[124] =	"odieresis",
	[125] =	"otilde",
	[126] =	"uacute",
	[127] =	"ugrave",
	[128] =	"ucircumflex",
	[129] =	"udieresis",
	[130] =	"dagger",
	[131] =	"degree",
	[132] =	"cent",
	[133] =	"sterling",
	[134] =	"section",
	[135] =	"bullet",
	[136] =	"paragraph",
	[137] =	"germandbls",
	[138] =	"registered",
	[139] =	"copyright",
	[140] =	"trademark",
	[141] =	"acute",
	[142] =	"dieresis",
	[143] =	"notequal",
	[144] =	"AE",
	[145] =	"Oslash",
	[146] =	"infinity",
	[147] =	"plusminus",
	[148] =	"lessequal",
	[149] =	"greaterequal",
	[150] =	"yen",
	[151] =	"mu",
	[152] =	"partialdiff",
	[153] =	"summation",
	[154] =	"product",
	[155] =	"pi",
	[156] =	"integral",
	[157] =	"ordfeminine",
	[158] =	"ordmasculine",
	[159] =	"Omega",
	[160] =	"ae",
	[161] =	"oslash",
	[162] =	"questiondown",
	[163] =	"exclamdown",
	[164] =	"logicalnot",
	[165] =	"radical",
	[166] =	"florin",
	[167] =	"approxequal",
	[168] =	"Delta",
	[169] =	"guillemotleft",
	[170] =	"guillemotright",
	[171] =	"ellipsis",
	[172] =	"nonbreakingspace",
	[173] =	"Agrave",
	[174] =	"Atilde",
	[175] =	"Otilde",
	[176] =	"OE",
	[177] =	"oe",
	[178] =	"endash",
	[179] =	"emdash",
	[180] =	"quotedblleft",
	[181] =	"quotedblright",
	[182] =	"quoteleft",
	[183] =	"quoteright",
	[184] =	"divide",
	[185] =	"lozenge",
	[186] =	"ydieresis",
	[187] =	"Ydieresis",
	[188] =	"fraction",
	[189] =	"currency",
	[190] =	"guilsinglleft",
	[191] =	"guilsinglright",
	[192] =	"fi",
	[193] =	"fl",
	[194] =	"daggerdbl",
	[195] =	"periodcentered",
	[196] =	"quotesinglbase",
	[197] =	"quotedblbase",
	[198] =	"perthousand",
	[199] =	"Acircumflex",
	[200] =	"Ecircumflex",
	[201] =	"Aacute",
	[202] =	"Edieresis",
	[203] =	"Egrave",
	[204] =	"Iacute",
	[205] =	"Icircumflex",
	[206] =	"Idieresis",
	[207] =	"Igrave",
	[208] =	"Oacute",
	[209] =	"Ocircumflex",
	[210] =	"apple",
	[211] =	"Ograve",
	[212] =	"Uacute",
	[213] =	"Ucircumflex",
	[214] =	"Ugrave",
	[215] =	"dotlessi",
	[216] =	"circumflex",
	[217] =	"tilde",
	[218] =	"macron",
	[219] =	"breve",
	[220] =	"dotaccent",
	[221] =	"ring",
	[222] =	"cedilla",
	[223] =	"hungarumlaut",
	[224] =	"ogonek",
	[225] =	"caron",
	[226] =	"Lslash",
	[227] =	"lslash",
	[228] =	"Scaron",
	[229] =	"scaron",
	[230] =	"Zcaron",
	[231] =	"zcaron",
	[232] =	"brokenbar",
	[233] =	"Eth",
	[234] =	"eth",
	[235] =	"Yacute",
	[236] =	"yacute",
	[237] =	"Thorn",
	[238] =	"thorn",
	[239] =	"minus",
	[240] =	"multiply",
	[241] =	"onesuperior",
	[242] =	"twosuperior",
	[243] =	"threesuperior",
	[244] =	"onehalf",
	[245] =	"onequarter",
	[246] =	"threequarters",
	[247] =	"franc",
	[248] =	"Gbreve",
	[249] =	"gbreve",
	[250] =	"Idotaccent",
	[251] =	"Scedilla",
	[252] =	"scedilla",
	[253] =	"Cacute",
	[254] =	"cacute",
	[255] =	"Ccaron",
	[256] =	"ccaron",
	[257] =	"dcroat"
};

uint8_t read_byte(int fd) {
	uint8_t b;
	if (read(fd, &b, sizeof(uint8_t)) < 0) {
		fprintf(stderr, "failed to read byte: %s\n", strerror(errno));
	}
	return b;
}

uint16_t read_hword(int fd) {
	uint16_t hw;
	if (read(fd, &hw, sizeof(uint16_t)) < 0) {
		fprintf(stderr, "failed to read hword: %s\n", strerror(errno));
	}
	return hw;
}

uint32_t read_word(int fd) {
	uint32_t w;
	if (read(fd, &w, sizeof(uint32_t)) < 0) {
		fprintf(stderr, "failed to read word: %s\n", strerror(errno));
	}
	return w;
}

uint64_t read_dword(int fd) {
	uint64_t dw;
	if (read(fd, &dw, sizeof(uint64_t)) < 0) {
		fprintf(stderr, "failed to read dword: %s\n", strerror(errno));
	}
	return dw;
}

uint16_t read_ushort(int fd) {
	uint16_t hw = read_hword(fd);
	uint8_t *b = (uint8_t *)&hw;
	return (b[0] << 8) | (b[1] << 0);
}

int16_t read_short(int fd) {
	uint16_t us = read_ushort(fd);
	return (us >= 0x8000) ? us - 0x10000 : us;
}

uint32_t read_ulong(int fd) {
	uint32_t w = read_word(fd);
	uint8_t *b = (uint8_t *)&w;
	if ((int8_t)b[3] < 0) {
		fprintf(stderr, "read invalid 32-bit unsigned int\n");
	}
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3] << 0);
}

uint32_t read_fixed(int fd) {
	return read_ulong(fd);
}

uint32_t read_tag(int fd) {
	return read_word(fd);
}

int64_t read_longdatetime(int fd) {
	uint64_t dw = read_dword(fd);
	uint8_t *b = (uint8_t *)&dw;
	return ((int64_t)b[0] << 56) | ((int64_t)b[1] << 48) |
		   ((int64_t)b[2] << 40) | ((int64_t)b[3] << 32) |
		   ((int64_t)b[4] << 24) | ((int64_t)b[5] << 16) |
		   ((int64_t)b[6] <<  8) | ((int64_t)b[7] <<  0);
}

float fixed_to_float(uint32_t fixed) {
	uint16_t *us = (uint16_t*)&fixed;
	return (us[1]) + (us[0]/(float)(1 << sizeof(uint16_t)));
}

uint32_t s_to_tag(const char *s) {
	if (!s || strlen(s) < 4) {
		return 0;
	}
	return (s[0] << 0) | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
}

int read_font_dir(TTF_Font *font) {
	// Seek to start of file
	if (lseek(font->fd, 0, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to font dir: %s\n", strerror(errno));
		return 0;
	}

	// Read offset subtable
	font->scaler_type = read_fixed(font->fd);
	font->num_tables = read_ushort(font->fd);
	font->search_range = read_ushort(font->fd);
	font->entry_selector = read_ushort(font->fd);
	font->range_shift = read_ushort(font->fd);

	// Ensure that 12 bytes have been read
	if (lseek(font->fd, 0, SEEK_CUR) != 12) {
		fprintf(stderr, "incorrect number of bytes in offset subtable\n");
		return 0;
	}

	/**
	 * Alloc table array with calloc:
	 * Ensures that all pointers are initialized to NULL.
	 */
	font->tables = (TTF_Table *) calloc(font->num_tables, sizeof(*font->tables));
	if (!font->tables) {
		fprintf(stderr, "failed to alloc font tables: %s\n", strerror(errno));
		return 0;
	}

	// Read the table directory entries
	int i;
	for (i = 0; i < font->num_tables; i++) {
		TTF_Table *table = &font->tables[i];

		table->tag = read_tag(font->fd);
		table->check_sum = read_ulong(font->fd);
		table->offset = read_ulong(font->fd);
		table->length = read_ulong(font->fd);

		table->status = STATUS_NONE;
	}

	return 1;
}

int read_table_raw(TTF_Font *font, TTF_Table *table, uint32_t *buf) {
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to font table: %s\n", strerror(errno));
		return 0;
	}

	uint32_t *p = buf;
	size_t n_bytes = table->length;
	size_t bytes_left = n_bytes;
	ssize_t bytes_read = 0;

	// Read n_bytes from the font file descriptor
	while (bytes_read < (ssize_t)n_bytes) {
		if ((bytes_read = read(font->fd, p, bytes_left)) < 0) {
			fprintf(stderr, "failed to read raw table: %s\n", strerror(errno));
			break;
		}
		p += bytes_read;
		bytes_left -= bytes_read;
	}

	return (bytes_read == (ssize_t)n_bytes);
}

uint32_t calc_table_check_sum(uint32_t *data, uint32_t length) {
	uint32_t sum = 0, *p = data;
	uint8_t *b;
	uint32_t n_longs = (length + 3) / sizeof(uint32_t);
	while (n_longs-- > 0) {
		b = (uint8_t *)p;
		sum += (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3] << 0);
		p++;
	}
	return sum;
}

int validate_check_sums(TTF_Font *font) {
	int i;
	for (i = 0; i < font->num_tables; i++) {
		TTF_Table *table = &font->tables[i];
		if (!table) {
			continue;
		}
		uint32_t *data = (uint32_t *) malloc(table->length);
		if (!data) {
			fprintf(stderr, "failed to alloc raw table data: %s\n", strerror(errno));
			return 0;
		}
		if (!read_table_raw(font, table, data)) {
			return 0;
		}
		uint32_t sum = calc_table_check_sum(data, table->length);
		if (sum != table->check_sum) {
			fprintf(stderr, "table '%.*s' data does not match check sum\n",
					TAG_LENGTH, (char *)&(table->tag));
			fprintf(stderr, "sum = %08x\n", sum);
			fprintf(stderr, "check sum = %08x\n", table->check_sum);
			return 0;
		}
		free(data);
	}
	return 1;
}

int load_cmap_subtable(TTF_Font *font, cmap_subTable *subtable) {
	maxp_Table *maxp = NULL;

	// Get maxp table
	TTF_Table *temp = get_table_by_name(font, "maxp");
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;

	subtable->format = read_ushort(font->fd);
	if (subtable->format < 8) {
		subtable->length = read_ushort(font->fd);
		subtable->language = read_ushort(font->fd);
	} else {
		read_ushort(font->fd); // format was actually a Fixed, this is the .X part
		subtable->length = read_ulong(font->fd);
		subtable->language = read_ulong(font->fd);
	}
	switch (subtable->format) {
		case 0:
			{
				subtable->glyph_index_array = (uint32_t *) malloc(256 * sizeof(*subtable->glyph_index_array));
				if (!subtable->glyph_index_array) {
					fprintf(stderr, "failed to alloc cmap format 0 glyph index array: %s\n", strerror(errno));
					return 0;
				}
				int i;
				for (i = 0; i < 256; i++) {
					uint8_t glyph_mapping = read_byte(font->fd);
					subtable->glyph_index_array[i] = (glyph_mapping + 256) % 256;
				}
			}
			break;
		case 4:
			{
				int i;
				uint16_t seg_count = read_ushort(font->fd) / 2;
				uint16_t search_range = read_ushort(font->fd);
				uint16_t entry_selector = read_ushort(font->fd);
				uint16_t range_shift = read_ushort(font->fd);

				// Alloc arrays
				uint16_t *end_code = (uint16_t *) malloc(seg_count * sizeof(*end_code));
				if (!end_code) {
					fprintf(stderr, "failed to alloc cmap format 4 endCode array: %s\n", strerror(errno));
					return 0;
				}
				uint16_t *start_code = (uint16_t *) malloc(seg_count * sizeof(*start_code));
				if (!start_code) {
					fprintf(stderr, "failed to alloc cmap format 4 startCode array: %s\n", strerror(errno));
					return 0;
				}
				uint16_t *id_delta = (uint16_t *) malloc(seg_count * sizeof(*id_delta));
				if (!id_delta) {
					fprintf(stderr, "failed to alloc cmap format 4 idDelta array: %s\n", strerror(errno));
					return 0;
				}
				uint16_t *id_range_offset = (uint16_t *) malloc(seg_count * sizeof(*id_range_offset));
				if (!id_range_offset) {
					fprintf(stderr, "failed to alloc cmap format 4 idRangeOffset array: %s\n", strerror(errno));
					return 0;
				}

				// Read arrays
				for (i = 0; i < seg_count; i++) {
					end_code[i] = read_ushort(font->fd);
				}
				uint16_t reserved_pad = read_ushort(font->fd);
				for (i = 0; i < seg_count; i++) {
					start_code[i] = read_ushort(font->fd);
				}
				for (i = 0; i < seg_count; i++) {
					id_delta[i] = read_ushort(font->fd);
				}
				for (i = 0; i < seg_count; i++) {
					id_range_offset[i] = read_ushort(font->fd);
				}

				/* Alloc with calloc to ensure that all glyphs point to 0 by default. */
				subtable->glyph_index_array = (uint32_t *) calloc(maxp->num_glyphs, sizeof(*subtable->glyph_index_array));
				if (!subtable->glyph_index_array) {
					fprintf(stderr, "failed to alloc cmap format 4 glyph index array: %s\n", strerror(errno));
					return 0;
				}

				off_t cur_pos;
				if ((cur_pos = lseek(font->fd, 0, SEEK_CUR)) < 0) {
					fprintf(stderr, "failed to get file position: %s\n", strerror(errno));
					return 0;
				}

				for (i = 0; i < seg_count; i++) {
					uint16_t start = start_code[i];
					uint16_t end = end_code[i];
					uint16_t delta = id_delta[i];
					uint16_t range_offset = id_range_offset[i];
					
					if (start != (65536-1) && end != (65536-1)) {
						int j;
						for (j = start; j <= end; j++) {
							if (range_offset == 0) {
								subtable->glyph_index_array[(j + delta) % 65536] = j;
							} else {
								uint32_t glyph_offset = cur_pos +
									((range_offset/2) + (j-start) + (i-seg_count))*2;
								if (lseek(font->fd, glyph_offset, SEEK_SET) < 0) {
									fprintf(stderr, "failed to seek to glyph offset: %s\n", strerror(errno));
									return 0;
								}
								uint16_t glyph_index = read_ushort(font->fd);
								if (glyph_index != 0) {
									glyph_index = (glyph_index + delta) % 65536;
									if (subtable->glyph_index_array[glyph_index] == 0) {
										subtable->glyph_index_array[glyph_index] = j;
									}
								}
							}
						}
					}
				}

				// FIXME: remove
				search_range = 0;
				entry_selector = 0;
				range_shift = 0;
				reserved_pad = 0;

				free(end_code);
				free(start_code);
				free(id_delta);
				free(id_range_offset);
			}
			break;
		default:
			fprintf(stderr, "unsupported cmap subtable format: %u\n", subtable->format);
			return 0;
	}

	return 1;
}

int load_cmap_table(TTF_Font *font, TTF_Table *table) {
	cmap_Table *cmap = &table->data.cmap;

	cmap->version = read_ushort(font->fd);
	cmap->num_subtables = read_ushort(font->fd);

	cmap->subtables = (cmap_subTable *) malloc(cmap->num_subtables * sizeof(*cmap->subtables));
	if (!cmap->subtables) {
		fprintf(stderr, "failed to alloc cmap subtables: %s\n", strerror(errno));
		return 0;
	}

	off_t pos;
	int i;
	for (i = 0; i < cmap->num_subtables; i++) {
		cmap_subTable *subtable = &cmap->subtables[i];

		subtable->platform_id = read_ushort(font->fd);
		subtable->platform_specifid_id = read_ushort(font->fd);
		subtable->offset = read_ulong(font->fd);

		// Save current file position
		if ((pos = lseek(font->fd, 0, SEEK_CUR)) < 0) {
			fprintf(stderr, "failed to save file position: %s\n", strerror(errno));
			return 0;
		}

		// Seek to start of mapping subtable (relative to cmap table offset)
		if (lseek(font->fd, table->offset + subtable->offset, SEEK_SET) < 0) {
			fprintf(stderr, "failed to seek to cmap mapping subtable: %s\n", strerror(errno));
			return 0;
		}

		load_cmap_subtable(font, subtable);

		// Restore file position
		if (lseek(font->fd, pos, SEEK_SET) < 0) {
			fprintf(stderr, "failed to restore file position: %s\n", strerror(errno));
			return 0;
		}
	}

	return 1;
}

int load_cvt_table(TTF_Font *font, TTF_Table *table) {
	cvt_Table *cvt = &table->data.cvt;

	int num_values = table->length / 4;
	cvt->control_values = (int16_t *) malloc(num_values * sizeof(*cvt->control_values));
	if (!cvt->control_values) {
		fprintf(stderr, "failed to alloc control values: %s\n", strerror(errno));
		return 0;
	}

	int i;
	for (i = 0; i < num_values; i++) {
		cvt->control_values[i] = read_short(font->fd);
	}

	return 1;
}

int load_glyph(TTF_Font *font, TTF_Glyph *glyph) {
	glyph->number_of_contours = read_short(font->fd);
	glyph->x_min = read_short(font->fd);
	glyph->y_min = read_short(font->fd);
	glyph->x_max = read_short(font->fd);
	glyph->y_max = read_short(font->fd);

	if (glyph->number_of_contours == 0) {
		glyph->num_points = 0;
		return 1;
	} else if (glyph->number_of_contours > 0) {
		// Simple glyph
		glyph->end_pts_of_contours = (uint16_t *) malloc(glyph->number_of_contours * sizeof(*glyph->end_pts_of_contours));
		if (!glyph->end_pts_of_contours) {
			fprintf(stderr, "failed to alloc glyph end points: %s\n", strerror(errno));
			return 0;
		}
		int i;
		for (i = 0; i < glyph->number_of_contours; i++) {
			glyph->end_pts_of_contours[i] = read_ushort(font->fd);
		}
		// Get the last contour's end point index
		int last_end_pt = glyph->end_pts_of_contours[glyph->number_of_contours - 1];
		if (glyph->number_of_contours == 1 && last_end_pt == 65535) {
			// Assume an empty glpyh
			glyph->num_points = 0;
			return 1;
		}
		// Last end point index indicates the number of points
		glyph->num_points = last_end_pt + 1;

		glyph->instruction_length = read_ushort(font->fd);

		glyph->instructions = (uint8_t *) malloc(glyph->instruction_length * sizeof(*glyph->instructions));
		if (!glyph->instructions) {
			fprintf(stderr, "failed to alloc glyph instructions: %s\n", strerror(errno));
			return 0;
		}

		glyph->flags = (uint8_t *) malloc(glyph->num_points * sizeof(*glyph->flags));
		if (!glyph->flags) {
			fprintf(stderr, "failed to alloc glyph flags: %s\n", strerror(errno));
			return 0;
		}
		glyph->x_coordinates = (int16_t *) malloc(glyph->num_points * sizeof(*glyph->x_coordinates));
		if (!glyph->x_coordinates) {
			fprintf(stderr, "failed to alloc glyph x coordinates: %s\n", strerror(errno));
			return 0;
		}
		glyph->y_coordinates = (int16_t *) malloc(glyph->num_points * sizeof(*glyph->y_coordinates));
		if (!glyph->y_coordinates) {
			fprintf(stderr, "failed to alloc glyph y coordinates: %s\n", strerror(errno));
			return 0;
		}

		for (i = 0; i < glyph->instruction_length; i++) {
			glyph->instructions[i] = read_byte(font->fd);
		}
		for (i = 0; i < glyph->num_points; i++) {
			glyph->flags[i] = read_byte(font->fd);
			if ((glyph->flags[i] & REPEAT) != 0) {
				// Last flag should be repeated some number of times
				uint8_t repeats = read_byte(font->fd);
				int j;
				for (j = 1; j <= repeats; j++) {
					glyph->flags[i + j] = glyph->flags[i];
				}
				i += repeats;
			}
		}

		int16_t x = 0, y = 0;
		for (i = 0; i < glyph->num_points; i++) {
			if ((glyph->flags[i] & X_DUAL) != 0) {
				if ((glyph->flags[i] & X_SHORT_VECTOR) != 0) {
					x += (int16_t) read_byte(font->fd);
				}
			} else {
				if ((glyph->flags[i] & X_SHORT_VECTOR) != 0) {
					x += -((int16_t) read_byte(font->fd));
				} else {
					x += read_short(font->fd);
				}
			}
			glyph->x_coordinates[i] = x;
		}
		for (i = 0; i < glyph->num_points; i++) {
			if ((glyph->flags[i] & Y_DUAL) != 0) {
				if ((glyph->flags[i] & Y_SHORT_VECTOR) != 0) {
					y += (int16_t) read_byte(font->fd);
				}
			} else {
				if ((glyph->flags[i] & Y_SHORT_VECTOR) != 0) {
					y += -((int16_t) read_byte(font->fd));
				} else {
					y += read_short(font->fd);
				}
			}
			glyph->y_coordinates[i] = y;
		}
	} else {
		// Compound glyph
//		fprintf(stderr, "unsupported compound glyph\n");
		return 0;
	}

	return 1;
}

int load_glyf_table(TTF_Font *font, TTF_Table *table) {
	glyf_Table *glyf = &table->data.glyf;
	maxp_Table *maxp = NULL;
	loca_Table *loca = NULL;

	// Get maxp and loca tables
	TTF_Table *temp = get_table_by_name(font, "maxp");
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;
	temp = get_table_by_name(font, "loca");
	if (!temp) {
		fprintf(stderr, "failed to get loca table\n");
		return 0;
	}
	loca = &temp->data.loca;

	// Save number of glyphs in glyf table so that the glyph array can be easily accessed
	glyf->num_glyphs = maxp->num_glyphs;

	/**
	 * Alloc glyph array with calloc:
	 * This ensures that all pointers are initialized to NULL
	 */
	glyf->glyphs = (TTF_Glyph *) calloc(glyf->num_glyphs, sizeof(*glyf->glyphs));
	if (!glyf->glyphs) {
		fprintf(stderr, "failed to alloc glyphs: %s\n", strerror(errno));
		return 0;
	}

	int i;
	for (i = 0; i < glyf->num_glyphs; i++) {
		TTF_Glyph *glyph = &glyf->glyphs[i];
		if (lseek(font->fd, table->offset + loca->offsets[i], SEEK_SET) < 0) {
			fprintf(stderr, "failed to seek to glyph %d: %s\n", i, strerror(errno));
			return 0;
		}
		load_glyph(font, glyph);
	}

	return 1;
}

int load_head_table(TTF_Font *font, TTF_Table *table) {
	head_Table *head = &table->data.head;

	head->version = read_fixed(font->fd);
	head->font_revision = read_fixed(font->fd);
	head->check_sum_adjustment = read_ulong(font->fd);
	head->magic_number = read_ulong(font->fd);
	head->flags = read_ushort(font->fd);
	head->units_per_em = read_ushort(font->fd);
	head->created = read_longdatetime(font->fd);
	head->modified = read_longdatetime(font->fd);
	head->x_min = read_short(font->fd);
	head->y_min = read_short(font->fd);
	head->x_max = read_short(font->fd);
	head->y_max = read_short(font->fd);
	head->mac_style = read_ushort(font->fd);
	head->lowest_rec_ppem = read_ushort(font->fd);
	head->font_direction_hint = read_short(font->fd);
	head->index_to_loc_format = read_short(font->fd);
	head->glyph_data_format = read_short(font->fd);

	return 1;
}

int load_hhea_table(TTF_Font *font, TTF_Table *table) {
	hhea_Table *hhea = &table->data.hhea;

	hhea->version = read_fixed(font->fd);
	hhea->ascent = read_short(font->fd);
	hhea->descent = read_short(font->fd);
	hhea->line_gap = read_short(font->fd);
	hhea->advance_width_max = read_ushort(font->fd);
	hhea->min_left_side_bearing = read_short(font->fd);
	hhea->min_right_side_bearing = read_short(font->fd);
	hhea->x_max_extent = read_short(font->fd);
	hhea->caret_slope_rise = read_short(font->fd);
	hhea->caret_slope_run = read_short(font->fd);
	hhea->caret_offset = read_short(font->fd);
	hhea->reserved1 = read_short(font->fd);
	hhea->reserved2 = read_short(font->fd);
	hhea->reserved3 = read_short(font->fd);
	hhea->reserved4 = read_short(font->fd);
	hhea->metric_data_format = read_short(font->fd);
	hhea->num_of_long_hor_metrics = read_ushort(font->fd);

	return 1;
}

int load_loca_table(TTF_Font *font, TTF_Table *table) {
	loca_Table *loca = &table->data.loca;
	head_Table *head = NULL;
	maxp_Table *maxp = NULL;

	// Get head and maxp tables
	TTF_Table *temp = get_table_by_name(font, "head");
	if (!temp) {
		fprintf(stderr, "failed to get head table\n");
		return 0;
	}
	head = &temp->data.head;
	temp = get_table_by_name(font, "maxp");
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;

	loca->offsets = (uint32_t *) malloc((maxp->num_glyphs + 1) * sizeof(*loca->offsets));
	if (!loca->offsets) {
		fprintf(stderr, "failed to alloc loca offsets: %s\n", strerror(errno));
		return 0;
	}

	int i;
	for (i = 0; i < maxp->num_glyphs; i++) {
		switch (head->index_to_loc_format) {
			case SHORT_OFFSETS:
				loca->offsets[i] = read_ushort(font->fd) * 2;
				break;
			case LONG_OFFSETS:
				loca->offsets[i] = read_ulong(font->fd);
				break;
			default:
				fprintf(stderr, "unknown loca offset format: %hd\n", head->index_to_loc_format);
				return 0;
		}
	}

	return 1;
}

int load_maxp_table(TTF_Font *font, TTF_Table *table) {
	maxp_Table *maxp = &table->data.maxp;

	maxp->version = read_fixed(font->fd);
	maxp->num_glyphs = read_ushort(font->fd);
	maxp->max_points = read_ushort(font->fd);
	maxp->max_contours = read_ushort(font->fd);
	maxp->max_component_points = read_ushort(font->fd);
	maxp->max_component_contours = read_ushort(font->fd);
	maxp->max_zones = read_ushort(font->fd);
	maxp->max_twilight_points = read_ushort(font->fd);
	maxp->max_storage = read_ushort(font->fd);
	maxp->max_function_defs = read_ushort(font->fd);
	maxp->max_instruction_defs = read_ushort(font->fd);
	maxp->max_stack_elements = read_ushort(font->fd);
	maxp->max_size_of_instructions = read_ushort(font->fd);
	maxp->max_component_elements = read_ushort(font->fd);
	maxp->max_component_depth = read_ushort(font->fd);

	return 1;
}

int load_post_table(TTF_Font *font, TTF_Table *table) {
	post_Table *post = &table->data.post;
	maxp_Table *maxp = NULL;

	// Get maxp table
	TTF_Table *temp = get_table_by_name(font, "maxp");
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;

	post->format = read_fixed(font->fd);
	post->italic_angle = read_fixed(font->fd);
	post->underline_position = read_short(font->fd);
	post->underline_thickness = read_short(font->fd);
	post->is_fixed_pitch = read_ulong(font->fd);
	post->min_mem_type_42 = read_ulong(font->fd);
	post->max_mem_type_42 = read_ulong(font->fd);
	post->min_mem_type_1 = read_ulong(font->fd);
	post->max_mem_type_1 = read_ulong(font->fd);

	switch (post->format) {
		case 0x00010000:
			{
				// Font contains exactly the 258 glyphs in the standard Macintosh ordering.
				post->num_glyphs = 258;

				post->glyph_names = (char **) malloc(post->num_glyphs * sizeof(*post->glyph_names));
				if (!post->glyph_names) {
					fprintf(stderr, "failed to alloc post glyph names: %s\n", strerror(errno));
					return 0;
				}
				int i;
				for (i = 0; i < post->num_glyphs; i++) {
					post->glyph_names[i] = (char *) malloc(strlen(mac_encoding[i]) * sizeof(*post->glyph_names[i]));
					if (!post->glyph_names) {
						fprintf(stderr, "failed to alloc glyph name %d: %s\n", i, strerror(errno));
						return 0;
					}
					strcpy(post->glyph_names[i], mac_encoding[i]);
				}
			}
			break;
		case 0x00020000:
			{
				// Font contains some glyphs not in the standard set or its glyph ordering is non-standard.
				post->num_glyphs = read_ushort(font->fd);

				uint16_t *glyph_name_index = (uint16_t *) malloc(post->num_glyphs * sizeof(*glyph_name_index));
				if (!glyph_name_index) {
					fprintf(stderr, "failed to alloc post glyphNameIndex array: %s\n", strerror(errno));
					return 0;
				}
				uint16_t max_index = 0;
				int i;
				for (i = 0; i < post->num_glyphs; i++) {
					glyph_name_index[i] = read_ushort(font->fd);
					max_index = (int) fmax(max_index, glyph_name_index[i]);
				}

				char **names = NULL;
				if (max_index >= 258) {
					names = (char **) malloc((max_index-258 + 1) * sizeof(*names));
					if (!names) {
						fprintf(stderr, "failed to alloc post names array: %s\n", strerror(errno));
						return 0;
					}
					int i;
					for (i = 0; i < max_index-258 + 1; i++) {
						uint8_t name_length = read_byte(font->fd);

						names[i] = (char *) malloc((name_length + 1) * sizeof(*names[i]));
						if (!names[i]) {
							fprintf(stderr, "failed to alloc glyph name %d: %s\n", i+258, strerror(errno));
							return 0;
						}

						int j;
						for (j = 0; j < name_length; j++) {
							names[i][j] = (char) read_byte(font->fd);
						}
						names[i][j] = '\0';
					}
				}

				post->glyph_names = (char **) malloc(post->num_glyphs * sizeof(*post->glyph_names));
				if (!post->glyph_names) {
					fprintf(stderr, "failed to alloc post glyph names: %s\n", strerror(errno));
					return 0;
				}
				for (i = 0; i < post->num_glyphs; i++) {
					uint16_t index = glyph_name_index[i];
					if (index < 258) {
						// Glyph name is mac_encoding[index]
						post->glyph_names[i] = (char *) malloc(strlen(mac_encoding[index]) * sizeof(*post->glyph_names[i]));
						if (!post->glyph_names[i]) {
							fprintf(stderr, "failed to alloc glyph name %hu: %s\n", index, strerror(errno));
							return 0;
						}
						strcpy(post->glyph_names[i], mac_encoding[index]);
					} else if (index >= 258 && index <= 32767) {
						post->glyph_names[i] = names[index-258];
					} else {
						fprintf(stderr, "unknown glyph name index: %hu\n", index);
					}
				}

				free(glyph_name_index);
				free(names);
			}
			break;
		case 0x00025000:
			{
				// Font contains a pure subset of the standard glyph set or a reordering of the standard set.
				post->num_glyphs = maxp->num_glyphs;

				uint16_t *glyph_name_index = (uint16_t *) malloc(post->num_glyphs * sizeof(*glyph_name_index));
				if (!glyph_name_index) {
					fprintf(stderr, "failed to alloc post glyphNameIndex array: %s\n", strerror(errno));
					return 0;
				}

				int i;
				for (i = 0; i < post->num_glyphs; i++) {
					int8_t offset = (int8_t) read_byte(font->fd);
					glyph_name_index[i] = offset + (i+1);
				}

				post->glyph_names = (char **) malloc(post->num_glyphs * sizeof(*post->glyph_names));
				if (!post->glyph_names) {
					fprintf(stderr, "failed to alloc post glyph names: %s\n", strerror(errno));
					return 0;
				}

				for (i = 0; i < post->num_glyphs; i++) {
					uint16_t index = glyph_name_index[i];
					post->glyph_names[i] = (char *) malloc(strlen(mac_encoding[index]) * sizeof(*post->glyph_names[i]));
					if (!post->glyph_names) {
						fprintf(stderr, "failed to alloc glyph name %d: %s\n", index, strerror(errno));
						return 0;
					}
					strcpy(post->glyph_names[i], mac_encoding[index]);
				}

				free(glyph_name_index);
			}
			break;
		case 0x00030000:
		default:
			// No postscript information provided
			break;
	}

	return 1;
}

int load_table(TTF_Font *font, TTF_Table *table) {
	if (!font || !table) {
		return 0;
	}
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to %.*s table: %s\n",
				TAG_LENGTH, (char *)&(table->tag), strerror(errno));
		return 0;
	}
	switch (table->tag) {
		case 0x322f534f:	/* OS/2 */
			break;
		case 0x544c4350:	/* PCLT */
			break;
		case 0x70616d63:	/* cmap */
			return load_cmap_table(font, table);
		case 0x20747663:	/* cvt  */
			return load_cvt_table(font, table);
		case 0x6d677066:	/* fpgm */
			break;
		case 0x70736167:	/* gasp */
			break;
		case 0x66796c67:	/* glyf */
			return load_glyf_table(font, table);
		case 0x786d6468:	/* hdmx */
			break;
		case 0x64616568:	/* head */
			return load_head_table(font, table);
		case 0x61656868:	/* hhea */
			return load_hhea_table(font, table);
		case 0x78746d68:	/* hmtx */
			break;
		case 0x6e72656b:	/* kern */
			break;
		case 0x61636f6c:	/* loca */
			return load_loca_table(font, table);
		case 0x7078616d:	/* maxp */
			return load_maxp_table(font, table);
		case 0x656d616e:	/* name */
			break;
		case 0x74736f70:	/* post */
			return load_post_table(font, table);
			break;
		case 0x70657270:	/* prep */
			break;
		default:
			fprintf(stderr, "unknown font table type '%.*s'\n", TAG_LENGTH, (char *)&(table->tag));
			break;
	}

	return 0;
}

int load_tables(TTF_Font *font) {
	if (!font) {
		return 0;
	}
	const char *required_tables[] = {
		"head",
		"hhea",
		"maxp",
		"post",
		"loca",
		NULL
	};
	int i;
	// Load several required tables in order
	for (i = 0; required_tables[i] != NULL; i++) {
		TTF_Table *table = get_table_by_name(font, required_tables[i]);
		if (!table) {
			fprintf(stderr, "%s table was not found\n", required_tables[i]);
			return 0;
		}

		if (!load_table(font, table)) {
			fprintf(stderr, "failed to load table '%s'\n", required_tables[i]);
			return 0;
		}
		table->status = STATUS_LOADED;
	}

	for (i = 0; i < font->num_tables; i++) {
		TTF_Table *table = &font->tables[i];
		if (table->status != STATUS_LOADED) {
			if (!load_table(font, table)) {
				fprintf(stderr, "failed to load table '%.*s'\n", TAG_LENGTH, (char *)&(table->tag));
				continue;
			}
			table->status = STATUS_LOADED;
		}
	}
	return 1;
}

void free_cmap_table(cmap_Table *cmap) {
	if (!cmap) {
		return;
	}
	if (cmap->subtables) {
		int i;
		for (i = 0; i < cmap->num_subtables; i++) {
			cmap_subTable *subtable = &cmap->subtables[i];
			if (subtable && subtable->glyph_index_array) {
				free(subtable->glyph_index_array);
			}
		}
		free(cmap->subtables);
	}
}

void free_cvt_table(cvt_Table *cvt) {
	if (!cvt) {
		return;
	}
	if (cvt->control_values) {
		free(cvt->control_values);
	}
}

void free_glyph(TTF_Glyph *glyph) {
	if (!glyph) {
		return;
	}
	if (glyph->end_pts_of_contours) {
		free(glyph->end_pts_of_contours);
	}
	if (glyph->flags) {
		free(glyph->flags);
	}
	if (glyph->instructions) {
		free(glyph->instructions);
	}
	if (glyph->x_coordinates) {
		free(glyph->x_coordinates);
	}
	if (glyph->y_coordinates) {
		free(glyph->y_coordinates);
	}
}

void free_glyf_table(glyf_Table *glyf) {
	if (!glyf) {
		return;
	}
	if (glyf->glyphs) {
		int i;
		for (i = 0; i < glyf->num_glyphs; i++) {
			TTF_Glyph *glyph = &glyf->glyphs[i];
			free_glyph(glyph);
		}
		free(glyf->glyphs);
	}
}

void free_head_table(head_Table *head) {
	if (!head) {
		return;
	}
}

void free_hhea_table(hhea_Table *hhea) {
	if (!hhea) {
		return;
	}
}

void free_loca_table(loca_Table *loca) {
	if (!loca) {
		return;
	}
	if (loca->offsets) {
		free(loca->offsets);
	}
}

void free_maxp_table(maxp_Table *maxp) {
	if (!maxp) {
		return;
	}
}

void free_post_table(post_Table *post) {
	if (!post) {
		return;
	}
	if (post->glyph_names) {
		int i;
		for (i = 0; i < post->num_glyphs; i++) {
			if (post->glyph_names[i]) {
				free(post->glyph_names[i]);
			}
		}
		free(post->glyph_names);
	}
}

void free_table(TTF_Table *table) {
	if (!table) {
		return;
	}
	switch (table->tag) {
		case 0x70616d63:	/* cmap */
			free_cmap_table(&table->data.cmap);
			break;
		case 0x20747663:	/* cvt  */
			free_cvt_table(&table->data.cvt);
			break;
		case 0x66796c67:	/* glyf */
			free_glyf_table(&table->data.glyf);
			break;
		case 0x64616568:	/* head */
			free_head_table(&table->data.head);
			break;
		case 0x61656868:	/* hhea */
			free_hhea_table(&table->data.hhea);
			break;
		case 0x61636f6c:	/* loca */
			free_loca_table(&table->data.loca);
			break;
		case 0x7078616d:	/* maxp */
			free_maxp_table(&table->data.maxp);
			break;
		case 0x74736f70:	/* post */
			free_post_table(&table->data.post);
			break;
		default:
			break;
	}
}

void free_font(TTF_Font *font) {
	if (!font) {
		return;
	}
	if (font->tables) {
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = &font->tables[i];
			free_table(table);
			table->status = STATUS_FREED;
		}
		free(font->tables);
	}
	free(font);
}

TTF_Table *get_table(TTF_Font *font, uint32_t tag) {
	if (!font) {
		return NULL;
	}
	if (font->tables) {
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = &font->tables[i];
			if (table != NULL && table->tag == tag) {
				return table;
			}
		}
	}

	return NULL;
}

TTF_Table *get_table_by_name(TTF_Font *font, const char *name) {
	return get_table(font, s_to_tag(name));
}

TTF_Font *parse_file(const char *filename) {
	if (!filename) {
		return NULL;
	}

	TTF_Font *font = (TTF_Font*) malloc(sizeof(TTF_Font));
	if (!font) {
		fprintf(stderr, "failed to alloc font: %s\n", strerror(errno));
		return NULL;
	}

	if ((font->fd = open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "failed to open font file: %s\n", strerror(errno));
		return NULL;
	}

	if (!read_font_dir(font)) {
		fprintf(stderr, "failed to read font dir\n");
	}
	if (!load_tables(font)) {
		fprintf(stderr, "failed to load font tables\n");
	}

	if (close(font->fd) < 0) {
		fprintf(stderr, "failed to close font file: %s\n", strerror(errno));
	}

	return font;
}

void print_cmap_table(cmap_Table *cmap) {
	if (!cmap) {
		return;
	}
	printf("\ntable: cmap\n");
	printf("----------------\n");

	printf("version: 0x%04x\n", cmap->version);
	printf("numberSubtables: %hd\n", cmap->num_subtables);

	if (cmap->subtables) {
		int i;
		for (i = 0; i < cmap->num_subtables; i++) {
			cmap_subTable *subtable = &cmap->subtables[i];
			if (!subtable) {
				continue;
			}
			printf("platformID: %hu\n", subtable->platform_id);
			printf("platformSpecificID: %hu\n", subtable->platform_specifid_id);
			printf("offset: %08X\n", subtable->offset);
		}
	}
}

void print_glyph(TTF_Glyph *glyph) {
	if (!glyph) {
		return;
	}

	printf("numberOfContours: %hd\n", glyph->number_of_contours);
	printf("xMin: %hd\n", glyph->x_min);
	printf("yMin: %hd\n", glyph->y_min);
	printf("xMax: %hd\n", glyph->x_max);
	printf("xMax: %hd\n", glyph->y_max);
}

void print_glyf_table(glyf_Table *glyf) {
	if (!glyf) {
		return;
	}
	printf("\ntable: glyf\n");
	printf("----------------\n");

	int i;
	for (i = 0; i < glyf->num_glyphs; i++) {
		TTF_Glyph *glyph = &glyf->glyphs[i];
		print_glyph(glyph);
	}
}

void print_head_table(head_Table *head) {
	if (!head) {
		return;
	}
	printf("\ntable: head\n");
	printf("----------------\n");

	printf("version: 0x%08x\n", head->version);
	printf("fontRevision: 0x%08x\n", head->font_revision);
	printf("checkSumAdjustment: 0x%08x\n", head->check_sum_adjustment);
	printf("magicNumber: 0x%08x\n", head->magic_number);
	printf("flags: 0x%04hx\n", head->flags);
	printf("unitsPerEm: %hu\n", head->units_per_em);
	printf("created: %016ld\n", head->created);
	printf("modified: %016ld\n", head->modified);
	printf("xMin: %hd\n", head->x_min);
	printf("yMin: %hd\n", head->y_min);
	printf("xMax: %hd\n", head->x_max);
	printf("yMax: %hd\n", head->y_max);
	printf("macStyle: %hu\n", head->mac_style);
	printf("lowestRecPPEM: %hu\n", head->lowest_rec_ppem);
	printf("fontDirectionHint: %hd\n", head->font_direction_hint);
	printf("indexToLocFormat: %hd\n", head->index_to_loc_format);
	printf("glyphDataFormat: %hd\n", head->glyph_data_format);
}

void print_hhea_table(hhea_Table *hhea) {
	if (!hhea) {
		return;
	}
	printf("\ntable: hhea\n");
	printf("----------------\n");

	printf("version: 0x%08x\n", hhea->version);
	printf("ascent: %hd\n", hhea->ascent);
	printf("descent: %hd\n", hhea->descent);
	printf("lineGap: %hd\n", hhea->line_gap);
	printf("advanceWidthMax: %hu\n", hhea->advance_width_max);
	printf("minLeftSideBearing: %hd\n", hhea->min_left_side_bearing);
	printf("minRightSideBearing: %hd\n", hhea->min_right_side_bearing);
	printf("xMaxExtent: %hd\n", hhea->x_max_extent);
	printf("caretSlopeRise: %hd\n", hhea->caret_slope_rise);
	printf("caretSlopeRun: %hd\n", hhea->caret_slope_run);
	printf("caretOffset: %hd\n", hhea->caret_offset);
	printf("reserved: %hd\n", hhea->reserved1);
	printf("reserved: %hd\n", hhea->reserved2);
	printf("reserved: %hd\n", hhea->reserved3);
	printf("reserved: %hd\n", hhea->reserved4);
	printf("metricDataFormat: %hd\n", hhea->metric_data_format);
	printf("numOfLongHorMetrics: %hu\n", hhea->num_of_long_hor_metrics);
}

void print_table(TTF_Table *table) {
	if (!table) {
		return;
	}
	switch (table->tag) {
		case 0x70616d63:	/* cmap */
			print_cmap_table(&table->data.cmap);
			break;
		case 0x66796c67:	/* glyf */
			print_glyf_table(&table->data.glyf);
			break;
		case 0x64616568:	/* head */
			print_head_table(&table->data.head);
			break;
		case 0x61656868:	/* hhea */
			print_hhea_table(&table->data.hhea);
		default:
			break;
	}
}

void print_font_dir(TTF_Font *font) {
	if (!font) {
		return;
	}
	printf("scaler type: 0x%08x\n", font->scaler_type);
	printf("numTables: %hu\n", font->num_tables);
	printf("searchRange: %hu\n", font->search_range);
	printf("entrySelector: %hu\n", font->entry_selector);
	printf("rangeShift: %hu\n", font->range_shift);

	if (font->tables) {
		printf("\nTag   Checksum    Offset      Length\n");
		printf("----  ----------  ----------  --------\n");
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = &font->tables[i];
			if (!table) {
				continue;
			}
			printf("%.*s  0x%08x  0x%08X  %8d\n",
					TAG_LENGTH, (char *)&(table->tag),
					table->check_sum, table->offset, table->length);
		}
	}
}

void print_font(TTF_Font *font) {
	if (!font) {
		return;
	}
	print_font_dir(font);

	if (font->tables) {
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = &font->tables[i];
			if (!table) {
				continue;
			}
			print_table(table);
		}
	}
}
