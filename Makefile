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

src/%.o: src/%.c
	$(CC) -c $< $(CFLAG) -o $@

all: static
	cat src/context.h > coroutine.h
	cat src/coroutine.h >> coroutine.h
	rm -f src/*.o

static: $(OBJ)
	ar crsv coroutine.a $(OBJ)

clean:
	rm -f coroutine.a coroutine.h
	make -C tests clean

indent:
	clang-format -i src/*.[ch]
