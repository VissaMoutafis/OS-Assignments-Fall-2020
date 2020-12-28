PROGRAM = main

CC = gcc

INCLUDE = ./include

SRC  = ./src

BIN = ./bin

BUILD = ./build

OBJS = 

LIBRARIES = 

CFLAGS = -g3 -Wall -Werror -I $(INCLUDE)

TESTCASE = ./testcases

testcase-num := 1K

ARGS = -i $(TESTCASE)/records$($<).txt

src_element = ./test_dir
dest = ./target_media/phony
ARGS = $(src_element) $(dest)

include $(SRC)/src.inc

$(BUILD)/$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)

$(BIN)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)/$(PROGRAM) $(OBJS)

clean-dest: 
	rm -rf $(dest)/*

run-noargs : $(BUILD)/$(PROGRAM)
	$(BUILD)/$(PROGRAM)

run : $(BUILD)/$(PROGRAM)
	$(BUILD)/$(PROGRAM) $(ARGS)
