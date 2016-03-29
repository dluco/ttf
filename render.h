#ifndef RENDER_H
#define RENDER_H

#include "ttf.h"

void free_segment(TTF_Segment *segment);
void free_contour(TTF_Contour *contour);
void free_outline(TTF_Outline *outline);

TTF_Outline *glyph_to_outline(TTF_Glyph *glyph);

#endif /* RENDER_H */
