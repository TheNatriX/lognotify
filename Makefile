
CC=gcc
CFLAGS=-O2 -Wall

SRC=main.c xclient.c

all:
	$(CC) $(CFLAGS) -o lognotify $(SRC) -lX11

clean:
	rm lognotify

