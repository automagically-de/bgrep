CC = gcc
CFLAGS = -Wall -ansi -pedantic

all: bgrep

bgrep: bgrep.c
	${CC} -o $@ ${CFLAGS} $<

clean:
	rm -f bgrep
