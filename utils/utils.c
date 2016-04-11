#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

int mod(int a, int b) {
	return ((a & b) + b) % b;
}

float symroundf(float f) {
	return (f < 0) ? floorf(f) : ceilf(f);
}

void warn(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
}

void warnerr(const char *fmt, ...) {
	int errno_save = errno;
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (errno_save != 0) {
		fprintf(stderr, ": %s\n", strerror(errno_save));
	} else {
		fprintf(stderr, "\n");
	}
}
