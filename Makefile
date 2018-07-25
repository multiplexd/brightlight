# C compiler to use; clang works as well
CC?=gcc

# Flags to pass to the C compiler
CFLAGS?= -O2 -Wall -Werror -std=c99 -D_POSIX_C_SOURCE=200809L

# Flags to pass to the linker; required to link against libbsd
LDFLAGS+= -lbsd

all:
	$(CC) brightlight.c -o brightlight $(CFLAGS) $(LDFLAGS)

clean:
	rm -f brightlight
