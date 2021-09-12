BUILD_DIR = $(shell pwd)
CC ?= gcc
CFLAG:=-g 
CFLAG+=-Wall
CFLAG+=-O1

SRC:=src/runqueue.c
SRC+=src/sched.c
SRC+=src/coroutine.c
SRC+=src/rbtree.c

OBJ=$(SRC:.c=.o)

src/%.o: %.c
	$(CC) -c $< $(CFLAG)


all: static
	cat src/context.h > coroutine.h
	cat src/coroutine.h >> coroutine.h

static: $(OBJ)
	ar crsv coroutine.a $(OBJ)

clean:
	rm -f src/*.o
	rm -f coroutine.a coroutine.h

indent:
	clang-format -i src/*[.ch]
