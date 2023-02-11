MYCFLAGS=-Ofast -march=native -Wall -Wformat
CC=clang


1bitvm:	src/1bitvm.c src/1bitvm.h src/config.h
	$(CC) $(MYCFLAGS) $(CFLAGS) -o $@ src/1bitvm.c 

debug:	
	$(CC) -Wall -Wformat -Wextra -g -pedantic -O0 -g3 -o 1bitvm src/1bitvm.c
clean:
	rm 1bitvm
.PHONY: clean

