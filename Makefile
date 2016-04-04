.DEFAULT_GOAL := all

include config.mk

SRC := $(wildcard *.c) $(wildcard $(patsubst %,%/*.c, $(MODULES)))

OBJS := $(SRC:.c=.o)
DOBJS := $(SRC:.c=.do)
DEPS := $(SRC:.c=.d)

all: release

release: CFLAGS += $(CFLAGS.release)
release: LDFLAGS += $(LDFLAGS.release)
release: $(PROG)

debug: CFLAGS += $(CFLAGS.debug)
debug: LDFLAGS += $(LDFLAGS.debug)
debug: $(PROG)-debug

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PROG)-debug: $(DOBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile and generate dependency info
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CPP) -MM $(DEPFLAGS) -MT $@ $*.c -MF $*.d

%.do: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.do
	$(CPP) -MM $(DEPFLAGS) -MT $@ $*.c -MF $*.d

# Include dependency info for *existing* object files
-include $(DEPS)

clean:
	$(RM) -rf $(PROG) $(PROG)-debug $(OBJS) $(DOBJS) $(DEPS)

tags:
	ctags -R -f .tags .

.PHONY: all release debug clean tags
