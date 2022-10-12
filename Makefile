.PHONY: all clean

CC = gcc
CFLAGS = $(CFLAG)
CFLAGS += -g3 -O2 -rdynamic -Wall -fPIC -shared -Wextra

all: bitpack.so
bitpack.so: bitpack.c luabinding.c
	$(CC) $(CFLAGS) -o $@ $^

all: zlib.so
zlib.so: lua_zlib.c
	$(CC) $(CFLAGS) -lz -o $@ $^

test:
	lua test.lua

clean:
	rm -f bitpack.so zlib.so

