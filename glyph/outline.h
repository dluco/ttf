#ifndef OUTLINE_H
#define OUTLINE_H

#include "../base/types.h"

void free_segment(TTF_Segment *segment);
void free_contour(TTF_Contour *contour);
void free_outline(TTF_Outline *outline);

int load_glyph_outline(TTF_Glyph *glyph);

#endif /* OUTLINE_H */
