#include "ttf.h"
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

#define FONT_FILENAME "data/Vera.ttf"

int main(int argc, char* argv[]) {
	char *font_filename = FONT_FILENAME;
	int render_method = RENDER_FP;
	int apply_gamma = 0;
	float gamma = 1.00;

	int c;
	while ((c = getopt(argc, argv, "f:m:g:")) != -1) {
		switch (c) {
			case 'f':
				font_filename = optarg;
				break;
			case 'm':
				if (strcmp(optarg, "fp") == 0) {
					render_method = RENDER_FP;
				} else if (strcmp(optarg, "fpaa") == 0) {
					render_method = RENDER_FPAA;
				} else if (strcmp(optarg, "aspaa") == 0) {
					render_method = RENDER_ASPAA;
				} else {
					warn("invalid rendering method '%s'", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'g':
				gamma = atof(optarg);
				apply_gamma = 1;
				break;
			default:
				break;
		}
	}

	char *string;
	if (optind < argc) {
		string = argv[optind];
	} else {
		string = "m";
	}

	TTF_Font *font = load_font(font_filename);
	raster_init(font, 12, 96, render_method);

	/* Calculate required size for output bitmap. */
	int16_t ascent = funit_to_pixel(font, get_font_ascent(font));
	int16_t descent = fabsf(funit_to_pixel(font, get_font_descent(font)));
	int text_width = get_text_width(font, string);
	int padding = 10;

	TTF_Bitmap *out = create_bitmap(text_width + 2*padding, (ascent + descent) + 2*padding, 0xFFFFFF);

	draw_string(font, out, (out->w - text_width)/2, (out->h - (ascent + descent))/2 + ascent, string);

	if (apply_gamma) {
		set_bitmap_gamma(out, gamma);
	}

	if (out) {
		save_bitmap(out, "data/output.png", NULL);
		free_bitmap(out);
	}

	free_font(font);
	return EXIT_SUCCESS;
}
