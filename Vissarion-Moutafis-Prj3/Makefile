# the top directory
MAKE-DIR = $(PWD)

# directories of src, includes and logs
SRC := $(MAKE-DIR)/src
INCLUDE := $(MAKE-DIR)/include
LOGS := $(MAKE-DIR)/logs

# directories of the .mk files
CHEF-DIR := $(SRC)/chef
SALAD-MAKER-DIR := $(SRC)/salad-maker

CC := gcc

CFLAGS := -g3 -Wall -I $(INCLUDE)

LIBS := -lpthread

OBJS := 

include $(SRC)/src.inc 

export MAKE-DIR CC CFLAGS LIBS OBJS

.PHONY : all
all : 
	@$(MAKE) -C $(CHEF-DIR) -f chef.mk
	@$(MAKE) -C $(SALAD-MAKER-DIR) -f salad_maker.mk

.PHONY : chef
chef : 
	@$(MAKE) -C $(CHEF-DIR) -f chef.mk

.PHONY : salad-maker
salad-maker :
	@$(MAKE) -C $(SALAD-MAKER-DIR) -f salad_maker.mk

.PHONY : clean
clean :
	@$(MAKE) -C $(CHEF-DIR) -f chef.mk clean
	@$(MAKE) -C $(SALAD-MAKER-DIR) -f salad_maker.mk clean
	@$(RM) -f $(OBJS)

.PHONY : clean-logs
clean-logs :
	@$(RM) -f $(LOGS)/*