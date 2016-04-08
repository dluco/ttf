#include "outline.h"
#include "../utils/utils.h"
#include <stdlib.h>

void free_segment(TTF_Segment *segment) {
	if (!segment) {
		return;
	}
	switch (segment->type) {
		default:
		case LINE:
			break;
		case CURVE:
			if (segment->curve.x) {
				free(segment->curve.x);
			}
			if (segment->curve.y) {
				free(segment->curve.y);
			}
			break;
	}
}

void free_contour(TTF_Contour *contour) {
	if (!contour) {
		return;
	}
	if (contour->segments) {
		int i;
		for (i = 0; i < contour->num_segments; i++) {
			free_segment(&contour->segments[i]);
		}
		free(contour->segments);
	}
}

void free_outline(TTF_Outline *outline) {
	if (!outline) {
		return;
	}
	if (outline->contours) {
		int i;
		for (i = 0; i < outline->num_contours; i++) {
			free_contour(&outline->contours[i]);
		}
		free(outline->contours);
	}
	free(outline);
}

static inline uint16_t wrap(uint16_t x, uint16_t start, uint16_t end) {
	return ((x - start) % (end - start + 1)) + start;
}

int load_glyph_outline(TTF_Glyph *glyph) {
	TTF_Outline *outline = NULL;

	RETINIT(SUCCESS);

	CHECKPTR(glyph);

	outline = (TTF_Outline *) malloc(sizeof(*outline));
	CHECKFAIL(outline, warnerr("failed to alloc glyph outline"));

	glyph->outline = outline;

	outline->num_contours = glyph->number_of_contours;

	outline->contours = (TTF_Contour *) malloc(outline->num_contours * sizeof(*outline->contours));
	CHECKFAIL(outline->contours, warnerr("failed to alloc outline contours"));

	outline->x_min = glyph->x_min;
	outline->y_min = glyph->y_min;
	outline->x_max = glyph->x_max;
	outline->y_max = glyph->y_max;

	int i;
	for (i = 0; i < glyph->number_of_contours; i++) {
		TTF_Contour *contour = &outline->contours[i];
		uint16_t start_pt = (i > 0) ? glyph->end_pts_of_contours[i-1] + 1 : 0;
		uint16_t end_pt = glyph->end_pts_of_contours[i];
	
		contour->num_segments = 0;

		// Count the number of segments in the contour (number of on-curve points)
		uint16_t j;
		for (j = start_pt; j <= end_pt; j++) {
			if ((glyph->flags[j] & ON_CURVE) != 0) {
				contour->num_segments++;
			}
		}

		contour->segments = (TTF_Segment *) malloc(contour->num_segments * sizeof(*contour->segments));
		CHECKFAIL(contour->segments, warnerr("failed to alloc contour segments"));

		uint16_t seg_start = start_pt;

		int seg_index;
		for (seg_index = 0; seg_index < contour->num_segments; seg_index++) {
			TTF_Segment *segment = &contour->segments[seg_index];

			if ((glyph->flags[seg_start] & ON_CURVE) == 0) {
				warn("contour segment starts with off-curve point");
			}

			// Check if next point is on- or off-curve
			if ((glyph->flags[wrap(seg_start+1, start_pt, end_pt)] & ON_CURVE) != 0) {
				// Segment is a line
				segment->type = LINE;

				TTF_Line *line = &segment->line;

				line->x[0] = glyph->x_coordinates[seg_start];
				line->y[0] = glyph->y_coordinates[seg_start];
				line->x[1] = glyph->x_coordinates[wrap(seg_start+1, start_pt, end_pt)];
				line->y[1] = glyph->y_coordinates[wrap(seg_start+1, start_pt, end_pt)];

				// Next segment starts at endpoint of current segment (line)
				seg_start++;
			} else {
				// Segment is a curve
				segment->type = CURVE;

				TTF_Curve *curve = &segment->curve;

				// Include first point (seg_start)
				curve->num_points = 1;

				// Measure number of points in curve, up to and including the next on-curve point
				uint16_t j;
				for (j = seg_start+1; j <= end_pt; j++) {
					curve->num_points++;
					if ((glyph->flags[j] & ON_CURVE) != 0) {
						break;
					}
				}
				if (wrap(j, start_pt, end_pt) == start_pt) {
					// Curve wraps around to start_pt
					curve->num_points++;
				}

				curve->x = malloc(curve->num_points * sizeof(*curve->x));
				CHECKFAIL(curve->x, warn("failed to alloc curve x-coordinates"));

				curve->y = malloc(curve->num_points * sizeof(*curve->y));
				CHECKFAIL(curve->y, warn("failed to alloc curve y-coordinates"));

				for (j = 0; j < curve->num_points; j++) {
					curve->x[j] = glyph->x_coordinates[wrap(seg_start+j, start_pt, end_pt)];
					curve->y[j] = glyph->y_coordinates[wrap(seg_start+j, start_pt, end_pt)];
				}

				// Next segment starts at endpoint of current segment (curve)
				seg_start += segment->curve.num_points - 1;
			}
		}
	}

	// Outline is unscaled
	outline->point = -1;

	RETURN(
			free_outline(outline),
			PASS
	);
}
