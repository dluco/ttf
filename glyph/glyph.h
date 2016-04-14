#ifndef GLYPH_H
#define GLYPH_H

#include "../base/types.h"

int32_t get_glyph_index(TTF_Font *font, int16_t c);
TTF_Glyph *get_glyph(TTF_Font *font, int16_t c);
uint16_t get_glyph_advance_width(TTF_Font *font, TTF_Glyph *glyph);
int16_t get_glyph_left_side_bearing(TTF_Font *font, TTF_Glyph *glyph);
void free_glyph(TTF_Glyph *glyph);

#endif /* GLYPH_H */
