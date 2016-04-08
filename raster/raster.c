#include "raster.h"
#include "bitmap.h"
#include "../glyph/outline.h"
#include "../tables/tables.h"
#include "../utils/utils.h"
#include <stdlib.h>
#include <math.h>

int raster_init(TTF_Font *font, uint16_t point, uint16_t dpi) {
	CHECKPTR(font);

	font->point = point;
	font->dpi = dpi;

	head_Table *head = get_head_table(font);
	CHECKPTR(head);
	// Copy units per em from head table
	font->upem = head->units_per_em;

	// Calculate pixel per em (ppem)
	font->ppem = (font->point * font->dpi) / 72;

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
			switch (segment->type) {
				case LINE:
					{
						TTF_Line *line = &segment->line;
						int k;
						for (k = 0; k < 2; k++) {
							line->x[k] = (line->x[k] - glyph->x_min);
							line->y[k] = (glyph->y_max - glyph->y_min) - (line->y[k] - glyph->y_min);
						}
					}
					break;
				case CURVE:
					{
						TTF_Curve *curve = &segment->curve;
						int k;
						for (k = 0; k < curve->num_points; k++) {
							curve->x[k] = (curve->x[k] - glyph->x_min);
							curve->y[k] = (glyph->y_max - glyph->y_min) - (curve->y[k] - glyph->y_min);
						}
					}
					break;
				default:
					// Do nothing
					break;
			}
		}
	}

	// Calculate required size for bitmap
	w = glyph->x_max - glyph->x_min + 1;
	h = glyph->y_max - glyph->y_min + 1;

	// Create bitmap and render outline
	bitmap = create_bitmap(w, h, 0x000000);
	if (!bitmap) {
		warn("failed to create bitmap for glyph");
		return NULL; // FIXME: release
	}
	render_outline(bitmap, outline, 0xFFFFFF);

//	free_outline(outline);

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
				case LINE:
					render_line(bitmap, &segment->line, c);
					break;
				case CURVE:
					render_curve(bitmap, &segment->curve, c);
					break;
				default:
					// Do nothing
					break;
			}
		}
	}

	return 1;
}

int render_line(TTF_Bitmap *bitmap, TTF_Line *line, uint32_t c) {
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
static inline int bezier(float mu, int16_t *p, int n) {
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

int render_curve(TTF_Bitmap *bitmap, TTF_Curve *curve, uint32_t c) {
	if (!bitmap || !curve) {
		return 0;
	}
	TTF_Line line;
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

	return 1;
}
