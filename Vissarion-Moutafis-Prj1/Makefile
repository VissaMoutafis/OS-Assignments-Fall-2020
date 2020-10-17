#	Makefile for the program
#	Written by Vissarion Moutafis sdi1800119

PROGRAM = mngstd

CC = gcc

INCLUDE = ./includes

MODULES = ./modules

CFLAGS += -I$(INCLUDE) -Wall -g3 -Werror

OBJS = main.o 

# -------------------------------------------------------------------
#	change these 2 variables accordingly 
# 	to give your own parameters to the program

# example: 
# INPUT_FILE = ./k22-prj1-test-data/input3000.txt
# CONFIG_FILE = config

# -------------------------------------------------------------------

# if you wanna add command line args un-comment the line below
# ARGS = -i $(INPUT_FILE) -c $(CONFIG_FILE)


include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) 

.PHONY clean :
	rm -rf $(PROGRAM) $(OBJS)

run : $(PROGRAM) 
	./$(PROGRAM) $(ARGS)

.PHONY re-run: clean run

valgrind : $(PROGRAM)
	valgrind ./$(PROGRAM) $(ARGS)