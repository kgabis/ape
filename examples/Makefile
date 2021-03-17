CC = gcc
#CFLAGS = -O0 -Wall -Wextra -std=c11 -pedantic-errors #todo: compile with pedantic errors
CFLAGS = -O0 -std=c11

all: repl ape api

.PHONY: repl ape api

repl: repl.c ../ape.c
	$(CC) $(CFLAGS) -DAPE_REPL_MAIN -I.. -o $@ repl.c ../ape.c -lm

ape: ape.c ../ape.c
	$(CC) $(CFLAGS) -I.. -o $@ ape.c ../ape.c -lm
	
api: api.c ../ape.c
	$(CC) $(CFLAGS) -I.. -o $@ api.c ../ape.c -lm

clean:
	rm -f repl ape api *.o *.dSYM