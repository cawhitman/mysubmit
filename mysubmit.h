CC=gcc
CFLAGS=-I.

mysubmit: mysubmit.c
	$(CC) -o mysubmit mysubmit.c $(CFLAGS)