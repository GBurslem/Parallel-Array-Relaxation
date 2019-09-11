CC=gcc
 CFLAGS= -pthread -Wall -Wextra -Wconversion -std=c99

mainmake: main.c
	$(CC) -o main main.c $(CFLAGS)