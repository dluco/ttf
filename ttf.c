#include "ttf.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

// from: http://two.pairlist.net/pipermail/reportlab-users/2003-October/002329.html

// Spike TrueType/OpenType subsetting

// Data Type     Description
// ------------- -------------------------------------------------------------
// BYTE          8-bit unsigned integer.
// CHAR          8-bit signed integer.
// USHORT        16-bit unsigned integer.
// SHORT         16-bit signed integer.
// ULONG         32-bit unsigned integer.
// LONG          32-bit signed integer.
// Fixed         32-bit signed fixed-point number (16.16)
// FUNIT         Smallest measurable distance in the em space.
// F2DOT14       16-bit signed fixed number with the low 14 bits of fraction (2.14).
// LONGDATETIME  Date represented in number of seconds since 12:00 midnight, January 1, 1904. The value is represented as a signed 64-bit integer.
// Tag           Array of four uint8s (length = 32 bits) used to identify a script, language system, feature, or baseline
// GlyphID       Glyph index number, same as uint16(length = 16 bits)
// Offset        Offset to a table, same as uint16 (length = 16 bits), NULL offset = 0x0000
//
// NOTE: All numbers are big-endian

// Font file begins with an offset table:
//   Fixed       sfnt version    0x00010000 for TrueType outlines, 'OTTO' for OpenType with CFF outlines (not relevant here)
//   USHORT      numTables       number of tables
//   USHORT      searchRange     16 * max(2^n <= numTables)
//   USHORT      entrySelector   max(n: 2^n <= numTables)
//   USHORT      rangeShift      numTables * 16 - searchRange
//   ------------------------------ (12 bytes)
// Table directory follows.  Each entry is 12 bytes.  Entries are sorted by
// tag in lexicographical order.  Offsets are from the start of the font file.
// Entry format:
//   ULONG       tag             4-byte identifier
//   ULONG       checkSum        CheckSum for this table
//   ULONG       offset          Offset from beginning of font file
//   ULONG       length          length of this table

// Checksum calculation:
//   ULONG CalcTableChecksum(ULONG *Table, ULONG Length) {
//   	ULONG Sum = 0L;
//   	ULONG *Endptr = Table+((Length+3) & ~3) / sizeof(ULONG);
//
//   	while (Table < EndPtr)
//      	Sum += *Table++;
//       	return Sum;
//   }
//
// Note: This function implies that the length of a table must be a multiple of
// four bytes. In fact, a font is not considered structurally proper without the
// correct padding. All tables must begin on four byte boundries, and any
// remaining space between tables is padded with zeros. The length of all tables
// should be recorded in the table directory with their actual length (not their
// padded length).

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
	if ((int32_t)b[3] < 0) {
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

	font->tables = (TTF_Table **) malloc(font->num_tables * sizeof(TTF_Table*));
	if (!font->tables) {
		fprintf(stderr, "failed to alloc font tables: %s\n", strerror(errno));
		return 0;
	}

	// Read the table directory entries
	int i;
	for (i = 0; i < font->num_tables; i++) {
		TTF_Table *table = (TTF_Table *) malloc(sizeof(TTF_Table));
		if (!table) {
			fprintf(stderr, "failed to alloc font table: %s\n", strerror(errno));
			return 0;
		}
		font->tables[i] = table;

		table->tag = read_tag(font->fd);
		table->check_sum = read_ulong(font->fd);
		table->offset = read_ulong(font->fd);
		table->length = read_ulong(font->fd);
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
		TTF_Table *table = font->tables[i];
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
	cmap_mappingTable *mapping_table = NULL;
	// Seek to start of mapping subtable
	if (lseek(font->fd, subtable->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to cmap mapping subtable: %s\n", strerror(errno));
		return 0;
	}
	mapping_table = &subtable->mapping_table;

	// TODO
	uint16_t format = read_ushort(font->fd);
	switch (format) {
		case 0:
			printf("format 0!!!\n");

			mapping_table->format_0.format = format;
			mapping_table->format_0.length = read_ushort(font->fd);
			mapping_table->format_0.language = read_ushort(font->fd);
			int i;
			for (i = 0; i < 256; i++) {
				mapping_table->format_0.glyph_index_array[i] = read_byte(font->fd);
			}
			break;
		case 2:
			printf("format 2!!!\n");
			break;
		case 4:
			printf("format 4!!!\n");
			break;
		default:
			break;
	}

	return 1;
}

int load_cmap_table(TTF_Font *font, TTF_Table *table) {
	cmap_Table *cmap = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to cmap table: %s\n", strerror(errno));
		return 0;
	}
	cmap = &table->data.cmap;

	cmap->version = read_ushort(font->fd);
	cmap->num_subtables = read_ushort(font->fd);

	cmap->subtables = (cmap_subTable **) malloc(cmap->num_subtables * sizeof(cmap_subTable*));
	if (!cmap->subtables) {
		fprintf(stderr, "failed to alloc cmap subtables: %s\n", strerror(errno));
		return 0;
	}

	off_t pos;
	int i;
	for (i = 0; i < cmap->num_subtables; i++) {
		cmap_subTable *subtable = (cmap_subTable *) malloc(sizeof(cmap_subTable));
		if (!subtable) {
			fprintf(stderr, "failed to alloc cmap subtable: %s\n", strerror(errno));
			return 0;
		}
		cmap->subtables[i] = subtable;

		subtable->platform_id = read_ushort(font->fd);
		subtable->platform_specifid_id = read_ushort(font->fd);
		subtable->offset = read_ulong(font->fd);

		// Save current file position
		if ((pos = lseek(font->fd, 0, SEEK_CUR)) < 0) {
			fprintf(stderr, "failed to save file position: %s\n", strerror(errno));
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

int load_glyph(TTF_Font *font, Glyph *glyph) {
	glyph->number_of_contours = read_short(font->fd);
	glyph->x_min = read_short(font->fd);
	glyph->y_min = read_short(font->fd);
	glyph->x_max = read_short(font->fd);
	glyph->y_max = read_short(font->fd);

	return 1;
}

int load_glyf_table(TTF_Font *font, TTF_Table *table) {
	glyf_Table *glyf = NULL;
	maxp_Table *maxp = NULL;
	loca_Table *loca = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to glyf table: %s\n", strerror(errno));
		return 0;
	}
	glyf = &table->data.glyf;

	// Get maxp and loca tables
	TTF_Table *temp = get_table(font, s_to_tag("maxp"));
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;
	temp = get_table(font, s_to_tag("loca"));
	if (!temp) {
		fprintf(stderr, "failed to get loca table\n");
		return 0;
	}
	loca = &temp->data.loca;

	glyf->glyphs = (Glyph *) malloc(maxp->num_glyphs * sizeof(Glyph));
	if (!glyf->glyphs) {
		fprintf(stderr, "failed to alloc glyphs: %s\n", strerror(errno));
		return 0;
	}

	int i;
	for (i = 0; i < maxp->num_glyphs; i++) {
		Glyph *glyph = &glyf->glyphs[i];
		if (lseek(font->fd, table->offset + loca->offsets[i], SEEK_SET) < 0) {
			fprintf(stderr, "failed to seek to glyph %d: %s\n", i, strerror(errno));
			return 0;
		}
		load_glyph(font, glyph);
	}

	return 1;
}

int load_head_table(TTF_Font *font, TTF_Table *table) {
	head_Table *head = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to head table: %s\n", strerror(errno));
		return 0;
	}
	head = &table->data.head;

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
	hhea_Table *hhea = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to hhea table: %s\n", strerror(errno));
		return 0;
	}
	hhea = &table->data.hhea;

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
	loca_Table *loca = NULL;
	head_Table *head = NULL;
	maxp_Table *maxp = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to loca table: %s\n", strerror(errno));
		return 0;
	}
	loca = &table->data.loca;

	// Get head and maxp tables
	TTF_Table *temp = get_table(font, s_to_tag("head"));
	if (!temp) {
		fprintf(stderr, "failed to get head table\n");
		return 0;
	}
	head = &temp->data.head;
	temp = get_table(font, s_to_tag("maxp"));
	if (!temp) {
		fprintf(stderr, "failed to get maxp table\n");
		return 0;
	}
	maxp = &temp->data.maxp;

	loca->offsets = (uint32_t *) malloc((maxp->num_glyphs + 1) * sizeof(uint32_t));
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
	maxp_Table *maxp = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to maxp table: %s\n", strerror(errno));
		return 0;
	}
	maxp = &table->data.maxp;

	maxp->version = read_fixed(font->fd);
	maxp->num_glyphs = read_ushort(font->fd);
	maxp->max_points = read_ushort(font->fd);
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
	post_Table *post = NULL;
	maxp_Table *maxp = NULL;
	// Seek to start of table
	if (lseek(font->fd, table->offset, SEEK_SET) < 0) {
		fprintf(stderr, "failed to seek to post table: %s\n", strerror(errno));
		return 0;
	}
	post = &table->data.post;

	// Get maxp table
	TTF_Table *temp = get_table(font, s_to_tag("maxp"));
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

	// TODO: handle different formats, need MacRomanEncoding

	return 1;
}

int load_tables(TTF_Font *font) {
	int i;
	for (i = 0; i < font->num_tables; i++) {
		TTF_Table *table = font->tables[i];
		if (!table) {
			continue;
		}
		// TODO: seek to start of table here, since it happens for *every* table
		//
		// TODO: use an array of function ptrs to dispatch the loading
		// 		array = {
		//			int tag, fptr load(), fptr free(),
		//			...
		// 		}
		switch (table->tag) {
			case 0x322f534f:	/* OS/2 */
				break;
			case 0x544c4350:	/* PCLT */
				break;
			case 0x70616d63:	/* cmap */
				load_cmap_table(font, table);
				break;
			case 0x20747663:	/* cvt  */
				break;
			case 0x6d677066:	/* fpgm */
				break;
			case 0x70736167:	/* gasp */
				break;
			case 0x66796c67:	/* glyf */
				load_glyf_table(font, table);
				break;
			case 0x786d6468:	/* hdmx */
				break;
			case 0x64616568:	/* head */
				load_head_table(font, table);
				break;
			case 0x61656868:	/* hhea */
				load_hhea_table(font, table);
				break;
			case 0x78746d68:	/* hmtx */
				break;
			case 0x6e72656b:	/* kern */
				break;
			case 0x61636f6c:	/* loca */
				load_loca_table(font, table);
				break;
			case 0x7078616d:	/* maxp */
				load_maxp_table(font, table);
				break;
			case 0x656d616e:	/* name */
				break;
			case 0x74736f70:	/* post */
				break;
			case 0x70657270:	/* prep */
				break;
			default:
				fprintf(stderr, "unknown font table type '%.*s'\n", TAG_LENGTH, (char *)&(table->tag));
				break;
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
			cmap_subTable *subtable = cmap->subtables[i];
			if (!subtable) {
				continue;
			}
			free(subtable);
		}
		free(cmap->subtables);
	}
}

void free_glyf_table(glyf_Table *glyf) {
	if (!glyf) {
		return;
	}
	if (glyf->glyphs) {
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

void free_table(TTF_Table *table) {
	if (!table) {
		return;
	}
	switch (table->tag) {
		case 0x70616d63:	/* cmap */
			free_cmap_table(&table->data.cmap);
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
		default:
			break;
	}
	free(table);
}

void free_font(TTF_Font *font) {
	if (!font) {
		return;
	}
	if (font->tables) {
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = font->tables[i];
			free_table(table);
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
			TTF_Table *table = font->tables[i];
			if (table != NULL && table->tag == tag) {
				return table;
			}
		}
	}

	return NULL;
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

void debug_print_cmap_table(cmap_Table *cmap) {
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
			cmap_subTable *subtable = cmap->subtables[i];
			if (!subtable) {
				continue;
			}
			printf("platformID: %hu\n", subtable->platform_id);
			printf("platformSpecificID: %hu\n", subtable->platform_specifid_id);
			printf("offset: %08X\n", subtable->offset);
		}
	}
}

void debug_print_head_table(head_Table *head) {
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

void debug_print_hhea_table(hhea_Table *hhea) {
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

void debug_print_table(TTF_Table *table) {
	if (!table) {
		return;
	}
	switch (table->tag) {
		case 0x70616d63:	/* cmap */
			debug_print_cmap_table(&table->data.cmap);
			break;
		case 0x64616568:	/* head */
			debug_print_head_table(&table->data.head);
			break;
		case 0x61656868:	/* hhea */
			debug_print_hhea_table(&table->data.hhea);
		default:
			break;
	}
}

void debug_print_font_dir(TTF_Font *font) {
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
			TTF_Table *table = font->tables[i];
			if (!table) {
				continue;
			}
			printf("%.*s  0x%08x  0x%08X  %8d\n",
					TAG_LENGTH, (char *)&(table->tag),
					table->check_sum, table->offset, table->length);
		}
	}
}

void debug_print_font(TTF_Font *font) {
	if (!font) {
		return;
	}
	debug_print_font_dir(font);

	if (font->tables) {
		int i;
		for (i = 0; i < font->num_tables; i++) {
			TTF_Table *table = font->tables[i];
			if (!table) {
				continue;
			}
			debug_print_table(table);
		}
	}
}
