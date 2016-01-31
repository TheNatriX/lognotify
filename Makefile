
CC=gcc
CFLAGS=-O2 -Wall -pedantic
DEBUG=-DDEBUG -g3

SRC=main.c xclient.c inotify.c daemon.c
OBJ=lognotify

all:
	$(CC) $(CFLAGS) -o $(OBJ) $(SRC) -lX11

debug:
	$(CC) $(CFLAGS) $(DEBUG) -o $(OBJ) $(SRC) -lX11

clean:
	rm -f $(OBJ)

