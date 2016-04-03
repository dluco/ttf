#include "ttf.h"
#include "render.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <png.h>

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

TTF_Bitmap *render_glyph(TTF_Glyph *glyph) {
	TTF_Bitmap *bitmap = NULL;
	TTF_Outline *outline = NULL;
	int w, h;

	if (!glyph) {
		return NULL;
	}

	// Get glyph's outline representation
	outline = glyph_to_outline(glyph);
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
						TTF_Line *line = &segment->data.line;
						int k;
						for (k = 0; k < 2; k++) {
							line->x[k] = (line->x[k] - glyph->x_min);
							line->y[k] = (glyph->y_max - glyph->y_min) - (line->y[k] - glyph->y_min);
						}
					}
					break;
				case CURVE:
					{
						TTF_Curve *curve = &segment->data.curve;
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

	free_outline(outline);

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
					render_line(bitmap, &segment->data.line, c);
					break;
				case CURVE:
					render_curve(bitmap, &segment->data.curve, c);
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

TTF_Bitmap *create_bitmap(int w, int h, uint32_t c) {
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

	int i;
	for (i = 0; i < (bitmap->w * bitmap->h); i++) {
		bitmap->data[i] = c;
	}

	return bitmap;
}

void free_bitmap(TTF_Bitmap *bitmap) {
	if (!bitmap) {
		return;
	}
	if (bitmap->data) {
		free(bitmap->data);
	}
	free(bitmap);
}

void bitmap_set(TTF_Bitmap *bitmap, int x, int y, uint32_t c) {
	if (!bitmap || x < 0 || x > bitmap->w || y < 0 || y > bitmap->h) {
		return;
	}
	bitmap->data[y*bitmap->w + x] = c;
}

uint32_t bitmap_get(TTF_Bitmap *bitmap, int x, int y) {
	if (!bitmap || x < 0 || x > bitmap->w || y < 0 || y > bitmap->h) {
		return 0;
	}
	return bitmap->data[y*bitmap->w + x];
}

static inline void set_rgb(png_byte *ptr, uint32_t value) {
	uint8_t *b = (uint8_t *)&value;
	ptr[0] = b[2]; ptr[1] = b[1]; ptr[2] = b[0];
}

int save_bitmap(TTF_Bitmap *bitmap, const char *filename, const char *title) {
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte *row = NULL;

	RETINIT(SUCCESS);

	CHECKPTR(bitmap);

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	CHECKFAIL(fp, warnerr("failed to open file '%s' for saving bitmap", filename));

	// Initialize the write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	CHECKFAIL(png_ptr, warn("failed to alloc png write struct"));

	// Initialize the info structure
	info_ptr = png_create_info_struct(png_ptr);
	CHECKFAIL(info_ptr, warn("failed to alloc png info struct"));

	// Setup libpng exception handling
	CHECKFAIL(setjmp(png_jmpbuf(png_ptr)) == 0, warn("error occurred during png creation"));

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, bitmap->w, bitmap->h, 8,
			PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set png title
	if (title) {
		png_text png_title;
		png_title.compression = PNG_TEXT_COMPRESSION_NONE;
		png_title.key = (png_charp) "Title";
		png_title.text = (png_charp) title;
		png_set_text(png_ptr, info_ptr, &png_title, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (RBG = 3 bytes / pixel)
	row = (png_byte *) malloc((bitmap->w * 3) * sizeof(*row));
	CHECKFAIL(row, warnerr("failed to alloc png row"));

	// Write image data
	int x, y;
	for (y = 0; y < bitmap->h; y++) {
		for (x = 0; x < bitmap->w; x++) {
			set_rgb(&(row[x*3]), bitmap->data[y*bitmap->w + x]);
		}
		png_write_row(png_ptr, (png_bytep)row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	RETURN(
		if (fp) fclose(fp);
		if (info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		if (png_ptr) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		if (row) free(row);
	);
}
