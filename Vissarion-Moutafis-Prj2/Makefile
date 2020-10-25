#	Makefile for the program
#	Written by Vissarion Moutafis sdi1800119

PROGRAM = myprime

CC = gcc

INCLUDE = ./include

SRC = ./src

MODULES = $(SRC)/modules

PRIME-FINDING-PROGS = $(MODULES)/prime-finding-programs

WORKERS = $(MODULES)/workers

INTERNALS = $(MODULES)/internal-processes

ROOT = $(MODULES)/root

CFLAGS += -I$(INCLUDE) -Wall -g3 -Werror

OBJS =  


# if you wanna add command line args un-comment the line below
ARGS = -l 1 -u 10 -w 3

include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) root
	$(CC) $(CFLAGS) $(OBJS) $(SRC)/main.o -o $(PROGRAM)

prime-finding-programs: $(PRIME-FINDING-PROGS)/*
	$(CC) $(PRIME-FINDING-PROGS)/primes1.c -lm -o primes1
	$(CC) $(PRIME-FINDING-PROGS)/primes2.c -lm -o primes2
	$(CC) $(PRIME-FINDING-PROGS)/primes3.c -lm -o primes3

workers: $(WORKERS)/workers.c $(OBJS) prime-finding-programs
	$(CC) $(CFLAGS) $(OBJS) $(WORKERS)/workers.c -o workers

internals: $(INTERNALS)/internals.c $(OBJS) workers
	$(CC) $(CFLAGS) $(OBJS) $(INTERNALS)/internals.c -o internals

root: $(ROOT)/root.c $(OBJS) internals
	$(CC) $(CFLAGS) $(OBJS) $(ROOT)/root.c -o root

clean :
	rm -rf ./$(PROGRAM) $(OBJS) ./primes* ./workers ./root ./internals

run : $(PROGRAM)
	./$(PROGRAM) $(ARGS)

run-no-args:
	./$(PROGRAM)

re-run: clean run

valgrind : $(PROGRAM)
	valgrind ./$(PROGRAM) $(ARGS)
