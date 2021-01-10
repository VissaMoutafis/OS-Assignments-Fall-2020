PROGRAM = quic

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

src_element = ./test_dir
dest = ./target_media/phony
ARGS = $(src_element) $(dest)

include $(SRC)/src.inc

$(BUILD)/$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/$(PROGRAM) $(LIBRARIES)
	cp $(BUILD)/$(PROGRAM) ./$(PROGRAM)

$(BIN)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)/$(PROGRAM) $(OBJS) ./$(PROGRAM)

clean-dest: 
	rm -rf ./target_media/*

run-noargs : $(BUILD)/$(PROGRAM)
	$(BUILD)/$(PROGRAM)

run : $(BUILD)/$(PROGRAM)
	$(BUILD)/$(PROGRAM) $(ARGS) 
