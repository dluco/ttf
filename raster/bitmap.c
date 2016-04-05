#include "bitmap.h"
#include <stdlib.h>
#include <png.h>

TTF_Bitmap *create_bitmap(int w, int h, uint32_t c) {
	TTF_Bitmap *bitmap = (TTF_Bitmap *) malloc(sizeof(*bitmap));
	if (!bitmap) {
		warn("failed to alloc bitmap");
		return NULL;
	}

	bitmap->w = w;
	bitmap->h = h;
	bitmap->data = (uint32_t *) malloc((bitmap->w * bitmap->h) * sizeof(*bitmap->data));
	if (!bitmap->data) {
		warn("failed to alloc bitmap data");
		free(bitmap);
		return NULL;
	}

	int i;
	for (i = 0; i < (bitmap->w * bitmap->h); i++) {
		bitmap->data[i] = c;
	}

	return bitmap;
}

void free_bitmap(TTF_Bitmap *bitmap) {
	if (!bitmap) {
		return;
	}
	if (bitmap->data) {
		free(bitmap->data);
	}
	free(bitmap);
}

void bitmap_set(TTF_Bitmap *bitmap, int x, int y, uint32_t c) {
	if (!bitmap || x < 0 || x > bitmap->w || y < 0 || y > bitmap->h) {
		return;
	}
	bitmap->data[y*bitmap->w + x] = c;
}

uint32_t bitmap_get(TTF_Bitmap *bitmap, int x, int y) {
	if (!bitmap || x < 0 || x > bitmap->w || y < 0 || y > bitmap->h) {
		return 0;
	}
	return bitmap->data[y*bitmap->w + x];
}

static inline void set_rgb(png_byte *ptr, uint32_t value) {
	uint8_t *b = (uint8_t *)&value;
	ptr[0] = b[2]; ptr[1] = b[1]; ptr[2] = b[0];
}

int save_bitmap(TTF_Bitmap *bitmap, const char *filename, const char *title) {
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte *row = NULL;

	RETINIT(SUCCESS);

	CHECKFAIL(bitmap, warn("failed to save uninitialized bitmap"));

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	CHECKFAIL(fp, warnerr("failed to open file '%s' for saving bitmap", filename));

	// Initialize the write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	CHECKFAIL(png_ptr, warn("failed to alloc png write struct"));

	// Initialize the info structure
	info_ptr = png_create_info_struct(png_ptr);
	CHECKFAIL(info_ptr, warn("failed to alloc png info struct"));

	// Setup libpng exception handling
	CHECKFAIL(setjmp(png_jmpbuf(png_ptr)) == 0, warn("error occurred during png creation"));

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, bitmap->w, bitmap->h, 8,
			PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set png title
	if (title) {
		png_text png_title;
		png_title.compression = PNG_TEXT_COMPRESSION_NONE;
		png_title.key = (png_charp) "Title";
		png_title.text = (png_charp) title;
		png_set_text(png_ptr, info_ptr, &png_title, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (RBG = 3 bytes / pixel)
	row = (png_byte *) malloc((bitmap->w * 3) * sizeof(*row));
	CHECKFAIL(row, warnerr("failed to alloc png row"));

	// Write image data
	int x, y;
	for (y = 0; y < bitmap->h; y++) {
		for (x = 0; x < bitmap->w; x++) {
			set_rgb(&(row[x*3]), bitmap->data[y*bitmap->w + x]);
		}
		png_write_row(png_ptr, (png_bytep)row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	RETURN(
		if (fp) fclose(fp);
		if (info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		if (png_ptr) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		if (row) free(row);
	);
}
