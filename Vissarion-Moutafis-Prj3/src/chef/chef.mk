PROGRAM := $(MAKE-DIR)/chef

ARGS :=

$(PROGRAM) : $(OBJS) ./chef.o
	$(CC) $(CFLAGS) $(OBJS) ./chef.o -o $(PROGRAM) $(LIBS)

run :
	./$(PROGRAM) $(ARGS)

clean :
	$@(RM) -rf $(PROGRAM)