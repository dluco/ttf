#ifndef SCALE_H
#define SCALE_H

#include "../base/consts.h"
#include "../base/types.h"

int scale_glyph(TTF_Font *font, TTF_Glyph *glyph);

float funit_to_pixel(TTF_Font *font, int16_t funit);
int16_t pixel_to_funit(TTF_Font *font, float pixel);

float round_pixel(float pixel);

#endif /* SCALE_H */
