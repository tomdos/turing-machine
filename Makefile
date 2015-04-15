CC=gcc
FLAGS=-O0 -ggdb

all: alan

alan: alan.c
	$(CC) $(FLAGS) $^ -o alan

clean:
	rm -f alan
