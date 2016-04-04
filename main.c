#include "ttf.h"
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

	TTF_Font *font = parse_file(filename);

	TTF_Glyph *glyph = get_glyph(font, 'G');
	if (!glyph) {
		printf("failed to get glyph\n");
	} else {
		TTF_Bitmap *bitmap = render_glyph(glyph);
		if (bitmap) {
			save_bitmap(bitmap, "data/output.png", NULL);
			free_bitmap(bitmap);
		}
	}

	free_font(font);
	return EXIT_SUCCESS;
}
