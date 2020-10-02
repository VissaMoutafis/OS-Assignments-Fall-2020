PROGRAM = mngstd

CC = gcc

INCLUDE = ./includes

MODULES = ./modules

CFLAGS += -I$(INCLUDE) -Wall -g3

OBJS = mngstd.o

ARGS = -i ./testcases/input1.csv

include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) 

.PHONY clean :
	rm -rf $(PROGRAM) $(OBJS)

run : $(PROGRAM) 
	./$(PROGRAM) $(ARGS)

