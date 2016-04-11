#include "scale.h"
#include "config.h"
#include "../glyph/outline.h"
#include "../utils/utils.h"
#include <math.h>

#include <stdio.h>

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
			int k;
			for (k = 0; k < segment->num_points; k++) {
				segment->x[k] = funit_to_pixel(font, segment->x[k]);
				segment->y[k] = funit_to_pixel(font, segment->y[k]);
			}
		}
	}

	printf("unscaled outline bounding box: (%f, %f, %f, %f)\n",
			outline->x_min, outline->y_min, outline->x_max, outline->y_max);

	/* Round outline bounding box to nearest whole pixel value */
	outline->x_min = symroundf(funit_to_pixel(font, outline->x_min));
	outline->y_min = symroundf(funit_to_pixel(font, outline->y_min));
	outline->x_max = symroundf(funit_to_pixel(font, outline->x_max));
	outline->y_max = symroundf(funit_to_pixel(font, outline->y_max));

	printf("scaled outline bounding box: (%f, %f, %f, %f)\n",
			outline->x_min, outline->y_min, outline->x_max, outline->y_max);

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

float funit_to_pixel(TTF_Font *font, int16_t funit) {
	return round_pixel(((float)(funit * font->ppem)) / font->upem);
}

int16_t pixel_to_funit(TTF_Font *font, float pixel) {
	return (pixel * font->upem) / font->ppem;
}

/**
 * Round a pixel coordinate to the nearest
 * sixty-fourth of a pixel.
 */
float round_pixel(float pixel) {
	return roundf(pixel * 64) / 64;
}
