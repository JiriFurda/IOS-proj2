CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic
#-Werror
LFLAGS=-lpthread
FILE=proj2

all: build

build:
	$(CC) $(CFLAGS) $(FILE).c -o $(FILE) $(LFLAGS)
	
run: build
	./proj2 1 2 0 0 0 0
	echo --------------------------------
	cat proj2.out


run2: build
	./proj2 1 4 1 10 5 5
	echo --------------------------------
	cat proj2.out
