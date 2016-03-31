#ifndef RENDER_H
#define RENDER_H

#include "ttf.h"
#include <stdint.h>

void free_segment(TTF_Segment *segment);
void free_contour(TTF_Contour *contour);
void free_outline(TTF_Outline *outline);

TTF_Outline *glyph_to_outline(TTF_Glyph *glyph);

TTF_Bitmap *create_bitmap(int w, int h, uint32_t);
void free_bitmap(TTF_Bitmap *bitmap);
int save_bitmap(TTF_Bitmap *bitmap, const char *filename, const char *title);

#endif /* RENDER_H */
