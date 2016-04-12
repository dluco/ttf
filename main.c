#include "ttf.h"
#include "raster/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char *filename = NULL;
	if (argc != 2) {
		// Default
		filename = (char *) "./data/Vera.ttf";
	} else {
		filename = argv[1];
	}

	TTF_Font *font = load_font(filename);

	raster_init(font, 12, DPI, RENDER_ASPAA);

	TTF_Bitmap *out = NULL, *tmp = NULL;
	char *string = "m";
	for (int i = 0; i < (int)strlen(string); i++) {
		TTF_Glyph *glyph = get_glyph(font, string[i]);
		if (!glyph) {
			printf("failed to get glyph for '%c'\n", string[i]);
		} else {
			scale_glyph(font, glyph);
			scan_glyph(font, glyph);

			if (glyph->bitmap) {
				if (!out) {
					out = copy_bitmap(glyph->bitmap);
				} else {
					tmp = combine_bitmaps(out, glyph->bitmap, 0xFFFFFF);
					free_bitmap(out);
					out = tmp;
				}
			}
		}
	}

	if (out) {
		save_bitmap(out, "data/output.png", NULL);
		free_bitmap(out);
	}

//	TTF_Glyph *glyph = get_glyph(font, 'W');
//	if (!glyph) {
//		printf("failed to get glyph\n");
//	} else {
//		raster_init(font, 1000, DPI);
//
//		scale_glyph(font, glyph);
//		scan_glyph(font, glyph);
//
//		if (glyph->bitmap) {
//			save_bitmap(glyph->bitmap, "data/output.png", NULL);
//		}

//		TTF_Bitmap *bitmap = render_glyph(glyph);
//		if (bitmap) {
//			save_bitmap(bitmap, "data/output.png", NULL);
//			free_bitmap(bitmap);
//		}
//	}

	free_font(font);
	return EXIT_SUCCESS;
}
