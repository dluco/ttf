#ifndef SCAN_H
#define SCAN_H

#include "../base/types.h"

typedef struct _TTF_Scan_Line {
	float y;
	float *x;
	int size_x;
	int num_intersections;
} TTF_Scan_Line;

int scan_glyph(TTF_Font *font, TTF_Glyph *glyph);

int init_scanline(TTF_Scan_Line *scanline, int base_size);
void free_scanline(TTF_Scan_Line *scanline);
void free_scanlines(TTF_Scan_Line *scanlines, int num_scanlines);

#endif /* SCAN_H */
