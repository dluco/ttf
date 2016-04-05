#ifndef SCALE_H
#define SCALE_H

#include "../base/consts.h"
#include "../base/types.h"

int scale_font(TTF_Font *font, uint16_t point, uint16_t dpi);

int scale_glyph(TTF_Font *font, TTF_Glyph *glyph);

int16_t funit_to_pixel(TTF_Font *font, int16_t coord);
int16_t pixel_to_funit(TTF_Font *font, int16_t coord);

#endif /* SCALE_H */
