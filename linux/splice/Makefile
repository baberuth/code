CFLAGS=-O2 -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wshadow -std=c99 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

test: main.c events.c events.h logger.c logger.h list.c
	gcc $(CFLAGS) events.c logger.c main.c -o test

clean:
	rm -f *.o test


