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

CFLAGS += -I$(INCLUDE) -Wall -g3 -Werror -D_GNU_SOURCE

OBJS =  

# if you wanna add command line args un-comment the line below
ARGS = -l 1 -u 100 -w 3

include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) root internals workers primes1 primes2 primes3 $(SRC)/main.o
	$(CC) $(CFLAGS) $(OBJS) $(SRC)/main.o -o $(PROGRAM)

primes1: $(PRIME-FINDING-PROGS)/primes1.o
	$(CC) $(PRIME-FINDING-PROGS)/primes1.o -lm -o primes1

primes2: $(PRIME-FINDING-PROGS)/primes2.o
	$(CC) $(PRIME-FINDING-PROGS)/primes2.o -lm -o primes2

primes3: $(PRIME-FINDING-PROGS)/primes3.o
	$(CC) $(PRIME-FINDING-PROGS)/primes3.o -lm -o primes3

workers: $(WORKERS)/workers.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(WORKERS)/workers.o -o workers

internals: $(INTERNALS)/internals.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(INTERNALS)/internals.o -o internals

root: $(ROOT)/root.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(ROOT)/root.o -o root

clean :
	rm -rf ./$(PROGRAM) $(OBJS) ./primes* ./workers ./root ./internals

run : $(PROGRAM)
	./$(PROGRAM) $(ARGS)

run-no-args:
	./$(PROGRAM)

re-run: clean run

valgrind : $(PROGRAM)
	valgrind ./$(PROGRAM) $(ARGS)
