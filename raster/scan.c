#include "scan.h"
#include "raster.h"
#include "scale.h"
#include "bitmap.h"
#include "../base/consts.h"
#include "../utils/utils.h"
#include <math.h>
#include <stdlib.h>

#include <stdio.h>

static inline float quad_bezier(float t, float p0, float p1, float p2) {
	return (1-t)*(1-t)*p0 + 2*(1-t)*t*p1 + t*t*p2;
}

static int add_intersection(TTF_Scan_Line *scanline, float x) {
	CHECKPTR(scanline);

	RETINIT(SUCCESS);

	if (scanline->num_intersections+1 > scanline->size_x) {
		/* Increase size of scan-line storage for new intersection. */
		scanline->x = realloc(scanline->x, (scanline->size_x * 2) * sizeof(*scanline->x));
		CHECKFAIL(scanline->x, warnerr("failed to adjust scanline size"));

		scanline->size_x *= 2;
	}

	scanline->x[scanline->num_intersections++] = x;

	RET;
}

static int cmp_intersections(const void *p1, const void *p2) {
	float f1 = *(float *)p1;
	float f2 = *(float *)p2;
	if (f1 < f2) {
		return -1;
	} else if (f1 == f2) {
		return 0;
	} else {
		return 1;
	}
}

static int intersect_line_segment(TTF_Segment *segment, TTF_Scan_Line *scanline) {
	if (!segment || !scanline) {
		return 0;
	}
	float y = scanline->y;
	float x0 = segment->x[0], x1 = segment->x[1];
	float y0 = segment->y[0], y1 = segment->y[1];

	if ((fabsf(y - y0) + fabsf(y - y1) - fabsf(y1 - y0)) != 0) {
		/* The scan-line at y is not within the y-bounds of the line segment. */
		return 0;
	}

	if (y0 == y1) {
		/* Intersecting with horizontal line (degenerate case). */
		if (y == y0) {
			 /* There are two intersections, x0 and x1. */
			add_intersection(scanline, x0);
			add_intersection(scanline, x1);
			return 2;
		} else {
			/* No intersections. */
			return 0;
		}
	}

	/* Calculate x-intersection of scan-line and line segment. */
	float x = ((x0*y1 - x1*y0) + y*(x1 - x0)) / (y1 - y0);

	if ((fabsf(x - x0) + fabsf(x - x1) - fabsf(x1 - x0)) != 0) {
		/* The x-intersection is not within the x-bounds of the line segment. */
		return 0;
	}

	add_intersection(scanline, x);
	return 1;
}

static int intersect_curve_segment(TTF_Segment *segment, TTF_Scan_Line *scanline) {
	if (!segment || !scanline) {
		return 0;
	}
	float t;
	float y = scanline->y;
	float y0 = segment->y[0], y1 = segment->y[1], y2 = segment->y[2];
	float x0 = segment->x[0], x1 = segment->x[1], x2 = segment->x[2];

	float a = y0 - 2*y1 + y2;
	float b = 2*y1 - 2*y0;
	float c = y0 - y;

	/* Handle degenerate cases first. */
	if (a == 0) {
		/* Curve is a straight line. */
		if (b == 0) {
			/* Curve is a horizontal line. */
			if (c == 0) {
				/* There are two intersections, x0 and x2. */
				add_intersection(scanline, x0);
				add_intersection(scanline, x2);
				return 2;
			} else {
				/* No intersections. */
				return 0;
			}
		} else {
			t = -c / b;
			if (IN(t, 0.0, 1.0)) {
				/* One intersection. */
				add_intersection(scanline, quad_bezier(t, x0, x1, x2));
				return 1;
			} else {
				/* No intersections. */
				return 0;
			}
		}
	}

	/* Test discriminant for number of roots. */
	float D = b*b - 4*a*c;
	if (D < 0) {
		/* No roots. */
		return 0;
	} else if (D == 0) {
		/* One distinct root. */
		t = -b / (2*a);
		if (IN(t, 0.0, 1.0)) {
			/* One intersection. */
			add_intersection(scanline, quad_bezier(t, x0, x1, x2));
			return 1;
		} else {
			/* No intersections. */
			return 0;
		}
	} else {
		/* Two distinct roots. */
		int num_intersections = 0;

		t = (-b + sqrt(D)) / (2*a);
		if (IN(t, 0.0, 1.0)) {
			/* Intersection point is within curve endpoints. */
			add_intersection(scanline, quad_bezier(t, x0, x1, x2));
			num_intersections++;
		}

		t = (-b - sqrt(D)) / (2*a);
		if (IN(t, 0.0, 1.0)) {
			/* Intersection point is within curve endpoints. */
			add_intersection(scanline, quad_bezier(t, x0, x1, x2));
			num_intersections++;
		}
		return num_intersections;
	}
}

static int intersect_segment(TTF_Segment *segment, TTF_Scan_Line *scanline) {
	if (!segment || !segment->x || !segment->y) {
		warn("failed to intersect uninitialized contour segment");
		return 0;
	}
	switch (segment->type) {
		case LINE_SEGMENT:
			return intersect_line_segment(segment, scanline);
			break;
		case CURVE_SEGMENT:
			return intersect_curve_segment(segment, scanline);
			break;
		default:
			break;
	}

	return 0;
}

int scan_glyph(TTF_Font *font, TTF_Glyph *glyph) {
	CHECKPTR(font);
	CHECKPTR(glyph);

	RETINIT(SUCCESS);

	if (!glyph->outline) {
		warn("failed to scan uninitialized glyph outline");
		return FAILURE;
	} else if (glyph->outline->point < 0) {
		warn("failed to scan unscaled glyph outline");
		return FAILURE;
	}

	TTF_Outline *outline = glyph->outline;
	TTF_Bitmap *bitmap = NULL;

	if (font->raster_flags & RENDER_FPAA) {
		/* Anti-aliased rendering - oversample outline then downsample. */
		if (!glyph->bitmap) {
			glyph->bitmap = create_bitmap((outline->x_max - outline->x_min) / 2,
					(outline->y_max - outline->y_min) / 2, 0xFFFFFF);
		}

		/* Intermediate oversampled bitmap. */
		bitmap = create_bitmap(outline->x_max - outline->x_min,
				outline->y_max - outline->y_min, 0xFFFFFF);
	} else {
		/* Normal rendering - write directly to glyph bitmap. */
		if (!glyph->bitmap) {
			glyph->bitmap = create_bitmap(outline->x_max - outline->x_min,
					outline->y_max - outline->y_min, 0xFFFFFF);
		}

		bitmap = glyph->bitmap;
	}

	/* Create a scan-line for each row in the scaled outline. */
	int num_scanlines = outline->y_max - outline->y_min;
	TTF_Scan_Line *scanlines = malloc(num_scanlines * sizeof(*scanlines));
	CHECKFAIL(scanlines, warnerr("failed to alloc glyph scan lines"));

	/* Find intersections of each scan-line with contour segments. */
	for (int i = 0; i < num_scanlines; i++) {
		TTF_Scan_Line *scanline = &scanlines[i];
		init_scanline(scanline, outline->num_contours * 2);
		scanline->y = outline->y_max - i;

		for (int j = 0; j < outline->num_contours; j++) {
			TTF_Contour *contour = &outline->contours[j];
			int k;
			for (k = 0; k < contour->num_segments; k++) {
				TTF_Segment *segment = &contour->segments[k];
				intersect_segment(segment, scanline);
			}
		}

		/* Round pixel intersection values to 1/64 of a pixel. */
		for (int j = 0; j < scanline->num_intersections; j++) {
			scanline->x[j] = round_pixel(scanline->x[j]);
		}

		/* Sort intersections from left to right. */
		qsort(scanline->x, scanline->num_intersections, sizeof(*scanline->x), cmp_intersections);

		printf("Intersections: ");
		char *sep = "";
		for (int j = 0; j < scanline->num_intersections; j++) {
			printf("%s%f", sep, scanline->x[j]);
			sep = ", ";
		}
		printf("\n");

		int int_index = 0, fill = 0;
		for (int j = 0; j < bitmap->w; j++) {
			if (int_index < scanline->num_intersections) {
				if ((outline->x_min + j) >= scanline->x[int_index]) {
					fill = !fill;

					/* Skip over duplicate intersections. */
					int k;
					for (k = 1; int_index+k < scanline->num_intersections; k++) {
						if (scanline->x[int_index] != scanline->x[int_index+k]) {
							break;
						}
					}
					int_index += k;
				}
			}
			if (fill) {
				bitmap_set(bitmap, j, i, 0x000000);
			}
		}
	}

	if (font->raster_flags & RENDER_FPAA) {
		/* Downsample intermediate bitmap. */
		for (int y = 0; y < glyph->bitmap->h; y++) {
			for (int x = 0; x < glyph->bitmap->w; x++) {
				/* Calculate pixel coverage:
				 * 	coverage = number filled samples / number samples */
				float coverage = 0;
				if (bitmap_get(bitmap, (2*x), (2*y)) == 0x000000) coverage++;
				if (bitmap_get(bitmap, (2*x)+1, (2*y)) == 0x000000) coverage++;
				if (bitmap_get(bitmap, (2*x), (2*y)+1) == 0x000000) coverage++;
				if (bitmap_get(bitmap, (2*x)+1, (2*y)+1) == 0x000000) coverage++;
				coverage /= 4;
				uint8_t shade = 0xFF*(1-coverage);
				uint32_t pixel = (shade << 16) | (shade << 8) | (shade << 0);

				bitmap_set(glyph->bitmap, x, y, pixel);
			}
		}

		free_bitmap(bitmap);
	}

	RETRELEASE(free_scanlines(scanlines, num_scanlines));
}

int init_scanline(TTF_Scan_Line *scanline, int base_size) {
	CHECKPTR(scanline);

	RETINIT(SUCCESS);

	scanline->x = malloc(base_size * sizeof(*scanline->x));
	CHECKFAIL(scanline->x, warnerr("failed to alloc scanline"));

	scanline->size_x = base_size;
	scanline->num_intersections = 0;

	RETFAIL(free_scanline(scanline));
}

void free_scanline(TTF_Scan_Line *scanline) {
	if (!scanline) {
		return;
	}
	if (scanline->x) {
		free(scanline->x);
	}
}

void free_scanlines(TTF_Scan_Line *scanlines, int num_scanlines) {
	if (!scanlines) {
		return;
	}
	int i;
	for (i = 0; i < num_scanlines; i++) {
		free_scanline(&scanlines[i]);
	}
	free(scanlines);
}
