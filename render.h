#ifndef RENDER_H
#define RENDER_H

#include "ttf.h"
#include <stdint.h>

void free_segment(TTF_Segment *segment);
void free_contour(TTF_Contour *contour);
void free_outline(TTF_Outline *outline);

TTF_Outline *glyph_to_outline(TTF_Glyph *glyph);

TTF_Bitmap *render_glyph(TTF_Glyph *glyph);
int render_outline(TTF_Bitmap *bitmap, TTF_Outline *outline, uint32_t c);
int render_line(TTF_Bitmap *bitmap, TTF_Line *line, uint32_t c);
int render_curve(TTF_Bitmap *bitmap, TTF_Curve *curve, uint32_t c);

TTF_Bitmap *create_bitmap(int w, int h, uint32_t);
void free_bitmap(TTF_Bitmap *bitmap);

void bitmap_set(TTF_Bitmap *bitmap, int x, int y, uint32_t c);
uint32_t bitmap_get(TTF_Bitmap *bitmap, int x, int y);

int save_bitmap(TTF_Bitmap *bitmap, const char *filename, const char *title);

#endif /* RENDER_H */
