BUILD_DIR = $(shell pwd)
CC ?= gcc
CFLAG:=-g 
CFLAG+=-Wall

SRC:=src/runqueue.c
SRC+=src/sched.c
SRC+=src/coroutine.c

OBJ=$(SRC:.c=.o)

src/%.o: %.c
	$(CC) -c $< $(CFLAG)


all: static

static: $(OBJ)
	ar crsv coroutine.a $(OBJ)

clean:
	rm -f src/*.o
	rm -f coroutine.a

indent:
	clang-format -i src/*[.ch]
