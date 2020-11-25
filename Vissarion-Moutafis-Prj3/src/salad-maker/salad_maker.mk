PROGRAM := $(MAKE-DIR)/salad-maker

OBJS += ./salad-maker.o

ARGS :=

$(PROGRAM) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)
.PHONY : run
run :
	./$(PROGRAM) $(ARGS)

.PHONY : clean
clean :
	@$(RM) -f $(PROGRAM) ./salad-maker.o