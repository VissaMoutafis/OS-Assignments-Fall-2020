PROGRAM := $(MAKE-DIR)/salad-maker

OBJS += ./salad-maker.o

ARGS :=

$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

run :
	./$(PROGRAM) $(ARGS)

clean :
	@$(RM) -f $(PROGRAM) ./salad-maker.o