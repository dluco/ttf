#include "utils.h"
#include "../glyph/glyph.h"
#include "../tables/tables.h"
#include "../raster/scale.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

int mod(int a, int b) {
	return ((a & b) + b) % b;
}

float symroundf(float f) {
	return (f < 0) ? floorf(f) : ceilf(f);
}

void warn(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
}

void warnerr(const char *fmt, ...) {
	int errno_save = errno;
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (errno_save != 0) {
		fprintf(stderr, ": %s\n", strerror(errno_save));
	} else {
		fprintf(stderr, "\n");
	}
}

int get_text_width(TTF_Font *font, const char *text) {
	if (!font || !text) {
		return 0;
	}

	hmtx_Table *hmtx = get_hmtx_table(font);
	if (!hmtx) {
		warn("failed to calculate text width");
		return 0;
	}

	int width = 0;
	/* Get advance width of each character's glyph. */
	for (int i = 0; i < (int)strlen(text); i++) {
		int32_t glyph_index = get_glyph_index(font, text[i]);
		if (glyph_index < 0) {
			continue;
		}
		width += funit_to_pixel(font, hmtx->advance_width[glyph_index]);
	}

	return width;
}
