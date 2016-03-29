#include "ttf.h"
#include "render.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define WRAP(x, start, end) ((((x) - (start)) % ((end) - (start) + 1)) + (start))

void free_segment(TTF_Segment *segment) {
	if (!segment) {
		return;
	}
	switch (segment->type) {
		default:
		case LINE:
			break;
		case CURVE:
			if (segment->data.curve.x) {
				free(segment->data.curve.x);
			}
			if (segment->data.curve.y) {
				free(segment->data.curve.y);
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

TTF_Outline *glyph_to_outline(TTF_Glyph *glyph) {
	if (!glyph) {
		return NULL;
	}

	TTF_Outline *outline = (TTF_Outline *) malloc(sizeof(*outline));
	if (!outline) {
		warnerr("failed to alloc glyph outline");
		return NULL;
	}

	outline->num_contours = glyph->number_of_contours;

	outline->contours = (TTF_Contour *) malloc(outline->num_contours * sizeof(*outline->contours));
	if (!outline->contours) {
		warnerr("failed to alloc outline contours");
		free_outline(outline);
		return NULL;
	}

	printf("num contours = %hd\n", glyph->number_of_contours);
	printf("num points = %hd\n", glyph->num_points);

	printf("bounding box = (%hd, %hd, %hd, %hd)\n", glyph->x_min, glyph->y_min, glyph->x_max, glyph->y_max);

	int i;
	for (i = 0; i < glyph->number_of_contours; i++) {
		TTF_Contour *contour = &outline->contours[i];
		uint16_t start_pt = (i > 0) ? glyph->end_pts_of_contours[i-1] + 1 : 0;
		uint16_t end_pt = glyph->end_pts_of_contours[i];
	
		uint16_t n_pts = (end_pt - start_pt) + 1;

		printf("start_pt = %hu\n", start_pt);
		printf("end_pt = %hu\n", end_pt);
		printf("n_pts = %hu\n", n_pts);

		contour->num_segments = 0;

		// Count the number of segments in the contour (number of on-curve points)
		uint16_t j;
		for (j = start_pt; j <= end_pt; j++) {
			if ((glyph->flags[j] & ON_CURVE) != 0) {
				contour->num_segments++;
			}
		}

		printf("num segments = %hd\n", contour->num_segments);

		contour->segments = (TTF_Segment *) malloc(contour->num_segments * sizeof(*contour->segments));
		if (!contour->segments) {
			warnerr("failed to alloc contour segments");
			free_outline(outline);
			return NULL;
		}

		uint16_t seg_start = start_pt;

		int seg_index;
		for (seg_index = 0; seg_index < contour->num_segments; seg_index++) {
			TTF_Segment *segment = &contour->segments[seg_index];

			if ((glyph->flags[seg_start] & ON_CURVE) == 0) {
				warn("contour segment starts with off-curve point");
			}

			// Check if next point is on- or off-curve
			if ((glyph->flags[WRAP(seg_start+1, start_pt, end_pt)] & ON_CURVE) != 0) {
				// Segment is a line
				segment->type = LINE;

				TTF_Line *line = &segment->data.line;

				line->x[0] = glyph->x_coordinates[seg_start];
				line->y[0] = glyph->y_coordinates[seg_start];
				line->x[1] = glyph->x_coordinates[WRAP(seg_start+1, start_pt, end_pt)];
				line->y[1] = glyph->y_coordinates[WRAP(seg_start+1, start_pt, end_pt)];

				// Next segment starts at endpoint of current segment (line)
				seg_start++;
			} else {
				// Segment is a curve
				segment->type = CURVE;

				TTF_Curve *curve = &segment->data.curve;

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
				if (WRAP(j, start_pt, end_pt) == start_pt) {
					// Curve wraps around to start_pt
					curve->num_points++;
				}

				curve->x = (int16_t *) malloc(curve->num_points * sizeof(*curve->x));
				if (!curve->x) {
					warn("failed to alloc curve x-coordinates");
					free_outline(outline);
					return NULL;
				}

				curve->y = (int16_t *) malloc(curve->num_points * sizeof(*curve->y));
				if (!curve->y) {
					warn("failed to alloc curve y-coordinates");
					free_outline(outline);
					return NULL;
				}

				for (j = 0; j < curve->num_points; j++) {
					curve->x[j] = glyph->x_coordinates[WRAP(seg_start+j, start_pt, end_pt)];
					curve->y[j] = glyph->y_coordinates[WRAP(seg_start+j, start_pt, end_pt)];
				}

				// Next segment starts at endpoint of current segment (curve)
				seg_start += segment->data.curve.num_points - 1;
			}
		}
	}

	return outline;
}

TTF_Bitmap *create_bitmap(int w, int h) {
	TTF_Bitmap *bitmap = (TTF_Bitmap *) malloc(sizeof(*bitmap));
	if (!bitmap) {
		warn("failed to alloc bitmap");
		return NULL;
	}

	bitmap->w = w;
	bitmap->h = h;
	bitmap->data = (uint32_t *) malloc((bitmap->w * bitmap->h) * sizeof(*bitmap->data));
	if (!bitmap->data) {
		warn("failed to alloc bitmap data");
		free(bitmap);
		return NULL;
	}

	return bitmap;
}
