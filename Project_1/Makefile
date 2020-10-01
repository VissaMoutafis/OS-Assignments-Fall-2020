PROGRAM = testHashtable

CC = gcc

INCLUDE = ./includes

MODULES = ./modules

CFLAGS += -I$(INCLUDE) -Wall -g3 -Ofast

OBJS = testHashtable.o

ARGS = -i input.txt

include $(MODULES)/modules.inc

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM)

.PHONY clean :
	rm -rf $(PROGRAM) $(OBJS)

run : $(PROGRAM)
	./$(PROGRAM)

