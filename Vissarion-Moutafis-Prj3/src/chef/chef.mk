PROGRAM := $(MAKE-DIR)/chef

OBJS += chef.o

ARGS := -m 0 -n 3

$(PROGRAM) : $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

run :
	./$(PROGRAM) $(ARGS)

clean :
	@$(RM) -f $(PROGRAM) ./chef.o