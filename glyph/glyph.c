#include "glyph.h"
#include "../tables/tables.h"
#include "../utils/utils.h"
#include <stdlib.h>

uint32_t get_glyph_index(TTF_Font *font, int16_t c) {
	if (!font) {
		return -1;
	}
	// Get cmap subtables
	cmap_Table *cmap = get_cmap_table(font);
	if (!cmap) {
		warn("failed to get cmap table");
		return -1;
	}

	int i;
	for (i = 0; i < cmap->num_subtables; i++) {
		cmap_subTable *subtable = &cmap->subtables[i];
		if (!subtable->glyph_index_array || c < 0 || c >= subtable->num_indices) {
			continue;
		}
		return subtable->glyph_index_array[c];
	}

	return -1;
}

TTF_Glyph *get_glyph(TTF_Font *font, int16_t c) {
	if (!font) {
		return NULL;
	}
	// Get glyf table
	glyf_Table *glyf = get_glyf_table(font);
	if (!glyf) {
		warn("failed to get glyf table");
		return NULL;
	}

	// Lookup glyph index in cmap table
	uint32_t glyph_index = get_glyph_index(font, c);
	if (glyph_index < glyf->num_glyphs) {
		return &glyf->glyphs[glyph_index];
	}

	return NULL;
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
