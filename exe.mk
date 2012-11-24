all: main

OBJS = $(addsuffix .o,$(MODULES))

main: Makefile main.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(filter-out Makefile,$^) -ldl -o $@

clean:
	rm -f $(OBJS)
	rm -rf *.dSYM
	rm -f main
