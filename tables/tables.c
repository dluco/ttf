#include "tables.h"
#include "../glyph/glyph.h"
#include "../parse/parse.h"
#include <stdlib.h>

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

cmap_Table *get_cmap_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x70616d63);
	return (table) ? &table->data.cmap : NULL;
}

cvt_Table *get_cvt_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x20747663);
	return (table) ? &table->data.cvt : NULL;
}

glyf_Table *get_glyf_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x66796c67);
	return (table) ? &table->data.glyf : NULL;
}

head_Table *get_head_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x64616568);
	return (table) ? &table->data.head : NULL;
}

hhea_Table *get_hhea_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x61656868);
	return (table) ? &table->data.hhea : NULL;
}

hmtx_Table *get_hmtx_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x78746d68);
	return (table) ? &table->data.hmtx : NULL;
}

loca_Table *get_loca_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x61636f6c);
	return (table) ? &table->data.loca : NULL;
}

maxp_Table *get_maxp_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x7078616d);
	return (table) ? &table->data.maxp : NULL;
}

post_Table *get_post_table(TTF_Font *font) {
	if (!font) {
		return NULL;
	}
	TTF_Table *table = get_table(font, 0x74736f70);
	return (table) ? &table->data.post : NULL;
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
		case 0x78746d68:	/* hmtx */
			free_hmtx_table(&table->data.hmtx);
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

void free_hmtx_table(hmtx_Table *hmtx) {
	if (!hmtx) {
		return;
	}
	if (hmtx->advance_width) {
		free(hmtx->advance_width);
	}
	if (hmtx->left_side_bearing) {
		free(hmtx->left_side_bearing);
	}
	if (hmtx->non_horizontal_left_side_bearing) {
		free(hmtx->non_horizontal_left_side_bearing);
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
