all: main

OBJS = $(addsuffix .o,$(MODULES))

main: Makefile main.c $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(filter-out Makefile,$^) -ldl -o $@

clean:
	rm -f $(OBJS)
	rm -rf *.dSYM
	rm -f main
