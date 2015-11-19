
CC=gcc
CFLAGS=-O2 -Wall -g3

SRC=main.c xclient.c inotify.c daemon.c
OBJ=lognotify

all:
	$(CC) $(CFLAGS) -o $(OBJ) $(SRC) -lX11

clean:
	rm -f $(OBJ)

