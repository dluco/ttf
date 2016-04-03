include config.mk

SRC := $(wildcard *.c) $(wildcard $(patsubst %, %/*.c, $(MODULES)))

OBJS := $(SRC:.c=.o)
DOBJS := $(SRC:.c=.do)
DEPS := $(SRC:.c=.d)

all: release

release: CFLAGS += $(RCFLAGS)
release: LFLAGS += $(RLFLAGS)
release: $(PROJECT)

debug: CFLAGS += $(DCFLAGS)
debug: LFLAGS += $(DLFLAGS)
debug: default $(PROJECT)-debug

$(PROJECT): $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

$(PROJECT)-debug: $(DOBJS)
	$(CC) $(LFLAGS) $^ -o $@

# Include dependency info for *existing* .o files
-include $(DEPS)

# Compile and generate dependency info
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	@$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

%.do: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.do
	@$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	$(RM) -rf $(PROJECT) $(PROJECT)-debug $(OBJS) $(DOBJS) $(DEPS)

tags:
	ctags -R -f .tags .

.PHONY: all release debug clean tags
