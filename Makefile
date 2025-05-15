CC=clang
CFLAGS=-Wall -Wextra -pedantic -std=gnu99 -ggdb -Werror=return-type

server: server.c string_operations.c string_operations.h file.h file.c
	${CC} ${CFLAGS} server.c string_operations.c file.c -o $@

.PHONY: clean
clean:
	rm -f server
