#include "ttf.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char *filename = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	filename = argv[1];
	TTF_Font *font = parse_file(filename);

	TTF_Glyph *glyph = get_glyph(font, 'A');
	if (!glyph) {
		printf("failed to get glyph\n");
	} else {
		TTF_Outline *outline = glyph_to_outline(glyph);

		free_outline(outline);
	}

//	print_font(font);

	free_font(font);
	return EXIT_SUCCESS;
}
