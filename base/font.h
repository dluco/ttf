#ifndef FONT_H
#define FONT_H

#include "types.h"

TTF_Font *load_font(const char *filename);

int init_font(TTF_Font *font);
void free_font(TTF_Font *font);

#endif /* FONT_H */
