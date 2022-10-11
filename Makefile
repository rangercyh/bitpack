.PHONY: all clean

CC = gcc
CFLAGS = $(CFLAG)
CFLAGS += -g3 -O2 -rdynamic -Wall -fPIC -shared -Wextra

bitpack.so: *.c
	$(CC) $(CFLAGS) -o $@ $^

test:
	lua test.lua

clean:
	rm -f *.so

