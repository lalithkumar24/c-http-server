CC=clang
CFLAGS=-Wall -Wextra -pedantic -std=c99 -ggdb -Werror=return-type

server: server.c string_operations.c string_operations.h
	${CC} ${CFLAGS} server.c string_operations.c -o $@

.PHONY: clean
clean:
	rm -f server
