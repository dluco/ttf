#include "ttf.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char* argv[]) {
	// Default
	char *filename = "./data/Vera.ttf";
	char *string = NULL;
	if (argc > 1) {
		string = argv[1];
	} else {
		string = "m";
	}

	TTF_Font *font = load_font(filename);
	raster_init(font, 12, 96, RENDER_ASPAA);

	/* Calculate required size for output bitmap. */
	int16_t ascent = funit_to_pixel(font, get_font_ascent(font));
	int16_t descent = fabsf(funit_to_pixel(font, get_font_descent(font)));
	int text_width = get_text_width(font, string);
	int padding = 10;

	TTF_Bitmap *out = create_bitmap(text_width + 2*padding, (ascent + descent) + 2*padding, 0xFFFFFF);

	draw_string(font, out, (out->w - text_width)/2, (out->h - (ascent + descent))/2 + ascent, string);

	if (out) {
		save_bitmap(out, "data/output.png", NULL);
		free_bitmap(out);
	}

	free_font(font);
	return EXIT_SUCCESS;
}
