#ifndef BITMAP_H
#define BITMAP_H

#include "../base/types.h"

TTF_Bitmap *create_bitmap(int w, int h, uint32_t);
void free_bitmap(TTF_Bitmap *bitmap);

void bitmap_set(TTF_Bitmap *bitmap, int x, int y, uint32_t c);
uint32_t bitmap_get(TTF_Bitmap *bitmap, int x, int y);

TTF_Bitmap *copy_bitmap(TTF_Bitmap *bitmap);
TTF_Bitmap *combine_bitmaps(TTF_Bitmap *a, TTF_Bitmap *b, uint32_t c);

int save_bitmap(TTF_Bitmap *bitmap, const char *filename, const char *title);

#endif /* BITMAP_H */
