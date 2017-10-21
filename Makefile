# C compiler to use; clang works as well
CC?=gcc

# Flags to pass to the C compiler; these are simply the default
# Arch Linux C compiler flags
CFLAGS?= -O2 -fstack-protector-strong

# Flags to pass to the linker; required to link against libbsd
LDFLAGS+= -lbsd

all:
	$(CC) brightlight.c -o brightlight $(CFLAGS) $(LDFLAGS)
