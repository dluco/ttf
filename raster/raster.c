#include "raster.h"
#include "scale.h"
#include "scan.h"
#include "bitmap.h"
#include "../glyph/glyph.h"
#include "../glyph/outline.h"
#include "../tables/tables.h"
#include "../utils/utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <stdio.h>

int raster_init(TTF_Font *font, uint16_t point, uint16_t dpi, uint32_t flags) {
	CHECKPTR(font);

	font->point = point;
	font->dpi = dpi;

	head_Table *head = get_head_table(font);
	CHECKPTR(head);
	// Copy units per em from head table
	font->upem = head->units_per_em;

	// Calculate pixel per em (ppem)
	font->ppem = (font->point * font->dpi) / 72;

	font->raster_flags = flags;

	return SUCCESS;
}

int draw_string(TTF_Font *font, TTF_Bitmap *canvas, int x, int y, const char *string) {
	CHECKPTR(font);
	CHECKPTR(canvas);
	CHECKPTR(string);

	RETINIT(SUCCESS);

	CHECKFAIL(IN(x, 0, canvas->w-1), warn("failed to draw string out of bounds"));
	CHECKFAIL(IN(y, 0, canvas->h-1), warn("failed to draw string out of bounds"));

	for (int i = 0; i < (int)strlen(string); i++) {
		TTF_Glyph *glyph = get_glyph(font, string[i]);
		if (!glyph) {
			warn("failed to get glyph for '%c'", string[i]);
			continue;
		}
		draw_glyph(font, canvas, glyph, x, y);

		/* Move x forward by the glyph's advance width. */
		x += roundf(funit_to_pixel(font, get_glyph_advance_width(font, glyph)));
	}

	RET;
}

int draw_glyph(TTF_Font *font, TTF_Bitmap *canvas, TTF_Glyph *glyph, int x, int y) {
	CHECKPTR(font);
	CHECKPTR(canvas);
	CHECKPTR(glyph);

	RETINIT(SUCCESS);

	CHECKFAIL(IN(x, 0, canvas->w-1), warn("failed to draw glyph out of bounds"));
	CHECKFAIL(IN(y, 0, canvas->h-1), warn("failed to draw glyph out of bounds"));

	/* Prepare glyph for rendering. */
	raster_glyph(font, glyph);

	if (glyph->number_of_contours == 0) {
		/* Glyph has no outline - don't draw anything. */
		return SUCCESS;
	} else if (!glyph->bitmap) {
		warn("failed to draw empty glyph");
		return FAILURE;
	}

	int16_t lsb = roundf(funit_to_pixel(font, get_glyph_left_side_bearing(font, glyph)));
	int16_t ascent = 0;

	// Position glyph baseline at y
	if (glyph->outline) {
		if (font->raster_flags & RENDER_FPAA) {
			ascent = roundf(glyph->outline->y_max / 2);
		} else {
			ascent = roundf(glyph->outline->y_max);
		}
	}

	// Draw glyph bitmap onto canvas
	draw_bitmap(canvas, glyph->bitmap, x + lsb, y - ascent);

	RET;
}

int raster_glyph(TTF_Font *font, TTF_Glyph *glyph) {
	CHECKPTR(font);
	CHECKPTR(glyph);

	if (!scale_glyph(font, glyph)) {
		warn("failed to scale glyph");
		return FAILURE;
	}
	if (!scan_glyph(font, glyph)) {
		warn("failed to scan glyph");
		return FAILURE;
	}

	return SUCCESS;
}

TTF_Bitmap *render_glyph(TTF_Glyph *glyph) {
	TTF_Bitmap *bitmap = NULL;
	TTF_Outline *outline = NULL;
	int w, h;

	if (!glyph) {
		return NULL;
	}

	// Get glyph's outline representation
	if (!glyph->outline) {
		load_glyph_outline(glyph);
	}
	outline = glyph->outline;
	if (!outline) {
		warn("failed to create glyph outline");
		return NULL; // FIXME: release
	}

	// Normalize all segment coordinates to bounding box and flip vertically
	int i;
	for (i = 0; i < outline->num_contours; i++) {
		TTF_Contour *contour = &outline->contours[i];
		int j;
		for (j = 0; j < contour->num_segments; j++) {
			TTF_Segment *segment = &contour->segments[j];
			int k;
			for (k = 0; k < segment->num_points; k++) {
				segment->x[k] = (segment->x[k] - outline->x_min);
				segment->y[k] = (outline->y_max - outline->y_min) - (segment->y[k] - outline->y_min);
			}
		}
	}

	// Calculate required size for bitmap
	w = outline->x_max - outline->x_min + 1;
	h = outline->y_max - outline->y_min + 1;

	// Create bitmap and render outline
	bitmap = create_bitmap(w, h, 0x000000);
	if (!bitmap) {
		warn("failed to create bitmap for glyph");
		return NULL; // FIXME: release
	}
	render_outline(bitmap, outline, 0xFFFFFF);

	return bitmap;
}

int render_outline(TTF_Bitmap *bitmap, TTF_Outline *outline, uint32_t c) {
	if (!bitmap || !outline) {
		return 0;
	}

	int i;
	for (i = 0; i < outline->num_contours; i++) {
		TTF_Contour *contour = &outline->contours[i];
		int j;
		for (j = 0; j < contour->num_segments; j++) {
			TTF_Segment *segment = &contour->segments[j];
			switch (segment->type) {
				case LINE_SEGMENT:
					render_line(bitmap, segment, c);
					break;
				case CURVE_SEGMENT:
					render_curve(bitmap, segment, c);
					break;
				default:
					// Do nothing
					break;
			}
		}
	}

	return 1;
}

int render_line(TTF_Bitmap *bitmap, TTF_Segment *line, uint32_t c) {
	if (!bitmap || !line) {
		return 0;
	}
	int dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;
	int x0 = line->x[0], y0 = line->y[0], x1 = line->x[1], y1 = line->y[1];

	// Handle simple cases first
	if (x0 == x1) {
		// Draw vertical line
		if (y0 < y1) {
			for (y = y0; y < y1; y++) {
				bitmap_set(bitmap, x0, y, c);
			}
		} else {
			for (y = y1; y < y0; y++) {
				bitmap_set(bitmap, x0, y, c);
			}
		}
		return 1;
	} else if (y0 == y1) {
		// Draw horizontal line
		if (x0 < x1) {
			for (x = x0; x < x1; x++) {
				bitmap_set(bitmap, x, y0, c);
			}
		} else {
			for (x = x1; x < x0; x++) {
				bitmap_set(bitmap, x, y0, c);
			}
		}
		return 1;
	}

	dx = x1 - x0;
	dy = y1 - y0;
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = (dx > 0) ? 1 : -1;
	sdy = (dy > 0) ? 1 : -1;
	x = dyabs >> 1;
	y = dxabs >> 1;
	px = x0;
	py = y0;

	if (dxabs >= dyabs) {
		// Draw along x
		int i;
		for (i = 0; i < dxabs; i++) {
			y += dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;

			bitmap_set(bitmap, px, py, c);
		}
	} else {
		// Draw along y
		int i;
		for (i = 0; i < dyabs; i++) {
			x += dxabs;
			if (x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
			py += sdy;

			bitmap_set(bitmap, px, py, c);
		}
	}

	return 1;
}

// Number of control points is n+1
static inline int bezier(float mu, float *p, int n) {
	int k, kn, nn, nkn;
	float blend, muk, munk;
	int b = 0;

	muk = 1;
	munk = pow(1 - mu, (float)n);

	for (k = 0; k <= n; k++) {
		nn = n;
		kn = k;
		nkn = n - k;
		blend = muk * munk;
		muk *= mu;
		munk /= (1 - mu);
		while (nn >= 1) {
			blend *= nn;
			nn--;
			if (kn > 1) {
				blend /= (float) kn;
				kn--;
			}
			if (nkn > 1) {
				blend /= (float) nkn;
				nkn--;
			}
		}
		b += p[k] * blend;
	}

	return b;
}

int render_curve(TTF_Bitmap *bitmap, TTF_Segment *curve, uint32_t c) {
	if (!bitmap || !curve) {
		return 0;
	}
	TTF_Segment line;
	init_segment(&line, 2);

	int n_steps = 100;
	float t, step = 1.0/n_steps;
	int i, x, y, xp = curve->x[0], yp = curve->y[0];
	for (i = 0, t = step; i < n_steps; i++, t += step, xp = x, yp = y) {
		x = bezier(t, curve->x, curve->num_points-1);
		y = bezier(t, curve->y, curve->num_points-1);

		line.x[0] = xp;
		line.y[0] = yp;
		line.x[1] = x;
		line.y[1] = y;

		render_line(bitmap, &line, c);
	}

	free_segment(&line);

	return 1;
}
