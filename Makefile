CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS=-lpthread

proj2: proj2.c
	$(CC) $(CFLAGS) proj2.c -o proj2 $(LFLAGS)
