#ifndef RASTER_H
#define RASTER_H

#include "../base/types.h"

typedef enum _Raster_Opts {
	RENDER_FP		=	1 << 0,
	RENDER_FPAA		=	1 << 1,
	RENDER_ASPAA	=	1 << 2,
} Raster_Opts;

int raster_init(TTF_Font *font, uint16_t point, uint16_t dpi, uint32_t flags);

TTF_Bitmap *render_glyph(TTF_Glyph *glyph);
int render_outline(TTF_Bitmap *bitmap, TTF_Outline *outline, uint32_t c);
int render_line(TTF_Bitmap *bitmap, TTF_Segment *line, uint32_t c);
int render_curve(TTF_Bitmap *bitmap, TTF_Segment *curve, uint32_t c);

#endif /* RASTER_H */
