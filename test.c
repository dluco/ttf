#include "ttf.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char *filename = NULL;

	if (argc != 2) {
		filename = (char *) "Vera.ttf";
//		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
//		exit(EXIT_FAILURE);
	} else {
		filename = argv[1];
	}
	TTF_Font *font = parse_file(filename);

	TTF_Glyph *glyph = get_glyph(font, 'A');
	if (!glyph) {
		printf("failed to get glyph\n");
	} else {
		TTF_Outline *outline = glyph_to_outline(glyph);

		free_outline(outline);
	}

	TTF_Bitmap *bitmap = create_bitmap(100, 100, 0x000000);
	save_bitmap(bitmap, "output.png", "Test");
	free_bitmap(bitmap);

//	print_font(font);

	free_font(font);
	return EXIT_SUCCESS;
}
