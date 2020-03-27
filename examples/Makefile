CC = gcc
#CFLAGS = -O0 -Wall -Wextra -std=c11 -pedantic-errors #todo: compile with pedantic errors
CFLAGS = -O0 -std=c11

all: repl ape

.PHONY: repl
repl: repl.c ../ape.c
	$(CC) $(CFLAGS) -DAPE_REPL_MAIN -I.. -o $@ repl.c ../ape.c

.PHONY: ape
ape: ape.c ../ape.c
	$(CC) $(CFLAGS) -I.. -o $@ ape.c ../ape.c
	
clean:
	rm -f repl ape *.o *.dSYM