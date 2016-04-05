#ifndef UTILS_H
#define UTILS_H

#include "../base/consts.h"

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
		goto _release;						\
	}										\
}

#define RETURN(RELEASE) {	\
	return SUCCESS;			\
							\
	_release:				\
		RELEASE;			\
	return _result;			\
}

void warn(const char *fmt, ...);
void warnerr(const char *fmt, ...);

#endif /* UTILS_H */
