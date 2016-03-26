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

//	print_font(font);

	free_font(font);
	return EXIT_SUCCESS;
}
