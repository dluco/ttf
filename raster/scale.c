#include "scale.h"
#include "config.h"
#include "../tables/tables.h"
#include "../utils/utils.h"

int scale_font(TTF_Font *font, uint16_t point, uint16_t dpi) {
	CHECKPTR(font);

	font->point = point;
	font->dpi = dpi;

	head_Table *head = get_head_table(font);
	CHECKPTR(head);
	// Copy units per em from head table
	head->units_per_em = font->upem;

	// Calculate pixel per em (ppem)
	font->ppem = (font->point * font->dpi) / 72;

	return SUCCESS;
}

int scale_glyph(TTF_Font *font, TTF_Glyph *glyph) {
	CHECKPTR(font);
	CHECKPTR(glyph);

	return SUCCESS;
}

int16_t funit_to_pixel(TTF_Font *font, int16_t funit) {
	return funit * (font->ppem / font->upem);
}

int16_t pixel_to_funit(TTF_Font *font, int16_t pixel) {
	return pixel * (font->upem / font->ppem);
}
