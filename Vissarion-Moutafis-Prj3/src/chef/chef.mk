PROGRAM := $(MAKE-DIR)/chef

OBJS += chef.o

ARGS := -m 0 -n 3

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

.PHONY : run
run :
	./$(PROGRAM) $(ARGS)

.PHONY : clean
clean :
	@$(RM) -f $(PROGRAM) ./chef.o