CC="distcc"
compile:
	$(CC) -o 1bitvm src/1bitvm.c -O0 -march=native
