#ifndef TTF_H
#define TTF_H

#include <stdint.h>

union _cmap_mappingTable;
struct _cmap_subTable;
struct _cmap_Table;
struct _TTF_Glyph;
struct _glyf_Table;
struct _head_Table;
struct _hhea_Table;
struct _maxp_Table;
struct _post_Table;

typedef struct _cmap_format_0 {
	uint16_t format;
	uint16_t length;
	uint16_t language;
	uint8_t glyph_index_array[256];
} cmap_format_0;

typedef struct _subheader {
	uint16_t first_code;
	uint16_t entry_count;
	int16_t id_delta;
	uint16_t id_range_offset;
} subheader;

typedef struct _cmap_format_2 {
	uint16_t format;
	uint16_t length;
	uint16_t language;
	uint16_t sub_header_keys[256];
	subheader *sub_headers;
	uint16_t glyph_index_array[256];
} cmap_format_2;

typedef struct _cmap_format_4 {
	uint16_t format;
	uint16_t length;
	uint16_t language;
	uint16_t seg_count_x2;
	uint16_t search_range;
	uint16_t entry_selector;
	uint16_t range_shift;
	uint16_t *start_code;
	uint16_t *id_delta;
	uint16_t *id_range_offset;
	uint16_t *glyph_index_array;
} cmap_format_4;

typedef union _cmap_mappingTable {
	uint16_t format;
	cmap_format_0 format_0;
	cmap_format_2 format_2;
	cmap_format_4 format_4;
} cmap_mappingTable;

typedef struct _cmap_subTable {
	uint16_t platform_id;
	uint16_t platform_specifid_id;
	uint32_t offset;

	uint32_t format;
	uint32_t length;
	uint32_t language;

	uint32_t *glyph_index_array;

	cmap_mappingTable mapping_table;
} cmap_subTable;

typedef struct _cmap_Table {
	uint16_t version;
	uint16_t num_subtables;

	cmap_subTable *subtables;
} cmap_Table;

typedef struct _TTF_Glyph {
	int16_t number_of_contours;
	int16_t x_min;
	int16_t y_min;
	int16_t x_max;
	int16_t y_max;

	uint16_t *end_pts_of_contours;
	uint16_t instruction_length;
	uint8_t *instructions;
	uint8_t *flags;
	int16_t *x_coordinates;
	int16_t *y_coordinates;

	int16_t num_points;

	// TODO: Compound glyphs
} TTF_Glyph;

typedef struct _glyf_Table {
	TTF_Glyph *glyphs;

	uint16_t num_glyphs; // Copied from maxp table
} glyf_Table;

typedef struct _head_Table {
	uint32_t version;
	uint32_t font_revision;
	uint32_t check_sum_adjustment;
	uint32_t magic_number;
	uint16_t flags;
	uint16_t units_per_em;
	int64_t created;
	int64_t modified;
	int16_t x_min;
	int16_t y_min;
	int16_t x_max;
	int16_t y_max;
	uint16_t mac_style;
	uint16_t lowest_rec_ppem;
	int16_t font_direction_hint;
	int16_t index_to_loc_format;
	int16_t glyph_data_format;
} head_Table;

typedef struct _hhea_Table {
	uint32_t version;
	int16_t ascent;
	int16_t descent;
	int16_t line_gap;
	uint16_t advance_width_max;
	int16_t min_left_side_bearing;
	int16_t min_right_side_bearing;
	int16_t x_max_extent;
	int16_t caret_slope_rise;
	int16_t caret_slope_run;
	int16_t caret_offset;
	int16_t reserved1;
	int16_t reserved2;
	int16_t reserved3;
	int16_t reserved4;
	int16_t metric_data_format;
	uint16_t num_of_long_hor_metrics;
} hhea_Table;

#define SHORT_OFFSETS	0
#define LONG_OFFSETS	1

typedef struct _loca_Table {
	uint32_t *offsets;
} loca_Table;

typedef struct _maxp_Table {
	uint32_t version;
	uint16_t num_glyphs;
	uint16_t max_points;
	uint16_t max_contours;
	uint16_t max_component_points;
	uint16_t max_component_contours;
	uint16_t max_zones;
	uint16_t max_twilight_points;
	uint16_t max_storage;
	uint16_t max_function_defs;
	uint16_t max_instruction_defs;
	uint16_t max_stack_elements;
	uint16_t max_size_of_instructions;
	uint16_t max_component_elements;
	uint16_t max_component_depth;
} maxp_Table;

typedef struct _post_Table {
	uint32_t format;
	uint32_t italic_angle;
	int16_t underline_position;
	int16_t underline_thickness;
	uint32_t is_fixed_pitch;
	uint32_t min_mem_type_42;
	uint32_t max_mem_type_42;
	uint32_t min_mem_type_1;
	uint32_t max_mem_type_1;

	char *glyph_names[];
} post_Table;

typedef struct _TTF_Table {
	uint32_t tag;
	uint32_t check_sum;
	uint32_t offset;
	uint32_t length;

	union {
		cmap_Table cmap;
		glyf_Table glyf;
		head_Table head;
		hhea_Table hhea;
		loca_Table loca;
		maxp_Table maxp;
		post_Table post;
	} data;
} TTF_Table;

typedef struct _TTF_Font {
	uint32_t scaler_type;
	uint16_t num_tables;
	uint16_t search_range;
	uint16_t entry_selector;
	uint16_t range_shift;

	int fd;
	TTF_Table *tables;
} TTF_Font;

#define TAG_LENGTH	4

uint8_t read_byte(int fd);
uint16_t read_hword(int fd);
uint32_t read_word(int fd);
uint64_t read_dword(int fd);
uint16_t read_ushort(int fd);
int16_t read_short(int fd);
uint32_t read_ulong(int fd);
uint32_t read_fixed(int fd);
uint32_t read_tag(int fd);
int64_t read_longdatetime(int fd);

float fixed_to_float(uint32_t fixed);
uint32_t s_to_tag(const char *s);

uint32_t calc_table_check_sum(uint32_t *data, uint32_t length);
int validate_check_sums(TTF_Font *font);

int read_font_dir(TTF_Font *font);

int read_table_raw(TTF_Font *font, TTF_Table *table, uint32_t *buf);
int load_tables(TTF_Font *font);

void free_table(TTF_Table *table);
void free_font(TTF_Font *font);

TTF_Table *get_table(TTF_Font *font, uint32_t tag);
TTF_Font *parse_file(const char *filename);

void print_font_dir(TTF_Font *font);
void print_cmap_table(cmap_Table *cmap);
void print_head_table(head_Table *head);
void print_hhea_table(hhea_Table *hhea);
void print_table(TTF_Table *table);
void print_font(TTF_Font *font);

#endif /* TTF_H */
