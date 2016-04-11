#ifndef OUTLINE_H
#define OUTLINE_H

#include "../base/types.h"

int load_glyph_outline(TTF_Glyph *glyph);

int init_segment(TTF_Segment *segment, int num_points);

void free_segment(TTF_Segment *segment);
void free_contour(TTF_Contour *contour);
void free_outline(TTF_Outline *outline);

#endif /* OUTLINE_H */
