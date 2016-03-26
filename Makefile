include config.mk

SRC := $(wildcard *.c)
OBJS := ${SRC:.c=.o}
DOBJS := ${SRC:.c=.do}
DEPS := ${SRC:.c=.d}

all: ${PROJECT}

debug: ${PROJECT}-debug

${PROJECT}: ${OBJS}
	${CC} ${LFLAGS} ${SFLAGS} $^ -o $@

${PROJECT}-debug: ${DOBJS}
	${CC} ${LFLAGS} $^ -o $@

# Include dependency info for *existing* .o files
-include ${DEPS}

# Compile and generate dependency info
%.o: %.c
	${CC} -c ${CFLAGS} ${RFLAGS} $*.c -o $*.o
	@${CC} -MM ${CFLAGS} ${RFLAGS} $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

%.do: %.c
	${CC} -c ${CFLAGS} ${DFLAGS} $*.c -o $*.do
	@${CC} -MM ${CFLAGS} $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	rm -rf ${PROJECT} ${PROJECT}-debug ${OBJS} ${DOBJS} ${DEPS}

tags:
	ctags -R -f .tags .

.PHONY: all clean tags
