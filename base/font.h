#ifndef FONT_H
#define FONT_H

#include "types.h"

TTF_Font *load_font(const char *filename);

int init_font(TTF_Font *font);
void free_font(TTF_Font *font);

int16_t get_font_ascent(TTF_Font *font);
int16_t get_font_descent(TTF_Font *font);

#endif /* FONT_H */
