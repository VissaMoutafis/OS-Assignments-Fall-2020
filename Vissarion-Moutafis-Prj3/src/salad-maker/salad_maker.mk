PROGRAM := $(MAKE-DIR)/salad-maker

ARGS :=

$(PROGRAM) : $(OBJS) ./salad-maker.o
	$(CC) $(CFLAGS) $(OBJS) ./salad-maker.o -o $(PROGRAM) $(LIBS)

run :
	./$(PROGRAM) $(ARGS)

clean :
	$@(RM) -rf $(PROGRAM)