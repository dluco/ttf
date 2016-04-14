#include "font.h"
#include "consts.h"
#include "../tables/tables.h"
#include "../parse/parse.h"
#include "../utils/utils.h"
#include <stdlib.h>

TTF_Font *load_font(const char *filename) {
	TTF_Font *font = (TTF_Font*) malloc(sizeof(TTF_Font));
	if (!font) {
		warnerr("failed to alloc font");
		return NULL;
	}

	if (!init_font(font)) {
		warn("failed to init font");
	}

	if (!parse_file(font, filename)) {
		warn("failed to parse font file");
		free_font(font);
		return NULL;
	}

	return font;
}

int init_font(TTF_Font *font) {
	CHECKPTR(font);

	font->fd = -1;

	font->num_tables = 0;
	font->tables = NULL;

	/* rasterizer is uninitialized */
	font->point = -1;
	font->dpi = 0;
	font->ppem = 0;
	font->upem = 0;

	return SUCCESS;
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

int16_t get_font_ascent(TTF_Font *font) {
	if (!font) {
		return 0;
	}
	hhea_Table *hhea = get_hhea_table(font);
	if (!hhea) {
		warn("failed to get font ascent");
		return 0;
	}
	return hhea->ascent;
}

int16_t get_font_descent(TTF_Font *font) {
	if (!font) {
		return 0;
	}
	hhea_Table *hhea = get_hhea_table(font);
	if (!hhea) {
		warn("failed to get font descent");
		return 0;
	}
	return hhea->descent;
}
