#	Makefile for the program
#	Written by Vissarion Moutafis sdi1800119

PROGRAM = mngstd

CC = gcc

INCLUDE = ./includes

MODULES = ./modules

CFLAGS += -I$(INCLUDE) -Wall -g3

OBJS = main.o 

# -------------------------------------------------------------------
#	change these 2 variables accordingly 
# 	to give your own parameters to the program

INPUT_FILE = ./testcases/input2.csv
CONFIG_FILE = config

# -------------------------------------------------------------------


ARGS = -i $(INPUT_FILE) -c $(CONFIG_FILE)


include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) 

.PHONY clean :
	rm -rf $(PROGRAM) $(OBJS)

run : $(PROGRAM) 
	./$(PROGRAM) $(ARGS)

