#include "scale.h"
#include "config.h"
#include "../glyph/outline.h"
#include "../utils/utils.h"

static int scale_outline(TTF_Font *font, TTF_Outline *outline) {
	CHECKPTR(outline);

	int i;
	for (i = 0; i < outline->num_contours; i++) {
		TTF_Contour *contour = &outline->contours[i];
		CHECKPTR(contour);
		int j;
		for (j = 0; j < contour->num_segments; j++) {
			TTF_Segment *segment = &contour->segments[j];
			CHECKPTR(segment);
			switch (segment->type) {
				case LINE:
					{
						TTF_Line *line = &segment->line;
						CHECKPTR(line);
						int k;
						for (k = 0; k < 2; k++) {
							funit_to_pixel(font, line->x[k]);
							funit_to_pixel(font, line->y[k]);
						}
					}
					break;
				case CURVE:
					{
						TTF_Curve *curve = &segment->curve;
						CHECKPTR(curve);
						int k;
						for (k = 0; k < curve->num_points; k++) {
							funit_to_pixel(font, curve->x[k]);
							funit_to_pixel(font, curve->y[k]);
						}
					}
					break;
				default:
					break;
			}
		}
	}

	outline->point = font->point;

	return SUCCESS;
}

int scale_glyph(TTF_Font *font, TTF_Glyph *glyph) {
	CHECKPTR(font);
	CHECKPTR(glyph);

	if (font->point < 0) {
		/* raster_init() has not been called yet */
		warn("font rasterizer has not been initialized");
		return FAILURE;
	}
	if (!glyph->outline) {
		load_glyph_outline(glyph);
	} else if (glyph->outline->point == font->point) {
		/* glyph is already scaled */
		return SUCCESS;
	}
	scale_outline(font, glyph->outline);

	return SUCCESS;
}

int16_t funit_to_pixel(TTF_Font *font, int16_t funit) {
	return funit * (font->ppem / font->upem);
}

int16_t pixel_to_funit(TTF_Font *font, int16_t pixel) {
	return pixel * (font->upem / font->ppem);
}
