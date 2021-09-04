BUILD_DIR = $(shell pwd)
CC ?= gcc
CFLAG:=-g 
CFLAG+=-Wall

SRC:=src/runqueue.c
SRC+=src/sched.c
SRC+=src/coroutine.c

all:
	$(CC) -c $(SRC) $(CFLAG)

clean:
	rm -f *.o

indent:
	clang-format -i src/*[.ch]
