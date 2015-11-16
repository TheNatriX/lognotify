
CC=gcc
CFLAGS=-O2 -Wall

SRC=main.c xclient.c inotify.c
OBJ=lognotify

all:
	$(CC) $(CFLAGS) -o $(OBJ) $(SRC) -lX11

clean:
	rm -f $(OBJ)

