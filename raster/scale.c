#include "scale.h"
#include "config.h"
#include "../utils/utils.h"

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
