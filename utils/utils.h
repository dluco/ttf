#ifndef UTILS_H
#define UTILS_H

#include "../base/consts.h"

#define PASS ;

#define RETINIT(VAL) \
	int _result = VAL

#define CHECKPTR(PTR) {	\
	if (!PTR) {			\
		return FAILURE;	\
	}					\
}

#define CHECKFAIL(VAL, FAIL) {				\
	_result = (VAL) ? SUCCESS : FAILURE;	\
	if (_result != SUCCESS) {				\
		FAIL;								\
		goto _failure;						\
	}										\
}

#define RETFAILRELEASE(FAIL, RELEASE) {	\
	goto _release;						\
_failure:								\
	FAIL;								\
_release:								\
	RELEASE;							\
	return _result;						\
}

#define RET \
	RETFAILRELEASE(PASS, PASS)

#define RETFAIL(FAIL) \
	RETFAILRELEASE(FAIL, PASS)

#define RETRELEASE(RELEASE) \
	RETFAILRELEASE(PASS, RELEASE)

#define IN(x, a, b) ((x) >= (a) && (x) <= (b))

int mod(int a, int b);
float symroundf(float f);

void warn(const char *fmt, ...);
void warnerr(const char *fmt, ...);

#endif /* UTILS_H */
