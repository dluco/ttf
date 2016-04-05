#ifndef GLYPH_H
#define GLYPH_H

#include "../base/types.h"

uint32_t get_glyph_index(TTF_Font *font, int16_t c);
TTF_Glyph *get_glyph(TTF_Font *font, int16_t c);
void free_glyph(TTF_Glyph *glyph);

#endif /* GLYPH_H */
