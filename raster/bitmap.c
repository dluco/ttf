#include "bitmap.h"
#include "../utils/utils.h"
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
	bitmap->c = c;

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
	if (!bitmap || !IN(x, 0, bitmap->w-1) || !IN(y, 0, bitmap->h-1)) {
		return;
	}
	bitmap->data[y*bitmap->w + x] = c;
}

uint32_t bitmap_get(TTF_Bitmap *bitmap, int x, int y) {
	if (!bitmap || !IN(x, 0, bitmap->w-1) || !IN(y, 0, bitmap->h-1)) {
		return bitmap->c;
	}
	return bitmap->data[y*bitmap->w + x];
}

TTF_Bitmap *copy_bitmap(TTF_Bitmap *bitmap) {
	if (!bitmap) {
		return NULL;
	}

	TTF_Bitmap *copy = create_bitmap(bitmap->w, bitmap->h, bitmap->c);

	int y;
	for (y = 0; y < bitmap->h; y++) {
		int x;
		for (x = 0; x < bitmap->w; x++) {
			bitmap_set(copy, x, y, bitmap_get(bitmap, x, y));
		}
	}

	return copy;
}

int draw_bitmap(TTF_Bitmap *canvas, TTF_Bitmap *bitmap, int x, int y) {
	CHECKPTR(canvas);
	CHECKPTR(bitmap);

	if (!IN(x, 0, canvas->w-1) || !IN(y, 0, canvas->h-1)) {
		warn("failed to draw bitmap out of bounds");
		return FAILURE;
	}

	/* Out of bounds checking is also done in bitmap_set,_get() */
	for (int yb = 0; yb < bitmap->h; yb++) {
		for (int xb = 0; xb < bitmap->w; xb++) {
			bitmap_set(canvas, x+xb, y+yb, bitmap_get(bitmap, xb, yb));
		}
	}

	return SUCCESS;
}

TTF_Bitmap *combine_bitmaps(TTF_Bitmap *a, TTF_Bitmap *b, uint32_t c) {
	if (!a || !b) {
		return NULL;
	}

	TTF_Bitmap *out = create_bitmap(a->w + b->w, MAX(a->h, b->h), c);
	if (!c) {
		warn("failed to combine bitmaps");
		return NULL;
	}

	int y;
	for (y = 0; y < a->h; y++) {
		int x;
		for (x = 0; x < a->w; x++) {
			bitmap_set(out, x, y, bitmap_get(a, x, y));
		}
	}

	for (y = 0; y < b->h; y++) {
		int x;
		for (x = 0; x < b->w; x++) {
			bitmap_set(out, x + a->w, y, bitmap_get(b, x, y));
		}
	}

	return out;
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

	RETRELEASE(
		/* RELEASE */
		if (row) free(row);
		if (info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		if (png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
		if (fp) fclose(fp);
	);
}
