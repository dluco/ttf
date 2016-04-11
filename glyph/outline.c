#include "outline.h"
#include "../utils/utils.h"
#include <stdlib.h>

static inline uint16_t wrap(uint16_t x, uint16_t start, uint16_t end) {
	return ((x - start) % (end - start + 1)) + start;
}

static int load_glyph_contour(TTF_Glyph *glyph, TTF_Contour *contour, uint16_t start_pt, uint16_t end_pt) {
	CHECKPTR(glyph);
	CHECKPTR(contour);

	RETINIT(SUCCESS);

	contour->num_segments = 0;

	/* Count the number of segments in the contour:
	 * number of on-curve points + interpolated on-curve points */
	uint16_t i;
	for (i = start_pt; i <= end_pt; i++) {
		if ((glyph->flags[i] & ON_CURVE)) {
			contour->num_segments++;
		} else if (i > start_pt && !(glyph->flags[i-1] & ON_CURVE)) {
			/* Points j and j-1 are both off-curve,
			 * interpolate an on-curve point between them. */
			contour->num_segments++;
		}
	}

	contour->segments = malloc(contour->num_segments * sizeof(*contour->segments));
	CHECKFAIL(contour->segments, warnerr("failed to alloc contour segments"));

	uint16_t seg_start = start_pt;
	uint16_t seg_index;
	for (seg_index = 0; seg_index < contour->num_segments; seg_index++) {
		TTF_Segment *segment = &contour->segments[seg_index];

		if ((glyph->flags[wrap(seg_start+1, start_pt, end_pt)] & ON_CURVE)) {
			/* Segment is a line */
			segment->type = LINE_SEGMENT;

			init_segment(segment, 2);

			uint16_t j;
			for (j = 0; j < 2; j++) {
				segment->x[j] = glyph->x_coordinates[wrap(seg_start+j, start_pt, end_pt)];
				segment->y[j] = glyph->y_coordinates[wrap(seg_start+j, start_pt, end_pt)];
			}

			/* Advance seg_start to end point of line. */
			seg_start++;
		} else {
			/* Segment is a curve */
			segment->type = CURVE_SEGMENT;

			init_segment(segment, 3);

			uint16_t j;
			for (j = 0; j < 2; j++) {
				segment->x[j] = glyph->x_coordinates[wrap(seg_start+j, start_pt, end_pt)];
				segment->y[j] = glyph->y_coordinates[wrap(seg_start+j, start_pt, end_pt)];
			}

			/* (Loop to decompose quadratic B-splines into quadratic Bezier curves.) */
			while (seg_start <= end_pt) {
				/* If the next point is on-curve, this curve ends. */
				if ((glyph->flags[wrap(seg_start+2, start_pt, end_pt)] & ON_CURVE)) {
					segment->x[2] = glyph->x_coordinates[wrap(seg_start+2, start_pt, end_pt)];
					segment->y[2] = glyph->y_coordinates[wrap(seg_start+2, start_pt, end_pt)];

					seg_start += 2;

					break;
				}

				/* Interpolate an on-curve point halfway between the current curve point,
				 * 	point 2 of 3, and the next off-curve point. */
				uint16_t p1 = wrap(seg_start+1, start_pt, end_pt);
				uint16_t p2 = wrap(seg_start+2, start_pt, end_pt);

				segment->x[2] = ((float)(glyph->x_coordinates[p1] + glyph->x_coordinates[p2])) / 2;
				segment->y[2] = ((float)(glyph->y_coordinates[p1] + glyph->y_coordinates[p2])) / 2;

				 /* The current curve is now complete. */

				/* Adjust seg_start to be the middle (off-curve) point of the curve. */
				seg_start++;

				if (seg_index+1 < contour->num_segments) {
					/* A new curve will start at the old curve's interpolated on-curve point. */
					segment = &contour->segments[++seg_index];

					segment->type = CURVE_SEGMENT;

					init_segment(segment, 3);

					segment->x[0] = contour->segments[seg_index-1].x[2];
					segment->y[0] = contour->segments[seg_index-1].y[2];

					/* The curve's second point is the off-curve point that appears after the 
					 * previous curve's off-curve point. */
					segment->x[1] = glyph->x_coordinates[wrap(seg_start+1, start_pt, end_pt)];
					segment->y[1] = glyph->y_coordinates[wrap(seg_start+1, start_pt, end_pt)];

					/* This is now the same situation as before. That is, the current curve will end
					 * if the next point is on-curve, and if it is off-curve, another endpoint will
					 * need to be interpolated as its endpoint. */
				}
			}
		}
	}

	RET;
}

int load_glyph_outline(TTF_Glyph *glyph) {
	CHECKPTR(glyph);

	RETINIT(SUCCESS);

	TTF_Outline *outline = malloc(sizeof(*outline));
	CHECKFAIL(outline, warnerr("failed to alloc glyph outline"));

	glyph->outline = outline;

	outline->num_contours = glyph->number_of_contours;

	outline->contours = malloc(outline->num_contours * sizeof(*outline->contours));
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

		load_glyph_contour(glyph, contour, start_pt, end_pt);
	}

	// Outline is unscaled
	outline->point = -1;

	RETFAIL(free_outline(outline));
}

int init_segment(TTF_Segment *segment, int num_points) {
	CHECKPTR(segment);

	RETINIT(SUCCESS);

	segment->x = malloc(num_points * sizeof(*segment->x));
	CHECKFAIL(segment->x, warnerr("failed to alloc segment x-coordinates"));

	segment->y = malloc(num_points *sizeof(*segment->y));
	CHECKFAIL(segment->y, warnerr("failed to alloc segment y-coordinates"));

	segment->num_points = num_points;

	RETFAIL(free_segment(segment));
}

void free_segment(TTF_Segment *segment) {
	if (!segment) {
		return;
	}
	if (segment->x) {
		free(segment->x);
	}
	if (segment->y) {
		free(segment->y);
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
