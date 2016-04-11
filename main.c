#include "ttf.h"
#include "raster/config.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char *filename = NULL;
	if (argc != 2) {
		// Default
		filename = (char *) "data/Vera.ttf";
	} else {
		filename = argv[1];
	}

	TTF_Font *font = load_font(filename);

	TTF_Glyph *glyph = get_glyph(font, 'A');
	if (!glyph) {
		printf("failed to get glyph\n");
	} else {
		raster_init(font, 12, DPI);

		scale_glyph(font, glyph);
		scan_glyph(font, glyph);

		if (glyph->bitmap) {
			save_bitmap(glyph->bitmap, "data/output.png", NULL);
		}

//		TTF_Bitmap *bitmap = render_glyph(glyph);
//		if (bitmap) {
//			save_bitmap(bitmap, "data/output.png", NULL);
//			free_bitmap(bitmap);
//		}
	}

	free_font(font);
	return EXIT_SUCCESS;
}
