PROG := ttf
VERSION := 0.0.0

MODULES := parse raster

# C compilter
CC := gcc

# C preprocessor flags
CPPFLAGS :=

# C compiler flags
CFLAGS := -Wall -Wextra -pedantic -std=c99

# Link flags
LDFLAGS := -Wl,--as-needed

# Libraries
LDLIBS := -lm -lpng

# Dependency creation flags
DEPENDFLAGS := -MG -MP

# Release flags
CFLAGS.release := -O2 -flto
LDFLAGS.release := -O2 -flto -s

# Debug flags
CFLAGS.debug := -g -O0 -DDEBUG
LDFLAGS.debug := -O0
