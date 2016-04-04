#ifndef UTILS_H
#define UTILS_H

#define SUCCESS	1
#define FAILURE 0

#define RETINIT(VAL) \
	int _result = VAL

#define CHECKPTR(PTR) {		\
	if (!PTR) {				\
		_result = FAILURE;	\
		goto _release;		\
	}						\
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
