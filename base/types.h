#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef struct _cmap_subTable {
	uint16_t platform_id;
	uint16_t platform_specifid_id;
	uint32_t offset;

	uint32_t format;
	uint32_t length;
	uint32_t language;

	uint16_t num_indices;
	uint32_t *glyph_index_array;
} cmap_subTable;

typedef struct _cmap_Table {
	uint16_t version;
	uint16_t num_subtables;

	cmap_subTable *subtables;
} cmap_Table;

typedef struct _cvt_Table {
	int16_t *control_values;
} cvt_Table;

typedef struct _TTF_Segment {
	int type;
	float *x, *y;
	int16_t num_points;
} TTF_Segment;

typedef struct _TTF_Contour {
	TTF_Segment *segments;
	int16_t num_segments;
} TTF_Contour;

typedef struct _TTF_Outline {
	TTF_Contour *contours;
	int16_t num_contours;
	float x_min;	/* Scaled outline bounds */
	float y_min;
	float x_max;
	float y_max;

	int16_t point;	/* Size outline is scaled to. < 0 if unscaled. */
} TTF_Outline;

typedef struct _TTF_Bitmap {
	int w, h;
	uint32_t *data;
	uint32_t c;
} TTF_Bitmap;

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
	uint32_t index;

	TTF_Outline *outline;
	TTF_Bitmap *bitmap;
} TTF_Glyph;

typedef struct _glyf_Table {
	TTF_Glyph *glyphs;

	uint16_t num_glyphs; /* Copied from maxp table. */
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

typedef struct _hmtx_Table {
	uint16_t *advance_width;
	int16_t *left_side_bearing;
	int16_t *non_horizontal_left_side_bearing;

	uint16_t num_h_metrics;
	uint16_t num_non_horizontal_metrics;
} hmtx_Table;

typedef struct _loca_Table {
	uint32_t *offsets;
	uint16_t num_offsets;
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

	uint16_t num_glyphs; /* Read from post table - must match maxp */
	char **glyph_names;
} post_Table;

typedef struct _TTF_Table {
	uint32_t tag;
	uint32_t check_sum;
	uint32_t offset;
	uint32_t length;

	uint8_t status;
	union {
		cmap_Table cmap;
		cvt_Table cvt;
		glyf_Table glyf;
		head_Table head;
		hhea_Table hhea;
		hmtx_Table hmtx;
		loca_Table loca;
		maxp_Table maxp;
		post_Table post;
	} data;
} TTF_Table;

typedef struct _TTF_Font {
	int fd;

	uint32_t scaler_type;
	uint16_t num_tables;
	uint16_t search_range;
	uint16_t entry_selector;
	uint16_t range_shift;

	TTF_Table *tables;

	int16_t point;
	uint16_t dpi;
	uint16_t ppem;
	uint16_t upem;
	uint32_t raster_flags;
} TTF_Font;

#endif /* TYPES_H */
