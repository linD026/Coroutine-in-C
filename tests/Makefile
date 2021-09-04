BUILD_DIR = $(shell pwd)
SRC = ../src
CC ?= gcc
CFLAG:=-g 
CFLAG+=-Wall
CFLAG+=-fsanitize=address

check:
	$(CC) -o test test_coroutine.c $(SRC)/runqueue.c $(SRC)/sched.c $(SRC)/coroutine.c  $(CFLAG)
	

rq:
	$(CC) -o test test_fifo_runqueue.c $(SRC)/runqueue.c $(CFLAG)

