BUILD_DIR = $(shell pwd)
SRC = ../src
CC ?= gcc
CFLAG:=-g 
CFLAG+=-Wall
CFLAG+=-fsanitize=address

LIB = ../coroutine.a

check:
	$(CC) -o test test_coroutine.c $(LIB)  $(CFLAG)

indent:
	clang-format -i *.[ch]	

clean:
	rm -f test
	rm -rf test.dSYM

rq:
	$(CC) -o test test_fifo_runqueue.c $(LIB) $(CFLAG)

