MYCFLAGS=-O3 -march=native -Wall -Wformat
CC=clang

default: 1bitvm


.PHONY: 1bitvm
1bitvm:	src/1bitvm.c
	$(CC) $(MYCFLAGS) $(CFLAGS) -o $@ src/1bitvm.c 
debug:	
	$(CC) -Wall -Wformat -Wextra -g -pedantic -O0 -g3 -o 1bitvm src/1bitvm.c


