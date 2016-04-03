PROJECT := ttf
VERSION := 0.0.0

MODULES := parse raster

# C compilter
CC := gcc

# C flags
CFLAGS := -Wall -Wextra -pedantic -std=c99

# Link flags
LFLAGS := -lm -lpng

# Release flags
RCFLAGS := -O2 -flto
RLFLAGS := -O2 -flto -s

# Debug flags
DCFLAGS := -g -O0 -DDEBUG
DLFLAGS := -O0
